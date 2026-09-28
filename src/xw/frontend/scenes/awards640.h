#ifndef XW_FRONTEND_SCENES_AWARDS640_H
#define XW_FRONTEND_SCENES_AWARDS640_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/landru_config.h"

#include <landru/actor.h>
#include <landru/film.h>
#include <landru/memhdl.h>
#include <landru/rect.h>
#include <landru/res.h>
#include <landru/sound.h>
#include <stddef.h>
#include <stdint.h>
#include <xw/frontend/shell.h>

struct XwShellContext;

enum { AWARDS640_MAIN_VIEW = 0, AWARDS640_FULL_REFRESH_FRAMES = 2 };

enum { AWARDS640_MUSIC_FADE_DURATION = 120 };

enum XwAwards640SoundCue { XW_AWARDS640_CUE_SPEECH = 1 };

enum XwAwards640MusicBeat {
	XW_AWARDS640_VICTORY_BEAT_1 = 97,
	XW_AWARDS640_VICTORY_BEAT_2 = 116,
	XW_AWARDS640_VICTORY_BEAT_3 = 129
};

enum { AWARDS640_BACKGROUND_WIDTH = 640, AWARDS640_BACKGROUND_HEIGHT = 480, AWARDS640_DRAW_PASS_COUNT = 2 };

enum {
	AWARDS640_ACTOR_SCROLL = 15,
	AWARDS640_STAMP_MEDAL = 5,
	AWARDS640_MEDAL_LINEAR_FIRST = 2,
	AWARDS640_MEDAL_LINEAR_END = 7,
	AWARDS640_MEDAL_SPECIAL_7 = 7,
	AWARDS640_MEDAL_SPECIAL_8 = 8,
	AWARDS640_MEDAL_SPECIAL_9 = 9,
	AWARDS640_MEDAL_DEFAULT_STATE = 2,
	AWARDS640_MEDAL_STATE_OFFSET = 2,
	AWARDS640_MEDAL_7_STATE = 0,
	AWARDS640_MEDAL_8_STATE = 3,
	AWARDS640_MEDAL_9_STATE = 1
};

enum {
	AWARDS640_CAPTION_CAPACITY = 49,
	AWARDS640_CAPTION_FONT = 3,
	AWARDS640_CAPTION_START = 4,
	AWARDS640_CAPTION_END = 50,
	AWARDS640_CAPTION_Y = 8,
	AWARDS640_CAPTION_COLOR = 14,
	AWARDS640_SPEED_THRESHOLD = 2,
	AWARDS640_BACKGROUND_Z = 200,
	AWARDS640_CLOSE_Z = -200
};

extern char g_awards640CongratulationsText[AWARDS640_CAPTION_CAPACITY];
extern Actor* g_awards640BackgroundActor;
extern Actor* g_awards640CloseActor;
extern Rect g_awards640DirtyRect;
extern Actor* g_awards640ScrollActor;
extern Film* g_awards640Film;
extern LandruHandle g_awards640BackgroundHandle;
extern int16_t g_awards640FullRefreshCountdown;
extern int16_t g_awards640PreviousScrollY;
extern Sound* g_awards640Music;
extern Film* g_awards640MusicFilm;
extern Sound* g_awards640Speech;

/* Declarations follow ascending original IDB address. */

/* 0x431B50 */
XwShellSceneResult Awards640_Play(struct XwShellContext* shell);

/* 0x431EA0 */
void Awards640_end_View(int unusedTime);

/* 0x431F50 */
int16_t Awards640_film_Callback(Film* film, FilmObject* object);

/* 0x432070 */
int16_t Awards640_ActorToScreenAndBackground(Actor* actor);

/* 0x432170 */
void Awards640_user_Sound(Actor* actor, int unusedTime);

/* 0x432190 */
int16_t Awards640_draw_Background(Actor* actor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
								  int16_t unusedY, int16_t refresh);

/* 0x4322C0 */
void Awards640_user_Actor(Actor* actor, int unusedTime);

/* 0x432310 */
void Awards640_user_Close(Actor* actor, int unusedTime);

/* 0x432410 */
void Awards640_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x432510 */
void Awards640_CloseMusic(void);

/* 0x432560 */
void Awards640_OpenSounds(ResFile* unusedResourceFile, Film* unusedFilm);

/* 0x432590 */
void Awards640_PlaySoundCue(int16_t cue);

#ifdef __cplusplus
}
#endif

#endif
