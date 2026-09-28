#include "xw/flight/object/collide.h"

#ifdef XW_MODERN
#include "xw_runtime/timing/flight_integration.h"
#endif

#ifdef XW_MODERN
#include "xw_runtime/snapshot/render_objects.h"
#endif

#ifdef XW_MODERN
#include "xw/flight/flight.h"
#endif
#ifdef XW_MODERN
#include "xw_dos94/flight/object/effects.h"
#include "xw_runtime/runtime/flight_math.h"
#include "xw_runtime/runtime/flight_types.h"
#include "xw_runtime/storage/dos94_assets.h"
#endif

#include "xw/assets/model_mesh.h"
#include "xw/audio/fsfx.h"
#include "xw/flight/ai/pai.h"
#include "xw/flight/death_star.h"
#include "xw/flight/fediskio.h"
#include "xw/flight/fview.h"
#include "xw/flight/gate.h"
#include "xw/flight/hud/msg.h"
#include "xw/flight/hud/panel.h"
#include "xw/flight/mission/fscript.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/mission/spec.h"
#include "xw/flight/object/anim.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/laser.h"
#include "xw/flight/object/starship.h"
#include "xw/flight/object/static.h"
#include "xw/flight/player/user.h"
#include "xw/flight/replay/replay.h"
#include "xw/flight/shell_flight.h"
#include "xw/flight/xw.h"
#include "xw/math/math2.h"

#include "xw/frontend/shell_preferences.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/gate_collision_task.h"
#endif

#include <stdlib.h>

// GLOBAL: XW 0x628628
int g_collisionProbeWorldZ = 0;

// GLOBAL: XW 0x62862C
int g_collisionProbeWorldX = 0;

// GLOBAL: XW 0x628630
int g_collisionProbeWorldY = 0;

// GLOBAL: XW 0x62AFFC
int g_collisionApproxDistance = 0;

// GLOBAL: XW 0x62B900
int g_collisionHitOffsetZ = 0;

// GLOBAL: XW 0x62B918
int g_collisionSweepStartZ = 0;

// GLOBAL: XW 0x62B948
int g_collisionHitOffsetY = 0;

// GLOBAL: XW 0x62BAD4
int g_collisionHitOffsetX = 0;

// GLOBAL: XW 0x62BAD8
int g_collisionSweepStartX = 0;

// GLOBAL: XW 0x62BB1C
int g_collisionSweepStartY = 0;

// GLOBAL: XW 0x62C708
int g_collisionSegmentStartWorldX = 0;

// GLOBAL: XW 0x62D0E4
int g_collisionScratchPoint1X = 0;

// GLOBAL: XW 0x62D0E8
int g_collisionScratchPoint1Y = 0;

// GLOBAL: XW 0x62D0EC
int g_collisionScratchPoint2X = 0;

// GLOBAL: XW 0x62D0F4
int g_collisionScratchPoint1Z = 0;

// GLOBAL: XW 0x62D110
int g_collisionScratchPoint2Y = 0;

// GLOBAL: XW 0x62D114
int g_collisionScratchPoint2Z = 0;

// GLOBAL: XW 0x62D119
uint8_t g_lastShieldDamageSide = 0;

// GLOBAL: XW 0x63730C
int g_collisionSweepEndX = 0;

// GLOBAL: XW 0x63731C
int g_collisionSweepEndY = 0;

// GLOBAL: XW 0x637320
int g_collisionSweepEndZ = 0;

// GLOBAL: XW 0x63738C
int g_collisionSegmentStartWorldY = 0;

// GLOBAL: XW 0x6377B8
int g_collisionSegmentStartWorldZ = 0;

