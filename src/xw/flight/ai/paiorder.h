#ifndef XW_FLIGHT_AI_PAIORDER_H
#define XW_FLIGHT_AI_PAIORDER_H

#include "xw/flight/ai/pai.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

enum {
	PAIORDER_HOME_FORMATION_SPACING = 1,
	PAIORDER_HOME_GROUP_OVERRIDE_MASK = 0x7F,
	PAIORDER_HOME_GROUP_NONE = 0xFFFF,
	PAIORDER_HOME_HANGAR_SCALE = 4,
	PAIORDER_HOME_HALF_THROTTLE_RANGE = 0x10000,
	PAIORDER_HOME_QUARTER_THROTTLE_RANGE = 0x4000,
	PAIORDER_HOME_QUARTER_THROTTLE = 0x4000,
	PAIORDER_HOME_ARRIVAL_RANGE = 2048
};

enum {
	PAIORDER_HANGAR_FORMATION_SPACING = 9,
	PAIORDER_HANGAR_SLOW_RANGE = 2048,
	PAIORDER_HANGAR_SLOW_THROTTLE = 0x2000,
	PAIORDER_HANGAR_APPROACH_THROTTLE = 0x5555,
	PAIORDER_HANGAR_ARRIVAL_RANGE = 512
};

enum { PAIORDER_LARGE_TARGET_WARHEAD = 149 };

enum { PAIORDER_BOARD_ABORT_PHASE_LIMIT = 3 };

enum { PAIORDER_AIM_DISTANCE_THRESHOLD = 131072 };

enum { PAIORDER_MANEUVER_COUNT = 29 };

enum { PAIORDER_RETURN_BOARD_DISTANCE = 4096, PAIORDER_RETURN_BOARD_ALT_DISTANCE = 16384 };

enum {
	PAIORDER_MANEUVER_TURN_INSIDE = 1,
	PAIORDER_MANEUVER_SCISSORS = 4,
	PAIORDER_MANEUVER_HEAD_ON_ATTACK = 9,
	PAIORDER_MANEUVER_FOLLOW_LEADER = 10,
	PAIORDER_MANEUVER_SETUP_ATTACK = 11,
	PAIORDER_MANEUVER_ATTACK = 12,
	PAIORDER_MANEUVER_ZOOM = 13,
	PAIORDER_MANEUVER_ZOOM_ALTERNATE = 14,
	PAIORDER_MANEUVER_SPLIT_S_ALTERNATE = 15,
	PAIORDER_MANEUVER_SPEED_AWAY = 16,
	PAIORDER_MANEUVER_INTO_HYPERSPACE = 21,
	PAIORDER_MANEUVER_ATTACK_ALTERNATE = 23,
	PAIORDER_MANEUVER_TURN_AWAY = 24,
	PAIORDER_MANEUVER_AVOID_STARSHIP = 28
};

enum {
	PAIORDER_STARSHIP_PREDICTION_SECONDS = 6,
	PAIORDER_STARSHIP_AVOID_YAW = 0x3800,
	PAIORDER_STARSHIP_AVOID_PITCH = 0x3000
};

enum {
	PAIORDER_THREAT_OCTANT_COUNT = 8,
	PAIORDER_THREAT_OCTANT_SHIFT = 13,
	PAIORDER_THREAT_FRONT = 0,
	PAIORDER_THREAT_SIDE = 1,
	PAIORDER_THREAT_REAR = 2,
	PAIORDER_NO_LAST_ATTACKER = 255,
	PAIORDER_TAIL_TURN_THRESHOLD = 24000,
	PAIORDER_TAIL_ZOOM_THRESHOLD = 40000,
	PAIORDER_TAIL_ALT_ZOOM_IFF = 1,
	PAIORDER_TAIL_ALT_ZOOM_MIN_ALTITUDE = 0x8000
};

enum {
	PAIORDER_AVOID_TARGET_DISTANCE = 0x6000,
	PAIORDER_TRACKED_WARHEAD_RANGE_MULTIPLIER = 3,
	PAIORDER_ATTACKER_ANGLE_LIMIT = 0x2000,
	PAIORDER_DISPLACEMENT_BASE = 256
};

enum {
	PAIORDER_FRONT_MANEUVER_COUNT = 4,
	PAIORDER_SIDE_REAR_MANEUVER_COUNT = 8,
	PAIORDER_NONCRAFT_ATTACKER_SPEED = 900,
	PAIORDER_UNDER_ATTACK_CLOSE_RANGE = 0x2000,
	PAIORDER_UNDER_ATTACK_ESCAPE_RANGE = 0x8000,
	PAIORDER_UNDER_ATTACK_TURN_RANDOM_LIMIT = 0x4000,
	PAIORDER_UNDER_ATTACK_MIN_ALTITUDE = 0x4000
};

extern uint8_t g_aiUnderAttackFrontManeuverChoices[PAIORDER_FRONT_MANEUVER_COUNT];
extern uint8_t g_aiUnderAttackSideRearManeuverChoices[PAIORDER_SIDE_REAR_MANEUVER_COUNT];

