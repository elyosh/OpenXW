#ifndef XW_FRONTEND_PILOT_LOG_H
#define XW_FRONTEND_PILOT_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/frontend/register.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/landru_config.h"

#include <landru/film.h>
#include <landru/input.h>
#include <landru/rect.h>
#include <landru/res.h>
#include <stddef.h>
#include <stdint.h>
#include <xw/frontend/shell.h>

struct XwShellContext;

enum {
	PILOT_LOG_FILENAME_CAPACITY = 32,
	PILOT_LOG_BASE_PAGE_COUNT = 3,
	PILOT_LOG_PAGE_LEFT = 28,
	PILOT_LOG_PAGE_TOP = 14,
	PILOT_LOG_PAGE_RIGHT = 610,
	PILOT_LOG_PAGE_BOTTOM = 430,
	PILOT_LOG_NAV_TOP = 450,
	PILOT_LOG_NAV_BOTTOM = 470,
	PILOT_LOG_PREVIOUS_LEFT = 192,
	PILOT_LOG_PREVIOUS_RIGHT = 212,
	PILOT_LOG_NEXT_LEFT = 406,
	PILOT_LOG_NEXT_RIGHT = 426,
	PILOT_LOG_EXIT_LEFT = 476,
	PILOT_LOG_EXIT_RIGHT = 620,
	PILOT_LOG_INDICATOR_LEFT = 216,
	PILOT_LOG_INDICATOR_RIGHT = 400
};

extern Input* g_pilotLogPageInput;
extern Film* g_pilotLogFilm;

enum { PILOT_LOG_MUSIC_FADE_DURATION = 300 };

enum {
	PILOT_LOG_TRAINING_PAGE = 0,
	PILOT_LOG_HISTORIC_PAGE = 1,
	PILOT_LOG_BONUS_PAGE = 2,
	PILOT_LOG_COMBAT_PAGE = 3,
	PILOT_LOG_RANK_COUNT = 6,
	PILOT_LOG_RANK_CAPACITY = 20,
	PILOT_LOG_STATUS_COUNT = 4,
	PILOT_LOG_STATUS_CAPACITY = 10,
	PILOT_LOG_HEADER_TEXT_CAPACITY = 72,
	PILOT_LOG_POINTS_TEXT_CAPACITY = 44,
	PILOT_LOG_BACKGROUND_COLOR = 23,
	PILOT_LOG_HEADER_BACKGROUND_COLOR = 27,
	PILOT_LOG_HEADER_LINE_COLOR = 1,
	PILOT_LOG_HEADER_TEXT_COLOR = 14,
	PILOT_LOG_HEADER_HEIGHT = 44,
	PILOT_LOG_HEADER_ROW_HEIGHT = 20
};

extern const char g_pilotLogRankText[PILOT_LOG_RANK_COUNT][PILOT_LOG_RANK_CAPACITY];
extern const char g_pilotLogPilotStatusText[PILOT_LOG_STATUS_COUNT][PILOT_LOG_STATUS_CAPACITY];
extern const char g_pilotLogPointsFormat[32];

enum {
	PILOT_LOG_COMBAT_CATEGORY_COUNT = 20,
	PILOT_LOG_COMBAT_CRAFT_COUNT = 24,
	PILOT_LOG_COMBAT_NAME_CAPACITY = 20,
	PILOT_LOG_COMBAT_VALUE_CAPACITY = 28,
	PILOT_LOG_COMBAT_HEADING_CAPACITY = 44,
	PILOT_LOG_COMBAT_COLUMN_GAP = 240,
	PILOT_LOG_COMBAT_TOTALS_GAP = 32,
	PILOT_LOG_COMBAT_WEAPON_GAP = 40,
	PILOT_LOG_COMBAT_HITS_GAP = 22,
	PILOT_LOG_PERCENT_SCALE = 100
};

