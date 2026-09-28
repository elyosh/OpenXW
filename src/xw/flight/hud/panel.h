#ifndef XW_FLIGHT_HUD_PANEL_H
#define XW_FLIGHT_HUD_PANEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/render/rtsvga2.h"

#include <stddef.h>
#include <stdint.h>

typedef struct CockpitLfdEntryHeader CockpitLfdEntryHeader;
typedef struct HudCockpitResource HudCockpitResource;
typedef struct HudCockpitResourceDescriptor HudCockpitResourceDescriptor;
typedef struct HudElementLayout HudElementLayout;
typedef struct HudPanelSpriteFileInfo HudPanelSpriteFileInfo;

struct XwRadarBlip;
extern struct XwRadarBlip* g_radarRearBlips;
extern uint16_t g_radarBlipColorIndex;
extern uint16_t g_radarFrontBlipCount;
extern struct XwRadarBlip* g_radarFrontBlips;
extern uint16_t g_radarRearBlipCount;
extern int16_t g_targetComputerMarkerScreenX;
extern int16_t g_targetComputerMarkerScreenY;

enum {
	PANEL_VIEW_NO_COCKPIT = 18,
	PANEL_VIEW_FULL_FORWARD = 19,
	PANEL_VIEW_DIRECTION_LABEL = 17,
	PANEL_VIEW_ALIAS = 0x80,
	PANEL_VIEW_MIRRORED_ALIAS = 0xC0,
	PANEL_VIEW_CACHE_INVALID = -1,
	PANEL_VIEW_WIDTH = 640,
	PANEL_COCKPIT_HEIGHT = 455,
	PANEL_COCKPIT_IMAGE_ENTRY = 0,
	PANEL_COCKPIT_MASK_ENTRY = 1,
	PANEL_COCKPIT_PALETTE_ENTRY = 2,
	PANEL_COCKPIT_PALETTE_COLORS = 64,
	PANEL_FORWARD_CRAFT_0 = 0,
	PANEL_FORWARD_CRAFT_1 = 1,
	PANEL_FORWARD_CRAFT_2 = 2,
	PANEL_FORWARD_CRAFT_3 = 3,
	PANEL_FORWARD_OFFSET_0 = -64,
	PANEL_FORWARD_OFFSET_1 = -86,
	PANEL_FORWARD_OFFSET_2 = -70,
	PANEL_FORWARD_OFFSET_3 = -66,
	PANEL_YWING_COCKPIT_OFFSET = -10,
	PANEL_DIRECTION_LOW_WIDTH = 320,
	PANEL_DIRECTION_LOW_LEFT = 139,
	PANEL_DIRECTION_HIGH_LEFT = 282,
	PANEL_DIRECTION_LOW_TOP = 6,
	PANEL_DIRECTION_HIGH_TOP = 12,
	PANEL_DIRECTION_LOW_RIGHT = 182,
	PANEL_DIRECTION_HIGH_RIGHT = 362,
	PANEL_DIRECTION_LOW_BOTTOM = 12,
	PANEL_DIRECTION_HIGH_BOTTOM = 30,
	PANEL_DIRECTION_LOW_TEXT_Y = 6,
	PANEL_DIRECTION_HIGH_TEXT_Y = 14,
	PANEL_DIRECTION_BACKGROUND = 0x40,
	PANEL_DIRECTION_TEXT_COLOR = 0x49
};

extern int g_hudClearHeight;
extern uint8_t g_hudLoadedPanelSetId;
extern uint8_t g_hudCockpitMirrorHorizontal;
extern int16_t g_hudLoadedCockpitView;

enum {
	PANEL_ELEMENT_CACHE_INVALID = -2,
	PANEL_FRAME_CACHE_INVALID = 0xFFFF,
	PANEL_DISPLAY_MODE_INVALID = 0xFF
};

enum { PANEL_SHIELD_DISTRIBUTION_ELEMENT = 19, PANEL_SFOIL_ELEMENT = 31 };