extern int g_aiAttackerSearchRangeBySkill[PAI_PROFICIENCY_COUNT];
extern int g_aiWarheadThreatRangeBySkill[PAI_PROFICIENCY_COUNT];

enum { PAIORDER_DAMAGE_THRESHOLD_Q16 = 0xE000 };

enum { PAIORDER_ABORT_ATTACK_DAMAGE_Q16 = 0xC000 };

enum { PAIORDER_WAIT_RUN_BASE_RANGE = 0x20000 };

enum { PAIORDER_LEADER_DELAY_TICKS = 59 };

enum {
	PAIORDER_NO_ORDER = 0,
	PAIORDER_AWAIT_REPAIR = 50,
	PAIORDER_BOARD_FIRST = 40,
	PAIORDER_BOARD_DISABLED_43 = 43,
	PAIORDER_BOARD_DISABLED_44 = 44,
	PAIORDER_BOARD_LAST = 44,
	PAIORDER_MANEUVER_AWAIT_BOARD = 19,
	PAIORDER_MANEUVER_AWAIT_BOARD_ALTERNATE = 25
};

typedef int16_t (*XwManeuverFunction)(void);

extern XwManeuverFunction g_orderTable[PAI_ORDER_PREDICATE_COUNT];

extern int g_aiStillAttackLastAttackerRangeBySkill[PAI_PROFICIENCY_COUNT];
extern uint8_t g_aiThreatBearingClassByOctant[PAIORDER_THREAT_OCTANT_COUNT];
extern XwManeuverFunction g_maneuverFunctions[PAIORDER_MANEUVER_COUNT];
extern XwManeuverFunction g_currentManeuverFunction;

/* Declarations follow ascending original IDB address. */

/* 0x412AB0 */
int16_t paiorder_updatecourseorder(void);

/* 0x412AD0 */
int16_t paiorder_underattackorder(void);

/* 0x412E20 */
int16_t paiorder_stillattackorder(void);

/* 0x412EB0 */
int16_t paiorder_flyhomeorder(void);

/* 0x413080 */
int16_t paiorder_enterhangarorder(void);

/* 0x4133E0 */
int16_t paiorder_CheckDamageOrTargetSentinel(void);

/* 0x4135B0 */
int16_t paiorder_waitrunorder(void);

/* 0x4135F0 */
int16_t paiorder_breakofforder(void);

/* 0x413660 */
int16_t paiorder_abortatkorder(void);

/* 0x4136D0 */
int16_t paiorder_leaderdeadorder(void);

/* 0x4137E0 */
int16_t paiorder_ontailorder(void);

/* 0x4138B0 */
int16_t paiorder_IsObjectKind4(void);

/* 0x4138C0 */
int16_t paiorder_leadergohomeorder(void);

/* 0x4138D0 */
int16_t paiorder_hyperspaceorder(void);

/* 0x413950 */
int16_t paiorder_mothershiporder(void);

/* 0x413980 */
int16_t paiorder_lookforcrafttoboardorder(void);

/* 0x413AA0 */
int16_t paiorder_abortboardorder(void);

/* 0x413B30 */
int16_t paiorder_returnboardorder(void);

/* 0x413B50 */
int16_t paiorder_awaitboardorder(void);

/* 0x413BF0 */
int16_t paiorder_makedisabledorder(void);

/* 0x413C00 */
int16_t paiorder_returnboardorder_2(void);

/* 0x413C70 */
int16_t paiorder_rocketsonboardorder(void);

/* 0x413D30 */
int16_t paiorder_avoidhitorder(void);

/* 0x414030 */
int16_t paiorder_waitforallreturnorder(void);

/* 0x4140E0 */
int16_t paiorder_waitforallcreateorder(void);

/* 0x414160 */
int16_t paiorder_TargetGroupsBoardingAndArrivalComplete(void);

/* 0x4141C0 */
int16_t paiorder_FlightGroupBoardingAndArrivalComplete(uint16_t flightGroupIndex);

/* 0x414270 */
int16_t paiorder_TargetGroupsAreExhausted(void);

/* 0x4142D0 */
int16_t paiorder_FlightGroupHasRemainingCraft(uint16_t flightGroupIndex, int16_t requireWorkingSubsystems);

/* 0x414370 */
int16_t paiorder_evasiveorder(void);

/* 0x4143B0 */
int16_t paiorder_targetfromplayerorder(void);

/* 0x414410 */
int16_t paiorder_avoidstarshiporder(void);

/* 0x414530 */
int16_t paiorder_IsFinalWaypointTarget(void);

/* 0x414550 */
int16_t paiorder_checkhyperorder(void);

/* 0x414580 */
int16_t paiorder_TargetGroupsHaveArrived(void);

/* 0x414B80 */
int16_t paiorder_IsAimDistanceAbove131072(void);

#ifdef __cplusplus
}
#endif

#endif
