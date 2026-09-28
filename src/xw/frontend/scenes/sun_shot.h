#ifndef XW_FRONTEND_SCENES_SUN_SHOT_H
#define XW_FRONTEND_SCENES_SUN_SHOT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/landru_config.h"

#include <landru/actor.h>
#include <landru/film.h>
#include <landru/memhdl.h>
#include <landru/rect.h>
#include <stddef.h>
#include <stdint.h>
#include <xw/frontend/shell.h>

struct XwShellContext;

enum { SUNSHOT_SOUND_ACTION_FIRST = 1, SUNSHOT_SOUND_ACTION_SECOND = 2 };

enum { SUNSHOT_BACKGROUND_WIDTH = 640, SUNSHOT_BACKGROUND_HEIGHT = 480 };

enum {
	SUNSHOT_ACTOR_ROLE_BACKGROUND = 10,
	SUNSHOT_CLOSE_Z = -200,
	SUNSHOT_BACKGROUND_BYTES = SUNSHOT_BACKGROUND_WIDTH * SUNSHOT_BACKGROUND_HEIGHT
};

extern const char g_sunshotResourceFilename[12];
extern const char g_sunshotFilmName[9];
extern Actor* g_sunshotCloseActor;

extern Film* g_sunshotFilm;
extern LandruHandle g_sunshotBackgroundHandle;
/* Declarations follow ascending original IDB address. */

/* 0x4331B0 */
XwShellSceneResult SunShot_Play(struct XwShellContext* context);

/* 0x433330 */
void SunShot_end_View(int time);

/* 0x433370 */
int16_t SunShot_film_Callback(Film* film, FilmObject* object);

/* 0x4333D0 */
int16_t SunShot_ActorToBackground(Actor* actor);

/* 0x4334B0 */
void SunShot_user_Sound(Actor* actor, int time);

/* 0x4334D0 */
int16_t SunShot_draw_Background(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh);

/* 0x433530 */
void SunShot_user_Close(Actor* actor, int time);

/* 0x434D90 */
void SunShot_LoadSounds(void);

/* 0x434DE0 */
void SunShot_PlaySoundAction(int16_t action);

#ifdef __cplusplus
}
#endif

#endif
