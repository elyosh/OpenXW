#ifndef XW_FRONTEND_SCENES_TORT640_H
#define XW_FRONTEND_SCENES_TORT640_H

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

enum {
	TORT640_RESOURCE_COUNT = 3,
	TORT640_RESOURCE_FILE = 0,
	TORT640_SLOW_FILM = 1,
	TORT640_FAST_FILM = 2,
	TORT640_CAPTION_PREFIX_CAPACITY = 33,
	TORT640_CAPTION_CAPACITY = 100,
	TORT640_SPEED_THRESHOLD = 2,
	TORT640_CAPTION_FONT = 3,
	TORT640_CAPTION_CENTER = 320,
	TORT640_CAPTION_Y = 450,
	TORT640_CAPTION_COLOR = 15,
	TORT640_SLOW_CAPTION_START = 30,
	TORT640_SLOW_CAPTION_END = 100,
	TORT640_FAST_CAPTION_START = 70,
	TORT640_FAST_CAPTION_END = 140,
	TORT640_FAST_VERTICAL_OFFSET = 101,
	TORT640_BACKGROUND_Z = 200,
	TORT640_CLOSE_Z = -200
};

extern const char* g_tort640ResourceNames[TORT640_RESOURCE_COUNT];
extern char g_tort640CaptionPrefix[TORT640_CAPTION_PREFIX_CAPACITY];
extern Actor* g_tort640BackgroundActor;
extern Actor* g_tort640CloseActor;

enum { TORT640_SCROLL_FULL_REFRESH = 1, TORT640_SCROLL_ACTOR = 1 };

enum { TORT640_BACKGROUND_WIDTH = 640, TORT640_BACKGROUND_HEIGHT = 480 };

enum {
	TORT640_MUSIC_START_BEAT = 87,
	TORT640_MUSIC_FADE_DURATION = 300,
	TORT640_MUSIC_QUIET_CEL = 70,
	TORT640_MUSIC_LOUD_CEL = 111,
	TORT640_MUSIC_QUIET_VOLUME = 84,
	TORT640_MUSIC_LOUD_VOLUME = 128,
	TORT640_MUSIC_CUE_FADE_DURATION = 120
};

enum {
	TORT640_CUE_BREATH = 1,
	TORT640_CUE_HUM = 2,
	TORT640_CUE_PAIN_TOOL = 3,
	TORT640_CUE_SPEECH = 4,
	TORT640_CUE_FADE_HUM = 10,
	TORT640_HUM_FADE_DURATION = 60,
	TORT640_BREATH_INTERVAL = 40
};

extern XwSceneMusicHandles g_tort640MusicState;
extern int16_t g_tort640VerticalOffset;
extern Film* g_tort640Film;
extern Rect g_tort640PreviousDirtyRect;
extern Actor* g_tort640ScrollActor;
extern Rect g_tort640CurrentDirtyRect;
extern LandruHandle g_tort640BackgroundHandle;
extern Sound* g_tort640Speech;

/* Declarations follow ascending original IDB address. */

/* 0x462900 */
void Tort640_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x4629A0 */
void Tort640_CloseMusic(void);

/* 0x4629F0 */
void Tort640_user_Music(Sound* unusedSound, int unusedTime);

/* 0x462A50 */
XwShellSceneResult Tort640_Play(struct XwShellContext* shell);

/* 0x462DB0 */
void Tort640_end_View(int time);

/* 0x462E20 */
int16_t Tort640_film_Callback(Film* film, FilmObject* object);

/* 0x462EA0 */
int16_t Tort640_film_Actor_To_Background(Actor* actor);

/* 0x462F90 */
void Tort640_user_SoundCue(Actor* actor, int unusedTime);

/* 0x462FB0 */
/* The scroll actor must exist for nonzero time. */
void Tort640_user_Background(Actor* unusedActor, int time);

/* 0x463000 */
int16_t Tort640_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
								int16_t unusedY, int16_t refresh);

/* 0x4630E0 */
void Tort640_user_DirtyActor(Actor* actor, int unusedTime);

/* 0x463150 */
void Tort640_user_Close(Actor* actor, int unusedTime);

/* 0x4631F0 */
void Tort640_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm);

/* 0x463260 */
void Tort640_PlaySoundCue(int16_t cue);

#ifdef __cplusplus
}
#endif

#endif
