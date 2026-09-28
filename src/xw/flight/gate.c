#include "xw/flight/gate.h"

#ifdef XW_MODERN
#include "xw_runtime/snapshot/render_hud.h"
#include "xw_runtime/timing/flight_integration.h"
#include "xw_runtime/timing/player_timing.h"
#include "xw_runtime/timing/reference_motion.h"
#endif

#ifdef XW_MODERN
#include "xw_runtime/snapshot/render_objects.h"
#include "xw_runtime/snapshot/render_world.h"
#endif

#ifdef XW_MODERN
#include "xw/flight/flight.h"
#endif
#ifdef XW_MODERN
#include "xw_dos94/flight/special_world.h"
#include "xw_runtime/runtime/port.h"
#endif
#ifdef XW_MODERN
#include "xw_runtime/runtime/flight_dispatch.h"
#include "xw_runtime/runtime/flight_types.h"
#include "xw_runtime/runtime/profile.h"
#endif

#include "xw/assets/model_mesh.h"
#include "xw/audio/fsfx.h"
#include "xw/flight/fediskio.h"
#include "xw/flight/feinput.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/fview.h"
#include "xw/flight/hud/festring.h"
#include "xw/flight/hud/msg.h"
#include "xw/flight/hud/panel.h"
#include "xw/flight/hud/panelrts.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/collide.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/laser.h"
#include "xw/flight/object/object.h"
#include "xw/flight/player/user.h"
#include "xw/flight/replay/replay.h"
#include "xw/flight/xw.h"
#include "xw/math/math2.h"
#include "xw/math/transfm2.h"
#include "xw/math/trig2.h"
#include "xw/render/render_scene.h"

#ifdef XW_MODERN
#include "xw_runtime/runtime/gate_collision_task.h"
#endif

#include <stdlib.h>
#ifdef XW_MODERN
#include <string.h>
#endif

// GLOBAL: XW 0x4C7828
int g_provingGroundsScoreDecimalDivisors[GATE_SCORE_DIVISOR_COUNT] = { 1,     1,      10,      100,     1000,
																	   10000, 100000, 1000000, 10000000 };

// GLOBAL: XW 0x4F48E4
int g_provingGroundsPanelHeight = 0;

// GLOBAL: XW 0x4F48E8
int g_provingGroundsBonusTextY = 0;

// GLOBAL: XW 0x4F48EC
int g_provingGroundsBonusTimerX = 0;

// GLOBAL: XW 0x4F48F0
int g_provingGroundsLevelLabelXOffset = 0;

// GLOBAL: XW 0x4F48F4
int g_provingGroundsPanelWidth = 0;

// GLOBAL: XW 0x4F48F8
int g_provingGroundsLevelValueXOffset = 0;

// GLOBAL: XW 0x4F48FC
int g_provingGroundsBonusClipTop = 0;

// GLOBAL: XW 0x4F4900
int g_provingGroundsCounterXOffset = 0;

// GLOBAL: XW 0x4F4904
int g_provingGroundsLineSpacing = 0;

// GLOBAL: XW 0x4F4908
int g_provingGroundsBonusValueX = 0;

// GLOBAL: XW 0x4F490C
int g_provingGroundsScoreXOffset = 0;

// GLOBAL: XW 0x4F4910
int g_provingGroundsBonusClipRight = 0;

// GLOBAL: XW 0x4F4914
int g_provingGroundsBonusClipBottom = 0;

// GLOBAL: XW 0x62B4EC
uint16_t g_gateGunTimer = 0;

// GLOBAL: XW 0x62D124
int g_gateIntersectionX = 0;

// GLOBAL: XW 0x62D12C
int g_gateIntersectionZ = 0;

// GLOBAL: XW 0x63735C
uint16_t g_provingGroundsRenderCheckpointState = 0;

// GLOBAL: XW 0x6377AC
uint16_t g_provingGroundsCheckpointBlinkTicks = 0;

// GLOBAL: XW 0x63B1D0
int16_t g_gatePreviousRoll[GATE_POSE_HISTORY_COUNT] = { 0 };

// GLOBAL: XW 0x63B1D8
int16_t g_gatePreviousPitch[GATE_POSE_HISTORY_COUNT] = { 0 };

// GLOBAL: XW 0x63B1E0
int16_t g_gatePreviousYaw[GATE_POSE_HISTORY_COUNT] = { 0 };

// GLOBAL: XW 0x63B1F0
int g_gatePreviousZ[GATE_POSE_HISTORY_COUNT] = { 0 };

// GLOBAL: XW 0x63B200
int g_gatePreviousX[GATE_POSE_HISTORY_COUNT] = { 0 };

// GLOBAL: XW 0x63B210
int g_gatePreviousY[GATE_POSE_HISTORY_COUNT] = { 0 };

// GLOBAL: XW 0x63BBE0
int16_t g_provingGroundsMeshDrawOrder[GATE_DRAW_ORDER_COUNT] = { 0 };

// FUNCTION: XW 0x40E320
void gate_savegatelastpos(void) {
	unsigned int index;
#ifdef XW_MODERN
	int32_t position[3];
	if (!XwPlayerTiming_RecordRecovery(position))
		return;
#endif
	for (index = GATE_POSE_HISTORY_COUNT - 1; index > 0; --index) {
		g_gatePreviousX[index] = g_gatePreviousX[index - 1];
		g_gatePreviousY[index] = g_gatePreviousY[index - 1];
		g_gatePreviousZ[index] = g_gatePreviousZ[index - 1];
		g_gatePreviousRoll[index] = g_gatePreviousRoll[index - 1];
		g_gatePreviousPitch[index] = g_gatePreviousPitch[index - 1];
		g_gatePreviousYaw[index] = g_gatePreviousYaw[index - 1];
	}
#ifdef XW_MODERN
	g_gatePreviousX[0] = position[0];
	g_gatePreviousY[0] = position[1];
	g_gatePreviousZ[0] = position[2];
#else
	g_gatePreviousX[0] = g_playerFlightState.object->prevWorldX;
	g_gatePreviousY[0] = g_playerFlightState.object->prevWorldY;
	g_gatePreviousZ[0] = g_playerFlightState.object->prevWorldZ;
#endif
	g_gatePreviousRoll[0] = g_playerFlightState.object->roll;
	g_gatePreviousPitch[0] = g_playerFlightState.object->pitch;
	g_gatePreviousYaw[0] = g_playerFlightState.object->yaw;
}

// FUNCTION: XW 0x40E400
void gate_DrawCourseObject(uint16_t missionObjectIndex) {
	int objectIndex = missionObjectIndex;
	uint8_t targetState;
	uint8_t checkpointStates;
	uint16_t objectType;
	uint16_t targetMesh0, targetMesh1, targetMesh2, targetMesh3, targetMesh4, targetMesh5;
	int orderIndex;
	g_billboardObjectOrTypeIndex = missionObjectIndex + GATE_RENDER_REFERENCE_BASE;
	targetState = g_missionObjects[objectIndex].stateByte;
	objectType = g_missionObjects[objectIndex].objectType;
	checkpointStates = g_missionObjects[objectIndex].typeSpecificByte;
	if ((g_flightGraphicsDetailPreset == 0 && g_objectViewZ > GATE_LOW_DETAIL_DRAW_DISTANCE) ||
		(g_flightGraphicsDetailPreset == 1 && g_objectViewZ > GATE_MEDIUM_DETAIL_DRAW_DISTANCE))
		return;
	if (g_objectViewZ > GATE_NEAR_DRAW_DISTANCE) {
		RenderScene_DrawObjectRootMeshWithSwitch(&g_missionObjects[objectIndex], 0, 0);
		return;
	}
	if (objectType == GATE_OBJECT_TYPE_202) {
		for (orderIndex = GATE_SIMPLE_MESH_COUNT; orderIndex != 0; --orderIndex)
			RenderScene_DrawObjectRootMeshWithSwitch(&g_missionObjects[objectIndex],
													 GATE_SIMPLE_MESH_COUNT - orderIndex, 0);
		return;
	}
	g_provingGroundsMeshDrawOrder[0] = 0;
	g_provingGroundsMeshDrawOrder[GATE_DRAW_ORDER_COUNT - 1] = -1;
	switch (objectType) {
		case GATE_OBJECT_TYPE_205:
			targetMesh0 = GATE_GUN_FIRST_MESH;
			targetMesh1 = GATE_GUN_FIRST_MESH + 1;
			targetMesh2 = GATE_GUN_FIRST_MESH + 2;
			targetMesh3 = GATE_GUN_FIRST_MESH + 3;
			targetMesh4 = GATE_GUN_FIRST_MESH + 4;
			targetMesh5 = GATE_GUN_FIRST_MESH + 5;
			break;
		case GATE_OBJECT_TYPE_207:
			targetMesh0 = GATE_GUN_FIRST_MESH;
			targetMesh1 = GATE_GUN_FIRST_MESH + 1;
			targetMesh2 = GATE_GUN_FIRST_MESH + 2;
			targetMesh3 = GATE_GUN_FIRST_MESH + 3;
			targetMesh5 = GATE_GUN_FIRST_MESH + 4;
			targetMesh4 = GATE_GUN_FIRST_MESH + 5;
			break;
		case GATE_OBJECT_TYPE_209:
			targetMesh4 = GATE_GUN_FIRST_MESH;
			targetMesh5 = GATE_GUN_FIRST_MESH + 1;
			targetMesh2 = GATE_GUN_FIRST_MESH + 2;
			targetMesh3 = GATE_GUN_FIRST_MESH + 3;
			targetMesh0 = GATE_GUN_FIRST_MESH + 4;
			targetMesh1 = GATE_GUN_FIRST_MESH + 5;
			break;
		case GATE_OBJECT_TYPE_203:
			targetMesh2 = GATE_GUN_FIRST_MESH;
			targetMesh3 = GATE_GUN_FIRST_MESH + 1;
			targetMesh0 = GATE_GUN_FIRST_MESH + 2;
			targetMesh1 = GATE_GUN_FIRST_MESH + 3;
			targetMesh4 = GATE_GUN_FIRST_MESH + 4;
			targetMesh5 = GATE_GUN_FIRST_MESH + 5;
			break;
		case GATE_OBJECT_TYPE_204:
			targetMesh5 = GATE_GUN_FIRST_MESH;
			targetMesh4 = GATE_GUN_FIRST_MESH + 1;
			targetMesh1 = GATE_GUN_FIRST_MESH + 2;
			targetMesh0 = GATE_GUN_FIRST_MESH + 3;
			targetMesh3 = GATE_GUN_FIRST_MESH + 4;
			targetMesh2 = GATE_GUN_FIRST_MESH + 5;
			break;
		case GATE_OBJECT_TYPE_206:
			targetMesh5 = GATE_GUN_FIRST_MESH;
			targetMesh4 = GATE_GUN_FIRST_MESH + 1;
			targetMesh3 = GATE_GUN_FIRST_MESH + 2;
			targetMesh2 = GATE_GUN_FIRST_MESH + 3;
			targetMesh1 = GATE_GUN_FIRST_MESH + 4;
			targetMesh0 = GATE_GUN_FIRST_MESH + 5;
			break;
		case GATE_OBJECT_TYPE_208:
			targetMesh0 = GATE_GUN_FIRST_MESH;
			targetMesh1 = GATE_GUN_FIRST_MESH + 1;
			targetMesh4 = GATE_GUN_FIRST_MESH + 2;
			targetMesh5 = GATE_GUN_FIRST_MESH + 3;
			targetMesh2 = GATE_GUN_FIRST_MESH + 4;
			targetMesh3 = GATE_GUN_FIRST_MESH + 5;
			break;
		case GATE_OBJECT_TYPE_210:
			targetMesh4 = GATE_GUN_FIRST_MESH;
			targetMesh5 = GATE_GUN_FIRST_MESH + 1;
			targetMesh3 = GATE_GUN_FIRST_MESH + 2;
			targetMesh2 = GATE_GUN_FIRST_MESH + 3;
			targetMesh1 = GATE_GUN_FIRST_MESH + 4;
			targetMesh0 = GATE_GUN_FIRST_MESH + 5;
			break;
		default:
			/* Valid mission indices are below 256, so the original reused low word is the state byte. */
			targetMesh0 = targetState;
			targetMesh1 = targetState;
			targetMesh2 = targetState;
			targetMesh3 = targetState;
			targetMesh4 = targetState;
			targetMesh5 = targetState;
			break;
	}
	g_provingGroundsMeshDrawOrder[1] = GATE_FIRST_CHECKPOINT_MESH;
	g_provingGroundsMeshDrawOrder[2] = targetMesh3;
	g_provingGroundsMeshDrawOrder[3] = targetMesh2;
	switch (objectType - GATE_OBJECT_TYPE_203) {
		case GATE_OBJECT_TYPE_203 - GATE_OBJECT_TYPE_203:
		case GATE_OBJECT_TYPE_204 - GATE_OBJECT_TYPE_203:
		case GATE_OBJECT_TYPE_205 - GATE_OBJECT_TYPE_203:
		case GATE_OBJECT_TYPE_206 - GATE_OBJECT_TYPE_203:
		case GATE_OBJECT_TYPE_207 - GATE_OBJECT_TYPE_203:
		case GATE_OBJECT_TYPE_208 - GATE_OBJECT_TYPE_203:
		case GATE_OBJECT_TYPE_209 - GATE_OBJECT_TYPE_203:
		case GATE_OBJECT_TYPE_210 - GATE_OBJECT_TYPE_203:
			g_provingGroundsMeshDrawOrder[4] = GATE_FIRST_CHECKPOINT_MESH + 1;
			g_provingGroundsMeshDrawOrder[5] = targetMesh1;
			g_provingGroundsMeshDrawOrder[6] = targetMesh0;
			break;
		default:
			break;
	}
	g_provingGroundsMeshDrawOrder[8] = targetMesh5;
	g_provingGroundsMeshDrawOrder[7] = GATE_CHECKPOINT_MESH_COUNT;
	g_provingGroundsMeshDrawOrder[9] = targetMesh4;
	for (orderIndex = GATE_DRAW_ORDER_COUNT; orderIndex != 0; --orderIndex) {
		uint16_t meshIndex = g_provingGroundsMeshDrawOrder[GATE_DRAW_ORDER_COUNT - orderIndex];
		uint16_t nodeSwitch = 0;
		if (meshIndex == GATE_NO_CHECKPOINT)
			continue;
		if (objectType == GATE_OBJECT_TYPE_204) {
			if (meshIndex == GATE_CHECKPOINT_MESH_COUNT)
				meshIndex = GATE_FIRST_CHECKPOINT_MESH;
			else if (meshIndex == GATE_FIRST_CHECKPOINT_MESH)
				meshIndex = GATE_CHECKPOINT_MESH_COUNT;
		}
		g_provingGroundsRenderCheckpointState = 0;
		if (meshIndex > GATE_CHECKPOINT_MESH_COUNT) {
			uint16_t targetFlags = targetState;
			if (meshIndex > GATE_GUN_FIRST_MESH) {
#ifdef XW_MODERN
				targetFlags >>= (meshIndex - GATE_GUN_FIRST_MESH) & TRANSFM2_SHIFT_COUNT_MASK;
#else
				targetFlags >>= meshIndex - GATE_GUN_FIRST_MESH;
#endif
			}
			if ((targetFlags & 1) == 0)
				continue;
		} else if (meshIndex != 0) {
			uint16_t checkpointState = checkpointStates;
			if (meshIndex > GATE_FIRST_CHECKPOINT_MESH)
				checkpointState >>= GATE_CHECKPOINT_PAIR_BITS * (meshIndex - 1);
			nodeSwitch = checkpointState & GATE_CHECKPOINT_STATE_MASK;
			g_provingGroundsRenderCheckpointState = nodeSwitch;
		}
		g_provingGroundsCheckpointBlinkTicks += g_elapsedTicks;
		if (g_provingGroundsCheckpointBlinkTicks > GATE_BLINK_PERIOD)
			g_provingGroundsCheckpointBlinkTicks = 0;
#ifdef XW_MODERN
		XwRenderWorld_Checkpoint(missionObjectIndex, meshIndex,
								 g_provingGroundsCheckpointBlinkTicks < GATE_BLINK_ON_TICKS);
#endif
		if (g_provingGroundsCheckpointBlinkTicks < GATE_BLINK_ON_TICKS)
			RenderScene_DrawObjectRootMeshWithSwitch(&g_missionObjects[objectIndex], meshIndex, nodeSwitch);
		else
			RenderScene_DrawObjectRootMeshWithSwitch(&g_missionObjects[objectIndex], meshIndex, 0);
	}
}

