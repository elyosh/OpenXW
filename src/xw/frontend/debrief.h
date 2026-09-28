#ifndef XW_FRONTEND_DEBRIEF_H
#define XW_FRONTEND_DEBRIEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/frontend/register.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/landru_config.h"

#include <landru/actor.h>
#include <landru/btnpush.h>
#include <landru/film.h>
#include <landru/input.h>
#include <landru/memhdl.h>
#include <landru/rect.h>
#include <landru/res.h>
#include <stddef.h>
#include <stdint.h>
#include <xw/frontend/shell.h>

struct XwShellContext;

enum {
	DEBRIEF_INITIAL_FOCUS = 1,
	DEBRIEF_INITIAL_MOUSE_X = 152,
	DEBRIEF_INITIAL_MOUSE_Y = 174,
	DEBRIEF_RESOURCE_FILE = 0,
	DEBRIEF_RESOURCE_FILM = 1,
	DEBRIEF_RESOURCE_LEFT_DOOR = 2,
	DEBRIEF_RESOURCE_RIGHT_DOOR = 3,
	DEBRIEF_RESOURCE_PREVIOUS = 4,
	DEBRIEF_RESOURCE_NEXT = 5,
	DEBRIEF_RESOURCE_BASE = 7,
	DEBRIEF_RESOURCE_INTERIOR = 9,
	DEBRIEF_BACKGROUND_Z = 100,
	DEBRIEF_STATISTICS_Z = -10,
	DEBRIEF_STATISTICS_LEFT = 216,
	DEBRIEF_STATISTICS_TOP = 68,
	DEBRIEF_STATISTICS_RIGHT = 636,
	DEBRIEF_STATISTICS_BOTTOM = 348,
	DEBRIEF_LEFT_DOOR_TOP = 185,
	DEBRIEF_LEFT_DOOR_RIGHT = 87,
	DEBRIEF_LEFT_DOOR_BOTTOM = 287,
	DEBRIEF_RIGHT_DOOR_LEFT = 115,
	DEBRIEF_RIGHT_DOOR_TOP = 195,
	DEBRIEF_RIGHT_DOOR_RIGHT = 200,
	DEBRIEF_RIGHT_DOOR_BOTTOM = 290,
	DEBRIEF_LABEL_LEFT = 381,
	DEBRIEF_LABEL_TOP = 381,
	DEBRIEF_LABEL_RIGHT = 482,
	DEBRIEF_LABEL_BOTTOM = 394,
	DEBRIEF_CONTROLS_LEFT = 328,
	DEBRIEF_CONTROLS_TOP = 366,
	DEBRIEF_CONTROLS_RIGHT = 543,
	DEBRIEF_CONTROLS_BOTTOM = 406,
	DEBRIEF_PREVIOUS_RIGHT = 369,
	DEBRIEF_NEXT_LEFT = 504,
	DEBRIEF_BUTTON_BOTTOM = 404,
	DEBRIEF_PAGE_NUMBER_INPUT = 1
};

enum {
	DEBRIEF_PROVING_LABEL_CAPACITY = 48,
	DEBRIEF_PROVING_HEADER_HEIGHT = 20,
	DEBRIEF_PROVING_SECTION_HEIGHT = 28
};

extern const char g_debriefAWingText[DEBRIEF_PROVING_LABEL_CAPACITY];
extern const char g_debriefXWingText[DEBRIEF_PROVING_LABEL_CAPACITY];
extern const char g_debriefYWingText[DEBRIEF_PROVING_LABEL_CAPACITY];
extern const char g_debriefStarfighterText[DEBRIEF_PROVING_LABEL_CAPACITY];
extern const char g_debriefProvingGroundText[DEBRIEF_PROVING_LABEL_CAPACITY];
extern const char g_debriefRunText[DEBRIEF_PROVING_LABEL_CAPACITY];
extern const char g_debriefGatesPassedText[DEBRIEF_PROVING_LABEL_CAPACITY];
extern const char g_debriefGatesMissedText[DEBRIEF_PROVING_LABEL_CAPACITY];
extern const char g_debriefGatesRemainingText[DEBRIEF_PROVING_LABEL_CAPACITY];
extern const char g_debriefRoundsFiredText[DEBRIEF_PROVING_LABEL_CAPACITY];
extern const char g_debriefTargetHitsText[DEBRIEF_PROVING_LABEL_CAPACITY];
extern const char g_debriefScoreText[DEBRIEF_PROVING_LABEL_CAPACITY];
extern const char g_debriefBWingText[DEBRIEF_PROVING_LABEL_CAPACITY];
typedef struct XwDebriefKillCategories XwDebriefKillCategories;
typedef struct XwDebriefLossCategories XwDebriefLossCategories;

