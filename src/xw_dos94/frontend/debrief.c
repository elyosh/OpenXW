#include "xw_dos94/frontend/debrief.h"
#include "xw/frontend/debrief.h"

#include "xw/audio/soundext.h"
#include "xw/util/landru_display.h"
#include "xw_runtime/integration/debrief_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/debrief_task.h"
#endif
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/player/user.h"
#include "xw/frontend/combat.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#include <ctype.h>
#include <landru/actanim.h>
#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/file.h>
#include <landru/font.h>
#include <landru/fourcc.h>
#include <landru/inpattr.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/paint.h>
#include <landru/res.h>
#include <landru/view.h>
#include <landru/viewadd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <landru/style.h>
static int16_t film_callback(Film* film, FilmObject* object);
static void draw_page_button(Input* input, Rect* frame, Rect* clip, int16_t refresh);
static int16_t draw_background(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh);
static int16_t draw_statistics(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh);
XwShellSceneResult Dos94_debrief_Debrief(struct XwShellContext* shell);
static void Dos94_debrief_iuser_Debrief(Input* input, int context);
static void Dos94_debrief_idraw_HoverLabel(Input* input, Rect* frame, Rect* unusedClip, int16_t refresh);
static void Dos94_debrief_idraw_PageNumber(Input* unusedInput, Rect* frame, Rect* unusedClip,
										   int16_t refresh);
static void Dos94_debrief_user_Door(Actor* actor, int time);
static void Dos94_debrief_draw_Background(Actor* unusedActor, Rect* frame, Rect* unusedClip, int16_t x,
										  int16_t y, int16_t refresh);
static void Dos94_debrief_draw_Statistics(Actor* unusedActor, Rect* frame, Rect* unusedClip, int16_t unusedX,
										  int16_t unusedY, int16_t refresh);
static void Dos94_debrief_DrawProvingGroundSummary(Rect* headerRect);
static int Dos94_debrief_BuildSectionPages(void);
static void Dos94_debrief_DrawMissionSummary(Rect* headerRect);
static void Dos94_debrief_DrawMissionHeader(Rect* headerRect);
static int16_t Dos94_debrief_SectionMissionResult(Rect* sectionRect, int16_t draw);
static int16_t Dos94_debrief_SectionPromotion(Rect* sectionRect, int16_t draw);
static int16_t Dos94_debrief_SectionBattlePatch(Rect* sectionRect, int16_t draw);
static int16_t Dos94_debrief_SectionUncompletedGoals(Rect* sectionRect, int16_t draw);
static int16_t Dos94_debrief_SectionCompletedGoals(Rect* sectionRect, int16_t draw);
static int16_t Dos94_debrief_SectionObjectGoals(Rect* sectionRect, int16_t draw);
static int16_t Dos94_debrief_SectionSpacecraftKills(Rect* sectionRect, int16_t draw);
static int16_t Dos94_debrief_SectionSpacecraftLosses(Rect* sectionRect, int16_t draw);
static int16_t Dos94_debrief_SectionSpaceObjectKills(Rect* sectionRect, int16_t draw);
static int16_t Dos94_debrief_SectionDeathStarBuildingKills(Rect* sectionRect, int16_t draw);
static int Dos94_debrief_SectionLaserAccuracy(Rect* sectionRect, int16_t draw);
static int Dos94_debrief_SectionIonAccuracy(Rect* sectionRect, int16_t draw);
static int Dos94_debrief_SectionWarheadAccuracy(Rect* sectionRect, int16_t draw);
static int Dos94_debrief_SectionWeaponAccuracy(Rect* sectionRect, int16_t weaponKind, uint16_t shotsFired,
											   uint16_t spacecraftHits, uint16_t surfaceHits, int16_t draw);
void Dos94_debrief_CloseMusic(void);

