#ifndef XW_FRONTEND_SCENES_DS_BAY_H
#define XW_FRONTEND_SCENES_DS_BAY_H

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

enum { DSBAY_MUSIC_CONTROL_CEL = 40, DSBAY_MUSIC_CONTROL_VALUE = 5, DSBAY_BACKGROUND_RESTORE_FRAMES = 2 };

enum { DSBAY_MUSIC_START_BEAT = 27 };

enum { DSBAY_BACKGROUND_SCROLL_CONTROLLER = 1 };

enum {
	DSBAY_BACKGROUND_TILE_COUNT = 2,
	DSBAY_BACKGROUND_TILE_WIDTH = 320,
	DSBAY_BACKGROUND_TILE_HEIGHT = 200
};

enum {
	DSBAY_FRAME_WIDTH = 640,
	DSBAY_FRAME_HEIGHT = 480,
	DSBAY_BACKGROUND_TILE_BYTES = DSBAY_BACKGROUND_TILE_WIDTH * DSBAY_BACKGROUND_TILE_HEIGHT,
	DSBAY_BACKGROUND_Z = 20,
	DSBAY_CLOSE_Z = -100,
	DSBAY_FILM_SPEED_THRESHOLD = 2
};

extern XwSceneMusicHandles g_dsbayMusicState;
extern Film* g_dsbayFilm;
extern Rect g_dsbayCurrentDirtyRect;
extern Actor* g_dsbayBackgroundActor;
extern Actor* g_dsbayScrollActor;
extern Rect g_dsbayRestoreRect;
extern Actor* g_dsbayCloseActor;
extern LandruHandle g_dsbayBackgroundHandles[DSBAY_BACKGROUND_TILE_COUNT];

/* Declarations follow ascending original IDB address. */

/* 0x4422B0 */
void DsBay_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x442350 */
void DsBay_user_Music(Sound* unusedSound, int unusedTime);

/* 0x447850 */
XwShellSceneResult DsBay_Play(struct XwShellContext* shell);

/* 0x447A60 */
void DsBay_end_View(int unusedTime);

/* 0x447AC0 */
int16_t DsBay_film_Callback(Film* film, FilmObject* object);

/* 0x447B40 */
int16_t DsBay_film_Actor_To_Background(Actor* actor);

/* 0x447C20 */
void DsBay_user_DirtyBounds(Actor* actor, int unusedTime);

/* 0x447C70 */
void DsBay_user_Close(Actor* actor, int unusedTime);

/* 0x447CF0 */
void DsBay_user_Background(Actor* actor, int unusedTime);

/* 0x447D40 */
int16_t DsBay_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
							  int16_t unusedY, int16_t refresh);

#ifdef __cplusplus
}
#endif

#endif
