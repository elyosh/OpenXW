#ifndef XW_FRONTEND_SCENES_HANGAR3_H
#define XW_FRONTEND_SCENES_HANGAR3_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/landru_config.h"

#include "xw/frontend/scenes/cutscene.h"

#include <landru/actor.h>
#include <landru/film.h>
#include <landru/rect.h>
#include <landru/res.h>
#include <stddef.h>
#include <stdint.h>
#include <xw/frontend/shell.h>

enum { HANGAR3_MUSIC_START_BEAT = 96, HANGAR3_LAUNCH_START_BEAT = 32, HANGAR3_MUSIC_START_TICK = 400 };

enum {
	HANGAR3_TRANSITION_MARKER = 1,
	HANGAR3_TRANSITION_VOLUME = 95,
	HANGAR3_TRANSITION_DURATION = 300,
	HANGAR3_TRANSITION_GROUP = 1,
	HANGAR3_TRANSITION_BEAT = 4,
	HANGAR3_TRANSITION_TICK_LIMIT = 400,
	HANGAR3_RETURN_FADE_DURATION = 120,
	HANGAR3_INSTANCE_ERROR = -1
};

enum { HANGAR3_ACTION_FLYBY_8 = 1, HANGAR3_ACTION_FLYBY_2 = 2, HANGAR3_ACTION_FLYBY_8_MODE1 = 3 };

enum { HANGAR3_BACKGROUND_WIDTH = 640, HANGAR3_BACKGROUND_HEIGHT = 480 };

enum {
	HANGAR3_RESOURCE_NAME_COUNT = 39,
	HANGAR3_RESOURCE_NAME_SIZE = 14,
	HANGAR3_HIGH_RESOLUTION_OFFSET = 20,
	HANGAR3_INTRO_FILM = 1,
	HANGAR3_SPEED_THRESHOLD = 1,
	HANGAR3_BACKGROUND_Z = 200,
	HANGAR3_ERASE_Z = -200,
	HANGAR3_CAPTION_FONT = 3,
	HANGAR3_CAPTION_START = 4,
	HANGAR3_CAPTION_END = 40,
	HANGAR3_CAPTION_Y = 450,
	HANGAR3_CAPTION_COLOR = 93,
	HANGAR3_LAUNCH_XWING_FILM = 3,
	HANGAR3_LAUNCH_YWING_FILM = 5,
	HANGAR3_LAUNCH_AWING_FILM = 7,
	HANGAR3_LAUNCH_BWING_FILM = 17,
	HANGAR3_RETURN_XWING_FILM = 9,
	HANGAR3_RETURN_YWING_FILM = 11,
	HANGAR3_RETURN_AWING_FILM = 13,
	HANGAR3_RETURN_BWING_FILM = 15
};

extern char g_hangar3ResourceNames[HANGAR3_RESOURCE_NAME_COUNT][HANGAR3_RESOURCE_NAME_SIZE];
extern Actor* g_hangar3BackgroundActor;
extern Actor* g_hangar3EraseActor;
extern Rect g_hangar3PreviousDirtyRect;
extern Film* g_hangar3Film;
extern Rect g_hangar3DirtyRect;
extern LandruHandle g_hangar3BackgroundHandle;
extern XwSceneMusicHandles g_hangar3MusicState;
struct XwShellContext;
/* Declarations follow ascending original IDB address. */

/* 0x44C470 */
XwShellSceneResult Hangar3_Play(struct XwShellContext* shell);

/* 0x44C8D0 */
void Hangar3_end_View(int unusedTime);

/* 0x44C9C0 */
int16_t Hangar3_film_Callback(Film* film, FilmObject* object);

/* 0x44CA40 */
int16_t Hangar3_StampBackground(Actor* actor);

/* 0x44CB20 */
void Hangar3_user_SoundAction(Actor* actor, int unusedTime);

/* 0x44CB40 */
void Hangar3_user_BeginDirtyFrame(Actor* unusedActor, int time);

/* 0x44CB90 */
int16_t Hangar3_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
								int16_t unusedY, int16_t refresh);

/* 0x44CBF0 */
void Hangar3_user_DirtyBounds(Actor* actor, int unusedTime);

/* 0x44CC60 */
void Hangar3_user_Erase(Actor* actor, int unusedTime);

/* 0x44D9A0 */
void Hangar3_OpenMusic(ResFile* unusedResourceFile, Film* film);

/* 0x44DC10 */
void Hangar3_CloseMusic(void);

/* 0x44DEE0 */
void Hangar3_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm);

/* 0x44DF20 */
void Hangar3_HandleSoundAction(int16_t action);

#ifdef __cplusplus
}
#endif

#endif
