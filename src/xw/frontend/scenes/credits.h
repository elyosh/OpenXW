#ifndef XW_FRONTEND_SCENES_CREDITS_H
#define XW_FRONTEND_SCENES_CREDITS_H

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
	XW_CREDITS_MUSIC_FADE_TIME = 1270,
	XW_CREDITS_MUSIC_FADE_DURATION = 120,
	CREDITS_MUSIC_START_VOLUME = 96,
	CREDITS_MUSIC_START_FADE_DURATION = 240
};

enum { CREDITS_BACKGROUND_WIDTH = 640, CREDITS_BACKGROUND_HEIGHT = 480 };

enum {
	CREDITS_BACKGROUND_BYTES = CREDITS_BACKGROUND_WIDTH * CREDITS_BACKGROUND_HEIGHT,
	CREDITS_BACKGROUND_Z = 200,
	CREDITS_OVERLAY_Z = -200,
	CREDITS_TEXT_Z = 10,
	CREDITS_TEXT_INSET_X = 60
};

enum { CREDITS_SCROLL_STEP = 2 };

enum {
	CREDITS_LINE_CAPACITY = 80,
	CREDITS_FONT = 3,
	CREDITS_BODY_SPACING = 20,
	CREDITS_HEADING_SPACING = 24,
	CREDITS_PARAGRAPH_SPACING = 40,
	CREDITS_COLOR_MIN = 16,
	CREDITS_COLOR_MAX = 47,
	CREDITS_BODY_COLOR_THRESHOLD = 38,
	CREDITS_BODY_COLOR = 14,
	CREDITS_TOP_FADE_LIMIT = 64,
	CREDITS_BOTTOM_FADE_START = 326,
	CREDITS_UNDERLINE_MAX_HALF_WIDTH = 192,
	CREDITS_UNDERLINE_TOP_LIMIT = 31,
	CREDITS_UNDERLINE_BOTTOM_START = 425,
	CREDITS_UNDERLINE_BOTTOM_END = 456,
	CREDITS_UNDERLINE_SCALE = 8,
	CREDITS_UNDERLINE_COLOR = 1,
	CREDITS_FINAL_SCROLL_START = 2680,
	CREDITS_FINAL_TOP = 230,
	CREDITS_FINAL_COLOR_OFFSET = 2564
};

extern const char* g_creditsMusicFilename;
extern const char* g_creditsMusicName;
extern XwSceneMusicHandles g_creditsMusicState;
extern Actor* g_creditsTextActor;
extern Actor* g_creditsBackgroundActor;
extern int16_t g_creditsCurrentParagraphLineCount;
extern Rect g_creditsPreviousDirtyRect;
extern int16_t g_creditsParagraphCount;
extern Actor* g_creditsOverlayActor;
extern Film* g_creditsFilm;
extern LandruHandle g_creditsText;
extern Rect g_creditsCurrentDirtyRect;
extern LandruHandle g_creditsBackgroundHandle;
/* Declarations follow ascending original IDB address. */

/* 0x43EA70 */
void credits_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x43EB10 */
void credits_user_Music(Sound* unusedSound, int time);

/* 0x43EB30 */
int16_t j_ShellPreferences_GetSfxEnabled(int16_t unusedSoundId);

/* 0x441770 */
XwShellSceneResult credits_Credits(struct XwShellContext* shell);

/* 0x4419A0 */
void credits_end_View(int time);

/* 0x441A40 */
int16_t credits_film_Callback(Film* film, FilmObject* object);

/* 0x441AC0 */
int16_t credits_Credit_Actor_To_Buffer(Actor* actor);

/* 0x441BA0 */
void credits_user_Sound(Actor* actor, int unusedTime);

/* 0x441BC0 */
void credits_user_Background(Actor* unusedActor, int time);

/* 0x441C10 */
int16_t credits_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
								int16_t unusedY, int16_t refresh);

/* 0x441C70 */
void credits_user_Credit(Actor* actor, int unusedTime);

/* 0x441CE0 */
int16_t credits_draw_Credit(Actor* actor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
							int16_t unusedY, int16_t refresh);

/* 0x441F70 */
void credits_user_DirtyBounds(Actor* actor, int unusedTime);

/* 0x441FE0 */
void credits_user_Close(Actor* actor, int unusedTime);

#ifdef __cplusplus
}
#endif

#endif