enum {
	PANEL_CMD_FEATURE = 1,
	PANEL_CMD_WIDTH = 132,
	PANEL_CMD_HEIGHT = 91,
	PANEL_CMD_TITLE_HEIGHT = 11,
	PANEL_CMD_FOOTER_BOTTOM = 103,
	PANEL_CMD_STATUS_BOTTOM = 106,
	PANEL_CMD_BACKGROUND = 0x1D,
	PANEL_CMD_TITLE_COLOR = 0x49,
	PANEL_CMD_DISTANCE_COLOR = 0x44,
	PANEL_CMD_CARGO_COLOR = 0x41,
	PANEL_CMD_PLAYER_COLOR = 0x45,
	PANEL_CMD_STATUS_COLOR = 0x47,
	PANEL_CMD_DISTANCE_LABEL_WIDTH = 88,
	PANEL_CMD_DISTANCE_LEFT = 15,
	PANEL_CMD_DISTANCE_RIGHT = 64,
	PANEL_CMD_CARGO_LEFT = 28,
	PANEL_CMD_CARGO_TOP = 74,
	PANEL_CMD_WARHEAD_LEFT = 26,
	PANEL_CMD_WARHEAD_TOP = 77,
	PANEL_CMD_CARGO_ELEMENT = 54,
	PANEL_CMD_STATUS_ELEMENT = 57,
	PANEL_CMD_RETICLE_SPRITE = 0,
	PANEL_CMD_RETICLE_TOP = 12,
	PANEL_CMD_DETAIL_SPRITE = 4,
	PANEL_CMD_DETAIL_LEFT = 68,
	PANEL_CMD_VARIANT_SPRITE_BASE = 5,
	PANEL_CMD_VARIANT_MASK = 3,
	PANEL_CMD_VARIANT_LEFT = 114,
	PANEL_CMD_VARIANT_TOP = 47,
	PANEL_CMD_SHIP_SPRITE_BASE = 8,
	PANEL_CMD_BWING_SPRITE = 109,
	PANEL_CMD_INTERDICTOR_SPRITE = 110,
	PANEL_CMD_SHARED_SHIP_FIRST = 75,
	PANEL_CMD_SHARED_SHIP_LAST = 78,
	PANEL_CMD_LOCK_SPRITE = 2,
	PANEL_CMD_UNLOCKED_SPRITE = 1,
	PANEL_CMD_LOCK_LEFT = 43,
	PANEL_CMD_LOCK_TOP = 35,
	PANEL_CMD_CROSS_COLOR = 0x3F,
	PANEL_CMD_EXHAUST_Y = 61248,
	PANEL_CMD_EXHAUST_RANGE = 0x1DD36,
	PANEL_CMD_EXHAUST_NEAR_FRAME = 3,
	PANEL_CMD_EXHAUST_STEP_SHIFT = 4,
	PANEL_CMD_EXHAUST_FRAME_DIVISOR = 85,
	PANEL_CMD_EXHAUST_LAST_STEP = 2,
	PANEL_CMD_EXHAUST_SPRITE_BASE = 170,
	PANEL_CMD_EXHAUST_STATUS_BASE = 9,
	PANEL_CMD_WARHEAD_STATUS_BASE = 4,
	PANEL_CMD_CARGO_UNKNOWN = 0,
	PANEL_CMD_CARGO_KNOWN = 1,
	PANEL_CMD_CARGO_EMPTY = 2
};

extern uint8_t g_hudExhaustPortDisplayMode;
extern uint16_t g_hudCachedExhaustPortFrame;
extern uint16_t g_targetComputerLockSpriteIndex;
extern uint16_t g_targetComputerCachedLockSpriteIndex;
extern uint8_t g_targetComputerCrossBackgroundSaved;
extern uint16_t g_targetComputerPreviousCrossX;
extern uint16_t g_targetComputerPreviousCrossY;

