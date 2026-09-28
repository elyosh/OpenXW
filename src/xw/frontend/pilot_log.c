#include "xw/frontend/pilot_log.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"
#include "xw/landru_config.h"
#include "xw_runtime/integration/awards_callbacks.h"
#include "xw_runtime/integration/input_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/pilot_log_task.h"
#endif

#include <landru/btnpush.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/file.h>
#include <landru/font.h>
#include <landru/inpattr.h>
#include <landru/io.h>
#include <landru/paint.h>
#include <landru/style.h>
#include <landru/view.h>
#include <landru/viewadd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4D5710
const char g_pilotLogRankText[PILOT_LOG_RANK_COUNT][PILOT_LOG_RANK_CAPACITY] = {
	"Flight Cadet ", "Flight Officer ", "Lieutenant ", "Captain ", "Commander ", "General "
};

// GLOBAL: XW 0x4D5788
const char g_pilotLogPilotStatusText[PILOT_LOG_STATUS_COUNT][PILOT_LOG_STATUS_CAPACITY] = {
	"Alive", "Captured", "Rescued", "Dead"
};

// GLOBAL: XW 0x4D59A8
const char g_pilotLogPointsFormat[32] = " with \002%lu\001 Tour of Duty points";

// GLOBAL: XW 0x4D5458
const char g_pilotLogSpaceVictoriesHeading[PILOT_LOG_COMBAT_HEADING_CAPACITY] =
	"TOD Space Victories (Captured): \002";

// GLOBAL: XW 0x4D5484
const char g_pilotLogSurfaceVictoriesHeading[PILOT_LOG_COMBAT_HEADING_CAPACITY] =
	"TOD Surface Victories: \002";

// GLOBAL: XW 0x4D54B0
const char g_pilotLogLasersFiredHeading[PILOT_LOG_COMBAT_HEADING_CAPACITY] = "TOD Lasers Fired: \002";

// GLOBAL: XW 0x4D54DC
const char g_pilotLogCraftHitsHeading[PILOT_LOG_COMBAT_HEADING_CAPACITY] = "Craft Hits: \002";

// GLOBAL: XW 0x4D5508
const char g_pilotLogGroundHitsHeading[PILOT_LOG_COMBAT_HEADING_CAPACITY] = "Ground Hits: \002";

// GLOBAL: XW 0x4D5534
const char g_pilotLogWarheadsFiredHeading[PILOT_LOG_COMBAT_HEADING_CAPACITY] =
	"TOD Homing Projectiles Fired: \002";

// GLOBAL: XW 0x4D5560
const char g_pilotLogCraftLostHeading[PILOT_LOG_COMBAT_HEADING_CAPACITY] = "TOD Craft Lost: \002";

// GLOBAL: XW 0x4D563C
const char g_pilotLogHistoricScoresHeading[] = "Best Scores for Historic Combat Mission";

// GLOBAL: XW 0x4D558C
const char g_pilotLogTrainingLevelsHeading[26] = "Completed Training Levels";

// GLOBAL: XW 0x4D55B8
const char g_pilotLogNoTrainingLevelsText[30] = " has not completed any levels";

// GLOBAL: XW 0x4D55E4
const char g_pilotLogTrainingScoresHeading[21] = "Training Best Scores";

// GLOBAL: XW 0x4D5610
const char g_pilotLogBestScorePrefix[23] = " has a best score of \002";

// GLOBAL: XW 0x4D57B0
const char g_pilotLogShipNames[PILOT_LOG_SHIP_NAME_COUNT][PILOT_LOG_SHIP_NAME_CAPACITY] = {
	"X-Wing", "Y-Wing", "A-Wing", "B-Wing", "Bonus"
};

// GLOBAL: XW 0x4D59CC
const char g_pilotLogTrainingLevelFormat[25] = " has completed \002level %u";

// GLOBAL: XW 0x4D5694
char g_pilotLogOperationHeading[PILOT_LOG_TOUR_HEADING_CAPACITY] = "Scores for Each Operation";

// GLOBAL: XW 0x4D56C0
const char g_pilotLogTourStatusText[PILOT_LOG_TOUR_STATUS_COUNT][PILOT_LOG_TOUR_STATUS_CAPACITY] = {
	"is Inactive", "is Active", "is Incomplete", "is Complete"
};

// GLOBAL: XW 0x4D57D8
const char g_pilotLogCraftNames[PILOT_LOG_COMBAT_CATEGORY_COUNT][PILOT_LOG_COMBAT_NAME_CAPACITY] = {
	"TIE Fighter", "TIE Interceptor", "TIE Bomber", "TIE Advanced", "Assault Gunboat",
	"X-Wing",      "Y-Wing",          "A-Wing",     "Transport",    "Shuttle",
	"Tug",         "Container",       "Freighter",  "Calamari",     "Nebulon B",
	"Corvette",    "Star Destroyer",  "Mine",       "Comm Sat",     "Space Probe",
};

// GLOBAL: XW 0x4D5968
const int16_t g_pilotLogCraftCategoryMap[PILOT_LOG_COMBAT_CRAFT_COUNT] = { 5, 6,  7,  0,  1,  2,  4,  8,
																		   9, 10, 11, 12, 13, 14, 15, 16,
																		   3, 17, 17, 17, 17, 18, 18, 19 };

// GLOBAL: XW 0x4D5A34
const char g_pilotLogKillCaptureFormat[9] = "\002%d (%d)";

// GLOBAL: XW 0x4D5B50
const char* g_pilotLogMusicFilename = "unmusic.lfd";

// GLOBAL: XW 0x4D5B54
const char* g_pilotLogMusicName = "uniform";

// GLOBAL: XW 0x4F79C8
int16_t g_pilotLogPageCount = 0;

// GLOBAL: XW 0x4F79CC
Input* g_pilotLogRootInput = NULL;

// GLOBAL: XW 0x4F79D0
int16_t g_pilotLogPage = 0;

// GLOBAL: XW 0x4F79D8
REGISTER_PilotFileRecord g_pilotLogRecord = { 0 };

