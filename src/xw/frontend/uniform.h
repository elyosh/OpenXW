#ifndef XW_FRONTEND_UNIFORM_H
#define XW_FRONTEND_UNIFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/landru_config.h"

#include <landru/actor.h>
#include <landru/btnpush.h>
#include <landru/film.h>
#include <landru/input.h>
#include <landru/rect.h>
#include <landru/res.h>
#include <landru/sound.h>
#include <stddef.h>
#include <stdint.h>
#include <xw/frontend/shell.h>

struct XwShellContext;
struct REGISTER_PilotFileRecord;

enum {
	UNIFORM_WIDTH = 640,
	UNIFORM_HEIGHT = 480,
	UNIFORM_BACKGROUND_Z = 200,
	UNIFORM_PILOT_FILENAME_CAPACITY = 32,
	UNIFORM_AWARDS_TOP = 16,
	UNIFORM_MEDALS_FIRST_TOUR = 3,
	UNIFORM_MEDALS_SECOND_TOUR = 4,
	UNIFORM_MEDALS_LEFT = 360,
	UNIFORM_MEDALS_RIGHT = 540,
	UNIFORM_BUTTON_BOTTOM = 32,
	UNIFORM_EXIT_LEFT = 550,
	UNIFORM_EXIT_RIGHT = 630,
	UNIFORM_BUTTON_EXIT = 1
};

extern PushButton* g_uniformMedalsButton;
extern Film* g_uniformFilm;
extern Input* g_uniformRootInput;
extern Input* g_uniformAwardsInput;
extern PushButton* g_uniformExitButton;

enum {
	UNIFORM_MUSIC_FADE_DURATION = 300,
	UNIFORM_MUSIC_RESUME_VOLUME = 128,
	UNIFORM_MUSIC_RESUME_DURATION = 60
};

enum {
	UNIFORM_AWARD_LABEL_COUNT = 26,
	UNIFORM_AWARD_LABEL_SIZE = 28,
	UNIFORM_TOOLTIP_LINES = 3,
	UNIFORM_TOOLTIP_LINE_SIZE = 40,
	UNIFORM_COUNT_TEXT_SIZE = 16,
	UNIFORM_HOVER_PATCH_FIRST = 3,
	UNIFORM_HOVER_RIBBON_FIRST = 21,
	UNIFORM_HOVER_MEDAL_FIRST = 25,
	UNIFORM_LABEL_PATCH_FIRST = 3,
	UNIFORM_LABEL_RANK_FIRST = 6,
	UNIFORM_LABEL_TOUR_PREFIX = 12,
	UNIFORM_LABEL_RIBBON_SUFFIX = 13,
	UNIFORM_LABEL_MEDAL_BIAS = 11,
	UNIFORM_LABEL_EMBELLISHMENT_BIAS = 12,
	UNIFORM_LABEL_SILVER_TALONS = 19,
	UNIFORM_LABEL_TOUR_FIRST = 23,
	UNIFORM_TOOLTIP_HALF_SCREEN = 320,
	UNIFORM_TOOLTIP_RIGHT_GAP = 48,
	UNIFORM_TOOLTIP_LEFT_GAP = 32,
	UNIFORM_TOOLTIP_HALF_LINE = 4,
	UNIFORM_TOOLTIP_LINE_HEIGHT = 10,
	UNIFORM_TOOLTIP_MIN = 4,
	UNIFORM_TOOLTIP_MAX_X = 632,
	UNIFORM_TOOLTIP_MAX_Y = 460,
	UNIFORM_TOOLTIP_BORDER_X = 4,
	UNIFORM_TOOLTIP_BORDER_Y = 2,
	UNIFORM_TOOLTIP_FRAME_COLOR = 16,
	UNIFORM_TOOLTIP_TEXT_COLOR = 15,
	UNIFORM_TOOLTIP_FONT = 0
};

extern char g_uniformAwardLabels[UNIFORM_AWARD_LABEL_COUNT][UNIFORM_AWARD_LABEL_SIZE];

enum { UNIFORM_NO_HOVER = -1 };

enum {
	UNIFORM_KEY_LEFT = 0x4B00,
	UNIFORM_KEY_UP = 0x4800,
	UNIFORM_KEY_RIGHT = 0x4D00,
	UNIFORM_KEY_DOWN = 0x5000,
	UNIFORM_KEYBOARD_SKIP_FIRST = 21,
	UNIFORM_KEYBOARD_SKIP_SECOND = 23,
	UNIFORM_KEYBOARD_SKIP_LAST = 35
};

enum { UNIFORM_HOTSPOT_COUNT = 36, UNIFORM_INITIAL_HOVER_DISTANCE = 999, UNIFORM_HOVER_DISTANCE = 40 };

enum { UNIFORM_BUTTON_MEDALS_CASE = 0 };

enum { UNIFORM_EXIT = 1, UNIFORM_VIEW_MEDALS_CASE = 2 };

enum { UNIFORM_ACTOR_STATE_COUNT = 16, UNIFORM_KEEP_ACTOR_STATE = -1 };

