#ifndef XW_FRONTEND_SCENES_INTRO1_H
#define XW_FRONTEND_SCENES_INTRO1_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/frontend/scenes/cutscene.h"
#include "xw/landru_config.h"

#include <landru/actor.h>
#include <landru/film.h>
#include <landru/res.h>
#include <landru/sound.h>
#include <stddef.h>
#include <stdint.h>
#include <xw/frontend/shell.h>

struct XwShellContext;

enum {
	INTRO1_MUSIC_QUIET_CEL = 235,
	INTRO1_MUSIC_LOUD_CEL = 275,
	INTRO1_MUSIC_QUIET_VOLUME = 74,
	INTRO1_MUSIC_LOUD_VOLUME = 128,
	INTRO1_MUSIC_QUIET_FADE_DURATION = 240,
	INTRO1_MUSIC_LOUD_FADE_DURATION = 120
};

enum {
	INTRO1_CAPTION_START = 30,
	INTRO1_CAPTION_END = 90,
	INTRO1_CAPTION_FONT = 3,
	INTRO1_CAPTION_X = 100,
	INTRO1_CAPTION_Y = 440,
	INTRO1_CAPTION_COLOR = 255
};

enum { INTRO1_ACTION_TIE_APPROACH_4 = 2, INTRO1_ACTION_TIE_APPROACH_1 = 3 };

extern Film* g_intro1SoundFilm;
extern const char* g_intro1MusicFilename;
extern const char* g_intro1MusicName;
extern XwSceneMusicHandles g_intro1MusicState;
extern const char* g_intro1ResourceFilename;
extern const char* g_intro1FilmName;
extern Film* g_intro1Film;
/* Declarations follow ascending original IDB address. */

/* 0x44FD00 */
void Intro1_OpenMusic(ResFile* unusedResourceFile, Film* film);

/* 0x44FDA0 */
void Intro1_user_Music(Sound* sound, int time);

/* 0x44FE00 */
void Intro1_LoadSoundEffects(ResFile* unusedResourceFile, Film* film);

/* 0x44FE40 */
void Intro1_CloseSoundEffects(void);

/* 0x44FE70 */
void Intro1_HandleSoundAction(int16_t action);

/* 0x454470 */
XwShellSceneResult Intro1_Opening(struct XwShellContext* shell);

/* 0x454560 */
void Intro1_end_View(int time);

/* 0x4545C0 */
int16_t Intro1_film_Callback(Film* film, FilmObject* filmObject);

/* 0x454600 */
void Intro1_user_SoundAction(Actor* actor, int time);

#ifdef __cplusplus
}
#endif

#endif
