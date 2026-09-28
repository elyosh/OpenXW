#ifndef XW_FLIGHT_AI_PAIMAN_H
#define XW_FLIGHT_AI_PAIMAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/flight/ai/pai.h"
#include "xw/flight/ai/paiorder.h"

#include <stddef.h>
#include <stdint.h>

enum {
	PAIMAN_ESCORT_SLOT_COUNT = 10,
	PAIMAN_ESCORT_SMALL_RANGE = 0x8000,
	PAIMAN_ESCORT_LARGE_RANGE = 0x20000,
	PAIMAN_ESCORT_LARGE_OFFSET_SCALE = 16,
	PAIMAN_ESCORT_APPROACH_THROTTLE = 0x4000
};

extern const int16_t g_escortSideOffsets[PAIMAN_ESCORT_SLOT_COUNT];
extern const int16_t g_escortUpOffsets[PAIMAN_ESCORT_SLOT_COUNT];
extern const int16_t g_escortForwardOffsets[PAIMAN_ESCORT_SLOT_COUNT];

enum {
	PAIMAN_FORMATION_SLOTS = 6,
	PAIMAN_FORMATION_POSITION_COUNT = 60,
	PAIMAN_FORMATION_SPACING_COUNT = 12,
	PAIMAN_LARGE_FORMATION_EXTENT = 3000,
	PAIMAN_LARGE_FORMATION_SCALE = 4
};

extern int16_t g_formPosX[PAIMAN_FORMATION_POSITION_COUNT];
extern int16_t g_formPosY[PAIMAN_FORMATION_POSITION_COUNT];
extern int16_t g_formPosZ[PAIMAN_FORMATION_POSITION_COUNT];
extern int16_t g_formationHorizontalSpacing[PAIMAN_FORMATION_SPACING_COUNT];
extern int16_t g_formationVerticalSpacing[PAIMAN_FORMATION_SPACING_COUNT];

enum {
	PAIMAN_ATTACK_BREAK_LARGE_RANGE = 0x2000,
	PAIMAN_ATTACK_BREAK_LARGE_END_RANGE = 0x4000,
	PAIMAN_ATTACK_LARGE_SIDE_ANGLE_MIN = 0x2800,
	PAIMAN_ATTACK_LARGE_SIDE_ANGLE_MAX = 0x5800,
	PAIMAN_ATTACK_BREAK_SMALL_RANGE = 5120,
	PAIMAN_ATTACK_BREAK_OPPOSING_RANGE = 10240,
	PAIMAN_ATTACK_BREAK_YAW_MASK = 0x3FFF,
	PAIMAN_ATTACK_BREAK_YAW_BASE = 0x3000,
	PAIMAN_ATTACK_SURFACE_PITCH_BASE = 0x3C00,
	PAIMAN_ATTACK_SURFACE_PITCH_MASK = 0xFFF,
	PAIMAN_ATTACK_PITCH_MASK = 0x7FFF,
	PAIMAN_ATTACK_BREAK_SECONDS_BASE = 2,
	PAIMAN_ATTACK_BREAK_SECONDS_MASK = 3,
	PAIMAN_ATTACK_ALTERNATE_THROTTLE = 0xC000
};

enum {
	PAIMAN_FOLLOW_CATCHUP_DISTANCE = 0x10000,
	PAIMAN_FOLLOW_SPEED_GAIN = 50,
	PAIMAN_FOLLOW_ANGLE_SNAP = 0x400,
	PAIMAN_FOLLOW_MIN_FORMATION_SPEED = 10
};

enum { PAIMAN_YAW_SNAP_THRESHOLD = 0x300 };

enum { PAIMAN_ATTACK_TURN_YAW_THRESHOLD = 0x2000, PAIMAN_ATTACK_TURN_RANGE_THRESHOLD = 0x10000 };

enum {
	PAIMAN_SETUP_ATTACK_HALF_THROTTLE_YAW_MIN = 0x3000,
	PAIMAN_SETUP_ATTACK_HALF_THROTTLE_YAW_MAX = 0xD000
};