enum {
	UNIFORM_COMBAT_PATCH_COUNT = 6,
	UNIFORM_COMBAT_PATCH_ROLE_BASE = 7,
	UNIFORM_COMBAT_PATCH_WRAPPED_ROW = 2,
	UNIFORM_COMBAT_PATCH_STATE_OFFSET = 31
};

enum {
	UNIFORM_ROLE_KALIDOR = 1,
	UNIFORM_ROLE_EMBELLISHMENTS = 2,
	UNIFORM_ROLE_BADGE_FIRST = 3,
	UNIFORM_ROLE_BADGE_LAST = 5,
	UNIFORM_ROLE_PATCH_FIRST = 6,
	UNIFORM_ROLE_PATCH_LAST = 8,
	UNIFORM_ROLE_RIBBON_FIRST = 9,
	UNIFORM_ROLE_RIBBON_LAST = 11,
	UNIFORM_ROLE_MEDAL_FIRST = 12,
	UNIFORM_ROLE_MEDAL_LAST = 14,
	UNIFORM_ROLE_RANK = 15,
	UNIFORM_HOVER_KALIDOR = 29,
	UNIFORM_HOVER_EMBELLISHMENT_FIRST = 30,
	UNIFORM_HOVER_EMBELLISHMENT_SOURCE = 31,
	UNIFORM_HOVER_EMBELLISHMENT_DUPLICATE = 35,
	UNIFORM_EMBELLISHMENT_MIN_LEVEL = 2,
	UNIFORM_BADGE_TRAINING_LEVEL = 8,
	UNIFORM_PATCH_HOVER_BIAS = 33,
	UNIFORM_RIBBONS_PER_TOUR = 12,
	UNIFORM_RIBBON_ABSENT = 255,
	UNIFORM_RIBBON_HOVER_BIAS = 12,
	UNIFORM_HOVER_RIBBON_SUMMARY = 24,
	UNIFORM_MEDAL_HOVER_BIAS = 13,
	UNIFORM_HOVER_RANK = 28,
	UNIFORM_RANK_EXTRA_STATE_THRESHOLD = 3
};

extern const char* g_uniformMusicResourceName;
extern const char* g_uniformMusicName;
extern int16_t g_uniformHoveredAward;
extern struct REGISTER_PilotFileRecord g_uniformPilot;
extern int16_t g_uniformPreviousHoveredAward;
extern int16_t g_uniformActorStates[UNIFORM_ACTOR_STATE_COUNT];
extern int16_t g_uniformExitAction;
extern int g_uniformKeyboardAward;
extern Sound* g_uniformMusic;
extern Film* g_uniformMusicFilm;
typedef struct XwUniformCombatAwards XwUniformCombatAwards;
typedef struct XwUniformHotspot XwUniformHotspot;

/* Original IDB size: 16 bytes. */
struct XwUniformCombatAwards {
	/* IDB +0x0 */
	uint8_t missionPatch[UNIFORM_COMBAT_PATCH_COUNT];
	/* IDB +0x6 */
	uint8_t gap06[10];
};

typedef char xw_size_XwUniformCombatAwards[(sizeof(XwUniformCombatAwards) == 16) ? 1 : -1];

/* Original IDB size: 4 bytes. */
struct XwUniformHotspot {
	/* IDB +0x0 */
	int16_t x;
	/* IDB +0x2 */
	int16_t y;
};

extern XwUniformHotspot g_uniformHotspots[UNIFORM_HOTSPOT_COUNT];
extern int16_t g_uniformAwardAvailable[UNIFORM_HOTSPOT_COUNT];

/* Declarations follow ascending original IDB address. */

/* 0x4687E0 */
XwShellSceneResult Uniform_Uniform(struct XwShellContext* shellContext);

/* 0x468B20 */
void Uniform_end_View(int time);

/* 0x468CF0 */
int16_t Uniform_film_Callback(Film* film, FilmObject* object);

/* 0x468FC0 */
int16_t Uniform_iupdate_Awards(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
							   int rightEvent, int16_t x, int16_t y);

/* 0x4690A0 */
void Uniform_iuser_Awards(Input* input, int context);

/* 0x4690D0 */
void Uniform_idraw_Awards(Input* input, Rect* frame, Rect* clip, int16_t refresh);

/* 0x469600 */
int16_t Uniform_iupdate_Button(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
							   int rightEvent, int16_t x, int16_t y);

/* 0x469650 */
void Uniform_iuser_Button(Input* input, int context);

/* 0x469680 */
void Uniform_user_Award(Actor* actor, int time);

/* 0x4696B0 */
int16_t Uniform_draw_CombatPatches(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
								   int16_t refresh);

/* 0x469740 */
int16_t Uniform_ReadPilot(const char* filename);

/* 0x469780 */
void Uniform_OpenMusic(ResFile* unusedResourceFile, Film* film);

/* 0x469840 */
void Uniform_CloseMusic(void);

#ifdef __cplusplus
}
#endif

#endif
