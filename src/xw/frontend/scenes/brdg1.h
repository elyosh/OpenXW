#ifndef XW_FRONTEND_SCENES_BRDG1_H
#define XW_FRONTEND_SCENES_BRDG1_H

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

enum { XW_BRDG1_SPEECH_COUNT = 6 };

enum {
	BRDG1_MUSIC_START_BEAT = 86,
	BRDG1_MUSIC_QUIET_CEL = 1,
	BRDG1_MUSIC_LOUD_CEL = 140,
	BRDG1_MUSIC_QUIET_VOLUME = 74,
	BRDG1_MUSIC_LOUD_VOLUME = 128,
	BRDG1_MUSIC_FADE_DURATION = 60
};

enum {
	BRDG1_SPEECH_SIR = 0,
	BRDG1_SPEECH_OUR_TIES = 1,
	BRDG1_SPEECH_EXCELLENT = 2,
	BRDG1_SPEECH_THE_ATTACK = 3,
	BRDG1_SPEECH_MOVE_OUR = 4,
	BRDG1_SPEECH_ONCE_SIR = 5
};

enum { BRDG1_BACKGROUND_WIDTH = 640, BRDG1_BACKGROUND_HEIGHT = 480 };

enum {
	BRDG1_BACKGROUND_Z = 200,
	BRDG1_ERASE_Z = -200,
	BRDG1_FILM_SPEED_THRESHOLD = 1,
	BRDG1_BACKGROUND_BYTES = BRDG1_BACKGROUND_WIDTH * BRDG1_BACKGROUND_HEIGHT,
	BRDG1_RESOURCE_FILE = 0,
	BRDG1_RESOURCE_SLOW_FILM = 1,
	BRDG1_RESOURCE_FAST_FILM = 2,
	BRDG1_RESOURCE_NAME_COUNT = 3,
	BRDG1_CAPTION_FONT = 3,
	BRDG1_OFFICER_COLOR = 14,
	BRDG1_COMMANDER_COLOR = 51,
	BRDG1_REPORT_START = 2,
	BRDG1_REPORT_END = 58,
	BRDG1_REPORT_X = 24,
	BRDG1_REPORT_FIRST_Y = 20,
	BRDG1_REPORT_SECOND_Y = 40,
	BRDG1_ATTACK_START = 59,
	BRDG1_ATTACK_END = 81,
	BRDG1_ATTACK_X = 280,
	BRDG1_ATTACK_Y = 50,
	BRDG1_ORDERS_START = 81,
	BRDG1_ORDERS_END = 135,
	BRDG1_ORDERS_X = 220,
	BRDG1_ORDERS_FIRST_Y = 40,
	BRDG1_ORDERS_SECOND_Y = 60,
	BRDG1_ACK_START = 136,
	BRDG1_ACK_END = 151,
	BRDG1_ACK_X = 60,
	BRDG1_ACK_Y = 20
};

extern const char* g_bridge1ResourceNames[BRDG1_RESOURCE_NAME_COUNT];
extern Actor* g_bridge1BackgroundActor;
extern Actor* g_bridge1EraseActor;
extern XwSceneMusicHandles g_bridge1MusicState;
extern Sound* g_bridge1SpeechSounds[XW_BRDG1_SPEECH_COUNT];
extern Film* g_bridge1SpeechFilm;
extern Film* g_bridge1Film;
extern Rect g_bridge1PreviousDirtyRect;
extern Rect g_bridge1DirtyRect;
extern LandruHandle g_bridge1BackgroundHandle;

/* Declarations follow ascending original IDB address. */

/* 0x438460 */
void Brdg1_OpenMusic(ResFile* unusedResourceFile, Film* film);

/* 0x438500 */
void Brdg1_user_Music(Sound* unusedSound, int unusedTime);

/* 0x438560 */
void Brdg1_LoadSpeech(ResFile* unusedResourceFile, Film* film);

/* 0x438600 */
void Brdg1_HandleSpeechAction(int16_t action);

/* 0x4387D0 */
XwShellSceneResult Brdg1_Play(struct XwShellContext* shell);

/* 0x438A50 */
void Brdg1_end_View(int unusedTime);

/* 0x438AB0 */
int16_t Brdg1_film_Callback(Film* film, FilmObject* object);

/* 0x438B30 */
int16_t Brdg1_StampBackground(Actor* actor);

/* 0x438C10 */
void Brdg1_user_SpeechAction(Actor* actor, int unusedTime);

/* 0x438C30 */
void Brdg1_user_BeginDirtyFrame(Actor* unusedActor, int time);

/* 0x438C80 */
int16_t Brdg1_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
							  int16_t unusedY, int16_t refresh);

/* 0x438CE0 */
void Brdg1_user_DirtyBounds(Actor* actor, int unusedTime);

/* 0x438D50 */
void Brdg1_user_Erase(Actor* actor, int unusedTime);

#ifdef __cplusplus
}
#endif

#endif
