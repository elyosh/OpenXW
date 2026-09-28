#ifndef XW_FRONTEND_MAINMENU_H
#define XW_FRONTEND_MAINMENU_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/frontend/shell.h"
#include "xw/landru_config.h"

#include <landru/actor.h>
#include <landru/film.h>
#include <landru/input.h>
#include <landru/rect.h>
#include <landru/res.h>
#include <landru/sound.h>
#include <stddef.h>
#include <stdint.h>

struct XwShellContext;

enum {
	MAINMENU_WIDTH = 640,
	MAINMENU_HEIGHT = 480,
	MAINMENU_INITIAL_FOCUS = 1,
	MAINMENU_INITIAL_MOUSE_X = 147,
	MAINMENU_INITIAL_MOUSE_Y = 100,
	MAINMENU_CHRISTMAS_DATE_CAPACITY = 6,
	MAINMENU_CURRENT_DATE_CAPACITY = 9,
	MAINMENU_AMBIENT_RANDOM_RANGE = 256,
	MAINMENU_WIND_ROLL = 31,
	MAINMENU_SHAKE_ROLL = 97,
	MAINMENU_AMBIENT_Z = -1,
	MAINMENU_TRAINING_LEFT = 39,
	MAINMENU_TRAINING_TOP = 132,
	MAINMENU_TRAINING_RIGHT = 137,
	MAINMENU_TRAINING_BOTTOM = 195,
	MAINMENU_COMBAT_LEFT = 247,
	MAINMENU_COMBAT_TOP = 125,
	MAINMENU_COMBAT_RIGHT = 368,
	MAINMENU_COMBAT_BOTTOM = 188,
	MAINMENU_TOUR_LEFT = 478,
	MAINMENU_TOUR_TOP = 128,
	MAINMENU_TOUR_RIGHT = 564,
	MAINMENU_TOUR_BOTTOM = 204,
	MAINMENU_TOUR_DESK_LEFT = 470,
	MAINMENU_TOUR_DESK_TOP = 176,
	MAINMENU_TOUR_DESK_RIGHT = 523,
	MAINMENU_TOUR_DESK_BOTTOM = 206,
	MAINMENU_TECH_LEFT = 136,
	MAINMENU_TECH_TOP = 269,
	MAINMENU_TECH_RIGHT = 155,
	MAINMENU_TECH_BOTTOM = 313,
	MAINMENU_FILM_LEFT = 466,
	MAINMENU_FILM_TOP = 270,
	MAINMENU_FILM_RIGHT = 482,
	MAINMENU_FILM_BOTTOM = 308,
	MAINMENU_REGISTER_LEFT = 580,
	MAINMENU_REGISTER_TOP = 332,
	MAINMENU_REGISTER_RIGHT = 622,
	MAINMENU_REGISTER_BOTTOM = 400
};
struct REGISTER_PilotFileRecord;

extern const char g_mainMenuChristmasDate[MAINMENU_CHRISTMAS_DATE_CAPACITY];
extern Actor* g_MainMenuBackground;
extern Input* g_MainMenuProvingGroundInput;
extern Input* g_MainMenuRootInput;
extern Film* g_MainMenuFilm;
extern Input* g_MainMenuTechInput;
extern Input* g_MainMenuHistoricalCombatInput;
extern Actor* g_MainMenuWindActor;
extern Actor* g_MainMenuShakeActor;
extern Actor* g_MainMenuWaveActor;
extern Input* g_MainMenuRegisterInput;
extern Input* g_MainMenuFilmInput;
extern Input* g_MainMenuTourInput;
extern struct REGISTER_PilotFileRecord g_MainMenuPilotData;
extern Sound* g_MainMenuMarchMusic;
extern Sound* g_MainMenuSecurityMusic;
extern int g_MainMenuLastMusicDoorId;