enum {
	PANEL_RADAR_HUD_FEATURE = 0x20,
	PANEL_RADAR_HIDDEN_CRAFT_KIND = 3,
	PANEL_RADAR_STATIC_COLOR = 2,
	PANEL_RADAR_PROJECTILE_FAMILY = 1,
	PANEL_RADAR_PROJECTILE_COLOR = 59,
	PANEL_RADAR_IFF0_COLOR = 63,
	PANEL_RADAR_IFF1_COLOR = 55,
	PANEL_RADAR_OTHER_COLOR = 51,
	PANEL_RADAR_STATIC_FAR_COLOR = 8,
	PANEL_RADAR_STATIC_MIDDLE_COLOR = 5,
	PANEL_RADAR_FAR_RANGE = 122166,
	PANEL_RADAR_MIDDLE_RANGE = 61083,
	PANEL_RADAR_TARGET_Y_LIMIT = 32,
	PANEL_RADAR_TARGET_CENTER_X = 66,
	PANEL_RADAR_TARGET_CENTER_Y = 46,
	PANEL_RADAR_FRONT_ELEMENT = 0,
	PANEL_RADAR_REAR_ELEMENT = 1,
	PANEL_RADAR_TARGET_ELEMENT = 2,
	PANEL_RADAR_BLIP_CAPACITY = 48
};

extern uint8_t g_radarBlipBufferParity;
extern uint8_t g_radarTargetMarkerBackgroundSaved;
extern uint16_t g_hudCachedTargetObjectIdx;

enum { PANEL_TARGET_CACHE_INVALIDATED = 0xFFFD };

extern uint16_t g_radarPreviousRearBlipCount;
extern struct XwRadarBlip* g_radarPreviousFrontBlips;
extern uint16_t g_radarPreviousFrontBlipCount;
extern struct XwRadarBlip g_radarFrontBlipsA[PANEL_RADAR_BLIP_CAPACITY];
extern struct XwRadarBlip g_radarFrontBlipsB[PANEL_RADAR_BLIP_CAPACITY];
extern struct XwRadarBlip* g_radarPreviousRearBlips;
extern struct XwRadarBlip g_radarRearBlipsA[PANEL_RADAR_BLIP_CAPACITY];
extern struct XwRadarBlip g_radarRearBlipsB[PANEL_RADAR_BLIP_CAPACITY];

enum { PANEL_OBJECT_BOX_CORNER_SHIFT = 3, PANEL_OBJECT_BOX_CORNER_MIN = 3 };

enum {
	PANEL_BUOY_NAME_COUNT = 3,
	PANEL_DYNAMIC_BUOY_OBJECT_TYPE = 83,
	PANEL_OBJECT_TYPE_IFF0_COLOR = 70,
	PANEL_OBJECT_TYPE_IFF1_COLOR = 68,
	PANEL_OBJECT_TYPE_OTHER_COLOR = 66,
	PANEL_OBJECT_NAME_IFF0_COLOR = 69,
	PANEL_OBJECT_NAME_IFF1_COLOR = 67,
	PANEL_OBJECT_NAME_OTHER_COLOR = 65,
	PANEL_STATIC_OBJECT_COLOR = 73,
	PANEL_BUOY_HUD_ID_FIRST = 30,
	PANEL_BUOY_HUD_ID_LAST = 32,
	PANEL_BUOY_BASE_OBJECT_TYPE = 70,
	PANEL_MINE_OBJECT_TYPE_FIRST = 75,
	PANEL_MINE_OBJECT_TYPE_LAST = 78
};

enum {
	PANEL_CRAFT_STATUS_NORMAL = 0,
	PANEL_CRAFT_STATUS_DISABLED = 2,
	PANEL_CRAFT_STATUS_CAPTURED = 3,
	PANEL_CRAFT_STATUS_SHIELDS_DOWN = 6,
	PANEL_CRAFT_STATUS_DAMAGED = 7,
	PANEL_CRAFT_STATUS_8 = 8
};

