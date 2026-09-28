#ifndef XW_FRONTEND_BRIEF_H
#define XW_FRONTEND_BRIEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/flight/object/craft.h"
#include "xw/landru_config.h"

#include "xw/assets/file.h"
#include <landru/actor.h>
#include <landru/btnpush.h>
#include <landru/film.h>
#include <landru/input.h>
#include <landru/rect.h>
#include <landru/res.h>
#include <landru/sound.h>
#include <stddef.h>
#include <stdint.h>
#include <xw/frontend/shell.h>

struct XwShellContext;
struct REGISTER_PilotFileRecord;

enum {
	XW_BRIEF_PAGE_CLICK_EVENT = 1,
	XW_BRIEF_LABEL_BUFFER_SIZE = 64,
	XW_BRIEF_TOUR_RESOURCE_NAME_CAPACITY = 16,
	XW_BRIEF_RUNTIME_MAP_SCALE = 20,
	XW_BRIEF_DEFAULT_MAP_RIGHT = 420,
	XW_BRIEF_DEFAULT_MAP_BOTTOM = 330,
	XW_BRIEF_COMMAND_COUNT = 42,
	XW_BRIEF_COMMAND_END = 41,
	XW_BRIEF_DEFAULT_END_TIME = 200,
	XW_BRIEF_SENTINEL_TIME = 9999,
	XW_BRIEF_COMMAND_HEADER_WORDS = 2,
	XW_BRIEF_COMMAND_ARGUMENT_CAPACITY = 4,
	XW_BRIEF_INITIALIZED_AGE = 64,
	XW_BRIEF_INITIAL_MAP_SCALE = 16,
	XW_BRIEF_NARRATION_TEXT_SLOT = 1,
	XW_BRIEF_COMMAND_PAUSE = 1,
	XW_BRIEF_COMMAND_CLEAR_TEXT = 10,
	XW_BRIEF_COMMAND_TEXT_FIRST = 11,
	XW_BRIEF_COMMAND_TEXT_LAST = 14,
	XW_BRIEF_COMMAND_CENTER = 15,
	XW_BRIEF_COMMAND_SCALE = 16,
	XW_BRIEF_COMMAND_CLEAR_MARKERS = 21,
	XW_BRIEF_COMMAND_MARKER_FIRST = 22,
	XW_BRIEF_COMMAND_MARKER_LAST = 25,
	XW_BRIEF_COMMAND_CLEAR_LABELS = 26,
	XW_BRIEF_COMMAND_LABEL_FIRST = 27,
	XW_BRIEF_COMMAND_LABEL_LAST = 30,
	XW_BRIEF_COMMAND_CLEAR_FIELD_37DC = 31,
	XW_BRIEF_COMMAND_SET_FIELD_37DC = 32
};

enum {
	XW_BRIEFING_FILM_COUNT = 2,
	XW_BRIEFING_RESOURCE_NAME_CAPACITY = 16,
	XW_BRIEFING_OFFICER_BUFFER_CAPACITY = 26624,
	XW_BRIEFING_NO_MISSION_CHOICE = 255,
	XW_BRIEFING_INITIAL_FOCUS = 9,
	XW_BRIEFING_INITIAL_MOUSE_X = 298,
	XW_BRIEFING_INITIAL_MOUSE_Y = 104,
	XW_BRIEFING_CANVAS_WIDTH = 640,
	XW_BRIEFING_CANVAS_HEIGHT = 480,
	XW_BRIEFING_BWING_TOUR = 4,
	XW_BRIEFING_MAP_Z = 90,
	XW_BRIEFING_RESTORE_Z = 200,
	XW_BRIEFING_MAP_LEFT = 135,
	XW_BRIEFING_MAP_TOP = 22,
	XW_BRIEFING_MAP_RIGHT = 555,
	XW_BRIEFING_MAP_BOTTOM = 352,
	XW_BRIEFING_DODONNA_LEFT = 38,
	XW_BRIEFING_DODONNA_TOP = 172,
	XW_BRIEFING_DODONNA_RIGHT = 207,
	XW_BRIEFING_DODONNA_BOTTOM = 323,
	XW_BRIEFING_ACKBAR_LEFT = 65,
	XW_BRIEFING_ACKBAR_TOP = 180,
	XW_BRIEFING_ACKBAR_RIGHT = 170,
	XW_BRIEFING_ACKBAR_BOTTOM = 320,
	XW_BRIEFING_LEFT_DOOR_LEFT = 0,
	XW_BRIEFING_LEFT_DOOR_TOP = 100,
	XW_BRIEFING_LEFT_DOOR_RIGHT = 100,
	XW_BRIEFING_LEFT_DOOR_BOTTOM = 350,
	XW_BRIEFING_RIGHT_DOOR_LEFT = 534,
	XW_BRIEFING_RIGHT_DOOR_TOP = 100,
	XW_BRIEFING_RIGHT_DOOR_RIGHT = 640,
	XW_BRIEFING_RIGHT_DOOR_BOTTOM = 350,
	XW_BRIEFING_PAGE_LABEL_LEFT = 270,
	XW_BRIEFING_PAGE_LABEL_TOP = 391,
	XW_BRIEFING_PAGE_LABEL_RIGHT = 384,
	XW_BRIEFING_PAGE_LABEL_BOTTOM = 403,
	XW_BRIEFING_REWIND_LEFT = 269,
	XW_BRIEFING_REWIND_TOP = 363,
	XW_BRIEFING_REWIND_RIGHT = 294,
	XW_BRIEFING_REWIND_BOTTOM = 388,
	XW_BRIEFING_STOP_LEFT = 301,
	XW_BRIEFING_STOP_TOP = 362,
	XW_BRIEFING_STOP_RIGHT = 350,
	XW_BRIEFING_STOP_BOTTOM = 388,
	XW_BRIEFING_PLAY_LEFT = 356,
	XW_BRIEFING_PLAY_TOP = 362,
	XW_BRIEFING_PLAY_RIGHT = 379,
	XW_BRIEFING_PLAY_BOTTOM = 388,
	XW_BRIEFING_PREVIOUS_LEFT = 223,
	XW_BRIEFING_PREVIOUS_TOP = 387,
	XW_BRIEFING_PREVIOUS_RIGHT = 261,
	XW_BRIEFING_PREVIOUS_BOTTOM = 416,
	XW_BRIEFING_NEXT_LEFT = 397,
	XW_BRIEFING_NEXT_TOP = 387,
	XW_BRIEFING_NEXT_RIGHT = 435,
	XW_BRIEFING_NEXT_BOTTOM = 416,
	XW_BRIEFING_RIGHT_AUX_LEFT = 337,
	XW_BRIEFING_RIGHT_AUX_TOP = 411,
	XW_BRIEFING_RIGHT_AUX_RIGHT = 399,
	XW_BRIEFING_RIGHT_AUX_BOTTOM = 439,
	XW_BRIEFING_LEFT_AUX_LEFT = 259,
	XW_BRIEFING_LEFT_AUX_TOP = 411,
	XW_BRIEFING_LEFT_AUX_RIGHT = 335,
	XW_BRIEFING_LEFT_AUX_BOTTOM = 439
};

