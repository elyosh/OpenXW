#ifndef XW_FRONTEND_SCENES_BINTRO_H
#define XW_FRONTEND_SCENES_BINTRO_H

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

enum { BINTRO_ERASE_FULL_CANVAS = 1 };

enum { BINTRO_BACKGROUND_WIDTH = 320, BINTRO_BACKGROUND_HEIGHT = 200 };

enum {
	BINTRO_ACTION_FLYBY_1 = 1,
	BINTRO_ACTION_FLYBY_5 = 2,
	BINTRO_ACTION_LUKE_1 = 10,
	BINTRO_ACTION_FARLAN = 11,
	BINTRO_ACTION_LUKE_2 = 12
};

enum {
	BINTRO_SPEECH_LUKE_1 = 0,
	BINTRO_SPEECH_FARLAN = 1,
	BINTRO_SPEECH_LUKE_2 = 2,
	BINTRO_SPEECH_COUNT = 3
};

enum { BINTRO_MUSIC_SPEECH_LEVEL = 74, BINTRO_MUSIC_FULL_LEVEL = 127, BINTRO_MUSIC_RISE_DURATION = 120 };

enum { BINTRO_MUSIC_ARRIVAL_BEAT = 20, BINTRO_MUSIC_FADE_CEL = 95, BINTRO_MUSIC_FADE_DURATION = 180 };

enum {
	BINTRO_RESOURCE_NAME_COUNT = 5,
	BINTRO_RESOURCE_NAME_SIZE = 14,
	BINTRO_INTRO_RESOURCE = 0,
	BINTRO_FLEET_RESOURCE = 1,
	BINTRO_ARRIVAL_FAST_FILM = 2,
	BINTRO_DIALOGUE_FILM = 4,
	BINTRO_SPEED_THRESHOLD = 2,
	BINTRO_BACKGROUND_Z = 200,
	BINTRO_ERASE_Z = -200,
	BINTRO_CAPTION_FONT = 0,
	BINTRO_QUESTION_FIRST_START = 4,
	BINTRO_QUESTION_FIRST_END = 38,
	BINTRO_QUESTION_FIRST_X = 6,
	BINTRO_QUESTION_FIRST_Y = 60,
	BINTRO_QUESTION_FIRST_COLOR = 51,
	BINTRO_QUESTION_SECOND_START = 4,
	BINTRO_QUESTION_SECOND_END = 38,
	BINTRO_QUESTION_SECOND_X = 6,
	BINTRO_QUESTION_SECOND_Y = 70,
	BINTRO_QUESTION_SECOND_COLOR = 51,
	BINTRO_ANSWER_FIRST_START = 40,
	BINTRO_ANSWER_FIRST_END = 74,
	BINTRO_ANSWER_FIRST_X = 90,
	BINTRO_ANSWER_FIRST_Y = 175,
	BINTRO_ANSWER_FIRST_COLOR = 14,
	BINTRO_ANSWER_SECOND_START = 40,
	BINTRO_ANSWER_SECOND_END = 74,
	BINTRO_ANSWER_SECOND_X = 90,
	BINTRO_ANSWER_SECOND_Y = 185,
	BINTRO_ANSWER_SECOND_COLOR = 14,
	BINTRO_PRAISE_START = 76,
	BINTRO_PRAISE_END = 90,
	BINTRO_PRAISE_X = 20,
	BINTRO_PRAISE_Y = 65,
	BINTRO_PRAISE_COLOR = 51,
	BINTRO_ARRIVAL_START = 10,
	BINTRO_ARRIVAL_END = 58,
	BINTRO_ARRIVAL_X = 90,
	BINTRO_ARRIVAL_Y = 10,
	BINTRO_ARRIVAL_COLOR = 16
};

extern char g_bintroResourceNames[BINTRO_RESOURCE_NAME_COUNT][BINTRO_RESOURCE_NAME_SIZE];
extern Actor* g_bintroBackgroundActor;
extern Actor* g_bintroEraseActor;
extern XwSceneMusicHandles g_bintroMusicState;
extern Rect g_bintroDirtyRect;
extern Film* g_bintroFilm;
extern Rect g_bintroPreviousDirtyRect;
extern LandruHandle g_bintroBackgroundBuffer;
extern Sound* g_bintroSpeechSounds[BINTRO_SPEECH_COUNT];
/* Declarations follow ascending original IDB address. */

/* 0x4343E0 */
void BIntro_OpenMusic(ResFile* unusedResourceFile, Film* film);

/* 0x434490 */
void BIntro_SetLegacyMusicLevel(int value, int duration);

/* 0x4344B0 */
void BIntro_user_Music(Sound* sound, int time);

/* 0x4344F0 */
XwShellSceneResult BIntro_BWingArrival(struct XwShellContext* shell);

/* 0x4347E0 */
void BIntro_end_View(int time);

/* 0x434850 */
int16_t BIntro_film_Callback(Film* film, FilmObject* filmObject);

/* 0x4348D0 */
int16_t BIntro_StampBackground(Actor* actor);

/* 0x434980 */
void BIntro_user_SoundAction(Actor* actor, int time);

/* 0x4349A0 */
int16_t BIntro_draw_Background(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh);

/* 0x434A10 */
void BIntro_user_DirtyBounds(Actor* actor, int time);

/* 0x434A60 */
void BIntro_user_Erase(Actor* actor, int time);

/* 0x434B40 */
void BIntro_LoadSoundEffects(void);

/* 0x434C00 */
void BIntro_HandleSoundAction(int16_t action);

#ifdef __cplusplus
}
#endif

#endif
