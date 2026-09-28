#include "xw/flight/ai/paifight.h"

#ifdef XW_MODERN
#include "xw_runtime/runtime/flight_types.h"
#endif

#include "xw/assets/model_mesh.h"
#include "xw/flight/ai/pai.h"
#include "xw/flight/ai/paiman.h"
#include "xw/flight/ai/paiorder.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/collide.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/laser.h"
#include "xw/flight/player/user.h"
#include "xw/flight/xw.h"
#include "xw/math/math2.h"
#include "xw/math/trig2.h"

#include <limits.h>
#include <stdlib.h>

// GLOBAL: XW 0x4C9000
unsigned int g_aiFighterShootMaxRangeBySkill[PAI_PROFICIENCY_COUNT] = { 0x8000, 0xB000, 0xE000 };

// GLOBAL: XW 0x4C900C
uint8_t g_aiFighterShootBurstCountBySkill[PAI_PROFICIENCY_COUNT] = { 3, 4, 5 };

// FUNCTION: XW 0x414BA0
int16_t paifight_SelectTurretTargets(void) {
	uint16_t weaponSlotIndex;
	uint16_t candidateIndex;
	uint16_t targetPlusOne;
	uint16_t primaryTarget, secondaryTarget;
	uint16_t candidateFlightGroup;
	uint8_t opposingIff;
	uint32_t bestRange, range;
	int32_t muzzleWorldX, muzzleWorldY, muzzleWorldZ;
	for (weaponSlotIndex = 0; weaponSlotIndex < g_curCraft->laserSlotCount; ++weaponSlotIndex) {
		if (g_curCraft->weaponSlots[weaponSlotIndex].firingGate == 0)
			continue;
		g_curCraft->weaponSlots[weaponSlotIndex].firingGate = PAIFIGHT_TURRET_NO_TARGET;
		if (g_curCraft->objectKind == XW_CRAFT_OBJECT_KIND_3 || g_curCraft->workingSubsystems == 0)
			continue;
		muzzleWorldX = g_objectTable[g_paiObjectIndex].worldX;
		muzzleWorldY = g_objectTable[g_paiObjectIndex].worldY;
		muzzleWorldZ = g_objectTable[g_paiObjectIndex].worldZ;
		pai_calcrotatedpoint(&g_objectTable[g_paiObjectIndex],
							 g_craftTypeDefs[g_curCraft->craftTypeIndex].weaponHardpoints[weaponSlotIndex].x,
							 g_craftTypeDefs[g_curCraft->craftTypeIndex].weaponHardpoints[weaponSlotIndex].z,
							 g_craftTypeDefs[g_curCraft->craftTypeIndex].weaponHardpoints[weaponSlotIndex].y);
#ifdef XW_MODERN
		muzzleWorldX = (int32_t)((uint32_t)muzzleWorldX + (uint32_t)g_rotatedX);
#else
		muzzleWorldX += g_rotatedX;
#endif
#ifdef XW_MODERN
		muzzleWorldZ = (int32_t)((uint32_t)muzzleWorldZ + (uint32_t)g_rotatedZ);
#else
		muzzleWorldZ += g_rotatedZ;
#endif
#ifdef XW_MODERN
		muzzleWorldY = (int32_t)((uint32_t)muzzleWorldY + (uint32_t)g_rotatedY);
#else
		muzzleWorldY += g_rotatedY;
#endif
		opposingIff = g_objectTable[g_paiObjectIndex].iff ^ XW_OBJECT_OPPOSING_IFF_MASK;
		bestRange = UINT_MAX;
		targetPlusOne = 0;
		for (candidateIndex = 0; candidateIndex < XW_CRAFT_OBJECT_COUNT; ++candidateIndex) {
			if (g_objectTable[candidateIndex].objectType != XW_OBJ_NONE &&
				g_objectTable[candidateIndex].iff == opposingIff &&
				g_objectTable[candidateIndex].genusId == XW_GENUS_STARSHIP) {
#ifdef XW_MODERN
				range = collide_roughdistance3d(
					(int32_t)((uint32_t)g_objectTable[candidateIndex].worldX - (uint32_t)muzzleWorldX),
					(int32_t)((uint32_t)g_objectTable[candidateIndex].worldY - (uint32_t)muzzleWorldY),
					(int32_t)((uint32_t)g_objectTable[candidateIndex].worldZ - (uint32_t)muzzleWorldZ));
#else
				range = collide_roughdistance3d(g_objectTable[candidateIndex].worldX - muzzleWorldX,
												g_objectTable[candidateIndex].worldY - muzzleWorldY,
												g_objectTable[candidateIndex].worldZ - muzzleWorldZ);
#endif
				if (range < bestRange) {
					bestRange = range;
					targetPlusOne = candidateIndex + 1;
				}
			}
		}
		if (bestRange < PAIFIGHT_TURRET_CAPITAL_RANGE) {
			g_curCraft->weaponSlots[weaponSlotIndex].firingGate = targetPlusOne;
			continue;
		}
		bestRange = UINT_MAX;
		targetPlusOne = 0;
		for (candidateIndex = 0; candidateIndex < XW_CRAFT_OBJECT_COUNT; ++candidateIndex) {
			if (g_objectTable[candidateIndex].objectType != XW_OBJ_NONE &&
				g_objectTable[candidateIndex].iff == opposingIff &&
				g_objectTable[candidateIndex].genusId != XW_GENUS_STARSHIP) {
#ifdef XW_MODERN
				range = collide_roughdistance3d(
					(int32_t)((uint32_t)g_objectTable[candidateIndex].worldX - (uint32_t)muzzleWorldX),
					(int32_t)((uint32_t)g_objectTable[candidateIndex].worldY - (uint32_t)muzzleWorldY),
					(int32_t)((uint32_t)g_objectTable[candidateIndex].worldZ - (uint32_t)muzzleWorldZ));
#else
				range = collide_roughdistance3d(g_objectTable[candidateIndex].worldX - muzzleWorldX,
												g_objectTable[candidateIndex].worldY - muzzleWorldY,
												g_objectTable[candidateIndex].worldZ - muzzleWorldZ);
#endif
				if (range < bestRange) {
					bestRange = range;
					targetPlusOne = candidateIndex + 1;
				}
			}
		}
		if (bestRange < PAIFIGHT_TURRET_OTHER_RANGE) {
			g_curCraft->weaponSlots[weaponSlotIndex].firingGate = targetPlusOne;
			continue;
		}
		if (g_curCraft->aiOrderPlanId < PAIFIGHT_TURRET_GROUP_ORDER_FIRST ||
			g_curCraft->aiOrderPlanId > PAIFIGHT_TURRET_GROUP_ORDER_LAST)
			continue;
		targetPlusOne = 0;
		bestRange = UINT_MAX;
		primaryTarget = g_missionFlightGroups[g_curCraft->flightGroupIndex].primaryTarget;
		secondaryTarget = g_missionFlightGroups[g_curCraft->flightGroupIndex].secondaryTarget;
		for (candidateIndex = 0; candidateIndex < XW_CRAFT_OBJECT_COUNT; ++candidateIndex) {
			if (g_objectTable[candidateIndex].objectType != XW_OBJ_NONE) {
				candidateFlightGroup =
					((CraftData*)g_objectTable[candidateIndex].instanceData)->flightGroupIndex;
				if (candidateFlightGroup == primaryTarget || candidateFlightGroup == secondaryTarget) {
#ifdef XW_MODERN
					range = collide_roughdistance3d(
						(int32_t)((uint32_t)g_objectTable[candidateIndex].worldX - (uint32_t)muzzleWorldX),
						(int32_t)((uint32_t)g_objectTable[candidateIndex].worldY - (uint32_t)muzzleWorldY),
						(int32_t)((uint32_t)g_objectTable[candidateIndex].worldZ - (uint32_t)muzzleWorldZ));
#else
					range = collide_roughdistance3d(g_objectTable[candidateIndex].worldX - muzzleWorldX,
													g_objectTable[candidateIndex].worldY - muzzleWorldY,
													g_objectTable[candidateIndex].worldZ - muzzleWorldZ);
#endif
					if (range < bestRange) {
						bestRange = range;
						targetPlusOne = candidateIndex + 1;
					}
				}
			}
		}
		if (bestRange < PAIFIGHT_TURRET_OTHER_RANGE) {
			g_curCraft->weaponSlots[weaponSlotIndex].firingGate = targetPlusOne;
		}
	}
	return 0;
}

