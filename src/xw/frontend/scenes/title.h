#ifndef XW_FRONTEND_SCENES_TITLE_H
#define XW_FRONTEND_SCENES_TITLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/landru_config.h"

#include <landru/actor.h>
#include <landru/font.h>
#include <landru/rect.h>
#include <landru/res.h>
#include <landru/sound.h>
#include <stddef.h>
#include <stdint.h>
#include <xw/frontend/shell.h>

struct XwShellContext;
typedef struct XwTitleMusicState XwTitleMusicState;

enum {
	TITLE_PARAGRAPH_TEMPLATE_CAPACITY = 8,
	TITLE_PARAGRAPH_NAME_CAPACITY = 16,
	TITLE_PARAGRAPH_TOUR_DIGIT = 6,
	TITLE_LOGO_INITIAL_STEP = 12,
	TITLE_TOUR_START_FRAME = 99,
	TITLE_SCALE_FRACTION_BITS = 16,
	TITLE_LOGO_Z = 20,
	TITLE_LOGO_X = 30,
	TITLE_LOGO_Y = 32,
	TITLE_LOGO_SPEED_THRESHOLD = 2,
	TITLE_STARS_Z = 100,
	TITLE_PAINTER_Z = 10
};

extern char g_titleTourParagraphTemplate[TITLE_PARAGRAPH_TEMPLATE_CAPACITY];
extern Actor* g_titleCrawlActor;
extern FontStruct* g_titleFont;
extern Actor* g_titleAlongActor;
extern Actor* g_titleStarsActor;
extern Actor* g_titleTextPainterActor;
extern Actor* g_titleStarWarsActor;

enum {
	TITLE_END_FRAME = 690,
	TITLE_SET_VIEW_FRAME = 100,
	TITLE_VIEW_ID = 0,
	TITLE_VIEW_TOP = 70,
	TITLE_FADE_START_FRAME = 620,
	TITLE_SLOW_SKIP_FROM_FRAME = 40,
	TITLE_SLOW_SKIP_TO_FRAME = 64
};

enum {
	TITLE_MUSIC_FADE_VOLUME = 127,
	TITLE_MUSIC_FADE_DURATION = 240,
	TITLE_LOGO_MUSIC_FADE_DURATION = 120,
	TITLE_MUSIC_TOUR_FADE_TIME = 560,
	TITLE_MUSIC_INTRO_FADE_TIME = 640,
	TITLE_MUSIC_END_FADE_DURATION = 360,
	TITLE_MUSIC_ATTACK_BEAT_THRESHOLD = 64,
	TITLE_MUSIC_ATTACK_MARKER = 1
};

enum {
	TITLE_LOGO_HIDE_FRAME = 84,
	TITLE_LOGO_FIRST_COLOR = 81,
	TITLE_LOGO_LAST_COLOR = 96,
	TITLE_PALETTE_LAST_COLOR = 255,
	TITLE_LOGO_LATE_MUSIC_FRAME = 1000,
	TITLE_LOGO_INITIAL_SCALE = 460,
	TITLE_LOGO_MIN_SCALE = 1,
	TITLE_LOGO_END_FRAME = 100,
	TITLE_LOGO_FRACTION_STEP = 48,
	TITLE_LOGO_FRACTION_SCALE = 256
};

enum { TITLE_STARS_FADE_FRAME = 38, TITLE_STARS_SHOW_FRAME = 39 };