enum {
	DEBRIEF_HIGH_SCORE_COUNT = 8,
	DEBRIEF_HIGH_SCORE_NAME_CAPACITY = 24,
	DEBRIEF_SCORE_FILENAME_CAPACITY = 32,
	DEBRIEF_SCORE_PATH_CAPACITY = 128,
	DEBRIEF_TOUR_SCORE_DIGIT = 4,
	DEBRIEF_COMBAT_SCORE_DIGIT = 6
};

enum {
	DEBRIEF_FLIGHT_GROUP_CAPACITY = 16,
	DEBRIEF_FLIGHT_GROUP_NAME_CAPACITY = 16,
	DEBRIEF_CRAFT_CATEGORY_COUNT = 20,
	DEBRIEF_CRAFT_CATEGORY_NAME_CAPACITY = 20,
	DEBRIEF_CRAFT_CATEGORY_MAP_COUNT = 24,
	DEBRIEF_CRAFT_CATEGORY_B_WING = 17,
	DEBRIEF_B_WING_INITIAL_STATUS_MIN = 10,
	DEBRIEF_GOAL_OUTCOME_COUNT = 32,
	DEBRIEF_GOAL_OUTCOME_CAPACITY = 32,
	DEBRIEF_GOAL_FAILURE_OFFSET = 15,
	DEBRIEF_GOAL_GROUP_SEPARATOR = 8,
	DEBRIEF_GOAL_LINE_CAPACITY = 80,
	DEBRIEF_GOAL_HEADING_HEIGHT = 22,
	DEBRIEF_GOAL_HEADING_LEFT = 4,
	DEBRIEF_GOAL_HEADING_FONT = 3,
	DEBRIEF_GOAL_HEADING_COLOR = 2,
	DEBRIEF_GOAL_LINE_LEFT = 10,
	DEBRIEF_GOAL_LINE_HEIGHT = 14,
	DEBRIEF_GOAL_LINE_FONT = 2,
	DEBRIEF_GOAL_LINE_COLOR = 51,
	DEBRIEF_GOAL_TRAILING_SPACE = 2
};

enum {
	DEBRIEF_SECTION_MISSION_RESULT = 0,
	DEBRIEF_SECTION_PROMOTION = 1,
	DEBRIEF_SECTION_BATTLE_PATCH = 2,
	DEBRIEF_SECTION_UNCOMPLETED_GOALS = 3,
	DEBRIEF_SECTION_COMPLETED_GOALS = 4,
	DEBRIEF_SECTION_OBJECT_GOALS = 5,
	DEBRIEF_SECTION_SPACECRAFT_KILLS = 6,
	DEBRIEF_SECTION_SPACECRAFT_LOSSES = 7,
	DEBRIEF_SECTION_SPACE_OBJECT_KILLS = 8,
	DEBRIEF_SECTION_DEATH_STAR_BUILDING_KILLS = 9,
	DEBRIEF_SECTION_LASER_ACCURACY = 10,
	DEBRIEF_SECTION_ION_ACCURACY = 11,
	DEBRIEF_SECTION_WARHEAD_ACCURACY = 12,
	DEBRIEF_SECTION_COUNT = 13,
	DEBRIEF_PAGE_HEIGHT = 228,
	DEBRIEF_SUMMARY_SECTIONS_TOP = 48
};

enum { DEBRIEF_FOCUS_ROWS = 1, DEBRIEF_FOCUS_COUNT = 4, DEBRIEF_TOUR_FOCUS_COUNT = 3 };

enum { DEBRIEF_BACKGROUND_WIDTH = 640, DEBRIEF_BACKGROUND_HEIGHT = 480 };

enum {
	DEBRIEF_STATISTICS_BACKGROUND_COLOR = 23,
	DEBRIEF_STATISTICS_HEADER_COLOR = 27,
	DEBRIEF_STATISTICS_SEPARATOR_COLOR = 1,
	DEBRIEF_STATISTICS_BOLD_COLOR = 15,
	DEBRIEF_STATISTICS_HEADER_HEIGHT = 42
};

