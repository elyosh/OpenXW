#include "xw_dos94/frontend/pilot_log.h"
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

/* DOS94 0x382378. */
XwShellSceneResult Dos94_PilotLog_Show(struct XwShellContext* shell) {
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
	resourceFile = xres_Open_Resource("log.lfd");
	xcanvas_Get_Drawing_Canvas_Bounds(&controlFrame);
	g_pilotLogFilm =
		xfilm_Res_Callback_Film(resourceFile, "log", &controlFrame, 0, 0, 0, PilotLog_film_Callback);
	xfilm_Set_Film_Def_Palette(g_pilotLogFilm, shell->standardPalette);
	/* Bottom-aligned controls use the VGA viewport, not the 640x480 backing canvas. */
	xrect_Set_Rect(&controlFrame, 0, 0, 320, 200);
	g_pilotLogRootInput = xinput_Alloc_Input(NULL, &controlFrame, 0, 0);
	xrect_Set_Rect(&controlFrame, 14, 6, 306, 180);
	g_pilotLogPageInput = xinput_Alloc_Input(g_pilotLogRootInput, &controlFrame, 0, 0);
	xinpattr_Set_Input_Update_Function(g_pilotLogPageInput, XwInput_UpdateNoOp);
	xinpattr_Set_Input_User_Function(g_pilotLogPageInput, XwInput_UserNoOp);
	xinpattr_Set_Input_Draw_Function(g_pilotLogPageInput, Dos94_PilotLog_idraw_Page);
	g_pilotLogPageInput->mouseUsage = allInput;
	xinpattr_Show_Input(g_pilotLogPageInput);
	xrect_Set_Rect(&controlFrame, 80, 1, 96, 17);
	previousButton = xbtnpush_Alloc_Button(g_pilotLogRootInput, &controlFrame, 0,
										   Dos94_PilotLog_iuser_Navigation, NULL, PILOT_LOG_NAV_PREVIOUS);
	xinpattr_Set_Input_Draw_Function(&previousButton->header, Dos94_PilotLog_idraw_Navigation);
	xinpattr_Set_Input_Allign(&previousButton->header, 0, 2);
	xrect_Set_Rect(&controlFrame, 80, 1, 96, 17);
	nextButton = xbtnpush_Alloc_Button(g_pilotLogRootInput, &controlFrame, 0, Dos94_PilotLog_iuser_Navigation,
									   NULL, PILOT_LOG_NAV_NEXT);
	xinpattr_Set_Input_Draw_Function(&nextButton->header, Dos94_PilotLog_idraw_Navigation);
	xinpattr_Set_Input_Allign(&nextButton->header, 2, 2);
	xrect_Set_Rect(&controlFrame, 15, 1, 74, 17);
	exitButton =
		xbtnpush_Alloc_Small_Button(g_pilotLogRootInput, &controlFrame, 0, Dos94_PilotLog_iuser_Navigation,
									"Exit Pilot Log", PILOT_LOG_NAV_EXIT);
	xinpattr_Set_Input_Allign(&exitButton->header, 2, 2);
	xrect_Set_Rect(&controlFrame, 98, 2, 222, 16);
	pageIndicator = xinput_Alloc_Input(g_pilotLogRootInput, &controlFrame, 0, 0);
	xinpattr_Set_Input_Draw_Function(pageIndicator, Dos94_PilotLog_idraw_Navigation);
	pageIndicator->id = PILOT_LOG_NAV_PAGE_LABEL;
	xinpattr_Set_Input_Allign(pageIndicator, 0, 2);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xview_Set_View_Update_Function(PilotLog_end_View);
	xio_Set_Key_Buttons();
	PilotLog_OpenMusic(resourceFile, g_pilotLogFilm);
	soundext_RecheckSfxPreference();
	XwPilotLog_RunView(resourceFile);
}