/* DOS94 0x280000. */
XwShellSceneResult Dos94_debrief_Debrief(struct XwShellContext* shell) {
	ResFile* resource;
	Input* input;
	Input* pageNumberInput;
	PushButton* previousButton;
	PushButton* nextButton;
	Rect rect;
	debrief_LoadMissionFlightGroups();
	debrief_ReloadPilotRecord();
	g_debriefPageIndex = 0;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_DEBRIEF_PROVING_GROUNDS:
			g_debriefPageCount = 1;
			break;
		case XW_SCENE_DEBRIEF_COMBAT:
			g_debriefPageCount = Dos94_debrief_BuildSectionPages();
			break;
		case XW_SCENE_DEBRIEF_TOUR:
			g_debriefPageCount = Dos94_debrief_BuildSectionPages();
			break;
	}
	g_debriefKeyboardFocusIndex = DEBRIEF_INITIAL_FOCUS;
	xio_Set_Mouse_Position(DEBRIEF_INITIAL_MOUSE_X, DEBRIEF_INITIAL_MOUSE_Y);
	debrief_Update_Debrief_Scores();
	resource = xres_Open_Resource("debrief.lfd");
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	g_debriefBackgroundHandle = xmemhdl_Alloc_Clear_Handle(320 * 200, LANDRU_MEMORY_RESOURCE);
	xrect_Set_Rect(&rect, 0, 0, 320, 200);
	const char* film_name = shellext_Get_Cur_Scene() == 115   ? "detrain"
							: shellext_Get_Cur_Scene() == 116 ? "decombat"
															  : "detour";
	g_debriefFilm = xfilm_Res_Callback_Film(resource, film_name, &rect, 0, 0, 0, film_callback);
	xfilm_Set_Film_Def_Palette(g_debriefFilm, shell->standardPalette);
	g_debriefBaseActor = xactor_Find_Actor(FOURCC_DELT, "dbrfcntl");
	xactor_Non_Refreshable_Actor(g_debriefBaseActor);
	g_debriefInteriorActor = xactor_Find_Actor(FOURCC_DELT, "db-scrn");
	xactor_Non_Refreshable_Actor(g_debriefInteriorActor);
	g_debriefDoorActors[DEBRIEF_DOOR_LEFT] = xactor_Find_Actor(FOURCC_ANIM, "debrfdor");
	xactor_Set_Actor_User_Function(g_debriefDoorActors[DEBRIEF_DOOR_LEFT], Dos94_debrief_user_Door);
	xactor_Non_Refreshable_Actor(g_debriefDoorActors[DEBRIEF_DOOR_LEFT]);
	g_debriefDoorActors[DEBRIEF_DOOR_LEFT]->id = DEBRIEF_DOOR_LEFT;
	g_debriefDoorActors[DEBRIEF_DOOR_RIGHT] = xactor_Find_Actor(FOURCC_ANIM, "dbrfdor2");
	xactor_Set_Actor_User_Function(g_debriefDoorActors[DEBRIEF_DOOR_RIGHT], Dos94_debrief_user_Door);
	xactor_Non_Refreshable_Actor(g_debriefDoorActors[DEBRIEF_DOOR_RIGHT]);
	g_debriefDoorActors[DEBRIEF_DOOR_RIGHT]->id = DEBRIEF_DOOR_RIGHT;
	xrect_Set_Rect(&rect, 0, 0, 320, 200);
	g_debriefBackgroundActor = xactcust_Alloc_Custom_Actor(0, &rect, 0, 0, DEBRIEF_BACKGROUND_Z);
	xactor_Set_Actor_User_Function(g_debriefBackgroundActor, XwDebrief_IgnoreActorEvent);
	xactor_Set_Actor_Draw_Function(g_debriefBackgroundActor, draw_background);
	xactor_Non_Refreshable_Actor(g_debriefBackgroundActor);
	xrect_Set_Rect(&rect, 19, 3, 231, 141);
	g_debriefStatisticsActor = xactcust_Alloc_Custom_Actor(0, &rect, 0, 0, DEBRIEF_STATISTICS_Z);
	xactor_Set_Actor_User_Function(g_debriefStatisticsActor, XwDebrief_IgnoreActorEvent);
	xactor_Set_Actor_Draw_Function(g_debriefStatisticsActor, draw_statistics);
	xactor_Non_Refreshable_Actor(g_debriefStatisticsActor);
	xrect_Set_Rect(&rect, 0, 0, 320, 200);
	g_debriefRootInput = xinput_Alloc_Input(NULL, &rect, 0, 0);
	xrect_Set_Rect(&rect, 236, 38, 278, 136);
	input = xinput_Alloc_Input(g_debriefRootInput, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(input, debrief_iupdate_Debrief);
	xinpattr_Set_Input_User_Function(input, Dos94_debrief_iuser_Debrief);
	input->mouseUsage = allInput;
	input->id = DEBRIEF_DOOR_LEFT;
	xrect_Set_Rect(&rect, 300, 38, 320, 136);
	input = xinput_Alloc_Input(g_debriefRootInput, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(input, debrief_iupdate_Debrief);
	xinpattr_Set_Input_User_Function(input, Dos94_debrief_iuser_Debrief);
	input->mouseUsage = allInput;
	input->id = DEBRIEF_DOOR_RIGHT;
	xrect_Set_Rect(&rect, 42, 166, 162, 184);
	g_debriefHoverLabelInput = xinput_Alloc_Input(g_debriefRootInput, &rect, 0, 0);
	xinpattr_Set_Input_Draw_Function(g_debriefHoverLabelInput, Dos94_debrief_idraw_HoverLabel);
	xinpattr_Hide_Input(g_debriefHoverLabelInput);
	xrect_Set_Rect(&rect, 42, 166, 162, 184);
	g_debriefPageControlsInput = xinput_Alloc_Input(g_debriefRootInput, &rect, 0, 0);
	xrect_Set_Rect(&rect, 0, 0, 16, 16);
	previousButton =
		xbtnpush_Alloc_Button(g_debriefPageControlsInput, &rect, 0, debrief_iuser_PageButton, NULL, 0);
	xinpattr_Set_Input_Draw_Function(&previousButton->header, draw_page_button);
	xinpattr_Set_Input_Allign(&previousButton->header, 0, 1);
	nextButton =
		xbtnpush_Alloc_Button(g_debriefPageControlsInput, &rect, 0, debrief_iuser_PageButton, NULL, 1);
	xinpattr_Set_Input_Draw_Function(&nextButton->header, draw_page_button);
	xinpattr_Set_Input_Allign(&nextButton->header, 2, 1);
	xrect_Set_Rect(&rect, 0, 0, 86, 14);
	pageNumberInput = xinput_Alloc_Input(g_debriefPageControlsInput, &rect, 0, 0);
	xinpattr_Set_Input_Draw_Function(pageNumberInput, Dos94_debrief_idraw_PageNumber);
	xinpattr_Set_Input_Allign(pageNumberInput, 1, 1);
	pageNumberInput->id = 1;
	xio_Set_Key_Buttons();
	xview_Set_View_Update_Function(debrief_end_View);
	debrief_OpenMusic(resource, g_debriefFilm);
	debrief_LoadSoundEffects(resource, g_debriefFilm);
	XwDebrief_RunView(resource);
}

/* DOS94 0x280a2a. */
static void Dos94_debrief_iuser_Debrief(Input* input, int context) {
	(void)context;
	switch (input->var1) {
		case DEBRIEF_INPUT_IDLE:
			if (xinpattr_Is_Input_Visible(g_debriefHoverLabelInput) != 0 &&
				input->id == g_debriefHoverLabelInput->var1) {
				xinpattr_Show_Input(g_debriefPageControlsInput);
				xinpattr_Hide_Input(g_debriefHoverLabelInput);
				xactor_Refresh_Actor(g_debriefBaseActor);
				xinpattr_Refresh_Input(g_debriefPageControlsInput);
			}
			break;
		case DEBRIEF_INPUT_EXIT:
			xerror_Set_Landru_Exit(input->var2);
			break;
		case DEBRIEF_INPUT_HOVER:
			if (xinpattr_Is_Input_Visible(g_debriefPageControlsInput) != 0) {
				xinpattr_Hide_Input(g_debriefPageControlsInput);
				xinpattr_Show_Input(g_debriefHoverLabelInput);
				xactor_Refresh_Actor(g_debriefBaseActor);
				xinpattr_Refresh_Input(g_debriefHoverLabelInput);
				g_debriefHoverLabelInput->var1 = input->id;
			}
			input->var1 = DEBRIEF_INPUT_IDLE;
			break;
	}
}

/* DOS94 0x280b88. */
static void Dos94_debrief_idraw_HoverLabel(Input* input, Rect* frame, Rect* unusedClip, int16_t refresh) {
	(void)unusedClip;
	if (refresh) {
		const char* text = input->var1 == 0 ? "Exit Debriefing" : "Reenter Mission";
		/* The DOS shadow call offsets its stack arguments instead of the rectangle. */
		Rect shadow = *frame;
		xrect_Offset_Rect(&shadow, 1, 1);
		xfont_Print_Centered_Text(text, &shadow, 0, 16);
		xfont_Print_Centered_Text(text, frame, 0, 15);
	}
}

/* DOS94 0x280cbe. */
static void Dos94_debrief_idraw_PageNumber(Input* unusedInput, Rect* frame, Rect* unusedClip,
										   int16_t refresh) {
	char pageText[DEBRIEF_PAGE_TEXT_CAPACITY];
	(void)unusedInput;
	(void)unusedClip;
	if (refresh != 0) {
		xstyle_Style_Paint_TextField(frame);
		sprintf(pageText, "Page %d of %d", g_debriefPageIndex + 1, g_debriefPageCount);
		xfont_Print_Centered_Text(pageText, frame, DEBRIEF_LABEL_FONT, DEBRIEF_LABEL_TEXT_COLOR);
	}
}

/* DOS94 0x280d14. */
static void Dos94_debrief_user_Door(Actor* actor, int time) {
	int16_t needsRefresh = 1;
	(void)time;
	if (actor->var1 != 0) {
		if (actor->state == 0) {
			debrief_HandleSoundAction(DEBRIEF_SOUND_DOOR_OPEN);
		}
		if (actor->state != actor->arraySize - 1) {
			xactor_Set_Actor_State(actor, actor->state + 1, 0);
		} else {
			needsRefresh = 0;
		}
		actor->var1 = 0;
	} else {
		if (actor->state == actor->arraySize - 1) {
			debrief_HandleSoundAction(DEBRIEF_SOUND_DOOR_CLOSE);
		}
		if (actor->state != 0) {
			xactor_Set_Actor_State(actor, actor->state - 1, 0);
		} else {
			needsRefresh = 0;
		}
	}
	if (needsRefresh != 0) {
		xview_Refresh_View();
	}
}

/* DOS94 0x280db4. */
static void Dos94_debrief_draw_Background(Actor* unusedActor, Rect* frame, Rect* unusedClip, int16_t x,
										  int16_t y, int16_t refresh) {
	(void)unusedActor;
	(void)unusedClip;
	if (refresh != 0) {
		const uint8_t* backgroundPixels = xmemhdl_Lock_Handle(g_debriefBackgroundHandle);
		stub_Copy_From_Clipped_Buffer(backgroundPixels, frame, x, y, 320, 200);
		xmemhdl_Unlock_Handle(g_debriefBackgroundHandle);
	}
}

/* DOS94 0x280e00. */
static void Dos94_debrief_draw_Statistics(Actor* unusedActor, Rect* frame, Rect* unusedClip, int16_t unusedX,
										  int16_t unusedY, int16_t refresh) {
	(void)unusedActor;
	(void)unusedClip;
	(void)unusedX;
	(void)unusedY;
	if (refresh != 0) {
		Rect headerRect;
		xpaint_Paint_Clipped_Rect(frame, DEBRIEF_STATISTICS_BACKGROUND_COLOR);
		xfont_Enable_FontID_Shadow(0);
		xfont_Set_FontID_Bold_Color(0, DEBRIEF_STATISTICS_BOLD_COLOR);
		xfont_Enable_FontID_Shadow(1);
		xfont_Set_FontID_Bold_Color(1, DEBRIEF_STATISTICS_BOLD_COLOR);
		xrect_Copy_Rect(&headerRect, frame);
		headerRect.bottom = headerRect.top + 22;
		xpaint_Paint_Clipped_Rect(&headerRect, DEBRIEF_STATISTICS_HEADER_COLOR);
		xpaint_Horiz_Clipped_Line(headerRect.left, headerRect.bottom, headerRect.right - headerRect.left,
								  DEBRIEF_STATISTICS_SEPARATOR_COLOR);
		++headerRect.top;
		headerRect.bottom = headerRect.top + 10;
		switch (shellext_Get_Cur_Scene()) {
			case XW_SCENE_DEBRIEF_PROVING_GROUNDS:
				Dos94_debrief_DrawProvingGroundSummary(&headerRect);
				break;
			case XW_SCENE_DEBRIEF_COMBAT:
			case XW_SCENE_DEBRIEF_TOUR:
				Dos94_debrief_DrawMissionSummary(&headerRect);
				break;
			default:
				break;
		}
		xfont_Disable_FontID_Shadow(0);
		xfont_Disable_FontID_Shadow(1);
	}
}

/* DOS94 0x280ef6. */
static void Dos94_debrief_DrawProvingGroundSummary(Rect* headerRect) {
	int16_t firstRun;
	int16_t lastRun;
	uint16_t roundsFired;
	uint16_t targetHits;
	int hitPercent;
	Rect lineRect;
	char numberText[DEBRIEF_HEADER_NUMBER_CAPACITY];
	char text[DEBRIEF_HEADER_TEXT_CAPACITY];

	xrect_Copy_Rect(&lineRect, headerRect);
	text[0] = 0;
	switch (shipext_Get_Train_Ship()) {
		case SHIPEXT_SHIP_AWING:
			strcpy(text, g_debriefAWingText);
			break;
		case SHIPEXT_SHIP_XWING:
			strcpy(text, g_debriefXWingText);
			break;
		case SHIPEXT_SHIP_YWING:
			strcpy(text, g_debriefYWingText);
			break;
		case SHIPEXT_SHIP_BWING:
			strcpy(text, g_debriefBWingText);
			break;
		default:
			break;
	}
	strcat(text, g_debriefStarfighterText);
	xfont_Print_Centered_Text(text, &lineRect, 0, DEBRIEF_HEADER_COLOR);
	strcpy(text, g_debriefProvingGroundText);
	strcat(text, g_debriefRunText);
	firstRun = shipext_Get_Train_Level() + 1;
	lastRun = g_missionRuntimeState.provingGroundsLevel;
	if (firstRun == lastRun)
		sprintf(numberText, " %d)", (int)firstRun);
	else
		sprintf(numberText, "s %d to %d)", (int)firstRun, (int)lastRun);
	strcat(text, numberText);
	xrect_Offset_Rect(&lineRect, 0, 10);
	xfont_Print_Centered_Text(text, &lineRect, 0, DEBRIEF_HEADER_COLOR);
	if (g_missionRuntimeState.flightBadgeAnnouncement != 0) {
		xrect_Offset_Rect(&lineRect, 0, 16);
		sprintf(text, "You got a Flight Badge!");
		xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_ACCURACY_HEADING_LEFT, lineRect.top, 1,
								 DEBRIEF_ACCURACY_HEADING_COLOR);
		xrect_Offset_Rect(&lineRect, 0, 12);
	}
	if (g_missionRuntimeState.newRank != 0) {
		if (g_missionRuntimeState.flightBadgeAnnouncement == 0)
			xrect_Offset_Rect(&lineRect, 0, 16);
		strcpy(text, g_debriefPromotionText);
		strcat(text, g_debriefRankNames[g_missionRuntimeState.newRank]);
		xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_ACCURACY_HEADING_LEFT, lineRect.top, 0,
								 DEBRIEF_ACCURACY_HEADING_COLOR);
		xrect_Offset_Rect(&lineRect, 0, 12);
	}
	strcpy(text, g_debriefScoreText);
	sprintf(numberText, "%ld", (long)g_missionRuntimeState.provingGroundsScore);
	strcat(text, numberText);
	xrect_Offset_Rect(&lineRect, 0, 16);
	xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_ACCURACY_HEADING_LEFT, lineRect.top, 0,
							 DEBRIEF_ACCURACY_HEADING_COLOR);
	strcpy(text, g_debriefGatesPassedText);
	sprintf(numberText, "%u", (unsigned int)g_missionRuntimeState.provingGroundsCheckpointsPassed);
	strcat(text, numberText);
	xrect_Offset_Rect(&lineRect, 0, 10);
	xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_ACCURACY_DETAIL_LEFT, lineRect.top, 1,
							 DEBRIEF_ACCURACY_DETAIL_COLOR);
	strcpy(text, g_debriefGatesMissedText);
	sprintf(numberText, "%u", (unsigned int)g_missionRuntimeState.provingGroundsCheckpointsMissed);
	strcat(text, numberText);
	xrect_Offset_Rect(&lineRect, 0, 8);
	xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_ACCURACY_DETAIL_LEFT, lineRect.top, 1,
							 DEBRIEF_ACCURACY_DETAIL_COLOR);
	strcpy(text, g_debriefGatesRemainingText);
	sprintf(numberText, "%u", (unsigned int)g_missionRuntimeState.provingGroundsCheckpointsRemaining);
	strcat(text, numberText);
	xrect_Offset_Rect(&lineRect, 0, 8);
	xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_ACCURACY_DETAIL_LEFT, lineRect.top, 1,
							 DEBRIEF_ACCURACY_DETAIL_COLOR);
	roundsFired =
		g_playerFlightState.weaponStats.warheadsFired + g_playerFlightState.weaponStats.laserShotsFired;
	strcpy(text, g_debriefRoundsFiredText);
	sprintf(numberText, "%u", (unsigned int)roundsFired);
	strcat(text, numberText);
	xrect_Offset_Rect(&lineRect, 0, 10);
	xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_ACCURACY_HEADING_LEFT, lineRect.top, 0,
							 DEBRIEF_ACCURACY_HEADING_COLOR);
	if (roundsFired != 0)
		hitPercent =
			DEBRIEF_PERCENT_SCALE * g_missionRuntimeState.provingGroundsTargetsDestroyed / (int)roundsFired;
	else
		hitPercent = 0;
	targetHits = g_missionRuntimeState.provingGroundsTargetsDestroyed;
	strcpy(text, g_debriefTargetHitsText);
	sprintf(numberText, "%u (%ld%%)", (unsigned int)targetHits, (long)hitPercent);
	strcat(text, numberText);
	xrect_Offset_Rect(&lineRect, 0, 10);
	xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_ACCURACY_DETAIL_LEFT, lineRect.top, 1,
							 DEBRIEF_ACCURACY_DETAIL_COLOR);
}