enum {
	PAIMAN_LEAD_DISABLE_PROJECTILE_SPEED = 350,
	PAIMAN_LEAD_PROJECTILE_SPEED = 900,
	PAIMAN_LEAD_TRAVEL_MULTIPLIER = 18,
	PAIMAN_LEAD_TRAVEL_DIVISOR = 5,
	PAIMAN_LEAD_MINIMUM_TRAVEL = 19
};

enum { PAIMAN_THROTTLE_PRESET_COUNT = 10 };

enum { PAIMAN_BOARD_APPROACH = 0, PAIMAN_OUT_OF_HANGAR_TICKS = 2360 };

enum {
	PAIMAN_BOARD_ALIGN = 1,
	PAIMAN_BOARD_TRANSFER = 2,
	PAIMAN_BOARD_DEPART = 3,
	PAIMAN_BOARD_FALLBACK_UP = 0x7000,
	PAIMAN_BOARD_SLOW_RANGE = 0x10000,
	PAIMAN_BOARD_ALIGN_RANGE = 2048,
	PAIMAN_BOARD_SLOW_THROTTLE = 0x4000,
	PAIMAN_BOARD_ANGLE_STEP = 0x8000,
	PAIMAN_BOARD_POSITION_TOLERANCE = 16,
	PAIMAN_BOARD_DURATION_UNIT_TICKS = 14160,
	PAIMAN_BOARD_DEPARTURE_TICKS = 2360,
	PAIMAN_BOARD_DEPARTURE_UP = 0x4000,
	PAIMAN_BOARD_UNLOAD = 40,
	PAIMAN_BOARD_LOAD = 41,
	PAIMAN_BOARD_SWAP = 42,
	PAIMAN_BOARD_CAPTURE = 43,
	PAIMAN_BOARD_CARGO_REMOVED = 1,
	PAIMAN_BOARD_CAPTURE_FLAG = 0x80,
	PAIMAN_BOARD_CAPTURED_PLAN = 53,
	PAIMAN_BOARD_FRIENDLY_SOUND = 0x25,
	PAIMAN_BOARD_ENEMY_SOUND = 0x26,
	PAIMAN_BOARD_CAPTURE_VOICE = 0x47,
	PAIMAN_BOARD_COMPLETE_VOICE = 0x4B
};

enum {
	PAIMAN_IMMELMANN_INITIAL_PITCH = 0,
	PAIMAN_IMMELMANN_SECOND_PITCH = 1,
	PAIMAN_IMMELMANN_ROLL_UPRIGHT = 2
};

enum { PAIMAN_QUARTER_TURN = 0x4000, PAIMAN_HALF_TURN = 0x8000, PAIMAN_SCISSORS_INTERVAL_TICKS = 944 };

enum { PAIMAN_TICKS_PER_SECOND = 236, PAIMAN_TURN_STEP_HALF = 0x8000 };

enum {
	PAIMAN_CRUISE_STARSHIP_WAYPOINT_DISTANCE = 0x2000,
	PAIMAN_CRUISE_WAYPOINT_DISTANCE = 0x4000,
	PAIMAN_CRUISE_REFRESH_TICKS = 5 * PAIMAN_TICKS_PER_SECOND,
	PAIMAN_CRUISE_VERTICAL_TOLERANCE = 512,
	PAIMAN_CRUISE_CLIMB_THROTTLE = 0xC000,
	PAIMAN_CRUISE_STOP_YAW_ERROR = 0x1000,
	PAIMAN_CRUISE_VERTICAL_ACTIVE = 1
};

enum { PAIMAN_TURN_STEP_QUARTER = 0x4000 };

enum { PAIMAN_HEAD_TOWARD_FULL_DURATION_TICKS = 5 * PAIMAN_TICKS_PER_SECOND };

enum { PAIMAN_HEAD_ON_ATTACK_DURATION_TICKS = 8 * PAIMAN_TICKS_PER_SECOND };