extern char g_briefingFilmNames[XW_BRIEFING_FILM_COUNT][XW_BRIEFING_RESOURCE_NAME_CAPACITY];
extern Actor* g_briefingAckbarActor;
extern Actor* g_briefingDodonnaActor;
extern Input* g_briefingRightDoorInput;
extern Input* g_briefingMapInput;
extern Actor* g_briefingBwingRadarActor;
extern Input* g_briefingLeftDoorInput;
extern Actor* g_briefingBackgroundActor;
extern Actor* g_briefingOfficerRestoreActor;

extern int16_t g_briefingScriptOpcodeArgCounts[XW_BRIEF_COMMAND_COUNT];

enum {
	XW_BRIEFING_ICON_CAPACITY = 40,
	XW_BRIEFING_POSITION_SET_CAPACITY = 4,
	XW_BRIEFING_ICON_BLOCK_SIZE = 16
};

enum {
	XW_BRIEFING_MAP_ICON_ACTOR_COUNT = 3,
	XW_BRIEFING_MAP_ICON_BASE_COUNT = 25,
	XW_BRIEFING_MAP_ICON_LOW_ZOOM_THRESHOLD = 32,
	XW_BRIEFING_MAP_ICON_GROUP0_LOW_ZOOM_OFFSET = 108,
	XW_BRIEFING_MARKER_RADIUS = 8
};

extern int16_t g_briefingMapDefaultIconColorGroups[XW_BRIEFING_MAP_ICON_BASE_COUNT];
extern char g_briefingIconResourceNames[XW_BRIEFING_MAP_ICON_ACTOR_COUNT][XW_BRIEFING_RESOURCE_NAME_CAPACITY];
extern Actor* g_briefingMapIconActors[XW_BRIEFING_MAP_ICON_ACTOR_COUNT];

enum {
	XW_BRIEFING_PREVIEW_TYPE_COUNT = 22,
	XW_BRIEFING_TYPE_LABEL_SIZE = 4,
	XW_BRIEFING_SELECTION_COLOR_COUNT = 3,
	XW_BRIEFING_SELECTION_LABEL_SIZE = 32,
	XW_BRIEFING_SELECTION_WIDTH = 136,
	XW_BRIEFING_SELECTION_HEIGHT = 84,
	XW_BRIEFING_SELECTION_MOUSE_LIMIT = 140,
	XW_BRIEFING_SELECTION_SHIFT = 142,
	XW_BRIEFING_SELECTION_PREVIEW_Y = 7,
	XW_BRIEFING_SELECTION_LABEL_X = 2,
	XW_BRIEFING_TEXT_INSET_X = 8,
	XW_BRIEFING_TEXT_INSET_Y = 4,
	XW_BRIEFING_TEXT_BACKGROUND_COLOR = 1,
	XW_BRIEFING_ICON_MINE_FIRST = 18,
	XW_BRIEFING_ICON_MINE_LAST = 21,
	XW_BRIEFING_ICON_COMMUNICATION = 22,
	XW_BRIEFING_ICON_BWING = 25,
	XW_BRIEFING_PREVIEW_BWING = 22,
	XW_BRIEFING_MINE_VARIANT_COUNT = 3,
	XW_BRIEFING_CONCEALED_TYPE_FIRST = 8,
	XW_BRIEFING_CONCEALED_TYPE_LAST = 16
};

extern char g_briefingSelectionTypeLabels[XW_BRIEFING_PREVIEW_TYPE_COUNT][XW_BRIEFING_TYPE_LABEL_SIZE];
extern int16_t g_briefingSelectionTextColors[XW_BRIEFING_SELECTION_COLOR_COUNT];
extern Actor* g_briefingSelectionPreviewActor;

enum {
	XW_BRIEFING_LAYOUT_COUNT = 4,
	XW_BRIEFING_LAYOUT_VIEWPORT_COUNT = 5,
	XW_BRIEFING_LAYOUT_MAP_VIEWPORT = 4
};

