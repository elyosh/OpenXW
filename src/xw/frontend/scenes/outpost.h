#ifndef XW_FRONTEND_SCENES_OUTPOST_H
#define XW_FRONTEND_SCENES_OUTPOST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/frontend/scenes/cutscene.h"
#include "xw/landru_config.h"

#include <landru/actor.h>
#include <landru/film.h>
#include <landru/memhdl.h>
#include <landru/rect.h>
#include <landru/res.h>
#include <landru/sound.h>
#include <stddef.h>
#include <stdint.h>
#include <xw/frontend/shell.h>

struct XwShellContext;

enum {
	OUTPOST_CAPTION_FONT = 0,
	OUTPOST_SPEED_THRESHOLD = 2,
	OUTPOST_BACKGROUND_Z = 200,
	OUTPOST_CLOSE_Z = -200
};

enum {
	OUTPOST_DEPLOY_FIRST_START = 4,
	OUTPOST_DEPLOY_FIRST_END = 80,
	OUTPOST_DEPLOY_FIRST_X = 80,
	OUTPOST_DEPLOY_FIRST_Y = 176,
	OUTPOST_DEPLOY_FIRST_COLOR = 16
};

enum {
	OUTPOST_DEPLOY_SECOND_START = 4,
	OUTPOST_DEPLOY_SECOND_END = 80,
	OUTPOST_DEPLOY_SECOND_X = 80,
	OUTPOST_DEPLOY_SECOND_Y = 186,
	OUTPOST_DEPLOY_SECOND_COLOR = 16
};

enum {
	OUTPOST_SCATTER_START = 4,
	OUTPOST_SCATTER_END = 60,
	OUTPOST_SCATTER_X = 30,
	OUTPOST_SCATTER_Y = 176,
	OUTPOST_SCATTER_COLOR = 16
};

enum {
	OUTPOST_TRANSMIT_FIRST_START = 4,
	OUTPOST_TRANSMIT_FIRST_END = 79,
	OUTPOST_TRANSMIT_FIRST_X = 40,
	OUTPOST_TRANSMIT_FIRST_Y = 176,
	OUTPOST_TRANSMIT_FIRST_COLOR = 16
};

enum {
	OUTPOST_TRANSMIT_SECOND_START = 4,
	OUTPOST_TRANSMIT_SECOND_END = 79,
	OUTPOST_TRANSMIT_SECOND_X = 40,
	OUTPOST_TRANSMIT_SECOND_Y = 186,
	OUTPOST_TRANSMIT_SECOND_COLOR = 16
};

enum {
	OUTPOST_OUTPOST_FIRST_START = 4,
	OUTPOST_OUTPOST_FIRST_END = 100,
	OUTPOST_OUTPOST_FIRST_X = 30,
	OUTPOST_OUTPOST_FIRST_Y = 176,
	OUTPOST_OUTPOST_FIRST_COLOR = 16
};

enum {
	OUTPOST_OUTPOST_SECOND_START = 4,
	OUTPOST_OUTPOST_SECOND_END = 100,
	OUTPOST_OUTPOST_SECOND_X = 30,
	OUTPOST_OUTPOST_SECOND_Y = 186,
	OUTPOST_OUTPOST_SECOND_COLOR = 16
};

enum {
	OUTPOST_INTERCEPT_START = 26,
	OUTPOST_INTERCEPT_END = 52,
	OUTPOST_INTERCEPT_X = 100,
	OUTPOST_INTERCEPT_Y = 80,
	OUTPOST_INTERCEPT_COLOR = 51
};

enum {
	OUTPOST_CODE_START = 60,
	OUTPOST_CODE_END = 85,
	OUTPOST_CODE_X = 10,
	OUTPOST_CODE_Y = 40,
	OUTPOST_CODE_COLOR = 14
};

enum {
	OUTPOST_DECODE_START = 90,
	OUTPOST_DECODE_END = 120,
	OUTPOST_DECODE_X = 100,
	OUTPOST_DECODE_Y = 80,
	OUTPOST_DECODE_COLOR = 51
};

enum {
	OUTPOST_PLANS_START = 40,
	OUTPOST_PLANS_END = 76,
	OUTPOST_PLANS_X = 60,
	OUTPOST_PLANS_Y = 146,
	OUTPOST_PLANS_COLOR = 51
};

enum {
	OUTPOST_RELAY_START = 82,
	OUTPOST_RELAY_END = 122,
	OUTPOST_RELAY_X = 40,
	OUTPOST_RELAY_Y = 146,
	OUTPOST_RELAY_COLOR = 14
};