enum {
	PAIMAN_HYPERSPACE_EXIT_SPEED = 3600,
	PAIMAN_HYPERSPACE_EXIT_SPEED_COUNT = 11,
	PAIMAN_HYPERSPACE_EXIT_LAST_PHASE = 8,
	PAIMAN_HYPERSPACE_EXIT_TARGET_DISTANCE = 0x4000,
	PAIMAN_HYPERSPACE_RESUME_SPEED = 250,
	PAIMAN_HYPERSPACE_EXIT_DURATION_TICKS = 11 * PAIMAN_TICKS_PER_SECOND,
	PAIMAN_HYPERSPACE_EXIT_THINK_INTERVAL_TICKS = PAIMAN_TICKS_PER_SECOND / 4
};

enum {
	PAIMAN_HYPERSPACE_ENTRY_DISTANCE = 0x4000,
	PAIMAN_HYPERSPACE_APPROACH_DISTANCE = 0x10000,
	PAIMAN_HYPERSPACE_ENTRY_AUX_TICKS = 4 * PAIMAN_TICKS_PER_SECOND,
	PAIMAN_HYPERSPACE_ENTRY_DURATION_TICKS = 7 * PAIMAN_TICKS_PER_SECOND
};

enum { PAIMAN_TURN_INSIDE_DURATION_TICKS = 3540 };

enum { PAIMAN_TURN_AWAY_DURATION_TICKS = 3540 };

enum { PAIMAN_SPEED_AWAY_DURATION_TICKS = 20 * PAIMAN_TICKS_PER_SECOND };

enum {
	PAIMAN_SCISSORS_DURATION_TICKS = 20 * PAIMAN_TICKS_PER_SECOND,
	PAIMAN_SCISSORS_INITIAL_INTERVAL_TICKS = 2 * PAIMAN_TICKS_PER_SECOND
};

enum {
	PAIMAN_DIVE_PITCH_BASE = 0x5800,
	PAIMAN_DIVE_PITCH_RANDOM_MASK = 0xFFF,
	PAIMAN_DIVE_DURATION_TICKS = 1180
};

enum {
	PAIMAN_ZOOM_PITCH_BASE = 0x2000,
	PAIMAN_ZOOM_PITCH_RANDOM_MASK = 0xFFF,
	PAIMAN_ZOOM_BASE_SECONDS = 3,
	PAIMAN_ZOOM_SECONDS_RANDOM_MASK = 3,
	PAIMAN_SURFACE_ZOOM_ALTITUDE = 0x4000
};

enum { PAIMAN_AVOID_STARSHIP_BASE_SECONDS = 20, PAIMAN_AVOID_STARSHIP_RANDOM_MASK = 7 };

enum {
	PAIMAN_JINK_DISPLACEMENT_BASE = 50,
	PAIMAN_JINK_DISPLACEMENT_RANDOM_MASK = 0x1F,
	PAIMAN_JINK_YAW_BASE = 384,
	PAIMAN_JINK_YAW_RANDOM_MASK = 0xFF,
	PAIMAN_JINK_INTERVAL_TICKS = PAIMAN_TICKS_PER_SECOND / 2
};

typedef void (*XwManeuverInitFunction)(void);

extern uint16_t g_aiTurnAwayStateDelayBySkill[PAI_PROFICIENCY_COUNT];
extern const uint16_t g_aiThrottleByPreset[PAIMAN_THROTTLE_PRESET_COUNT];
extern XwManeuverInitFunction g_maneuverInitFunctions[PAIORDER_MANEUVER_COUNT];
extern const uint16_t g_hyperspaceExitSpeedByPhase[PAIMAN_HYPERSPACE_EXIT_SPEED_COUNT];
extern XwManeuverInitFunction g_currentManeuverInitFunction;

/* Declarations follow ascending original IDB address. */

/* 0x4161E0 */
void paiman_initmaneuver(void);

/* 0x416230 */
void paiman_initturninsidemaneuver(void);

/* 0x416250 */
int16_t paiman_turninsidemaneuver(void);

/* 0x416280 */
void paiman_setnewturninside(uint16_t ownObjectIndex);

/* 0x4162F0 */
void paiman_initsplitsmaneuver(void);

/* 0x416350 */
void paiman_initimmelmannmaneuver(void);

/* 0x4163F0 */
int16_t paiman_immelmannmaneuver(void);

