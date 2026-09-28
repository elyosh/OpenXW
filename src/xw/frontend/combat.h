#ifndef XW_FRONTEND_COMBAT_H
#define XW_FRONTEND_COMBAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/frontend/register.h"
#include "xw/frontend/shipext.h"
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
#include <xw/frontend/shell.h>

struct XwShellContext;

enum {
	COMBAT_PARAGRAPH_NAME_CAPACITY = 32,
	COMBAT_INITIAL_FOCUS = 9,
	COMBAT_INITIAL_MOUSE_X = 284,
	COMBAT_INITIAL_MOUSE_Y = 116,
	COMBAT_RESOURCE_NAMES_PARAGRAPH = 2,
	COMBAT_CANVAS_WIDTH = 640,
	COMBAT_CANVAS_HEIGHT = 480,
	COMBAT_STARS_Z = 120,
	COMBAT_BACKGROUND_Z = 140,
	COMBAT_MONITOR_TEXT_Z = 110,
	COMBAT_MISSION_LABEL = 1,
	COMBAT_CANVAS_LEFT = 0,
	COMBAT_CANVAS_TOP = 0,
	COMBAT_CANVAS_RIGHT = 640,
	COMBAT_CANVAS_BOTTOM = 480,
	COMBAT_LEFT_DOOR_LEFT = 0,
	COMBAT_LEFT_DOOR_TOP = 134,
	COMBAT_LEFT_DOOR_RIGHT = 89,
	COMBAT_LEFT_DOOR_BOTTOM = 420,
	COMBAT_RIGHT_DOOR_LEFT = 529,
	COMBAT_RIGHT_DOOR_TOP = 134,
	COMBAT_RIGHT_DOOR_RIGHT = 640,
	COMBAT_RIGHT_DOOR_BOTTOM = 420,
	COMBAT_SHIP_LABEL_LEFT = 227,
	COMBAT_SHIP_LABEL_TOP = 396,
	COMBAT_SHIP_LABEL_RIGHT = 415,
	COMBAT_SHIP_LABEL_BOTTOM = 415,
	COMBAT_NAVIGATION_LEFT = 136,
	COMBAT_NAVIGATION_TOP = 379,
	COMBAT_NAVIGATION_RIGHT = 508,
	COMBAT_NAVIGATION_BOTTOM = 479,
	COMBAT_PREVIOUS_SHIP_LEFT = 149,
	COMBAT_PREVIOUS_SHIP_TOP = 381,
	COMBAT_PREVIOUS_SHIP_RIGHT = 212,
	COMBAT_PREVIOUS_SHIP_BOTTOM = 426,
	COMBAT_NEXT_SHIP_LEFT = 433,
	COMBAT_NEXT_SHIP_TOP = 381,
	COMBAT_NEXT_SHIP_RIGHT = 494,
	COMBAT_NEXT_SHIP_BOTTOM = 425,
	COMBAT_PREVIOUS_MISSION_LEFT = 136,
	COMBAT_PREVIOUS_MISSION_TOP = 440,
	COMBAT_PREVIOUS_MISSION_RIGHT = 203,
	COMBAT_PREVIOUS_MISSION_BOTTOM = 479,
	COMBAT_NEXT_MISSION_LEFT = 441,
	COMBAT_NEXT_MISSION_TOP = 440,
	COMBAT_NEXT_MISSION_RIGHT = 508,
	COMBAT_NEXT_MISSION_BOTTOM = 479,
	COMBAT_MISSION_LABEL_LEFT = 219,
	COMBAT_MISSION_LABEL_TOP = 447,
	COMBAT_MISSION_LABEL_RIGHT = 418,
	COMBAT_MISSION_LABEL_BOTTOM = 465
};

extern Actor* g_combatStarsActor;
extern Actor* g_combatStarsWrapActor;
extern Actor* g_combatMonitorBackgroundActor;
extern Actor* g_combatScoreActor;
extern Actor* g_combatWelcomeTextActor;
extern Input* g_combatRootInput;
extern Input* g_combatLeftDoorInput;
extern Input* g_combatRightDoorInput;

