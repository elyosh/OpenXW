#ifndef XW_FRONTEND_SCENES_DEATH640_H
#define XW_FRONTEND_SCENES_DEATH640_H

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

enum { XW_DEATH640_CUE_EXPLOSION = 1 };

enum { DEATH640_BACKGROUND_WIDTH = 640, DEATH640_BACKGROUND_HEIGHT = 480 };

enum { DEATH640_REFRESH_FULL_CANVAS = 1 };

enum {
	DEATH640_MUSIC_INTERIOR_BEAT = 19,
	DEATH640_MUSIC_EXTERIOR_BEAT = 35,
	DEATH640_MUSIC_FADE_DURATION = 300
};

enum {
	DEATH640_BACKGROUND_BYTES = DEATH640_BACKGROUND_WIDTH * DEATH640_BACKGROUND_HEIGHT,
	DEATH640_BACKGROUND_Z = 200,
	DEATH640_CLOSE_Z = -200
};

extern const char* g_death640ResourceFilename;
extern const char* g_death640InteriorFilmName;
extern const char* g_death640ExteriorFilmName;
extern const char* g_death640PlanetFilmName;
extern XwSceneMusicHandles g_death640MusicState;
extern Actor* g_death640BackgroundActor;
extern Rect g_death640PreviousDirtyRect;
extern Actor* g_death640CloseActor;
extern Rect g_death640CurrentDirtyRect;
extern Film* g_death640Film;
extern LandruHandle g_death640BackgroundHandle;

/* Declarations follow ascending original IDB address. */

/* 0x44A9D0 */
void Death640_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x44AA90 */
void Death640_CloseMusic(void);

/* 0x44AAF0 */
void Death640_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm);

/* 0x44AB20 */
void Death640_PlaySoundCue(int16_t cue);

/* 0x44AB40 */
XwShellSceneResult Death640_Play(struct XwShellContext* shell);

/* 0x44AD70 */
void Death640_end_View(int unusedTime);

/* 0x44ADE0 */
int16_t Death640_film_InteriorCallback(Film* film, FilmObject* object);

/* 0x44AE50 */
int16_t Death640_film_ExteriorCallback(Film* film, FilmObject* object);

/* 0x44AE90 */
int16_t Death640_film_Actor_To_Background(Actor* actor);

/* 0x44AF70 */
void Death640_user_Sound(Actor* actor, int unusedTime);

/* 0x44AF90 */
void Death640_user_Background(Actor* unusedActor, int time);

/* 0x44AFE0 */
int16_t Death640_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
								 int16_t unusedY, int16_t refresh);

/* 0x44B040 */
void Death640_user_DirtyBounds(Actor* actor, int unusedTime);

/* 0x44B0C0 */
void Death640_user_Close(Actor* actor, int unusedTime);

#ifdef __cplusplus
}
#endif

#endif