enum {
	PANEL_HUD_ELEMENT_COUNT = 75,
	PANEL_HUD_SPRITE_COUNT = 174,
	PANEL_SHARED_SPRITE_COUNT = 111,
	PANEL_RESOURCE_FILENAME_CAPACITY = 20,
	PANEL_SPRITE_RECORD_END = 0xFF,
	PANEL_COCKPIT_DESCRIPTOR_COUNT = 20,
	PANEL_COCKPIT_PRELOAD_ENABLED = 1,
	PANEL_COCKPIT_ENTRY_COUNT = 3,
	PANEL_COCKPIT_PATH_CAPACITY = 32,
	PANEL_COCKPIT_DIRECTORY_CAPACITY = 8,
	PANEL_LFD_TYPE_TAG_LENGTH = 4,
	PANEL_PALETTE_COMPONENT_SHIFT = 2,
	PANEL_PALETTE_RANGE_HEADER_SIZE = 2,
	PANEL_SETTING_TRANSPARENT_COLOR = 253,
	PANEL_SPAN_MASK_INITIAL_OFFSET = 0xC000,
	PANEL_SPAN_MASK_CLEAR_ROW = 1,
	PANEL_SPAN_MASK_EXTENDED_RUN = 0,
	PANEL_SPAN_MASK_BYTE_RANGE = 256,
	PANEL_SPAN_MASK_FIRST_EXTENSION = 255,
	PANEL_SPAN_MASK_ROW_RUN_CAPACITY = 99
};

enum {
	PANEL_SYSTEM_STATUS_FIRST_ELEMENT = 46,
	PANEL_SYSTEM_STATUS_ELEMENT_COUNT = 8,
	PANEL_SYSTEM_STATUS_INITIAL_MASK = 1,
	PANEL_SYSTEM_STATUS_HOLD_MASK = 6
};

enum {
	PANEL_COCKPIT_DAMAGE_FIRST_ELEMENT = 58,
	PANEL_COCKPIT_DAMAGE_ELEMENT_COUNT = 16,
	PANEL_COCKPIT_MIRRORED_RESOURCE = 0xC0,
	PANEL_COCKPIT_WIDTH = 320
};

enum {
	PANEL_HUD_WEAPON_WARNING_FEATURE = 2,
	PANEL_WEAPON_WARNING_ELEMENT = 23,
	PANEL_WEAPON_LOCK_COMPLETE_TICKS = 944,
	PANEL_WEAPON_WARNING_FLASH_TICKS = 59,
	PANEL_WEAPON_WARNING_LOCKED = 2
};

enum {
	PANEL_HUD_WEAPONS_FEATURE = 0x10,
	PANEL_HARDPOINT_INDICATOR_FIRST_ELEMENT = 11,
	PANEL_HARDPOINT_COUNT_FIRST_ELEMENT = 13,
	PANEL_HARDPOINT_TEXT_COLOR = 0x43,
	PANEL_HARDPOINT_DIGITS = 1,
	PANEL_HARDPOINT_DISABLED = 0,
	PANEL_HARDPOINT_UNSELECTED = 1,
	PANEL_HARDPOINT_SELECTED = 4
};