enum {
	XW_BRIEFING_LAYOUT_SCALE = 2,
	XW_BRIEFING_TEXT_MINIMUM_HEIGHT = 40,
	XW_BRIEFING_VIEWPORT_BOTTOM = 352
};

enum {
	XW_BRIEFING_NARRATION_BUFFER_SIZE = 0xFFF0,
	XW_BRIEFING_NARRATION_PATH_CAPACITY = 256,
	XW_BRIEFING_NARRATION_PRIORITY = 127,
	XW_BRIEFING_TWO_DIGIT_PAGE = 10
};

enum {
	XW_BRIEFING_PAGE_LABEL_CAPACITY = 32,
	XW_BRIEFING_TEXT_BUFFER_CAPACITY = 32,
	XW_BRIEFING_TEXT_BUFFER_SIZE = 512,
	XW_BRIEFING_PAGE_BACKGROUND_COLOR = 16,
	XW_BRIEFING_PAGE_TEXT_COLOR = 15,
	XW_BRIEFING_PAGE_FONT = 0
};

enum {
	XW_LAUNCH_GROUP_CAPACITY = 16,
	XW_LAUNCH_GROUP_NAME_CAPACITY = 16,
	XW_LAUNCH_IFF_LIMIT = 2,
	XW_BRIEFING_MISSION_PATH_CAPACITY = 256,
	XW_LAUNCH_CRAFTS_PER_GROUP = 6,
	XW_LOCAL_PILOT_ASSIGNMENT_TOKEN = 32000
};

enum { XW_BRIEFING_MAP_BACKGROUND_COLOR = 24, XW_BRIEFING_STARS_BACKGROUND_COLOR = 16 };

enum { XW_BRIEFING_MAP_COORDINATE_SCALE = 256 };

enum {
	XW_BRIEFING_GRID_DARK_MAJOR_COLOR = 40,
	XW_BRIEFING_GRID_DARK_MINOR_COLOR = 28,
	XW_BRIEFING_GRID_LIGHT_MAJOR_COLOR = 62,
	XW_BRIEFING_GRID_LIGHT_MINOR_COLOR = 63,
	XW_BRIEFING_GRID_HALF_DETAIL_SCALE = 16,
	XW_BRIEFING_GRID_FULL_DETAIL_SCALE = 32,
	XW_BRIEFING_GRID_MAJOR_INTERVAL = 4,
	XW_BRIEFING_GRID_CELL_FRACTION_BITS = 8,
	XW_BRIEFING_GRID_CELL_FRACTION_MASK = (1 << XW_BRIEFING_GRID_CELL_FRACTION_BITS) - 1
};

enum {
	XW_BRIEFING_MAP_SCALE_STEP = 2,
	XW_BRIEFING_MAP_FAST_SCALE_STEP = 8,
	XW_BRIEFING_MAP_FAST_SCALE_DISTANCE = 12,
	XW_BRIEFING_MAP_FINE_SCALE_LIMIT = 10,
	XW_BRIEFING_MAP_FINE_SCALE_STEP = 1,
	XW_BRIEFING_MAP_CENTER_STEP_MULTIPLIER = 2,
	XW_BRIEFING_MAP_FAST_CENTER_DISTANCE = 16
};

enum {
	XW_BRIEFING_FOCUS_COLUMNS = 5,
	XW_BRIEFING_FOCUS_ROWS = 2,
	XW_BRIEFING_CHOICE_FOCUS_ROWS = 3,
	XW_BRIEFING_FOCUS_CAPACITY = 12,
	XW_BRIEFING_CHOICE_FOCUS_CAPACITY = 16
};

enum {
	XW_BRIEFING_LAST_SHIP_ICON_TYPE = 25,
	XW_BRIEFING_INITIAL_PICK_DISTANCE = 999,
	XW_BRIEFING_PICK_DISTANCE_LIMIT = 16
};

enum {
	XW_BRIEF_HIGHLIGHT_SOUND_PHASE = 2,
	XW_BRIEF_HIGHLIGHT_MOVE_PHASE = 3,
	XW_BRIEF_HIGHLIGHT_SHRINK_PHASE = 9,
	XW_BRIEF_HIGHLIGHT_LAST_OUTLINE_PHASE = 12,
	XW_BRIEF_HIGHLIGHT_FILLED_PHASE = 13,
	XW_BRIEF_HIGHLIGHT_LAST_BLINK_PHASE = 18,
	XW_BRIEF_HIGHLIGHT_INITIAL_OUTSET = 18,
	XW_BRIEF_HIGHLIGHT_RECT_STEP = 2,
	XW_BRIEF_HIGHLIGHT_FRAME_COUNT = 4,
	XW_BRIEF_HIGHLIGHT_COLOR_STEP = 8,
	XW_BRIEF_HIGHLIGHT_INITIAL_COLOR = 47,
	XW_BRIEF_HIGHLIGHT_OUTLINE_COLOR = 23,
	XW_BRIEF_HIGHLIGHT_FILL_COLOR = 52,
	XW_BRIEF_HIGHLIGHT_FRAME_COLOR = 54
};

enum {
	XW_BRIEF_READOUT_CAPACITY = 64,
	XW_BRIEF_READOUT_DOUBLE_RATE = 2,
	XW_BRIEF_READOUT_RAMP_STEPS = 3,
	XW_BRIEF_READOUT_FIRST_COLOR = 48,
	XW_BRIEF_READOUT_FINAL_COLOR = 51,
	XW_BRIEF_READOUT_SETTLE_STEPS = 2,
	XW_BRIEF_READOUT_CURSOR_GAP = 2,
	XW_BRIEF_READOUT_CURSOR_SIZE = 6
};

