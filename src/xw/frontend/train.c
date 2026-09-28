#include "xw/frontend/train.h"

#ifdef XW_MODERN
#include "xw_runtime/integration/landru_adapter.h"
#endif

#include "xw/audio/frontend_audio.h"
#include "xw/audio/soundext.h"
#include "xw/frontend/register.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"
#include "xw/frontend/textext.h"
#include "xw/util/landru_display.h"
#include "xw/util/shared.h"
#include "xw_runtime/integration/combat_callbacks.h"
#include "xw_runtime/integration/train_callbacks.h"

#ifdef XW_MODERN
#include "xw_runtime/runtime/train_task.h"
#endif

#include <landru/actanim.h>
#include <landru/actcust.h>
#include <landru/actdelt.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/file.h>
#include <landru/font.h>
#include <landru/fourcc.h>
#include <landru/inpattr.h>
#include <landru/io.h>
#include <landru/paint.h>
#include <landru/paragrp.h>
#include <landru/view.h>
#include <landru/viewadd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4D7F28
const char g_trainingShipNames[TRAIN_SHIP_COUNT][TRAIN_SHIP_NAME_CAPACITY] = { "X-Wing", "Y-Wing", "A-Wing",
																			   "B-Wing" };

// GLOBAL: XW 0x4D7F58
const char g_trainingWelcomeLines[TRAIN_WELCOME_LINE_COUNT][TRAIN_WELCOME_LINE_CAPACITY] = {
	"Welcome to the Rebel Alliance", "Pilot Proving Ground.  Here you", "will learn to fly the main four",
	"Alliance starfighters.  The",   "'maze' is designed to test your", "skill in maneuvering and firing",
	"at fixed targets.  Scoring is", "based on speed and accuracy.",    ""
};

// GLOBAL: XW 0x4D80C0
char g_trainingScoreNames[TRAIN_SCORE_COUNT][TRAIN_SCORE_NAME_CAPACITY] = {
	"Luke", "Jon", "Larry", "Peter", "Bucky", "Jim", "Edward", "Wade"
};

// GLOBAL: XW 0x4D81C0
int32_t g_trainingScorePoints[TRAIN_SCORE_COUNT] = { 100, 100, 100, 100, 100, 100, 100, 100 };

// GLOBAL: XW 0x4D81E0
uint16_t g_trainingScoreLevels[TRAIN_SCORE_COUNT] = { 1, 1, 1, 1, 1, 1, 1, 1 };

// GLOBAL: XW 0x4D8830
const int16_t g_trainingFocusX[TRAIN_FOCUS_COUNT] = { 32,  160, 160, 160, 310, 32,  94, 144,
													  194, 310, 32,  94,  144, 194, 310 };

// GLOBAL: XW 0x4D8850
const int16_t g_trainingFocusY[TRAIN_FOCUS_COUNT] = { 98,  74, 74, 74,  98,  98,  166, 166,
													  166, 98, 98, 184, 184, 184, 98 };

// GLOBAL: XW 0x4FB4F0
Actor* g_trainingRebelLogoActor = NULL;

// GLOBAL: XW 0x4FB4F4
Actor* g_trainingBWingActor = NULL;

// GLOBAL: XW 0x4FB4FC
Actor* g_trainingShipInfoActor = NULL;

// GLOBAL: XW 0x4FB500
LandruHandle g_trainingLevelParagraph = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4FB504
Actor* g_trainingScreenActor = NULL;

// GLOBAL: XW 0x4FB508
Actor* g_trainingWelcomeActor = NULL;

// GLOBAL: XW 0x4FB50C
Actor* g_trainingNextShipButtonActor = NULL;

// GLOBAL: XW 0x4FB510
Actor* g_trainingYWingActor = NULL;

// GLOBAL: XW 0x4FB514
Input* g_trainingMonitorInput = NULL;

// GLOBAL: XW 0x4FB518
Actor* g_trainingLaunchDoorActor = NULL;

// GLOBAL: XW 0x4FB51C
Actor* g_trainingIncomLogoActor = NULL;

// GLOBAL: XW 0x4FB520
Film* g_trainingFilm = NULL;

// GLOBAL: XW 0x4FB524
ResFile* g_trainingResourceFile = NULL;

// GLOBAL: XW 0x4FB528
Input* g_trainingLaunchDoorInput = NULL;

// GLOBAL: XW 0x4FB530
Actor* g_trainingStarsLeftActor = NULL;

// GLOBAL: XW 0x4FB534
Actor* g_trainingStarsRightActor = NULL;

// GLOBAL: XW 0x4FB538
int16_t g_trainingAvailableLevels = 0;

// GLOBAL: XW 0x4FB53C
Actor* g_trainingNextLevelButtonActor = NULL;

// GLOBAL: XW 0x4FB540
Actor* g_trainingPreviousShipButtonActor = NULL;

// GLOBAL: XW 0x4FB544
Actor* g_trainingHighScoresActor = NULL;

// GLOBAL: XW 0x4FB54C
Input* g_trainingRootInput = NULL;

// GLOBAL: XW 0x4FB550
Input* g_trainingDoorHintInput = NULL;

// GLOBAL: XW 0x4FB554
Actor* g_trainingExitDoorActor = NULL;

// GLOBAL: XW 0x4FB560
REGISTER_PilotFileRecord g_trainingPilot = { 0 };

// GLOBAL: XW 0x4FBC0C
Input* g_trainingNavigationInput = NULL;

// GLOBAL: XW 0x4FBC10
Input* g_trainingExitDoorInput = NULL;

// GLOBAL: XW 0x4FBC14
Actor* g_trainingMissionTextActor = NULL;

// GLOBAL: XW 0x4FBC18
Actor* g_trainingAWingActor = NULL;

// GLOBAL: XW 0x4FBC1C
Actor* g_trainingDisplayedShipActor = NULL;

// GLOBAL: XW 0x4FBC28
Actor* g_trainingBackgroundActor = NULL;

// GLOBAL: XW 0x4FBC2C
Actor* g_trainingPreviousLevelButtonActor = NULL;

// GLOBAL: XW 0x4FBC30
Actor* g_trainingXWingActor = NULL;

// GLOBAL: XW 0x4FBC34
int g_trainingPresentationFrame = 0;

// GLOBAL: XW 0x4FBC3C
int16_t g_trainingTotalLevels = 0;

// GLOBAL: XW 0x4FBC40
int16_t g_trainingFocusIndex = 0;

// GLOBAL: XW 0x4FBC44
XwTrainingMusicState g_trainingMusic = { NULL, NULL, NULL, 0, 0 };

// GLOBAL: XW 0x4FBC58
int16_t g_trainingTypingSoundActive = 0;

// GLOBAL: XW 0x4FBC5C
int16_t g_trainingTypingSoundActivity = 0;