enum {
	COMBAT_MISSION_PARAGRAPH_COUNT = 14,
	COMBAT_SCORE_ENTRY_COUNT = 8,
	COMBAT_SCORE_PILOT_TEXT_CAPACITY = 32,
	COMBAT_SCORE_TEXT_CAPACITY = 40,
	COMBAT_SCORE_MARGIN = 4,
	COMBAT_SCORE_SCROLL_LIMIT = 168,
	COMBAT_SCORE_DETAIL_Y_OFFSET = 2,
	COMBAT_SCORE_COLOR_BASE = 16,
	COMBAT_SCORE_FADE_LIMIT = 31,
	COMBAT_SCORE_COLOR_MAX = 47,
	COMBAT_SCORE_PILOT_FONT = 2,
	COMBAT_SCORE_DETAIL_FONT = 0,
	COMBAT_SCORE_EMPTY_FONT = 3,
	COMBAT_SCORE_ENTRY_REVEAL_STEP = 12,
	COMBAT_SCORE_LINE_HEIGHT = 14
};

extern LandruHandle g_combatMissionParagraphs[COMBAT_MISSION_PARAGRAPH_COUNT];

enum {
	COMBAT_ROLE_TAG_1 = 1,
	COMBAT_ROLE_SCREEN = 2,
	COMBAT_ROLE_TAG_3 = 3,
	COMBAT_ROLE_LEFT_DOOR = 4,
	COMBAT_ROLE_RIGHT_DOOR = 5,
	COMBAT_ROLE_NAVIGATION_REVEAL = 6,
	COMBAT_ROLE_SHIP_ICON_FRAME = 7,
	COMBAT_ROLE_FIRST_DECOR = 8,
	COMBAT_ROLE_SECOND_DECOR = 9,
	COMBAT_ROLE_RANDOM_ANIMATION = 10
};

enum { COMBAT_PILOT_FILENAME_CAPACITY = 32 };

enum {
	COMBAT_SELECTION_SHIP_LABEL = 0,
	COMBAT_SHIP_NAMES_PARAGRAPH = 0,
	COMBAT_TOUR_NAMES_PARAGRAPH = 1,
	COMBAT_SELECTION_LABEL_CAPACITY = 64,
	COMBAT_MISSION_NUMBER_CAPACITY = 32,
	COMBAT_SELECTION_LABEL_BACKGROUND_COLOR = 16,
	COMBAT_SELECTION_LABEL_FONT = 0,
	COMBAT_SELECTION_LABEL_TEXT_COLOR = 15
};

enum {
	COMBAT_SCORE_FILENAME_CAPACITY = 256,
	COMBAT_SCORE_MISSION_CAPACITY = 16,
	COMBAT_TOUR_SCORE_MISSION_CAPACITY = 25
};

enum { COMBAT_STARFIELD_STEP = 4, COMBAT_STARFIELD_WIDTH = 640, COMBAT_STARFIELD_WRAP = 1280 };

enum { COMBAT_MOUSE_PRESS = 1, COMBAT_MOUSE_SELECT = 3 };

enum { COMBAT_FOCUS_ROWS = 3, COMBAT_FOCUS_COLUMNS = 5, COMBAT_FOCUS_CAPACITY = 16 };

enum {
	COMBAT_HYDRAULICS_START_FRAME = 7,
	COMBAT_HYDRAULICS_STOP_FRAME = 20,
	COMBAT_SCREEN_RELEASE_FRAME = 18,
	COMBAT_MONITOR_INTRO_OVERLAP = 16,
	COMBAT_MONITOR_PERIOD = 653
};

enum {
	COMBAT_DOOR_LEFT = 0,
	COMBAT_DOOR_RIGHT = 1,
	COMBAT_DOOR_SCREEN = 2,
	COMBAT_DOOR_IDLE = 0,
	COMBAT_DOOR_EXIT_REQUEST = 1,
	COMBAT_DOOR_HOVER_REQUEST = 2
};

enum {
	COMBAT_DOOR_LABEL_FONT = 0,
	COMBAT_DOOR_LABEL_BACKGROUND_COLOR = 16,
	COMBAT_DOOR_LABEL_SHADOW_COLOR = 16,
	COMBAT_DOOR_LABEL_TEXT_COLOR = 15,
	COMBAT_DOOR_LABEL_SHADOW_OFFSET = 1
};

