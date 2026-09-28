#include "xw_dos94/flight/object/collision.h"
#include "xw/assets/model_mesh.h"
#include "xw/flight/fview.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/collide.h"
#include "xw/flight/object/create.h"
#include "xw/flight/player/user.h"
#include "xw/flight/xw.h"
#include "xw_dos94/render/detail.h"
#include "xw_dos94/render/fview.h"

static int32_t difference(int32_t a, int32_t b) { return (int32_t)((uint32_t)a - (uint32_t)b); }

/* DOS always halves once, even when the high-word magnitude mask is zero. */
static uint8_t scale_segment(int32_t points[2][3]) {
	uint16_t magnitude = 0;
	for (int p = 0; p < 2; ++p)
		for (int i = 0; i < 3; ++i) {
			int16_t word = (int16_t)(points[p][i] >> 14);
			magnitude |= word < 0 ? (uint16_t)-word : (uint16_t)word;
		}
	uint8_t shift = 0;
	do {
		++shift;
		magnitude >>= 1;
		for (int p = 0; p < 2; ++p)
			for (int i = 0; i < 3; ++i)
				points[p][i] >>= 1;
	} while (magnitude);
	return shift;
}

static void relative_segment(int32_t points[2][3], int32_t x, int32_t y, int32_t z) {
	points[0][0] = difference(g_collisionProbeWorldX, x);
	points[0][1] = difference(g_collisionProbeWorldY, y);
	points[0][2] = difference(g_collisionProbeWorldZ, z);
	points[1][0] = difference(g_collisionSegmentStartWorldX, x);
	points[1][1] = difference(g_collisionSegmentStartWorldY, y);
	points[1][2] = difference(g_collisionSegmentStartWorldZ, z);
}

static void rotate_segment(const int32_t points[2][3], const ObjectRecord* object, int16_t local[2][3]) {
	int16_t axes[3][3] = { { object->cachedSideX, object->cachedSideY, object->cachedSideZ },
						   { object->cachedForwardX, object->cachedForwardY, object->cachedForwardZ },
						   { object->cachedUpX, object->cachedUpY, object->cachedUpZ } };
	for (int p = 0; p < 2; ++p) {
		int16_t v[3] = { (int16_t)points[p][0], (int16_t)points[p][1], (int16_t)points[p][2] };
		for (int i = 0; i < 3; ++i)
			local[p][i] = Dos94_math2_dot3Q15(axes[i], v);
		local[p][1] = (int16_t)-local[p][1];
	}
}

void Dos94_collide_hitoffsets(uint16_t fraction) {
	int16_t x = difference(g_collisionProbeWorldX, g_collisionSegmentStartWorldX);
	int16_t y = difference(g_collisionProbeWorldY, g_collisionSegmentStartWorldY);
	int16_t z = difference(g_collisionProbeWorldZ, g_collisionSegmentStartWorldZ);
	g_collisionHitOffsetX = (int16_t)(((int16_t)fraction * x) >> 15);
	g_collisionHitOffsetY = (int16_t)(((int16_t)fraction * y) >> 15);
	g_collisionHitOffsetZ = (int16_t)(((int16_t)fraction * z) >> 15);
}

/* DOS94 0x7C0E6E: descending component order, first contained face. */
int16_t Dos94_starship_checkstarshiphit(uint16_t source, uint16_t target) {
	(void)source;
	ObjectRecord* object = &g_objectTable[target];
	g_curCraft = object->instanceData;
	int32_t points[2][3];
	int16_t local[2][3];
	relative_segment(points, object->worldX, object->worldY, object->worldZ);
	uint8_t shift = scale_segment(points);
	if (object->objectType != 13 && object->objectType != 16)
		shift += 2;
	if (object->orientMatrixDirty) {
		fview_calcrotatemove(object->pitch, object->yaw, object);
		fview_calcrotateorient(object->roll, 0, object);
	}
	rotate_segment(points, object, local);
	const Dos94Model* model = Dos94Assets_Model(object->objectType);
	if (!model || !g_curCraft)
		return 0;
	for (int component = model->metadata.componentCount - 1; component >= 0; --component) {
		if (!g_curCraft->componentHp[component])
			continue;
		uint16_t count;
		const Dos94Lod* lod = Dos94_DRAW_getcomponentptr(object->objectType, component, &count);
		if (!lod || !count)
			continue;
		unsigned threshold = object->objectType == 14 && component == 2 ? 35 : 16;
		unsigned index = 0;
		while (index + 1 < count && lod[index].maxDepth != INT32_MAX &&
			   lod[index].mesh.faceCount >= threshold)
			++index;
		uint16_t fraction = Dos94_COLLIDE_checkhitpolygons(&lod[index].mesh, local[0], local[1], shift - 1);
		if (fraction) {
			Dos94_collide_hitoffsets(fraction);
			return component + 1;
		}
	}
	return 0;
}