// FUNCTION: XW 0x465210
XwShellSceneResult train_Train(struct XwShellContext* shell) {
	LandruFile* scoreFile;
	int16_t scoreIndex;
	uint16_t scoreLevel;
	uint32_t scorePoints;
	Rect frame;
	PushButton* button;
	Input* shipLabelInput;
	Input* levelLabelInput;
	g_trainingFocusIndex = TRAIN_INITIAL_FOCUS;
	xio_Set_Mouse_Position(TRAIN_INITIAL_MOUSE_X, TRAIN_INITIAL_MOUSE_Y);
	g_trainingPresentationFrame = 0;
	scoreFile = xfile_Open_File(LANDRU_FILE_ROOT_USER, "X-Wing Data\\train.hgh", "rb");
	if (scoreFile != NULL) {
		for (scoreIndex = 0; scoreIndex < TRAIN_SCORE_COUNT; ++scoreIndex) {
			xfile_Read_Data_From_File(scoreFile, g_trainingScoreNames[scoreIndex],
									  TRAIN_SCORE_DISK_NAME_CAPACITY);
			xfile_Read_Long_From_File(scoreFile, &scorePoints);
			g_trainingScorePoints[scoreIndex] = scorePoints;
			xfile_Read_Word_From_File(scoreFile, &scoreLevel);
			g_trainingScoreLevels[scoreIndex] = scoreLevel;
		}
		xfile_Close_File(scoreFile);
	}
#ifdef XW_MODERN
	g_trainingResourceFile = XwLandru_OpenMissionResource("missions.lfd");
#else
	g_trainingResourceFile = xres_Open_Resource("missions.lfd");
#endif
	g_trainingLevelParagraph = xparagrp_Res_Paragraph(g_trainingResourceFile, "level");
	g_trainingTotalLevels = xparagrp_Count_Paragraph_Strings(g_trainingLevelParagraph, 0);
	xres_Close_Resource(g_trainingResourceFile);
	g_trainingResourceFile = xres_Open_Resource(":X-Wing Data\\RESOURCE\\train640.lfd");
	if (g_trainingResourceFile == NULL)
		g_trainingResourceFile = xres_Open_Resource("train640.lfd");
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xview_Set_View_ZPlane(TRAIN_ROOM_VIEW, TRAIN_ROOM_NEAR_Z, TRAIN_ROOM_FAR_Z);
	xrect_Set_Rect(&frame, TRAIN_INFO_PANEL_LEFT, TRAIN_INFO_PANEL_TOP, TRAIN_INFO_PANEL_RIGHT,
				   TRAIN_INFO_PANEL_BOTTOM);
	xview_Set_View_Frame(TRAIN_SCREEN_VIEW, &frame);
	xview_Set_View_ZPlane(TRAIN_SCREEN_VIEW, 0, TRAIN_SCREEN_FAR_Z);
	xview_Disable_View_Copy(TRAIN_SCREEN_VIEW);
	xview_Enable_View_Erase(TRAIN_SCREEN_VIEW);
	xrect_Set_Rect(&frame, TRAIN_CANVAS_LEFT, TRAIN_CANVAS_TOP, TRAIN_CANVAS_RIGHT, TRAIN_CANVAS_BOTTOM);
	g_trainingFilm = xfilm_Res_Film(g_trainingResourceFile, "train", &frame, 0, 0, 0);
	xfilm_Set_Film_Def_Palette(g_trainingFilm, shell->standardPalette);
	g_trainingBackgroundActor = xactor_Find_Actor(FOURCC_DELT, "train");
	xactor_Non_Refreshable_Actor(g_trainingBackgroundActor);
	g_trainingExitDoorActor = xactor_Find_Actor(FOURCC_ANIM, "ldoor");
	xactor_Set_Actor_User_Function(g_trainingExitDoorActor, train_user_Door);
	xactor_Non_Refreshable_Actor(g_trainingExitDoorActor);
	g_trainingExitDoorActor->id = 0;
	g_trainingLaunchDoorActor = xactor_Find_Actor(FOURCC_ANIM, "rdoor");
	xactor_Set_Actor_User_Function(g_trainingLaunchDoorActor, train_user_Door);
	xactor_Non_Refreshable_Actor(g_trainingLaunchDoorActor);
	g_trainingLaunchDoorActor->id = TRAIN_DOOR_LAUNCH;
	g_trainingScreenActor = xactor_Find_Actor(FOURCC_ANIM, "screen");
	xactor_Non_Refreshable_Actor(g_trainingScreenActor);
	g_trainingStarsLeftActor = xactdelt_Res_Delta_Actor(g_trainingResourceFile, "stars", &frame, 0,
														-TRAIN_INFO_PANEL_TOP, TRAIN_SCREEN_FAR_Z);
	g_trainingStarsRightActor =
		xactdelt_Res_Delta_Actor(g_trainingResourceFile, "stars", &frame, TRAIN_STAR_VIEW_WIDTH,
								 -TRAIN_INFO_PANEL_TOP, TRAIN_SCREEN_FAR_Z);
	xactor_Set_Actor_Time(g_trainingStarsLeftActor, TRAIN_PRESENTATION_LEAD_TICKS, 0);
	xactor_Set_Actor_Time(g_trainingStarsRightActor, TRAIN_PRESENTATION_LEAD_TICKS, 0);
	xactor_Set_Actor_User_Function(g_trainingStarsLeftActor, train_user_Stars);
	xactor_Set_Actor_User_Function(g_trainingStarsRightActor, train_user_Stars);
	g_trainingIncomLogoActor =
		xactdelt_Res_Delta_Actor(g_trainingResourceFile, "incom", &frame, 0, 0, TRAIN_PRESENTATION_DEPTH);
	g_trainingRebelLogoActor =
		xactdelt_Res_Delta_Actor(g_trainingResourceFile, "rebel", &frame, 0, 0, TRAIN_PRESENTATION_DEPTH);
	xactor_Set_Actor_Time(g_trainingIncomLogoActor, 0, 0);
	xactor_Set_Actor_Time(g_trainingRebelLogoActor, 0, 0);
	g_trainingShipInfoActor =
		xactcust_Alloc_Custom_Actor(0, &frame, TRAIN_INFO_PANEL_LEFT, TRAIN_INFO_PANEL_TOP, 0);
	xactor_Set_Actor_Update_Function(g_trainingShipInfoActor, train_UpdateShipInfoAnimation);
	xactor_Set_Actor_User_Function(g_trainingShipInfoActor, train_user_ShipInfo);
	xactor_Set_Actor_Draw_Function(g_trainingShipInfoActor, train_DrawShipInfoPanel);
	g_trainingShipInfoActor->var1 = 0;
	xrect_Set_Rect(&frame, TRAIN_MONITOR_CONTENT_LEFT, TRAIN_MONITOR_CONTENT_TOP, TRAIN_MONITOR_CONTENT_RIGHT,
				   TRAIN_MONITOR_CONTENT_BOTTOM);
	g_trainingHighScoresActor = xactcust_Alloc_Custom_Actor(0, &frame, 0, 0, 0);
	xactor_Set_Actor_User_Function(g_trainingHighScoresActor, train_user_HighScores);
	xactor_Set_Actor_Draw_Function(g_trainingHighScoresActor, train_Draw_Train_Screen_Score);
	g_trainingHighScoresActor->var1 = 0;
	g_trainingWelcomeActor = xactcust_Alloc_Custom_Actor(0, &frame, 0, 0, 0);
	xactor_Set_Actor_User_Function(g_trainingWelcomeActor, train_user_Welcome);
	xactor_Set_Actor_Draw_Function(g_trainingWelcomeActor, train_DrawWelcomeText);
	g_trainingWelcomeActor->var1 = 0;
	g_trainingMissionTextActor = xactcust_Alloc_Custom_Actor(0, &frame, 0, 0, 0);
	xactor_Set_Actor_User_Function(g_trainingMissionTextActor, train_user_MissionText);
	xactor_Set_Actor_Draw_Function(g_trainingMissionTextActor, train_Draw_Train_Screen_Mission);
	g_trainingMissionTextActor->var1 = 0;
	xrect_Set_Rect(&frame, TRAIN_CANVAS_LEFT, TRAIN_CANVAS_TOP, TRAIN_CANVAS_RIGHT, TRAIN_CANVAS_BOTTOM);
	g_trainingRootInput = xinput_Alloc_Input(NULL, &frame, 0, 0);
	xrect_Set_Rect(&frame, TRAIN_INFO_PANEL_LEFT, TRAIN_INFO_PANEL_TOP, TRAIN_INFO_PANEL_RIGHT,
				   TRAIN_INFO_PANEL_BOTTOM);
	g_trainingMonitorInput = xinput_Alloc_Input(g_trainingRootInput, &frame, 0, 0);
	xinpattr_Set_Input_Update_Function(g_trainingMonitorInput, XwCombat_UpdateMonitor);
	xinpattr_Set_Input_User_Function(g_trainingMonitorInput, train_iuser_Train_Screen);
	xinpattr_Hide_Input(g_trainingMonitorInput);
	xrect_Set_Rect(&frame, TRAIN_EXIT_DOOR_LEFT, TRAIN_EXIT_DOOR_TOP, TRAIN_EXIT_DOOR_RIGHT,
				   TRAIN_EXIT_DOOR_BOTTOM);
	g_trainingExitDoorInput = xinput_Alloc_Input(g_trainingRootInput, &frame, 0, 0);
	xinpattr_Set_Input_Update_Function(g_trainingExitDoorInput, train_iupdate_Door);
	xinpattr_Set_Input_User_Function(g_trainingExitDoorInput, train_iuser_Door);
	g_trainingExitDoorInput->mouseUsage = allInput;
	g_trainingExitDoorInput->id = 0;
	xrect_Set_Rect(&frame, TRAIN_LAUNCH_DOOR_LEFT, TRAIN_LAUNCH_DOOR_TOP, TRAIN_LAUNCH_DOOR_RIGHT,
				   TRAIN_LAUNCH_DOOR_BOTTOM);
	g_trainingLaunchDoorInput = xinput_Alloc_Input(g_trainingRootInput, &frame, 0, 0);
	xinpattr_Set_Input_Update_Function(g_trainingLaunchDoorInput, train_iupdate_Door);
	xinpattr_Set_Input_User_Function(g_trainingLaunchDoorInput, train_iuser_Door);
	g_trainingLaunchDoorInput->mouseUsage = allInput;
	g_trainingLaunchDoorInput->id = TRAIN_DOOR_LAUNCH;
	xrect_Set_Rect(&frame, TRAIN_SHIP_LABEL_LEFT, TRAIN_SHIP_LABEL_TOP, TRAIN_SHIP_LABEL_RIGHT,
				   TRAIN_SHIP_LABEL_BOTTOM);
	g_trainingDoorHintInput = xinput_Alloc_Input(g_trainingRootInput, &frame, 0, 0);
	xinpattr_Set_Input_Draw_Function(g_trainingDoorHintInput, train_idraw_DoorHint);
	xinpattr_Hide_Input(g_trainingDoorHintInput);
	xrect_Set_Rect(&frame, TRAIN_NAVIGATION_LEFT, TRAIN_NAVIGATION_TOP, TRAIN_NAVIGATION_RIGHT,
				   TRAIN_NAVIGATION_BOTTOM);
	g_trainingNavigationInput = xinput_Alloc_Input(g_trainingRootInput, &frame, 0, 0);
	xrect_Set_Rect(&frame, TRAIN_PREVIOUS_SHIP_LEFT, TRAIN_PREVIOUS_SHIP_TOP, TRAIN_PREVIOUS_SHIP_RIGHT,
				   TRAIN_PREVIOUS_SHIP_BOTTOM);
	xrect_Offset_Rect(&frame, -TRAIN_NAVIGATION_LEFT, -TRAIN_NAVIGATION_TOP);
	button = xbtnpush_Alloc_Button(g_trainingNavigationInput, &frame, 0, train_iuser_Train, NULL,
								   TRAIN_BUTTON_PREVIOUS_SHIP);
	xinpattr_Set_Input_Draw_Function(&button->header, XwTrain_DrawNavigationButton);
	g_trainingPreviousShipButtonActor = xactor_Find_Actor(FOURCC_ANIM, "butttl");
	xrect_Set_Rect(&frame, TRAIN_NEXT_SHIP_LEFT, TRAIN_NEXT_SHIP_TOP, TRAIN_NEXT_SHIP_RIGHT,
				   TRAIN_NEXT_SHIP_BOTTOM);
	xrect_Offset_Rect(&frame, -TRAIN_NAVIGATION_LEFT, -TRAIN_NAVIGATION_TOP);
	button = xbtnpush_Alloc_Button(g_trainingNavigationInput, &frame, 0, train_iuser_Train, NULL,
								   TRAIN_BUTTON_NEXT_SHIP);
	xinpattr_Set_Input_Draw_Function(&button->header, XwTrain_DrawNavigationButton);
	g_trainingNextShipButtonActor = xactor_Find_Actor(FOURCC_ANIM, "butttr");
	xrect_Set_Rect(&frame, TRAIN_SHIP_LABEL_LEFT, TRAIN_SHIP_LABEL_TOP, TRAIN_SHIP_LABEL_RIGHT,
				   TRAIN_SHIP_LABEL_BOTTOM);
	xrect_Offset_Rect(&frame, -TRAIN_NAVIGATION_LEFT, -TRAIN_NAVIGATION_TOP);
	shipLabelInput = xinput_Alloc_Input(g_trainingNavigationInput, &frame, 0, 0);
	xinpattr_Set_Input_Update_Function(shipLabelInput, train_iupdate_SelectionLabel);
	xinpattr_Set_Input_Draw_Function(shipLabelInput, train_idraw_Train);
	shipLabelInput->id = 0;
	xrect_Set_Rect(&frame, TRAIN_PREVIOUS_LEVEL_LEFT, TRAIN_PREVIOUS_LEVEL_TOP, TRAIN_PREVIOUS_LEVEL_RIGHT,
				   TRAIN_PREVIOUS_LEVEL_BOTTOM);
	xrect_Offset_Rect(&frame, -TRAIN_NAVIGATION_LEFT, -TRAIN_NAVIGATION_TOP);
	button = xbtnpush_Alloc_Button(g_trainingNavigationInput, &frame, 0, train_iuser_Train, NULL,
								   TRAIN_BUTTON_PREVIOUS_LEVEL);
	xinpattr_Set_Input_Draw_Function(&button->header, XwTrain_DrawNavigationButton);
	g_trainingPreviousLevelButtonActor = xactor_Find_Actor(FOURCC_ANIM, "buttbl");
	xrect_Set_Rect(&frame, TRAIN_NEXT_LEVEL_LEFT, TRAIN_NEXT_LEVEL_TOP, TRAIN_NEXT_LEVEL_RIGHT,
				   TRAIN_NEXT_LEVEL_BOTTOM);
	xrect_Offset_Rect(&frame, -TRAIN_NAVIGATION_LEFT, -TRAIN_NAVIGATION_TOP);
	button = xbtnpush_Alloc_Button(g_trainingNavigationInput, &frame, 0, train_iuser_Train, NULL,
								   TRAIN_BUTTON_NEXT_LEVEL);
	xinpattr_Set_Input_Draw_Function(&button->header, XwTrain_DrawNavigationButton);
	g_trainingNextLevelButtonActor = xactor_Find_Actor(FOURCC_ANIM, "buttbr");
	xrect_Set_Rect(&frame, TRAIN_LEVEL_LABEL_LEFT, TRAIN_LEVEL_LABEL_TOP, TRAIN_LEVEL_LABEL_RIGHT,
				   TRAIN_LEVEL_LABEL_BOTTOM);
	xrect_Offset_Rect(&frame, -TRAIN_NAVIGATION_LEFT, -TRAIN_NAVIGATION_TOP);
	levelLabelInput = xinput_Alloc_Input(g_trainingNavigationInput, &frame, 0, 0);
	xinpattr_Set_Input_Update_Function(levelLabelInput, train_iupdate_SelectionLabel);
	xinpattr_Set_Input_Draw_Function(levelLabelInput, train_idraw_Train);
	levelLabelInput->id = TRAIN_LABEL_LEVEL;
	xrect_Set_Rect(&frame, TRAIN_CANVAS_LEFT, TRAIN_CANVAS_TOP, TRAIN_CANVAS_RIGHT, TRAIN_CANVAS_BOTTOM);
	if (Shared_ReturnZero() != 0) {
		g_trainingAWingActor = NULL;
		g_trainingXWingActor = NULL;
		g_trainingYWingActor = NULL;
		g_trainingBWingActor = NULL;
	} else {
		g_trainingAWingActor =
			xactanim_Res_Anim_Actor(g_trainingResourceFile, "a_wing", &frame, TRAIN_PRESENTATION_RESET_X,
									TRAIN_AWING_RESET_Y, TRAIN_PRESENTATION_DEPTH);
		xactor_Set_Actor_User_Function(g_trainingAWingActor, train_user_AWingPresentation);
		g_trainingAWingActor->var1 = TRAIN_AWING_TYPE_MARKER;
		g_trainingXWingActor =
			xactanim_Res_Anim_Actor(g_trainingResourceFile, "x_wing", &frame, TRAIN_PRESENTATION_RESET_X,
									TRAIN_PRESENTATION_RESET_Y, TRAIN_PRESENTATION_DEPTH);
		xactor_Set_Actor_User_Function(g_trainingXWingActor, train_user_XWingPresentation);
		g_trainingXWingActor->var1 = TRAIN_XWING_TYPE_MARKER;
		g_trainingYWingActor =
			xactanim_Res_Anim_Actor(g_trainingResourceFile, "y-wing", &frame, TRAIN_YWING_RESET_X,
									TRAIN_YWING_RESET_Y, TRAIN_PRESENTATION_DEPTH);
		xactor_Set_Actor_User_Function(g_trainingYWingActor, train_user_YWingPresentation);
		g_trainingYWingActor->var1 = TRAIN_YWING_TYPE_MARKER;
		g_trainingBWingActor =
			xactanim_Res_Anim_Actor(g_trainingResourceFile, "b_wing", &frame, TRAIN_BWING_RESET_X,
									TRAIN_BWING_RESET_Y, TRAIN_PRESENTATION_DEPTH);
		xactor_Set_Actor_User_Function(g_trainingBWingActor, train_user_BWingPresentation);
		g_trainingBWingActor->var1 = TRAIN_BWING_TYPE_MARKER;
	}
	train_LoadPilotProgress(g_RegisterShellPilot.name);
	g_trainingAvailableLevels = g_trainingPilot.trainingLevelProgress[shipext_Get_Train_Ship()] + 1;
	if (g_trainingAvailableLevels > g_trainingTotalLevels)
		g_trainingAvailableLevels = g_trainingTotalLevels;
	xio_Set_Key_Buttons();
	train_OpenMusic(g_trainingResourceFile, g_trainingFilm, &g_trainingPresentationFrame);
	train_LoadSoundEffects();
	FrontendAudio_PlayFile("XwingCD\\music\\combat.wav", 1u);
	xview_Set_View_Update_Function(train_end_Train_View);
#ifdef XW_MODERN
	XwTrain_RunView();
#else
	j_xviewadd_Handle_View();
	xview_Clear_View_Update_Function();
	train_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xio_Clear_Key_Buttons();
	xparagrp_Get_Paragraph_String(g_trainingLevelParagraph, g_shellMissionName, 0, shipext_Get_Train_Level());
	xparagrp_Free_Paragraph(g_trainingLevelParagraph);
	xview_Enable_All_View_Erase();
	xview_Set_View_ZPlane(TRAIN_ROOM_VIEW, TRAIN_FULL_NEAR_Z, TRAIN_FULL_FAR_Z);
	xview_Set_View_ZPlane(TRAIN_SCREEN_VIEW, TRAIN_FULL_NEAR_Z, TRAIN_FULL_FAR_Z);
	xrect_Set_Rect(&frame, 0, 0, 0, 0);
	xview_Set_View_Frame(TRAIN_SCREEN_VIEW, &frame);
	xview_Enable_View_Copy(TRAIN_SCREEN_VIEW);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	xres_Close_Resource(g_trainingResourceFile);
	LandruDisplay_ForwardLegacyNoOp(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x465E10
void train_end_Train_View(int time) {
	int16_t key;
	int cels;
	if (time == 0 && (uint16_t)xcursor_Is_Cursor_Visible() == 0)
		xcursor_Show_Cursor();
	key = xio_Get_Free_Key();
	if (key != 0 && shellext_MoveGridFocus(&g_trainingFocusIndex, g_trainingFocusX, g_trainingFocusY,
										   TRAIN_FOCUS_ROWS, TRAIN_FOCUS_COLUMNS, key) != 0) {
		xio_Set_Mouse_Position(g_trainingFocusX[g_trainingFocusIndex],
							   g_trainingFocusY[g_trainingFocusIndex]);
		xio_Get_Key();
	}
	cels = g_trainingFilm->cels;
	if (time >= cels - TRAIN_PRESENTATION_LEAD_TICKS) {
		if (time == cels)
			xinpattr_Show_Input(g_trainingMonitorInput);
		g_trainingPresentationFrame = (g_trainingPresentationFrame + 1) % (TRAIN_WELCOME_END + 1);
	}
	if (time == 0)
		train_HandleSoundAction(TRAIN_SOUND_HYDRAULIC_START);
	if (time == TRAIN_HYDRAULIC_STOP_TICK)
		train_HandleSoundAction(TRAIN_SOUND_HYDRAULIC_STOP);
	train_HandleSoundAction(TRAIN_SOUND_TYPING_UPDATE);
	train_LoadPresentationShipAtFrame(g_trainingPresentationFrame);
}

// FUNCTION: XW 0x465EF0
void train_iuser_Train_Screen(Input* input, int time) {
	(void)time;
	if (input->var1 != 0) {
		if ((uint16_t)xactor_Is_Actor_Visible(g_trainingShipInfoActor) != 0) {
			xactor_Hide_Actor(g_trainingShipInfoActor);
			g_trainingShipInfoActor->var1 = 0;
			g_trainingShipInfoActor->var2 = 0;
		}
		if (g_trainingPresentationFrame < TRAIN_MISSION_TEXT_END) {
			g_trainingPresentationFrame = TRAIN_MISSION_TEXT_END - 1;
		} else if (g_trainingPresentationFrame < TRAIN_HIGH_SCORES_END) {
			g_trainingPresentationFrame = TRAIN_HIGH_SCORES_END - 1;
		} else if (g_trainingPresentationFrame < TRAIN_PRESENTATION_INTERVAL_3_END) {
			g_trainingPresentationFrame = TRAIN_PRESENTATION_INTERVAL_3_END - 1;
		} else if (g_trainingPresentationFrame < TRAIN_PRESENTATION_INTERVAL_4_END) {
			g_trainingPresentationFrame = TRAIN_PRESENTATION_INTERVAL_4_END - 1;
		} else if (g_trainingPresentationFrame < TRAIN_PRESENTATION_INTERVAL_5_END) {
			g_trainingPresentationFrame = TRAIN_PRESENTATION_INTERVAL_5_END - 1;
		} else {
			g_trainingPresentationFrame = TRAIN_WELCOME_END - 1;
		}
		input->var1 = 0;
	}
}

// FUNCTION: XW 0x465FD0
int16_t train_iupdate_Door(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent, int rightEvent,
						   int16_t x, int16_t y) {
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (key != 0)
		return 0;
	if (leftEvent == TRAIN_DOOR_ACTIVATE_EVENT || rightEvent == TRAIN_DOOR_ACTIVATE_EVENT) {
		input->var1 = TRAIN_DOOR_ACTION_EXIT;
		if (input->id != TRAIN_DOOR_EXIT) {
			switch (shipext_Get_Train_Ship()) {
				case SHIPEXT_SHIP_AWING:
					input->var2 = XW_SCENE_TRAINING_LAUNCH_AWING;
					break;
				case SHIPEXT_SHIP_XWING:
					input->var2 = XW_SCENE_TRAINING_LAUNCH_XWING;
					break;
				case SHIPEXT_SHIP_YWING:
					input->var2 = XW_SCENE_TRAINING_LAUNCH_YWING;
					break;
				case SHIPEXT_SHIP_BWING:
					input->var2 = XW_SCENE_TRAINING_LAUNCH_BWING;
					break;
			}
		} else {
			input->var2 = XW_SCENE_TRAINING_RETURN_SHUTTLE;
		}
	} else {
		input->var1 = TRAIN_DOOR_ACTION_HINT;
	}
	if (input->id == TRAIN_DOOR_EXIT)
		g_trainingExitDoorActor->var1 = 1;
	else
		g_trainingLaunchDoorActor->var1 = 1;
	return 1;
}

// FUNCTION: XW 0x466080
void train_iuser_Door(Input* input, int time) {
	(void)time;
	switch (input->var1) {
		case TRAIN_DOOR_ACTION_IDLE:
			if (xinpattr_Is_Input_Visible(g_trainingDoorHintInput) != 0 &&
				input->id == g_trainingDoorHintInput->var1) {
				xinpattr_Show_Input(g_trainingNavigationInput);
				xinpattr_Hide_Input(g_trainingDoorHintInput);
				xinpattr_Refresh_Input(g_trainingNavigationInput);
			}
			break;
		case TRAIN_DOOR_ACTION_EXIT:
			xerror_Set_Landru_Exit(input->var2);
			break;
		case TRAIN_DOOR_ACTION_HINT:
			if (xinpattr_Is_Input_Visible(g_trainingNavigationInput) != 0) {
				xinpattr_Hide_Input(g_trainingNavigationInput);
				xinpattr_Show_Input(g_trainingDoorHintInput);
				xinpattr_Refresh_Input(g_trainingDoorHintInput);
				g_trainingDoorHintInput->var1 = input->id;
			}
			input->var1 = TRAIN_DOOR_ACTION_IDLE;
			break;
	}
}

// FUNCTION: XW 0x466150
void train_idraw_DoorHint(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	Rect levelLabelRect;
	(void)clip;
	if (refresh != 0) {
		xrect_Set_Rect(&levelLabelRect, TRAIN_LEVEL_LABEL_LEFT, TRAIN_LEVEL_LABEL_TOP,
					   TRAIN_LEVEL_LABEL_RIGHT, TRAIN_LEVEL_LABEL_BOTTOM);
		xpaint_Paint_Clipped_Rect(&levelLabelRect, TRAIN_LABEL_BACKGROUND_COLOR);
		xpaint_Paint_Clipped_Rect(frame, TRAIN_LABEL_BACKGROUND_COLOR);
		if (input->var1 != 0) {
			xrect_Offset_Rect(frame, TRAIN_HINT_SHADOW_OFFSET, TRAIN_HINT_SHADOW_OFFSET);
			xfont_Print_Centered_Text("Enter Maze", frame, TRAIN_LABEL_FONT, TRAIN_LABEL_BACKGROUND_COLOR);
			xrect_Offset_Rect(frame, -TRAIN_HINT_SHADOW_OFFSET, -TRAIN_HINT_SHADOW_OFFSET);
			xfont_Print_Centered_Text("Enter Maze", frame, TRAIN_LABEL_FONT, TRAIN_LABEL_TEXT_COLOR);
		} else {
			xrect_Offset_Rect(frame, TRAIN_HINT_SHADOW_OFFSET, TRAIN_HINT_SHADOW_OFFSET);
			xfont_Print_Centered_Text("Return To Spaceport", frame, TRAIN_LABEL_FONT,
									  TRAIN_LABEL_BACKGROUND_COLOR);
			xrect_Offset_Rect(frame, -TRAIN_HINT_SHADOW_OFFSET, -TRAIN_HINT_SHADOW_OFFSET);
			xfont_Print_Centered_Text("Return To Spaceport", frame, TRAIN_LABEL_FONT, TRAIN_LABEL_TEXT_COLOR);
		}
	}
}

// FUNCTION: XW 0x466230
void train_iuser_Train(Input* input, int time) {
	(void)time;
	if (xinpattr_Get_Input_Selected(input) != 0) {
		int16_t selectionChanged = 0;
		int16_t selectedShip = shipext_Get_Train_Ship();
		int16_t selectedLevel = shipext_Get_Train_Level();
		int buttonId = input->id;
		switch (buttonId) {
			case TRAIN_BUTTON_PREVIOUS_SHIP:
			case TRAIN_BUTTON_NEXT_SHIP: {
				int16_t availableLevels;
				if (buttonId == TRAIN_BUTTON_PREVIOUS_SHIP) {
					do {
						if (selectedShip != 0)
							--selectedShip;
						else
							selectedShip = SHIPEXT_SHIP_COUNT - 1;
						selectionChanged =
							shipext_IsShipAvailable(selectedShip) & (selectedShip != TRAIN_SKIPPED_SHIP_SLOT);
					} while (selectionChanged == 0);
				} else {
					do {
						if (selectedShip < SHIPEXT_SHIP_COUNT - 1)
							++selectedShip;
						else
							selectedShip = 0;
						selectionChanged =
							shipext_IsShipAvailable(selectedShip) & (selectedShip != TRAIN_SKIPPED_SHIP_SLOT);
					} while (selectionChanged == 0);
				}
				shipext_Set_Train_Ship(selectedShip);
				availableLevels = g_trainingPilot.trainingLevelProgress[shipext_Get_Train_Ship()] + 1;
				g_trainingAvailableLevels = availableLevels;
				if (availableLevels > g_trainingTotalLevels) {
					availableLevels = g_trainingTotalLevels;
					g_trainingAvailableLevels = g_trainingTotalLevels;
				}
				if (selectedLevel >= availableLevels)
					shipext_Set_Train_Level(g_trainingAvailableLevels - 1);
				break;
			}
			case TRAIN_BUTTON_PREVIOUS_LEVEL:
				if (selectedLevel != 0)
					shipext_Set_Train_Level(selectedLevel - 1);
				else
					shipext_Set_Train_Level(g_trainingAvailableLevels - 1);
				selectionChanged = 1;
				break;
			case TRAIN_BUTTON_NEXT_LEVEL:
				if (selectedLevel < g_trainingAvailableLevels - 1)
					shipext_Set_Train_Level(selectedLevel + 1);
				else
					shipext_Set_Train_Level(0);
				selectionChanged = 1;
				break;
		}
		if (selectionChanged != 0) {
			xinpattr_Refresh_Input(g_trainingNavigationInput);
			train_RestartMissionText();
		}
	}
}

// FUNCTION: XW 0x466370
void train_idraw_NavigationButton(PushButton* button, Rect* frame, Rect* clip, int16_t refresh) {
	if (refresh != 0) {
		Actor* buttonActor;
		switch (button->header.id) {
			case TRAIN_BUTTON_PREVIOUS_SHIP:
				buttonActor = g_trainingPreviousShipButtonActor;
				break;
			case TRAIN_BUTTON_NEXT_SHIP:
				buttonActor = g_trainingNextShipButtonActor;
				break;
			case TRAIN_BUTTON_PREVIOUS_LEVEL:
				buttonActor = g_trainingPreviousLevelButtonActor;
				break;
			case TRAIN_BUTTON_NEXT_LEVEL:
				buttonActor = g_trainingNextLevelButtonActor;
				break;
			default:
				buttonActor = NULL;
				break;
		}
		if (buttonActor != NULL) {
			xactor_Set_Actor_State(buttonActor, button->pressed, 0);
			xactanim_Draw_Anim_Actor(buttonActor, frame, clip, 0, 0, refresh);
		}
	}
}

// FUNCTION: XW 0x4663F0
int16_t train_iupdate_SelectionLabel(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
									 int rightEvent, int16_t x, int16_t y) {
	(void)input;
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (key == 0 && (leftEvent == TRAIN_MOUSE_PRESS || rightEvent == TRAIN_MOUSE_PRESS)) {
		if (xactor_Is_Actor_Visible(g_trainingMissionTextActor) != 0) {
			g_trainingPresentationFrame = TRAIN_MISSION_TEXT_START;
			g_trainingMissionTextActor->var2 = TRAIN_MISSION_TEXT_REVEAL_ALL;
		} else {
			train_RestartMissionText();
		}
	}
	return 0;
}

// FUNCTION: XW 0x466450
void train_idraw_Train(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	enum { LABEL_CAPACITY = 32 };

	char label[LABEL_CAPACITY];
	(void)clip;
	if (refresh != 0) {
		xpaint_Paint_Clipped_Rect(frame, TRAIN_LABEL_BACKGROUND_COLOR);
		if (input->id == TRAIN_LABEL_SHIP) {
			xfont_Print_Centered_Text(g_trainingShipNames[shipext_Get_Train_Ship()], frame, TRAIN_LABEL_FONT,
									  TRAIN_LABEL_TEXT_COLOR);
		} else {
			sprintf(label, "Level %d/%d", shipext_Get_Train_Level() + 1, g_trainingAvailableLevels);
			xfont_Print_Centered_Text(label, frame, TRAIN_LABEL_FONT, TRAIN_LABEL_TEXT_COLOR);
		}
	}
}

// FUNCTION: XW 0x4664E0
void train_user_XWingPresentation(Actor* actor, int time) {
	(void)time;
	if ((g_trainingPresentationFrame < TRAIN_XWING_START || g_trainingPresentationFrame >= TRAIN_XWING_END) &&
		(uint16_t)xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Set_Actor_Pos(actor, TRAIN_PRESENTATION_RESET_X, TRAIN_PRESENTATION_RESET_Y, 0, 0);
		xactor_Set_Actor_Speed(actor, 0, 0, 0, 0);
		xactor_Set_Actor_State(actor, 0, 0);
		xactor_Set_Actor_State_Speed(actor, 0, 0);
		xactor_Set_Actor_Scale(actor, TRAIN_PRESENTATION_FULL_SCALE, TRAIN_PRESENTATION_FULL_SCALE);
		xactor_Hide_Actor(actor);
	}
	switch (g_trainingPresentationFrame) {
		case TRAIN_XWING_START:
			xactor_Show_Actor(actor);
			xactor_Set_Actor_State_Speed(actor, 1, 0);
			actor->state = 0;
			break;
		case TRAIN_XWING_INFO:
			xactor_Show_Actor(g_trainingShipInfoActor);
			xactor_Set_Actor_State_Speed(actor, 0, 0);
			g_trainingShipInfoActor->var1 = TRAIN_SHIP_INFO_SHRINK;
			g_trainingShipInfoActor->var2 = 0;
			g_trainingDisplayedShipActor = actor;
			break;
		case TRAIN_XWING_RESUME:
			xactor_Set_Actor_State_Speed(actor, 1, 0);
			xactor_Hide_Actor(g_trainingShipInfoActor);
			g_trainingShipInfoActor->var1 = TRAIN_SHIP_INFO_IDLE;
			break;
	}
}

// FUNCTION: XW 0x466600
void train_user_AWingPresentation(Actor* actor, int time) {
	(void)time;
	if ((g_trainingPresentationFrame < TRAIN_AWING_START || g_trainingPresentationFrame >= TRAIN_AWING_END) &&
		(uint16_t)xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Set_Actor_Pos(actor, TRAIN_PRESENTATION_RESET_X, TRAIN_AWING_RESET_Y, 0, 0);
		xactor_Set_Actor_Speed(actor, 0, 0, 0, 0);
		xactor_Set_Actor_State(actor, 0, 0);
		xactor_Set_Actor_State_Speed(actor, 0, 0);
		xactor_Set_Actor_Scale(actor, TRAIN_PRESENTATION_FULL_SCALE, TRAIN_PRESENTATION_FULL_SCALE);
		xactor_Hide_Actor(actor);
	}
	switch (g_trainingPresentationFrame) {
		case TRAIN_AWING_START:
			xactor_Show_Actor(actor);
			xactor_Set_Actor_State_Speed(actor, 1, 0);
			actor->state = 0;
			break;
		case TRAIN_AWING_INFO:
			xactor_Show_Actor(g_trainingShipInfoActor);
			xactor_Set_Actor_State_Speed(actor, 0, 0);
			g_trainingShipInfoActor->var1 = TRAIN_SHIP_INFO_SHRINK;
			g_trainingShipInfoActor->var2 = 0;
			g_trainingDisplayedShipActor = actor;
			break;
		case TRAIN_AWING_RESUME:
			xactor_Set_Actor_State_Speed(actor, 1, 0);
			xactor_Hide_Actor(g_trainingShipInfoActor);
			g_trainingShipInfoActor->var1 = TRAIN_SHIP_INFO_IDLE;
			break;
	}
}

// FUNCTION: XW 0x466720
void train_user_YWingPresentation(Actor* actor, int time) {
	(void)time;
	if ((g_trainingPresentationFrame < TRAIN_YWING_START || g_trainingPresentationFrame >= TRAIN_YWING_END) &&
		(uint16_t)xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Set_Actor_Pos(actor, TRAIN_YWING_RESET_X, TRAIN_YWING_RESET_Y, 0, 0);
		xactor_Set_Actor_Speed(actor, 0, 0, 0, 0);
		xactor_Set_Actor_State(actor, 0, 0);
		xactor_Set_Actor_State_Speed(actor, 0, 0);
		xactor_Set_Actor_Scale(actor, TRAIN_PRESENTATION_FULL_SCALE, TRAIN_PRESENTATION_FULL_SCALE);
		xactor_Hide_Actor(actor);
	}
	switch (g_trainingPresentationFrame) {
		case TRAIN_YWING_START:
			xactor_Show_Actor(actor);
			xactor_Set_Actor_State_Speed(actor, 1, 0);
			actor->state = 0;
			break;
		case TRAIN_YWING_INFO:
			xactor_Show_Actor(g_trainingShipInfoActor);
			xactor_Set_Actor_State_Speed(actor, 0, 0);
			g_trainingShipInfoActor->var1 = TRAIN_SHIP_INFO_SHRINK;
			g_trainingShipInfoActor->var2 = 0;
			g_trainingDisplayedShipActor = actor;
			break;
		case TRAIN_YWING_RESUME:
			xactor_Set_Actor_State_Speed(actor, 1, 0);
			xactor_Hide_Actor(g_trainingShipInfoActor);
			g_trainingShipInfoActor->var1 = TRAIN_SHIP_INFO_IDLE;
			break;
	}
}

// FUNCTION: XW 0x466840
void train_user_BWingPresentation(Actor* actor, int time) {
	(void)time;
	if ((g_trainingPresentationFrame < TRAIN_BWING_START || g_trainingPresentationFrame >= TRAIN_BWING_END) &&
		(uint16_t)xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Set_Actor_Pos(actor, TRAIN_BWING_RESET_X, TRAIN_BWING_RESET_Y, 0, 0);
		xactor_Set_Actor_Speed(actor, 0, 0, 0, 0);
		xactor_Set_Actor_State(actor, 0, 0);
		xactor_Set_Actor_State_Speed(actor, 0, 0);
		xactor_Set_Actor_Scale(actor, TRAIN_PRESENTATION_FULL_SCALE, TRAIN_PRESENTATION_FULL_SCALE);
		xactor_Hide_Actor(actor);
	}
	switch (g_trainingPresentationFrame) {
		case TRAIN_BWING_START:
			xactor_Show_Actor(actor);
			xactor_Set_Actor_State_Speed(actor, 1, 0);
			actor->state = 0;
			break;
		case TRAIN_BWING_INFO:
			xactor_Show_Actor(g_trainingShipInfoActor);
			xactor_Set_Actor_State_Speed(actor, 0, 0);
			g_trainingShipInfoActor->var1 = TRAIN_SHIP_INFO_SHRINK;
			g_trainingShipInfoActor->var2 = 0;
			g_trainingDisplayedShipActor = actor;
			break;
		case TRAIN_BWING_RESUME:
			xactor_Set_Actor_State_Speed(actor, 1, 0);
			xactor_Hide_Actor(g_trainingShipInfoActor);
			g_trainingShipInfoActor->var1 = TRAIN_SHIP_INFO_IDLE;
			break;
	}
}

// FUNCTION: XW 0x466960
void train_user_Stars(Actor* actor, int time) {
	(void)time;
	actor->x += TRAIN_STAR_SCROLL_STEP;
	if (actor->x >= TRAIN_STAR_VIEW_WIDTH) {
		actor->x -= TRAIN_STAR_WRAP_WIDTH;
	}
}

// FUNCTION: XW 0x466980
void train_user_Door(Actor* actor, int time) {
	int16_t needsRefresh = 1;
	(void)time;
	if (actor->var1 == 0) {
		if (actor->state == 1) {
			train_HandleSoundAction(TRAIN_SOUND_DOOR_CLOSE);
		}
		if (actor->state != 0) {
			xactor_Set_Actor_State(actor, actor->state - 1, 0);
		} else {
			needsRefresh = 0;
		}
	} else {
		if (actor->state == 0) {
			train_HandleSoundAction(TRAIN_SOUND_DOOR_OPEN);
		}
		if (actor->state != actor->arraySize - 1) {
			xactor_Set_Actor_State(actor, actor->state + 1, 0);
		} else {
			needsRefresh = 0;
		}
		actor->var1 = 0;
	}
	if (needsRefresh != 0) {
		xactor_Refresh_Actor(actor);
	}
}

// FUNCTION: XW 0x466A10
void train_UpdateShipInfoAnimation(Actor* actor) {
	switch (actor->var1) {
		case TRAIN_SHIP_INFO_SHRINK:
			if (actor->var2 < TRAIN_SHIP_INFO_SCALE_PROGRESS_END) {
				actor->var2 += TRAIN_SHIP_INFO_SCALE_PROGRESS_STEP;
				g_trainingDisplayedShipActor->xscale -= TRAIN_SHIP_INFO_SCALE_STEP;
				g_trainingDisplayedShipActor->yscale -= TRAIN_SHIP_INFO_SCALE_STEP;
				g_trainingDisplayedShipActor->x -= g_trainingDisplayedShipActor->var1;
			} else {
				actor->var1 = TRAIN_SHIP_INFO_TEXT;
				actor->var2 = 0;
			}
			break;
		case TRAIN_SHIP_INFO_TEXT:
			if (actor->var2 < TRAIN_SHIP_INFO_TEXT_PROGRESS_END) {
				actor->var2 += TRAIN_SHIP_INFO_TEXT_PROGRESS_STEP;
			} else {
				actor->var1 = TRAIN_SHIP_INFO_EXPAND;
				actor->var2 = 0;
			}
			break;
		case TRAIN_SHIP_INFO_EXPAND:
			if (actor->var2 < TRAIN_SHIP_INFO_SCALE_PROGRESS_END) {
				actor->var2 += TRAIN_SHIP_INFO_SCALE_PROGRESS_STEP;
				g_trainingDisplayedShipActor->xscale += TRAIN_SHIP_INFO_SCALE_STEP;
				g_trainingDisplayedShipActor->yscale += TRAIN_SHIP_INFO_SCALE_STEP;
				g_trainingDisplayedShipActor->x += g_trainingDisplayedShipActor->var1;
			} else {
				actor->var1 = TRAIN_SHIP_INFO_COMPLETE;
				actor->var2 = 0;
			}
			break;
	}
}

// FUNCTION: XW 0x466AD0
void train_user_ShipInfo(Actor* actor, int time) {
	Rect viewFrame;
	if (g_trainingPresentationFrame == 0) {
		xactor_Hide_Actor(actor);
		actor->var1 = 0;
	}
	if (time > 0 && time <= TRAIN_SHIP_INFO_REFRESH_LAST_FRAME) {
		xview_Get_View_Frame(TRAIN_SCREEN_VIEW, &viewFrame);
		xview_Set_View_Frame(TRAIN_SCREEN_VIEW, &viewFrame);
		xactor_Refresh_Actor(g_trainingScreenActor);
	}
}

// FUNCTION: XW 0x466B30
int16_t train_DrawShipInfoPanel(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
								int16_t refresh) {
	Rect panelRect;
	(void)refresh;
	switch (actor->var1) {
		case TRAIN_SHIP_INFO_SHRINK:
			xrect_Set_Rect(&panelRect, TRAIN_INFO_PANEL_RIGHT - actor->var2, TRAIN_INFO_PANEL_TOP,
						   TRAIN_INFO_PANEL_RIGHT, TRAIN_INFO_PANEL_BOTTOM);
			xpaint_Paint_Clipped_Rect(&panelRect, TRAIN_INFO_PANEL_COLOR);
			xrect_Set_Rect(&panelRect, TRAIN_INFO_PANEL_LEFT,
						   TRAIN_INFO_PANEL_BOTTOM - (actor->var2 >> TRAIN_INFO_PANEL_VERTICAL_SHIFT),
						   TRAIN_INFO_PANEL_RIGHT, TRAIN_INFO_PANEL_BOTTOM);
			xpaint_Paint_Clipped_Rect(&panelRect, TRAIN_INFO_PANEL_COLOR);
			break;
		case TRAIN_SHIP_INFO_TEXT:
			xrect_Set_Rect(&panelRect, TRAIN_INFO_PANEL_RIGHT_BAND_LEFT, TRAIN_INFO_PANEL_TOP,
						   TRAIN_INFO_PANEL_RIGHT, TRAIN_INFO_PANEL_BOTTOM);
			xpaint_Paint_Clipped_Rect(&panelRect, TRAIN_INFO_PANEL_COLOR);
			xrect_Set_Rect(&panelRect, TRAIN_INFO_PANEL_LEFT, TRAIN_INFO_PANEL_BOTTOM_BAND_TOP,
						   TRAIN_INFO_PANEL_RIGHT, TRAIN_INFO_PANEL_BOTTOM);
			xpaint_Paint_Clipped_Rect(&panelRect, TRAIN_INFO_PANEL_COLOR);
			if (g_trainingDisplayedShipActor == g_trainingAWingActor)
				train_DrawAWingSpecifications(actor, frame, clip, x, y);
			else if (g_trainingDisplayedShipActor == g_trainingYWingActor)
				train_DrawYWingSpecifications(actor, frame, clip, x, y);
			else if (g_trainingDisplayedShipActor == g_trainingXWingActor)
				train_DrawXWingSpecifications(actor, frame, clip, x, y);
			else
				train_DrawBWingSpecifications(actor, frame, clip, x, y);
			break;
		case TRAIN_SHIP_INFO_EXPAND:
			xrect_Set_Rect(&panelRect, actor->var2 + TRAIN_INFO_PANEL_RIGHT_BAND_LEFT, TRAIN_INFO_PANEL_TOP,
						   TRAIN_INFO_PANEL_RIGHT, TRAIN_INFO_PANEL_BOTTOM);
			xpaint_Paint_Clipped_Rect(&panelRect, TRAIN_INFO_PANEL_COLOR);
			xrect_Set_Rect(&panelRect, TRAIN_INFO_PANEL_LEFT,
						   (actor->var2 >> TRAIN_INFO_PANEL_VERTICAL_SHIFT) +
							   TRAIN_INFO_PANEL_BOTTOM_BAND_TOP,
						   TRAIN_INFO_PANEL_RIGHT, TRAIN_INFO_PANEL_BOTTOM);
			xpaint_Paint_Clipped_Rect(&panelRect, TRAIN_INFO_PANEL_COLOR);
			break;
	}
	return 1;
}

// FUNCTION: XW 0x466D30
void train_DrawAWingSpecifications(Actor* infoActor, Rect* frame, Rect* clip, int16_t x, int16_t y) {
	int16_t logoReveal;
	int16_t logoOffset;
	(void)x;
	(void)y;
	textext_Draw_Typewriter_Line("Speed: 120 MGLT", TRAIN_SPEC_FONT, TRAIN_SPEC_X, TRAIN_SPEC_SPEED_Y,
								 infoActor->var2);
	if (infoActor->var2 >= TRAIN_SPEC_FIRST_DELAY)
		textext_Draw_Typewriter_Line("Shields/Hull: 50 SBD/15 RU", TRAIN_SPEC_FONT, TRAIN_SPEC_X,
									 TRAIN_SPEC_PROTECTION_Y, infoActor->var2 - TRAIN_SPEC_FIRST_DELAY);
	if (infoActor->var2 >= TRAIN_SPEC_SECOND_DELAY)
		textext_Draw_Typewriter_Line("Lasers and Concussion Missiles", TRAIN_SPEC_FONT, TRAIN_SPEC_X,
									 TRAIN_SPEC_WEAPONS_Y, infoActor->var2 - TRAIN_SPEC_SECOND_DELAY);
	if (infoActor->var2 >= TRAIN_SPEC_FIRST_DELAY)
		textext_Draw_Typewriter_Line("A-WING", TRAIN_SPEC_NAME_FONT, TRAIN_SPEC_NAME_X, TRAIN_SPEC_NAME_Y,
									 infoActor->var2 - TRAIN_SPEC_FIRST_DELAY);
	if (infoActor->var2 >= TRAIN_SPEC_SECOND_DELAY)
		textext_Draw_Typewriter_Line("Fighter", TRAIN_SPEC_NAME_FONT, TRAIN_SPEC_NAME_X, TRAIN_SPEC_CLASS_Y,
									 infoActor->var2 - TRAIN_SPEC_SECOND_DELAY);
	logoReveal = infoActor->var2;
	logoOffset = TRAIN_SPEC_LOGO_STEP * logoReveal;
	if (logoReveal >= TRAIN_SPEC_SECOND_DELAY)
		logoOffset = TRAIN_SPEC_LOGO_MAX_OFFSET;
	xactdelt_Draw_Delta_Actor(g_trainingRebelLogoActor, frame, clip,
							  TRAIN_SPEC_AWING_LOGO_START_X - logoOffset, TRAIN_SPEC_LOGO_Y, 1);
}

// FUNCTION: XW 0x466E30
void train_DrawXWingSpecifications(Actor* infoActor, Rect* frame, Rect* clip, int16_t x, int16_t y) {
	int16_t logoReveal;
	int16_t logoOffset;
	(void)x;
	(void)y;
	textext_Draw_Typewriter_Line("Speed: 100 MGLT", TRAIN_SPEC_FONT, TRAIN_SPEC_X, TRAIN_SPEC_SPEED_Y,
								 infoActor->var2);
	if (infoActor->var2 >= TRAIN_SPEC_FIRST_DELAY)
		textext_Draw_Typewriter_Line("Shields/Hull: 50 SBD/20 RU", TRAIN_SPEC_FONT, TRAIN_SPEC_X,
									 TRAIN_SPEC_PROTECTION_Y, infoActor->var2 - TRAIN_SPEC_FIRST_DELAY);
	if (infoActor->var2 >= TRAIN_SPEC_SECOND_DELAY)
		textext_Draw_Typewriter_Line("Lasers and Proton Torpedoes", TRAIN_SPEC_FONT, TRAIN_SPEC_X,
									 TRAIN_SPEC_WEAPONS_Y, infoActor->var2 - TRAIN_SPEC_SECOND_DELAY);
	if (infoActor->var2 >= TRAIN_SPEC_FIRST_DELAY)
		textext_Draw_Typewriter_Line("X-WING", TRAIN_SPEC_NAME_FONT, TRAIN_SPEC_NAME_X, TRAIN_SPEC_NAME_Y,
									 infoActor->var2 - TRAIN_SPEC_FIRST_DELAY);
	if (infoActor->var2 >= TRAIN_SPEC_SECOND_DELAY)
		textext_Draw_Typewriter_Line("Fighter", TRAIN_SPEC_NAME_FONT, TRAIN_SPEC_NAME_X, TRAIN_SPEC_CLASS_Y,
									 infoActor->var2 - TRAIN_SPEC_SECOND_DELAY);
	logoReveal = infoActor->var2;
	logoOffset = TRAIN_SPEC_LOGO_STEP * logoReveal;
	if (logoReveal >= TRAIN_SPEC_SECOND_DELAY)
		logoOffset = TRAIN_SPEC_LOGO_MAX_OFFSET;
	xactdelt_Draw_Delta_Actor(g_trainingIncomLogoActor, frame, clip, TRAIN_SPEC_LOGO_START_X - logoOffset,
							  TRAIN_SPEC_LOGO_Y, 1);
}

// FUNCTION: XW 0x466F30
void train_DrawYWingSpecifications(Actor* infoActor, Rect* frame, Rect* clip, int16_t x, int16_t y) {
	int16_t logoReveal;
	int16_t logoOffset;
	(void)x;
	(void)y;
	textext_Draw_Typewriter_Line("Speed: 80 MGLT", TRAIN_SPEC_FONT, TRAIN_SPEC_X, TRAIN_SPEC_SPEED_Y,
								 infoActor->var2);
	if (infoActor->var2 >= TRAIN_SPEC_FIRST_DELAY)
		textext_Draw_Typewriter_Line("Shields/Hull: 75 SBD/40 RU", TRAIN_SPEC_FONT, TRAIN_SPEC_X,
									 TRAIN_SPEC_PROTECTION_Y, infoActor->var2 - TRAIN_SPEC_FIRST_DELAY);
	if (infoActor->var2 >= TRAIN_SPEC_SECOND_DELAY)
		textext_Draw_Typewriter_Line("Lasers, Ion Cannons and Proton Torpedoes ", TRAIN_SPEC_FONT,
									 TRAIN_SPEC_X, TRAIN_SPEC_WEAPONS_Y,
									 infoActor->var2 - TRAIN_SPEC_SECOND_DELAY);
	if (infoActor->var2 >= TRAIN_SPEC_FIRST_DELAY)
		textext_Draw_Typewriter_Line("Y-WING", TRAIN_SPEC_NAME_FONT, TRAIN_SPEC_NAME_X, TRAIN_SPEC_NAME_Y,
									 infoActor->var2 - TRAIN_SPEC_FIRST_DELAY);
	if (infoActor->var2 >= TRAIN_SPEC_SECOND_DELAY)
		textext_Draw_Typewriter_Line("Bomber", TRAIN_SPEC_NAME_FONT, TRAIN_SPEC_NAME_X, TRAIN_SPEC_CLASS_Y,
									 infoActor->var2 - TRAIN_SPEC_SECOND_DELAY);
	logoReveal = infoActor->var2;
	logoOffset = TRAIN_SPEC_LOGO_STEP * logoReveal;
	if (logoReveal >= TRAIN_SPEC_SECOND_DELAY)
		logoOffset = TRAIN_SPEC_LOGO_MAX_OFFSET;
	xactdelt_Draw_Delta_Actor(g_trainingIncomLogoActor, frame, clip, TRAIN_SPEC_LOGO_START_X - logoOffset,
							  TRAIN_SPEC_LOGO_Y, 1);
}

// FUNCTION: XW 0x467030
void train_DrawBWingSpecifications(Actor* infoActor, Rect* frame, Rect* clip, int16_t x, int16_t y) {
	int16_t logoReveal;
	int16_t logoOffset;
	(void)x;
	(void)y;
	textext_Draw_Typewriter_Line("Speed:  90 MGLT", TRAIN_SPEC_FONT, TRAIN_SPEC_X, TRAIN_SPEC_SPEED_Y,
								 infoActor->var2);
	if (infoActor->var2 >= TRAIN_SPEC_FIRST_DELAY)
		textext_Draw_Typewriter_Line("Shields/Hull:  125 SBD/60 RU", TRAIN_SPEC_FONT, TRAIN_SPEC_X,
									 TRAIN_SPEC_PROTECTION_Y, infoActor->var2 - TRAIN_SPEC_FIRST_DELAY);
	if (infoActor->var2 >= TRAIN_SPEC_SECOND_DELAY)
		textext_Draw_Typewriter_Line("Lasers, Ion Cannons and Proton Torpedoes ", TRAIN_SPEC_FONT,
									 TRAIN_SPEC_X, TRAIN_SPEC_WEAPONS_Y,
									 infoActor->var2 - TRAIN_SPEC_SECOND_DELAY);
	if (infoActor->var2 >= TRAIN_SPEC_FIRST_DELAY)
		textext_Draw_Typewriter_Line("B-Wing", TRAIN_SPEC_NAME_FONT, TRAIN_SPEC_NAME_X, TRAIN_SPEC_NAME_Y,
									 infoActor->var2 - TRAIN_SPEC_FIRST_DELAY);
	if (infoActor->var2 >= TRAIN_SPEC_SECOND_DELAY)
		textext_Draw_Typewriter_Line("Fighter", TRAIN_SPEC_NAME_FONT, TRAIN_SPEC_NAME_X, TRAIN_SPEC_CLASS_Y,
									 infoActor->var2 - TRAIN_SPEC_SECOND_DELAY);
	logoReveal = infoActor->var2;
	logoOffset = TRAIN_SPEC_LOGO_STEP * logoReveal;
	if (logoReveal >= TRAIN_SPEC_SECOND_DELAY)
		logoOffset = TRAIN_SPEC_LOGO_MAX_OFFSET;
	xactdelt_Draw_Delta_Actor(g_trainingRebelLogoActor, frame, clip, TRAIN_SPEC_LOGO_START_X - logoOffset,
							  TRAIN_SPEC_LOGO_Y, 1);
}

// FUNCTION: XW 0x467130
void train_user_MissionText(Actor* actor, int time) {
	(void)time;
	if (g_trainingPresentationFrame != 0 && g_trainingPresentationFrame >= TRAIN_MISSION_TEXT_START &&
		g_trainingPresentationFrame < TRAIN_MISSION_TEXT_END) {
		if (actor->var1 != 0) {
			++actor->var2;
		} else {
			xactor_Show_Actor(actor);
			actor->var1 = 1;
			actor->var2 = 0;
		}
	} else if (xactor_Is_Actor_Visible(actor)) {
		xactor_Hide_Actor(actor);
		actor->var1 = 0;
	}
}

// FUNCTION: XW 0x4671A0
int16_t train_Draw_Train_Screen_Mission(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
										int16_t refresh) {
	(void)clip;
	(void)x;
	(void)y;
	if (refresh != 0) {
		Rect headingRect;
		char lineText[TRAIN_MISSION_TEXT_CAPACITY];
		char levelLabel[TRAIN_MISSION_TEXT_CAPACITY];
		int16_t revealTicks;
		int16_t boldColor;
		xrect_Copy_Rect(&headingRect, frame);
		headingRect.top += TRAIN_MISSION_HEADING_INSET;
		headingRect.bottom = headingRect.top + TRAIN_MISSION_HEADING_HEIGHT;
		strcpy(lineText, "Rebel Proving Ground");
		xfont_Print_Centered_Text(lineText, &headingRect, TRAIN_MISSION_FONT, TRAIN_MISSION_TEXT_COLOR);
		xrect_Offset_Rect(&headingRect, 0, TRAIN_MISSION_LINE_SPACING);
		strcpy(lineText, g_trainingShipNames[shipext_Get_Train_Ship()]);
		xparagrp_Get_Paragraph_String(g_trainingLevelParagraph, levelLabel, TRAIN_MISSION_LEVEL_PARAGRAPH,
									  shipext_Get_Train_Level());
		strcat(lineText, " ");
		strcat(lineText, levelLabel);
		xfont_Print_Centered_Text(lineText, &headingRect, TRAIN_MISSION_FONT, TRAIN_MISSION_TEXT_COLOR);
		revealTicks = (int16_t)(actor->var2 - TRAIN_MISSION_REVEAL_DELAY);
		xfont_Set_FontID_Bold_Color(TRAIN_MISSION_FONT, TRAIN_MISSION_BOLD_COLOR);
		boldColor = xfont_Get_FontID_Bold_Color(TRAIN_MISSION_FONT);
		if (revealTicks > 0) {
			int16_t paragraphIndex = shipext_Get_Train_Level() + TRAIN_MISSION_BODY_PARAGRAPH_BASE;
			int16_t paragraphWidth;
			int16_t paragraphHeight;
			int16_t lineX;
			int16_t lineY;
			int16_t lineCount;
			int16_t lineIndex;
			xparagrp_Get_Paragraph_Size(g_trainingLevelParagraph, TRAIN_MISSION_FONT, paragraphIndex,
										&paragraphWidth, &paragraphHeight);
			lineX = (int16_t)(frame->left + ((frame->right - paragraphWidth - frame->left) >> 1));
			lineY = frame->top + TRAIN_MISSION_BODY_INSET;
			lineCount = xparagrp_Count_Paragraph_Strings(g_trainingLevelParagraph, paragraphIndex);
			for (lineIndex = 0; lineIndex < lineCount;) {
				xparagrp_Get_Paragraph_String(g_trainingLevelParagraph, lineText, paragraphIndex, lineIndex);
				train_DrawTypewriterLineWithSound(lineText, TRAIN_MISSION_FONT, lineX, lineY, revealTicks);
				++lineIndex;
				lineY += TRAIN_MISSION_LINE_SPACING;
				revealTicks = (int16_t)(revealTicks - (strlen(lineText) / TEXTEXT_CHARACTERS_PER_STEP));
				if (revealTicks <= 0) {
					break;
				}
			}
		}
		/* Original reads the bold color after setting it. */
		xfont_Set_FontID_Bold_Color(TRAIN_MISSION_FONT, boldColor);
	}
	return 1;
}

// FUNCTION: XW 0x467400
void train_user_HighScores(Actor* actor, int time) {
	(void)time;
	if (g_trainingPresentationFrame != 0 && g_trainingPresentationFrame >= TRAIN_HIGH_SCORES_START &&
		g_trainingPresentationFrame < TRAIN_HIGH_SCORES_END) {
		if (actor->var1 != 0) {
			actor->var2 += TRAIN_HIGH_SCORES_REVEAL_STEP;
		} else {
			xactor_Show_Actor(actor);
			actor->var1 = 1;
			actor->var2 = 0;
		}
	} else if (xactor_Is_Actor_Visible(actor)) {
		xactor_Hide_Actor(actor);
		actor->var1 = 0;
	}
}

// FUNCTION: XW 0x467470
int16_t train_Draw_Train_Screen_Score(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
									  int16_t refresh) {
	(void)clip;
	(void)x;
	(void)y;
	if (refresh != 0) {
		int16_t revealTicks = actor->var2;
		int16_t nameX = frame->left + TRAIN_SCORE_LEFT_INSET;
		int16_t rowY;
		int16_t scoreY;
		int16_t index;
		char scoreText[TRAIN_SCORE_TEXT_CAPACITY];
		if (revealTicks > TRAIN_SCORE_SCROLL_END)
			rowY = frame->top + TRAIN_SCORE_TOP_INSET;
		else
			rowY = frame->top - revealTicks + TRAIN_SCORE_SCROLL_ORIGIN;
		for (index = 0, scoreY = rowY + TRAIN_SCORE_TEXT_Y_OFFSET; index < TRAIN_SCORE_COUNT;
			 revealTicks -= TRAIN_SCORE_ROW_DELAY, rowY += TRAIN_SCORE_ROW_SPACING,
			scoreY += TRAIN_SCORE_ROW_SPACING, ++index) {
			int16_t nameColor;
			if (revealTicks < 0)
				break;
			nameColor = revealTicks + TRAIN_SCORE_COLOR_START;
			if (revealTicks > TRAIN_SCORE_FADE_END)
				nameColor = TRAIN_SCORE_COLOR_END;
			if (g_trainingScoreNames[index][0] != '\0') {
				int16_t nameColumnWidth;
				xfont_Print_Clipped_Text(g_trainingScoreNames[index], nameX, rowY, TRAIN_SCORE_NAME_FONT,
										 nameColor);
				nameColumnWidth = xfont_Get_String_Width_0(TRAIN_SCORE_NAME_FONT, "abcdefghijklmnop  ");
				sprintf(scoreText, "Score %6ld    Level %u", (long)g_trainingScorePoints[index],
						(unsigned int)g_trainingScoreLevels[index]);
				textext_Draw_Typewriter_Line(scoreText, TRAIN_SCORE_TEXT_FONT, nameX + nameColumnWidth,
											 scoreY, revealTicks);
			}
		}
	}
	return 1;
}

// FUNCTION: XW 0x467590
void train_user_Welcome(Actor* actor, int time) {
	(void)time;
	if (g_trainingPresentationFrame != 0 && g_trainingPresentationFrame >= TRAIN_WELCOME_START &&
		g_trainingPresentationFrame < TRAIN_WELCOME_END) {
		if (actor->var1 != 0) {
			actor->var2 += TRAIN_WELCOME_REVEAL_STEP;
		} else {
			xactor_Show_Actor(actor);
			actor->var1 = 1;
			actor->var2 = 0;
		}
	} else if (xactor_Is_Actor_Visible(actor)) {
		xactor_Hide_Actor(actor);
		actor->var1 = 0;
	}
}

// FUNCTION: XW 0x467600
int16_t train_DrawWelcomeText(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	(void)clip;
	(void)x;
	(void)y;
	if (refresh != 0) {
		int16_t revealBudget = actor->var2;
		int16_t drawX = frame->left + TRAIN_WELCOME_LEFT_INSET;
		int16_t drawY;
		int16_t lineIndex;
		int16_t lineLength;
		if (revealBudget > TRAIN_WELCOME_SCROLL_END)
			drawY = frame->top + TRAIN_WELCOME_TOP_INSET;
		else
			drawY = frame->top - revealBudget + TRAIN_WELCOME_SCROLL_ORIGIN;
		for (lineIndex = 0;
			 (lineLength = (int16_t)strlen(g_trainingWelcomeLines[lineIndex])) != 0 && revealBudget >= 0;
			 ++lineIndex, revealBudget -= lineLength >> TRAIN_WELCOME_LENGTH_SHIFT) {
			train_DrawTypewriterLineWithSound(g_trainingWelcomeLines[lineIndex], TRAIN_WELCOME_FONT, drawX,
											  drawY, revealBudget);
			drawY += TRAIN_WELCOME_LINE_SPACING;
		}
	}
	return 1;
}

// FUNCTION: XW 0x4676D0
void train_DrawTypewriterLineWithSound(const char* text, uint16_t fontId, int16_t x, int16_t y,
									   int16_t revealTicks) {
	if (revealTicks >= 0) {
		if (strlen(text) > (size_t)(TEXTEXT_CHARACTERS_PER_STEP * revealTicks)) {
			if (text[TEXTEXT_CHARACTERS_PER_STEP * revealTicks] == ' ' ||
				text[TEXTEXT_CHARACTERS_PER_STEP * revealTicks + 1] == ' ')
				train_HandleSoundAction(TRAIN_SOUND_TYPING_STOP);
			else
				train_HandleSoundAction(TRAIN_SOUND_TYPING_START);
		}
		textext_Draw_Typewriter_Line(text, fontId, x, y, TEXTEXT_CHARACTERS_PER_STEP * revealTicks);
	}
}

// FUNCTION: XW 0x467730
void train_RestartMissionText(void) {
	g_trainingPresentationFrame = TRAIN_MISSION_TEXT_START;
	if (xactor_Is_Actor_Visible(g_trainingMissionTextActor) != 0) {
		g_trainingMissionTextActor->var1 = 1;
		g_trainingMissionTextActor->var2 = TRAIN_MISSION_TEXT_REVEAL_ALL;
	}
	if (xactor_Is_Actor_Visible(g_trainingShipInfoActor) != 0) {
		xactor_Hide_Actor(g_trainingShipInfoActor);
		g_trainingShipInfoActor->var1 = 0;
		g_trainingShipInfoActor->var2 = 0;
	}
}

// FUNCTION: XW 0x4677A0
void train_LoadPresentationShipAtFrame(int presentationFrame) {
	Rect canvasRect;
	if (Shared_ReturnZero() != 0) {
		Actor* xwingActor = g_trainingXWingActor;
		if (g_trainingAWingActor == NULL || xwingActor == NULL || g_trainingYWingActor == NULL ||
			g_trainingBWingActor == NULL) {
			int16_t loadFrame = 0;
			int16_t shipIndex;
			if (presentationFrame == TRAIN_AWING_START - 1) {
				shipIndex = TRAIN_PRESENTATION_AWING;
				loadFrame = 1;
			} else {
				shipIndex = (int16_t)presentationFrame;
			}
			if (presentationFrame == TRAIN_XWING_START - 1) {
				shipIndex = TRAIN_PRESENTATION_XWING;
				loadFrame = 1;
			}
			if (presentationFrame == TRAIN_YWING_START - 1) {
				shipIndex = TRAIN_PRESENTATION_YWING;
				loadFrame = 1;
			}
			if (presentationFrame == TRAIN_BWING_START - 1) {
				shipIndex = TRAIN_PRESENTATION_BWING;
				loadFrame = 1;
			}
			if (loadFrame != 0) {
				if (g_trainingAWingActor != NULL) {
					xactor_Free_Actor_From_System(g_trainingAWingActor);
					xactor_Free_Actor(g_trainingAWingActor);
					xwingActor = g_trainingXWingActor;
					g_trainingAWingActor = NULL;
				}
				if (xwingActor != NULL) {
					xactor_Free_Actor_From_System(xwingActor);
					xactor_Free_Actor(g_trainingXWingActor);
					g_trainingXWingActor = NULL;
				}
				if (g_trainingYWingActor != NULL) {
					xactor_Free_Actor_From_System(g_trainingYWingActor);
					xactor_Free_Actor(g_trainingYWingActor);
					g_trainingYWingActor = NULL;
				}
				if (g_trainingBWingActor != NULL) {
					xactor_Free_Actor_From_System(g_trainingBWingActor);
					xactor_Free_Actor(g_trainingBWingActor);
					g_trainingBWingActor = NULL;
				}
				xrect_Set_Rect(&canvasRect, 0, 0, TRAIN_STAR_VIEW_WIDTH, TRAIN_PRESENTATION_VIEW_HEIGHT);
				switch (shipIndex) {
					case TRAIN_PRESENTATION_AWING:
						g_trainingAWingActor = xactanim_Res_Anim_Actor(
							g_trainingResourceFile, "a_wing", &canvasRect, TRAIN_PRESENTATION_RESET_X,
							TRAIN_AWING_RESET_Y, TRAIN_PRESENTATION_DEPTH);
						xactor_Set_Actor_User_Function(g_trainingAWingActor, train_user_AWingPresentation);
						xactor_Start_Actor(g_trainingAWingActor);
						g_trainingAWingActor->var1 = TRAIN_AWING_TYPE_MARKER;
						break;
					case TRAIN_PRESENTATION_XWING:
						g_trainingXWingActor = xactanim_Res_Anim_Actor(
							g_trainingResourceFile, "x_wing", &canvasRect, TRAIN_PRESENTATION_RESET_X,
							TRAIN_PRESENTATION_RESET_Y, TRAIN_PRESENTATION_DEPTH);
						xactor_Set_Actor_User_Function(g_trainingXWingActor, train_user_XWingPresentation);
						xactor_Start_Actor(g_trainingXWingActor);
						g_trainingXWingActor->var1 = TRAIN_XWING_TYPE_MARKER;
						break;
					case TRAIN_PRESENTATION_YWING:
						g_trainingYWingActor = xactanim_Res_Anim_Actor(
							g_trainingResourceFile, "y-wing", &canvasRect, TRAIN_YWING_RESET_X,
							TRAIN_YWING_RESET_Y, TRAIN_PRESENTATION_DEPTH);
						xactor_Set_Actor_User_Function(g_trainingYWingActor, train_user_YWingPresentation);
						xactor_Start_Actor(g_trainingYWingActor);
						g_trainingYWingActor->var1 = TRAIN_YWING_TYPE_MARKER;
						break;
					case TRAIN_PRESENTATION_BWING:
						g_trainingBWingActor = xactanim_Res_Anim_Actor(
							g_trainingResourceFile, "b_wing", &canvasRect, TRAIN_BWING_RESET_X,
							TRAIN_BWING_RESET_Y, TRAIN_PRESENTATION_DEPTH);
						xactor_Set_Actor_User_Function(g_trainingBWingActor, train_user_BWingPresentation);
						xactor_Start_Actor(g_trainingBWingActor);
						g_trainingBWingActor->var1 = TRAIN_BWING_TYPE_MARKER;
						break;
				}
			}
		}
	}
}

// FUNCTION: XW 0x467A60
int16_t train_LoadPilotProgress(const char* pilotName) {
	char pilotPath[TRAIN_PILOT_PATH_CAPACITY];
	LandruFile* pilotFile;
	strcpy(pilotPath, pilotName);
	strcat(pilotPath, ".PLT");
	pilotFile = xfile_Open_File(LANDRU_FILE_ROOT_USER, pilotPath, "rb");
	if (pilotFile != NULL) {
		register_ReadPilotRecord(pilotFile, &g_trainingPilot);
		xfile_Close_File(pilotFile);
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x467B10
void train_OpenMusic(ResFile* unusedResourceFile, Film* film, int* presentationFrame) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_trainingMusic.sound = xsound_Find_Gmid("rebels");
		g_trainingMusic.film = film;
		g_trainingMusic.presentationFrame = presentationFrame;
		if (g_trainingMusic.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource(":X-Wing Data\\RESOURCE\\trmusic.lfd");
			if (musicResource == NULL)
				musicResource = xres_Open_Resource("trmusic.lfd");
			g_trainingMusic.sound = xsound_Res_Music(musicResource, "rebels");
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_trainingMusic.sound);
			if (g_trainingMusic.openCount > TRAIN_MUSIC_INITIAL_LOAD_LIMIT)
				soundext_SetHook(g_trainingMusic.sound, XW_SOUND_CONTROL_DIRECT,
								 TRAIN_MUSIC_REPEAT_LOAD_CONTROL, 0);
			else
				soundext_ScanMidi(g_trainingMusic.sound, 0, TRAIN_MUSIC_INITIAL_LOAD_BEAT, 0);
		}
		soundext_FadeVolume(g_trainingMusic.sound, TRAIN_MUSIC_OPEN_VOLUME, TRAIN_MUSIC_CLOSE_FADE_DURATION);
		xsound_Set_Sound_Keep(g_trainingMusic.sound);
		xsound_Set_Sound_User_Function(g_trainingMusic.sound, train_user_Music);
		++g_trainingMusic.openCount;
	}
}

// FUNCTION: XW 0x467C10
void train_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0) {
		switch (xerror_Get_Landru_Exit()) {
			case XW_SCENE_TRAINING_RETURN_SHUTTLE: {
				int tick = soundext_GetMusicParam(g_trainingMusic.sound, XW_SOUND_QUERY_TICK, 0);
				soundext_JumpMidi(g_trainingMusic.sound, TRAIN_MUSIC_RETURN_GROUP, TRAIN_MUSIC_RETURN_BEAT,
								  tick);
				soundext_FadeVolume(g_trainingMusic.sound, 0, TRAIN_MUSIC_RETURN_FADE_OUT_DURATION);
				soundext_FadeVolume(g_trainingMusic.sound, TRAIN_MUSIC_RETURN_VOLUME,
									TRAIN_MUSIC_RETURN_FADE_IN_DURATION);
				break;
			}
			case XW_SCENE_LEGACY_TRAINING_TRANSITION_260:
			case XW_SCENE_LEGACY_TRAINING_TRANSITION_261:
			case XW_SCENE_LEGACY_TRAINING_TRANSITION_262:
			case XW_SCENE_LEGACY_TRAINING_TRANSITION_266:
				break;
			default:
				/* Resource pointers are outside the numeric flight-sound ID range. */
				soundext_SetPriority(0, 0);
				soundext_FadeVolume(g_trainingMusic.sound, 0, TRAIN_MUSIC_CLOSE_FADE_DURATION);
				break;
		}
		g_trainingMusic.phase = TRAIN_MUSIC_WAIT_FILM;
	}
}

