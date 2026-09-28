#include "xw/flight/ai/paiorder.h"

#ifdef XW_MODERN
#include "xw_runtime/runtime/flight_types.h"
#include "xw_runtime/timing/flight_integration.h"
#endif

#include "xw/audio/fsfx.h"
#include "xw/flight/ai/pai.h"
#include "xw/flight/ai/paiman.h"
#include "xw/flight/death_star.h"
#include "xw/flight/fediskio.h"
#include "xw/flight/hud/msg.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/collide.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/laser.h"
#include "xw/flight/player/user.h"
#include "xw/flight/xw.h"
#include "xw/math/math2.h"
#include "xw/math/trig2.h"
#include "xw/util/shared.h"

#include <stdlib.h>

// GLOBAL: XW 0x4C8E70
int g_aiStillAttackLastAttackerRangeBySkill[PAI_PROFICIENCY_COUNT] = { 0x8000, 0xC000, 0xE000 };

// GLOBAL: XW 0x4C8EB0
int g_aiAttackerSearchRangeBySkill[PAI_PROFICIENCY_COUNT] = { 0x2000, 0x3000, 0x4000 };

// GLOBAL: XW 0x4C8EC0
int g_aiWarheadThreatRangeBySkill[PAI_PROFICIENCY_COUNT] = { 0x400, 0x800, 0x1000 };

// GLOBAL: XW 0x4C8ED0
uint8_t g_aiThreatBearingClassByOctant[PAIORDER_THREAT_OCTANT_COUNT] = { 0, 1, 1, 2, 2, 1, 1, 0 };

// GLOBAL: XW 0x4C8ED8
uint8_t g_aiUnderAttackFrontManeuverChoices[PAIORDER_FRONT_MANEUVER_COUNT] = { 13, 14, 15, 3 };

// GLOBAL: XW 0x4C8EE0
uint8_t g_aiUnderAttackSideRearManeuverChoices[PAIORDER_SIDE_REAR_MANEUVER_COUNT] = {
	1, 4, 1, 3, 1, 4, 15, 4
};

// GLOBAL: XW 0x4C9078
XwManeuverFunction g_maneuverFunctions[PAIORDER_MANEUVER_COUNT] = { Shared_ReturnZero,
																	paiman_turninsidemaneuver,
																	paiman_splitsmaneuver,
																	paiman_immelmannmaneuver,
																	paiman_scissorsmaneuver,
																	paiman_rendezvousmaneuver,
																	paiman_cruisemaneuver,
																	paiman_headtowardfullmaneuver,
																	paiman_runawaymaneuver,
																	paiman_headonattackmaneuver,
																	paiman_followleadermaneuver,
																	paiman_setupattackmaneuver,
																	paiman_attackmaneuver,
																	paiman_zoommaneuver,
																	paiman_zoommaneuver_2,
																	paiman_splitsmaneuver,
																	paiman_speedawaymaneuver,
																	paiman_escortmaneuver,
																	paiman_boardmaneuver,
																	paiman_awaitboardmaneuver,
																	paiman_headtowardmaneuver,
																	paiman_intohyperspacemaneuver,
																	paiman_outofhyperspacemaneuver,
																	paiman_attackmaneuver,
																	paiman_turnawaymaneuver,
																	paiman_awaitboardmaneuver,
																	paiman_outofhangarmaneuver,
																	paiman_splitsmaneuver,
																	Shared_ReturnZero };

// GLOBAL: XW 0x63ADC0
XwManeuverFunction g_currentManeuverFunction = NULL;

// FUNCTION: XW 0x412AB0
int16_t paiorder_updatecourseorder(void) {
	g_currentManeuverFunction = g_maneuverFunctions[g_curCraft->aiManeuverId];
	return g_currentManeuverFunction();
}

