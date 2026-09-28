#ifndef XW_FRONTEND_SCENES_YAVIN1_H
#define XW_FRONTEND_SCENES_YAVIN1_H

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

enum { XW_YAVIN1_CUE_FLYBY = 1 };

enum { YAVIN1_MUSIC_START_BEAT = 32 };

enum { YAVIN1_BACKGROUND_WIDTH = 320, YAVIN1_BACKGROUND_HEIGHT = 200 };

enum {
	YAVIN1_BACKGROUND_Z = 200,
	YAVIN1_CLOSE_Z = -200,
	YAVIN1_FILM_SPEED_THRESHOLD = 2,
	YAVIN1_BACKGROUND_BYTES = YAVIN1_BACKGROUND_WIDTH * YAVIN1_BACKGROUND_HEIGHT,
	YAVIN1_CAPTION_START = 4,
	YAVIN1_CAPTION_END = 40,
	YAVIN1_CAPTION_FONT = 0,
	YAVIN1_CAPTION_X = 80,
	YAVIN1_CAPTION_Y = 180,
	YAVIN1_CAPTION_COLOR = 16
};

extern Actor* g_yavin1BackgroundActor;
extern Actor* g_yavin1CloseActor;
extern Film* g_yavin1Film;
extern Rect g_yavin1PreviousDirtyRect;
extern Rect g_yavin1CurrentDirtyRect;
extern LandruHandle g_yavin1BackgroundHandle;
extern XwSceneMusicHandles g_yavin1MusicState;
extern Film* g_yavin1SoundEffectsFilm;

/* Declarations follow ascending original IDB address. */

/* 0x46A190 */
XwShellSceneResult Yavin1_Play(struct XwShellContext* shell);

/* 0x46A390 */
void Yavin1_end_View(int unusedTime);

/* 0x46A3D0 */
int16_t Yavin1_film_Callback(Film* film, FilmObject* object);

/* 0x46A450 */
int16_t Yavin1_film_Actor_To_Background(Actor* actor);

/* 0x46A530 */
void Yavin1_user_SoundCue(Actor* actor, int unusedTime);

/* 0x46A550 */
void Yavin1_user_Background(Actor* unusedActor, int time);

/* 0x46A5A0 */
int16_t Yavin1_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
							   int16_t unusedY, int16_t refresh);

/* 0x46A600 */
void Yavin1_user_DirtyActor(Actor* actor, int unusedTime);

/* 0x46A670 */
void Yavin1_user_Close(Actor* actor, int unusedTime);

/* 0x46B260 */
void Yavin1_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x46B2F0 */
void Yavin1_LoadSoundEffects(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x46B320 */
void Yavin1_PlaySoundCue(int16_t cue);

#ifdef __cplusplus
}
#endif

#endif
