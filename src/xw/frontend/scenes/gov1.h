#ifndef XW_FRONTEND_SCENES_GOV1_H
#define XW_FRONTEND_SCENES_GOV1_H

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

enum { XW_GOV1_CUE_TIE_APPROACH = 1, XW_GOV1_MUSIC_CONTROL_CEL = 95 };

enum { GOV1_BACKGROUND_WIDTH = 320, GOV1_BACKGROUND_HEIGHT = 200, GOV1_TILE_ROWS = 2, GOV1_TILE_COLUMNS = 2 };

enum { GOV1_BACKGROUND_CONSUME = 0, GOV1_BACKGROUND_TILED = 1 };

enum {
	GOV1_CLOSE_Z = -200,
	GOV1_BACKGROUND_BYTES = GOV1_BACKGROUND_WIDTH * GOV1_BACKGROUND_HEIGHT,
	GOV1_FILM_SPEED_THRESHOLD = 2,
	GOV1_CAPTION_START = 4,
	GOV1_CAPTION_END = 52,
	GOV1_CAPTION_FONT = 0,
	GOV1_CAPTION_X = 60,
	GOV1_CAPTION_Y = 10,
	GOV1_CAPTION_COLOR = 16
};

extern Actor* g_gov1CloseActor;
extern Film* g_gov1Film;
extern LandruHandle g_gov1BackgroundHandle;
extern XwSceneMusicHandles g_gov1MusicState;
/* Declarations follow ascending original IDB address. */

/* 0x44B2D0 */
XwShellSceneResult Gov1_Play(struct XwShellContext* shell);

/* 0x44B490 */
void Gov1_end_View(int unusedTime);

/* 0x44B4D0 */
int16_t Gov1_film_Callback(Film* film, FilmObject* object);

/* 0x44B540 */
int16_t Gov1_film_Actor_To_Background(Actor* actor);

/* 0x44B620 */
void Gov1_user_Sound(Actor* actor, int unusedTime);

/* 0x44B640 */
int16_t Gov1_draw_TiledBackground(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t x,
								  int16_t y, int16_t refresh);

/* 0x44B720 */
void Gov1_user_Close(Actor* actor, int unusedTime);

/* 0x44C040 */
void Gov1_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x44C0D0 */
void Gov1_user_Music(Sound* unusedSound, int unusedTime);

/* 0x44C110 */
void Gov1_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm);

/* 0x44C140 */
void Gov1_PlaySoundCue(int16_t cue);

#ifdef __cplusplus
}
#endif

#endif
