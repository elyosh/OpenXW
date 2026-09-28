#include "xw/flight/mission/mission.h"

#ifdef XW_MODERN
#include "xw_runtime/runtime/flight_types.h"
#endif
#ifdef XW_MODERN
#include "xw_runtime/runtime/flight_types.h"
#endif

#include "xw/audio/fsfx.h"
#include "xw/flight/ai/pai.h"
#include "xw/flight/death_star.h"
#include "xw/flight/hud/msg.h"
#include "xw/flight/object/anim.h"
#include "xw/flight/object/create.h"
#include "xw/flight/player/user.h"
#include "xw/flight/xw.h"

// GLOBAL: XW 0x4C52A8
const XwObjectTypeId g_craftTypeToObjectType[MISSION_CRAFT_TYPE_COUNT] = {
	0x00, 0x01, 0x02, 0x03, 0x05, 0x06, 0x07, 0x10, 0x15, 0x11, 0x18, 0x1A, 0x20, 0x31, 0x2A, 0x28,
	0x35, 0x08, 0x4B, 0x4B, 0x4D, 0x4D, 0x46, 0x53, 0x50, 0x55, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
	0x69, 0x69, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x79, 0x79, 0x79, 0x79, 0x79, 0x79,
	0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0xCA, 0xCB, 0xCC, 0xCC, 0xCD, 0xCE,
	0xCE, 0xCF, 0xD0, 0xD0, 0xD1, 0xD2, 0xD2, 0xD1, 0xD2, 0xD2, 0xD1, 0xD2, 0xD2, 0xCA, 0xCB, 0xCA,
	0xCB, 0xCA, 0xCB, 0xCA, 0xCB, 0xCA, 0xCB, 0xCA, 0xCB, 0xCA, 0xCB, 0xCA, 0xCB, 0xCA, 0xCB, 0xCA,
	0xCB, 0xCA, 0xCB, 0xCA, 0xCB, 0xCA, 0xCB, 0xCA, 0xCB, 0xCA, 0xCB, 0xCA, 0xCB, 0x00, 0x00, 0x00,
};

// GLOBAL: XW 0x62AF20
XwMissionHeader g_missionHeader = { 0 };

// GLOBAL: XW 0x62B960
XwMissionRuntimeState g_missionRuntimeState = { 0 };

// GLOBAL: XW 0x62BB40
MissionFlightGroupState g_missionFlightGroupStates[MISSION_FLIGHT_GROUP_COUNT] = { 0 };

// GLOBAL: XW 0x62BDC0
XwMissionFlightGroup g_missionFlightGroups[MISSION_FLIGHT_GROUP_COUNT] = { 0 };

// GLOBAL: XW 0x62CD20
XwMissionObjectRecord g_missionObjects[MISSION_OBJECT_COUNT] = { 0 };