// FUNCTION: XW 0x467DC0
void train_user_Music(Sound* sound, int time) {
	(void)sound;
	(void)time;
	switch (g_trainingMusic.phase) {
		case TRAIN_MUSIC_WAIT_FILM:
			if (g_trainingMusic.film->cur_cel == TRAIN_MUSIC_START_CEL) {
				soundext_ClearTriggers();
				if (g_trainingMusic.openCount > TRAIN_MUSIC_INITIAL_OPEN_LIMIT)
					soundext_SetHook(g_trainingMusic.sound, XW_SOUND_CONTROL_DIRECT,
									 TRAIN_MUSIC_REPEAT_CONTROL, 0);
				else
					soundext_SetHook(g_trainingMusic.sound, XW_SOUND_CONTROL_DIRECT,
									 TRAIN_MUSIC_INITIAL_CONTROL, 0);
				g_trainingMusic.phase = TRAIN_MUSIC_WAIT_FIRST_FRAME;
			}
			break;
		case TRAIN_MUSIC_WAIT_FIRST_FRAME:
			if (*g_trainingMusic.presentationFrame >= TRAIN_MUSIC_FIRST_FRAME) {
				soundext_SetHook(g_trainingMusic.sound, XW_SOUND_CONTROL_DIRECT,
								 TRAIN_MUSIC_PRESENTATION_CONTROL, 0);
				g_trainingMusic.phase = TRAIN_MUSIC_WAIT_LAST_FRAME;
			}
			break;
		case TRAIN_MUSIC_WAIT_LAST_FRAME:
			if (*g_trainingMusic.presentationFrame >= TRAIN_MUSIC_LAST_FRAME) {
				int beat = soundext_GetMusicParam(g_trainingMusic.sound, XW_SOUND_QUERY_BEAT, 0);
				if (beat > TRAIN_MUSIC_BEAT_THRESHOLD)
					soundext_SetHook(g_trainingMusic.sound, XW_SOUND_CONTROL_DIRECT,
									 TRAIN_MUSIC_LATE_BEAT_CONTROL, 0);
				else
					soundext_SetHook(g_trainingMusic.sound, XW_SOUND_CONTROL_DIRECT,
									 TRAIN_MUSIC_REPEAT_CONTROL, 0);
				g_trainingMusic.phase = TRAIN_MUSIC_COMPLETE;
			}
			break;
	}
}