/* DOS94 0x3827da. */
void Dos94_PilotLog_idraw_Page(Input* unusedInput, Rect* frame, Rect* unusedClip, int16_t refresh) {
	Rect headerRect;
	char pointsSuffix[PILOT_LOG_POINTS_TEXT_CAPACITY];
	char headerText[PILOT_LOG_HEADER_TEXT_CAPACITY];
	(void)unusedInput;
	(void)unusedClip;
	if (refresh != 0) {
		xpaint_Paint_Clipped_Rect(frame, PILOT_LOG_BACKGROUND_COLOR);
		xfont_Enable_FontID_Shadow(0);
		xfont_Set_FontID_Bold_Color(0, PILOT_LOG_PAGE_COLOR);
		xfont_Enable_FontID_Shadow(1);
		xfont_Set_FontID_Bold_Color(1, PILOT_LOG_PAGE_COLOR);
		xrect_Copy_Rect(&headerRect, frame);
		headerRect.bottom = headerRect.top + 22;
		xpaint_Paint_Clipped_Rect(&headerRect, PILOT_LOG_HEADER_BACKGROUND_COLOR);
		xpaint_Horiz_Clipped_Line(headerRect.left, headerRect.bottom, headerRect.right - headerRect.left,
								  PILOT_LOG_HEADER_LINE_COLOR);
		++headerRect.top;
		headerRect.bottom = headerRect.top + 10;
		strcpy(headerText, g_pilotLogRankText[g_RegisterShellPilot.rank]);
		strcat(headerText, g_RegisterShellPilot.name);
		xfont_Print_Centered_Text(headerText, &headerRect, 0, PILOT_LOG_HEADER_TEXT_COLOR);
		strcpy(headerText, g_pilotLogPilotStatusText[g_RegisterShellPilot.lost_status]);
		xrect_Offset_Rect(&headerRect, 0, 10);
		sprintf(pointsSuffix, g_pilotLogPointsFormat, (unsigned long)g_pilotLogRecord.score);
		strcat(headerText, pointsSuffix);
		xfont_Print_Centered_Text(headerText, &headerRect, 0, PILOT_LOG_HEADER_TEXT_COLOR);
		switch (g_pilotLogPage) {
			case PILOT_LOG_TRAINING_PAGE:
				Dos94_PilotLog_DrawTrainingPage(frame);
				break;
			case PILOT_LOG_HISTORIC_PAGE:
				Dos94_PilotLog_DrawHistoricPage(frame);
				break;
			case PILOT_LOG_BONUS_PAGE:
				Dos94_PilotLog_DrawBonusPage(frame);
				break;
			case PILOT_LOG_COMBAT_PAGE:
				Dos94_PilotLog_DrawCombatStatistics(frame);
				break;
			default:
				Dos94_PilotLog_DrawTourPage(frame);
				break;
		}
		xfont_Disable_FontID_Shadow(0);
		xfont_Disable_FontID_Shadow(1);
	}
}