extern const char g_pilotLogCraftNames[PILOT_LOG_COMBAT_CATEGORY_COUNT][PILOT_LOG_COMBAT_NAME_CAPACITY];
extern const int16_t g_pilotLogCraftCategoryMap[PILOT_LOG_COMBAT_CRAFT_COUNT];
extern const char g_pilotLogKillCaptureFormat[9];
extern const char g_pilotLogSpaceVictoriesHeading[PILOT_LOG_COMBAT_HEADING_CAPACITY];
extern const char g_pilotLogSurfaceVictoriesHeading[PILOT_LOG_COMBAT_HEADING_CAPACITY];
extern const char g_pilotLogLasersFiredHeading[PILOT_LOG_COMBAT_HEADING_CAPACITY];
extern const char g_pilotLogCraftHitsHeading[PILOT_LOG_COMBAT_HEADING_CAPACITY];
extern const char g_pilotLogGroundHitsHeading[PILOT_LOG_COMBAT_HEADING_CAPACITY];
extern const char g_pilotLogWarheadsFiredHeading[PILOT_LOG_COMBAT_HEADING_CAPACITY];
extern const char g_pilotLogCraftLostHeading[PILOT_LOG_COMBAT_HEADING_CAPACITY];

enum {
	PILOT_LOG_NAV_PREVIOUS = 0,
	PILOT_LOG_NAV_NEXT = 1,
	PILOT_LOG_NAV_EXIT = 2,
	PILOT_LOG_NAV_PAGE_LABEL = 3,
	PILOT_LOG_PAGE_TEXT_CAPACITY = 32,
	PILOT_LOG_PAGE_FONT = 2,
	PILOT_LOG_PAGE_COLOR = 15
};

enum {
	PILOT_LOG_TOUR_FIRST_PAGE = 4,
	PILOT_LOG_TOUR_STATUS_COUNT = 4,
	PILOT_LOG_TOUR_STATUS_CAPACITY = 20,
	PILOT_LOG_TOUR_LINE_CAPACITY = 64,
	PILOT_LOG_TOUR_HEADING_CAPACITY = 26,
	PILOT_LOG_TOUR_FRAME_INSET = 4,
	PILOT_LOG_TOUR_TITLE_TOP = 48,
	PILOT_LOG_TOUR_TITLE_HEIGHT = 20,
	PILOT_LOG_TOUR_TITLE_FONT = 3,
	PILOT_LOG_TOUR_TITLE_COLOR = 2,
	PILOT_LOG_TOUR_HEADING_OFFSET_Y = 22,
	PILOT_LOG_TOUR_HEADING_OFFSET_X = 10,
	PILOT_LOG_TOUR_HEADING_COLOR = 51,
	PILOT_LOG_TOUR_SCORE_OFFSET_X = 20,
	PILOT_LOG_TOUR_SCORE_ROW_HEIGHT = 14,
	PILOT_LOG_TOUR_SCORE_COLOR = 50
};

enum {
	PILOT_LOG_SHIP_NAME_COUNT = 5,
	PILOT_LOG_SHIP_NAME_CAPACITY = 8,
	PILOT_LOG_BONUS_SHIP_INDEX = 4,
	PILOT_LOG_BONUS_MISSION_COUNT = 6,
	PILOT_LOG_BONUS_RANGE_NONE = -1,
	PILOT_LOG_BONUS_SUMMARY_LEFT = 10,
	PILOT_LOG_BONUS_SCORE_COLOR = 50,
	PILOT_LOG_TRAINING_SHIP_COUNT = 4,
	PILOT_LOG_TRAINING_TEXT_CAPACITY = 64,
	PILOT_LOG_TRAINING_INSET = 4,
	PILOT_LOG_TRAINING_TOP = 48,
	PILOT_LOG_TRAINING_TITLE_HEIGHT = 20,
	PILOT_LOG_TRAINING_TITLE_FONT = 3,
	PILOT_LOG_TRAINING_TITLE_COLOR = 2,
	PILOT_LOG_TRAINING_HEADING_GAP = 8,
	PILOT_LOG_TRAINING_ROW_HEIGHT = 14,
	PILOT_LOG_TRAINING_ROW_LEFT = 20,
	PILOT_LOG_TRAINING_ROW_FONT = 2,
	PILOT_LOG_TRAINING_ROW_COLOR = 51,
	PILOT_LOG_TRAINING_SECTION_GAP = 24
};