/* DOS94 0x2813de. */
static int Dos94_debrief_BuildSectionPages(void) {
	Rect measureRect;
	int pageIndex;
	int16_t pageHeight;
	int sectionIndex;
	xrect_Clear_Rect(&measureRect);
	pageIndex = 0;
	pageHeight = 0;
	for (sectionIndex = 0; sectionIndex < DEBRIEF_SECTION_COUNT; ++sectionIndex) {
		int16_t sectionHeight = 0;
		switch (sectionIndex) {
			case DEBRIEF_SECTION_MISSION_RESULT:
				sectionHeight = Dos94_debrief_SectionMissionResult(&measureRect, 0);
				break;
			case DEBRIEF_SECTION_PROMOTION:
				sectionHeight = Dos94_debrief_SectionPromotion(&measureRect, 0);
				break;
			case DEBRIEF_SECTION_BATTLE_PATCH:
				sectionHeight = Dos94_debrief_SectionBattlePatch(&measureRect, 0);
				break;
			case DEBRIEF_SECTION_UNCOMPLETED_GOALS:
				sectionHeight = Dos94_debrief_SectionUncompletedGoals(&measureRect, 0);
				break;
			case DEBRIEF_SECTION_COMPLETED_GOALS:
				sectionHeight = Dos94_debrief_SectionCompletedGoals(&measureRect, 0);
				break;
			case DEBRIEF_SECTION_OBJECT_GOALS:
				sectionHeight = Dos94_debrief_SectionObjectGoals(&measureRect, 0);
				break;
			case DEBRIEF_SECTION_SPACECRAFT_KILLS:
				sectionHeight = Dos94_debrief_SectionSpacecraftKills(&measureRect, 0);
				break;
			case DEBRIEF_SECTION_SPACECRAFT_LOSSES:
				sectionHeight = Dos94_debrief_SectionSpacecraftLosses(&measureRect, 0);
				break;
			case DEBRIEF_SECTION_SPACE_OBJECT_KILLS:
				sectionHeight = Dos94_debrief_SectionSpaceObjectKills(&measureRect, 0);
				break;
			case DEBRIEF_SECTION_DEATH_STAR_BUILDING_KILLS:
				sectionHeight = Dos94_debrief_SectionDeathStarBuildingKills(&measureRect, 0);
				break;
			case DEBRIEF_SECTION_LASER_ACCURACY:
				sectionHeight = Dos94_debrief_SectionLaserAccuracy(&measureRect, 0);
				break;
			case DEBRIEF_SECTION_ION_ACCURACY:
				sectionHeight = Dos94_debrief_SectionIonAccuracy(&measureRect, 0);
				break;
			case DEBRIEF_SECTION_WARHEAD_ACCURACY:
				sectionHeight = Dos94_debrief_SectionWarheadAccuracy(&measureRect, 0);
				break;
		}
		pageHeight += sectionHeight;
		if (pageHeight > 114) {
			pageHeight = sectionHeight;
			++pageIndex;
		}
		g_debriefSectionPages[sectionIndex] = pageIndex;
	}
	return pageIndex + 1;
}