/* DOS94 0x3829be. */
void Dos94_PilotLog_DrawTrainingPage(const Rect* frame) {
	Rect rowRect;
	char text[PILOT_LOG_TRAINING_TEXT_CAPACITY];
	char valueText[PILOT_LOG_TRAINING_TEXT_CAPACITY];
	int16_t shipIndex;
	xrect_Copy_Rect(&rowRect, (Rect*)frame);
	xrect_Inset_Rect(&rowRect, PILOT_LOG_TRAINING_INSET, 0);
	rowRect.top += 25;
	rowRect.bottom = rowRect.top + 10;
	strcpy(text, g_pilotLogTrainingLevelsHeading);
	xfont_Print_Clipped_Text(text, rowRect.left, rowRect.top, 0, PILOT_LOG_TRAINING_TITLE_COLOR);
	xrect_Offset_Rect(&rowRect, 0, 2);
	for (shipIndex = 0; shipIndex < PILOT_LOG_TRAINING_SHIP_COUNT; ++shipIndex) {
		if (shipext_IsShipAvailable(shipIndex) != 0) {
			xrect_Offset_Rect(&rowRect, 0, 8);
			strcpy(text, g_pilotLogShipNames[shipIndex]);
			if (g_pilotLogRecord.trainingLevelProgress[shipIndex] != 0)
				sprintf(valueText, g_pilotLogTrainingLevelFormat,
						g_pilotLogRecord.trainingLevelProgress[shipIndex]);
			else
				strcpy(valueText, g_pilotLogNoTrainingLevelsText);
			strcat(text, valueText);
			xfont_Print_Clipped_Text(text, rowRect.left + PILOT_LOG_TRAINING_ROW_LEFT, rowRect.top, 1,
									 PILOT_LOG_TRAINING_ROW_COLOR);
		}
	}
	xrect_Offset_Rect(&rowRect, 0, 12);
	strcpy(text, g_pilotLogTrainingScoresHeading);
	xfont_Print_Clipped_Text(text, rowRect.left, rowRect.top, 0, PILOT_LOG_TRAINING_TITLE_COLOR);
	xrect_Offset_Rect(&rowRect, 0, 2);
	for (shipIndex = 0; shipIndex < PILOT_LOG_TRAINING_SHIP_COUNT; ++shipIndex) {
		if (shipext_IsShipAvailable(shipIndex) != 0) {
			xrect_Offset_Rect(&rowRect, 0, 8);
			strcpy(text, g_pilotLogShipNames[shipIndex]);
			strcat(text, g_pilotLogBestScorePrefix);
			sprintf(valueText, "%lu", (unsigned long)g_pilotLogRecord.trainingBestScores[shipIndex]);
			strcat(text, valueText);
			xfont_Print_Clipped_Text(text, rowRect.left + PILOT_LOG_TRAINING_ROW_LEFT, rowRect.top, 1,
									 PILOT_LOG_TRAINING_ROW_COLOR);
		}
	}
}

/* DOS94 0x382c10. */
void Dos94_PilotLog_DrawHistoricPage(const Rect* frame) {
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
	rowRect.top += 25;
	rowRect.bottom = rowRect.top + 10;
	strcpy(text, g_pilotLogHistoricScoresHeading);
	xfont_Print_Clipped_Text(text, rowRect.left, rowRect.top, 0, PILOT_LOG_TRAINING_TITLE_COLOR);
	xrect_Offset_Rect(&rowRect, 0, 12);
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
		xfont_Print_Clipped_Text(text, rowRect.left + PILOT_LOG_BONUS_SUMMARY_LEFT, rowRect.top, 1,
								 headingColor);
		for (missionIndex = 0; missionIndex < PILOT_LOG_HISTORIC_MISSION_COUNT; ++missionIndex) {
			xrect_Offset_Rect(&rowRect, 0, 8);
			sprintf(text, "Mission \002%d\001 with \002%lu\001 Points", missionIndex + 1,
					(unsigned long)g_pilotLogRecord.historicBestScores[shipIndex][missionIndex]);
			if (missionIndex >= PILOT_LOG_HISTORIC_COLUMN_ROWS) {
				if (missionIndex == PILOT_LOG_HISTORIC_COLUMN_ROWS) {
					xrect_Offset_Rect(&rowRect, 0, -PILOT_LOG_HISTORIC_COLUMN_ROWS * 8);
				}
				xfont_Print_Clipped_Text(text, rowRect.left + 160, rowRect.top, 1, scoreColor);
			} else {
				xfont_Print_Clipped_Text(text, rowRect.left + PILOT_LOG_TRAINING_ROW_LEFT, rowRect.top, 1,
										 scoreColor);
			}
		}
		xrect_Offset_Rect(&rowRect, 0, 10);
	}
}

