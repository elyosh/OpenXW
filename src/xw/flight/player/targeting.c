#include "xw/flight/player/targeting.h"

#ifdef XW_MODERN
#include "xw_runtime/snapshot/render_hud.h"
#endif
#include "xw/assets/model_mesh.h"
#include "xw/flight/ai/pai.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/hud/hud.h"
#include "xw/flight/hud/panel.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/object/create.h"
#include "xw/flight/player/user.h"
#include "xw/flight/replay/replay.h"
#include "xw/math/transfm2.h"
#include "xw/render/rtsvga2.h"

// FUNCTION: XW 0x42E1B0
void Targeting_w_DrawObjectBox(void) {
	Targeting_DrawObjectBox(g_playerFlightState.currentTargetObjectIdx, TARGETING_COMPONENT_NONE,
							TARGETING_CURRENT_TARGET_COLOR);
}

// FUNCTION: XW 0x42E1D0
void Targeting_DrawObjectBox(int objectOrMissionPointRef, int componentIndex, uint8_t colorIndex) {
	uint16_t objectRef = (uint16_t)objectOrMissionPointRef;
	uint16_t componentRef = (uint16_t)componentIndex;
	int screenX, screenY, viewDepth;
	int extent, boxSize, paddedSize, minimumPixels, maximumPixels;
	if (objectRef == XW_PLAYER_NO_TARGET || g_replayviewmode != 0)
		return;
	Targeting_ProjectObjectOrMissionPoint(objectRef, componentRef, &screenX, &screenY, &viewDepth);
	if (viewDepth <= 0)
		return;
	if (objectRef >= XW_MISSION_OBJECT_REF_BASE) {
		extent = Targeting_GetObjectBoxExtent(objectRef);
	} else {
		int craftDefinitionIndex = *(const uint8_t*)g_objectTable[objectRef].instanceData;
		if (componentRef == TARGETING_COMPONENT_NONE ||
			g_objectTable[objectRef].genusId == XW_GENUS_STARFIGHTER ||
			(g_objectTable[objectRef].genusId == XW_GENUS_FREIGHTER &&
			 g_craftTypeDefs[craftDefinitionIndex].maxSpeed == 0))
			extent = Targeting_GetObjectBoxExtent(objectRef);
		else
			extent = ModelMesh_GetComponentMaxExtent(g_objectTable[objectRef].objectType, componentRef);
	}
#ifdef XW_MODERN
	XwHud_Target(objectRef, componentRef, extent, colorIndex, g_replayUiEventConsumed != 0);
#endif
	boxSize = (int)((uint32_t)g_projScaleInt * (uint32_t)extent / (uint32_t)viewDepth);
	minimumPixels = TARGETING_BOX_MIN_LOW;
	if (g_flightResolutionMode != RTSVGA2_MODE_13H)
		minimumPixels = TARGETING_BOX_MIN_HIGH;
	maximumPixels = ((unsigned int)g_flightScreenWidth >> 1) + ((unsigned int)g_flightScreenWidth >> 2);
	if (boxSize < minimumPixels)
		boxSize = minimumPixels;
	if (boxSize > maximumPixels)
		boxSize = maximumPixels;
	paddedSize = boxSize + TARGETING_BOX_PADDING;
	screenX -= paddedSize / 2;
	screenY -= paddedSize / 2;
	if (g_replayUiEventConsumed != 0)
		panel_DrawObjectBoxCorners(screenX, screenY, paddedSize, paddedSize, colorIndex);
	else
		Hud_DrawBoxInXTrans(screenX, screenY, paddedSize, paddedSize, colorIndex, viewDepth);
}

// FUNCTION: XW 0x42E350
int Targeting_GetObjectBoxExtent(int objectOrMissionPointRef) {
	if (objectOrMissionPointRef < XW_MISSION_OBJECT_REF_BASE) {
		int craftDefinitionIndex = *(const uint8_t*)g_objectTable[objectOrMissionPointRef].instanceData;
		if (objectOrMissionPointRef < XW_CRAFT_OBJECT_COUNT &&
			g_objectTable[objectOrMissionPointRef].genusId != XW_GENUS_STARFIGHTER &&
			(g_objectTable[objectOrMissionPointRef].genusId != XW_GENUS_FREIGHTER ||
			 g_craftTypeDefs[craftDefinitionIndex].maxSpeed != 0)) {
			int meanExtent = (g_craftModelBounds[craftDefinitionIndex].boundSizeZ +
							  g_craftModelBounds[craftDefinitionIndex].boundSizeY +
							  g_craftModelBounds[craftDefinitionIndex].boundSizeX) /
							 TARGETING_BOUND_AXIS_COUNT;
#ifdef XW_MODERN
			return (int32_t)((uint32_t)meanExtent
							 << (g_craftModelBounds[craftDefinitionIndex].boundSizeShift &
								 TARGETING_SHIFT_COUNT_MASK));
#else
			return meanExtent << (uint8_t)g_craftModelBounds[craftDefinitionIndex].boundSizeShift;
#endif
		}
		return g_modelTypeTable[g_objectTable[objectOrMissionPointRef].objectType].maxBoundsExtent;
	}
	return g_modelTypeTable[g_missionObjects[objectOrMissionPointRef - XW_MISSION_OBJECT_REF_BASE].objectType]
		.maxBoundsExtent;
}