enum { DEBRIEF_RESOURCE_STRING_COUNT = 11, DEBRIEF_RESOURCE_STRING_CAPACITY = 20 };

enum { DEBRIEF_MISSION_PATH_CAPACITY = 256 };

extern int16_t g_debriefLegacyWord;
extern int16_t g_debriefFlightGroupIffOverrides[DEBRIEF_FLIGHT_GROUP_CAPACITY];

enum { DEBRIEF_RANK_COUNT = 6, DEBRIEF_RANK_NAME_CAPACITY = 14 };

enum {
	DEBRIEF_SOUND_DOOR_OPEN = 1,
	DEBRIEF_SOUND_DOOR_CLOSE = 2,
	DEBRIEF_SOUND_TARGET = 3,
	DEBRIEF_SOUND_REQUEST_TEXT = 4,
	DEBRIEF_SOUND_STOP_TEXT = 5,
	DEBRIEF_SOUND_TICK = 6
};

extern const char g_debriefTourHeaderPrefix[6];

enum { DEBRIEF_PAGE_TEXT_CAPACITY = 32 };

enum {
	DEBRIEF_HEADER_TEXT_CAPACITY = 64,
	DEBRIEF_HEADER_NUMBER_CAPACITY = 32,
	DEBRIEF_HEADER_FONT = 3,
	DEBRIEF_HEADER_COLOR = 14,
	DEBRIEF_HEADER_LINE_HEIGHT = 22,
	DEBRIEF_HISTORIC_TOUR_SOURCE_BASE = 6,
	DEBRIEF_NO_SECOND_MISSION = 255
};

enum {
	DEBRIEF_OBJECT_GOAL_PROTECT_MINES = 0,
	DEBRIEF_OBJECT_GOAL_DESTROY_MINES = 1,
	DEBRIEF_OBJECT_GOAL_PROTECT_SATELLITES = 2,
	DEBRIEF_OBJECT_GOAL_DESTROY_SATELLITES = 3,
	DEBRIEF_OBJECT_GOAL_PROTECT_PROBES = 4,
	DEBRIEF_OBJECT_GOAL_DESTROY_PROBES = 5,
	DEBRIEF_OBJECT_GOAL_COUNT = 6,
	DEBRIEF_GOAL_PREFIX_COUNT = 4,
	DEBRIEF_OBJECT_CATEGORY_COUNT = 3,
	DEBRIEF_SURFACE_GOAL_CATEGORY_COUNT = 2,
	DEBRIEF_GOAL_STRING_CAPACITY = 48,
	DEBRIEF_OBJECT_GOAL_TEXT_CAPACITY = 80,
	DEBRIEF_OBJECT_GOAL_LINE_HEIGHT = 22,
	DEBRIEF_OBJECT_GOAL_FONT = 3,
	DEBRIEF_OBJECT_GOAL_COLOR = 2,
	DEBRIEF_OBJECT_GOAL_LEFT_INSET = 4,
	DEBRIEF_OBJECT_GOAL_RESULTS_PER_ACTION = 2
};

enum {
	DEBRIEF_KILL_NUMBER_CAPACITY = 32,
	DEBRIEF_KILL_TEXT_CAPACITY = 64,
	DEBRIEF_KILL_LINE_HEIGHT = 22,
	DEBRIEF_KILL_LINE_FONT = 3,
	DEBRIEF_KILL_LINE_COLOR = 2,
	DEBRIEF_KILL_LINE_LEFT_INSET = 4
};

enum {
	DEBRIEF_LOSS_TEXT_CAPACITY = 64,
	DEBRIEF_LOSS_NUMBER_CAPACITY = 32,
	DEBRIEF_LOSS_HEADER_LEFT = 4,
	DEBRIEF_LOSS_HEADER_FONT = 3,
	DEBRIEF_LOSS_HEADER_COLOR = 2,
	DEBRIEF_LOSS_COLUMN_LEFT = 20,
	DEBRIEF_LOSS_COLUMN_SPACING = 200,
	DEBRIEF_LOSS_ROWS_TOP = 4,
	DEBRIEF_LOSS_ROW_HEIGHT = 14,
	DEBRIEF_LOSS_FONT = 2,
	DEBRIEF_LOSS_LABEL_COLOR = 51,
	DEBRIEF_LOSS_VALUE_COLOR = 50,
	DEBRIEF_LOSS_BUDGET_ROW_HEIGHT = 20
};

