#ifndef XW_FLIGHT_XW_H
#define XW_FLIGHT_XW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

struct CraftData;
struct ObjectRecord;
struct RenderObjectListEntry;
typedef struct XwFlightGlobalCountdownTimers XwFlightGlobalCountdownTimers;
typedef struct XwFlightRandomBytePair XwFlightRandomBytePair;
typedef struct XwMissionClock XwMissionClock;

typedef int32_t FlightEntryMode;

enum {
	XW_TARGET_BLINK_BIT = 0x400,
	XW_TARGET_BLINK_DISTANCE_SHIFT = 5,
	XW_TARGET_BLINK_SHORT_TICKS = 14,
	XW_TARGET_BLINK_LONG_TICKS = 118,
	XW_HOURS_PER_DAY = 24,
	XW_CLOCK_BYTE_UNDERFLOW = 0xFF,
	XW_TIMED_MISSION_MODE = 1,
	XW_COUNTDOWN_SILENT_MODE_3 = 3,
	XW_COUNTDOWN_SILENT_MODE_5 = 5,
	XW_MISSION_TIMEOUT_EXIT = 3,
	XW_COURSE_TIMEOUT_EXIT = 1,
	XW_MISSION_MINUTE_WARNING_SOUND = 38,
	XW_TIMEOUT_HYPERSPACE_SOUND = 29,
	XW_COUNTDOWN_WARNING_SECONDS = 15,
	XW_COUNTDOWN_WARNING_SOUND = 23,
	XW_SUBSYSTEM_FULL_HEALTH = 100,
	XW_SUBSYSTEM_REPAIRED_MESSAGE_ARG = 26,
	XW_SUBSYSTEM_REPAIRED_SOUND = 37
};

enum {
	XW_POINT_LIGHT_RADIUS_MARGIN = 0x4000,
	XW_LIGHT_CLAMP_LIMIT = 0x40000000,
	XW_LIGHT_CLAMP_MAX = 0x3FFFFFFF,
	XW_LIGHT_CLAMP_MIN = -0x3FFF0000,
	XW_LIGHT_DEFAULT_INTENSITY = 16,
	XW_LIGHT_INTENSITY_SCALE = 8,
	XW_LIGHT_BRIGHTNESS_BASE = 256,
	XW_LIGHT_INTENSITY_32 = 32,
	XW_LIGHT_INTENSITY_48 = 48,
	XW_LIGHT_INTENSITY_64 = 64,
	XW_LIGHT_INTENSITY_96 = 96,
	XW_LIGHT_INTENSITY_192 = 192,
	XW_LIGHT_INTENSITY_320 = 320,
	XW_LIGHT_INTENSITY_480 = 480
};

enum {
	XW_LIGHT_ANIMATION_FRAME_2 = 2,
	XW_LIGHT_ANIMATION_FRAME_3 = 3,
	XW_LIGHT_ANIMATION_FRAME_4 = 4,
	XW_LIGHT_ANIMATION_FRAME_5 = 5,
	XW_LIGHT_ANIMATION_FRAME_6 = 6,
	XW_LIGHT_ANIMATION_FRAME_7 = 7,
	XW_LIGHT_ANIMATION_FRAME_8 = 8,
	XW_LIGHT_ANIMATION_FRAME_9 = 9,
	XW_LIGHT_ANIMATION_FRAME_10 = 10,
	XW_LIGHT_ANIMATION_FRAME_11 = 11
};

enum {
	XW_HUD_SHIP_ID_COUNT = 112,
	XW_RANDOM_PAIR_COUNT = 256,
	XW_RANDOM_PAIR_LARGE_MASK = 63,
	XW_RANDOM_PAIR_SMALL_MASK = 7,
	XW_MUSIC_START_CHOICE_COUNT = 4,
	XW_BYTES_PER_KB = 1024,
	XW_REPLAY_SURFACE_EXIT_SECOND = 70,
	XW_REPLAY_GLOW_BYTE_FACTOR = 128,
	XW_EXIT_REQUEST_ABORT = 2,
	XW_HULL_QUARTER_SHIFT = 14,
	XW_REPLAY_PROMPT_WIDTH_DIVISOR = 10,
	XW_REPLAY_PROMPT_HEIGHT_DIVISOR = 8,
	XW_REPLAY_PROMPT_LINES = 4,
	XW_REPLAY_PROMPT_HALF_LINES = 2,
	XW_REPLAY_PROMPT_BACKGROUND = 0x40,
	XW_REPLAY_PROMPT_BORDER = 0x43,
	XW_REPLAY_PROMPT_TEXT = 0x49,
	XW_REPLAY_PROMPT_SHADOW = 0x12
};

