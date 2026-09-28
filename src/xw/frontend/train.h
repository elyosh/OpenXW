#ifndef XW_FRONTEND_TRAIN_H
#define XW_FRONTEND_TRAIN_H

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
#include <xw/frontend/shell.h>

enum {
	TRAIN_INITIAL_FOCUS = 9,
	TRAIN_INITIAL_MOUSE_X = 270,
	TRAIN_INITIAL_MOUSE_Y = 98,
	TRAIN_SCORE_DISK_NAME_CAPACITY = 24,
	TRAIN_ROOM_VIEW = 0,
	TRAIN_ROOM_NEAR_Z = 60,
	TRAIN_ROOM_FAR_Z = 100,
	TRAIN_SCREEN_FAR_Z = 50,
	TRAIN_FULL_NEAR_Z = -32000,
	TRAIN_FULL_FAR_Z = 32000,
	TRAIN_LABEL_LEVEL = 1,
	TRAIN_DOOR_LAUNCH = 1,
	TRAIN_CANVAS_LEFT = 0,
	TRAIN_CANVAS_TOP = 0,
	TRAIN_CANVAS_RIGHT = 640,
	TRAIN_CANVAS_BOTTOM = 480,
	TRAIN_MONITOR_CONTENT_LEFT = 0,
	TRAIN_MONITOR_CONTENT_TOP = 0,
	TRAIN_MONITOR_CONTENT_RIGHT = 318,
	TRAIN_MONITOR_CONTENT_BOTTOM = 202,
	TRAIN_EXIT_DOOR_LEFT = 0,
	TRAIN_EXIT_DOOR_TOP = 132,
	TRAIN_EXIT_DOOR_RIGHT = 117,
	TRAIN_EXIT_DOOR_BOTTOM = 370,
	TRAIN_LAUNCH_DOOR_LEFT = 518,
	TRAIN_LAUNCH_DOOR_TOP = 132,
	TRAIN_LAUNCH_DOOR_RIGHT = 640,
	TRAIN_LAUNCH_DOOR_BOTTOM = 480,
	TRAIN_SHIP_LABEL_LEFT = 171,
	TRAIN_SHIP_LABEL_TOP = 392,
	TRAIN_SHIP_LABEL_RIGHT = 357,
	TRAIN_SHIP_LABEL_BOTTOM = 404,
	TRAIN_NAVIGATION_LEFT = 67,
	TRAIN_NAVIGATION_TOP = 378,
	TRAIN_NAVIGATION_RIGHT = 447,
	TRAIN_NAVIGATION_BOTTOM = 479,
	TRAIN_PREVIOUS_SHIP_LEFT = 87,
	TRAIN_PREVIOUS_SHIP_TOP = 378,
	TRAIN_PREVIOUS_SHIP_RIGHT = 151,
	TRAIN_PREVIOUS_SHIP_BOTTOM = 422,
	TRAIN_NEXT_SHIP_LEFT = 372,
	TRAIN_NEXT_SHIP_TOP = 378,
	TRAIN_NEXT_SHIP_RIGHT = 436,
	TRAIN_NEXT_SHIP_BOTTOM = 423,
	TRAIN_PREVIOUS_LEVEL_LEFT = 67,
	TRAIN_PREVIOUS_LEVEL_TOP = 436,
	TRAIN_PREVIOUS_LEVEL_RIGHT = 139,
	TRAIN_PREVIOUS_LEVEL_BOTTOM = 479,
	TRAIN_NEXT_LEVEL_LEFT = 378,
	TRAIN_NEXT_LEVEL_TOP = 435,
	TRAIN_NEXT_LEVEL_RIGHT = 447,
	TRAIN_NEXT_LEVEL_BOTTOM = 479
};

