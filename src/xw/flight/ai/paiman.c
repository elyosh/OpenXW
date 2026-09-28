#include "xw/flight/ai/paiman.h"

#ifdef XW_MODERN
#include "xw_runtime/timing/flight_integration.h"
#include "xw_runtime/timing/reference_motion.h"
#endif

#include "xw/assets/model_mesh.h"
#include "xw/audio/fsfx.h"
#include "xw/flight/ai/pai.h"
#include "xw/flight/death_star.h"
#include "xw/flight/fediskio.h"
#include "xw/flight/hud/msg.h"
#include "xw/flight/hud/panel.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/object.h"
#include "xw/flight/player/user.h"
#include "xw/flight/xw.h"
#include "xw/math/math2.h"
#include "xw/math/trig2.h"
#include "xw/util/shared.h"

#include <stdlib.h>

// GLOBAL: XW 0x4C9010
uint16_t g_aiTurnAwayStateDelayBySkill[PAI_PROFICIENCY_COUNT] = { 9, 6, 3 };

// GLOBAL: XW 0x4C9018
const int16_t g_escortSideOffsets[PAIMAN_ESCORT_SLOT_COUNT] = { 0, 0,     -4096, 0,    4096,
																0, -4096, -4096, 4096, 4096 };

// GLOBAL: XW 0x4C9030
const int16_t g_escortUpOffsets[PAIMAN_ESCORT_SLOT_COUNT] = { 2048,  2048,  2048,  2048,  2048,
															  -2048, -2048, -2048, -2048, -2048 };

// GLOBAL: XW 0x4C9048
const int16_t g_escortForwardOffsets[PAIMAN_ESCORT_SLOT_COUNT] = { 0, 4096, 0,     -4096, 0,
																   0, 4096, -4096, -4096, 4096 };

// GLOBAL: XW 0x4C9060
const uint16_t g_aiThrottleByPreset[PAIMAN_THROTTLE_PRESET_COUNT] = {
	0xFFFF, 0x3334, 0x4CCE, 0x6668, 0x8000, 0x999A, 0xB334, 0xCCCE, 0xE668, 0xFFFF
};

// GLOBAL: XW 0x4C90F0
XwManeuverInitFunction g_maneuverInitFunctions[PAIORDER_MANEUVER_COUNT] = {
	(XwManeuverInitFunction)Shared_ReturnZero,
	paiman_initturninsidemaneuver,
	paiman_initsplitsmaneuver,
	paiman_initimmelmannmaneuver,
	paiman_initscissorsmaneuver,
	paiman_initrendezvousmaneuver,
	paiman_initcruisemaneuver,
	paiman_initheadtowardfullmaneuver,
	paiman_initrunawaymaneuver,
	paiman_initheadonattackmaneuver,
	nullsub_SharedNoOp,
	paiman_initsetupattackmaneuver,
	paiman_initsetupattackmaneuver,
	paiman_initzoommaneuver,
	paiman_initdivemaneuver,
	paiman_initsplitsdivemaneuver,
	paiman_initspeedawaymaneuver,
	nullsub_SharedNoOp,
	paiman_initboardmaneuver,
	paiman_initawaitboardmaneuver,
	paiman_initheadtowardmaneuver,
	paiman_initintohyperspacemaneuver,
	paiman_initoutofhyperspacemaneuver,
	paiman_initsetupattackmaneuver,
	paiman_initturnawaymaneuver,
	paiman_initawaitboardmaneuver,
	paiman_initoutofhangarmaneuver,
	paiman_initsplitsdivemaneuver,
	paiman_initavoidstarshipmaneuver
};

// GLOBAL: XW 0x4C9168
const uint16_t g_hyperspaceExitSpeedByPhase[PAIMAN_HYPERSPACE_EXIT_SPEED_COUNT] = { 3600, 3600, 3600, 3600,
																					3600, 3600, 1800, 1800,
																					1800, 900,  900 };

// GLOBAL: XW 0x4C9180
int16_t g_formPosX[PAIMAN_FORMATION_POSITION_COUNT] = {
	0, 2,  -2, 0,  2,  -2, 0, -2, 6, 7,  -6, -7, 0, 0, 0, 0,  0, 0, 0, 1, -1, 2, -2, 3, 0, 1, 2,  3, 4,  5,
	0, -1, -2, -3, -4, -5, 0, 0,  0, -2, -2, -2, 0, 1, 0, -1, 0, 0, 0, 0, 0,  0, 0,  0, 0, 1, -1, 1, -1, 0
};

// GLOBAL: XW 0x4C91F8
int16_t g_formPosY[PAIMAN_FORMATION_POSITION_COUNT] = { 0, -2, -2, -4, -6, -6, 0, -2, -6, -7, -6, -7,
														0, -1, -2, -3, -4, -5, 0, 0,  0,  0,  0,  0,
														0, -1, -2, -3, -4, -5, 0, -1, -2, -3, -4, -5,
														0, -1, -2, 0,  -1, -2, 0, -1, -2, -1, -1, -1,
														0, 0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  -1 };

// GLOBAL: XW 0x4C9270
int16_t g_formPosZ[PAIMAN_FORMATION_POSITION_COUNT] = { 0, 1, 1,  4, 5, 5, 0, -1, 5, 4, 5, 4, 0,  1,  2,
														3, 4, 5,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0,  0,
														0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0,  0,
														0, 1, -1, 0, 1, 2, 3, 4,  5, 0, 1, 1, -1, -1, 0 };

// GLOBAL: XW 0x4C92E8
int16_t g_formationHorizontalSpacing[PAIMAN_FORMATION_SPACING_COUNT] = { 1000, 1350, 1800, 2250, 2600, 3000,
																		 3250, 3400, 3500, 250,  0,    0 };

// GLOBAL: XW 0x4C9300
int16_t g_formationVerticalSpacing[PAIMAN_FORMATION_SPACING_COUNT] = { 500,  700,  1150, 1600, 2050, 2400,
																	   2850, 2100, 3300, 75,   0,    0 };

// GLOBAL: XW 0x63ADC4
XwManeuverInitFunction g_currentManeuverInitFunction = NULL;

// FUNCTION: XW 0x4161E0
void paiman_initmaneuver(void) {
#ifdef XW_MODERN
	XwFlightIntegration_ResetManeuver(g_paiObjectIndex);
#endif
	g_curCraft->aiDisplacementX = 0;
	g_curCraft->aiDisplacementY = 0;
	g_curCraft->aiDisplacementZ = 0;
	g_curCraft->aiHitsThisManeuver = 0;
	g_curCraft->aiManeuverPhase = 0;
	g_currentManeuverInitFunction = g_maneuverInitFunctions[g_curCraft->aiManeuverId];
	/* The folded zero-return entry has a different return type from the initializers. */
	if (g_currentManeuverInitFunction == (XwManeuverInitFunction)Shared_ReturnZero) {
		((XwManeuverFunction)g_currentManeuverInitFunction)();
	} else {
		g_currentManeuverInitFunction();
	}
}

// FUNCTION: XW 0x416230
void paiman_initturninsidemaneuver(void) {
	paiman_setnewturninside(g_paiObjectIndex);
	g_curCraft->aiManeuverTimerTicks = PAIMAN_TURN_INSIDE_DURATION_TICKS;
}

// FUNCTION: XW 0x416250
int16_t paiman_turninsidemaneuver(void) {
	if (g_curCraft->aiManeuverTimerTicks == 0) {
		return 1;
	}
	if (g_curCraft->aiManeuverAuxTimerTicks == 0) {
		paiman_setnewturninside(g_paiObjectIndex);
	}
	return 0;
}

// FUNCTION: XW 0x416280
void paiman_setnewturninside(uint16_t ownObjectIndex) {
	uint16_t targetObjectIndex = g_curCraft->lastAttackerObjIdx;
	if (targetObjectIndex == PAI_TARGET_NONE) {
		targetObjectIndex = ownObjectIndex;
	}
	g_curCraft->aiTargetYaw = g_objectTable[targetObjectIndex].yaw + PAIMAN_HALF_TURN;
	paiman_setturn((g_curCraft->aiSkillQ16 >> 1) + PAIMAN_TURN_STEP_HALF);
	g_curCraft->aiManeuverAuxTimerTicks =
		PAIMAN_TICKS_PER_SECOND * g_aiTurnAwayStateDelayBySkill[g_paiSkillTier];
}

// FUNCTION: XW 0x4162F0
void paiman_initsplitsmaneuver(void) {
	g_curCraft->aiRollState = XW_AI_ROLL_STATE_1;
	g_curCraft->aiRollStepQ16 = XW_AI_COMMAND_FULL;
	g_curCraft->aiTargetRoll = PAIMAN_HALF_TURN;
	g_curCraft->aiYawState = XW_AI_YAW_INACTIVE;
	g_curCraft->aiPitchForce = 1;
	g_curCraft->aiTargetPitch = PAIMAN_QUARTER_TURN;
	g_curCraft->aiPitchState = XW_AI_PITCH_INCREMENT;
	g_curCraft->aiPitchStepQ16 = XW_AI_COMMAND_FULL;
}

// FUNCTION: XW 0x416350
void paiman_initimmelmannmaneuver(void) {
	paiman_setpower(g_paiObjectIndex, XW_AI_COMMAND_FULL);
	g_curCraft->aiRollState = XW_AI_ROLL_STATE_1;
	g_curCraft->aiRollStepQ16 = XW_AI_COMMAND_FULL;
	g_curCraft->aiTargetRoll = 0;
	g_curCraft->aiYawState = XW_AI_YAW_INACTIVE;
	g_curCraft->aiTargetPitch = PAIMAN_QUARTER_TURN;
	g_curCraft->aiPitchStepQ16 = XW_AI_COMMAND_FULL;
	g_curCraft->aiPitchForce = 0;
	if (g_curCraft->pitch < PAIMAN_QUARTER_TURN)
		g_curCraft->aiPitchState = XW_AI_PITCH_INCREMENT;
	else if (g_curCraft->pitch > PAIMAN_QUARTER_TURN)
		g_curCraft->aiPitchState = XW_AI_PITCH_DECREMENT;
	else
		g_curCraft->aiPitchState = XW_AI_PITCH_TARGET_REACHED;
	g_curCraft->aiManeuverTimerTicks = 0;
}

// FUNCTION: XW 0x4163F0
int16_t paiman_immelmannmaneuver(void) {
	switch (g_curCraft->aiManeuverPhase) {
		case PAIMAN_IMMELMANN_INITIAL_PITCH:
			if (g_curCraft->aiPitchState == XW_AI_PITCH_TARGET_REACHED) {
				g_curCraft->aiPitchState = XW_AI_PITCH_DECREMENT;
				g_curCraft->aiPitchStepQ16 = XW_AI_COMMAND_FULL;
				g_curCraft->aiPitchForce = 1;
				g_curCraft->aiTargetPitch = PAIMAN_QUARTER_TURN;
				g_curCraft->aiManeuverPhase = PAIMAN_IMMELMANN_SECOND_PITCH;
			}
			break;
		case PAIMAN_IMMELMANN_SECOND_PITCH:
			if (g_curCraft->aiPitchState == XW_AI_PITCH_TARGET_REACHED) {
				g_curCraft->aiRollState = XW_AI_ROLL_STATE_1;
				g_curCraft->aiRollStepQ16 = XW_AI_COMMAND_FULL;
				g_curCraft->aiTargetRoll = 0;
				g_curCraft->aiYawState = XW_AI_YAW_INACTIVE;
				g_curCraft->aiManeuverPhase = PAIMAN_IMMELMANN_ROLL_UPRIGHT;
			}
			break;
		case PAIMAN_IMMELMANN_ROLL_UPRIGHT:
			if (g_curCraft->aiRollState == XW_AI_ROLL_TARGET_REACHED &&
				g_curCraft->aiPitchState == XW_AI_PITCH_TARGET_REACHED) {
				return 1;
			}
			break;
	}
	return 0;
}

