#ifndef XW_FRONTEND_SCENES_B4_640_H
#define XW_FRONTEND_SCENES_B4_640_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/frontend/scenes/cutscene.h"
#include "xw/landru_config.h"

#include <landru/actor.h>
#include <landru/film.h>
#include <landru/res.h>
#include <stddef.h>
#include <stdint.h>
#include <xw/frontend/shell.h>

struct XwShellContext;

enum { B4_SCENE_WIDTH = 640, B4_SCENE_HEIGHT = 480, B4_CLOSE_Z = -200 };

extern const char* g_b4ResourceFilename;
extern const char* g_b4FilmName;
extern Actor* g_b4CloseActor;
extern LandruHandle g_b4LegacyBufferHandle;
extern const char* g_b4MusicFilename;
extern const char* g_b4MusicName;
extern const char* g_b4PreviousMusicName;
extern Film* g_b4Film;
extern XwSceneMusicHandles g_b4MusicState;
/* Declarations follow ascending original IDB address. */

/* 0x433C40 */
XwShellSceneResult B4_640_Play(struct XwShellContext* shell);

/* 0x433D90 */
void B4_640_end_View(int unusedTime);

/* 0x433DF0 */
void B4_640_user_Close(Actor* actor, int unusedTime);

/* 0x435240 */
void B4_640_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x435310 */
void B4_640_OpenSounds(ResFile* unusedResourceFile, Film* unusedFilm);

#ifdef __cplusplus
}
#endif

#endif