/* 0x416490 */
void paiman_initscissorsmaneuver(void);

/* 0x416520 */
int16_t paiman_scissorsmaneuver(void);

/* 0x416570 */
void paiman_initrendezvousmaneuver(void);

/* 0x4165B0 */
int16_t paiman_rendezvousmaneuver(void);

/* 0x4165F0 */
void paiman_initcruisemaneuver(void);

/* 0x416650 */
int16_t paiman_cruisemaneuver(void);

/* 0x416820 */
void paiman_gonextwaypoint(uint16_t unusedObjectIndex);

/* 0x4168D0 */
void paiman_initheadtowardfullmaneuver(void);

/* 0x416900 */
int16_t paiman_headtowardfullmaneuver(void);

/* 0x416950 */
void paiman_initrunawaymaneuver(void);

/* 0x416980 */
int16_t paiman_runawaymaneuver(void);

/* 0x4169B0 */
void paiman_initheadonattackmaneuver(void);

/* 0x4169F0 */
int16_t paiman_headonattackmaneuver(void);

/* 0x416A30 */
int16_t paiman_followleadermaneuver(void);

/* 0x416D80 */
void paiman_initsetupattackmaneuver(void);

/* 0x416DA0 */
int16_t paiman_setupattackmaneuver(void);

/* 0x416E00 */
int16_t paiman_attackmaneuver(void);

/* 0x417110 */
void paiman_initzoommaneuver(void);

/* 0x4171E0 */
int16_t paiman_zoommaneuver(void);

/* 0x4171F0 */
void paiman_initdivemaneuver(void);

/* 0x417270 */
int16_t paiman_zoommaneuver_2(void);

/* 0x4172C0 */
void paiman_initsplitsdivemaneuver(void);

/* 0x417330 */
int16_t paiman_splitsmaneuver(void);

/* 0x417350 */
void paiman_initspeedawaymaneuver(void);

/* 0x4173C0 */
int16_t paiman_speedawaymaneuver(void);

/* 0x417400 */
void paiman_setjink(uint16_t ownObjectIndex);

/* 0x417490 */
void paiman_initintohyperspacemaneuver(void);

/* 0x4174C0 */
int16_t paiman_intohyperspacemaneuver(void);

/* 0x4176C0 */
void paiman_initoutofhyperspacemaneuver(void);

/* 0x417730 */
int16_t paiman_outofhyperspacemaneuver(void);

/* 0x4178B0 */
int16_t paiman_escortmaneuver(void);

/* 0x417D60 */
void paiman_initboardmaneuver(void);

/* 0x417D70 */
int16_t paiman_boardmaneuver(void);

/* 0x418600 */
void paiman_initawaitboardmaneuver(void);

/* 0x418630 */
int16_t paiman_awaitboardmaneuver(void);

/* 0x418670 */
void paiman_initheadtowardmaneuver(void);

/* 0x418680 */
int16_t paiman_headtowardmaneuver(void);

/* 0x418690 */
void paiman_initturnawaymaneuver(void);

/* 0x4186B0 */
int16_t paiman_turnawaymaneuver(void);

/* 0x4186E0 */
void paiman_setnewturnaway(uint16_t ownObjectIndex);

/* 0x418760 */
void paiman_initoutofhangarmaneuver(void);

/* 0x418770 */
int16_t paiman_outofhangarmaneuver(void);

/* 0x4187D0 */
void paiman_initavoidstarshipmaneuver(void);

/* 0x418840 */
void paiman_setflighttotarget(int16_t yawBias, int16_t driveHeading);

/* 0x418930 */
void paiman_controlplane(void);

/* 0x4189B0 */
void paiman_attacktarget(int16_t yawBias);

/* 0x418B10 */
void paiman_calcplanelead(uint16_t targetObjectRef);

/* 0x418CF0 */
void paiman_calcformation(void);

/* 0x418E80 */
void paiman_setturn(uint16_t turnStep);

/* 0x418EF0 */
void paiman_setpower(uint16_t objIdx, uint16_t throttle);

#ifdef __cplusplus
}
#endif

#endif
