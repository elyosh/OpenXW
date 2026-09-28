#ifndef XW_FRONTEND_SCENES_B2_640_H
#define XW_FRONTEND_SCENES_B2_640_H

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

enum {
	B2_640_MUSIC_START_BEAT = 19,
	B2_640_BACKGROUND_WIDTH = 320,
	B2_640_BACKGROUND_HEIGHT = 200,
	B2_640_TILE_ROWS = 2,
	B2_640_TILE_COLUMNS = 2
};

enum { B2_640_BACKGROUND_CONSUME = 0, B2_640_BACKGROUND_TILED = 1 };

enum {
	B2_640_CUE_TIE_APPROACH = 1,
	B2_640_CUE_TORPEDO = 2,
	B2_640_CUE_EXPLOSION = 3,
	B2_640_CUE_DISTANT_EXPLOSION = 4,
	B2_640_CUE_FLYBY = 5
};

enum {
	B2_640_SCENE_WIDTH = 640,
	B2_640_SCENE_HEIGHT = 480,
	B2_640_BACKGROUND_BYTES = B2_640_SCENE_WIDTH * B2_640_SCENE_HEIGHT,
	B2_640_CLOSE_Z = -200,
	B2_640_CAPTION_FONT = 3,
	B2_640_CAPTION_COLOR = 176,
	B2_640_FIRST_CAPTION_START = 25,
	B2_640_FIRST_CAPTION_END = 35,
	B2_640_FIRST_CAPTION_X = 35,
	B2_640_FIRST_CAPTION_Y = 20,
	B2_640_SECOND_CAPTION_START = 37,
	B2_640_SECOND_CAPTION_END = 46,
	B2_640_SECOND_CAPTION_X = 540,
	B2_640_SECOND_CAPTION_Y = 450
};

extern const char* g_b2ResourceFilename;
extern const char* g_b2FilmName;
extern Actor* g_b2CloseActor;
extern const char* g_b2MusicFilename;
extern const char* g_b2MusicName;
extern Film* g_b2Film;
extern LandruHandle g_b2BackgroundHandle;
extern XwSceneMusicHandles g_b2MusicState;
extern Film* g_b2SoundFilm;
/* Declarations follow ascending original IDB address. */

/* 0x433560 */
XwShellSceneResult B2_640_Play(struct XwShellContext* shell);

/* 0x433710 */
void B2_640_end_View(int unusedTime);

/* 0x433770 */
int16_t B2_640_film_Callback(Film* film, FilmObject* object);

/* 0x4337F0 */
int16_t B2_640_film_Actor_To_Background(Actor* actor);

/* 0x4338D0 */
void B2_640_user_Sound(Actor* actor, int unusedTime);

/* 0x4338F0 */
int16_t B2_640_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t x, int16_t y,
							   int16_t refresh);

/* 0x4339D0 */
void B2_640_user_Close(Actor* actor, int unusedTime);

/* 0x434E30 */
void B2_640_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x434ED0 */
void B2_640_OpenSounds(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x434F50 */
void B2_640_PlaySoundCue(int16_t cue);

#ifdef __cplusplus
}
#endif

#endif
