#ifndef XW_FLIGHT_PLAYER_USER_H
#define XW_FLIGHT_PLAYER_USER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <xw/flight/hud/msg.h>
#include <xw/flight/object/craft.h>
#include <xw/render/flight_view.h>

struct CraftData;
struct ObjectRecord;
typedef struct XwPlayerFlightState XwPlayerFlightState;
typedef struct XwPlayerSmoothedTurnInput XwPlayerSmoothedTurnInput;

enum {
	USER_KEY_REBUILD_COCKPIT = 0x82,
	USER_KEY_INTERLACE = 0x88,
	USER_KEY_PAUSE = 0x8F,
	USER_KEY_VERSION = 0x95,
	USER_EXIT_REPLAY_CHECKPOINT = 13,
	USER_TARGET_HOLD_TICKS = 78,
	USER_INPUT_BUTTON_MASK = 15,
	USER_FIRE_BUTTON_MASK = 13,
	USER_BUTTON_FIRE = 1,
	USER_BUTTON_CYCLE_TARGET = 4,
	USER_BUTTON_COCKPIT = 8,
	USER_BUTTON_WEAPON = 15,
	USER_BUTTON_TRANSFER = 11,
	USER_BUTTON_SHIELD = 7,
	USER_KEY_THROTTLE_0 = 219,
	USER_KEY_THROTTLE_1 = 220,
	USER_KEY_THROTTLE_2 = 221,
	USER_KEY_THROTTLE_3 = 222,
	USER_KEY_THROTTLE_4 = 223,
	USER_KEY_THROTTLE_5 = 224,
	USER_KEY_THROTTLE_6 = 225,
	USER_KEY_THROTTLE_7 = 226,
	USER_KEY_THROTTLE_8 = 227,
	USER_KEY_THROTTLE_9 = 228,
	USER_KEY_THROTTLE_10 = 229,
	USER_KEY_THROTTLE_11 = 230,
	USER_KEY_THROTTLE_12 = 231,
};

enum {
	USER_KEY_ESCAPE = 27,
	USER_HUD_TARGET_CACHE_RESET = -2,
	USER_LAUNCHER_LINK_TOGGLE = 2,
	USER_CANNON_PAIRED_GROUP_SIZE = 4,
	USER_TARGET_MEMORY_COUNT = 4,
	USER_VIEW_DIGIT_COUNT = 10,
	USER_AI_STATUS_COUNT = 72,
	USER_KEY_DETAIL = 0x83,
	USER_KEY_EJECT = 0x84,
	USER_KEY_FRAME_RATE = 0x85,
	USER_KEY_MUSIC = 0x8C,
	USER_KEY_SOUND = 0x92,
	USER_KEY_TARGET_CROSSHAIR = 0x9B,
	USER_KEY_FIRE = 0x9C,
	USER_KEY_VIEW_ZERO = 0xB2,
	USER_KEY_VIEW_ONE = 0xB3,
	USER_KEY_VIEW_TWO = 0xB4,
	USER_KEY_VIEW_THREE = 0xB5,
	USER_KEY_VIEW_FOUR = 0xB6,
	USER_KEY_VIEW_FIVE = 0xB7,
	USER_KEY_VIEW_SIX = 0xB8,
	USER_KEY_VIEW_SEVEN = 0xB9,
	USER_KEY_VIEW_EIGHT = 0xBA,
	USER_KEY_VIEW_NINE = 0xBB,
	USER_KEY_EXTERNAL_VIEW = 0xBD,
	USER_KEY_MANUAL_CAMERA = 0xBE,
	USER_KEY_COCKPIT = 0xC2,
	USER_KEY_PLAYER_VIEW = 0xC3,
	USER_KEY_WARHEAD_VIEW = 0xC4,
	USER_KEY_EXTERNAL_VIEW_ALT = 0xC5,
	USER_KEY_MANUAL_CAMERA_ALT = 0xC6,
	USER_KEY_RECALL_TARGET_1 = 0xC7,
	USER_KEY_RECALL_TARGET_2 = 0xC8,
	USER_KEY_RECALL_TARGET_3 = 0xC9,
	USER_KEY_RECALL_TARGET_4 = 0xCA,
	USER_KEY_LASER_RECHARGE = 0xCB,
	USER_KEY_SHIELD_RECHARGE = 0xCC,
	USER_KEY_STORE_TARGET_1 = 0xD3,
	USER_KEY_STORE_TARGET_2 = 0xD4,
	USER_KEY_STORE_TARGET_3 = 0xD5,
	USER_KEY_STORE_TARGET_4 = 0xD6,
	USER_KEY_SHIELDS_TO_LASERS = 0xD7,
	USER_KEY_LASERS_TO_SHIELDS = 0xD8
};