enum {
	PANEL_LASER_CHARGE_FIRST_ELEMENT = 3,
	PANEL_LASER_WEAPON_MODE = 0,
	PANEL_XWING_CRAFT_TYPE = 0,
	PANEL_LASER_READY_FIRST_ELEMENT = 7,
	PANEL_LASER_RETICLE_FIRST_ELEMENT = 38,
	PANEL_LASER_RANGE_FIRST_ELEMENT = 42,
	PANEL_BWING_LASER_READY_FIRST_ELEMENT = 58,
	PANEL_BWING_LASER_RANGE_FIRST_ELEMENT = 66,
	PANEL_LASER_LOW_RES_SEGMENTS = 8,
	PANEL_LASER_HIGH_RES_SEGMENTS = 10,
	PANEL_LASER_LOW_RES_CHARGE_SHIFT = 3,
	PANEL_LASER_HIGH_RES_CHARGE_DIVISOR = 6,
	PANEL_LASER_LOW_RES_X_STEP = 4,
	PANEL_LASER_HIGH_RES_X_STEP = 6,
	PANEL_LASER_DISABLED = 0,
	PANEL_LASER_UNSELECTED = 1,
	PANEL_LASER_READY = 3,
	PANEL_LASER_COOLDOWN = 5,
	PANEL_LASER_RETICLE_COOLDOWN = 2,
	PANEL_LASER_CHARGE_NORMAL = 1,
	PANEL_LASER_CHARGE_HIGH = 2
};

enum {
	PANEL_HUD_SHIELDS_FEATURE = 8,
	PANEL_FRONT_SHIELD_ELEMENT = 15,
	PANEL_FRONT_OVERCHARGE_ELEMENT = 16,
	PANEL_REAR_SHIELD_ELEMENT = 17,
	PANEL_REAR_OVERCHARGE_ELEMENT = 18,
	PANEL_HULL_ELEMENT = 20,
	PANEL_SHIELD_FULL_STATE = 9,
	PANEL_SHIELD_FLASH_STATE = 10,
	PANEL_GUNSIGHT_ELEMENT = 37,
	PANEL_GUNSIGHT_NO_TARGET_STATE = 1,
	PANEL_GUNSIGHT_LASER_LOCK_STATE = 4,
	PANEL_GUNSIGHT_MISSILE_LOCKED = 2,
	PANEL_CRITICAL_WARNING_ELEMENT = 36,
	PANEL_CRITICAL_SHIELD_THRESHOLD = 100,
	PANEL_CRITICAL_HULL_BAND = 2,
	PANEL_CRITICAL_FLASH_TICKS = 59,
	PANEL_HULL_TIERS = 3,
	PANEL_HULL_FULL_STATE = 2,
	PANEL_HULL_FLASH_STATE = 3
};

enum {
	PANEL_HUD_SPEED_FEATURE = 4,
	PANEL_SPEED_ELEMENT = 21,
	PANEL_SPEED_SCALE_Q16 = 0x71C7,
	PANEL_SPEED_TEXT_COLOR = 0x45,
	PANEL_SPEED_DIGITS = 3,
	PANEL_SPEED_MIN_DIGITS = 1
};

enum {
	PANEL_HUD_THROTTLE_FEATURE = 4,
	PANEL_THROTTLE_ELEMENT = 22,
	PANEL_THROTTLE_LOW_RES_DIVISOR = 1725,
	PANEL_THROTTLE_HIGH_RES_DIVISOR = 862,
	PANEL_THROTTLE_LOW_RES_COLUMNS = 38,
	PANEL_THROTTLE_HIGH_RES_COLUMNS = 74,
	PANEL_THROTTLE_LOW_RES_TOP_BAND = 4,
	PANEL_THROTTLE_HIGH_RES_TOP_BAND = 7,
	PANEL_THROTTLE_LOW_RES_UPPER_BANDS = 8,
	PANEL_THROTTLE_HIGH_RES_UPPER_BANDS = 14,
	PANEL_THROTTLE_LOW_COLOR = 62,
	PANEL_THROTTLE_MIDDLE_COLOR = 58,
	PANEL_THROTTLE_HIGH_COLOR = 54,
	PANEL_THROTTLE_ALTERNATE_COLOR_STEP = 2
};