enum { COMBAT_MISSION_TEXT_START = 4, COMBAT_MISSION_TEXT_END = 120 };

enum { COMBAT_MISSION_TEXT_REVEAL_ALL = 120 };

enum {
	COMBAT_MISSION_SCREEN_TEXT_CAPACITY = 64,
	COMBAT_MISSION_SCREEN_TOP_INSET = 4,
	COMBAT_MISSION_SCREEN_TITLE_HEIGHT = 20,
	COMBAT_MISSION_SCREEN_FONT = 3,
	COMBAT_MISSION_SCREEN_COLOR = 15,
	COMBAT_MISSION_SCREEN_LINE_SPACING = 22,
	COMBAT_MISSION_SCREEN_DESCRIPTION_TOP = 46,
	COMBAT_MISSION_SCREEN_REVEAL_DELAY = 10,
	COMBAT_MISSION_SCREEN_LENGTH_SHIFT = 1,
	COMBAT_MISSION_TITLES_PARAGRAPH = 1,
	COMBAT_MISSION_DESCRIPTION_PARAGRAPH_BASE = 2
};

extern char g_combatMissionFormatA[];
extern char g_combatMissionFormatB[];

enum { COMBAT_SCORE_START = 124, COMBAT_SCORE_END = 244, COMBAT_SCORE_REVEAL_STEP = 2 };

enum {
	COMBAT_MONITOR_INTERVAL_3_END = 323,
	COMBAT_MONITOR_INTERVAL_4_END = 408,
	COMBAT_MONITOR_INTERVAL_5_END = 508
};

enum { COMBAT_WELCOME_START = 512, COMBAT_WELCOME_END = 652, COMBAT_WELCOME_REVEAL_STEP = 2 };

enum {
	COMBAT_WELCOME_LINE_COUNT = 9,
	COMBAT_WELCOME_LINE_CAPACITY = 40,
	COMBAT_WELCOME_FONT = 3,
	COMBAT_WELCOME_LEFT_INSET = 2,
	COMBAT_WELCOME_TOP_INSET = 4,
	COMBAT_WELCOME_SCROLL_END = 168,
	COMBAT_WELCOME_SCROLL_ORIGIN = 172,
	COMBAT_WELCOME_LINE_SPACING = 22,
	COMBAT_WELCOME_LENGTH_SHIFT = 1
};

extern char g_combatWelcomeLines[COMBAT_WELCOME_LINE_COUNT][COMBAT_WELCOME_LINE_CAPACITY];

enum {
	COMBAT_TIE_FIGHTER_START = 248,
	COMBAT_TIE_FIGHTER_INFO = 265,
	COMBAT_TIE_FIGHTER_RESUME = 315,
	COMBAT_TIE_INTERCEPTOR_START = 327,
	COMBAT_TIE_INTERCEPTOR_INFO = 354,
	COMBAT_TIE_INTERCEPTOR_RESUME = 404,
	COMBAT_TIE_BOMBER_START = 412,
	COMBAT_TIE_BOMBER_INFO = 438,
	COMBAT_TIE_BOMBER_RESUME = 488,
	COMBAT_TIE_BOMBER_X = 214,
	COMBAT_TIE_BOMBER_Y = 114,
	COMBAT_ENEMY_PREVIEW_X = 158,
	COMBAT_ENEMY_PREVIEW_Y = 80
};

enum {
	COMBAT_ENEMY_FIGHTER = 0,
	COMBAT_ENEMY_INTERCEPTOR = 1,
	COMBAT_ENEMY_BOMBER = 2,
	COMBAT_ENEMY_PREVIEW_COUNT = 3,
	COMBAT_ENEMY_RESOURCE_NAME_CAPACITY = 24,
	COMBAT_PREVIEW_FRAME_LEFT = 156,
	COMBAT_PREVIEW_FRAME_TOP = 78,
	COMBAT_PREVIEW_FRAME_RIGHT = 475,
	COMBAT_PREVIEW_FRAME_BOTTOM = 280,
	COMBAT_PREVIEW_Z = 100,
	COMBAT_PREVIEW_ROLE = 3
};

