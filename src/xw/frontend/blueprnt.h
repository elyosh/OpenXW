#ifndef XW_FRONTEND_BLUEPRNT_H
#define XW_FRONTEND_BLUEPRNT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/landru_config.h"

#include <landru/actor.h>
#include <landru/btnpush.h>
#include <landru/film.h>
#include <landru/input.h>
#include <landru/rect.h>
#include <landru/res.h>
#include <landru/sound.h>
#include <stddef.h>
#include <stdint.h>

enum { BLUEPRINT_MUSIC_FADE_DURATION = 300, BLUEPRINT_HALL_MUSIC_CONTROL_VALUE = 3 };

enum {
	BP_SHIP_ANIMATION_COUNT = 18,
	BP_SHIP_ANIMATION_NAME_CAPACITY = 24,
	BP_ANIMATION_WIDTH = 640,
	BP_ANIMATION_HEIGHT = 480
};

enum { BP_HINT_FONT = 0, BP_HINT_SHADOW_OFFSET = 1, BP_HINT_SHADOW_COLOR = 16, BP_HINT_TEXT_COLOR = 15 };

#include <xw/frontend/shell.h>

struct XwShellContext;

enum {
	BP_INITIAL_FOCUS = 2,
	BP_INITIAL_MOUSE_X = 214,
	BP_INITIAL_MOUSE_Y = 170,
	BP_DETAILED_SHIP_COUNT = 8,
	BP_NO_LOADED_SHIP = -1,
	BP_LEFT_DOOR = 0,
	BP_RIGHT_DOOR = 1,
	BP_DOOR_TOP = 45,
	BP_DOOR_BOTTOM = 435,
	BP_LEFT_DOOR_RIGHT = 135,
	BP_RIGHT_DOOR_LEFT = 525,
	BP_LABEL_LEFT = 258,
	BP_SHIP_LABEL_TOP = 410,
	BP_SHIP_LABEL_RIGHT = 430,
	BP_SHIP_LABEL_BOTTOM = 422,
	BP_PREVIOUS_LEFT = 195,
	BP_PREVIOUS_RIGHT = 239,
	BP_NEXT_LEFT = 447,
	BP_NEXT_SHIP_RIGHT = 490,
	BP_SHIP_BUTTON_TOP = 402,
	BP_SHIP_BUTTON_BOTTOM = 437,
	BP_COMPONENT_BUTTON_BOTTOM = 469,
	BP_NEXT_COMPONENT_RIGHT = 491,
	BP_COMPONENT_LABEL_TOP = 442,
	BP_COMPONENT_LABEL_RIGHT = 426,
	BP_COMPONENT_LABEL_BOTTOM = 454,
	BP_COMPONENT_NAME_INPUT = 1
};

enum { BP_FOCUS_ROWS = 2, BP_FOCUS_COLUMNS = 4, BP_FOCUS_COUNT = BP_FOCUS_ROWS * BP_FOCUS_COLUMNS };

enum { BP_COLOR_TABLE_SIZE = 1024, BP_COLOR_RESOURCE_TYPE = 0x5441424C };

enum { BP_PREVIOUS_SHIP = 0, BP_NEXT_SHIP = 1, BP_PREVIOUS_COMPONENT = 2, BP_NEXT_COMPONENT = 3 };

enum { BP_DOOR_ANIMATION_START_TIME = 8 };

enum { BP_LOADING_TICKS = 6, BP_REVEAL_TICKS = 12, BP_DISPLAY_PHASE_TICK_LIMIT = 3 };

enum {
	BP_LABEL_CAPACITY = 32,
	BP_LABEL_BACKGROUND_COLOR = 16,
	BP_LABEL_TEXT_COLOR = 15,
	BP_LABEL_FONT = 0,
	BP_SHIP_NAME_INPUT = 0,
	BP_SHIP_NAME_PARAGRAPH = 0,
	BP_COMPONENT_PARAGRAPH_BASE = 2,
	BP_COMPONENT_STRING_COUNT = 5,
	BP_UNAVAILABLE_LABEL_CAPACITY = 3
};

enum {
	BP_SOUND_DOOR_OPEN = 1,
	BP_SOUND_DOOR_CLOSE = 2,
	BP_SOUND_HYDRAULICS = 3,
	BP_SOUND_FADE_HYDRAULICS = 4,
	BP_SOUND_FADE_TICKS = 120
};

enum {
	BP_DESCRIPTION_CAPACITY = 64,
	BP_LOADING_INSET_X = 210,
	BP_LOADING_INSET_Y = 225,
	BP_APERTURE_LEFT = 196,
	BP_APERTURE_RIGHT = 470,
	BP_APERTURE_TOP = 85,
	BP_APERTURE_BOTTOM = 362,
	BP_APERTURE_REVEAL_TOP = 222,
	BP_APERTURE_REVEAL_BOTTOM = 224,
	BP_APERTURE_DELAY_TICKS = 2,
	BP_APERTURE_GROWTH = 8,
	BP_APERTURE_GROWTH_SHIFT = 3,
	BP_APERTURE_EDGE_MARGIN = 5,
	BP_APERTURE_COLOR = 1,
	BP_DESCRIPTION_HEADING_COLOR = 2,
	BP_DESCRIPTION_TEXT_COLOR = 14,
	BP_DESCRIPTION_SHADOW_FONT = 1,
	BP_DESCRIPTION_X = 220,
	BP_DESCRIPTION_Y = 320,
	BP_DESCRIPTION_LEFT = 217,
	BP_DESCRIPTION_TOP = 318,
	BP_SHIP_DESCRIPTION_RIGHT = 452,
	BP_COMPONENT_DESCRIPTION_RIGHT = 436,
	BP_DESCRIPTION_BOTTOM_BASE = 323,
	BP_DESCRIPTION_LINE_HEIGHT = 9,
	BP_DESCRIPTION_GROWTH = 3,
	BP_DESCRIPTION_REVEAL_LEAD = 2,
	BP_DESCRIPTION_MAX_HEIGHT = 34,
	BP_DESCRIPTION_BOTTOM_TRIM = 3,
	BP_SHIP_DESCRIPTION_PARAGRAPH = 1
};