enum {
	TITLE_CRAWL_LINE_COUNT = 18,
	TITLE_CRAWL_NORMAL_START_Y = 100,
	TITLE_CRAWL_SLOW_START_Y = 60,
	TITLE_CRAWL_OTHER_START_Y = 1,
	TITLE_CRAWL_SCREEN_WIDTH = 320,
	TITLE_CRAWL_SCREEN_HEIGHT = 200,
	TITLE_CRAWL_TEXT_CAPACITY = 64,
	TITLE_CRAWL_FONT = 4,
	TITLE_CRAWL_TEXT_COLOR = 15,
	TITLE_CRAWL_LINE_SPACING = 28,
	TITLE_CRAWL_BITMAP_LINE_HEIGHT = 20,
	TITLE_CRAWL_BITMAP_LINE_COUNT = 10,
	TITLE_CRAWL_FRACTION_SCALE = 4096,
	TITLE_CRAWL_STEPS_PER_UPDATE = 1,
	TITLE_CRAWL_VELOCITY_DECAY = 16,
	TITLE_CRAWL_HORIZON_Y = 40,
	TITLE_CRAWL_MIN_COLOR = 96,
	TITLE_CRAWL_SCALE_TABLE_COUNT = 642
};

extern const int16_t g_titleRowScaleThreshold[TITLE_CRAWL_BITMAP_LINE_HEIGHT];
extern int16_t g_titleLineYVelocityFraction[TITLE_CRAWL_LINE_COUNT];
extern int16_t g_titleScaleSkip[TITLE_CRAWL_SCALE_TABLE_COUNT];
extern int16_t g_titleLogoScaleStep;
extern int16_t g_titleLineActive[TITLE_CRAWL_LINE_COUNT];
extern uint16_t g_titleScaleSkipFraction[TITLE_CRAWL_SCALE_TABLE_COUNT];
extern int16_t g_titleFilmTime;
extern int16_t g_titleLineYFraction[TITLE_CRAWL_LINE_COUNT];
extern int16_t g_titleLineYVelocity[TITLE_CRAWL_LINE_COUNT];
extern int16_t g_titleFadeColorOffset;
extern int16_t g_titleLineDrawn[TITLE_CRAWL_LINE_COUNT];
extern int16_t g_titleLogoScaleFraction;
extern int16_t g_titleLineY[TITLE_CRAWL_LINE_COUNT];
extern int16_t g_titleLineBitmapY[TITLE_CRAWL_LINE_COUNT];
extern int16_t g_titleLineCount;
extern LandruHandle g_titleTextHandle;
extern LandruHandle g_titleTextBitmapHandle;
extern XwTitleMusicState g_titleMusicState;

/* Original IDB size: 12 bytes. */
struct XwTitleMusicState {
	/* IDB +0x0: Pointer to g_titleFilmTime supplied by TITLE_Title; stored but no readers found. */
	int16_t* sceneTime;
	/* IDB +0x4: Loaded starwars music. */
	Sound* sound;
	/* IDB +0x8: Set after legacy inattack command; no reset found. Selector-7 guard returns -5 so branch is
	 * unreachable. */
	int attackTransitionQueued;
};

/* Declarations follow ascending original IDB address. */

/* 0x461A40 */
XwShellSceneResult title_Title(struct XwShellContext* shell);

/* 0x461DB0 */
void title_end_View(int unusedTime);

/* 0x461EF0 */
void title_user_StarWars(Actor* actor, int unusedTime);

/* 0x462020 */
void title_user_Slow_StarWars(Actor* actor, int unusedTime);

/* 0x4620B0 */
void title_user_Stars(Actor* actor, int time);

/* 0x462120 */
void title_user_Title(Actor* unusedActor, int unusedTime);

/* 0x462210 */
void title_draw_Title(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
					  int16_t unusedY, int16_t refresh);

/* 0x462340 */
void title_user_Back(Actor* unusedActor, int time);

/* 0x462410 */
int16_t title_draw_Back(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
						int16_t unusedY, int16_t refresh);

/* 0x462690 */
void title_OpenMusic(ResFile* unusedResourceFile, int16_t* sceneTime);

/* 0x462740 */
void title_CloseMusic(void);

/* 0x4627C0 */
void title_user_Music(Sound* sound, int time);

/* 0x4628B0 */
void title_StartMusic(void);

#ifdef __cplusplus
}
#endif

#endif
