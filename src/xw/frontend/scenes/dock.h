#ifndef XW_FRONTEND_SCENES_DOCK_H
#define XW_FRONTEND_SCENES_DOCK_H

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
extern const char* g_dockMusicFilename;
extern const char* g_dockMusicName;
extern XwSceneMusicHandles g_dockMusicState;
extern Rect g_dockPreviousDirtyRect;
extern Rect g_dockCurrentDirtyRect;
extern Film* g_dockFilm;
extern LandruHandle g_dockBackgroundHandle;

enum { DOCK_CUE_SHUTTLE_2 = 1, DOCK_CUE_SHUTTLE_4 = 2 };

enum { DOCK_MUSIC_START_BEAT = 16, DOCK_MUSIC_START_TICK = 400, DOCK_MUSIC_INITIAL_CONTROL = 5 };

enum { DOCK_MUSIC_FADE_CEL = 60, DOCK_MUSIC_FADE_DURATION = 240 };

enum { DOCK_BACKGROUND_WIDTH = 320, DOCK_BACKGROUND_HEIGHT = 200 };

enum { DOCK_BACKGROUND_CACHE_SPEED_THRESHOLD = 1 };

enum {
	DOCK_RESOURCE_FILE = 0,
	DOCK_RESOURCE_NAME_COUNT = 7,
	DOCK_RESOURCE_NAME_SIZE = 14,
	DOCK_FIRST_SLOW_FILM = 1,
	DOCK_SECOND_SLOW_FILM = 3,
	DOCK_THIRD_SLOW_FILM = 5,
	DOCK_FAST_FILM_OFFSET = 1,
	DOCK_BACKGROUND_Z = 200,
	DOCK_CLOSE_Z = -200,
	DOCK_CAPTION_START = 4,
	DOCK_FIRST_CAPTION_END = 44,
	DOCK_SECOND_CAPTION_END = 50,
	DOCK_THIRD_CAPTION_END = 70,
	DOCK_CAPTION_FONT = 0,
	DOCK_CAPTION_X = 40,
	DOCK_FIRST_CAPTION_Y = 10,
	DOCK_THIRD_CAPTION_Y = 180,
	DOCK_BOTTOM_CAPTION_Y = 190,
	DOCK_CAPTION_COLOR = 16
};

extern char g_dockResourceNames[DOCK_RESOURCE_NAME_COUNT][DOCK_RESOURCE_NAME_SIZE];
extern Actor* g_dockBackgroundActor;
extern Actor* g_dockCloseActor;

/* Declarations follow ascending original IDB address. */

/* 0x446E20 */
void Dock_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x446EE0 */
void Dock_user_Music(Sound* unusedSound, int unusedTime);

/* 0x446F20 */
void Dock_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm);

/* 0x446F60 */
void Dock_CloseSoundEffects(void);

/* 0x446F80 */
void Dock_PlaySoundCue(int16_t cue);

/* 0x4471C0 */
XwShellSceneResult Dock_Play(struct XwShellContext* shell);

/* 0x447480 */
void Dock_end_View(int unusedTime);

/* 0x4474F0 */
int16_t Dock_film_Callback(Film* film, FilmObject* object);

/* 0x447590 */
int16_t Dock_film_Actor_To_Background(Actor* actor);

/* 0x447670 */
void Dock_user_Sound(Actor* actor, int unusedTime);

/* 0x447690 */
void Dock_user_Background(Actor* unusedActor, int time);

/* 0x4476E0 */
int16_t Dock_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
							 int16_t unusedY, int16_t refresh);

/* 0x447740 */
void Dock_user_DirtyBounds(Actor* actor, int unusedTime);

/* 0x4477B0 */
void Dock_user_Close(Actor* actor, int unusedTime);

#ifdef __cplusplus
}
#endif

#endif