/* DOS94 0x382efc. */
void Dos94_PilotLog_DrawBonusPage(const Rect* frame) {
	Rect rowRect;
	char text[PILOT_LOG_TRAINING_TEXT_CAPACITY];
	char rangeText[PILOT_LOG_TRAINING_TEXT_CAPACITY];
	int16_t rangeStart;
	int16_t rangeEnd;
	int16_t printedRanges;
	int16_t missionIndex;
	xrect_Copy_Rect(&rowRect, (Rect*)frame);
	xrect_Inset_Rect(&rowRect, PILOT_LOG_TRAINING_INSET, 0);
	rowRect.top += 25;
	rowRect.bottom = rowRect.top + 10;
	strcpy(text, g_pilotLogHistoricScoresHeading);
	xfont_Print_Clipped_Text(text, rowRect.left, rowRect.top, 0, PILOT_LOG_TRAINING_TITLE_COLOR);
	xrect_Offset_Rect(&rowRect, 0, 12);
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
	xfont_Print_Clipped_Text(text, rowRect.left + PILOT_LOG_BONUS_SUMMARY_LEFT, rowRect.top, 1,
							 PILOT_LOG_TRAINING_ROW_COLOR);
	for (missionIndex = 0; missionIndex < PILOT_LOG_BONUS_MISSION_COUNT; ++missionIndex) {
		xrect_Offset_Rect(&rowRect, 0, 8);
		sprintf(text, "Mission \002%d\001 with \002%lu\001 Points", missionIndex + 1,
				(unsigned long)g_pilotLogRecord.bonusHistoricBestScores[missionIndex]);
		if (missionIndex == 3)
			xrect_Offset_Rect(&rowRect, 0, -24);
		xfont_Print_Clipped_Text(text, rowRect.left + (missionIndex < 3 ? 20 : 160), rowRect.top, 1,
								 PILOT_LOG_BONUS_SCORE_COLOR);
	}
}