enum {
	PANEL_DISTANCE_WHOLE_ELEMENT = 55,
	PANEL_DISTANCE_FRACTION_ELEMENT = 56,
	PANEL_DISTANCE_SCALE = 161,
	PANEL_DISTANCE_SCALE_SHIFT = 16,
	PANEL_DISTANCE_MAX_HUNDREDTHS = 9999,
	PANEL_DISTANCE_HUNDREDTHS_PER_UNIT = 100,
	PANEL_DISTANCE_WHOLE_X = 24,
	PANEL_DISTANCE_FRACTION_X = 44,
	PANEL_DISTANCE_Y = 91,
	PANEL_DISTANCE_TEXT_COLOR = 0x43,
	PANEL_DISTANCE_FIELD_DIGITS = 2,
	PANEL_DISTANCE_WHOLE_MIN_DIGITS = 1
};

enum {
	PANEL_CLOCK_ELEMENT = 32,
	PANEL_CLOCK_TEXT_COLOR = 0x4E,
	PANEL_CLOCK_SECONDS_X_OFFSET = 20,
	PANEL_CLOCK_FIELD_DIGITS = 2,
	PANEL_CLOCK_MINUTE_MIN_DIGITS = 1
};

enum {
	PANEL_REPLAY_RECORDING_ELEMENT = 33,
	PANEL_REPLAY_COUNTER_ELEMENT = 34,
	PANEL_REPLAY_LOW_RES_WIDTH = 13,
	PANEL_REPLAY_HIGH_RES_WIDTH = 26,
	PANEL_REPLAY_LOW_RES_HEIGHT = 6,
	PANEL_REPLAY_HIGH_RES_HEIGHT = 10,
	PANEL_REPLAY_BACKGROUND_COLOR = 0x1D,
	PANEL_REPLAY_TEXT_COLOR = 0x49,
	PANEL_REPLAY_FULL_PERCENT = 100,
	PANEL_REPLAY_MAX_PERCENT = 99,
	PANEL_REPLAY_IDLE_PERCENT = -1,
	PANEL_REPLAY_DIGITS = 3,
	PANEL_REPLAY_MIN_DIGITS = 1
};

enum {
	PANEL_HUD_POWER_FEATURE = 2,
	PANEL_MAX_SPEED_ELEMENT = 27,
	PANEL_ENGINE_POWER_ELEMENT = 28,
	PANEL_LASER_POWER_ELEMENT = 29,
	PANEL_SHIELD_POWER_ELEMENT = 30,
	PANEL_REDIRECT_POWER_SEGMENTS = 4,
	PANEL_REDIRECT_POWER_Y_STEP = 10,
	PANEL_ENGINE_POWER_SEGMENTS = 8,
	PANEL_ENGINE_POWER_Y_STEP = 5,
	PANEL_NEUTRAL_ENGINE_POWER = 4,
	PANEL_POWER_SPEED_STEP_Q16 = 0x2000,
	PANEL_MAX_SPEED_TEXT_COLOR = 0x41,
	PANEL_MAX_SPEED_DIGITS = 3,
	PANEL_MAX_SPEED_MIN_DIGITS = 1
};

extern char g_hudCockpitResolutionDirectory[PANEL_COCKPIT_DIRECTORY_CAPACITY];
extern const char g_lfdPaletteResourceTypeTag[PANEL_LFD_TYPE_TAG_LENGTH + 1];
extern const char* g_hudBuoyNameStrings[PANEL_BUOY_NAME_COUNT];
extern unsigned int g_viewportSpanMaskOffset;
extern int g_replayHudShowedSimStepScale;
extern uint8_t* g_hudPanelSpriteDataBuffer;
extern int16_t g_ReplayProgressPercent;
extern uint8_t* g_hudPanelSpriteDataByIndex[PANEL_HUD_SPRITE_COUNT];
extern uint8_t g_hudCockpitResourcesLoaded;
extern uint8_t* g_hudPanelSpriteDataWriteCursor;
extern char g_hudCockpitBasePath[PANEL_COCKPIT_PATH_CAPACITY];
extern uint8_t* g_hudCockpitResourceWriteCursor;
extern uint8_t* g_hudPanelSpriteCraftDataStart;
extern uint8_t g_hudPanelSetId;
extern uint16_t g_hudElementStateCache[PANEL_HUD_ELEMENT_COUNT];