enum {
	USER_REAR_VIEW_OFFSET = 8,
	USER_VIEW_ANGLE_SHIFT = 10,
	USER_UP_VIEW_STATE = 16,
	USER_UP_VIEW_ANGLE = 0x4000,
	USER_WARHEAD_SLOT_END = 76,
	USER_THROTTLE_STEP = 0x800,
	USER_ENGINE_POWER_BASE = 4,
	USER_ENGINE_POWER_SHIFT = 13,
	USER_ENERGY_TRANSFER_LIMIT = 100,
	USER_SHIELD_PER_LASER_CHARGE = 8,
	USER_CAMERA_MODIFIER_MASK = 0xF,
	USER_CAMERA_ZOOM_IN = 1,
	USER_CAMERA_ZOOM_OUT = 2,
	USER_CAMERA_STEP = 32,
	USER_CAMERA_MAX_STEP = 1024,
	USER_CAMERA_MIN_DISTANCE = 768,
	USER_CAMERA_MAX_DISTANCE = 5120,
	USER_ROLL_RATE_SCALE = 0x3000,
	USER_PITCH_RATE_SCALE = 0x1000,
	USER_TURN_INPUT_SHIFT = 15,
	USER_SMOOTHING_THRESHOLD = 8,
	USER_SMOOTHING_SCALE = 4,
	USER_EJECT_TUMBLE_MASK = 0x3FFF,
	USER_EJECT_MIN_TUMBLE = 0x2000,
	USER_EJECT_LIFETIME_MASK = 3,
	USER_EJECT_MIN_SECONDS = 3,
	USER_RADIO_EVASIVE_REQUEST = 251,
	USER_RADIO_IGNORE_TARGET = 0xFF,
	USER_EXIT_LOST = 1,
	USER_EXIT_RESCUED = 2,
	USER_EXIT_MAP = 10,
	USER_EXIT_DAMAGE = 11,
	USER_EXIT_BRIEFING = 12,
	USER_EXIT_OPTIONS = 13,
	USER_WARHEAD_ALERT_END_MISSION = 2
};

extern const uint8_t g_viewKeyHudStateOffsets[USER_VIEW_DIGIT_COUNT];
extern const int16_t g_viewKeyPitchAngles[USER_VIEW_DIGIT_COUNT];
extern const uint8_t g_aiPlanStatusMessageIds[USER_AI_STATUS_COUNT];

enum { USER_TURN_MODIFIER_MASK = 0xE, USER_TURN_ROLL_MODE = 2 };

enum {
	USER_PICK_MAX_ANGLE = 50,
	USER_CROSS_FAR_RANGE = 0xA0000,
	USER_CROSS_NEAR_SHIFT = 4,
	USER_CROSS_FAR_SHIFT = 8,
	USER_CROSS_BASIS_SHIFT = 15,
	USER_CROSS_MAX_DEPTH = 0x20000,
	USER_CROSS_PROJECTION_SHIFT = 8,
	USER_CROSS_PROJECTION_BIAS = 128,
	USER_CROSS_HIGH_WORD_SHIFT = 32,
	USER_CROSS_PROJECTION_OVERFLOW = 0x7FFFFF00,
	USER_CROSS_SIDE_LIMIT = 160,
	USER_CROSS_UP_LIMIT = 100,
	USER_CROSS_UP_WEIGHT = 59578,
	USER_CROSS_WEIGHT_SHIFT = 16
};

enum {
	USER_SNAPSHOT_DISTANCE_SCALE = 1609,
	USER_SNAPSHOT_DISTANCE_SHIFT = 16,
	USER_SNAPSHOT_DISTANCE_DIVISOR = 10
};