/* DOS94 0x281534. */
static void Dos94_debrief_DrawMissionSummary(Rect* headerRect) {
	Rect lineRect;
	Dos94_debrief_DrawMissionHeader(headerRect);
	xrect_Copy_Rect(&lineRect, headerRect);
	xrect_Offset_Rect(&lineRect, 0, 24);
	if ((uint16_t)g_debriefSectionPages[DEBRIEF_SECTION_MISSION_RESULT] == g_debriefPageIndex) {
		int16_t height = Dos94_debrief_SectionMissionResult(&lineRect, 1);
		xrect_Offset_Rect(&lineRect, 0, height);
	}
	if ((uint16_t)g_debriefSectionPages[DEBRIEF_SECTION_PROMOTION] == g_debriefPageIndex) {
		int16_t height = Dos94_debrief_SectionPromotion(&lineRect, 1);
		xrect_Offset_Rect(&lineRect, 0, height);
	}
	if ((uint16_t)g_debriefSectionPages[DEBRIEF_SECTION_BATTLE_PATCH] == g_debriefPageIndex) {
		int16_t height = Dos94_debrief_SectionBattlePatch(&lineRect, 1);
		xrect_Offset_Rect(&lineRect, 0, height);
	}
	if ((uint16_t)g_debriefSectionPages[DEBRIEF_SECTION_UNCOMPLETED_GOALS] == g_debriefPageIndex) {
		int16_t height = Dos94_debrief_SectionUncompletedGoals(&lineRect, 1);
		xrect_Offset_Rect(&lineRect, 0, height);
	}
	if ((uint16_t)g_debriefSectionPages[DEBRIEF_SECTION_COMPLETED_GOALS] == g_debriefPageIndex) {
		int16_t height = Dos94_debrief_SectionCompletedGoals(&lineRect, 1);
		xrect_Offset_Rect(&lineRect, 0, height);
	}
	if ((uint16_t)g_debriefSectionPages[DEBRIEF_SECTION_OBJECT_GOALS] == g_debriefPageIndex) {
		int16_t height = Dos94_debrief_SectionObjectGoals(&lineRect, 1);
		xrect_Offset_Rect(&lineRect, 0, height);
	}
	if ((uint16_t)g_debriefSectionPages[DEBRIEF_SECTION_SPACECRAFT_KILLS] == g_debriefPageIndex) {
		int16_t height = Dos94_debrief_SectionSpacecraftKills(&lineRect, 1);
		xrect_Offset_Rect(&lineRect, 0, height);
	}
	if ((uint16_t)g_debriefSectionPages[DEBRIEF_SECTION_SPACECRAFT_LOSSES] == g_debriefPageIndex) {
		int16_t height = Dos94_debrief_SectionSpacecraftLosses(&lineRect, 1);
		xrect_Offset_Rect(&lineRect, 0, height);
	}
	if ((uint16_t)g_debriefSectionPages[DEBRIEF_SECTION_SPACE_OBJECT_KILLS] == g_debriefPageIndex) {
		int16_t height = Dos94_debrief_SectionSpaceObjectKills(&lineRect, 1);
		xrect_Offset_Rect(&lineRect, 0, height);
	}
	if ((uint16_t)g_debriefSectionPages[DEBRIEF_SECTION_DEATH_STAR_BUILDING_KILLS] == g_debriefPageIndex) {
		int16_t height = Dos94_debrief_SectionDeathStarBuildingKills(&lineRect, 1);
		xrect_Offset_Rect(&lineRect, 0, height);
	}
	if ((uint16_t)g_debriefSectionPages[DEBRIEF_SECTION_LASER_ACCURACY] == g_debriefPageIndex) {
		int16_t height = Dos94_debrief_SectionLaserAccuracy(&lineRect, 1);
		xrect_Offset_Rect(&lineRect, 0, height);
	}
	if ((uint16_t)g_debriefSectionPages[DEBRIEF_SECTION_ION_ACCURACY] == g_debriefPageIndex) {
		int16_t height = Dos94_debrief_SectionIonAccuracy(&lineRect, 1);
		xrect_Offset_Rect(&lineRect, 0, height);
	}
	if ((uint16_t)g_debriefSectionPages[DEBRIEF_SECTION_WARHEAD_ACCURACY] == g_debriefPageIndex) {
		int16_t height = Dos94_debrief_SectionWarheadAccuracy(&lineRect, 1);
		xrect_Offset_Rect(&lineRect, 0, height);
	}
}