/* Original IDB size: 16 bytes. */
struct CockpitLfdEntryHeader {
	/* IDB +0x0 */
	char typeTag[PANEL_LFD_TYPE_TAG_LENGTH];
	/* IDB +0x4 */
	char resourceName[8];
	/* IDB +0xC */
	uint32_t payloadSize;
};

typedef char xw_size_CockpitLfdEntryHeader[(sizeof(CockpitLfdEntryHeader) == 16) ? 1 : -1];
typedef char xw_offset_CockpitLfdEntryHeader_payloadSize[(offsetof(CockpitLfdEntryHeader, payloadSize) == 12)
															 ? 1
															 : -1];

/* Original IDB size: 14 bytes. */
struct HudCockpitResource {
	/* IDB +0x0 */
	uint16_t memoryHandle;
	/* IDB +0x2: Three payload pointers: cockpit image, encoded viewport span mask, palette RGB bytes. */
	uint8_t* entries[PANEL_COCKPIT_ENTRY_COUNT];
};

/* Original IDB size: 30 bytes. */
struct HudCockpitResourceDescriptor {
	/* IDB +0x0: 1 preloads this LFD. Values 0x80..0xBF alias another view; 0xC0 and above alias with
	 * horizontal mirroring. View 18 is handled specially. */
	uint8_t enabled;
	/* IDB +0x1 */
	char lfdName[9];
	/* IDB +0xA */
	int16_t viewportX;
	/* IDB +0xC */
	int16_t viewportY;
	/* IDB +0xE */
	int16_t viewportWidth;
	/* IDB +0x10 */
	int16_t viewportHeight;
	/* IDB +0x12: Text drawn for the mapped view-17 overlay; shipped files contain clock-face directions. */
	char viewLabel[12];
};

typedef char xw_size_HudCockpitResourceDescriptor[(sizeof(HudCockpitResourceDescriptor) == 30) ? 1 : -1];
typedef char xw_offset_HudCockpitResourceDescriptor_viewportX
	[(offsetof(HudCockpitResourceDescriptor, viewportX) == 10) ? 1 : -1];
typedef char xw_offset_HudCockpitResourceDescriptor_viewLabel
	[(offsetof(HudCockpitResourceDescriptor, viewLabel) == 18) ? 1 : -1];

/* Original IDB size: 6 bytes. */
struct HudElementLayout {
	/* IDB +0x0 */
	int16_t x;
	/* IDB +0x2 */
	int16_t y;
	/* IDB +0x4 */
	uint8_t spriteIndex;
	/* IDB +0x5: Context-dependent byte: some consumers compare it to the current view, others pass it as the
	 * sprite draw selector. */
	uint8_t selector;
};

typedef char xw_size_HudElementLayout[(sizeof(HudElementLayout) == 6) ? 1 : -1];

extern HudCockpitResourceDescriptor g_hudCockpitResourceDescriptors[PANEL_COCKPIT_DESCRIPTOR_COUNT];
extern uint8_t g_hudFullRedrawInProgress;
extern HudCockpitResource g_hudCockpitResources[PANEL_COCKPIT_DESCRIPTOR_COUNT];
extern HudPanelSpriteFileInfo g_hudPanelSpriteFileInfo;
extern char g_hudCockpitResourcePath[PANEL_COCKPIT_PATH_CAPACITY];
extern HudElementLayout g_hudElementLayouts[PANEL_HUD_ELEMENT_COUNT];

/* Original IDB size: 11 bytes. */
struct HudPanelSpriteFileInfo {
	/* IDB +0x0 */
	char baseName[9];
	/* IDB +0x9 */
	uint8_t spriteCount;
	/* IDB +0xA: Added to spriteCount when loading the craft panel records; both bytes are nonzero in shipped
	 * X-Wing INT files. */
	uint8_t spriteCountAddend;
};