enum {
	USER_DETAIL_TABLE_COUNT = 16,
	USER_EXPLOSION_DETAIL_BIAS = 3,
	USER_EXPLOSION_DETAIL_SHIFT = 11,
	USER_HIGH_DETAIL_STAR_DENSITY = 1,
	USER_LOW_DETAIL_STAR_DENSITY = 2,
	USER_HIGH_DETAIL_HYPERSPACE_OBJECTS = 45,
	USER_LOW_DETAIL_HYPERSPACE_OBJECTS = 25,
	USER_OBJECT_DETAIL_BIAS = 8,
	USER_TRENCH_DETAIL_DIVISOR = 4,
	USER_TRENCH_DETAIL_BIAS = 4,
	USER_BRIGHTNESS_MAX = 7,
	USER_BRIGHTNESS_BIAS = 4,
	USER_BRIGHTNESS_STEP = 64
};

extern uint16_t g_targetAngleScore;
extern int g_flightBrightnessScaleQ8;
extern const int16_t g_shipDetailValueByDetail[USER_DETAIL_TABLE_COUNT];
extern const uint8_t g_starshipDetailByDetail[USER_DETAIL_TABLE_COUNT];
extern const uint8_t g_deathStarDetailLevelByDetail[USER_DETAIL_TABLE_COUNT];
extern uint8_t g_engineGlowEnabled;
extern uint8_t g_flightSfxGroupUnmuted;
extern uint8_t g_flightMusicPlaybackEnabled;

enum { XW_PLAYER_SUBSYSTEM_COUNT = 8, XW_HUD_FAILURE_RANDOM_SLOT_COUNT = 16 };

enum { USER_HUD_TARGETING_FEATURE = 1, USER_TARGET_ANNOUNCEMENT_HUD_STATE = 19 };

enum { XW_RADIO_CRAFT_SLOT_LIMIT = 28, XW_PLAYER_NO_TARGET = 0xFFFF };

enum { USER_WARHEAD_ALERT_TRACKING = 1 };

enum {
	USER_RESCUE_MAX_DISTANCE = 0x1000000,
	USER_RESCUE_FRIENDLY_DISTANCE_SHIFT = 1,
	USER_RESCUE_FRIENDLY_IFF = 0,
	USER_RESCUE_HOSTILE_IFF = 1
};

/* Original IDB size: 4 bytes. */
struct XwPlayerSmoothedTurnInput {
	/* IDB +0x0: Signed 16-bit smoothed roll input. Reset with pitch; scaled by elapsed ticks before roll
	 * update. */
	int16_t roll;
	/* IDB +0x2: Signed 16-bit smoothed pitch input. Reset with roll; consumed by USER_calcdeltapitch. */
	int16_t pitch;
};