/* DOS94 0x383196. */
void Dos94_PilotLog_DrawCombatStatistics(const Rect* frame) {
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
	sectionRect.top += 25;
	sectionRect.bottom = sectionRect.top + 10;
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
	xfont_Print_Clipped_Text(text, sectionRect.left, sectionRect.top, 0, PILOT_LOG_TRAINING_TITLE_COLOR);
	columnX = sectionRect.left + PILOT_LOG_TRAINING_ROW_LEFT;
	rowY = sectionRect.bottom + 2;
	if (nonzeroCategories > 0) {
		rowsPerColumn = (nonzeroCategories + 1) >> 1;
		for (rowIndex = 0, rowsRemaining = nonzeroCategories; rowsRemaining != 0;
			 ++rowIndex, --rowsRemaining) {
			int16_t displayCategory = categories.order[rowIndex];
			if (rowIndex == rowsPerColumn) {
				columnX += 140;
				rowY = sectionRect.bottom + 2;
			}
			if (categories.kills[displayCategory] != 0 || categories.captures[displayCategory] != 0) {
				int16_t labelWidth;
				strcpy(text, g_pilotLogCraftNames[displayCategory]);
				strcat(text, ":  ");
				xfont_Print_Clipped_Text(text, columnX, rowY, 1, PILOT_LOG_TRAINING_ROW_COLOR);
				labelWidth = 64;
				sprintf(text, g_pilotLogKillCaptureFormat, categories.kills[displayCategory],
						categories.captures[displayCategory]);
				xfont_Print_Clipped_Text(text, columnX + labelWidth, rowY, 1, PILOT_LOG_BONUS_SCORE_COLOR);
				rowY += 7;
			}
		}
	}
	xrect_Offset_Rect(&sectionRect, 0, 7 * ((nonzeroCategories + 1) / 2) + 16);
	strcpy(text, g_pilotLogSurfaceVictoriesHeading);
	sprintf(valueText, "%d", g_pilotLogRecord.surfaceVictories);
	strcat(text, valueText);
	xfont_Print_Clipped_Text(text, sectionRect.left, sectionRect.top, 0, PILOT_LOG_TRAINING_TITLE_COLOR);
	xrect_Offset_Rect(&sectionRect, 0, 12);
	strcpy(text, g_pilotLogLasersFiredHeading);
	sprintf(valueText, "%ld", (long)(int32_t)g_pilotLogRecord.laser_shots_fired);
	strcat(text, valueText);
	xfont_Print_Clipped_Text(text, sectionRect.left, sectionRect.top, 0, PILOT_LOG_TRAINING_TITLE_COLOR);
	if (g_pilotLogRecord.laser_shots_fired != 0)
		laserCraftPercent = PILOT_LOG_PERCENT_SCALE * g_pilotLogRecord.laser_spacecraft_hits /
							g_pilotLogRecord.laser_shots_fired;
	else
		laserCraftPercent = 0;
	xrect_Offset_Rect(&sectionRect, 0, 10);
	strcpy(text, g_pilotLogCraftHitsHeading);
	sprintf(valueText, "%ld (%ld%%)", (long)(int32_t)g_pilotLogRecord.laser_spacecraft_hits,
			(long)(int32_t)laserCraftPercent);
	strcat(text, valueText);
	xfont_Print_Clipped_Text(text, sectionRect.left + PILOT_LOG_TRAINING_ROW_LEFT, sectionRect.top, 1,
							 PILOT_LOG_TRAINING_ROW_COLOR);
	if (g_pilotLogRecord.laser_shots_fired != 0)
		laserGroundPercent = PILOT_LOG_PERCENT_SCALE * g_pilotLogRecord.laser_surface_hits /
							 g_pilotLogRecord.laser_shots_fired;
	else
		laserGroundPercent = 0;
	strcpy(text, g_pilotLogGroundHitsHeading);
	sprintf(valueText, "%ld (%ld%%)", (long)(int32_t)g_pilotLogRecord.laser_surface_hits,
			(long)(int32_t)laserGroundPercent);
	strcat(text, valueText);
	xfont_Print_Clipped_Text(text, 180, sectionRect.top, 1, PILOT_LOG_TRAINING_ROW_COLOR);
	xrect_Offset_Rect(&sectionRect, 0, 10);
	strcpy(text, g_pilotLogWarheadsFiredHeading);
	sprintf(valueText, "%u", (unsigned int)g_pilotLogRecord.warheads_fired);
	strcat(text, valueText);
	xfont_Print_Clipped_Text(text, sectionRect.left, sectionRect.top, 0, PILOT_LOG_TRAINING_TITLE_COLOR);
	if (g_pilotLogRecord.warheads_fired != 0)
		warheadCraftPercent = PILOT_LOG_PERCENT_SCALE * g_pilotLogRecord.warhead_spacecraft_hits /
							  g_pilotLogRecord.warheads_fired;
	else
		warheadCraftPercent = 0;
	xrect_Offset_Rect(&sectionRect, 0, 10);
	strcpy(text, g_pilotLogCraftHitsHeading);
	sprintf(valueText, "%u (%ld%%)", (unsigned int)g_pilotLogRecord.warhead_spacecraft_hits,
			(long)warheadCraftPercent);
	strcat(text, valueText);
	xfont_Print_Clipped_Text(text, sectionRect.left + PILOT_LOG_TRAINING_ROW_LEFT, sectionRect.top, 1,
							 PILOT_LOG_TRAINING_ROW_COLOR);
	if (g_pilotLogRecord.warheads_fired != 0)
		warheadGroundPercent =
			PILOT_LOG_PERCENT_SCALE * g_pilotLogRecord.warhead_surface_hits / g_pilotLogRecord.warheads_fired;
	else
		warheadGroundPercent = 0;
	strcpy(text, g_pilotLogGroundHitsHeading);
	sprintf(valueText, "%u (%ld%%)", (unsigned int)g_pilotLogRecord.warhead_surface_hits,
			(long)warheadGroundPercent);
	strcat(text, valueText);
	xfont_Print_Clipped_Text(text, 180, sectionRect.top, 1, PILOT_LOG_TRAINING_ROW_COLOR);
	xrect_Offset_Rect(&sectionRect, 0, 10);
	strcpy(text, g_pilotLogCraftLostHeading);
	sprintf(valueText, "%u", (unsigned int)g_pilotLogRecord.ejections);
	strcat(text, valueText);
	xfont_Print_Clipped_Text(text, sectionRect.left, sectionRect.top, 0, PILOT_LOG_TRAINING_TITLE_COLOR);
}