// FUNCTION: XW 0x412AD0
int16_t paiorder_underattackorder(void) {
	uint16_t objectIndex = g_paiObjectIndex;
	if ((g_objectTable[objectIndex].genusId == XW_GENUS_STARFIGHTER ||
		 g_objectTable[objectIndex].genusId == XW_GENUS_TRANSPORT) &&
		g_curCraft->aiManeuverId == g_paiInitialManeuverId) {
		if (g_curCraft->lastAttackerObjIdx == PAIORDER_NO_LAST_ATTACKER) {
			int threatRange = g_aiWarheadThreatRangeBySkill[g_paiSkillTier];
			uint16_t warheadIndex;
			uint8_t enemyIff;
			int searchRange;
			for (warheadIndex = XW_CRAFT_OBJECT_COUNT;
				 warheadIndex < XW_CRAFT_OBJECT_COUNT + LASER_WARHEAD_GUIDANCE_COUNT; ++warheadIndex) {
				uint8_t objectType = g_objectTable[warheadIndex].objectType;
				if (objectType != XW_OBJ_NONE) {
					WarheadGuidanceState* guidance = g_objectTable[warheadIndex].instanceData;
#ifdef XW_MODERN
					/* DOS impact effects retain the projectile slot but have no guidance. */
					if (guidance == NULL)
						continue;
#endif
					if (guidance->homingTier != 0 && guidance->targetObjIdx == objectIndex) {
						int effectiveRange = threatRange * PAIORDER_TRACKED_WARHEAD_RANGE_MULTIPLIER;
#ifdef XW_MODERN
						if (objectType != XwFlightTypes_ObjectType(XW_OBJ_TRACKED_WARHEAD))
#else
						if (objectType != XW_OBJ_TRACKED_WARHEAD)
#endif
							effectiveRange = threatRange;
						if (pai_roughproximitycheck(warheadIndex, effectiveRange) == 1) {
							g_curCraft->lastAttackerObjIdx = warheadIndex;
							pai_distancebetween(objectIndex, warheadIndex);
							if (g_aiThreatBearingClassByOctant[(uint16_t)(g_trig2Yaw -
																		  g_objectTable[objectIndex].yaw) >>
															   PAIORDER_THREAT_OCTANT_SHIFT] == 0)
								g_curCraft->aiManeuverId = PAIORDER_MANEUVER_TURN_AWAY;
							else
								g_curCraft->aiManeuverId = PAIORDER_MANEUVER_TURN_INSIDE;
							paiman_initmaneuver();
							return 0;
						}
					}
				}
			}
			enemyIff = g_objectTable[objectIndex].iff ^ 1;
			searchRange = g_aiAttackerSearchRangeBySkill[g_paiSkillTier];
			if (g_curCraft->lastAttackerObjIdx == PAIORDER_NO_LAST_ATTACKER) {
				uint16_t attackerIndex;
				for (attackerIndex = 0; attackerIndex < XW_CRAFT_OBJECT_COUNT; ++attackerIndex) {
					if (attackerIndex != g_playerFlightState.objectIndex &&
						g_objectTable[attackerIndex].objectType != XW_OBJ_NONE &&
						g_objectTable[attackerIndex].iff == enemyIff &&
						g_curCraft->objectKind == XW_CRAFT_OBJECT_KIND_0 &&
						g_objectTable[attackerIndex].genusId == XW_GENUS_STARFIGHTER &&
						pai_roughproximitycheck(attackerIndex, searchRange) == 1) {
						uint16_t yawDifference;
						uint16_t pitchDifference;
						pai_distancebetween(attackerIndex, objectIndex);
						yawDifference = g_trig2Yaw - g_objectTable[attackerIndex].yaw;
						if (yawDifference >= TRIG2_ANGLE_SIGN_BIT)
							yawDifference = -yawDifference;
						pitchDifference = g_trig2Pitch - g_objectTable[attackerIndex].pitch;
						if (pitchDifference >= TRIG2_ANGLE_SIGN_BIT)
							pitchDifference = -pitchDifference;
						if (yawDifference < PAIORDER_ATTACKER_ANGLE_LIMIT &&
							pitchDifference < PAIORDER_ATTACKER_ANGLE_LIMIT) {
							g_curCraft->lastAttackerObjIdx = attackerIndex;
							break;
						}
					}
				}
			}
		}
		if (g_curCraft->lastAttackerObjIdx != PAIORDER_NO_LAST_ATTACKER) {
			uint8_t bearingClass;
			uint16_t craftMaxSpeed;
			uint16_t attackerMaxSpeed;
			uint16_t attackerIndex;
			uint16_t randomChoice;
			uint8_t nextManeuver;
			pai_distancebetween(objectIndex, g_curCraft->lastAttackerObjIdx);
			bearingClass =
				g_aiThreatBearingClassByOctant[(uint16_t)(g_trig2Yaw - g_objectTable[objectIndex].yaw) >>
											   PAIORDER_THREAT_OCTANT_SHIFT];
			craftMaxSpeed = g_craftTypeDefs[g_curCraft->craftTypeIndex].maxSpeed;
			attackerIndex = g_curCraft->lastAttackerObjIdx;
			if (g_objectTable[attackerIndex].familyId == XW_OBJECT_FAMILY_CRAFT) {
				CraftData* attacker = g_objectTable[attackerIndex].instanceData;
				attackerMaxSpeed = g_craftTypeDefs[attacker->craftTypeIndex].maxSpeed;
			} else {
				attackerMaxSpeed = PAIORDER_NONCRAFT_ATTACKER_SPEED;
			}
			randomChoice = math2_getrandom();
			if (bearingClass == PAIORDER_THREAT_SIDE) {
				if (g_trig2PolarDistance < PAIORDER_UNDER_ATTACK_CLOSE_RANGE &&
					randomChoice < PAIORDER_UNDER_ATTACK_TURN_RANDOM_LIMIT)
					nextManeuver = PAIORDER_MANEUVER_TURN_INSIDE;
				else
					nextManeuver = g_aiUnderAttackFrontManeuverChoices[randomChoice &
																	   (PAIORDER_FRONT_MANEUVER_COUNT - 1)];
			} else if (bearingClass == PAIORDER_THREAT_FRONT) {
				if (randomChoice > TRIG2_ANGLE_SIGN_BIT)
					nextManeuver = PAIORDER_MANEUVER_HEAD_ON_ATTACK;
				else
					nextManeuver = g_aiUnderAttackFrontManeuverChoices[randomChoice &
																	   (PAIORDER_FRONT_MANEUVER_COUNT - 1)];
			} else {
				if (craftMaxSpeed >= attackerMaxSpeed &&
					g_trig2PolarDistance > PAIORDER_UNDER_ATTACK_ESCAPE_RANGE)
					nextManeuver = PAIORDER_MANEUVER_SPEED_AWAY;
				else
					nextManeuver =
						g_aiUnderAttackSideRearManeuverChoices[randomChoice &
															   (PAIORDER_SIDE_REAR_MANEUVER_COUNT - 1)];
			}
			if (g_deathStarSurfaceModeActive != 0 &&
				g_objectTable[g_paiObjectIndex].worldZ < PAIORDER_UNDER_ATTACK_MIN_ALTITUDE &&
				(nextManeuver == PAIORDER_MANEUVER_ZOOM_ALTERNATE ||
				 nextManeuver == PAIORDER_MANEUVER_SPLIT_S_ALTERNATE))
				nextManeuver = PAIORDER_MANEUVER_ZOOM;
			g_curCraft->aiManeuverId = nextManeuver;
			paiman_initmaneuver();
		}
	}
	return 0;
}

// FUNCTION: XW 0x412E20
int16_t paiorder_stillattackorder(void) {
	if (g_curCraft->aiManeuverId != g_paiInitialManeuverId) {
		uint16_t attackerIndex = g_curCraft->lastAttackerObjIdx;
		if (attackerIndex != PAI_TARGET_NONE) {
			if (attackerIndex >= XW_CRAFT_OBJECT_COUNT) {
				WarheadGuidanceState* guidance = g_objectTable[attackerIndex].instanceData;
				if (g_objectTable[attackerIndex].objectType == XW_OBJ_NONE ||
#ifdef XW_MODERN
					guidance == NULL ||
#endif
					guidance->targetObjIdx != g_paiObjectIndex) {
					g_curCraft->lastAttackerObjIdx = PAI_TARGET_NONE;
					return 1;
				}
			} else {
				if (!pai_roughproximitycheck(attackerIndex,
											 g_aiStillAttackLastAttackerRangeBySkill[g_paiSkillTier])) {
					g_curCraft->lastAttackerObjIdx = PAI_TARGET_NONE;
					return 1;
				}
			}
		}
	}
	return 0;
}