enum { XW_BRIEFING_PILOT_PATH_CAPACITY = 256 };

enum { XW_BRIEFING_PILOT_FILENAME_CAPACITY = 32 };

enum {
	XW_BRIEF_DOOR_ABORT = 0,
	XW_BRIEF_DOOR_ACTIVATE_EVENT = 3,
	XW_BRIEF_DOOR_ACTION_EXIT = 1,
	XW_BRIEF_DOOR_ACTION_HOVER = 2
};

enum {
	XW_BRIEF_DOOR_HINT_CAPACITY = 16,
	XW_BRIEF_DOOR_HINT_BACKGROUND = 16,
	XW_BRIEF_DOOR_HINT_COLOR = 15,
	XW_BRIEF_DOOR_HINT_FONT = 2,
	XW_BRIEF_DOOR_HINT_OFFSET = -1
};

enum {
	XW_BRIEF_BUTTON_PREVIOUS_PAGE = 0,
	XW_BRIEF_BUTTON_NEXT_PAGE = 1,
	XW_BRIEF_BUTTON_REWIND = 2,
	XW_BRIEF_BUTTON_STOP = 3,
	XW_BRIEF_BUTTON_PLAY = 4,
	XW_BRIEF_BUTTON_MISSION_A = 5,
	XW_BRIEF_BUTTON_MISSION_B = 6,
	XW_BRIEF_BUTTON_LOG = 5,
	XW_BRIEF_BUTTON_MEDALS = 6,
	XW_BRIEF_BUTTON_LOG_ALTERNATE = 7,
	XW_BRIEF_BUTTON_MEDALS_ALTERNATE = 8,
	XW_BRIEF_BUTTON_ACTOR_X = -85,
	XW_BRIEF_BUTTON_ACTOR_Y = -2,
	XW_BRIEF_BUTTON_LABEL_Y = -4
};

/* Original IDB size: 4 bytes. */
struct XwOfficerAnimationStep {
	/* IDB +0x0: Nonzero frames are skipped while narration is inactive or SFX is disabled/volume zero. */
	int16_t skipWithoutNarration;
	/* IDB +0x2: Next frame while a narration instance is playing, stored one-based; consumer subtracts 1. */
	int16_t narrationNextFramePlusOne;
};

enum {
	XW_BRIEFING_OFFICER_SCRIPT = 0,
	XW_BRIEFING_ACKBAR_FRAME_COUNT = 45,
	XW_BRIEFING_DODONNA_FRAME_COUNT = 55
};

extern struct XwOfficerAnimationStep g_briefingDodonnaAnimationSteps[XW_BRIEFING_DODONNA_FRAME_COUNT];
extern struct XwOfficerAnimationStep g_briefingAckbarAnimationSteps[XW_BRIEFING_ACKBAR_FRAME_COUNT];
extern int16_t g_briefingFocusX[XW_BRIEFING_FOCUS_CAPACITY];
extern int16_t g_briefingFocusY[XW_BRIEFING_FOCUS_CAPACITY];
extern int16_t g_briefingMissionChoiceFocusX[XW_BRIEFING_CHOICE_FOCUS_CAPACITY];
extern int16_t g_briefingMissionChoiceFocusY[XW_BRIEFING_CHOICE_FOCUS_CAPACITY];
extern int16_t g_localPilotAssignmentToken;

enum {
	BRIEF_MUSIC_OPEN_VOLUME = 95,
	BRIEF_MUSIC_OPEN_DURATION = 300,
	BRIEF_MUSIC_INITIAL_LEVEL = 128,
	BRIEF_MUSIC_PREVIOUS_GROUP = 1,
	BRIEF_MUSIC_MISSION_CHANNEL = 8,
	BRIEF_MUSIC_TRANSITION_MARKER = 2
};

extern struct XwBriefingMusicState g_briefMusicState;
extern int16_t g_briefingTextSoundActive;
extern int16_t g_briefingTextSoundRequested;
extern int16_t g_launchSelectedGroupIndex;
extern Actor* g_briefingRewindButtonActor;
extern int16_t g_launchCraftPilotTokens[XW_LAUNCH_GROUP_CAPACITY][XW_LAUNCH_CRAFTS_PER_GROUP];
extern Actor* g_briefingPlayButtonActor;
extern char g_launchGroupNames[XW_LAUNCH_GROUP_CAPACITY][XW_LAUNCH_GROUP_NAME_CAPACITY];
extern XwCraftSpecies g_launchGroupCraftTypes[XW_LAUNCH_GROUP_CAPACITY];
extern uint16_t g_launchGroupInitialStatus[XW_LAUNCH_GROUP_CAPACITY];
extern uint16_t g_launchGroupFormations[XW_LAUNCH_GROUP_CAPACITY];
extern uint16_t g_launchGroupCraftCounts[XW_LAUNCH_GROUP_CAPACITY];
extern Actor* g_briefingRightDoorActor;
extern int16_t g_briefingHasMissionChoice;
extern int16_t g_briefingRequestedNarrationPage;
extern uint16_t g_launchGroupMissionIndices[XW_LAUNCH_GROUP_CAPACITY];
extern Actor* g_briefingLogButtonActor;
extern Actor* g_briefingStarsActor;
extern Sound* g_briefingNarrationSound;
extern Film* g_briefingFilm;
extern int16_t g_briefingMissionChoice;
extern int16_t g_launchGroupCount;
extern Rect g_briefingOfficerRect;
extern Actor* g_briefingInteriorActor;
extern Actor* g_briefingMedalsButtonActor;
extern LandruHandle g_briefingRuntimeHandle;
extern Actor* g_briefingNextPageButtonActor;
extern Input* g_briefingWorldInput;
extern Input* g_briefingDoorHintInput;
extern int16_t g_briefingOfficerRegionRestored;
extern int16_t g_launchAssignedPilotCount;
extern Actor* g_briefingLeftDoorActor;
extern int16_t g_briefingLoadedNarrationPage;
extern Actor* g_briefingPreviousPageButtonActor;
extern Input* g_briefingHintCompanionInput;
extern LandruHandle g_briefingOfficerRegionBuffer;
extern Actor* g_briefingTextActor;
extern struct BriefingRuntimeState* g_briefingRuntime;
extern struct REGISTER_PilotFileRecord g_briefingPilotRecord;
extern Actor* g_briefingStopButtonActor;
extern uint16_t g_launchGroupPlayerCraftOrdinals[XW_LAUNCH_GROUP_CAPACITY];
extern int16_t g_briefingSelectedIconPlusOne;
extern int16_t g_briefingFocusIndex;
extern int16_t g_launchSelectedCraftIndex;

