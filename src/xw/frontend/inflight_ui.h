#ifndef XW_FRONTEND_INFLIGHT_UI_H
#define XW_FRONTEND_INFLIGHT_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/landru_config.h"

#include <landru/btnpush.h>
#include <landru/input.h>
#include <landru/memhdl.h>
#include <landru/rect.h>
#include <landru/res.h>
#include <landru/sound.h>
#include <stddef.h>
#include <stdint.h>
#include <xw/frontend/shell.h>

struct XwShellContext;

enum {
	INFLIGHT_WIDTH = 640,
	INFLIGHT_HEIGHT = 480,
	INFLIGHT_INITIAL_ZOOM_LEVEL = 10,
	INFLIGHT_INITIAL_MAP_SCALE = 16,
	INFLIGHT_INITIAL_FOCUS = 8,
	INFLIGHT_ICON_RESOURCE_NAME_CAPACITY = 20,
	INFLIGHT_ICON_RESOURCE_COUNT = 3,
	INFLIGHT_ALIGN_NEAR = 0,
	INFLIGHT_ALIGN_FAR = 2,
	INFLIGHT_VIEWPORT_LEFT = 25,
	INFLIGHT_VIEWPORT_TOP = 71,
	INFLIGHT_VIEWPORT_RIGHT = 446,
	INFLIGHT_VIEWPORT_BOTTOM = 398,
	INFLIGHT_TITLE_LEFT = 120,
	INFLIGHT_TITLE_TOP = 10,
	INFLIGHT_TITLE_RIGHT = 380,
	INFLIGHT_TITLE_BOTTOM = 30,
	INFLIGHT_PREVIOUS_VIEW_LEFT = 95,
	INFLIGHT_PREVIOUS_VIEW_TOP = 10,
	INFLIGHT_PREVIOUS_VIEW_RIGHT = 115,
	INFLIGHT_PREVIOUS_VIEW_BOTTOM = 30,
	INFLIGHT_NEXT_VIEW_LEFT = 385,
	INFLIGHT_NEXT_VIEW_TOP = 10,
	INFLIGHT_NEXT_VIEW_RIGHT = 405,
	INFLIGHT_NEXT_VIEW_BOTTOM = 30,
	INFLIGHT_EXIT_LEFT = 505,
	INFLIGHT_EXIT_TOP = 450,
	INFLIGHT_EXIT_RIGHT = 625,
	INFLIGHT_EXIT_BOTTOM = 470,
	INFLIGHT_PANEL_LEFT = 491,
	INFLIGHT_PANEL_TOP = 95,
	INFLIGHT_PANEL_RIGHT = 628,
	INFLIGHT_PANEL_BOTTOM = 453,
	INFLIGHT_SHIP_DETAILS_LEFT = 492,
	INFLIGHT_SHIP_DETAILS_TOP = 96,
	INFLIGHT_SHIP_DETAILS_RIGHT = 626,
	INFLIGHT_SHIP_DETAILS_BOTTOM = 203,
	INFLIGHT_PAN_UP_LEFT = 532,
	INFLIGHT_PAN_UP_TOP = 265,
	INFLIGHT_PAN_UP_RIGHT = 596,
	INFLIGHT_PAN_UP_BOTTOM = 285,
	INFLIGHT_PAN_LEFT_LEFT = 498,
	INFLIGHT_PAN_LEFT_TOP = 288,
	INFLIGHT_PAN_LEFT_RIGHT = 562,
	INFLIGHT_PAN_LEFT_BOTTOM = 308,
	INFLIGHT_PAN_RIGHT_LEFT = 566,
	INFLIGHT_PAN_RIGHT_TOP = 288,
	INFLIGHT_PAN_RIGHT_RIGHT = 626,
	INFLIGHT_PAN_RIGHT_BOTTOM = 308,
	INFLIGHT_PAN_DOWN_LEFT = 532,
	INFLIGHT_PAN_DOWN_TOP = 312,
	INFLIGHT_PAN_DOWN_RIGHT = 596,
	INFLIGHT_PAN_DOWN_BOTTOM = 332,
	INFLIGHT_ZOOM_LEFT = 532,
	INFLIGHT_ZOOM_TOP = 352,
	INFLIGHT_ZOOM_RIGHT = 596,
	INFLIGHT_ZOOM_BOTTOM = 372,
	INFLIGHT_SELECTION_LEFT = 120,
	INFLIGHT_SELECTION_TOP = 13,
	INFLIGHT_SELECTION_RIGHT = 380,
	INFLIGHT_SELECTION_BOTTOM = 33,
	INFLIGHT_PREVIOUS_ITEM_LEFT = 95,
	INFLIGHT_PREVIOUS_ITEM_TOP = 13,
	INFLIGHT_PREVIOUS_ITEM_RIGHT = 115,
	INFLIGHT_PREVIOUS_ITEM_BOTTOM = 33,
	INFLIGHT_NEXT_ITEM_LEFT = 385,
	INFLIGHT_NEXT_ITEM_TOP = 13,
	INFLIGHT_NEXT_ITEM_RIGHT = 405,
	INFLIGHT_NEXT_ITEM_BOTTOM = 33
};
struct BriefingRuntimeState;

