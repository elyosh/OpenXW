#ifndef XW_FRONTEND_SCENES_DS_LAND_H
#define XW_FRONTEND_SCENES_DS_LAND_H

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

enum { XW_DS_LAND_CUE_SHUTTLE = 1 };

enum { DSLAND_BACKGROUND_WIDTH = 320, DSLAND_BACKGROUND_HEIGHT = 200 };

enum { DSLAND_MUSIC_START_BEAT = 61 };

extern XwSceneMusicHandles g_dslandMusicState;

enum {
	DSLAND_BACKGROUND_Z = 200,
	DSLAND_CLOSE_Z = -200,
	DSLAND_FILM_SPEED_THRESHOLD = 1,
	DSLAND_BACKGROUND_BYTES = DSLAND_BACKGROUND_WIDTH * DSLAND_BACKGROUND_HEIGHT,
	DSLAND_CAPTION_START = 4,
	DSLAND_CAPTION_END = 60,
	DSLAND_CAPTION_FONT = 0,
	DSLAND_CAPTION_X = 40,
	DSLAND_CAPTION_Y = 20,
	DSLAND_CAPTION_COLOR = 47
};

extern Actor* g_dslandBackgroundActor;
extern Actor* g_dslandCloseActor;
extern Film* g_dslandFilm;
extern Rect g_dslandPreviousDirtyRect;
extern Rect g_dslandCurrentDirtyRect;
extern LandruHandle g_dslandBackgroundHandle;

/* Declarations follow ascending original IDB address. */

/* 0x446FC0 */
void DsLand_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x447060 */
void DsLand_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm);

/* 0x447090 */
void DsLand_PlaySoundCue(int16_t cue);

/* 0x448770 */
XwShellSceneResult DsLand_Play(struct XwShellContext* shell);

/* 0x448960 */
void DsLand_end_View(int unusedTime);

/* 0x4489A0 */
int16_t DsLand_film_Callback(Film* film, FilmObject* object);

/* 0x448A20 */
int16_t DsLand_film_Actor_To_Background(Actor* actor);

/* 0x448B00 */
void DsLand_user_Background(Actor* unusedActor, int time);

/* 0x448B50 */
int16_t DsLand_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
							   int16_t unusedY, int16_t refresh);

/* 0x448BB0 */
void DsLand_user_DirtyBounds(Actor* actor, int unusedTime);

/* 0x448C20 */
void DsLand_user_Close(Actor* actor, int unusedTime);

/* 0x44A690 */
void DsLand_user_Sound(Actor* actor, int unusedTime);

#ifdef __cplusplus
}
#endif

#endif