// FUNCTION: XW 0x416490
void paiman_initscissorsmaneuver(void) {
	int targetObjectIndex;
	if (g_curCraft->lastAttackerObjIdx != PAI_TARGET_NONE) {
		targetObjectIndex = g_curCraft->lastAttackerObjIdx;
	} else {
		targetObjectIndex = g_paiObjectIndex;
	}
	g_curCraft->aiTargetYaw = g_objectTable[targetObjectIndex].yaw + PAIMAN_HALF_TURN;
	paiman_setturn((g_curCraft->aiSkillQ16 >> 1) + PAIMAN_TURN_STEP_HALF);
	g_curCraft->aiRollState = XW_AI_ROLL_STATE_3;
	g_curCraft->aiRollStepQ16 = XW_AI_COMMAND_FULL;
	g_curCraft->aiTargetRoll = math2_getrandom();
	g_curCraft->aiManeuverTimerTicks = PAIMAN_SCISSORS_DURATION_TICKS;
	g_curCraft->aiManeuverAuxTimerTicks = PAIMAN_SCISSORS_INITIAL_INTERVAL_TICKS;
}

// FUNCTION: XW 0x416520
int16_t paiman_scissorsmaneuver(void) {
	if (g_curCraft->aiManeuverTimerTicks == 0) {
		g_curCraft->aiRollState = XW_AI_ROLL_TARGET_REACHED;
		return 1;
	}
	if (g_curCraft->aiManeuverAuxTimerTicks == 0) {
		g_curCraft->aiYawState = XW_AI_YAW_STATE_1;
		g_curCraft->aiTargetYaw += PAIMAN_HALF_TURN;
		g_curCraft->aiTargetRoll ^= PAIMAN_HALF_TURN;
		g_curCraft->aiManeuverAuxTimerTicks = PAIMAN_SCISSORS_INTERVAL_TICKS;
	}
	return 0;
}

// FUNCTION: XW 0x416570
void paiman_initrendezvousmaneuver(void) {
	paiman_setflighttotarget(0, 1);
	paiman_setpower(g_paiObjectIndex, g_aiThrottleByPreset[g_curCraft->aiOrderParameter]);
}

// FUNCTION: XW 0x4165B0
int16_t paiman_rendezvousmaneuver(void) {
	paiman_setflighttotarget(0, 1);
	paiman_setpower(g_paiObjectIndex, g_aiThrottleByPreset[g_curCraft->aiOrderParameter]);
	return 0;
}

// FUNCTION: XW 0x4165F0
void paiman_initcruisemaneuver(void) {
	paiman_controlplane();
	if ((uint16_t)g_objectTable[g_paiObjectIndex].roll < PAIMAN_HALF_TURN) {
		paiman_setflighttotarget(0, 1);
	}
	g_curCraft->aiManeuverAuxTimerTicks = PAIMAN_TICKS_PER_SECOND;
	paiman_setpower(g_paiObjectIndex, g_aiThrottleByPreset[g_curCraft->aiOrderParameter]);
}

// FUNCTION: XW 0x416650
int16_t paiman_cruisemaneuver(void) {
	uint16_t objectIndex = g_paiObjectIndex;
#ifdef XW_MODERN
	trig2_ctop((int32_t)((uint32_t)g_curCraft->aiAimPointX - (uint32_t)g_objectTable[objectIndex].worldX),
			   (int32_t)((uint32_t)g_curCraft->aiAimPointY - (uint32_t)g_objectTable[objectIndex].worldY),
			   (int32_t)((uint32_t)g_curCraft->aiAimPointZ - (uint32_t)g_objectTable[objectIndex].worldZ));
#else
	trig2_ctop(g_curCraft->aiAimPointX - g_objectTable[objectIndex].worldX,
			   g_curCraft->aiAimPointY - g_objectTable[objectIndex].worldY,
			   g_curCraft->aiAimPointZ - g_objectTable[objectIndex].worldZ);
#endif
	if (g_trig2PolarDistance < (g_objectTable[objectIndex].genusId == XW_GENUS_STARSHIP
									? PAIMAN_CRUISE_STARSHIP_WAYPOINT_DISTANCE
									: PAIMAN_CRUISE_WAYPOINT_DISTANCE)) {
		paiman_gonextwaypoint(objectIndex);
	}
	if (g_curCraft->aiManeuverAuxTimerTicks == 0) {
		if (g_curCraft->aiDiveState != PAIMAN_CRUISE_VERTICAL_ACTIVE &&
			g_curCraft->aiClimbState != PAIMAN_CRUISE_VERTICAL_ACTIVE) {
#ifdef XW_MODERN
			int verticalError =
				(int32_t)((uint32_t)g_curCraft->aiAimPointZ - (uint32_t)g_objectTable[objectIndex].worldZ);
#else
			int verticalError = g_curCraft->aiAimPointZ - g_objectTable[objectIndex].worldZ;
#endif
			if (verticalError < 0) {
#ifdef XW_MODERN
				verticalError = (int32_t)(0u - (uint32_t)verticalError);
#else
				verticalError = -verticalError;
#endif
			}
			if (verticalError > PAIMAN_CRUISE_VERTICAL_TOLERANCE) {
				g_curCraft->aiTargetPitch = g_trig2Pitch;
				if (g_curCraft->aiTargetPitch <= g_curCraft->pitch) {
					g_curCraft->aiPitchState = XW_AI_PITCH_DECREMENT;
				} else {
					g_curCraft->aiPitchState = XW_AI_PITCH_INCREMENT;
				}
				g_curCraft->aiClimbState = PAIMAN_CRUISE_VERTICAL_ACTIVE;
				paiman_setpower(objectIndex, PAIMAN_CRUISE_CLIMB_THROTTLE);
			}
		}
		paiman_setflighttotarget(0, 1);
		g_curCraft->aiManeuverAuxTimerTicks = PAIMAN_CRUISE_REFRESH_TICKS;
		if (g_curCraft->aiYawState == XW_AI_YAW_TARGET_REACHED && g_objectTable[g_paiObjectIndex].roll != 0) {
			g_curCraft->aiRollState = XW_AI_ROLL_STATE_1;
			g_curCraft->aiRollStepQ16 = XW_AI_COMMAND_FULL;
			g_curCraft->aiTargetRoll = 0;
		}
	}
	if (g_objectTable[objectIndex].genusId == XW_GENUS_STARSHIP) {
		uint16_t yawError = g_objectTable[g_paiObjectIndex].yaw - g_curCraft->aiTargetYaw;
		if (yawError >= PAIMAN_HALF_TURN) {
			yawError = -yawError;
		}
		if (g_curCraft->aiYawState == XW_AI_YAW_STEER && yawError >= PAIMAN_CRUISE_STOP_YAW_ERROR) {
			paiman_setpower(g_paiObjectIndex, 0);
			return 0;
		}
	}
	paiman_setpower(g_paiObjectIndex, g_aiThrottleByPreset[g_curCraft->aiOrderParameter]);
	return 0;
}

// FUNCTION: XW 0x416820
void paiman_gonextwaypoint(uint16_t unusedObjectIndex) {
	uint8_t plan = g_curCraft->aiOrderPlanId;
	(void)unusedObjectIndex;
	++g_curCraft->aiWaypointIndex;
	if (plan == PAI_PLAN_1 || plan == PAI_PLAN_3 || plan == PAI_PLAN_61) {
		if (g_curCraft->aiWaypointIndex > PAI_WAYPOINT_LAST) {
			g_curCraft->aiWaypointIndex = PAI_WAYPOINT_FINAL;
		}
		if (g_missionFlightGroups[g_curCraft->flightGroupIndex]
				.waypointEnabled[g_curCraft->aiWaypointIndex] == 0) {
			g_curCraft->aiWaypointIndex = PAI_WAYPOINT_FINAL;
		}
	} else {
		if (g_curCraft->aiWaypointIndex > PAI_WAYPOINT_LAST) {
			g_curCraft->aiWaypointIndex = PAI_WAYPOINT_FIRST;
		}
		if (g_missionFlightGroups[g_curCraft->flightGroupIndex]
				.waypointEnabled[g_curCraft->aiWaypointIndex] == 0) {
			g_curCraft->aiWaypointIndex = PAI_WAYPOINT_FIRST;
		}
	}
	g_curCraft->aiTargetRef = g_curCraft->aiWaypointIndex + PAI_TARGET_WAYPOINT_BASE;
	pai_settarget();
}

// FUNCTION: XW 0x4168D0
void paiman_initheadtowardfullmaneuver(void) {
	paiman_setflighttotarget(0, 0);
	paiman_setpower(g_paiObjectIndex, XW_CRAFT_THROTTLE_FULL);
	g_curCraft->aiManeuverAuxTimerTicks = PAIMAN_HEAD_TOWARD_FULL_DURATION_TICKS;
}

// FUNCTION: XW 0x416900
int16_t paiman_headtowardfullmaneuver(void) {
	if (g_curCraft->aiManeuverAuxTimerTicks == 0) {
		pai_settarget();
		paiman_setflighttotarget(0, 0);
		paiman_setpower(g_paiObjectIndex, XW_CRAFT_THROTTLE_FULL);
		g_curCraft->aiManeuverAuxTimerTicks = PAIMAN_HEAD_TOWARD_FULL_DURATION_TICKS;
	}
	return 0;
}

// FUNCTION: XW 0x416950
void paiman_initrunawaymaneuver(void) {
	paiman_controlplane();
	if ((uint16_t)g_objectTable[g_paiObjectIndex].roll < PAIMAN_HALF_TURN) {
		paiman_setflighttotarget((int16_t)PAIMAN_HALF_TURN, 1);
	}
}

// FUNCTION: XW 0x416980
int16_t paiman_runawaymaneuver(void) {
	paiman_setflighttotarget((int16_t)PAIMAN_HALF_TURN, 1);
	paiman_setpower(g_paiObjectIndex, XW_CRAFT_THROTTLE_FULL);
	return 0;
}

// FUNCTION: XW 0x4169B0
void paiman_initheadonattackmaneuver(void) {
	g_curCraft->aiTargetRef = g_curCraft->lastAttackerObjIdx;
	paiman_setflighttotarget(0, 1);
	paiman_setpower(g_paiObjectIndex, XW_CRAFT_THROTTLE_FULL);
	g_curCraft->aiManeuverTimerTicks = PAIMAN_HEAD_ON_ATTACK_DURATION_TICKS;
}

// FUNCTION: XW 0x4169F0
int16_t paiman_headonattackmaneuver(void) {
	if (g_curCraft->aiManeuverTimerTicks == 0) {
		return 1;
	}
	paiman_setflighttotarget(0, 1);
	paiman_setpower(g_paiObjectIndex, XW_CRAFT_THROTTLE_FULL);
	return 0;
}