/* DOS94 0x7C0266: statics use their first component/LOD and their original axis order. */
int16_t Dos94_static_laserstaticcollide(uint16_t source, uint16_t target) {
	const XwMissionObjectRecord* object = &g_missionObjects[target];
	if (g_objectTable[source].sourceObjectRef == target + XW_MISSION_OBJECT_REF_BASE ||
		object->genusId == 14 || object->genusId == 10 ||
		((object->genusId == 7 || object->genusId == 8) && g_objectTable[source].iff == 1))
		return 0;
	create_getworldposition(target + XW_MISSION_OBJECT_REF_BASE, 0);
	int32_t points[2][3];
	relative_segment(points, g_resolvedWorldX, g_resolvedWorldY, g_resolvedWorldZ);
	for (int p = 0; p < 2; ++p)
		if ((uint32_t)collide_roughdistance3d(points[p][0], points[p][1], points[p][2]) > 0x20000)
			return 0;
	uint16_t extent = g_modelTypeTable[object->objectType].maxBoundsExtent;
	if (extent <= 2800) {
		g_collisionSweepStartX = g_collisionSweepEndX = g_resolvedWorldX;
		g_collisionSweepStartY = g_collisionSweepEndY = g_resolvedWorldY;
		g_collisionSweepStartZ = g_collisionSweepEndZ = g_resolvedWorldZ;
		extent >>= 2;
		return collide_checkboxcollision(extent + (extent >> 1));
	}
	uint8_t shift = scale_segment(points) + 1;
	fview_calcrotatemove((uint16_t)object->pitchAngle8 << 8, (uint16_t)object->yawAngle8 << 8, NULL);
	fview_calcrotateorient((uint16_t)object->rollAngle8 << 8, 0, NULL);
	g_fviewForwardX_Q15 = (int16_t)-g_fviewForwardX_Q15;
	g_fviewForwardY_Q15 = (int16_t)-g_fviewForwardY_Q15;
	g_fviewForwardZ_Q15 = (int16_t)-g_fviewForwardZ_Q15;
	int16_t axes[3][3] = { { g_fviewSideX_Q15, g_fviewSideY_Q15, g_fviewSideZ_Q15 },
						   { g_fviewUpX_Q15, g_fviewUpY_Q15, g_fviewUpZ_Q15 },
						   { g_fviewForwardX_Q15, g_fviewForwardY_Q15, g_fviewForwardZ_Q15 } };
	int16_t local[2][3];
	for (int p = 0; p < 2; ++p) {
		int16_t v[3] = { points[p][0], points[p][1], points[p][2] };
		for (int i = 0; i < 3; ++i)
			local[p][i] = Dos94_math2_dot3Q15(axes[i], v);
	}
	g_collisionScratchPoint1X = local[0][0];
	g_collisionScratchPoint1Y = local[0][1];
	g_collisionScratchPoint1Z = local[0][2];
	g_collisionScratchPoint2X = local[1][0];
	g_collisionScratchPoint2Y = local[1][1];
	g_collisionScratchPoint2Z = local[1][2];
	uint16_t count;
	const Dos94Lod* lod = Dos94_DRAW_getcomponentptr(object->objectType, 0, &count);
	if (!lod || !count)
		return 0;
	/* This preliminary check swaps Y/Z; the polygon primitive does not. */
	const int order[3] = { 0, 2, 1 };
	for (int i = 0; i < 3; ++i) {
		int lo = lod->mesh.bounds[order[i]] >> shift, hi = lod->mesh.bounds[order[i] + 3] >> shift;
		if ((lo > local[0][i] && lo > local[1][i]) || (hi < local[0][i] && hi < local[1][i]))
			return 0;
	}
	uint16_t fraction = Dos94_COLLIDE_checkhitpolygons(&lod->mesh, local[0], local[1], shift);
	if (!fraction)
		return 0;
	Dos94_collide_hitoffsets(fraction);
	return 1;
}