// FUNCTION: XW 0x408530
void Mission_UpdateLogic(void) {
	if (g_missionRuntimeState.flightExitReason == MISSION_GOAL_EVALUATION_EXIT_REASON &&
		g_missionRuntimeState.provingGroundsActive == 0 && g_hyperspaceflag == 0) {
		int16_t allGoalsComplete = 1;
		int16_t hasRequiredGoals = 0;
		if (g_flightGlobalCountdownTimers.ticks[XW_TIMER_MISSION_GOAL] == 0 ||
			g_missionRuntimeState.flightExitRequested != 0) {
			uint16_t groupIndex;
			uint16_t objectIndex;
			uint8_t destroyMines;
			uint8_t destroySatellites;
			for (groupIndex = 0; groupIndex < g_missionHeader.flightGroupCount; ++groupIndex) {
				const XwMissionFlightGroup* group = &g_missionFlightGroups[groupIndex];
				const MissionFlightGroupState* state = &g_missionFlightGroupStates[groupIndex];
				uint16_t goalCondition = group->missionGoal;
				if (goalCondition == MISSION_GOAL_NONE) {
					g_missionRuntimeState.flightGroupGoalState[groupIndex] = 0;
				} else {
					int16_t goalComplete = 1;
					uint16_t totalCraft =
						(uint16_t)((group->additionalWaveCount + 1u) * group->numberOfCraft);
					uint16_t halfCraft = totalCraft >> 1;
					int16_t leaderPlan;
					hasRequiredGoals = 1;
					if ((totalCraft & 1) != 0)
						++halfCraft;
					leaderPlan = g_orderLeaderPlanId[group->order];
					if (state->spawnedCraftCount != totalCraft ||
						(state->wavesRemaining != 0 && goalCondition < MISSION_GOAL_ARRIVE)) {
						goalComplete = 0;
					} else {
						switch (goalCondition) {
							case MISSION_GOAL_DESTROY_ALL:
								if (totalCraft != state->destroyedCount)
									goalComplete = 0;
								break;
							case MISSION_GOAL_DESTROY_HALF:
								if (state->destroyedCount < halfCraft)
									goalComplete = 0;
								break;
							case MISSION_GOAL_DESTROY_SPECIAL:
								if (state->specialCraftDestroyed == 0)
									goalComplete = 0;
								break;
							case MISSION_GOAL_COMPLETE_ALL:
								if (leaderPlan == 0) {
									if (state->destroyedCount != 0)
										goalComplete = 0;
								} else if (totalCraft != state->outcomes[MISSION_OUTCOME_COMPLETED_SECOND] +
															 state->outcomes[MISSION_OUTCOME_COMPLETED_FIRST])
									goalComplete = 0;
								break;
							case MISSION_GOAL_COMPLETE_HALF:
								if (leaderPlan == 0) {
#ifdef XW_MODERN
									if (XwFlightTypes_Dos() ? state->destroyedCount >= halfCraft
															: state->destroyedCount > halfCraft)
#else
									if (state->destroyedCount > halfCraft)
#endif
										goalComplete = 0;
								} else if (state->outcomes[MISSION_OUTCOME_COMPLETED_SECOND] +
											   state->outcomes[MISSION_OUTCOME_COMPLETED_FIRST] <
										   halfCraft)
									goalComplete = 0;
								break;
							case MISSION_GOAL_COMPLETE_SPECIAL:
								if (state->outcomes[MISSION_OUTCOME_SPECIAL_COMPLETED] == 0)
									goalComplete = 0;
								break;
							case MISSION_GOAL_RECOVER_ALL:
								if (totalCraft != state->outcomes[MISSION_OUTCOME_RECOVERED])
									goalComplete = 0;
								break;
							case MISSION_GOAL_RECOVER_HALF:
								if (state->outcomes[MISSION_OUTCOME_RECOVERED] < halfCraft)
									goalComplete = 0;
								break;
							case MISSION_GOAL_RECOVER_SPECIAL:
								if (state->outcomes[MISSION_OUTCOME_SPECIAL_RECOVERED] == 0)
									goalComplete = 0;
								break;
							case MISSION_GOAL_BOARD_ALL:
								if (totalCraft != state->outcomes[MISSION_OUTCOME_BOARDED])
									goalComplete = 0;
								break;
							case MISSION_GOAL_BOARD_HALF:
								if (state->outcomes[MISSION_OUTCOME_BOARDED] < halfCraft)
									goalComplete = 0;
								break;
							case MISSION_GOAL_BOARD_SPECIAL:
								if (state->outcomes[MISSION_OUTCOME_SPECIAL_BOARDED] == 0)
									goalComplete = 0;
								break;
							case MISSION_GOAL_IDENTIFY_ALL:
								if (totalCraft != state->inspectedCount)
									goalComplete = 0;
								break;
							case MISSION_GOAL_IDENTIFY_HALF:
								if (state->inspectedCount < halfCraft)
									goalComplete = 0;
								break;
							case MISSION_GOAL_IDENTIFY_SPECIAL:
								if (state->specialCraftInspected == 0)
									goalComplete = 0;
								break;
							case MISSION_GOAL_ARRIVE:
								if (state->spawnedCraftCount == 0)
									goalComplete = 0;
								break;
						}
					}
					if (goalComplete == 0)
						allGoalsComplete = 0;
					g_missionRuntimeState.flightGroupGoalState[groupIndex] =
						goalComplete + XW_MISSION_GOAL_STATE_UNCOMPLETED;
				}
			}
			destroyMines = 0;
			destroySatellites = 0;
			g_missionRuntimeState.objectGoalResults.protectMines = 0;
			g_missionRuntimeState.objectGoalResults.destroyMines = 0;
			g_missionRuntimeState.objectGoalResults.protectSatellites = 0;
			g_missionRuntimeState.objectGoalResults.destroySatellites = 0;
			g_missionRuntimeState.objectGoalResults.protectProbes = 0;
			g_missionRuntimeState.objectGoalResults.destroyProbes = 0;
			for (objectIndex = 0; objectIndex < MISSION_OBJECT_COUNT; ++objectIndex) {
				const XwMissionObjectRecord* object = &g_missionObjects[objectIndex];
				if ((object->goalFlags & MISSION_OBJECT_GOAL_DESTROY) != 0) {
#ifdef XW_MODERN
					int16_t objectType = XwFlightTypes_CanonicalType(object->objectType);
#else
					int16_t objectType = object->objectType;
#endif
					hasRequiredGoals = 1;
					if (objectType == XW_OBJ_NONE)
#ifdef XW_MODERN
						objectType = XwFlightTypes_CanonicalType(object->genusId);
#else
						objectType = object->genusId;
#endif
					switch (objectType) {
						case MISSION_GOAL_SATELLITE_TYPE_FIRST:
						case MISSION_GOAL_SATELLITE_TYPE_SECOND:
							destroySatellites = XW_MISSION_GOAL_STATE_COMPLETED;
							break;
						case MISSION_GOAL_PROBE_TYPE:
							g_missionRuntimeState.objectGoalResults.destroyProbes =
								XW_MISSION_GOAL_STATE_COMPLETED;
							break;
						default:
							destroyMines = XW_MISSION_GOAL_STATE_COMPLETED;
							break;
					}
				} else if ((object->goalFlags & MISSION_OBJECT_GOAL_PROTECT) != 0) {
#ifdef XW_MODERN
					int16_t objectType = XwFlightTypes_CanonicalType(object->objectType);
#else
					int16_t objectType = object->objectType;
#endif
					hasRequiredGoals = 1;
					if (objectType == XW_OBJ_NONE)
#ifdef XW_MODERN
						objectType = XwFlightTypes_CanonicalType(object->genusId);
#else
						objectType = object->genusId;
#endif
					if (objectType == MISSION_GOAL_SATELLITE_TYPE_FIRST && destroySatellites == 0)
						g_missionRuntimeState.objectGoalResults.protectSatellites =
							XW_MISSION_GOAL_STATE_COMPLETED;
					else if (objectType == MISSION_GOAL_SATELLITE_TYPE_SECOND && destroySatellites == 0)
						g_missionRuntimeState.objectGoalResults.protectSatellites =
							XW_MISSION_GOAL_STATE_COMPLETED;
					else if (objectType == MISSION_GOAL_PROBE_TYPE &&
							 g_missionRuntimeState.objectGoalResults.destroyProbes == 0)
						g_missionRuntimeState.objectGoalResults.protectProbes =
							XW_MISSION_GOAL_STATE_COMPLETED;
					else if (destroyMines == 0)
						g_missionRuntimeState.objectGoalResults.protectMines =
							XW_MISSION_GOAL_STATE_COMPLETED;
				}
			}
			g_missionRuntimeState.objectGoalResults.destroyMines = destroyMines;
			g_missionRuntimeState.objectGoalResults.destroySatellites = destroySatellites;
			for (objectIndex = 0; objectIndex < MISSION_OBJECT_COUNT; ++objectIndex) {
				const XwMissionObjectRecord* object = &g_missionObjects[objectIndex];
				if ((object->goalFlags & MISSION_OBJECT_GOAL_DESTROY) != 0) {
					if (object->objectType != XW_OBJ_NONE) {
						allGoalsComplete = 0;
#ifdef XW_MODERN
						switch (XwFlightTypes_CanonicalType(object->objectType)) {
#else
						switch (object->objectType) {
#endif
							case MISSION_GOAL_SATELLITE_TYPE_FIRST:
							case MISSION_GOAL_SATELLITE_TYPE_SECOND:
								g_missionRuntimeState.objectGoalResults.destroySatellites =
									XW_MISSION_GOAL_STATE_UNCOMPLETED;
								break;
							case MISSION_GOAL_PROBE_TYPE:
								g_missionRuntimeState.objectGoalResults.destroyProbes =
									XW_MISSION_GOAL_STATE_UNCOMPLETED;
								break;
							default:
								g_missionRuntimeState.objectGoalResults.destroyMines =
									XW_MISSION_GOAL_STATE_UNCOMPLETED;
								break;
						}
					}
				} else if ((object->goalFlags & MISSION_OBJECT_GOAL_PROTECT) != 0 &&
						   object->objectType == XW_OBJ_NONE) {
					allGoalsComplete = 0;
					switch (object->genusId) {
						case MISSION_GOAL_SATELLITE_TYPE_FIRST:
						case MISSION_GOAL_SATELLITE_TYPE_SECOND:
							g_missionRuntimeState.objectGoalResults.protectSatellites =
								XW_MISSION_GOAL_STATE_UNCOMPLETED;
							break;
						case MISSION_GOAL_PROBE_TYPE:
							g_missionRuntimeState.objectGoalResults.protectProbes =
								XW_MISSION_GOAL_STATE_UNCOMPLETED;
							break;
						default:
							g_missionRuntimeState.objectGoalResults.protectMines =
								XW_MISSION_GOAL_STATE_UNCOMPLETED;
							break;
					}
				}
			}
			if (g_deathStarSurfaceModeActive != 0) {
				hasRequiredGoals = 1;
				if ((g_missionHeader.missionRuleFlags & MISSION_RULE_SURFACE_SPECIAL_TARGET) != 0) {
					if (g_surfaceSpecialTargetHit != 0) {
						g_missionRuntimeState.objectGoalResults.destroyMines =
							XW_MISSION_GOAL_STATE_COMPLETED;
					} else {
						allGoalsComplete = 0;
						g_missionRuntimeState.objectGoalResults.destroyMines =
							XW_MISSION_GOAL_STATE_UNCOMPLETED;
					}
				} else {
					uint16_t goalIndex;
					int16_t surfaceGoalsComplete = allGoalsComplete;
					g_missionRuntimeState.objectGoalResults.destroySatellites =
						XW_MISSION_GOAL_STATE_COMPLETED;
					for (goalIndex = 0; goalIndex < DEATH_STAR_SURFACE_GOAL_COUNT; ++goalIndex) {
						if (g_surfaceGoalCellKeys[goalIndex] != 0) {
							uint16_t initialSlot = g_surfaceGoalCellHashSlots[goalIndex];
							uint16_t slot = initialSlot;
							const XwSurfaceCellDamageState* cell = &g_surfaceCellDamageStates[slot];
							if (cell->cellKey != 0) {
								if ((int16_t)cell->cellKey != (int)g_surfaceGoalCellKeys[goalIndex]) {
									for (;;) {
										++slot;
										if (slot == DEATH_STAR_SURFACE_CELL_COUNT)
											slot = 0;
										cell = &g_surfaceCellDamageStates[slot];
										if (slot == initialSlot || cell->cellKey == 0) {
											surfaceGoalsComplete = 0;
											break;
										}
										if ((int16_t)cell->cellKey == (int)g_surfaceGoalCellKeys[goalIndex])
											break;
									}
								}
								if (surfaceGoalsComplete != 0) {
									int healthIndex;
									for (healthIndex = 0; healthIndex < MISSION_SURFACE_GOAL_HEALTH_COUNT;
										 ++healthIndex) {
										if (cell->objectHealthOrEffectState[healthIndex] != 0)
											surfaceGoalsComplete = 0;
									}
								}
							} else {
								surfaceGoalsComplete = 0;
							}
						}
					}
					allGoalsComplete = surfaceGoalsComplete;
					if (surfaceGoalsComplete == 0)
						g_missionRuntimeState.objectGoalResults.destroySatellites =
							XW_MISSION_GOAL_STATE_UNCOMPLETED;
				}
			}
			if (allGoalsComplete != 0 && hasRequiredGoals != 0) {
				uint16_t messageIndex;
				g_missionRuntimeState.objectivesCompleted = 1;
				g_flightGlobalCountdownTimers.ticks[XW_TIMER_MISSION_GOAL] = MISSION_GOAL_OUTCOME_SCAN_TICKS;
				msg_messageprintf(XW_MSG_MISSION_COMPLETE);
				fsfx_triggersfx(MISSION_COMPLETION_SOUND_SLOT, FSFX_UNPOSITIONED_OBJECT);
				for (messageIndex = 0; messageIndex < sizeof(g_missionHeader.completionMessages) /
														  sizeof(g_missionHeader.completionMessages[0]);
					 ++messageIndex) {
					if (g_missionHeader.completionMessages[messageIndex][0] != 0) {
						msg_addmessageptr(0, g_missionHeader.completionMessages[messageIndex]);
						msg_messageprintf(XW_MSG_MISSION_COMPLETION_TEXT);
					}
				}
			} else {
				int16_t noRemainingGoalObjects = 1;
				uint16_t craftIndex;
				for (craftIndex = 0; craftIndex < XW_CRAFT_OBJECT_COUNT; ++craftIndex) {
					if (craftIndex != g_playerFlightState.objectIndex &&
						g_objectTable[craftIndex].objectType != XW_OBJ_NONE) {
						const CraftData* craft = g_objectTable[craftIndex].instanceData;
						if (g_missionFlightGroups[craft->flightGroupIndex].missionGoal != MISSION_GOAL_NONE)
							noRemainingGoalObjects = 0;
					}
				}
				for (groupIndex = 0; groupIndex < g_missionHeader.flightGroupCount; ++groupIndex) {
					if (g_missionFlightGroupStates[groupIndex].hasArrived == 0)
						noRemainingGoalObjects = 0;
					if (g_missionFlightGroupStates[groupIndex].wavesRemaining != 0)
						noRemainingGoalObjects = 0;
				}
				for (objectIndex = 0; objectIndex < MISSION_OBJECT_COUNT; ++objectIndex) {
#ifdef XW_MODERN
					uint16_t objectType =
						XwFlightTypes_CanonicalType(g_missionObjects[objectIndex].objectType);
#else
					uint8_t objectType = g_missionObjects[objectIndex].objectType;
#endif
					if (objectType == MISSION_GOAL_SATELLITE_TYPE_FIRST ||
						objectType == MISSION_GOAL_SATELLITE_TYPE_SECOND ||
						objectType == MISSION_GOAL_PROBE_TYPE ||
						(objectType >= MISSION_GOAL_MINE_TYPE_FIRST &&
						 objectType <= MISSION_GOAL_MINE_TYPE_LAST))
						noRemainingGoalObjects = 0;
				}
				if (g_deathStarSurfaceModeActive != 0)
					noRemainingGoalObjects = 0;
				if (noRemainingGoalObjects != 0) {
#ifdef XW_MODERN
					if (!XwFlightTypes_Dos())
						g_missionRuntimeState.objectivesUnfinishable = 1;
#else
					g_missionRuntimeState.objectivesUnfinishable = 1;
#endif
					msg_messageprintf(XW_MSG_MISSION_OBJECTIVES_FAILED_RETURN_TO_BASE);
					g_flightGlobalCountdownTimers.ticks[XW_TIMER_MISSION_GOAL] =
						MISSION_GOAL_OUTCOME_SCAN_TICKS;
				} else {
					g_flightGlobalCountdownTimers.ticks[XW_TIMER_MISSION_GOAL] = MISSION_GOAL_SCAN_TICKS;
				}
			}
		}
	}
}