extern Actor* g_trainingBackgroundActor;
extern Actor* g_trainingStarsLeftActor;
extern Actor* g_trainingStarsRightActor;
extern Actor* g_trainingHighScoresActor;
extern Actor* g_trainingWelcomeActor;
extern Input* g_trainingRootInput;
extern Input* g_trainingExitDoorInput;
extern Input* g_trainingLaunchDoorInput;

enum {
	TRAIN_STAR_SCROLL_STEP = 4,
	TRAIN_STAR_VIEW_WIDTH = 640,
	TRAIN_STAR_WRAP_WIDTH = 2 * TRAIN_STAR_VIEW_WIDTH
};

enum {
	TRAIN_FOCUS_ROWS = 3,
	TRAIN_FOCUS_COLUMNS = 5,
	TRAIN_FOCUS_COUNT = TRAIN_FOCUS_ROWS * TRAIN_FOCUS_COLUMNS,
	TRAIN_PRESENTATION_LEAD_TICKS = 11,
	TRAIN_HYDRAULIC_STOP_TICK = 11
};

extern const int16_t g_trainingFocusX[TRAIN_FOCUS_COUNT];
extern const int16_t g_trainingFocusY[TRAIN_FOCUS_COUNT];
extern Input* g_trainingMonitorInput;
extern Film* g_trainingFilm;
extern int16_t g_trainingFocusIndex;

enum { TRAIN_SCREEN_VIEW = 1, TRAIN_SHIP_INFO_REFRESH_LAST_FRAME = 11 };

enum {
	TRAIN_INFO_PANEL_LEFT = 156,
	TRAIN_INFO_PANEL_TOP = 78,
	TRAIN_INFO_PANEL_RIGHT = 473,
	TRAIN_INFO_PANEL_BOTTOM = 279,
	TRAIN_INFO_PANEL_RIGHT_BAND_LEFT = 365,
	TRAIN_INFO_PANEL_BOTTOM_BAND_TOP = 225,
	TRAIN_INFO_PANEL_COLOR = 48,
	TRAIN_INFO_PANEL_VERTICAL_SHIFT = 1
};

enum {
	TRAIN_SHIP_INFO_IDLE = 0,
	TRAIN_SHIP_INFO_SHRINK = 1,
	TRAIN_SHIP_INFO_TEXT = 2,
	TRAIN_SHIP_INFO_EXPAND = 3,
	TRAIN_SHIP_INFO_COMPLETE = 4,
	TRAIN_SHIP_INFO_SCALE_PROGRESS_END = 108,
	TRAIN_SHIP_INFO_SCALE_PROGRESS_STEP = 18,
	TRAIN_SHIP_INFO_SCALE_STEP = 20,
	TRAIN_SHIP_INFO_TEXT_PROGRESS_END = 168,
	TRAIN_SHIP_INFO_TEXT_PROGRESS_STEP = 2
};

enum { TRAIN_PRESENTATION_RESET_X = 1, TRAIN_PRESENTATION_RESET_Y = 2, TRAIN_PRESENTATION_FULL_SCALE = 256 };

enum { TRAIN_XWING_START = 477, TRAIN_XWING_INFO = 533, TRAIN_XWING_RESUME = 633, TRAIN_XWING_END = 663 };

enum {
	TRAIN_AWING_START = 292,
	TRAIN_AWING_INFO = 326,
	TRAIN_AWING_RESUME = 426,
	TRAIN_AWING_END = 473,
	TRAIN_AWING_RESET_Y = 8
};

enum {
	TRAIN_YWING_START = 667,
	TRAIN_YWING_INFO = 708,
	TRAIN_YWING_RESUME = 808,
	TRAIN_YWING_END = 848,
	TRAIN_YWING_RESET_X = 19,
	TRAIN_YWING_RESET_Y = 14
};

enum {
	TRAIN_BWING_START = 852,
	TRAIN_BWING_INFO = 877,
	TRAIN_BWING_RESUME = 977,
	TRAIN_BWING_END = 1033,
	TRAIN_BWING_RESET_X = 24,
	TRAIN_BWING_RESET_Y = 1
};