// FUNCTION: XW 0x42E410
void Targeting_ProjectObjectOrMissionPoint(int objectOrMissionPointRef, int componentIndex, int* outScreenX,
										   int* outScreenY, int* outViewZ) {
	int relativeX;
	int relativeY;
	int relativeZ;
	int viewDepth;
	create_getworldposition(objectOrMissionPointRef, 0);
	if (componentIndex != TARGETING_COMPONENT_NONE && objectOrMissionPointRef < XW_MISSION_OBJECT_REF_BASE) {
		int craftDefinitionIndex = *(const uint8_t*)g_objectTable[objectOrMissionPointRef].instanceData;
		if (g_objectTable[objectOrMissionPointRef].genusId != XW_GENUS_STARFIGHTER &&
			(g_objectTable[objectOrMissionPointRef].genusId != XW_GENUS_FREIGHTER ||
			 g_craftTypeDefs[craftDefinitionIndex].maxSpeed != 0)) {
			int localForward;
			int localUp;
			int localSide;
#ifdef XW_MODERN
			localForward =
				(int32_t)(0u - (uint32_t)ModelMesh_GetCenterY(
								   g_objectTable[objectOrMissionPointRef].objectType, componentIndex));
#else
			localForward =
				-ModelMesh_GetCenterY(g_objectTable[objectOrMissionPointRef].objectType, componentIndex);
#endif
			localUp = ModelMesh_GetCenterZ(g_objectTable[objectOrMissionPointRef].objectType, componentIndex);
			localSide =
				ModelMesh_GetCenterX(g_objectTable[objectOrMissionPointRef].objectType, componentIndex);
			pai_RotateLocalVectorToWorldScratch(&g_objectTable[objectOrMissionPointRef], localSide, localUp,
												localForward);
#ifdef XW_MODERN
			g_resolvedWorldX = (int32_t)((uint32_t)g_resolvedWorldX + (uint32_t)g_rotatedX);
#else
			g_resolvedWorldX += g_rotatedX;
#endif
#ifdef XW_MODERN
			g_resolvedWorldY = (int32_t)((uint32_t)g_resolvedWorldY + (uint32_t)g_rotatedY);
#else
			g_resolvedWorldY += g_rotatedY;
#endif
#ifdef XW_MODERN
			g_resolvedWorldZ = (int32_t)((uint32_t)g_resolvedWorldZ + (uint32_t)g_rotatedZ);
#else
			g_resolvedWorldZ += g_rotatedZ;
#endif
		}
	}
#ifdef XW_MODERN
	relativeX = (int32_t)((uint32_t)g_resolvedWorldX - (uint32_t)g_flightCamera.worldPosition.x);
#else
	relativeX = g_resolvedWorldX - g_flightCamera.worldPosition.x;
#endif
#ifdef XW_MODERN
	relativeY = (int32_t)((uint32_t)g_resolvedWorldY - (uint32_t)g_flightCamera.worldPosition.y);
#else
	relativeY = g_resolvedWorldY - g_flightCamera.worldPosition.y;
#endif
#ifdef XW_MODERN
	relativeZ = (int32_t)((uint32_t)g_resolvedWorldZ - (uint32_t)g_flightCamera.worldPosition.z);
#else
	relativeZ = g_resolvedWorldZ - g_flightCamera.worldPosition.z;
#endif
	viewDepth = transfm2_geteyez(relativeX, relativeY, relativeZ);
	*outViewZ = viewDepth;
	if (viewDepth > 0) {
		int viewX = transfm2_geteyex(relativeX, relativeY, relativeZ);
		int viewY = transfm2_geteyey(relativeX, relativeY, relativeZ);
		*outScreenX = transfm2_getscreencoordx(viewX, viewDepth);
		*outScreenY = transfm2_getscreencoordy(viewY, viewDepth);
	}
}
