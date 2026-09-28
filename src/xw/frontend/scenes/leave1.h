#ifndef XW_FRONTEND_SCENES_LEAVE1_H
#define XW_FRONTEND_SCENES_LEAVE1_H

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

enum { XW_LEAVE1_CUE_FLYBY = 1, XW_LEAVE1_MUSIC_CONTROL_CEL = 1 };

enum { XW_LEAVE1_TRANSITION_BEAT = 78, XW_LEAVE1_START_BEAT = 12, XW_LEAVE1_START_TICK = 400 };

extern const char* g_leave1MusicFilename;
extern const char* g_leave1MusicName;

enum { LEAVE1_BACKGROUND_Z = 200, LEAVE1_CLOSE_Z = -200 };

extern Actor* g_leave1BackgroundActor;
extern Actor* g_leave1CloseActor;
extern Film* g_leave1Film;
extern Rect g_leave1PreviousDirtyRect;
extern Rect g_leave1CurrentDirtyRect;
extern XwSceneMusicHandles g_leave1MusicState;

/* Declarations follow ascending original IDB address. */

/* 0x454DA0 */
XwShellSceneResult Leave1_Play(struct XwShellContext* shell);

/* 0x454F50 */
void Leave1_end_View(int unusedTime);

/* 0x454F90 */
int16_t Leave1_film_Callback(Film* film, FilmObject* object);

/* 0x454FE0 */
void Leave1_user_Sound(Actor* actor, int unusedTime);

/* 0x455000 */
void Leave1_user_Background(Actor* unusedActor, int time);

/* 0x455090 */
void Leave1_user_DirtyBounds(Actor* actor, int unusedTime);

/* 0x455100 */
void Leave1_user_Close(Actor* actor, int unusedTime);

/* 0x457B90 */
void Leave1_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x457C50 */
void Leave1_TransitionMusic(void);

/* 0x457C90 */
void Leave1_user_Music(Sound* unusedSound, int unusedTime);

/* 0x457CC0 */
void Leave1_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm);

/* 0x457CF0 */
void Leave1_PlaySoundCue(int16_t cue);

#ifdef __cplusplus
}
#endif

#endif