typedef int16_t XwBlueprintDisplayPhase;

enum XwBlueprintDisplayPhaseValues {
	BP_IDLE = 0,
	BP_INITIAL_DELAY = 0x1,
	BP_LOADING = 0x3,
	BP_REVEAL = 0x5,
	BP_DISPLAY = 0x6
};

extern char g_blueprintShipAnimationNames[BP_SHIP_ANIMATION_COUNT][BP_SHIP_ANIMATION_NAME_CAPACITY];
extern ResFile* g_blueprintShipResourceFile;
extern Actor* g_blueprintShipActor;
extern Actor* g_blueprintBaseHologramActor;

extern int16_t g_blueprintFocusX[BP_FOCUS_COUNT];
extern int16_t g_blueprintFocusY[BP_FOCUS_COUNT];
extern const char g_blueprintUnavailableLabel[BP_UNAVAILABLE_LABEL_CAPACITY];
extern const char* g_blueprintWaitingMusicName;
extern const char* g_blueprintMusicResourceName;
extern const char* g_blueprintHallMarchMusicName;
extern ResFile* g_blueprintResourceFile;
extern Input* g_blueprintRightDoorInput;
extern Input* g_blueprintWorldInput;
extern Film* g_blueprintFilm;
extern Input* g_blueprintLeftDoorInput;
extern int16_t g_blueprintLegacyWord;
extern LandruHandle g_blueprintText;
extern Actor* g_blueprintPreviousShipButtonActor;
extern Actor* g_blueprintNextShipButtonActor;
extern Actor* g_blueprintNextComponentButtonActor;
extern Actor* g_blueprintRightDoorActor;
extern int16_t g_blueprintLoadedShipIndex;
extern int16_t g_blueprintSelectedComponent;
extern Actor* g_blueprintPreviousComponentButtonActor;
extern int16_t g_blueprintPhaseTick;
extern Input* g_blueprintDoorHintInput;
extern XwBlueprintDisplayPhase g_blueprintDisplayPhase;
extern int g_blueprintTextRevealTick;
extern Actor* g_blueprintLeftDoorActor;
extern Input* g_blueprintControlsInput;
extern int16_t g_blueprintDetailedShipCount;
extern int16_t g_blueprintDisplayedComponent;
extern int16_t g_blueprintSelectedShip;
extern int16_t g_blueprintDisplayedShip;
extern int16_t g_blueprintFocusIndex;
extern Sound* g_blueprintWaitingMusic;
extern Sound* g_blueprintHallMarchMusic;
extern Film* g_blueprintMusicFilm;
extern uint8_t g_blueprintColorTable[BP_COLOR_TABLE_SIZE];

/* Declarations follow ascending original IDB address. */

/* 0x435370 */
XwShellSceneResult blueprnt_Blueprint(struct XwShellContext* context);

/* 0x4359F0 */
void blueprnt_end_Blueprint_View(int time);

/* 0x435A60 */
int16_t blueprnt_iupdate_Blueprint_Door(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
										int rightEvent, int16_t x, int16_t y);

/* 0x435AE0 */
void blueprnt_iuser_Blueprint_Door(Input* input, int time);

/* 0x435BB0 */
void blueprnt_DrawDoorHint(Input* input, Rect* frame, Rect* clip, int16_t refresh);

/* 0x435C50 */
void blueprnt_iuser_Blueprint(Input* input, int time);

/* 0x435E70 */
void blueprnt_DrawNavigationButton(PushButton* button, Rect* frame, Rect* clip, int16_t refresh);

/* 0x435EF0 */
void blueprnt_draw_Blueprint_Text(Input* input, Rect* frame, Rect* clip, int16_t refresh);

/* 0x435FA0 */
void blueprnt_user_Blueprint_Door(Actor* actor, int time);

/* 0x436040 */
void blueprnt_UpdateHologramActor(Actor* actor, int time);

/* 0x4361B0 */
void blueprnt_LoadSelectedShipAnimation(void);

/* 0x436500 */
int16_t blueprnt_DrawHologramActor(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
								   int16_t refresh);

/* 0x4381E0 */
void blueprnt_OpenMusic(void* unusedResourceFile, Film* film);

/* 0x4382C0 */
void blueprnt_CloseMusic(void);

/* 0x438350 */
void blueprnt_LoadColorTable(void* resourceFile);

/* 0x4383B0 */
void blueprnt_LoadUiSounds(void);

/* 0x438400 */
void blueprnt_HandleSoundAction(int16_t action);

#ifdef __cplusplus
}
#endif

#endif