/* DOS94 0x2817e8. */
static void Dos94_debrief_DrawMissionHeader(Rect* headerRect) {
	Rect lineRect;
	char numberText[DEBRIEF_HEADER_NUMBER_CAPACITY];
	char text[DEBRIEF_HEADER_TEXT_CAPACITY];
	xrect_Copy_Rect(&lineRect, headerRect);
	text[0] = 0;
	{
		int ship = shipext_Get_Mission_Ship();
		if ((unsigned int)ship <= SHIPEXT_SHIP_BWING) {
			const char* shipName = "";
			switch (ship) {
				case SHIPEXT_SHIP_AWING:
					shipName = "A-Wing";
					break;
				case SHIPEXT_SHIP_XWING:
					shipName = "X-Wing";
					break;
				case SHIPEXT_SHIP_YWING:
					shipName = "Y-Wing";
					break;
				case SHIPEXT_SHIP_BWING:
					shipName = "B-Wing";
					break;
				default:
					break;
			}
			strcpy(text, shipName);
		}
	}
	strcat(text, " Starfighter");
	xfont_Print_Centered_Text(text, &lineRect, 0, DEBRIEF_HEADER_COLOR);
	if (shellext_Get_Cur_Scene() == XW_SCENE_DEBRIEF_TOUR) {
		int16_t operationIndex = shipext_Get_Last_Briefed_Tour_Operation();
		strcpy(text, g_debriefTourHeaderPrefix);
		sprintf(numberText, "%d \001Mission \002%d", g_debriefPilotRecord.current_tour + 1,
				operationIndex + 1);
	} else {
		int16_t operationIndex;
		text[0] = 0;
		if (shipext_Get_Combat_Source_Index() >= DEBRIEF_HISTORIC_TOUR_SOURCE_BASE) {
			int secondChoice;
			int16_t choiceIndex;
			strcpy(text, g_debriefTourHeaderPrefix);
			sprintf(numberText, "%d \001",
					shipext_Get_Combat_Source_Index() - DEBRIEF_HISTORIC_TOUR_SOURCE_BASE + 1);
			strcat(text, numberText);
			operationIndex = 0;
			secondChoice = 0;
			for (choiceIndex = 0; choiceIndex < shipext_Get_Combat_Mission(); ++choiceIndex) {
				if (g_tourOperationTables[shipext_Get_Combat_Source_Index() -
										  DEBRIEF_HISTORIC_TOUR_SOURCE_BASE][operationIndex]
						.missionChoiceB != DEBRIEF_NO_SECOND_MISSION) {
					if (secondChoice != 0) {
						secondChoice = 0;
					} else {
						secondChoice = 1;
						continue;
					}
				}
				++operationIndex;
			}
		} else {
			operationIndex = shipext_Get_Combat_Mission();
		}
		strcat(text, "Historic Mission \002");
		sprintf(numberText, "%d", operationIndex + 1);
	}
	strcat(text, numberText);
	strcat(text, " \001Score \002");
	sprintf(numberText, "%ld", (long)g_missionRuntimeState.provingGroundsScore);
	strcat(text, numberText);
	xrect_Offset_Rect(&lineRect, 0, 10);
	xfont_Print_Centered_Text(text, &lineRect, 0, DEBRIEF_HEADER_COLOR);
}

/* DOS94 0x281a08. */
static int16_t Dos94_debrief_SectionMissionResult(Rect* sectionRect, int16_t draw) {
	Rect lineRect;
	xrect_Copy_Rect(&lineRect, sectionRect);
	if (draw != 0) {
		if (g_missionRuntimeState.objectivesCompleted != 0) {
			xfont_Print_Centered_Text("The Mission was a Success!", &lineRect, 0,
									  DEBRIEF_MISSION_SUCCESS_COLOR);
		} else {
			xfont_Print_Centered_Text("The Mission was a Failure!", &lineRect, 0, 54);
		}
	}
	return 12;
}

/* DOS94 0x281a60. */
static int16_t Dos94_debrief_SectionPromotion(Rect* sectionRect, int16_t draw) {
	Rect lineRect;
	char text[DEBRIEF_AWARD_TEXT_CAPACITY];
	xrect_Copy_Rect(&lineRect, sectionRect);
	if (g_missionRuntimeState.newRank != 0) {
		if (draw != 0) {
			strcpy(text, g_debriefPromotionText);
			strcat(text, g_debriefRankNames[g_missionRuntimeState.newRank]);
			xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_AWARD_LEFT_INSET, lineRect.top, 0,
									 DEBRIEF_AWARD_COLOR);
		}
		return 12;
	}
	return 0;
}

/* DOS94 0x281aec. */
static int16_t Dos94_debrief_SectionBattlePatch(Rect* sectionRect, int16_t draw) {
	Rect lineRect;
	char text[DEBRIEF_AWARD_TEXT_CAPACITY];
	const char* awardText = "Battle Patch Awarded!";
	xrect_Copy_Rect(&lineRect, sectionRect);
	if (g_missionRuntimeState.newBattlePatch != 0) {
		if (draw != 0) {
			strcpy(text, awardText);
			xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_AWARD_LEFT_INSET, lineRect.top, 0,
									 DEBRIEF_AWARD_COLOR);
		}
		return 12;
	}
	return 0;
}

/* DOS94 0x281b52. */
static int16_t Dos94_debrief_SectionUncompletedGoals(Rect* sectionRect, int16_t draw) {
	Rect lineRect;
	char text[DEBRIEF_GOAL_LINE_CAPACITY];
	int16_t flightGroupCount;
	int16_t hasGoals;
	int16_t height;
	int16_t scanIndex;
	int16_t groupIndex;
	xrect_Copy_Rect(&lineRect, sectionRect);
	flightGroupCount = g_debriefFlightGroupCount;
	hasGoals = 0;
	height = 0;
	for (scanIndex = 0; scanIndex < flightGroupCount; ++scanIndex) {
		if (g_missionRuntimeState.flightGroupGoalState[scanIndex] == XW_MISSION_GOAL_STATE_UNCOMPLETED)
			hasGoals = 1;
	}
	if (hasGoals != 0) {
		if (draw != 0) {
			xfont_Print_Clipped_Text("Uncompleted Mission Goals", lineRect.left + DEBRIEF_GOAL_HEADING_LEFT,
									 lineRect.top, 0, DEBRIEF_GOAL_HEADING_COLOR);
			xrect_Offset_Rect(&lineRect, 0, 10);
			flightGroupCount = g_debriefFlightGroupCount;
		}
		height = 10;
		for (groupIndex = 0; groupIndex < flightGroupCount; ++groupIndex) {
			if (g_missionRuntimeState.flightGroupGoalState[groupIndex] == XW_MISSION_GOAL_STATE_UNCOMPLETED) {
				if (draw != 0) {
					int16_t craftType = g_debriefFlightGroupCraftTypes[groupIndex];
					const char* separator;
					if (craftType == XW_CRAFT_SPECIES_Y_WING &&
						g_debriefFlightGroupInitialStatus[groupIndex] >= DEBRIEF_B_WING_INITIAL_STATUS_MIN)
						strcpy(text, g_debriefCraftCategoryNames[DEBRIEF_CRAFT_CATEGORY_B_WING]);
					else
						strcpy(text, g_debriefCraftCategoryNames[g_debriefCraftCategoryMap[craftType - 1]]);
					separator = g_debriefResourceStrings[DEBRIEF_GOAL_GROUP_SEPARATOR];
					if (g_debriefFlightGroupCraftCounts[groupIndex] <= 1)
						separator = " ";
					strcat(text, separator);
					if (g_debriefFlightGroupNames[groupIndex][0] != 0) {
						strcat(text, g_debriefFlightGroupNames[groupIndex]);
						strcat(text, " ");
					}
					strcat(text, g_debriefGoalOutcomeText[g_debriefFlightGroupGoals[groupIndex] +
														  DEBRIEF_GOAL_FAILURE_OFFSET]);
					xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_GOAL_LINE_LEFT, lineRect.top, 1,
											 DEBRIEF_GOAL_LINE_COLOR);
					xrect_Offset_Rect(&lineRect, 0, 8);
					flightGroupCount = g_debriefFlightGroupCount;
				}
				height += 8;
			}
		}
		xrect_Offset_Rect(&lineRect, 0, DEBRIEF_GOAL_TRAILING_SPACE);
		height += DEBRIEF_GOAL_TRAILING_SPACE;
	}
	return (int16_t)height;
}