typedef char xw_size_HudPanelSpriteFileInfo[(sizeof(HudPanelSpriteFileInfo) == 11) ? 1 : -1];
typedef char xw_offset_HudPanelSpriteFileInfo_spriteCount[(offsetof(HudPanelSpriteFileInfo, spriteCount) == 9)
															  ? 1
															  : -1];

extern uint8_t g_targetLockActive;

/* Declarations follow ascending original IDB address. */

/* 0x418F60 */
void panel_initpanel(void);

/* 0x418FD0 */
void panel_updatepanel(void);

/* 0x4190B0 */
void panel_updateforwardpanel(void);

/* 0x419140 */
void panel_updatefullforward(void);

/* 0x419150 */
void panel_updateradar(void);

/* 0x419390 */
void panel_addbliptoradar(uint16_t objectRef);

/* 0x419830 */
void panel_updatecmd(void);

/* 0x41A1C0 */
void panel_buildobjectname(uint16_t objectRef, uint16_t objectType);

/* 0x41A3E0 */
int panel_getcraftstatus(uint16_t objectIndex);

/* 0x41A470 */
void panel_outputdistance(int polarDistance, int16_t panelX, int16_t panelY);

/* 0x41A530 */
void panel_updategunsight(void);

/* 0x41A590 */
void panel_updatelasers(void);

/* 0x41AA00 */
void panel_updateweapons(void);

/* 0x41AA70 */
void panel_updatehardpoint(uint16_t warheadSlotIdx, uint16_t displaySlot);

/* 0x41ABB0 */
void panel_updateshields(void);

/* 0x41ADA0 */
void panel_updatespeed(void);

/* 0x41AE50 */
void panel_updateclock(void);

/* 0x41AF20 */
void panel_updatepower(void);

/* 0x41B060 */
void panel_updatesetting(uint16_t filledCount, uint16_t elementIdx, uint16_t segmentCount, int16_t yStep);

/* 0x41B100 */
void panel_updatethrottle(void);

/* 0x41B2C0 */
void panel_updateweaponwarnings(void);

/* 0x41B370 */
void panel_UpdateCriticalHullShieldWarning(void);

/* 0x41B430 */
void panel_updatereplaystuff(void);

/* 0x41B630 */
void panel_UpdateCraftSystemStatusIndicators(void);

/* 0x41B6B0 */
void panel_updatecockpitdamage(void);

/* 0x41B780 */
void panel_DrawSpriteElement(uint16_t elementIdx, uint16_t state);

/* 0x41B7E0 */
void panel_updatelever(uint16_t elementIdx, uint16_t state);

/* 0x41B810 */
void panel_loadpaneldata(void);

/* 0x41B950 */
void panel_loadpanelviewdefs(const char* basePath);

/* 0x41BA10 */
void panel_forcenewviewdir(uint16_t hudViewState);

/* 0x41BA40 */
void panel_dosetnewpilotview(uint16_t hudViewState);

/* 0x41BF00 */
void panel_loadcontrolpanel(const char* lfdName, uint8_t** outEntries, uint16_t entryCount);

/* 0x41C060 */
void panel_tryEMSforpanels(void);

/* 0x41C190 */
void panel_copymaskdata(const uint8_t* encodedMask, uint16_t width, uint16_t height,
						int16_t mirrorHorizontal);

/* 0x41C350 */
void panel_clearmaskdata(uint16_t width, unsigned int height);

/* 0x41C3B0 */
void panel_DrawHorizontalColorSpan(int xStart, int xEnd, int y, uint8_t colorIndex);

/* 0x41C460 */
void panel_DrawObjectBoxCorners(int x, int y, int width, int height, uint8_t colorIndex);

#ifdef __cplusplus
}
#endif

#endif