typedef struct XwInflightMusicState XwInflightMusicState;

enum { INFLIGHT_MUSIC_CLOSE_DURATION = 300, INFLIGHT_MUSIC_TOUR_CLOSE_DURATION = 200 };

enum {
	INFLIGHT_MUSIC_OPEN_VOLUME = 95,
	INFLIGHT_MUSIC_OPEN_DURATION = 300,
	INFLIGHT_MUSIC_INITIAL_LEVEL = 128,
	INFLIGHT_MUSIC_PREVIOUS_GROUP = 1,
	INFLIGHT_MUSIC_MISSION_CHANNEL = 8,
	INFLIGHT_MUSIC_TRANSITION_MARKER = 2
};

/* Original IDB size: 16 bytes. */
struct XwInflightMusicState {
	/* IDB +0x0 */
	int legacyLevel;
	/* IDB +0x4 */
	Sound* music;
	/* IDB +0x8 */
	Sound* previousMusic;
	/* IDB +0xC: Stored but never read; options callers supply a persistent -1 sentinel; map callers supply
	 * zero. */
	int unusedSceneContext;
};

typedef int16_t XwInflightViewMode;

enum XwInflightViewModeValues {
	INFLIGHT_VIEW_MAP = 0x0,
	INFLIGHT_VIEW_BRIEFING = 0x1,
	INFLIGHT_VIEW_DAMAGE_CONTROL = 0x2,
	INFLIGHT_VIEW_COUNT = 3
};

enum {
	INFLIGHT_MAP_FOCUS_ROWS = 6,
	INFLIGHT_MAP_FOCUS_COLUMNS = 6,
	INFLIGHT_MAP_FOCUS_COUNT = INFLIGHT_MAP_FOCUS_ROWS * INFLIGHT_MAP_FOCUS_COLUMNS,
	INFLIGHT_BRIEFING_FOCUS_ROWS = 2,
	INFLIGHT_BRIEFING_FOCUS_COLUMNS = 4,
	INFLIGHT_BRIEFING_FOCUS_COUNT = INFLIGHT_BRIEFING_FOCUS_ROWS * INFLIGHT_BRIEFING_FOCUS_COLUMNS,
	INFLIGHT_DAMAGE_FOCUS_ROWS = 10,
	INFLIGHT_DAMAGE_FOCUS_COLUMNS = 4,
	INFLIGHT_DAMAGE_FOCUS_COUNT = INFLIGHT_DAMAGE_FOCUS_ROWS * INFLIGHT_DAMAGE_FOCUS_COLUMNS
};

