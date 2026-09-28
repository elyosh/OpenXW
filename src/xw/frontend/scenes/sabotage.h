#ifndef XW_FRONTEND_SCENES_SABOTAGE_H
#define XW_FRONTEND_SCENES_SABOTAGE_H

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

enum {
	SABOTAGE_MUSIC_CONTROL_CEL = 5,
	SABOTAGE_MUSIC_CONTROL_VALUE = 2,
	SABOTAGE_FINAL_MUSIC_START_BEAT = 96,
	SABOTAGE_MIDDLE_MUSIC_CONTROL_CEL = 95,
	SABOTAGE_MIDDLE_MUSIC_START_BEAT = 42,
	SABOTAGE_MUSIC_FADE_DURATION = 300
};

enum { SABOTAGE_SOUND_SHUTTLE_2 = 1, SABOTAGE_SOUND_SHUTTLE_4 = 2, SABOTAGE_SOUND_EXPLOSION = 3 };

enum {
	SABOTAGE_MIDDLE_SOUND_RAMP = 1,
	SABOTAGE_MIDDLE_SOUND_BEEP = 2,
	SABOTAGE_MIDDLE_SOUND_STOP_RAMP = 3,
	SABOTAGE_MIDDLE_SOUND_THAT_IT = 10,
	SABOTAGE_MIDDLE_SOUND_CRATE = 11,
	SABOTAGE_MIDDLE_SOUND_ALL_RIGHT = 12,
	SABOTAGE_MIDDLE_SOUND_LIFT_OFF = 13
};

enum {
	SABOTAGE_ACTOR_SLOW = 10,
	SABOTAGE_ACTOR_NON_REFRESHABLE_FIRST = 20,
	SABOTAGE_ACTOR_CLEAR_CONTROL = 30,
	SABOTAGE_ACTOR_SOUND = 50
};

enum { SABOTAGE_BACKGROUND_DRAW_TILED = 1 };

enum {
	SABOTAGE_MIDDLE_MAIN_VIEW = 0,
	SABOTAGE_MIDDLE_FULL_REDRAW_FRAMES = 2,
	SABOTAGE_MIDDLE_ACTOR_SCROLL = 15,
	SABOTAGE_MIDDLE_CANVAS_COUNT = 2,
	SABOTAGE_MIDDLE_BACKGROUND_WIDTH = 320,
	SABOTAGE_MIDDLE_BACKGROUND_HEIGHT = 200
};

enum {
	SABOTAGE_BACKGROUND_WIDTH = 320,
	SABOTAGE_BACKGROUND_HEIGHT = 200,
	SABOTAGE_HALF_BACKGROUND_HEIGHT = SABOTAGE_BACKGROUND_HEIGHT / 2,
	SABOTAGE_TILE_COVERAGE_WIDTH = 640,
	SABOTAGE_TILE_COVERAGE_HEIGHT = 400
};

enum {
	SABOTAGE_BACKGROUND_BYTES = SABOTAGE_BACKGROUND_WIDTH * SABOTAGE_BACKGROUND_HEIGHT,
	SABOTAGE_HALF_BACKGROUND_BYTES = SABOTAGE_BACKGROUND_WIDTH * SABOTAGE_HALF_BACKGROUND_HEIGHT,
	SABOTAGE_CLEAR_Z = 200,
	SABOTAGE_SAVE_Z = 1,
	SABOTAGE_FILM_SPEED_THRESHOLD = 1,
	SABOTAGE_CAPTION_FONT = 0,
	SABOTAGE_CAPTION_X = 20,
	SABOTAGE_CAPTION_COLOR = 16,
	SABOTAGE_APPROACH_CAPTION_START = 4,
	SABOTAGE_APPROACH_CAPTION_END = 60,
	SABOTAGE_APPROACH_CAPTION_Y = 176,
	SABOTAGE_LANDING_CAPTION_START = 80,
	SABOTAGE_LANDING_CAPTION_END = 140,
	SABOTAGE_LANDING_CAPTION_Y = 186
};

enum { SABOTAGE_MIDDLE_CAPTION_FONT = 0 };

enum {
	SABOTAGE_MIDDLE_QUESTION_START = 6,
	SABOTAGE_MIDDLE_QUESTION_END = 20,
	SABOTAGE_MIDDLE_QUESTION_X = 240,
	SABOTAGE_MIDDLE_QUESTION_Y = 115,
	SABOTAGE_MIDDLE_QUESTION_COLOR = 14
};

enum {
	SABOTAGE_MIDDLE_ANSWER_START = 21,
	SABOTAGE_MIDDLE_ANSWER_END = 35,
	SABOTAGE_MIDDLE_ANSWER_X = 76,
	SABOTAGE_MIDDLE_ANSWER_Y = 120,
	SABOTAGE_MIDDLE_ANSWER_COLOR = 51
};

enum {
	SABOTAGE_MIDDLE_LIFTOFF_START = 36,
	SABOTAGE_MIDDLE_LIFTOFF_END = 64,
	SABOTAGE_MIDDLE_LIFTOFF_X = 220,
	SABOTAGE_MIDDLE_LIFTOFF_Y = 115,
	SABOTAGE_MIDDLE_LIFTOFF_COLOR = 14
};