// FUNCTION: XW 0x412EB0
int16_t paiorder_flyhomeorder(void) {
	CraftData* currentCraft;
	CraftData* mothershipCraft = NULL;
	int flightGroupIndex;
	uint16_t mothershipFlightGroupIndex;
	uint16_t mothershipObjectIndex;
	g_curCraft->aiFormationSpacing = PAIORDER_HOME_FORMATION_SPACING;
	currentCraft = g_curCraft;
	if (currentCraft->captorFlightGroupOverride != 0)
		flightGroupIndex = currentCraft->captorFlightGroupOverride & PAIORDER_HOME_GROUP_OVERRIDE_MASK;
	else
		flightGroupIndex = currentCraft->flightGroupIndex;
	mothershipFlightGroupIndex = g_missionFlightGroups[flightGroupIndex].mothershipFlightGroupIndex;
	if (mothershipFlightGroupIndex == g_playerFlightState.craft->flightGroupIndex)
		mothershipFlightGroupIndex = PAIORDER_HOME_GROUP_NONE;
	for (mothershipObjectIndex = 0; mothershipObjectIndex < XW_CRAFT_OBJECT_COUNT; ++mothershipObjectIndex) {
		if (g_objectTable[mothershipObjectIndex].objectType != XW_OBJ_NONE) {
			mothershipCraft = (CraftData*)g_objectTable[mothershipObjectIndex].instanceData;
			if (mothershipCraft->flightGroupIndex == mothershipFlightGroupIndex &&
				mothershipCraft->aiLeaderObjectIndex == XW_CRAFT_NO_AI_LEADER)
				break;
		}
	}
	if (mothershipObjectIndex == XW_CRAFT_OBJECT_COUNT) {
		if (g_missionFlightGroups[currentCraft->flightGroupIndex].waypointEnabled[PAI_WAYPOINT_FINAL] != 0)
			currentCraft->aiTargetRef = PAI_TARGET_FINAL_WAYPOINT;
		else
			currentCraft->aiTargetRef = PAI_TARGET_WAYPOINT_BASE;
		pai_settarget();
		return 0;
	}
	currentCraft->aiTargetRef = mothershipObjectIndex;
	pai_calcrotatedpoint(&g_objectTable[mothershipObjectIndex],
						 g_craftTypeDefs[mothershipCraft->craftTypeIndex].hangarOutside.side,
						 g_craftTypeDefs[mothershipCraft->craftTypeIndex].hangarOutside.up,
						 g_craftTypeDefs[mothershipCraft->craftTypeIndex].hangarOutside.forward);
	g_rotatedX *= PAIORDER_HOME_HANGAR_SCALE;
	g_rotatedY *= PAIORDER_HOME_HANGAR_SCALE;
	g_rotatedZ *= PAIORDER_HOME_HANGAR_SCALE;
#ifdef XW_MODERN
	g_curCraft->aiAimPointX =
		(int32_t)((uint32_t)g_objectTable[mothershipObjectIndex].worldX + (uint32_t)g_rotatedX);
#else
	g_curCraft->aiAimPointX = g_objectTable[mothershipObjectIndex].worldX + g_rotatedX;
#endif
#ifdef XW_MODERN
	g_curCraft->aiAimPointY =
		(int32_t)((uint32_t)g_objectTable[mothershipObjectIndex].worldY + (uint32_t)g_rotatedY);
#else
	g_curCraft->aiAimPointY = g_objectTable[mothershipObjectIndex].worldY + g_rotatedY;
#endif
#ifdef XW_MODERN
	g_curCraft->aiAimPointZ =
		(int32_t)((uint32_t)g_objectTable[mothershipObjectIndex].worldZ + (uint32_t)g_rotatedZ);
#else
	g_curCraft->aiAimPointZ = g_objectTable[mothershipObjectIndex].worldZ + g_rotatedZ;
#endif
	pai_targetdistance();
	if (g_trig2PolarDistance < PAIORDER_HOME_HALF_THROTTLE_RANGE)
		paiman_setpower(g_paiObjectIndex, XW_CRAFT_THROTTLE_HALF);
	if (g_trig2PolarDistance < PAIORDER_HOME_QUARTER_THROTTLE_RANGE)
		paiman_setpower(g_paiObjectIndex, PAIORDER_HOME_QUARTER_THROTTLE);
	return g_trig2PolarDistance < PAIORDER_HOME_ARRIVAL_RANGE;
}

