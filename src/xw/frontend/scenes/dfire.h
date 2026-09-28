#ifndef XW_FRONTEND_SCENES_DFIRE_H
#define XW_FRONTEND_SCENES_DFIRE_H

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

enum { DFIRE_BACKGROUND_Z = 200, DFIRE_CLOSE_Z = -200, DFIRE_CAPTION_FONT = 0 };

enum {
	DFIRE_ARRIVAL_START = 4,
	DFIRE_ARRIVAL_END = 79,
	DFIRE_ARRIVAL_X = 40,
	DFIRE_ARRIVAL_Y = 180,
	DFIRE_ARRIVAL_COLOR = 16
};

enum {
	DFIRE_FIRE_START = 46,
	DFIRE_FIRE_END = 66,
	DFIRE_FIRE_X = 100,
	DFIRE_FIRE_Y = 50,
	DFIRE_FIRE_COLOR = 14
};

enum {
	DFIRE_ORDER_START = 10,
	DFIRE_ORDER_END = 30,
	DFIRE_ORDER_X = 100,
	DFIRE_ORDER_Y = 50,
	DFIRE_ORDER_COLOR = 14
};

enum {
	DFIRE_COMPLETE_FIRST_START = 4,
	DFIRE_COMPLETE_FIRST_END = 70,
	DFIRE_COMPLETE_FIRST_X = 5,
	DFIRE_COMPLETE_FIRST_Y = 165,
	DFIRE_COMPLETE_FIRST_COLOR = 50
};

enum {
	DFIRE_COMPLETE_SECOND_START = 4,
	DFIRE_COMPLETE_SECOND_END = 70,
	DFIRE_COMPLETE_SECOND_X = 5,
	DFIRE_COMPLETE_SECOND_Y = 175,
	DFIRE_COMPLETE_SECOND_COLOR = 50
};

enum {
	DFIRE_POWER_FIRST_START = 71,
	DFIRE_POWER_FIRST_END = 120,
	DFIRE_POWER_FIRST_X = 10,
	DFIRE_POWER_FIRST_Y = 170,
	DFIRE_POWER_FIRST_COLOR = 14
};

enum {
	DFIRE_POWER_SECOND_START = 71,
	DFIRE_POWER_SECOND_END = 120,
	DFIRE_POWER_SECOND_X = 10,
	DFIRE_POWER_SECOND_Y = 180,
	DFIRE_POWER_SECOND_COLOR = 14
};

extern Actor* g_dfireCloseActor;

enum { DFIRE_ERASE_FULL_CANVAS = 1 };

enum { DFIRE_BACKGROUND_CACHE_SPEED_THRESHOLD = 1 };

enum {
	DFIRE_BACKGROUND_WIDTH = 320,
	DFIRE_BACKGROUND_HEIGHT = 200,
	DFIRE_HALF_BACKGROUND_HEIGHT = DFIRE_BACKGROUND_HEIGHT / 2
};

enum {
	DFIRE_MUSIC_FIRE_2_BEAT = 52,
	DFIRE_MUSIC_FIRE_3_BEAT = 117,
	DFIRE_MUSIC_FIRE_4_BEAT = 133,
	DFIRE_MUSIC_FIRE_5_BEAT = 139,
	DFIRE_MUSIC_SPEECH_BEAT = 77,
	DFIRE_MUSIC_FIRE_ORDER_BEAT = 107,
	DFIRE_MUSIC_INITIAL_CONTROL = 4,
	DFIRE_MUSIC_FADE_DURATION = 300
};

enum {
	DFIRE_MUSIC_FIRE_TRANSITION_CEL = 50,
	DFIRE_MUSIC_FIRE_5_START_CEL = 1,
	DFIRE_MUSIC_FIRE_5_END_CEL = 25,
	DFIRE_MUSIC_DIALOGUE_START_CEL = 10,
	DFIRE_MUSIC_SPEECH_END_CEL = 100,
	DFIRE_MUSIC_FIRE_ORDER_END_CEL = 20,
	DFIRE_MUSIC_PRISON_START_CEL = 7,
	DFIRE_MUSIC_PRISON_END_CEL = 35,
	DFIRE_MUSIC_FIRE_1_CONTROL = 5,
	DFIRE_MUSIC_FIRE_2_CONTROL = 7,
	DFIRE_MUSIC_FINALE_START_CONTROL = 9,
	DFIRE_MUSIC_FINALE_END_CONTROL = 8,
	DFIRE_MUSIC_SPEECH_VOLUME = 94,
	DFIRE_MUSIC_FIRE_ORDER_VOLUME = 84,
	DFIRE_MUSIC_FULL_VOLUME = 128,
	DFIRE_MUSIC_DIALOGUE_FADE_DURATION = 120
};

enum { DFIRE_SPEECH_COUNT = 4 };

enum {
	DFIRE_CUE_BEEP_5 = 1,
	DFIRE_CUE_BEEP_6 = 2,
	DFIRE_CUE_EXPLOSION = 4,
	DFIRE_CUE_ALARM = 5,
	DFIRE_CUE_FIRE_1 = 6,
	DFIRE_CUE_FIRE_2 = 7,
	DFIRE_CUE_FIRE_3 = 8,
	DFIRE_CUE_STOP_FIRE_3 = 9,
	DFIRE_CUE_SPEECH_1 = 10,
	DFIRE_CUE_SPEECH_2 = 11,
	DFIRE_CUE_SPEECH_3 = 12,
	DFIRE_CUE_SPEECH_4 = 13,
	DFIRE_CUE_REPEAT_SPEECH_1 = 14
};

extern Actor* g_dfireBackgroundActor;
extern Rect g_dfirePreviousDirtyRect;
extern int16_t g_dfireRepeatHalfBackground;
extern Rect g_dfireCurrentDirtyRect;
extern Film* g_dfireFilm;
extern LandruHandle g_dfireBackgroundHandle;
extern XwSceneMusicHandles g_dfireMusicState;
extern Sound* g_dfireSpeechSounds[DFIRE_SPEECH_COUNT];
/* Declarations follow ascending original IDB address. */

/* 0x445D10 */
XwShellSceneResult DFire_Play(struct XwShellContext* shell);

/* 0x446220 */
void DFire_end_View(int unusedTime);

/* 0x446310 */
int16_t DFire_film_Callback(Film* film, FilmObject* object);

/* 0x4463B0 */
int16_t DFire_film_Actor_To_Background(Actor* actor);

/* 0x4464A0 */
void DFire_user_Sound(Actor* actor, int unusedTime);

/* 0x4464C0 */
void DFire_user_Background(Actor* unusedActor, int time);

/* 0x446510 */
int16_t DFire_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
							  int16_t unusedY, int16_t refresh);

/* 0x446640 */
void DFire_user_DirtyBounds(Actor* actor, int unusedTime);

/* 0x4466C0 */
void DFire_user_Close(Actor* unusedActor, int unusedTime);

/* 0x446740 */
void DFire_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x4468A0 */
void DFire_CloseMusic(void);

/* 0x446900 */
void DFire_user_Music(Sound* unusedSound, int unusedTime);

/* 0x446AB0 */
void DFire_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm);

/* 0x446C80 */
void DFire_PlaySoundCue(int16_t cue);

#ifdef __cplusplus
}
#endif

#endif