enum { COMBAT_DECOR_ROLE_BASE = 8, COMBAT_DECOR_STATE_COUNT = 2, COMBAT_DECOR_SKIPPED_STATE = 1 };

enum {
	COMBAT_SHIP_INFO_IDLE = 0,
	COMBAT_SHIP_INFO_PHASE_1 = 1,
	COMBAT_SHIP_INFO_PHASE_2 = 2,
	COMBAT_SHIP_INFO_PHASE_1_TICKS = 13
};

enum {
	COMBAT_BUTTON_PREVIOUS_SHIP = 0,
	COMBAT_BUTTON_NEXT_SHIP = 1,
	COMBAT_BUTTON_PREVIOUS_MISSION = 2,
	COMBAT_BUTTON_NEXT_MISSION = 3
};

enum {
	COMBAT_SOUND_DOOR_OPEN = 1,
	COMBAT_SOUND_DOOR_CLOSE = 2,
	COMBAT_SOUND_HYDRAULICS_START = 3,
	COMBAT_SOUND_HYDRAULICS_STOP = 4,
	COMBAT_SOUND_TARGET = 5,
	COMBAT_SOUND_TYPING_START = 6,
	COMBAT_SOUND_TYPING_STOP = 7,
	COMBAT_SOUND_TYPING_UPDATE = 8
};

enum {
	COMBAT_ENEMY_TEXT_COUNT = 15,
	COMBAT_ENEMY_TEXT_CAPACITY = 20,
	COMBAT_ENEMY_TITLE_HEIGHT = 20,
	COMBAT_ENEMY_TITLE_TOP = 4,
	COMBAT_ENEMY_TITLE_SPACING = 22,
	COMBAT_ENEMY_TITLE_FONT = 3,
	COMBAT_ENEMY_SPEC_FONT = 2,
	COMBAT_ENEMY_SPEC_SPACING = 14,
	COMBAT_ENEMY_LINE_DELAY = 8,
	COMBAT_ENEMY_TITLE_COLOR_START = 4,
	COMBAT_ENEMY_TITLE_COLOR_STEP = 4,
	COMBAT_ENEMY_COLOR_MAX = 47,
	COMBAT_ENEMY_COLOR_MIN = 23,
	COMBAT_ENEMY_COLOR_STEP = 8,
	COMBAT_ENEMY_BORDER_GROW_END = 3,
	COMBAT_ENEMY_BORDER_SHRINK_START = 9,
	COMBAT_ENEMY_BORDER_END = 12,
	COMBAT_ENEMY_BORDER_MAX_COUNT = 4,
	COMBAT_ENEMY_BORDER_INITIAL_EXPANSION = 18,
	COMBAT_ENEMY_BORDER_STEP = 2,
	COMBAT_ENEMY_BACKGROUND_COLOR = 52,
	COMBAT_ENEMY_FRAME_COLOR = 54
};