// FUNCTION: XW 0x413080
int16_t paiorder_enterhangarorder(void) {
	CraftData *currentCraft, *mothershipCraft;
	const CraftData* followerCraft;
	int flightGroupIndex;
	uint16_t mothershipFlightGroupIndex, mothershipObjectIndex, followerObjectIndex;
	uint16_t craftTypeIndex;
	g_curCraft->aiFormationSpacing = PAIORDER_HANGAR_FORMATION_SPACING;
	g_curCraft->aiThinkIntervalTicks = PAIORDER_LEADER_DELAY_TICKS;
	currentCraft = g_curCraft;
	if (currentCraft->captorFlightGroupOverride != 0)
		flightGroupIndex = currentCraft->captorFlightGroupOverride & PAIORDER_HOME_GROUP_OVERRIDE_MASK;
	else
		flightGroupIndex = currentCraft->flightGroupIndex;
	mothershipFlightGroupIndex = g_missionFlightGroups[flightGroupIndex].mothershipFlightGroupIndex;
	for (mothershipObjectIndex = 0; mothershipObjectIndex < XW_CRAFT_OBJECT_COUNT; ++mothershipObjectIndex) {
		if (g_objectTable[mothershipObjectIndex].objectType != XW_OBJ_NONE) {
			mothershipCraft = (CraftData*)g_objectTable[mothershipObjectIndex].instanceData;
			if (mothershipCraft->flightGroupIndex == mothershipFlightGroupIndex &&
				mothershipCraft->aiLeaderObjectIndex == XW_CRAFT_NO_AI_LEADER) {
				currentCraft->aiTargetRef = mothershipObjectIndex;
				craftTypeIndex = mothershipCraft->craftTypeIndex;
				pai_calcrotatedpoint(&g_objectTable[mothershipObjectIndex],
									 g_craftTypeDefs[craftTypeIndex].hangarInside.side,
									 g_craftTypeDefs[craftTypeIndex].hangarInside.up,
									 g_craftTypeDefs[craftTypeIndex].hangarInside.forward);
#ifdef XW_MODERN
				g_curCraft->aiAimPointX =
					(int32_t)((uint32_t)g_objectTable[mothershipObjectIndex].worldX + (uint32_t)g_rotatedX);
#else
				g_curCraft->aiAimPointX = g_objectTable[mothershipObjectIndex].worldX + g_rotatedX;
#endif
#ifdef XW_MODERN
				g_curCraft->aiAimPointY =
					(int32_t)((uint32_t)g_objectTable[mothershipObjectIndex].worldY + (uint32_t)g_rotatedY);
#else
				g_curCraft->aiAimPointY = g_objectTable[mothershipObjectIndex].worldY + g_rotatedY;
#endif
#ifdef XW_MODERN
				g_curCraft->aiAimPointZ =
					(int32_t)((uint32_t)g_objectTable[mothershipObjectIndex].worldZ + (uint32_t)g_rotatedZ);
#else
				g_curCraft->aiAimPointZ = g_objectTable[mothershipObjectIndex].worldZ + g_rotatedZ;
#endif
				currentCraft = g_curCraft;
			}
		}
	}
	pai_targetdistance();
	if (g_trig2PolarDistance < PAIORDER_HANGAR_SLOW_RANGE)
		paiman_setpower(g_paiObjectIndex, PAIORDER_HANGAR_SLOW_THROTTLE);
	else
		paiman_setpower(g_paiObjectIndex, PAIORDER_HANGAR_APPROACH_THROTTLE);
	if (g_trig2PolarDistance < PAIORDER_HANGAR_ARRIVAL_RANGE) {
		currentCraft = g_curCraft;
		for (followerObjectIndex = 0; followerObjectIndex < XW_CRAFT_OBJECT_COUNT; ++followerObjectIndex) {
			if (g_objectTable[followerObjectIndex].objectType != XW_OBJ_NONE) {
				followerCraft = (const CraftData*)g_objectTable[followerObjectIndex].instanceData;
				if (followerCraft->flightGroupIndex == currentCraft->flightGroupIndex &&
					followerCraft->aiLeaderObjectIndex != XW_CRAFT_NO_AI_LEADER &&
					followerCraft->aiManeuverId == PAIORDER_MANEUVER_FOLLOW_LEADER) {
					if (g_objectTable[g_paiObjectIndex].iff == g_playerFlightState.object->iff) {
						msg_craftmessage(followerObjectIndex, followerCraft, XW_MSG_CRAFT_ENTERED_HANGAR);
						fsfx_triggersfx(FSFX_FRIENDLY_DEPARTURE_SLOT, FSFX_UNPOSITIONED_OBJECT);
						if (followerCraft->captorFlightGroupOverride != 0) {
							++g_missionFlightGroupStates[followerCraft->flightGroupIndex]
								  .outcomes[MISSION_OUTCOME_RECOVERED];
							flightGroupIndex = followerCraft->flightGroupIndex;
							if (g_missionFlightGroups[flightGroupIndex].specialCraftIndex ==
								followerCraft->craftIndexInFlightGroup)
								g_missionFlightGroupStates[flightGroupIndex]
									.outcomes[MISSION_OUTCOME_SPECIAL_RECOVERED] = 1;
						}
					}
					++g_missionFlightGroupStates[followerCraft->flightGroupIndex]
						  .outcomes[MISSION_OUTCOME_COMPLETED_SECOND];
					flightGroupIndex = followerCraft->flightGroupIndex;
					if (g_missionFlightGroups[flightGroupIndex].specialCraftIndex ==
						followerCraft->craftIndexInFlightGroup)
						g_missionFlightGroupStates[flightGroupIndex]
							.outcomes[MISSION_OUTCOME_SPECIAL_COMPLETED] = 1;
					fediskio_updatepilotrecord(followerObjectIndex, 0, 0);
					currentCraft = g_curCraft;
					g_objectTable[followerObjectIndex].objectType = XW_OBJ_NONE;
				}
			}
		}
		if (g_objectTable[g_paiObjectIndex].iff == g_playerFlightState.object->iff) {
			msg_craftmessage(g_paiObjectIndex, currentCraft, XW_MSG_CRAFT_ENTERED_HANGAR);
			fsfx_triggersfx(FSFX_FRIENDLY_DEPARTURE_SLOT, FSFX_UNPOSITIONED_OBJECT);
			currentCraft = g_curCraft;
			++g_missionFlightGroupStates[currentCraft->flightGroupIndex].outcomes[MISSION_OUTCOME_RECOVERED];
			flightGroupIndex = currentCraft->flightGroupIndex;
			if (g_missionFlightGroups[flightGroupIndex].specialCraftIndex ==
				currentCraft->craftIndexInFlightGroup)
				g_missionFlightGroupStates[flightGroupIndex].outcomes[MISSION_OUTCOME_SPECIAL_RECOVERED] = 1;
		}
		++g_missionFlightGroupStates[currentCraft->flightGroupIndex]
			  .outcomes[MISSION_OUTCOME_COMPLETED_SECOND];
		flightGroupIndex = currentCraft->flightGroupIndex;
		if (g_missionFlightGroups[flightGroupIndex].specialCraftIndex ==
			currentCraft->craftIndexInFlightGroup)
			g_missionFlightGroupStates[flightGroupIndex].outcomes[MISSION_OUTCOME_SPECIAL_COMPLETED] = 1;
		fediskio_updatepilotrecord(g_paiObjectIndex, 0, 0);
		g_objectTable[g_paiObjectIndex].objectType = XW_OBJ_NONE;
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x4133E0
int16_t paiorder_CheckDamageOrTargetSentinel(void) {
	if (g_curCraft->aiManeuverId == g_paiInitialManeuverId) {
		uint16_t damageThreshold = (uint16_t)math2_fraction(pai_getcraftdoomedlevel(g_paiObjectIndex),
															PAIORDER_DAMAGE_THRESHOLD_Q16);
		if (g_curCraft->hullDamage >= damageThreshold) {
			nullsub_SharedNoOp();
			return 1;
		} else if (g_curCraft->aiCandidateTargetOrSavedInterval == PAI_TARGET_DAMAGE_SENTINEL) {
			return 1;
		}
	}
	return 0;
}

// FUNCTION: XW 0x4135B0
int16_t paiorder_waitrunorder(void) {
	if (g_curCraft->aiManeuverId == g_paiInitialManeuverId) {
		int16_t inRange = pai_roughproximitycheck(g_curCraft->aiTargetRef,
												  g_curCraft->aiSkillQ16 + PAIORDER_WAIT_RUN_BASE_RANGE);
		if (inRange == 1) {
			return inRange;
		}
	}
	return 0;
}

// FUNCTION: XW 0x4135F0
int16_t paiorder_breakofforder(void) {
	uint16_t target = g_curCraft->aiTargetRef;
	uint16_t orderPlan;
	if (pai_worthytarget(target) == 0) {
		g_curCraft->aiTargetRef = PAI_TARGET_NONE;
		return 1;
	}
	orderPlan = g_curCraft->aiOrderPlanId;
	if (orderPlan >= PAI_PLAN_DISABLE_FIRST && orderPlan <= PAI_PLAN_DISABLE_LAST) {
		CraftData* targetCraft = g_objectTable[target].instanceData;
		if (targetCraft->workingSubsystems == 0) {
			return 1;
		}
	}
	return 0;
}

// FUNCTION: XW 0x413660
int16_t paiorder_abortatkorder(void) {
	uint16_t objectIndex = g_paiObjectIndex;
	if (g_objectTable[objectIndex].genusId != XW_GENUS_STARFIGHTER) {
		return 0;
	}
	if (g_curCraft->shieldEnergy[XW_SHIELD_REAR] + g_curCraft->shieldEnergy[XW_SHIELD_FRONT] != 0) {
		return 0;
	}
	return g_curCraft->hullDamage >=
		   (uint16_t)math2_fraction(g_curCraft->hullMax, PAIORDER_ABORT_ATTACK_DAMAGE_Q16);
}

// FUNCTION: XW 0x4136D0
int16_t paiorder_leaderdeadorder(void) {
	unsigned int oldLeaderObjectIndex = g_curCraft->aiLeaderObjectIndex;
	uint8_t leaderUnavailable = 0;
	CraftData* leaderCraft;
	if (oldLeaderObjectIndex == XW_CRAFT_NO_AI_LEADER) {
		return 1;
	}
	if (oldLeaderObjectIndex >= XW_CRAFT_OBJECT_COUNT) {
		return 0;
	}
	leaderCraft = g_objectTable[oldLeaderObjectIndex].instanceData;
	if (g_objectTable[oldLeaderObjectIndex].objectType == XW_OBJ_NONE) {
		leaderUnavailable = 1;
	}
	if (leaderCraft->hullDamage >= leaderCraft->systemDamageHullThreshold) {
		leaderUnavailable = 1;
	}
	if (leaderCraft->objectKind != 0) {
		leaderUnavailable = 1;
	}
	if (leaderUnavailable != 0) {
		uint8_t newLeaderObjectIndex = g_paiObjectIndex;
		uint16_t objectIndex;
		for (objectIndex = 0; objectIndex < XW_CRAFT_OBJECT_COUNT; ++objectIndex) {
			CraftData* followerCraft = g_objectTable[objectIndex].instanceData;
			if (g_objectTable[objectIndex].objectType != XW_OBJ_NONE &&
				followerCraft->aiLeaderObjectIndex == oldLeaderObjectIndex) {
				if (objectIndex == g_paiObjectIndex) {
					followerCraft->aiLeaderObjectIndex = XW_CRAFT_NO_AI_LEADER;
					followerCraft->aiFormationSpacing = leaderCraft->aiFormationSpacing;
					followerCraft->aiWaypointIndex = leaderCraft->aiWaypointIndex;
					followerCraft->aiTargetRef = leaderCraft->aiTargetRef;
					pai_settarget();
				} else {
					followerCraft->aiLeaderObjectIndex = newLeaderObjectIndex;
				}
			}
		}
	} else {
		if (g_paiLeaderCraft->aiCurrentPlanId == PAI_PLAN_55) {
			g_curCraft->aiThinkIntervalTicks = PAIORDER_LEADER_DELAY_TICKS;
		}
	}
	return leaderUnavailable;
}

// FUNCTION: XW 0x4137E0
int16_t paiorder_ontailorder(void) {
	uint16_t objectIndex = g_paiObjectIndex;
	if (g_curCraft->aiManeuverId == g_paiInitialManeuverId &&
		g_curCraft->lastAttackerObjIdx != PAIORDER_NO_LAST_ATTACKER) {
		pai_distancebetween(objectIndex, g_curCraft->lastAttackerObjIdx);
		if (g_aiThreatBearingClassByOctant[(uint16_t)(g_trig2Yaw - g_objectTable[objectIndex].yaw) >>
										   PAIORDER_THREAT_OCTANT_SHIFT] == PAIORDER_THREAT_REAR) {
			uint16_t randomChoice = math2_getrandom();
			uint8_t evasiveManeuverId;
			if (randomChoice < PAIORDER_TAIL_TURN_THRESHOLD) {
				evasiveManeuverId = PAIORDER_MANEUVER_TURN_INSIDE;
			} else {
				if (randomChoice < PAIORDER_TAIL_ZOOM_THRESHOLD) {
					evasiveManeuverId = PAIORDER_MANEUVER_ZOOM;
				} else {
					evasiveManeuverId = PAIORDER_MANEUVER_SCISSORS;
				}
			}
			if (g_objectTable[objectIndex].iff == PAIORDER_TAIL_ALT_ZOOM_IFF &&
				g_objectTable[objectIndex].worldZ > PAIORDER_TAIL_ALT_ZOOM_MIN_ALTITUDE &&
				(uint16_t)math2_getrandom() < PAIORDER_TAIL_TURN_THRESHOLD) {
				evasiveManeuverId = PAIORDER_MANEUVER_ZOOM_ALTERNATE;
			}
			g_curCraft->aiManeuverId = evasiveManeuverId;
			paiman_initmaneuver();
		}
	}
	return 0;
}

// FUNCTION: XW 0x4138B0
int16_t paiorder_IsObjectKind4(void) { return g_curCraft->objectKind == XW_CRAFT_OBJECT_KIND_4; }

// FUNCTION: XW 0x4138C0
int16_t paiorder_leadergohomeorder(void) { return g_paiLeaderCraft->aiCurrentPlanId == PAI_PLAN_FLY_HOME; }

// FUNCTION: XW 0x4138D0
int16_t paiorder_hyperspaceorder(void) {
	if (g_craftTypeDefs[g_curCraft->craftTypeIndex].hasHyperdrive != 0) {
		if (g_curCraft->captorFlightGroupOverride == 0) {
			if (g_missionFlightGroups[g_curCraft->flightGroupIndex].departureMethod ==
				MISSION_DEPARTURE_HYPERSPACE) {
				return 1;
			}
		} else {
			uint8_t genus = g_objectTable[g_paiObjectIndex].genusId;
			if (genus == XW_GENUS_STARSHIP || genus == XW_GENUS_FREIGHTER) {
				return 1;
			}
		}
	}
	return 0;
}

// FUNCTION: XW 0x413950
int16_t paiorder_mothershiporder(void) {
	if (g_curCraft->aiLeaderObjectIndex == XW_CRAFT_NO_AI_LEADER) {
		return g_curCraft->aiTargetRef == PAI_TARGET_FINAL_WAYPOINT;
	} else {
		return g_paiLeaderCraft->aiTargetRef == PAI_TARGET_FINAL_WAYPOINT;
	}
}

// FUNCTION: XW 0x413980
int16_t paiorder_lookforcrafttoboardorder(void) {
	uint16_t primaryTargetGroup = g_missionFlightGroups[g_curCraft->flightGroupIndex].primaryTarget;
	uint16_t secondaryTargetGroup = g_missionFlightGroups[g_curCraft->flightGroupIndex].secondaryTarget;
	uint16_t candidateIndex;
	for (candidateIndex = 0; candidateIndex < XW_CRAFT_OBJECT_COUNT; ++candidateIndex) {
		if (g_objectTable[candidateIndex].objectType != XW_OBJ_NONE) {
			CraftData* candidate = g_objectTable[candidateIndex].instanceData;
			uint16_t candidateGroup = candidate->flightGroupIndex;
			if (candidateGroup == primaryTargetGroup || candidateGroup == secondaryTargetGroup) {
				int16_t eligible = 0;
				if (candidate->aiOrderPlanId == PAIORDER_NO_ORDER) {
					eligible = 1;
				} else if (g_curCraft->aiOrderPlanId != PAIORDER_BOARD_DISABLED_43 &&
						   g_curCraft->aiOrderPlanId != PAIORDER_BOARD_DISABLED_44) {
					if (candidate->workingSubsystems == 0 ||
						candidate->aiManeuverId == PAIORDER_MANEUVER_AWAIT_BOARD ||
						candidate->aiManeuverId == PAIORDER_MANEUVER_AWAIT_BOARD_ALTERNATE) {
						eligible = 1;
					}
				} else if (candidate->workingSubsystems == 0) {
					eligible = 1;
				}
				if (eligible != 0) {
					int16_t assignedBoarders = 0;
					uint16_t otherIndex;
					for (otherIndex = 0; otherIndex < XW_CRAFT_OBJECT_COUNT; ++otherIndex) {
						if (g_objectTable[otherIndex].objectType != XW_OBJ_NONE &&
							otherIndex != g_paiObjectIndex) {
							CraftData* otherCraft = g_objectTable[otherIndex].instanceData;
							if (otherCraft->aiOrderPlanId >= PAIORDER_BOARD_FIRST &&
								otherCraft->aiOrderPlanId <= PAIORDER_BOARD_LAST &&
								otherCraft->aiTargetRef == candidateIndex) {
								++assignedBoarders;
							}
						}
					}
					if (assignedBoarders == 0) {
						g_curCraft->aiTargetRef = candidateIndex;
						return 1;
					}
				}
			}
		}
	}
	return 0;
}

// FUNCTION: XW 0x413AA0
int16_t paiorder_abortboardorder(void) {
	if (g_curCraft->aiManeuverPhase < PAIORDER_BOARD_ABORT_PHASE_LIMIT) {
		int16_t abortBoarding = 0;
		if (g_objectTable[g_curCraft->aiTargetRef].objectType == XW_OBJ_NONE) {
			abortBoarding = 1;
		}
		if (g_objectTable[g_curCraft->aiTargetRef].familyId == XW_OBJECT_FAMILY_5) {
			abortBoarding = 1;
		}
		if (g_curCraft->workingSubsystems == 0) {
			abortBoarding = 1;
		}
		if (abortBoarding != 0) {
			g_curCraft->aiDisplacementZ = 0;
			g_curCraft->aiDisplacementY = 0;
			g_curCraft->aiDisplacementX = 0;
#ifdef XW_MODERN
			XwFlightIntegration_ClearPush(g_paiObjectIndex);
#endif
			g_curCraft->aiTargetRef = PAI_TARGET_WAYPOINT_BASE;
			pai_settarget();
			return 1;
		}
	}
	return 0;
}

// FUNCTION: XW 0x413B30
int16_t paiorder_returnboardorder(void) {
	pai_targetdistance();
	return g_trig2PolarDistance < PAIORDER_RETURN_BOARD_DISTANCE;
}

// FUNCTION: XW 0x413B50
int16_t paiorder_awaitboardorder(void) {
	if (g_curCraft->boardingState == XW_CRAFT_BOARDING_COMPLETE) {
		g_curCraft->workingSubsystems = XW_CRAFT_SUBSYSTEM_ALL;
		if (g_curCraft->aiOrderPlanId == PAIORDER_AWAIT_REPAIR) {
			msg_craftmessage(g_paiObjectIndex, g_curCraft, XW_MSG_CRAFT_REPAIRED);
			if (g_objectTable[g_paiObjectIndex].iff == g_playerFlightState.object->iff) {
				if (g_fsfxLoaded != 0)
					fsfx_triggervoicesfx(FSFX_REPAIR_VOICE_SLOT);
				else
					fsfx_triggersfx(FSFX_FRIENDLY_REPAIR_SLOT, FSFX_UNPOSITIONED_OBJECT);
			} else {
				fsfx_triggersfx(FSFX_OTHER_REPAIR_SLOT, FSFX_UNPOSITIONED_OBJECT);
			}
		}
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x413BF0
int16_t paiorder_makedisabledorder(void) {
	g_curCraft->workingSubsystems = 0;
	return 0;
}

// FUNCTION: XW 0x413C00
int16_t paiorder_returnboardorder_2(void) {
	uint16_t missionPointIndex;
	for (missionPointIndex = PAI_WAYPOINT_LAST; missionPointIndex > 0; --missionPointIndex) {
		if (g_missionFlightGroups[g_curCraft->flightGroupIndex].waypointEnabled[missionPointIndex] != 0) {
			break;
		}
	}
	create_getworldposition(PAI_TARGET_WAYPOINT_BASE + missionPointIndex, g_curCraft->flightGroupIndex);
	pai_targetdistance();
	return g_trig2PolarDistance < PAIORDER_RETURN_BOARD_ALT_DISTANCE;
}

// FUNCTION: XW 0x413C70
int16_t paiorder_rocketsonboardorder(void) {
	CraftData* craft = g_curCraft;
	int16_t targetGenus = g_objectTable[craft->aiTargetRef].genusId;
	int16_t requiredProjectileType;
	uint16_t launcherIndex;
	uint16_t launcherCount;
	if (targetGenus == XW_GENUS_FREIGHTER || targetGenus == XW_GENUS_STARSHIP) {
		requiredProjectileType = PAIORDER_LARGE_TARGET_WARHEAD;
	} else {
#ifdef XW_MODERN
		requiredProjectileType = XwFlightTypes_ObjectType(XW_OBJ_TRACKED_WARHEAD);
#else
		requiredProjectileType = XW_OBJ_TRACKED_WARHEAD;
#endif
	}
	launcherCount = craft->warheadLauncherCount;
	for (launcherIndex = 0; launcherIndex < launcherCount; ++launcherIndex) {
		if (craft->warheadSlotTypeIds[launcherIndex] == requiredProjectileType) {
			uint16_t lastWeaponSlot = g_craftTypeDefs[craft->craftTypeIndex].warheadLastSlot[launcherIndex];
			uint16_t weaponSlotIndex;
			for (weaponSlotIndex = g_craftTypeDefs[craft->craftTypeIndex].warheadFirstSlot[launcherIndex];
				 weaponSlotIndex <= lastWeaponSlot; ++weaponSlotIndex) {
				if (craft->weaponSlots[weaponSlotIndex].count != 0) {
					return 1;
				}
			}
		}
	}
	return 0;
}

// FUNCTION: XW 0x413D30
int16_t paiorder_avoidhitorder(void) {
	uint16_t objectIndex = g_paiObjectIndex;
	if (g_objectTable[objectIndex].genusId != XW_GENUS_FREIGHTER &&
		g_objectTable[objectIndex].genusId != XW_GENUS_STARSHIP &&
		g_curCraft->aiManeuverId == g_paiInitialManeuverId) {
		if (g_curCraft->lastAttackerObjIdx == PAIORDER_NO_LAST_ATTACKER) {
			int threatRange = g_aiWarheadThreatRangeBySkill[g_paiSkillTier];
			uint16_t warheadIndex;
			for (warheadIndex = XW_CRAFT_OBJECT_COUNT;
				 warheadIndex < XW_CRAFT_OBJECT_COUNT + LASER_WARHEAD_GUIDANCE_COUNT; ++warheadIndex) {
				uint8_t objectType = g_objectTable[warheadIndex].objectType;
				if (objectType != XW_OBJ_NONE) {
					WarheadGuidanceState* guidance = g_objectTable[warheadIndex].instanceData;
#ifdef XW_MODERN
					/* DOS impact effects retain the projectile slot but have no guidance. */
					if (guidance == NULL)
						continue;
#endif
					if (guidance->homingTier != 0 && guidance->targetObjIdx == objectIndex) {
						int effectiveRange = threatRange * PAIORDER_TRACKED_WARHEAD_RANGE_MULTIPLIER;
#ifdef XW_MODERN
						if (objectType != XwFlightTypes_ObjectType(XW_OBJ_TRACKED_WARHEAD))
#else
						if (objectType != XW_OBJ_TRACKED_WARHEAD)
#endif
							effectiveRange = threatRange;
						if (pai_roughproximitycheck(warheadIndex, effectiveRange) == 1) {
							g_curCraft->lastAttackerObjIdx = warheadIndex;
							pai_distancebetween(objectIndex, warheadIndex);
							if (g_aiThreatBearingClassByOctant[(uint16_t)(g_trig2Yaw -
																		  g_objectTable[objectIndex].yaw) >>
															   PAIORDER_THREAT_OCTANT_SHIFT] == 0)
								g_curCraft->aiManeuverId = PAIORDER_MANEUVER_TURN_AWAY;
							else
								g_curCraft->aiManeuverId = PAIORDER_MANEUVER_TURN_INSIDE;
							paiman_initmaneuver();
							return 0;
						}
					}
				}
			}
			if (g_curCraft->aiManeuverId == PAIORDER_MANEUVER_ATTACK ||
				g_curCraft->aiManeuverId == PAIORDER_MANEUVER_ATTACK_ALTERNATE) {
				pai_targetdistance();
				if (g_trig2PolarDistance < PAIORDER_AVOID_TARGET_DISTANCE)
					return 0;
			} else {
				uint8_t enemyIff = g_objectTable[objectIndex].iff ^ 1;
				int searchRange = g_aiAttackerSearchRangeBySkill[g_paiSkillTier];
				if (g_curCraft->lastAttackerObjIdx == PAIORDER_NO_LAST_ATTACKER) {
					uint16_t attackerIndex;
					for (attackerIndex = 0; attackerIndex < XW_CRAFT_OBJECT_COUNT; ++attackerIndex) {
						if (g_objectTable[attackerIndex].objectType != XW_OBJ_NONE &&
							g_objectTable[attackerIndex].iff == enemyIff &&
							g_curCraft->objectKind == XW_CRAFT_OBJECT_KIND_0 &&
							g_objectTable[attackerIndex].genusId == XW_GENUS_STARFIGHTER &&
							pai_roughproximitycheck(attackerIndex, searchRange) == 1) {
							uint16_t yawDifference;
							uint16_t pitchDifference;
							pai_distancebetween(attackerIndex, objectIndex);
							yawDifference = g_trig2Yaw - g_objectTable[attackerIndex].yaw;
							if (yawDifference >= TRIG2_ANGLE_SIGN_BIT)
								yawDifference = -yawDifference;
							pitchDifference = g_trig2Pitch - g_objectTable[attackerIndex].pitch;
							if (pitchDifference >= TRIG2_ANGLE_SIGN_BIT)
								pitchDifference = -pitchDifference;
							if (yawDifference < PAIORDER_ATTACKER_ANGLE_LIMIT &&
								pitchDifference < PAIORDER_ATTACKER_ANGLE_LIMIT) {
								g_curCraft->lastAttackerObjIdx = attackerIndex;
								break;
							}
						}
					}
				}
			}
		}
		if (g_curCraft->lastAttackerObjIdx != PAIORDER_NO_LAST_ATTACKER) {
			uint16_t speedFraction =
				math2_percentage(g_objectTable[g_paiObjectIndex].speed, g_curCraft->maxSpeed);
			unsigned int displacement =
				math2_fraction((math2_getrandom() & UINT8_MAX) + PAIORDER_DISPLACEMENT_BASE, speedFraction);
			if (math2_getrandom() & TRIG2_ANGLE_SIGN_BIT)
				displacement = -displacement;
			g_curCraft->aiDisplacementX = (int16_t)displacement;
			displacement =
				math2_fraction((math2_getrandom() & UINT8_MAX) + PAIORDER_DISPLACEMENT_BASE, speedFraction);
			if (math2_getrandom() & TRIG2_ANGLE_SIGN_BIT)
				displacement = -displacement;
			g_curCraft->aiDisplacementY = (int16_t)displacement;
			displacement =
				math2_fraction((math2_getrandom() & UINT8_MAX) + PAIORDER_DISPLACEMENT_BASE, speedFraction);
			if (math2_getrandom() & TRIG2_ANGLE_SIGN_BIT)
				displacement = -displacement;
			g_curCraft->aiDisplacementZ = (int16_t)displacement;
		}
	}
	return 0;
}

// FUNCTION: XW 0x414030
int16_t paiorder_waitforallreturnorder(void) {
	uint16_t dependentFlightGroup;
	for (dependentFlightGroup = 0; dependentFlightGroup < g_missionHeader.flightGroupCount;
		 ++dependentFlightGroup) {
		if (dependentFlightGroup != g_curCraft->flightGroupIndex &&
			g_missionFlightGroups[dependentFlightGroup].departureMethod == MISSION_DEPARTURE_MOTHERSHIP &&
			g_missionFlightGroups[dependentFlightGroup].mothershipFlightGroupIndex ==
				g_curCraft->flightGroupIndex) {
			uint16_t craftObjectIndex;
			if (g_missionFlightGroupStates[dependentFlightGroup].hasArrived == 0 ||
				g_missionFlightGroupStates[dependentFlightGroup].wavesRemaining != 0) {
				return 0;
			}
			for (craftObjectIndex = 0; craftObjectIndex < XW_CRAFT_OBJECT_COUNT; ++craftObjectIndex) {
				if (g_objectTable[craftObjectIndex].objectType != XW_OBJ_NONE) {
					CraftData* craft = g_objectTable[craftObjectIndex].instanceData;
					if (craft->flightGroupIndex == dependentFlightGroup) {
						return 0;
					}
				}
			}
		}
	}
	return 1;
}

// FUNCTION: XW 0x4140E0
int16_t paiorder_waitforallcreateorder(void) {
	uint16_t dependentFlightGroup;
	for (dependentFlightGroup = 0; dependentFlightGroup < g_missionHeader.flightGroupCount;
		 ++dependentFlightGroup) {
		if (dependentFlightGroup != g_curCraft->flightGroupIndex &&
			g_missionFlightGroups[dependentFlightGroup].arrivalMethod == MISSION_ARRIVAL_MOTHERSHIP &&
			g_missionFlightGroups[dependentFlightGroup].mothershipFlightGroupIndex ==
				g_curCraft->flightGroupIndex &&
			(g_missionFlightGroupStates[dependentFlightGroup].hasArrived == 0 ||
			 g_missionFlightGroupStates[dependentFlightGroup].wavesRemaining != 0)) {
			return 0;
		}
	}
	return 1;
}

// FUNCTION: XW 0x414160
int16_t paiorder_TargetGroupsBoardingAndArrivalComplete(void) {
	int16_t primaryComplete = paiorder_FlightGroupBoardingAndArrivalComplete(
		g_missionFlightGroups[g_curCraft->flightGroupIndex].primaryTarget);
	if (primaryComplete == 0) {
		return primaryComplete;
	}
	return paiorder_FlightGroupBoardingAndArrivalComplete(
			   g_missionFlightGroups[g_curCraft->flightGroupIndex].secondaryTarget) != 0;
}

// FUNCTION: XW 0x4141C0
int16_t paiorder_FlightGroupBoardingAndArrivalComplete(uint16_t flightGroupIndex) {
	if (flightGroupIndex != MISSION_TARGET_GROUP_UNSPECIFIED) {
		uint16_t objectIndex;
		if (g_missionFlightGroupStates[flightGroupIndex].hasArrived == 0) {
			return 0;
		}
		if (g_missionFlightGroupStates[flightGroupIndex].wavesRemaining != 0) {
			return 0;
		}
		for (objectIndex = 0; objectIndex < XW_CRAFT_OBJECT_COUNT; ++objectIndex) {
			if (g_objectTable[objectIndex].objectType != XW_OBJ_NONE) {
				const CraftData* craft = g_objectTable[objectIndex].instanceData;
				if (craft->flightGroupIndex == flightGroupIndex && craft->workingSubsystems != 0) {
					uint8_t plan = craft->aiCurrentPlanId;
					if (plan == PAI_PLAN_OUT_OF_HYPERSPACE || plan == PAI_PLAN_OUT_OF_HANGAR ||
						(plan >= PAI_PLAN_BOARDING_STAGE_FIRST && plan <= PAI_PLAN_BOARDING_STAGE_LAST)) {
						return 0;
					}
				}
			}
		}
	}
	return 1;
}

// FUNCTION: XW 0x414270
int16_t paiorder_TargetGroupsAreExhausted(void) {
	if (paiorder_FlightGroupHasRemainingCraft(
			g_missionFlightGroups[g_curCraft->flightGroupIndex].primaryTarget, 0) != 0) {
		return 0;
	}
	return paiorder_FlightGroupHasRemainingCraft(
			   g_missionFlightGroups[g_curCraft->flightGroupIndex].secondaryTarget, 0) == 0;
}

// FUNCTION: XW 0x4142D0
int16_t paiorder_FlightGroupHasRemainingCraft(uint16_t flightGroupIndex, int16_t requireWorkingSubsystems) {
	if (flightGroupIndex != MISSION_TARGET_GROUP_UNSPECIFIED) {
		uint16_t objectIndex;
		if (g_missionFlightGroupStates[flightGroupIndex].hasArrived == 0) {
			return 1;
		}
		if (g_missionFlightGroupStates[flightGroupIndex].wavesRemaining != 0) {
			return 1;
		}
		for (objectIndex = 0; objectIndex < XW_CRAFT_OBJECT_COUNT; ++objectIndex) {
			if (g_objectTable[objectIndex].objectType != XW_OBJ_NONE) {
				const CraftData* craft = g_objectTable[objectIndex].instanceData;
				if ((requireWorkingSubsystems == 0 || craft->workingSubsystems != 0) &&
					craft->flightGroupIndex == flightGroupIndex) {
					return 1;
				}
			}
		}
	}
	return 0;
}

// FUNCTION: XW 0x414370
int16_t paiorder_evasiveorder(void) {
	if (g_curCraft->aiManeuverId == g_paiInitialManeuverId &&
		g_curCraft->aiCandidateTargetOrSavedInterval == PAI_TARGET_EVASIVE) {
		g_curCraft->aiTargetRef = PAI_TARGET_NONE;
		g_curCraft->aiCandidateTargetOrSavedInterval = PAI_TARGET_NONE;
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x4143B0
int16_t paiorder_targetfromplayerorder(void) {
	if (g_curCraft->flightGroupIndex == g_playerFlightState.craft->flightGroupIndex) {
		uint16_t candidate = g_curCraft->aiCandidateTargetOrSavedInterval;
		if (candidate != PAI_TARGET_NONE && candidate != PAI_TARGET_EVASIVE) {
			if (pai_worthytarget(candidate) != 0) {
				if (g_curCraft->aiCandidateTargetOrSavedInterval != g_curCraft->aiTargetRef) {
					g_curCraft->aiTargetRef = g_curCraft->aiCandidateTargetOrSavedInterval;
				}
			} else {
				g_curCraft->aiCandidateTargetOrSavedInterval = PAI_TARGET_NONE;
			}
		}
	}
	return 0;
}

// FUNCTION: XW 0x414410
int16_t paiorder_avoidstarshiporder(void) {
	CraftData* savedCraft = g_curCraft;
	int16_t aiManeuverId = savedCraft->aiManeuverId;
	uint16_t collisionObjectIndex;
	uint16_t avoidYaw;
	uint16_t avoidPitch;

	if (aiManeuverId != PAIORDER_MANEUVER_AVOID_STARSHIP) {
		collisionObjectIndex =
			collide_craftstarshipcollision(g_paiObjectIndex, PAIORDER_STARSHIP_PREDICTION_SECONDS);
		g_curCraft = savedCraft;
		if (collisionObjectIndex == XW_OBJECT_SLOT_UNAVAILABLE) {
			return 0;
		}
		if (collisionObjectIndex == savedCraft->aiTargetRef &&
			(aiManeuverId == PAIORDER_MANEUVER_ATTACK ||
			 aiManeuverId == PAIORDER_MANEUVER_ATTACK_ALTERNATE)) {
			uint8_t objectType = g_objectTable[collisionObjectIndex].objectType;
#ifdef XW_MODERN
			if (objectType != XwFlightTypes_ObjectType(XW_OBJ_CALAMARI_CRUISER) &&
				objectType != XwFlightTypes_ObjectType(XW_OBJ_IMPERIAL_STAR_DESTROYER)) {
#else
			if (objectType != XW_OBJ_CALAMARI_CRUISER && objectType != XW_OBJ_IMPERIAL_STAR_DESTROYER) {
#endif
				return 0;
			}
		}
		if (savedCraft->craftIndexInFlightGroup & 1) {
			avoidYaw = (uint16_t)(g_objectTable[g_paiObjectIndex].yaw + PAIORDER_STARSHIP_AVOID_YAW);
		} else {
			avoidYaw = (uint16_t)(g_objectTable[g_paiObjectIndex].yaw - PAIORDER_STARSHIP_AVOID_YAW);
		}
		savedCraft->aiTargetYaw = avoidYaw;
		if (g_curCraft->aiTargetPitch > PAIORDER_STARSHIP_AVOID_PITCH) {
			avoidPitch = (uint16_t)(g_objectTable[g_paiObjectIndex].pitch - PAIORDER_STARSHIP_AVOID_PITCH);
		} else {
			avoidPitch = (uint16_t)(g_objectTable[g_paiObjectIndex].pitch + PAIORDER_STARSHIP_AVOID_PITCH);
		}
		g_curCraft->aiTargetPitch = avoidPitch;
		g_curCraft->aiManeuverId = PAIORDER_MANEUVER_AVOID_STARSHIP;
		paiman_initmaneuver();
	} else if (savedCraft->aiManeuverAuxTimerTicks == 0) {
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x414530
int16_t paiorder_IsFinalWaypointTarget(void) { return g_curCraft->aiTargetRef == PAI_TARGET_FINAL_WAYPOINT; }

// FUNCTION: XW 0x414550
int16_t paiorder_checkhyperorder(void) {
	return g_missionFlightGroups[g_curCraft->flightGroupIndex].departureMethod ==
		   MISSION_DEPARTURE_MOTHERSHIP;
}

// FUNCTION: XW 0x414580
int16_t paiorder_TargetGroupsHaveArrived(void) {
	uint16_t primaryTarget = g_missionFlightGroups[g_curCraft->flightGroupIndex].primaryTarget;
	if (primaryTarget != MISSION_TARGET_GROUP_UNSPECIFIED &&
		g_missionFlightGroupStates[primaryTarget].hasArrived == 0) {
		return 0;
	}
	if (g_missionFlightGroups[g_curCraft->flightGroupIndex].secondaryTarget !=
			MISSION_TARGET_GROUP_UNSPECIFIED &&
		g_missionFlightGroupStates[g_missionFlightGroups[g_curCraft->flightGroupIndex].secondaryTarget]
				.hasArrived == 0) {
		return 0;
	}
	return 1;
}

// FUNCTION: XW 0x414B80
int16_t paiorder_IsAimDistanceAbove131072(void) {
	return g_trig2PolarDistance > PAIORDER_AIM_DISTANCE_THRESHOLD;
}
