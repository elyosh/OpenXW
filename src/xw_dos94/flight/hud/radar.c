#include "xw/flight/fview.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/create.h"
#include "xw/flight/player/user.h"
#include "xw/flight/xw.h"
#include "xw/math/math2.h"
#include "xw/math/trig2.h"
#include "xw_dos94/assets/models.h"
#include "xw_dos94/flight/hud/panel.h"
#include "xw_dos94/render/dos93_math.h"

/* DOS93 0x6A82A1 / DOS94 0x6A8988 share the boundary table. Only DOS93 rounds ratios. */
void Dos94_math2_getradarcoord(int32_t x, int32_t y, uint32_t depth) {
	static const uint8_t boundary[37][2] = {
		{ 0, 18 },  { 1, 18 },  { 2, 18 },  { 3, 18 },  { 4, 17 },  { 5, 17 },  { 6, 17 },  { 7, 17 },
		{ 8, 16 },  { 9, 16 },  { 10, 16 }, { 10, 15 }, { 11, 15 }, { 12, 15 }, { 12, 14 }, { 13, 14 },
		{ 14, 14 }, { 14, 13 }, { 15, 13 }, { 15, 12 }, { 16, 12 }, { 16, 11 }, { 17, 11 }, { 17, 10 },
		{ 18, 10 }, { 18, 9 },  { 18, 8 },  { 19, 8 },  { 19, 7 },  { 19, 6 },  { 20, 6 },  { 20, 5 },
		{ 21, 4 },  { 21, 3 },  { 21, 2 },  { 21, 1 },  { 21, 0 }
	};
	uint32_t px = (x < 0 ? 0u - (uint32_t)x : (uint32_t)x) << 3;
	uint32_t py = (y < 0 ? 0u - (uint32_t)y : (uint32_t)y) << 3;
	bool dos93 = Dos94Assets_Version() == XW_GAME_VERSION_93;
	if (dos93) {
		px = Dos93_math2_divide32u(px, depth);
		py = Dos93_math2_divide32u(py, depth);
	} else if (depth) {
		px /= depth;
		py /= depth;
	}
	if (px > 65535)
		px = 65535;
	if (py > 65535)
		py = 65535;
	uint16_t angle, ratio;
	if (dos93)
		Dos93_trig2_calcarctan(py, px, &angle, &ratio);
	else
		trig2_calcarctan(px, py, &angle, &ratio);
	unsigned index = (uint16_t)(0x4000 - angle) / 443;
	if (index >= 37)
		index = 36;
	if (px > boundary[index][0])
		px = boundary[index][0];
	if (py > boundary[index][1])
		py = boundary[index][1];
	g_radarProjectedX = x < 0 ? -(int16_t)px : (int16_t)px;
	g_radarProjectedY = y < 0 ? -(int16_t)py : (int16_t)py;
}

static int16_t q15(int16_t a, int16_t b) { return (int16_t)(((int32_t)a * b) >> 15); }

/* Craft use the renderer's cached eye coordinates; other objects retain DOS Q15 narrowing. */
bool Dos94_panel_getradareye(uint16_t objectRef, int32_t* x, int32_t* y, int32_t* depth) {
	int32_t relativeX, relativeY, relativeZ, viewX, viewY, viewDepth;
	ObjectRecord* playerObject;
	if (objectRef < XW_CRAFT_OBJECT_COUNT) {
		CraftData* craft = g_objectTable[objectRef].instanceData;
		if (!craft)
			return false;
		viewX = craft->viewX;
		viewY = craft->viewY;
		viewDepth = craft->viewZ;
	} else {
		playerObject = g_playerFlightState.object;
		if (objectRef >= XW_MISSION_OBJECT_REF_BASE) {
			if (objectRef - XW_MISSION_OBJECT_REF_BASE >= MISSION_OBJECT_COUNT)
				return false;
			XwMissionObjectRecord* object = &g_missionObjects[objectRef - XW_MISSION_OBJECT_REF_BASE];
			relativeX = (int16_t)(object->worldX - (playerObject->worldX >> 8));
			relativeY = (int16_t)(object->worldY - (playerObject->worldY >> 8));
			relativeZ = (int16_t)(object->worldZ - (playerObject->worldZ >> 8));
		} else {
			if (objectRef >= XW_OBJECT_COUNT)
				return false;
			ObjectRecord* object = &g_objectTable[objectRef];
			relativeX = (int16_t)((int32_t)((uint32_t)object->worldX - (uint32_t)playerObject->worldX) >> 8);
			relativeY = (int16_t)((int32_t)((uint32_t)object->worldY - (uint32_t)playerObject->worldY) >> 8);
			relativeZ = (int16_t)((int32_t)((uint32_t)object->worldZ - (uint32_t)playerObject->worldZ) >> 8);
		}
		if (playerObject->orientMatrixDirty) {
			fview_calcrotatemove(playerObject->pitch, playerObject->yaw, playerObject);
			fview_calcrotateorient(playerObject->roll, 0, playerObject);
		}
		viewDepth = q15(relativeX, playerObject->cachedForwardX) +
					q15(relativeY, playerObject->cachedForwardY) +
					q15(relativeZ, playerObject->cachedForwardZ);
		viewX = q15(relativeX, playerObject->cachedSideX) + q15(relativeY, playerObject->cachedSideY) +
				q15(relativeZ, playerObject->cachedSideZ);
		viewY = -(q15(relativeX, playerObject->cachedUpX) + q15(relativeY, playerObject->cachedUpY) +
				  q15(relativeZ, playerObject->cachedUpZ));
	}
	*x = viewX;
	*y = viewY;
	*depth = viewDepth;
	return true;
}
