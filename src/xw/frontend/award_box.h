#ifndef XW_FRONTEND_AWARD_BOX_H
#define XW_FRONTEND_AWARD_BOX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/frontend/register.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/uniform.h"
#include "xw/landru_config.h"

#include <landru/actor.h>
#include <landru/btnpush.h>
#include <landru/film.h>
#include <landru/input.h>
#include <landru/rect.h>
#include <landru/res.h>
#include <stddef.h>
#include <stdint.h>
#include <xw/frontend/shell.h>

struct XwShellContext;

enum { AWARD_BOX_NO_HOVER = -1 };

enum {
	AWARD_BOX_MUSIC_FADE_DURATION = 300,
	AWARD_BOX_MUSIC_RESUME_VOLUME = 128,
	AWARD_BOX_MUSIC_RESUME_DURATION = 60
};

enum {
	AWARD_BOX_KEY_LEFT = 0x4B00,
	AWARD_BOX_KEY_UP = 0x4800,
	AWARD_BOX_KEY_RIGHT = 0x4D00,
	AWARD_BOX_KEY_DOWN = 0x5000,
	AWARD_BOX_KEYBOARD_SKIP_FIRST = 7,
	AWARD_BOX_KEYBOARD_SKIP_SECOND = 8,
	AWARD_BOX_KEYBOARD_SKIP_THIRD = 10,
	AWARD_BOX_KEYBOARD_SKIP_LAST = 11
};

enum { AWARD_BOX_HOTSPOT_COUNT = 15, AWARD_BOX_INITIAL_HOVER_DISTANCE = 999, AWARD_BOX_HOVER_DISTANCE = 40 };

enum { AWARD_BOX_BUTTON_UNIFORM = 0 };

enum { AWARD_BOX_EXIT = 1, AWARD_BOX_VIEW_UNIFORM = 2 };

enum { AWARD_BOX_ACTOR_STATE_COUNT = 8, AWARD_BOX_KEEP_ACTOR_STATE = -1 };

enum { AWARD_BOX_BWING_COMBAT_AWARDS = 3 };

enum {
	AWARD_BOX_ROLE_BWING_BADGE = 1,
	AWARD_BOX_ROLE_BWING_PATCHES = 2,
	AWARD_BOX_ROLE_TOUR_FOUR = 3,
	AWARD_BOX_ROLE_TOUR_FIVE = 4,
	AWARD_BOX_ROLE_TOUR_SIX = 5,
	AWARD_BOX_ROLE_SHIELD_OF_YAVIN = 6,
	AWARD_BOX_ROLE_TALONS_OF_HOTH = 7,
	AWARD_BOX_ROLE_UNUSED_MEDAL = 8
};

enum {
	AWARD_BOX_BWING_TRAINING_INDEX = 3,
	AWARD_BOX_BADGE_TRAINING_LEVEL = 8,
	AWARD_BOX_FIRST_PATCH_HOTSPOT = 1,
	AWARD_BOX_FIRST_TOUR_HOTSPOT = 7,
	AWARD_BOX_SECOND_TOUR_HOTSPOT = 9,
	AWARD_BOX_FIRST_MEDAL_HOTSPOT = 12,
	AWARD_BOX_TOUR_PROGRESS_UNAVAILABLE = 255,
	AWARD_BOX_TOUR_INITIAL_PROGRESS_LIMIT = 19,
	AWARD_BOX_TOUR_FIRST_STATE = 7
};

enum {
	AWARD_BOX_TOUR_RIBBON_COUNT = 12,
	AWARD_BOX_TOUR_PROGRESS_DIVISOR = 20,
	AWARD_BOX_TOUR_RIBBON_STATE_OFFSET = 29
};

