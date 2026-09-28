#include "xw_dos94/render/world.h"
#include "xw/flight/death_star.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/anim.h"
#include "xw/flight/object/create.h"
#include "xw/flight/player/user.h"
#include "xw/flight/replay/replay.h"
#include "xw/flight/xw.h"
#include "xw/render/flight_view.h"
#include "xw_dos94/flight/special_world.h"
#include "xw_dos94/render/display.h"
#include "xw_dos94/render/draw.h"
#include "xw_dos94/render/view.h"
#include "xw_runtime/runtime/flight_camera.h"
#include "xw_runtime/runtime/presentation.h"
#include "xw_runtime/snapshot/render_capture.h"
#include "xw_runtime/snapshot/render_objects.h"
#include "xw_runtime/timing/flight_timing.h"
#include <string.h>

static int32_t subtract(int32_t a, int32_t b) { return (int32_t)((uint32_t)a - (uint32_t)b); }

static int32_t absolute(int32_t x) { return x < 0 ? (int32_t)(0u - (uint32_t)x) : x; }

static void world_position(Dos94EyePoint position) {
	Dos94_display->bitmaps.world = (Dos94EyePoint) { subtract(position.x, g_flightCamera.worldPosition.x),
													 subtract(position.y, g_flightCamera.worldPosition.y),
													 subtract(position.z, g_flightCamera.worldPosition.z) };
	g_camRelWorldX = Dos94_display->bitmaps.world.x;
	g_camRelWorldY = Dos94_display->bitmaps.world.y;
	g_camRelWorldZ = Dos94_display->bitmaps.world.z;
}

static void eye_coordinate(unsigned axis) {
	Dos94EyePoint* world = &Dos94_display->bitmaps.world;
	int32_t value = Dos94_transfm2_geteye(Dos94_display->draw.camera.matrix[axis], world);
	if (!axis)
		g_objectViewX = Dos94_display->bitmaps.eye.x = value;
	else if (axis == 1)
		g_objectViewY = Dos94_display->bitmaps.eye.y = value;
	else
		g_objectViewZ = Dos94_display->bitmaps.eye.z = value;
}

/* DOS94 0x68115E/0x6812C6: rejected probes retain uncomputed eye axes. */
static bool check_eye(Dos94EyePoint position, uint16_t radius) {
	world_position(position);
	eye_coordinate(2);
	int32_t depth = (int32_t)((uint32_t)g_objectViewZ + radius);
	if (depth < 0 || (depth >> 8) > radius)
		return false;
	eye_coordinate(0);
	if (subtract(absolute(g_objectViewX), radius) > depth)
		return false;
	eye_coordinate(1);
	return subtract(absolute(g_objectViewY), radius) <= depth;
}

static bool craft_eye(ObjectRecord* object, uint16_t radius) {
	world_position((Dos94EyePoint) { object->worldX, object->worldY, object->worldZ });
	eye_coordinate(0);
	eye_coordinate(1);
	eye_coordinate(2);
	g_curCraft = object->instanceData;
	if (!g_curCraft)
		return false;
	g_curCraft->viewX = g_objectViewX;
	g_curCraft->viewY = g_objectViewY;
	g_curCraft->viewZ = g_objectViewZ;
	int32_t depth = (int32_t)((uint32_t)g_objectViewZ + radius);
	return (g_objectViewZ >> 8) < radius && depth > 0 && subtract(absolute(g_objectViewX), radius) < depth &&
		   subtract(absolute(g_objectViewY), radius) < depth;
}

static void mobile_objects(void) {
	for (unsigned i = 0; i < 116; ++i) {
		if (i == 108 && (!g_debrisEnabled || g_hyperspaceflag))
			break;
		if (i == g_flightCamera.focusObjectRef && !g_flightCamera.externalViewActive && !g_replayviewmode)
			continue;
		ObjectRecord* object = &g_objectTable[i];
		if (!object->objectType)
			continue;
		Dos94Model* model = Dos94Assets_Model(object->objectType);
		if (!model)
			continue;
		uint16_t radius = model->metadata.maxBoundsExtent;
		if (object->objectType == 13 || object->objectType == 16)
			radius = (uint16_t)(radius * 4u);
		Dos94_display->draw.sphereRadius = radius;
		g_renderSphereRadius = radius;
		if (object->genusId <= 4) {
			if (!craft_eye(object, radius))
				continue;
		} else if (object->genusId == 5 || object->genusId == 6 || object->genusId == 10 ||
				   object->genusId == 13) {
			if (!check_eye((Dos94EyePoint) { object->worldX, object->worldY, object->worldZ }, radius))
				continue;
		} else
			continue;
		Dos94_fview_newcalcrotate(object->roll, object->pitch, object->yaw, 0, object);
		if (object->genusId <= 4)
			Dos94_DRAW_drawcomplexobject(i);
		else if (object->genusId <= 6)
			Dos94_DRAW_drawlaser(i);
		else
			Dos94_ANIM_drawverysimpleobject(i);
	}
}