enum {
	PILOT_LOG_HISTORIC_SHIP_COUNT = 4,
	PILOT_LOG_HISTORIC_MISSION_COUNT = 6,
	PILOT_LOG_HISTORIC_COLUMN_ROWS = 3,
	PILOT_LOG_HISTORIC_SECOND_COLUMN_LEFT = 240,
	PILOT_LOG_HISTORIC_SECTION_GAP = 20,
	PILOT_LOG_HISTORIC_ODD_HEADING_COLOR = 61,
	PILOT_LOG_HISTORIC_ODD_SCORE_COLOR = 62
};

extern const char g_pilotLogShipNames[PILOT_LOG_SHIP_NAME_COUNT][PILOT_LOG_SHIP_NAME_CAPACITY];
extern const char g_pilotLogTrainingLevelsHeading[];
extern const char g_pilotLogNoTrainingLevelsText[];
extern const char g_pilotLogTrainingScoresHeading[];
extern const char g_pilotLogBestScorePrefix[];
extern const char g_pilotLogTrainingLevelFormat[];

extern char g_pilotLogOperationHeading[PILOT_LOG_TOUR_HEADING_CAPACITY];
extern const char g_pilotLogTourStatusText[PILOT_LOG_TOUR_STATUS_COUNT][PILOT_LOG_TOUR_STATUS_CAPACITY];
extern const char* g_pilotLogMusicFilename;
extern const char* g_pilotLogMusicName;
extern int16_t g_pilotLogPageCount;
extern Input* g_pilotLogRootInput;
extern int16_t g_pilotLogPage;
extern REGISTER_PilotFileRecord g_pilotLogRecord;
extern XwSceneMusicHandles g_pilotLogMusicState;
typedef struct XwPilotLogCombatTotals XwPilotLogCombatTotals;

/* Working arrays; original stack padding is not part of their data. */
struct XwPilotLogCombatTotals {
	/* IDB +0x0: 20 wrapping word sums; sorting interprets both kills and captures as signed16 (JL/JGE
	 * at45690A/45691E). */
	int16_t kills[PILOT_LOG_COMBAT_CATEGORY_COUNT];
	int16_t captures[PILOT_LOG_COMBAT_CATEGORY_COUNT];
	int16_t order[PILOT_LOG_COMBAT_CATEGORY_COUNT];
};

extern const char g_pilotLogHistoricScoresHeading[40];

/* Declarations follow ascending original IDB address. */

/* 0x4557F0 */
XwShellSceneResult PilotLog_Show(struct XwShellContext* shell);

/* 0x455B50 */
void PilotLog_end_View(int time);

/* 0x455B70 */
int16_t PilotLog_film_Callback(Film* film, FilmObject* object);

/* 0x455BA0 */
void PilotLog_idraw_Page(Input* unusedInput, Rect* frame, Rect* unusedClip, int16_t refresh);

/* 0x455DD0 */
void PilotLog_DrawTrainingPage(const Rect* frame);

/* 0x4560D0 */
void PilotLog_DrawHistoricPage(const Rect* frame);

/* 0x4564B0 */
void PilotLog_DrawBonusPage(const Rect* frame);

/* 0x4567E0 */
void PilotLog_DrawCombatStatistics(const Rect* frame);

/* 0x4570C0 */
void PilotLog_DrawTourPage(const Rect* frame);

/* 0x4572B0 */
void PilotLog_iuser_Navigation(Input* input, int unusedTime);

/* 0x4573A0 */
void PilotLog_idraw_Navigation(Input* input, Rect* frame, Rect* clip, int16_t refresh);

/* 0x4574C0 */
int16_t PilotLog_LoadPilotRecord(const char* filename);

/* 0x457A90 */
void PilotLog_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x457B30 */
void PilotLog_CloseMusic(void);

#ifdef __cplusplus
}
#endif

#endif