enum {
	TRAIN_PRESENTATION_VIEW_HEIGHT = 480,
	TRAIN_PRESENTATION_DEPTH = 10,
	TRAIN_PRESENTATION_AWING = 0,
	TRAIN_PRESENTATION_XWING = 1,
	TRAIN_PRESENTATION_YWING = 2,
	TRAIN_PRESENTATION_BWING = 3,
	TRAIN_AWING_TYPE_MARKER = 3,
	TRAIN_XWING_TYPE_MARKER = 4,
	TRAIN_YWING_TYPE_MARKER = 7,
	TRAIN_BWING_TYPE_MARKER = 8
};

extern Actor* g_trainingBWingActor;
extern ResFile* g_trainingResourceFile;

enum { TRAIN_MISSION_TEXT_START = 4, TRAIN_MISSION_TEXT_END = 144 };

enum { TRAIN_MISSION_TEXT_REVEAL_ALL = 120 };

enum {
	TRAIN_MISSION_TEXT_CAPACITY = 64,
	TRAIN_MISSION_FONT = 3,
	TRAIN_MISSION_TEXT_COLOR = 15,
	TRAIN_MISSION_BOLD_COLOR = 59,
	TRAIN_MISSION_HEADING_INSET = 4,
	TRAIN_MISSION_HEADING_HEIGHT = 14,
	TRAIN_MISSION_LINE_SPACING = 22,
	TRAIN_MISSION_BODY_INSET = 46,
	TRAIN_MISSION_REVEAL_DELAY = 10,
	TRAIN_MISSION_LEVEL_PARAGRAPH = 1,
	TRAIN_MISSION_BODY_PARAGRAPH_BASE = 2
};

extern LandruHandle g_trainingLevelParagraph;

enum { TRAIN_MOUSE_PRESS = 1 };

enum { TRAIN_SKIPPED_SHIP_SLOT = 4 };

enum {
	TRAIN_SOUND_R2_WHISTLE = 1,
	TRAIN_SOUND_R2_A = 2,
	TRAIN_SOUND_R2_B = 3,
	TRAIN_SOUND_R2_C = 4,
	TRAIN_SOUND_R2_D = 5,
	TRAIN_SOUND_DOOR_OPEN = 6,
	TRAIN_SOUND_DOOR_CLOSE = 7,
	TRAIN_SOUND_HYDRAULIC_START = 8,
	TRAIN_SOUND_HYDRAULIC_STOP = 9,
	TRAIN_SOUND_TYPING_START = 10,
	TRAIN_SOUND_TYPING_STOP = 11,
	TRAIN_SOUND_TYPING_UPDATE = 12,
	TRAIN_SOUND_TYPING_VOLUME = 32
};

enum { TRAIN_PILOT_PATH_CAPACITY = 256 };

enum {
	TRAIN_DOOR_EXIT = 0,
	TRAIN_DOOR_ACTIVATE_EVENT = 3,
	TRAIN_DOOR_ACTION_IDLE = 0,
	TRAIN_DOOR_ACTION_EXIT = 1,
	TRAIN_DOOR_ACTION_HINT = 2
};

enum {
	TRAIN_SHIP_COUNT = 4,
	TRAIN_SHIP_NAME_CAPACITY = 8,
	TRAIN_LABEL_SHIP = 0,
	TRAIN_LABEL_BACKGROUND_COLOR = 16,
	TRAIN_LABEL_TEXT_COLOR = 15,
	TRAIN_LABEL_FONT = 0
};

enum { TRAIN_HIGH_SCORES_START = 148, TRAIN_HIGH_SCORES_END = 288, TRAIN_HIGH_SCORES_REVEAL_STEP = 2 };

