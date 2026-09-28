#include "xw/flight/hud/panel.h"

#ifdef XW_MODERN
#include "xw_runtime/snapshot/render_hud.h"
#endif
#ifdef XW_MODERN
#include "xw_runtime/snapshot/render_assets.h"
#endif

#ifdef XW_MODERN
#include "xw_dos94/flight/hud/panel.h"
#include "xw_dos94/flight/hud/replay.h"
#include "xw_dos94/render/display.h"
#include "xw_runtime/runtime/flight_camera.h"
#include "xw_runtime/runtime/flight_panel.h"
#include "xw_runtime/runtime/flight_types.h"
#endif

#include "xw/assets/model_mesh.h"
#include "xw/audio/fsfx.h"
#include "xw/flight/ai/pai.h"
#include "xw/flight/ai/paiorder.h"
#include "xw/flight/death_star.h"
#include "xw/flight/fediskio.h"
#include "xw/flight/feinput.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/fview.h"
#include "xw/flight/gate.h"
#include "xw/flight/hud/festring.h"
#include "xw/flight/hud/msg.h"
#include "xw/flight/hud/panelrts.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/collide.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/laser.h"
#include "xw/flight/object/object.h"
#include "xw/flight/player/user.h"
#include "xw/flight/replay/replay.h"
#include "xw/flight/xw.h"
#include "xw/math/math2.h"
#include "xw/math/trig2.h"
#include "xw/render/flight_view.h"
#include "xw/render/rtsrgb.h"
#include "xw/render/rtsvga2.h"
#include "xw/util/memory.h"

#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4C9710
char g_hudCockpitResolutionDirectory[PANEL_COCKPIT_DIRECTORY_CAPACITY] = ";CP640\\";

// GLOBAL: XW 0x4C9718
const char g_lfdPaletteResourceTypeTag[PANEL_LFD_TYPE_TAG_LENGTH + 1] = "PLTT";

// GLOBAL: XW 0x4C9830
const char* g_hudBuoyNameStrings[PANEL_BUOY_NAME_COUNT] = { "COMM SAT", "NAV BUOY", "PROBE" };

// GLOBAL: XW 0x4DA7F4
unsigned int g_viewportSpanMaskOffset = PANEL_SPAN_MASK_INITIAL_OFFSET;

// GLOBAL: XW 0x4F4920
uint8_t g_radarBlipBufferParity = 0;

// GLOBAL: XW 0x4F4924
uint8_t g_radarTargetMarkerBackgroundSaved = 0;

// GLOBAL: XW 0x4F4928
uint8_t g_targetComputerCrossBackgroundSaved = 0;

// GLOBAL: XW 0x4F492C
int g_replayHudShowedSimStepScale = 0;

// GLOBAL: XW 0x62B014
int g_hudClearHeight = 0;

// GLOBAL: XW 0x62B5E4
uint8_t* g_hudPanelSpriteDataBuffer = NULL;

// GLOBAL: XW 0x62BAFC
int16_t g_ReplayProgressPercent = 0;

// GLOBAL: XW 0x62C940
uint8_t* g_hudPanelSpriteDataByIndex[PANEL_HUD_SPRITE_COUNT] = { NULL };

// GLOBAL: XW 0x6377AB
uint8_t g_hudCockpitResourcesLoaded = 0;

// GLOBAL: XW 0x6377BC
uint8_t* g_hudPanelSpriteDataWriteCursor = NULL;

// GLOBAL: XW 0x63A280
char g_hudCockpitBasePath[PANEL_COCKPIT_PATH_CAPACITY] = { 0 };

// GLOBAL: XW 0x63A2A0
XwRadarBlip* g_radarRearBlips = NULL;

// GLOBAL: XW 0x63A2A4
uint16_t g_radarPreviousRearBlipCount = 0;

// GLOBAL: XW 0x63A2A8
XwRadarBlip* g_radarPreviousFrontBlips = NULL;

// GLOBAL: XW 0x63A2AC
uint16_t g_radarBlipColorIndex = 0;

// GLOBAL: XW 0x63A2B0
uint8_t* g_hudCockpitResourceWriteCursor = NULL;

// GLOBAL: XW 0x63A2B4
uint8_t g_hudLoadedPanelSetId = 0;

// GLOBAL: XW 0x63A2B6
uint16_t g_targetComputerCachedLockSpriteIndex = 0;

// GLOBAL: XW 0x63A2B8
uint8_t* g_hudPanelSpriteCraftDataStart = NULL;

// GLOBAL: XW 0x63A2BC
uint16_t g_radarPreviousFrontBlipCount = 0;

// GLOBAL: XW 0x63A2C0
XwRadarBlip g_radarFrontBlipsA[PANEL_RADAR_BLIP_CAPACITY] = { 0 };

// GLOBAL: XW 0x63A3E0
XwRadarBlip g_radarFrontBlipsB[PANEL_RADAR_BLIP_CAPACITY] = { 0 };

// GLOBAL: XW 0x63A500
uint8_t g_hudCockpitMirrorHorizontal = 0;

// GLOBAL: XW 0x63A502
uint16_t g_hudCachedExhaustPortFrame = 0;

// GLOBAL: XW 0x63A520
HudCockpitResourceDescriptor g_hudCockpitResourceDescriptors[PANEL_COCKPIT_DESCRIPTOR_COUNT] = { 0 };

// GLOBAL: XW 0x63A778
uint8_t g_hudFullRedrawInProgress = 0;

// GLOBAL: XW 0x63A77A
uint16_t g_radarFrontBlipCount = 0;

// GLOBAL: XW 0x63A77C
XwRadarBlip* g_radarFrontBlips = NULL;

// GLOBAL: XW 0x63A780
HudCockpitResource g_hudCockpitResources[PANEL_COCKPIT_DESCRIPTOR_COUNT] = { 0 };

// GLOBAL: XW 0x63A898
XwRadarBlip* g_radarPreviousRearBlips = NULL;

// GLOBAL: XW 0x63A8A0
HudPanelSpriteFileInfo g_hudPanelSpriteFileInfo = { 0 };

// GLOBAL: XW 0x63A8C0
char g_hudCockpitResourcePath[PANEL_COCKPIT_PATH_CAPACITY] = { 0 };

// GLOBAL: XW 0x63A8E0
uint8_t g_hudExhaustPortDisplayMode = 0;

// GLOBAL: XW 0x63A8E2
uint16_t g_hudCachedTargetObjectIdx = 0;

// GLOBAL: XW 0x63A8E8
uint16_t g_radarRearBlipCount = 0;

// GLOBAL: XW 0x63A8EE
int16_t g_hudLoadedCockpitView = 0;

// GLOBAL: XW 0x63A900
HudElementLayout g_hudElementLayouts[PANEL_HUD_ELEMENT_COUNT] = { 0 };

// GLOBAL: XW 0x63AAC2
uint16_t g_targetComputerPreviousCrossY = 0;

// GLOBAL: XW 0x63AAC4
uint16_t g_targetComputerPreviousCrossX = 0;

// GLOBAL: XW 0x63AAC6
uint8_t g_targetLockActive = 0;

// GLOBAL: XW 0x63AACC
uint8_t g_hudPanelSetId = 0;

// GLOBAL: XW 0x63AACE
int16_t g_targetComputerMarkerScreenX = 0;

// GLOBAL: XW 0x63AAD0
int16_t g_targetComputerMarkerScreenY = 0;

// GLOBAL: XW 0x63AAE0
uint16_t g_hudElementStateCache[PANEL_HUD_ELEMENT_COUNT] = { 0 };

// GLOBAL: XW 0x63AB76
uint16_t g_targetComputerLockSpriteIndex = 0;

// GLOBAL: XW 0x63AB80
XwRadarBlip g_radarRearBlipsA[PANEL_RADAR_BLIP_CAPACITY] = { 0 };

// GLOBAL: XW 0x63ACA0
XwRadarBlip g_radarRearBlipsB[PANEL_RADAR_BLIP_CAPACITY] = { 0 };

// FUNCTION: XW 0x418F60
void panel_initpanel(void) {
	unsigned int elementIndex;
#ifdef XW_MODERN
	if (XwFlightTypes_Dos() && (!Dos94_display || !g_hudCockpitResourcesLoaded))
		return;
#endif
	for (elementIndex = 0; elementIndex < sizeof(g_hudElementStateCache) / sizeof(g_hudElementStateCache[0]);
		 ++elementIndex)
		g_hudElementStateCache[elementIndex] = PANEL_ELEMENT_CACHE_INVALID;
	g_hudFullRedrawInProgress = 1;
	g_ReplayProgressPercent = PANEL_ELEMENT_CACHE_INVALID;
	g_radarFrontBlipCount = 0;
	g_radarRearBlipCount = 0;
	g_radarTargetMarkerBackgroundSaved = 0;
	g_hudCachedTargetObjectIdx = XW_OBJECT_SLOT_UNAVAILABLE;
	g_hudCachedExhaustPortFrame = PANEL_FRAME_CACHE_INVALID;
	g_hudExhaustPortDisplayMode = PANEL_DISPLAY_MODE_INVALID;
	panel_updatepanel();
	panel_UpdateCraftSystemStatusIndicators();
	panel_updatecockpitdamage();
	g_hudFullRedrawInProgress = 0;
}