/* Original IDB size: 180 bytes. */
struct XwPlayerFlightState {
	/* IDB +0x0: Snapshot block 9, offset +0x0. Existing field purpose checked against its X-Wing users. */
	struct ObjectRecord* object;
	/* IDB +0x4: Snapshot block 9, offset +0x4. Existing field purpose checked against its X-Wing users. */
	struct CraftData* craft;
	/* IDB +0x8: Player craft object index. Set when the mission-selected player craft is created; adjacent
	 * bytes belong to separate player state. Snapshot block 9, offset +0x8. Existing field purpose checked
	 * against its X-Wing users. */
	uint16_t objectIndex;
	/* IDB +0xA: Player flight group selected while loading the mission; 255 when no player flag was found.
	 * Snapshot block 9, offset +0xA. Existing field purpose checked against its X-Wing users. */
	uint8_t flightGroupIndex;
	/* IDB +0xB: Nonzero suppresses player HUD rendering; set by sub_42E100. Precise transition semantics not
	 * yet resolved. Snapshot block 9, offset +0xB. Existing field purpose checked against its X-Wing users.
	 */
	uint8_t hudSuppressed;
	/* IDB +0xC: Flight_MainLoop stores Q16 hull-damage fraction shifted right 14 at mission exit. No direct
	 * consumer located. Snapshot block 9, offset +0xC. Existing field purpose checked against its X-Wing
	 * users. */
	uint8_t missionExitHullDamageQuarter;
	/* IDB +0xD: Player craft definition index copied from CraftData.craftTypeIndex at spawn; indexes
	 * g_craftTypeDefs and cockpit offset tables. Not runtime species or mission craftType. Snapshot block9
	 * offset +0xD. */
	uint8_t craftTypeIndex;
	/* IDB +0xE: Player engine count cached from craft definition when binding the player craft; bounds
	 * per-engine throttle controls. Snapshot block 9, offset +0xE. Existing field purpose checked against its
	 * X-Wing users. */
	uint8_t engineCount;
	/* IDB +0xF: Select detailed target/cargo presentation versus the aiming reticle. Exhaust-port mode clears
	 * it. Snapshot block 9, offset +0xF. Existing field purpose checked against its X-Wing users. */
	uint8_t hudTargetDetailsEnabled;
	/* IDB +0x10: Current player target reference; 0xFFFF means no target. Current-target box wrapper forwards
	 * it with whole-object component sentinel and color 0x3B. Snapshot block 9, offset +0x10. Existing field
	 * purpose checked against its X-Wing users. */
	uint16_t currentTargetObjectIdx;
	/* IDB +0x12: Previous target reference retained when current target is invalidated; used as the starting
	 * point by forward/reverse target cycling. Snapshot block 9, offset +0x12. Existing field purpose checked
	 * against its X-Wing users. */
	uint16_t previousTargetObjectIdx;
	/* IDB +0x14: Four F5-F8 target memories; 0xFFFF denotes an empty slot. */
	uint16_t savedTargetRefs[USER_TARGET_MEMORY_COUNT];
	/* IDB +0x1C: Warhead lock state: 0=none, 1=acquiring, 2=locked. Weapon update sets locked at 1180
	 * accumulated lock ticks. Snapshot block 9, offset +0x1C. Existing field purpose checked against its
	 * X-Wing users. */
	uint8_t missileLockState;
	/* IDB +0x1D: Selected bank index within the current laser/warhead weapon mode. The weapon-cycle action
	 * increments it and wraps against that mode's bank count. Snapshot block 9, offset +0x1D. Existing field
	 * purpose checked against its X-Wing users. */
	uint8_t selectedWeaponBank;
	/* IDB +0x1E: Selected weapon mode: 0 lasers, nonzero warheads. Shared with the weapon-cycle input handler
	 * and player firing dispatcher. Snapshot block 9, offset +0x1E. Existing field purpose checked against
	 * its X-Wing users. */
	uint8_t selectedWeaponMode;
	/* IDB +0x1F: Snapshot block 9, offset +0x1F. Existing field purpose checked against its X-Wing users. */
	uint8_t incomingWarheadAlertState;
	/* IDB +0x20: Snapshot block 9, offset +0x20. Existing field purpose checked against its X-Wing users. */
	uint16_t incomingWarheadObjectIndex;
	/* IDB +0x22: Input smoothing resets when (g_flightKeyMods & 0xE)==2 changes; same mode doubles roll
	 * adjustment. Snapshot block 9, offset +0x22. Existing field purpose checked against its X-Wing users. */
	uint16_t previousRollModifierMode;
	/* IDB +0x24: Paired signed 16-bit roll and pitch smoothing accumulators. Both reset by one dword store;
	 * consumed by Player_ScaleControlStepByElapsedTicks and USER_calcdeltapitch. Snapshot block 9, offset
	 * +0x24. Existing field purpose checked against its X-Wing users. */
	struct XwPlayerSmoothedTurnInput smoothedTurnInput;
	/* IDB +0x28: Previous input modifiers used to distinguish a target-button tap from a hold. Snapshot block
	 * 9, offset +0x28. Existing field purpose checked against its X-Wing users. */
	uint16_t savedKeyModifiers;
	/* IDB +0x2A: Accumulated target-button hold time in 236 Hz ticks. Snapshot block 9, offset +0x2A.
	 * Existing field purpose checked against its X-Wing users. */
	uint16_t targetButtonHoldTicks;
	/* IDB +0x2C: Player shots fired and hit categories, shared layout with CraftData.weaponStats. Initialized
	 * on player craft spawn and displayed in debrief. Snapshot block 9, offset +0x2C. Existing field purpose
	 * checked against its X-Wing users. */
	struct XwWeaponHitStats weaponStats;
	/* IDB +0x3B: 24 per-mission player spacecraft kill counters. Cleared as 48 bytes at player creation,
	 * incremented by COLLIDE_updatekills, aggregated by debrief; wrap from 65535 is replaced by 255. Snapshot
	 * block 9, offset +0x3B. Existing field purpose checked against its X-Wing users. */
	uint16_t spacecraftKillsByType[24];
	/* IDB +0x6B: Per-mission static space objects destroyed by the player; shown by the debrief Space Objects
	 * Destroyed line. Snapshot block 9, offset +0x6B. Existing field purpose checked against its X-Wing
	 * users. */
	uint16_t spaceObjectKills;
	/* IDB +0x6D: Per-mission Death Star buildings destroyed by the player; separate from static space
	 * objects. Snapshot block 9, offset +0x6D. Existing field purpose checked against its X-Wing users. */
	uint16_t deathStarBuildingKills;
	/* IDB +0x6F: Eight subsystem IDs in saved-flight repair-priority order. Initialized to 0..7, copied
	 * to/from active priorities across the frontend, and decoded from replay records. Snapshot block 9,
	 * offset +0x6F. Existing field purpose checked against its X-Wing users. */
	uint8_t savedSubsystemRepairPriority[XW_PLAYER_SUBSYSTEM_COUNT];
	/* IDB +0x77: Serialized bytes with no identified direct semantic use. Do not assume padding or assign
	 * field meaning. */
	uint8_t gap_77[1];
	/* IDB +0x78: Snapshot block 9, offset +0x78. Existing field purpose checked against its X-Wing users. */
	uint16_t subsystemHealth[XW_PLAYER_SUBSYSTEM_COUNT];
	/* IDB +0x88: Serialized bytes with no identified direct semantic use. Do not assume padding or assign
	 * field meaning. */
	uint8_t gap_88[2];
	/* IDB +0x8A: Snapshot block 9, offset +0x8A. Existing field purpose checked against its X-Wing users. */
	uint16_t subsystemRepairTimers[XW_PLAYER_SUBSYSTEM_COUNT];
	/* IDB +0x9A: Serialized bytes with no identified direct semantic use. Do not assume padding or assign
	 * field meaning. */
	uint8_t gap_9A[2];
	/* IDB +0x9C: MOVE_moveobjects rotates cockpit local offset and stores X/Y/Z. Camera adds it to world
	 * position; collision probe uses same point. Snapshot block 9, offset +0x9C. Existing field purpose
	 * checked against its X-Wing users. */
	struct XwCameraPosition rotatedCockpitOffset;
	/* IDB +0xA8: MOVE_moveobjects snapshots previous rotated cockpit offset; collision segment start adds
	 * this to previous object world position. Snapshot block 9, offset +0xA8. Existing field purpose checked
	 * against its X-Wing users. */
	struct XwCameraPosition previousRotatedCockpitOffset;
};