enum {
	TRAIN_SCORE_COUNT = 8,
	TRAIN_SCORE_NAME_CAPACITY = 32,
	TRAIN_SCORE_TEXT_CAPACITY = 40,
	TRAIN_SCORE_NAME_FONT = 2,
	TRAIN_SCORE_TEXT_FONT = 0,
	TRAIN_SCORE_LEFT_INSET = 4,
	TRAIN_SCORE_TOP_INSET = 4,
	TRAIN_SCORE_SCROLL_END = 168,
	TRAIN_SCORE_SCROLL_ORIGIN = 172,
	TRAIN_SCORE_TEXT_Y_OFFSET = 2,
	TRAIN_SCORE_COLOR_START = 16,
	TRAIN_SCORE_COLOR_END = 47,
	TRAIN_SCORE_FADE_END = 31,
	TRAIN_SCORE_ROW_DELAY = 12,
	TRAIN_SCORE_ROW_SPACING = 14
};

extern char g_trainingScoreNames[TRAIN_SCORE_COUNT][TRAIN_SCORE_NAME_CAPACITY];
extern int32_t g_trainingScorePoints[TRAIN_SCORE_COUNT];
extern uint16_t g_trainingScoreLevels[TRAIN_SCORE_COUNT];

enum {
	TRAIN_LEVEL_LABEL_LEFT = 161,
	TRAIN_LEVEL_LABEL_TOP = 441,
	TRAIN_LEVEL_LABEL_RIGHT = 361,
	TRAIN_LEVEL_LABEL_BOTTOM = 455,
	TRAIN_HINT_SHADOW_OFFSET = 1
};

enum { TRAIN_WELCOME_START = 1037, TRAIN_WELCOME_END = 1177, TRAIN_WELCOME_REVEAL_STEP = 2 };

enum {
	TRAIN_WELCOME_LINE_COUNT = 9,
	TRAIN_WELCOME_LINE_CAPACITY = 40,
	TRAIN_WELCOME_FONT = 3,
	TRAIN_WELCOME_LEFT_INSET = 4,
	TRAIN_WELCOME_TOP_INSET = 4,
	TRAIN_WELCOME_SCROLL_END = 168,
	TRAIN_WELCOME_SCROLL_ORIGIN = 172,
	TRAIN_WELCOME_LINE_SPACING = 22,
	TRAIN_WELCOME_LENGTH_SHIFT = 1
};

extern const char g_trainingWelcomeLines[TRAIN_WELCOME_LINE_COUNT][TRAIN_WELCOME_LINE_CAPACITY];

enum {
	TRAIN_SPEC_FONT = 2,
	TRAIN_SPEC_X = 160,
	TRAIN_SPEC_SPEED_Y = 227,
	TRAIN_SPEC_PROTECTION_Y = 241,
	TRAIN_SPEC_WEAPONS_Y = 255,
	TRAIN_SPEC_NAME_FONT = 3,
	TRAIN_SPEC_NAME_X = 375,
	TRAIN_SPEC_NAME_Y = 152,
	TRAIN_SPEC_CLASS_Y = 174,
	TRAIN_SPEC_FIRST_DELAY = 10,
	TRAIN_SPEC_SECOND_DELAY = 20,
	TRAIN_SPEC_LOGO_STEP = 2,
	TRAIN_SPEC_LOGO_MAX_OFFSET = 40,
	TRAIN_SPEC_AWING_LOGO_START_X = 419,
	TRAIN_SPEC_LOGO_START_X = 415,
	TRAIN_SPEC_LOGO_Y = 86
};

extern Actor* g_trainingRebelLogoActor;
extern Actor* g_trainingIncomLogoActor;

enum {
	TRAIN_PRESENTATION_INTERVAL_3_END = 473,
	TRAIN_PRESENTATION_INTERVAL_4_END = 663,
	TRAIN_PRESENTATION_INTERVAL_5_END = 848
};

