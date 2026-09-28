#ifndef XW_FRONTEND_SCENES_PROBE_H
#define XW_FRONTEND_SCENES_PROBE_H

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
	PROBE_BACKGROUND_WIDTH = 320,
	PROBE_BACKGROUND_HEIGHT = 200,
	PROBE_TILE_ROWS = 2,
	PROBE_TILE_COLUMNS = 2
};

enum { PROBE_BACKGROUND_CONSUME = 0, PROBE_BACKGROUND_TILED = 1 };

enum {
	PROBE_MUSIC_SCENE_2_BEAT = 22,
	PROBE_MUSIC_SCENE_3_BEAT = 38,
	PROBE_MUSIC_INITIAL_PARAM4 = 123,
	PROBE_MUSIC_ADJUST_AFTER_BEAT = 52,
	PROBE_MUSIC_ADJUSTED_PARAM4 = 110,
	PROBE_MUSIC_FADE_CEL = 160,
	PROBE_MUSIC_FADE_DURATION = 240
};

enum {
	PROBE_CUE_FLYBY_1 = 1,
	PROBE_CUE_FLYBY_5 = 2,
	PROBE_CUE_TORPEDO = 3,
	PROBE_CUE_EXPLOSION = 4,
	PROBE_CUE_BEEP = 5,
	PROBE_CUE_START_HOVER = 6,
	PROBE_CUE_STOP_HOVER = 7,
	PROBE_CUE_DROID = 8,
	PROBE_CUE_DROID_4 = 9
};

enum { PROBE_BEEP_VOLUME = 128 };

enum {
	PROBE_BATTLE_RESOURCE = 0,
	PROBE_SCENE_RESOURCE = 1,
	PROBE_FIRST_FILM = 2,
	PROBE_SECOND_FILM = 3,
	PROBE_THIRD_FILM = 4,
	PROBE_RESOURCE_NAME_COUNT = 5,
	PROBE_RESOURCE_NAME_CAPACITY = 14
};

enum { PROBE_BACKGROUND_BYTES = PROBE_BACKGROUND_WIDTH * PROBE_BACKGROUND_HEIGHT, PROBE_CLOSE_Z = -200 };

enum {
	PROBE_CAPTION_START = 4,
	PROBE_CAPTION_END = 52,
	PROBE_CAPTION_FONT = 0,
	PROBE_CAPTION_X = 60,
	PROBE_CAPTION_COLOR = 16,
	PROBE_FIRST_CAPTION_Y = 10,
	PROBE_FIRST_CAPTION_SECOND_Y = 20,
	PROBE_SECOND_CAPTION_Y = 170,
	PROBE_SECOND_CAPTION_SECOND_Y = 180,
	PROBE_THIRD_CAPTION_Y = 15
};

extern char g_probeResourceNames[PROBE_RESOURCE_NAME_COUNT][PROBE_RESOURCE_NAME_CAPACITY];
extern Film* g_probeFilm;
extern Actor* g_probeCloseActor;
extern LandruHandle g_probeBackgroundHandle;
typedef struct XwProbeMusicState XwProbeMusicState;

/* Original IDB size: 12 bytes. */
struct XwProbeMusicState {
	/* IDB +0x0: Reset on open; increment after selector7>52 triggers param4=110. Selector7 returns -5 in this
	 * build, making that branch unreachable. */
	int param4Adjusted;
	/* IDB +0x4: Retained plans sound, found or loaded from pnmusic.lfd. */
	Sound* sound;
	/* IDB +0x8: Current scene film; callback reads unsigned cel at offset46. */
	Film* film;
};

extern XwProbeMusicState g_probeMusicState;

/* Declarations follow ascending original IDB address. */

/* 0x45ABC0 */
void Probe_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x45ACB0 */
void Probe_user_Music(Sound* unusedSound, int unusedTime);

/* 0x45AD30 */
void Probe_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm);

/* 0x45AE00 */
void Probe_PlaySoundCue(int16_t cue);

/* 0x45B9F0 */
XwShellSceneResult Probe_Play(struct XwShellContext* shell);

/* 0x45BC60 */
void Probe_end_View(int unusedTime);

/* 0x45BCD0 */
int16_t Probe_film_Callback(Film* film, FilmObject* object);

/* 0x45BD40 */
int16_t Probe_film_Actor_To_Background(Actor* actor);

/* 0x45BE20 */
void Probe_user_SoundCue(Actor* actor, int unusedTime);

/* 0x45BE40 */
int16_t Probe_draw_TiledBackground(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
								   int16_t refresh);

/* 0x45BF20 */
void Probe_user_Close(Actor* actor, int unusedTime);

#ifdef __cplusplus
}
#endif

#endif