enum { USER_GRAPHICS_DETAIL_PRESET_COUNT = 4 };

extern uint16_t g_starDensity;
extern const uint16_t g_craftExplosionSpawnThresholdByGraphicsDetailPreset[USER_GRAPHICS_DETAIL_PRESET_COUNT];
extern const uint16_t g_starshipDetailByPreset[USER_GRAPHICS_DETAIL_PRESET_COUNT];
extern const uint16_t g_starDensityByGraphicsDetailPreset[USER_GRAPHICS_DETAIL_PRESET_COUNT];
extern const uint16_t g_backdropsEnabledByGraphicsDetailPreset[USER_GRAPHICS_DETAIL_PRESET_COUNT];
extern const uint16_t g_debrisEnabledByGraphicsDetailPreset[USER_GRAPHICS_DETAIL_PRESET_COUNT];
extern const int16_t g_shipDetailValueByPreset[USER_GRAPHICS_DETAIL_PRESET_COUNT];
extern const uint16_t g_shipDetailPolyCountByPreset[USER_GRAPHICS_DETAIL_PRESET_COUNT];
extern const int16_t g_drawMarkingsByPreset[USER_GRAPHICS_DETAIL_PRESET_COUNT];
extern const uint16_t g_deathStarDetailLevelByPreset[USER_GRAPHICS_DETAIL_PRESET_COUNT];
extern const uint16_t g_surfaceObjectDetailLimitByPreset[USER_GRAPHICS_DETAIL_PRESET_COUNT];
extern uint16_t g_trenchObjectDetailLimitByPreset[USER_GRAPHICS_DETAIL_PRESET_COUNT];
extern const uint16_t g_hyperspaceEffectObjectCountByPreset[USER_GRAPHICS_DETAIL_PRESET_COUNT];
extern const uint16_t g_gouraudEnableMaskByPreset[USER_GRAPHICS_DETAIL_PRESET_COUNT];
extern uint16_t g_missionCheatOptionsUsed;
extern uint8_t g_unlimitedWeaponsEnabled;
extern uint8_t g_flightInvulnerabilityEnabled;
extern uint8_t g_subsystemIdToFlag[XW_PLAYER_SUBSYSTEM_COUNT];
extern uint8_t g_subsystemMessageArgById[XW_PLAYER_SUBSYSTEM_COUNT];
extern uint8_t g_subsystemRepairDuration[XW_PLAYER_SUBSYSTEM_COUNT];
extern uint8_t g_subsystemFailureHudMaskByRandomSlot[XW_HUD_FAILURE_RANDOM_SLOT_COUNT];
extern uint8_t g_playerSubsystemRepairPriority[XW_PLAYER_SUBSYSTEM_COUNT];
extern uint16_t g_hyperspaceEffectObjectCount;
extern uint8_t g_debrisEnabled;
/* TIE shipdetailpolycnt: polygon-count cutoff for DOS LOD fallback; the DOS reader uses the low byte. */
extern uint16_t g_shipDetailPolyCount;
extern uint16_t g_craftExplosionSpawnThreshold;
/* Geometry tier limit for DOS Death Star surface/trench rendering, separate from object detail limits. */
extern uint16_t g_deathStarDetailLevel;
extern uint16_t g_surfaceObjectDetailLimit;
extern XwPlayerFlightState g_playerFlightState;
extern uint8_t g_transformLightDirectionToObjectSpace;
/* TIE shipdetailvalue: -1 selects the preceding DOS LOD, 0 keeps it, positive values allow LOD skips. */
extern int16_t g_shipDetailValue;
/* TIE starshipdetail: whole-ship LOD depth threshold in units of 32768 for DOS capital-ship draw paths. */
extern uint16_t g_starshipDetail;
extern uint16_t g_trenchObjectDetailLimit;
extern uint8_t g_backdropsEnabled;
extern uint8_t g_replayUiEventConsumed;
/* TIE drawmarkingsflag counterpart; X-Wing stores a word and treats any nonzero value as enabled. */
extern int16_t g_drawMarkingsFlag;
/* Bit 0x40 permits DOS94 per-vertex lighting; intersected with each face's Gouraud bit. */
extern uint16_t g_gouraudEnableMask;