// FUNCTION: XW 0x40E7C0
int16_t gate_ProcessCourseCollision(uint16_t objectIndex) {
#ifdef XW_MODERN
	const uint16_t finishType = XwFlightTypes_ObjectType(GATE_OBJECT_TYPE_202);
#else
	const uint16_t finishType = GATE_OBJECT_TYPE_202;
#endif
	uint16_t gateIndex;
#ifdef XW_MODERN
	const int checkpointDepth = XwFlightTypes_Dos() ? 1280 : GATE_CHECKPOINT_DEPTH;
#else
	const int checkpointDepth = GATE_CHECKPOINT_DEPTH;
#endif
#ifdef XW_MODERN
	XwGateCollisionResume resume;
	int resuming = XwGateCollision_Resume(objectIndex, &resume);
	if (resuming == XW_GATE_COLLISION_PENDING)
		return XW_GATE_COLLISION_PENDING;
	if (!resuming && !XwFlightTypes_Dos() && g_objectTable[objectIndex].iff == GATE_GUN_IFF)
		return 0;
	for (gateIndex = resuming ? resume.gateIndex : 0; gateIndex < MISSION_OBJECT_COUNT; ++gateIndex) {
#else
	if (g_objectTable[objectIndex].iff == GATE_GUN_IFF)
		return 0;
	for (gateIndex = 0; gateIndex < MISSION_OBJECT_COUNT; ++gateIndex) {
#endif
		uint16_t objectType;
		int gateWorldX, gateWorldY, gateWorldZ;
		int16_t originalStartGateUp;
		uint16_t impactObjectType;
		uint16_t targetMeshIndex;
		uint8_t savedTargetFlags;
		int16_t targetMinX, targetMinY, targetMinZ, targetMaxX, targetMaxY, targetMaxZ;
		uint16_t checkpointNumber;
#ifdef XW_MODERN
		if (resuming) {
			targetMeshIndex = GATE_GUN_FIRST_MESH + GATE_GUN_COUNT;
			objectType = resume.objectType;
			gateWorldX = resume.gateWorldX;
			gateWorldY = resume.gateWorldY;
			gateWorldZ = resume.gateWorldZ;
			originalStartGateUp = resume.originalStartGateUp;
		} else
#endif
		{
			int endRelativeX, endRelativeY, endRelativeZ;
			int startRelativeX, startRelativeY, startRelativeZ;
			int16_t roll, yaw, pitch;
			int side, up, forward;
			uint16_t objectGenus = g_missionObjects[gateIndex].genusId;
			objectType = g_missionObjects[gateIndex].objectType;
			if (objectType == XW_OBJ_NONE || objectGenus != XW_GENUS_SCENERY)
				continue;
			create_getworldposition(gateIndex + XW_MISSION_OBJECT_REF_BASE, 0);
#ifdef XW_MODERN
			endRelativeZ = (int32_t)((uint32_t)g_collisionProbeWorldZ - (uint32_t)g_resolvedWorldZ);
#else
			endRelativeZ = g_collisionProbeWorldZ - g_resolvedWorldZ;
#endif
#ifdef XW_MODERN
			endRelativeY = (int32_t)((uint32_t)g_collisionProbeWorldY - (uint32_t)g_resolvedWorldY);
#else
			endRelativeY = g_collisionProbeWorldY - g_resolvedWorldY;
#endif
#ifdef XW_MODERN
			endRelativeX = (int32_t)((uint32_t)g_collisionProbeWorldX - (uint32_t)g_resolvedWorldX);
#else
			endRelativeX = g_collisionProbeWorldX - g_resolvedWorldX;
#endif
			gateWorldX = g_resolvedWorldX;
			gateWorldY = g_resolvedWorldY;
			gateWorldZ = g_resolvedWorldZ;
			if ((unsigned int)collide_roughdistance3d(endRelativeX, endRelativeY, endRelativeZ) >
				GATE_COLLISION_RANGE)
				continue;
#ifdef XW_MODERN
			startRelativeX = (int32_t)((uint32_t)g_collisionSegmentStartWorldX - (uint32_t)gateWorldX);
#else
			startRelativeX = g_collisionSegmentStartWorldX - gateWorldX;
#endif
#ifdef XW_MODERN
			startRelativeY = (int32_t)((uint32_t)g_collisionSegmentStartWorldY - (uint32_t)gateWorldY);
#else
			startRelativeY = g_collisionSegmentStartWorldY - gateWorldY;
#endif
#ifdef XW_MODERN
			startRelativeZ = (int32_t)((uint32_t)g_collisionSegmentStartWorldZ - (uint32_t)gateWorldZ);
#else
			startRelativeZ = g_collisionSegmentStartWorldZ - gateWorldZ;
#endif
			if ((unsigned int)collide_roughdistance3d(startRelativeX, startRelativeY, startRelativeZ) >
				GATE_COLLISION_RANGE)
				continue;
			g_collisionScratchPoint2X = startRelativeX;
			g_collisionScratchPoint2Y = startRelativeY;
			g_collisionScratchPoint2Z = startRelativeZ;
			g_collisionScratchPoint1X = endRelativeX;
			g_collisionScratchPoint1Y = endRelativeY;
			g_collisionScratchPoint1Z = endRelativeZ;
#ifdef XW_MODERN
			if (XwFlightTypes_Dos()) {
				g_collisionScratchPoint1X = (int16_t)g_collisionScratchPoint1X;
				g_collisionScratchPoint1Y = (int16_t)g_collisionScratchPoint1Y;
				g_collisionScratchPoint1Z = (int16_t)g_collisionScratchPoint1Z;
				g_collisionScratchPoint2X = (int16_t)g_collisionScratchPoint2X;
				g_collisionScratchPoint2Y = (int16_t)g_collisionScratchPoint2Y;
				g_collisionScratchPoint2Z = (int16_t)g_collisionScratchPoint2Z;
			}
#endif
			roll = (uint16_t)g_missionObjects[gateIndex].rollAngle8 << GATE_ANGLE_SHIFT;
			yaw = (uint16_t)g_missionObjects[gateIndex].yawAngle8 << GATE_ANGLE_SHIFT;
			pitch = (uint16_t)g_missionObjects[gateIndex].pitchAngle8 << GATE_ANGLE_SHIFT;
			fview_calcrotatemove(pitch, yaw, NULL);
			fview_calcrotateorient(roll, 0, NULL);
			g_fviewForwardX_Q15 = -g_fviewForwardX_Q15;
			g_fviewForwardY_Q15 = -g_fviewForwardY_Q15;
			g_fviewForwardZ_Q15 = -g_fviewForwardZ_Q15;
#ifdef XW_MODERN
			side = (int32_t)((uint32_t)g_collisionScratchPoint1Z * (uint32_t)g_fviewSideZ_Q15 +
										   (uint32_t)g_collisionScratchPoint1Y * (uint32_t)g_fviewSideY_Q15 +
										   (uint32_t)g_collisionScratchPoint1X * (uint32_t)g_fviewSideX_Q15);
#else
			side = g_collisionScratchPoint1Z * g_fviewSideZ_Q15 +
								 g_collisionScratchPoint1Y * g_fviewSideY_Q15 +
								 g_collisionScratchPoint1X * g_fviewSideX_Q15;
#endif
#ifdef XW_MODERN
			if (XwFlightTypes_Dos())
				side = Dos94World_ClampDot(side);
			else
#endif
			{
				if (side >= FVIEW_DOT_CLAMP_LIMIT)
					side = FVIEW_DOT_CLAMP_MAX;
				if (side <= -FVIEW_DOT_CLAMP_LIMIT)
					side = FVIEW_DOT_CLAMP_MIN;
			}
			side >>= FVIEW_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
			up = (int32_t)((uint32_t)g_collisionScratchPoint1Z * (uint32_t)g_fviewUpZ_Q15 +
										 (uint32_t)g_collisionScratchPoint1Y * (uint32_t)g_fviewUpY_Q15 +
										 (uint32_t)g_collisionScratchPoint1X * (uint32_t)g_fviewUpX_Q15);
#else
			up = g_collisionScratchPoint1Z * g_fviewUpZ_Q15 +
							   g_collisionScratchPoint1Y * g_fviewUpY_Q15 +
							   g_collisionScratchPoint1X * g_fviewUpX_Q15;
#endif
#ifdef XW_MODERN
			if (XwFlightTypes_Dos())
				up = Dos94World_ClampDot(up);
			else
#endif
			{
				if (up >= FVIEW_DOT_CLAMP_LIMIT)
					up = FVIEW_DOT_CLAMP_MAX;
				if (up <= -FVIEW_DOT_CLAMP_LIMIT)
					up = FVIEW_DOT_CLAMP_MIN;
			}
			up >>= FVIEW_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
			forward =
				(int32_t)((uint32_t)g_collisionScratchPoint1Z * (uint32_t)g_fviewForwardZ_Q15 +
						  (uint32_t)g_collisionScratchPoint1Y * (uint32_t)g_fviewForwardY_Q15 +
						  (uint32_t)g_collisionScratchPoint1X * (uint32_t)g_fviewForwardX_Q15);
#else
			forward = g_collisionScratchPoint1Z * g_fviewForwardZ_Q15 +
									g_collisionScratchPoint1Y * g_fviewForwardY_Q15 +
									g_collisionScratchPoint1X * g_fviewForwardX_Q15;
#endif
#ifdef XW_MODERN
			if (XwFlightTypes_Dos())
				forward = Dos94World_ClampDot(forward);
			else
#endif
			{
				if (forward >= FVIEW_DOT_CLAMP_LIMIT)
					forward = FVIEW_DOT_CLAMP_MAX;
				if (forward <= -FVIEW_DOT_CLAMP_LIMIT)
					forward = FVIEW_DOT_CLAMP_MIN;
			}
			g_collisionScratchPoint1X = (int16_t)side;
			g_collisionScratchPoint1Y = (int16_t)up;
			g_collisionScratchPoint1Z = (int16_t)(forward >> FVIEW_MATRIX_FRACTION_BITS);
#ifdef XW_MODERN
			side =
				(int32_t)((uint32_t)g_collisionScratchPoint2Z * (uint32_t)g_fviewSideZ_Q15 +
						  (uint32_t)g_collisionScratchPoint2Y * (uint32_t)g_fviewSideY_Q15 +
						  (uint32_t)g_collisionScratchPoint2X * (uint32_t)g_fviewSideX_Q15);
#else
			side = g_collisionScratchPoint2Z * g_fviewSideZ_Q15 +
								   g_collisionScratchPoint2Y * g_fviewSideY_Q15 +
								   g_collisionScratchPoint2X * g_fviewSideX_Q15;
#endif
#ifdef XW_MODERN
			if (XwFlightTypes_Dos())
				side = Dos94World_ClampDot(side);
			else
#endif
			{
				if (side >= FVIEW_DOT_CLAMP_LIMIT)
					side = FVIEW_DOT_CLAMP_MAX;
				if (side <= -FVIEW_DOT_CLAMP_LIMIT)
					side = FVIEW_DOT_CLAMP_MIN;
			}
			side >>= FVIEW_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
			up = (int32_t)((uint32_t)g_collisionScratchPoint2Z * (uint32_t)g_fviewUpZ_Q15 +
										   (uint32_t)g_collisionScratchPoint2Y * (uint32_t)g_fviewUpY_Q15 +
										   (uint32_t)g_collisionScratchPoint2X * (uint32_t)g_fviewUpX_Q15);
#else
			up = g_collisionScratchPoint2Z * g_fviewUpZ_Q15 +
								 g_collisionScratchPoint2Y * g_fviewUpY_Q15 +
								 g_collisionScratchPoint2X * g_fviewUpX_Q15;
#endif
#ifdef XW_MODERN
			if (XwFlightTypes_Dos())
				up = Dos94World_ClampDot(up);
			else
#endif
			{
				if (up >= FVIEW_DOT_CLAMP_LIMIT)
					up = FVIEW_DOT_CLAMP_MAX;
				if (up <= -FVIEW_DOT_CLAMP_LIMIT)
					up = FVIEW_DOT_CLAMP_MIN;
			}
			up >>= FVIEW_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
			forward =
				(int32_t)((uint32_t)g_collisionScratchPoint2Z * (uint32_t)g_fviewForwardZ_Q15 +
						  (uint32_t)g_collisionScratchPoint2Y * (uint32_t)g_fviewForwardY_Q15 +
						  (uint32_t)g_collisionScratchPoint2X * (uint32_t)g_fviewForwardX_Q15);
#else
			forward = g_collisionScratchPoint2Z * g_fviewForwardZ_Q15 +
									  g_collisionScratchPoint2Y * g_fviewForwardY_Q15 +
									  g_collisionScratchPoint2X * g_fviewForwardX_Q15;
#endif
#ifdef XW_MODERN
			if (XwFlightTypes_Dos())
				forward = Dos94World_ClampDot(forward);
			else
#endif
			{
				if (forward >= FVIEW_DOT_CLAMP_LIMIT)
					forward = FVIEW_DOT_CLAMP_MAX;
				if (forward <= -FVIEW_DOT_CLAMP_LIMIT)
					forward = FVIEW_DOT_CLAMP_MIN;
			}
			g_collisionScratchPoint2X = (int16_t)side;
			g_collisionScratchPoint2Y = (int16_t)up;
			g_collisionScratchPoint2Z = (int16_t)(forward >> FVIEW_MATRIX_FRACTION_BITS);
			originalStartGateUp = (int16_t)up;
			if ((g_collisionScratchPoint1Y > 0 || g_collisionScratchPoint2Y > 0) &&
				(g_collisionScratchPoint1Y < GATE_TARGET_DEPTH ||
				 g_collisionScratchPoint2Y < GATE_TARGET_DEPTH) &&
				objectType != finishType
#ifdef XW_MODERN
				&& (!XwFlightTypes_Dos() || g_objectTable[objectIndex].iff != 1)
#endif
			) {
				for (targetMeshIndex = GATE_GUN_FIRST_MESH;
					 targetMeshIndex < GATE_GUN_FIRST_MESH + GATE_GUN_COUNT; ++targetMeshIndex) {
					uint8_t targetFlags;
					savedTargetFlags = g_missionObjects[gateIndex].stateByte;
					targetFlags = savedTargetFlags;
					if (targetMeshIndex > GATE_GUN_FIRST_MESH)
						targetFlags >>= targetMeshIndex - GATE_GUN_FIRST_MESH;
					if ((targetFlags & 1) == 0)
						continue;
#ifdef XW_MODERN
					if (XwFlightTypes_Dos()) {
						XwBounds16 b;
						if (!Dos94World_Bounds(objectType, targetMeshIndex, &b))
							continue;
						targetMinX = b.minX;
						targetMinY = b.minY;
						targetMinZ = b.minZ;
						targetMaxX = b.maxX;
						targetMaxY = b.maxY;
						targetMaxZ = b.maxZ;
					} else
#endif
					{
						targetMinX = ModelMesh_GetBoundsMinX(objectType, targetMeshIndex);
						targetMinY = ModelMesh_GetBoundsMinY(objectType, targetMeshIndex);
						targetMinZ = ModelMesh_GetBoundsMinZ(objectType, targetMeshIndex);
						targetMaxX = ModelMesh_GetBoundsMaxX(objectType, targetMeshIndex);
						targetMaxY = ModelMesh_GetBoundsMaxY(objectType, targetMeshIndex);
						targetMaxZ = ModelMesh_GetBoundsMaxZ(objectType, targetMeshIndex);
					}
					if ((g_collisionScratchPoint1X < targetMinX && g_collisionScratchPoint2X < targetMinX) ||
						(g_collisionScratchPoint1Y < targetMinZ && g_collisionScratchPoint2Y < targetMinZ) ||
						(g_collisionScratchPoint1Z < targetMinY && g_collisionScratchPoint2Z < targetMinY) ||
						(g_collisionScratchPoint1X > targetMaxX && g_collisionScratchPoint2X > targetMaxX) ||
						(g_collisionScratchPoint1Y > targetMaxZ && g_collisionScratchPoint2Y > targetMaxZ) ||
						(g_collisionScratchPoint1Z > targetMaxY && g_collisionScratchPoint2Z > targetMaxY))
						continue;
					break;
				}
			} else {
				targetMeshIndex = GATE_GUN_FIRST_MESH + GATE_GUN_COUNT;
			}
		}
		if (targetMeshIndex < GATE_GUN_FIRST_MESH + GATE_GUN_COUNT) {
			++g_missionRuntimeState.provingGroundsTargetsDestroyed;
			g_missionRuntimeState.provingGroundsScore += GATE_TARGET_SCORE;
			g_missionCountdownClock.seconds += GATE_TARGET_SECONDS;
			if (g_missionCountdownClock.seconds >= XW_SECONDS_PER_MINUTE) {
				g_missionCountdownClock.seconds -= XW_SECONDS_PER_MINUTE;
				++g_missionCountdownClock.minutes;
			}
			g_missionObjects[gateIndex].stateByte =
				savedTargetFlags & ~(1u << (targetMeshIndex - GATE_GUN_FIRST_MESH));
			g_collisionScratchPoint1X = (int16_t)(targetMinX + ((targetMaxX - targetMinX) >> 1));
			g_collisionScratchPoint1Y = (int16_t)(targetMinZ + ((targetMaxZ - targetMinZ) >> 1));
			g_collisionScratchPoint1Z = (int16_t)(targetMinY + ((targetMaxY - targetMinY) >> 1));
			impactObjectType = (math2_getrandom() & 1) + XW_OBJ_EXPLOSION_133;
		} else {
#ifdef XW_MODERN
			if (resuming ||
#else
			if (
#endif
				(objectIndex == g_playerFlightState.objectIndex &&
				 (g_collisionScratchPoint1Y < checkpointDepth ||
				  g_collisionScratchPoint2Y < checkpointDepth))) {
#ifdef XW_MODERN
				checkpointNumber = resuming ? resume.checkpointNumber + 1 : GATE_FIRST_CHECKPOINT_MESH;
				resuming = 0;
#else
				checkpointNumber = GATE_FIRST_CHECKPOINT_MESH;
#endif
				for (; checkpointNumber < GATE_CHECKPOINT_MESH_COUNT + 1; ++checkpointNumber) {
					uint8_t savedFlags, selectedPair;
					int16_t minX, minForward, minUp, maxX, maxForward, maxUp;
					uint16_t previousCheckpointIndex;
					int16_t playCheckpointSound;
					if (objectType == finishType && checkpointNumber > GATE_FIRST_CHECKPOINT_MESH)
						continue;
					savedFlags = g_missionObjects[gateIndex].typeSpecificByte;
					selectedPair = savedFlags;
					if (checkpointNumber > GATE_FIRST_CHECKPOINT_MESH)
						selectedPair >>=
							GATE_CHECKPOINT_PAIR_BITS * (checkpointNumber - GATE_FIRST_CHECKPOINT_MESH);
					if (selectedPair & GATE_CHECKPOINT_COMPLETE)
						continue;
#ifdef XW_MODERN
					if (XwFlightTypes_Dos()) {
						XwBounds16 b;
						if (!Dos94World_Bounds(objectType, checkpointNumber, &b))
							continue;
						minX = b.minX;
						minForward = b.minY;
						minUp = b.minZ;
						maxX = b.maxX;
						maxForward = b.maxY;
						maxUp = b.maxZ;
					} else
#endif
					{
						minX = ModelMesh_GetBoundsMinX(objectType, checkpointNumber);
						minForward = ModelMesh_GetBoundsMinY(objectType, checkpointNumber);
						minUp = ModelMesh_GetBoundsMinZ(objectType, checkpointNumber);
						maxX = ModelMesh_GetBoundsMaxX(objectType, checkpointNumber);
						maxForward = ModelMesh_GetBoundsMaxY(objectType, checkpointNumber);
						maxUp = ModelMesh_GetBoundsMaxZ(objectType, checkpointNumber);
					}
#ifdef XW_MODERN
					if (XwFlightTypes_Dos() ? maxUp == 1280 : maxUp >= checkpointDepth)
#else
					if (maxUp >= checkpointDepth)
#endif
						minUp = GATE_CHECKPOINT_FRONT;
					if ((g_collisionScratchPoint1X < minX && g_collisionScratchPoint2X < minX) ||
						(g_collisionScratchPoint1X > maxX && g_collisionScratchPoint2X > maxX) ||
						(g_collisionScratchPoint1Z < minForward && g_collisionScratchPoint2Z < minForward) ||
						(g_collisionScratchPoint1Z > maxForward && g_collisionScratchPoint2Z > maxForward) ||
						(g_collisionScratchPoint1Y > maxUp && g_collisionScratchPoint2Y > maxUp))
						continue;
					if (g_collisionScratchPoint1Y < minUp && g_collisionScratchPoint2Y < minUp)
						return 1;
					++g_missionRuntimeState.provingGroundsCheckpointsPassed;
					--g_missionRuntimeState.provingGroundsCheckpointsRemaining;
					previousCheckpointIndex = g_missionRuntimeState.provingGroundsCurrentCheckpointIndex;
					g_missionRuntimeState.provingGroundsScore += GATE_CHECKPOINT_SCORE;
					playCheckpointSound = 1;
					if (objectType == finishType &&
						g_missionRuntimeState.provingGroundsCurrentCheckpointIndex == -1) {
						g_missionRuntimeState.provingGroundsCurrentCheckpointIndex = 0;
						g_missionObjects[0].typeSpecificByte |= GATE_COURSE_FIRST_CHECKPOINT_FLAGS;
					} else if (objectType == finishType &&
							   g_missionRuntimeState.provingGroundsCurrentCheckpointIndex == 0) {
						--g_missionRuntimeState.provingGroundsCheckpointsPassed;
						++g_missionRuntimeState.provingGroundsCheckpointsRemaining;
						g_missionRuntimeState.provingGroundsScore -= GATE_CHECKPOINT_SCORE;
						if (g_missionRuntimeState.provingGroundsScore < 0)
							g_missionRuntimeState.provingGroundsScore = 0;
						playCheckpointSound = 0;
					} else if (selectedPair == 0) {
						uint16_t skippedGateIndex = gateIndex;
						g_missionRuntimeState.provingGroundsCurrentCheckpointIndex = gateIndex;
						do {
							uint16_t skippedCheckpoint;
							--skippedGateIndex;
							for (skippedCheckpoint = GATE_FIRST_CHECKPOINT_MESH;
								 skippedCheckpoint < GATE_CHECKPOINT_MESH_COUNT + 1; ++skippedCheckpoint) {
								uint8_t skippedFlags, currentBit;
								if (skippedGateIndex != GATE_NO_CHECKPOINT) {
									skippedFlags = g_missionObjects[skippedGateIndex].typeSpecificByte;
									if (skippedCheckpoint > GATE_FIRST_CHECKPOINT_MESH)
										skippedFlags >>= GATE_CHECKPOINT_PAIR_BITS *
														 (skippedCheckpoint - GATE_FIRST_CHECKPOINT_MESH);
									if (skippedFlags & GATE_CHECKPOINT_COMPLETE)
										continue;
								}
								if (g_missionCountdownClock.seconds < GATE_MISSED_CHECKPOINT_SECONDS) {
									g_missionCountdownClock.seconds += XW_SECONDS_PER_MINUTE;
									if (g_missionCountdownClock.minutes == 0) {
										user_checkreplaycamera();
										g_missionRuntimeState.flightExitRequested = 1;
										g_missionRuntimeState.flightExitReason = GATE_TIMEOUT_EXIT_REASON;
									} else
										--g_missionCountdownClock.minutes;
								}
								++g_missionRuntimeState.provingGroundsCheckpointsMissed;
								g_missionCountdownClock.seconds -= GATE_MISSED_CHECKPOINT_SECONDS;
								if (g_missionRuntimeState.provingGroundsCheckpointsRemaining != 0)
									--g_missionRuntimeState.provingGroundsCheckpointsRemaining;
								g_missionRuntimeState.provingGroundsScore -= GATE_CHECKPOINT_SCORE;
								if (g_missionRuntimeState.provingGroundsScore < 0)
									g_missionRuntimeState.provingGroundsScore = 0;
								if (skippedGateIndex == GATE_NO_CHECKPOINT)
									break;
								currentBit = GATE_CHECKPOINT_CURRENT;
								if (skippedCheckpoint > GATE_FIRST_CHECKPOINT_MESH)
									currentBit <<= GATE_CHECKPOINT_PAIR_BITS *
										(skippedCheckpoint - GATE_FIRST_CHECKPOINT_MESH);
								g_missionObjects[skippedGateIndex].typeSpecificByte =
									(GATE_CHECKPOINT_COMPLETE * currentBit) |
									(g_missionObjects[skippedGateIndex].typeSpecificByte & ~currentBit);
							}
						} while (skippedGateIndex > previousCheckpointIndex &&
								 skippedGateIndex != GATE_NO_CHECKPOINT);
						if (objectType != finishType)
							g_missionObjects[(uint16_t)
												 g_missionRuntimeState.provingGroundsCurrentCheckpointIndex]
								.typeSpecificByte |= GATE_COURSE_FIRST_CHECKPOINT_FLAGS;
					}
					if (playCheckpointSound)
						fsfx_triggersfx(GATE_CHECKPOINT_SOUND, FSFX_UNPOSITIONED_OBJECT);
					if (objectType == finishType) {
						if (g_missionRuntimeState.provingGroundsCurrentCheckpointIndex != -1 &&
							g_missionRuntimeState.provingGroundsCurrentCheckpointIndex != 0 &&
							g_missionRuntimeState.flightExitRequested == 0) {
#ifdef XW_MODERN
							resume.version = XwProfile_ActiveFlight()->version;
							resume.objectIndex = objectIndex;
							resume.gateIndex = gateIndex;
							resume.objectType = objectType;
							resume.checkpointNumber = checkpointNumber;
							resume.originalStartGateUp = originalStartGateUp;
							resume.gateWorldX = gateWorldX;
							resume.gateWorldY = gateWorldY;
							resume.gateWorldZ = gateWorldZ;
							if (XwGateCollision_BeginBonus(&resume))
								return XW_GATE_COLLISION_PENDING;
							if (g_quitRequested)
								return 0;
#else
							msg_messageprintf(XW_MSG_PROVING_GROUNDS_LEVEL_COMPLETED);
							g_missionRuntimeState.provingGroundsTimeBonus = 0;
							gate_updatebonuspoints();
							while (g_missionCountdownClock.minutes != 0 ||
								   g_missionCountdownClock.seconds != 0) {
								if (g_missionCountdownClock.seconds != 0)
									--g_missionCountdownClock.seconds;
								else {
									g_missionCountdownClock.seconds = XW_SECONDS_PER_MINUTE - 1;
									--g_missionCountdownClock.minutes;
								}
								g_missionRuntimeState.provingGroundsTimeBonus += GATE_BONUS_POINTS_PER_SECOND;
								g_missionRuntimeState.provingGroundsScore += GATE_BONUS_POINTS_PER_SECOND;
								if (g_missionRuntimeState.provingGroundsScore % GATE_BONUS_SOUND_INTERVAL ==
									0)
									fsfx_triggersfx(GATE_BONUS_SOUND, FSFX_UNPOSITIONED_OBJECT);
								gate_updatebonuspoints();
								while (g_flightAccumulatedTicks < GATE_BONUS_WAIT_TICKS)
									g_flightAccumulatedTicks += xtimer_Time_Elapsed();
								g_flightAccumulatedTicks = 0;
							}
							g_msgArgTable[0] = g_missionRuntimeState.provingGroundsTimeBonus;
							msg_messageprintf(XW_MSG_BONUS_POINTS_AWARDED);
							gate_LoadNextCourse();
#endif
						}
					} else {
						uint8_t completedBit = GATE_CHECKPOINT_COMPLETE;
						uint8_t checkpointShift = checkpointNumber - GATE_FIRST_CHECKPOINT_MESH;
						uint16_t nextCheckpointIndex = gateIndex;
						if (checkpointShift != 0)
							completedBit <<= GATE_CHECKPOINT_PAIR_BITS * checkpointShift;
						g_missionObjects[gateIndex].typeSpecificByte = completedBit | savedFlags;
						g_missionRuntimeState.provingGroundsCurrentCheckpointIndex = gateIndex;
						if (((completedBit | savedFlags) & GATE_CHECKPOINT_ALL_COMPLETE) ==
							GATE_CHECKPOINT_ALL_COMPLETE) {
							do {
								++nextCheckpointIndex;
							} while ((g_missionObjects[nextCheckpointIndex].typeSpecificByte &
									  GATE_CHECKPOINT_ALL_COMPLETE) == GATE_CHECKPOINT_ALL_COMPLETE);
							g_missionRuntimeState.provingGroundsCurrentCheckpointIndex = nextCheckpointIndex;
						}
						g_missionObjects[nextCheckpointIndex].typeSpecificByte |=
							GATE_COURSE_FIRST_CHECKPOINT_FLAGS;
					}
				}
			}
			if (!gate_TestGatePlaneCollision(objectType))
				continue;
			g_collisionScratchPoint1Z = g_gateIntersectionZ;
			g_collisionScratchPoint1X = g_gateIntersectionX;
			impactObjectType = XW_OBJ_ASTEROID_IMPACT;
			g_collisionScratchPoint1Y =
				originalStartGateUp < 0 ? -GATE_IMPACT_PLANE_OFFSET : GATE_IMPACT_PLANE_OFFSET;
		}
		if (objectIndex == g_playerFlightState.objectIndex)
			return 1;
		{
			int impactRelativeX, impactRelativeY, impactRelativeZ;
			XwObjectTypeId originalObjectType;
#ifdef XW_MODERN
			impactRelativeX =
				(int32_t)((uint32_t)g_collisionScratchPoint1Z * (uint32_t)g_fviewForwardX_Q15 +
						  (uint32_t)g_collisionScratchPoint1Y * (uint32_t)g_fviewUpX_Q15 +
						  (uint32_t)g_collisionScratchPoint1X * (uint32_t)g_fviewSideX_Q15);
#else
			impactRelativeX = g_collisionScratchPoint1Z * g_fviewForwardX_Q15 +
									  g_collisionScratchPoint1Y * g_fviewUpX_Q15 +
									  g_collisionScratchPoint1X * g_fviewSideX_Q15;
#endif
#ifdef XW_MODERN
			if (XwFlightTypes_Dos())
				impactRelativeX = Dos94World_ClampDot(impactRelativeX);
			else
#endif
			{
				if (impactRelativeX >= FVIEW_DOT_CLAMP_LIMIT)
					impactRelativeX = FVIEW_DOT_CLAMP_MAX;
				if (impactRelativeX <= -FVIEW_DOT_CLAMP_LIMIT)
					impactRelativeX = FVIEW_DOT_CLAMP_MIN;
			}
			impactRelativeX >>= FVIEW_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
			impactRelativeY =
				(int32_t)((uint32_t)g_collisionScratchPoint1Z * (uint32_t)g_fviewForwardY_Q15 +
						  (uint32_t)g_collisionScratchPoint1Y * (uint32_t)g_fviewUpY_Q15 +
						  (uint32_t)g_collisionScratchPoint1X * (uint32_t)g_fviewSideY_Q15);
#else
			impactRelativeY = g_collisionScratchPoint1Z * g_fviewForwardY_Q15 +
									  g_collisionScratchPoint1Y * g_fviewUpY_Q15 +
									  g_collisionScratchPoint1X * g_fviewSideY_Q15;
#endif
#ifdef XW_MODERN
			if (XwFlightTypes_Dos())
				impactRelativeY = Dos94World_ClampDot(impactRelativeY);
			else
#endif
			{
				if (impactRelativeY >= FVIEW_DOT_CLAMP_LIMIT)
					impactRelativeY = FVIEW_DOT_CLAMP_MAX;
				if (impactRelativeY <= -FVIEW_DOT_CLAMP_LIMIT)
					impactRelativeY = FVIEW_DOT_CLAMP_MIN;
			}
			impactRelativeY >>= FVIEW_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
			impactRelativeZ =
				(int32_t)((uint32_t)g_collisionScratchPoint1Z * (uint32_t)g_fviewForwardZ_Q15 +
						  (uint32_t)g_collisionScratchPoint1Y * (uint32_t)g_fviewUpZ_Q15 +
						  (uint32_t)g_collisionScratchPoint1X * (uint32_t)g_fviewSideZ_Q15);
#else
			impactRelativeZ = g_collisionScratchPoint1Z * g_fviewForwardZ_Q15 +
									  g_collisionScratchPoint1Y * g_fviewUpZ_Q15 +
									  g_collisionScratchPoint1X * g_fviewSideZ_Q15;
#endif
#ifdef XW_MODERN
			if (XwFlightTypes_Dos())
				impactRelativeZ = Dos94World_ClampDot(impactRelativeZ);
			else
#endif
			{
				if (impactRelativeZ >= FVIEW_DOT_CLAMP_LIMIT)
					impactRelativeZ = FVIEW_DOT_CLAMP_MAX;
				if (impactRelativeZ <= -FVIEW_DOT_CLAMP_LIMIT)
					impactRelativeZ = FVIEW_DOT_CLAMP_MIN;
			}
			g_collisionScratchPoint1X = (int16_t)impactRelativeX;
			g_collisionScratchPoint1Y = (int16_t)impactRelativeY;
			g_collisionScratchPoint1Z = (int16_t)(impactRelativeZ >> FVIEW_MATRIX_FRACTION_BITS);
#ifdef XW_MODERN
			g_objectTable[objectIndex].worldX =
				(int32_t)((uint32_t)gateWorldX + (uint32_t)(int16_t)impactRelativeX);
#else
			g_objectTable[objectIndex].worldX = gateWorldX + (int16_t)impactRelativeX;
#endif
#ifdef XW_MODERN
			g_objectTable[objectIndex].worldY =
				(int32_t)((uint32_t)gateWorldY + (uint32_t)(int16_t)impactRelativeY);
#else
			g_objectTable[objectIndex].worldY = gateWorldY + (int16_t)impactRelativeY;
#endif
#ifdef XW_MODERN
			g_objectTable[objectIndex].worldZ =
				(int32_t)((uint32_t)gateWorldZ +
						  (uint32_t)(int16_t)(impactRelativeZ >> FVIEW_MATRIX_FRACTION_BITS));
#else
			g_objectTable[objectIndex].worldZ =
				gateWorldZ + (int16_t)(impactRelativeZ >> FVIEW_MATRIX_FRACTION_BITS);
#endif
#ifdef XW_MODERN
			originalObjectType = XwFlightTypes_CanonicalType(g_objectTable[objectIndex].objectType);
#else
			originalObjectType = g_objectTable[objectIndex].objectType;
#endif
			if (originalObjectType == XW_OBJ_WARHEAD_149 || originalObjectType == XW_OBJ_TRACKED_WARHEAD)
				impactObjectType = (math2_getrandom() & 1) + XW_OBJ_EXPLOSION_133;
#ifdef XW_MODERN
			g_objectTable[objectIndex].objectType = XwFlightTypes_ObjectType(impactObjectType);
			if (XwFlightTypes_Dos())
				g_objectTable[objectIndex].instanceData = NULL;
#else
			g_objectTable[objectIndex].objectType = impactObjectType;
#endif
#ifdef XW_MODERN
			XwFlightIntegration_Reset(objectIndex);
			XwRenderObjects_ReplaceMobile(objectIndex);
#endif
			g_objectTable[objectIndex].genusId = XW_GENUS_EXPLOSION_EFFECT;
			g_objectTable[objectIndex].familyId = XW_OBJECT_FAMILY_5;
			g_objectTable[objectIndex].animationState = XW_OBJECT_ANIMATION_BREAKUP;
			g_objectTable[objectIndex].speed = 0;
			if (impactObjectType == XW_OBJ_ASTEROID_IMPACT)
				g_objectTable[objectIndex].billboardScaleCode = 0;
			else
				g_objectTable[objectIndex].billboardScaleCode = GATE_IMPACT_LARGE_SCALE;
			g_objectTable[objectIndex].ageSeconds = 0;
			g_objectTable[objectIndex].lifetimeTicks = 0;
			g_objectTable[objectIndex].pitch = 0;
			g_objectTable[objectIndex].yaw = 0;
			g_objectTable[objectIndex].roll = 0;
			g_objectTable[objectIndex].orientMatrixDirty = 1;
			g_objectTable[objectIndex].moveVectorDirty = 1;
			if (impactObjectType == XW_OBJ_ASTEROID_IMPACT)
				fsfx_triggersfx(FSFX_ASTEROID_IMPACT_SLOT, objectIndex);
			else
				fsfx_triggersfx(FSFX_OBJECT_EXPLOSION_SLOT, objectIndex);
		}
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x40F4D0
int16_t gate_TestGatePlaneCollision(uint16_t objectType) {
	int absoluteX;
	int absoluteZ;
#ifdef XW_MODERN
	if (XwFlightTypes_Dos())
		return Dos94_gate_TestGatePlaneCollision(objectType);
#endif
	if ((int16_t)(g_collisionScratchPoint1Y ^ g_collisionScratchPoint2Y) >= 0)
		return 0;
	if (g_collisionScratchPoint1Y == 0) {
		g_gateIntersectionX = g_collisionScratchPoint1X;
		g_gateIntersectionZ = g_collisionScratchPoint1Z;
	} else if (g_collisionScratchPoint2Y == 0) {
		g_gateIntersectionX = g_collisionScratchPoint2X;
		g_gateIntersectionZ = g_collisionScratchPoint2Z;
	} else {
		int16_t fraction;
		if (g_collisionScratchPoint1Y < 0) {
			int16_t coordinate = (int16_t)g_collisionScratchPoint2Y;
			g_collisionScratchPoint2Y = g_collisionScratchPoint1Y;
			g_collisionScratchPoint1Y = coordinate;
			coordinate = (int16_t)g_collisionScratchPoint1X;
			g_collisionScratchPoint1X = g_collisionScratchPoint2X;
			g_collisionScratchPoint2X = coordinate;
			coordinate = (int16_t)g_collisionScratchPoint1Z;
			g_collisionScratchPoint1Z = g_collisionScratchPoint2Z;
			g_collisionScratchPoint2Z = coordinate;
		}
#ifdef XW_MODERN
		fraction = (int16_t)(((uint32_t)g_collisionScratchPoint1Y << GATE_FRACTION_DIVIDEND_SHIFT) /
							 (uint32_t)(int16_t)((uint32_t)g_collisionScratchPoint1Y -
												 (uint32_t)g_collisionScratchPoint2Y));
#else
		fraction = (int16_t)(((uint32_t)g_collisionScratchPoint1Y << GATE_FRACTION_DIVIDEND_SHIFT) /
							 (uint32_t)(int16_t)(g_collisionScratchPoint1Y - g_collisionScratchPoint2Y));
#endif
		fraction >>= GATE_FRACTION_SCALE_SHIFT;
#ifdef XW_MODERN
		g_gateIntersectionX =
			(int32_t)((uint32_t)g_collisionScratchPoint1X +
					  (uint32_t)((int32_t)((uint32_t)fraction * ((uint32_t)g_collisionScratchPoint2X -
																 (uint32_t)g_collisionScratchPoint1X)) >>
								 GATE_INTERPOLATION_SHIFT));
		g_gateIntersectionZ =
			(int32_t)((uint32_t)g_collisionScratchPoint1Z +
					  (uint32_t)((int32_t)((uint32_t)fraction * ((uint32_t)g_collisionScratchPoint2Z -
																 (uint32_t)g_collisionScratchPoint1Z)) >>
								 GATE_INTERPOLATION_SHIFT));
#else
		g_gateIntersectionX = g_collisionScratchPoint1X +
							  ((fraction * (g_collisionScratchPoint2X - g_collisionScratchPoint1X)) >>
							   GATE_INTERPOLATION_SHIFT);
		g_gateIntersectionZ = g_collisionScratchPoint1Z +
							  ((fraction * (g_collisionScratchPoint2Z - g_collisionScratchPoint1Z)) >>
							   GATE_INTERPOLATION_SHIFT);
#endif
	}
	absoluteX = g_gateIntersectionX;
	absoluteZ = g_gateIntersectionZ;
	if ((int16_t)absoluteX < 0) {
#ifdef XW_MODERN
		absoluteX = (int32_t)(0u - (uint32_t)absoluteX);
#else
		absoluteX = -absoluteX;
#endif
	}
	if ((int16_t)absoluteZ < 0) {
#ifdef XW_MODERN
		absoluteZ = (int32_t)(0u - (uint32_t)absoluteZ);
#else
		absoluteZ = -absoluteZ;
#endif
	}
	if (objectType == GATE_OBJECT_TYPE_202) {
		if ((int16_t)absoluteX >= GATE_TYPE_202_HALF_WIDTH || (int16_t)absoluteZ >= GATE_TYPE_202_HALF_HEIGHT)
			return 0;
	} else {
		if ((int16_t)absoluteX > GATE_HALF_WIDTH)
			return 0;
		if ((int16_t)absoluteZ > GATE_HALF_HEIGHT)
			return 0;
		if ((int16_t)absoluteZ > GATE_BEVEL_START_HEIGHT) {
#ifdef XW_MODERN
			if ((int16_t)absoluteX >
				(int16_t)(GATE_BEVEL_SLOPE * ((uint32_t)GATE_HALF_HEIGHT - (uint32_t)g_gateIntersectionZ)))
#else
			if ((int16_t)absoluteX > (int16_t)(GATE_BEVEL_SLOPE * (GATE_HALF_HEIGHT - g_gateIntersectionZ)))
#endif
				return 0;
		}
	}
	return 1;
}

// FUNCTION: XW 0x40F660
void gate_updategateguns(void) {
#ifdef XW_MODERN
	const uint16_t finishType = XwFlightTypes_ObjectType(GATE_OBJECT_TYPE_202);
#else
	const uint16_t finishType = GATE_OBJECT_TYPE_202;
#endif
	enum { GATE_GUN_PROJECTILE_DATA_INDEX = XW_OBJ_LASER_145 - LASER_PROJECTILE_FIRST_TYPE };

	int worldX, worldY, worldZ;
#ifndef XW_MODERN
	int prevWorldX, prevWorldY, prevWorldZ;
#endif
	uint16_t speed;
	int objectsRemaining;
	if (g_gateGunTimer > g_elapsedTicks) {
		g_gateGunTimer -= g_elapsedTicks;
		return;
	}
	g_gateGunTimer = GATE_GUN_PERIOD;
	worldX = g_playerFlightState.object->worldX;
	worldY = g_playerFlightState.object->worldY;
	worldZ = g_playerFlightState.object->worldZ;
#ifndef XW_MODERN
	prevWorldX = g_playerFlightState.object->prevWorldX;
	prevWorldY = g_playerFlightState.object->prevWorldY;
	prevWorldZ = g_playerFlightState.object->prevWorldZ;
#endif
	speed = g_playerFlightState.object->speed;
	for (objectsRemaining = MISSION_OBJECT_COUNT; objectsRemaining != 0; --objectsRemaining) {
		XwMissionObjectRecord* missionObject = &g_missionObjects[MISSION_OBJECT_COUNT - objectsRemaining];
		uint16_t gateObjectType = missionObject->objectType;
		uint8_t gunFlags;
		int gateWorldX, gateWorldY, gateWorldZ;
		int deltaX, deltaY, deltaZ;
		int16_t gateRoll;
		int meshIndex;
		uint8_t gunBit;
		if (gateObjectType == XW_OBJ_NONE || gateObjectType == finishType ||
			missionObject->genusId != XW_GENUS_SCENERY)
			continue;
		gunFlags = missionObject->stateByte;
		if (!(gunFlags & GATE_GUN_ARMED))
			continue;
		gateWorldX = missionObject->worldX * GATE_COORDINATE_SCALE;
		gateWorldY = missionObject->worldY * GATE_COORDINATE_SCALE;
		gateWorldZ = missionObject->worldZ * GATE_COORDINATE_SCALE;
		deltaX = (int)((uint32_t)worldX - gateWorldX);
		deltaY = (int)((uint32_t)worldY - gateWorldY);
		deltaZ = (int)((uint32_t)worldZ - gateWorldZ);
		if ((unsigned int)collide_roughdistance3d(deltaX, deltaY, deltaZ) > GATE_GUN_RANGE)
			continue;
		gateRoll = (int16_t)(missionObject->rollAngle8 << GATE_ANGLE_SHIFT);
		fview_calcrotatemove((int16_t)(missionObject->pitchAngle8 << GATE_ANGLE_SHIFT),
							 (int16_t)(missionObject->yawAngle8 << GATE_ANGLE_SHIFT), NULL);
		fview_calcrotateorient(gateRoll, 0, NULL);
		g_fviewForwardY_Q15 = -g_fviewForwardY_Q15;
		g_fviewForwardX_Q15 = -g_fviewForwardX_Q15;
		g_fviewForwardZ_Q15 = -g_fviewForwardZ_Q15;
		{
			int dot = (int)((uint32_t)(int16_t)deltaX * g_fviewUpX_Q15 +
							(uint32_t)(int16_t)deltaY * g_fviewUpY_Q15 +
							(uint32_t)(int16_t)deltaZ * g_fviewUpZ_Q15);
#ifdef XW_MODERN
			if (XwFlightTypes_Dos())
				dot = Dos94World_ClampDot(dot);
			else
#endif
			{
				if (dot >= FVIEW_DOT_CLAMP_LIMIT)
					dot = FVIEW_DOT_CLAMP_MAX;
				if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
					dot = FVIEW_DOT_CLAMP_MIN;
			}
			g_collisionScratchPoint1Z = (dot >> FVIEW_MATRIX_FRACTION_BITS);
		}
		if (g_collisionScratchPoint1Z < 0)
			continue;
		{
			int dot = (int)((uint32_t)(int16_t)deltaX * g_fviewForwardX_Q15 +
							(uint32_t)(int16_t)deltaY * g_fviewForwardY_Q15 +
							(uint32_t)(int16_t)deltaZ * g_fviewForwardZ_Q15);
#ifdef XW_MODERN
			if (XwFlightTypes_Dos())
				dot = Dos94World_ClampDot(dot);
			else
#endif
			{
				if (dot >= FVIEW_DOT_CLAMP_LIMIT)
					dot = FVIEW_DOT_CLAMP_MAX;
				if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
					dot = FVIEW_DOT_CLAMP_MIN;
			}
			g_collisionScratchPoint1Y = (dot >> FVIEW_MATRIX_FRACTION_BITS);
		}
		for (meshIndex = GATE_GUN_FIRST_MESH, gunBit = 1; gunBit <= (1u << (GATE_GUN_COUNT - 1));
			 ++meshIndex, gunBit <<= 1) {
			int16_t fireRandom;
			int16_t muzzleLocalX, muzzleLocalY, muzzleLocalZ;
			int muzzleWorldX, muzzleWorldY, muzzleWorldZ;
			uint16_t leadSteps;
			int16_t aimYaw;
			int16_t aimPitch;
			uint16_t distanceAccuracy, speedAccuracy, accuracyThreshold;
			uint16_t projectileObjectIndex;
			int projectileIndex;
			uint16_t guidanceIndex;
			int launchX, launchY, launchZ;
			if (g_missionRuntimeState.provingGroundsLevel < GATE_GUN_MEDIUM_LEVEL)
				fireRandom = math2_getrandom() & GATE_GUN_LOW_RANDOM_MASK;
			else if (g_missionRuntimeState.provingGroundsLevel < GATE_GUN_HIGH_LEVEL)
				fireRandom = math2_getrandom() & GATE_GUN_MEDIUM_RANDOM_MASK;
			else
				fireRandom = math2_getrandom() & GATE_GUN_HIGH_RANDOM_MASK;
			if (fireRandom != 0 || !(gunBit & gunFlags))
				continue;
#ifdef XW_MODERN
			if (XwFlightTypes_Dos()) {
				XwBounds16 bounds;
				const Dos94MeshView* mesh = Dos94World_Bounds(gateObjectType, meshIndex, &bounds);
				if (!mesh)
					continue;
				muzzleLocalX = (int16_t)((mesh->bounds[0] >> 2) + (mesh->bounds[3] >> 2));
				muzzleLocalY = (int16_t)((mesh->bounds[1] >> 2) + (mesh->bounds[4] >> 2));
				muzzleLocalZ = (mesh->bounds[5] >> 1) + GATE_GUN_MUZZLE_OFFSET;
			} else
#endif
			{
				muzzleLocalX = (int16_t)ModelMesh_GetCenterX(gateObjectType, (uint16_t)meshIndex);
				muzzleLocalY = (int16_t)ModelMesh_GetCenterY(gateObjectType, (uint16_t)meshIndex);
				muzzleLocalZ = (int16_t)(ModelMesh_GetBoundsMaxZ(gateObjectType, (uint16_t)meshIndex) +
										 GATE_GUN_MUZZLE_OFFSET);
			}
			if (g_collisionScratchPoint1Y > muzzleLocalY)
				continue;
			{
				int dot = (int)((uint32_t)muzzleLocalX * g_fviewSideX_Q15 +
								(uint32_t)muzzleLocalY * g_fviewForwardX_Q15 +
								(uint32_t)muzzleLocalZ * g_fviewUpX_Q15);
#ifdef XW_MODERN
				if (XwFlightTypes_Dos())
					dot = Dos94World_ClampDot(dot);
				else
#endif
				{
					if (dot >= FVIEW_DOT_CLAMP_LIMIT)
						dot = FVIEW_DOT_CLAMP_MAX;
					if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
						dot = FVIEW_DOT_CLAMP_MIN;
				}
				muzzleWorldX = gateWorldX + (dot >> FVIEW_MATRIX_FRACTION_BITS);
			}
			{
				int dot = (int)((uint32_t)muzzleLocalX * g_fviewSideY_Q15 +
								(uint32_t)muzzleLocalY * g_fviewForwardY_Q15 +
								(uint32_t)muzzleLocalZ * g_fviewUpY_Q15);
#ifdef XW_MODERN
				if (XwFlightTypes_Dos())
					dot = Dos94World_ClampDot(dot);
				else
#endif
				{
					if (dot >= FVIEW_DOT_CLAMP_LIMIT)
						dot = FVIEW_DOT_CLAMP_MAX;
					if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
						dot = FVIEW_DOT_CLAMP_MIN;
				}
				muzzleWorldY = gateWorldY + (dot >> FVIEW_MATRIX_FRACTION_BITS);
			}
			{
				int dot = (int)((uint32_t)muzzleLocalX * g_fviewSideZ_Q15 +
								(uint32_t)muzzleLocalY * g_fviewForwardZ_Q15 +
								(uint32_t)muzzleLocalZ * g_fviewUpZ_Q15);
#ifdef XW_MODERN
				if (XwFlightTypes_Dos())
					dot = Dos94World_ClampDot(dot);
				else
#endif
				{
					if (dot >= FVIEW_DOT_CLAMP_LIMIT)
						dot = FVIEW_DOT_CLAMP_MAX;
					if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
						dot = FVIEW_DOT_CLAMP_MIN;
				}
				muzzleWorldZ = gateWorldZ + (dot >> FVIEW_MATRIX_FRACTION_BITS);
			}
			trig2_ctop((int)((uint32_t)worldX - muzzleWorldX), (int)((uint32_t)worldY - muzzleWorldY),
					   (int)((uint32_t)worldZ - muzzleWorldZ));
#ifdef XW_MODERN
			g_trig2PolarDistance =
				(int)((uint32_t)g_simStepScale * g_trig2PolarDistance) >> GATE_GUN_LEAD_SHIFT;
#else
			g_trig2PolarDistance = (g_simStepScale * g_trig2PolarDistance) >> GATE_GUN_LEAD_SHIFT;
#endif
			leadSteps = (uint16_t)g_trig2PolarDistance;
			leadSteps += math2_getrandom() & GATE_GUN_LEAD_RANDOM_MASK;
			--leadSteps;
#ifdef XW_MODERN
			trig2_ctop(
				(int)((uint32_t)worldX +
					  leadSteps * (uint32_t)XwReferenceMotion_Axis(g_playerFlightState.objectIndex, 0) -
					  muzzleWorldX),
				(int)((uint32_t)worldY +
					  leadSteps * (uint32_t)XwReferenceMotion_Axis(g_playerFlightState.objectIndex, 1) -
					  muzzleWorldY),
				(int)((uint32_t)worldZ +
					  leadSteps * (uint32_t)XwReferenceMotion_Axis(g_playerFlightState.objectIndex, 2) -
					  muzzleWorldZ));
#else
			trig2_ctop((int)(leadSteps * ((uint32_t)worldX - prevWorldX) - muzzleWorldX + (uint32_t)worldX),
					   (int)(leadSteps * ((uint32_t)worldY - prevWorldY) - muzzleWorldY + (uint32_t)worldY),
					   (int)(leadSteps * ((uint32_t)worldZ - prevWorldZ) - muzzleWorldZ + (uint32_t)worldZ));
#endif
			aimYaw = g_trig2Yaw;
			aimPitch = g_trig2Pitch;
			if (g_missionRuntimeState.provingGroundsLevel < GATE_GUN_ACCURACY_LEVEL)
				g_trig2PolarDistance = (int)((uint32_t)g_trig2PolarDistance * 2);
			distanceAccuracy =
				(uint16_t)~(g_trig2PolarDistance >= GATE_GUN_DISTANCE_LIMIT ? GATE_GUN_MAX_ACCURACY
																			: g_trig2PolarDistance);
			if (speed <= GATE_GUN_SLOW_SPEED)
				speedAccuracy = GATE_GUN_MAX_ACCURACY;
			else if (speed >= GATE_GUN_FAST_SPEED)
				speedAccuracy = GATE_GUN_FAST_ACCURACY;
			else
				speedAccuracy = (uint16_t)(GATE_GUN_SPEED_ACCURACY_BASE - (speed << GATE_ANGLE_SHIFT));
			accuracyThreshold = (uint16_t)math2_fraction(distanceAccuracy, speedAccuracy);
			if ((uint16_t)math2_getrandom() > accuracyThreshold) {
				int yawScatter =
					((uint16_t)math2_getrandom() - GATE_GUN_SCATTER_OFFSET) & GATE_GUN_SCATTER_MASK;
				int pitchScatter;
				if ((uint16_t)math2_getrandom() >= GATE_GUN_SCATTER_SIGN)
					yawScatter = -yawScatter;
				aimYaw = (int16_t)(aimYaw + yawScatter);
				pitchScatter =
					((uint16_t)math2_getrandom() - GATE_GUN_SCATTER_OFFSET) & GATE_GUN_SCATTER_MASK;
				if ((uint16_t)math2_getrandom() >= GATE_GUN_SCATTER_SIGN)
					aimPitch -= pitchScatter;
				else
					aimPitch += pitchScatter;
			}
			projectileObjectIndex = create_findslot(XW_GENUS_OTHER_PROJECTILE);
			if (projectileObjectIndex == XW_OBJECT_SLOT_UNAVAILABLE)
				continue;
			projectileIndex = projectileObjectIndex;
			g_objectTable[projectileIndex].familyId = LASER_PROJECTILE_FAMILY;
			g_objectTable[projectileIndex].genusId = XW_GENUS_OTHER_PROJECTILE;
#ifdef XW_MODERN
			g_objectTable[projectileIndex].objectType = XwFlightTypes_ObjectType(XW_OBJ_LASER_145);
#else
			g_objectTable[projectileIndex].objectType = XW_OBJ_LASER_145;
#endif
			g_objectTable[projectileIndex].ageSeconds = LASER_PROJECTILE_INITIAL_AGE;
			g_objectTable[projectileIndex].sourceObjectRef = GATE_GUN_SOURCE_REF;
			g_objectTable[projectileIndex].sourceObjectType = XW_OBJ_NONE;
			g_objectTable[projectileIndex].iff = GATE_GUN_IFF;
			g_objectTable[projectileIndex].pitch = (int16_t)aimPitch;
			g_objectTable[projectileIndex].roll = 0;
			g_objectTable[projectileIndex].yaw = aimYaw;
			g_objectTable[projectileIndex].orientMatrixDirty = 1;
			g_objectTable[projectileIndex].moveVectorDirty = 1;
			g_objectTable[projectileIndex].speed = g_projectileSpeedByType[GATE_GUN_PROJECTILE_DATA_INDEX];
			g_objectTable[projectileIndex].damageAmount =
				g_projectileBaseDamageByType[GATE_GUN_PROJECTILE_DATA_INDEX];
			g_objectTable[projectileIndex].lifetimeTicks =
				(int16_t)(XW_SIMULATION_TICKS_PER_SECOND *
						  (int16_t)g_projectileLifetimeSecondsByType[GATE_GUN_PROJECTILE_DATA_INDEX]);
			fview_calcrotatemove((int16_t)aimPitch, aimYaw, &g_objectTable[projectileIndex]);
			g_objectTable[projectileIndex].prevWorldX = muzzleWorldX;
			g_objectTable[projectileIndex].prevWorldY = muzzleWorldY;
			g_objectTable[projectileIndex].prevWorldZ = muzzleWorldZ;
			launchX =
				(int)((uint64_t)((int64_t)g_projectileLaunchOffsetByType[GATE_GUN_PROJECTILE_DATA_INDEX] *
								 g_craftMoveX) >>
					  LASER_LAUNCH_BASIS_SHIFT);
			launchY =
				(int)((uint64_t)((int64_t)g_projectileLaunchOffsetByType[GATE_GUN_PROJECTILE_DATA_INDEX] *
								 g_craftMoveZ) >>
					  LASER_LAUNCH_BASIS_SHIFT);
			launchZ =
				(int)((uint64_t)((int64_t)g_projectileLaunchOffsetByType[GATE_GUN_PROJECTILE_DATA_INDEX] *
								 g_craftMoveY) >>
					  LASER_LAUNCH_BASIS_SHIFT);
			g_objectTable[projectileIndex].worldX = (int)((uint32_t)muzzleWorldX + launchX);
			g_objectTable[projectileIndex].worldY = (int)((uint32_t)muzzleWorldY + launchY);
			g_objectTable[projectileIndex].worldZ = (int)((uint32_t)muzzleWorldZ + launchZ);
			fsfx_triggerlasersfx(projectileObjectIndex);
			guidanceIndex = (uint16_t)(projectileObjectIndex - XW_CRAFT_OBJECT_COUNT);
			g_objectTable[projectileIndex].instanceData = &g_warheadGuidanceTable[guidanceIndex];
			g_warheadGuidanceTable[guidanceIndex].homingTier = 0;
			g_warheadGuidanceTable[guidanceIndex].targetObjIdx = 0;
		}
	}
}

// FUNCTION: XW 0x40FD40
void gate_LoadNextCourse(void) {
#ifdef XW_MODERN
	bool complete;
	const uint16_t finishType = XwFlightTypes_ObjectType(GATE_OBJECT_TYPE_202);
#else
	const uint16_t finishType = GATE_OBJECT_TYPE_202;
#endif
	uint16_t filenameDigitIndex, gateIndex;
	XwMissionObjectDiskRecord diskObject;
#ifdef XW_MODERN
	XwMissionHeader loadedHeader;
	XwMissionFlightGroup loadedGroups[MISSION_FLIGHT_GROUP_COUNT];
	XwMissionObjectDiskRecord loadedObjects[MISSION_OBJECT_COUNT];
#endif
	for (filenameDigitIndex = 0; filenameDigitIndex < GATE_COURSE_FILENAME_SCAN_LIMIT; ++filenameDigitIndex) {
		if (g_currentMissionFile[filenameDigitIndex + 1] == '.')
			break;
	}
	if (g_currentMissionFile[filenameDigitIndex + 1] == '.') {
		++g_currentMissionFile[filenameDigitIndex];
		if (g_currentMissionFile[filenameDigitIndex] == '9')
			g_currentMissionFile[filenameDigitIndex] = '8';
	}
#ifdef XW_MODERN
	g_stream = XwStorage_OpenMission(g_currentMissionFile);
	if (!g_stream) {
		XwFlightMode_ResourceError(g_currentMissionFile, "cannot open proving-ground course");
		return;
	}
	complete = File_RawRead(&loadedHeader, sizeof loadedHeader, 1, g_stream) == 1;
	complete = complete && loadedHeader.flightGroupCount <= MISSION_FLIGHT_GROUP_COUNT &&
			   loadedHeader.objectRecordCount <= MISSION_OBJECT_COUNT;
	if (complete)
		complete = File_RawRead(loadedGroups, sizeof loadedGroups[0], loadedHeader.flightGroupCount,
								g_stream) == loadedHeader.flightGroupCount;
	if (complete)
		complete = File_RawRead(loadedObjects, sizeof loadedObjects[0], loadedHeader.objectRecordCount,
								g_stream) == loadedHeader.objectRecordCount;
	if (File_RawClose(g_stream) != 0)
		complete = false;
	g_stream = NULL;
	if (!complete) {
		XwFlightMode_ResourceError(g_currentMissionFile, "truncated or oversized proving-ground course");
		return;
	}
	if (XwFlightTypes_Dos()) {
		char error[256];
		unsigned i;
		for (i = 0; i < loadedHeader.objectRecordCount; ++i) {
			uint16_t type;
			if (loadedObjects[i].craftType >= DOS94_MISSION_TYPE_COUNT) {
				XwFlightMode_ResourceError(g_currentMissionFile, "invalid proving-ground model type");
				return;
			}
			type = XwFlightTypes_MissionType(loadedObjects[i].craftType);
			if (type && !Dos94Models_Ensure(type, error, sizeof error)) {
				XwPort_Fail(error);
				return;
			}
		}
	}
	g_missionHeader = loadedHeader;
	memcpy(g_missionFlightGroups, loadedGroups, loadedHeader.flightGroupCount * sizeof loadedGroups[0]);
#else
	if (fediskio_tryopenfile(g_currentMissionFile, "rb", 1) == 0)
		return;
	File_RawRead(&g_missionHeader, sizeof(g_missionHeader), 1, g_stream);
	fediskio_ReadFileBlockBuffered(g_missionFlightGroups, sizeof(g_missionFlightGroups[0]),
								   g_missionHeader.flightGroupCount, g_stream);
#endif
	for (gateIndex = 0; gateIndex < g_missionHeader.objectRecordCount; ++gateIndex) {
		uint16_t objectType;
#ifdef XW_MODERN
		diskObject = loadedObjects[gateIndex];
#else
		File_RawRead(&diskObject, sizeof(diskObject), 1, g_stream);
#endif
		g_missionRuntimeState.provingGroundsActive = 1;
#ifdef XW_MODERN
		objectType = XwFlightTypes_MissionType(diskObject.craftType);
#else
		objectType = g_craftTypeToObjectType[diskObject.craftType];
#endif
#ifdef XW_MODERN
		XwRenderObjects_ReplaceMission(gateIndex);
#endif
		g_missionObjects[gateIndex].objectType = objectType;
		g_missionObjects[gateIndex].yawAngle8 = diskObject.yawAngle8;
		g_missionObjects[gateIndex].pitchAngle8 = diskObject.pitchAngle8;
		g_missionObjects[gateIndex].rollAngle8 = diskObject.rollAngle8;
		g_missionObjects[gateIndex].worldX = diskObject.worldX;
		g_missionObjects[gateIndex].worldY = diskObject.worldY;
		g_missionObjects[gateIndex].worldZ = diskObject.worldZ;
		g_missionObjects[gateIndex].stateByte = 0;
		g_missionObjects[gateIndex].typeSpecificByte = 0;
		g_missionObjects[gateIndex].genusId = g_modelTypeTable[objectType].genusId;
		if (objectType != finishType) {
			g_missionObjects[gateIndex].stateByte = diskObject.stateOrSeconds;
			if (diskObject.countOrMinutes > 1)
				g_missionObjects[gateIndex].stateByte |= GATE_GUN_ARMED;
		} else {
			g_missionCountdownClock.minutes += (uint8_t)diskObject.countOrMinutes;
			g_missionCountdownClock.seconds += (uint8_t)diskObject.stateOrSeconds;
			if (g_missionCountdownClock.seconds >= XW_SECONDS_PER_MINUTE) {
				g_missionCountdownClock.seconds -= XW_SECONDS_PER_MINUTE;
				++g_missionCountdownClock.minutes;
			}
		}
		g_missionObjects[gateIndex].pitchAngle8 =
			GATE_COURSE_HALF_TURN - g_missionObjects[gateIndex].pitchAngle8;
		g_missionObjects[gateIndex].yawAngle8 += GATE_COURSE_HALF_TURN;
		g_missionObjects[gateIndex].rollAngle8 = -g_missionObjects[gateIndex].rollAngle8;
	}
#ifndef XW_MODERN
	if (fediskio_tryclosefile(0) != 0)
		return;
#endif
	g_missionRuntimeState.provingGroundsCurrentCheckpointIndex = 0;
	g_missionObjects[0].typeSpecificByte |= GATE_COURSE_FIRST_CHECKPOINT_FLAGS;
	g_missionRuntimeState.provingGroundsCheckpointsRemaining = GATE_COURSE_CHECKPOINTS_REMAINING;
	++g_missionRuntimeState.provingGroundsLevel;
	if (g_replayviewmode == 0)
		panel_initpanel();
}

// FUNCTION: XW 0x40FFB0
void gate_trainingupdatecrt(int16_t x, int16_t y) {
#ifdef XW_MODERN
	int hud_pane = XwHud_Push(XW_SNAP_PANE_COURSE);
#endif

#ifdef XW_MODERN
	if (XwFlightTypes_Dos()) {
		Dos94_gate_trainingupdatecrt(x, y);
		{
#ifdef XW_MODERN
			XwHud_Pop(hud_pane);
#endif
			return;
		}
	}
#endif
	if (g_flightScreenWidth == FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH) {
		g_provingGroundsPanelWidth = GATE_LOW_PANEL_WIDTH;
		g_provingGroundsPanelHeight = GATE_LOW_PANEL_HEIGHT;
		g_provingGroundsLevelLabelXOffset = GATE_LOW_LEVEL_LABEL_XOFFSET;
		g_provingGroundsLevelValueXOffset = GATE_LOW_LEVEL_VALUE_XOFFSET;
		g_provingGroundsLineSpacing = GATE_LOW_LINE_SPACING;
		g_provingGroundsCounterXOffset = GATE_LOW_COUNTER_XOFFSET;
		g_provingGroundsScoreXOffset = GATE_LOW_SCORE_XOFFSET;
		g_provingGroundsBonusClipRight = GATE_LOW_BONUS_CLIP_RIGHT;
		g_provingGroundsBonusClipBottom = GATE_LOW_BONUS_CLIP_BOTTOM;
		g_provingGroundsBonusClipTop = GATE_LOW_BONUS_CLIP_TOP;
		g_provingGroundsBonusValueX = GATE_LOW_BONUS_VALUE_X;
		g_provingGroundsBonusTimerX = GATE_LOW_BONUS_TIMER_X;
		g_provingGroundsBonusTextY = GATE_LOW_BONUS_TEXT_Y;
	} else {
		g_provingGroundsPanelWidth = GATE_HIGH_PANEL_WIDTH;
		g_provingGroundsPanelHeight = GATE_HIGH_PANEL_HEIGHT;
		g_provingGroundsLevelLabelXOffset = GATE_HIGH_LEVEL_LABEL_XOFFSET;
		g_provingGroundsLevelValueXOffset = GATE_HIGH_LEVEL_VALUE_XOFFSET;
		g_provingGroundsLineSpacing = GATE_HIGH_LINE_SPACING;
		g_provingGroundsCounterXOffset = GATE_HIGH_COUNTER_XOFFSET;
		g_provingGroundsScoreXOffset = GATE_HIGH_SCORE_XOFFSET;
		g_provingGroundsBonusClipRight = GATE_HIGH_BONUS_CLIP_RIGHT;
		g_provingGroundsBonusClipBottom = GATE_HIGH_BONUS_CLIP_BOTTOM;
		g_provingGroundsBonusClipTop = GATE_HIGH_BONUS_CLIP_TOP;
		g_provingGroundsBonusValueX = GATE_HIGH_BONUS_VALUE_X;
		g_provingGroundsBonusTimerX = GATE_HIGH_BONUS_TIMER_X;
		g_provingGroundsBonusTextY = GATE_HIGH_BONUS_TEXT_Y;
	}
	x += GATE_PANEL_LEFT_INSET;
	y += GATE_PANEL_TOP_INSET;
	if (g_hudFullRedrawInProgress != 0) {
		festring_setfontsize(FLIGHT_FONT_MICRO);
		festring_setbound(x, y, x + g_provingGroundsPanelWidth, y + g_provingGroundsPanelHeight);
		festring_setautofill(1);
		festring_setbackcolor(GATE_PANEL_BACKGROUND_COLOR);
		g_flightFillClipRectFn();
		festring_settextcolor(GATE_LEVEL_LABEL_COLOR);
		festring_setcursor(x + g_provingGroundsLevelLabelXOffset, y);
		festring_outstring("LEVEL:");
		festring_settextcolor(GATE_LEVEL_VALUE_COLOR);
		festring_setcursor(x + g_provingGroundsLevelValueXOffset, y);
		panelrts_outnum(g_missionRuntimeState.provingGroundsLevel, GATE_LEVEL_DIGITS, GATE_LEVEL_DIGITS);
		festring_settextcolor(GATE_CHECKPOINT_LABEL_COLOR);
		festring_setcursor(x, y + GATE_REMAINING_ROW * g_provingGroundsLineSpacing);
		festring_outstring("REMAINING:");
		festring_setcursor(x, y + GATE_MISSED_ROW * g_provingGroundsLineSpacing);
		festring_outstring("MISSED:");
		festring_setcursor(x, y + GATE_PASSED_ROW * g_provingGroundsLineSpacing);
		festring_outstring("PASSED:");
		festring_settextcolor(GATE_TARGET_LABEL_COLOR);
		festring_setcursor(x, y + GATE_TARGET_ROW * g_provingGroundsLineSpacing);
		festring_outstring("TARGETS:");
		festring_settextcolor(GATE_SCORE_LABEL_COLOR);
		festring_setcursor(x, y + GATE_SCORE_ROW * g_provingGroundsLineSpacing);
		festring_outstring("SCORE:");
	}
	festring_setfontsize(FLIGHT_FONT_MICRO);
	festring_setbound(x, y, x + g_provingGroundsPanelWidth, y + g_provingGroundsPanelHeight);
	festring_setautofill(1);
	festring_setbackcolor(GATE_PANEL_BACKGROUND_COLOR);
	festring_settextcolor(GATE_CHECKPOINT_VALUE_COLOR);
	festring_setcursor(x + g_provingGroundsCounterXOffset,
					   y + GATE_REMAINING_ROW * g_provingGroundsLineSpacing);
	panelrts_outnum(g_missionRuntimeState.provingGroundsCheckpointsRemaining, GATE_COUNTER_WIDTH, 1);
	festring_setcursor(x + g_provingGroundsCounterXOffset, y + GATE_MISSED_ROW * g_provingGroundsLineSpacing);
	panelrts_outnum(g_missionRuntimeState.provingGroundsCheckpointsMissed, GATE_COUNTER_WIDTH, 1);
	festring_setcursor(x + g_provingGroundsCounterXOffset, y + GATE_PASSED_ROW * g_provingGroundsLineSpacing);
	panelrts_outnum(g_missionRuntimeState.provingGroundsCheckpointsPassed, GATE_COUNTER_WIDTH, 1);
	festring_settextcolor(GATE_TARGET_VALUE_COLOR);
	festring_setcursor(x + g_provingGroundsCounterXOffset, y + GATE_TARGET_ROW * g_provingGroundsLineSpacing);
	panelrts_outnum(g_missionRuntimeState.provingGroundsTargetsDestroyed, GATE_COUNTER_WIDTH, 1);
	festring_settextcolor(GATE_SCORE_VALUE_COLOR);
	festring_setcursor(g_provingGroundsScoreXOffset + x, y + GATE_SCORE_ROW * g_provingGroundsLineSpacing);
	gate_outdnum(g_missionRuntimeState.provingGroundsScore, GATE_SCORE_WIDTH, 1);

#ifdef XW_MODERN
	XwHud_Pop(hud_pane);
#endif
}

// FUNCTION: XW 0x410390
void gate_outdnum(int score, uint16_t width, uint16_t minDigits) {
	int16_t sawDigit = 0;
	for (; width > 0; --width) {
		int divisor = g_provingGroundsScoreDecimalDivisors[width];
		int digit = score / divisor;
		int drawChar;
#ifdef XW_MODERN
		score = (int32_t)((uint32_t)score - (uint16_t)digit * (uint32_t)divisor);
#else
		score -= (uint16_t)digit * divisor;
#endif
		if (sawDigit != 0 || width <= minDigits || (uint16_t)digit != 0) {
			sawDigit = 1;
			if ((uint16_t)digit > GATE_DECIMAL_MAX_DIGIT) {
				digit = GATE_DECIMAL_MAX_DIGIT;
			}
			drawChar = digit + '0';
		} else {
			drawChar = ' ';
		}
		g_flightDrawCharFn((char)drawChar);
	}
}

// FUNCTION: XW 0x410410
void gate_updatebonuspoints(void) {
#ifdef XW_MODERN
	int hud_pane = XwHud_Push(XW_SNAP_PANE_COURSE);
#endif

	FlightDisplay_LockSurface();
	g_flightTextShadowEnabled = 1;
	festring_setbackcolor(GATE_BONUS_BACKGROUND_COLOR);
	festring_settextcolor(GATE_BONUS_TEXT_COLOR);
	festring_setautofill(0);
	festring_setfontsize(FLIGHT_FONT_TINY);
	festring_setbound(0, g_provingGroundsBonusClipTop, g_provingGroundsBonusClipRight,
					  g_provingGroundsBonusClipBottom);
	festring_setcursor(g_provingGroundsBonusTimerX, g_provingGroundsBonusTextY);
	panelrts_outnum(g_missionCountdownClock.minutes, GATE_BONUS_TIMER_DIGITS, GATE_BONUS_TIMER_DIGITS);
	g_flightDrawCharFn(':');
	panelrts_outnum(g_missionCountdownClock.seconds, GATE_BONUS_TIMER_DIGITS, GATE_BONUS_TIMER_DIGITS);
	festring_setcursor(g_provingGroundsBonusValueX, g_provingGroundsBonusTextY);
	panelrts_outnum(g_missionRuntimeState.provingGroundsTimeBonus, GATE_BONUS_VALUE_DIGITS,
					GATE_BONUS_VALUE_DIGITS);
	FlightDisplay_UnlockSurface();
	if (g_replayviewmode == 0) {
		FlightDisplay_LockSurface();
		panel_updatepanel();
		FlightDisplay_UnlockSurface();
	}
	FlightDisplay_BlitRenderSurface();
	FlightDisplay_Flip();

#ifdef XW_MODERN
	XwHud_Pop(hud_pane);
#endif
}
