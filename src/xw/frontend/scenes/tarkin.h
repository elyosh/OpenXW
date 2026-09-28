#ifndef XW_FRONTEND_SCENES_TARKIN_H
#define XW_FRONTEND_SCENES_TARKIN_H

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

enum XwTarkinSoundCue { XW_TARKIN_CUE_SPEECH = 1 };

enum { TARKIN_BACKGROUND_WIDTH = 320, TARKIN_BACKGROUND_HEIGHT = 200 };

enum { TARKIN_ERASE_FULL_CANVAS = 1 };

enum {
	TARKIN_MUSIC_START_BEAT = 97,
	TARKIN_MUSIC_QUIET_CEL = 5,
	TARKIN_MUSIC_LOUD_CEL = 40,
	TARKIN_MUSIC_QUIET_VOLUME = 94,
	TARKIN_MUSIC_LOUD_VOLUME = 128,
	TARKIN_MUSIC_FADE_DURATION = 120
};

enum {
	TARKIN_BACKGROUND_Z = 200,
	TARKIN_CLOSE_Z = -200,
	TARKIN_BACKGROUND_BYTES = TARKIN_BACKGROUND_WIDTH * TARKIN_BACKGROUND_HEIGHT,
	TARKIN_CAPTION_START = 4,
	TARKIN_CAPTION_END = 79,
	TARKIN_CAPTION_FONT = 0,
	TARKIN_FIRST_CAPTION_X = 80,
	TARKIN_FIRST_CAPTION_Y = 130,
	TARKIN_SECOND_CAPTION_X = 90,
	TARKIN_SECOND_CAPTION_Y = 140,
	TARKIN_CAPTION_COLOR = 50
};

extern Actor* g_tarkinCloseActor;
extern Actor* g_tarkinBackgroundActor;
extern Film* g_tarkinFilm;
extern Rect g_tarkinPreviousDirtyRect;
extern Rect g_tarkinCurrentDirtyRect;
extern LandruHandle g_tarkinBackgroundHandle;
extern XwSceneMusicHandles g_tarkinMusicState;
extern Sound* g_tarkinSpeech;

/* Declarations follow ascending original IDB address. */

/* 0x461220 */
XwShellSceneResult Tarkin_Play(struct XwShellContext* shell);

/* 0x461410 */
void Tarkin_end_View(int unusedTime);

/* 0x461450 */
int16_t Tarkin_film_Callback(Film* film, FilmObject* object);

/* 0x4614D0 */
int16_t Tarkin_film_Actor_To_Background(Actor* actor);

/* 0x461590 */
void Tarkin_user_SoundCue(Actor* actor, int unusedTime);

/* 0x4615B0 */
void Tarkin_user_Background(Actor* unusedActor, int time);

/* 0x461600 */
int16_t Tarkin_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
							   int16_t unusedY, int16_t refresh);

/* 0x461670 */
void Tarkin_user_DirtyActor(Actor* actor, int unusedTime);

/* 0x4616F0 */
void Tarkin_user_Close(Actor* unusedActor, int unusedTime);

/* 0x462530 */
void Tarkin_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x4625D0 */
void Tarkin_user_Music(Sound* unusedSound, int unusedTime);

/* 0x462630 */
void Tarkin_LoadSpeech(ResFile* unusedResourceFile, Film* unusedFilm);

/* 0x462660 */
void Tarkin_PlaySoundCue(int16_t cue);

#ifdef __cplusplus
}
#endif

#endif