typedef struct BriefingRuntimeState BriefingRuntimeState;
typedef struct XwBriefingMusicState XwBriefingMusicState;
typedef struct XwOfficerAnimationStep XwOfficerAnimationStep;

enum {
	XW_BRIEFING_EXTENDED_HEADER_BLOCK_COUNT = 3,
	XW_BRIEFING_EXTENDED_HEADER_BLOCK_SIZE = 64,
	XW_BRIEFING_EXTENDED_ICON_BLOCK_SIZE = 14
};

/* Original IDB size: 14402 bytes. */
struct BriefingRuntimeState {
	/* IDB +0x0 */
	int16_t playbackActive;
	/* IDB +0x2: 32 handles, each allocated as 64 bytes by 0x43C500; labelTextIndex selects a string in the
	 * overlay renderer. */
	LandruHandle labelTextHandles[XW_BRIEFING_PAGE_LABEL_CAPACITY];
	/* IDB +0x42: 32 text buffers of 512 bytes, allocated by 0x43C500, loaded by 0x43D9F0, consumed by
	 * FrontendText_DrawFormattedWrappedText. */
	LandruHandle textBlockHandles[XW_BRIEFING_TEXT_BUFFER_CAPACITY];
	/* IDB +0x82: 32 buffers of 512 attribute bytes parallel to textBlockHandles. Renderer emits color control
	 * bytes 2/1 on transitions to nonzero/zero attributes. */
	LandruHandle textAttributeHandles[XW_BRIEFING_TEXT_BUFFER_CAPACITY];
	/* IDB +0xC2 */
	int16_t field_00C2;
	/* IDB +0xC4: Count read by 0x43D300 and consumed by map renderer and hit testing. Arrays have
	 * capacity 40. */
	int16_t mapIconCount;
	/* IDB +0xC6 */
	int16_t field_00C6;
	/* IDB +0xC8: Number of position sets read by 0x43D300; X/Y arrays use a 40-word stride and have capacity
	 * four. */
	int16_t mapPositionSetCount;
	/* IDB +0xCA */
	int16_t field_00CA;
	/* IDB +0xCC */
	int16_t field_00CC;
	/* IDB +0xCE: File word read by 0x43D610: zero selects palette background 16 / grid 62,63; nonzero selects
	 * 24 / 40,28. */
	int16_t mapColorMode;
	/* IDB +0xD0 */
	int16_t field_00D0;
	/* IDB +0xD2: Three blocks read by the extended icon loaders. */
	uint8_t extendedHeaderBlocks[XW_BRIEFING_EXTENDED_HEADER_BLOCK_COUNT]
								[XW_BRIEFING_EXTENDED_HEADER_BLOCK_SIZE];
	/* IDB +0x192: File icon type. Renderer subtracts one to select an animation state; hit tester accepts
	 * types <=25. */
	int16_t mapIconType[XW_BRIEFING_ICON_CAPACITY];
	/* IDB +0x1E2: Zero uses the type default; nonzero uses value-1, clamped to icon actor group 0..2. */
	int16_t mapIconColorOverride[XW_BRIEFING_ICON_CAPACITY];
	/* IDB +0x232 */
	int16_t field_0232[XW_BRIEFING_ICON_CAPACITY];
	/* IDB +0x282 */
	int16_t field_0282[XW_BRIEFING_ICON_CAPACITY];
	/* IDB +0x2D2: Icon names loaded from BRF records and displayed in the map selection label. */
	char mapIconName[XW_BRIEFING_ICON_CAPACITY][XW_BRIEFING_ICON_BLOCK_SIZE];
	/* IDB +0x552 */
	uint8_t field_0552[XW_BRIEFING_ICON_CAPACITY][XW_BRIEFING_ICON_BLOCK_SIZE];
	/* IDB +0x7D2 */
	uint8_t field_07D2[XW_BRIEFING_ICON_CAPACITY][XW_BRIEFING_ICON_BLOCK_SIZE];
	/* IDB +0xA52 */
	int16_t field_0A52[XW_BRIEFING_ICON_CAPACITY];
	/* IDB +0xAA2: Map X coordinates indexed [positionSet][icon]; 40 words per set, loaded by 0x43D300. */
	int16_t mapIconX[XW_BRIEFING_POSITION_SET_CAPACITY][XW_BRIEFING_ICON_CAPACITY];
	/* IDB +0xBE2: Map Y coordinates indexed [positionSet][icon]; 40 words per set, loaded by 0x43D300. */
	int16_t mapIconY[XW_BRIEFING_POSITION_SET_CAPACITY][XW_BRIEFING_ICON_CAPACITY];
	/* IDB +0xD22 */
	int16_t mapIconZ[XW_BRIEFING_POSITION_SET_CAPACITY][XW_BRIEFING_ICON_CAPACITY];
	/* IDB +0xE62 */
	int16_t field_0E62[XW_BRIEFING_ICON_CAPACITY];
	/* IDB +0xEB2 */
	int16_t field_0EB2[XW_BRIEFING_ICON_CAPACITY];
	/* IDB +0xF02 */
	int16_t field_0F02[XW_BRIEFING_ICON_CAPACITY];
	/* IDB +0xF52 */
	int16_t field_0F52[XW_BRIEFING_ICON_CAPACITY];
	int16_t field_0FA2[XW_BRIEFING_ICON_CAPACITY];
	int16_t field_0FF2[XW_BRIEFING_ICON_CAPACITY];
	int16_t field_1042[XW_BRIEFING_ICON_CAPACITY];
	int16_t field_1092[XW_BRIEFING_ICON_CAPACITY];
	int16_t field_10E2;
	int16_t field_10E4;
	uint8_t field_10E6[XW_BRIEFING_ICON_CAPACITY][XW_BRIEFING_EXTENDED_ICON_BLOCK_SIZE];
	uint8_t field_1316[XW_BRIEFING_ICON_CAPACITY][XW_BRIEFING_EXTENDED_ICON_BLOCK_SIZE];
	uint8_t field_1546[XW_BRIEFING_ICON_CAPACITY][XW_BRIEFING_EXTENDED_ICON_BLOCK_SIZE];
	uint8_t field_1776[XW_BRIEFING_ICON_CAPACITY][XW_BRIEFING_EXTENDED_ICON_BLOCK_SIZE];
	int16_t field_19A6[XW_BRIEFING_ICON_CAPACITY];
	/* IDB +0x19F6: Nonzero selects this icon as the player mission craft in BRIEF_ReadExtendedIconData. Icon
	 * type 25 maps to mission ship 3; other types map to type-1. Second word after the four 14-byte blocks of
	 * each 90-byte extended icon record. */
	int16_t mapIconPlayerShipFlag[40];
	/* IDB +0x1A46 */
	int16_t field_1A46[XW_BRIEFING_ICON_CAPACITY];
	int16_t field_1A96[XW_BRIEFING_ICON_CAPACITY];
	int16_t field_1AE6[XW_BRIEFING_ICON_CAPACITY];
	int16_t field_1B36[XW_BRIEFING_ICON_CAPACITY];
	int16_t field_1B86[XW_BRIEFING_ICON_CAPACITY];
	int16_t field_1BD6[XW_BRIEFING_ICON_CAPACITY];
	int16_t field_1C26[XW_BRIEFING_ICON_CAPACITY];
	int16_t field_1C76[XW_BRIEFING_ICON_CAPACITY];
	int16_t field_1CC6[XW_BRIEFING_ICON_CAPACITY];
	int16_t field_1D16[XW_BRIEFING_ICON_CAPACITY];
	/* IDB +0x1D66: Cleared by player_Init_Display_Map; semantics not yet established. */
	int16_t field_1D66;
	/* IDB +0x1D68: Number of five-viewport layouts. Initialized to 1 by PLAYER_Init_Display_Map; read from
	 * file at 0x453A86 and controls calls to 0x453EB0. Layout storage has capacity four. */
	int16_t layoutCount;
	/* IDB +0x1D6A: Four layouts of five viewports: text slots 0..3, map slot 4. Initialized at 0x452D50 and
	 * applied at 0x453120. */
	Rect layoutRects[XW_BRIEFING_LAYOUT_COUNT][XW_BRIEFING_LAYOUT_VIEWPORT_COUNT];
	/* IDB +0x1E0A: Per-layout viewport enable flags, initialized at 0x452D50 and copied at 0x453120. */
	int16_t layoutActive[XW_BRIEFING_LAYOUT_COUNT][XW_BRIEFING_LAYOUT_VIEWPORT_COUNT];
	/* IDB +0x1E32 */
	int16_t activeScriptIndex;
	/* IDB +0x1E34: Script count read from file by 0x453B00; initialized to 1. Used by 0x451D70 to wrap script
	 * selection. Script arrays have capacity eight. */
	int16_t scriptCount;
	/* IDB +0x1E36: Per-track playback end time; the active callback resets when current time reaches this
	 * value. */
	int16_t scriptEndTime[8];
	/* IDB +0x1E46: Per-track next playback time; reset to zero then advanced by the interpreter. */
	int16_t scriptCurrentTime[8];
	/* IDB +0x1E56: Per-track word cursor into scriptWords; retains the first future command position. */
	int16_t scriptCursorWordIndex[8];
	/* IDB +0x1E66 */
	int16_t scriptWordCount[8];
	/* IDB +0x1E76 */
	int16_t scriptPositionSetIndex[8];
	/* IDB +0x1E86 */
	int16_t scriptLayoutIndex[8];
	/* IDB +0x1E96: Eight tracks of 400 signed words. Commands contain timestamp, opcode, then opcode-specific
	 * arguments. */
	int16_t scriptWords[8][400];
	/* IDB +0x3796: Current signed X/Y map center. AnimateViewState steps toward mapTargetCenter after
	 * updating scale. */
	int16_t mapCenter[2];
	/* IDB +0x379A: Target X/Y center set by script opcode 15; initialization snaps current center too. */
	int16_t mapTargetCenter[2];
	/* IDB +0x379E */
	int16_t mapCenterDirty;
	/* IDB +0x37A0: Current signed X/Y scale. AnimateViewState uses X for 256/X+1 motion base with no zero
	 * guard. Reset initializes both to 16. */
	int16_t mapScale[2];
	/* IDB +0x37A4: Target X/Y scale from twice script opcode 16 arguments; initialization also snaps current
	 * scale. */
	int16_t mapTargetScale[2];
	/* IDB +0x37A8 */
	int16_t mapScaleDirty;
	/* IDB +0x37AA: Set from selected layout by 0x43C970; controls map viewport and 65-pixel inset in text
	 * slot 1. */
	int16_t mapViewportActive;
	/* IDB +0x37AC: Selected map viewport rectangle copied from layout and adjusted by 0x43C970. */
	Rect mapViewportRect;
	/* IDB +0x37B4: Active text viewport enable flags, copied from layoutActive by 0x453120. */
	int16_t textViewportActive[4];
	/* IDB +0x37BC: Active text viewport bounds, copied from selected layout at 0x453120 and read at 0x4525B0.
	 */
	Rect textViewportRects[4];
	/* IDB +0x37DC */
	int16_t field_37DC;
	/* IDB +0x37DE */
	int16_t field_37DE;
	/* IDB +0x37E0 */
	int16_t field_37E0;
	/* IDB +0x37E2 */
	int16_t field_37E2;
	/* IDB +0x37E4 */
	int16_t field_37E4;
	/* IDB +0x37E6 */
	int16_t field_37E6;
	/* IDB +0x37E8 */
	int16_t textSlotActive[4];
	/* IDB +0x37F0 */
	int16_t textSlotBlockIndex[4];
	/* IDB +0x37F8 */
	int16_t textSlotsChanged;
	/* IDB +0x37FA: Four highlight slots: opcodes 22..25 activate, 21 clears. Nonzero gates age increment and
	 * highlight drawing. */
	int16_t fgMarkerActive[4];
	/* IDB +0x3802: Map icon index for each active highlight slot, supplied by opcodes 22..25. */
	int16_t fgMarkerIconIndex[4];
	/* IDB +0x380A: Four wrapping 16-bit animation ages: script activation initializes 0, or 64 during state
	 * rebuild; AnimateViewState increments when active. DrawCraftIconHighlight consumes phase (sound at 2,
	 * settled highlight from 13). */
	int16_t fgMarkerAge[4];
	/* IDB +0x3812 */
	int16_t fgMarkersChanged;
	/* IDB +0x3814 */
	int16_t field_3814;
	/* IDB +0x3816: Four label slots: opcodes 27..30 activate, 26 clears. Nonzero gates age increment and
	 * label drawing. */
	int16_t labelActive[4];
	/* IDB +0x381E */
	int16_t labelTextIndex[4];
	/* IDB +0x3826 */
	int16_t labelX[4];
	/* IDB +0x382E */
	int16_t labelY[4];
	/* IDB +0x3836: Four wrapping 16-bit reveal ages: script activation initializes 0, or 64 during state
	 * rebuild; AnimateViewState increments when active. Passed as revealCount when drawing label. */
	int16_t labelAge[4];
	/* IDB +0x383E */
	int16_t labelsChanged;
	/* IDB +0x3840 */
	int16_t pauseMarkerReached;
};

