#ifndef XW_FRONTEND_SCENES_YAVIN2_H
#define XW_FRONTEND_SCENES_YAVIN2_H

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

enum { YAVIN2_BACKGROUND_WIDTH = 320, YAVIN2_BACKGROUND_HEIGHT = 200 };

enum {
	YAVIN2_MUSIC_START_BEAT = 50,
	YAVIN2_MUSIC_CLOSE_CONTROL = 2,
	YAVIN2_MUSIC_CLOSE_MARKER = 1,
	YAVIN2_MUSIC_CLOSE_HOOK = 1,
	YAVIN2_MUSIC_CLOSE_VOLUME = 1,
	YAVIN2_MUSIC_CLOSE_FADE_DURATION = 300
};

enum {
	YAVIN2_BACKGROUND_Z = 200,
	YAVIN2_CLOSE_Z = -200,
	YAVIN2_FILM_SPEED_THRESHOLD = 2,
	YAVIN2_BACKGROUND_BYTES = YAVIN2_BACKGROUND_WIDTH * YAVIN2_BACKGROUND_HEIGHT
};

extern Actor* g_yavin2BackgroundActor;
extern Actor* g_yavin2CloseActor;
extern Film* g_yavin2Film;
extern Rect g_yavin2PreviousDirtyRect;
extern Rect g_yavin2CurrentDirtyRect;
extern LandruHandle g_yavin2BackgroundHandle;
extern XwSceneMusicHandles g_yavin2MusicState;
extern Film* g_yavin2SoundEffectsFilm;

/* Declarations follow ascending original IDB address. */

/* 0x46A740 */
XwShellSceneResult Yavin2_Play(struct XwShellContext* shell);

/* 0x46A920 */
void Yavin2_end_View(int unusedTime);

/* 0x46A960 */
int16_t Yavin2_film_Callback(Film* film, FilmObject* object);

/* 0x46A9E0 */
int16_t Yavin2_film_Actor_To_Background(Actor* actor);

/* 0x46AAC0 */
void Yavin2_user_Background(Actor* unusedActor, int time);

/* 0x46AB10 */
int16_t Yavin2_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
							   int16_t unusedY, int16_t refresh);

/* 0x46AB70 */
void Yavin2_user_DirtyActor(Actor* actor, int unusedTime);

/* 0x46ABE0 */
void Yavin2_user_Close(Actor* actor, int unusedTime);

/* 0x46B340 */
void Yavin2_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x46B3D0 */
void Yavin2_CloseMusic(void);

/* 0x46B470 */
void Yavin2_LoadSoundEffects(ResFile* unusedResourceFile, Film* sceneFilm);

#ifdef __cplusplus
}
#endif

#endif