// FUNCTION: XW 0x414EC0
int16_t paifight_scanfortargetorder(void) {
	uint16_t orderPlan;
	uint16_t candidateTarget;
	int requireWorkingSubsystems;
	if (g_curCraft->aiManeuverId != g_paiInitialManeuverId)
		return 0;
	candidateTarget = g_curCraft->aiCandidateTargetOrSavedInterval;
	if (candidateTarget != PAI_TARGET_NONE && candidateTarget != PAI_TARGET_EVASIVE) {
		if (pai_worthytarget(candidateTarget) != 0) {
			g_curCraft->aiTargetRef = candidateTarget;
			return 1;
		}
		g_curCraft->aiCandidateTargetOrSavedInterval = PAI_TARGET_NONE;
	}
	orderPlan = g_curCraft->aiOrderPlanId;
	requireWorkingSubsystems = orderPlan >= PAI_PLAN_DISABLE_FIRST && orderPlan <= PAI_PLAN_DISABLE_LAST;
	if (orderPlan != PAI_PLAN_FIGHTER_TARGETS) {
		if (paifight_findtargetingroup(g_missionFlightGroups[g_curCraft->flightGroupIndex].primaryTarget,
									   requireWorkingSubsystems) != 0)
			return 1;
		if (paifight_findtargetingroup(g_missionFlightGroups[g_curCraft->flightGroupIndex].secondaryTarget,
									   requireWorkingSubsystems) != 0)
			return 1;
		switch (orderPlan) {
			case PAI_PLAN_FIGHTER_OR_TRANSPORT_TARGETS:
			case PAI_PLAN_DISABLE_FIGHTER_OR_TRANSPORT_TARGETS:
				if (paifight_FindNearestTargetOfGenus(XW_GENUS_STARFIGHTER, requireWorkingSubsystems) != 0)
					return 1;
				/* Fall through to transport targets when no fighter is available. */
			case PAI_PLAN_TRANSPORT_TARGETS:
			case PAI_PLAN_DISABLE_TRANSPORT_TARGETS:
				if (paifight_FindNearestTargetOfGenus(XW_GENUS_TRANSPORT, requireWorkingSubsystems) != 0)
					return 1;
				break;
			case PAI_PLAN_FREIGHTER_TARGETS:
			case PAI_PLAN_DISABLE_FREIGHTER_TARGETS:
				if (paifight_FindNearestTargetOfGenus(XW_GENUS_FREIGHTER, requireWorkingSubsystems) != 0)
					return 1;
				break;
			case PAI_PLAN_STARSHIP_TARGETS:
			case PAI_PLAN_DISABLE_STARSHIP_TARGETS:
				if (paifight_FindNearestTargetOfGenus(XW_GENUS_STARSHIP, requireWorkingSubsystems) != 0)
					return 1;
				break;
		}
	} else {
		if (paifight_findescorterofgroup(g_missionFlightGroups[g_curCraft->flightGroupIndex].primaryTarget) !=
			0)
			return 1;
		if (paifight_findescorterofgroup(
				g_missionFlightGroups[g_curCraft->flightGroupIndex].secondaryTarget) != 0)
			return 1;
	}
	return 0;
}

