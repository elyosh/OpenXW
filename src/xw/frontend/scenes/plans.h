#ifndef XW_FRONTEND_SCENES_PLANS_H
#define XW_FRONTEND_SCENES_PLANS_H

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

enum { PLANS_BACKGROUND_WIDTH = 320, PLANS_BACKGROUND_HEIGHT = 200 };

enum { PLANS_REFRESH_FULL_CANVAS = 1 };

enum {
	PLANS_CUE_MESSAGE = 1,
	PLANS_CUE_BREATH = 2,
	PLANS_CUE_SPEECH_FIRST = 10,
	PLANS_CUE_SPEECH_SECOND = 11,
	PLANS_CUE_SPEECH_FIRST_REPEAT = 12
};

enum { PLANS_SPEECH_FIRST = 0, PLANS_SPEECH_SECOND = 1, PLANS_SPEECH_COUNT = 2 };

enum {
	PLANS_MUSIC_SECOND_START_BEAT = 22,
	PLANS_MUSIC_VADER_START_BEAT = 54,
	PLANS_MUSIC_CHANNEL = 3,
	PLANS_MUSIC_FADE_DURATION = 800,
	PLANS_MUSIC_SECOND_QUIET_CEL = 1,
	PLANS_MUSIC_SECOND_LOUD_CEL = 45,
	PLANS_MUSIC_VADER_QUIET_CEL = 30,
	PLANS_MUSIC_VADER_LOUD_CEL = 60,
	PLANS_MUSIC_QUIET_VOLUME = 84,
	PLANS_MUSIC_LOUD_VOLUME = 128,
	PLANS_MUSIC_LEVEL_DURATION = 120
};

enum {
	PLANS_BACKGROUND_Z = 200,
	PLANS_CLOSE_Z = -200,
	PLANS_SPEED_THRESHOLD = 1,
	PLANS_CAPTION_FONT = 0,
	PLANS_INTRO_FIRST_START = 4,
	PLANS_INTRO_FIRST_END = 80,
	PLANS_INTRO_FIRST_X = 5,
	PLANS_INTRO_FIRST_Y = 6,
	PLANS_INTRO_FIRST_COLOR = 16,
	PLANS_INTRO_SECOND_START = 4,
	PLANS_INTRO_SECOND_END = 80,
	PLANS_INTRO_SECOND_X = 5,
	PLANS_INTRO_SECOND_Y = 16,
	PLANS_INTRO_SECOND_COLOR = 16,
	PLANS_PLANS_START = 4,
	PLANS_PLANS_END = 30,
	PLANS_PLANS_X = 80,
	PLANS_PLANS_Y = 10,
	PLANS_PLANS_COLOR = 51,
	PLANS_LEIA_START = 31,
	PLANS_LEIA_END = 70,
	PLANS_LEIA_X = 10,
	PLANS_LEIA_Y = 24,
	PLANS_LEIA_COLOR = 14,
	PLANS_VADER_START = 36,
	PLANS_VADER_END = 70,
	PLANS_VADER_X = 10,
	PLANS_VADER_Y = 50,
	PLANS_VADER_COLOR = 50
};

extern Actor* g_plansBackgroundActor;
extern Actor* g_plansCloseActor;
extern Film* g_plansFilm;
extern Rect g_plansPreviousDirtyRect;
extern Rect g_plansCurrentDirtyRect;
extern LandruHandle g_plansBackgroundHandle;
extern XwSceneMusicHandles g_plansMusicState;
extern Sound* g_plansSpeech[PLANS_SPEECH_COUNT];

/* Declarations follow ascending original IDB address. */

/* 0x45AF20 */
XwShellSceneResult Plans_Play(struct XwShellContext* shell);

/* 0x45B250 */
void Plans_end_View(int unusedTime);

/* 0x45B2C0 */
int16_t Plans_film_SavedBackgroundCallback(Film* film, FilmObject* object);

/* 0x45B340 */
int16_t Plans_film_NormalCallback(Film* film, FilmObject* object);

/* 0x45B380 */
int16_t Plans_film_Actor_To_Background(Actor* actor);

/* 0x45B460 */
void Plans_user_SoundCue(Actor* actor, int unusedTime);

/* 0x45B480 */
void Plans_user_Background(Actor* unusedActor, int time);

/* 0x45B4D0 */
int16_t Plans_draw_Background(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh);

/* 0x45B530 */
void Plans_user_DirtyActor(Actor* actor, int unusedTime);

/* 0x45B5B0 */
void Plans_user_Close(Actor* actor, int unusedTime);

/* 0x45B660 */
void Plans_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x45B740 */
void Plans_CloseMusic(void);

/* 0x45B7A0 */
void Plans_user_Music(Sound* unusedSound, int unusedTime);

/* 0x45B870 */
void Plans_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm);

/* 0x45B930 */
void Plans_PlaySoundCue(int16_t cue);

#ifdef __cplusplus
}
#endif

#endif