enum {
	MAINMENU_EXIT_MUSIC_FADE_DURATION = 120,
	MAINMENU_CLOSE_MUSIC_FADE_DURATION = 180,
	MAINMENU_SECURITY_FADE_DURATION = 60,
	MAINMENU_MARCH_VOLUME = 127,
	MAINMENU_MUSIC_TRANSITION_MARKER = 1,
	MAINMENU_TOUR_DESK_JUMP_MARKER = 2,
	MAINMENU_TOUR_DESK_JUMP_GROUP = 3,
	MAINMENU_TOUR_DESK_MUSIC_CONTROL = 1,
	MAINMENU_TRAINING_MUSIC_CONTROL = 1,
	MAINMENU_COMBAT_MUSIC_CONTROL = 1,
	MAINMENU_FILM_MUSIC_CONTROL = 2,
	MAINMENU_BLUEPRINT_MUSIC_CONTROL = 2,
	MAINMENU_TARGET_MUSIC_VOLUME = 95,
	MAINMENU_TARGET_MUSIC_FADE_DURATION = 300
};

enum {
	MAINMENU_DOOR_CLOSED = 0,
	MAINMENU_DOOR_OPEN = 1,
	MAINMENU_DOOR_TOUR_CLOSED = 4,
	MAINMENU_DOOR_TOUR_OPEN = 5,
	MAINMENU_MUSIC_TRAINING_CHANNEL = 3,
	MAINMENU_MUSIC_COMBAT_CHANNEL = 5,
	MAINMENU_MUSIC_TOUR_CHANNEL = 6,
	MAINMENU_MUSIC_DOOR_ACTIVE_VOLUME = 110,
	MAINMENU_MUSIC_TOUR_ACTIVE_VOLUME = 105,
	MAINMENU_MUSIC_DOOR_ACTIVE_DURATION = 120,
	MAINMENU_MUSIC_DOOR_INACTIVE_VOLUME = 95,
	MAINMENU_MUSIC_DOOR_INACTIVE_DURATION = 180
};

enum XwMainMenuDoorSoundCue { XW_MAINMENU_CUE_DOOR_OPEN = 1, XW_MAINMENU_CUE_DOOR_CLOSE = 2 };

enum { MAINMENU_TITLE_SHOW_PULSE = 1, MAINMENU_DOOR_OPEN_PULSE = 1 };

enum {
	MAINMENU_TOUR_SUFFIX_COUNT = 8,
	MAINMENU_TOUR_SUFFIX_CAPACITY = 24,
	MAINMENU_TITLE_CAPACITY = 32,
	MAINMENU_TITLE_STRING_CAPACITY = 24,
	MAINMENU_TITLE_FONT = 3,
	MAINMENU_TITLE_SHADOW_COLOR = 16,
	MAINMENU_TITLE_TEXT_COLOR = 15,
	MAINMENU_TITLE_SHADOW_OFFSET = 1
};

extern char g_MainMenuTrainingTitle[MAINMENU_TITLE_STRING_CAPACITY];
extern char g_MainMenuCombatTitle[MAINMENU_TITLE_STRING_CAPACITY];
extern char g_MainMenuContinueTourTitle[MAINMENU_TITLE_STRING_CAPACITY];
extern char g_MainMenuTechRoomTitle[MAINMENU_TITLE_STRING_CAPACITY];
extern char g_MainMenuFilmRoomTitle[MAINMENU_TITLE_STRING_CAPACITY];
extern char g_MainMenuRegisterTitle[MAINMENU_TITLE_STRING_CAPACITY];
extern char g_MainMenuNewTourTitle[MAINMENU_TITLE_STRING_CAPACITY];
extern char g_MainMenuChangeToursTitle[MAINMENU_TITLE_STRING_CAPACITY];
extern char g_MainMenuCutscenesTitle[MAINMENU_TITLE_STRING_CAPACITY];

extern const char g_MainMenuTourSuffixes[MAINMENU_TOUR_SUFFIX_COUNT][MAINMENU_TOUR_SUFFIX_CAPACITY];

enum XwMainMenuInputId {
	MAINMENU_INPUT_TRAINING = 0,
	MAINMENU_INPUT_COMBAT = 1,
	MAINMENU_INPUT_TOUR = 2,
	MAINMENU_INPUT_TECH_ROOM = 3,
	MAINMENU_INPUT_FILM_ROOM = 4,
	MAINMENU_INPUT_REGISTER = 5,
	MAINMENU_INPUT_TOUR_DESK = 6
};