extern int16_t g_combatEnemySpecLineBounds[COMBAT_ENEMY_PREVIEW_COUNT + 1];
extern int16_t g_combatEnemySpecX[COMBAT_ENEMY_PREVIEW_COUNT];
extern int16_t g_combatEnemySpecY[COMBAT_ENEMY_PREVIEW_COUNT];
extern char g_combatEnemyText[COMBAT_ENEMY_TEXT_COUNT][COMBAT_ENEMY_TEXT_CAPACITY];
extern char g_combatEnemyResourceNames[COMBAT_ENEMY_PREVIEW_COUNT][COMBAT_ENEMY_RESOURCE_NAME_CAPACITY];
extern int16_t g_combatKeyboardFocusX[COMBAT_FOCUS_CAPACITY];
extern int16_t g_combatKeyboardFocusY[COMBAT_FOCUS_CAPACITY];
extern int16_t g_combatTypingSoundActive;
extern int16_t g_combatTypingRequested;
extern Actor* g_combatShipInfoActor;
extern Actor* g_combatNextShipButtonActor;
extern LandruHandle g_combatShipListText;
extern Actor* g_combatScreenActor;
extern Actor* g_combatFilmTag3Actor;
extern Actor* g_combatLeftDoorActor;
extern int16_t g_combatSavedDecorStates[COMBAT_DECOR_STATE_COUNT];
extern Actor* g_combatNextMissionButtonActor;
extern LandruHandle g_combatScoreHandle;
extern Actor* g_combatFallbackIconActor;
extern Actor* g_combatEnemyPreviewActors[COMBAT_ENEMY_PREVIEW_COUNT];
extern Actor* g_combatPreviousShipButtonActor;
extern REGISTER_PilotFileRecord g_combatPilotData;
extern Actor* g_combatNavigationRevealActor;
extern Input* g_combatHoverLabelInput;
extern Actor* g_combatRightDoorActor;
extern int16_t g_combatAvailableTourCount;
extern int16_t g_combatScoreSelection;
extern int16_t g_combatAvailableShipCount;
extern int16_t g_combatMissionCounts[SHIPEXT_COMBAT_SELECTION_COUNT];
extern Actor* g_combatShipIconsActor;
extern Actor* g_combatShipIconFrameActor;
extern Input* g_combatNavigationInput;
extern Film* g_combatFilm;
extern Actor* g_combatMissionTextActor;
extern ResFile* g_combatResourceFile;
extern int16_t g_combatSelectionSourceIndices[SHIPEXT_COMBAT_SELECTION_COUNT];
extern Actor* g_combatFilmTag1Actor;
extern Actor* g_combatCurrentEnemyPreview;
extern Actor* g_combatPreviousMissionButtonActor;
extern int16_t g_combatScoreMissionCount;
extern int g_combatMonitorTime;
extern int16_t g_combatKeyboardFocusIndex;
typedef struct XwCombatMusicState XwCombatMusicState;

enum {
	COMBAT_MUSIC_WAIT_FILM = 0,
	COMBAT_MUSIC_WAIT_SCORE_END = 1,
	COMBAT_MUSIC_WAIT_FIGHTER = 3,
	COMBAT_MUSIC_WAIT_INTERCEPTOR = 4,
	COMBAT_MUSIC_WAIT_BOMBER = 5,
	COMBAT_MUSIC_WAIT_BOMBER_RESUME = 6,
	COMBAT_MUSIC_WAIT_REPEAT = 7
};

enum {
	COMBAT_MUSIC_INITIAL_VISIT_LIMIT = 2,
	COMBAT_MUSIC_START_BEAT = 33,
	COMBAT_MUSIC_OPEN_VOLUME = 95,
	COMBAT_MUSIC_OPEN_DURATION = 300,
	COMBAT_MUSIC_CONTROL_1 = 1,
	COMBAT_MUSIC_CONTROL_2 = 2,
	COMBAT_MUSIC_CONTROL_3 = 3,
	COMBAT_MUSIC_CONTROL_4 = 4,
	COMBAT_MUSIC_CONTROL_5 = 5,
	COMBAT_MUSIC_RETURN_GROUP = 1,
	COMBAT_MUSIC_RETURN_BEAT = 3,
	COMBAT_MUSIC_RETURN_VOLUME = 127,
	COMBAT_MUSIC_RETURN_DURATION = 180,
	COMBAT_MUSIC_REPEAT_VISITS = 3,
	COMBAT_MUSIC_SECTION_END = 182,
	COMBAT_MUSIC_SECTION_4_START = 133,
	COMBAT_MUSIC_SECTION_3_START = 80,
	COMBAT_MUSIC_SECTION_2_START = 32,
	COMBAT_MUSIC_CHANNEL_FIGHTER = 5,
	COMBAT_MUSIC_CHANNEL_INTERCEPTOR = 4,
	COMBAT_MUSIC_CHANNEL_BOMBER = 6,
	COMBAT_MUSIC_CHANNEL_BOMBER_SECONDARY = 8
};

extern XwCombatMusicState g_combatMusicState;

/* Original IDB size: 20 bytes. */
struct XwCombatMusicState {
	/* IDB +0x0 */
	Sound* sound;
	/* IDB +0x4 */
	Film* film;
	/* IDB +0x8 */
	int* monitorTime;
	/* IDB +0xC: Phases 0,1,3,4,5,6,7; driven by film cel, active sound count and monitor timeline. */
	int phase;
	/* IDB +0x10: Incremented on music-enabled screen exits; values>=3 choose alternate opening/exit controls.
	 */
	int completedVisits;
};