enum {
	TRAIN_BUTTON_PREVIOUS_SHIP = 0,
	TRAIN_BUTTON_NEXT_SHIP = 1,
	TRAIN_BUTTON_PREVIOUS_LEVEL = 2,
	TRAIN_BUTTON_NEXT_LEVEL = 3
};

extern const char g_trainingShipNames[TRAIN_SHIP_COUNT][TRAIN_SHIP_NAME_CAPACITY];
extern Actor* g_trainingShipInfoActor;
extern Actor* g_trainingScreenActor;
extern Actor* g_trainingNextShipButtonActor;
extern Actor* g_trainingYWingActor;
extern Actor* g_trainingLaunchDoorActor;
extern int16_t g_trainingAvailableLevels;
extern Actor* g_trainingNextLevelButtonActor;
extern Actor* g_trainingPreviousShipButtonActor;
extern Input* g_trainingDoorHintInput;
extern Actor* g_trainingExitDoorActor;
extern struct REGISTER_PilotFileRecord g_trainingPilot;
extern Input* g_trainingNavigationInput;
extern Actor* g_trainingMissionTextActor;
extern Actor* g_trainingAWingActor;
extern Actor* g_trainingDisplayedShipActor;
extern Actor* g_trainingPreviousLevelButtonActor;
extern Actor* g_trainingXWingActor;
extern int g_trainingPresentationFrame;
extern int16_t g_trainingTotalLevels;
extern int16_t g_trainingTypingSoundActive;
extern int16_t g_trainingTypingSoundActivity;

struct XwShellContext;
typedef struct XwTrainingMusicState XwTrainingMusicState;

/* Original IDB size: 20 bytes. */
struct XwTrainingMusicState {
	/* IDB +0x0: Kept rebels GMID resource sound. */
	Sound* sound;
	/* IDB +0x4: Training film used to detect cur_cel4. */
	Film* film;
	/* IDB +0x8: Pointer to g_trainingPresentationFrame. */
	int* presentationFrame;
	/* IDB +0xC: Legacy music controller phases0,1,2,4. */
	int phase;
	/* IDB +0x10: Persistent count incremented by each enabled TRAIN_OpenMusic call. */
	int openCount;
};

enum {
	TRAIN_MUSIC_WAIT_FILM = 0,
	TRAIN_MUSIC_WAIT_FIRST_FRAME = 1,
	TRAIN_MUSIC_WAIT_LAST_FRAME = 2,
	TRAIN_MUSIC_COMPLETE = 4,
	TRAIN_MUSIC_START_CEL = 4,
	TRAIN_MUSIC_FIRST_FRAME = 273,
	TRAIN_MUSIC_LAST_FRAME = 983,
	TRAIN_MUSIC_INITIAL_OPEN_LIMIT = 3,
	TRAIN_MUSIC_BEAT_THRESHOLD = 127,
	TRAIN_MUSIC_INITIAL_CONTROL = 1,
	TRAIN_MUSIC_LATE_BEAT_CONTROL = 2,
	TRAIN_MUSIC_REPEAT_CONTROL = 3,
	TRAIN_MUSIC_PRESENTATION_CONTROL = 5,
	TRAIN_MUSIC_RETURN_GROUP = 3,
	TRAIN_MUSIC_RETURN_BEAT = 3,
	TRAIN_MUSIC_RETURN_VOLUME = 127,
	TRAIN_MUSIC_RETURN_FADE_OUT_DURATION = 600,
	TRAIN_MUSIC_RETURN_FADE_IN_DURATION = 180,
	TRAIN_MUSIC_CLOSE_FADE_DURATION = 300,
	TRAIN_MUSIC_INITIAL_LOAD_LIMIT = 2,
	TRAIN_MUSIC_REPEAT_LOAD_CONTROL = 4,
	TRAIN_MUSIC_INITIAL_LOAD_BEAT = 13,
	TRAIN_MUSIC_OPEN_VOLUME = 95
};

extern XwTrainingMusicState g_trainingMusic;

