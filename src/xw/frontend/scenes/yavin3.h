#ifndef XW_FRONTEND_SCENES_YAVIN3_H
#define XW_FRONTEND_SCENES_YAVIN3_H

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

enum { XW_YAVIN3_CUE_FLYBY = 1 };

enum { YAVIN3_BACKGROUND_WIDTH = 320, YAVIN3_BACKGROUND_HEIGHT = 200 };

enum {
	YAVIN3_BACKGROUND_BYTES = YAVIN3_BACKGROUND_WIDTH * YAVIN3_BACKGROUND_HEIGHT,
	YAVIN3_BACKGROUND_Z = 200,
	YAVIN3_CLOSE_Z = -200,
	YAVIN3_FILM_SPEED_THRESHOLD = 2
};

enum {
	YAVIN3_CAPTION_START = 4,
	YAVIN3_CAPTION_END = 60,
	YAVIN3_CAPTION_FONT = 0,
	YAVIN3_CAPTION_X = 20,
	YAVIN3_CAPTION_Y = 180,
	YAVIN3_CAPTION_COLOR = 47
};

enum { YAVIN3_ACTOR_ROLE_SCROLL = 15 };

enum { YAVIN3_MUSIC_START_BEAT = 78, YAVIN3_MUSIC_FULL_LEVEL_AFTER_BEAT = 86, YAVIN3_MUSIC_FULL_LEVEL = 127 };

enum { YAVIN3_FULL_REFRESH_FRAMES = 2, YAVIN3_CLOSE_FILM_COMPLETE = 1 };

extern Rect g_yavin3CurrentDirtyRect;
extern Actor* g_yavin3BackgroundActor;
extern Actor* g_yavin3ScrollActor;
extern Film* g_yavin3Film;
extern Rect g_yavin3PreviousDirtyRect;
extern Actor* g_yavin3CloseActor;
extern LandruHandle g_yavin3BackgroundHandle;
extern int16_t g_yavin3FullRefreshCountdown;
extern int16_t g_yavin3LastDrawnX;
extern XwSceneMusicHandles g_yavin3MusicState;
extern Film* g_yavin3SoundEffectsFilm;

/* Declarations follow ascending original IDB address. */

/* 0x46AC80 */
XwShellSceneResult Yavin3_Play(struct XwShellContext* shell);

/* 0x46AE90 */
void Yavin3_end_View(int unusedTime);

/* 0x46AED0 */
int16_t Yavin3_film_Callback(Film* film, FilmObject* object);

/* 0x46AFA0 */
int16_t Yavin3_film_Actor_To_Background(Actor* actor);

/* 0x46B050 */
void Yavin3_user_SoundCue(Actor* actor, int unusedTime);

/* 0x46B070 */
int16_t Yavin3_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
							   int16_t unusedY, int16_t refresh);

/* 0x46B0F0 */
void Yavin3_user_DirtyActor(Actor* actor, int unusedTime);

/* 0x46B140 */
void Yavin3_user_Close(Actor* actor, int unusedTime);

/* 0x46B4C0 */
void Yavin3_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x46B570 */
void Yavin3_user_Music(Sound* sound, int unusedTime);

/* 0x46B5B0 */
void Yavin3_LoadSoundEffects(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x46B5E0 */
void Yavin3_PlaySoundCue(int16_t cue);

#ifdef __cplusplus
}
#endif

#endif
