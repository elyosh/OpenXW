#ifndef XW_FRONTEND_SCENES_B1B640_H
#define XW_FRONTEND_SCENES_B1B640_H

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

enum {
	B1B640_CAPTION_COUNT = 4,
	B1B640_CAPTION_CAPACITY = 100,
	B1B640_MUSIC_PATH_CAPACITY = 27,
	B1B640_COMBAT_READY_LEVEL = 6,
	B1B640_FORMATION_COLOR = 248,
	B1B640_AWING_COLOR = 249,
	B1B640_XWING_COLOR = 7,
	B1B640_YWING_COLOR = 135,
	B1B640_BWING_COLOR = 118,
	B1B640_CAPTION_START = 10,
	B1B640_CAPTION_END = 50,
	B1B640_CAPTION_FONT = 3,
	B1B640_CAPTION_X = 50,
	B1B640_CAPTION_Y = 440,
	B1B640_CLOSE_Z = -200
};

extern char g_b1b640DigitalMusicPath[B1B640_MUSIC_PATH_CAPACITY];
extern int16_t g_b1b640CaptionVariant;
extern Actor* g_b1b640CloseActor;

enum { B1B640_CAPTION_SEPARATOR_CAPACITY = 2 };

extern char g_b1b640CaptionSeparator[B1B640_CAPTION_SEPARATOR_CAPACITY];

enum {
	XW_B1B640_SPEECH_COUNT = 3,
	XW_B1B640_JITTER_COUNT = 8,
	B1B640_BACKGROUND_WIDTH = 640,
	B1B640_BACKGROUND_HEIGHT = 480,
	B1B640_TILE_STEP_X = 320,
	B1B640_TILE_STEP_Y = 200,
	B1B640_TILE_ROWS = 2
};

enum {
	B1B640_MUSIC_FADE_DURATION = 120,
	B1B640_MUSIC_QUIET_CEL = 10,
	B1B640_MUSIC_CONTROL_CEL = 30,
	B1B640_MUSIC_LOUD_CEL = 40,
	B1B640_MUSIC_QUIET_VOLUME = 84,
	B1B640_MUSIC_LOUD_VOLUME = 128,
	B1B640_MUSIC_CONTROL_VALUE = 2
};

enum B1b640CaptionVariant {
	B1B640_CAPTION_FORMATION = 0,
	B1B640_CAPTION_TRY_AGAIN = 1,
	B1B640_CAPTION_KEEP_TRAINING = 2,
	B1B640_CAPTION_READY_COMBAT = 3
};

enum { B1B640_ACTOR_ROLE_JITTER = 10, B1B640_BACKGROUND_CONSUME = 0, B1B640_BACKGROUND_TILED = 1 };

extern const char* g_b1b640MusicFilename;
extern const char* g_b1b640MusicName;
extern const char* g_b1b640HangarMusicName;
extern int16_t g_b1b640JitterX[XW_B1B640_JITTER_COUNT];
extern int16_t g_b1b640JitterY[XW_B1B640_JITTER_COUNT];
extern Sound* g_b1b640HangarMusic;
extern Sound* g_b1b640Music;
extern Film* g_b1b640MusicFilm;
extern Sound* g_b1b640Speech[XW_B1B640_SPEECH_COUNT];
extern Film* g_b1b640Film;
extern LandruHandle g_b1b640BackgroundHandle;
typedef struct B1B640Caption B1B640Caption;

/* Original IDB size: 80 bytes. */
struct B1B640Caption {
	/* IDB +0x0: First part of subtitle; joined with one space and secondLine. */
	char firstLine[40];
	/* IDB +0x28: Second part of subtitle; four caption variants use 80-byte stride. */
	char secondLine[40];
};

extern B1B640Caption g_b1b640Captions[B1B640_CAPTION_COUNT];

/* Declarations follow ascending original IDB address. */

/* 0x4325C0 */
void B1b640_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x432680 */
void B1b640_CloseMusic(void);

/* 0x4326C0 */
void B1b640_MusicCallback(Sound* unusedSound, int unusedTime);

/* 0x432750 */
void B1b640_OpenSounds(ResFile* unusedResourceFile, Film* unusedFilm, int16_t variant);

/* 0x432840 */
void B1b640_PlaySoundCue(int16_t cue);

/* 0x432A60 */
XwShellSceneResult B1b640_Play(struct XwShellContext* shell);

/* 0x432DC0 */
void B1b640_end_View(int unusedTime);

/* 0x432E70 */
int16_t B1b640_film_Callback(Film* film, FilmObject* object);

/* 0x432F10 */
int16_t B1b640_film_Actor_To_Background(Actor* actor);

/* 0x432FF0 */
void B1b640_user_Sound(Actor* actor, int unusedTime);

/* 0x433010 */
int16_t B1b640_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t x, int16_t y,
							   int16_t refresh);

/* 0x4330F0 */
void B1b640_user_Close(Actor* actor, int unusedTime);

/* 0x433120 */
void B1b640_user_Jitter(Actor* actor, int time);

/* 0x433150 */
int16_t B1b640_draw_Jitter(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh);

#ifdef __cplusplus
}
#endif

#endif