/* Original IDB size: 16 bytes. */
struct XwBriefingMusicState {
	/* IDB +0x0 */
	int legacyLevel;
	/* IDB +0x4 */
	Sound* music;
	/* IDB +0x8 */
	Sound* previousMusic;
	/* IDB +0xC */
	Film* film;
};

typedef int16_t XwBriefingSoundAction;

enum XwBriefingSoundActionValues {
	XW_BRIEF_SOUND_DOOR_OPEN = 0x1,
	XW_BRIEF_SOUND_DOOR_CLOSE = 0x2,
	XW_BRIEF_SOUND_TARGET = 0x3,
	XW_BRIEF_SOUND_REQUEST_TEXT = 0x4,
	XW_BRIEF_SOUND_STOP_TEXT = 0x5,
	XW_BRIEF_SOUND_TICK = 0x6
};

enum { XW_BRIEFING_MAP_PARAGRAPH_SLOT = 1, XW_BRIEFING_MAP_PARAGRAPH_INSET = 65 };

/* Declarations follow ascending original IDB address. */

/* 0x433E20 */
void brief_OpenMusic(ResFile* unusedResourceFile, Film* film);

/* 0x434120 */
void brief_CloseMusic(void);

/* 0x4341D0 */
void brief_user_Music(Sound* sound, int time);

