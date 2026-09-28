#ifndef XW_FRONTEND_SCENES_BRDG2_H
#define XW_FRONTEND_SCENES_BRDG2_H

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

enum { BRDG2_BACKGROUND_WIDTH = 640, BRDG2_BACKGROUND_HEIGHT = 480 };

enum {
	BRDG2_MUSIC_RESOURCE_FILE = 0,
	BRDG2_MUSIC_RESOURCE_NAME = 1,
	BRDG2_MUSIC_RESOURCE_NAME_COUNT = 2,
	BRDG2_MUSIC_START_BEAT = 48,
	BRDG2_MUSIC_QUIET_CEL = 5,
	BRDG2_MUSIC_LOUD_CEL = 60,
	BRDG2_MUSIC_QUIET_VOLUME = 74,
	BRDG2_MUSIC_LOUD_VOLUME = 128,
	BRDG2_MUSIC_FADE_DURATION = 120
};

enum {
	XW_BRDG2_SPEECH_COUNT = 3,
	XW_BRDG2_SPEECH_ACTION_FIRST = 2,
	XW_BRDG2_SPEECH_ACTION_SECOND = 3,
	XW_BRDG2_SPEECH_ACTION_THIRD = 4
};

extern const char* g_bridge2MusicResourceNames[BRDG2_MUSIC_RESOURCE_NAME_COUNT];
extern XwSceneMusicHandles g_bridge2MusicState;
extern Sound* g_bridge2SpeechSounds[XW_BRDG2_SPEECH_COUNT];
extern Film* g_bridge2SpeechFilm;

enum {
	BRDG2_BACKGROUND_Z = 200,
	BRDG2_ERASE_Z = -200,
	BRDG2_FILM_SPEED_THRESHOLD = 2,
	BRDG2_BACKGROUND_BYTES = BRDG2_BACKGROUND_WIDTH * BRDG2_BACKGROUND_HEIGHT,
	BRDG2_RESOURCE_FILE = 0,
	BRDG2_RESOURCE_SLOW_FILM = 1,
	BRDG2_RESOURCE_FAST_FILM = 2,
	BRDG2_RESOURCE_NAME_COUNT = 3,
	BRDG2_CAPTION_START = 2,
	BRDG2_CAPTION_END = 100,
	BRDG2_CAPTION_FONT = 3,
	BRDG2_CAPTION_X = 12,
	BRDG2_FIRST_CAPTION_Y = 10,
	BRDG2_SECOND_CAPTION_Y = 30,
	BRDG2_CAPTION_COLOR = 14
};

extern const char* g_bridge2ResourceNames[BRDG2_RESOURCE_NAME_COUNT];
extern Actor* g_bridge2BackgroundActor;
extern Actor* g_bridge2EraseActor;
extern Film* g_bridge2Film;
extern Rect g_bridge2PreviousDirtyRect;
extern Rect g_bridge2DirtyRect;
extern LandruHandle g_bridge2BackgroundHandle;
/* Declarations follow ascending original IDB address. */

/* 0x438620 */
void Brdg2_OpenMusic(ResFile* unusedResourceFile, Film* film);

/* 0x4386C0 */
void Brdg2_user_Music(Sound* unusedSound, int unusedTime);

/* 0x438720 */
void Brdg2_LoadSpeech(ResFile* unusedResourceFile, Film* film);

/* 0x438780 */
void Brdg2_HandleSpeechAction(int16_t action);

/* 0x438DF0 */
XwShellSceneResult Brdg2_Play(struct XwShellContext* shell);

/* 0x438FF0 */
void Brdg2_end_View(int unusedTime);

/* 0x439050 */
int16_t Brdg2_film_Callback(Film* film, FilmObject* object);

/* 0x4390D0 */
int16_t Brdg2_StampBackground(Actor* actor);

/* 0x4391B0 */
void Brdg2_user_SpeechAction(Actor* actor, int unusedTime);

/* 0x4391D0 */
void Brdg2_user_BeginDirtyFrame(Actor* unusedActor, int time);

/* 0x439220 */
int16_t Brdg2_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
							  int16_t unusedY, int16_t refresh);

/* 0x439280 */
void Brdg2_user_DirtyBounds(Actor* actor, int unusedTime);

/* 0x4392F0 */
void Brdg2_user_Erase(Actor* actor, int unusedTime);

#ifdef __cplusplus
}
#endif

#endif