/* DOS94 0x3838d4. */
void Dos94_PilotLog_DrawTourPage(const Rect* frame) {
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
	rowRect.top += 25;
	rowRect.bottom = rowRect.top + 10;
	xfont_Print_Clipped_Text(lineText, rowRect.left, rowRect.top, 0, PILOT_LOG_TOUR_TITLE_COLOR);
	xrect_Offset_Rect(&rowRect, 0, 12);
	strcpy(lineText, g_pilotLogOperationHeading);
	xfont_Print_Clipped_Text(lineText, rowRect.left + PILOT_LOG_TOUR_HEADING_OFFSET_X, rowRect.top, 1,
							 PILOT_LOG_TOUR_HEADING_COLOR);
	for (operationIndex = 0, remainingOperations = operationCount; remainingOperations > 0;
		 ++operationIndex, --remainingOperations) {
		const XwTourOperation* operations;
		int missionChoiceA;
		int missionChoiceB;
		unsigned int operationScore;
		xrect_Offset_Rect(&rowRect, 0, 8);
		operations = g_tourOperationTables[selectedTour];
		missionChoiceA = operations[operationIndex].missionChoiceA;
		missionChoiceB = operations[operationIndex].missionChoiceB;
		operationScore = g_pilotLogRecord.tourMissionScores[selectedTour][missionChoiceA];
		if (missionChoiceB != SHIPEXT_TOUR_NO_ENTRY &&
			operationScore < g_pilotLogRecord.tourMissionScores[selectedTour][missionChoiceB])
			operationScore = g_pilotLogRecord.tourMissionScores[selectedTour][missionChoiceB];
		sprintf(lineText, "Operation \x02%d\x01 with \x02%lu\x01 Points", operationIndex + 1,
				(unsigned long)operationScore);
		if (operationIndex == (operationCount + 1) / 2)
			xrect_Offset_Rect(&rowRect, 140, -8 * ((operationCount + 1) / 2));
		xfont_Print_Clipped_Text(lineText, rowRect.left + PILOT_LOG_TOUR_SCORE_OFFSET_X, rowRect.top, 1,
								 PILOT_LOG_TOUR_SCORE_COLOR);
	}
}

/* DOS94 0x383b18. */
void Dos94_PilotLog_iuser_Navigation(Input* input, int unusedTime) {
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
					xerror_Set_Landru_Exit(XW_SCENE_REQUEST_COMBAT_LAUNCH);
					break;
				case XW_SCENE_PILOT_LOG_FROM_TOUR_BRIEFING:
					xerror_Set_Landru_Exit(XW_SCENE_REQUEST_TOUR_LAUNCH);
					break;
				case XW_SCENE_PILOT_LOG_FROM_REGISTER:
					xerror_Set_Landru_Exit(XW_SCENE_REGISTER_RETURN);
					break;
			}
			break;
		}
	}
}

/* DOS94 0x383bc0. */
void Dos94_PilotLog_idraw_Navigation(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
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
				xpaint_Paint_Clipped_Bevel(frame, 38, 26, 0, 1);
				sprintf(pageText, "Page %d/%d", g_pilotLogPage + 1, g_pilotLogPageCount);
				xfont_Print_Centered_Text(pageText, frame, 0, PILOT_LOG_PAGE_COLOR);
				break;
		}
	}
}

/* DOS94 0x383da8 retains the sound after requesting its fade. */
void Dos94_PilotLog_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled()) {
		soundext_SetPriority((intptr_t)g_pilotLogMusicState.sound, 0);
		soundext_FadeVolume(g_pilotLogMusicState.sound, 0, 300);
	}
}