/* 0x4342A0 */
int16_t brief_LoadUiSounds(void);

/* 0x434300 */
void brief_HandleSoundAction(XwBriefingSoundAction action);

/* 0x439390 */
XwShellSceneResult brief_Brief(struct XwShellContext* context);

/* 0x439EB0 */
void brief_end_View(int time);

/* 0x439FD0 */
void brief_RefreshRuntimePointer(int16_t refresh);

/* 0x43A000 */
int16_t brief_UpdateMapSelection(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
								 int rightEvent, int16_t x, int16_t y);

/* 0x43A080 */
void brief_ApplyMapSelection(Input* input, int time);

/* 0x43A0A0 */
int16_t brief_iupdate_Brief(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent, int rightEvent,
							int16_t x, int16_t y);

/* 0x43A140 */
void brief_iuser_Brief(Input* input, int time);

/* 0x43A340 */
void brief_DrawDoorHint(Input* input, Rect* frame, Rect* clip, int16_t refresh);

/* 0x43A3C0 */
void brief_HandlePlaybackButton(Input* input, int time);

/* 0x43A540 */
void brief_DrawPlaybackButton(PushButton* button, Rect* frame, Rect* clip, int16_t refresh);

/* 0x43A620 */
int16_t brief_UpdatePageLabel(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
							  int rightEvent, int16_t x, int16_t y);