enum { MAINMENU_DOOR_COUNT = 6, MAINMENU_ACTIVATE_EVENT = 3 };

enum {
	MAINMENU_FOCUS_ROWS = 2,
	MAINMENU_FOCUS_COLUMNS = 4,
	MAINMENU_FOCUS_COUNT = MAINMENU_FOCUS_ROWS * MAINMENU_FOCUS_COLUMNS
};

extern int16_t g_MainMenuMouseX[MAINMENU_FOCUS_COUNT];
extern int16_t g_MainMenuMouseY[MAINMENU_FOCUS_COUNT];
extern Actor* g_MainMenuStars;

extern Actor* g_MainMenuDoors[MAINMENU_DOOR_COUNT];
extern Actor* g_MainMenuTitle;
extern int16_t g_MainMenuFocus;

enum { MAINMENU_PILOT_FILENAME_CAPACITY = 32 };

enum {
	MAINMENU_AMBIENT_SPEECH_COUNT = 3,
	MAINMENU_AMBIENT_INITIAL_DELAY = 16,
	MAINMENU_AMBIENT_INITIAL_RANDOM_MASK = 31,
	MAINMENU_AMBIENT_REPEAT_DELAY = 64,
	MAINMENU_AMBIENT_REPEAT_RANDOM_MASK = 63
};

extern Sound* g_MainMenuAmbientSpeech[MAINMENU_AMBIENT_SPEECH_COUNT];

/* Declarations follow ascending original IDB address. */

/* 0x457E50 */
XwShellSceneResult mainmenu_Main_Menu(struct XwShellContext* shell);

/* 0x458630 */
void mainmenu_end_View(int time);

/* 0x4586C0 */
int16_t mainmenu_iupdate_MainMenu(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
								  int rightEvent, int16_t x, int16_t y);

/* 0x4587C0 */
void mainmenu_iuser_MainMenu(Input* input, int time);

/* 0x4587E0 */
int16_t mainmenu_draw_AmbientAnimation(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
									   int16_t refresh);

/* 0x458840 */
void mainmenu_user_Title(Actor* actor, int time);

/* 0x458890 */
void mainmenu_draw_Title(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh);

/* 0x458AA0 */
void mainmenu_user_Door(Actor* actor, int time);

/* 0x458B60 */
int16_t mainmenu_LoadPilotRecord(void);

/* 0x4593C0 */
void mainmenu_OpenMusic(ResFile* unusedResourceFile, Film* unusedFilm);

/* 0x4594F0 */
void mainmenu_UpdateDoorMusic(Actor* door);

/* 0x4596E0 */
void mainmenu_CloseMusic(void);

/* 0x4597B0 */
void mainmenu_user_Music(Sound* unusedSound, int unusedTime);

/* 0x4597F0 */
void mainmenu_TransitionToTrainingMusic(void);

/* 0x459810 */
void mainmenu_TransitionToCombatMusic(void);

/* 0x459830 */
void mainmenu_TransitionToTourDeskMusic(void);

/* 0x4599B0 */
void mainmenu_TransitionToFilmMusic(void);

/* 0x4599D0 */
void mainmenu_TransitionToBlueprintMusic(void);

/* 0x4599F0 */
void mainmenu_StopMusicForExit(void);

/* 0x459A20 */
void mainmenu_PrepareFadedMusicTransition(const char* filename, const char* name, uint8_t controlValue);

/* 0x459B40 */
void mainmenu_PrepareMusicTransition(const char* filename, const char* name, uint8_t controlValue,
									 Sound** outSound);

/* 0x459C40 */
void mainmenu_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm);

/* 0x459CC0 */
void mainmenu_user_AmbientSpeech(Sound* sound, int time);

/* 0x459D30 */
void mainmenu_PlayDoorSoundCue(int16_t cue);

#ifdef __cplusplus
}
#endif

#endif
