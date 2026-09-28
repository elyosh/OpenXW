#ifndef XW_FRONTEND_SCENES_GOV2_H
#define XW_FRONTEND_SCENES_GOV2_H

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

enum { GOV2_ACTOR_MOVING_REGION = 2 };

enum { GOV2_ACTOR_ROLE_SCROLL = 15 };

enum { GOV2_MAIN_VIEW = 0, GOV2_FULL_REFRESH_FRAMES = 2 };

enum { GOV2_STAMP_CANVAS_COUNT = 2, GOV2_BACKGROUND_WIDTH = 320, GOV2_BACKGROUND_HEIGHT = 200 };

enum {
	GOV2_SPEECH_GHORIN_0 = 0,
	GOV2_SPEECH_GHORIN_1 = 1,
	GOV2_SPEECH_GHORIN_2 = 2,
	GOV2_SPEECH_DARTH_1 = 3,
	GOV2_SPEECH_DARTH_2 = 4,
	GOV2_SPEECH_COUNT = 5
};

enum {
	GOV2_CUE_GHORIN_0 = 10,
	GOV2_CUE_GHORIN_1 = 11,
	GOV2_CUE_GHORIN_2 = 12,
	GOV2_CUE_DARTH_1 = 13,
	GOV2_CUE_DARTH_2 = 14
};

enum {
	GOV2_MUSIC_SCENE_2_BEAT = 24,
	GOV2_MUSIC_SCENE_3_BEAT = 39,
	GOV2_MUSIC_SCENE_4_BEAT = 58,
	GOV2_MUSIC_SCENE_5_BEAT = 33,
	GOV2_MUSIC_SCENE_6_BEAT = 66,
	GOV2_MUSIC_LATE_GROUP = 1,
	GOV2_MUSIC_START_TICK = 400,
	GOV2_MUSIC_FADE_CEL = 65,
	GOV2_MUSIC_FADE_DURATION = 180,
	GOV2_MUSIC_SEEK_CEL = 35,
	GOV2_MUSIC_SEEK_BEAT = 4
};

enum {
	GOV2_RESOURCE_NAME_COUNT = 11,
	GOV2_RESOURCE_NAME_SIZE = 14,
	GOV2_RESOURCE_FILE = 0,
	GOV2_SCENE_2_SLOW_FILM = 1,
	GOV2_SCENE_3_SLOW_FILM = 3,
	GOV2_SCENE_4_SLOW_FILM = 5,
	GOV2_SCENE_5_SLOW_FILM = 7,
	GOV2_SCENE_6_SLOW_FILM = 9,
	GOV2_FAST_FILM_OFFSET = 1,
	GOV2_SPEED_THRESHOLD = 2,
	GOV2_BACKGROUND_Z = 200,
	GOV2_CLOSE_Z = -200,
	GOV2_CAPTION_FONT = 0,
	GOV2_CAPTION_COLOR = 15,
	GOV2_GREETING_FIRST_START = 6,
	GOV2_GREETING_FIRST_END = 76,
	GOV2_GREETING_FIRST_X = 20,
	GOV2_GREETING_FIRST_Y = 175,
	GOV2_GREETING_SECOND_START = 6,
	GOV2_GREETING_SECOND_END = 76,
	GOV2_GREETING_SECOND_X = 20,
	GOV2_GREETING_SECOND_Y = 185,
	GOV2_QUESTION_START = 80,
	GOV2_QUESTION_END = 100,
	GOV2_QUESTION_X = 100,
	GOV2_QUESTION_Y = 180,
	GOV2_SHIPMENT_START = 2,
	GOV2_SHIPMENT_END = 28,
	GOV2_SHIPMENT_X = 20,
	GOV2_SHIPMENT_Y = 180,
	GOV2_REPLY_FIRST_START = 32,
	GOV2_REPLY_FIRST_END = 100,
	GOV2_REPLY_FIRST_X = 20,
	GOV2_REPLY_FIRST_Y = 175,
	GOV2_REPLY_SECOND_START = 32,
	GOV2_REPLY_SECOND_END = 100,
	GOV2_REPLY_SECOND_X = 20,
	GOV2_REPLY_SECOND_Y = 185
};

extern char g_gov2ResourceNames[GOV2_RESOURCE_NAME_COUNT][GOV2_RESOURCE_NAME_SIZE];
extern Actor* g_gov2BackgroundActor;
extern Actor* g_gov2CloseActor;
extern Rect g_gov2CurrentDirtyRect;
extern Rect g_gov2RestoreRect;
extern Actor* g_gov2ScrollActor;
extern Film* g_gov2Film;
extern Rect g_gov2PreviousMovingBounds;
extern LandruHandle g_gov2BackgroundHandle;
extern int16_t g_gov2FullRefreshFrames;
extern int16_t g_gov2PreviousScrollX;
extern XwSceneMusicHandles g_gov2MusicState;
extern Sound* g_gov2SpeechSounds[GOV2_SPEECH_COUNT];

/* Declarations follow ascending original IDB address. */

/* 0x44B750 */
XwShellSceneResult Gov2_Play(struct XwShellContext* shell);

/* 0x44BA70 */
void Gov2_end_View(int unusedTime);

/* 0x44BB10 */
int16_t Gov2_film_Callback(Film* film, FilmObject* object);

/* 0x44BC10 */
int16_t Gov2_StampForegroundAndBackground(Actor* actor);

/* 0x44BD00 */
void Gov2_user_Sound(Actor* actor, int unusedTime);

/* 0x44BD20 */
int16_t Gov2_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
							 int16_t unusedY, int16_t refresh);

/* 0x44BE70 */
void Gov2_user_DirtyBounds(Actor* actor, int time);

/* 0x44BF40 */
void Gov2_user_Close(Actor* actor, int unusedTime);

/* 0x44C160 */
void Gov2_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x44C270 */
void Gov2_user_Music(Sound* unusedSound, int unusedTime);

/* 0x44C2F0 */
void Gov2_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm);

/* 0x44C3C0 */
void Gov2_PlaySoundCue(int16_t cue);

#ifdef __cplusplus
}
#endif

#endif