enum {
	DEBRIEF_WEAPON_LASER = 0,
	DEBRIEF_WEAPON_ION = 1,
	DEBRIEF_WEAPON_WARHEAD = 2,
	DEBRIEF_WEAPON_KIND_COUNT = 3,
	DEBRIEF_WEAPON_LABEL_CAPACITY = 48,
	DEBRIEF_ACCURACY_NUMBER_CAPACITY = 32,
	DEBRIEF_ACCURACY_TEXT_CAPACITY = 64,
	DEBRIEF_ACCURACY_HEADING_LEFT = 4,
	DEBRIEF_ACCURACY_HEADING_FONT = 3,
	DEBRIEF_ACCURACY_HEADING_COLOR = 2,
	DEBRIEF_ACCURACY_HEADING_HEIGHT = 22,
	DEBRIEF_ACCURACY_DETAIL_LEFT = 20,
	DEBRIEF_ACCURACY_DETAIL_FONT = 2,
	DEBRIEF_ACCURACY_DETAIL_COLOR = 51,
	DEBRIEF_ACCURACY_DETAIL_HEIGHT = 14,
	DEBRIEF_ACCURACY_SPACE_HEIGHT = 44,
	DEBRIEF_ACCURACY_SURFACE_HEIGHT = 72,
	DEBRIEF_PERCENT_SCALE = 100
};

extern const char g_debriefWeaponShotLabels[DEBRIEF_WEAPON_KIND_COUNT][DEBRIEF_WEAPON_LABEL_CAPACITY];
extern const char g_debriefTotalHitsText[DEBRIEF_WEAPON_LABEL_CAPACITY];
extern const char g_debriefSpacecraftHitsText[DEBRIEF_WEAPON_LABEL_CAPACITY];
extern const char g_debriefSurfaceHitsText[DEBRIEF_WEAPON_LABEL_CAPACITY];

enum { DEBRIEF_KILL_FIRST_IFF = 1, DEBRIEF_KILL_IFF_COUNT = 2 };

extern const char g_debriefSpacecraftKillsText[];
extern const char g_debriefKillsCountFormat[];
extern const char g_debriefLossCountFormat[];
extern const char g_debriefSpacecraftLossesText[];

enum { DEBRIEF_MOUSE_SELECT = 3, DEBRIEF_DOOR_LEFT = 0, DEBRIEF_DOOR_RIGHT = 1, DEBRIEF_DOOR_COUNT = 2 };

enum { DEBRIEF_INPUT_IDLE = 0, DEBRIEF_INPUT_EXIT = 1, DEBRIEF_INPUT_HOVER = 2 };

enum {
	DEBRIEF_AWARD_TEXT_CAPACITY = 64,
	DEBRIEF_AWARD_HEIGHT = 22,
	DEBRIEF_AWARD_FONT = 3,
	DEBRIEF_AWARD_COLOR = 2,
	DEBRIEF_AWARD_LEFT_INSET = 4
};

enum {
	DEBRIEF_MISSION_RESULT_HEIGHT = 22,
	DEBRIEF_MISSION_RESULT_FONT = 3,
	DEBRIEF_MISSION_SUCCESS_COLOR = 14,
	DEBRIEF_MISSION_FAILURE_COLOR = 4
};

enum { DEBRIEF_PREVIOUS_PAGE_BUTTON = 0, DEBRIEF_NEXT_PAGE_BUTTON = 1, DEBRIEF_PAGE_BUTTON_X_OFFSET = -14 };

enum {
	DEBRIEF_HOVER_EXIT_CAPTION = 6,
	DEBRIEF_HOVER_CAPTION_STRIDE = 4,
	DEBRIEF_LABEL_BACKGROUND_COLOR = 16,
	DEBRIEF_LABEL_TEXT_COLOR = 15,
	DEBRIEF_LABEL_FONT = 0,
	DEBRIEF_HOVER_TEXT_OFFSET = 1
};