/* DOS94 0x281d3a. */
static int16_t Dos94_debrief_SectionCompletedGoals(Rect* sectionRect, int16_t draw) {
	Rect lineRect;
	char text[DEBRIEF_GOAL_LINE_CAPACITY];
	int16_t flightGroupCount;
	int16_t hasGoals;
	int16_t height;
	int16_t scanIndex;
	int16_t groupIndex;
	xrect_Copy_Rect(&lineRect, sectionRect);
	flightGroupCount = g_debriefFlightGroupCount;
	hasGoals = 0;
	height = 0;
	for (scanIndex = 0; scanIndex < flightGroupCount; ++scanIndex) {
		if (g_missionRuntimeState.flightGroupGoalState[scanIndex] == XW_MISSION_GOAL_STATE_COMPLETED)
			hasGoals = 1;
	}
	if (hasGoals != 0) {
		if (draw != 0) {
			xfont_Print_Clipped_Text("Completed Mission Goals", lineRect.left + DEBRIEF_GOAL_HEADING_LEFT,
									 lineRect.top, 0, DEBRIEF_GOAL_HEADING_COLOR);
			xrect_Offset_Rect(&lineRect, 0, 10);
			flightGroupCount = g_debriefFlightGroupCount;
		}
		height = 10;
		for (groupIndex = 0; groupIndex < flightGroupCount; ++groupIndex) {
			if (g_missionRuntimeState.flightGroupGoalState[groupIndex] == XW_MISSION_GOAL_STATE_COMPLETED) {
				if (draw != 0) {
					int16_t craftType = g_debriefFlightGroupCraftTypes[groupIndex];
					const char* separator;
					if (craftType == XW_CRAFT_SPECIES_Y_WING &&
						g_debriefFlightGroupInitialStatus[groupIndex] >= DEBRIEF_B_WING_INITIAL_STATUS_MIN)
						strcpy(text, g_debriefCraftCategoryNames[DEBRIEF_CRAFT_CATEGORY_B_WING]);
					else
						strcpy(text, g_debriefCraftCategoryNames[g_debriefCraftCategoryMap[craftType - 1]]);
					separator = g_debriefResourceStrings[DEBRIEF_GOAL_GROUP_SEPARATOR];
					if (g_debriefFlightGroupCraftCounts[groupIndex] <= 1)
						separator = " ";
					strcat(text, separator);
					if (g_debriefFlightGroupNames[groupIndex][0] != 0) {
						strcat(text, g_debriefFlightGroupNames[groupIndex]);
						strcat(text, " ");
					}
					strcat(text, g_debriefGoalOutcomeText[g_debriefFlightGroupGoals[groupIndex] - 1]);
					xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_GOAL_LINE_LEFT, lineRect.top, 1,
											 DEBRIEF_GOAL_LINE_COLOR);
					xrect_Offset_Rect(&lineRect, 0, 8);
					flightGroupCount = g_debriefFlightGroupCount;
				}
				height += 8;
			}
		}
		xrect_Offset_Rect(&lineRect, 0, DEBRIEF_GOAL_TRAILING_SPACE);
		height += DEBRIEF_GOAL_TRAILING_SPACE;
	}
	return (int16_t)height;
}

/* DOS94 0x281f22. */
static int16_t Dos94_debrief_SectionObjectGoals(Rect* sectionRect, int16_t draw) {
	Rect lineRect;
	char text[DEBRIEF_OBJECT_GOAL_TEXT_CAPACITY];
	int16_t goalIndex;
	int16_t goalResult;
	int16_t height;
	xrect_Copy_Rect(&lineRect, sectionRect);
	height = 0;
	for (goalIndex = 0; goalIndex < DEBRIEF_OBJECT_GOAL_COUNT; ++goalIndex) {
		switch (goalIndex) {
			case DEBRIEF_OBJECT_GOAL_PROTECT_MINES:
				goalResult = g_missionRuntimeState.objectGoalResults.protectMines;
				break;
			case DEBRIEF_OBJECT_GOAL_DESTROY_MINES:
				goalResult = g_missionRuntimeState.objectGoalResults.destroyMines;
				break;
			case DEBRIEF_OBJECT_GOAL_PROTECT_SATELLITES:
				goalResult = g_missionRuntimeState.objectGoalResults.protectSatellites;
				break;
			case DEBRIEF_OBJECT_GOAL_DESTROY_SATELLITES:
				goalResult = g_missionRuntimeState.objectGoalResults.destroySatellites;
				break;
			case DEBRIEF_OBJECT_GOAL_PROTECT_PROBES:
				goalResult = g_missionRuntimeState.objectGoalResults.protectProbes;
				break;
			case DEBRIEF_OBJECT_GOAL_DESTROY_PROBES:
				goalResult = g_missionRuntimeState.objectGoalResults.destroyProbes;
				break;
		}
		if (goalResult != 0) {
			if (draw != 0) {
				strcpy(
					text,
					g_debriefObjectGoalPrefixes[(
						int16_t)(goalResult + DEBRIEF_OBJECT_GOAL_RESULTS_PER_ACTION * (goalIndex & 1) - 1)]);
				if (g_debriefMissionSurfaceWord != 0 && goalIndex < DEBRIEF_OBJECT_GOAL_PROTECT_PROBES)
					strcat(text, g_debriefSurfaceGoalNames[goalIndex >> 1]);
				else
					strcat(text, g_debriefObjectCategoryNames[goalIndex >> 1]);
				xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_OBJECT_GOAL_LEFT_INSET, lineRect.top,
										 0, DEBRIEF_OBJECT_GOAL_COLOR);
				xrect_Offset_Rect(&lineRect, 0, 12);
			}
			height += 12;
		}
	}
	return height;
}