/* Declarations follow ascending original IDB address. */

/* 0x42A640 */
void user_userinterface(void);

/* 0x42ADA0 */
void user_nextreplaycount(void);

/* 0x42ADF0 */
void user_nextreplaystore(void);

/* 0x42AE90 */
void user_inputforplane(void);

/* 0x42CC70 */
int user_framerateadjust(int16_t step);

/* 0x42CC90 */
void user_increasepower(uint16_t engineIndex, uint16_t step);

/* 0x42CCD0 */
void user_decreasepower(uint16_t engineIndex, uint16_t step);

/* 0x42CD10 */
void user_adjustshields(uint16_t destinationBank, uint16_t sourceBank);

/* 0x42CDC0 */
void user_resetview(void);

/* 0x42CE60 */
uint16_t user_picktarget(void);

/* 0x42CF30 */
uint16_t user_picknexttarget(uint16_t start, int16_t step);

/* 0x42D030 */
int16_t user_targetincross(uint16_t candidateObjRef, uint16_t maxAngleScore);

/* 0x42D380 */
void user_setnewtarget(uint16_t newTargetObjIdx);

/* 0x42D540 */
int16_t user_CanRevealCraftIdentity(uint16_t objectIndex, const struct CraftData* craft);

/* 0x42D580 */
void user_calcdeltapitch(int16_t pitchDelta, int16_t yawDelta, uint16_t objectIndex, struct CraftData* craft);

/* 0x42DB00 */
void user_setdetaillevel(uint16_t preset);

/* 0x42DBE0 */
void user_ApplyPreferences(void);

/* 0x42DD90 */
int16_t user_checkradio(void);

/* 0x42DE00 */
void user_PrepareFlightStatusSnapshot(void);

/* 0x42DEA0 */
void user_RestoreRepairPriorities(void);

/* 0x42DEE0 */
void user_assigntarget(uint16_t targetObjIdx, XwFlightMessageId commandTextId);

/* 0x42DF90 */
int16_t user_findclosestattacker(void);

/* 0x42E010 */
int16_t user_isrescued(uint16_t subjectObjectIndex);

/* 0x42E0B0 */
void user_checkreplaycamera(void);

/* 0x42E100 */
void user_ejectcamera(void);

#ifdef __cplusplus
}
#endif

#endif