extern int16_t g_debriefKeyboardFocusX[DEBRIEF_FOCUS_COUNT];
extern int16_t g_debriefKeyboardFocusY[DEBRIEF_FOCUS_COUNT];
extern const char g_debriefResourceStrings[DEBRIEF_RESOURCE_STRING_COUNT][DEBRIEF_RESOURCE_STRING_CAPACITY];
extern const char g_debriefSpaceObjectKillsText[];
extern const char g_debriefDeathStarBuildingKillsText[];
extern const char g_debriefObjectGoalPrefixes[DEBRIEF_GOAL_PREFIX_COUNT][DEBRIEF_GOAL_STRING_CAPACITY];
extern const char g_debriefObjectCategoryNames[DEBRIEF_OBJECT_CATEGORY_COUNT][DEBRIEF_GOAL_STRING_CAPACITY];
extern const char g_debriefPromotionText[];
extern const char g_debriefSurfaceGoalNames[DEBRIEF_SURFACE_GOAL_CATEGORY_COUNT]
										   [DEBRIEF_GOAL_STRING_CAPACITY];
extern const char g_debriefRankNames[DEBRIEF_RANK_COUNT][DEBRIEF_RANK_NAME_CAPACITY];
extern const char g_debriefCraftCategoryNames[DEBRIEF_CRAFT_CATEGORY_COUNT]
											 [DEBRIEF_CRAFT_CATEGORY_NAME_CAPACITY];
extern const char g_debriefGoalOutcomeText[DEBRIEF_GOAL_OUTCOME_COUNT][DEBRIEF_GOAL_OUTCOME_CAPACITY];
extern const int16_t g_debriefCraftCategoryMap[DEBRIEF_CRAFT_CATEGORY_MAP_COUNT];
extern char g_trainingHighScoreNames[DEBRIEF_HIGH_SCORE_COUNT][DEBRIEF_HIGH_SCORE_NAME_CAPACITY];
extern int g_trainingHighScorePoints[DEBRIEF_HIGH_SCORE_COUNT];
extern int16_t g_trainingHighScoreLevels[DEBRIEF_HIGH_SCORE_COUNT];
extern int16_t g_debriefSound36Playing;
extern int16_t g_debriefSound36Activity;
extern int16_t g_debriefFlightGroupInitialStatus[DEBRIEF_FLIGHT_GROUP_CAPACITY];
extern int16_t g_debriefFlightGroupGoals[DEBRIEF_FLIGHT_GROUP_CAPACITY];
extern int16_t g_debriefFlightGroupCraftCounts[DEBRIEF_FLIGHT_GROUP_CAPACITY];
extern REGISTER_PilotFileRecord g_debriefPilotRecord;
extern char g_debriefFlightGroupNames[DEBRIEF_FLIGHT_GROUP_CAPACITY][DEBRIEF_FLIGHT_GROUP_NAME_CAPACITY];
extern int16_t g_debriefPageCount;
extern uint16_t g_debriefMissionSurfaceWord;
extern int16_t g_debriefFlightGroupCount;
extern Actor* g_debriefBackgroundActor;
extern Film* g_debriefFilm;
extern Actor* g_debriefInteriorActor;
extern Actor* g_debriefBaseActor;
extern Actor* g_debriefStatisticsActor;
extern Actor* g_debriefNextPageActor;
extern Input* g_debriefRootInput;
extern Input* g_debriefHoverLabelInput;
extern int16_t g_debriefPageIndex;
extern Actor* g_debriefPreviousPageActor;
extern Input* g_debriefPageControlsInput;
extern int16_t g_debriefSectionPages[DEBRIEF_SECTION_COUNT];
extern int16_t g_debriefFlightGroupCraftTypes[DEBRIEF_FLIGHT_GROUP_CAPACITY];
extern Actor* g_debriefDoorActors[DEBRIEF_DOOR_COUNT];
extern LandruHandle g_debriefBackgroundHandle;
extern int16_t g_debriefKeyboardFocusIndex;

/* Original IDB size: 136 bytes. */
struct XwDebriefKillCategories {
	/* IDB +0x0 */
	uint16_t total[20];
	/* IDB +0x28 */
	uint8_t gap28[8];
	/* IDB +0x30 */
	uint16_t player[20];
	/* IDB +0x58 */
	uint8_t gap58[8];
	/* IDB +0x60 */
	int16_t order[20];
};

/* Original IDB size: 88 bytes. */
struct XwDebriefLossCategories {
	/* IDB +0x0 */
	uint16_t total[20];
	/* IDB +0x28 */
	uint8_t gap28[8];
	/* IDB +0x30 */
	int16_t order[20];
};

