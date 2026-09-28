#ifndef XW_FRONTEND_SCENES_XLOGO_H
#define XW_FRONTEND_SCENES_XLOGO_H

#ifdef __cplusplus
extern "C" {
#endif

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

enum { XLOGO_ACTION_FLYBY_2 = 1, XLOGO_ACTION_FLYBY_5 = 2, XLOGO_MUSIC_CONTROL_CEL = 1 };

extern const char* g_xlogoMusicResourceName;
extern const char* g_xlogoMusicName;
extern Sound* g_xlogoMusic;
extern Film* g_xlogoMusicFilm;

enum { XLOGO_UPDATE_Z = 200, XLOGO_CLOSE_Z = -200 };

extern Actor* g_xlogoUpdateActor;
extern Actor* g_xlogoCloseActor;
extern Film* g_xlogoFilm;
extern Rect g_xlogoPreviousDirtyRect;
extern Rect g_xlogoCurrentDirtyRect;

/* Declarations follow ascending original IDB address. */

/* 0x469C70 */
void XLogo_OpenMusic(ResFile* unusedResourceFile, Film* film);

/* 0x469D10 */
void XLogo_user_Music(Sound* sound, int time);

/* 0x469D40 */
XwShellSceneResult XLogo_XLogo(struct XwShellContext* shellContext);

/* 0x469EE0 */
void XLogo_end_View(int time);

/* 0x469F40 */
int16_t XLogo_film_Callback(Film* film, FilmObject* object);

/* 0x469F90 */
void XLogo_user_Sound(Actor* actor, int time);

/* 0x469FB0 */
void XLogo_user_BeginDirtyFrame(Actor* actor, int time);

/* 0x46A000 */
void XLogo_user_AccumulateDirtyRect(Actor* actor, int time);

/* 0x46A070 */
void XLogo_user_Close(Actor* actor, int time);

/* 0x46A110 */
void XLogo_LoadSoundEffects(void);

/* 0x46A150 */
void XLogo_HandleSoundAction(int16_t action);

#ifdef __cplusplus
}
#endif

#endif