// FUNCTION: XW 0x467EC0
void train_LoadSoundEffects(void) {
	if (ShellPreferences_GetSfxEnabled() != 0) {
		soundext_LoadSfx(XW_SHELL_SFX_R2_WHISTLE, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_R2_A, 0, NULL, 0, 0);
		soundext_LoadSfx(XW_SHELL_SFX_R2_B, 0, NULL, 0, 0);
		soundext_LoadSfx(XW_SHELL_SFX_R2_C, 0, NULL, 0, 0);
		soundext_LoadSfx(XW_SHELL_SFX_R2_D, 0, NULL, 0, 0);
		soundext_LoadSfx(XW_SHELL_SFX_DOOR_1A, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_TOUR_DOOR_CLOSE_1, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_HYDROL_3, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_TEXT_5, 0, NULL, 0, 1);
		g_trainingTypingSoundActive = 0;
	}
	ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x467F80
void train_HandleSoundAction(int16_t action) {
	if (ShellPreferences_GetSfxEnabled() != 0) {
		switch (action) {
			case TRAIN_SOUND_R2_WHISTLE:
				soundext_Play_SFX(XW_SHELL_SFX_R2_WHISTLE);
				break;
			case TRAIN_SOUND_R2_A:
				soundext_Play_SFX(XW_SHELL_SFX_R2_A);
				break;
			case TRAIN_SOUND_R2_B:
				soundext_Play_SFX(XW_SHELL_SFX_R2_B);
				break;
			case TRAIN_SOUND_R2_C:
				soundext_Play_SFX(XW_SHELL_SFX_R2_C);
				break;
			case TRAIN_SOUND_R2_D:
				soundext_Play_SFX(XW_SHELL_SFX_R2_D);
				break;
			case TRAIN_SOUND_DOOR_OPEN:
				soundext_Play_SFX(XW_SHELL_SFX_DOOR_1A);
				break;
			case TRAIN_SOUND_DOOR_CLOSE:
				soundext_Play_SFX(XW_SHELL_SFX_TOUR_DOOR_CLOSE_1);
				break;
			case TRAIN_SOUND_HYDRAULIC_START:
				soundext_Play_SFX(XW_SHELL_SFX_HYDROL_3);
				break;
			case TRAIN_SOUND_HYDRAULIC_STOP:
				soundext_Stop_SFX(XW_SHELL_SFX_HYDROL_3);
				break;
			case TRAIN_SOUND_TYPING_START:
				if (g_trainingTypingSoundActive == 0) {
					soundext_Play_SFX(XW_SHELL_SFX_TEXT_5);
					soundext_Fade_SFX(XW_SHELL_SFX_TEXT_5, TRAIN_SOUND_TYPING_VOLUME, 0);
					g_trainingTypingSoundActive = 1;
				}
				g_trainingTypingSoundActivity = 1;
				break;
			case TRAIN_SOUND_TYPING_STOP:
				if (g_trainingTypingSoundActive != 0) {
					soundext_Stop_SFX(XW_SHELL_SFX_TEXT_5);
					g_trainingTypingSoundActive = 0;
				}
				g_trainingTypingSoundActivity = 1;
				break;
			case TRAIN_SOUND_TYPING_UPDATE:
				if (g_trainingTypingSoundActivity == 0) {
					soundext_Stop_SFX(XW_SHELL_SFX_TEXT_5);
					g_trainingTypingSoundActive = 0;
				}
				g_trainingTypingSoundActivity = 0;
				break;
		}
	}
}
