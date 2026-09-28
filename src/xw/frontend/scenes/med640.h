#ifndef XW_FRONTEND_SCENES_MED640_H
#define XW_FRONTEND_SCENES_MED640_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/frontend/scenes/cutscene.h"
#include "xw/landru_config.h"

#include <landru/actor.h>
#include <landru/film.h>
#include <landru/rect.h>
#include <landru/res.h>
#include <stddef.h>
#include <stdint.h>
#include <xw/frontend/shell.h>

struct XwShellContext;

enum { MED640_BACKGROUND_WIDTH = 640, MED640_BACKGROUND_HEIGHT = 480 };

enum { MED640_MUSIC_START_BEAT = 99, MED640_MUSIC_FADE_DURATION = 400 };

enum {
	MED640_MOTION_SAMPLE_COUNT = 16,
	MED640_MOTION_TIME_SHIFT = 1,
	MED640_MOTION_PHASE_MASK = 2 * MED640_MOTION_SAMPLE_COUNT - 1
};

enum {
	MED640_CUE_BEEP_4 = 1,
	MED640_CUE_BEEP_5 = 2,
	MED640_CUE_BEEP_6 = 3,
	MED640_CUE_DROID = 4,
	MED640_CUE_DROID_4 = 5
};

extern const int8_t g_med640MotionXTable[MED640_MOTION_SAMPLE_COUNT];
extern const int8_t g_med640MotionYTable[MED640_MOTION_SAMPLE_COUNT];
extern XwSceneMusicHandles g_med640MusicState;
extern int16_t g_med640MotionX;
extern int16_t g_med640MotionY;
extern Rect g_med640PreviousDirtyRect;
extern Rect g_med640CurrentDirtyRect;

enum {
	MED640_BACKGROUND_Z = 200,
	MED640_CLOSE_Z = -200,
	MED640_BACKGROUND_BYTES = MED640_BACKGROUND_WIDTH * MED640_BACKGROUND_HEIGHT
};

extern Actor* g_med640BackgroundActor;
extern Actor* g_med640CloseActor;
extern Film* g_med640Film;
extern LandruHandle g_med640BackgroundHandle;

/* Declarations follow ascending original IDB address. */

/* 0x458C10 */
void Med640_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x458CB0 */
void Med640_CloseMusic(void);

/* 0x458D10 */
void Med640_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm);

/* 0x458D80 */
void Med640_PlaySoundCue(int16_t cue);

/* 0x458DF0 */
XwShellSceneResult Med640_Play(struct XwShellContext* shell);

/* 0x458FB0 */
void Med640_end_View(int time);

/* 0x459030 */
int16_t Med640_film_Callback(Film* film, FilmObject* object);

/* 0x4590C0 */
int16_t Med640_film_Actor_To_Background(Actor* actor);

/* 0x4591A0 */
void Med640_user_SoundCue(Actor* actor, int unusedTime);

/* 0x4591C0 */
void Med640_user_Background(Actor* unusedActor, int time);

/* 0x459210 */
int16_t Med640_draw_Background(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh);

/* 0x459270 */
void Med640_user_DirtyActor(Actor* actor, int time);

/* 0x459320 */
void Med640_user_Close(Actor* actor, int unusedTime);

#ifdef __cplusplus
}
#endif

#endif