enum {
	INFLIGHT_BUTTON_PREVIOUS_VIEW = 1,
	INFLIGHT_BUTTON_NEXT_VIEW = 2,
	INFLIGHT_BUTTON_PREVIOUS_ITEM = 4,
	INFLIGHT_BUTTON_NEXT_ITEM = 5,
	INFLIGHT_BUTTON_PAN_UP = 6,
	INFLIGHT_BUTTON_PAN_LEFT = 7,
	INFLIGHT_BUTTON_PAN_RIGHT = 8,
	INFLIGHT_BUTTON_PAN_DOWN = 9,
	INFLIGHT_BUTTON_ZOOM = 10,
	INFLIGHT_BUTTON_RESUME = 13,
	INFLIGHT_BUTTON_REPAIR_EARLIER = 14,
	INFLIGHT_BUTTON_REPAIR_LATER = 15,
	INFLIGHT_MAP_PAN_STEP = 512,
	INFLIGHT_MAP_FAST_PAN_STEP = 2048,
	INFLIGHT_MAP_ZOOM_SCALE_COUNT = 32,
	INFLIGHT_MAP_MAX_ZOOM_LEVEL = 29
};

enum {
	INFLIGHT_CRAFT_STATUS_COUNT = 9,
	INFLIGHT_CRAFT_STATUS_CAPACITY = 9,
	INFLIGHT_CRAFT_ABBREVIATION_COUNT = 18,
	INFLIGHT_CRAFT_ABBREVIATION_CAPACITY = 4,
	INFLIGHT_DETAIL_TEXT_CAPACITY = 32,
	INFLIGHT_DETAIL_STATUS_CAPACITY = 12,
	INFLIGHT_DETAIL_BACKGROUND = 18,
	INFLIGHT_DETAIL_LAST_SHIP_IMAGE = 25,
	INFLIGHT_DETAIL_HIDDEN_ID_FIRST = 8,
	INFLIGHT_DETAIL_HIDDEN_ID_LAST = 16,
	INFLIGHT_DETAIL_LINE_HEIGHT = 14,
	INFLIGHT_DETAIL_FALLBACK_Y = 7,
	INFLIGHT_DETAIL_TEXT_INSET = 2,
	INFLIGHT_DETAIL_STATUS_COLOR = 14,
	INFLIGHT_DISTANCE_FRACTION_SCALE = 100,
	INFLIGHT_DISTANCE_LEADING_ZERO_LIMIT = 10,
	INFLIGHT_DETAIL_DISTANCE_COLOR_INDEX = 1,
	INFLIGHT_DETAIL_CARGO_COLOR_INDEX = 2
};

extern char g_inflightViewTitles[INFLIGHT_VIEW_COUNT][INFLIGHT_DETAIL_TEXT_CAPACITY];
extern char g_inflightCraftStatusText[INFLIGHT_CRAFT_STATUS_COUNT][INFLIGHT_CRAFT_STATUS_CAPACITY];
extern char g_inflightCraftAbbreviations[INFLIGHT_CRAFT_ABBREVIATION_COUNT]
										[INFLIGHT_CRAFT_ABBREVIATION_CAPACITY];
extern Actor* g_inflightShipDetailsActor;

extern int16_t g_inflightMapZoomScales[INFLIGHT_MAP_ZOOM_SCALE_COUNT];
extern Input* g_inflightPanelInputs[INFLIGHT_VIEW_COUNT];

enum { INFLIGHT_SELECTION_IDLE = 0, INFLIGHT_SELECTION_CLEAR = -1 };

enum {
	INFLIGHT_LABEL_TITLE = 0,
	INFLIGHT_LABEL_CENTER_SELECTED = 3,
	INFLIGHT_LABEL_SHIP_DETAILS = 12,
	INFLIGHT_LABEL_COLOR = 15,
	INFLIGHT_LABEL_ARROW_LEFT = 1,
	INFLIGHT_LABEL_ARROW_RIGHT = 3,
	INFLIGHT_MOUSE_PRESS = 1,
	INFLIGHT_MOUSE_HOLD = 2,
	INFLIGHT_MOUSE_RELEASE = 3,
	INFLIGHT_BUTTON_REPEAT_DELAY = 4,
	INFLIGHT_MAP_WORLD_UNITS_PER_PIXEL = 256,
	INFLIGHT_MAP_PAN_MOUSE_X = 235,
	INFLIGHT_MAP_PAN_MOUSE_Y = 234,
	INFLIGHT_REPAIR_ROWS_TOP = 24,
	INFLIGHT_REPAIR_ROWS_BOTTOM = 200,
	INFLIGHT_REPAIR_ROW_HEIGHT = 22
};