// FUNCTION: XW 0x416A30
int16_t paiman_followleadermaneuver(void) {
	uint16_t selfObjectIndex = g_paiObjectIndex;
	uint8_t leaderIndex = g_curCraft->aiLeaderObjectIndex;
	uint16_t yaw, leaderSpeed, selfSpeed, oldThrottle, pitch, pitchError, roll, rollError;
	ObjectRecord* selfObject;

	pai_distancebetween(leaderIndex, selfObjectIndex);
	if (g_trig2PolarDistance > PAIMAN_FOLLOW_CATCHUP_DISTANCE) {
		g_curCraft->aiAimPointX = g_objectTable[leaderIndex].worldX;
		g_curCraft->aiAimPointY = g_objectTable[leaderIndex].worldY;
		g_curCraft->aiAimPointZ = g_objectTable[leaderIndex].worldZ;
		paiman_setflighttotarget(0, 1);
		paiman_setpower(selfObjectIndex, XW_CRAFT_THROTTLE_FULL);
		g_curCraft->aiDisplacementX = 0;
		g_curCraft->aiDisplacementY = 0;
		g_curCraft->aiDisplacementZ = 0;
#ifdef XW_MODERN
		XwFlightIntegration_ClearPush(g_paiObjectIndex);
#endif
		return 0;
	}
	if (g_paiLeaderCraft->aiYawState == XW_AI_YAW_STEER) {
		g_curCraft->aiTargetYaw = g_paiLeaderCraft->aiTargetYaw;
		paiman_setturn((g_curCraft->aiSkillQ16 >> 3) + PAIMAN_TURN_STEP_QUARTER);
	} else {
		yaw = g_objectTable[leaderIndex].yaw;
		if (yaw != (uint16_t)g_objectTable[selfObjectIndex].yaw) {
			g_curCraft->aiTargetYaw = yaw;
			paiman_setturn((g_curCraft->aiSkillQ16 >> 3) + PAIMAN_TURN_STEP_QUARTER);
		}
	}
	if (leaderIndex == g_playerFlightState.objectIndex) {
		leaderSpeed = g_objectTable[leaderIndex].speed;
		selfSpeed = g_objectTable[selfObjectIndex].speed;
		if (leaderSpeed > selfSpeed) {
			oldThrottle = g_curCraft->engineThrottle[0];
			paiman_setpower(selfObjectIndex, oldThrottle + PAIMAN_FOLLOW_SPEED_GAIN * leaderSpeed -
												 PAIMAN_FOLLOW_SPEED_GAIN * selfSpeed);
			if (g_curCraft->engineThrottle[0] < oldThrottle)
				paiman_setpower(selfObjectIndex, XW_CRAFT_THROTTLE_FULL);
		} else if (leaderSpeed < selfSpeed) {
			oldThrottle = g_curCraft->engineThrottle[0];
			paiman_setpower(selfObjectIndex, oldThrottle + PAIMAN_FOLLOW_SPEED_GAIN * leaderSpeed -
												 PAIMAN_FOLLOW_SPEED_GAIN * selfSpeed);
			if (g_curCraft->engineThrottle[0] > oldThrottle)
				paiman_setpower(selfObjectIndex, 0);
		}
	} else {
		paiman_setpower(selfObjectIndex, g_paiLeaderCraft->engineThrottle[0]);
	}
	pitch = g_paiLeaderCraft->pitch;
	pitchError = g_curCraft->pitch - pitch;
	if (pitchError >= PAIMAN_HALF_TURN)
		pitchError = -pitchError;
	if (pitchError < PAIMAN_FOLLOW_ANGLE_SNAP) {
		g_curCraft->pitch = pitch;
		g_curCraft->aiPitchState = XW_AI_PITCH_INACTIVE;
	} else {
		g_curCraft->aiTargetPitch = pitch;
		if (g_curCraft->aiTargetPitch <= g_curCraft->pitch)
			g_curCraft->aiPitchState = XW_AI_PITCH_DECREMENT;
		else
			g_curCraft->aiPitchState = XW_AI_PITCH_INCREMENT;
		g_curCraft->aiPitchStepQ16 = XW_AI_COMMAND_FULL;
		g_curCraft->aiPitchForce = 0;
	}
	selfObject = &g_objectTable[selfObjectIndex];
	roll = g_objectTable[g_paiLeaderObjectIndex].roll;
	rollError = selfObject->roll - roll;
	if (rollError >= PAIMAN_HALF_TURN)
		rollError = -rollError;
	if (rollError < PAIMAN_FOLLOW_ANGLE_SNAP) {
		selfObject->roll = roll;
		selfObject->orientMatrixDirty = 1;
		g_curCraft->aiRollState = 0;
	} else {
		g_curCraft->aiTargetRoll = roll;
		g_curCraft->aiRollStepQ16 = XW_AI_COMMAND_FULL;
		g_curCraft->aiRollState = XW_AI_ROLL_STATE_1;
	}
	if (g_curCraft->aiLeaderObjectIndex != g_playerFlightState.objectIndex) {
		paiman_calcformation();
		return 0;
	}
	if (g_playerFlightState.object->speed > PAIMAN_FOLLOW_MIN_FORMATION_SPEED) {
		paiman_calcformation();
		return 0;
	}
	g_curCraft->aiDisplacementX = 0;
	g_curCraft->aiDisplacementY = 0;
	g_curCraft->aiDisplacementZ = 0;
#ifdef XW_MODERN
	XwFlightIntegration_ClearPush(g_paiObjectIndex);
#endif
	return 0;
}

// FUNCTION: XW 0x416D80
void paiman_initsetupattackmaneuver(void) {
	paiman_setpower(g_paiObjectIndex, XW_CRAFT_THROTTLE_FULL);
	paiman_attacktarget(0);
}

// FUNCTION: XW 0x416DA0
int16_t paiman_setupattackmaneuver(void) {
	uint16_t yawError;
	paiman_attacktarget(0);
	yawError = g_objectTable[g_paiObjectIndex].yaw - g_curCraft->aiTargetYaw;
	if (yawError >= PAIMAN_SETUP_ATTACK_HALF_THROTTLE_YAW_MIN &&
		yawError <= PAIMAN_SETUP_ATTACK_HALF_THROTTLE_YAW_MAX) {
		paiman_setpower(g_paiObjectIndex, XW_CRAFT_THROTTLE_HALF);
		return 0;
	} else {
		paiman_setpower(g_paiObjectIndex, XW_CRAFT_THROTTLE_FULL);
		return 0;
	}
}

// FUNCTION: XW 0x416E00
int16_t paiman_attackmaneuver(void) {
	uint16_t objectIndex = g_paiObjectIndex;
	int breakRange;
	switch (g_curCraft->aiManeuverPhase) {
		case 1:
			if (g_curCraft->aiManeuverTimerTicks == 0)
				return 1;
			break;
		case 0: {
			pai_distancebetween(objectIndex, g_curCraft->aiTargetRef);
			if (g_objectTable[g_curCraft->aiTargetRef].genusId == XW_GENUS_STARSHIP ||
				g_objectTable[g_curCraft->aiTargetRef].genusId == XW_GENUS_FREIGHTER) {
				uint16_t angle = g_trig2Yaw - g_objectTable[g_curCraft->aiTargetRef].yaw;
				breakRange = PAIMAN_ATTACK_BREAK_LARGE_RANGE;
				if (angle >= PAIMAN_HALF_TURN)
					angle = -angle;
				if (angle < PAIMAN_ATTACK_LARGE_SIDE_ANGLE_MIN || angle > PAIMAN_ATTACK_LARGE_SIDE_ANGLE_MAX)
					breakRange = PAIMAN_ATTACK_BREAK_LARGE_END_RANGE;
			} else {
				uint16_t yawError;
				uint16_t pitchError;
				breakRange = PAIMAN_ATTACK_BREAK_SMALL_RANGE;
				yawError = g_objectTable[objectIndex].yaw - g_objectTable[g_curCraft->aiTargetRef].yaw;
				if (yawError >= PAIMAN_HALF_TURN)
					yawError = -yawError;
				pitchError = g_objectTable[objectIndex].pitch - g_objectTable[g_curCraft->aiTargetRef].pitch;
				if (pitchError >= PAIMAN_HALF_TURN)
					pitchError = -pitchError;
				if (pitchError > PAIMAN_QUARTER_TURN || yawError > PAIMAN_QUARTER_TURN)
					breakRange = PAIMAN_ATTACK_BREAK_OPPOSING_RANGE;
			}
			if (g_trig2PolarDistance < breakRange ||
				g_curCraft->aiHitsThisManeuver >=
					g_craftTypeDefs[g_curCraft->craftTypeIndex].evadeHitThreshold ||
				(g_deathStarSurfaceModeActive != 0 && g_objectTable[g_paiObjectIndex].worldZ < 0)) {
				int yawBias = ((uint16_t)math2_getrandom() & PAIMAN_ATTACK_BREAK_YAW_MASK) +
							  PAIMAN_ATTACK_BREAK_YAW_BASE;
				uint8_t targetGenus;
				if ((uint16_t)math2_getrandom() >= PAIMAN_HALF_TURN)
					yawBias = -yawBias;
				g_curCraft->aiTargetYaw = yawBias + g_objectTable[objectIndex].yaw;
				paiman_setturn((g_curCraft->aiSkillQ16 >> 1) + PAIMAN_TURN_STEP_HALF);
				if (g_deathStarSurfaceModeActive != 0)
					g_curCraft->aiTargetPitch = PAIMAN_ATTACK_SURFACE_PITCH_BASE -
												(math2_getrandom() & PAIMAN_ATTACK_SURFACE_PITCH_MASK);
				else
					g_curCraft->aiTargetPitch = math2_getrandom() & PAIMAN_ATTACK_PITCH_MASK;
				g_curCraft->aiClimbState = 0;
				g_curCraft->aiDiveState = 0;
				if (g_curCraft->aiTargetPitch <= g_curCraft->pitch)
					g_curCraft->aiPitchState = XW_AI_PITCH_DECREMENT;
				else
					g_curCraft->aiPitchState = XW_AI_PITCH_INCREMENT;
				g_curCraft->aiPitchForce = 0;
				g_curCraft->aiPitchStepQ16 = XW_AI_COMMAND_FULL;
				++g_curCraft->aiOrderProgress;
				g_curCraft->aiWarheadsFiredThisRun = 0;
				g_curCraft->warheadLockTicks = 0;
				paiman_setpower(objectIndex, XW_AI_COMMAND_FULL);
				targetGenus = g_objectTable[g_curCraft->aiTargetRef].genusId;
				if (targetGenus != XW_GENUS_STARSHIP && targetGenus != XW_GENUS_FREIGHTER)
					g_curCraft->aiManeuverTimerTicks =
						PAIMAN_TICKS_PER_SECOND * ((math2_getrandom() & PAIMAN_ATTACK_BREAK_SECONDS_MASK) +
												   PAIMAN_ATTACK_BREAK_SECONDS_BASE);
				else
					g_curCraft->aiManeuverTimerTicks =
						PAIMAN_TICKS_PER_SECOND * ((math2_getrandom() & PAIMAN_AVOID_STARSHIP_RANDOM_MASK) +
												   PAIMAN_AVOID_STARSHIP_BASE_SECONDS);
				g_curCraft->aiManeuverPhase = 1;
			} else {
				paiman_attacktarget(0);
				if (g_curCraft->aiManeuverId == PAIORDER_MANEUVER_ATTACK)
					paiman_setpower(objectIndex, XW_AI_COMMAND_FULL);
				else
					paiman_setpower(objectIndex, PAIMAN_ATTACK_ALTERNATE_THROTTLE);
			}
			break;
		}
		default:
			break;
	}
	return 0;
}

