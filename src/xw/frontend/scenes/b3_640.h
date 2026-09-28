#ifndef XW_FRONTEND_SCENES_B3_640_H
#define XW_FRONTEND_SCENES_B3_640_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/frontend/scenes/cutscene.h"
#include "xw/landru_config.h"

#include <landru/actor.h>
#include <landru/film.h>
#include <landru/res.h>
#include <landru/sound.h>
#include <stddef.h>
#include <stdint.h>
#include <xw/frontend/shell.h>

struct XwShellContext;

enum { B3_SPEECH_FIRST_CEL = 5, B3_SPEECH_SECOND_CEL = 14, B3_SPEECH_THIRD_CEL = 25 };

enum { B3_MUSIC_START_BEAT = 32, B3_MUSIC_INITIAL_CONTROL = 1, B3_MUSIC_TRANSITION_MARKER = 1 };

enum {
	B3_640_SCENE_WIDTH = 640,
	B3_640_SCENE_HEIGHT = 480,
	B3_640_CLOSE_Z = -200,
	B3_640_CAPTION_FONT = 3,
	B3_640_CAPTION_X = 439,
	B3_640_FIRST_CAPTION_START = 4,
	B3_640_FIRST_CAPTION_END = 13,
	B3_640_FIRST_CAPTION_Y = 342,
	B3_640_FIRST_CAPTION_COLOR = 95,
	B3_640_SECOND_CAPTION_START = 14,
	B3_640_SECOND_CAPTION_END = 24,
	B3_640_SECOND_CAPTION_Y = 20,
	B3_640_SECOND_CAPTION_COLOR = 183,
	B3_640_THIRD_CAPTION_START = 25,
	B3_640_THIRD_CAPTION_END = 39,
	B3_640_THIRD_CAPTION_Y = 400
};

extern const char* g_b3ResourceFilename;
extern const char* g_b3FilmName;
extern Actor* g_b3CloseActor;
extern LandruHandle g_b3LegacyBufferHandle;
extern const char* g_b3MusicFilename;
extern const char* g_b3MusicName;
extern const char* g_b3NextMusicFilename;
extern const char* g_b3NextMusicName;
extern Film* g_b3Film;
extern XwSceneMusicHandles g_b3MusicState;
extern XwCutsceneSpeechState g_b3SpeechState;
/* Declarations follow ascending original IDB address. */

/* 0x433A00 */
XwShellSceneResult B3_640_Play(struct XwShellContext* shell);

/* 0x433BB0 */
void B3_640_end_View(int unusedTime);

/* 0x433C10 */
void B3_640_user_Close(Actor* actor, int unusedTime);

/* 0x434FC0 */
void B3_640_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x435070 */
void B3_640_CloseMusic(void);

/* 0x435140 */
void B3_640_OpenSounds(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x4351D0 */
void B3_640_SpeechCallback(Sound* sound, int unusedTime);

#ifdef __cplusplus
}
#endif

#endif