extern int16_t g_inflightMapFocusX[INFLIGHT_MAP_FOCUS_COUNT];
extern int16_t g_inflightMapFocusY[INFLIGHT_MAP_FOCUS_COUNT];
extern int16_t g_inflightBriefingFocusX[INFLIGHT_BRIEFING_FOCUS_COUNT];
extern int16_t g_inflightBriefingFocusY[INFLIGHT_BRIEFING_FOCUS_COUNT];
extern int16_t g_inflightDamageFocusX[INFLIGHT_DAMAGE_FOCUS_COUNT];
extern int16_t g_inflightDamageFocusY[INFLIGHT_DAMAGE_FOCUS_COUNT];
extern Input* g_inflightShipDetailsInput;
extern Input* g_inflightRootInput;
extern Actor* g_inflightBackgroundActor;
extern int g_inflightUIMusicContext;
extern int16_t g_inflightMapPreferencesInitialized;
extern char g_inflightIconResourceNames[INFLIGHT_ICON_RESOURCE_COUNT][INFLIGHT_ICON_RESOURCE_NAME_CAPACITY];
extern XwInflightMusicState g_inflightMusicState;
extern XwInflightViewMode g_inflightViewMode;
extern Input* g_inflightViewportInput;
extern int16_t g_inflightMapScaleY;
extern int16_t g_inflightMapScaleX;
extern int16_t g_inflightMapSelectedShipIndex;
extern LandruHandle g_inflightMapStateHandle;
extern int16_t g_inflightRepairDragActive;
extern int16_t g_inflightShipDetailsIndexPlusOne;
extern int16_t g_inflightMapZoomLevel;
extern int16_t g_inflightMapCenterX;
extern int16_t g_inflightMapCenterY;
extern struct BriefingRuntimeState* g_inflightMapState;
extern int16_t g_inflightRepairSelectionIndex;
extern int16_t g_inflightFocusIndex;

/* Declarations follow ascending original IDB address. */

/* 0x44DF60 */
void InflightUI_OpenMusic(ResFile* unusedResourceFile, int unusedSceneContext);

/* 0x44E240 */
void InflightUI_CloseMusic(void);

/* 0x44E3C0 */
void InflightUI_user_Music(Sound* sound, int time);

/* 0x4501E0 */
XwShellSceneResult InflightUI_Show(struct XwShellContext* context);

/* 0x450BA0 */
void InflightUI_UpdateView(int time);

/* 0x450CC0 */
void InflightUI_RelockState(int16_t relock);

/* 0x450CF0 */
int16_t InflightUI_UpdateViewport(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
								  int rightEvent, int16_t x, int16_t y);

/* 0x450F80 */
void InflightUI_ApplyShipSelection(Input* input);

/* 0x450FD0 */
void InflightUI_DrawViewport(Input* input, Rect* frame, Rect* clip, int16_t refresh);

/* 0x451030 */
int16_t InflightUI_UpdateSelectionLabel(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
										int rightEvent, int16_t x, int16_t y);

/* 0x4510D0 */
void InflightUI_HandleButton(Input* input);

/* 0x451550 */
void InflightUI_DrawLabel(Input* input, Rect* frame, Rect* clip, int16_t refresh);

/* 0x451750 */
int16_t InflightUI_UpdateRepeatingButton(PushButton* button, Rect* frame, Rect* clip, int16_t key,
										 int leftEvent, int rightEvent, int16_t x, int16_t y);

/* 0x451840 */
void InflightUI_DrawTextButton(PushButton* button, Rect* frame, Rect* clip, int16_t refresh);

/* 0x451880 */
void InflightUI_DrawShipDetails(Rect* frame, Rect* clip);

/* 0x451D70 */
void InflightUI_SelectBriefingPage(int16_t scriptIndex);

#ifdef __cplusplus
}
#endif

#endif
