#ifndef XW_FRONTEND_SCENES_HOTH_H
#define XW_FRONTEND_SCENES_HOTH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/frontend/scenes/cutscene.h"
#include "xw/landru_config.h"

#include <landru/actor.h>
#include <landru/film.h>
#include <landru/rect.h>
#include <landru/res.h>
#include <landru/sound.h>
#include <stddef.h>
#include <stdint.h>
#include <xw/frontend/shell.h>

struct XwShellContext;

enum { HOTH_ACTOR_MOVING_REGION = 2, HOTH_ACTOR_ROLE_SCROLL = 15 };

enum {
	HOTH_BACKGROUND_PARTS = 2,
	HOTH_BACKGROUND_WIDTH = 320,
	HOTH_EXTENSION_WIDTH = 160,
	HOTH_BACKGROUND_HEIGHT = 200
};

enum { HOTH_FULL_REDRAW_FRAMES = 2, HOTH_FILM_COMPLETE = 1, HOTH_SCENE_1_LEFT_MARGIN = 2 };

enum {
	HOTH_ACTION_FLYBY = 1,
	HOTH_ACTION_SPEECH_10 = 10,
	HOTH_ACTION_SPEECH_11 = 11,
	HOTH_ACTION_SPEECH_12 = 12,
	HOTH_ACTION_SPEECH_13 = 13,
	HOTH_ACTION_SPEECH_20 = 20,
	HOTH_ACTION_SPEECH_21 = 21,
	HOTH_SPEECH_SLOT_COUNT = 4,
	HOTH_MUSIC_FULL_LEVEL = 127,
	HOTH_MUSIC_SPEECH_LEVEL = 74,
	HOTH_MUSIC_SPEECH_FADE_TICKS = 120,
	HOTH_MUSIC_RESTORE_TICKS = 180
};

enum {
	HOTH_MUSIC_FADE_CEL = 100,
	HOTH_MUSIC_FADE_DURATION = 180,
	HOTH_MUSIC_PREPARE_BEAT = 2,
	HOTH_MUSIC_SCENE_2_BEAT = 4,
	HOTH_MUSIC_SCENE_3_BEAT = 35,
	HOTH_MUSIC_START_GROUP = 1,
	HOTH_MUSIC_START_TICK = 400
};

enum {
	HOTH_RESOURCE_NAME_COUNT = 5,
	HOTH_RESOURCE_NAME_SIZE = 14,
	HOTH_RESOURCE_FILE = 0,
	HOTH_FIRST_FILM = 1,
	HOTH_SECOND_FAST_FILM = 2,
	HOTH_SECOND_SLOW_FILM = 3,
	HOTH_THIRD_FILM = 4,
	HOTH_SPEED_THRESHOLD = 2,
	HOTH_BACKGROUND_Z = 200,
	HOTH_ERASE_Z = -200,
	HOTH_CAPTION_FONT = 0,
	HOTH_ARRIVAL_START = 2,
	HOTH_ARRIVAL_END = 48,
	HOTH_ARRIVAL_X = 40,
	HOTH_ARRIVAL_Y = 15,
	HOTH_ARRIVAL_COLOR = 16,
	HOTH_QUESTION_FIRST_START = 2,
	HOTH_QUESTION_FIRST_END = 34,
	HOTH_QUESTION_FIRST_X = 40,
	HOTH_QUESTION_FIRST_Y = 45,
	HOTH_QUESTION_FIRST_COLOR = 14,
	HOTH_QUESTION_SECOND_START = 2,
	HOTH_QUESTION_SECOND_END = 34,
	HOTH_QUESTION_SECOND_X = 40,
	HOTH_QUESTION_SECOND_Y = 55,
	HOTH_QUESTION_SECOND_COLOR = 14,
	HOTH_ANSWER_START = 36,
	HOTH_ANSWER_END = 55,
	HOTH_ANSWER_X = 20,
	HOTH_ANSWER_Y = 80,
	HOTH_ANSWER_COLOR = 51,
	HOTH_BASE_START = 58,
	HOTH_BASE_END = 78,
	HOTH_BASE_X = 40,
	HOTH_BASE_Y = 45,
	HOTH_BASE_COLOR = 14,
	HOTH_HOME_FIRST_START = 80,
	HOTH_HOME_FIRST_END = 100,
	HOTH_HOME_FIRST_X = 20,
	HOTH_HOME_FIRST_Y = 80,
	HOTH_HOME_FIRST_COLOR = 51,
	HOTH_HOME_SECOND_START = 80,
	HOTH_HOME_SECOND_END = 100,
	HOTH_HOME_SECOND_X = 20,
	HOTH_HOME_SECOND_Y = 90,
	HOTH_HOME_SECOND_COLOR = 51
};

extern char g_hothResourceNames[HOTH_RESOURCE_NAME_COUNT][HOTH_RESOURCE_NAME_SIZE];
extern Actor* g_hothBackgroundActor;
extern Actor* g_hothEraseActor;
extern XwSceneMusicHandles g_hothMusicState;
extern Sound* g_hothSpeechSounds[HOTH_SPEECH_SLOT_COUNT];
extern int16_t g_hothLastBackgroundX;
extern Rect g_hothDirtyRect;
extern Rect g_hothRestoreRect;
extern LandruHandle g_hothExtensionHandle;
extern Actor* g_hothScrollActor;
extern Film* g_hothFilm;
extern Rect g_hothTrackedActorRect;
extern LandruHandle g_hothBackgroundHandle;
extern uint16_t g_hothFullRedrawCountdown;

/* Declarations follow ascending original IDB address. */

/* 0x44CD00 */
void Hoth_OpenMusic(ResFile* unusedResourceFile, Film* film);

/* 0x44CDF0 */
void Hoth_SetMusicLevel(int value, int duration);

/* 0x44CE10 */
void Hoth_CloseMusic(void);

/* 0x44CE40 */
void Hoth_user_Music(Sound* unusedSound, int unusedTime);

/* 0x44CE80 */
void Hoth_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm);

/* 0x44CF70 */
void Hoth_HandleSoundAction(int16_t action);

/* 0x44D090 */
XwShellSceneResult Hoth_Play(struct XwShellContext* shell);

/* 0x44D3D0 */
void Hoth_end_View(int unusedTime);

/* 0x44D440 */
int16_t Hoth_film_Callback(Film* film, FilmObject* object);

/* 0x44D510 */
int16_t Hoth_StampBackground(Actor* actor);

/* 0x44D640 */
void Hoth_user_SoundAction(Actor* actor, int unusedTime);

/* 0x44D660 */
int16_t Hoth_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
							 int16_t unusedY, int16_t refresh);

/* 0x44D7A0 */
void Hoth_user_DirtyBounds(Actor* actor, int time);

/* 0x44D870 */
void Hoth_user_Erase(Actor* actor, int unusedTime);

#ifdef __cplusplus
}
#endif

#endif
