#ifndef XW_FRONTEND_SCENES_LOGO640_H
#define XW_FRONTEND_SCENES_LOGO640_H

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

enum { XW_LOGO640_ACTION_PING = 1, LOGO640_MUSIC_START_TIME = 1 };

enum { LOGO640_BACKGROUND_WIDTH = 640, LOGO640_BACKGROUND_HEIGHT = 480 };

enum { LOGO640_BACKGROUND_CACHE_SPEED_THRESHOLD = 1 };

enum { LOGO640_MUSIC_FILE = 0, LOGO640_MUSIC_NAME = 1, LOGO640_MUSIC_RESOURCE_NAME_COUNT = 2 };

extern const char* g_logo640MusicResourceNames[LOGO640_MUSIC_RESOURCE_NAME_COUNT];

enum {
	LOGO640_RESOURCE_FILE = 0,
	LOGO640_RESOURCE_SLOW_FILM = 1,
	LOGO640_RESOURCE_FAST_FILM = 2,
	LOGO640_RESOURCE_NAME_COUNT = 3
};

/* Original slow-path allocation is 320x200 although drawing helpers use 640x480. */
enum { LOGO640_BACKGROUND_ALLOCATION_BYTES = 320 * 200, LOGO640_BACKGROUND_Z = 200, LOGO640_ERASE_Z = -200 };

extern const char* g_logo640ResourceNames[LOGO640_RESOURCE_NAME_COUNT];
extern Actor* g_logo640BackgroundActor;
extern Actor* g_logo640EraseActor;
extern XwSceneMusicHandles g_logo640MusicState;
extern Rect g_logo640PreviousDirtyRect;
extern Film* g_logo640Film;
extern Rect g_logo640DirtyRect;
extern LandruHandle g_logo640BackgroundHandle;

/* Declarations follow ascending original IDB address. */

/* 0x455730 */
void Logo640_OpenMusic(ResFile* unusedResourceFile, Film* film);

/* 0x4557A0 */
void Logo640_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm);

/* 0x4557D0 */
void Logo640_HandleSoundAction(int16_t action);

/* 0x457500 */
XwShellSceneResult Logo640_Play(struct XwShellContext* shell);

/* 0x457700 */
void Logo640_end_View(int time);

/* 0x457730 */
int16_t Logo640_film_Callback(Film* film, FilmObject* object);

/* 0x4577D0 */
int16_t Logo640_StampBackground(Actor* actor);

/* 0x4578B0 */
void Logo640_user_SoundAction(Actor* actor, int unusedTime);

/* 0x4578D0 */
void Logo640_user_BeginDirtyFrame(Actor* unusedActor, int time);

/* 0x457920 */
int16_t Logo640_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
								int16_t unusedY, int16_t refresh);

/* 0x457980 */
void Logo640_user_DirtyBounds(Actor* actor, int unusedTime);

/* 0x4579F0 */
void Logo640_user_Erase(Actor* actor, int unusedTime);

#ifdef __cplusplus
}
#endif

#endif