/* Declarations follow ascending original IDB address. */

/* 0x43E4B0 */
void combat_OpenMusic(ResFile* unusedResourceFile, Film* film, int* monitorTime);

/* 0x43E5F0 */
void combat_CloseMusic(void);

/* 0x43E700 */
void combat_user_Music(Sound* sound, int time);

/* 0x43E900 */
void combat_LoadSoundEffects(void);

/* 0x43E980 */
void combat_HandleSoundAction(int16_t action);

/* 0x43EB40 */
XwShellSceneResult combat_Combat(struct XwShellContext* shell);

/* 0x43F6E0 */
void combat_end_Combat_View(int time);

/* 0x43F870 */
int16_t combat_film_Combat_Callback(Film* film, FilmObject* filmObject);

/* 0x43FA50 */
int16_t combat_iupdate_Combat_Screen(Input* input, Rect* frame, Rect* clip, int16_t key, uint8_t leftEvent,
									 uint8_t rightEvent, int16_t x, int16_t y);

/* 0x43FA80 */
void combat_iuser_Combat_Screen(Input* input, int context);

/* 0x43FB60 */
int16_t combat_iupdate_Door(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent, int rightEvent,
							int16_t x, int16_t y);

/* 0x43FBD0 */
void combat_iuser_Door(Input* input, int context);

/* 0x43FCE0 */
void combat_idraw_DoorLabel(Input* input, Rect* frame, Rect* clip, int16_t refresh);

/* 0x43FD90 */
void combat_iuser_Combat(Input* input, int time);

/* 0x43FF00 */
void combat_idraw_ArrowButton(PushButton* button, Rect* frame, Rect* clip, int16_t refresh);

/* 0x43FF80 */
int16_t combat_iupdate_SelectionLabel(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
									  int rightEvent, int16_t x, int16_t y);

/* 0x43FFE0 */
void combat_idraw_Combat(Input* input, Rect* frame, Rect* clip, int16_t refresh);

/* 0x4401D0 */
void combat_user_DecorState(Actor* actor, int time);

/* 0x440230 */
void combat_user_Stars(Actor* actor, int time);

/* 0x440270 */
void combat_user_Door(Actor* actor, int time);

/* 0x440300 */
void combat_user_NavigationReveal(Actor* actor, int time);

/* 0x440350 */
void combat_user_ShipIconFrame(Actor* actor, int time);

/* 0x440380 */
int16_t combat_draw_ShipIconFrame(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
								  int16_t refresh);

/* 0x440450 */
void combat_user_TieFighter(Actor* actor, int time);

/* 0x440560 */
void combat_user_TieInterceptor(Actor* actor, int time);

/* 0x440670 */
void combat_user_TieBomber(Actor* actor, int time);

/* 0x440780 */
void combat_update_ShipInfo(Actor* actor);

/* 0x4407B0 */
void combat_user_ShipInfo(Actor* actor, int time);

/* 0x4407E0 */
int16_t combat_Draw_Combat_Screen_Flyby(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
										int16_t refresh);

/* 0x440A40 */
void combat_user_Score(Actor* actor, int time);

/* 0x440AB0 */
int16_t combat_Draw_Combat_Screen_Score(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
										int16_t refresh);

/* 0x440D50 */
void combat_user_Welcome(Actor* actor, int time);

/* 0x440DC0 */
int16_t combat_draw_Welcome(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh);

/* 0x440E90 */
void combat_user_MissionText(Actor* actor, int time);

/* 0x440EF0 */
int16_t combat_Draw_Combat_Screen_Mission(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
										  int16_t refresh);

/* 0x4411E0 */
void combat_DrawSoundTypewriterLine(const char* text, uint16_t fontId, int16_t x, int16_t y,
									int16_t revealSteps);

/* 0x4413C0 */
void combat_ShowMissionText(void);

/* 0x441430 */
void combat_LoadPreviewOnDemand(int monitorTime);

/* 0x441590 */
void combat_ReadPilot(void);

/* 0x441630 */
void combat_Load_Combat_High_Scores(void);

#ifdef __cplusplus
}
#endif

#endif
