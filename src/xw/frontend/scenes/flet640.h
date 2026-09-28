#ifndef XW_FRONTEND_SCENES_FLET640_H
#define XW_FRONTEND_SCENES_FLET640_H

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
	FLET640_LEGACY_BACKGROUND_WIDTH = 320,
	FLET640_LEGACY_BACKGROUND_HEIGHT = 200,
	FLET640_CAPTION_FONT = 3,
	FLET640_CAPTION_START = 4,
	FLET640_CAPTION_END = 60,
	FLET640_CAPTION_CENTER = 320,
	FLET640_CAPTION_Y = 450,
	FLET640_DEFIANCE_COLOR = 239,
	FLET640_SALVATION_COLOR = 240,
	FLET640_EXECUTOR_COLOR = 242,
	FLET640_PLANS_FONT = 0,
	FLET640_PLANS_X = 50,
	FLET640_PLANS_Y = 180,
	FLET640_PLANS_COLOR = 16,
	FLET640_BACKGROUND_Z = 200,
	FLET640_CLOSE_Z = -200
};

extern const char* g_flet640ResourceFilename;
extern const char* g_flet640RebelSlowFilmName;
extern const char* g_flet640RebelFastFilmName;
extern const char* g_flet640ImperialSlowFilmName;
extern const char* g_flet640ImperialFastFilmName;
extern const char* g_flet640PlansSlowFilmName;
extern const char* g_flet640PlansFastFilmName;
extern const char* g_flet640DefianceSlowFilmName;
extern const char* g_flet640DefianceFastFilmName;
extern Actor* g_flet640BackgroundActor;
extern Actor* g_flet640CloseActor;

enum { FLET640_BACKGROUND_WIDTH = 640, FLET640_BACKGROUND_HEIGHT = 480 };

enum { FLET640_BACKGROUND_CACHE_SPEED_THRESHOLD = 1 };

enum { FLET640_INVALIDATE_SCREEN_DIFF = 1 };

enum {
	FLET640_MUSIC_DEFIANCE_BEAT = 29,
	FLET640_MUSIC_RESCUE_BEAT = 71,
	FLET640_MUSIC_CAPTURE_BEAT = 69,
	FLET640_MUSIC_PLANS_BEAT = 36,
	FLET640_MUSIC_FADE_DURATION = 120,
	FLET640_MUSIC_CLOSE_PRIORITY = 125
};

extern Film* g_flet640Film;
extern Rect g_flet640PreviousDirtyRect;
extern Rect g_flet640CurrentDirtyRect;
extern LandruHandle g_flet640BackgroundHandle;
extern XwSceneMusicHandles g_flet640MusicState;

/* Declarations follow ascending original IDB address. */

/* 0x449EE0 */
XwShellSceneResult Flet640_Play(struct XwShellContext* shell);

/* 0x44A400 */
void Flet640_end_View(int unusedTime);

/* 0x44A510 */
int16_t Flet640_film_Callback(Film* film, FilmObject* object);

/* 0x44A5B0 */
int16_t Flet640_film_Actor_To_Background(Actor* actor);

/* 0x44A6B0 */
void Flet640_user_Background(Actor* unusedActor, int time);

/* 0x44A700 */
int16_t Flet640_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
								int16_t unusedY, int16_t refresh);

/* 0x44A760 */
void Flet640_user_DirtyBounds(Actor* actor, int unusedTime);

/* 0x44A7E0 */
void Flet640_user_Close(Actor* actor, int unusedTime);

/* 0x44A880 */
void Flet640_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x44A980 */
void Flet640_CloseMusic(void);

#ifdef __cplusplus
}
#endif

#endif