// FUNCTION: XW 0x417110
void paiman_initzoommaneuver(void) {
	uint16_t objectIndex = g_paiObjectIndex;
	paiman_setpower(objectIndex, XW_AI_COMMAND_FULL);
	g_curCraft->aiRollState = XW_AI_ROLL_STATE_3;
	g_curCraft->aiRollStepQ16 = XW_AI_COMMAND_FULL;
	g_curCraft->aiTargetRoll = math2_getrandom();
	g_curCraft->aiTargetPitch = PAIMAN_ZOOM_PITCH_BASE - (math2_getrandom() & PAIMAN_ZOOM_PITCH_RANDOM_MASK);
	g_curCraft->aiManeuverTimerTicks =
		PAIMAN_TICKS_PER_SECOND *
		((math2_getrandom() & PAIMAN_ZOOM_SECONDS_RANDOM_MASK) + PAIMAN_ZOOM_BASE_SECONDS);
	g_curCraft->aiClimbState = 0;
	g_curCraft->aiDiveState = 0;
	if (g_curCraft->aiTargetPitch <= g_curCraft->pitch) {
		g_curCraft->aiPitchState = XW_AI_PITCH_DECREMENT;
	} else {
		g_curCraft->aiPitchState = XW_AI_PITCH_INCREMENT;
	}
	g_curCraft->aiPitchForce = 0;
	g_curCraft->aiPitchStepQ16 = XW_AI_COMMAND_FULL;
}

// FUNCTION: XW 0x4171E0
int16_t paiman_zoommaneuver(void) { return g_curCraft->aiManeuverTimerTicks == 0; }

// FUNCTION: XW 0x4171F0
void paiman_initdivemaneuver(void) {
	uint16_t objectIndex = g_paiObjectIndex;
	paiman_setpower(objectIndex, XW_AI_COMMAND_FULL);
	g_curCraft->aiTargetPitch = (math2_getrandom() & PAIMAN_DIVE_PITCH_RANDOM_MASK) + PAIMAN_DIVE_PITCH_BASE;
	g_curCraft->aiClimbState = 0;
	if (g_curCraft->aiTargetPitch <= g_curCraft->pitch) {
		g_curCraft->aiPitchState = XW_AI_PITCH_DECREMENT;
	} else {
		g_curCraft->aiPitchState = XW_AI_PITCH_INCREMENT;
	}
	g_curCraft->aiPitchForce = 0;
	g_curCraft->aiPitchStepQ16 = XW_AI_COMMAND_FULL;
	g_curCraft->aiManeuverTimerTicks = PAIMAN_DIVE_DURATION_TICKS;
}

// FUNCTION: XW 0x417270
int16_t paiman_zoommaneuver_2(void) {
	if (g_deathStarSurfaceModeActive != 0 &&
		g_objectTable[g_paiObjectIndex].worldZ < PAIMAN_SURFACE_ZOOM_ALTITUDE) {
		paiman_initzoommaneuver();
		g_curCraft->aiManeuverId = PAIORDER_MANEUVER_ZOOM;
	}
	return g_curCraft->aiManeuverTimerTicks == 0;
}

// FUNCTION: XW 0x4172C0
void paiman_initsplitsdivemaneuver(void) {
	g_curCraft->aiRollState = XW_AI_ROLL_STATE_1;
	g_curCraft->aiRollStepQ16 = XW_AI_COMMAND_FULL;
	g_curCraft->aiTargetRoll = PAIMAN_HALF_TURN;
	g_curCraft->aiYawState = XW_AI_YAW_INACTIVE;
	g_curCraft->aiPitchForce = 1;
	g_curCraft->aiTargetPitch = (math2_getrandom() & (PAIMAN_QUARTER_TURN - 1)) + PAIMAN_QUARTER_TURN;
	g_curCraft->aiPitchState = XW_AI_PITCH_INCREMENT;
	g_curCraft->aiPitchStepQ16 = XW_AI_COMMAND_FULL;
}