enum { DEBRIEF_MUSIC_FADE_DURATION = 300, DEBRIEF_MUSIC_VOLUME = 95 };

extern XwSceneMusicHandles g_debriefMusicState;

/* Declarations follow ascending original IDB address. */

/* 0x442080 */
void debrief_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x442130 */
void debrief_CloseMusic(void);

/* 0x442170 */
void debrief_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm);

/* 0x4421D0 */
void debrief_HandleSoundAction(int16_t action);

/* 0x442470 */
XwShellSceneResult debrief_Debrief(struct XwShellContext* shell);

/* 0x442A60 */
void debrief_end_View(int time);

/* 0x442AF0 */
int16_t debrief_iupdate_Debrief(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
								int rightEvent, int16_t x, int16_t y);

/* 0x442BD0 */
void debrief_iuser_Debrief(Input* input, int context);

/* 0x442CA0 */
void debrief_idraw_HoverLabel(Input* input, Rect* frame, Rect* unusedClip, int16_t refresh);

/* 0x442D00 */
void debrief_iuser_PageButton(Input* input, int unusedTime);

/* 0x442D80 */
void debrief_idraw_PageButton(PushButton* button, Rect* frame, Rect* clip, int16_t refresh);

/* 0x442DE0 */
void debrief_idraw_PageNumber(Input* unusedInput, Rect* frame, Rect* unusedClip, int16_t refresh);

/* 0x442E40 */
void debrief_user_Door(Actor* actor, int time);

/* 0x442ED0 */
void debrief_draw_Background(Actor* unusedActor, Rect* frame, Rect* unusedClip, int16_t x, int16_t y,
							 int16_t refresh);

/* 0x442F20 */
void debrief_draw_Statistics(Actor* unusedActor, Rect* frame, Rect* unusedClip, int16_t unusedX,
							 int16_t unusedY, int16_t refresh);

/* 0x443010 */
void debrief_DrawProvingGroundSummary(Rect* headerRect);

/* 0x4436C0 */
int debrief_BuildSectionPages(void);

/* 0x443820 */
void debrief_DrawMissionSummary(Rect* headerRect);

/* 0x443B00 */
void debrief_DrawMissionHeader(Rect* headerRect);

/* 0x443E20 */
int16_t debrief_SectionMissionResult(Rect* sectionRect, int16_t draw);

/* 0x443E90 */
int16_t debrief_SectionPromotion(Rect* sectionRect, int16_t draw);

/* 0x443F60 */
int16_t debrief_SectionBattlePatch(Rect* sectionRect, int16_t draw);

/* 0x443FE0 */
int16_t debrief_SectionUncompletedGoals(Rect* sectionRect, int16_t draw);

/* 0x444260 */
int16_t debrief_SectionCompletedGoals(Rect* sectionRect, int16_t draw);

/* 0x4444D0 */
int16_t debrief_SectionObjectGoals(Rect* sectionRect, int16_t draw);

/* 0x444660 */
int16_t debrief_SectionSpacecraftKills(Rect* sectionRect, int16_t draw);

/* 0x4449A0 */
int16_t debrief_SectionSpacecraftLosses(Rect* sectionRect, int16_t draw);

/* 0x444CA0 */
int16_t debrief_SectionSpaceObjectKills(Rect* sectionRect, int16_t draw);

/* 0x444D70 */
int16_t debrief_SectionDeathStarBuildingKills(Rect* sectionRect, int16_t draw);

/* 0x444E40 */
int debrief_SectionLaserAccuracy(Rect* sectionRect, int16_t draw);

/* 0x444E80 */
int debrief_SectionIonAccuracy(Rect* sectionRect, int16_t draw);

/* 0x444ED0 */
int debrief_SectionWarheadAccuracy(Rect* sectionRect, int16_t draw);

/* 0x444F00 */
int debrief_SectionWeaponAccuracy(Rect* sectionRect, int16_t weaponKind, uint16_t shotsFired,
								  uint16_t spacecraftHits, uint16_t surfaceHits, int16_t draw);

/* 0x445300 */
void debrief_Update_Debrief_Scores(void);

/* 0x445950 */
int debrief_LoadMissionFlightGroups(void);

/* 0x445C30 */
int16_t debrief_ReloadPilotRecord(void);

#ifdef __cplusplus
}
#endif

#endif