// FUNCTION: XW 0x415090
int16_t paifight_findtargetingroup(int16_t flightGroupIndex, int16_t requireWorkingSubsystems) {
	if ((uint16_t)flightGroupIndex != PAIFIGHT_NO_OBJECT) {
		unsigned int bestRangeScore = UINT_MAX;
		uint16_t bestObjectIndex = PAIFIGHT_NO_OBJECT;
		uint16_t objectIndex;
		for (objectIndex = 0; objectIndex < XW_CRAFT_OBJECT_COUNT; ++objectIndex) {
			if (g_objectTable[objectIndex].objectType != XW_OBJ_NONE) {
				CraftData* candidateCraft = g_objectTable[objectIndex].instanceData;
				if (candidateCraft->flightGroupIndex == flightGroupIndex &&
					pai_checktargetforattack(g_paiObjectIndex, objectIndex, 0) != 0 &&
					(requireWorkingSubsystems == 0 || candidateCraft->workingSubsystems != 0) &&
					(uint16_t)paifight_countattackers(objectIndex) != 0) {
					pai_roughdistancebetween(g_paiObjectIndex, objectIndex);
					if (g_targetRangeScore < bestRangeScore) {
						bestRangeScore = g_targetRangeScore;
						bestObjectIndex = objectIndex;
					}
				}
			}
		}
		if (bestObjectIndex != PAIFIGHT_NO_OBJECT) {
			g_curCraft->aiTargetRef = bestObjectIndex;
			return 1;
		}
	}
	return 0;
}