/* 0x43A660 */
void brief_DrawPageLabel(Input* input, Rect* frame, Rect* clip, int16_t refresh);

/* 0x43A6C0 */
void brief_user_Door(Actor* actor, int time);

/* 0x43A750 */
void brief_AdvanceOrResetAtEnd(Actor* actor, int time);

/* 0x43A7B0 */
int16_t brief_DrawMapViewport(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh);

/* 0x43A7E0 */
void brief_UpdateAckbarActor(Actor* actor, int time);

/* 0x43A8F0 */
void brief_UpdateDodonnaActor(Actor* actor, int time);

/* 0x43AA00 */
int16_t brief_RestoreOfficerRegion(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
								   int16_t refresh);

/* 0x43AA20 */
void brief_SelectPage(int16_t pageIndex);

/* 0x43AA70 */
void brief_Rewind_Page(int16_t scriptIndex);

/* 0x43AB70 */
void brief_Step_Page(int16_t scriptIndex, int16_t initializeState);

/* 0x43B0D0 */
int16_t brief_Seek_Page(int16_t scriptIndex, int16_t targetTime, int16_t initializeState);

/* 0x43B140 */
void brief_Seek_Page_Section(int16_t scriptIndex);

/* 0x43B2B0 */
void brief_Reseek_Page(int16_t scriptIndex);

/* 0x43B2F0 */
void brief_Move_Display_Map(void);

/* 0x43B490 */
int16_t brief_DrawViewportAndSelection(const Rect* viewportRect, const Rect* clipRect, int16_t redrawText);

/* 0x43B8E0 */
int16_t brief_Draw_Display_Grid(const Rect* viewportRect, const Rect* clipRect);

/* 0x43BBD0 */
int16_t brief_DrawOverlays(const Rect* viewportRect, const Rect* clipRect);

/* 0x43BEC0 */
void brief_DrawBackground(void);

/* 0x43BF30 */
void brief_Draw_Map_Paragraph(const Rect* rect, LandruHandle textHandle, LandruHandle attributeHandle,
							  int16_t suppressCenteredHeadings, int16_t textSlotIndex);

/* 0x43C230 */
void brief_Draw_Double_Readout_Text(const char* text, uint16_t fontId, int16_t x, int16_t y,
									int16_t revealCount);

/* 0x43C260 */
void brief_Draw_Readout_Text(const char* text, uint16_t fontId, int16_t x, int16_t y, int16_t revealCount);

/* 0x43C3D0 */
void brief_DrawCraftIconHighlight(const Rect* iconRect, int16_t highlightPhase);

/* 0x43C500 */
void brief_InitRuntime(int16_t reuseTextBuffers);

/* 0x43C860 */
void brief_Free_Display_Map(void);

/* 0x43C8D0 */
void brief_InitDefaultScript(int16_t scriptIndex);

/* 0x43C970 */
void brief_ApplyLayout(int16_t layoutIndex);

/* 0x43CB10 */
int brief_Find_Ship_On_Screen(const Rect* viewportRect, int16_t mouseX, int16_t mouseY,
							  int16_t* outIconIndex);

/* 0x43CBF0 */
void brief_Map_To_Screen_Pos(const Rect* viewportRect, int16_t mapX, int16_t mapY, int16_t* outX,
							 int16_t* outY);

/* 0x43CC90 */
int16_t brief_Move_To_Value(int16_t current, int16_t target, int16_t step);

/* 0x43CCC0 */
void brief_SetOfficerRegionRestored(int16_t restore);

/* 0x43CD70 */
int16_t brief_LoadPilotRecord(void);

/* 0x43CE20 */
void brief_WritePilotRecord(void);

/* 0x43CEC0 */
void brief_CommitMissionChoice(void);

/* 0x43CEE0 */
void brief_LoadMissionChoice(int16_t choiceIndex, int16_t reuseTextBuffers);

/* 0x43CFB0 */
int brief_LoadBriefingFile(const char* missionName);

/* 0x43D1E0 */
void brief_ReadScripts(XwFile* stream);

/* 0x43D300 */
void brief_ReadIconData(XwFile* stream);

/* 0x43D590 */
void brief_ReadLayout(XwFile* stream, int16_t layoutIndex);

/* 0x43D610 */
void brief_ReadExtendedIconData(XwFile* stream);

/* 0x43D9F0 */
void brief_ReadTextBuffers(XwFile* stream);

/* 0x43DB80 */
void brief_LoadNarration(void);

/* 0x43DD80 */
int16_t brief_PrepareLaunchAndSavePilot(void);

/* 0x43DE90 */
void brief_BuildLaunchPilotRoster(void);

/* 0x43DFE0 */
void brief_LoadLaunchFlightGroups(void);

/* 0x43E330 */
void brief_AssignLocalPilotToPlayerCraft(void);

#ifdef __cplusplus
}
#endif

#endif