/* DOS94 0x282038. */
static int16_t Dos94_debrief_SectionSpacecraftKills(Rect* sectionRect, int16_t draw) {
	Rect lineRect;
	char text[DEBRIEF_LOSS_TEXT_CAPACITY];
	XwDebriefKillCategories categories;
	char numberText[DEBRIEF_LOSS_NUMBER_CAPACITY];
	int16_t categoryIndex;
	int index;
	int remainingTypes;
	int16_t totalKills;
	int16_t totalPlayerKills;
	int16_t nonzeroCategories;
	int16_t sorted;
	xrect_Copy_Rect(&lineRect, sectionRect);
	totalKills = 0;
	categoryIndex = 0;
	memset(categories.player, 0, sizeof(categories.player));
	memset(categories.total, 0, sizeof(categories.total));
	for (; categoryIndex < DEBRIEF_CRAFT_CATEGORY_COUNT; ++categoryIndex)
		categories.order[categoryIndex] = categoryIndex;
	totalPlayerKills = 0;
	for (index = 0, remainingTypes = DEBRIEF_CRAFT_CATEGORY_MAP_COUNT; remainingTypes != 0;
		 ++index, --remainingTypes) {
		int category = g_debriefCraftCategoryMap[index];
		uint16_t playerKills = g_playerFlightState.spacecraftKillsByType[index];
		int side;
		int remainingSides;
		categories.player[category] += playerKills;
		totalPlayerKills += playerKills;
		for (side = DEBRIEF_KILL_FIRST_IFF, remainingSides = DEBRIEF_KILL_IFF_COUNT; remainingSides != 0;
			 ++side, --remainingSides) {
			int16_t sideKills = g_missionRuntimeState.destroyedCountsByIffAndCategory[side][index];
			categories.total[category] += sideKills;
			totalKills += sideKills;
		}
	}
	nonzeroCategories = 0;
	for (index = 0; index < DEBRIEF_CRAFT_CATEGORY_COUNT; ++index) {
		if (categories.total[index] != 0)
			++nonzeroCategories;
	}
	do {
		sorted = 1;
		for (index = 1; index < DEBRIEF_CRAFT_CATEGORY_COUNT; ++index) {
			int16_t previousCategory = categories.order[index - 1];
			int16_t currentCategory = categories.order[index];
			if ((int16_t)categories.total[previousCategory] < (int16_t)categories.total[currentCategory]) {
				categories.order[index - 1] = currentCategory;
				categories.order[index] = previousCategory;
				sorted = 0;
			}
		}
	} while (sorted == 0);
	if (totalKills != 0) {
		if (draw != 0) {
			int16_t columnX;
			int16_t rowY;
			int rowsPerColumn;
			int rowIndex;
			int16_t remainingRows;
			strcpy(text, g_debriefSpacecraftKillsText);
			sprintf(numberText, "%d(%d)", totalKills, totalPlayerKills);
			strcat(text, numberText);
			xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_LOSS_HEADER_LEFT, lineRect.top, 0,
									 DEBRIEF_LOSS_HEADER_COLOR);
			columnX = lineRect.left + DEBRIEF_LOSS_COLUMN_LEFT;
			rowY = lineRect.bottom + 2;
			if (nonzeroCategories > 0) {
				rowsPerColumn = (nonzeroCategories + 1) >> 1;
				for (rowIndex = 0, remainingRows = nonzeroCategories; remainingRows != 0;
					 ++rowIndex, --remainingRows) {
					int16_t category = categories.order[rowIndex];
					int16_t displayCount;
					if (rowIndex == rowsPerColumn) {
						columnX += 102;
						rowY = lineRect.bottom + 2;
					}
					displayCount = categories.total[category];
					if (displayCount != 0) {
						int16_t valueOffsetX;
						strcpy(text, g_debriefCraftCategoryNames[category]);
						strcat(text, ": ");
						xfont_Print_Clipped_Text(text, columnX, rowY, 1, DEBRIEF_LOSS_LABEL_COLOR);
						valueOffsetX = 64;
						sprintf(text, g_debriefKillsCountFormat, displayCount,
								(int16_t)categories.player[category]);
						xfont_Print_Clipped_Text(text, columnX + valueOffsetX, rowY, 1,
												 DEBRIEF_LOSS_VALUE_COLOR);
						rowY += 7;
					}
				}
			}
		}
		return 16 + 7 * ((nonzeroCategories + 1) >> 1);
	}
	return 0;
}

/* DOS94 0x2822c0. */
static int16_t Dos94_debrief_SectionSpacecraftLosses(Rect* sectionRect, int16_t draw) {
	Rect lineRect;
	char text[DEBRIEF_LOSS_TEXT_CAPACITY];
	XwDebriefLossCategories categories;
	char numberText[DEBRIEF_LOSS_NUMBER_CAPACITY];
	int16_t categoryIndex;
	int index;
	int remainingTypes;
	int16_t totalLosses;
	int16_t nonzeroCategories;
	int16_t sorted;
	xrect_Copy_Rect(&lineRect, sectionRect);
	categoryIndex = 0;
	memset(categories.total, 0, sizeof(categories.total));
	for (; categoryIndex < DEBRIEF_CRAFT_CATEGORY_COUNT; ++categoryIndex)
		categories.order[categoryIndex] = categoryIndex;
	totalLosses = 0;
	for (index = 0, remainingTypes = DEBRIEF_CRAFT_CATEGORY_MAP_COUNT; remainingTypes != 0;
		 ++index, --remainingTypes) {
		int category = g_debriefCraftCategoryMap[index];
		int16_t craftLosses = g_missionRuntimeState.destroyedCountsByIffAndCategory[0][index];
		categories.total[category] += craftLosses;
		totalLosses += craftLosses;
	}
	nonzeroCategories = 0;
	for (index = 0; index < DEBRIEF_CRAFT_CATEGORY_COUNT; ++index) {
		if (categories.total[index] != 0)
			++nonzeroCategories;
	}
	do {
		sorted = 1;
		for (index = 1; index < DEBRIEF_CRAFT_CATEGORY_COUNT; ++index) {
			int16_t previousCategory = categories.order[index - 1];
			int16_t currentCategory = categories.order[index];
			if ((int16_t)categories.total[previousCategory] < (int16_t)categories.total[currentCategory]) {
				categories.order[index - 1] = currentCategory;
				categories.order[index] = previousCategory;
				sorted = 0;
			}
		}
	} while (sorted == 0);
	if (totalLosses != 0) {
		if (draw != 0) {
			int16_t columnX;
			int16_t rowY;
			int rowsPerColumn;
			int rowIndex;
			int16_t remainingRows;
			strcpy(text, g_debriefSpacecraftLossesText);
			sprintf(numberText, "%d", totalLosses);
			strcat(text, numberText);
			xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_LOSS_HEADER_LEFT, lineRect.top, 0,
									 DEBRIEF_LOSS_HEADER_COLOR);
			columnX = lineRect.left + DEBRIEF_LOSS_COLUMN_LEFT;
			rowY = lineRect.bottom + 2;
			if (nonzeroCategories > 0) {
				rowsPerColumn = (nonzeroCategories + 1) >> 1;
				for (rowIndex = 0, remainingRows = nonzeroCategories; remainingRows != 0;
					 ++rowIndex, --remainingRows) {
					int16_t category = categories.order[rowIndex];
					int16_t displayCount;
					if (rowIndex == rowsPerColumn) {
						columnX += 102;
						rowY = lineRect.bottom + 2;
					}
					displayCount = categories.total[category];
					if (displayCount != 0) {
						int valueOffsetX;
						strcpy(text, g_debriefCraftCategoryNames[category]);
						strcat(text, ":");
						xfont_Print_Clipped_Text(text, columnX, rowY, 1, DEBRIEF_LOSS_LABEL_COLOR);
						valueOffsetX = 64;
						sprintf(text, g_debriefLossCountFormat, displayCount);
						xfont_Print_Clipped_Text(text, columnX + valueOffsetX, rowY, 1,
												 DEBRIEF_LOSS_VALUE_COLOR);
						rowY += 7;
					}
				}
			}
		}
		return 16 + 7 * ((nonzeroCategories + 1) >> 1);
	}
	return 0;
}

/* DOS94 0x2824f8. */
static int16_t Dos94_debrief_SectionSpaceObjectKills(Rect* sectionRect, int16_t draw) {
	Rect lineRect;
	char numberText[DEBRIEF_KILL_NUMBER_CAPACITY];
	char text[DEBRIEF_KILL_TEXT_CAPACITY];
	xrect_Copy_Rect(&lineRect, sectionRect);
	if (g_playerFlightState.spaceObjectKills != 0) {
		if (draw != 0) {
			strcpy(text, g_debriefSpaceObjectKillsText);
			sprintf(numberText, "%d", g_playerFlightState.spaceObjectKills);
			strcat(text, numberText);
			xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_KILL_LINE_LEFT_INSET, lineRect.top, 0,
									 DEBRIEF_KILL_LINE_COLOR);
		}
		return 12;
	}
	return 0;
}