extern const uint8_t g_hudShipIdByCraftType[XW_HUD_SHIP_ID_COUNT];
extern uint8_t g_dynamicMusicInitialStartMinuteChoices[XW_MUSIC_START_CHOICE_COUNT];
extern uint8_t g_dynamicMusicInitialStartSecondChoices[XW_MUSIC_START_CHOICE_COUNT];
extern uint8_t g_flightField62B930;
extern uint8_t g_flightField62C924;
extern uint8_t g_flightField62D138;
extern uint8_t g_flightField637324;

extern int g_localLightsLevel;
extern uint16_t g_legacyOscillatorFrameCounter;
extern int16_t g_legacyOscillatorDirection;
extern uint16_t g_previousFrameMeasuredTicks;
extern int g_objectPointLightCount;

enum { XW_RENDER_OBJECT_LIST_CAPACITY = 180 };

enum {
	XW_EXTERNAL_VIEW_LAG = 5,
	XW_CAMERA_MATRIX_SHIFT = 15,
	XW_SURFACE_VIEW_FORWARD_LIMIT = 0x6000,
	XW_BACKDROP_OBJECT_REF = 0x3000,
	XW_SPINNING_MODEL_FIRST = 100,
	XW_SPINNING_MODEL_LAST = 105,
	XW_SPIN_ROLL_GROUP_SHIFT = 4,
	XW_SPIN_PITCH_GROUP_SHIFT = 3,
	XW_SPIN_ROLL_DIVISOR = 16,
	XW_SPIN_PITCH_DIVISOR = 32,
	XW_SPIN_YAW_BASE = 4,
	XW_MISSION_ANGLE_SHIFT = 8,
	XW_HYPERSPACE_EFFECT_RADIUS = 0xFFFF,
	XW_MULTI_MESH_DEBRIS_MODEL = 83
};

enum { XW_OBJECT_DEPTH_SHIFT = 8 };

enum { XW_SIMULATION_TICKS_PER_SECOND = 236, XW_SECONDS_PER_MINUTE = 60 };

enum {
	XW_FRAME_MINIMUM_TICKS = 8,
	XW_FRAME_RATE_FONT_TIER = 2,
	XW_FRAME_RATE_BACKGROUND = 0x40,
	XW_FRAME_RATE_FOREGROUND = 0x4E,
	XW_FRAME_RATE_RIGHT_MARGIN = 25,
	XW_FRAME_RATE_BOTTOM_LINES = 2,
	XW_FRAME_RATE_DIGITS = 2,
	XW_OSCILLATOR_FRAME_LIMIT = 8,
	XW_OSCILLATOR_MAXIMUM = 4
};

enum {
	XW_MILLISECONDS_PER_SECOND = 1000,
	XW_MUSIC_BASE_TRACK = 2,
	XW_MUSIC_SUCCESS_TRACK = 3,
	XW_MUSIC_FAILURE_TRACK = 7
};

enum FlightEntryModeValues {
	FLIGHT_ENTRY_NEW_MISSION = 0x0,
	FLIGHT_ENTRY_RESUME_SAVED = 0x1,
	FLIGHT_ENTRY_REPLAY_VIEWER = 0x2
};

enum {
	XW_TIMER_READY_MESSAGE = 0,
	XW_TIMER_FIELD_02 = 1,
	XW_TIMER_MISSION_GOAL = 2,
	XW_TIMER_ANIMATION = 3,
	XW_TIMER_SHIELD_FLASH = 4,
	XW_TIMER_HULL_FLASH = 5,
	XW_TIMER_WEAPON_POWER = 6,
	XW_TIMER_ARRIVAL_TRIGGER = 7,
	XW_TIMER_ARRIVAL_DELAY = 8,
	XW_TIMER_INCOMING_WARHEAD = 9,
	XW_GLOBAL_COUNTDOWN_TIMER_COUNT = 10
};

/* Original IDB size: 20 bytes. Each word is decremented with signed-underflow clamping. */
struct XwFlightGlobalCountdownTimers {
	uint16_t ticks[XW_GLOBAL_COUNTDOWN_TIMER_COUNT];
};

