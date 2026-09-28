#ifndef XW_FRONTEND_SCENES_RESCUE64_H
#define XW_FRONTEND_SCENES_RESCUE64_H

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

enum { RESCUE64_CUE_DOOR_OPEN = 1, RESCUE64_CUE_DOOR_CLOSE = 2, RESCUE64_CUE_TRACTOR = 3 };

enum { RESCUE64_BACKGROUND_WIDTH = 320, RESCUE64_BACKGROUND_HEIGHT = 200 };

enum { RESCUE64_BACKGROUND_CACHE_SPEED_THRESHOLD = 1 };

enum { RESCUE64_SPEECH_COUNT = 2 };

enum { RESCUE64_RESCUE_START_BEAT = 19, RESCUE64_TORTURE_START_BEAT = 23 };

enum {
	RESCUE64_RESOURCE_FILE = 0,
	RESCUE64_SLOW_RESCUE_FILM = 1,
	RESCUE64_SLOW_CAPTURE_FILM = 2,
	RESCUE64_FAST_RESCUE_FILM = 5,
	RESCUE64_FAST_CAPTURE_FILM = 6,
	RESCUE64_RESOURCE_NAME_COUNT = 7,
	RESCUE64_BACKGROUND_Z = 200,
	RESCUE64_CLOSE_Z = -200,
	RESCUE64_FIRST_CAPTION_START = 3,
	RESCUE64_FIRST_CAPTION_END = 20,
	RESCUE64_SECOND_CAPTION_START = 21,
	RESCUE64_SECOND_CAPTION_END = 50,
	RESCUE64_CAPTION_FONT = 3,
	RESCUE64_CAPTION_X = 200,
	RESCUE64_CAPTION_Y = 8,
	RESCUE64_CAPTURE_SECOND_X = 220,
	RESCUE64_RESCUE_COLOR = 4,
	RESCUE64_CAPTURE_COLOR = 1
};

extern const char* g_rescue64ResourceNames[RESCUE64_RESOURCE_NAME_COUNT];
extern Actor* g_rescue64BackgroundActor;
extern Actor* g_rescue64CloseActor;
extern Rect g_rescue64PreviousDirtyRect;
extern Rect g_rescue64CurrentDirtyRect;
extern Film* g_rescue64Film;
extern LandruHandle g_rescue64BackgroundHandle;
extern XwSceneMusicHandles g_rescue64MusicState;
extern Sound* g_rescue64Speech[RESCUE64_SPEECH_COUNT];
extern Film* g_rescue64SpeechFilm;

/* Declarations follow ascending original IDB address. */

/* 0x45F360 */
XwShellSceneResult Rescue64_Play(struct XwShellContext* shell);

/* 0x45F5F0 */
void Rescue64_end_View(int unusedTime);

/* 0x45F650 */
int16_t Rescue64_film_Callback(Film* film, FilmObject* object);

/* 0x45F6F0 */
int16_t Rescue64_film_Actor_To_Background(Actor* actor);

/* 0x45F7D0 */
void Rescue64_user_SoundCue(Actor* actor, int unusedTime);

/* 0x45F7F0 */
void Rescue64_user_Background(Actor* unusedActor, int time);

/* 0x45F840 */
int16_t Rescue64_draw_Background(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
								 int16_t refresh);

/* 0x45F8A0 */
void Rescue64_user_DirtyActor(Actor* actor, int unusedTime);

/* 0x45F910 */
void Rescue64_user_Close(Actor* actor, int unusedTime);

/* 0x45FD40 */
void Rescue64_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x45FE10 */
void Rescue64_LoadSoundEffects(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x45FEC0 */
int16_t j_shellext_Get_Cur_Scene(void);

/* 0x45FED0 */
void Rescue64_PlaySoundCue(int16_t cue);

#ifdef __cplusplus
}
#endif

#endif