/* DOS94 0x282584. */
static int16_t Dos94_debrief_SectionDeathStarBuildingKills(Rect* sectionRect, int16_t draw) {
	Rect lineRect;
	char numberText[DEBRIEF_KILL_NUMBER_CAPACITY];
	char text[DEBRIEF_KILL_TEXT_CAPACITY];
	xrect_Copy_Rect(&lineRect, sectionRect);
	if (g_playerFlightState.deathStarBuildingKills != 0) {
		if (draw != 0) {
			strcpy(text, g_debriefDeathStarBuildingKillsText);
			sprintf(numberText, "%d", g_playerFlightState.deathStarBuildingKills);
			strcat(text, numberText);
			xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_KILL_LINE_LEFT_INSET, lineRect.top, 0,
									 DEBRIEF_KILL_LINE_COLOR);
		}
		return 12;
	}
	return 0;
}

/* DOS94 0x282610. */
static int Dos94_debrief_SectionLaserAccuracy(Rect* sectionRect, int16_t draw) {
	uint16_t shotsFired = g_playerFlightState.weaponStats.laserShotsFired;
	uint16_t spacecraftHits = g_playerFlightState.weaponStats.laserSpacecraftHits;
	uint16_t surfaceHits = g_playerFlightState.weaponStats.laserSurfaceHits;
	return Dos94_debrief_SectionWeaponAccuracy(sectionRect, DEBRIEF_WEAPON_LASER, shotsFired, spacecraftHits,
											   surfaceHits, draw);
}

/* DOS94 0x282634. */
static int Dos94_debrief_SectionIonAccuracy(Rect* sectionRect, int16_t draw) {
	uint16_t shotsFired = g_playerFlightState.weaponStats.ionShotsFired;
	uint16_t spacecraftHits = g_playerFlightState.weaponStats.ionSpacecraftHits;
	uint16_t surfaceHits = g_playerFlightState.weaponStats.ionSurfaceHits;
	return Dos94_debrief_SectionWeaponAccuracy(sectionRect, DEBRIEF_WEAPON_ION, shotsFired, spacecraftHits,
											   surfaceHits, draw);
}

/* DOS94 0x28265a. */
static int Dos94_debrief_SectionWarheadAccuracy(Rect* sectionRect, int16_t draw) {
	return Dos94_debrief_SectionWeaponAccuracy(sectionRect, DEBRIEF_WEAPON_WARHEAD,
											   g_playerFlightState.weaponStats.warheadsFired,
											   g_playerFlightState.weaponStats.warheadSpacecraftHits,
											   g_playerFlightState.weaponStats.warheadSurfaceHits, draw);
}

/* DOS94 0x282682. */
static int Dos94_debrief_SectionWeaponAccuracy(Rect* sectionRect, int16_t weaponKind, uint16_t shotsFired,
											   uint16_t spacecraftHits, uint16_t surfaceHits, int16_t draw) {
	Rect lineRect;
	char numberText[DEBRIEF_ACCURACY_NUMBER_CAPACITY];
	char text[DEBRIEF_ACCURACY_TEXT_CAPACITY];
	xrect_Copy_Rect(&lineRect, sectionRect);
	if (shotsFired != 0) {
		if (draw != 0) {
			strcpy(text, g_debriefWeaponShotLabels[weaponKind]);
			sprintf(numberText, "%u", (unsigned int)shotsFired);
			strcat(text, numberText);
			xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_ACCURACY_HEADING_LEFT, lineRect.top, 0,
									 DEBRIEF_ACCURACY_HEADING_COLOR);
		}
		if (surfaceHits == 0) {
			if (draw != 0) {
				xrect_Offset_Rect(&lineRect, 0, 12);
				strcpy(text, g_debriefSpacecraftHitsText);
				sprintf(numberText, "%u (%lu%%)", (unsigned int)spacecraftHits,
						(unsigned long)(DEBRIEF_PERCENT_SCALE * spacecraftHits / shotsFired));
				strcat(text, numberText);
				xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_ACCURACY_DETAIL_LEFT, lineRect.top, 1,
										 DEBRIEF_ACCURACY_DETAIL_COLOR);
			}
			return 24;
		}
		if (draw != 0) {
			uint16_t totalHits = surfaceHits + spacecraftHits;
			xrect_Offset_Rect(&lineRect, 0, 12);
			strcpy(text, g_debriefTotalHitsText);
			sprintf(numberText, "%u (%lu%%)", (unsigned int)totalHits,
					(unsigned long)(DEBRIEF_PERCENT_SCALE * totalHits / shotsFired));
			strcat(text, numberText);
			xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_ACCURACY_DETAIL_LEFT, lineRect.top, 1,
									 DEBRIEF_ACCURACY_DETAIL_COLOR);
			xrect_Offset_Rect(&lineRect, 0, 8);
			strcpy(text, g_debriefSpacecraftHitsText);
			sprintf(numberText, "%u (%lu%%)", (unsigned int)spacecraftHits,
					(unsigned long)(DEBRIEF_PERCENT_SCALE * spacecraftHits / shotsFired));
			strcat(text, numberText);
			xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_ACCURACY_DETAIL_LEFT, lineRect.top, 1,
									 DEBRIEF_ACCURACY_DETAIL_COLOR);
			xrect_Offset_Rect(&lineRect, 0, 8);
			strcpy(text, g_debriefSurfaceHitsText);
			sprintf(numberText, "%u (%lu%%)", (unsigned int)surfaceHits,
					(unsigned long)(DEBRIEF_PERCENT_SCALE * surfaceHits / shotsFired));
			strcat(text, numberText);
			xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_ACCURACY_DETAIL_LEFT, lineRect.top, 1,
									 DEBRIEF_ACCURACY_DETAIL_COLOR);
		}
		return 40;
	}
	return 0;
}

/* DOS94 0x283318. */
void Dos94_debrief_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0) {
		soundext_SetPriority((intptr_t)g_debriefMusicState.sound, 0);
		soundext_FadeVolume(g_debriefMusicState.sound, 0, DEBRIEF_MUSIC_FADE_DURATION);
	}
}

/* DOS94 0x2807e8 / 0x280840: role 20 is stamped and consumed. */
static int16_t film_callback(Film* film, FilmObject* object) {
	if (object->id != FTC_ACTOR)
		return 0;
	xfilm_Rewind_Actor_Film(film, object, object + 1);
	Actor* actor = object->object;
	if (actor->var1 != 20)
		return 0;
	Rect bounds, clip, saved_clip;
	uint8_t* saved_pixels;
	int16_t saved_width, saved_height;
	xcanvas_Get_Drawing_Canvas_Bounds(&bounds);
	uint8_t* pixels = xmemhdl_Lock_Handle(g_debriefBackgroundHandle);
	xcanvas_Push_Canvas(&saved_pixels, pixels, &saved_clip, &saved_width, &saved_height, 320, 200, 0);
	if (actor->draw) {
		clip = actor->frame;
		xcanvas_Clip_Rect_To_Canvas(&clip);
		xcanvas_Set_Drawing_Canvas_Clip(&clip);
		actor->draw(actor, &bounds, &clip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(saved_pixels, &saved_clip, saved_width, saved_height);
	xmemhdl_Unlock_Handle(g_debriefBackgroundHandle);
	return 1;
}

/* DOS94 0x280c62. */
static void draw_page_button(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	PushButton* button = (PushButton*)input;
	if (refresh) {
		xstyle_Style_Paint_Border(frame, button->pressed);
		xstyle_Style_Draw_Centered_Icon(input->id == 0 ? 1 : 3, frame, clip, button->pressed);
	}
}

static int16_t draw_background(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	Dos94_debrief_draw_Background(actor, frame, clip, x, y, refresh);
	return refresh != 0;
}

static int16_t draw_statistics(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	Dos94_debrief_draw_Statistics(actor, frame, clip, x, y, refresh);
	return refresh != 0;
}