// FUNCTION: XW 0x417330
int16_t paiman_splitsmaneuver(void) {
	if (g_curCraft->aiRollState == XW_AI_ROLL_TARGET_REACHED &&
		g_curCraft->aiPitchState == XW_AI_PITCH_TARGET_REACHED) {
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x417350
void paiman_initspeedawaymaneuver(void) {
	uint16_t objectIndex = g_paiObjectIndex;
	paiman_setpower(objectIndex, XW_AI_COMMAND_FULL);
	g_curCraft->aiManeuverTimerTicks = PAIMAN_SPEED_AWAY_DURATION_TICKS;
	g_curCraft->aiTargetYaw =
		g_objectTable[objectIndex].yaw + ((uint16_t)math2_getrandom() & PAIMAN_JINK_YAW_RANDOM_MASK);
	paiman_setjink(objectIndex);
}

// FUNCTION: XW 0x4173C0
int16_t paiman_speedawaymaneuver(void) {
	if (g_curCraft->aiManeuverAuxTimerTicks == 0) {
		g_curCraft->aiTargetYaw = -g_curCraft->aiTargetYaw;
		paiman_setjink(g_paiObjectIndex);
	}
	return g_curCraft->aiManeuverTimerTicks == 0;
}

// FUNCTION: XW 0x417400
void paiman_setjink(uint16_t ownObjectIndex) {
	int verticalJink =
		(math2_getrandom() & PAIMAN_JINK_DISPLACEMENT_RANDOM_MASK) + PAIMAN_JINK_DISPLACEMENT_BASE;
	int yawJink = (math2_getrandom() & PAIMAN_JINK_YAW_RANDOM_MASK) + PAIMAN_JINK_YAW_BASE;
	if (g_curCraft->aiDisplacementZ >= 0) {
		verticalJink = -verticalJink;
		yawJink = -yawJink;
	}
	/* The original zero-extends the displacement even after negation. */
	g_curCraft->aiDisplacementZ = (uint16_t)verticalJink;
#ifdef XW_MODERN
	XwFlightIntegration_Clear(ownObjectIndex, XW_INTEGRATE_PUSH_Z);
#endif
	g_curCraft->aiTargetYaw = g_objectTable[ownObjectIndex].yaw + yawJink;
	paiman_setturn((g_curCraft->aiSkillQ16 >> 1) + PAIMAN_TURN_STEP_HALF);
	g_curCraft->aiManeuverAuxTimerTicks = PAIMAN_JINK_INTERVAL_TICKS;
}

// FUNCTION: XW 0x417490
void paiman_initintohyperspacemaneuver(void) {
	paiman_setflighttotarget(0, 1);
	paiman_setpower(g_paiObjectIndex, XW_CRAFT_THROTTLE_FULL);
	g_curCraft->aiManeuverPhase = 0;
}

// FUNCTION: XW 0x4174C0
int16_t paiman_intohyperspacemaneuver(void) {
	switch (g_curCraft->aiManeuverPhase) {
		case 0:
			paiman_setflighttotarget(0, 1);
			if (g_trig2PolarDistance < PAIMAN_HYPERSPACE_ENTRY_DISTANCE) {
				g_curCraft->objectKind = XW_CRAFT_OBJECT_KIND_5;
				g_curCraft->aiRollState = 0;
				g_curCraft->aiPitchState = 0;
				g_curCraft->aiYawState = 0;
				g_curCraft->aiManeuverPhase = 1;
				g_curCraft->aiManeuverAuxTimerTicks = PAIMAN_HYPERSPACE_ENTRY_AUX_TICKS;
				g_curCraft->aiManeuverTimerTicks = PAIMAN_HYPERSPACE_ENTRY_DURATION_TICKS;
				paiman_setpower(g_paiObjectIndex, XW_CRAFT_THROTTLE_FULL);
			} else if (g_trig2PolarDistance < PAIMAN_HYPERSPACE_APPROACH_DISTANCE &&
					   g_objectTable[g_paiObjectIndex].genusId == XW_GENUS_STARFIGHTER) {
				paiman_setpower(g_paiObjectIndex, XW_CRAFT_THROTTLE_HALF);
			} else {
				paiman_setpower(g_paiObjectIndex, XW_CRAFT_THROTTLE_FULL);
			}
			break;
		case 1:
			g_curCraft->objectKind = XW_CRAFT_OBJECT_KIND_5;
			if (g_objectTable[g_paiObjectIndex].speed >= PAIMAN_HYPERSPACE_EXIT_SPEED) {
				if (g_objectTable[g_paiObjectIndex].iff == g_playerFlightState.object->iff) {
					msg_craftmessage(g_paiObjectIndex, g_curCraft, XW_MSG_CRAFT_ENTERED_HYPERSPACE);
					fsfx_triggersfx(FSFX_FRIENDLY_DEPARTURE_SLOT, FSFX_UNPOSITIONED_OBJECT);
					if (g_curCraft->captorFlightGroupOverride != 0) {
						++g_missionFlightGroupStates[g_curCraft->flightGroupIndex]
							  .outcomes[MISSION_OUTCOME_RECOVERED];
						if (g_missionFlightGroups[g_curCraft->flightGroupIndex].specialCraftIndex ==
							g_curCraft->craftIndexInFlightGroup)
							g_missionFlightGroupStates[g_curCraft->flightGroupIndex]
								.outcomes[MISSION_OUTCOME_SPECIAL_RECOVERED] = 1;
					}
				}
				++g_missionFlightGroupStates[g_curCraft->flightGroupIndex]
					  .outcomes[MISSION_OUTCOME_COMPLETED_FIRST];
				if (g_missionFlightGroups[g_curCraft->flightGroupIndex].specialCraftIndex ==
					g_curCraft->craftIndexInFlightGroup)
					g_missionFlightGroupStates[g_curCraft->flightGroupIndex]
						.outcomes[MISSION_OUTCOME_SPECIAL_COMPLETED] = 1;
				fediskio_updatepilotrecord(g_paiObjectIndex, 0, 0);
				g_objectTable[g_paiObjectIndex].objectType = XW_OBJ_NONE;
			}
			break;
	}
	return 0;
}

// FUNCTION: XW 0x4176C0
void paiman_initoutofhyperspacemaneuver(void) {
	g_curCraft->objectKind = XW_CRAFT_OBJECT_KIND_6;
	g_objectTable[g_paiObjectIndex].speed = PAIMAN_HYPERSPACE_EXIT_SPEED;
	g_curCraft->aiManeuverPhase = 0;
	g_curCraft->aiManeuverAuxTimerTicks = PAIMAN_TICKS_PER_SECOND;
	g_curCraft->aiTargetRef = PAI_TARGET_WAYPOINT_BASE;
	pai_settarget();
	g_curCraft->aiManeuverTimerTicks = PAIMAN_HYPERSPACE_EXIT_DURATION_TICKS;
	g_curCraft->aiCandidateTargetOrSavedInterval = g_curCraft->aiThinkIntervalTicks;
	g_curCraft->aiThinkIntervalTicks = PAIMAN_HYPERSPACE_EXIT_THINK_INTERVAL_TICKS;
}

// FUNCTION: XW 0x417730
int16_t paiman_outofhyperspacemaneuver(void) {
	uint8_t exitComplete;
	if (g_curCraft->aiManeuverAuxTimerTicks == 0) {
		++g_curCraft->aiManeuverPhase;
		if (g_curCraft->aiManeuverPhase > PAIMAN_HYPERSPACE_EXIT_LAST_PHASE)
			g_curCraft->aiManeuverPhase = PAIMAN_HYPERSPACE_EXIT_LAST_PHASE;
		g_objectTable[g_paiObjectIndex].speed = g_hyperspaceExitSpeedByPhase[g_curCraft->aiManeuverPhase];
		g_curCraft->aiManeuverAuxTimerTicks = PAIMAN_TICKS_PER_SECOND;
	}
	exitComplete = 0;
	if (g_curCraft->aiLeaderObjectIndex == XW_CRAFT_NO_AI_LEADER) {
		trig2_ctop(g_curCraft->aiAimPointX - g_objectTable[g_paiObjectIndex].worldX,
				   g_curCraft->aiAimPointY - g_objectTable[g_paiObjectIndex].worldY,
				   g_curCraft->aiAimPointZ - g_objectTable[g_paiObjectIndex].worldZ);
		if (g_trig2PolarDistance < PAIMAN_HYPERSPACE_EXIT_TARGET_DISTANCE ||
			g_curCraft->aiManeuverTimerTicks == 0)
			exitComplete = 1;
	} else if (g_paiLeaderCraft->aiCurrentPlanId != PAI_PLAN_OUT_OF_HYPERSPACE) {
		exitComplete = 1;
	}
	if (exitComplete != 0) {
		uint16_t order = g_missionFlightGroups[g_curCraft->flightGroupIndex].order;
		uint8_t nextPlanId;
		if (g_curCraft->aiLeaderObjectIndex == XW_CRAFT_NO_AI_LEADER)
			nextPlanId = g_orderLeaderPlanId[order];
		else
			nextPlanId = g_orderFollowerPlanId[order];
		g_paiOutOfHyperspacePlan.transition.nextPlanId = nextPlanId;
		g_curCraft->aiThinkIntervalTicks = g_curCraft->aiCandidateTargetOrSavedInterval;
		g_curCraft->objectKind = XW_CRAFT_OBJECT_KIND_0;
		g_curCraft->aiCandidateTargetOrSavedInterval = PAI_TARGET_NONE;
		if (nextPlanId == 0)
			g_objectTable[g_paiObjectIndex].speed = 0;
		else
			g_objectTable[g_paiObjectIndex].speed = PAIMAN_HYPERSPACE_RESUME_SPEED;
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x4178B0
int16_t paiman_escortmaneuver(void) {
	uint16_t selfObjectIndex = g_paiObjectIndex;
	uint16_t escortGroup = g_curCraft->aiEscortTargetFlightGroup;
	uint16_t escortIndex = XW_CRAFT_NO_AI_LEADER, candidateIndex;
	CraftData *escortCraft, *candidateCraft;
	uint16_t yaw, escortSpeed, selfSpeed, oldThrottle, pitch, pitchError, roll, rollError;
	uint32_t followRange, distance;
	uint16_t escortSlot;
	for (candidateIndex = 0; candidateIndex < XW_CRAFT_OBJECT_COUNT; ++candidateIndex) {
		if (g_objectTable[candidateIndex].objectType != XW_OBJ_NONE) {
			candidateCraft = (CraftData*)g_objectTable[candidateIndex].instanceData;
			if (candidateCraft->flightGroupIndex == escortGroup &&
				candidateCraft->aiLeaderObjectIndex == XW_CRAFT_NO_AI_LEADER) {
				escortIndex = candidateIndex;
				break;
			}
		}
	}
	if (escortIndex != XW_CRAFT_NO_AI_LEADER) {
		pai_distancebetween(escortIndex, selfObjectIndex);
		escortCraft = (CraftData*)g_objectTable[escortIndex].instanceData;
		distance = g_trig2PolarDistance;
		followRange = g_modelTypeTable[g_objectTable[escortIndex].objectType].maxBoundsExtent <
							  PAIMAN_LARGE_FORMATION_EXTENT
						  ? PAIMAN_ESCORT_SMALL_RANGE
						  : PAIMAN_ESCORT_LARGE_RANGE;
		if (distance <= followRange && escortCraft->workingSubsystems != 0) {
			if (escortCraft->aiYawState == XW_AI_YAW_STEER) {
				g_curCraft->aiTargetYaw = escortCraft->aiTargetYaw;
				paiman_setturn((g_curCraft->aiSkillQ16 >> 3) + PAIMAN_TURN_STEP_QUARTER);
			} else {
				yaw = g_objectTable[escortIndex].yaw;
				if (yaw != (uint16_t)g_objectTable[selfObjectIndex].yaw) {
					g_curCraft->aiTargetYaw = yaw;
					paiman_setturn((g_curCraft->aiSkillQ16 >> 3) + PAIMAN_TURN_STEP_QUARTER);
				}
			}
			selfSpeed = g_objectTable[selfObjectIndex].speed;
			escortSpeed = g_objectTable[escortIndex].speed;
			if (escortSpeed > selfSpeed) {
				oldThrottle = g_curCraft->engineThrottle[0];
				paiman_setpower(selfObjectIndex, oldThrottle + PAIMAN_FOLLOW_SPEED_GAIN * escortSpeed -
													 PAIMAN_FOLLOW_SPEED_GAIN * selfSpeed);
				if (g_curCraft->engineThrottle[0] < oldThrottle)
					paiman_setpower(selfObjectIndex, XW_CRAFT_THROTTLE_FULL);
			} else if (escortSpeed < selfSpeed) {
				oldThrottle = g_curCraft->engineThrottle[0];
				paiman_setpower(selfObjectIndex, oldThrottle + PAIMAN_FOLLOW_SPEED_GAIN * escortSpeed -
													 PAIMAN_FOLLOW_SPEED_GAIN * selfSpeed);
				if (g_curCraft->engineThrottle[0] > oldThrottle)
					paiman_setpower(selfObjectIndex, 0);
			}
			pitch = escortCraft->pitch;
			pitchError = g_curCraft->pitch - pitch;
			if (pitchError >= PAIMAN_HALF_TURN)
				pitchError = -pitchError;
			if (pitchError < PAIMAN_FOLLOW_ANGLE_SNAP) {
				g_curCraft->pitch = pitch;
				g_curCraft->aiPitchState = XW_AI_PITCH_INACTIVE;
			} else {
				g_curCraft->aiTargetPitch = pitch;
				if (g_curCraft->aiTargetPitch <= g_curCraft->pitch)
					g_curCraft->aiPitchState = XW_AI_PITCH_DECREMENT;
				else
					g_curCraft->aiPitchState = XW_AI_PITCH_INCREMENT;
				g_curCraft->aiPitchStepQ16 = XW_AI_COMMAND_FULL;
				g_curCraft->aiPitchForce = 0;
			}
			roll = g_objectTable[escortIndex].roll;
			rollError = g_objectTable[selfObjectIndex].roll - roll;
			if (rollError >= PAIMAN_HALF_TURN)
				rollError = -rollError;
			if (rollError < PAIMAN_FOLLOW_ANGLE_SNAP) {
				g_objectTable[selfObjectIndex].roll = roll;
				g_objectTable[selfObjectIndex].orientMatrixDirty = 1;
				g_curCraft->aiRollState = 0;
			} else {
				g_curCraft->aiTargetRoll = roll;
				g_curCraft->aiRollStepQ16 = XW_AI_COMMAND_FULL;
				g_curCraft->aiRollState = XW_AI_ROLL_STATE_1;
			}
			escortSlot = g_curCraft->aiOrderParameter;
			pai_calcrotatedpoint(&g_objectTable[escortIndex], g_escortSideOffsets[escortSlot],
								 g_escortUpOffsets[escortSlot], g_escortForwardOffsets[escortSlot]);
			if (g_modelTypeTable[g_objectTable[escortIndex].objectType].maxBoundsExtent >=
				PAIMAN_LARGE_FORMATION_EXTENT) {
				g_rotatedX *= PAIMAN_ESCORT_LARGE_OFFSET_SCALE;
				g_rotatedY *= PAIMAN_ESCORT_LARGE_OFFSET_SCALE;
				g_rotatedZ *= PAIMAN_ESCORT_LARGE_OFFSET_SCALE;
			}
			if (escortCraft->aiLeaderObjectIndex == g_playerFlightState.objectIndex &&
				g_playerFlightState.object->speed <= PAIMAN_FOLLOW_MIN_FORMATION_SPEED) {
				g_curCraft->aiDisplacementX = 0;
				g_curCraft->aiDisplacementY = 0;
				g_curCraft->aiDisplacementZ = 0;
#ifdef XW_MODERN
				XwFlightIntegration_ClearPush(g_paiObjectIndex);
#endif
			} else {
#ifdef XW_MODERN
				g_curCraft->aiDisplacementX =
					(int32_t)((uint32_t)g_objectTable[escortIndex].worldX -
							  (uint32_t)g_objectTable[g_paiObjectIndex].worldX + (uint32_t)g_rotatedX);
#else
				g_curCraft->aiDisplacementX =
					g_objectTable[escortIndex].worldX - g_objectTable[g_paiObjectIndex].worldX + g_rotatedX;
#endif
#ifdef XW_MODERN
				g_curCraft->aiDisplacementY =
					(int32_t)((uint32_t)g_objectTable[escortIndex].worldY -
							  (uint32_t)g_objectTable[g_paiObjectIndex].worldY + (uint32_t)g_rotatedY);
#else
				g_curCraft->aiDisplacementY =
					g_objectTable[escortIndex].worldY - g_objectTable[g_paiObjectIndex].worldY + g_rotatedY;
#endif
#ifdef XW_MODERN
				g_curCraft->aiDisplacementZ =
					(int32_t)((uint32_t)g_objectTable[escortIndex].worldZ -
							  (uint32_t)g_objectTable[g_paiObjectIndex].worldZ + (uint32_t)g_rotatedZ);
#else
				g_curCraft->aiDisplacementZ =
					g_objectTable[escortIndex].worldZ - g_objectTable[g_paiObjectIndex].worldZ + g_rotatedZ;
#endif
			}
		} else {
			g_curCraft->aiAimPointX = g_objectTable[escortIndex].worldX;
			g_curCraft->aiAimPointY = g_objectTable[escortIndex].worldY;
			g_curCraft->aiAimPointZ = g_objectTable[escortIndex].worldZ;
			if (escortCraft->workingSubsystems == 0)
				paiman_setflighttotarget(PAIMAN_QUARTER_TURN, 1);
			else
				paiman_setflighttotarget(0, 1);
			if (g_trig2PolarDistance > PAIMAN_FOLLOW_CATCHUP_DISTANCE)
				paiman_setpower(selfObjectIndex, XW_CRAFT_THROTTLE_FULL);
			else
				paiman_setpower(selfObjectIndex, PAIMAN_ESCORT_APPROACH_THROTTLE);
			g_curCraft->aiDisplacementX = 0;
			g_curCraft->aiDisplacementY = 0;
			g_curCraft->aiDisplacementZ = 0;
#ifdef XW_MODERN
			XwFlightIntegration_ClearPush(g_paiObjectIndex);
#endif
		}
	} else {
		paiman_setflighttotarget(0, 1);
		paiman_setpower(g_paiObjectIndex, XW_CRAFT_THROTTLE_HALF);
	}
	return 0;
}

// FUNCTION: XW 0x417D60
void paiman_initboardmaneuver(void) { g_curCraft->aiManeuverPhase = PAIMAN_BOARD_APPROACH; }

// FUNCTION: XW 0x417D70
int16_t paiman_boardmaneuver(void) {
	CraftData* craft = g_curCraft;
	uint16_t targetRef = craft->aiTargetRef;
	int targetIndex = targetRef;
	CraftData* targetCraft = (CraftData*)g_objectTable[targetIndex].instanceData;
	uint16_t targetType = targetCraft->craftTypeIndex;
	switch (craft->aiManeuverPhase) {
		case PAIMAN_BOARD_APPROACH: {
			int craftType;
			int16_t forward = g_craftTypeDefs[targetType].docking.targetForward;
			int16_t up;
			if ((g_objectTable[targetIndex].genusId == XW_GENUS_STARFIGHTER ||
				 g_objectTable[targetIndex].genusId == XW_GENUS_TRANSPORT)) {
				craftType = craft->craftTypeIndex;
				up = (int16_t)(g_craftTypeDefs[targetType].docking.targetSmallUp -
							   g_craftTypeDefs[craftType].docking.selfSmallUp);
			} else if ((g_objectTable[g_paiObjectIndex].genusId == XW_GENUS_STARFIGHTER ||
						g_objectTable[g_paiObjectIndex].genusId == XW_GENUS_TRANSPORT)) {
				craftType = craft->craftTypeIndex;
				up = (int16_t)(g_craftTypeDefs[targetType].docking.targetSmallUp +
							   g_craftTypeDefs[targetType].docking.targetLargeUp -
							   g_craftTypeDefs[craftType].docking.selfLargeUp);
			} else {
				craftType = craft->craftTypeIndex;
				up = (int16_t)(2 * g_craftTypeDefs[targetType].docking.targetLargeUp -
							   g_craftTypeDefs[craftType].docking.selfLargeUp);
			}
			up = (int16_t)(up + g_craftTypeDefs[targetType].docking.targetLargeUp -
						   g_craftTypeDefs[craftType].docking.selfLargeUp);
			if (up < 0)
				up = PAIMAN_BOARD_FALLBACK_UP;
			pai_calcrotatedpoint(&g_objectTable[targetIndex], 0, up, forward);
			g_curCraft->aiAimPointX = (int)((uint32_t)g_objectTable[targetIndex].worldX + g_rotatedX);
			g_curCraft->aiAimPointY = (int)((uint32_t)g_objectTable[targetIndex].worldY + g_rotatedY);
			g_curCraft->aiAimPointZ = (int)((uint32_t)g_objectTable[targetIndex].worldZ + g_rotatedZ);
			paiman_setflighttotarget(0, 1);
			if (g_trig2PolarDistance > PAIMAN_BOARD_SLOW_RANGE)
				paiman_setpower(g_paiObjectIndex, XW_CRAFT_THROTTLE_FULL);
			else if (g_trig2PolarDistance > PAIMAN_BOARD_ALIGN_RANGE)
				paiman_setpower(g_paiObjectIndex, PAIMAN_BOARD_SLOW_THROTTLE);
			else {
				paiman_setpower(g_paiObjectIndex, 0);
				g_curCraft->aiManeuverPhase = PAIMAN_BOARD_ALIGN;
			}
			return 0;
		}
		case PAIMAN_BOARD_ALIGN: {
			int craftType;
			int16_t forward = g_craftTypeDefs[targetType].docking.targetForward;
			int16_t up;
			int displacementX, displacementY, displacementZ;
			uint16_t pitch;
			if ((g_objectTable[targetIndex].genusId == XW_GENUS_STARFIGHTER ||
				 g_objectTable[targetIndex].genusId == XW_GENUS_TRANSPORT)) {
				craftType = craft->craftTypeIndex;
				up = (int16_t)(g_craftTypeDefs[targetType].docking.targetSmallUp -
							   g_craftTypeDefs[craftType].docking.selfSmallUp);
			} else if ((g_objectTable[g_paiObjectIndex].genusId == XW_GENUS_STARFIGHTER ||
						g_objectTable[g_paiObjectIndex].genusId == XW_GENUS_TRANSPORT)) {
				craftType = craft->craftTypeIndex;
				up = (int16_t)(g_craftTypeDefs[targetType].docking.targetSmallUp -
							   g_craftTypeDefs[craftType].docking.selfLargeUp);
			} else {
				craftType = craft->craftTypeIndex;
				up = (int16_t)(g_craftTypeDefs[targetType].docking.targetLargeUp -
							   g_craftTypeDefs[craftType].docking.selfLargeUp);
			}
			pai_calcrotatedpoint(&g_objectTable[targetIndex], 0, up, forward);
			g_curCraft->aiDisplacementX = (int)((uint32_t)g_objectTable[targetIndex].worldX -
												g_objectTable[g_paiObjectIndex].worldX + g_rotatedX);
			displacementX = g_curCraft->aiDisplacementX;
			g_curCraft->aiDisplacementY = (int)((uint32_t)g_objectTable[targetIndex].worldY -
												g_objectTable[g_paiObjectIndex].worldY + g_rotatedY);
			displacementY = g_curCraft->aiDisplacementY;
			g_curCraft->aiDisplacementZ = (int)((uint32_t)g_objectTable[targetIndex].worldZ -
												g_objectTable[g_paiObjectIndex].worldZ + g_rotatedZ);
			displacementZ = g_curCraft->aiDisplacementZ;
			if (g_objectTable[g_paiObjectIndex].roll != g_objectTable[targetIndex].roll) {
				g_curCraft->aiRollState = XW_AI_ROLL_STATE_1;
				g_curCraft->aiRollStepQ16 = PAIMAN_BOARD_ANGLE_STEP;
				g_curCraft->aiTargetRoll = g_objectTable[targetIndex].roll;
			}
			if (g_objectTable[g_paiObjectIndex].yaw != g_objectTable[targetIndex].yaw) {
				g_curCraft->aiYawState = XW_AI_YAW_STEER;
				g_curCraft->aiYawStepQ16 = PAIMAN_BOARD_ANGLE_STEP;
				g_curCraft->aiTargetYaw = g_objectTable[targetIndex].yaw;
			}
			pitch = (uint16_t)g_objectTable[targetIndex].pitch;
			if ((uint16_t)g_objectTable[g_paiObjectIndex].pitch != pitch) {
				g_curCraft->aiTargetPitch = pitch;
				g_curCraft->aiPitchStepQ16 = PAIMAN_BOARD_ANGLE_STEP;
				g_curCraft->aiPitchForce = 0;
				if (g_curCraft->aiTargetPitch <= g_curCraft->pitch)
					g_curCraft->aiPitchState = XW_AI_PITCH_DECREMENT;
				else
					g_curCraft->aiPitchState = XW_AI_PITCH_INCREMENT;
			}
			if (displacementX < 0)
				displacementX = (int)(0u - (uint32_t)displacementX);
			if (displacementY < 0)
				displacementY = (int)(0u - (uint32_t)displacementY);
			if (displacementZ < 0)
				displacementZ = (int)(0u - (uint32_t)displacementZ);
			if ((int)((uint32_t)displacementX + displacementY + displacementZ) >=
				PAIMAN_BOARD_POSITION_TOLERANCE)
				return 0;
			g_curCraft->aiDisplacementX = 0;
			g_curCraft->aiDisplacementY = 0;
			g_curCraft->aiDisplacementZ = 0;
#ifdef XW_MODERN
			XwFlightIntegration_ClearPush(g_paiObjectIndex);
#endif
			g_curCraft->aiManeuverPhase = PAIMAN_BOARD_TRANSFER;
			g_curCraft->aiOrderProgress = g_curCraft->aiOrderParameter;
			if (g_curCraft->aiOrderProgress != 0)
				g_curCraft->aiManeuverTimerTicks = PAIMAN_BOARD_DURATION_UNIT_TICKS;
			else
				g_curCraft->aiManeuverTimerTicks = 0;
			msg_craftmessage(g_paiObjectIndex, g_curCraft, XW_MSG_CRAFT_DOCKED);
			if (g_objectTable[g_paiObjectIndex].iff == g_playerFlightState.object->iff)
				fsfx_triggersfx(PAIMAN_BOARD_FRIENDLY_SOUND, XW_OBJECT_SLOT_UNAVAILABLE);
			else
				fsfx_triggersfx(PAIMAN_BOARD_ENEMY_SOUND, XW_OBJECT_SLOT_UNAVAILABLE);
			return 0;
		}
		case PAIMAN_BOARD_TRANSFER: {
			int flightGroupIndex;
			if (craft->aiManeuverTimerTicks != 0)
				return 0;
			if (craft->aiOrderProgress != 0) {
				--craft->aiOrderProgress;
				g_curCraft->aiManeuverTimerTicks = PAIMAN_BOARD_DURATION_UNIT_TICKS;
				craft = g_curCraft;
			}
			if (craft->aiOrderProgress != 0)
				return 0;
			switch (craft->aiOrderPlanId) {
				case PAIMAN_BOARD_UNLOAD: {
					unsigned int byteIndex;
					for (byteIndex = 0; byteIndex < sizeof(targetCraft->cargoName); ++byteIndex) {
						targetCraft->cargoName[byteIndex] = g_curCraft->cargoName[byteIndex];
					}
					g_curCraft->cargoName[0] = 0;
					g_curCraft->boardingState = PAIMAN_BOARD_CARGO_REMOVED;
					targetCraft->boardingState = XW_CRAFT_BOARDING_COMPLETE;
					msg_craftmessage(g_paiObjectIndex, g_curCraft, XW_MSG_CRAFT_DOCKING_OPERATION_COMPLETE);
					craft = g_curCraft;
					break;
				}
				case PAIMAN_BOARD_LOAD: {
					unsigned int byteIndex;
					for (byteIndex = 0; byteIndex < sizeof(targetCraft->cargoName); ++byteIndex) {
						g_curCraft->cargoName[byteIndex] = targetCraft->cargoName[byteIndex];
					}
					targetCraft->cargoName[0] = 0;
					g_curCraft->boardingState = XW_CRAFT_BOARDING_COMPLETE;
					targetCraft->boardingState = PAIMAN_BOARD_CARGO_REMOVED;
					msg_craftmessage(g_paiObjectIndex, g_curCraft, XW_MSG_CRAFT_DOCKING_OPERATION_COMPLETE);
					craft = g_curCraft;
					break;
				}
				case PAIMAN_BOARD_SWAP: {
					unsigned int byteIndex;
					for (byteIndex = 0; byteIndex < sizeof(targetCraft->cargoName); ++byteIndex) {
						char cargoByte = g_curCraft->cargoName[byteIndex];
						g_curCraft->cargoName[byteIndex] = targetCraft->cargoName[byteIndex];
						targetCraft->cargoName[byteIndex] = cargoByte;
					}
					g_curCraft->boardingState = XW_CRAFT_BOARDING_COMPLETE;
					targetCraft->boardingState = XW_CRAFT_BOARDING_COMPLETE;
					msg_craftmessage(g_paiObjectIndex, g_curCraft, XW_MSG_CRAFT_DOCKING_OPERATION_COMPLETE);
					craft = g_curCraft;
					break;
				}
				case PAIMAN_BOARD_CAPTURE: {
					CraftData* savedCraft;
					uint16_t savedObjectIndex;
					targetCraft->captorFlightGroupOverride =
						craft->flightGroupIndex | PAIMAN_BOARD_CAPTURE_FLAG;
					g_objectTable[targetIndex].iff = g_objectTable[g_paiObjectIndex].iff;
					targetCraft->aiCurrentPlanId = PAIMAN_BOARD_CAPTURED_PLAN;
					targetCraft->workingSubsystems = XW_CRAFT_SUBSYSTEM_ALL;
					savedCraft = g_curCraft;
					savedObjectIndex = g_paiObjectIndex;
					g_curCraft = targetCraft;
					pai_initplan(targetRef);
					g_paiObjectIndex = savedObjectIndex;
					g_curCraft = savedCraft;
					msg_craftmessage(targetRef, targetCraft, XW_MSG_CRAFT_CAPTURED);
					craft = g_curCraft;
					break;
				}
				default:
					break;
			}
			if (g_objectTable[g_paiObjectIndex].iff == g_playerFlightState.object->iff) {
				if (targetCraft->isInspected == 0) {
					targetCraft->isInspected = 1;
					++g_missionFlightGroupStates[targetCraft->flightGroupIndex].inspectedCount;
					flightGroupIndex = targetCraft->flightGroupIndex;
					if (g_missionFlightGroups[flightGroupIndex].specialCraftIndex ==
						targetCraft->craftIndexInFlightGroup)
						g_missionFlightGroupStates[flightGroupIndex].specialCraftInspected = 1;
					craft = g_curCraft;
				}
				if (g_fsfxLoaded != 0) {
					if (craft->aiOrderPlanId == PAIMAN_BOARD_CAPTURE)
						fsfx_triggervoicesfx(PAIMAN_BOARD_CAPTURE_VOICE);
					else
						fsfx_triggervoicesfx(PAIMAN_BOARD_COMPLETE_VOICE);
				} else
					fsfx_triggersfx(PAIMAN_BOARD_FRIENDLY_SOUND, XW_OBJECT_SLOT_UNAVAILABLE);
			} else
				fsfx_triggersfx(PAIMAN_BOARD_ENEMY_SOUND, XW_OBJECT_SLOT_UNAVAILABLE);
			++g_missionFlightGroupStates[targetCraft->flightGroupIndex].outcomes[MISSION_OUTCOME_BOARDED];
			flightGroupIndex = targetCraft->flightGroupIndex;
			if (g_missionFlightGroups[flightGroupIndex].specialCraftIndex ==
				targetCraft->craftIndexInFlightGroup)
				g_missionFlightGroupStates[flightGroupIndex].outcomes[MISSION_OUTCOME_SPECIAL_BOARDED] = 1;
			g_curCraft->aiManeuverPhase = PAIMAN_BOARD_DEPART;
			g_curCraft->aiManeuverTimerTicks = PAIMAN_BOARD_DEPARTURE_TICKS;
			if (targetRef == g_playerFlightState.currentTargetObjectIdx)
				g_hudCachedTargetObjectIdx = PANEL_TARGET_CACHE_INVALIDATED;
			return 0;
		}
		case PAIMAN_BOARD_DEPART:
			if (craft->aiManeuverTimerTicks == 0)
				return 1;
			pai_calcrotatedpoint(&g_objectTable[g_paiObjectIndex], 0, PAIMAN_BOARD_DEPARTURE_UP, 0);
			g_curCraft->aiDisplacementX = (int)((uint32_t)g_objectTable[targetIndex].worldX -
												g_objectTable[g_paiObjectIndex].worldX + g_rotatedX);
			g_curCraft->aiDisplacementY = (int)((uint32_t)g_objectTable[targetIndex].worldY -
												g_objectTable[g_paiObjectIndex].worldY + g_rotatedY);
			g_curCraft->aiDisplacementZ = (int)((uint32_t)g_objectTable[targetIndex].worldZ -
												g_objectTable[g_paiObjectIndex].worldZ + g_rotatedZ);
			return 0;
		default:
			return 0;
	}
}

// FUNCTION: XW 0x418600
void paiman_initawaitboardmaneuver(void) {
	g_curCraft->aiRollState = 0;
	g_curCraft->aiPitchState = 0;
	g_curCraft->aiYawState = XW_AI_YAW_INACTIVE;
	paiman_setpower(g_paiObjectIndex, 0);
}

// FUNCTION: XW 0x418630
int16_t paiman_awaitboardmaneuver(void) {
	g_curCraft->aiRollState = 0;
	g_curCraft->aiPitchState = 0;
	g_curCraft->aiYawState = XW_AI_YAW_INACTIVE;
	paiman_setpower(g_paiObjectIndex, 0);
	return 0;
}

// FUNCTION: XW 0x418670
void paiman_initheadtowardmaneuver(void) { paiman_setflighttotarget(0, 1); }

// FUNCTION: XW 0x418680
int16_t paiman_headtowardmaneuver(void) {
	paiman_setflighttotarget(0, 1);
	return 0;
}

// FUNCTION: XW 0x418690
void paiman_initturnawaymaneuver(void) {
	paiman_setnewturnaway(g_paiObjectIndex);
	g_curCraft->aiManeuverTimerTicks = PAIMAN_TURN_AWAY_DURATION_TICKS;
}

// FUNCTION: XW 0x4186B0
int16_t paiman_turnawaymaneuver(void) {
	if (g_curCraft->aiManeuverTimerTicks == 0) {
		return 1;
	}
	if (g_curCraft->aiManeuverAuxTimerTicks == 0) {
		paiman_setnewturnaway(g_paiObjectIndex);
	}
	return 0;
}

/* Remaining matching differences are register allocation in the turn-step and delay calculations. */
// FUNCTION: XW 0x4186E0
void paiman_setnewturnaway(uint16_t ownObjectIndex) {
	uint16_t targetYaw;
	if (g_curCraft->lastAttackerObjIdx != PAI_TARGET_NONE) {
		targetYaw = g_objectTable[g_curCraft->lastAttackerObjIdx].yaw;
	} else {
		targetYaw = g_objectTable[ownObjectIndex].yaw + PAIMAN_HALF_TURN;
	}
	g_curCraft->aiTargetYaw = targetYaw;
	paiman_setturn((g_curCraft->aiSkillQ16 >> 1) + PAIMAN_TURN_STEP_HALF);
	g_curCraft->aiManeuverAuxTimerTicks =
		PAIMAN_TICKS_PER_SECOND * g_aiTurnAwayStateDelayBySkill[g_paiSkillTier];
}

// FUNCTION: XW 0x418760
void paiman_initoutofhangarmaneuver(void) { g_curCraft->aiManeuverTimerTicks = PAIMAN_OUT_OF_HANGAR_TICKS; }

// FUNCTION: XW 0x418770
int16_t paiman_outofhangarmaneuver(void) {
	if (g_curCraft->aiManeuverTimerTicks == 0) {
		uint16_t order = g_missionFlightGroups[g_curCraft->flightGroupIndex].order;
		uint8_t nextPlanId;
		if (g_curCraft->aiLeaderObjectIndex == XW_CRAFT_NO_AI_LEADER) {
			nextPlanId = g_orderLeaderPlanId[order];
		} else {
			nextPlanId = g_orderFollowerPlanId[order];
		}
		g_curCraft->aiFormationSpacing = 0;
		g_paiOutOfHangarPlan.transition.nextPlanId = nextPlanId;
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x4187D0
void paiman_initavoidstarshipmaneuver(void) {
	g_curCraft->aiManeuverAuxTimerTicks =
		PAIMAN_TICKS_PER_SECOND *
		((math2_getrandom() & PAIMAN_AVOID_STARSHIP_RANDOM_MASK) + PAIMAN_AVOID_STARSHIP_BASE_SECONDS);
	paiman_setturn((g_curCraft->aiSkillQ16 >> 1) + PAIMAN_TURN_STEP_HALF);
	g_curCraft->aiPitchStepQ16 = XW_AI_COMMAND_FULL;
	g_curCraft->aiPitchForce = 0;
	if (g_curCraft->aiTargetPitch <= g_curCraft->pitch) {
		g_curCraft->aiPitchState = XW_AI_PITCH_DECREMENT;
	} else {
		g_curCraft->aiPitchState = XW_AI_PITCH_INCREMENT;
	}
}

// FUNCTION: XW 0x418840
void paiman_setflighttotarget(int16_t yawBias, int16_t driveHeading) {
	uint16_t objectIndex = g_paiObjectIndex;
	trig2_ctop(
		(int)((unsigned int)g_curCraft->aiAimPointX - (unsigned int)g_objectTable[objectIndex].worldX),
		(int)((unsigned int)g_curCraft->aiAimPointY - (unsigned int)g_objectTable[objectIndex].worldY),
		(int)((unsigned int)g_curCraft->aiAimPointZ - (unsigned int)g_objectTable[objectIndex].worldZ));
	g_curCraft->aiTargetYaw = g_trig2Yaw + yawBias;
	paiman_setturn((g_curCraft->aiSkillQ16 >> 1) + PAIMAN_TURN_STEP_QUARTER);
	if (driveHeading != 0) {
		g_curCraft->aiTargetPitch = g_trig2Pitch;
		g_curCraft->aiPitchStepQ16 = XW_AI_COMMAND_FULL;
		g_curCraft->aiPitchForce = 0;
		if (g_curCraft->aiTargetPitch <= g_curCraft->pitch) {
			g_curCraft->aiPitchState = XW_AI_PITCH_DECREMENT;
		} else {
			g_curCraft->aiPitchState = XW_AI_PITCH_INCREMENT;
		}
		g_curCraft->aiClimbState = 0;
		g_curCraft->aiDiveState = 0;
	}
}

// FUNCTION: XW 0x418930
void paiman_controlplane(void) {
	g_curCraft->aiDiveState = 0;
	g_curCraft->aiClimbState = 0;
	g_curCraft->aiTargetPitch = PAIMAN_QUARTER_TURN;
	g_curCraft->aiPitchStepQ16 = XW_AI_COMMAND_FULL;
	g_curCraft->aiPitchForce = 0;
	if (g_curCraft->pitch < PAIMAN_QUARTER_TURN) {
		g_curCraft->aiPitchState = XW_AI_PITCH_INCREMENT;
	} else if (g_curCraft->pitch > PAIMAN_QUARTER_TURN) {
		g_curCraft->aiPitchState = XW_AI_PITCH_DECREMENT;
	} else {
		g_curCraft->aiPitchState = XW_AI_PITCH_TARGET_REACHED;
	}
	g_curCraft->aiRollState = XW_AI_ROLL_STATE_1;
	g_curCraft->aiRollStepQ16 = XW_AI_COMMAND_FULL;
	g_curCraft->aiTargetRoll = 0;
	g_curCraft->aiYawState = XW_AI_YAW_INACTIVE;
}

// FUNCTION: XW 0x4189B0
void paiman_attacktarget(int16_t yawBias) {
	uint16_t objectIndex = g_paiObjectIndex;
	if (g_curCraft->aiManeuverId == PAIORDER_MANEUVER_ATTACK_ALTERNATE)
		pai_settarget();
	else
		paiman_calcplanelead(g_curCraft->aiTargetRef);
#ifdef XW_MODERN
	trig2_ctop((int32_t)((uint32_t)g_curCraft->aiAimPointX - (uint32_t)g_objectTable[objectIndex].worldX),
			   (int32_t)((uint32_t)g_curCraft->aiAimPointY - (uint32_t)g_objectTable[objectIndex].worldY),
			   (int32_t)((uint32_t)g_curCraft->aiAimPointZ - (uint32_t)g_objectTable[objectIndex].worldZ));
#else
	trig2_ctop(g_curCraft->aiAimPointX - g_objectTable[objectIndex].worldX,
			   g_curCraft->aiAimPointY - g_objectTable[objectIndex].worldY,
			   g_curCraft->aiAimPointZ - g_objectTable[objectIndex].worldZ);
#endif
	g_curCraft->aiTargetYaw = g_trig2Yaw + yawBias;
	if (g_curCraft->aiRollState == XW_AI_ROLL_STATE_2) {
		uint16_t yawError = g_objectTable[objectIndex].yaw - g_curCraft->aiTargetYaw;
		if (yawError > PAIMAN_HALF_TURN)
			yawError = -yawError;
		if (yawError >= PAIMAN_ATTACK_TURN_YAW_THRESHOLD ||
			g_trig2PolarDistance < PAIMAN_ATTACK_TURN_RANGE_THRESHOLD)
			paiman_setturn((g_curCraft->aiSkillQ16 >> 1) + PAIMAN_TURN_STEP_HALF);
	} else {
		paiman_setturn((g_curCraft->aiSkillQ16 >> 1) + PAIMAN_TURN_STEP_HALF);
	}
	if (g_curCraft->aiRollState == XW_AI_ROLL_STATE_3)
		g_curCraft->aiRollState = 0;
	if ((uint16_t)g_trig2Pitch != g_curCraft->pitch) {
		g_curCraft->aiTargetPitch = g_trig2Pitch;
		g_curCraft->aiPitchStepQ16 = XW_AI_COMMAND_FULL;
		if (g_curCraft->aiTargetPitch <= g_curCraft->pitch)
			g_curCraft->aiPitchState = XW_AI_PITCH_DECREMENT;
		else
			g_curCraft->aiPitchState = XW_AI_PITCH_INCREMENT;
		paiman_setpower(objectIndex, XW_CRAFT_THROTTLE_FULL);
		g_curCraft->aiClimbState = 0;
		g_curCraft->aiDiveState = 0;
		g_curCraft->aiPitchForce = 0;
	}
}

// FUNCTION: XW 0x418B10
void paiman_calcplanelead(uint16_t targetObjectRef) {
	uint16_t attackerObjectIndex = g_paiObjectIndex;
	if (targetObjectRef < XW_MISSION_OBJECT_REF_BASE) {
		uint16_t leadSteps;
		int leadX;
		int leadY;
		int leadZ;
		if (g_objectTable[targetObjectRef].speed == 0) {
			leadSteps = 0;
		} else {
			uint16_t orderPlanId;
			uint16_t baseProjectileSpeed;
			uint16_t yawDifference;
			uint16_t targetSpeed;
			uint16_t closingSpeed;
			uint16_t travelPerStep;
			pai_distancebetween(attackerObjectIndex, g_curCraft->aiTargetRef);
			orderPlanId = g_curCraft->aiOrderPlanId;
			if (orderPlanId >= PAI_PLAN_DISABLE_FIRST && orderPlanId <= PAI_PLAN_DISABLE_LAST)
				baseProjectileSpeed = PAIMAN_LEAD_DISABLE_PROJECTILE_SPEED;
			else
				baseProjectileSpeed = PAIMAN_LEAD_PROJECTILE_SPEED;
			yawDifference = g_objectTable[targetObjectRef].yaw - g_objectTable[attackerObjectIndex].yaw;
			closingSpeed = g_objectTable[attackerObjectIndex].speed + baseProjectileSpeed;
			targetSpeed = g_objectTable[targetObjectRef].speed;
			if (targetSpeed >= baseProjectileSpeed)
				targetSpeed = baseProjectileSpeed;
			if (yawDifference >= PAIMAN_HALF_TURN)
				yawDifference = -yawDifference;
			if (yawDifference < PAIMAN_QUARTER_TURN)
				closingSpeed -= trig2_cosinewordmult(targetSpeed, yawDifference);
			else
				closingSpeed += trig2_cosinewordmult(targetSpeed, yawDifference);
			travelPerStep =
				closingSpeed / PAIMAN_LEAD_TRAVEL_DIVISOR + PAIMAN_LEAD_TRAVEL_MULTIPLIER * closingSpeed;
			if (travelPerStep == 0)
				travelPerStep = PAIMAN_LEAD_MINIMUM_TRAVEL;
			leadSteps = g_trig2PolarDistance / travelPerStep;
			leadSteps = math2_fraction((unsigned int)g_simStepScale * leadSteps, g_curCraft->aiSkillQ16);
		}
#ifdef XW_MODERN
		leadX = (int32_t)(leadSteps * (uint32_t)XwReferenceMotion_Axis(targetObjectRef, 0));
		leadY = (int32_t)(leadSteps * (uint32_t)XwReferenceMotion_Axis(targetObjectRef, 1));
		leadZ = (int32_t)(leadSteps * (uint32_t)XwReferenceMotion_Axis(targetObjectRef, 2));
		g_curCraft->aiAimPointX =
			(int32_t)((uint32_t)g_objectTable[targetObjectRef].worldX + (uint32_t)leadX);
		g_curCraft->aiAimPointY =
			(int32_t)((uint32_t)g_objectTable[targetObjectRef].worldY + (uint32_t)leadY);
		g_curCraft->aiAimPointZ =
			(int32_t)((uint32_t)g_objectTable[targetObjectRef].worldZ + (uint32_t)leadZ);
#else
		leadY =
			leadSteps * (g_objectTable[targetObjectRef].worldY - g_objectTable[targetObjectRef].prevWorldY);
		leadZ =
			leadSteps * (g_objectTable[targetObjectRef].worldZ - g_objectTable[targetObjectRef].prevWorldZ);
		leadX =
			leadSteps * (g_objectTable[targetObjectRef].worldX - g_objectTable[targetObjectRef].prevWorldX);
		g_curCraft->aiAimPointX = g_objectTable[targetObjectRef].worldX + leadX;
		g_curCraft->aiAimPointY = g_objectTable[targetObjectRef].worldY + leadY;
		g_curCraft->aiAimPointZ = g_objectTable[targetObjectRef].worldZ + leadZ;
#endif
	} else {
		int missionObjectIndex = targetObjectRef - XW_MISSION_OBJECT_REF_BASE;
		g_curCraft->aiAimPointX =
			g_missionObjects[missionObjectIndex].worldX * MISSION_OBJECT_WORLD_COORDINATE_SCALE;
		g_curCraft->aiAimPointY =
			g_missionObjects[missionObjectIndex].worldY * MISSION_OBJECT_WORLD_COORDINATE_SCALE;
		g_curCraft->aiAimPointZ =
			g_missionObjects[missionObjectIndex].worldZ * MISSION_OBJECT_WORLD_COORDINATE_SCALE;
	}
}

// FUNCTION: XW 0x418CF0
void paiman_calcformation(void) {
	uint8_t spacing = g_paiLeaderCraft->aiFormationSpacing;
	uint8_t slot = g_curCraft->craftIndexInFlightGroup;
	uint16_t formation = g_curCraft->aiFormationType;
	int formationSlotIndex = slot + PAIMAN_FORMATION_SLOTS * formation;
	int16_t horizontalSpacing = g_formationHorizontalSpacing[spacing];
	int16_t verticalSpacing = g_formationVerticalSpacing[spacing];
	pai_calcrotatedpoint(
		&g_objectTable[g_paiLeaderObjectIndex], horizontalSpacing * g_formPosX[formationSlotIndex],
		verticalSpacing * g_formPosZ[formationSlotIndex], horizontalSpacing * g_formPosY[formationSlotIndex]);
	if (g_modelTypeTable[g_objectTable[g_paiObjectIndex].objectType].maxBoundsExtent >=
		PAIMAN_LARGE_FORMATION_EXTENT) {
		g_rotatedX *= PAIMAN_LARGE_FORMATION_SCALE;
		g_rotatedY *= PAIMAN_LARGE_FORMATION_SCALE;
		g_rotatedZ *= PAIMAN_LARGE_FORMATION_SCALE;
	}
#ifdef XW_MODERN
	g_curCraft->aiDisplacementX =
		(int32_t)((uint32_t)g_objectTable[g_paiLeaderObjectIndex].worldX -
				  (uint32_t)g_objectTable[g_paiObjectIndex].worldX + (uint32_t)g_rotatedX);
#else
	g_curCraft->aiDisplacementX =
		g_objectTable[g_paiLeaderObjectIndex].worldX - g_objectTable[g_paiObjectIndex].worldX + g_rotatedX;
#endif
#ifdef XW_MODERN
	g_curCraft->aiDisplacementY =
		(int32_t)((uint32_t)g_objectTable[g_paiLeaderObjectIndex].worldY -
				  (uint32_t)g_objectTable[g_paiObjectIndex].worldY + (uint32_t)g_rotatedY);
#else
	g_curCraft->aiDisplacementY =
		g_objectTable[g_paiLeaderObjectIndex].worldY - g_objectTable[g_paiObjectIndex].worldY + g_rotatedY;
#endif
#ifdef XW_MODERN
	g_curCraft->aiDisplacementZ =
		(int32_t)((uint32_t)g_objectTable[g_paiLeaderObjectIndex].worldZ -
				  (uint32_t)g_objectTable[g_paiObjectIndex].worldZ + (uint32_t)g_rotatedZ);
#else
	g_curCraft->aiDisplacementZ =
		g_objectTable[g_paiLeaderObjectIndex].worldZ - g_objectTable[g_paiObjectIndex].worldZ + g_rotatedZ;
#endif
}

// FUNCTION: XW 0x418E80
void paiman_setturn(uint16_t turnStep) {
	uint16_t yawError = g_objectTable[g_paiObjectIndex].yaw - g_curCraft->aiTargetYaw;
	if (yawError >= PAIMAN_HALF_TURN) {
		yawError = -yawError;
	}
	if (yawError <= PAIMAN_YAW_SNAP_THRESHOLD) {
		g_objectTable[g_paiObjectIndex].yaw = g_curCraft->aiTargetYaw;
		g_objectTable[g_paiObjectIndex].orientMatrixDirty = 1;
		g_objectTable[g_paiObjectIndex].moveVectorDirty = 1;
		g_curCraft->aiYawState = XW_AI_YAW_TARGET_REACHED;
#ifdef XW_MODERN
		XwFlightIntegration_Clear(g_paiObjectIndex, XW_INTEGRATE_YAW);
		XwFlightIntegration_Clear(g_paiObjectIndex, XW_INTEGRATE_BANK);
#endif
	} else {
		g_curCraft->aiYawState = XW_AI_YAW_STEER;
		g_curCraft->aiYawStepQ16 = turnStep;
	}
}

// FUNCTION: XW 0x418EF0
void paiman_setpower(uint16_t objIdx, uint16_t throttle) {
	uint16_t engineIndex;
	(void)objIdx;
	for (engineIndex = 0; engineIndex < g_craftTypeDefs[g_curCraft->craftTypeIndex].engineCount;
		 ++engineIndex) {
		g_curCraft->engineThrottle[engineIndex] = throttle;
	}
}
