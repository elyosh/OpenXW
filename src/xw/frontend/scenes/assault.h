#ifndef XW_FRONTEND_SCENES_ASSAULT_H
#define XW_FRONTEND_SCENES_ASSAULT_H

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

enum { ASSAULT_BACKGROUND_WIDTH = 320, ASSAULT_BACKGROUND_HEIGHT = 200 };

enum {
	ASSAULT_CUE_ROBOT_WALK = 1,
	ASSAULT_CUE_FLYBY_6A = 2,
	ASSAULT_CUE_GUN_BLAST = 3,
	ASSAULT_CUE_EXPLOSION = 4,
	ASSAULT_CUE_SPEECH = 5,
	ASSAULT_CUE_FLYBY_6 = 6
};

enum {
	ASSAULT_MUSIC_FIRST_CUE_CEL = 30,
	ASSAULT_MUSIC_SECOND_CUE_CEL = 80,
	ASSAULT_MUSIC_SCENE_1_FIRST_CONTROL = 2,
	ASSAULT_MUSIC_SCENE_1_SECOND_CONTROL = 3,
	ASSAULT_MUSIC_SCENE_2_CONTROL = 4,
	ASSAULT_MUSIC_FADE_DURATION = 300,
	ASSAULT_MUSIC_FIRST_START_BEAT = 128,
	ASSAULT_MUSIC_SECOND_START_BEAT = 176,
	ASSAULT_MUSIC_ATTACK_FADE_DURATION = 2100,
	ASSAULT_MUSIC_START_TICK = 400
};

enum {
	ASSAULT_ACTOR_REQUEST_REFRESH = 1,
	ASSAULT_ACTOR_MOVE_ODD_TICKS = 2,
	ASSAULT_ACTOR_MOVE_EVEN_TICKS = 3,
	ASSAULT_ACTOR_HALF_SCALE = 4,
	ASSAULT_ACTOR_HALF_SCALE_VALUE = 128
};

enum {
	ASSAULT_BACKGROUND_Z = 200,
	ASSAULT_CLOSE_Z = -200,
	ASSAULT_CAPTION_FONT = 0,
	ASSAULT_INTRO_CAPTION_START = 4,
	ASSAULT_INTRO_CAPTION_END = 60,
	ASSAULT_ORBIT_CAPTION_X = 90,
	ASSAULT_ORBIT_FIRST_Y = 180,
	ASSAULT_ORBIT_SECOND_Y = 190,
	ASSAULT_ORBIT_CAPTION_COLOR = 16,
	ASSAULT_WALKER_CAPTION_X = 50,
	ASSAULT_GROUND_CAPTION_Y = 176,
	ASSAULT_WALKER_CAPTION_COLOR = 47,
	ASSAULT_SECURED_CAPTION_START = 36,
	ASSAULT_SECURED_CAPTION_END = 80,
	ASSAULT_SECURED_CAPTION_X = 130,
	ASSAULT_SECURED_CAPTION_COLOR = 14
};

extern Actor* g_assaultBackgroundActor;
extern Actor* g_assaultCloseActor;
extern Sound* g_assaultMusic;
extern Film* g_assaultMusicFilm;

extern Rect g_assaultPreviousDirtyRect;
extern Film* g_assaultFilm;
extern Rect g_assaultCurrentDirtyRect;
extern LandruHandle g_assaultBackgroundHandle;
extern Sound* g_assaultSpeech;
/* Declarations follow ascending original IDB address. */

/* 0x4310F0 */
void Assault_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x431210 */
void Assault_CloseMusic(void);

/* 0x431270 */
void Assault_MusicCallback(Sound* unusedSound, int unusedTime);

/* 0x4312F0 */
XwShellSceneResult Assault_Play(struct XwShellContext* shell);

/* 0x431580 */
void Assault_end_View(int unusedTime);

/* 0x4315F0 */
int16_t Assault_film_Callback(Film* film, FilmObject* object);

/* 0x431660 */
int16_t Assault_ActorToBackground(Actor* actor);

/* 0x431740 */
void Assault_user_Sound(Actor* actor, int unusedTime);

/* 0x431760 */
void Assault_user_Background(Actor* unusedActor, int time);

/* 0x4317B0 */
int16_t Assault_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
								int16_t unusedY, int16_t refresh);

/* 0x431810 */
void Assault_user_Actor(Actor* actor, int time);

/* 0x431920 */
void Assault_user_Close(Actor* actor, int unusedTime);

/* 0x4319D0 */
void Assault_OpenSounds(ResFile* unusedResourceFile, Film* unusedFilm);

/* 0x431AA0 */
void Assault_PlaySoundCue(int16_t cue);

#ifdef __cplusplus
}
#endif

#endif
