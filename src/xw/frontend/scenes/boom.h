#ifndef XW_FRONTEND_SCENES_BOOM_H
#define XW_FRONTEND_SCENES_BOOM_H

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

enum {
	BOOM_BACKGROUND_WIDTH = 320,
	BOOM_BACKGROUND_HEIGHT = 200,
	BOOM_CROPPED_HEIGHT = 120,
	BOOM_CROP_ORIGIN_Y = 40
};

enum { BOOM_TILE_COLUMNS = 2, BOOM_TILE_ROWS = 2 };

enum { BOOM_BACKGROUND_CONSUME = 0, BOOM_BACKGROUND_DRAW = 1 };

enum {
	BOOM_CUE_FLYBY_1 = 1,
	BOOM_CUE_EXPLOSION = 2,
	BOOM_CUE_TORCH = 3,
	BOOM_CUE_FLYBY_5 = 4,
	BOOM_CUE_REBEL_1 = 10,
	BOOM_CUE_REBEL_2 = 11,
	BOOM_SPEECH_COUNT = 2
};

enum {
	BOOM_MUSIC_FIRST_BEAT = 12,
	BOOM_MUSIC_SECOND_BEAT = 34,
	BOOM_MUSIC_THIRD_BEAT = 11,
	BOOM_MUSIC_START_TICK = 400,
	BOOM_MUSIC_TRANSITION_CEL = 100,
	BOOM_MUSIC_REPOSITION_CEL = 65,
	BOOM_MUSIC_TRANSITION_MARKER = 1,
	BOOM_MUSIC_REPOSITION_GROUP = 2,
	BOOM_MUSIC_REPOSITION_BEAT = 38,
	BOOM_MUSIC_BEAT_PERIOD = 2,
	BOOM_MUSIC_PARAM9_VALUE = -3
};

extern const char* g_boomFirstMusicFilename;
extern const char* g_boomFirstMusicName;
extern const char* g_boomSecondMusicFilename;
extern const char* g_boomSecondMusicName;

enum {
	BOOM_RESOURCE_NAME_CAPACITY = 14,
	BOOM_FILM_COUNT = 5,
	BOOM_FIRST_FILM = 1,
	BOOM_SECOND_FAST_FILM = 2,
	BOOM_SECOND_SLOW_FILM = 3,
	BOOM_THIRD_FULL_FILM = 4,
	BOOM_THIRD_CROPPED_FILM = 5,
	BOOM_FILM_SPEED_THRESHOLD = 2
};

enum {
	BOOM_BACKGROUND_BYTES = BOOM_BACKGROUND_WIDTH * BOOM_BACKGROUND_HEIGHT,
	BOOM_CROPPED_BYTES = BOOM_BACKGROUND_WIDTH * BOOM_CROPPED_HEIGHT,
	BOOM_CLOSE_Z = -200
};

enum {
	BOOM_FIRST_CAPTION_START = 6,
	BOOM_FIRST_CAPTION_END = 24,
	BOOM_SECOND_CAPTION_START = 28,
	BOOM_SECOND_CAPTION_END = 44,
	BOOM_CAPTION_FONT = 0,
	BOOM_FIRST_CAPTION_X = 80,
	BOOM_SECOND_CAPTION_X = 150,
	BOOM_CAPTION_Y = 120,
	BOOM_CAPTION_COLOR = 14
};

extern char g_boomResourceFilename[BOOM_RESOURCE_NAME_CAPACITY];
extern char g_boomFilmNames[BOOM_FILM_COUNT][BOOM_RESOURCE_NAME_CAPACITY];
extern XwCutsceneMusicTransitionState g_boomMusicState;
extern Sound* g_boomSpeech[BOOM_SPEECH_COUNT];
extern int16_t g_boomFullHeightBackground;
extern Actor* g_boomCloseActor;
extern Film* g_boomFilm;
extern LandruHandle g_boomBackgroundHandle;
/* Declarations follow ascending original IDB address. */

/* 0x4369B0 */
void Boom_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x436AD0 */
void Boom_MusicCallback(Sound* unusedSound, int unusedTime);

/* 0x436BE0 */
void Boom_OpenSounds(ResFile* unusedResourceFile, Film* unusedFilm);

/* 0x436C90 */
void Boom_PlaySoundCue(int16_t cue);

/* 0x436D40 */
XwShellSceneResult Boom_Play(struct XwShellContext* shell);

/* 0x436FB0 */
void Boom_end_View(int unusedTime);

/* 0x437020 */
int16_t Boom_film_Callback(Film* film, FilmObject* object);

/* 0x4370A0 */
int16_t Boom_film_Actor_To_Background(Actor* actor);

/* 0x4371D0 */
void Boom_user_Sound(Actor* actor, int unusedTime);

/* 0x4371F0 */
int16_t Boom_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t x, int16_t y,
							 int16_t refresh);

/* 0x437360 */
void Boom_user_Close(Actor* actor, int unusedTime);

#ifdef __cplusplus
}
#endif

#endif