// FUNCTION: XW 0x418FD0
void panel_updatepanel(void) {
	uint16_t currentTargetRef;
#ifdef XW_MODERN
	if (XwFlightTypes_Dos() && (!Dos94_display || !g_hudCockpitResourcesLoaded))
		return;
#endif
	currentTargetRef = g_playerFlightState.currentTargetObjectIdx;
	if (currentTargetRef != XW_OBJECT_SLOT_UNAVAILABLE) {
		uint16_t previousTargetRef = g_playerFlightState.currentTargetObjectIdx;
		if (currentTargetRef < XW_MISSION_OBJECT_REF_BASE) {
#ifdef XW_MODERN
			if (previousTargetRef >= XW_OBJECT_COUNT ||
				g_objectTable[previousTargetRef].objectType == XW_OBJ_NONE ||
#else
			if (g_objectTable[previousTargetRef].objectType == XW_OBJ_NONE ||
#endif
				g_objectTable[previousTargetRef].genusId == XW_GENUS_EXPLOSION_EFFECT ||
				(g_objectTable[previousTargetRef].familyId == XW_OBJECT_FAMILY_CRAFT &&
#ifdef XW_MODERN
				 (!g_objectTable[previousTargetRef].instanceData ||
				  ((CraftData*)g_objectTable[previousTargetRef].instanceData)->objectKind ==
#else
				 (((CraftData*)g_objectTable[previousTargetRef].instanceData)->objectKind ==
#endif
					  XW_CRAFT_OBJECT_KIND_3 ||
				  ((CraftData*)g_objectTable[previousTargetRef].instanceData)->objectKind ==
					  XW_CRAFT_OBJECT_KIND_4))) {
				currentTargetRef = XW_OBJECT_SLOT_UNAVAILABLE;
				g_playerFlightState.currentTargetObjectIdx = currentTargetRef;
			}
		} else {
			unsigned int staticIndex = previousTargetRef - XW_MISSION_OBJECT_REF_BASE;
#ifdef XW_MODERN
			if (staticIndex >= MISSION_OBJECT_COUNT ||
				g_missionObjects[staticIndex].objectType == XW_OBJ_NONE ||
#else
			if (g_missionObjects[staticIndex].objectType == XW_OBJ_NONE ||
#endif
				g_missionObjects[staticIndex].genusId == XW_GENUS_EXPLOSION_EFFECT) {
				currentTargetRef = XW_OBJECT_SLOT_UNAVAILABLE;
				g_playerFlightState.currentTargetObjectIdx = currentTargetRef;
			}
		}
		if (!(g_playerFlightState.craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_TARGETING)) {
			currentTargetRef = XW_OBJECT_SLOT_UNAVAILABLE;
			g_playerFlightState.currentTargetObjectIdx = currentTargetRef;
		}
		if (currentTargetRef == XW_OBJECT_SLOT_UNAVAILABLE)
			g_playerFlightState.previousTargetObjectIdx = previousTargetRef;
	}
	if (g_playerFlightState.hudSuppressed == 0) {
		if (g_flightCamera.hudStateLive == 0)
			panel_updateforwardpanel();
		else if (g_flightCamera.hudStateLive == USER_TARGET_ANNOUNCEMENT_HUD_STATE)
			panel_updatefullforward();
	}
}

// FUNCTION: XW 0x4190B0
void panel_updateforwardpanel(void) {
	panel_updateradar();
	panel_updatelasers();
	panel_updategunsight();
	panel_updatecmd();
	panel_updateweapons();
	panel_updateshields();
	panel_UpdateCriticalHullShieldWarning();
	panel_updatespeed();
	panel_updateclock();
	panel_updatelever(PANEL_SHIELD_DISTRIBUTION_ELEMENT, g_playerFlightState.craft->shieldDistribMode);
	panel_updatepower();
	panel_updatethrottle();
	panel_updateweaponwarnings();
	panel_updatereplaystuff();
	if (g_hudElementLayouts[PANEL_SFOIL_ELEMENT].x != 0) {
		unsigned int sFoilsOpen = (g_playerFlightState.craft->sFoilState & XW_SFOIL_CLOSED) == 0;
		panel_updatelever(PANEL_SFOIL_ELEMENT, sFoilsOpen);
	}
}

// FUNCTION: XW 0x419140
void panel_updatefullforward(void) {
	panel_updateradar();
	panel_updatelasers();
	panel_updategunsight();
}

// FUNCTION: XW 0x419150
void panel_updateradar(void) {
	uint16_t objectIndex, missionIndex;
	int blipCount;
	uint16_t frontCount, rearCount;
	int16_t selectedX;
	if ((g_playerFlightState.craft->activeHudFeatureMask & PANEL_RADAR_HUD_FEATURE) != 0) {
		selectedX = g_radarSelectedTargetScreenX;
		frontCount = g_radarFrontBlipCount;
		rearCount = g_radarRearBlipCount;
		g_radarPreviousTargetScreenX = selectedX;
		g_radarPreviousFrontBlipCount = frontCount;
		g_radarPreviousRearBlipCount = rearCount;
		g_radarRearBlipCount = 0;
		g_radarFrontBlipCount = 0;
		g_radarPreviousTargetScreenY = g_radarSelectedTargetScreenY;
		if (g_radarBlipBufferParity != 0) {
			g_radarPreviousFrontBlips = g_radarFrontBlipsB;
			g_radarFrontBlips = g_radarFrontBlipsA;
			g_radarPreviousRearBlips = g_radarRearBlipsB;
			g_radarRearBlips = g_radarRearBlipsA;
		} else {
			g_radarPreviousFrontBlips = g_radarFrontBlipsA;
			g_radarFrontBlips = g_radarFrontBlipsB;
			g_radarPreviousRearBlips = g_radarRearBlipsA;
			g_radarRearBlips = g_radarRearBlipsB;
		}
		if (g_missionRuntimeState.provingGroundsActive != 0 &&
			g_missionRuntimeState.provingGroundsCurrentCheckpointIndex != -1)
			panel_addbliptoradar(g_missionRuntimeState.provingGroundsCurrentCheckpointIndex +
								 XW_MISSION_OBJECT_REF_BASE);
		for (objectIndex = 0; objectIndex < XW_CRAFT_OBJECT_COUNT; ++objectIndex) {
			if (objectIndex != g_playerFlightState.objectIndex &&
#ifdef XW_MODERN
				XwFlightTypes_Targetable(g_objectTable[objectIndex].objectType) &&
#else
				(g_modelTypeTable[g_objectTable[objectIndex].objectType].objectFlags &
				 MODEL_TYPE_TARGETABLE) != 0 &&
#endif
				((CraftData*)g_objectTable[objectIndex].instanceData)->objectKind !=
					PANEL_RADAR_HIDDEN_CRAFT_KIND)
				panel_addbliptoradar(objectIndex);
		}
		for (objectIndex = XW_CRAFT_OBJECT_COUNT;
			 objectIndex < XW_CRAFT_OBJECT_COUNT + LASER_WARHEAD_GUIDANCE_COUNT; ++objectIndex) {
#ifdef XW_MODERN
			if (XwFlightTypes_Targetable(g_objectTable[objectIndex].objectType))
#else
			if ((g_modelTypeTable[g_objectTable[objectIndex].objectType].objectFlags &
				 MODEL_TYPE_TARGETABLE) != 0)
#endif
				panel_addbliptoradar(objectIndex);
		}
		for (missionIndex = 0; missionIndex < MISSION_OBJECT_COUNT; ++missionIndex) {
#ifdef XW_MODERN
			if (XwFlightTypes_Targetable(g_missionObjects[missionIndex].objectType))
#else
			if ((g_modelTypeTable[g_missionObjects[missionIndex].objectType].objectFlags &
				 MODEL_TYPE_TARGETABLE) != 0)
#endif
				panel_addbliptoradar(missionIndex + XW_MISSION_OBJECT_REF_BASE);
		}
#ifdef XW_MODERN
		XwHud_Radar(g_radarFrontBlips, g_radarFrontBlipCount, false);
		XwHud_Radar(g_radarRearBlips, g_radarRearBlipCount, false);
#endif
		if (g_radarTargetMarkerBackgroundSaved != 0)
			rtsvga2_removebracket();
		blipCount = g_radarPreviousFrontBlipCount;
		if (blipCount != 0)
			rtsvga2_removeblips(g_radarPreviousFrontBlips, blipCount);
		blipCount = g_radarFrontBlipCount;
		if (blipCount != 0)
			rtsvga2_drawblips(g_radarFrontBlips, blipCount);
		blipCount = g_radarPreviousRearBlipCount;
		if (blipCount != 0)
			rtsvga2_removeblips(g_radarPreviousRearBlips, blipCount);
		blipCount = g_radarRearBlipCount;
		if (blipCount != 0)
			rtsvga2_drawblips(g_radarRearBlips, blipCount);
		if (g_playerFlightState.currentTargetObjectIdx != XW_OBJECT_SLOT_UNAVAILABLE) {
			rtsvga2_drawbracket();
			g_radarTargetMarkerBackgroundSaved = 1;
		} else {
			g_radarTargetMarkerBackgroundSaved = 0;
		}
#ifdef XW_MODERN
		XwHud_Radar(g_radarFrontBlips, g_radarFrontBlipCount, true);
		XwHud_Radar(g_radarRearBlips, g_radarRearBlipCount, true);
#endif
		g_radarBlipBufferParity ^= 1;
	}
}

// FUNCTION: XW 0x419390
void panel_addbliptoradar(uint16_t objectRef) {
#ifdef XW_MODERN
	const XwFlightPanelLayout* layout = XwFlightPanel_Layout();
#endif
	int32_t relativeX, relativeY, relativeZ, viewDepth, viewX, viewY, absoluteDepth;
	ObjectRecord* playerObject;
	int objectIndex = objectRef;
	int16_t frontHemisphere, targetY;
#ifdef XW_MODERN
	if (XwFlightTypes_Dos()) {
		if (!Dos94_panel_getradareye(objectRef, &viewX, &viewY, &viewDepth))
			return;
	} else {
#endif
		if (objectRef >= XW_MISSION_OBJECT_REF_BASE) {
			relativeX = g_missionObjects[objectIndex - XW_MISSION_OBJECT_REF_BASE].worldX *
						MISSION_OBJECT_WORLD_COORDINATE_SCALE;
			relativeY = g_missionObjects[objectIndex - XW_MISSION_OBJECT_REF_BASE].worldY *
						MISSION_OBJECT_WORLD_COORDINATE_SCALE;
			relativeZ = g_missionObjects[objectIndex - XW_MISSION_OBJECT_REF_BASE].worldZ *
						MISSION_OBJECT_WORLD_COORDINATE_SCALE;
		} else {
			relativeX = g_objectTable[objectIndex].worldX;
			relativeY = g_objectTable[objectIndex].worldY;
			relativeZ = g_objectTable[objectIndex].worldZ;
		}
		playerObject = g_playerFlightState.object;
#ifdef XW_MODERN
		relativeX = (int32_t)((uint32_t)relativeX - (uint32_t)playerObject->worldX);
		relativeY = (int32_t)((uint32_t)relativeY - (uint32_t)playerObject->worldY);
		relativeZ = (int32_t)((uint32_t)relativeZ - (uint32_t)playerObject->worldZ);
#else
	relativeX -= playerObject->worldX;
	relativeY -= playerObject->worldY;
	relativeZ -= playerObject->worldZ;
#endif
		if (playerObject->orientMatrixDirty != 0) {
			fview_calcrotatemove(playerObject->pitch, playerObject->yaw, playerObject);
			fview_calcrotateorient(g_playerFlightState.object->roll, 0, g_playerFlightState.object);
			playerObject = g_playerFlightState.object;
		}
		viewDepth = (int32_t)((uint32_t)(((int64_t)playerObject->cachedForwardX * relativeX) >>
										 FVIEW_MATRIX_FRACTION_BITS) +
							  (uint32_t)(((int64_t)playerObject->cachedForwardY * relativeY) >>
										 FVIEW_MATRIX_FRACTION_BITS) +
							  (uint32_t)(((int64_t)playerObject->cachedForwardZ * relativeZ) >>
										 FVIEW_MATRIX_FRACTION_BITS));
#ifdef XW_MODERN
		viewX = (int32_t)((uint32_t)(((int64_t)playerObject->cachedSideX * relativeX) >>
									 FVIEW_MATRIX_FRACTION_BITS) +
						  (uint32_t)(((int64_t)playerObject->cachedSideY * relativeY) >>
									 FVIEW_MATRIX_FRACTION_BITS) +
						  (uint32_t)(((int64_t)playerObject->cachedSideZ * relativeZ) >>
									 FVIEW_MATRIX_FRACTION_BITS));
		viewY = (int32_t)(0u - ((uint32_t)(((int64_t)playerObject->cachedUpX * relativeX) >>
										   FVIEW_MATRIX_FRACTION_BITS) +
								(uint32_t)(((int64_t)playerObject->cachedUpY * relativeY) >>
										   FVIEW_MATRIX_FRACTION_BITS) +
								(uint32_t)(((int64_t)playerObject->cachedUpZ * relativeZ) >>
										   FVIEW_MATRIX_FRACTION_BITS)));
	}
#else
	viewX =
		(int32_t)((uint32_t)(((int64_t)playerObject->cachedSideX * relativeX) >> FVIEW_MATRIX_FRACTION_BITS) +
				  (uint32_t)(((int64_t)playerObject->cachedSideY * relativeY) >> FVIEW_MATRIX_FRACTION_BITS) +
				  (uint32_t)(((int64_t)playerObject->cachedSideZ * relativeZ) >> FVIEW_MATRIX_FRACTION_BITS));
	viewY =
		(int32_t)(0u -
				  ((uint32_t)(((int64_t)playerObject->cachedUpX * relativeX) >> FVIEW_MATRIX_FRACTION_BITS) +
				   (uint32_t)(((int64_t)playerObject->cachedUpY * relativeY) >> FVIEW_MATRIX_FRACTION_BITS) +
				   (uint32_t)(((int64_t)playerObject->cachedUpZ * relativeZ) >> FVIEW_MATRIX_FRACTION_BITS)));
#endif
	absoluteDepth = viewDepth;
	if (viewDepth < 0) {
#ifdef XW_MODERN
		absoluteDepth = (int32_t)(0u - (uint32_t)viewDepth);
#else
		absoluteDepth = -viewDepth;
#endif
		frontHemisphere = 0;
	} else {
		frontHemisphere = 1;
	}
	if (objectRef >= XW_MISSION_OBJECT_REF_BASE) {
		g_radarBlipColorIndex = PANEL_RADAR_STATIC_COLOR;
#ifdef XW_MODERN
	} else if (g_objectTable[objectIndex].objectType ==
			   XwFlightTypes_ObjectType(PANEL_DYNAMIC_BUOY_OBJECT_TYPE)) {
#else
	} else if (g_objectTable[objectIndex].objectType == PANEL_DYNAMIC_BUOY_OBJECT_TYPE) {
#endif
		g_radarBlipColorIndex = PANEL_RADAR_STATIC_COLOR;
	} else if (g_objectTable[objectIndex].familyId == PANEL_RADAR_PROJECTILE_FAMILY) {
		g_radarBlipColorIndex = PANEL_RADAR_PROJECTILE_COLOR;
	} else if (g_objectTable[objectIndex].iff == 0) {
		g_radarBlipColorIndex = PANEL_RADAR_IFF0_COLOR;
	} else if (g_objectTable[objectIndex].iff == 1) {
		g_radarBlipColorIndex = PANEL_RADAR_IFF1_COLOR;
	} else {
		g_radarBlipColorIndex = PANEL_RADAR_OTHER_COLOR;
	}
	pai_roughdistancebetween(g_playerFlightState.objectIndex, objectRef);
	if (g_targetRangeScore > PANEL_RADAR_FAR_RANGE) {
		if (g_radarBlipColorIndex == PANEL_RADAR_STATIC_COLOR)
			g_radarBlipColorIndex = PANEL_RADAR_STATIC_FAR_COLOR;
		else
			g_radarBlipColorIndex -= 2;
	} else if (g_targetRangeScore > PANEL_RADAR_MIDDLE_RANGE) {
		if (g_radarBlipColorIndex == PANEL_RADAR_STATIC_COLOR)
			g_radarBlipColorIndex = PANEL_RADAR_STATIC_MIDDLE_COLOR;
		else
			--g_radarBlipColorIndex;
	}
#ifdef XW_MODERN
	if (XwFlightTypes_Dos())
		Dos94_math2_getradarcoord(viewX, viewY, absoluteDepth);
	else
#endif
		math2_getradarcoord(viewX, viewY, absoluteDepth);
	if (g_playerFlightState.currentTargetObjectIdx == objectRef) {
		if (frontHemisphere != 0) {
			targetY = g_radarProjectedY;
#ifdef XW_MODERN
			if (targetY >= layout->radarYLimit)
				targetY = layout->radarYLimit;
			if (targetY <= -layout->radarYLimit)
				targetY = -layout->radarYLimit;
			g_targetComputerMarkerScreenX =
				g_radarProjectedX + g_hudElementLayouts[PANEL_RADAR_TARGET_ELEMENT].x + layout->radarCenterX;
#else
			if (targetY >= PANEL_RADAR_TARGET_Y_LIMIT)
				targetY = PANEL_RADAR_TARGET_Y_LIMIT;
			if (targetY <= -PANEL_RADAR_TARGET_Y_LIMIT)
				targetY = -PANEL_RADAR_TARGET_Y_LIMIT;
			g_targetComputerMarkerScreenX = g_radarProjectedX +
											g_hudElementLayouts[PANEL_RADAR_TARGET_ELEMENT].x +
											PANEL_RADAR_TARGET_CENTER_X;
#endif
			g_targetComputerMarkerScreenY =
#ifdef XW_MODERN
				targetY + g_hudElementLayouts[PANEL_RADAR_TARGET_ELEMENT].y + layout->radarCenterY;
#else
				targetY + g_hudElementLayouts[PANEL_RADAR_TARGET_ELEMENT].y + PANEL_RADAR_TARGET_CENTER_Y;
#endif
		} else {
			g_targetComputerMarkerScreenX = -1;
		}
	}
	if (frontHemisphere != 0) {
		g_radarProjectedY += g_hudElementLayouts[PANEL_RADAR_FRONT_ELEMENT].y;
		g_radarProjectedX += g_hudElementLayouts[PANEL_RADAR_FRONT_ELEMENT].x;
		g_radarFrontBlips[g_radarFrontBlipCount].x = g_radarProjectedX;
		g_radarFrontBlips[g_radarFrontBlipCount].y = g_radarProjectedY;
		g_radarFrontBlips[g_radarFrontBlipCount].colorOrDrawMask = g_radarBlipColorIndex;
		++g_radarFrontBlipCount;
		if (g_radarFrontBlipCount == PANEL_RADAR_BLIP_CAPACITY)
			g_radarFrontBlipCount = PANEL_RADAR_BLIP_CAPACITY - 1;
	} else {
		g_radarProjectedY += g_hudElementLayouts[PANEL_RADAR_REAR_ELEMENT].y;
		g_radarProjectedX += g_hudElementLayouts[PANEL_RADAR_REAR_ELEMENT].x;
		g_radarRearBlips[g_radarRearBlipCount].x = g_radarProjectedX;
		g_radarRearBlips[g_radarRearBlipCount].y = g_radarProjectedY;
		g_radarRearBlips[g_radarRearBlipCount].colorOrDrawMask = g_radarBlipColorIndex;
		++g_radarRearBlipCount;
		if (g_radarRearBlipCount == PANEL_RADAR_BLIP_CAPACITY)
			g_radarRearBlipCount = PANEL_RADAR_BLIP_CAPACITY - 1;
	}
	if (g_playerFlightState.currentTargetObjectIdx == objectRef) {
		g_radarSelectedTargetScreenX = g_radarProjectedX;
		g_radarSelectedTargetScreenY = g_radarProjectedY;
	}
}

// FUNCTION: XW 0x419830
void panel_updatecmd(void) {
#ifdef XW_MODERN
	int hud_pane = XwHud_Push(XW_SNAP_PANE_TARGET_BODY);
#endif

#ifdef XW_MODERN
	const XwFlightPanelLayout* layout = XwFlightPanel_Layout();
#endif
	int16_t panelX = g_hudElementLayouts[PANEL_RADAR_TARGET_ELEMENT].x;
	int16_t panelY = g_hudElementLayouts[PANEL_RADAR_TARGET_ELEMENT].y;
	int16_t exhaustPortMode;
#ifdef XW_MODERN
	int16_t exhaustPortInRange = 0;
	unsigned int polarDistance = 0;
#else
	int16_t exhaustPortInRange;
	unsigned int polarDistance;
#endif
	uint16_t objectType = XW_OBJ_NONE;
	uint16_t previousDisplayedTargetRef;
	CraftData* targetCraft = NULL;
	WarheadGuidanceState* targetGuidance = NULL;
	g_flightTextShadowEnabled = 0;
	if (g_missionRuntimeState.provingGroundsActive != 0) {
		gate_trainingupdatecrt(panelX, panelY);
		{
#ifdef XW_MODERN
			XwHud_Pop(hud_pane);
#endif
			return;
		}
	}
	if (!(g_playerFlightState.craft->activeHudFeatureMask & PANEL_CMD_FEATURE)) {
#ifdef XW_MODERN
		XwHud_Pop(hud_pane);
#endif
		return;
	}
	if (g_hudExhaustPortDisplayMode == 1 ||
		(g_deathStarSurfaceModeActive != 0 && g_objectTable[g_playerFlightState.objectIndex].worldZ < 0)) {
		exhaustPortMode = 1;
#ifdef XW_MODERN
		XwFlightCamera_CartesianToPolar(
			g_playerFlightState.object->worldX,
#else
		trig2_ctop(g_playerFlightState.object->worldX,
#endif
			(int)((uint32_t)PANEL_CMD_EXHAUST_Y - g_playerFlightState.object->worldY),
			g_playerFlightState.object->worldZ);
		polarDistance = (unsigned int)g_trig2PolarDistance;
		exhaustPortInRange = polarDistance < PANEL_CMD_EXHAUST_RANGE;
	} else
		exhaustPortMode = 0;
	/* The original reads an uninitialized stack word here when the target has not changed. */
#ifdef XW_MODERN
	previousDisplayedTargetRef = XwFlightTypes_Dos() ? 0 : g_hudCachedTargetObjectIdx;
	if ((g_hudExhaustPortDisplayMode != exhaustPortMode && exhaustPortMode != 0) ||
		(XwFlightTypes_Dos() && g_hudCachedTargetObjectIdx == (uint16_t)PANEL_ELEMENT_CACHE_INVALID &&
		 exhaustPortMode && !g_playerFlightState.hudTargetDetailsEnabled)) {
#else
	previousDisplayedTargetRef = g_hudCachedTargetObjectIdx;
	/* Its zero-extended target-cache comparison against signed -2 can never succeed. */
	if (g_hudExhaustPortDisplayMode != exhaustPortMode && exhaustPortMode != 0) {
#endif
		g_hudExhaustPortDisplayMode = (uint8_t)exhaustPortMode;
		g_playerFlightState.hudTargetDetailsEnabled = 0;
		festring_setfontsize(FLIGHT_FONT_MICRO);
#ifdef XW_MODERN
		festring_setbound(panelX, panelY, panelX + layout->width, panelY + layout->height);
#else
		festring_setbound(panelX, panelY, panelX + PANEL_CMD_WIDTH, panelY + PANEL_CMD_HEIGHT);
#endif
		festring_setcursor(panelX, panelY);
		festring_setautofill(1);
		festring_setbackcolor(PANEL_CMD_BACKGROUND);
		g_flightFillClipRectFn();
#ifdef XW_MODERN
		festring_setbound(panelX, panelY, panelX + layout->width, panelY + layout->titleHeight);
#else
		festring_setbound(panelX, panelY, panelX + PANEL_CMD_WIDTH, panelY + PANEL_CMD_TITLE_HEIGHT);
#endif
		festring_settextcolor(PANEL_CMD_TITLE_COLOR);
		festring_outstringcenter("EXHAUST PORT");
		festring_setbackcolor(PANEL_CMD_BACKGROUND);
#ifdef XW_MODERN
		festring_setcursor(panelX, panelY + layout->height);
		if (!XwFlightTypes_Dos())
			festring_setbound(panelX, panelY + layout->height, panelX + PANEL_CMD_DISTANCE_LABEL_WIDTH,
							  panelY + layout->footerBottom);
#else
		festring_setcursor(panelX, panelY + PANEL_CMD_HEIGHT);
		festring_setbound(panelX, panelY + PANEL_CMD_HEIGHT, panelX + PANEL_CMD_DISTANCE_LABEL_WIDTH,
						  panelY + PANEL_CMD_FOOTER_BOTTOM);
#endif
		festring_settextcolor(PANEL_CMD_DISTANCE_COLOR);
		festring_outstring("DIS:   .");
		g_hudElementStateCache[PANEL_DISTANCE_WHOLE_ELEMENT] = -1;
		g_hudCachedTargetObjectIdx = XW_OBJECT_SLOT_UNAVAILABLE;
	} else if ((exhaustPortMode == 0 || g_playerFlightState.hudTargetDetailsEnabled != 0) &&
			   g_hudCachedTargetObjectIdx != g_playerFlightState.currentTargetObjectIdx) {
		previousDisplayedTargetRef = g_hudCachedTargetObjectIdx;
		g_hudCachedTargetObjectIdx = g_playerFlightState.currentTargetObjectIdx;
		if (g_playerFlightState.currentTargetObjectIdx != XW_OBJECT_SLOT_UNAVAILABLE) {
			if (g_playerFlightState.currentTargetObjectIdx < XW_MISSION_OBJECT_REF_BASE)
				objectType = g_objectTable[g_playerFlightState.currentTargetObjectIdx].objectType;
			else
				objectType =
					g_missionObjects[g_playerFlightState.currentTargetObjectIdx - XW_MISSION_OBJECT_REF_BASE]
						.objectType;
		}
		g_hudElementStateCache[PANEL_CMD_CARGO_ELEMENT] = -1;
		g_hudElementStateCache[PANEL_DISTANCE_WHOLE_ELEMENT] = -1;
		g_hudElementStateCache[PANEL_DISTANCE_FRACTION_ELEMENT] = -1;
		g_hudElementStateCache[PANEL_CMD_STATUS_ELEMENT] = -1;
		festring_setfontsize(FLIGHT_FONT_MICRO);
#ifdef XW_MODERN
		festring_setbound(panelX, panelY, panelX + layout->width, panelY + layout->height);
#else
		festring_setbound(panelX, panelY, panelX + PANEL_CMD_WIDTH, panelY + PANEL_CMD_HEIGHT);
#endif
		festring_setcursor(panelX, panelY);
		festring_setautofill(1);
		festring_setbackcolor(PANEL_CMD_BACKGROUND);
		g_flightFillClipRectFn();
#ifdef XW_MODERN
		festring_setbound(panelX, panelY, panelX + layout->width, panelY + layout->titleHeight);
#else
		festring_setbound(panelX, panelY, panelX + PANEL_CMD_WIDTH, panelY + PANEL_CMD_TITLE_HEIGHT);
#endif
		if (g_playerFlightState.currentTargetObjectIdx != XW_OBJECT_SLOT_UNAVAILABLE) {
			if (g_playerFlightState.currentTargetObjectIdx < XW_MISSION_OBJECT_REF_BASE) {
				targetCraft = g_objectTable[g_playerFlightState.currentTargetObjectIdx].instanceData;
				targetGuidance = g_objectTable[g_playerFlightState.currentTargetObjectIdx].instanceData;
			}
			panel_buildobjectname(g_playerFlightState.currentTargetObjectIdx, objectType);
			festring_outstringcenter(g_flightTextScratchBuffer);
		}
		if (g_playerFlightState.currentTargetObjectIdx != XW_OBJECT_SLOT_UNAVAILABLE) {
			if (g_playerFlightState.hudTargetDetailsEnabled == 0) {
				if (exhaustPortMode == 0) {
					g_flightBlitSpriteFn(g_hudPanelSpriteDataByIndex[PANEL_CMD_RETICLE_SPRITE], panelX,
#ifdef XW_MODERN
										 panelY + layout->reticleTop, PANEL_SETTING_TRANSPARENT_COLOR, 0);
#else
										 panelY + PANEL_CMD_RETICLE_TOP, PANEL_SETTING_TRANSPARENT_COLOR, 0);
#endif
					g_targetComputerLockSpriteIndex = PANEL_CMD_UNLOCKED_SPRITE;
					g_targetComputerCrossBackgroundSaved = 0;
				}
			} else {
				uint16_t hudShipId, spriteIndex;
				g_flightBlitSpriteFn(g_hudPanelSpriteDataByIndex[PANEL_CMD_DETAIL_SPRITE],
#ifdef XW_MODERN
									 panelX + layout->detailLeft, panelY + layout->titleHeight,
#else
									 panelX + PANEL_CMD_DETAIL_LEFT, panelY + PANEL_CMD_TITLE_HEIGHT,
#endif
									 PANEL_SETTING_TRANSPARENT_COLOR, 0);
				g_flightBlitSpriteFn(
					g_hudPanelSpriteDataByIndex[(g_hudCachedTargetObjectIdx & PANEL_CMD_VARIANT_MASK) +
												PANEL_CMD_VARIANT_SPRITE_BASE],
#ifdef XW_MODERN
					panelX + layout->variantLeft, panelY + layout->variantTop,
#else
					panelX + PANEL_CMD_VARIANT_LEFT, panelY + PANEL_CMD_VARIANT_TOP,
#endif
					PANEL_SETTING_TRANSPARENT_COLOR, 0);
#ifdef XW_MODERN
				if (XwFlightTypes_Dos()) {
					spriteIndex = XwFlightTypes_IsMine(objectType) ? 34
								  : objectType == XwFlightTypes_ObjectType(XW_OBJ_B_WING)
									  ? PANEL_CMD_BWING_SPRITE
								  : objectType == XwFlightTypes_ObjectType(XW_OBJ_INTERDICTOR)
									  ? PANEL_CMD_INTERDICTOR_SPRITE
									  : objectType + PANEL_CMD_SHIP_SPRITE_BASE;
				} else {
#endif
					hudShipId = g_objectTypeHudShipIds[objectType];
					if (objectType >= PANEL_CMD_SHARED_SHIP_FIRST &&
						objectType <= PANEL_CMD_SHARED_SHIP_LAST) {
						hudShipId = g_objectTypeHudShipIds[PANEL_CMD_SHARED_SHIP_FIRST];
						spriteIndex = hudShipId + PANEL_CMD_SHIP_SPRITE_BASE;
					} else if (objectType == XW_OBJ_B_WING)
						spriteIndex = PANEL_CMD_BWING_SPRITE;
					else if (objectType == XW_OBJ_INTERDICTOR)
						spriteIndex = PANEL_CMD_INTERDICTOR_SPRITE;
					else
						spriteIndex = hudShipId + PANEL_CMD_SHIP_SPRITE_BASE;
#ifdef XW_MODERN
				}
#endif
				g_flightBlitSpriteFn(g_hudPanelSpriteDataByIndex[spriteIndex], panelX,
#ifdef XW_MODERN
									 panelY + layout->titleHeight, PANEL_SETTING_TRANSPARENT_COLOR, 0);
#else
									 panelY + PANEL_CMD_TITLE_HEIGHT, PANEL_SETTING_TRANSPARENT_COLOR, 0);
#endif
			}
#ifdef XW_MODERN
			if (objectType == XwFlightTypes_ObjectType(XW_OBJ_TRACKED_WARHEAD) ||
				objectType == XwFlightTypes_ObjectType(XW_OBJ_WARHEAD_149)) {
#else
			if (objectType == XW_OBJ_TRACKED_WARHEAD || objectType == XW_OBJ_WARHEAD_149) {
#endif
				WarheadGuidanceState* guidance =
					(WarheadGuidanceState*)g_objectTable[g_playerFlightState.currentTargetObjectIdx]
						.instanceData;
				targetGuidance = guidance;
				if (guidance->homingTier != 0) {
					uint16_t warheadTarget = guidance->targetObjIdx;
					if (warheadTarget == g_playerFlightState.objectIndex) {
						festring_settextcolor(PANEL_CMD_PLAYER_COLOR);
						festring_farstrcpy("OUR CRAFT");
					} else
						panel_buildobjectname(warheadTarget, objectType);
				} else {
					festring_settextcolor(PANEL_CMD_CARGO_COLOR);
					festring_farstrcpy("NONE");
				}
#ifdef XW_MODERN
				festring_setbound(panelX + layout->warheadLeft, panelY + layout->warheadTop,
								  panelX + layout->width, panelY + layout->height);
#else
				festring_setbound(panelX + PANEL_CMD_WARHEAD_LEFT, panelY + PANEL_CMD_WARHEAD_TOP,
								  panelX + PANEL_CMD_WIDTH, panelY + PANEL_CMD_HEIGHT);
#endif
				g_flightFillClipRectFn();
#ifdef XW_MODERN
				festring_setcursor(panelX + layout->warheadLeft, panelY + layout->warheadTop);
#else
				festring_setcursor(panelX + PANEL_CMD_WARHEAD_LEFT, panelY + PANEL_CMD_WARHEAD_TOP);
#endif
				festring_outstringright(g_flightTextScratchBuffer);
			}
		} else {
#ifdef XW_MODERN
			festring_setbound(panelX, panelY + layout->height, panelX + layout->width,
							  panelY + layout->footerBottom);
#else
			festring_setbound(panelX, panelY + PANEL_CMD_HEIGHT, panelX + PANEL_CMD_WIDTH,
							  panelY + PANEL_CMD_FOOTER_BOTTOM);
#endif
			g_flightFillClipRectFn();
		}
	}
	if (g_playerFlightState.hudTargetDetailsEnabled == 0) {
		if (g_playerFlightState.currentTargetObjectIdx != XW_OBJECT_SLOT_UNAVAILABLE &&
			exhaustPortMode == 0) {
			uint16_t lockSprite;
			if (g_targetComputerCrossBackgroundSaved != 0)
				rtsvga2_removecross(g_targetComputerPreviousCrossX, g_targetComputerPreviousCrossY);
			if (g_targetLockActive != 0) {
				lockSprite = PANEL_CMD_LOCK_SPRITE;
				g_targetComputerLockSpriteIndex = lockSprite;
#ifdef XW_MODERN
				g_targetComputerMarkerScreenX = panelX + layout->radarCenterX;
				g_targetComputerMarkerScreenY = panelY + layout->radarCenterY;
#else
				g_targetComputerMarkerScreenX = panelX + PANEL_RADAR_TARGET_CENTER_X;
				g_targetComputerMarkerScreenY = panelY + PANEL_RADAR_TARGET_CENTER_Y;
#endif
			} else {
				lockSprite = PANEL_CMD_UNLOCKED_SPRITE;
				g_targetComputerLockSpriteIndex = lockSprite;
			}
			if (lockSprite != g_targetComputerCachedLockSpriteIndex) {
#ifdef XW_MODERN
				g_flightBlitSpriteFn(g_hudPanelSpriteDataByIndex[lockSprite], panelX + layout->lockLeft,
									 panelY + layout->lockTop, PANEL_SETTING_TRANSPARENT_COLOR, 0);
#else
				g_flightBlitSpriteFn(g_hudPanelSpriteDataByIndex[lockSprite], panelX + PANEL_CMD_LOCK_LEFT,
									 panelY + PANEL_CMD_LOCK_TOP, PANEL_SETTING_TRANSPARENT_COLOR, 0);
#endif
				g_targetComputerCachedLockSpriteIndex = g_targetComputerLockSpriteIndex;
			}
			if (g_targetComputerMarkerScreenX > 0) {
				g_targetComputerCrossBackgroundSaved = 1;
				rtsvga2_drawcross(g_targetComputerMarkerScreenX, g_targetComputerMarkerScreenY,
								  PANEL_CMD_CROSS_COLOR);
				g_targetComputerPreviousCrossX = g_targetComputerMarkerScreenX;
				g_targetComputerPreviousCrossY = g_targetComputerMarkerScreenY;
			} else
				g_targetComputerCrossBackgroundSaved = 0;
		} else if (exhaustPortMode != 0) {
			uint16_t frame;
			if (exhaustPortInRange)
				frame = PANEL_CMD_EXHAUST_NEAR_FRAME;
			else {
				uint16_t step = (uint8_t)(polarDistance >> PANEL_CMD_EXHAUST_STEP_SHIFT) /
								PANEL_CMD_EXHAUST_FRAME_DIVISOR;
				if (step >= PANEL_CMD_EXHAUST_NEAR_FRAME)
					step = PANEL_CMD_EXHAUST_LAST_STEP;
				frame = PANEL_CMD_EXHAUST_LAST_STEP - step;
			}
			if (g_hudCachedExhaustPortFrame != frame) {
				g_hudCachedExhaustPortFrame = frame;
				g_flightBlitSpriteFn(g_hudPanelSpriteDataByIndex[frame + PANEL_CMD_EXHAUST_SPRITE_BASE],
#ifdef XW_MODERN
									 panelX, panelY + layout->titleHeight, PANEL_SETTING_TRANSPARENT_COLOR,
#else
									 panelX, panelY + PANEL_CMD_TITLE_HEIGHT, PANEL_SETTING_TRANSPARENT_COLOR,
#endif
									 0);
			}
#ifdef XW_MODERN
			festring_setbound(panelX + layout->distanceLeft, panelY + layout->height,
							  panelX + layout->distanceRight, panelY + layout->footerBottom);
#else
			festring_setbound(panelX + PANEL_CMD_DISTANCE_LEFT, panelY + PANEL_CMD_HEIGHT,
							  panelX + PANEL_CMD_DISTANCE_RIGHT, panelY + PANEL_CMD_FOOTER_BOTTOM);
#endif
			panel_outputdistance(polarDistance, panelX, panelY);
		}
	}
	if (g_playerFlightState.currentTargetObjectIdx != XW_OBJECT_SLOT_UNAVAILABLE &&
		(exhaustPortMode == 0 || g_playerFlightState.hudTargetDetailsEnabled != 0)) {
		if (g_hudCachedTargetObjectIdx < XW_MISSION_OBJECT_REF_BASE) {
			objectType = g_objectTable[g_hudCachedTargetObjectIdx].objectType;
			{
				targetCraft = g_objectTable[g_playerFlightState.currentTargetObjectIdx].instanceData;
				targetGuidance = g_objectTable[g_playerFlightState.currentTargetObjectIdx].instanceData;
			}
		} else
			objectType = g_missionObjects[g_hudCachedTargetObjectIdx - XW_MISSION_OBJECT_REF_BASE].objectType;
		festring_setbackcolor(PANEL_CMD_BACKGROUND);
		if (previousDisplayedTargetRef == XW_OBJECT_SLOT_UNAVAILABLE) {
#ifdef XW_MODERN
			festring_setcursor(panelX, panelY + layout->height);
			if (!XwFlightTypes_Dos())
				festring_setbound(panelX, panelY + layout->height, panelX + PANEL_CMD_DISTANCE_LABEL_WIDTH,
								  panelY + layout->footerBottom);
#else
			festring_setcursor(panelX, panelY + PANEL_CMD_HEIGHT);
			festring_setbound(panelX, panelY + PANEL_CMD_HEIGHT, panelX + PANEL_CMD_DISTANCE_LABEL_WIDTH,
							  panelY + PANEL_CMD_FOOTER_BOTTOM);
#endif
			festring_settextcolor(PANEL_CMD_DISTANCE_COLOR);
			festring_outstring("DIS:   .");
			g_hudElementStateCache[PANEL_DISTANCE_WHOLE_ELEMENT] = -1;
		}
		pai_distancebetween(g_playerFlightState.objectIndex, g_hudCachedTargetObjectIdx);
#ifdef XW_MODERN
		festring_setbound(panelX + layout->distanceLeft, panelY + layout->height,
						  panelX + layout->distanceRight, panelY + layout->footerBottom);
#else
		festring_setbound(panelX + PANEL_CMD_DISTANCE_LEFT, panelY + PANEL_CMD_HEIGHT,
						  panelX + PANEL_CMD_DISTANCE_RIGHT, panelY + PANEL_CMD_FOOTER_BOTTOM);
#endif
		panel_outputdistance(g_trig2PolarDistance, panelX, panelY);
		if (g_hudCachedTargetObjectIdx < XW_MISSION_OBJECT_REF_BASE &&
			g_playerFlightState.hudTargetDetailsEnabled != 0 &&
			g_playerFlightState.currentTargetObjectIdx < XW_OBJECT_COUNT &&
			g_objectTable[g_playerFlightState.currentTargetObjectIdx].familyId == 0) {
			if (g_craftTypeDefs[targetCraft->craftTypeIndex].hasCargo != 0) {
				uint16_t cargoState;
				char* cargoText;
				if (targetCraft->isInspected != 0) {
					cargoState = PANEL_CMD_CARGO_KNOWN;
					cargoText = targetCraft->cargoName;
					if (cargoText[0] == 0) {
						cargoText = "NONE";
						cargoState = PANEL_CMD_CARGO_EMPTY;
					}
				} else {
					cargoText = "UNKNOWN";
					cargoState = PANEL_CMD_CARGO_UNKNOWN;
				}
				if (g_hudElementStateCache[PANEL_CMD_CARGO_ELEMENT] != cargoState) {
					g_hudElementStateCache[PANEL_CMD_CARGO_ELEMENT] = cargoState;
#ifdef XW_MODERN
					festring_setbound(panelX + layout->cargoLeft, panelY + layout->cargoTop,
									  panelX + layout->width, panelY + layout->height);
#else
					festring_setbound(panelX + PANEL_CMD_CARGO_LEFT, panelY + PANEL_CMD_CARGO_TOP,
									  panelX + PANEL_CMD_WIDTH, panelY + PANEL_CMD_HEIGHT);
#endif
					g_flightFillClipRectFn();
#ifdef XW_MODERN
					festring_setcursor(panelX + layout->cargoLeft, panelY + layout->cargoTop);
#else
					festring_setcursor(panelX + PANEL_CMD_CARGO_LEFT, panelY + PANEL_CMD_CARGO_TOP);
#endif
					festring_settextcolor(PANEL_CMD_CARGO_COLOR);
					festring_outstringright(cargoText);
				}
			}
		}
	}
	if ((exhaustPortMode != 0 && g_playerFlightState.hudTargetDetailsEnabled == 0) ||
		g_playerFlightState.currentTargetObjectIdx != XW_OBJECT_SLOT_UNAVAILABLE) {
		uint16_t status;
		if (exhaustPortMode != 0 && g_playerFlightState.hudTargetDetailsEnabled == 0)
			status = (exhaustPortInRange != 0) + PANEL_CMD_EXHAUST_STATUS_BASE;
#ifdef XW_MODERN
		else if (objectType == XwFlightTypes_ObjectType(XW_OBJ_TRACKED_WARHEAD) ||
				 objectType == XwFlightTypes_ObjectType(XW_OBJ_WARHEAD_149))
#else
		else if (objectType == XW_OBJ_TRACKED_WARHEAD || objectType == XW_OBJ_WARHEAD_149)
#endif
			status = (targetGuidance->homingTier != 0) + PANEL_CMD_WARHEAD_STATUS_BASE;
#ifdef XW_MODERN
		else if (g_modelTypeTable[objectType].familyId == 0 &&
				 (!XwFlightTypes_Dos() ||
				  (g_playerFlightState.currentTargetObjectIdx < XW_OBJECT_COUNT && targetCraft)))
#else
		else if (g_modelTypeTable[objectType].familyId == 0)
#endif
			status = panel_getcraftstatus(g_playerFlightState.currentTargetObjectIdx);
		else
			status = PANEL_CRAFT_STATUS_NORMAL;
		festring_settextcolor(PANEL_CMD_STATUS_COLOR);
		if (g_hudElementStateCache[PANEL_CMD_STATUS_ELEMENT] != status) {
			g_hudElementStateCache[PANEL_CMD_STATUS_ELEMENT] = status;
#ifdef XW_MODERN
			festring_setbound(panelX + layout->distanceRight, panelY + layout->height, panelX + layout->width,
							  panelY + layout->statusBottom);
#else
			festring_setbound(panelX + PANEL_CMD_DISTANCE_RIGHT, panelY + PANEL_CMD_HEIGHT,
							  panelX + PANEL_CMD_WIDTH, panelY + PANEL_CMD_STATUS_BOTTOM);
#endif
			g_flightFillClipRectFn();
#ifdef XW_MODERN
			festring_setcursor(panelX + layout->distanceRight, panelY + layout->height);
			festring_outstringright(XwFlightTypes_Dos() ? Dos94_panelStatusStrings[status]
														: g_ReplayStatusStrings[status]);
#else
			festring_setcursor(panelX + PANEL_CMD_DISTANCE_RIGHT, panelY + PANEL_CMD_HEIGHT);
			festring_outstringright(g_ReplayStatusStrings[status]);
#endif
		}
	}

#ifdef XW_MODERN
	XwHud_Pop(hud_pane);
#endif
}

// FUNCTION: XW 0x41A1C0
void panel_buildobjectname(uint16_t objectRef, uint16_t objectType) {
	g_flightTextScratchBuffer[0] = 0;
	if (objectRef < XW_MISSION_OBJECT_REF_BASE) {
		festring_farstradd(FLIGHT_TEXT_COLOR_ESCAPE);
		if (g_objectTable[objectRef].iff == 0)
			festring_farstradd(PANEL_OBJECT_TYPE_IFF0_COLOR);
		else if (g_objectTable[objectRef].iff == 1)
			festring_farstradd(PANEL_OBJECT_TYPE_IFF1_COLOR);
		else
			festring_farstradd(PANEL_OBJECT_TYPE_OTHER_COLOR);
		if (g_objectTable[objectRef].familyId == XW_OBJECT_FAMILY_CRAFT) {
			CraftData* craft;
			craft = (CraftData*)g_objectTable[objectRef].instanceData;
			festring_farstrcat(g_craftTypeDefs[craft->craftTypeIndex].shortName);
			festring_farstradd(':');
			festring_farstradd(' ');
			festring_farstradd(FLIGHT_TEXT_COLOR_ESCAPE);
			if (g_objectTable[objectRef].iff == 0)
				festring_farstradd(PANEL_OBJECT_NAME_IFF0_COLOR);
			else if (g_objectTable[objectRef].iff == 1)
				festring_farstradd(PANEL_OBJECT_NAME_IFF1_COLOR);
			else
				festring_farstradd(PANEL_OBJECT_NAME_OTHER_COLOR);
			if (g_objectTable[objectRef].genusId != XW_GENUS_STARFIGHTER && !craft->isInspected &&
				g_objectTable[objectRef].iff != g_playerFlightState.object->iff) {
				festring_farstrcat("UNKNOWN");
			} else {
				festring_farstrcat(g_missionFlightGroups[craft->flightGroupIndex].name);
				if (g_missionFlightGroups[craft->flightGroupIndex].numberOfCraft > 1) {
					festring_farstradd(' ');
					festring_farstradd(craft->craftIndexInFlightGroup + '1');
				}
			}
		} else {
#ifdef XW_MODERN
			if (objectType == XwFlightTypes_ObjectType(XW_OBJ_WARHEAD_149))
#else
			if (objectType == XW_OBJ_WARHEAD_149)
#endif
				festring_farstrcat("TORPEDO");
#ifdef XW_MODERN
			else if (objectType == XwFlightTypes_ObjectType(XW_OBJ_TRACKED_WARHEAD))
#else
			else if (objectType == XW_OBJ_TRACKED_WARHEAD)
#endif
				festring_farstrcat("MISSILE");
#ifdef XW_MODERN
			else if (objectType == XwFlightTypes_ObjectType(PANEL_DYNAMIC_BUOY_OBJECT_TYPE))
				festring_farstrcat(g_hudBuoyNameStrings[(
					XwFlightTypes_Dos() ? 1
										: g_objectTypeHudShipIds[PANEL_DYNAMIC_BUOY_OBJECT_TYPE] -
											  g_objectTypeHudShipIds[PANEL_BUOY_BASE_OBJECT_TYPE])]);
#else
			else if (objectType == PANEL_DYNAMIC_BUOY_OBJECT_TYPE)
				festring_farstrcat(
					g_hudBuoyNameStrings[g_objectTypeHudShipIds[PANEL_DYNAMIC_BUOY_OBJECT_TYPE] -
										 g_objectTypeHudShipIds[PANEL_BUOY_BASE_OBJECT_TYPE]]);
#endif
		}
	} else {
		festring_farstradd(FLIGHT_TEXT_COLOR_ESCAPE);
		festring_farstradd(PANEL_STATIC_OBJECT_COLOR);
#ifdef XW_MODERN
		if (XwFlightTypes_IsMine(objectType)) {
#else
		if (objectType >= PANEL_MINE_OBJECT_TYPE_FIRST && objectType <= PANEL_MINE_OBJECT_TYPE_LAST) {
#endif
			festring_farstrcat("MINE");
		} else {
#ifdef XW_MODERN
			uint16_t hudId = XwFlightTypes_Dos() ? objectType : g_objectTypeHudShipIds[objectType];
			if (hudId >= PANEL_BUOY_HUD_ID_FIRST && hudId <= PANEL_BUOY_HUD_ID_LAST)
				festring_farstrcat(
					g_hudBuoyNameStrings[hudId -
										 (XwFlightTypes_Dos()
											  ? PANEL_BUOY_HUD_ID_FIRST
											  : g_objectTypeHudShipIds[PANEL_BUOY_BASE_OBJECT_TYPE])]);
#else
			if (g_objectTypeHudShipIds[objectType] >= PANEL_BUOY_HUD_ID_FIRST &&
				g_objectTypeHudShipIds[objectType] <= PANEL_BUOY_HUD_ID_LAST)
				festring_farstrcat(g_hudBuoyNameStrings[g_objectTypeHudShipIds[objectType] -
														g_objectTypeHudShipIds[PANEL_BUOY_BASE_OBJECT_TYPE]]);
#endif
		}
	}
}

// FUNCTION: XW 0x41A3E0
int panel_getcraftstatus(uint16_t objectIndex) {
	CraftData* craft = g_objectTable[objectIndex].instanceData;
	if (craft->workingSubsystems == 0) {
		return PANEL_CRAFT_STATUS_DISABLED;
	}
	if (craft->captorFlightGroupOverride != 0) {
		return PANEL_CRAFT_STATUS_CAPTURED;
	}
	if (craft->hullDamage >= craft->systemDamageHullThreshold) {
		return PANEL_CRAFT_STATUS_DAMAGED;
	}
	if (g_craftTypeDefs[craft->craftTypeIndex].nominalShieldEnergy[XW_SHIELD_FRONT] != 0 &&
		craft->shieldEnergy[XW_SHIELD_FRONT] + craft->shieldEnergy[XW_SHIELD_REAR] == 0) {
		return PANEL_CRAFT_STATUS_SHIELDS_DOWN;
	}
	if (craft->aiCurrentPlanId == PAI_PLAN_66 || craft->aiCurrentPlanId == PAI_PLAN_67) {
		return PANEL_CRAFT_STATUS_8;
	}
	return PANEL_CRAFT_STATUS_NORMAL;
}

// FUNCTION: XW 0x41A470
void panel_outputdistance(int polarDistance, int16_t panelX, int16_t panelY) {
#ifdef XW_MODERN
	const XwFlightPanelLayout* layout = XwFlightPanel_Layout();
#endif
	unsigned int distanceHundredths;
	int wholeDistance;
	g_flightTextShadowEnabled = 0;
	festring_settextcolor(PANEL_DISTANCE_TEXT_COLOR);
	distanceHundredths =
		(uint16_t)(((unsigned int)polarDistance * PANEL_DISTANCE_SCALE) >> PANEL_DISTANCE_SCALE_SHIFT);
	if ((uint16_t)distanceHundredths >= PANEL_DISTANCE_MAX_HUNDREDTHS + 1) {
		distanceHundredths = PANEL_DISTANCE_MAX_HUNDREDTHS;
	}
	wholeDistance = (uint16_t)distanceHundredths / PANEL_DISTANCE_HUNDREDTHS_PER_UNIT;
	if (g_hudElementStateCache[PANEL_DISTANCE_WHOLE_ELEMENT] != (uint16_t)wholeDistance) {
		g_hudElementStateCache[PANEL_DISTANCE_WHOLE_ELEMENT] = wholeDistance;
#ifdef XW_MODERN
		festring_setcursor(panelX + layout->distanceWholeX, panelY + layout->distanceY);
#else
		festring_setcursor(panelX + PANEL_DISTANCE_WHOLE_X, panelY + PANEL_DISTANCE_Y);
#endif
		panelrts_outnum(wholeDistance, PANEL_DISTANCE_FIELD_DIGITS, PANEL_DISTANCE_WHOLE_MIN_DIGITS);
	}
	distanceHundredths -= (uint16_t)(wholeDistance * PANEL_DISTANCE_HUNDREDTHS_PER_UNIT);
	if (g_hudElementStateCache[PANEL_DISTANCE_FRACTION_ELEMENT] != (uint16_t)distanceHundredths) {
		g_hudElementStateCache[PANEL_DISTANCE_FRACTION_ELEMENT] = distanceHundredths;
#ifdef XW_MODERN
		festring_setcursor(panelX + layout->distanceFractionX, panelY + layout->distanceY);
#else
		festring_setcursor(panelX + PANEL_DISTANCE_FRACTION_X, panelY + PANEL_DISTANCE_Y);
#endif
		panelrts_outnum(distanceHundredths, PANEL_DISTANCE_FIELD_DIGITS, PANEL_DISTANCE_FIELD_DIGITS);
	}
}

// FUNCTION: XW 0x41A530
void panel_updategunsight(void) {
	uint16_t indicatorState;
	if (!g_playerFlightState.selectedWeaponMode) {
		indicatorState = g_targetLockActive ? PANEL_GUNSIGHT_LASER_LOCK_STATE : 0;
	} else {
		if (g_playerFlightState.currentTargetObjectIdx != XW_OBJECT_SLOT_UNAVAILABLE)
			indicatorState = g_playerFlightState.missileLockState + PANEL_GUNSIGHT_NO_TARGET_STATE;
		else
			indicatorState = PANEL_GUNSIGHT_NO_TARGET_STATE;
		if (g_playerFlightState.missileLockState == PANEL_GUNSIGHT_MISSILE_LOCKED)
			g_targetLockActive = 1;
		else
			g_targetLockActive = 0;
	}
	fsfx_triggergunsightsfx(indicatorState);
	panel_updatelever(PANEL_GUNSIGHT_ELEMENT, indicatorState);
}

// FUNCTION: XW 0x41A590
void panel_updatelasers(void) {
#ifdef XW_MODERN
	int hud_pane = XwHud_Push(-1);
#endif
	CraftData* playerCraft = g_playerFlightState.craft;
	uint16_t craftTypeIndex = playerCraft->craftTypeIndex;
	uint16_t laserSlotCount = playerCraft->laserSlotCount;
	uint16_t laserSlot;
	uint16_t state = PANEL_LASER_DISABLED;
	int16_t filledSegmentState = PANEL_LASER_DISABLED;
	g_targetLockActive = 0;
	for (laserSlot = 0; laserSlot < laserSlotCount; ++laserSlot) {
		uint16_t laserGroup = g_craftTypeDefs[craftTypeIndex].laserGroupLastSlot[0] < laserSlot;
		unsigned int elementIndex = laserSlot + PANEL_LASER_CHARGE_FIRST_ELEMENT;
		int16_t x = g_hudElementLayouts[elementIndex].x;
		int16_t y = g_hudElementLayouts[elementIndex].y;
		uint16_t segmentSpriteBase = g_hudElementLayouts[elementIndex].spriteIndex;
		int16_t laserCharge;
		uint16_t reticleState;
		playerCraft = g_playerFlightState.craft;
		laserCharge = playerCraft->weaponSlots[laserSlot].laserCharge;
		if (g_flightCamera.hudStateLive == 0 &&
			(playerCraft->activeHudFeatureMask & PANEL_HUD_WEAPONS_FEATURE) != 0) {
			uint16_t filledSegments;
			int16_t emptySegmentState;
			if (laserCharge > 0 && (playerCraft->workingSubsystems & LASER_CANNON_SUBSYSTEM_MASK) != 0) {
				uint16_t tierCharge;
				++laserCharge;
				tierCharge = laserCharge;
				if (laserCharge <= LASER_HIGH_CHARGE_THRESHOLD) {
					emptySegmentState = PANEL_LASER_DISABLED;
					filledSegmentState = PANEL_LASER_CHARGE_NORMAL;
				} else {
					tierCharge = laserCharge - LASER_HIGH_CHARGE_THRESHOLD;
					emptySegmentState = PANEL_LASER_CHARGE_NORMAL;
					filledSegmentState = PANEL_LASER_CHARGE_HIGH;
				}
				if (g_flightScreenWidth == PANEL_COCKPIT_WIDTH) {
					filledSegments = tierCharge >> PANEL_LASER_LOW_RES_CHARGE_SHIFT;
				} else {
					filledSegments = (int)tierCharge / PANEL_LASER_HIGH_RES_CHARGE_DIVISOR;
					if (filledSegments > PANEL_LASER_HIGH_RES_SEGMENTS)
						filledSegments = PANEL_LASER_HIGH_RES_SEGMENTS;
				}
			} else {
				filledSegments = 0;
				emptySegmentState = PANEL_LASER_DISABLED;
				filledSegmentState = PANEL_LASER_DISABLED;
			}
#ifdef XW_MODERN
			XwHud_Widget(elementIndex, filledSegments, g_flightScreenWidth == 320 ? 8 : 10,
						 (g_hudElementLayouts[elementIndex].selector ? -1 : 1) *
							 (g_flightScreenWidth == 320 ? 4 : 6),
						 0, emptySegmentState, filledSegmentState, XW_SNAP_WIDGET_LASER);
#endif
			if (filledSegments != g_hudElementStateCache[elementIndex]) {
				int segmentStepX;
				int16_t mirror;
				uint16_t segmentIndex;
				g_hudElementStateCache[elementIndex] = filledSegments;
				if (g_hudElementLayouts[elementIndex].selector != 0) {
					mirror = 1;
					segmentStepX = g_flightScreenWidth == PANEL_COCKPIT_WIDTH ? -PANEL_LASER_LOW_RES_X_STEP
																			  : -PANEL_LASER_HIGH_RES_X_STEP;
				} else {
					mirror = 0;
					segmentStepX = g_flightScreenWidth == PANEL_COCKPIT_WIDTH ? PANEL_LASER_LOW_RES_X_STEP
																			  : PANEL_LASER_HIGH_RES_X_STEP;
				}
				for (segmentIndex = 0; segmentIndex < (g_flightScreenWidth == PANEL_COCKPIT_WIDTH
														   ? PANEL_LASER_LOW_RES_SEGMENTS
														   : PANEL_LASER_HIGH_RES_SEGMENTS);
					 ++segmentIndex) {
					uint16_t segmentState =
						segmentIndex < filledSegments ? filledSegmentState : emptySegmentState;
					g_flightBlitSpriteFn(g_hudPanelSpriteDataByIndex[segmentSpriteBase + segmentState], x, y,
										 PANEL_SETTING_TRANSPARENT_COLOR, mirror);
					x = (int16_t)(x + segmentStepX);
				}
				playerCraft = g_playerFlightState.craft;
			}
		}
		if (laserCharge > 0 && (playerCraft->workingSubsystems & LASER_CANNON_SUBSYSTEM_MASK) != 0) {
			if (g_playerFlightState.selectedWeaponMode == PANEL_LASER_WEAPON_MODE &&
				g_playerFlightState.selectedWeaponBank == laserGroup) {
				switch (playerCraft->laserState.linkMode[laserGroup]) {
					case LASER_LINK_SINGLE:
						state = playerCraft->laserState.nextSlot[laserGroup] == laserSlot
									? PANEL_LASER_READY
									: PANEL_LASER_UNSELECTED;
						break;
					case LASER_LINK_PAIR:
						if (playerCraft->laserState.nextSlot[laserGroup] == laserSlot ||
							(craftTypeIndex == PANEL_XWING_CRAFT_TYPE &&
							 playerCraft->laserState.nextSlot[laserGroup] + LASER_PAIR_SLOT_STEP ==
								 laserSlot))
							state = PANEL_LASER_READY;
						else
							state = PANEL_LASER_UNSELECTED;
						break;
					case LASER_LINK_ALL:
						state = PANEL_LASER_READY;
						break;
					default:
						break;
				}
				reticleState = state;
				if (state == PANEL_LASER_READY &&
					playerCraft->laserState.fireCooldownTicks[laserGroup] != 0) {
					state = PANEL_LASER_COOLDOWN;
					reticleState = PANEL_LASER_RETICLE_COOLDOWN;
				}
			} else {
				state = PANEL_LASER_UNSELECTED;
				reticleState = PANEL_LASER_DISABLED;
			}
		} else {
			state = PANEL_LASER_DISABLED;
			reticleState = PANEL_LASER_DISABLED;
		}
		if (filledSegmentState == PANEL_LASER_CHARGE_HIGH)
			++state;
		if (g_flightCamera.hudStateLive == 0 &&
			(playerCraft->activeHudFeatureMask & PANEL_HUD_WEAPONS_FEATURE) != 0) {
#ifdef XW_MODERN
			if (g_playerFlightState.object->objectType != XwFlightTypes_ObjectType(XW_OBJ_B_WING))
#else
			if (g_playerFlightState.object->objectType != XW_OBJ_B_WING)
#endif
				panel_updatelever(laserSlot + PANEL_LASER_READY_FIRST_ELEMENT, state);
			else
				panel_updatelever(laserSlot + PANEL_BWING_LASER_READY_FIRST_ELEMENT, state);
		}
		panel_updatelever(laserSlot + PANEL_LASER_RETICLE_FIRST_ELEMENT, reticleState);
		if ((g_playerFlightState.craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_TARGETING) != 0 &&
			reticleState == PANEL_LASER_READY) {
			if (g_playerFlightState.currentTargetObjectIdx != XW_OBJECT_SLOT_UNAVAILABLE &&
				collide_targetinrange(g_playerFlightState.objectIndex,
									  g_playerFlightState.currentTargetObjectIdx, laserSlot) != 0) {
				reticleState = PANEL_LASER_RETICLE_COOLDOWN;
				g_targetLockActive = 1;
			} else {
				reticleState = PANEL_LASER_UNSELECTED;
			}
		} else if ((g_playerFlightState.craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_TARGETING) == 0) {
			reticleState = PANEL_LASER_DISABLED;
		} else if (reticleState == PANEL_LASER_RETICLE_COOLDOWN) {
			reticleState = PANEL_LASER_UNSELECTED;
		}
		if (reticleState == PANEL_LASER_READY)
			reticleState = PANEL_LASER_DISABLED;
#ifdef XW_MODERN
		if (g_playerFlightState.object->objectType != XwFlightTypes_ObjectType(XW_OBJ_B_WING))
#else
		if (g_playerFlightState.object->objectType != XW_OBJ_B_WING)
#endif
			panel_updatelever(laserSlot + PANEL_LASER_RANGE_FIRST_ELEMENT, reticleState);
		else
			panel_updatelever(laserSlot + PANEL_BWING_LASER_RANGE_FIRST_ELEMENT, reticleState);
	}
#ifdef XW_MODERN
	XwHud_Pop(hud_pane);
#endif
}

// FUNCTION: XW 0x41AA00
void panel_updateweapons(void) {
	if ((g_playerFlightState.craft->activeHudFeatureMask & PANEL_HUD_WEAPONS_FEATURE) != 0 &&
		g_craftTypeDefs[g_playerFlightState.craftTypeIndex].warheadLauncherSlotCount[0] != 0) {
		panel_updatehardpoint(g_craftTypeDefs[g_playerFlightState.craftTypeIndex].warheadFirstSlot[0], 0);
		panel_updatehardpoint(g_craftTypeDefs[g_playerFlightState.craftTypeIndex].warheadLastSlot[0], 1);
	}
}

// FUNCTION: XW 0x41AA70
void panel_updatehardpoint(uint16_t warheadSlotIdx, uint16_t displaySlot) {
#ifdef XW_MODERN
	int hud_pane = XwHud_Push(XW_SNAP_PANE_WARHEADS);
#endif

	uint16_t count = g_playerFlightState.craft->weaponSlots[warheadSlotIdx].count;
	uint16_t state;
	if (count != g_hudElementStateCache[displaySlot + PANEL_HARDPOINT_COUNT_FIRST_ELEMENT]) {
		g_hudElementStateCache[displaySlot + PANEL_HARDPOINT_COUNT_FIRST_ELEMENT] = count;
		festring_setfontsize(FLIGHT_FONT_MICRO);
		festring_setbound(0, 0, g_flightScreenWidth, g_flightScreenHeight);
		festring_setcursor(g_hudElementLayouts[displaySlot + PANEL_HARDPOINT_COUNT_FIRST_ELEMENT].x,
						   g_hudElementLayouts[displaySlot + PANEL_HARDPOINT_COUNT_FIRST_ELEMENT].y);
		festring_setbackcolor(0);
		festring_settextcolor(PANEL_HARDPOINT_TEXT_COLOR);
		g_flightTextShadowEnabled = 0;
		panelrts_outnum(count, PANEL_HARDPOINT_DIGITS, PANEL_HARDPOINT_DIGITS);
	}
	if (count != 0 && (g_playerFlightState.craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_LAUNCHER) != 0) {
		if (g_playerFlightState.selectedWeaponMode == 0) {
			state = PANEL_HARDPOINT_UNSELECTED;
		} else {
			uint8_t launcherFlags =
				g_playerFlightState.craft->warheadLauncherFlags[g_playerFlightState.selectedWeaponBank];
			if ((launcherFlags & XW_LAUNCHER_FIRE_MODE_MASK) == XW_LAUNCHER_FIRE_LINKED) {
				state = PANEL_HARDPOINT_SELECTED;
			} else {
				state = (uint16_t)(launcherFlags >> XW_LAUNCHER_SELECTED_SLOT_SHIFT) == displaySlot
							? PANEL_HARDPOINT_SELECTED
							: PANEL_HARDPOINT_UNSELECTED;
			}
		}
	} else {
		state = PANEL_HARDPOINT_DISABLED;
	}
	panel_updatelever(displaySlot + PANEL_HARDPOINT_INDICATOR_FIRST_ELEMENT, state);

#ifdef XW_MODERN
	XwHud_Pop(hud_pane);
#endif
}

// FUNCTION: XW 0x41ABB0
void panel_updateshields(void) {
	if ((g_playerFlightState.craft->activeHudFeatureMask & PANEL_HUD_SHIELDS_FEATURE) != 0) {
		int16_t energy;
		int16_t nominalEnergy;
		uint16_t baseState;
		uint16_t overchargeState;
		uint16_t hullState;
		energy = g_playerFlightState.craft->shieldEnergy[XW_SHIELD_FRONT];
		if (energy < 0)
			energy = 0;
		if ((g_playerFlightState.craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_SHIELDS) == 0)
			energy = 0;
		nominalEnergy =
			g_craftTypeDefs[g_playerFlightState.craftTypeIndex].nominalShieldEnergy[XW_SHIELD_FRONT];
		if (energy >= nominalEnergy) {
			baseState = PANEL_SHIELD_FULL_STATE;
			overchargeState = math2_fraction(PANEL_SHIELD_FULL_STATE,
											 math2_percentage(energy - nominalEnergy, nominalEnergy));
		} else {
			baseState = math2_fraction(PANEL_SHIELD_FULL_STATE, math2_percentage(energy, nominalEnergy));
			overchargeState = 0;
		}
		if (g_flightGlobalCountdownTimers.ticks[XW_TIMER_SHIELD_FLASH] != 0 &&
			g_lastShieldDamageSide == XW_SHIELD_FRONT) {
			if (overchargeState == 0)
				baseState = PANEL_SHIELD_FLASH_STATE;
			else
				overchargeState = PANEL_SHIELD_FLASH_STATE;
		}
		panel_updatelever(PANEL_FRONT_SHIELD_ELEMENT, baseState);
		panel_updatelever(PANEL_FRONT_OVERCHARGE_ELEMENT, overchargeState);
		energy = g_playerFlightState.craft->shieldEnergy[XW_SHIELD_REAR];
		if (energy < 0)
			energy = 0;
		if ((g_playerFlightState.craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_SHIELDS) == 0)
			energy = 0;
		nominalEnergy =
			g_craftTypeDefs[g_playerFlightState.craftTypeIndex].nominalShieldEnergy[XW_SHIELD_REAR];
		if (energy >= nominalEnergy) {
			baseState = PANEL_SHIELD_FULL_STATE;
			overchargeState = math2_fraction(PANEL_SHIELD_FULL_STATE,
											 math2_percentage(energy - nominalEnergy, nominalEnergy));
		} else {
			baseState = math2_fraction(PANEL_SHIELD_FULL_STATE, math2_percentage(energy, nominalEnergy));
			overchargeState = 0;
		}
		if (g_flightGlobalCountdownTimers.ticks[XW_TIMER_SHIELD_FLASH] != 0 &&
			g_lastShieldDamageSide == XW_SHIELD_REAR) {
			if (overchargeState == 0)
				baseState = PANEL_SHIELD_FLASH_STATE;
			else
				overchargeState = PANEL_SHIELD_FLASH_STATE;
		}
		panel_updatelever(PANEL_REAR_SHIELD_ELEMENT, baseState);
		panel_updatelever(PANEL_REAR_OVERCHARGE_ELEMENT, overchargeState);
		if (g_flightGlobalCountdownTimers.ticks[XW_TIMER_HULL_FLASH] != 0) {
			hullState = PANEL_HULL_FLASH_STATE;
		} else {
			hullState = g_playerFlightState.craft->hullMax / PANEL_HULL_TIERS;
			if (hullState == 0)
				hullState = PANEL_HULL_FULL_STATE;
			else
				hullState = PANEL_HULL_FULL_STATE - g_playerFlightState.craft->hullDamage / hullState;
		}
		panel_updatelever(PANEL_HULL_ELEMENT, hullState);
	}
}

// FUNCTION: XW 0x41ADA0
void panel_updatespeed(void) {
#ifdef XW_MODERN
	int hud_pane = XwHud_Push(XW_SNAP_PANE_SPEED);
#endif

	if ((g_playerFlightState.craft->activeHudFeatureMask & PANEL_HUD_SPEED_FEATURE) != 0) {
		uint16_t displaySpeed = math2_fraction(g_playerFlightState.object->speed, PANEL_SPEED_SCALE_Q16);
		if (displaySpeed != g_hudElementStateCache[PANEL_SPEED_ELEMENT]) {
			g_hudElementStateCache[PANEL_SPEED_ELEMENT] = displaySpeed;
			festring_setfontsize(FLIGHT_FONT_MICRO);
			festring_setbound(0, 0, g_flightScreenWidth, g_flightScreenHeight);
			festring_setcursor(g_hudElementLayouts[PANEL_SPEED_ELEMENT].x,
							   g_hudElementLayouts[PANEL_SPEED_ELEMENT].y);
			festring_setbackcolor(0);
			festring_settextcolor(PANEL_SPEED_TEXT_COLOR);
			g_flightTextShadowEnabled = 0;
			panelrts_outnum(displaySpeed, PANEL_SPEED_DIGITS, PANEL_SPEED_MIN_DIGITS);
		}
	}

#ifdef XW_MODERN
	XwHud_Pop(hud_pane);
#endif
}

// FUNCTION: XW 0x41AE50
void panel_updateclock(void) {
#ifdef XW_MODERN
	int hud_pane = XwHud_Push(XW_SNAP_PANE_CLOCK);
#endif

#ifdef XW_MODERN
	const XwFlightPanelLayout* layout = XwFlightPanel_Layout();
#endif
	uint16_t remainingSeconds =
		g_missionCountdownClock.seconds + XW_SECONDS_PER_MINUTE * g_missionCountdownClock.minutes;
	if (remainingSeconds != g_hudElementStateCache[PANEL_CLOCK_ELEMENT]) {
		g_hudElementStateCache[PANEL_CLOCK_ELEMENT] = remainingSeconds;
		festring_setfontsize(FLIGHT_FONT_MICRO);
		festring_setbound(0, 0, g_flightScreenWidth, g_flightScreenHeight);
		festring_setbackcolor(FLIGHT_TEXT_ENCODED_COLOR_BASE);
		festring_settextcolor(PANEL_CLOCK_TEXT_COLOR);
		g_flightTextShadowEnabled = 0;
		festring_setcursor(g_hudElementLayouts[PANEL_CLOCK_ELEMENT].x,
						   g_hudElementLayouts[PANEL_CLOCK_ELEMENT].y);
		panelrts_outnum(g_missionCountdownClock.minutes, PANEL_CLOCK_FIELD_DIGITS,
						PANEL_CLOCK_MINUTE_MIN_DIGITS);
#ifdef XW_MODERN
		festring_setcursor(g_hudElementLayouts[PANEL_CLOCK_ELEMENT].x + layout->clockSecondsX,
#else
		festring_setcursor(g_hudElementLayouts[PANEL_CLOCK_ELEMENT].x + PANEL_CLOCK_SECONDS_X_OFFSET,
#endif
						   g_hudElementLayouts[PANEL_CLOCK_ELEMENT].y);
		panelrts_outnum(g_missionCountdownClock.seconds, PANEL_CLOCK_FIELD_DIGITS, PANEL_CLOCK_FIELD_DIGITS);
	}

#ifdef XW_MODERN
	XwHud_Pop(hud_pane);
#endif
}

// FUNCTION: XW 0x41AF20
void panel_updatepower(void) {
#ifdef XW_MODERN
	int hud_pane = XwHud_Push(XW_SNAP_PANE_MAX_SPEED);
#endif

#ifdef XW_MODERN
	const XwFlightPanelLayout* layout = XwFlightPanel_Layout();
#endif
	if ((g_playerFlightState.craft->activeHudFeatureMask & PANEL_HUD_POWER_FEATURE) != 0) {
		uint16_t maxSpeed;
		int16_t powerDelta;
		panel_updatesetting(g_playerFlightState.craft->laserRedirect, PANEL_LASER_POWER_ELEMENT,
#ifdef XW_MODERN
							PANEL_REDIRECT_POWER_SEGMENTS, layout->redirectStepY);
#else
							PANEL_REDIRECT_POWER_SEGMENTS, PANEL_REDIRECT_POWER_Y_STEP);
#endif
		panel_updatesetting(g_playerFlightState.craft->shieldRedirect, PANEL_SHIELD_POWER_ELEMENT,
#ifdef XW_MODERN
							PANEL_REDIRECT_POWER_SEGMENTS, layout->redirectStepY);
#else
							PANEL_REDIRECT_POWER_SEGMENTS, PANEL_REDIRECT_POWER_Y_STEP);
#endif
		panel_updatesetting(PANEL_ENGINE_POWER_SEGMENTS - g_playerFlightState.craft->shieldRedirect -
								g_playerFlightState.craft->laserRedirect,
#ifdef XW_MODERN
							PANEL_ENGINE_POWER_ELEMENT, PANEL_ENGINE_POWER_SEGMENTS, layout->engineStepY);
#else
							PANEL_ENGINE_POWER_ELEMENT, PANEL_ENGINE_POWER_SEGMENTS,
							PANEL_ENGINE_POWER_Y_STEP);
#endif
		maxSpeed = g_craftTypeDefs[g_playerFlightState.craftTypeIndex].displayMaxSpeed;
		powerDelta = PANEL_NEUTRAL_ENGINE_POWER - g_playerFlightState.craft->shieldRedirect -
					 g_playerFlightState.craft->laserRedirect;
		maxSpeed += powerDelta * math2_fraction(maxSpeed, PANEL_POWER_SPEED_STEP_Q16);
		if (maxSpeed != g_hudElementStateCache[PANEL_MAX_SPEED_ELEMENT]) {
			g_hudElementStateCache[PANEL_MAX_SPEED_ELEMENT] = maxSpeed;
			festring_setfontsize(FLIGHT_FONT_MICRO);
			festring_setbound(0, 0, g_flightScreenWidth, g_flightScreenHeight);
			festring_setcursor(g_hudElementLayouts[PANEL_MAX_SPEED_ELEMENT].x,
							   g_hudElementLayouts[PANEL_MAX_SPEED_ELEMENT].y);
			festring_setbackcolor(0);
			festring_settextcolor(PANEL_MAX_SPEED_TEXT_COLOR);
			g_flightTextShadowEnabled = 0;
			panelrts_outnum(maxSpeed, PANEL_MAX_SPEED_DIGITS, PANEL_MAX_SPEED_MIN_DIGITS);
		}
	}

#ifdef XW_MODERN
	XwHud_Pop(hud_pane);
#endif
}

// FUNCTION: XW 0x41B060
void panel_updatesetting(uint16_t filledCount, uint16_t elementIdx, uint16_t segmentCount, int16_t yStep) {
#ifdef XW_MODERN
	int hud_pane = XwHud_Push(-1);
#endif

#ifdef XW_MODERN
	XwHud_Widget(elementIdx, filledCount, segmentCount, 0, yStep, 0, 1, XW_SNAP_WIDGET_GAUGE);
#endif

	if (filledCount != g_hudElementStateCache[elementIdx]) {
		uint16_t segmentIndex;
		int16_t x;
		int16_t y;
		uint16_t baseSpriteIndex;
		g_hudElementStateCache[elementIdx] = filledCount;
		x = g_hudElementLayouts[elementIdx].x;
		y = g_hudElementLayouts[elementIdx].y;
		baseSpriteIndex = g_hudElementLayouts[elementIdx].spriteIndex;
		for (segmentIndex = 0; segmentIndex < segmentCount; ++segmentIndex) {
			uint16_t state = segmentIndex < filledCount;
			g_flightBlitSpriteFn(g_hudPanelSpriteDataByIndex[state + baseSpriteIndex], x, y,
								 PANEL_SETTING_TRANSPARENT_COLOR, 0);
			y -= yStep;
		}
	}

#ifdef XW_MODERN
	XwHud_Pop(hud_pane);
#endif
}

// FUNCTION: XW 0x41B100
void panel_updatethrottle(void) {
#ifdef XW_MODERN
	if (XwFlightTypes_Dos()) {
		Dos94_panel_updatethrottle();
		return;
	}
#endif
	if ((g_playerFlightState.craft->activeHudFeatureMask & PANEL_HUD_THROTTLE_FEATURE) != 0) {
		int throttleDivisor;
		uint16_t filledColumns;
		if (g_flightScreenWidth != FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH) {
			throttleDivisor = PANEL_THROTTLE_HIGH_RES_DIVISOR;
		} else {
			throttleDivisor = PANEL_THROTTLE_LOW_RES_DIVISOR;
		}
		filledColumns = g_playerFlightState.craft->engineThrottle[0] / throttleDivisor;
#ifdef XW_MODERN
		XwHud_Widget(PANEL_THROTTLE_ELEMENT, filledColumns, 0, 0, 0, 0, 0, XW_SNAP_WIDGET_THROTTLE);
#endif
		if (filledColumns != g_hudElementStateCache[PANEL_THROTTLE_ELEMENT]) {
			uint16_t x;
			uint16_t y;
			uint16_t column;
			g_hudElementStateCache[PANEL_THROTTLE_ELEMENT] = filledColumns;
			x = (g_flightResolutionMode == RTSVGA2_MODE_13H ||
				 g_flightResolutionMode == FLIGHT_DISPLAY_MODE_101H)
					? g_hudElementLayouts[PANEL_THROTTLE_ELEMENT].x
					: 2 * g_hudElementLayouts[PANEL_THROTTLE_ELEMENT].x;
			y = g_hudElementLayouts[PANEL_THROTTLE_ELEMENT].y;
			for (column = 0; column < (g_flightScreenWidth == FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH
										   ? PANEL_THROTTLE_LOW_RES_COLUMNS
										   : PANEL_THROTTLE_HIGH_RES_COLUMNS);
				 ++column) {
				uint16_t color;
				if (column >= filledColumns) {
					color = 0;
				} else {
					int barColumns = g_flightScreenWidth == FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH
										 ? PANEL_THROTTLE_LOW_RES_COLUMNS
										 : PANEL_THROTTLE_HIGH_RES_COLUMNS;
					if (column >= barColumns - (g_flightScreenWidth == FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH
													? PANEL_THROTTLE_LOW_RES_TOP_BAND
													: PANEL_THROTTLE_HIGH_RES_TOP_BAND)) {
						color = PANEL_THROTTLE_HIGH_COLOR;
					} else {
						color =
							column >= barColumns - (g_flightScreenWidth == FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH
														? PANEL_THROTTLE_LOW_RES_UPPER_BANDS
														: PANEL_THROTTLE_HIGH_RES_UPPER_BANDS)
								? PANEL_THROTTLE_MIDDLE_COLOR
								: PANEL_THROTTLE_LOW_COLOR;
					}
					if ((column & 1) != 0) {
						color -= PANEL_THROTTLE_ALTERNATE_COLOR_STEP;
					}
				}
				if (g_flightResolutionMode != RTSVGA2_MODE_13H &&
					g_flightResolutionMode != FLIGHT_DISPLAY_MODE_101H) {
					uint16_t doubleWidthX = x + 2 * column;
					rtsvga2_drawdotVGA(doubleWidthX, y, color);
					rtsvga2_drawdotVGA(doubleWidthX, y + 1, color);
					rtsvga2_drawdotVGA(doubleWidthX + 1, y, color);
					rtsvga2_drawdotVGA(doubleWidthX + 1, y + 1, color);
				} else {
					rtsvga2_drawdotVGA(x + column, y, color);
					rtsvga2_drawdotVGA(x + column, y + 1, color);
				}
			}
		}
	}
}

// FUNCTION: XW 0x41B2C0
void panel_updateweaponwarnings(void) {
	if ((g_playerFlightState.craft->activeHudFeatureMask & PANEL_HUD_WEAPON_WARNING_FEATURE) != 0) {
		int16_t maxLockTicks = 0;
		uint16_t warningState;
		uint16_t objectIndex;
		for (objectIndex = 0; objectIndex < XW_CRAFT_OBJECT_COUNT; ++objectIndex) {
			if (g_objectTable[objectIndex].objectType != XW_OBJ_NONE) {
				CraftData* attackerCraft = g_objectTable[objectIndex].instanceData;
				if (attackerCraft->aiTargetRef == g_playerFlightState.objectIndex &&
					attackerCraft->aiManeuverId == PAIORDER_MANEUVER_ATTACK_ALTERNATE &&
					attackerCraft->warheadLockTicks > maxLockTicks) {
					maxLockTicks = attackerCraft->warheadLockTicks;
				}
			}
		}
		if (maxLockTicks > PANEL_WEAPON_LOCK_COMPLETE_TICKS) {
			warningState = PANEL_WEAPON_WARNING_LOCKED;
		} else if (maxLockTicks > 0) {
			warningState = (g_missionElapsedClock.subsecondTicks / PANEL_WEAPON_WARNING_FLASH_TICKS) & 1;
		} else {
			warningState = 0;
		}
		panel_updatelever(PANEL_WEAPON_WARNING_ELEMENT, warningState);
	}
}

// FUNCTION: XW 0x41B370
void panel_UpdateCriticalHullShieldWarning(void) {
	uint16_t hullBandSize;
	uint16_t shieldEnergy;
	uint16_t hullDamageBand;
	uint16_t warningState;
	hullBandSize = g_playerFlightState.craft->hullMax / PANEL_HULL_TIERS;
	shieldEnergy = g_playerFlightState.craft->shieldEnergy[XW_SHIELD_FRONT] +
				   g_playerFlightState.craft->shieldEnergy[XW_SHIELD_REAR];
	if (!hullBandSize)
		hullDamageBand = PANEL_CRITICAL_HULL_BAND;
	else
		hullDamageBand = g_playerFlightState.craft->hullDamage / hullBandSize;
	if (shieldEnergy < PANEL_CRITICAL_SHIELD_THRESHOLD && hullDamageBand == PANEL_CRITICAL_HULL_BAND) {
		warningState = (g_missionElapsedClock.subsecondTicks / PANEL_CRITICAL_FLASH_TICKS) & 1;
		if (warningState) {
			fsfx_triggersfx(FSFX_CRITICAL_HULL_WARNING_SLOT, XW_OBJECT_SLOT_UNAVAILABLE);
			panel_updatelever(PANEL_CRITICAL_WARNING_ELEMENT, warningState);
			return;
		}
	} else {
		warningState = (g_playerFlightState.craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_2) == 0;
	}
	panel_updatelever(PANEL_CRITICAL_WARNING_ELEMENT, warningState);
}

// FUNCTION: XW 0x41B430
void panel_updatereplaystuff(void) {
#ifdef XW_MODERN
	int hud_pane = XwHud_Push(XW_SNAP_PANE_REPLAY_COUNTER);
#endif

	int16_t x;
	int16_t y;
	int16_t value;
#ifdef XW_MODERN
	if (XwFlightTypes_Dos() || g_showSimStepScale == 0) {
#else
	if (g_showSimStepScale == 0) {
#endif
		panel_updatelever(PANEL_REPLAY_RECORDING_ELEMENT, g_ReplayRecording);
		x = g_hudElementLayouts[PANEL_REPLAY_COUNTER_ELEMENT].x;
		y = g_hudElementLayouts[PANEL_REPLAY_COUNTER_ELEMENT].y;
		festring_setfontsize(FLIGHT_FONT_MICRO);
		festring_setbound(
			x, y,
			x + (g_flightScreenWidth == FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH ? PANEL_REPLAY_LOW_RES_WIDTH
																			: PANEL_REPLAY_HIGH_RES_WIDTH),
			y + (g_flightScreenWidth == FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH ? PANEL_REPLAY_LOW_RES_HEIGHT
																			: PANEL_REPLAY_HIGH_RES_HEIGHT));
		festring_setcursor(x, y);
		festring_setbackcolor(PANEL_REPLAY_BACKGROUND_COLOR);
		if (g_ReplayRecording == 0) {
			value = PANEL_REPLAY_IDLE_PERCENT;
			if (g_ReplayProgressPercent != value) {
				g_ReplayProgressPercent = value;
				g_flightFillClipRectFn();
			}
		} else {
			value = PANEL_REPLAY_FULL_PERCENT -
					math2_fraction(PANEL_REPLAY_FULL_PERCENT,
								   math2_longpercentage(g_ReplayFrameCount, g_ReplayCapacityFrames));
			if ((uint16_t)value > PANEL_REPLAY_MAX_PERCENT)
				value = PANEL_REPLAY_MAX_PERCENT;
			if (value != g_ReplayProgressPercent ||
#ifdef XW_MODERN
				(!XwFlightTypes_Dos() && (int8_t)g_showSimStepScale != g_replayHudShowedSimStepScale)) {
#else
				(int8_t)g_showSimStepScale != g_replayHudShowedSimStepScale) {
#endif
				g_ReplayProgressPercent = value;
				g_flightFillClipRectFn();
				festring_settextcolor(PANEL_REPLAY_TEXT_COLOR);
				panelrts_outnum(value, PANEL_REPLAY_DIGITS, PANEL_REPLAY_MIN_DIGITS);
			}
		}
#ifdef XW_MODERN
		if (!XwFlightTypes_Dos())
#endif
			g_replayHudShowedSimStepScale = 0;
	} else {
		panel_updatelever(PANEL_REPLAY_RECORDING_ELEMENT, g_ReplayRecording);
		x = g_hudElementLayouts[PANEL_REPLAY_COUNTER_ELEMENT].x;
		y = g_hudElementLayouts[PANEL_REPLAY_COUNTER_ELEMENT].y;
		festring_setfontsize(FLIGHT_FONT_MICRO);
		festring_setbound(
			x, y,
			x + (g_flightScreenWidth == FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH ? PANEL_REPLAY_LOW_RES_WIDTH
																			: PANEL_REPLAY_HIGH_RES_WIDTH),
			y + (g_flightScreenWidth == FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH ? PANEL_REPLAY_LOW_RES_HEIGHT
																			: PANEL_REPLAY_HIGH_RES_HEIGHT));
		festring_setcursor(x, y);
		festring_setbackcolor(PANEL_REPLAY_BACKGROUND_COLOR);
		value = g_simStepScale;
		g_flightFillClipRectFn();
		festring_settextcolor(PANEL_REPLAY_TEXT_COLOR);
		panelrts_outnum(value, PANEL_REPLAY_DIGITS, PANEL_REPLAY_MIN_DIGITS);
		g_replayHudShowedSimStepScale = 1;
	}

#ifdef XW_MODERN
	XwHud_Pop(hud_pane);
#endif
}

// FUNCTION: XW 0x41B630
void panel_UpdateCraftSystemStatusIndicators(void) {
#ifdef XW_MODERN
	int hud_pane = XwHud_Push(XW_SNAP_PANE_VIEW_LABEL);
#endif

	uint8_t featureBit = PANEL_SYSTEM_STATUS_INITIAL_MASK;
	uint16_t elementIndex;
	for (elementIndex = PANEL_SYSTEM_STATUS_FIRST_ELEMENT;
		 (uint16_t)(elementIndex - PANEL_SYSTEM_STATUS_FIRST_ELEMENT) < PANEL_SYSTEM_STATUS_ELEMENT_COUNT;
		 ++elementIndex) {
		if (g_hudElementLayouts[elementIndex].selector == g_flightCamera.hudStateLive) {
			int16_t x = g_hudElementLayouts[elementIndex].x;
			int16_t y = g_hudElementLayouts[elementIndex].y;
			int16_t coordinateSum = x + y;
			if ((g_playerFlightState.craft->activeHudFeatureMask & featureBit) == 0 && coordinateSum != 0) {
				g_flightBlitSpriteFn(
					g_hudPanelSpriteDataByIndex[g_hudElementLayouts[elementIndex].spriteIndex], x, y, 0, 0);
			}
		}
		if (featureBit != PANEL_SYSTEM_STATUS_HOLD_MASK) {
			featureBit <<= 1;
		}
	}

#ifdef XW_MODERN
	XwHud_Pop(hud_pane);
#endif
}

// FUNCTION: XW 0x41B6B0
void panel_updatecockpitdamage(void) {
#ifdef XW_MODERN
	int hud_pane = XwHud_Push(XW_SNAP_PANE_VIEW_LABEL);
#endif

#ifdef XW_MODERN
	if (g_playerFlightState.object->objectType != XwFlightTypes_ObjectType(XW_OBJ_B_WING)) {
#else
	if (g_playerFlightState.object->objectType != XW_OBJ_B_WING) {
#endif
		uint16_t overlayBit = 1;
		uint16_t elementIndex;
		for (elementIndex = PANEL_COCKPIT_DAMAGE_FIRST_ELEMENT;
			 (uint16_t)(elementIndex - PANEL_COCKPIT_DAMAGE_FIRST_ELEMENT) <
			 PANEL_COCKPIT_DAMAGE_ELEMENT_COUNT;
			 overlayBit <<= 1, ++elementIndex) {
			if (g_hudElementLayouts[elementIndex].selector == g_flightCamera.hudStateLive) {
				int16_t x = g_hudElementLayouts[elementIndex].x;
				int16_t y = g_hudElementLayouts[elementIndex].y;
				int16_t coordinateSum = x + y;
				if ((g_playerFlightState.craft->cockpitOverlayMask & overlayBit) != 0 && coordinateSum != 0) {
					int16_t drawX;
					int16_t mirror;
					if (g_hudCockpitResourceDescriptors[g_flightCamera.hudStateLive].enabled >=
						PANEL_COCKPIT_MIRRORED_RESOURCE) {
						mirror = 1;
						drawX = PANEL_COCKPIT_WIDTH - x;
					} else {
						drawX = x;
						mirror = 0;
					}
					g_flightBlitSpriteFn(
						g_hudPanelSpriteDataByIndex[g_hudElementLayouts[elementIndex].spriteIndex], drawX, y,
						0, mirror);
				}
			}
		}
	}

#ifdef XW_MODERN
	XwHud_Pop(hud_pane);
#endif
}

// FUNCTION: XW 0x41B780
void panel_DrawSpriteElement(uint16_t elementIdx, uint16_t state) {
#ifdef XW_MODERN
	int hud_pane = XwHud_Push(-1);
#endif

#ifdef XW_MODERN
	XwHud_Widget(elementIdx, state, 0, 0, 0, 0, 0, XW_SNAP_WIDGET_SPRITE);
#endif

	g_hudElementStateCache[elementIdx] = state;
	g_flightBlitSpriteFn(g_hudPanelSpriteDataByIndex[g_hudElementLayouts[elementIdx].spriteIndex + state],
						 g_hudElementLayouts[elementIdx].x, g_hudElementLayouts[elementIdx].y,
						 g_hudElementLayouts[elementIdx].selector, 0);

#ifdef XW_MODERN
	XwHud_Pop(hud_pane);
#endif
}

// FUNCTION: XW 0x41B7E0
void panel_updatelever(uint16_t elementIdx, uint16_t state) {
#ifdef XW_MODERN
	int hud_pane = XwHud_Push(-1);
#endif

#ifdef XW_MODERN
	XwHud_Widget(elementIdx, state, 0, 0, 0, 0, 0, XW_SNAP_WIDGET_SPRITE);
#endif

	if (g_hudElementStateCache[elementIdx] != state) {
		panel_DrawSpriteElement(elementIdx, state);
	}

#ifdef XW_MODERN
	XwHud_Pop(hud_pane);
#endif
}

// FUNCTION: XW 0x41B810
void panel_loadpaneldata(void) {
	char fileName[PANEL_RESOURCE_FILENAME_CAPACITY];
	const uint8_t* cockpitBaseName;
	unsigned int nameIndex;
	uint8_t nameChar;
	strcpy(g_hudCockpitBasePath, g_hudCockpitResolutionDirectory);
	cockpitBaseName = g_craftTypeDefs[g_playerFlightState.craftTypeIndex].cockpitBaseName;
	nameChar = cockpitBaseName[0];
	for (nameIndex = 0; nameChar != '\0'; ++nameIndex) {
		fileName[nameIndex] = nameChar;
		nameChar = cockpitBaseName[nameIndex + 1];
	}
	fileName[nameIndex] = cockpitBaseName[nameIndex];
	g_hudPanelSetId = 0;
	strcat(g_hudCockpitBasePath, fileName);
	panel_loadpanelviewdefs(g_hudCockpitBasePath);
	g_hudPanelSpriteDataWriteCursor = g_hudPanelSpriteDataBuffer;
	strcpy(fileName, g_hudCockpitResolutionDirectory);
	strcat(fileName, "PARTS.PNL");
	fediskio_loadbufferdata(fileName, 0, PANEL_SHARED_SPRITE_COUNT, 0);
	g_hudPanelSpriteCraftDataStart = g_hudPanelSpriteDataWriteCursor;
	panel_tryEMSforpanels();
	g_hudCockpitResourcesLoaded = 1;
}

// FUNCTION: XW 0x41B950
void panel_loadpanelviewdefs(const char* basePath) {
	strcpy(g_hudCockpitResourcePath, basePath);
	strcat(g_hudCockpitResourcePath, ".INT");
	fediskio_tryopenfile(g_hudCockpitResourcePath, "rb", 1);
#ifdef XW_MODERN
	XwRenderAssets_RegisterStream(XW_SOURCE_COCKPIT_LAYOUT, 0, g_stream, XwStorage_LastPath());
#endif
	fediskio_readfileblock(g_hudCockpitResourceDescriptors, sizeof(g_hudCockpitResourceDescriptors[0]),
						   PANEL_COCKPIT_DESCRIPTOR_COUNT, g_stream);
	fediskio_readfileblock(g_hudElementLayouts, sizeof(g_hudElementLayouts[0]), PANEL_HUD_ELEMENT_COUNT,
						   g_stream);
	fediskio_readfileblock(&g_hudPanelSpriteFileInfo, sizeof(g_hudPanelSpriteFileInfo), 1, g_stream);
	fediskio_tryclosefile(0);
}

// FUNCTION: XW 0x41BA10
void panel_forcenewviewdir(uint16_t hudViewState) {
	msg_messageinit();
	g_hudLoadedCockpitView = PANEL_VIEW_CACHE_INVALID;
	g_flightCamera.hudStateLive = PANEL_DISPLAY_MODE_INVALID;
	g_hudLoadedPanelSetId = PANEL_DISPLAY_MODE_INVALID;
	panelrts_setnewpilotview(hudViewState);
}

// FUNCTION: XW 0x41BA40
void panel_dosetnewpilotview(uint16_t hudViewState) {
#ifdef XW_MODERN
	int hud_pane = XwHud_Push(XW_SNAP_PANE_VIEW_LABEL);
#endif

	uint16_t enabled, resourceViewIndex;
	uint16_t viewportViewIndex = hudViewState;
	int16_t cockpitDrawX = 0;
	g_hudCockpitMirrorHorizontal = 0;
	enabled = g_hudCockpitResourceDescriptors[viewportViewIndex].enabled;
	if (hudViewState == PANEL_VIEW_NO_COCKPIT)
		resourceViewIndex = PANEL_VIEW_NO_COCKPIT;
	else if (enabled >= PANEL_VIEW_MIRRORED_ALIAS) {
		resourceViewIndex = enabled - PANEL_VIEW_MIRRORED_ALIAS;
		g_hudCockpitMirrorHorizontal = 1;
		cockpitDrawX = PANEL_VIEW_WIDTH - 1;
		g_hudLoadedCockpitView = PANEL_VIEW_CACHE_INVALID;
	} else if (enabled >= PANEL_VIEW_ALIAS)
		resourceViewIndex = enabled - PANEL_VIEW_ALIAS;
	else
		resourceViewIndex = hudViewState;
	if ((uint16_t)g_hudLoadedCockpitView != resourceViewIndex) {
		if (g_flightBytesPerPixel != 1)
			festring_hidescreen();
		festring_setbound(0, 0, PANEL_VIEW_WIDTH, PANEL_COCKPIT_HEIGHT);
		festring_setbackcolor(g_flightColorEscapeBypassChar);
		rtsvga2_clearwindowVGA();
		if (g_hudLoadedPanelSetId != g_hudPanelSetId) {
			g_hudPanelSpriteDataWriteCursor = g_hudPanelSpriteCraftDataStart;
			strcpy(g_hudCockpitBasePath, g_hudCockpitResolutionDirectory);
			strcat(g_hudCockpitBasePath, g_hudPanelSpriteFileInfo.baseName);
			strcat(g_hudCockpitBasePath, ".PNL");
			fediskio_loadbufferdata(
				g_hudCockpitBasePath, PANEL_SHARED_SPRITE_COUNT,
				g_hudPanelSpriteFileInfo.spriteCount + g_hudPanelSpriteFileInfo.spriteCountAddend, 0);
			g_hudLoadedPanelSetId = g_hudPanelSetId;
		}
		if (g_flightCamera.hudStateLive != PANEL_VIEW_NO_COCKPIT) {
			unsigned int viewportByteOffset;
			if (g_hudCockpitResources[resourceViewIndex].memoryHandle != 0)
				Memory_LockHandle(g_hudCockpitResources[resourceViewIndex].memoryHandle);
			else {
				g_hudCockpitResourceWriteCursor = g_flightOffscreenBuffer;
				panel_loadcontrolpanel(g_hudCockpitResourceDescriptors[resourceViewIndex].lfdName,
									   g_hudCockpitResources[resourceViewIndex].entries,
									   PANEL_COCKPIT_ENTRY_COUNT);
			}
			g_flightSetPaletteRangeFn((const RgbTriplet*)g_hudCockpitResources[resourceViewIndex]
										  .entries[PANEL_COCKPIT_PALETTE_ENTRY],
									  0, PANEL_COCKPIT_PALETTE_COLORS);
			festring_setbound(0, 0, g_flightScreenWidth, g_hudClearHeight);
			festring_setbackcolor(g_flightColorEscapeBypassChar);
			g_flightFillClipRectFn();
			g_flightBlitSpriteFn(g_hudCockpitResources[resourceViewIndex].entries[PANEL_COCKPIT_IMAGE_ENTRY],
								 cockpitDrawX, 0, 0, g_hudCockpitMirrorHorizontal);
			if (g_hudCockpitMirrorHorizontal != 1)
				viewportViewIndex = resourceViewIndex;
			viewportByteOffset =
				g_flightComputePixelOffsetFn(g_hudCockpitResourceDescriptors[viewportViewIndex].viewportX,
											 g_hudCockpitResourceDescriptors[viewportViewIndex].viewportY);
			SetFlightViewport(g_hudCockpitResourceDescriptors[viewportViewIndex].viewportWidth,
							  g_hudCockpitResourceDescriptors[viewportViewIndex].viewportHeight,
							  g_flightViewportMode, viewportByteOffset);
			panel_copymaskdata(g_hudCockpitResources[resourceViewIndex].entries[PANEL_COCKPIT_MASK_ENTRY],
							   g_flightVpWidth, g_flightVpHeight, g_hudCockpitMirrorHorizontal);
		} else {
			festring_setbound(0, 0, g_flightScreenWidth, g_hudClearHeight);
			festring_setbackcolor(g_flightColorEscapeBypassChar);
			g_flightFillClipRectFn();
			SetFlightViewport(PANEL_VIEW_WIDTH, PANEL_COCKPIT_HEIGHT, g_flightViewportMode, 0);
			panel_clearmaskdata(g_flightVpWidth, g_flightVpHeight);
		}
		if (g_flightCamera.hudStateLive != PANEL_VIEW_FULL_FORWARD) {
			if (g_playerFlightState.object->objectType == XW_OBJ_Y_WING && g_flightCamera.hudStateLive == 0)
				g_projOffsetY = PANEL_YWING_COCKPIT_OFFSET;
			else
				g_projOffsetY = 0;
		} else {
			switch (g_playerFlightState.craftTypeIndex) {
				case PANEL_FORWARD_CRAFT_0:
					g_projOffsetY = PANEL_FORWARD_OFFSET_0;
					break;
				case PANEL_FORWARD_CRAFT_1:
					g_projOffsetY = PANEL_FORWARD_OFFSET_1;
					break;
				case PANEL_FORWARD_CRAFT_2:
					g_projOffsetY = PANEL_FORWARD_OFFSET_2;
					break;
				case PANEL_FORWARD_CRAFT_3:
					g_projOffsetY = PANEL_FORWARD_OFFSET_3;
					break;
				default:
					break;
			}
		}
		if (g_hudCockpitMirrorHorizontal == 1)
			g_hudLoadedCockpitView = PANEL_VIEW_CACHE_INVALID;
		else {
			g_hudLoadedCockpitView = PANEL_VIEW_NO_COCKPIT;
			if (g_flightCamera.hudStateLive != PANEL_VIEW_NO_COCKPIT)
				g_hudLoadedCockpitView = resourceViewIndex;
		}
#ifdef XW_MODERN
		{
			char base[16];
			const HudCockpitResourceDescriptor* descriptor =
				&g_hudCockpitResourceDescriptors[resourceViewIndex];
			strcpy(base, descriptor->lfdName);
			strcat(base, ".LFD");
			XwHud_View(hudViewState, hudViewState == 18 ? NULL : base,
					   (XwSnapRect) { descriptor->viewportX, descriptor->viewportY, descriptor->viewportWidth,
									  descriptor->viewportHeight },
					   g_hudCockpitMirrorHorizontal != 0);
		}
#endif
		panel_initpanel();
		g_textureCacheFlushPending = 1;
		festring_showscreen();
		calcframerate = 0;
	}
	if (resourceViewIndex == PANEL_VIEW_DIRECTION_LABEL) {
		festring_setbound(g_flightScreenWidth != PANEL_DIRECTION_LOW_WIDTH ? PANEL_DIRECTION_HIGH_LEFT
																		   : PANEL_DIRECTION_LOW_LEFT,
						  g_flightScreenWidth != PANEL_DIRECTION_LOW_WIDTH ? PANEL_DIRECTION_HIGH_TOP
																		   : PANEL_DIRECTION_LOW_TOP,
						  g_flightScreenWidth != PANEL_DIRECTION_LOW_WIDTH ? PANEL_DIRECTION_HIGH_RIGHT
																		   : PANEL_DIRECTION_LOW_RIGHT,
						  g_flightScreenWidth != PANEL_DIRECTION_LOW_WIDTH ? PANEL_DIRECTION_HIGH_BOTTOM
																		   : PANEL_DIRECTION_LOW_BOTTOM);
		festring_setbackcolor(PANEL_DIRECTION_BACKGROUND);
		g_flightFillClipRectFn();
		festring_settextcolor(PANEL_DIRECTION_TEXT_COLOR);
		festring_setcursor(0, g_flightScreenWidth != PANEL_DIRECTION_LOW_WIDTH ? PANEL_DIRECTION_HIGH_TEXT_Y
																			   : PANEL_DIRECTION_LOW_TEXT_Y);
		festring_outstringcenter(g_hudCockpitResourceDescriptors[g_flightCamera.hudStateLive].viewLabel);
	}

#ifdef XW_MODERN
	XwHud_Pop(hud_pane);
#endif
#ifdef XW_MODERN
	XwHud_ViewSelection(hudViewState);
#endif
}

// FUNCTION: XW 0x41BF00
void panel_loadcontrolpanel(const char* lfdName, uint8_t** outEntries, uint16_t entryCount) {
	CockpitLfdEntryHeader entryHeader;
	uint16_t entryIndex;
	strcpy(g_hudCockpitResourcePath, g_hudCockpitResolutionDirectory);
	strcat(g_hudCockpitResourcePath, lfdName);
	strcat(g_hudCockpitResourcePath, ".LFD");
	fediskio_tryopenfile(g_hudCockpitResourcePath, "rb", 1);
#ifdef XW_MODERN
	XwRenderAssets_RegisterStream(XW_SOURCE_COCKPIT, 0, g_stream, XwStorage_LastPath());
#endif
	for (entryIndex = 0; entryIndex < entryCount; ++entryIndex) {
		int16_t isPalette;
		uint16_t tagIndex;
		uint32_t payloadIndex;
		uint32_t payloadSize;
		outEntries[entryIndex] = g_hudCockpitResourceWriteCursor;
		fediskio_readfileblock(&entryHeader, sizeof(entryHeader), 1, g_stream);
		isPalette = 1;
		for (tagIndex = 0; tagIndex != sizeof(entryHeader.typeTag); ++tagIndex) {
			if (entryHeader.typeTag[tagIndex] != g_lfdPaletteResourceTypeTag[tagIndex])
				isPalette = 0;
		}
		payloadSize = entryHeader.payloadSize;
		for (payloadIndex = 0; payloadIndex != payloadSize; ++payloadIndex) {
			int16_t byteValue = File_RawGetChar(g_stream);
			if (isPalette != 0)
				byteValue >>= PANEL_PALETTE_COMPONENT_SHIFT;
			*g_hudCockpitResourceWriteCursor = byteValue;
			++g_hudCockpitResourceWriteCursor;
		}
		if (isPalette != 0)
			outEntries[entryIndex] += PANEL_PALETTE_RANGE_HEADER_SIZE;
	}
	fediskio_tryclosefile(0);
}

// FUNCTION: XW 0x41C060
void panel_tryEMSforpanels(void) {
	uint16_t descriptorIndex;
	for (descriptorIndex = 0; descriptorIndex < PANEL_COCKPIT_DESCRIPTOR_COUNT; ++descriptorIndex) {
		g_hudCockpitResources[descriptorIndex].memoryHandle = 0;
		if (g_hudCockpitResourceDescriptors[descriptorIndex].enabled == PANEL_COCKPIT_PRELOAD_ENABLED) {
			strcpy(g_hudCockpitResourcePath, g_hudCockpitResolutionDirectory);
			strcat(g_hudCockpitResourcePath, g_hudCockpitResourceDescriptors[descriptorIndex].lfdName);
			strcat(g_hudCockpitResourcePath, ".LFD");
			fediskio_tryopenfile(g_hudCockpitResourcePath, "rb", 1);
#ifdef XW_MODERN
			XwRenderAssets_RegisterStream(XW_SOURCE_COCKPIT, 0, g_stream, XwStorage_LastPath());
#endif
			if (g_stream != NULL) {
				size_t fileSize = File_RawLength(g_stream);
				uint16_t memoryHandle;
				fediskio_tryclosefile(0);
				memoryHandle = Memory_AllocHandle(fileSize, 0);
				if (memoryHandle != 0) {
					g_hudCockpitResources[descriptorIndex].memoryHandle = memoryHandle;
					g_hudCockpitResourceWriteCursor = Memory_LockHandle(memoryHandle);
					panel_loadcontrolpanel(g_hudCockpitResourceDescriptors[descriptorIndex].lfdName,
										   g_hudCockpitResources[descriptorIndex].entries,
										   PANEL_COCKPIT_ENTRY_COUNT);
				}
			}
		}
	}
}

// FUNCTION: XW 0x41C190
void panel_copymaskdata(const uint8_t* encodedMask, uint16_t width, uint16_t height,
						int16_t mirrorHorizontal) {
	uint8_t* output = (uint8_t*)g_flightAuxBuffer + (uint16_t)g_viewportSpanMaskOffset;
	size_t inputIndex = 0;
	size_t outputIndex = 0;
	uint16_t rowsRemaining;
	for (rowsRemaining = height; rowsRemaining != 0; --rowsRemaining) {
		if (mirrorHorizontal == 0) {
			uint16_t copiedPixels;
			output[outputIndex++] = encodedMask[inputIndex++];
			for (copiedPixels = 0; copiedPixels < width;) {
				uint8_t run = encodedMask[inputIndex++];
				if (run == PANEL_SPAN_MASK_EXTENDED_RUN) {
					copiedPixels += PANEL_SPAN_MASK_FIRST_EXTENSION;
					output[outputIndex++] = PANEL_SPAN_MASK_EXTENDED_RUN;
					run = encodedMask[inputIndex++];
					if (g_flightScreenWidth != FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH) {
						if (run == PANEL_SPAN_MASK_EXTENDED_RUN) {
							copiedPixels += PANEL_SPAN_MASK_BYTE_RANGE;
							output[outputIndex++] = PANEL_SPAN_MASK_EXTENDED_RUN;
							run = encodedMask[inputIndex++];
						} else if (run == PANEL_SPAN_MASK_FIRST_EXTENSION) {
							copiedPixels += PANEL_SPAN_MASK_BYTE_RANGE;
							output[outputIndex++] = PANEL_SPAN_MASK_EXTENDED_RUN;
						}
					}
					++run;
				}
				copiedPixels += run;
				output[outputIndex++] = run;
			}
		} else {
			uint8_t rowPolarity = encodedMask[inputIndex++];
			uint8_t rowRuns[PANEL_SPAN_MASK_ROW_RUN_CAPACITY];
			size_t scratchIndex = 0;
			uint16_t decodedPixels;
			uint16_t runCount;
			int screenWidth = g_flightScreenWidth;
			for (decodedPixels = 0; decodedPixels < width;) {
				uint8_t run = encodedMask[inputIndex++];
				if (run == PANEL_SPAN_MASK_EXTENDED_RUN) {
					decodedPixels += PANEL_SPAN_MASK_FIRST_EXTENSION;
					rowRuns[scratchIndex++] = PANEL_SPAN_MASK_EXTENDED_RUN;
					run = encodedMask[inputIndex++];
					if (screenWidth != FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH) {
						if (run == PANEL_SPAN_MASK_EXTENDED_RUN) {
							decodedPixels += PANEL_SPAN_MASK_BYTE_RANGE;
							rowRuns[scratchIndex++] = PANEL_SPAN_MASK_EXTENDED_RUN;
							run = encodedMask[inputIndex++];
						} else if (run == PANEL_SPAN_MASK_FIRST_EXTENSION) {
							decodedPixels += PANEL_SPAN_MASK_BYTE_RANGE;
							rowRuns[scratchIndex++] = PANEL_SPAN_MASK_EXTENDED_RUN;
						}
					}
					++run;
				}
				decodedPixels += run;
				rowRuns[scratchIndex++] = run;
			}
			output[outputIndex] = rowPolarity;
			scratchIndex = 0;
			decodedPixels = 0;
			for (runCount = 0; decodedPixels < width; ++runCount) {
				uint8_t run = rowRuns[scratchIndex++];
				if (run == PANEL_SPAN_MASK_EXTENDED_RUN) {
					run = rowRuns[scratchIndex++];
					decodedPixels += PANEL_SPAN_MASK_FIRST_EXTENSION;
					rowRuns[scratchIndex - 2] = run;
					rowRuns[scratchIndex - 1] = PANEL_SPAN_MASK_EXTENDED_RUN;
					if (run == PANEL_SPAN_MASK_EXTENDED_RUN) {
						run = rowRuns[scratchIndex++];
						decodedPixels += PANEL_SPAN_MASK_BYTE_RANGE;
						rowRuns[scratchIndex - 3] = run;
						rowRuns[scratchIndex - 1] = PANEL_SPAN_MASK_EXTENDED_RUN;
					}
				}
				decodedPixels += run;
			}
			if ((runCount & 1) == 0) {
				output[outputIndex] = -output[outputIndex];
			}
			++outputIndex;
			for (; runCount != 0; --runCount) {
				uint8_t run = rowRuns[--scratchIndex];
				output[outputIndex++] = run;
				if (run == PANEL_SPAN_MASK_EXTENDED_RUN) {
					run = rowRuns[--scratchIndex];
					output[outputIndex++] = run;
					if (run == PANEL_SPAN_MASK_EXTENDED_RUN) {
						output[outputIndex++] = rowRuns[--scratchIndex];
					}
				}
			}
		}
	}
}

// FUNCTION: XW 0x41C350
void panel_clearmaskdata(uint16_t width, unsigned int height) {
	uint8_t* mask = (uint8_t*)g_flightAuxBuffer + (uint16_t)g_viewportSpanMaskOffset;
	size_t outputIndex = 0;
	uint16_t rowsRemaining;
	if ((uint16_t)height > 0) {
		for (rowsRemaining = (uint16_t)height; rowsRemaining > 0; --rowsRemaining) {
			uint16_t runLength = width;
			mask[outputIndex++] = PANEL_SPAN_MASK_CLEAR_ROW;
			if (runLength >= PANEL_SPAN_MASK_BYTE_RANGE) {
				mask[outputIndex++] = PANEL_SPAN_MASK_EXTENDED_RUN;
				runLength -= PANEL_SPAN_MASK_FIRST_EXTENSION;
				if (runLength >= PANEL_SPAN_MASK_BYTE_RANGE) {
					mask[outputIndex++] = PANEL_SPAN_MASK_EXTENDED_RUN;
					runLength -= PANEL_SPAN_MASK_BYTE_RANGE;
				}
			}
			mask[outputIndex++] = (uint8_t)runLength;
		}
	}
}

// FUNCTION: XW 0x41C3B0
void panel_DrawHorizontalColorSpan(int xStart, int xEnd, int y, uint8_t colorIndex) {
	xStart += g_flightClipLeft;
	xEnd += g_flightClipLeft;
	y += g_flightClipTop;
	if (g_flightBytesPerPixel == sizeof(uint16_t)) {
		uint8_t* row = g_flightSwFramebufferBase + y * g_surfacePitch;
		uint16_t color = g_flightTextPalette[colorIndex];
		uint16_t* pixels = (uint16_t*)row;
		int index;
		for (index = xStart; index < xEnd; ++index) {
			pixels[index] = color;
		}
	} else {
		uint8_t* row = g_flightSwFramebufferBase + y * g_surfacePitch;
		int index;
		for (index = xStart; index < xEnd; ++index) {
			row[index] = colorIndex;
		}
	}
}

// FUNCTION: XW 0x41C460
void panel_DrawObjectBoxCorners(int x, int y, int width, int height, uint8_t colorIndex) {
#ifdef XW_MODERN
	int bottomExclusive = (int32_t)((uint32_t)height + (uint32_t)y);
#else
	int bottomExclusive = height + y;
#endif
	if (bottomExclusive > 0) {
#ifdef XW_MODERN
		int rightExclusive = (int32_t)((uint32_t)width + (uint32_t)x);
#else
		int rightExclusive = width + x;
#endif
		if (rightExclusive > 0 && x < g_flightVpWidth && y < g_flightVpHeight && height > 0 && width > 0) {
			int cornerWidth = width >> PANEL_OBJECT_BOX_CORNER_SHIFT;
			int cornerHeight = height >> PANEL_OBJECT_BOX_CORNER_SHIFT;
			int row;
			if (cornerWidth < PANEL_OBJECT_BOX_CORNER_MIN)
				cornerWidth = PANEL_OBJECT_BOX_CORNER_MIN;
			if (cornerHeight < PANEL_OBJECT_BOX_CORNER_MIN)
				cornerHeight = PANEL_OBJECT_BOX_CORNER_MIN;
			if (cornerWidth > width)
				cornerWidth = width;
			if (cornerHeight > height)
				cornerHeight = height;
			FlightDisplay_LockSurface();
			if (y >= 0) {
				int end = cornerWidth + x;
				int start = x;
				if (end > 0 && start < g_flightVpWidth) {
					if (start < 0)
						start = 0;
					if (end > g_flightVpWidth)
						end = g_flightVpWidth;
					panel_DrawHorizontalColorSpan(start, end, y, colorIndex);
				}
				end = width + x;
				start = width - cornerWidth + x;
				if (end > 0 && start < g_flightVpWidth) {
					if (start < 0)
						start = 0;
					if (end > g_flightVpWidth)
						end = g_flightVpWidth;
					panel_DrawHorizontalColorSpan(start, end, y, colorIndex);
				}
			}
			if (bottomExclusive <= g_flightVpHeight) {
				int end = cornerWidth + x;
				int start = x;
				if (end > 0 && start < g_flightVpWidth) {
					if (start < 0)
						start = 0;
					if (end > g_flightVpWidth)
						end = g_flightVpWidth;
					panel_DrawHorizontalColorSpan(start, end, bottomExclusive - 1, colorIndex);
				}
				end = width + x;
				start = width - cornerWidth + x;
				if (end > 0 && start < g_flightVpWidth) {
					if (start < 0)
						start = 0;
					if (end > g_flightVpWidth)
						end = g_flightVpWidth;
					panel_DrawHorizontalColorSpan(start, end, bottomExclusive - 1, colorIndex);
				}
			}
			for (row = 1; row < cornerHeight; ++row) {
				int rowY = y + row;
				if (rowY >= 0 && rowY < g_flightVpHeight) {
					if (x >= 0)
						panel_DrawHorizontalColorSpan(x, x + 1, rowY, colorIndex);
					if (rightExclusive <= g_flightVpWidth)
						panel_DrawHorizontalColorSpan(rightExclusive - 1, rightExclusive, rowY, colorIndex);
				}
			}
			for (row = height - cornerHeight; row < height - 1; ++row) {
				int rowY = y + row;
				if (row >= cornerHeight && rowY >= 0 && rowY < g_flightVpHeight) {
					if (x >= 0)
						panel_DrawHorizontalColorSpan(x, x + 1, rowY, colorIndex);
					if (rightExclusive <= g_flightVpWidth)
						panel_DrawHorizontalColorSpan(rightExclusive - 1, rightExclusive, rowY, colorIndex);
				}
			}
			FlightDisplay_UnlockSurface();
		}
	}
}