enum {
	AWARD_BOX_LABEL_COUNT = 10,
	AWARD_BOX_LABEL_SIZE = 28,
	AWARD_BOX_TEXT_LINES = 3,
	AWARD_BOX_TEXT_LINE_SIZE = 40,
	AWARD_BOX_COUNT_TEXT_SIZE = 16,
	AWARD_BOX_LABEL_BADGE = 0,
	AWARD_BOX_LABEL_PATCH = 1,
	AWARD_BOX_LABEL_TOUR = 2,
	AWARD_BOX_LABEL_RIBBONS = 3,
	AWARD_BOX_LABEL_FIRST_MEDAL = 4,
	AWARD_BOX_LABEL_FIRST_TOUR = 7,
	AWARD_BOX_FIRST_EXPANSION_TOUR = 3,
	AWARD_BOX_TOOLTIP_MIDPOINT = 160,
	AWARD_BOX_TOOLTIP_RIGHT_OFFSET = 24,
	AWARD_BOX_TOOLTIP_LEFT_OFFSET = 16,
	AWARD_BOX_TOOLTIP_MARGIN = 4,
	AWARD_BOX_TOOLTIP_RIGHT = 316,
	AWARD_BOX_TOOLTIP_BOTTOM = 180,
	AWARD_BOX_TEXT_HALF_HEIGHT = 4,
	AWARD_BOX_TEXT_HEIGHT = 8,
	AWARD_BOX_TEXT_SPACING = 10,
	AWARD_BOX_TOOLTIP_INSET_Y = 2,
	AWARD_BOX_TOOLTIP_FRAME_COLOR = 16,
	AWARD_BOX_TOOLTIP_TEXT_COLOR = 15,
	AWARD_BOX_TOOLTIP_FONT = 0
};

enum {
	AWARD_BOX_PILOT_FILENAME_CAPACITY = 32,
	AWARD_BOX_WIDTH = 640,
	AWARD_BOX_HEIGHT = 480,
	AWARD_BOX_BACKGROUND_Z = 200,
	AWARD_BOX_AWARDS_BOTTOM = 464,
	AWARD_BOX_BUTTON_TOP = 448,
	AWARD_BOX_UNIFORM_LEFT = 320,
	AWARD_BOX_UNIFORM_RIGHT = 500,
	AWARD_BOX_EXIT_LEFT = 510,
	AWARD_BOX_EXIT_RIGHT = 630,
	AWARD_BOX_BUTTON_EXIT = 1
};

extern Film* g_awardBoxFilm;
extern Input* g_awardBoxRootInput;
extern Input* g_awardBoxAwardsInput;
extern PushButton* g_awardBoxUniformButton;
extern PushButton* g_awardBoxExitButton;
extern char g_awardBoxLabels[AWARD_BOX_LABEL_COUNT][AWARD_BOX_LABEL_SIZE];

extern const char* g_awardBoxMusicFilename;
extern const char* g_awardBoxMusicName;
extern int g_awardBoxKeyboardAward;
extern int16_t g_awardBoxActorStates[AWARD_BOX_ACTOR_STATE_COUNT];
extern XwUniformHotspot g_awardBoxHotspots[AWARD_BOX_HOTSPOT_COUNT];
extern int16_t g_awardBoxAwardAvailable[AWARD_BOX_HOTSPOT_COUNT];
extern int16_t g_awardBoxPreviousHoveredAward;
extern int16_t g_awardBoxHoveredAward;
extern int16_t g_awardBoxExitAction;
extern REGISTER_PilotFileRecord g_awardBoxPilot;
extern XwSceneMusicHandles g_awardBoxMusicState;
/* Declarations follow ascending original IDB address. */

/* 0x4373B0 */
XwShellSceneResult AwardBox_AwardBox(struct XwShellContext* shell);

/* 0x4376D0 */
void AwardBox_end_View(int time);

/* 0x4378B0 */
int16_t AwardBox_film_Callback(Film* film, FilmObject* filmObject);

/* 0x437A80 */
int16_t AwardBox_iupdate_Awards(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
								int rightEvent, int16_t x, int16_t y);

/* 0x437B60 */
void AwardBox_iuser_Awards(Input* input, int context);

/* 0x437B90 */
void AwardBox_idraw_Awards(Input* input, Rect* frame, Rect* clip, int16_t refresh);

/* 0x437FF0 */
int16_t AwardBox_iupdate_Button(PushButton* button, Rect* frame, Rect* clip, int16_t key, int leftEvent,
								int rightEvent, int16_t x, int16_t y);

/* 0x438040 */
void AwardBox_iuser_Button(Input* input, int context);

/* 0x438070 */
void AwardBox_user_AwardState(Actor* actor, int time);

/* 0x4380A0 */
int16_t AwardBox_draw_BWingPatches(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
								   int16_t refresh);

/* 0x438110 */
int16_t AwardBox_draw_TourRibbons(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
								  int16_t refresh);

/* 0x4381A0 */
int16_t AwardBox_ReadPilot(const char* filename);

/* 0x43E390 */
void AwardBox_OpenMusic(ResFile* unusedResourceFile, Film* film);

/* 0x43E450 */
void AwardBox_CloseMusic(void);

#ifdef __cplusplus
}
#endif

#endif