static void hyperspace_objects(unsigned index) {
	XwMissionObjectRecord* object = &g_missionObjects[index];
	int16_t x = object->worldX, y = object->worldY, z = object->worldZ;
	Dos94_display->draw.sphereRadius = 0xFFFF;
	check_eye((Dos94EyePoint) { x * 256, y * 256, z * 256 }, 0xFFFF);
	Dos94_DRAW_drawhyperstar(index);
	z = (int16_t)-z;
	check_eye((Dos94EyePoint) { x * 256, y * 256, z * 256 }, 0xFFFF);
	Dos94_DRAW_drawhyperstar(index);
	++Dos94_display->raster.flatObjectNumber;
	if (index >= g_hyperspaceEffectObjectCount / 2)
		return;
	x = (int16_t)-x;
	x >>= 1;
	z >>= 1;
	check_eye((Dos94EyePoint) { x * 256, y * 256, z * 256 }, 0xFFFF);
	Dos94_DRAW_drawhyperstar(index);
	z = (int16_t)-z;
	z >>= 1;
	x >>= 1;
	check_eye((Dos94EyePoint) { x * 256, y * 256, z * 256 }, 0xFFFF);
	Dos94_DRAW_drawhyperstar(index);
	++Dos94_display->raster.flatObjectNumber;
}

static void mission_objects(void) {
	bool hyperspace = g_hyperspaceflag == 3 || g_hyperspaceflag == 5;
	for (unsigned i = 0; i < 64; ++i) {
		if (hyperspace) {
			if (i < g_hyperspaceEffectObjectCount)
				hyperspace_objects(i);
			continue;
		}
		XwMissionObjectRecord* object = &g_missionObjects[i];
		if (!object->objectType || object->genusId < 7 || (object->genusId > 10 && object->genusId != 14))
			continue;
		Dos94Model* model = Dos94Assets_Model(object->objectType);
		if (!model)
			continue;
		uint16_t radius = model->metadata.maxBoundsExtent;
		g_renderSphereRadius = radius;
		Dos94_display->draw.sphereRadius = radius;
		if (!check_eye((Dos94EyePoint) { object->worldX * 256, object->worldY * 256, object->worldZ * 256 },
					   radius))
			continue;
		if (object->objectType >= 33 && object->objectType <= 38) {
			uint16_t elapsed = XwFlightTiming_MissionPoseElapsed(i);
			if (elapsed) {
				object->rollAngle8 += (uint16_t)((i >> 4) * elapsed) >> 4;
				object->pitchAngle8 += (uint16_t)((i >> 3) * elapsed) >> 5;
				object->yawAngle8 += (uint16_t)((4 - (i >> 4)) * elapsed) >> 4;
			}
		}
		XwRenderObjects_MissionPose(i);
		if (object->genusId == 14)
			g_transformLightDirectionToObjectSpace = 0;
		Dos94_fview_newcalcrotate((int16_t)(object->rollAngle8 << 8), (int16_t)(object->pitchAngle8 << 8),
								  (int16_t)(object->yawAngle8 << 8), 0, NULL);
		g_transformLightDirectionToObjectSpace = 1;
		if (object->genusId == 14)
			Dos94_gate_DrawCourseObject(i);
		else
			Dos94_static_drawstaticobject(i);
	}
}

/* DOS94 0x680826: camera, backdrop, objects, bitmap flush, VGA sweep, stars. */
void Dos94_Xw_updatescreen(void) {
	Dos94Display* d = Dos94_display;
	if (!d || !d->ready)
		return;
	XwFlightCamera_Update();
	XwRenderCapture_CaptureWorld();
	if (d->fullUpdate || g_textureCacheFlushPending) {
		Dos94_LOGBUF2_clearbuffer(d->raster.backgroundColor);
		Dos94_xtrans2_clearruntable();
		d->fullUpdate = false;
		g_textureCacheFlushPending = 0;
	}
	Dos94_XTRANS2_initxtrans();
	d->draw.target = g_targetHighlightObjectAndBlinkBits;
	Dos94_backdrp2_backdrop();
	if (g_deathStarSurfaceModeActive &&
		(d->draw.camera.matrix[2][2] < 0x6000 || g_flightCamera.worldPosition.z < 0))
		Dos94_DeathStar_DrawSurfaceAndTrench();
	Dos94BitmapQueue_Begin(&d->bitmaps);
	mobile_objects();
	mission_objects();
	Dos94_ANIM_sort_and_draw_bitmaps();
	bool surfaceBackground = g_deathStarSurfaceModeActive && d->draw.camera.matrix[2][2] < (int16_t)0xA000;
	if (surfaceBackground)
		d->raster.backgroundColor = 0xB1;
	Dos94_XTRANS2_drawxtrans();
	d->raster.backgroundColor = 0xF7;
	if (!surfaceBackground &&
		(g_deathStarSurfaceModeActive || (g_hyperspaceflag != 3 && g_hyperspaceflag != 5)))
		Dos94_RTSVGA2_drawstars();
	g_objectViewX = d->bitmaps.eye.x;
	g_objectViewY = d->bitmaps.eye.y;
	g_objectViewZ = d->bitmaps.eye.z;
	g_camRelWorldX = d->bitmaps.world.x;
	g_camRelWorldY = d->bitmaps.world.y;
	g_camRelWorldZ = d->bitmaps.world.z;
}

void Dos94Renderer_InvalidateAssets(void) {
	Dos94Display* display = Dos94_display;
	if (!display)
		return;
	/* Keep cockpit resources/mask and owned buffers; rebuild all derived drawing state. */
	memset(&display->draw, 0, sizeof display->draw);
	memset(&display->projection, 0, sizeof display->projection);
	memset(&display->bitmaps, 0, sizeof display->bitmaps);
	memset(&display->sky, 0, sizeof display->sky);
	memset(&display->rotation, 0, sizeof display->rotation);
	memset(&display->sweep, 0, sizeof display->sweep);
	Dos94Raster_Init(&display->raster);
	display->generation = 0;
	/* The next scene pass clears the logical pixels and span cache for this viewport. */
	display->fullUpdate = true;
	XwPresentation_Invalidate();
}
