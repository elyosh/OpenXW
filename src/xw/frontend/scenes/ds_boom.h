#ifndef XW_FRONTEND_SCENES_DS_BOOM_H
#define XW_FRONTEND_SCENES_DS_BOOM_H

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

enum { DS_BOOM_BACKGROUND_COLOR = 0, DS_BOOM_BACKGROUND_Z = 200, DS_BOOM_CLOSE_Z = -200 };

extern Actor* g_dsboomBackgroundActor;
extern Actor* g_dsboomCloseActor;

enum XwDsBoomSoundCue { XW_DS_BOOM_CUE_FLYBY = 1, XW_DS_BOOM_CUE_EXPLOSION = 2 };

extern const char* g_dsboomMusicFilename;
extern const char* g_dsboomMusicName;
extern XwSceneMusicHandles g_dsboomMusicState;
extern Film* g_dsboomFilm;
extern Rect g_dsboomPreviousDirtyRect;
extern Rect g_dsboomCurrentDirtyRect;

/* Declarations follow ascending original IDB address. */

/* 0x4470B0 */
void DsBoom_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x447130 */
void DsBoom_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm);

/* 0x447170 */
void DsBoom_CloseSoundEffects(void);

/* 0x447190 */
void DsBoom_PlaySoundCue(int16_t cue);

/* 0x447E10 */
XwShellSceneResult DsBoom_Play(struct XwShellContext* shell);

/* 0x447F90 */
void DsBoom_end_View(int unusedTime);

/* 0x447FD0 */
int16_t DsBoom_film_Callback(Film* film, FilmObject* object);

/* 0x448020 */
void DsBoom_user_Sound(Actor* actor, int unusedTime);

/* 0x448040 */
void DsBoom_user_Background(Actor* unusedActor, int time);

/* 0x448090 */
int DsBoom_draw_Background(Actor* unusedActor, Rect* frame, Rect* unusedClip, int16_t unusedX,
						   int16_t unusedY, int16_t refresh);

/* 0x4480B0 */
void DsBoom_user_DirtyBounds(Actor* actor, int unusedTime);

/* 0x448120 */
void DsBoom_user_Close(Actor* actor, int unusedTime);

#ifdef __cplusplus
}
#endif

#endif