// FUNCTION: XW 0x415160
int16_t paifight_FindNearestTargetOfGenus(int16_t genusId, int16_t requireWorkingSubsystems) {
	unsigned int bestRangeScore = UINT_MAX;
	uint16_t bestObjectIndex = PAIFIGHT_NO_OBJECT;
	int16_t opposingIff = g_objectTable[g_paiObjectIndex].iff ^ XW_OBJECT_OPPOSING_IFF_MASK;
	uint16_t objectIndex;
	for (objectIndex = 0; objectIndex < XW_CRAFT_OBJECT_COUNT; ++objectIndex) {
		if (g_objectTable[objectIndex].objectType != XW_OBJ_NONE &&
			g_objectTable[objectIndex].iff == opposingIff && g_objectTable[objectIndex].genusId == genusId) {
			const CraftData* candidateCraft = g_objectTable[objectIndex].instanceData;
			if (pai_checktargetforattack(g_paiObjectIndex, objectIndex, 0) != 0 &&
				(requireWorkingSubsystems == 0 || candidateCraft->workingSubsystems != 0) &&
				(uint16_t)paifight_countattackers(objectIndex) != 0) {
				pai_roughdistancebetween(g_paiObjectIndex, objectIndex);
				if (g_targetRangeScore < bestRangeScore) {
					bestRangeScore = g_targetRangeScore;
					bestObjectIndex = objectIndex;
				}
			}
		}
	}
	if (bestObjectIndex != PAIFIGHT_NO_OBJECT) {
		g_curCraft->aiTargetRef = bestObjectIndex;
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x415250
int16_t paifight_findescorterofgroup(int16_t escortedFlightGroup) {
	uint16_t bestTargetObjIdx = PAIFIGHT_NO_OBJECT;
	if ((uint16_t)escortedFlightGroup != PAIFIGHT_NO_OBJECT) {
		unsigned int bestRangeScore = UINT_MAX;
		uint16_t candidateObjIdx;
		for (candidateObjIdx = 0; candidateObjIdx < XW_CRAFT_OBJECT_COUNT; ++candidateObjIdx) {
			if (g_objectTable[candidateObjIdx].objectType != XW_OBJ_NONE) {
				CraftData* candidateCraft = g_objectTable[candidateObjIdx].instanceData;
				if ((candidateCraft->aiOrderPlanId == PAI_PLAN_ESCORT_30 ||
					 candidateCraft->aiOrderPlanId == PAI_PLAN_ESCORT_31) &&
					candidateCraft->aiEscortTargetFlightGroup == escortedFlightGroup &&
					pai_checktargetforattack(g_paiObjectIndex, candidateObjIdx, 0) != 0 &&
					(uint16_t)paifight_countattackers(candidateObjIdx) != 0) {
					pai_roughdistancebetween(g_paiObjectIndex, candidateObjIdx);
					if (g_targetRangeScore < bestRangeScore) {
						bestRangeScore = g_targetRangeScore;
						bestTargetObjIdx = candidateObjIdx;
					}
				}
			}
		}
		if (bestTargetObjIdx != PAIFIGHT_NO_OBJECT) {
			g_curCraft->aiTargetRef = bestTargetObjIdx;
			return 1;
		}
	}
	return 0;
}

// FUNCTION: XW 0x415300
int paifight_countattackers(uint16_t targetObjIdx) {
	uint16_t attackerCount = 0;
	uint16_t attackerLimit;
	uint16_t objectIndex;
	for (objectIndex = 0; objectIndex < XW_CRAFT_OBJECT_COUNT; ++objectIndex) {
		if (g_objectTable[objectIndex].objectType != XW_OBJ_NONE) {
			CraftData* craft = g_objectTable[objectIndex].instanceData;
			if (craft->aiTargetRef == targetObjIdx && objectIndex != targetObjIdx) {
				uint8_t maneuver = craft->aiManeuverId;
				if (maneuver == PAIORDER_MANEUVER_SETUP_ATTACK || maneuver == PAIORDER_MANEUVER_ATTACK ||
					maneuver == PAIORDER_MANEUVER_ATTACK_ALTERNATE) {
					++attackerCount;
				}
			}
		}
	}
	if (g_objectTable[targetObjIdx].genusId == XW_GENUS_STARSHIP) {
		attackerLimit = PAIFIGHT_STARSHIP_ATTACKER_LIMIT;
	} else if (g_objectTable[targetObjIdx].genusId == XW_GENUS_FREIGHTER) {
		attackerLimit = PAIFIGHT_FREIGHTER_ATTACKER_LIMIT;
	} else {
		attackerLimit = g_objectTable[targetObjIdx].iff == g_objectTable[g_playerFlightState.objectIndex].iff
							? PAIFIGHT_PLAYER_IFF_ATTACKER_LIMIT
							: PAIFIGHT_OTHER_IFF_ATTACKER_LIMIT;
	}
	return attackerCount < attackerLimit;
}

// FUNCTION: XW 0x4153B0
int16_t paifight_escorttargetorder(void) {
	uint16_t objectIndex = g_paiObjectIndex;
	uint16_t candidateIndex;
	uint16_t bestTargetIndex;
	uint8_t oppositeIff;
	int16_t escortedFlightGroup;
	unsigned int bestRangeScore;
	unsigned int maxRangeScore;
	if (g_curCraft->aiManeuverId != g_paiInitialManeuverId) {
		return 0;
	}
	candidateIndex = g_curCraft->aiCandidateTargetOrSavedInterval;
	if (candidateIndex != PAI_TARGET_NONE && candidateIndex != PAI_TARGET_EVASIVE) {
		if (pai_worthytarget(candidateIndex) != 0) {
			g_curCraft->aiTargetRef = candidateIndex;
			return 1;
		}
		g_curCraft->aiCandidateTargetOrSavedInterval = PAI_TARGET_NONE;
	}
	oppositeIff = g_objectTable[objectIndex].iff ^ XW_OBJECT_OPPOSING_IFF_MASK;
	bestTargetIndex = PAIFIGHT_NO_OBJECT;
	escortedFlightGroup = g_curCraft->aiEscortTargetFlightGroup;
	maxRangeScore = g_curCraft->aiOrderPlanId == PAI_PLAN_ESCORT_30 ? PAIFIGHT_CLOSE_ESCORT_RANGE_LIMIT
																	: PAIFIGHT_ESCORT_RANGE_LIMIT;
	bestRangeScore = UINT_MAX;
	for (candidateIndex = 0; candidateIndex < XW_CRAFT_OBJECT_COUNT; ++candidateIndex) {
		if (g_objectTable[candidateIndex].objectType != XW_OBJ_NONE &&
			g_objectTable[candidateIndex].iff == oppositeIff) {
			CraftData* candidateCraft = g_objectTable[candidateIndex].instanceData;
			if (candidateIndex == g_playerFlightState.objectIndex ||
				(candidateCraft->aiLeaderObjectIndex != XW_CRAFT_NO_AI_LEADER &&
				 candidateCraft->flightGroupIndex == g_playerFlightState.craft->flightGroupIndex)) {
				if ((uint16_t)paifight_countattackers(candidateIndex) != 0) {
					pai_roughdistancebetween(objectIndex, candidateIndex);
					if (g_targetRangeScore < bestRangeScore && g_targetRangeScore < maxRangeScore) {
						bestRangeScore = g_targetRangeScore;
						bestTargetIndex = candidateIndex;
					}
				}
			} else {
				uint16_t targetIndex = candidateCraft->aiTargetRef;
				if (targetIndex < XW_OBJECT_COUNT && g_objectTable[targetIndex].objectType != XW_OBJ_NONE &&
					((CraftData*)g_objectTable[targetIndex].instanceData)->flightGroupIndex ==
						escortedFlightGroup &&
					(uint16_t)paifight_countattackers(candidateIndex) != 0) {
					pai_roughdistancebetween(objectIndex, candidateIndex);
					if (g_targetRangeScore < bestRangeScore && g_targetRangeScore < maxRangeScore) {
						bestRangeScore = g_targetRangeScore;
						bestTargetIndex = candidateIndex;
					}
				}
			}
		}
	}
	if (bestTargetIndex != PAIFIGHT_NO_OBJECT) {
		g_curCraft->aiTargetRef = bestTargetIndex;
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x415570
int16_t paifight_fightershootorder(void) {
	CraftData* currentCraft = g_curCraft;
	uint16_t target = currentCraft->aiTargetRef;
	uint16_t source = g_paiObjectIndex;
	uint16_t yawError, pitchError, headingError, order, launcherIndex, inboundCount;
#ifdef XW_MODERN
	uint16_t maxInbound, maxLaunches, desiredWarhead, burstCount = XwFlightTypes_Dos() ? 0 : source;
#else
	uint16_t maxInbound, maxLaunches, desiredWarhead, burstCount = source;
#endif
	uint32_t maxCannonRange, maxWarheadRange;
	int16_t linkMode;
	int cannonGroup, objectIndex;
	uint16_t cannonCount;
	uint16_t firstWeaponSlot;
	uint16_t clearGroup;
	WarheadGuidanceState* guidance;
	if (target < XW_CRAFT_OBJECT_COUNT && pai_worthytarget(target)) {
		maxCannonRange = g_aiFighterShootMaxRangeBySkill[g_paiSkillTier];
		headingError = g_objectTable[source].yaw - g_objectTable[g_curCraft->aiTargetRef].yaw;
		if (headingError >= PAIMAN_HALF_TURN)
			headingError = -headingError;
		if (headingError < PAIFIGHT_SHOOT_PARALLEL_ANGLE)
			maxCannonRange -= PAIFIGHT_SHOOT_PARALLEL_RANGE_REDUCTION;
		else if (headingError < PAIFIGHT_SHOOT_CROSSING_ANGLE)
			maxCannonRange -= PAIFIGHT_SHOOT_CROSSING_RANGE_REDUCTION;
		pai_distancebetween(source, target);
		yawError = g_trig2Yaw - g_objectTable[source].yaw;
		if (yawError >= PAIMAN_HALF_TURN)
			yawError = -yawError;
		currentCraft = g_curCraft;
		pitchError = g_trig2Pitch - currentCraft->pitch;
		if (pitchError >= PAIMAN_HALF_TURN)
			pitchError = -pitchError;
		if (yawError < PAIFIGHT_SHOOT_CANNON_ANGLE && pitchError < PAIFIGHT_SHOOT_CANNON_ANGLE &&
			(uint32_t)g_trig2PolarDistance < maxCannonRange) {
			if (g_trig2PolarDistance < PAIFIGHT_SHOOT_ALL_RANGE)
				linkMode = LASER_LINK_ALL;
			else
				linkMode = LASER_LINK_SINGLE + (g_trig2PolarDistance < PAIFIGHT_SHOOT_PAIR_RANGE);
			burstCount = g_aiFighterShootBurstCountBySkill[g_paiSkillTier];
		} else {
			linkMode = 0;
		}
		order = currentCraft->aiOrderPlanId;
		cannonCount = currentCraft->cannonClassCount;
		for (cannonGroup = 0; cannonGroup < cannonCount; ++cannonGroup) {
#ifdef XW_MODERN
			if (linkMode != 0 && ((currentCraft->laserState.projectileTypeId[cannonGroup] ==
									   XwFlightTypes_ObjectType(XW_OBJ_ION_147) &&
								   order >= PAI_PLAN_DISABLE_FIRST && order <= PAI_PLAN_DISABLE_LAST) ||
								  (currentCraft->laserState.projectileTypeId[cannonGroup] !=
									   XwFlightTypes_ObjectType(XW_OBJ_ION_147) &&
								   (order < PAI_PLAN_DISABLE_FIRST || order > PAI_PLAN_DISABLE_LAST))))
#else
			if (linkMode != 0 && ((currentCraft->laserState.projectileTypeId[cannonGroup] == XW_OBJ_ION_147 &&
								   order >= PAI_PLAN_DISABLE_FIRST && order <= PAI_PLAN_DISABLE_LAST) ||
								  (currentCraft->laserState.projectileTypeId[cannonGroup] != XW_OBJ_ION_147 &&
								   (order < PAI_PLAN_DISABLE_FIRST || order > PAI_PLAN_DISABLE_LAST))))
#endif
				currentCraft->laserState.linkMode[cannonGroup] = linkMode;
			else
				currentCraft->laserState.linkMode[cannonGroup] = 0;
			g_curCraft->laserState.burstShotsRemaining[cannonGroup] = burstCount;
			currentCraft = g_curCraft;
		}
		if (order >= PAI_PLAN_DISABLE_FIRST && order <= PAI_PLAN_DISABLE_LAST)
			return 0;
		if (g_objectTable[target].genusId == XW_GENUS_STARSHIP ||
			g_objectTable[target].genusId == XW_GENUS_FREIGHTER) {
#ifdef XW_MODERN
			desiredWarhead = XwFlightTypes_ObjectType(XW_OBJ_WARHEAD_149);
#else
			desiredWarhead = XW_OBJ_WARHEAD_149;
#endif
			maxLaunches = PAIFIGHT_SHOOT_CAPITAL_LAUNCH_LIMIT;
			maxInbound = PAIFIGHT_SHOOT_CAPITAL_INBOUND_LIMIT;
			maxWarheadRange = PAIFIGHT_SHOOT_CAPITAL_WARHEAD_RANGE;
		} else {
#ifdef XW_MODERN
			desiredWarhead = XwFlightTypes_ObjectType(XW_OBJ_TRACKED_WARHEAD);
#else
			desiredWarhead = XW_OBJ_TRACKED_WARHEAD;
#endif
			maxLaunches = PAIFIGHT_SHOOT_SMALL_LAUNCH_LIMIT;
			maxInbound = PAIFIGHT_SHOOT_SMALL_INBOUND_LIMIT;
			maxWarheadRange = PAIFIGHT_SHOOT_SMALL_WARHEAD_RANGE;
		}
		inboundCount = 0;
		for (objectIndex = XW_CRAFT_OBJECT_COUNT;
			 objectIndex < XW_CRAFT_OBJECT_COUNT + LASER_WARHEAD_GUIDANCE_COUNT; ++objectIndex) {
#ifdef XW_MODERN
			if (g_objectTable[objectIndex].objectType == XwFlightTypes_ObjectType(XW_OBJ_TRACKED_WARHEAD) ||
				g_objectTable[objectIndex].objectType == XwFlightTypes_ObjectType(XW_OBJ_WARHEAD_149)) {
#else
			if (g_objectTable[objectIndex].objectType == XW_OBJ_TRACKED_WARHEAD ||
				g_objectTable[objectIndex].objectType == XW_OBJ_WARHEAD_149) {
#endif
				guidance = (WarheadGuidanceState*)g_objectTable[objectIndex].instanceData;
				if (guidance->homingTier != 0) {
					if (guidance->targetObjIdx == target)
						++inboundCount;
					currentCraft = g_curCraft;
				}
			}
		}
		if (inboundCount >= maxInbound || currentCraft->aiManeuverId != PAIORDER_MANEUVER_ATTACK_ALTERNATE ||
			currentCraft->aiWarheadsFiredThisRun >= maxLaunches)
			return 0;
		if (yawError < PAIFIGHT_SHOOT_WARHEAD_ANGLE && pitchError < PAIFIGHT_SHOOT_WARHEAD_ANGLE &&
			(uint32_t)g_trig2PolarDistance < maxWarheadRange) {
			currentCraft->warheadLockTicks +=
				math2_fraction(currentCraft->aiThinkIntervalTicks, PAIFIGHT_SHOOT_LOCK_FRACTION);
			currentCraft = g_curCraft;
			if (currentCraft->warheadLockTicks <
				(int)(uint16_t)(PAIFIGHT_SHOOT_LOCK_TICKS * (g_paiSkillTier + 1)))
				return 0;
			for (launcherIndex = 0; launcherIndex < currentCraft->warheadLauncherCount; ++launcherIndex) {
				if (currentCraft->warheadFireTimers[launcherIndex] == 0 &&
					currentCraft->warheadSlotTypeIds[launcherIndex] == desiredWarhead) {
					firstWeaponSlot =
						g_craftTypeDefs[currentCraft->craftTypeIndex].warheadFirstSlot[launcherIndex];
					if (currentCraft->weaponSlots[firstWeaponSlot].count >=
						currentCraft->weaponSlots[firstWeaponSlot + 1].count)
						currentCraft->warheadLauncherFlags[launcherIndex] = PAIFIGHT_SHOOT_FIRST_LAUNCHER;
					else
						currentCraft->warheadLauncherFlags[launcherIndex] = PAIFIGHT_SHOOT_SECOND_LAUNCHER;
					laser_firerocketsystem(g_paiObjectIndex, launcherIndex);
					++g_curCraft->aiWarheadsFiredThisRun;
					if (g_objectTable[target].genusId == XW_GENUS_STARFIGHTER ||
						g_objectTable[target].genusId == XW_GENUS_TRANSPORT)
						g_curCraft->warheadLockTicks = 0;
					currentCraft = g_curCraft;
				}
			}
		} else {
			currentCraft->warheadLockTicks -= currentCraft->aiThinkIntervalTicks;
			if (g_curCraft->warheadLockTicks < 0)
				g_curCraft->warheadLockTicks = 0;
		}
	} else {
		currentCraft = g_curCraft;
		for (clearGroup = 0; clearGroup < currentCraft->cannonClassCount; ++clearGroup) {
			currentCraft->laserState.linkMode[clearGroup] = 0;
			currentCraft = g_curCraft;
		}
	}
	return 0;
}

// FUNCTION: XW 0x4159A0
int16_t paifight_coverleaderorder(void) {
	uint16_t objectIndex = g_paiObjectIndex;
	uint16_t leaderIndex = g_paiLeaderObjectIndex;
	CraftData* leaderCraft = g_objectTable[leaderIndex].instanceData;
	uint16_t attackerIndex = leaderCraft->lastAttackerObjIdx;
	if (attackerIndex != PAI_TARGET_NONE && g_objectTable[attackerIndex].objectType != XW_OBJ_NONE &&
		leaderCraft->objectKind == 0 &&
		(g_objectTable[attackerIndex].genusId == XW_GENUS_STARFIGHTER ||
		 g_objectTable[attackerIndex].genusId == XW_GENUS_TRANSPORT)) {
		g_curCraft->aiTargetRef = attackerIndex;
		return 1;
	}
	if (g_curCraft->aiOrderPlanId == PAI_PLAN_FIGHTER_OR_TRANSPORT_TARGETS) {
		uint16_t candidateIndex;
		for (candidateIndex = 0; candidateIndex < XW_CRAFT_OBJECT_COUNT; ++candidateIndex) {
			if (g_objectTable[candidateIndex].objectType != XW_OBJ_NONE &&
				(g_objectTable[candidateIndex].genusId == XW_GENUS_STARFIGHTER ||
				 g_objectTable[candidateIndex].genusId == XW_GENUS_TRANSPORT)) {
				CraftData* candidateCraft = g_objectTable[candidateIndex].instanceData;
				if (candidateCraft->aiTargetRef == leaderIndex) {
					g_curCraft->aiTargetRef = candidateIndex;
					return 1;
				}
			}
		}
		if (g_objectTable[objectIndex].iff != g_playerFlightState.object->iff) {
			pai_distancebetween(objectIndex, g_playerFlightState.objectIndex);
			if (g_trig2PolarDistance < PAIFIGHT_COVER_PLAYER_DISTANCE_LIMIT &&
				pai_worthytarget(g_playerFlightState.objectIndex) != 0) {
				g_curCraft->aiTargetRef = g_playerFlightState.objectIndex;
				return 1;
			}
		}
	}
	return 0;
}

// FUNCTION: XW 0x415B00
int16_t paifight_followleadatkorder(void) {
	CraftData* craft = g_curCraft;
	int16_t leaderManeuver = g_paiLeaderCraft->aiManeuverId;
	uint16_t orderPlan = craft->aiOrderPlanId;
	uint16_t leaderIndex = g_paiLeaderObjectIndex;
	int16_t requireWorkingSubsystems;
	uint16_t playerIndex;
	uint16_t candidateIndex;
	int16_t targetFlightGroup;
	uint16_t scannedCraftCount;
	requireWorkingSubsystems = orderPlan >= PAI_PLAN_DISABLE_FIRST && orderPlan <= PAI_PLAN_DISABLE_LAST;
	playerIndex = g_playerFlightState.objectIndex;
	if (leaderManeuver != PAIORDER_MANEUVER_ATTACK && leaderManeuver != PAIORDER_MANEUVER_ATTACK_ALTERNATE &&
		leaderIndex != playerIndex)
		return 0;
	candidateIndex = craft->aiCandidateTargetOrSavedInterval;
	if (candidateIndex != PAI_TARGET_NONE && candidateIndex != PAI_TARGET_EVASIVE) {
		if (pai_worthytarget(candidateIndex) != 0) {
			if (leaderIndex == g_playerFlightState.objectIndex) {
				pai_roughdistancebetween(g_paiObjectIndex, candidateIndex);
				if ((int)g_targetRangeScore > PAIFIGHT_PLAYER_LEADER_RANGE_LIMIT)
					return 0;
			}
			g_curCraft->aiTargetRef = candidateIndex;
			return 1;
		}
		g_curCraft->aiCandidateTargetOrSavedInterval = PAI_TARGET_NONE;
		craft = g_curCraft;
		playerIndex = g_playerFlightState.objectIndex;
	}
	candidateIndex = PAI_TARGET_NONE;
	if (leaderIndex != playerIndex) {
		candidateIndex = g_paiLeaderCraft->aiTargetRef;
	} else {
		ObjectRecord* playerObject = g_playerFlightState.object;
		uint16_t objectIndex;
		for (objectIndex = 0; objectIndex < XW_CRAFT_OBJECT_COUNT; ++objectIndex) {
			if (g_objectTable[objectIndex].objectType != XW_OBJ_NONE) {
				CraftData* scanCraft = g_objectTable[objectIndex].instanceData;
				if ((requireWorkingSubsystems == 0 || scanCraft->workingSubsystems != 0) &&
					scanCraft->lastAttackerObjIdx == playerIndex &&
					g_objectTable[objectIndex].iff != playerObject->iff) {
					candidateIndex = objectIndex;
				}
			}
		}
	}
	if (candidateIndex == PAI_TARGET_NONE)
		return 0;
	targetFlightGroup = ((CraftData*)g_objectTable[candidateIndex].instanceData)->flightGroupIndex;
	candidateIndex = craft->craftIndexInFlightGroup + candidateIndex;
	if (candidateIndex >= XW_CRAFT_OBJECT_COUNT)
		candidateIndex = 0;
	for (scannedCraftCount = 0; scannedCraftCount < XW_CRAFT_OBJECT_COUNT; ++scannedCraftCount) {
		if (g_objectTable[candidateIndex].objectType != XW_OBJ_NONE) {
			CraftData* candidateCraft = g_objectTable[candidateIndex].instanceData;
			/* The original consumes this iteration without advancing the candidate. */
			if (requireWorkingSubsystems != 0 && candidateCraft->workingSubsystems == 0)
				continue;
			if (targetFlightGroup == candidateCraft->flightGroupIndex &&
				pai_checktargetforattack(g_paiObjectIndex, candidateIndex, 1) != 0) {
				g_curCraft->aiTargetRef = candidateIndex;
				return 1;
			}
		}
		++candidateIndex;
		if (candidateIndex >= XW_CRAFT_OBJECT_COUNT)
			candidateIndex = 0;
	}
	return 0;
}

// FUNCTION: XW 0x415CF0
int16_t paifight_checkescortorder(void) {
	uint16_t primaryTarget;
	uint16_t secondaryTarget;
	int primaryLeader;
	g_curCraft->aiEscortTargetFlightGroup = XW_CRAFT_ESCORT_NO_FUTURE_GROUP;
	primaryTarget = g_missionFlightGroups[g_curCraft->flightGroupIndex].primaryTarget;
	primaryLeader = paifight_FindEscortableFlightGroupLeader(primaryTarget);
	/* Preserve the original comparison of a zero-extended word against signed -1. */
	if (primaryLeader != PAIFIGHT_PRIMARY_LEADER_NOT_FOUND) {
		g_curCraft->aiEscortTargetFlightGroup = primaryTarget;
		return 0;
	}
	secondaryTarget = g_missionFlightGroups[g_curCraft->flightGroupIndex].secondaryTarget;
	if (paifight_FindEscortableFlightGroupLeader(secondaryTarget) != PAIFIGHT_NO_OBJECT) {
		g_curCraft->aiEscortTargetFlightGroup = secondaryTarget;
		return 0;
	}
	if (primaryTarget != MISSION_TARGET_GROUP_UNSPECIFIED &&
		(g_missionFlightGroupStates[primaryTarget].hasArrived == 0 ||
		 g_missionFlightGroupStates[primaryTarget].wavesRemaining != 0))
		g_curCraft->aiEscortTargetFlightGroup = XW_CRAFT_ESCORT_WAITING_FOR_GROUP;
	if (secondaryTarget != MISSION_TARGET_GROUP_UNSPECIFIED &&
		(g_missionFlightGroupStates[secondaryTarget].hasArrived == 0 ||
		 g_missionFlightGroupStates[secondaryTarget].wavesRemaining != 0))
		g_curCraft->aiEscortTargetFlightGroup = XW_CRAFT_ESCORT_WAITING_FOR_GROUP;
	if (g_curCraft->aiEscortTargetFlightGroup == XW_CRAFT_ESCORT_NO_FUTURE_GROUP)
		return 1;
	if (g_missionFlightGroups[g_curCraft->flightGroupIndex].waypointEnabled[PAIFIGHT_ESCORT_HOLD_WAYPOINT] !=
		0) {
		g_curCraft->aiTargetRef = PAI_TARGET_WAYPOINT_BASE + PAIFIGHT_ESCORT_HOLD_WAYPOINT;
		pai_settarget();
		return 0;
	} else {
		g_curCraft->aiTargetRef = PAI_TARGET_WAYPOINT_BASE;
		pai_settarget();
		return 0;
	}
}

// FUNCTION: XW 0x415E30
int16_t paifight_OrderHasNoRemainingTargets(void) {
	uint16_t orderPlan;
	int16_t requireWorkingSubsystems;
	if (g_curCraft->aiManeuverId != g_paiInitialManeuverId) {
		return 0;
	}
	orderPlan = g_curCraft->aiOrderPlanId;
	requireWorkingSubsystems = orderPlan >= PAI_PLAN_DISABLE_FIRST && orderPlan <= PAI_PLAN_DISABLE_LAST;
	if (paiorder_FlightGroupHasRemainingCraft(
			g_missionFlightGroups[g_curCraft->flightGroupIndex].primaryTarget, requireWorkingSubsystems) !=
			0 ||
		paiorder_FlightGroupHasRemainingCraft(
			g_missionFlightGroups[g_curCraft->flightGroupIndex].secondaryTarget, requireWorkingSubsystems) !=
			0) {
		return 0;
	}
	switch (orderPlan) {
		case PAI_PLAN_FIGHTER_OR_TRANSPORT_TARGETS:
		case PAI_PLAN_DISABLE_FIGHTER_OR_TRANSPORT_TARGETS:
			if (paifight_HasLiveOpponentsOrPendingGenus(XW_GENUS_STARFIGHTER, requireWorkingSubsystems) !=
					0 ||
				paifight_HasLiveOpponentsOrPendingGenus(XW_GENUS_TRANSPORT, requireWorkingSubsystems) != 0) {
				return 0;
			}
			break;
		case PAI_PLAN_TRANSPORT_TARGETS:
		case PAI_PLAN_DISABLE_TRANSPORT_TARGETS:
			if (paifight_HasLiveOpponentsOrPendingGenus(XW_GENUS_TRANSPORT, requireWorkingSubsystems) != 0) {
				return 0;
			}
			break;
		case PAI_PLAN_FREIGHTER_TARGETS:
		case PAI_PLAN_DISABLE_FREIGHTER_TARGETS:
			if (paifight_HasLiveOpponentsOrPendingGenus(XW_GENUS_FREIGHTER, requireWorkingSubsystems) != 0) {
				return 0;
			}
			break;
		case PAI_PLAN_STARSHIP_TARGETS:
		case PAI_PLAN_DISABLE_STARSHIP_TARGETS:
			if (paifight_HasLiveOpponentsOrPendingGenus(XW_GENUS_STARSHIP, requireWorkingSubsystems) != 0) {
				return 0;
			}
			break;
		case PAI_PLAN_FIGHTER_TARGETS:
			if (paifight_HasLiveOpponentsOrPendingGenus(XW_GENUS_STARFIGHTER, requireWorkingSubsystems) !=
				0) {
				return 0;
			}
			break;
	}
	return 1;
}

// FUNCTION: XW 0x415F70
int16_t paifight_HasLiveOpponentsOrPendingGenus(int16_t genusId, int16_t requireWorkingSubsystems) {
	uint16_t objectIndex;
	uint16_t flightGroupIndex;
	for (objectIndex = 0; objectIndex < XW_CRAFT_OBJECT_COUNT; ++objectIndex) {
		if (g_objectTable[objectIndex].objectType != XW_OBJ_NONE &&
			g_objectTable[objectIndex].iff != g_objectTable[g_paiObjectIndex].iff) {
			const CraftData* craft = g_objectTable[objectIndex].instanceData;
			if ((requireWorkingSubsystems == 0 || craft->workingSubsystems != 0) &&
				g_objectTable[objectIndex].genusId == genusId) {
				return 1;
			}
		}
	}
	for (flightGroupIndex = 0; flightGroupIndex < g_missionHeader.flightGroupCount; ++flightGroupIndex) {
#ifdef XW_MODERN
		if ((g_missionFlightGroupStates[flightGroupIndex].hasArrived == 0 ||
			 g_missionFlightGroupStates[flightGroupIndex].wavesRemaining != 0) &&
			g_modelTypeTable[XwFlightTypes_MissionType(g_missionFlightGroups[flightGroupIndex].craftType)]
					.genusId == genusId) {
#else
		if ((g_missionFlightGroupStates[flightGroupIndex].hasArrived == 0 ||
			 g_missionFlightGroupStates[flightGroupIndex].wavesRemaining != 0) &&
			g_modelTypeTable[g_craftTypeToObjectType[g_missionFlightGroups[flightGroupIndex].craftType]]
					.genusId == genusId) {
#endif
			return 1;
		}
	}
	return 0;
}

// FUNCTION: XW 0x416060
int16_t paifight_SelectLowAltitudeTarget(void) {
	uint16_t sourceObjectIndex = g_paiObjectIndex;
	if (g_curCraft->aiManeuverId == g_paiInitialManeuverId) {
		uint8_t opposingIff = g_objectTable[sourceObjectIndex].iff ^ XW_OBJECT_OPPOSING_IFF_MASK;
		uint16_t candidateIndex = math2_getrandom();
		uint16_t slotsExamined;
		candidateIndex &= PAIFIGHT_LOW_ALTITUDE_START_MASK;
		for (slotsExamined = 0; slotsExamined < XW_CRAFT_OBJECT_COUNT; ++slotsExamined) {
			if (g_objectTable[candidateIndex].objectType != XW_OBJ_NONE &&
				g_objectTable[candidateIndex].iff == opposingIff &&
				g_objectTable[candidateIndex].worldZ < PAIFIGHT_LOW_ALTITUDE_LIMIT &&
				pai_checktargetforattack(sourceObjectIndex, candidateIndex, 0) != 0) {
				g_curCraft->aiTargetRef = candidateIndex;
				pai_settarget();
				return 1;
			}
			++candidateIndex;
			if (candidateIndex >= XW_CRAFT_OBJECT_COUNT) {
				candidateIndex = 0;
			}
		}
	}
	return 0;
}

// FUNCTION: XW 0x416130
int16_t paifight_AdvanceWaypointIfProgressComplete(void) {
	if (g_curCraft->aiOrderProgress >= PAIFIGHT_WAYPOINT_PROGRESS_REQUIRED) {
		paiman_gonextwaypoint(g_paiObjectIndex);
		g_curCraft->aiOrderProgress = 0;
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x416160
int16_t paifight_AdvanceWaypoint(void) {
	paiman_gonextwaypoint(g_paiObjectIndex);
	return 1;
}

// FUNCTION: XW 0x416180
uint16_t paifight_FindEscortableFlightGroupLeader(int16_t flightGroupIndex) {
	uint16_t objectIndex;
	if ((uint16_t)flightGroupIndex != (uint16_t)PAIFIGHT_NO_FLIGHT_GROUP) {
		for (objectIndex = 0; objectIndex < PAIFIGHT_ESCORT_OBJECT_LIMIT; ++objectIndex) {
			if (g_objectTable[objectIndex].objectType != XW_OBJ_NONE) {
				const CraftData* candidateCraft = g_objectTable[objectIndex].instanceData;
				if (candidateCraft->flightGroupIndex == flightGroupIndex &&
					candidateCraft->aiLeaderObjectIndex == XW_CRAFT_NO_AI_LEADER &&
					candidateCraft->aiCurrentPlanId != PAI_PLAN_55) {
					return objectIndex;
				}
			}
		}
	}
	return PAIFIGHT_NO_OBJECT;
}