// FUNCTION: XW 0x402E10
void collide_collisions(void) {
	uint16_t collisionObjectIndex;
#ifdef XW_MODERN
	XwCollisionLoopState continuation = *XwCollisionLoop_GetState();
	if (continuation.phase != XW_COLLISION_LOOP_PROJECTILE_GATE) {
		if (continuation.phase == XW_COLLISION_LOOP_IDLE &&
#else
	{
		if (
#endif
			(g_surfaceSpecialTargetHit != 0 ||
			 (g_hyperspaceflag != 0 && g_hyperspaceAbortAndCollisionsAllowed == 0)))
			return;
#ifdef XW_MODERN
		if (continuation.phase == XW_COLLISION_LOOP_PLAYER_GATE ||
#else
		if (
#endif
			g_playerFlightState.object->genusId == XW_GENUS_STARFIGHTER) {
			ObjectRecord* object;
#ifdef XW_MODERN
			if (continuation.phase != XW_COLLISION_LOOP_PLAYER_GATE)
#endif
			{
#ifdef XW_MODERN
				g_collisionProbeWorldX = (int32_t)((uint32_t)g_playerFlightState.rotatedCockpitOffset.x +
												   (uint32_t)g_playerFlightState.object->worldX);
#else
				g_collisionProbeWorldX =
					g_playerFlightState.rotatedCockpitOffset.x + g_playerFlightState.object->worldX;
#endif
#ifdef XW_MODERN
				g_collisionProbeWorldY = (int32_t)((uint32_t)g_playerFlightState.rotatedCockpitOffset.y +
												   (uint32_t)g_playerFlightState.object->worldY);
#else
				g_collisionProbeWorldY =
					g_playerFlightState.rotatedCockpitOffset.y + g_playerFlightState.object->worldY;
#endif
#ifdef XW_MODERN
				g_collisionProbeWorldZ = (int32_t)((uint32_t)g_playerFlightState.rotatedCockpitOffset.z +
												   (uint32_t)g_playerFlightState.object->worldZ);
#else
				g_collisionProbeWorldZ =
					g_playerFlightState.rotatedCockpitOffset.z + g_playerFlightState.object->worldZ;
#endif
#ifdef XW_MODERN
				g_collisionSegmentStartWorldX =
					(int32_t)((uint32_t)g_playerFlightState.previousRotatedCockpitOffset.x +
							  (uint32_t)g_playerFlightState.object->prevWorldX);
#else
				g_collisionSegmentStartWorldX = g_playerFlightState.previousRotatedCockpitOffset.x +
												g_playerFlightState.object->prevWorldX;
#endif
#ifdef XW_MODERN
				g_collisionSegmentStartWorldY =
					(int32_t)((uint32_t)g_playerFlightState.previousRotatedCockpitOffset.y +
							  (uint32_t)g_playerFlightState.object->prevWorldY);
#else
				g_collisionSegmentStartWorldY = g_playerFlightState.previousRotatedCockpitOffset.y +
												g_playerFlightState.object->prevWorldY;
#endif
#ifdef XW_MODERN
				g_collisionSegmentStartWorldZ =
					(int32_t)((uint32_t)g_playerFlightState.previousRotatedCockpitOffset.z +
							  (uint32_t)g_playerFlightState.object->prevWorldZ);
#else
				g_collisionSegmentStartWorldZ = g_playerFlightState.previousRotatedCockpitOffset.z +
												g_playerFlightState.object->prevWorldZ;
#endif
			}
#ifdef XW_MODERN
			if (continuation.phase == XW_COLLISION_LOOP_PLAYER_GATE ||
#else
			if (
#endif
				g_missionRuntimeState.provingGroundsActive != 0) {
				int16_t gateHit;
#ifdef XW_MODERN
				gateHit = gate_ProcessCourseCollision(continuation.phase == XW_COLLISION_LOOP_PLAYER_GATE
														  ? continuation.objectIndex
														  : g_playerFlightState.objectIndex);
				if (g_quitRequested)
					return;
				if (gateHit == XW_GATE_COLLISION_PENDING) {
					if (continuation.phase == XW_COLLISION_LOOP_IDLE)
						continuation.objectIndex = g_playerFlightState.objectIndex;
					continuation.phase = XW_COLLISION_LOOP_PLAYER_GATE;
					XwCollisionLoop_Suspend(&continuation);
					return;
				}
				XwCollisionLoop_Complete();
				continuation.phase = XW_COLLISION_LOOP_IDLE;
#else
				gateHit = gate_ProcessCourseCollision(g_playerFlightState.objectIndex);
#endif
				if (gateHit != 0) {
					g_playerFlightState.object->speed = 0;
					g_playerFlightState.object->worldX = g_gatePreviousX[GATE_POSE_HISTORY_COUNT - 1];
					g_playerFlightState.object->worldY = g_gatePreviousY[GATE_POSE_HISTORY_COUNT - 1];
					g_playerFlightState.object->worldZ = g_gatePreviousZ[GATE_POSE_HISTORY_COUNT - 1];
					g_playerFlightState.object->roll = g_gatePreviousRoll[GATE_POSE_HISTORY_COUNT - 1];
					g_playerFlightState.object->pitch = g_gatePreviousPitch[GATE_POSE_HISTORY_COUNT - 1];
					g_playerFlightState.craft->pitch = g_gatePreviousPitch[GATE_POSE_HISTORY_COUNT - 1];
					g_playerFlightState.object->yaw = g_gatePreviousYaw[GATE_POSE_HISTORY_COUNT - 1];
					g_playerFlightState.object->moveVectorDirty = 1;
					g_playerFlightState.object->orientMatrixDirty = 1;
#ifdef XW_MODERN
					if (!XwFlightTypes_Dos()) {
#endif
						if (g_playerFlightState.object->moveVectorDirty != 0)
							fview_calcrotatemove(g_playerFlightState.object->pitch,
												 g_playerFlightState.object->yaw, g_playerFlightState.object);
						g_playerFlightState.object->worldX =
							(int32_t)((uint32_t)g_playerFlightState.object->worldX -
									  (uint32_t)((g_playerFlightState.object->moveX *
												  COLLIDE_GATE_RETREAT_DISTANCE) >>
												 FVIEW_MATRIX_FRACTION_BITS));
						g_playerFlightState.object->worldY =
							(int32_t)((uint32_t)g_playerFlightState.object->worldY -
									  (uint32_t)((g_playerFlightState.object->moveY *
												  COLLIDE_GATE_RETREAT_DISTANCE) >>
												 FVIEW_MATRIX_FRACTION_BITS));
						g_playerFlightState.object->worldZ =
							(int32_t)((uint32_t)g_playerFlightState.object->worldZ -
									  (uint32_t)((g_playerFlightState.object->moveZ *
												  COLLIDE_GATE_RETREAT_DISTANCE) >>
												 FVIEW_MATRIX_FRACTION_BITS));
#ifdef XW_MODERN
					}
					XwFlightIntegration_Reposition(g_playerFlightState.objectIndex);
					XwFlightIntegration_Clear(g_playerFlightState.objectIndex, XW_INTEGRATE_VELOCITY);
#endif
					fsfx_triggersfx(COLLIDE_GATE_HIT_SOUND, g_playerFlightState.objectIndex);
				}
			}
			object = g_playerFlightState.object;
			if (g_deathStarSurfaceModeActive != 0 &&
				(object->worldZ < COLLIDE_SURFACE_SCAN_HEIGHT ||
				 object->prevWorldZ < COLLIDE_SURFACE_SCAN_HEIGHT) &&
				DeathStar_TestSurfaceCollision(g_playerFlightState.objectIndex) != 0)
				collide_damagecraft(g_playerFlightState.objectIndex, 0, COLLIDE_STATIC_VICTIM, 1);
			if (g_playerFlightState.craft->workingSubsystems != 0) {
				uint16_t playerScanIndex;
				for (playerScanIndex = 0; playerScanIndex < XW_CRAFT_OBJECT_COUNT; ++playerScanIndex) {
					ObjectRecord* candidate = &g_objectTable[playerScanIndex];
					if (candidate->objectType == XW_OBJ_NONE)
						continue;
					g_collisionSweepEndX = candidate->worldX;
					g_collisionSweepEndY = candidate->worldY;
					g_collisionSweepEndZ = candidate->worldZ;
					g_collisionSweepStartX = candidate->prevWorldX;
					g_collisionSweepStartY = candidate->prevWorldY;
					g_collisionSweepStartZ = candidate->prevWorldZ;
					if (playerScanIndex == g_playerFlightState.objectIndex ||
						candidate->genusId == XW_GENUS_EXPLOSION_EFFECT ||
						((CraftData*)candidate->instanceData)->flightGroupIndex ==
							g_playerFlightState.craft->flightGroupIndex)
						continue;
					if (g_flightInvulnerabilityEnabled == 0) {
						if (g_flightCraftCollisionsEnabled != 0 || candidate->genusId == XW_GENUS_STARSHIP ||
							candidate->genusId == XW_GENUS_FREIGHTER) {
							if (collide_lasercraftcollide(g_playerFlightState.objectIndex, playerScanIndex) !=
								0) {
								int impactForwardDotQ30;
								collide_damagecraft(playerScanIndex, 0, g_playerFlightState.objectIndex, 0);
								object = g_playerFlightState.object;
								if (object->orientMatrixDirty != 0) {
									fview_calcrotatemove(object->pitch, object->yaw, object);
									fview_calcrotateorient(g_playerFlightState.object->roll, 0,
														   g_playerFlightState.object);
									object = g_playerFlightState.object;
								}
#ifdef XW_MODERN
								impactForwardDotQ30 =
									(int32_t)((uint32_t)((int16_t)((uint32_t)g_collisionSweepEndZ -
																   (uint32_t)g_collisionSweepStartZ) *
														 object->cachedForwardZ) +
											  (uint32_t)((int16_t)((uint32_t)g_collisionSweepEndY -
																   (uint32_t)g_collisionSweepStartY) *
														 object->cachedForwardY) +
											  (uint32_t)((int16_t)((uint32_t)g_collisionSweepEndX -
																   (uint32_t)g_collisionSweepStartX) *
														 object->cachedForwardX));
#else
								impactForwardDotQ30 =
									(int16_t)(g_collisionSweepEndZ - g_collisionSweepStartZ) *
										object->cachedForwardZ +
									(int16_t)(g_collisionSweepEndY - g_collisionSweepStartY) *
										object->cachedForwardY +
									(int16_t)(g_collisionSweepEndX - g_collisionSweepStartX) *
										object->cachedForwardX;
#endif
								if (impactForwardDotQ30 >= FVIEW_DOT_CLAMP_LIMIT)
									impactForwardDotQ30 = FVIEW_DOT_CLAMP_MAX;
								if (impactForwardDotQ30 <= -FVIEW_DOT_CLAMP_LIMIT)
									impactForwardDotQ30 = FVIEW_DOT_CLAMP_MIN;
								collide_damagecraft(
									g_playerFlightState.objectIndex, 0, playerScanIndex,
									(int16_t)(impactForwardDotQ30 >> FVIEW_MATRIX_FRACTION_BITS) >= 0);
							}
						}
					} else {
#ifdef XW_MODERN
						g_collisionApproxDistance = collide_roughdistance3d(
							(int32_t)((uint32_t)g_collisionProbeWorldX - (uint32_t)candidate->worldX),
							(int32_t)((uint32_t)g_collisionProbeWorldY - (uint32_t)candidate->worldY),
							(int32_t)((uint32_t)g_collisionProbeWorldZ - (uint32_t)candidate->worldZ));
#else
						g_collisionApproxDistance =
							collide_roughdistance3d(g_collisionProbeWorldX - candidate->worldX,
													g_collisionProbeWorldY - candidate->worldY,
													g_collisionProbeWorldZ - candidate->worldZ);
#endif
					}
					if (playerScanIndex == g_playerFlightState.currentTargetObjectIdx) {
						CraftData* inspectionCraft = candidate->instanceData;
						g_curCraft = inspectionCraft;
						if (inspectionCraft->isInspected == 0) {
							uint16_t maxBoundsExtent =
								g_modelTypeTable[candidate->objectType].maxBoundsExtent;
							int inspectionDistance;
							if (maxBoundsExtent > COLLIDE_INSPECTION_LARGE_EXTENT)
								maxBoundsExtent >>= 1;
#ifdef XW_MODERN
							if (XwFlightTypes_CanonicalType(candidate->objectType) ==
									XW_OBJ_IMPERIAL_STAR_DESTROYER ||
								XwFlightTypes_CanonicalType(candidate->objectType) == XW_OBJ_CALAMARI_CRUISER)
#else
							if (candidate->objectType == XW_OBJ_IMPERIAL_STAR_DESTROYER ||
								candidate->objectType == XW_OBJ_CALAMARI_CRUISER)
#endif
								inspectionDistance = COLLIDE_INSPECTION_CAPITAL_SCALE * maxBoundsExtent;
							else
								inspectionDistance = COLLIDE_INSPECTION_NORMAL_SCALE * maxBoundsExtent;
							if (g_playerFlightState.craft->craftTypeIndex ==
								COLLIDE_LONG_INSPECTION_CRAFT_TYPE)
								inspectionDistance *= COLLIDE_INSPECTION_NORMAL_SCALE;
							if (g_collisionApproxDistance < inspectionDistance) {
								int category, flightGroupIndex;
								inspectionCraft->isInspected = 1;
								category = spec_getstatisticscategory(candidate->objectType);
								++g_missionRuntimeState.inspectedCountsByCategory[category];
								inspectionCraft = g_curCraft;
								g_hudCachedTargetObjectIdx = PANEL_TARGET_CACHE_INVALIDATED;
								flightGroupIndex = g_curCraft->flightGroupIndex;
								++g_missionFlightGroupStates[flightGroupIndex].inspectedCount;
								if (g_missionFlightGroups[flightGroupIndex].specialCraftIndex ==
									inspectionCraft->craftIndexInFlightGroup)
									g_missionFlightGroupStates[flightGroupIndex].specialCraftInspected = 1;
								msg_craftmessage(playerScanIndex, inspectionCraft, XW_MSG_CRAFT_IDENTIFIED);
							}
						}
					}
				}
			}
			if (g_missionRuntimeState.provingGroundsActive == 0 && g_flightCraftCollisionsEnabled != 0 &&
				g_flightInvulnerabilityEnabled == 0) {
				uint16_t staticObjectIndex;
				for (staticObjectIndex = 0; staticObjectIndex < MISSION_OBJECT_COUNT; ++staticObjectIndex) {
					if (g_missionObjects[staticObjectIndex].objectType != XW_OBJ_NONE &&
						static_laserstaticcollide(g_playerFlightState.objectIndex, staticObjectIndex) != 0)
						collide_damagecraft(g_playerFlightState.objectIndex, 0,
											staticObjectIndex + XW_MISSION_OBJECT_REF_BASE, 0);
				}
			}
		}
	}
#ifdef XW_MODERN
	for (collisionObjectIndex =
			 continuation.phase == XW_COLLISION_LOOP_PROJECTILE_GATE ? continuation.objectIndex : 0;
		 collisionObjectIndex < XW_OBJECT_COUNT; ++collisionObjectIndex) {
#else
	for (collisionObjectIndex = 0; collisionObjectIndex < XW_OBJECT_COUNT; ++collisionObjectIndex) {
#endif
		ObjectRecord* sourceObject = &g_objectTable[collisionObjectIndex];
		int16_t objectGenus, projectileIff, projectileSourceIndex;
		uint16_t projectileTargetIndex, projectileTargetLimit;
		int16_t projectileHit = 0;
#ifdef XW_MODERN
		if (continuation.phase == XW_COLLISION_LOOP_PROJECTILE_GATE) {
			objectGenus = continuation.objectGenus;
			projectileIff = continuation.projectileIff;
			projectileSourceIndex = continuation.projectileSourceIndex;
		} else
#endif
		{
			if (sourceObject->objectType == XW_OBJ_NONE)
				continue;
			objectGenus = sourceObject->genusId;
			projectileIff = sourceObject->iff;
			projectileSourceIndex = sourceObject->sourceObjectRef;
		}
		switch (objectGenus) {
			case XW_GENUS_FREIGHTER:
			case XW_GENUS_STARSHIP: {
				uint16_t craftScanIndex;
				g_collisionSweepEndX = sourceObject->worldX;
				g_collisionSweepEndY = sourceObject->worldY;
				g_collisionSweepEndZ = sourceObject->worldZ;
				g_collisionSweepStartX = sourceObject->prevWorldX;
				g_collisionSweepStartY = sourceObject->prevWorldY;
				g_collisionSweepStartZ = sourceObject->prevWorldZ;
				for (craftScanIndex = 0; craftScanIndex < XW_CRAFT_OBJECT_COUNT; ++craftScanIndex) {
					ObjectRecord* candidate = &g_objectTable[craftScanIndex];
					CraftData* candidateCraft;
					int16_t hitMeshIndex;
					if (craftScanIndex == collisionObjectIndex ||
						craftScanIndex == g_playerFlightState.objectIndex ||
						candidate->objectType == XW_OBJ_NONE ||
						candidate->genusId == XW_GENUS_EXPLOSION_EFFECT)
						continue;
					candidateCraft = candidate->instanceData;
					if (candidateCraft->aiCurrentPlanId == PAI_PLAN_OUT_OF_HANGAR ||
						candidateCraft->aiCurrentPlanId == PAI_PLAN_OUT_OF_HYPERSPACE ||
						candidateCraft->aiCurrentPlanId == PAI_PLAN_55)
						continue;
					if (candidateCraft->aiCurrentPlanId == COLLIDE_FOLLOWER_PLAN) {
						CraftData* leader = g_objectTable[candidateCraft->aiLeaderObjectIndex].instanceData;
						if (leader->aiCurrentPlanId == PAI_PLAN_OUT_OF_HANGAR ||
							leader->aiCurrentPlanId == PAI_PLAN_55)
							continue;
					}
					if (candidateCraft->aiManeuverId == COLLIDE_EXEMPT_MANEUVER_18 ||
						candidateCraft->aiManeuverId == COLLIDE_EXEMPT_MANEUVER_21)
						continue;
					g_collisionProbeWorldX = candidate->worldX;
					g_collisionProbeWorldY = candidate->worldY;
					g_collisionProbeWorldZ = candidate->worldZ;
					g_collisionSegmentStartWorldX = candidate->prevWorldX;
					g_collisionSegmentStartWorldY = candidate->prevWorldY;
					g_collisionSegmentStartWorldZ = candidate->prevWorldZ;
					hitMeshIndex = collide_lasercraftcollide(craftScanIndex, collisionObjectIndex);
					if (hitMeshIndex != 0) {
						collide_damagecraft(collisionObjectIndex, hitMeshIndex, craftScanIndex, 0);
						collide_damagecraft(craftScanIndex, 0, collisionObjectIndex, 0);
					}
				}
				continue;
			}
			case XW_GENUS_DEBRIS:
				g_collisionProbeWorldX = sourceObject->worldX;
				g_collisionProbeWorldY = sourceObject->worldY;
				g_collisionProbeWorldZ = sourceObject->worldZ;
				g_collisionSegmentStartWorldX = sourceObject->prevWorldX;
				g_collisionSegmentStartWorldY = sourceObject->prevWorldY;
				g_collisionSegmentStartWorldZ = sourceObject->prevWorldZ;
				if (g_deathStarSurfaceModeActive != 0 &&
					(g_collisionProbeWorldZ < COLLIDE_SURFACE_SCAN_HEIGHT ||
					 g_collisionSegmentStartWorldZ < COLLIDE_SURFACE_SCAN_HEIGHT))
					DeathStar_TestSurfaceCollision(collisionObjectIndex);
				continue;
			case XW_GENUS_PLAYER_PROJECTILE:
			case XW_GENUS_OTHER_PROJECTILE:
#ifdef XW_MODERN
				if (continuation.phase != XW_COLLISION_LOOP_PROJECTILE_GATE)
#endif
				{
					g_collisionProbeWorldX = sourceObject->worldX;
					g_collisionProbeWorldY = sourceObject->worldY;
					g_collisionProbeWorldZ = sourceObject->worldZ;
					g_collisionSegmentStartWorldX = sourceObject->prevWorldX;
					g_collisionSegmentStartWorldY = sourceObject->prevWorldY;
					g_collisionSegmentStartWorldZ = sourceObject->prevWorldZ;
					if (g_deathStarSurfaceModeActive != 0 &&
						(g_collisionProbeWorldZ < COLLIDE_SURFACE_SCAN_HEIGHT ||
						 g_collisionSegmentStartWorldZ < COLLIDE_SURFACE_SCAN_HEIGHT) &&
						DeathStar_TestSurfaceCollision(collisionObjectIndex) != 0)
						continue;
				}
#ifdef XW_MODERN
				if (continuation.phase == XW_COLLISION_LOOP_PROJECTILE_GATE ||
#else
				if (
#endif
					g_missionRuntimeState.provingGroundsActive != 0) {
					int16_t gateHit = gate_ProcessCourseCollision(collisionObjectIndex);
#ifdef XW_MODERN
					if (g_quitRequested)
						return;
					if (gateHit == XW_GATE_COLLISION_PENDING) {
						continuation.phase = XW_COLLISION_LOOP_PROJECTILE_GATE;
						continuation.objectIndex = collisionObjectIndex;
						continuation.objectGenus = objectGenus;
						continuation.projectileIff = projectileIff;
						continuation.projectileSourceIndex = projectileSourceIndex;
						XwCollisionLoop_Suspend(&continuation);
						return;
					}
					XwCollisionLoop_Complete();
					continuation.phase = XW_COLLISION_LOOP_IDLE;
#endif
					if (gateHit != 0)
						continue;
				}
				projectileTargetLimit = objectGenus == XW_GENUS_PLAYER_PROJECTILE
											? COLLIDE_PLAYER_PROJECTILE_TARGET_LIMIT
											: XW_CRAFT_OBJECT_COUNT;
				break;
			default:
				continue;
		}
		for (projectileTargetIndex = 0; projectileTargetIndex < projectileTargetLimit;
			 ++projectileTargetIndex) {
			ObjectRecord* target = &g_objectTable[projectileTargetIndex];
			int16_t hitMeshIndex;
			if (target->objectType == XW_OBJ_NONE || projectileTargetIndex == projectileSourceIndex ||
				target->genusId == XW_GENUS_EXPLOSION_EFFECT)
				continue;
			if (projectileTargetIndex >= XW_CRAFT_OBJECT_COUNT &&
				(projectileTargetIndex == collisionObjectIndex ||
#ifdef XW_MODERN
				 !XwFlightTypes_IsWarhead(target->objectType) ||
#else
				 (target->objectType != XW_OBJ_TRACKED_WARHEAD && target->objectType != XW_OBJ_WARHEAD_149) ||
#endif
				 target->sourceObjectRef == (uint16_t)projectileSourceIndex))
				continue;
			if (projectileTargetIndex == g_playerFlightState.objectIndex) {
				if (g_flightInvulnerabilityEnabled != 0)
					continue;
			} else if (projectileIff == target->iff &&
					   g_playerFlightState.objectIndex != projectileSourceIndex) {
				continue;
			}
			g_collisionSweepEndX = target->worldX;
			g_collisionSweepEndY = target->worldY;
			g_collisionSweepEndZ = target->worldZ;
			g_collisionSweepStartX = target->prevWorldX;
			g_collisionSweepStartY = target->prevWorldY;
			g_collisionSweepStartZ = target->prevWorldZ;
			hitMeshIndex = collide_lasercraftcollide(collisionObjectIndex, projectileTargetIndex);
			if (hitMeshIndex == 0)
				continue;
			collide_updatehits(collisionObjectIndex, 1);
			if (projectileTargetIndex < XW_CRAFT_OBJECT_COUNT)
				collide_laserhitcraft(collisionObjectIndex, projectileTargetIndex, hitMeshIndex);
			else
				collide_makeobjectexplosion(
					projectileTargetIndex,
#ifdef XW_MODERN
					XwFlightTypes_ObjectType((math2_getrandom() & 1) + XW_OBJ_EXPLOSION_133));
#else
					(math2_getrandom() & 1) + XW_OBJ_EXPLOSION_133);
#endif
			projectileHit = 1;
			break;
		}
		if (projectileHit == 0 && g_missionRuntimeState.provingGroundsActive == 0) {
			uint16_t staticObjectIndex;
			for (staticObjectIndex = 0; staticObjectIndex < MISSION_OBJECT_COUNT; ++staticObjectIndex) {
				if (g_missionObjects[staticObjectIndex].objectType != XW_OBJ_NONE &&
					static_laserstaticcollide(collisionObjectIndex, staticObjectIndex) != 0) {
					static_laserhitstatic(collisionObjectIndex, staticObjectIndex);
					collide_updatehits(collisionObjectIndex, 1);
					break;
				}
			}
		}
	}
}

// FUNCTION: XW 0x403820
int16_t collide_lasercraftcollide(uint16_t sourceObjIdx, uint16_t targetObjIdx) {
	int endDistanceX, endDistanceY, endDistanceZ;
	int segmentDistanceX, segmentDistanceY, segmentDistanceZ;
	int sweepDistance;
	int approxDistance;
	uint8_t objectType;
	int maxBoundsExtent;
	g_collisionApproxDistance = COLLIDE_CRAFT_MAX_DISTANCE;
#ifdef XW_MODERN
	endDistanceX = (int32_t)((uint32_t)g_collisionProbeWorldX - (uint32_t)g_collisionSweepEndX);
	if (endDistanceX < 0)
		endDistanceX = (int32_t)(0u - (uint32_t)endDistanceX);
#else
	endDistanceX = g_collisionProbeWorldX - g_collisionSweepEndX;
	if (endDistanceX < 0)
		endDistanceX = -endDistanceX;
#endif
	if (endDistanceX > COLLIDE_CRAFT_MAX_DISTANCE)
		return 0;
#ifdef XW_MODERN
	endDistanceY = (int32_t)((uint32_t)g_collisionProbeWorldY - (uint32_t)g_collisionSweepEndY);
	if (endDistanceY < 0)
		endDistanceY = (int32_t)(0u - (uint32_t)endDistanceY);
#else
	endDistanceY = g_collisionProbeWorldY - g_collisionSweepEndY;
	if (endDistanceY < 0)
		endDistanceY = -endDistanceY;
#endif
	if (endDistanceY > COLLIDE_CRAFT_MAX_DISTANCE)
		return 0;
#ifdef XW_MODERN
	endDistanceZ = (int32_t)((uint32_t)g_collisionProbeWorldZ - (uint32_t)g_collisionSweepEndZ);
	if (endDistanceZ < 0)
		endDistanceZ = (int32_t)(0u - (uint32_t)endDistanceZ);
#else
	endDistanceZ = g_collisionProbeWorldZ - g_collisionSweepEndZ;
	if (endDistanceZ < 0)
		endDistanceZ = -endDistanceZ;
#endif
	if (endDistanceZ > COLLIDE_CRAFT_MAX_DISTANCE)
		return 0;
	approxDistance = (int)collide_roughdistance3du(endDistanceX, endDistanceY, endDistanceZ);
	g_collisionApproxDistance = approxDistance;
	if (approxDistance > COLLIDE_CRAFT_MAX_DISTANCE)
		return 0;
#ifdef XW_MODERN
	segmentDistanceX = (int32_t)((uint32_t)g_collisionProbeWorldX - (uint32_t)g_collisionSegmentStartWorldX);
	if (segmentDistanceX < 0)
		segmentDistanceX = (int32_t)(0u - (uint32_t)segmentDistanceX);
#else
	segmentDistanceX = g_collisionProbeWorldX - g_collisionSegmentStartWorldX;
	if (segmentDistanceX < 0)
		segmentDistanceX = -segmentDistanceX;
#endif
#ifdef XW_MODERN
	segmentDistanceY = (int32_t)((uint32_t)g_collisionProbeWorldY - (uint32_t)g_collisionSegmentStartWorldY);
	if (segmentDistanceY < 0)
		segmentDistanceY = (int32_t)(0u - (uint32_t)segmentDistanceY);
#else
	segmentDistanceY = g_collisionProbeWorldY - g_collisionSegmentStartWorldY;
	if (segmentDistanceY < 0)
		segmentDistanceY = -segmentDistanceY;
#endif
#ifdef XW_MODERN
	segmentDistanceZ = (int32_t)((uint32_t)g_collisionProbeWorldZ - (uint32_t)g_collisionSegmentStartWorldZ);
	if (segmentDistanceZ < 0)
		segmentDistanceZ = (int32_t)(0u - (uint32_t)segmentDistanceZ);
#else
	segmentDistanceZ = g_collisionProbeWorldZ - g_collisionSegmentStartWorldZ;
	if (segmentDistanceZ < 0)
		segmentDistanceZ = -segmentDistanceZ;
#endif
#ifdef XW_MODERN
	sweepDistance = (int32_t)((uint32_t)g_collisionSweepEndX - (uint32_t)g_collisionSweepStartX);
	if (sweepDistance < 0)
		sweepDistance = (int32_t)(0u - (uint32_t)sweepDistance);
	segmentDistanceX = (int32_t)((uint32_t)segmentDistanceX + (uint32_t)sweepDistance);
#else
	sweepDistance = g_collisionSweepEndX - g_collisionSweepStartX;
	if (sweepDistance < 0)
		sweepDistance = -sweepDistance;
	segmentDistanceX = segmentDistanceX + sweepDistance;
#endif
#ifdef XW_MODERN
	sweepDistance = (int32_t)((uint32_t)g_collisionSweepEndY - (uint32_t)g_collisionSweepStartY);
	if (sweepDistance < 0)
		sweepDistance = (int32_t)(0u - (uint32_t)sweepDistance);
	segmentDistanceY = (int32_t)((uint32_t)segmentDistanceY + (uint32_t)sweepDistance);
#else
	sweepDistance = g_collisionSweepEndY - g_collisionSweepStartY;
	if (sweepDistance < 0)
		sweepDistance = -sweepDistance;
	segmentDistanceY = segmentDistanceY + sweepDistance;
#endif
#ifdef XW_MODERN
	sweepDistance = (int32_t)((uint32_t)g_collisionSweepEndZ - (uint32_t)g_collisionSweepStartZ);
	if (sweepDistance < 0)
		sweepDistance = (int32_t)(0u - (uint32_t)sweepDistance);
	segmentDistanceZ = (int32_t)((uint32_t)segmentDistanceZ + (uint32_t)sweepDistance);
#else
	sweepDistance = g_collisionSweepEndZ - g_collisionSweepStartZ;
	if (sweepDistance < 0)
		sweepDistance = -sweepDistance;
	segmentDistanceZ = segmentDistanceZ + sweepDistance;
#endif
	objectType = g_objectTable[targetObjIdx].objectType;
	maxBoundsExtent = g_modelTypeTable[objectType].maxBoundsExtent;
#ifdef XW_MODERN
	if (XwFlightTypes_CanonicalType(objectType) == XW_OBJ_IMPERIAL_STAR_DESTROYER ||
		XwFlightTypes_CanonicalType(objectType) == XW_OBJ_CALAMARI_CRUISER) {
#else
	if (objectType == XW_OBJ_IMPERIAL_STAR_DESTROYER || objectType == XW_OBJ_CALAMARI_CRUISER) {
#endif
		maxBoundsExtent *= COLLIDE_CAPITAL_EXTENT_MULTIPLIER;
	} else {
		uint8_t genusId = g_objectTable[targetObjIdx].genusId;
		if (genusId == XW_GENUS_OTHER_PROJECTILE || genusId == XW_GENUS_PLAYER_PROJECTILE)
#ifdef XW_MODERN
			maxBoundsExtent >>= XwFlightTypes_Dos() ? 4 : 1;
#else
			maxBoundsExtent >>= 1;
#endif
	}
	if ((unsigned int)approxDistance >
		collide_roughdistance3du((unsigned int)maxBoundsExtent + (unsigned int)segmentDistanceX,
								 (unsigned int)maxBoundsExtent + (unsigned int)segmentDistanceY,
								 (unsigned int)maxBoundsExtent + (unsigned int)segmentDistanceZ))
		return 0;
	if (g_deathStarSurfaceModeActive == 0 && maxBoundsExtent > COLLIDE_CRAFT_LARGE_EXTENT)
		return (int16_t)starship_checkstarshiphit(sourceObjIdx, targetObjIdx);
	maxBoundsExtent >>= COLLIDE_BOX_QUARTER_SHIFT;
	return collide_checkboxcollision(maxBoundsExtent + (maxBoundsExtent >> 1));
}

// FUNCTION: XW 0x403A00
int16_t collide_checkboxcollision(int halfExtent) {
	int segmentDeltaX;
	int sweepDeltaX;
	int outsideDeltaX;
	int minDistanceX;
	int maxDistanceX;
	int segmentDeltaY;
	int sweepDeltaY;
	int outsideDeltaY;
	int minDistanceY;
	int maxDistanceY;
	int segmentDeltaZ;
	int sweepDeltaZ;
	int outsideDeltaZ;
	int minDistanceZ;
	int maxDistanceZ;
	int relativeStart;
	int entryTime;
	int exitTime;
	int16_t entryFraction;
#ifdef XW_MODERN
	segmentDeltaX = (int32_t)((uint32_t)g_collisionProbeWorldX - (uint32_t)g_collisionSegmentStartWorldX);
	segmentDeltaY = (int32_t)((uint32_t)g_collisionProbeWorldY - (uint32_t)g_collisionSegmentStartWorldY);
	segmentDeltaZ = (int32_t)((uint32_t)g_collisionProbeWorldZ - (uint32_t)g_collisionSegmentStartWorldZ);
	sweepDeltaX = (int32_t)((uint32_t)g_collisionSweepEndX - (uint32_t)g_collisionSweepStartX);
	sweepDeltaY = (int32_t)((uint32_t)g_collisionSweepEndY - (uint32_t)g_collisionSweepStartY);
	sweepDeltaZ = (int32_t)((uint32_t)g_collisionSweepEndZ - (uint32_t)g_collisionSweepStartZ);
	relativeStart = (int32_t)((uint32_t)g_collisionSweepStartX - (uint32_t)g_collisionSegmentStartWorldX);
#else
	segmentDeltaX = g_collisionProbeWorldX - g_collisionSegmentStartWorldX;
	segmentDeltaY = g_collisionProbeWorldY - g_collisionSegmentStartWorldY;
	segmentDeltaZ = g_collisionProbeWorldZ - g_collisionSegmentStartWorldZ;
	sweepDeltaX = g_collisionSweepEndX - g_collisionSweepStartX;
	sweepDeltaY = g_collisionSweepEndY - g_collisionSweepStartY;
	sweepDeltaZ = g_collisionSweepEndZ - g_collisionSweepStartZ;
	relativeStart = g_collisionSweepStartX - g_collisionSegmentStartWorldX;
#endif
	if (relativeStart > halfExtent) {
#ifdef XW_MODERN
		outsideDeltaX = (int32_t)((uint32_t)sweepDeltaX - (uint32_t)segmentDeltaX);
		maxDistanceX = (int32_t)((uint32_t)halfExtent - (uint32_t)relativeStart);
#else
		outsideDeltaX = sweepDeltaX - segmentDeltaX;
		maxDistanceX = halfExtent - relativeStart;
#endif
		if (outsideDeltaX >= 0)
			return 0;
		if (maxDistanceX < outsideDeltaX)
			return 0;
#ifdef XW_MODERN
		minDistanceX = (int32_t)(0u - (uint32_t)halfExtent - (uint32_t)relativeStart);
#else
		minDistanceX = -(halfExtent + relativeStart);
#endif
	} else if (relativeStart < -halfExtent) {
#ifdef XW_MODERN
		minDistanceX = (int32_t)(0u - (uint32_t)halfExtent - (uint32_t)relativeStart);
		outsideDeltaX = (int32_t)((uint32_t)sweepDeltaX - (uint32_t)segmentDeltaX);
#else
		minDistanceX = -(halfExtent + relativeStart);
		outsideDeltaX = sweepDeltaX - segmentDeltaX;
#endif
		if (outsideDeltaX < 0)
			return 0;
		if (minDistanceX >= outsideDeltaX)
			return 0;
#ifdef XW_MODERN
		maxDistanceX = (int32_t)((uint32_t)halfExtent - (uint32_t)relativeStart);
#else
		maxDistanceX = halfExtent - relativeStart;
#endif
	} else {
#ifdef XW_MODERN
		maxDistanceX = (int32_t)((uint32_t)halfExtent - (uint32_t)relativeStart);
		minDistanceX = (int32_t)(0u - (uint32_t)halfExtent - (uint32_t)relativeStart);
#else
		maxDistanceX = halfExtent - relativeStart;
		minDistanceX = -(halfExtent + relativeStart);
#endif
		outsideDeltaX = 0;
	}
#ifdef XW_MODERN
	relativeStart = (int32_t)((uint32_t)g_collisionSweepStartY - (uint32_t)g_collisionSegmentStartWorldY);
#else
	relativeStart = g_collisionSweepStartY - g_collisionSegmentStartWorldY;
#endif
	if (relativeStart > halfExtent) {
#ifdef XW_MODERN
		outsideDeltaY = (int32_t)((uint32_t)sweepDeltaY - (uint32_t)segmentDeltaY);
		maxDistanceY = (int32_t)((uint32_t)halfExtent - (uint32_t)relativeStart);
#else
		outsideDeltaY = sweepDeltaY - segmentDeltaY;
		maxDistanceY = halfExtent - relativeStart;
#endif
		if (outsideDeltaY >= 0)
			return 0;
		if (maxDistanceY < outsideDeltaY)
			return 0;
#ifdef XW_MODERN
		minDistanceY = (int32_t)(0u - (uint32_t)halfExtent - (uint32_t)relativeStart);
#else
		minDistanceY = -(halfExtent + relativeStart);
#endif
	} else if (relativeStart < -halfExtent) {
#ifdef XW_MODERN
		minDistanceY = (int32_t)(0u - (uint32_t)halfExtent - (uint32_t)relativeStart);
		outsideDeltaY = (int32_t)((uint32_t)sweepDeltaY - (uint32_t)segmentDeltaY);
#else
		minDistanceY = -(halfExtent + relativeStart);
		outsideDeltaY = sweepDeltaY - segmentDeltaY;
#endif
		if (outsideDeltaY < 0)
			return 0;
		if (minDistanceY >= outsideDeltaY)
			return 0;
#ifdef XW_MODERN
		maxDistanceY = (int32_t)((uint32_t)halfExtent - (uint32_t)relativeStart);
#else
		maxDistanceY = halfExtent - relativeStart;
#endif
	} else {
#ifdef XW_MODERN
		maxDistanceY = (int32_t)((uint32_t)halfExtent - (uint32_t)relativeStart);
		minDistanceY = (int32_t)(0u - (uint32_t)halfExtent - (uint32_t)relativeStart);
#else
		maxDistanceY = halfExtent - relativeStart;
		minDistanceY = -(halfExtent + relativeStart);
#endif
		outsideDeltaY = 0;
	}
#ifdef XW_MODERN
	relativeStart = (int32_t)((uint32_t)g_collisionSweepStartZ - (uint32_t)g_collisionSegmentStartWorldZ);
#else
	relativeStart = g_collisionSweepStartZ - g_collisionSegmentStartWorldZ;
#endif
	if (relativeStart > halfExtent) {
#ifdef XW_MODERN
		outsideDeltaZ = (int32_t)((uint32_t)sweepDeltaZ - (uint32_t)segmentDeltaZ);
		maxDistanceZ = (int32_t)((uint32_t)halfExtent - (uint32_t)relativeStart);
#else
		outsideDeltaZ = sweepDeltaZ - segmentDeltaZ;
		maxDistanceZ = halfExtent - relativeStart;
#endif
		if (outsideDeltaZ >= 0)
			return 0;
		if (maxDistanceZ < outsideDeltaZ)
			return 0;
#ifdef XW_MODERN
		minDistanceZ = (int32_t)(0u - (uint32_t)halfExtent - (uint32_t)relativeStart);
#else
		minDistanceZ = -(halfExtent + relativeStart);
#endif
	} else if (relativeStart < -halfExtent) {
#ifdef XW_MODERN
		minDistanceZ = (int32_t)(0u - (uint32_t)halfExtent - (uint32_t)relativeStart);
		outsideDeltaZ = (int32_t)((uint32_t)sweepDeltaZ - (uint32_t)segmentDeltaZ);
#else
		minDistanceZ = -(halfExtent + relativeStart);
		outsideDeltaZ = sweepDeltaZ - segmentDeltaZ;
#endif
		if (outsideDeltaZ < 0)
			return 0;
		if (minDistanceZ >= outsideDeltaZ)
			return 0;
#ifdef XW_MODERN
		maxDistanceZ = (int32_t)((uint32_t)halfExtent - (uint32_t)relativeStart);
#else
		maxDistanceZ = halfExtent - relativeStart;
#endif
	} else {
#ifdef XW_MODERN
		maxDistanceZ = (int32_t)((uint32_t)halfExtent - (uint32_t)relativeStart);
		minDistanceZ = (int32_t)(0u - (uint32_t)halfExtent - (uint32_t)relativeStart);
#else
		maxDistanceZ = halfExtent - relativeStart;
		minDistanceZ = -(halfExtent + relativeStart);
#endif
		outsideDeltaZ = 0;
	}
#ifdef XW_MODERN
	maxDistanceX = (int32_t)((uint32_t)maxDistanceX << COLLIDE_TIME_FRACTION_BITS);
	minDistanceX = (int32_t)((uint32_t)minDistanceX << COLLIDE_TIME_FRACTION_BITS);
	maxDistanceY = (int32_t)((uint32_t)maxDistanceY << COLLIDE_TIME_FRACTION_BITS);
	minDistanceY = (int32_t)((uint32_t)minDistanceY << COLLIDE_TIME_FRACTION_BITS);
	maxDistanceZ = (int32_t)((uint32_t)maxDistanceZ << COLLIDE_TIME_FRACTION_BITS);
	minDistanceZ = (int32_t)((uint32_t)minDistanceZ << COLLIDE_TIME_FRACTION_BITS);
#else
	maxDistanceX = maxDistanceX << COLLIDE_TIME_FRACTION_BITS;
	minDistanceX = minDistanceX << COLLIDE_TIME_FRACTION_BITS;
	maxDistanceY = maxDistanceY << COLLIDE_TIME_FRACTION_BITS;
	minDistanceY = minDistanceY << COLLIDE_TIME_FRACTION_BITS;
	maxDistanceZ = maxDistanceZ << COLLIDE_TIME_FRACTION_BITS;
	minDistanceZ = minDistanceZ << COLLIDE_TIME_FRACTION_BITS;
#endif
	if (outsideDeltaX == 0) {
		int relativeDelta;
		int relativeEnd;
#ifdef XW_MODERN
		relativeDelta = (int32_t)((uint32_t)sweepDeltaX - (uint32_t)segmentDeltaX);
		relativeEnd = (int32_t)((uint32_t)g_collisionSweepEndX - (uint32_t)g_collisionProbeWorldX);
#else
		relativeDelta = sweepDeltaX - segmentDeltaX;
		relativeEnd = g_collisionSweepEndX - g_collisionProbeWorldX;
#endif
		if (relativeEnd > halfExtent)
			exitTime = maxDistanceX / relativeDelta;
		else if (relativeEnd < -halfExtent)
			exitTime = minDistanceX / relativeDelta;
		else
			exitTime = COLLIDE_TIME_LAST_FRACTION;
		entryTime = 0;
	} else {
		int minTime;
		entryTime = maxDistanceX / outsideDeltaX;
		minTime = minDistanceX / outsideDeltaX;
		exitTime = minTime;
		if (outsideDeltaX >= 0) {
			exitTime = entryTime;
			entryTime = minTime;
		}
	}
	if (outsideDeltaY == 0) {
		int relativeDelta;
		int relativeEnd;
		int axisExit;
#ifdef XW_MODERN
		relativeDelta = (int32_t)((uint32_t)sweepDeltaY - (uint32_t)segmentDeltaY);
		relativeEnd = (int32_t)((uint32_t)g_collisionSweepEndY - (uint32_t)g_collisionProbeWorldY);
#else
		relativeDelta = sweepDeltaY - segmentDeltaY;
		relativeEnd = g_collisionSweepEndY - g_collisionProbeWorldY;
#endif
		if (relativeEnd > halfExtent)
			axisExit = maxDistanceY / relativeDelta;
		else if (relativeEnd < -halfExtent)
			axisExit = minDistanceY / relativeDelta;
		else
			axisExit = COLLIDE_TIME_LAST_FRACTION;
		if (exitTime < 0)
			return 0;
		if (entryTime < 0)
			entryTime = 0;
		if (axisExit < entryTime)
			return 0;
		if (axisExit < exitTime)
			exitTime = axisExit;
	} else {
		int maxTime = maxDistanceY / outsideDeltaY;
		int minTime;
		if (outsideDeltaY >= 0) {
			int clippedExit = exitTime;
			if (maxTime < entryTime)
				return 0;
			if (maxTime < exitTime) {
				clippedExit = maxTime;
				exitTime = maxTime;
			}
			minTime = minDistanceY / outsideDeltaY;
			if (minTime > clippedExit)
				return 0;
			if (minTime > entryTime)
				entryTime = minTime;
		} else {
			if (maxTime > exitTime)
				return 0;
			if (maxTime > entryTime)
				entryTime = maxTime;
			minTime = minDistanceY / outsideDeltaY;
			if (minTime < entryTime)
				return 0;
			if (minTime < exitTime)
				exitTime = minTime;
		}
	}
	if (outsideDeltaZ == 0) {
		int relativeDelta;
		int relativeEnd;
		int axisExit;
#ifdef XW_MODERN
		relativeDelta = (int32_t)((uint32_t)sweepDeltaZ - (uint32_t)segmentDeltaZ);
		relativeEnd = (int32_t)((uint32_t)g_collisionSweepEndZ - (uint32_t)g_collisionProbeWorldZ);
#else
		relativeDelta = sweepDeltaZ - segmentDeltaZ;
		relativeEnd = g_collisionSweepEndZ - g_collisionProbeWorldZ;
#endif
		if (relativeEnd > halfExtent)
			axisExit = maxDistanceZ / relativeDelta;
		else if (relativeEnd < -halfExtent)
			axisExit = minDistanceZ / relativeDelta;
		else
			axisExit = COLLIDE_TIME_LAST_FRACTION;
		if (exitTime < 0)
			return 0;
		if (entryTime < 0)
			entryTime = 0;
		if (axisExit < entryTime)
			return 0;
	} else {
		int maxTime = maxDistanceZ / outsideDeltaZ;
		int minTime;
		if (outsideDeltaZ >= 0) {
			int clippedExit = exitTime;
			if (maxTime < entryTime)
				return 0;
			if (maxTime < exitTime)
				clippedExit = maxTime;
			minTime = minDistanceZ / outsideDeltaZ;
			if (minTime > clippedExit)
				return 0;
			if (minTime > entryTime)
				entryTime = minTime;
		} else {
			if (maxTime > exitTime)
				return 0;
			if (maxTime > entryTime)
				entryTime = maxTime;
			minTime = minDistanceZ / outsideDeltaZ;
			if (minTime < entryTime)
				return 0;
		}
	}
	if (entryTime > COLLIDE_TIME_LAST_FRACTION)
		return 0;
#ifdef XW_MODERN
	entryFraction = (XwFlightTypes_Dos() ? (uint8_t)entryTime : (uint16_t)entryTime)
					<< COLLIDE_TIME_TO_Q15_SHIFT;
#else
	entryFraction = (uint16_t)entryTime << COLLIDE_TIME_TO_Q15_SHIFT;
#endif
	g_collisionHitOffsetX = (int16_t)segmentDeltaX;
	g_collisionHitOffsetY = (int16_t)segmentDeltaY;
	g_collisionHitOffsetZ = (int16_t)segmentDeltaZ;
	/* Both factors were narrowed to signed 16-bit, so their product fits in 32 bits. */
	g_collisionHitOffsetX = (entryFraction * g_collisionHitOffsetX) >> COLLIDE_HIT_FRACTION_BITS;
	g_collisionHitOffsetY = (entryFraction * g_collisionHitOffsetY) >> COLLIDE_HIT_FRACTION_BITS;
	g_collisionHitOffsetZ = (entryFraction * g_collisionHitOffsetZ) >> COLLIDE_HIT_FRACTION_BITS;
	return 1;
}

// FUNCTION: XW 0x403F00
int16_t collide_targetinrange(uint16_t sourceObjectIndex, uint16_t targetObjectRef, uint16_t weaponSlot) {
	uint16_t projectileType = g_craftTypeDefs[g_playerFlightState.craftTypeIndex]
								  .laserGroupWeaponType[g_playerFlightState.selectedWeaponBank];
#ifdef XW_MODERN
	int projectileIndex = XwFlightTypes_ProjectileIndex(projectileType);
#else
	int projectileIndex = projectileType - LASER_PROJECTILE_FIRST_TYPE;
#endif
	int16_t projectileSpeed = g_projectileSpeedByType[projectileIndex];
	int16_t predictionSteps = (int16_t)(g_simStepScale * g_projectileLifetimeSecondsByType[projectileIndex]);
	ObjectRecord* sourceObject;
	uint16_t craftTypeIndex;
	int16_t projectileStepDistance;
#ifdef XW_MODERN
	if (XwFlightTypes_Dos())
		predictionSteps = (int16_t)(3 * g_simStepScale);
	else if (projectileType == XW_OBJ_LASER_143)
		predictionSteps = (int16_t)(predictionSteps + predictionSteps / 2);
#else
	if (projectileType == XW_OBJ_LASER_143)
		predictionSteps = (int16_t)(predictionSteps + predictionSteps / 2);
#endif
	sourceObject = &g_objectTable[sourceObjectIndex];
	craftTypeIndex = ((CraftData*)sourceObject->instanceData)->craftTypeIndex;
	g_collisionSegmentStartWorldX = sourceObject->worldX;
	g_collisionSegmentStartWorldY = sourceObject->worldY;
	g_collisionSegmentStartWorldZ = sourceObject->worldZ;
	pai_calcrotatedpoint(sourceObject, g_craftTypeDefs[craftTypeIndex].weaponHardpoints[weaponSlot].x,
						 g_craftTypeDefs[craftTypeIndex].weaponHardpoints[weaponSlot].z,
						 g_craftTypeDefs[craftTypeIndex].weaponHardpoints[weaponSlot].y);
#ifdef XW_MODERN
	g_collisionSegmentStartWorldX = (int32_t)((uint32_t)g_collisionSegmentStartWorldX + (uint32_t)g_rotatedX);
	g_collisionSegmentStartWorldY = (int32_t)((uint32_t)g_collisionSegmentStartWorldY + (uint32_t)g_rotatedY);
	g_collisionSegmentStartWorldZ = (int32_t)((uint32_t)g_collisionSegmentStartWorldZ + (uint32_t)g_rotatedZ);
#else
	g_collisionSegmentStartWorldX += g_rotatedX;
	g_collisionSegmentStartWorldY += g_rotatedY;
	g_collisionSegmentStartWorldZ += g_rotatedZ;
#endif
	projectileStepDistance =
		(int16_t)math2_mphconvert((int16_t)(projectileSpeed + sourceObject->speed), g_simStepScale);
	if (sourceObject->moveVectorDirty != 0)
		fview_calcrotatemove(sourceObject->pitch, sourceObject->yaw, sourceObject);
	g_collisionProbeWorldX =
		(int32_t)((uint32_t)g_collisionSegmentStartWorldX +
				  (uint32_t)predictionSteps *
#ifdef XW_MODERN
					  (uint32_t)XwFlightMath_PredictionStep(projectileStepDistance, sourceObject->moveX)
#else
					  (uint32_t)((uint64_t)((int64_t)projectileStepDistance * sourceObject->moveX) >>
								 FVIEW_MATRIX_FRACTION_BITS)
#endif
		);
	g_collisionProbeWorldY =
		(int32_t)((uint32_t)g_collisionSegmentStartWorldY +
				  (uint32_t)predictionSteps *
#ifdef XW_MODERN
					  (uint32_t)XwFlightMath_PredictionStep(projectileStepDistance, sourceObject->moveY)
#else
					  (uint32_t)((uint64_t)((int64_t)projectileStepDistance * sourceObject->moveY) >>
								 FVIEW_MATRIX_FRACTION_BITS)
#endif
		);
	g_collisionProbeWorldZ =
		(int32_t)((uint32_t)g_collisionSegmentStartWorldZ +
				  (uint32_t)predictionSteps *
#ifdef XW_MODERN
					  (uint32_t)XwFlightMath_PredictionStep(projectileStepDistance, sourceObject->moveZ)
#else
					  (uint32_t)((uint64_t)((int64_t)projectileStepDistance * sourceObject->moveZ) >>
								 FVIEW_MATRIX_FRACTION_BITS)
#endif
		);
	if (targetObjectRef < XW_MISSION_OBJECT_REF_BASE) {
		ObjectRecord* targetObject = &g_objectTable[targetObjectRef];
		int16_t targetStepDistance;
		g_collisionSweepStartX = targetObject->worldX;
		g_collisionSweepStartY = targetObject->worldY;
		g_collisionSweepStartZ = targetObject->worldZ;
		targetStepDistance = (int16_t)math2_mphconvert(targetObject->speed, g_simStepScale);
		if (targetObject->moveVectorDirty != 0)
			fview_calcrotatemove(targetObject->pitch, targetObject->yaw, targetObject);
		g_collisionSweepEndX =
			(int32_t)((uint32_t)g_collisionSweepStartX +
					  (uint32_t)predictionSteps *
#ifdef XW_MODERN
						  (uint32_t)XwFlightMath_PredictionStep(targetStepDistance, targetObject->moveX)
#else
						  (uint32_t)((uint64_t)((int64_t)targetStepDistance * targetObject->moveX) >>
									 FVIEW_MATRIX_FRACTION_BITS)
#endif
			);
		g_collisionSweepEndY =
			(int32_t)((uint32_t)g_collisionSweepStartY +
					  (uint32_t)predictionSteps *
#ifdef XW_MODERN
						  (uint32_t)XwFlightMath_PredictionStep(targetStepDistance, targetObject->moveY)
#else
						  (uint32_t)((uint64_t)((int64_t)targetStepDistance * targetObject->moveY) >>
									 FVIEW_MATRIX_FRACTION_BITS)
#endif
			);
		g_collisionSweepEndZ =
			(int32_t)((uint32_t)g_collisionSweepStartZ +
					  (uint32_t)predictionSteps *
#ifdef XW_MODERN
						  (uint32_t)XwFlightMath_PredictionStep(targetStepDistance, targetObject->moveZ)
#else
						  (uint32_t)((uint64_t)((int64_t)targetStepDistance * targetObject->moveZ) >>
									 FVIEW_MATRIX_FRACTION_BITS)
#endif
			);
		return collide_lasercraftcollide(sourceObjectIndex, targetObjectRef);
	}
	return static_laserstaticcollide(sourceObjectIndex,
									 (uint16_t)(targetObjectRef - XW_MISSION_OBJECT_REF_BASE));
}

// FUNCTION: XW 0x404210
uint16_t collide_craftstarshipcollision(uint16_t sourceObjectIndex, int16_t predictionSeconds) {
	int16_t predictionSteps = (int16_t)(g_simStepScale * predictionSeconds);
	ObjectRecord* sourceObject = &g_objectTable[sourceObjectIndex];
	int16_t sourceStepDistance;
	uint16_t candidateIndex;
	g_collisionSegmentStartWorldX = sourceObject->worldX;
	g_collisionSegmentStartWorldY = sourceObject->worldY;
	g_collisionSegmentStartWorldZ = sourceObject->worldZ;
	sourceStepDistance = (int16_t)math2_mphconvert(sourceObject->speed, g_simStepScale);
	if (sourceObject->moveVectorDirty != 0)
		fview_calcrotatemove(sourceObject->pitch, sourceObject->yaw, sourceObject);
	g_collisionProbeWorldX =
		(int32_t)((uint32_t)g_collisionSegmentStartWorldX +
				  (uint32_t)predictionSteps *
#ifdef XW_MODERN
					  (uint32_t)XwFlightMath_PredictionStep(sourceStepDistance, sourceObject->moveX)
#else
					  (uint32_t)((uint64_t)((int64_t)sourceStepDistance * sourceObject->moveX) >>
								 FVIEW_MATRIX_FRACTION_BITS)
#endif
		);
	g_collisionProbeWorldY =
		(int32_t)((uint32_t)g_collisionSegmentStartWorldY +
				  (uint32_t)predictionSteps *
#ifdef XW_MODERN
					  (uint32_t)XwFlightMath_PredictionStep(sourceStepDistance, sourceObject->moveY)
#else
					  (uint32_t)((uint64_t)((int64_t)sourceStepDistance * sourceObject->moveY) >>
								 FVIEW_MATRIX_FRACTION_BITS)
#endif
		);
	g_collisionProbeWorldZ =
		(int32_t)((uint32_t)g_collisionSegmentStartWorldZ +
				  (uint32_t)predictionSteps *
#ifdef XW_MODERN
					  (uint32_t)XwFlightMath_PredictionStep(sourceStepDistance, sourceObject->moveZ)
#else
					  (uint32_t)((uint64_t)((int64_t)sourceStepDistance * sourceObject->moveZ) >>
								 FVIEW_MATRIX_FRACTION_BITS)
#endif
		);
	for (candidateIndex = 0; candidateIndex < XW_CRAFT_OBJECT_COUNT; ++candidateIndex) {
		if (g_objectTable[candidateIndex].objectType != XW_OBJ_NONE) {
			uint8_t genusId = g_objectTable[candidateIndex].genusId;
			if (genusId == XW_GENUS_STARSHIP || genusId == XW_GENUS_FREIGHTER) {
				ObjectRecord* candidateObject = &g_objectTable[candidateIndex];
				int16_t candidateStepDistance;
				g_collisionSweepStartY = candidateObject->worldY;
				g_collisionSweepStartX = candidateObject->worldX;
				g_collisionSweepStartZ = candidateObject->worldZ;
				candidateStepDistance = (int16_t)math2_mphconvert(candidateObject->speed, g_simStepScale);
				if (candidateObject->moveVectorDirty != 0)
					fview_calcrotatemove(candidateObject->pitch, candidateObject->yaw, candidateObject);
				g_collisionSweepEndX = (int32_t)((uint32_t)g_collisionSweepStartX +
												 (uint32_t)predictionSteps *
#ifdef XW_MODERN
													 (uint32_t)XwFlightMath_PredictionStep(
														 candidateStepDistance, candidateObject->moveX)
#else
													 (uint32_t)((uint64_t)((int64_t)candidateStepDistance *
																		   candidateObject->moveX) >>
																FVIEW_MATRIX_FRACTION_BITS)
#endif
				);
				g_collisionSweepEndY = (int32_t)((uint32_t)g_collisionSweepStartY +
												 (uint32_t)predictionSteps *
#ifdef XW_MODERN
													 (uint32_t)XwFlightMath_PredictionStep(
														 candidateStepDistance, candidateObject->moveY)
#else
													 (uint32_t)((uint64_t)((int64_t)candidateStepDistance *
																		   candidateObject->moveY) >>
																FVIEW_MATRIX_FRACTION_BITS)
#endif
				);
				g_collisionSweepEndZ = (int32_t)((uint32_t)g_collisionSweepStartZ +
												 (uint32_t)predictionSteps *
#ifdef XW_MODERN
													 (uint32_t)XwFlightMath_PredictionStep(
														 candidateStepDistance, candidateObject->moveZ)
#else
													 (uint32_t)((uint64_t)((int64_t)candidateStepDistance *
																		   candidateObject->moveZ) >>
																FVIEW_MATRIX_FRACTION_BITS)
#endif
				);
				if (collide_lasercraftcollide(sourceObjectIndex, candidateIndex) != 0)
					return candidateIndex;
			}
		}
	}
	return XW_OBJECT_SLOT_UNAVAILABLE;
}

// FUNCTION: XW 0x404450
void collide_laserhitcraft(uint16_t projectileObjIdx, uint16_t craftObjIdx, int16_t hitMeshIndex) {
	uint16_t sourceObjectRef = g_objectTable[projectileObjIdx].sourceObjectRef;
	int16_t shieldSide;
	char playGenericImpactSound;
	if (sourceObjectRef == craftObjIdx)
		return;
	g_curCraft = (CraftData*)g_objectTable[craftObjIdx].instanceData;
	if (g_curCraft->lastAttackerObjIdx == COLLIDE_NO_ATTACKER && sourceObjectRef < XW_CRAFT_OBJECT_COUNT)
		g_curCraft->lastAttackerObjIdx = sourceObjectRef;
	++g_curCraft->aiHitsThisManeuver;
	if (craftObjIdx == g_playerFlightState.objectIndex) {
		int deltaZ, deltaY, deltaX;
		int forwardZ, forwardY, forwardX;
		int dot;
		if (g_playerFlightState.object->orientMatrixDirty != 0) {
			fview_calcrotatemove(g_playerFlightState.object->pitch, g_playerFlightState.object->yaw,
								 g_playerFlightState.object);
			fview_calcrotateorient(g_playerFlightState.object->roll, 0, g_playerFlightState.object);
		}
#ifdef XW_MODERN
		deltaZ = (int16_t)((uint32_t)g_collisionProbeWorldZ - (uint32_t)g_collisionSegmentStartWorldZ);
#else
		deltaZ = (int16_t)(g_collisionProbeWorldZ - g_collisionSegmentStartWorldZ);
#endif
#ifdef XW_MODERN
		deltaY = (int16_t)((uint32_t)g_collisionProbeWorldY - (uint32_t)g_collisionSegmentStartWorldY);
#else
		deltaY = (int16_t)(g_collisionProbeWorldY - g_collisionSegmentStartWorldY);
#endif
#ifdef XW_MODERN
		deltaX = (int16_t)((uint32_t)g_collisionProbeWorldX - (uint32_t)g_collisionSegmentStartWorldX);
#else
		deltaX = (int16_t)(g_collisionProbeWorldX - g_collisionSegmentStartWorldX);
#endif
		forwardZ = g_playerFlightState.object->cachedForwardZ;
		forwardY = g_playerFlightState.object->cachedForwardY;
		forwardX = g_playerFlightState.object->cachedForwardX;
#ifdef XW_MODERN
		dot = (int32_t)((uint32_t)(forwardX * deltaX) + (uint32_t)(forwardY * deltaY) +
						(uint32_t)(forwardZ * deltaZ));
#else
		dot = forwardX * deltaX + forwardY * deltaY + forwardZ * deltaZ;
#endif
		if (dot >= FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MAX;
		if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MIN;
		shieldSide = (int16_t)(dot >> FVIEW_MATRIX_FRACTION_BITS) >= 0;
	} else {
		shieldSide = 0;
	}
	playGenericImpactSound = collide_damagecraft(craftObjIdx, hitMeshIndex, projectileObjIdx, shieldSide);
#ifdef XW_MODERN
	g_objectTable[projectileObjIdx].worldX =
		(int32_t)((uint32_t)g_collisionSegmentStartWorldX + (uint32_t)g_collisionHitOffsetX);
#else
	g_objectTable[projectileObjIdx].worldX = g_collisionSegmentStartWorldX + g_collisionHitOffsetX;
#endif
#ifdef XW_MODERN
	g_objectTable[projectileObjIdx].worldY =
		(int32_t)((uint32_t)g_collisionSegmentStartWorldY + (uint32_t)g_collisionHitOffsetY);
#else
	g_objectTable[projectileObjIdx].worldY = g_collisionSegmentStartWorldY + g_collisionHitOffsetY;
#endif
#ifdef XW_MODERN
	g_objectTable[projectileObjIdx].worldZ =
		(int32_t)((uint32_t)g_collisionSegmentStartWorldZ + (uint32_t)g_collisionHitOffsetZ);
#else
	g_objectTable[projectileObjIdx].worldZ = g_collisionSegmentStartWorldZ + g_collisionHitOffsetZ;
#endif
#ifdef XW_MODERN
	if (XwFlightTypes_IsWarhead(g_objectTable[projectileObjIdx].objectType))
#else
	if (g_objectTable[projectileObjIdx].objectType == XW_OBJ_WARHEAD_149 ||
		g_objectTable[projectileObjIdx].objectType == XW_OBJ_TRACKED_WARHEAD)
#endif
		g_objectTable[projectileObjIdx].objectType = (math2_getrandom() & 1) + XW_OBJ_EXPLOSION_133;
	else
		g_objectTable[projectileObjIdx].objectType = XW_OBJ_ASTEROID_IMPACT;
#ifdef XW_MODERN
	g_objectTable[projectileObjIdx].objectType =
		XwFlightTypes_ObjectType(g_objectTable[projectileObjIdx].objectType);
	if (XwFlightTypes_Dos())
		g_objectTable[projectileObjIdx].instanceData = NULL;
#endif
#ifdef XW_MODERN
	XwFlightIntegration_Reset(projectileObjIdx);
	XwRenderObjects_ReplaceMobile(projectileObjIdx);
#endif
	g_objectTable[projectileObjIdx].genusId = XW_GENUS_EXPLOSION_EFFECT;
	g_objectTable[projectileObjIdx].familyId = XW_OBJECT_FAMILY_5;
	g_objectTable[projectileObjIdx].animationState = XW_OBJECT_ANIMATION_BREAKUP;
	g_objectTable[projectileObjIdx].ageSeconds = 0;
	g_objectTable[projectileObjIdx].lifetimeTicks = 0;
	g_objectTable[projectileObjIdx].billboardScaleCode = 0;
	g_objectTable[projectileObjIdx].speed = g_objectTable[craftObjIdx].speed;
	g_objectTable[projectileObjIdx].pitch = g_objectTable[craftObjIdx].pitch;
	g_objectTable[projectileObjIdx].yaw = g_objectTable[craftObjIdx].yaw;
	g_objectTable[projectileObjIdx].roll = 0;
	g_objectTable[projectileObjIdx].orientMatrixDirty = 1;
	g_objectTable[projectileObjIdx].moveVectorDirty = 1;
	if (playGenericImpactSound != 0) {
		if (craftObjIdx == g_playerFlightState.objectIndex)
			fsfx_triggersfx(COLLIDE_PLAYER_IMPACT_SOUND, projectileObjIdx);
#ifdef XW_MODERN
		else if (g_objectTable[projectileObjIdx].objectType ==
				 XwFlightTypes_ObjectType(XW_OBJ_ASTEROID_IMPACT))
#else
		else if (g_objectTable[projectileObjIdx].objectType == XW_OBJ_ASTEROID_IMPACT)
#endif
			fsfx_triggersfx(FSFX_ASTEROID_IMPACT_SLOT, projectileObjIdx);
		else
			fsfx_triggersfx(FSFX_OBJECT_EXPLOSION_SLOT, projectileObjIdx);
	}
}

// FUNCTION: XW 0x4046D0
int16_t collide_damagecraft(uint16_t victimObjIdx, int16_t hitMeshIndex, uint16_t sourceObjOrMissionPointRef,
							uint16_t shieldSide) {
	uint8_t playGenericImpactSound = 1;
	uint8_t hudStateChanged = 0;
	CraftData* victimCraft = (CraftData*)g_objectTable[victimObjIdx].instanceData;
	int16_t damageAmount;
#ifdef XW_MODERN
	int16_t finalDamagePastShields = 0;
#endif
	uint16_t sourceObjectType;
	uint16_t victimObjectType;
	uint16_t meshCount;
	int16_t componentRollRate;
	int16_t componentYawOffset;
	g_curCraft = victimCraft;
	if (sourceObjOrMissionPointRef == XW_OBJECT_SLOT_UNAVAILABLE) {
#ifdef XW_MODERN
		sourceObjectType = XwFlightTypes_ObjectType(XW_OBJ_IMPERIAL_STAR_DESTROYER);
#else
		sourceObjectType = XW_OBJ_IMPERIAL_STAR_DESTROYER;
#endif
		damageAmount = COLLIDE_LETHAL_DAMAGE;
	} else if (sourceObjOrMissionPointRef >= XW_MISSION_OBJECT_REF_BASE) {
		int16_t extent;
		sourceObjectType =
			g_missionObjects[sourceObjOrMissionPointRef - XW_MISSION_OBJECT_REF_BASE].objectType;
		extent = g_modelTypeTable[sourceObjectType].maxBoundsExtent;
		if (extent >= COLLIDE_STATIC_LETHAL_EXTENT) {
			damageAmount = COLLIDE_LETHAL_DAMAGE;
		} else {
			damageAmount = COLLIDE_CAPITAL_EXTENT_MULTIPLIER * extent;
		}
	} else {
		sourceObjectType = g_objectTable[sourceObjOrMissionPointRef].objectType;
		damageAmount = g_objectTable[sourceObjOrMissionPointRef].damageAmount;
	}
	if (g_objectTable[victimObjIdx].genusId == XW_GENUS_STARSHIP) {
		damageAmount >>= COLLIDE_STARSHIP_DAMAGE_SHIFT;
	}
	if (g_objectTable[victimObjIdx].genusId == XW_GENUS_FREIGHTER) {
		damageAmount >>= COLLIDE_FREIGHTER_DAMAGE_SHIFT;
	}
#ifdef XW_MODERN
	if (XwFlightTypes_CanonicalType(g_objectTable[victimObjIdx].objectType) ==
			XW_OBJ_IMPERIAL_STAR_DESTROYER ||
		XwFlightTypes_CanonicalType(g_objectTable[victimObjIdx].objectType) == XW_OBJ_CORELLIAN_CORVETTE) {
#else
	if (g_objectTable[victimObjIdx].objectType == XW_OBJ_IMPERIAL_STAR_DESTROYER ||
		g_objectTable[victimObjIdx].objectType == XW_OBJ_CORELLIAN_CORVETTE) {
#endif
		damageAmount = starship_damagecomponent(victimObjIdx, hitMeshIndex, damageAmount);
		victimCraft = g_curCraft;
	}
	if (victimCraft->shieldEnergy[shieldSide] > damageAmount) {
		victimCraft->shieldEnergy[shieldSide] -= damageAmount;
		if (victimObjIdx == g_playerFlightState.objectIndex) {
			g_flightGlobalCountdownTimers.ticks[XW_TIMER_SHIELD_FLASH] += COLLIDE_HIT_FLASH_TICKS;
			g_lastShieldDamageSide = shieldSide;
		}
	} else {
		int16_t damagePastShields;
		if (victimObjIdx == g_playerFlightState.objectIndex && shieldSide == XW_SHIELD_FRONT) {
			if ((uint16_t)math2_getrandom() < COLLIDE_SHIELD_WARNING_RANDOM_THRESHOLD) {
				fsfx_triggersfx(COLLIDE_SHIELD_WARNING_SOUND, FSFX_UNPOSITIONED_OBJECT);
			}
			victimCraft = g_curCraft;
		}
		damagePastShields = damageAmount - victimCraft->shieldEnergy[shieldSide];
#ifdef XW_MODERN
		finalDamagePastShields = damagePastShields;
#endif
		victimCraft->shieldEnergy[shieldSide] = 0;
		if (damagePastShields != 0) {
#ifdef XW_MODERN
			if (XwFlightTypes_CanonicalType(sourceObjectType) == XW_OBJ_ION_147 ||
				XwFlightTypes_CanonicalType(sourceObjectType) == XW_OBJ_ION_148) {
#else
			if (sourceObjectType == XW_OBJ_ION_147 || sourceObjectType == XW_OBJ_ION_148) {
#endif
				if (g_curCraft->workingSubsystems != 0) {
					if (damagePastShields > 0) {
						unsigned int disableCount =
							(damagePastShields + COLLIDE_ION_DAMAGE_PER_SUBSYSTEM - 1) /
							(unsigned int)COLLIDE_ION_DAMAGE_PER_SUBSYSTEM;
						unsigned int disabled;
						for (disabled = 0; disabled < disableCount; ++disabled) {
							uint16_t subsystemIndex;
							uint8_t workingSubsystems = g_curCraft->workingSubsystems;
							for (subsystemIndex = 0; subsystemIndex < XW_PLAYER_SUBSYSTEM_COUNT;
								 ++subsystemIndex) {
								uint8_t subsystemMask = g_subsystemIdToFlag[subsystemIndex];
								if ((subsystemMask & workingSubsystems) != 0) {
									hudStateChanged = 1;
									g_curCraft->workingSubsystems = workingSubsystems & ~subsystemMask;
									break;
								}
							}
#ifdef XW_MODERN
							/* The DOS exhausted loop writes two trailing words, not a ninth subsystem. */
							if (XwFlightTypes_Dos() && subsystemIndex == XW_PLAYER_SUBSYSTEM_COUNT &&
								victimObjIdx == g_playerFlightState.objectIndex) {
								Dos94_ionExhaustedHealth = 0;
								Dos94_ionExhaustedRepairTimer = 2;
							}
							if (subsystemIndex < XW_PLAYER_SUBSYSTEM_COUNT)
#endif
								if (victimObjIdx == g_playerFlightState.objectIndex) {
									uint16_t duration = g_subsystemRepairDuration[subsystemIndex];
									g_playerFlightState.subsystemHealth[subsystemIndex] = 0;
									g_playerFlightState.subsystemRepairTimers[subsystemIndex] = duration;
								}
						}
					}
					if (g_curCraft->workingSubsystems == 0) {
						uint8_t disabledType;
						msg_craftmessage(victimObjIdx, g_curCraft, XW_MSG_CRAFT_DISABLED);
						if (g_objectTable[victimObjIdx].iff == g_playerFlightState.object->iff) {
							fsfx_triggersfx(COLLIDE_FRIENDLY_LOSS_SOUND, FSFX_UNPOSITIONED_OBJECT);
						} else if (fsfx_speakeravailable() != 0) {
							fsfx_triggervoicesfx(COLLIDE_DISABLED_VOICE);
						} else {
							fsfx_triggersfx(COLLIDE_OTHER_LOSS_SOUND, FSFX_UNPOSITIONED_OBJECT);
						}
#ifdef XW_MODERN
						disabledType = XwFlightTypes_CanonicalType(g_objectTable[victimObjIdx].objectType);
#else
						disabledType = g_objectTable[victimObjIdx].objectType;
#endif
						if (disabledType >= XW_OBJ_TIE_FIGHTER && disabledType <= XW_OBJ_TIE_BOMBER) {
							g_objectTable[victimObjIdx].lifetimeTicks = COLLIDE_DISABLED_TIE_LIFETIME;
						}
					}
				}
				if (victimObjIdx == g_playerFlightState.objectIndex) {
					fsfx_triggersfx(COLLIDE_ION_HIT_SOUND, victimObjIdx);
				}
			} else {
				g_curCraft->hullDamage += damagePastShields;
				if (victimObjIdx == g_playerFlightState.objectIndex) {
					uint16_t* cockpitOverlayMask;
					int16_t cockpitDamageRandom;
					g_flightGlobalCountdownTimers.ticks[XW_TIMER_HULL_FLASH] += COLLIDE_HIT_FLASH_TICKS;
					cockpitOverlayMask = &g_curCraft->cockpitOverlayMask;
					cockpitDamageRandom = math2_getrandom();
					*cockpitOverlayMask &= (uint16_t)math2_getrandom() | cockpitDamageRandom;
					if (g_replayviewmode == 0) {
						panel_updatecockpitdamage();
					}
					if ((uint16_t)math2_getrandom() < COLLIDE_SUBSYSTEM_DAMAGE_RANDOM_THRESHOLD) {
						int subsystemIndex = math2_getrandom() & (XW_PLAYER_SUBSYSTEM_COUNT - 1);
						uint8_t workingSubsystems = g_curCraft->workingSubsystems;
						uint8_t subsystemMask = g_subsystemIdToFlag[subsystemIndex];
						if ((subsystemMask & workingSubsystems) != 0) {
							uint16_t duration;
							g_curCraft->workingSubsystems = workingSubsystems & ~subsystemMask;
							g_msgArgTable[1] = XW_MSG_SUBSYSTEM_DAMAGED_AND_INOPERATIVE;
							g_msgArgTable[0] = g_subsystemMessageArgById[subsystemIndex];
							msg_messageprintf(XW_MSG_SUBSYSTEM_STATUS);
							duration = g_subsystemRepairDuration[subsystemIndex];
							g_playerFlightState.subsystemHealth[subsystemIndex] = 0;
							g_playerFlightState.subsystemRepairTimers[subsystemIndex] = duration;
						}
					}
				}
			}
			if (g_curCraft->hullDamage >= g_curCraft->systemDamageHullThreshold) {
				int16_t failedHudFeatureMask =
					g_subsystemFailureHudMaskByRandomSlot[math2_getrandom() &
														  (XW_HUD_FAILURE_RANDOM_SLOT_COUNT - 1)];
				if (g_missionRuntimeState.provingGroundsActive == 0 ||
					failedHudFeatureMask != COLLIDE_COURSE_REQUIRED_HUD_FEATURE) {
					g_curCraft->activeHudFeatureMask &= ~(uint8_t)failedHudFeatureMask;
					if (victimObjIdx == g_playerFlightState.objectIndex) {
						fsfx_triggersfx(COLLIDE_SYSTEM_HIT_SOUND, victimObjIdx);
						playGenericImpactSound = 0;
					}
					hudStateChanged = 1;
				}
			} else if (victimObjIdx == g_playerFlightState.objectIndex) {
				fsfx_triggersfx(COLLIDE_HULL_HIT_SOUND, victimObjIdx);
				playGenericImpactSound = 0;
			}
			if (hudStateChanged != 0 && victimObjIdx == g_playerFlightState.objectIndex &&
				g_replayviewmode == 0) {
				panel_UpdateCraftSystemStatusIndicators();
			}
		}
	}

	if (g_curCraft->objectKind != XW_CRAFT_OBJECT_KIND_0 || g_curCraft->hullDamage < g_curCraft->hullMax) {
		return playGenericImpactSound;
	}
	if (sourceObjOrMissionPointRef < XW_MISSION_OBJECT_REF_BASE) {
		if (g_objectTable[sourceObjOrMissionPointRef].familyId != XW_OBJECT_FAMILY_CRAFT) {
			collide_updatekills(g_objectTable[sourceObjOrMissionPointRef].sourceObjectRef, victimObjIdx, 1);
		} else {
			collide_updatekills(sourceObjOrMissionPointRef, victimObjIdx, 1);
		}
	}
	if (g_missionRuntimeState.provingGroundsActive != 0) {
		user_checkreplaycamera();
		g_missionRuntimeState.flightExitRequested = 1;
		g_missionRuntimeState.flightExitReason = MISSION_GOAL_EVALUATION_EXIT_REASON;
	} else if ((g_curCraft->workingSubsystems & XW_CRAFT_SUBSYSTEM_2) != 0) {
		if (user_isrescued(victimObjIdx) != 0) {
			if (victimObjIdx == g_playerFlightState.objectIndex && g_replayviewmode == 0) {
				g_missionRuntimeState.flightExitReason = COLLIDE_EXIT_RESCUED;
				user_ejectcamera();
			}
			fediskio_updatepilotrecord(victimObjIdx, COLLIDE_PILOT_RESCUED, 1);
		} else {
			if (victimObjIdx == g_playerFlightState.objectIndex && g_replayviewmode == 0) {
				g_missionRuntimeState.flightExitReason = COLLIDE_EXIT_CAPTURED;
				user_ejectcamera();
			}
			fediskio_updatepilotrecord(victimObjIdx, COLLIDE_PILOT_CAPTURED, 1);
		}
	} else {
		if (victimObjIdx == g_playerFlightState.objectIndex && g_replayviewmode == 0) {
			g_missionRuntimeState.flightExitReason = COLLIDE_EXIT_KILLED;
			user_ejectcamera();
		}
		fediskio_updatepilotrecord(victimObjIdx, COLLIDE_PILOT_KILLED, 1);
	}
	{
		CraftData* destroyedCraft = g_curCraft;
		int flightGroupIndex = destroyedCraft->flightGroupIndex;
		++g_missionFlightGroupStates[flightGroupIndex].destroyedCount;
		if (g_missionFlightGroups[flightGroupIndex].specialCraftIndex ==
			destroyedCraft->craftIndexInFlightGroup) {
			g_missionFlightGroupStates[flightGroupIndex].specialCraftDestroyed = 1;
		}
		msg_craftmessage(victimObjIdx, destroyedCraft, XW_MSG_CRAFT_DESTROYED);
	}
	if (g_objectTable[victimObjIdx].iff == g_playerFlightState.object->iff) {
		fsfx_triggersfx(COLLIDE_FRIENDLY_LOSS_SOUND, FSFX_UNPOSITIONED_OBJECT);
	} else if (g_objectTable[victimObjIdx].genusId != XW_GENUS_STARFIGHTER) {
		if (fsfx_speakeravailable() != 0) {
			fsfx_triggervoicesfx(COLLIDE_DESTROYED_VOICE);
		} else {
			fsfx_triggersfx(COLLIDE_OTHER_LOSS_SOUND, FSFX_UNPOSITIONED_OBJECT);
		}
	}
	if (g_flightAudioMode != 0) {
		uint8_t iff = g_objectTable[victimObjIdx].iff;
		if (iff == 0 && g_curCraft->flightGroupIndex == g_playerFlightState.craft->flightGroupIndex) {
			fscript_DispatchMusicEvent(COLLIDE_MUSIC_WINGMAN_LOSS);
		} else if (sourceObjOrMissionPointRef < XW_MISSION_OBJECT_REF_BASE &&
				   g_objectTable[sourceObjOrMissionPointRef].sourceObjectRef ==
					   g_playerFlightState.objectIndex) {
			if (iff == g_playerFlightState.object->iff) {
				fscript_DispatchMusicEvent(COLLIDE_MUSIC_FRIENDLY_KILL);
			} else {
				fscript_DispatchMusicEvent(COLLIDE_MUSIC_ENEMY_KILL);
			}
		}
	}
	victimObjectType = g_objectTable[victimObjIdx].objectType;
	if ((uint16_t)g_modelTypeTable[victimObjectType].maxBoundsExtent > COLLIDE_CRAFT_LARGE_EXTENT) {
		uint16_t craftType = g_curCraft->craftTypeIndex;
		uint16_t rollRate = (math2_getrandom() & COLLIDE_TUMBLE_RANDOM_MASK) + COLLIDE_TUMBLE_MIN;
		uint16_t maxTumbleAngle = g_craftTypeDefs[craftType].maxTumbleAngle;
		while (rollRate > maxTumbleAngle) {
			rollRate >>= 1;
		}
#ifdef XW_MODERN
		if ((XwFlightTypes_Dos() ? (uint16_t)finalDamagePastShields : victimObjIdx) <
			COLLIDE_ANGLE_SIGN_BIT) {
#else
		if (victimObjIdx < COLLIDE_ANGLE_SIGN_BIT) {
#endif
			rollRate = -rollRate;
		}
		g_objectTable[victimObjIdx].rollImpulseRate = rollRate;
		g_curCraft->objectKind = XW_CRAFT_OBJECT_KIND_3;
		g_objectTable[victimObjIdx].lifetimeTicks =
			XW_SIMULATION_TICKS_PER_SECOND *
			((math2_getrandom() & COLLIDE_LARGE_LIFETIME_RANDOM_MASK) + COLLIDE_LARGE_LIFETIME_MIN);
		return playGenericImpactSound;
	}
	if (victimObjIdx == g_playerFlightState.objectIndex &&
		(g_playerFlightState.craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_2) == 0) {
#ifdef XW_MODERN
		g_objectTable[victimObjIdx].objectType =
			XwFlightTypes_ObjectType((math2_getrandom() & 1) + XW_OBJ_EXPLOSION_133);
#else
		g_objectTable[victimObjIdx].objectType = (math2_getrandom() & 1) + XW_OBJ_EXPLOSION_133;
#endif
#ifdef XW_MODERN
		XwFlightIntegration_Reset(victimObjIdx);
		XwRenderObjects_ReplaceMobile(victimObjIdx);
#endif
		g_objectTable[victimObjIdx].genusId = XW_GENUS_EXPLOSION_EFFECT;
		g_objectTable[victimObjIdx].familyId = XW_OBJECT_FAMILY_5;
		g_objectTable[victimObjIdx].animationState = XW_OBJECT_ANIMATION_BREAKUP;
		g_objectTable[victimObjIdx].billboardScaleCode = COLLIDE_PLAYER_EXPLOSION_SCALE;
		g_objectTable[victimObjIdx].speed = 0;
		g_objectTable[victimObjIdx].ageSeconds = 0;
		g_objectTable[victimObjIdx].lifetimeTicks = 0;
		g_objectTable[victimObjIdx].roll = 0;
		g_objectTable[victimObjIdx].rollImpulseRate = 0;
		g_objectTable[victimObjIdx].orientMatrixDirty = 1;
		fsfx_triggersfx(COLLIDE_CRAFT_EXPLOSION_SOUND, victimObjIdx);
		g_curCraft->objectKind = XW_CRAFT_OBJECT_KIND_4;
		return 0;
	}
	if ((uint16_t)g_modelTypeTable[sourceObjectType].maxBoundsExtent > COLLIDE_CRAFT_LARGE_EXTENT ||
		g_objectTable[victimObjIdx].speed == 0) {
		g_objectTable[victimObjIdx].lifetimeTicks = 1;
		fsfx_triggersfx(COLLIDE_CRAFT_EXPLOSION_SOUND, victimObjIdx);
		g_curCraft->objectKind = XW_CRAFT_OBJECT_KIND_4;
		return 0;
	}
	if ((uint16_t)math2_getrandom() < COLLIDE_SUBSYSTEM_DAMAGE_RANDOM_THRESHOLD &&
		victimObjIdx != g_playerFlightState.objectIndex) {
		g_objectTable[victimObjIdx].lifetimeTicks = 1;
		fsfx_triggersfx(COLLIDE_CRAFT_EXPLOSION_SOUND, victimObjIdx);
		g_curCraft->objectKind = XW_CRAFT_OBJECT_KIND_4;
		return 0;
	}
#ifdef XW_MODERN
	meshCount = XwFlightTypes_ComponentCount(victimObjectType);
#else
	meshCount = ModelMesh_GetCachedObjectTypeMeshCount(victimObjectType);
#endif
	componentRollRate = 0;
	componentYawOffset = 0;
	if (meshCount > 1) {
		uint16_t detachedMesh;
		uint16_t reverseTumble = 0;
		if (meshCount == COLLIDE_THREE_MESH_COUNT) {
			detachedMesh = (math2_getrandom() & 1) + 1;
			reverseTumble = detachedMesh == 1;
		} else if (meshCount == COLLIDE_FOUR_MESH_COUNT) {
			detachedMesh = (math2_getrandom() & 3) + 1;
			if (detachedMesh >= COLLIDE_FOUR_MESH_COUNT) {
				detachedMesh = 1;
			}
			reverseTumble = detachedMesh == 1;
#ifdef XW_MODERN
		} else if (XwFlightTypes_CanonicalType(victimObjectType) == XW_OBJ_B_WING &&
				   meshCount == COLLIDE_BWING_MESH_COUNT) {
#else
		} else if (victimObjectType == XW_OBJ_B_WING && meshCount == COLLIDE_BWING_MESH_COUNT) {
#endif
			detachedMesh = (math2_getrandom() & 3) + CREATE_DETACH_BWING_FIRST_MESH;
			if (detachedMesh >= COLLIDE_BWING_MESH_COUNT) {
				detachedMesh = CREATE_DETACH_BWING_FIRST_MESH;
			}
			reverseTumble = detachedMesh == COLLIDE_BWING_REVERSE_MESH;
		} else {
			detachedMesh = (math2_getrandom() & 3) + 1;
			reverseTumble = detachedMesh <= COLLIDE_THREE_MESH_COUNT;
		}
		if (g_curCraft->componentState[detachedMesh] == 0) {
			uint16_t componentObject = create_createcomponent(victimObjIdx, (char)detachedMesh);
			if (componentObject != XW_OBJECT_SLOT_UNAVAILABLE) {
				componentRollRate = (math2_getrandom() & CREATE_DETACH_ROLL_MASK) + CREATE_DETACH_MIN_ROLL;
				componentYawOffset = (math2_getrandom() & CREATE_DETACH_YAW_MASK) + COLLIDE_COMPONENT_YAW_MIN;
				if (reverseTumble != 0) {
					componentRollRate = -componentRollRate;
					componentYawOffset = -componentYawOffset;
				}
				g_objectTable[componentObject].rollImpulseRate = componentRollRate;
				g_objectTable[componentObject].yaw += componentYawOffset;
				g_objectTable[componentObject].orientMatrixDirty = 1;
				g_objectTable[componentObject].moveVectorDirty = 1;
				g_objectTable[componentObject].secondaryAnimationState = XW_OBJECT_ANIMATION_BREAKUP;
				g_curCraft->componentState[detachedMesh] = CREATE_DETACH_STATE;
				fsfx_triggersfx(COLLIDE_COMPONENT_DETACH_SOUND, victimObjIdx);
				playGenericImpactSound = 0;
			}
		}
	}
	{
		uint16_t craftType = g_curCraft->craftTypeIndex;
		uint16_t rollRate = (math2_getrandom() & COLLIDE_TUMBLE_RANDOM_MASK) + COLLIDE_TUMBLE_MIN;
		uint16_t maxTumbleAngle = g_craftTypeDefs[craftType].maxTumbleAngle;
		while (rollRate > maxTumbleAngle) {
			rollRate >>= 1;
		}
		if ((uint16_t)componentRollRate < COLLIDE_ANGLE_SIGN_BIT) {
			rollRate = -rollRate;
		}
		g_objectTable[victimObjIdx].rollImpulseRate = rollRate;
	}
	if ((uint16_t)componentYawOffset != 0) {
		g_objectTable[victimObjIdx].yaw -= (uint16_t)componentYawOffset >> 1;
		g_objectTable[victimObjIdx].orientMatrixDirty = 1;
		g_objectTable[victimObjIdx].moveVectorDirty = 1;
	}
	g_curCraft->objectKind = XW_CRAFT_OBJECT_KIND_3;
	g_objectTable[victimObjIdx].lifetimeTicks =
		XW_SIMULATION_TICKS_PER_SECOND * ((math2_getrandom() & COLLIDE_CRAFT_LIFETIME_RANDOM_MASK) + 1);
	if (victimObjIdx == g_playerFlightState.objectIndex) {
		g_objectTable[victimObjIdx].lifetimeTicks =
			XW_SIMULATION_TICKS_PER_SECOND *
			((math2_getrandom() & COLLIDE_PLAYER_LIFETIME_RANDOM_MASK) + COLLIDE_PLAYER_LIFETIME_MIN);
	}
#ifdef XW_MODERN
	if (XwFlightTypes_Dos())
		meshCount = Dos94Assets_Model(victimObjectType)->metadata.components[0].nextChildIndex;
	g_curCraft->componentState[meshCount] = CREATE_DETACH_STATE;
#else
	g_curCraft->componentState[meshCount] = CREATE_DETACH_STATE;
#endif
	return playGenericImpactSound;
}

// FUNCTION: XW 0x405190
void collide_makeobjectexplosion(uint16_t objectIndex, uint8_t explosionObjectType) {
#ifdef XW_MODERN
	XwFlightIntegration_Reset(objectIndex);
	XwRenderObjects_ReplaceMobile(objectIndex);
#endif
	g_objectTable[objectIndex].objectType = explosionObjectType;
	g_objectTable[objectIndex].genusId = XW_GENUS_EXPLOSION_EFFECT;
	g_objectTable[objectIndex].familyId = XW_OBJECT_FAMILY_5;
	g_objectTable[objectIndex].animationState = XW_OBJECT_ANIMATION_BREAKUP;
	g_objectTable[objectIndex].billboardScaleCode = 0;
	g_objectTable[objectIndex].speed = 0;
	g_objectTable[objectIndex].ageSeconds = 0;
	g_objectTable[objectIndex].lifetimeTicks = 0;
	g_objectTable[objectIndex].roll = 0;
	g_objectTable[objectIndex].rollImpulseRate = 0;
	g_objectTable[objectIndex].orientMatrixDirty = 1;
	fsfx_triggersfx(FSFX_OBJECT_EXPLOSION_SLOT, objectIndex);
}

// FUNCTION: XW 0x405210
unsigned int collide_roughdistance3du(unsigned int absDx, unsigned int absDy, unsigned int absDz) {
	if (absDx > absDy && absDx > absDz) {
		return absDx + (absDy >> COLLIDE_ROUGH_DISTANCE_SECONDARY_SHIFT) +
			   (absDz >> COLLIDE_ROUGH_DISTANCE_SECONDARY_SHIFT);
	}
	if (absDy > absDx && absDy > absDz) {
		return absDy + (absDz >> COLLIDE_ROUGH_DISTANCE_SECONDARY_SHIFT) +
			   (absDx >> COLLIDE_ROUGH_DISTANCE_SECONDARY_SHIFT);
	}
	return absDz + (absDy >> COLLIDE_ROUGH_DISTANCE_SECONDARY_SHIFT) +
		   (absDx >> COLLIDE_ROUGH_DISTANCE_SECONDARY_SHIFT);
}

// FUNCTION: XW 0x405260
int collide_roughdistance3d(int dx, int dy, int dz) {
	int absDx;
	int absDy;
	int absDz;

	absDx = dx;
	if (absDx < 0) {
		absDx = (int)(0u - (unsigned int)absDx);
	}
	absDy = dy;
	if (absDy < 0) {
		absDy = (int)(0u - (unsigned int)absDy);
	}
	absDz = dz;
	if (absDz < 0) {
		absDz = (int)(0u - (unsigned int)absDz);
	}

	if (absDx > absDy && absDx > absDz) {
		return (int)((unsigned int)absDx + (unsigned int)(absDy >> COLLIDE_ROUGH_DISTANCE_SECONDARY_SHIFT) +
					 (unsigned int)(absDz >> COLLIDE_ROUGH_DISTANCE_SECONDARY_SHIFT));
	}
	if (absDy > absDx && absDy > absDz) {
		return (int)((unsigned int)absDy + (unsigned int)(absDx >> COLLIDE_ROUGH_DISTANCE_SECONDARY_SHIFT) +
					 (unsigned int)(absDz >> COLLIDE_ROUGH_DISTANCE_SECONDARY_SHIFT));
	}
	return (int)((unsigned int)absDz + (unsigned int)(absDx >> COLLIDE_ROUGH_DISTANCE_SECONDARY_SHIFT) +
				 (unsigned int)(absDy >> COLLIDE_ROUGH_DISTANCE_SECONDARY_SHIFT));
}

// FUNCTION: XW 0x4052C0
void collide_updatekills(uint16_t shooterObjectIndex, uint16_t victimObjectIndex, int16_t spaceObject) {
	if (shooterObjectIndex < XW_CRAFT_OBJECT_COUNT) {
		CraftData* shooterCraft = g_objectTable[shooterObjectIndex].instanceData;
		if (victimObjectIndex != COLLIDE_STATIC_VICTIM) {
			unsigned int victimType = g_objectTable[victimObjectIndex].objectType;
			uint16_t typeCategory = spec_getstatisticscategory(victimType);
			uint8_t* missionKillCount;
			uint8_t newMissionCount;
			if (++shooterCraft->killStats.spacecraftByType[typeCategory] == 0)
				shooterCraft->killStats.spacecraftByType[typeCategory] = COLLIDE_KILL_COUNT_SATURATION;
			if (shooterObjectIndex == g_playerFlightState.objectIndex) {
				if ((uint16_t)math2_getrandom() < COLLIDE_KILL_VOICE_RANDOM_THRESHOLD &&
					fsfx_speakeravailable() != 0)
					fsfx_triggervoicesfx(FSFX_KILL_VOICE_SLOT);
				if (++g_playerFlightState.spacecraftKillsByType[typeCategory] == 0)
					g_playerFlightState.spacecraftKillsByType[typeCategory] = COLLIDE_KILL_COUNT_SATURATION;
			}
			missionKillCount =
				&g_missionRuntimeState
					 .destroyedCountsByIffAndCategory[g_objectTable[victimObjectIndex].iff][typeCategory];
			newMissionCount = *missionKillCount + 1;
			*missionKillCount = newMissionCount;
			if (newMissionCount == 0)
				*missionKillCount = COLLIDE_KILL_COUNT_SATURATION;
		} else if (spaceObject == 0) {
			++shooterCraft->killStats.deathStarBuildings;
			if (shooterObjectIndex == g_playerFlightState.objectIndex)
				++g_playerFlightState.deathStarBuildingKills;
		} else {
			++shooterCraft->killStats.spaceObjects;
			if (shooterObjectIndex == g_playerFlightState.objectIndex)
				++g_playerFlightState.spaceObjectKills;
		}
	}
}

// FUNCTION: XW 0x4053F0
void collide_updatehits(uint16_t projectileObjIdx, int16_t spacecraftHit) {
	uint16_t sourceObjectRef = g_objectTable[projectileObjIdx].sourceObjectRef;
	uint16_t projectileType = g_objectTable[projectileObjIdx].objectType;
	if (sourceObjectRef < XW_CRAFT_OBJECT_COUNT) {
		CraftData* sourceCraft = g_objectTable[sourceObjectRef].instanceData;
#ifdef XW_MODERN
		switch (XwFlightTypes_CanonicalType(projectileType)) {
#else
		switch (projectileType) {
#endif
			case XW_OBJ_LASER_143:
			case XW_OBJ_LASER_144:
			case XW_OBJ_LASER_145:
			case XW_OBJ_LASER_146:
				if (spacecraftHit == 0) {
					++sourceCraft->weaponStats.laserSurfaceHits;
					if (sourceObjectRef == g_playerFlightState.objectIndex)
						++g_playerFlightState.weaponStats.laserSurfaceHits;
				} else {
					++sourceCraft->weaponStats.laserSpacecraftHits;
					if (sourceObjectRef == g_playerFlightState.objectIndex)
						++g_playerFlightState.weaponStats.laserSpacecraftHits;
				}
				break;
			case XW_OBJ_ION_147:
			case XW_OBJ_ION_148:
				if (spacecraftHit == 0) {
					++sourceCraft->weaponStats.ionSurfaceHits;
					if (sourceObjectRef == g_playerFlightState.objectIndex)
						++g_playerFlightState.weaponStats.ionSurfaceHits;
				} else {
					++sourceCraft->weaponStats.ionSpacecraftHits;
					if (sourceObjectRef == g_playerFlightState.objectIndex)
						++g_playerFlightState.weaponStats.ionSpacecraftHits;
				}
				break;
			case XW_OBJ_WARHEAD_149:
			case XW_OBJ_TRACKED_WARHEAD:
				if (spacecraftHit == 0) {
					++sourceCraft->weaponStats.warheadSurfaceHits;
					if (sourceObjectRef == g_playerFlightState.objectIndex)
						++g_playerFlightState.weaponStats.warheadSurfaceHits;
				} else {
					++sourceCraft->weaponStats.warheadSpacecraftHits;
					if (sourceObjectRef == g_playerFlightState.objectIndex)
						++g_playerFlightState.weaponStats.warheadSpacecraftHits;
				}
				break;
		}
	}
}