// GLOBAL: XW 0x4F8084
Input* g_pilotLogPageInput = NULL;

// GLOBAL: XW 0x4F8088
Film* g_pilotLogFilm = NULL;

// GLOBAL: XW 0x4F80B4
XwSceneMusicHandles g_pilotLogMusicState = { NULL, NULL };

// FUNCTION: XW 0x4557F0
XwShellSceneResult PilotLog_Show(struct XwShellContext* shell) {
	ResFile* resourceFile;
	PushButton* previousButton;
	PushButton* nextButton;
	PushButton* exitButton;
	Input* pageIndicator;
	Rect controlFrame;
	char pilotFilename[PILOT_LOG_FILENAME_CAPACITY];
	size_t tourIndex;
	strcpy(pilotFilename, g_RegisterShellPilot.name);
	strcat(pilotFilename, ".PLT");
	memset(&g_pilotLogRecord, 0, sizeof(g_pilotLogRecord));
	PilotLog_LoadPilotRecord(pilotFilename);
	g_pilotLogPage = PILOT_LOG_TRAINING_PAGE;
	g_pilotLogPageCount = PILOT_LOG_BASE_PAGE_COUNT;
	for (tourIndex = 0; tourIndex < sizeof(g_pilotLogRecord.tourOperationProgress) /
										sizeof(g_pilotLogRecord.tourOperationProgress[0]);
		 ++tourIndex) {
		if (g_pilotLogRecord.tourOperationProgress[tourIndex] != 0)
			++g_pilotLogPageCount;
	}
	if (g_pilotLogPageCount > PILOT_LOG_BASE_PAGE_COUNT)
		++g_pilotLogPageCount;
	resourceFile = xres_Open_Resource("log640.lfd");
	xcanvas_Get_Drawing_Canvas_Bounds(&controlFrame);
	g_pilotLogFilm =
		xfilm_Res_Callback_Film(resourceFile, "log_640", &controlFrame, 0, 0, 0, PilotLog_film_Callback);
	xfilm_Set_Film_Def_Palette(g_pilotLogFilm, shell->standardPalette);
	g_pilotLogRootInput = xinput_Alloc_Input(NULL, &controlFrame, 0, 0);
	xrect_Set_Rect(&controlFrame, PILOT_LOG_PAGE_LEFT, PILOT_LOG_PAGE_TOP, PILOT_LOG_PAGE_RIGHT,
				   PILOT_LOG_PAGE_BOTTOM);
	g_pilotLogPageInput = xinput_Alloc_Input(g_pilotLogRootInput, &controlFrame, 0, 0);
	xinpattr_Set_Input_Update_Function(g_pilotLogPageInput, XwInput_UpdateNoOp);
	xinpattr_Set_Input_User_Function(g_pilotLogPageInput, XwInput_UserNoOp);
	xinpattr_Set_Input_Draw_Function(g_pilotLogPageInput, PilotLog_idraw_Page);
	g_pilotLogPageInput->mouseUsage = allInput;
	xinpattr_Show_Input(g_pilotLogPageInput);
	xrect_Set_Rect(&controlFrame, PILOT_LOG_PREVIOUS_LEFT, PILOT_LOG_NAV_TOP, PILOT_LOG_PREVIOUS_RIGHT,
				   PILOT_LOG_NAV_BOTTOM);
	previousButton = xbtnpush_Alloc_Button(g_pilotLogRootInput, &controlFrame, 0, PilotLog_iuser_Navigation,
										   NULL, PILOT_LOG_NAV_PREVIOUS);
	xinpattr_Set_Input_Draw_Function(&previousButton->header, PilotLog_idraw_Navigation);
	xrect_Set_Rect(&controlFrame, PILOT_LOG_NEXT_LEFT, PILOT_LOG_NAV_TOP, PILOT_LOG_NEXT_RIGHT,
				   PILOT_LOG_NAV_BOTTOM);
	nextButton = xbtnpush_Alloc_Button(g_pilotLogRootInput, &controlFrame, 0, PilotLog_iuser_Navigation, NULL,
									   PILOT_LOG_NAV_NEXT);
	xinpattr_Set_Input_Draw_Function(&nextButton->header, PilotLog_idraw_Navigation);
	xrect_Set_Rect(&controlFrame, PILOT_LOG_EXIT_LEFT, PILOT_LOG_NAV_TOP, PILOT_LOG_EXIT_RIGHT,
				   PILOT_LOG_NAV_BOTTOM);
	exitButton = xbtnpush_Alloc_Small_Button(g_pilotLogRootInput, &controlFrame, 0, PilotLog_iuser_Navigation,
											 "Exit Pilot Log", PILOT_LOG_NAV_EXIT);
	xinpattr_Set_Input_Draw_Function(&exitButton->header, XwAwardsUI_DrawTextButton);
	xrect_Set_Rect(&controlFrame, PILOT_LOG_INDICATOR_LEFT, PILOT_LOG_NAV_TOP, PILOT_LOG_INDICATOR_RIGHT,
				   PILOT_LOG_NAV_BOTTOM);
	pageIndicator = xinput_Alloc_Input(g_pilotLogRootInput, &controlFrame, 0, 0);
	xinpattr_Set_Input_Draw_Function(pageIndicator, PilotLog_idraw_Navigation);
	pageIndicator->id = PILOT_LOG_NAV_PAGE_LABEL;
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xview_Set_View_Update_Function(PilotLog_end_View);
	xio_Set_Key_Buttons();
	PilotLog_OpenMusic(resourceFile, g_pilotLogFilm);
	soundext_RecheckSfxPreference();
#ifdef XW_MODERN
	XwPilotLog_RunView(resourceFile);
#else
	j_xviewadd_Handle_View();
	PilotLog_CloseMusic();
	soundext_RecheckSfxPreference();
	xcursor_Hide_Cursor();
	xio_Clear_Key_Buttons();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xres_Close_Resource(resourceFile);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x455B50
void PilotLog_end_View(int time) {
	if (time == 0 && (uint16_t)xcursor_Is_Cursor_Visible() == 0) {
		xcursor_Show_Cursor();
	}
	xio_Get_Free_Key();
}

// FUNCTION: XW 0x455B70
int16_t PilotLog_film_Callback(Film* film, FilmObject* object) {
	if (object->id == FTC_ACTOR) {
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		xactor_Non_Refreshable_Actor(object->object);
	}
	return 0;
}

// FUNCTION: XW 0x455BA0
void PilotLog_idraw_Page(Input* unusedInput, Rect* frame, Rect* unusedClip, int16_t refresh) {
	Rect headerRect;
	char pointsSuffix[PILOT_LOG_POINTS_TEXT_CAPACITY];
	char headerText[PILOT_LOG_HEADER_TEXT_CAPACITY];
	(void)unusedInput;
	(void)unusedClip;
	if (refresh != 0) {
		xpaint_Paint_Clipped_Rect(frame, PILOT_LOG_BACKGROUND_COLOR);
		xfont_Enable_FontID_Shadow(PILOT_LOG_TRAINING_TITLE_FONT);
		xfont_Set_FontID_Bold_Color(PILOT_LOG_TRAINING_TITLE_FONT, PILOT_LOG_PAGE_COLOR);
		xfont_Enable_FontID_Shadow(PILOT_LOG_TRAINING_ROW_FONT);
		xfont_Set_FontID_Bold_Color(PILOT_LOG_TRAINING_ROW_FONT, PILOT_LOG_PAGE_COLOR);
		xrect_Copy_Rect(&headerRect, frame);
		headerRect.bottom = headerRect.top + PILOT_LOG_HEADER_HEIGHT;
		xpaint_Paint_Clipped_Rect(&headerRect, PILOT_LOG_HEADER_BACKGROUND_COLOR);
		xpaint_Horiz_Clipped_Line(headerRect.left, headerRect.bottom, headerRect.right - headerRect.left,
								  PILOT_LOG_HEADER_LINE_COLOR);
		++headerRect.top;
		headerRect.bottom = headerRect.top + PILOT_LOG_HEADER_ROW_HEIGHT;
		strcpy(headerText, g_pilotLogRankText[g_RegisterShellPilot.rank]);
		strcat(headerText, g_RegisterShellPilot.name);
		xfont_Print_Centered_Text(headerText, &headerRect, PILOT_LOG_TRAINING_TITLE_FONT,
								  PILOT_LOG_HEADER_TEXT_COLOR);
		strcpy(headerText, g_pilotLogPilotStatusText[g_RegisterShellPilot.lost_status]);
		xrect_Offset_Rect(&headerRect, 0, PILOT_LOG_HEADER_ROW_HEIGHT);
		sprintf(pointsSuffix, g_pilotLogPointsFormat, (unsigned long)g_pilotLogRecord.score);
		strcat(headerText, pointsSuffix);
		xfont_Print_Centered_Text(headerText, &headerRect, PILOT_LOG_TRAINING_TITLE_FONT,
								  PILOT_LOG_HEADER_TEXT_COLOR);
		switch (g_pilotLogPage) {
			case PILOT_LOG_TRAINING_PAGE:
				PilotLog_DrawTrainingPage(frame);
				break;
			case PILOT_LOG_HISTORIC_PAGE:
				PilotLog_DrawHistoricPage(frame);
				break;
			case PILOT_LOG_BONUS_PAGE:
				PilotLog_DrawBonusPage(frame);
				break;
			case PILOT_LOG_COMBAT_PAGE:
				PilotLog_DrawCombatStatistics(frame);
				break;
			default:
				PilotLog_DrawTourPage(frame);
				break;
		}
		xfont_Disable_FontID_Shadow(PILOT_LOG_TRAINING_TITLE_FONT);
		xfont_Disable_FontID_Shadow(PILOT_LOG_TRAINING_ROW_FONT);
	}
}

// FUNCTION: XW 0x455DD0
void PilotLog_DrawTrainingPage(const Rect* frame) {
	Rect rowRect;
	char text[PILOT_LOG_TRAINING_TEXT_CAPACITY];
	char valueText[PILOT_LOG_TRAINING_TEXT_CAPACITY];
	int16_t shipIndex;
	xrect_Copy_Rect(&rowRect, (Rect*)frame);
	xrect_Inset_Rect(&rowRect, PILOT_LOG_TRAINING_INSET, 0);
	rowRect.top += PILOT_LOG_TRAINING_TOP;
	rowRect.bottom = rowRect.top + PILOT_LOG_TRAINING_TITLE_HEIGHT;
	strcpy(text, g_pilotLogTrainingLevelsHeading);
	xfont_Print_Clipped_Text(text, rowRect.left, rowRect.top, PILOT_LOG_TRAINING_TITLE_FONT,
							 PILOT_LOG_TRAINING_TITLE_COLOR);
	xrect_Offset_Rect(&rowRect, 0, PILOT_LOG_TRAINING_HEADING_GAP);
	for (shipIndex = 0; shipIndex < PILOT_LOG_TRAINING_SHIP_COUNT; ++shipIndex) {
		if (shipext_IsShipAvailable(shipIndex) != 0) {
			xrect_Offset_Rect(&rowRect, 0, PILOT_LOG_TRAINING_ROW_HEIGHT);
			strcpy(text, g_pilotLogShipNames[shipIndex]);
			if (g_pilotLogRecord.trainingLevelProgress[shipIndex] != 0)
				sprintf(valueText, g_pilotLogTrainingLevelFormat,
						g_pilotLogRecord.trainingLevelProgress[shipIndex]);
			else
				strcpy(valueText, g_pilotLogNoTrainingLevelsText);
			strcat(text, valueText);
			xfont_Print_Clipped_Text(text, rowRect.left + PILOT_LOG_TRAINING_ROW_LEFT, rowRect.top,
									 PILOT_LOG_TRAINING_ROW_FONT, PILOT_LOG_TRAINING_ROW_COLOR);
		}
	}
	xrect_Offset_Rect(&rowRect, 0, PILOT_LOG_TRAINING_SECTION_GAP);
	strcpy(text, g_pilotLogTrainingScoresHeading);
	xfont_Print_Clipped_Text(text, rowRect.left, rowRect.top, PILOT_LOG_TRAINING_TITLE_FONT,
							 PILOT_LOG_TRAINING_TITLE_COLOR);
	xrect_Offset_Rect(&rowRect, 0, PILOT_LOG_TRAINING_HEADING_GAP);
	for (shipIndex = 0; shipIndex < PILOT_LOG_TRAINING_SHIP_COUNT; ++shipIndex) {
		if (shipext_IsShipAvailable(shipIndex) != 0) {
			xrect_Offset_Rect(&rowRect, 0, PILOT_LOG_TRAINING_ROW_HEIGHT);
			strcpy(text, g_pilotLogShipNames[shipIndex]);
			strcat(text, g_pilotLogBestScorePrefix);
			sprintf(valueText, "%lu", (unsigned long)g_pilotLogRecord.trainingBestScores[shipIndex]);
			strcat(text, valueText);
			xfont_Print_Clipped_Text(text, rowRect.left + PILOT_LOG_TRAINING_ROW_LEFT, rowRect.top,
									 PILOT_LOG_TRAINING_ROW_FONT, PILOT_LOG_TRAINING_ROW_COLOR);
		}
	}
}

// FUNCTION: XW 0x4560D0
void PilotLog_DrawHistoricPage(const Rect* frame) {
	Rect rowRect;
	char text[PILOT_LOG_TRAINING_TEXT_CAPACITY];
	char rangeText[PILOT_LOG_TRAINING_TEXT_CAPACITY];
	int16_t rangeStart;
	int16_t rangeEnd;
	int16_t printedRanges;
	int16_t missionIndex;
	int16_t shipIndex;
	int16_t headingColor;
	int16_t scoreColor;
	xrect_Copy_Rect(&rowRect, (Rect*)frame);
	xrect_Inset_Rect(&rowRect, PILOT_LOG_TRAINING_INSET, 0);
	rowRect.top += PILOT_LOG_TRAINING_TOP;
	rowRect.bottom = rowRect.top + PILOT_LOG_TRAINING_TITLE_HEIGHT;
	strcpy(text, g_pilotLogHistoricScoresHeading);
	xfont_Print_Clipped_Text(text, rowRect.left, rowRect.top, PILOT_LOG_TRAINING_TITLE_FONT,
							 PILOT_LOG_TRAINING_TITLE_COLOR);
	xrect_Offset_Rect(&rowRect, 0, PILOT_LOG_TRAINING_SECTION_GAP);
	for (shipIndex = 0; shipIndex < PILOT_LOG_HISTORIC_SHIP_COUNT; ++shipIndex) {
		strcpy(text, g_pilotLogShipNames[shipIndex]);
		strcat(text, " Historic Combat Missions (Completed ");
		rangeEnd = PILOT_LOG_BONUS_RANGE_NONE;
		rangeStart = PILOT_LOG_BONUS_RANGE_NONE;
		printedRanges = 0;
		for (missionIndex = 0; missionIndex < PILOT_LOG_HISTORIC_MISSION_COUNT; ++missionIndex) {
			if (g_pilotLogRecord.combatAwards[shipIndex].missionPatch[missionIndex] != 0) {
				if (rangeStart == PILOT_LOG_BONUS_RANGE_NONE) {
					rangeStart = missionIndex;
					rangeEnd = missionIndex;
				} else if (rangeEnd == missionIndex - 1) {
					rangeEnd = missionIndex;
				} else {
					if (printedRanges != 0)
						strcat(text, ", ");
					if (rangeEnd == rangeStart)
						sprintf(rangeText, "%d", rangeEnd + 1);
					else
						sprintf(rangeText, "%d-%d", rangeStart + 1, rangeEnd + 1);
					strcat(text, rangeText);
					rangeStart = missionIndex;
					rangeEnd = missionIndex;
					++printedRanges;
				}
			}
		}
		if (rangeEnd != PILOT_LOG_BONUS_RANGE_NONE) {
			if (printedRanges != 0)
				strcat(text, ", ");
			if (rangeEnd == rangeStart)
				sprintf(rangeText, "%d)", rangeEnd + 1);
			else
				sprintf(rangeText, "%d-%d)", rangeStart + 1, rangeEnd + 1);
			strcat(text, rangeText);
		} else
			strcat(text, "None)");
		if (shipIndex & 1) {
			headingColor = PILOT_LOG_HISTORIC_ODD_HEADING_COLOR;
			scoreColor = PILOT_LOG_HISTORIC_ODD_SCORE_COLOR;
		} else {
			headingColor = PILOT_LOG_TRAINING_ROW_COLOR;
			scoreColor = PILOT_LOG_BONUS_SCORE_COLOR;
		}
		xfont_Print_Clipped_Text(text, rowRect.left + PILOT_LOG_BONUS_SUMMARY_LEFT, rowRect.top,
								 PILOT_LOG_TRAINING_ROW_FONT, headingColor);
		for (missionIndex = 0; missionIndex < PILOT_LOG_HISTORIC_MISSION_COUNT; ++missionIndex) {
			xrect_Offset_Rect(&rowRect, 0, PILOT_LOG_TRAINING_ROW_HEIGHT);
			sprintf(text, "Mission \002%d\001 with \002%lu\001 Points", missionIndex + 1,
					(unsigned long)g_pilotLogRecord.historicBestScores[shipIndex][missionIndex]);
			if (missionIndex >= PILOT_LOG_HISTORIC_COLUMN_ROWS) {
				if (missionIndex == PILOT_LOG_HISTORIC_COLUMN_ROWS) {
					xrect_Offset_Rect(&rowRect, 0,
									  -PILOT_LOG_HISTORIC_COLUMN_ROWS * PILOT_LOG_TRAINING_ROW_HEIGHT);
				}
				xfont_Print_Clipped_Text(text, rowRect.left + PILOT_LOG_HISTORIC_SECOND_COLUMN_LEFT,
										 rowRect.top, PILOT_LOG_TRAINING_ROW_FONT, scoreColor);
			} else {
				xfont_Print_Clipped_Text(text, rowRect.left + PILOT_LOG_TRAINING_ROW_LEFT, rowRect.top,
										 PILOT_LOG_TRAINING_ROW_FONT, scoreColor);
			}
		}
		xrect_Offset_Rect(&rowRect, 0, PILOT_LOG_HISTORIC_SECTION_GAP);
	}
}

// FUNCTION: XW 0x4564B0
void PilotLog_DrawBonusPage(const Rect* frame) {
	Rect rowRect;
	char text[PILOT_LOG_TRAINING_TEXT_CAPACITY];
	char rangeText[PILOT_LOG_TRAINING_TEXT_CAPACITY];
	int16_t rangeStart;
	int16_t rangeEnd;
	int16_t printedRanges;
	int16_t missionIndex;
	xrect_Copy_Rect(&rowRect, (Rect*)frame);
	xrect_Inset_Rect(&rowRect, PILOT_LOG_TRAINING_INSET, 0);
	rowRect.top += PILOT_LOG_TRAINING_TOP;
	rowRect.bottom = rowRect.top + PILOT_LOG_TRAINING_TITLE_HEIGHT;
	strcpy(text, g_pilotLogHistoricScoresHeading);
	xfont_Print_Clipped_Text(text, rowRect.left, rowRect.top, PILOT_LOG_TRAINING_TITLE_FONT,
							 PILOT_LOG_TRAINING_TITLE_COLOR);
	xrect_Offset_Rect(&rowRect, 0, PILOT_LOG_TRAINING_SECTION_GAP);
	strcpy(text, g_pilotLogShipNames[PILOT_LOG_BONUS_SHIP_INDEX]);
	strcat(text, " Historic Combat Missions (Completed ");
	rangeEnd = PILOT_LOG_BONUS_RANGE_NONE;
	rangeStart = PILOT_LOG_BONUS_RANGE_NONE;
	printedRanges = 0;
	for (missionIndex = 0; missionIndex < PILOT_LOG_BONUS_MISSION_COUNT; ++missionIndex) {
		if (g_pilotLogRecord.bonusHistoricCompleted[missionIndex] != 0) {
			if (rangeStart == PILOT_LOG_BONUS_RANGE_NONE) {
				rangeStart = missionIndex;
				rangeEnd = missionIndex;
			} else if (rangeEnd == missionIndex - 1) {
				rangeEnd = missionIndex;
			} else {
				if (printedRanges != 0)
					strcat(text, ", ");
				if (rangeEnd == rangeStart)
					sprintf(rangeText, "%d", rangeEnd + 1);
				else
					sprintf(rangeText, "%d-%d", rangeStart + 1, rangeEnd + 1);
				strcat(text, rangeText);
				rangeStart = missionIndex;
				rangeEnd = missionIndex;
				++printedRanges;
			}
		}
	}
	if (rangeEnd != PILOT_LOG_BONUS_RANGE_NONE) {
		if (printedRanges != 0)
			strcat(text, ", ");
		if (rangeEnd == rangeStart)
			sprintf(rangeText, "%d)", rangeEnd + 1);
		else
			sprintf(rangeText, "%d-%d)", rangeStart + 1, rangeEnd + 1);
		strcat(text, rangeText);
	} else
		strcat(text, "None)");
	xfont_Print_Clipped_Text(text, rowRect.left + PILOT_LOG_BONUS_SUMMARY_LEFT, rowRect.top,
							 PILOT_LOG_TRAINING_ROW_FONT, PILOT_LOG_TRAINING_ROW_COLOR);
	for (missionIndex = 0; missionIndex < PILOT_LOG_BONUS_MISSION_COUNT; ++missionIndex) {
		xrect_Offset_Rect(&rowRect, 0, PILOT_LOG_TRAINING_ROW_HEIGHT);
		sprintf(text, "Mission \002%d\001 with \002%lu\001 Points", missionIndex + 1,
				(unsigned long)g_pilotLogRecord.bonusHistoricBestScores[missionIndex]);
		xfont_Print_Clipped_Text(text, rowRect.left + PILOT_LOG_TRAINING_ROW_LEFT, rowRect.top,
								 PILOT_LOG_TRAINING_ROW_FONT, PILOT_LOG_BONUS_SCORE_COLOR);
	}
}

// FUNCTION: XW 0x4567E0
void PilotLog_DrawCombatStatistics(const Rect* frame) {
	Rect sectionRect;
	char valueText[PILOT_LOG_COMBAT_VALUE_CAPACITY];
	char text[PILOT_LOG_TRAINING_TEXT_CAPACITY];
	XwPilotLogCombatTotals categories;
	int16_t categoryIndex;
	int craftIndex, remainingCraftTypes;
	uint16_t killTotal, captureTotal;
	int16_t nonzeroCategories;
	int16_t sortDone;
	int16_t columnX, rowY;
	int rowsPerColumn, rowIndex, rowsRemaining;
	unsigned int laserCraftPercent, laserGroundPercent;
	int warheadCraftPercent, warheadGroundPercent;
	xrect_Copy_Rect(&sectionRect, (Rect*)frame);
	xrect_Inset_Rect(&sectionRect, PILOT_LOG_TRAINING_INSET, 0);
	sectionRect.top += PILOT_LOG_TRAINING_TOP;
	sectionRect.bottom = sectionRect.top + PILOT_LOG_TRAINING_TITLE_HEIGHT;
	memset(categories.captures, 0, sizeof(categories.captures));
	memset(categories.kills, 0, sizeof(categories.kills));
	for (categoryIndex = 0; categoryIndex < PILOT_LOG_COMBAT_CATEGORY_COUNT; ++categoryIndex)
		categories.order[categoryIndex] = categoryIndex;
	captureTotal = 0;
	killTotal = 0;
	for (craftIndex = 0, remainingCraftTypes = PILOT_LOG_COMBAT_CRAFT_COUNT; remainingCraftTypes != 0;
		 ++craftIndex, --remainingCraftTypes) {
		uint16_t kills = g_pilotLogRecord.kills_by_type[craftIndex];
		uint16_t captures = g_pilotLogRecord.captures_by_type[craftIndex];
		int category = g_pilotLogCraftCategoryMap[craftIndex];
		killTotal += kills;
		captureTotal += captures;
		categories.kills[category] += kills;
		categories.captures[category] += captures;
	}
	nonzeroCategories = 0;
	for (categoryIndex = 0; categoryIndex < PILOT_LOG_COMBAT_CATEGORY_COUNT; ++categoryIndex) {
		if (categories.kills[categoryIndex] != 0 || categories.captures[categoryIndex] != 0)
			++nonzeroCategories;
	}
	do {
		sortDone = 1;
		for (categoryIndex = 1; categoryIndex < PILOT_LOG_COMBAT_CATEGORY_COUNT; ++categoryIndex) {
			int16_t previousCategory = categories.order[categoryIndex - 1];
			int16_t nextCategory = categories.order[categoryIndex];
			int16_t previousKills = categories.kills[previousCategory];
			int16_t nextKills = categories.kills[nextCategory];
			if (previousKills < nextKills ||
				(previousKills == nextKills &&
				 categories.captures[previousCategory] < categories.captures[nextCategory])) {
				categories.order[categoryIndex - 1] = nextCategory;
				categories.order[categoryIndex] = previousCategory;
				sortDone = 0;
			}
		}
	} while (!sortDone);
	strcpy(text, g_pilotLogSpaceVictoriesHeading);
	sprintf(valueText, "%d (%d)", (int16_t)killTotal, (int16_t)captureTotal);
	strcat(text, valueText);
	xfont_Print_Clipped_Text(text, sectionRect.left, sectionRect.top, PILOT_LOG_TRAINING_TITLE_FONT,
							 PILOT_LOG_TRAINING_TITLE_COLOR);
	columnX = sectionRect.left + PILOT_LOG_TRAINING_ROW_LEFT;
	rowY = sectionRect.bottom + PILOT_LOG_TRAINING_HEADING_GAP;
	if (nonzeroCategories > 0) {
		rowsPerColumn = (nonzeroCategories + 1) >> 1;
		for (rowIndex = 0, rowsRemaining = nonzeroCategories; rowsRemaining != 0;
			 ++rowIndex, --rowsRemaining) {
			int16_t displayCategory = categories.order[rowIndex];
			if (rowIndex == rowsPerColumn) {
				columnX += PILOT_LOG_COMBAT_COLUMN_GAP;
				rowY = sectionRect.bottom + PILOT_LOG_TRAINING_HEADING_GAP;
			}
			if (categories.kills[displayCategory] != 0 || categories.captures[displayCategory] != 0) {
				int16_t labelWidth;
				strcpy(text, g_pilotLogCraftNames[displayCategory]);
				strcat(text, ":  ");
				xfont_Print_Clipped_Text(text, columnX, rowY, PILOT_LOG_TRAINING_ROW_FONT,
										 PILOT_LOG_TRAINING_ROW_COLOR);
				labelWidth = xfont_Get_String_Width_0(PILOT_LOG_TRAINING_ROW_FONT, text);
				sprintf(text, g_pilotLogKillCaptureFormat, categories.kills[displayCategory],
						categories.captures[displayCategory]);
				xfont_Print_Clipped_Text(text, columnX + labelWidth, rowY, PILOT_LOG_TRAINING_ROW_FONT,
										 PILOT_LOG_BONUS_SCORE_COLOR);
				rowY += PILOT_LOG_TRAINING_ROW_HEIGHT;
			}
		}
	}
	xrect_Offset_Rect(&sectionRect, 0,
					  PILOT_LOG_TRAINING_ROW_HEIGHT * ((nonzeroCategories + 1) / 2) +
						  PILOT_LOG_COMBAT_TOTALS_GAP);
	strcpy(text, g_pilotLogSurfaceVictoriesHeading);
	sprintf(valueText, "%d", g_pilotLogRecord.surfaceVictories);
	strcat(text, valueText);
	xfont_Print_Clipped_Text(text, sectionRect.left, sectionRect.top, PILOT_LOG_TRAINING_TITLE_FONT,
							 PILOT_LOG_TRAINING_TITLE_COLOR);
	xrect_Offset_Rect(&sectionRect, 0, PILOT_LOG_TRAINING_SECTION_GAP);
	strcpy(text, g_pilotLogLasersFiredHeading);
	sprintf(valueText, "%ld", (long)(int32_t)g_pilotLogRecord.laser_shots_fired);
	strcat(text, valueText);
	xfont_Print_Clipped_Text(text, sectionRect.left, sectionRect.top, PILOT_LOG_TRAINING_TITLE_FONT,
							 PILOT_LOG_TRAINING_TITLE_COLOR);
	if (g_pilotLogRecord.laser_shots_fired != 0)
		laserCraftPercent = PILOT_LOG_PERCENT_SCALE * g_pilotLogRecord.laser_spacecraft_hits /
							g_pilotLogRecord.laser_shots_fired;
	else
		laserCraftPercent = 0;
	xrect_Offset_Rect(&sectionRect, 0, PILOT_LOG_COMBAT_HITS_GAP);
	strcpy(text, g_pilotLogCraftHitsHeading);
	sprintf(valueText, "%ld (%ld%%)", (long)(int32_t)g_pilotLogRecord.laser_spacecraft_hits,
			(long)(int32_t)laserCraftPercent);
	strcat(text, valueText);
	xfont_Print_Clipped_Text(text, sectionRect.left + PILOT_LOG_TRAINING_ROW_LEFT, sectionRect.top,
							 PILOT_LOG_TRAINING_ROW_FONT, PILOT_LOG_TRAINING_ROW_COLOR);
	if (g_pilotLogRecord.laser_shots_fired != 0)
		laserGroundPercent = PILOT_LOG_PERCENT_SCALE * g_pilotLogRecord.laser_surface_hits /
							 g_pilotLogRecord.laser_shots_fired;
	else
		laserGroundPercent = 0;
	strcpy(text, g_pilotLogGroundHitsHeading);
	sprintf(valueText, "%ld (%ld%%)", (long)(int32_t)g_pilotLogRecord.laser_surface_hits,
			(long)(int32_t)laserGroundPercent);
	strcat(text, valueText);
	xfont_Print_Clipped_Text(text, sectionRect.left + PILOT_LOG_TRAINING_ROW_LEFT,
							 sectionRect.top + PILOT_LOG_TRAINING_ROW_HEIGHT, PILOT_LOG_TRAINING_ROW_FONT,
							 PILOT_LOG_TRAINING_ROW_COLOR);
	xrect_Offset_Rect(&sectionRect, 0, PILOT_LOG_COMBAT_WEAPON_GAP);
	strcpy(text, g_pilotLogWarheadsFiredHeading);
	sprintf(valueText, "%u", (unsigned int)g_pilotLogRecord.warheads_fired);
	strcat(text, valueText);
	xfont_Print_Clipped_Text(text, sectionRect.left, sectionRect.top, PILOT_LOG_TRAINING_TITLE_FONT,
							 PILOT_LOG_TRAINING_TITLE_COLOR);
	if (g_pilotLogRecord.warheads_fired != 0)
		warheadCraftPercent = PILOT_LOG_PERCENT_SCALE * g_pilotLogRecord.warhead_spacecraft_hits /
							  g_pilotLogRecord.warheads_fired;
	else
		warheadCraftPercent = 0;
	xrect_Offset_Rect(&sectionRect, 0, PILOT_LOG_COMBAT_HITS_GAP);
	strcpy(text, g_pilotLogCraftHitsHeading);
	sprintf(valueText, "%u (%ld%%)", (unsigned int)g_pilotLogRecord.warhead_spacecraft_hits,
			(long)warheadCraftPercent);
	strcat(text, valueText);
	xfont_Print_Clipped_Text(text, sectionRect.left + PILOT_LOG_TRAINING_ROW_LEFT, sectionRect.top,
							 PILOT_LOG_TRAINING_ROW_FONT, PILOT_LOG_TRAINING_ROW_COLOR);
	if (g_pilotLogRecord.warheads_fired != 0)
		warheadGroundPercent =
			PILOT_LOG_PERCENT_SCALE * g_pilotLogRecord.warhead_surface_hits / g_pilotLogRecord.warheads_fired;
	else
		warheadGroundPercent = 0;
	strcpy(text, g_pilotLogGroundHitsHeading);
	sprintf(valueText, "%u (%ld%%)", (unsigned int)g_pilotLogRecord.warhead_surface_hits,
			(long)warheadGroundPercent);
	strcat(text, valueText);
	xfont_Print_Clipped_Text(text, sectionRect.left + PILOT_LOG_TRAINING_ROW_LEFT,
							 sectionRect.top + PILOT_LOG_TRAINING_ROW_HEIGHT, PILOT_LOG_TRAINING_ROW_FONT,
							 PILOT_LOG_TRAINING_ROW_COLOR);
	xrect_Offset_Rect(&sectionRect, 0, PILOT_LOG_COMBAT_WEAPON_GAP);
	strcpy(text, g_pilotLogCraftLostHeading);
	sprintf(valueText, "%u", (unsigned int)g_pilotLogRecord.ejections);
	strcat(text, valueText);
	xfont_Print_Clipped_Text(text, sectionRect.left, sectionRect.top, PILOT_LOG_TRAINING_TITLE_FONT,
							 PILOT_LOG_TRAINING_TITLE_COLOR);
}

// FUNCTION: XW 0x4570C0
void PilotLog_DrawTourPage(const Rect* frame) {
	int16_t selectedTour = 0;
	int16_t tourPage = PILOT_LOG_TOUR_FIRST_PAGE;
	int16_t tourIndex;
	int16_t operationCount;
	int operationIndex;
	int remainingOperations;
	Rect rowRect;
	char lineText[PILOT_LOG_TOUR_LINE_CAPACITY];
	for (tourIndex = 0; tourIndex < SHIPEXT_TOUR_COUNT; ++tourIndex) {
		if (g_pilotLogRecord.tourOperationProgress[tourIndex] != 0) {
			if (tourPage == g_pilotLogPage) {
				selectedTour = tourIndex;
				break;
			}
			++tourPage;
		}
	}
	operationCount = g_pilotLogRecord.tourOperationProgress[selectedTour];
	sprintf(lineText, "Tour \x02%d\x01 ", selectedTour + 1);
	strcat(lineText, g_pilotLogTourStatusText[g_pilotLogRecord.tour_status[selectedTour]]);
	xrect_Copy_Rect(&rowRect, (Rect*)frame);
	xrect_Inset_Rect(&rowRect, PILOT_LOG_TOUR_FRAME_INSET, 0);
	rowRect.top += PILOT_LOG_TOUR_TITLE_TOP;
	rowRect.bottom = rowRect.top + PILOT_LOG_TOUR_TITLE_HEIGHT;
	xfont_Print_Clipped_Text(lineText, rowRect.left, rowRect.top, PILOT_LOG_TOUR_TITLE_FONT,
							 PILOT_LOG_TOUR_TITLE_COLOR);
	xrect_Offset_Rect(&rowRect, 0, PILOT_LOG_TOUR_HEADING_OFFSET_Y);
	strcpy(lineText, g_pilotLogOperationHeading);
	xfont_Print_Clipped_Text(lineText, rowRect.left + PILOT_LOG_TOUR_HEADING_OFFSET_X, rowRect.top,
							 PILOT_LOG_PAGE_FONT, PILOT_LOG_TOUR_HEADING_COLOR);
	for (operationIndex = 0, remainingOperations = operationCount; remainingOperations > 0;
		 ++operationIndex, --remainingOperations) {
		const XwTourOperation* operations;
		int missionChoiceA;
		int missionChoiceB;
		unsigned int operationScore;
		xrect_Offset_Rect(&rowRect, 0, PILOT_LOG_TOUR_SCORE_ROW_HEIGHT);
		operations = g_tourOperationTables[selectedTour];
		missionChoiceA = operations[operationIndex].missionChoiceA;
		missionChoiceB = operations[operationIndex].missionChoiceB;
		operationScore = g_pilotLogRecord.tourMissionScores[selectedTour][missionChoiceA];
		if (missionChoiceB != SHIPEXT_TOUR_NO_ENTRY &&
			operationScore < g_pilotLogRecord.tourMissionScores[selectedTour][missionChoiceB])
			operationScore = g_pilotLogRecord.tourMissionScores[selectedTour][missionChoiceB];
		sprintf(lineText, "Operation \x02%d\x01 with \x02%lu\x01 Points", operationIndex + 1,
				(unsigned long)operationScore);
		xfont_Print_Clipped_Text(lineText, rowRect.left + PILOT_LOG_TOUR_SCORE_OFFSET_X, rowRect.top,
								 PILOT_LOG_PAGE_FONT, PILOT_LOG_TOUR_SCORE_COLOR);
	}
}

// FUNCTION: XW 0x4572B0
void PilotLog_iuser_Navigation(Input* input, int unusedTime) {
	(void)unusedTime;
	if (xinpattr_Get_Input_Selected(input) == 0) {
		return;
	}
	switch (input->id) {
		case PILOT_LOG_NAV_PREVIOUS:
			if (g_pilotLogPage == 0) {
				g_pilotLogPage = g_pilotLogPageCount - 1;
			} else {
				--g_pilotLogPage;
			}
			xinpattr_Refresh_Input(g_pilotLogRootInput);
			break;
		case PILOT_LOG_NAV_NEXT:
			if (g_pilotLogPage == g_pilotLogPageCount - 1) {
				g_pilotLogPage = 0;
			} else {
				++g_pilotLogPage;
			}
			xinpattr_Refresh_Input(g_pilotLogRootInput);
			break;
		case PILOT_LOG_NAV_EXIT: {
			int16_t scene = shellext_Get_Cur_Scene();
			switch (scene) {
				case XW_SCENE_PILOT_LOG_FROM_COMBAT_BRIEFING:
					xerror_Set_Landru_Exit(XW_SCENE_BRIEFING_COMBAT);
					break;
				case XW_SCENE_PILOT_LOG_FROM_TOUR_BRIEFING:
					xerror_Set_Landru_Exit(XW_SCENE_BRIEFING_TOUR);
					break;
				case XW_SCENE_PILOT_LOG_FROM_REGISTER:
					xerror_Set_Landru_Exit(XW_SCENE_REGISTER_RETURN);
					break;
			}
			break;
		}
	}
}

// FUNCTION: XW 0x4573A0
void PilotLog_idraw_Navigation(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	char pageText[PILOT_LOG_PAGE_TEXT_CAPACITY];
	if (refresh != 0) {
		switch (input->id) {
			case PILOT_LOG_NAV_PREVIOUS:
			case PILOT_LOG_NAV_NEXT: {
				PushButton* button = (PushButton*)input;
				xstyle_Style_Paint_Border(frame, button->pressed);
				xstyle_Style_Draw_Centered_Icon(input->id != 0 ? iconRightArrow : iconLeftArrow, frame, clip,
												button->pressed);
				break;
			}
			case PILOT_LOG_NAV_PAGE_LABEL:
				xstyle_Style_Paint_TextField(frame);
				sprintf(pageText, "Page %d/%d", g_pilotLogPage + 1, g_pilotLogPageCount);
				xfont_Print_Centered_Text(pageText, frame, PILOT_LOG_PAGE_FONT, PILOT_LOG_PAGE_COLOR);
				break;
		}
	}
}

// FUNCTION: XW 0x4574C0
int16_t PilotLog_LoadPilotRecord(const char* filename) {
	LandruFile* pilotFile;
	pilotFile = xfile_Open_File(LANDRU_FILE_ROOT_USER, filename, "rb");
	if (pilotFile != NULL) {
		register_ReadPilotRecord(pilotFile, &g_pilotLogRecord);
		xfile_Close_File(pilotFile);
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x457A90
void PilotLog_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_pilotLogMusicState.film = sceneFilm;
		g_pilotLogMusicState.sound = xsound_Find_Gmid(g_pilotLogMusicName);
		if (g_pilotLogMusicState.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource(":X-Wing Data\\RESOURCE\\unmusic.lfd");
			if (musicResource == NULL)
				musicResource = xres_Open_Resource(g_pilotLogMusicFilename);
			g_pilotLogMusicState.sound = xsound_Res_Music(musicResource, g_pilotLogMusicName);
			soundext_Start_Resource_Sound(g_pilotLogMusicState.sound);
			xres_Close_Resource(musicResource);
		}
		xsound_Set_Sound_Keep(g_pilotLogMusicState.sound);
		xsound_Set_Sound_User_Function(g_pilotLogMusicState.sound, Cutscene_IgnoreSoundEvent);
	}
}

// FUNCTION: XW 0x457B30
void PilotLog_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0) {
		Sound* music = xsound_Find_Gmid(g_pilotLogMusicName);
		g_pilotLogMusicState.sound = music;
		if (music != NULL) {
			/* Resource pointers are outside the numeric flight-sound ID range. */
			soundext_SetPriority(0, 0);
			soundext_FadeVolume(g_pilotLogMusicState.sound, 0, PILOT_LOG_MUSIC_FADE_DURATION);
			xsound_Clear_Sound_Keep(g_pilotLogMusicState.sound);
			xsound_Free_Sound(g_pilotLogMusicState.sound);
		}
	}
}
