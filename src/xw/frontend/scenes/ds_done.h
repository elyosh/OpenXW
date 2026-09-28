#ifndef XW_FRONTEND_SCENES_DS_DONE_H
#define XW_FRONTEND_SCENES_DS_DONE_H

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

enum { XW_DS_DONE_CUE_TIE_APPROACH = 1 };

enum { DSDONE_BACKGROUND_WIDTH = 320, DSDONE_BACKGROUND_HEIGHT = 200 };

enum { DSDONE_BACKGROUND_CACHE_SPEED_THRESHOLD = 2 };

enum { DSDONE_MUSIC_CONTROL_VALUE = 1 };

enum {
	DSDONE_BACKGROUND_BYTES = DSDONE_BACKGROUND_WIDTH * DSDONE_BACKGROUND_HEIGHT,
	DSDONE_BACKGROUND_Z = 200,
	DSDONE_CLOSE_Z = -200
};

enum {
	DSDONE_CAPTION_START = 4,
	DSDONE_CAPTION_END = 60,
	DSDONE_CAPTION_FONT = 0,
	DSDONE_CAPTION_X = 30,
	DSDONE_CAPTION_Y = 186,
	DSDONE_CAPTION_COLOR = 16
};

extern XwSceneMusicHandles g_dsdoneMusicState;
extern Actor* g_dsdoneBackgroundActor;
extern Rect g_dsdonePreviousDirtyRect;
extern Film* g_dsdoneFilm;
extern Actor* g_dsdoneCloseActor;
extern Rect g_dsdoneCurrentDirtyRect;
extern LandruHandle g_dsdoneBackgroundHandle;

/* Declarations follow ascending original IDB address. */

/* 0x442380 */
void DsDone_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x442420 */
void DsDone_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm);

/* 0x442450 */
void DsDone_PlaySoundCue(int16_t cue);

/* 0x4481B0 */
XwShellSceneResult DsDone_Play(struct XwShellContext* shell);

/* 0x4483D0 */
void DsDone_end_View(int unusedTime);

/* 0x448410 */
int16_t DsDone_film_Callback(Film* film, FilmObject* object);

/* 0x4484B0 */
int16_t DsDone_film_Actor_To_Background(Actor* actor);

/* 0x448590 */
void DsDone_user_Sound(Actor* actor, int unusedTime);

/* 0x4485B0 */
void DsDone_user_Background(Actor* unusedActor, int time);

/* 0x448600 */
int16_t DsDone_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
							   int16_t unusedY, int16_t refresh);

/* 0x448660 */
void DsDone_user_DirtyBounds(Actor* actor, int unusedTime);

/* 0x4486D0 */
void DsDone_user_Close(Actor* actor, int unusedTime);

#ifdef __cplusplus
}
#endif

#endif
