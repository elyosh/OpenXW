#ifndef XW_FRONTEND_SCENES_INTRO2_H
#define XW_FRONTEND_SCENES_INTRO2_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/frontend/scenes/cutscene.h"
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
	INTRO2_BACKGROUND_REDRAW_FRAMES = 2,
	INTRO2_BACKGROUND_CACHE_SPEED = 2,
	INTRO2_BACKGROUND_MOVEMENT_SOURCE = 1,
	INTRO2_BACKGROUND_BUFFER_COUNT = 2,
	INTRO2_BACKGROUND_WIDTH = 320,
	INTRO2_BACKGROUND_HEIGHT = 200,
	INTRO2_SCROLL_BAND_HEIGHT = 75,
	INTRO2_LOWER_BUFFER_HEIGHT = INTRO2_BACKGROUND_HEIGHT - INTRO2_SCROLL_BAND_HEIGHT
};

enum {
	INTRO2_MUSIC_START_BEAT = 131,
	INTRO2_MUSIC_FIRST_CONTROL_CEL = 63,
	INTRO2_MUSIC_QUIET_CEL = 80,
	INTRO2_MUSIC_SECOND_CONTROL_CEL = 110,
	INTRO2_MUSIC_THIRD_CONTROL_CEL = 155,
	INTRO2_MUSIC_LOUD_CEL = 260,
	INTRO2_MUSIC_FIRST_CONTROL = 2,
	INTRO2_MUSIC_SECOND_CONTROL = 3,
	INTRO2_MUSIC_THIRD_CONTROL = 4,
	INTRO2_MUSIC_QUIET_VOLUME = 110,
	INTRO2_MUSIC_QUIET_DURATION = 180,
	INTRO2_MUSIC_LOUD_VOLUME = 127,
	INTRO2_MUSIC_LOUD_DURATION = 60
};

enum {
	INTRO2_ACTION_TIE_APPROACH = 2,
	INTRO2_ACTION_FLY_SHOTS = 3,
	INTRO2_ACTION_EXPLOSION = 4,
	INTRO2_ACTION_BIG_EXPLOSION = 5
};

enum {
	INTRO2_FRAME_WIDTH = 640,
	INTRO2_FRAME_HEIGHT = 480,
	INTRO2_LOWER_BUFFER_BYTES = INTRO2_BACKGROUND_WIDTH * INTRO2_LOWER_BUFFER_HEIGHT,
	INTRO2_BACKGROUND_BYTES = INTRO2_BACKGROUND_WIDTH * INTRO2_BACKGROUND_HEIGHT,
	INTRO2_BACKGROUND_Z = 20,
	INTRO2_ERASE_Z = -100
};

extern const char* g_intro2ResourceFilename;
extern const char* g_intro2SlowFilmName;
extern const char* g_intro2FilmName;
extern XwSceneMusicHandles g_intro2MusicState;
extern Rect g_intro2DirtyRect;
extern Film* g_intro2Film;
extern Actor* g_intro2BackgroundActor;
extern Actor* g_intro2BackgroundSource;
extern Rect g_intro2BackgroundRedrawRect;
extern int16_t g_intro2PreviousXStorage;
extern Actor* g_intro2EraseActor;
extern LandruHandle g_intro2BackgroundBuffers[INTRO2_BACKGROUND_BUFFER_COUNT];

/* Declarations follow ascending original IDB address. */

/* 0x44FEC0 */
void Intro2_OpenMusic(ResFile* unusedResourceFile, Film* film);

/* 0x44FF70 */
void Intro2_CloseMusic(void);

/* 0x44FF80 */
void Intro2_user_Music(Sound* sound, int time);

/* 0x450100 */
void Intro2_LoadSoundEffects(void);

/* 0x450170 */
void Intro2_HandleSoundAction(int16_t action);

/* 0x454620 */
XwShellSceneResult Intro2_Attack(struct XwShellContext* shell);

/* 0x454870 */
void Intro2_end_View(int time);

/* 0x4548E0 */
int16_t Intro2_film_Callback(Film* film, FilmObject* filmObject);

/* 0x4549A0 */
int16_t Intro2_StampBackgroundBuffers(Actor* actor);

/* 0x454B00 */
void Intro2_user_DirtyBounds(Actor* actor, int time);

/* 0x454B50 */
void Intro2_user_SoundAction(Actor* actor, int time);

/* 0x454B70 */
void Intro2_user_Erase(Actor* actor, int time);

/* 0x454BF0 */
void Intro2_user_Background(Actor* actor, int time);

/* 0x454C50 */
int16_t Intro2_draw_Background(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh);

#ifdef __cplusplus
}
#endif

#endif