/* Original IDB size: 2 bytes. */
struct XwFlightRandomBytePair {
	/* IDB +0x0: Initialized with random value masked to 0..63. Consumer/purpose unresolved. */
	uint8_t value0To63;
	/* IDB +0x1: Initialized with random value masked to 0..7. Consumer/purpose unresolved. */
	uint8_t value0To7;
};

extern XwFlightRandomBytePair g_flightRandomBytePairs[XW_RANDOM_PAIR_COUNT];

/* Original IDB size: 8 bytes. */
struct XwMissionClock {
	/* IDB +0x0: Serialized leading bytes; no direct X-Wing semantic use identified. */
	uint8_t gap_0[3];
	/* IDB +0x3: Elapsed clock: hours, wraps at 24. Countdown instance: initialized to zero only; no hour
	 * countdown is implemented. */
	uint8_t hours;
	/* IDB +0x4: Elapsed clock wraps at 60; countdown holds remaining mission minutes. */
	uint8_t minutes;
	/* IDB +0x5: Elapsed clock wraps at 60; countdown borrows from minutes at underflow. */
	uint8_t seconds;
	/* IDB +0x6: Elapsed clock: signed countdown to next second, replenished by 236 in XW_updatetime. No
	 * direct use identified in countdown instance. */
	int16_t subsecondTicks;
};

extern int g_renderObjectListCount;
extern uint32_t g_dynamicMusicLastUpdateTick;
extern int16_t g_targetHighlightBlinkTicks;
extern struct RenderObjectListEntry* g_renderListHead;
extern uint16_t g_flightAccumulatedTicks;
extern uint16_t g_renderSphereRadius;
extern int g_objectViewX;
extern int g_objectViewY;
extern int g_objectViewZ;
extern uint16_t g_targetHighlightObjectAndBlinkBits;
extern int g_dynamicMusicResumeMinute;
extern uint8_t g_showSimStepScale;
extern int g_dynamicMusicResumeOffsetSeconds;
extern uint8_t g_flightField62BAD0;
extern uint16_t g_simStepScale;
extern int g_flightResolutionScale;
extern uint32_t g_dynamicMusicCurrentTick;
extern XwMissionClock g_missionCountdownClock;
extern int g_dynamicMusicResumeSecond;
extern int g_dynamicMusicSavedRemainingMs;
extern struct XwFlightGlobalCountdownTimers g_flightGlobalCountdownTimers;
extern int g_dynamicMusicTrackRemainingMs;
extern struct RenderObjectListEntry* g_renderObjectListEntries;
extern uint16_t g_elapsedTicks;
extern uint32_t g_dynamicMusicElapsedMs;
extern XwMissionClock g_missionElapsedClock;
extern uint16_t g_textureCacheFlushPending;
extern uint8_t g_legacyOscillatorValue;
extern uint8_t calcframerate;
extern int g_camRelWorldZ;
extern int g_camRelWorldX;
extern int g_camRelWorldY;
extern uint8_t g_dynamicMusicState;
extern uint8_t g_snapshotField62D118;
extern uint8_t g_dynamicMusicOutcomeLatched;
extern struct CraftData* g_curCraft;
extern int g_dynamicMusicBaseTrackDurationMs;

/* Declarations follow ascending original IDB address. */

/* 0x42E590 */
void Xw_InitFlightResolution(void);

/* 0x42E6C0 */
void Xw_simulator(FlightEntryMode entryMode);

/* 0x42F3E0 */
void Xw_doframe(void);

/* 0x42F6A0 */
void Xw_QueueRenderObject(int objectIdx, int sortDepth);

/* 0x42F710 */
void Xw_ResetRenderList(void);

/* 0x42F720 */
void Xw_SortRenderListDepthAscending(void);

/* 0x42F810 */
void Xw_updatescreen(void);

/* 0x430380 */
void Xw_getobjecteyexyz(uint16_t objectIndex);

/* 0x4304D0 */
void Xw_GetMissionObjectEyeXYZ(uint16_t missionObjectIndex);

/* 0x430580 */
int16_t Xw_checkobjecteyexyz(uint16_t objectIndex, uint16_t sphereRadius);

/* 0x430670 */
int16_t Xw_checkstaticobjecteyexyz(int16_t worldX, int16_t worldY, int16_t worldZ, uint16_t sphereRadius);

/* 0x430760 */
void Xw_updatetime(void);

/* 0x430C20 */
void Xw_UpdateDynamicMusicState(void);

/* 0x430DF0 */
int Xw_MakeLocalLights(const struct ObjectRecord* object);

#ifdef __cplusplus
}
#endif

#endif