/* Declarations follow ascending original IDB address. */

/* 0x465210 */
XwShellSceneResult train_Train(struct XwShellContext* shell);

/* 0x465E10 */
void train_end_Train_View(int time);

/* 0x465EF0 */
void train_iuser_Train_Screen(Input* input, int time);

/* 0x465FD0 */
int16_t train_iupdate_Door(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent, int rightEvent,
						   int16_t x, int16_t y);

/* 0x466080 */
void train_iuser_Door(Input* input, int time);

/* 0x466150 */
void train_idraw_DoorHint(Input* input, Rect* frame, Rect* clip, int16_t refresh);

/* 0x466230 */
void train_iuser_Train(Input* input, int time);

/* 0x466370 */
void train_idraw_NavigationButton(PushButton* button, Rect* frame, Rect* clip, int16_t refresh);

/* 0x4663F0 */
int16_t train_iupdate_SelectionLabel(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
									 int rightEvent, int16_t x, int16_t y);

/* 0x466450 */
void train_idraw_Train(Input* input, Rect* frame, Rect* clip, int16_t refresh);

/* 0x4664E0 */
void train_user_XWingPresentation(Actor* actor, int time);

/* 0x466600 */
void train_user_AWingPresentation(Actor* actor, int time);

/* 0x466720 */
void train_user_YWingPresentation(Actor* actor, int time);

/* 0x466840 */
void train_user_BWingPresentation(Actor* actor, int time);

/* 0x466960 */
void train_user_Stars(Actor* actor, int time);

/* 0x466980 */
void train_user_Door(Actor* actor, int time);

/* 0x466A10 */
void train_UpdateShipInfoAnimation(Actor* actor);

/* 0x466AD0 */
void train_user_ShipInfo(Actor* actor, int time);

/* 0x466B30 */
int16_t train_DrawShipInfoPanel(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh);

/* 0x466D30 */
void train_DrawAWingSpecifications(Actor* infoActor, Rect* frame, Rect* clip, int16_t x, int16_t y);

/* 0x466E30 */
void train_DrawXWingSpecifications(Actor* infoActor, Rect* frame, Rect* clip, int16_t x, int16_t y);

/* 0x466F30 */
void train_DrawYWingSpecifications(Actor* infoActor, Rect* frame, Rect* clip, int16_t x, int16_t y);

/* 0x467030 */
void train_DrawBWingSpecifications(Actor* infoActor, Rect* frame, Rect* clip, int16_t x, int16_t y);

/* 0x467130 */
void train_user_MissionText(Actor* actor, int time);

/* 0x4671A0 */
int16_t train_Draw_Train_Screen_Mission(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
										int16_t refresh);

/* 0x467400 */
void train_user_HighScores(Actor* actor, int time);

/* 0x467470 */
int16_t train_Draw_Train_Screen_Score(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
									  int16_t refresh);

/* 0x467590 */
void train_user_Welcome(Actor* actor, int time);

/* 0x467600 */
int16_t train_DrawWelcomeText(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh);

/* 0x4676D0 */
void train_DrawTypewriterLineWithSound(const char* text, uint16_t fontId, int16_t x, int16_t y,
									   int16_t revealTicks);

/* 0x467730 */
void train_RestartMissionText(void);

/* 0x4677A0 */
void train_LoadPresentationShipAtFrame(int presentationFrame);

/* 0x467A60 */
int16_t train_LoadPilotProgress(const char* pilotName);

/* 0x467B10 */
void train_OpenMusic(ResFile* unusedResourceFile, Film* film, int* presentationFrame);

/* 0x467C10 */
void train_CloseMusic(void);

/* 0x467DC0 */
void train_user_Music(Sound* sound, int time);

/* 0x467EC0 */
void train_LoadSoundEffects(void);

/* 0x467F80 */
void train_HandleSoundAction(int16_t action);

#ifdef __cplusplus
}
#endif

#endif