extern Actor* g_outpostBackgroundActor;
extern Actor* g_outpostCloseActor;

enum { OUTPOST_REFRESH_FULL_CANVAS = 1 };

enum {
	OUTPOST_BACKGROUND_WIDTH = 320,
	OUTPOST_BACKGROUND_HEIGHT = 200,
	OUTPOST_HALF_BACKGROUND_HEIGHT = OUTPOST_BACKGROUND_HEIGHT / 2
};

enum {
	OUTPOST_CUE_MESSAGE = 1,
	OUTPOST_CUE_SHUTTLE_3 = 2,
	OUTPOST_CUE_SHUTTLE_1 = 3,
	OUTPOST_CUE_SPEECH_1 = 4,
	OUTPOST_CUE_SPEECH_2 = 5,
	OUTPOST_CUE_SPEECH_3 = 6,
	OUTPOST_CUE_SPEECH_1_REPEAT = 7,
	OUTPOST_CUE_SPEECH_2_REPEAT = 8,
	OUTPOST_CUE_FADE_MESSAGE = 9,
	OUTPOST_MESSAGE_FADE_DURATION = 120,
	OUTPOST_SPEECH_COUNT = 3
};

enum {
	OUTPOST_SCENE_5_SPEECH_1 = 54,
	OUTPOST_SCENE_5_SPEECH_2 = 55,
	OUTPOST_SCENE_5_SPEECH_3 = 56,
	OUTPOST_SCENE_6_SPEECH_1 = 57,
	OUTPOST_SCENE_6_SPEECH_2 = 58
};

enum {
	OUTPOST_MUSIC_SCENE_2_BEAT = 21,
	OUTPOST_MUSIC_SCENE_3_BEAT = 38,
	OUTPOST_MUSIC_SCENE_4_BEAT = 50,
	OUTPOST_MUSIC_SCENE_5_BEAT = 77,
	OUTPOST_MUSIC_SCENE_6_BEAT = 98,
	OUTPOST_MUSIC_FADE_DURATION = 300,
	OUTPOST_MUSIC_SCENE_5_QUIET_TIME = 12,
	OUTPOST_MUSIC_SCENE_5_LOUD_TIME = 110,
	OUTPOST_MUSIC_SCENE_6_QUIET_TIME = 30,
	OUTPOST_MUSIC_SCENE_6_LOUD_TIME = 112,
	OUTPOST_MUSIC_SCENE_5_QUIET_VOLUME = 50,
	OUTPOST_MUSIC_SCENE_6_QUIET_VOLUME = 64,
	OUTPOST_MUSIC_LOUD_VOLUME = 128,
	OUTPOST_MUSIC_CUE_FADE_DURATION = 60
};

extern XwSceneMusicHandles g_outpostMusicState;
extern Sound* g_outpostSpeech[OUTPOST_SPEECH_COUNT];
extern Rect g_outpostPreviousDirtyRect;
extern int16_t g_outpostRepeatBackground;
extern Film* g_outpostFilm;
extern Rect g_outpostCurrentDirtyRect;
extern LandruHandle g_outpostBackgroundHandle;

/* Declarations follow ascending original IDB address. */

/* 0x459D60 */
void Outpost_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x459E60 */
void Outpost_CloseMusic(void);

/* 0x459EB0 */
void Outpost_user_Music(Sound* sound, int time);

/* 0x459F30 */
void Outpost_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm);

/* 0x45A090 */
void Outpost_PlaySoundCue(int16_t cue);

/* 0x45A170 */
XwShellSceneResult Outpost_Play(struct XwShellContext* shell);

/* 0x45A6B0 */
void Outpost_end_View(int unusedTime);

/* 0x45A750 */
int16_t Outpost_film_SavedBackgroundCallback(Film* film, FilmObject* object);

/* 0x45A7D0 */
int16_t Outpost_film_NormalCallback(Film* film, FilmObject* object);

/* 0x45A810 */
int16_t Outpost_film_Actor_To_Background(Actor* actor);

/* 0x45A900 */
void Outpost_user_SoundCue(Actor* actor, int unusedTime);

/* 0x45A920 */
void Outpost_user_Background(Actor* unusedActor, int time);

/* 0x45A970 */
int16_t Outpost_draw_Background(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh);

/* 0x45AA90 */
void Outpost_user_DirtyActor(Actor* actor, int unusedTime);

/* 0x45AB10 */
void Outpost_user_Close(Actor* actor, int unusedTime);

#ifdef __cplusplus
}
#endif

#endif