enum {
	SABOTAGE_MIDDLE_NARRATION_FIRST_START = 90,
	SABOTAGE_MIDDLE_NARRATION_FIRST_END = 140,
	SABOTAGE_MIDDLE_NARRATION_FIRST_X = 64,
	SABOTAGE_MIDDLE_NARRATION_FIRST_Y = 176,
	SABOTAGE_MIDDLE_NARRATION_FIRST_COLOR = 16
};

enum {
	SABOTAGE_MIDDLE_NARRATION_SECOND_START = 90,
	SABOTAGE_MIDDLE_NARRATION_SECOND_END = 140,
	SABOTAGE_MIDDLE_NARRATION_SECOND_X = 64,
	SABOTAGE_MIDDLE_NARRATION_SECOND_Y = 186,
	SABOTAGE_MIDDLE_NARRATION_SECOND_COLOR = 16
};

enum {
	SABOTAGE_MIDDLE_BACKGROUND_BYTES = SABOTAGE_MIDDLE_BACKGROUND_WIDTH * SABOTAGE_MIDDLE_BACKGROUND_HEIGHT,
	SABOTAGE_MIDDLE_BACKGROUND_Z = 200,
	SABOTAGE_MIDDLE_CLOSE_Z = -200,
	SABOTAGE_MIDDLE_FILM_SPEED_THRESHOLD = 2
};

extern Film* g_sabotageFilm;
extern Actor* g_sabotageBackgroundSaveActor;
extern Actor* g_sabotageClearActor;
extern int16_t g_sabotageHalfHeightBackground;
extern LandruHandle g_sabotageBackgroundHandle;
extern Rect g_sabotageMiddleDirtyRect;
extern Film* g_sabotageMiddleFilm;
extern Actor* g_sabotageMiddleBackgroundActor;
extern Actor* g_sabotageMiddleScrollActor;
extern Actor* g_sabotageMiddleViewActor;
extern LandruHandle g_sabotageMiddleBackgroundHandle;
extern int16_t g_sabotageMiddleFullRedrawFrames;
extern int16_t g_sabotageMiddlePreviousX;
extern Sound* g_sabotageMusic;
extern Film* g_sabotageMusicFilm;
extern Sound* g_sabotageMiddleMusic;
extern Film* g_sabotageMiddleMusicFilm;
extern Sound* g_sabotageMiddleSpeech33;
extern Sound* g_sabotageMiddleSpeech10;
extern Sound* g_sabotageMiddleSpeech3;
extern Sound* g_sabotageMiddleSpeech17;

/* Declarations follow ascending original IDB address. */

/* 0x45FF10 */
XwShellSceneResult Sabotage_Sabotage(struct XwShellContext* shellContext);

/* 0x4601F0 */
void Sabotage_end_View(int time);

/* 0x460290 */
int16_t Sabotage_film_Callback(Film* film, FilmObject* object);

/* 0x460300 */
int16_t Sabotage_film_Slow_Callback(Film* film, FilmObject* object);

/* 0x460370 */
int16_t Sabotage_film_Actor_To_Background(Actor* actor);

/* 0x460460 */
void Sabotage_user_Sound(Actor* actor, int time);

/* 0x460480 */
void Sabotage_user_SlowActor(Actor* actor, int time);

/* 0x460540 */
int16_t Sabotage_draw_TiledBackground(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
									  int16_t refresh);

/* 0x460630 */
int16_t Sabotage_draw_Clear(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh);

/* 0x460660 */
XwShellSceneResult Sabotage_MiddleScene(struct XwShellContext* shell);

/* 0x4608E0 */
void Sabotage_middle_EndView(int time);

/* 0x460920 */
int16_t Sabotage_middle_FilmCallback(Film* film, FilmObject* object);

/* 0x4609F0 */
int16_t Sabotage_middle_StampBackground(Actor* actor);

/* 0x460AD0 */
void Sabotage_middle_UserSound(Actor* actor, int time);

/* 0x460AF0 */
int16_t Sabotage_middle_DrawBackground(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
									   int16_t refresh);

/* 0x460BE0 */
void Sabotage_middle_AccumulateDirtyRect(Actor* actor, int time);

/* 0x460C30 */
void Sabotage_middle_UpdateViewFrame(Actor* actor, int time);

/* 0x460D30 */
void Sabotage_OpenMusic(ResFile* unusedResourceFile, Film* film);

/* 0x460DE0 */
void Sabotage_CloseMusic(void);

/* 0x460E30 */
void Sabotage_user_Music(Sound* sound, int time);

/* 0x460E70 */
void Sabotage_LoadSoundEffects(void);

/* 0x460EC0 */
void Sabotage_HandleSoundAction(int16_t action);

/* 0x460F10 */
void Sabotage_middle_OpenMusic(ResFile* unusedResourceFile, Film* film);

/* 0x460FC0 */
void Sabotage_middle_UserMusic(Sound* sound, int time);

/* 0x461000 */
void Sabotage_middle_LoadSounds(void);

/* 0x461090 */
void Sabotage_middle_HandleSoundAction(int16_t action);

#ifdef __cplusplus
}
#endif

#endif
