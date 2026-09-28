#ifndef XW_FRONTEND_SCENES_LEAVE2_H
#define XW_FRONTEND_SCENES_LEAVE2_H

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

enum { XW_LEAVE2_CUE_FLYBY = 1 };

enum { LEAVE2_BACKGROUND_WIDTH = 320, LEAVE2_BACKGROUND_HEIGHT = 200 };

enum { LEAVE2_MUSIC_FADE_CEL = 50, LEAVE2_MUSIC_FADE_DURATION = 180 };

enum { LEAVE2_MUSIC_START_BEAT = 78, LEAVE2_MUSIC_START_TICK = 400 };

extern const char* g_leave2MusicFilename;
extern const char* g_leave2MusicName;

enum {
	LEAVE2_BACKGROUND_Z = 200,
	LEAVE2_CLOSE_Z = -200,
	LEAVE2_FILM_SPEED_THRESHOLD = 2,
	LEAVE2_BACKGROUND_BYTES = LEAVE2_BACKGROUND_WIDTH * LEAVE2_BACKGROUND_HEIGHT
};

extern Actor* g_leave2BackgroundActor;
extern Actor* g_leave2CloseActor;
extern Film* g_leave2Film;
extern Rect g_leave2PreviousDirtyRect;
extern Rect g_leave2CurrentDirtyRect;
extern LandruHandle g_leave2BackgroundHandle;
extern XwSceneMusicHandles g_leave2MusicState;

/* Declarations follow ascending original IDB address. */

/* 0x4551A0 */
XwShellSceneResult Leave2_Play(struct XwShellContext* shell);

/* 0x4553A0 */
void Leave2_end_View(int unusedTime);

/* 0x4553F0 */
int16_t Leave2_film_Callback(Film* film, FilmObject* object);

/* 0x455470 */
int16_t Leave2_film_Actor_To_Background(Actor* actor);

/* 0x455550 */
void Leave2_user_Sound(Actor* actor, int unusedTime);

/* 0x455570 */
void Leave2_user_Background(Actor* unusedActor, int time);

/* 0x4555C0 */
int16_t Leave2_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
							   int16_t unusedY, int16_t refresh);

/* 0x455620 */
void Leave2_user_DirtyBounds(Actor* actor, int unusedTime);

/* 0x455690 */
void Leave2_user_Close(Actor* actor, int unusedTime);

/* 0x457D10 */
void Leave2_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x457DC0 */
void Leave2_user_Music(Sound* unusedSound, int unusedTime);

/* 0x457E00 */
void Leave2_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm);

/* 0x457E30 */
void Leave2_PlaySoundCue(int16_t cue);

#ifdef __cplusplus
}
#endif

#endif
