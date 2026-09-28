#include "xw/frontend/debrief.h"

#include "xw/audio/frontend_audio.h"
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

// GLOBAL: XW 0x4D1E98
const char g_debriefAWingText[DEBRIEF_PROVING_LABEL_CAPACITY] = "A-Wing";

// GLOBAL: XW 0x4D1EC8
const char g_debriefXWingText[DEBRIEF_PROVING_LABEL_CAPACITY] = "X-Wing";

// GLOBAL: XW 0x4D1EF8
const char g_debriefYWingText[DEBRIEF_PROVING_LABEL_CAPACITY] = "Y-Wing";

// GLOBAL: XW 0x4D1F28
const char g_debriefStarfighterText[DEBRIEF_PROVING_LABEL_CAPACITY] = " Starfighter";

// GLOBAL: XW 0x4D1F58
const char g_debriefProvingGroundText[DEBRIEF_PROVING_LABEL_CAPACITY] = "Proving Ground";

// GLOBAL: XW 0x4D1F88
const char g_debriefRunText[DEBRIEF_PROVING_LABEL_CAPACITY] = " Run \002(Level";

// GLOBAL: XW 0x4D1FE8
const char g_debriefGatesPassedText[DEBRIEF_PROVING_LABEL_CAPACITY] = "Gates Passed: \002";

// GLOBAL: XW 0x4D2018
const char g_debriefGatesMissedText[DEBRIEF_PROVING_LABEL_CAPACITY] = "Gates Missed: \002";

// GLOBAL: XW 0x4D2048
const char g_debriefGatesRemainingText[DEBRIEF_PROVING_LABEL_CAPACITY] = "Gates Remaining: \002";

// GLOBAL: XW 0x4D2078
const char g_debriefRoundsFiredText[DEBRIEF_PROVING_LABEL_CAPACITY] = "Rounds Fired: \002";

// GLOBAL: XW 0x4D20A8
const char g_debriefTargetHitsText[DEBRIEF_PROVING_LABEL_CAPACITY] = "Target Hits: \002";

// GLOBAL: XW 0x4D2588
const char g_debriefScoreText[DEBRIEF_PROVING_LABEL_CAPACITY] = "Score: \002";

// GLOBAL: XW 0x4D2618
const char g_debriefBWingText[DEBRIEF_PROVING_LABEL_CAPACITY] = "B-Wing";

// GLOBAL: XW 0x4D2528
const char g_debriefTourHeaderPrefix[] = "TOD \002";

// GLOBAL: XW 0x4D2228
const char g_debriefSpacecraftKillsText[] = "Spacecraft Kills (Yours): \002";

// GLOBAL: XW 0x4D2DCC
const char g_debriefKillsCountFormat[] = "\002%d(%d)";

// GLOBAL: XW 0x4D2258
const char g_debriefSpacecraftLossesText[] = "Spacecraft Losses: \002";

// GLOBAL: XW 0x4D2DDC
const char g_debriefLossCountFormat[] = "\002%d";

// GLOBAL: XW 0x4D1DA8
int16_t g_debriefKeyboardFocusX[DEBRIEF_FOCUS_COUNT] = { 50, 152, 250, 314 };

// GLOBAL: XW 0x4D1DB0
int16_t g_debriefKeyboardFocusY[DEBRIEF_FOCUS_COUNT] = { 174, 174, 90, 90 };

// GLOBAL: XW 0x4D1DB8
const char g_debriefResourceStrings[DEBRIEF_RESOURCE_STRING_COUNT][DEBRIEF_RESOURCE_STRING_CAPACITY] = {
	"debrief.lfd",     "dbrief_3", "db_dor_l", "db_dor_r", "db_but_l",       "db_but_r",
	"Exit Debriefing", "dbr_base", " Group ",  "db_intr",  "Reenter Mission"
};

// GLOBAL: XW 0x4D2108
const char g_debriefWeaponShotLabels[DEBRIEF_WEAPON_KIND_COUNT][DEBRIEF_WEAPON_LABEL_CAPACITY] = {
	"Lasers Fired: \002", "Ion Blasts Fired: \002", "Homing Projectiles Fired: \002"
};

// GLOBAL: XW 0x4D2198
const char g_debriefTotalHitsText[DEBRIEF_WEAPON_LABEL_CAPACITY] = "Total Hits: \002";

// GLOBAL: XW 0x4D21C8
const char g_debriefSpacecraftHitsText[DEBRIEF_WEAPON_LABEL_CAPACITY] = "Spacecraft Hits: \002";

// GLOBAL: XW 0x4D21F8
const char g_debriefSurfaceHitsText[DEBRIEF_WEAPON_LABEL_CAPACITY] = "Surface Hits: \002";

// GLOBAL: XW 0x4D2288
const char g_debriefSpaceObjectKillsText[] = "Space Objects Destroyed: \002";

// GLOBAL: XW 0x4D22B8
const char g_debriefDeathStarBuildingKillsText[] = "Death Star Buildings Destroyed: \002";

// GLOBAL: XW 0x4D23A8
const char g_debriefObjectGoalPrefixes[DEBRIEF_GOAL_PREFIX_COUNT][DEBRIEF_GOAL_STRING_CAPACITY] = {
	"You did not Protect the ", "You did Protect the ", "You did not Destroy ", "You did Destroy the "
};

// GLOBAL: XW 0x4D2468
const char g_debriefObjectCategoryNames[DEBRIEF_OBJECT_CATEGORY_COUNT][DEBRIEF_GOAL_STRING_CAPACITY] = {
	"Mines.", "Sats.", "Probes."
};

// GLOBAL: XW 0x4D24F8
const char g_debriefPromotionText[] = "Promotion to \002";

// GLOBAL: XW 0x4D25B8
const char g_debriefSurfaceGoalNames[DEBRIEF_SURFACE_GOAL_CATEGORY_COUNT][DEBRIEF_GOAL_STRING_CAPACITY] = {
	"Exhaust Port.", "Laser Towers."
};

// GLOBAL: XW 0x4D2648
const char g_debriefRankNames[DEBRIEF_RANK_COUNT][DEBRIEF_RANK_NAME_CAPACITY] = {
	"Flt. Cadet.", "Flt. Officer.", "Lieutenant.", "Captain.", "Commander.", "General."
};

// GLOBAL: XW 0x4D26A0
const char g_debriefCraftCategoryNames[DEBRIEF_CRAFT_CATEGORY_COUNT][DEBRIEF_CRAFT_CATEGORY_NAME_CAPACITY] = {
	"TIE Fighter", "TIE Interceptor", "TIE Bomber", "TIE Advanced", "Assault Gunboat",
	"X-Wing",      "Y-Wing",          "A-Wing",     "Transport",    "Shuttle",
	"Tug",         "Container",       "Freighter",  "Calamari",     "Nebulon B",
	"Corvette",    "Star Destroyer",  "B-Wing",     "Comm Sat",     "Space Probe"
};

// GLOBAL: XW 0x4D2830
const char g_debriefGoalOutcomeText[DEBRIEF_GOAL_OUTCOME_COUNT][DEBRIEF_GOAL_OUTCOME_CAPACITY] = {
	"was Destroyed.",
	"Completed Mission.",
	"was Recovered.",
	"was Boarded.",
	"was Destroyed.",
	"Completed Mission.",
	"was Recovered.",
	"was Boarded.",
	"50% was Destroyed.",
	"50% Completed Mission.",
	"50% was Recovered.",
	"50% was Boarded.",
	"was Identified.",
	"was Identified.",
	"(50%) was Identified.",
	"Arrived.",
	"was not Destroyed.",
	"Failed Mission.",
	"was not Recovered.",
	"was not Boarded.",
	"was not Destroyed.",
	"Failed Mission.",
	"was not Recovered.",
	"was not Boarded.",
	"50% was not Destroyed.",
	"50% Failed Mission.",
	"50% was not Recovered.",
	"50% was not Boarded.",
	"was not Identified.",
	"was not Identified.",
	"(50%) was not Identified.",
	"did not Arrive."
};

// GLOBAL: XW 0x4D2C30
const int16_t g_debriefCraftCategoryMap[DEBRIEF_CRAFT_CATEGORY_MAP_COUNT] = { 5, 6,  7,  0,  1,  2,  4,  8,
																			  9, 10, 11, 12, 13, 14, 15, 16,
																			  3, 17, 17, 17, 17, 18, 18, 19 };

// GLOBAL: XW 0x4D2C60
char g_trainingHighScoreNames[DEBRIEF_HIGH_SCORE_COUNT][DEBRIEF_HIGH_SCORE_NAME_CAPACITY] = {
	"Luke", "Jon", "Larry", "Peter", "Bucky", "Jim", "Edward", "Wade"
};

// GLOBAL: XW 0x4D2D20
int g_trainingHighScorePoints[DEBRIEF_HIGH_SCORE_COUNT] = { 100, 100, 100, 100, 100, 100, 100, 100 };

// GLOBAL: XW 0x4D2D40
int16_t g_trainingHighScoreLevels[DEBRIEF_HIGH_SCORE_COUNT] = { 1, 1, 1, 1, 1, 1, 1, 1 };

// GLOBAL: XW 0x4F6A74
XwSceneMusicHandles g_debriefMusicState = { NULL, NULL };

// GLOBAL: XW 0x4F6A7C
int16_t g_debriefSound36Playing = 0;

// GLOBAL: XW 0x4F6A80
int16_t g_debriefSound36Activity = 0;

// GLOBAL: XW 0x4F6A98
int16_t g_debriefFlightGroupInitialStatus[DEBRIEF_FLIGHT_GROUP_CAPACITY] = { 0 };

// GLOBAL: XW 0x4F6AB8
int16_t g_debriefFlightGroupGoals[DEBRIEF_FLIGHT_GROUP_CAPACITY] = { 0 };

// GLOBAL: XW 0x4F6AD8
int16_t g_debriefLegacyWord = 0;

// GLOBAL: XW 0x4F6AE0
int16_t g_debriefFlightGroupCraftCounts[DEBRIEF_FLIGHT_GROUP_CAPACITY] = { 0 };

// GLOBAL: XW 0x4F6B00
REGISTER_PilotFileRecord g_debriefPilotRecord = { 0 };

// GLOBAL: XW 0x4F71B0
char g_debriefFlightGroupNames[DEBRIEF_FLIGHT_GROUP_CAPACITY][DEBRIEF_FLIGHT_GROUP_NAME_CAPACITY] = { { 0 } };

// GLOBAL: XW 0x4F72B0
int16_t g_debriefPageCount = 0;

// GLOBAL: XW 0x4F72B4
Actor* g_debriefBackgroundActor = NULL;

// GLOBAL: XW 0x4F72B8
Film* g_debriefFilm = NULL;

// GLOBAL: XW 0x4F72BC
uint16_t g_debriefMissionSurfaceWord = 0;

// GLOBAL: XW 0x4F72C0
int16_t g_debriefFlightGroupCount = 0;

// GLOBAL: XW 0x4F72C4
Actor* g_debriefInteriorActor = NULL;

// GLOBAL: XW 0x4F72CC
Actor* g_debriefNextPageActor = NULL;

// GLOBAL: XW 0x4F72D0
Input* g_debriefRootInput = NULL;

// GLOBAL: XW 0x4F72D4
Input* g_debriefHoverLabelInput = NULL;

// GLOBAL: XW 0x4F72D8
int16_t g_debriefPageIndex = 0;

// GLOBAL: XW 0x4F72DC
Actor* g_debriefBaseActor = NULL;

// GLOBAL: XW 0x4F72E0
int16_t g_debriefFlightGroupIffOverrides[DEBRIEF_FLIGHT_GROUP_CAPACITY] = { 0 };

// GLOBAL: XW 0x4F7300
Actor* g_debriefPreviousPageActor = NULL;

// GLOBAL: XW 0x4F7304
Input* g_debriefPageControlsInput = NULL;

// GLOBAL: XW 0x4F7308
int16_t g_debriefSectionPages[DEBRIEF_SECTION_COUNT] = { 0 };

// GLOBAL: XW 0x4F7328
int16_t g_debriefFlightGroupCraftTypes[DEBRIEF_FLIGHT_GROUP_CAPACITY] = { 0 };

// GLOBAL: XW 0x4F7348
Actor* g_debriefStatisticsActor = NULL;

// GLOBAL: XW 0x4F7350
Actor* g_debriefDoorActors[DEBRIEF_DOOR_COUNT] = { NULL, NULL };

// GLOBAL: XW 0x4F7358
LandruHandle g_debriefBackgroundHandle = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4F735C
int16_t g_debriefKeyboardFocusIndex = 0;

// FUNCTION: XW 0x442080
void debrief_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_debriefMusicState.film = sceneFilm;
		g_debriefMusicState.sound = xsound_Find_Gmid("patrol");
		if (g_debriefMusicState.sound == NULL) {
			ResFile* resourceFile = xres_Open_Resource(":X-Wing Data\\RESOURCE\\bfmusic.lfd");
			if (resourceFile == NULL)
				resourceFile = xres_Open_Resource("bfmusic.lfd");
			g_debriefMusicState.sound = xsound_Res_Music(resourceFile, "patrol");
			soundext_Start_Resource_Sound(g_debriefMusicState.sound);
			xres_Close_Resource(resourceFile);
		}
		soundext_FadeVolume(g_debriefMusicState.sound, DEBRIEF_MUSIC_VOLUME, DEBRIEF_MUSIC_FADE_DURATION);
		xsound_Set_Sound_Keep(g_debriefMusicState.sound);
		xsound_Set_Sound_User_Function(g_debriefMusicState.sound, Cutscene_IgnoreSoundEvent);
	}
}

// FUNCTION: XW 0x442130
void debrief_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0) {
		/* Resource pointers are outside the numeric flight-sound ID range. */
		soundext_SetPriority(0, 0);
		soundext_FadeVolume(g_debriefMusicState.sound, 0, DEBRIEF_MUSIC_FADE_DURATION);
	}
}

// FUNCTION: XW 0x442170
void debrief_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	if (ShellPreferences_GetSfxEnabled() != 0) {
		soundext_LoadSfx(XW_SHELL_SFX_DOOR_6, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_DOOR_CLOSE_2, 0, NULL, 1, 1);
		soundext_LoadSfx(XW_SHELL_SFX_TEXT_5, 0, NULL, 0, 0);
		soundext_LoadSfx(XW_SHELL_SFX_TARGET_5, 0, NULL, 0, 0);
		g_debriefSound36Playing = 0;
	}
	ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x4421D0
void debrief_HandleSoundAction(int16_t action) {
	if (ShellPreferences_GetSfxEnabled() != 0) {
		switch (action) {
			case DEBRIEF_SOUND_DOOR_OPEN:
				soundext_Play_SFX(XW_SHELL_SFX_DOOR_6);
				break;
			case DEBRIEF_SOUND_DOOR_CLOSE:
				soundext_Play_SFX(XW_SHELL_SFX_DOOR_CLOSE_2);
				break;
			case DEBRIEF_SOUND_TARGET:
				soundext_Play_SFX(XW_SHELL_SFX_TARGET_5);
				break;
			case DEBRIEF_SOUND_REQUEST_TEXT:
				if (g_debriefSound36Playing == 0) {
					soundext_Play_SFX(XW_SHELL_SFX_TEXT_5);
					g_debriefSound36Playing = 1;
				}
				g_debriefSound36Activity = 1;
				break;
			case DEBRIEF_SOUND_STOP_TEXT:
				if (g_debriefSound36Playing != 0) {
					soundext_Stop_SFX(XW_SHELL_SFX_TEXT_5);
					g_debriefSound36Playing = 0;
				}
				g_debriefSound36Activity = 1;
				break;
			case DEBRIEF_SOUND_TICK:
				if (g_debriefSound36Activity == 0) {
					soundext_Stop_SFX(XW_SHELL_SFX_TEXT_5);
					g_debriefSound36Playing = 0;
				}
				g_debriefSound36Activity = 0;
				break;
		}
	}
}

// FUNCTION: XW 0x442470
XwShellSceneResult debrief_Debrief(struct XwShellContext* shell) {
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
			g_debriefPageCount = debrief_BuildSectionPages();
			break;
		case XW_SCENE_DEBRIEF_TOUR:
			g_debriefPageCount = debrief_BuildSectionPages();
			break;
	}
	g_debriefKeyboardFocusIndex = DEBRIEF_INITIAL_FOCUS;
	xio_Set_Mouse_Position(DEBRIEF_INITIAL_MOUSE_X, DEBRIEF_INITIAL_MOUSE_Y);
	debrief_Update_Debrief_Scores();
	resource = xres_Open_Resource(":X-Wing Data\\RESOURCE\\debrief.lfd");
	if (resource == NULL)
		resource = xres_Open_Resource(g_debriefResourceStrings[DEBRIEF_RESOURCE_FILE]);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	g_debriefBackgroundHandle = xmemhdl_Alloc_Clear_Handle(
		DEBRIEF_BACKGROUND_WIDTH * DEBRIEF_BACKGROUND_HEIGHT, LANDRU_MEMORY_RESOURCE);
	xrect_Set_Rect(&rect, 0, 0, DEBRIEF_BACKGROUND_WIDTH, DEBRIEF_BACKGROUND_HEIGHT);
	g_debriefFilm = xfilm_Res_Film(resource, g_debriefResourceStrings[DEBRIEF_RESOURCE_FILM], &rect, 0, 0, 0);
	xfilm_Set_Film_Def_Palette(g_debriefFilm, shell->standardPalette);
	g_debriefBaseActor = xactor_Find_Actor(FOURCC_DELT, g_debriefResourceStrings[DEBRIEF_RESOURCE_BASE]);
	xactor_Non_Refreshable_Actor(g_debriefBaseActor);
	g_debriefInteriorActor =
		xactor_Find_Actor(FOURCC_DELT, g_debriefResourceStrings[DEBRIEF_RESOURCE_INTERIOR]);
	xactor_Non_Refreshable_Actor(g_debriefInteriorActor);
	g_debriefDoorActors[DEBRIEF_DOOR_LEFT] =
		xactor_Find_Actor(FOURCC_ANIM, g_debriefResourceStrings[DEBRIEF_RESOURCE_LEFT_DOOR]);
	xactor_Set_Actor_User_Function(g_debriefDoorActors[DEBRIEF_DOOR_LEFT], debrief_user_Door);
	xactor_Non_Refreshable_Actor(g_debriefDoorActors[DEBRIEF_DOOR_LEFT]);
	g_debriefDoorActors[DEBRIEF_DOOR_LEFT]->id = DEBRIEF_DOOR_LEFT;
	g_debriefDoorActors[DEBRIEF_DOOR_RIGHT] =
		xactor_Find_Actor(FOURCC_ANIM, g_debriefResourceStrings[DEBRIEF_RESOURCE_RIGHT_DOOR]);
	xactor_Set_Actor_User_Function(g_debriefDoorActors[DEBRIEF_DOOR_RIGHT], debrief_user_Door);
	xactor_Non_Refreshable_Actor(g_debriefDoorActors[DEBRIEF_DOOR_RIGHT]);
	g_debriefDoorActors[DEBRIEF_DOOR_RIGHT]->id = DEBRIEF_DOOR_RIGHT;
	xrect_Set_Rect(&rect, 0, 0, DEBRIEF_BACKGROUND_WIDTH, DEBRIEF_BACKGROUND_HEIGHT);
	g_debriefBackgroundActor = xactcust_Alloc_Custom_Actor(0, &rect, 0, 0, DEBRIEF_BACKGROUND_Z);
	xactor_Set_Actor_User_Function(g_debriefBackgroundActor, XwDebrief_IgnoreActorEvent);
	xactor_Set_Actor_Draw_Function(g_debriefBackgroundActor, XwDebrief_DrawBackground);
	xactor_Non_Refreshable_Actor(g_debriefBackgroundActor);
	xrect_Set_Rect(&rect, DEBRIEF_STATISTICS_LEFT, DEBRIEF_STATISTICS_TOP, DEBRIEF_STATISTICS_RIGHT,
				   DEBRIEF_STATISTICS_BOTTOM);
	g_debriefStatisticsActor = xactcust_Alloc_Custom_Actor(0, &rect, 0, 0, DEBRIEF_STATISTICS_Z);
	xactor_Set_Actor_User_Function(g_debriefStatisticsActor, XwDebrief_IgnoreActorEvent);
	xactor_Set_Actor_Draw_Function(g_debriefStatisticsActor, XwDebrief_DrawStatistics);
	xactor_Non_Refreshable_Actor(g_debriefStatisticsActor);
	xrect_Set_Rect(&rect, 0, 0, DEBRIEF_BACKGROUND_WIDTH, DEBRIEF_BACKGROUND_HEIGHT);
	g_debriefRootInput = xinput_Alloc_Input(NULL, &rect, 0, 0);
	xrect_Set_Rect(&rect, 0, DEBRIEF_LEFT_DOOR_TOP, DEBRIEF_LEFT_DOOR_RIGHT, DEBRIEF_LEFT_DOOR_BOTTOM);
	input = xinput_Alloc_Input(g_debriefRootInput, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(input, debrief_iupdate_Debrief);
	xinpattr_Set_Input_User_Function(input, debrief_iuser_Debrief);
	input->mouseUsage = allInput;
	input->id = DEBRIEF_DOOR_LEFT;
	xrect_Set_Rect(&rect, DEBRIEF_RIGHT_DOOR_LEFT, DEBRIEF_RIGHT_DOOR_TOP, DEBRIEF_RIGHT_DOOR_RIGHT,
				   DEBRIEF_RIGHT_DOOR_BOTTOM);
	input = xinput_Alloc_Input(g_debriefRootInput, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(input, debrief_iupdate_Debrief);
	xinpattr_Set_Input_User_Function(input, debrief_iuser_Debrief);
	input->mouseUsage = allInput;
	input->id = DEBRIEF_DOOR_RIGHT;
	xrect_Set_Rect(&rect, DEBRIEF_LABEL_LEFT, DEBRIEF_LABEL_TOP, DEBRIEF_LABEL_RIGHT, DEBRIEF_LABEL_BOTTOM);
	g_debriefHoverLabelInput = xinput_Alloc_Input(g_debriefRootInput, &rect, 0, 0);
	xinpattr_Set_Input_Draw_Function(g_debriefHoverLabelInput, debrief_idraw_HoverLabel);
	xinpattr_Hide_Input(g_debriefHoverLabelInput);
	xrect_Set_Rect(&rect, DEBRIEF_CONTROLS_LEFT, DEBRIEF_CONTROLS_TOP, DEBRIEF_CONTROLS_RIGHT,
				   DEBRIEF_CONTROLS_BOTTOM);
	g_debriefPageControlsInput = xinput_Alloc_Input(g_debriefRootInput, &rect, 0, 0);
	xrect_Set_Rect(&rect, DEBRIEF_CONTROLS_LEFT, DEBRIEF_CONTROLS_TOP, DEBRIEF_PREVIOUS_RIGHT,
				   DEBRIEF_BUTTON_BOTTOM);
	xrect_Offset_Rect(&rect, -DEBRIEF_CONTROLS_LEFT, -DEBRIEF_CONTROLS_TOP);
	previousButton = xbtnpush_Alloc_Button(g_debriefPageControlsInput, &rect, 0, debrief_iuser_PageButton,
										   NULL, DEBRIEF_PREVIOUS_PAGE_BUTTON);
	xinpattr_Set_Input_Draw_Function(&previousButton->header, XwDebrief_DrawPageButton);
	g_debriefPreviousPageActor =
		xactor_Find_Actor(FOURCC_ANIM, g_debriefResourceStrings[DEBRIEF_RESOURCE_PREVIOUS]);
	xrect_Set_Rect(&rect, DEBRIEF_NEXT_LEFT, DEBRIEF_CONTROLS_TOP, DEBRIEF_CONTROLS_RIGHT,
				   DEBRIEF_BUTTON_BOTTOM);
	xrect_Offset_Rect(&rect, -DEBRIEF_CONTROLS_LEFT, -DEBRIEF_CONTROLS_TOP);
	nextButton = xbtnpush_Alloc_Button(g_debriefPageControlsInput, &rect, 0, debrief_iuser_PageButton, NULL,
									   DEBRIEF_NEXT_PAGE_BUTTON);
	xinpattr_Set_Input_Draw_Function(&nextButton->header, XwDebrief_DrawPageButton);
	g_debriefNextPageActor = xactor_Find_Actor(FOURCC_ANIM, g_debriefResourceStrings[DEBRIEF_RESOURCE_NEXT]);
	xrect_Set_Rect(&rect, DEBRIEF_LABEL_LEFT, DEBRIEF_LABEL_TOP, DEBRIEF_LABEL_RIGHT, DEBRIEF_LABEL_BOTTOM);
	xrect_Offset_Rect(&rect, -DEBRIEF_CONTROLS_LEFT, -DEBRIEF_CONTROLS_TOP);
	pageNumberInput = xinput_Alloc_Input(g_debriefPageControlsInput, &rect, 0, 0);
	xinpattr_Set_Input_Draw_Function(pageNumberInput, debrief_idraw_PageNumber);
	pageNumberInput->id = DEBRIEF_PAGE_NUMBER_INPUT;
	xio_Set_Key_Buttons();
	xview_Set_View_Update_Function(debrief_end_View);
	debrief_OpenMusic(resource, g_debriefFilm);
	debrief_LoadSoundEffects(resource, g_debriefFilm);
	FrontendAudio_PlayFile("XwingCD\\music\\regbrief.wav", 1);
#ifdef XW_MODERN
	XwDebrief_RunView(resource);
#else
	j_xviewadd_Handle_View();
	debrief_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xio_Clear_Key_Buttons();
	if ((uint16_t)xcursor_Is_Cursor_Visible())
		xcursor_Hide_Cursor();
	xmemhdl_Free_Handle(g_debriefBackgroundHandle);
	xres_Close_Resource(resource);
	LandruDisplay_ForwardLegacyNoOp(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x442A60
void debrief_end_View(int time) {
	int16_t key;
	if (time == 0 && (uint16_t)xcursor_Is_Cursor_Visible() == 0)
		xcursor_Show_Cursor();
	key = xio_Get_Free_Key();
	if (key != 0) {
		int focusCount = DEBRIEF_FOCUS_COUNT;
		if ((int16_t)shellext_Get_Cur_Scene() == XW_SCENE_DEBRIEF_TOUR)
			focusCount = DEBRIEF_TOUR_FOCUS_COUNT;
		if (shellext_MoveGridFocus(&g_debriefKeyboardFocusIndex, g_debriefKeyboardFocusX,
								   g_debriefKeyboardFocusY, DEBRIEF_FOCUS_ROWS, focusCount, key) != 0) {
			xio_Set_Mouse_Position(g_debriefKeyboardFocusX[g_debriefKeyboardFocusIndex],
								   g_debriefKeyboardFocusY[g_debriefKeyboardFocusIndex]);
			xio_Get_Key();
		}
	}
}

// FUNCTION: XW 0x442AF0
int16_t debrief_iupdate_Debrief(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
								int rightEvent, int16_t x, int16_t y) {
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (key != 0) {
		return 0;
	}
	if (leftEvent != DEBRIEF_MOUSE_SELECT && rightEvent != DEBRIEF_MOUSE_SELECT) {
		input->var1 = DEBRIEF_INPUT_HOVER;
	} else {
		input->var1 = DEBRIEF_INPUT_EXIT;
		if (input->id != DEBRIEF_DOOR_LEFT) {
			switch (shellext_Get_Cur_Scene()) {
				case XW_SCENE_DEBRIEF_PROVING_GROUNDS:
					input->var2 = XW_SCENE_FLIGHT_PROVING_GROUNDS;
					break;
				case XW_SCENE_DEBRIEF_COMBAT:
					input->var2 = XW_SCENE_FLIGHT_COMBAT;
					break;
			}
		} else {
			switch (shellext_Get_Cur_Scene()) {
				case XW_SCENE_DEBRIEF_PROVING_GROUNDS:
					input->var2 = XW_SCENE_PROVING_GROUNDS_ROOM;
					break;
				case XW_SCENE_DEBRIEF_COMBAT:
					input->var2 = XW_SCENE_COMBAT_SIMULATOR_ROOM;
					break;
				case XW_SCENE_DEBRIEF_TOUR:
					if (g_debriefPilotRecord.tour_status[g_debriefPilotRecord.current_tour] ==
						SHIPEXT_TOUR_STATUS_ACTIVE) {
						input->var2 = XW_SCENE_BRIEFING_TOUR;
					} else {
						input->var2 = XW_SCENE_CONCOURSE;
					}
					break;
			}
		}
	}
	if (input->id != DEBRIEF_DOOR_LEFT && shellext_Get_Cur_Scene() == XW_SCENE_DEBRIEF_TOUR) {
		input->var1 = DEBRIEF_INPUT_IDLE;
	} else {
		g_debriefDoorActors[input->id]->var1 = 1;
	}
	return 1;
}

// FUNCTION: XW 0x442BD0
void debrief_iuser_Debrief(Input* input, int context) {
	(void)context;
	switch (input->var1) {
		case DEBRIEF_INPUT_IDLE:
			if (xinpattr_Is_Input_Visible(g_debriefHoverLabelInput) != 0 &&
				input->id == g_debriefHoverLabelInput->var1) {
				xinpattr_Show_Input(g_debriefPageControlsInput);
				xinpattr_Hide_Input(g_debriefHoverLabelInput);
				xinpattr_Refresh_Input(g_debriefPageControlsInput);
			}
			break;
		case DEBRIEF_INPUT_EXIT:
			xerror_Set_Landru_Exit(input->var2);
			break;
		case DEBRIEF_INPUT_HOVER:
			if (xinpattr_Is_Input_Visible(g_debriefRootInput) != 0) {
				xinpattr_Hide_Input(g_debriefPageControlsInput);
				xinpattr_Show_Input(g_debriefHoverLabelInput);
				xinpattr_Refresh_Input(g_debriefHoverLabelInput);
				g_debriefHoverLabelInput->var1 = input->id;
			}
			input->var1 = DEBRIEF_INPUT_IDLE;
			break;
	}
}

// FUNCTION: XW 0x442CA0
void debrief_idraw_HoverLabel(Input* input, Rect* frame, Rect* unusedClip, int16_t refresh) {
	(void)unusedClip;
	if (refresh != 0) {
		int16_t captionSlot = DEBRIEF_HOVER_CAPTION_STRIDE * input->var1 + DEBRIEF_HOVER_EXIT_CAPTION;
		xpaint_Paint_Clipped_Rect(frame, DEBRIEF_LABEL_BACKGROUND_COLOR);
		xrect_Offset_Rect(frame, DEBRIEF_HOVER_TEXT_OFFSET, DEBRIEF_HOVER_TEXT_OFFSET);
		xfont_Print_Centered_Text(g_debriefResourceStrings[captionSlot], frame, DEBRIEF_LABEL_FONT,
								  DEBRIEF_LABEL_TEXT_COLOR);
	}
}

// FUNCTION: XW 0x442D00
void debrief_iuser_PageButton(Input* input, int unusedTime) {
	(void)unusedTime;
	if (xinpattr_Get_Input_Selected(input) != 0) {
		if (input->id == DEBRIEF_PREVIOUS_PAGE_BUTTON) {
			if (g_debriefPageIndex > 0) {
				--g_debriefPageIndex;
			} else {
				g_debriefPageIndex = g_debriefPageCount - 1;
			}
		} else if (g_debriefPageIndex < g_debriefPageCount - 1) {
			++g_debriefPageIndex;
		} else {
			g_debriefPageIndex = 0;
		}
		xview_Refresh_View();
	}
}

// FUNCTION: XW 0x442D80
void debrief_idraw_PageButton(PushButton* button, Rect* frame, Rect* clip, int16_t refresh) {
	if (refresh != 0) {
		Actor* buttonActor;
		switch (button->header.id) {
			case DEBRIEF_PREVIOUS_PAGE_BUTTON:
				buttonActor = g_debriefPreviousPageActor;
				break;
			case DEBRIEF_NEXT_PAGE_BUTTON:
				buttonActor = g_debriefNextPageActor;
				break;
			default:
				buttonActor = NULL;
				break;
		}
		if (buttonActor != NULL) {
			xactor_Set_Actor_State(buttonActor, button->pressed, 0);
			xactanim_Draw_Anim_Actor(buttonActor, frame, clip, DEBRIEF_PAGE_BUTTON_X_OFFSET, 0, refresh);
		}
	}
}

// FUNCTION: XW 0x442DE0
void debrief_idraw_PageNumber(Input* unusedInput, Rect* frame, Rect* unusedClip, int16_t refresh) {
	char pageText[DEBRIEF_PAGE_TEXT_CAPACITY];
	(void)unusedInput;
	(void)unusedClip;
	if (refresh != 0) {
		xpaint_Paint_Clipped_Rect(frame, DEBRIEF_LABEL_BACKGROUND_COLOR);
		sprintf(pageText, "Page %d of %d", g_debriefPageIndex + 1, g_debriefPageCount);
		xfont_Print_Centered_Text(pageText, frame, DEBRIEF_LABEL_FONT, DEBRIEF_LABEL_TEXT_COLOR);
	}
}

// FUNCTION: XW 0x442E40
void debrief_user_Door(Actor* actor, int time) {
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
		if (actor->state == 1) {
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

// FUNCTION: XW 0x442ED0
void debrief_draw_Background(Actor* unusedActor, Rect* frame, Rect* unusedClip, int16_t x, int16_t y,
							 int16_t refresh) {
	(void)unusedActor;
	(void)unusedClip;
	if (refresh != 0) {
		const uint8_t* backgroundPixels = xmemhdl_Lock_Handle(g_debriefBackgroundHandle);
		stub_Copy_From_Clipped_Buffer(backgroundPixels, frame, x, y, DEBRIEF_BACKGROUND_WIDTH,
									  DEBRIEF_BACKGROUND_HEIGHT);
		xmemhdl_Unlock_Handle(g_debriefBackgroundHandle);
	}
}

// FUNCTION: XW 0x442F20
void debrief_draw_Statistics(Actor* unusedActor, Rect* frame, Rect* unusedClip, int16_t unusedX,
							 int16_t unusedY, int16_t refresh) {
	(void)unusedActor;
	(void)unusedClip;
	(void)unusedX;
	(void)unusedY;
	if (refresh != 0) {
		Rect headerRect;
		xpaint_Paint_Clipped_Rect(frame, DEBRIEF_STATISTICS_BACKGROUND_COLOR);
		xfont_Enable_FontID_Shadow(DEBRIEF_HEADER_FONT);
		xfont_Set_FontID_Bold_Color(DEBRIEF_HEADER_FONT, DEBRIEF_STATISTICS_BOLD_COLOR);
		xfont_Enable_FontID_Shadow(DEBRIEF_ACCURACY_DETAIL_FONT);
		xfont_Set_FontID_Bold_Color(DEBRIEF_ACCURACY_DETAIL_FONT, DEBRIEF_STATISTICS_BOLD_COLOR);
		xrect_Copy_Rect(&headerRect, frame);
		headerRect.bottom = headerRect.top + DEBRIEF_STATISTICS_HEADER_HEIGHT;
		xpaint_Paint_Clipped_Rect(&headerRect, DEBRIEF_STATISTICS_HEADER_COLOR);
		xpaint_Horiz_Clipped_Line(headerRect.left, headerRect.bottom, headerRect.right - headerRect.left,
								  DEBRIEF_STATISTICS_SEPARATOR_COLOR);
		++headerRect.top;
		headerRect.bottom = headerRect.top + DEBRIEF_PROVING_HEADER_HEIGHT;
		switch (shellext_Get_Cur_Scene()) {
			case XW_SCENE_DEBRIEF_PROVING_GROUNDS:
				debrief_DrawProvingGroundSummary(&headerRect);
				break;
			case XW_SCENE_DEBRIEF_COMBAT:
			case XW_SCENE_DEBRIEF_TOUR:
				debrief_DrawMissionSummary(&headerRect);
				break;
			default:
				break;
		}
		xfont_Disable_FontID_Shadow(DEBRIEF_HEADER_FONT);
		xfont_Disable_FontID_Shadow(DEBRIEF_ACCURACY_DETAIL_FONT);
	}
}

// FUNCTION: XW 0x443010
void debrief_DrawProvingGroundSummary(Rect* headerRect) {
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
	xfont_Print_Centered_Text(text, &lineRect, DEBRIEF_HEADER_FONT, DEBRIEF_HEADER_COLOR);
	strcpy(text, g_debriefProvingGroundText);
	strcat(text, g_debriefRunText);
	firstRun = shipext_Get_Train_Level() + 1;
	lastRun = g_missionRuntimeState.provingGroundsLevel;
	if (firstRun == lastRun)
		sprintf(numberText, " %d)", (int)firstRun);
	else
		sprintf(numberText, "s %d to %d)", (int)firstRun, (int)lastRun);
	strcat(text, numberText);
	xrect_Offset_Rect(&lineRect, 0, DEBRIEF_PROVING_HEADER_HEIGHT);
	xfont_Print_Centered_Text(text, &lineRect, DEBRIEF_HEADER_FONT, DEBRIEF_HEADER_COLOR);
	if (g_missionRuntimeState.flightBadgeAnnouncement != 0) {
		xrect_Offset_Rect(&lineRect, 0, DEBRIEF_PROVING_SECTION_HEIGHT);
		sprintf(text, "You got a Flight Badge!");
		xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_ACCURACY_HEADING_LEFT, lineRect.top,
								 DEBRIEF_ACCURACY_HEADING_FONT, DEBRIEF_ACCURACY_HEADING_COLOR);
		xrect_Offset_Rect(&lineRect, 0, DEBRIEF_ACCURACY_HEADING_HEIGHT);
	}
	if (g_missionRuntimeState.newRank != 0) {
		if (g_missionRuntimeState.flightBadgeAnnouncement == 0)
			xrect_Offset_Rect(&lineRect, 0, DEBRIEF_PROVING_SECTION_HEIGHT);
		strcpy(text, g_debriefPromotionText);
		strcat(text, g_debriefRankNames[g_missionRuntimeState.newRank]);
		xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_ACCURACY_HEADING_LEFT, lineRect.top,
								 DEBRIEF_ACCURACY_HEADING_FONT, DEBRIEF_ACCURACY_HEADING_COLOR);
		xrect_Offset_Rect(&lineRect, 0, DEBRIEF_ACCURACY_HEADING_HEIGHT);
	}
	strcpy(text, g_debriefScoreText);
	sprintf(numberText, "%ld", (long)g_missionRuntimeState.provingGroundsScore);
	strcat(text, numberText);
	xrect_Offset_Rect(&lineRect, 0, DEBRIEF_PROVING_SECTION_HEIGHT);
	xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_ACCURACY_HEADING_LEFT, lineRect.top,
							 DEBRIEF_ACCURACY_HEADING_FONT, DEBRIEF_ACCURACY_HEADING_COLOR);
	strcpy(text, g_debriefGatesPassedText);
	sprintf(numberText, "%u", (unsigned int)g_missionRuntimeState.provingGroundsCheckpointsPassed);
	strcat(text, numberText);
	xrect_Offset_Rect(&lineRect, 0, DEBRIEF_ACCURACY_HEADING_HEIGHT);
	xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_ACCURACY_DETAIL_LEFT, lineRect.top,
							 DEBRIEF_ACCURACY_DETAIL_FONT, DEBRIEF_ACCURACY_DETAIL_COLOR);
	strcpy(text, g_debriefGatesMissedText);
	sprintf(numberText, "%u", (unsigned int)g_missionRuntimeState.provingGroundsCheckpointsMissed);
	strcat(text, numberText);
	xrect_Offset_Rect(&lineRect, 0, DEBRIEF_ACCURACY_DETAIL_HEIGHT);
	xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_ACCURACY_DETAIL_LEFT, lineRect.top,
							 DEBRIEF_ACCURACY_DETAIL_FONT, DEBRIEF_ACCURACY_DETAIL_COLOR);
	strcpy(text, g_debriefGatesRemainingText);
	sprintf(numberText, "%u", (unsigned int)g_missionRuntimeState.provingGroundsCheckpointsRemaining);
	strcat(text, numberText);
	xrect_Offset_Rect(&lineRect, 0, DEBRIEF_ACCURACY_DETAIL_HEIGHT);
	xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_ACCURACY_DETAIL_LEFT, lineRect.top,
							 DEBRIEF_ACCURACY_DETAIL_FONT, DEBRIEF_ACCURACY_DETAIL_COLOR);
	roundsFired =
		g_playerFlightState.weaponStats.warheadsFired + g_playerFlightState.weaponStats.laserShotsFired;
	strcpy(text, g_debriefRoundsFiredText);
	sprintf(numberText, "%u", (unsigned int)roundsFired);
	strcat(text, numberText);
	xrect_Offset_Rect(&lineRect, 0, DEBRIEF_ACCURACY_DETAIL_HEIGHT);
	xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_ACCURACY_HEADING_LEFT, lineRect.top,
							 DEBRIEF_ACCURACY_HEADING_FONT, DEBRIEF_ACCURACY_HEADING_COLOR);
	if (roundsFired != 0)
		hitPercent =
			DEBRIEF_PERCENT_SCALE * g_missionRuntimeState.provingGroundsTargetsDestroyed / (int)roundsFired;
	else
		hitPercent = 0;
	targetHits = g_missionRuntimeState.provingGroundsTargetsDestroyed;
	strcpy(text, g_debriefTargetHitsText);
	sprintf(numberText, "%u (%ld%%)", (unsigned int)targetHits, (long)hitPercent);
	strcat(text, numberText);
	xrect_Offset_Rect(&lineRect, 0, DEBRIEF_ACCURACY_HEADING_HEIGHT);
	xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_ACCURACY_DETAIL_LEFT, lineRect.top,
							 DEBRIEF_ACCURACY_DETAIL_FONT, DEBRIEF_ACCURACY_DETAIL_COLOR);
}

// FUNCTION: XW 0x4436C0
int debrief_BuildSectionPages(void) {
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
				sectionHeight = debrief_SectionMissionResult(&measureRect, 0);
				break;
			case DEBRIEF_SECTION_PROMOTION:
				sectionHeight = debrief_SectionPromotion(&measureRect, 0);
				break;
			case DEBRIEF_SECTION_BATTLE_PATCH:
				sectionHeight = debrief_SectionBattlePatch(&measureRect, 0);
				break;
			case DEBRIEF_SECTION_UNCOMPLETED_GOALS:
				sectionHeight = debrief_SectionUncompletedGoals(&measureRect, 0);
				break;
			case DEBRIEF_SECTION_COMPLETED_GOALS:
				sectionHeight = debrief_SectionCompletedGoals(&measureRect, 0);
				break;
			case DEBRIEF_SECTION_OBJECT_GOALS:
				sectionHeight = debrief_SectionObjectGoals(&measureRect, 0);
				break;
			case DEBRIEF_SECTION_SPACECRAFT_KILLS:
				sectionHeight = debrief_SectionSpacecraftKills(&measureRect, 0);
				break;
			case DEBRIEF_SECTION_SPACECRAFT_LOSSES:
				sectionHeight = debrief_SectionSpacecraftLosses(&measureRect, 0);
				break;
			case DEBRIEF_SECTION_SPACE_OBJECT_KILLS:
				sectionHeight = debrief_SectionSpaceObjectKills(&measureRect, 0);
				break;
			case DEBRIEF_SECTION_DEATH_STAR_BUILDING_KILLS:
				sectionHeight = debrief_SectionDeathStarBuildingKills(&measureRect, 0);
				break;
			case DEBRIEF_SECTION_LASER_ACCURACY:
				sectionHeight = debrief_SectionLaserAccuracy(&measureRect, 0);
				break;
			case DEBRIEF_SECTION_ION_ACCURACY:
				sectionHeight = debrief_SectionIonAccuracy(&measureRect, 0);
				break;
			case DEBRIEF_SECTION_WARHEAD_ACCURACY:
				sectionHeight = debrief_SectionWarheadAccuracy(&measureRect, 0);
				break;
		}
		pageHeight += sectionHeight;
		if (pageHeight > DEBRIEF_PAGE_HEIGHT) {
			pageHeight = sectionHeight;
			++pageIndex;
		}
		g_debriefSectionPages[sectionIndex] = pageIndex;
	}
	return pageIndex + 1;
}

// FUNCTION: XW 0x443820
void debrief_DrawMissionSummary(Rect* headerRect) {
	Rect lineRect;
	debrief_DrawMissionHeader(headerRect);
	xrect_Copy_Rect(&lineRect, headerRect);
	xrect_Offset_Rect(&lineRect, 0, DEBRIEF_SUMMARY_SECTIONS_TOP);
	if ((uint16_t)g_debriefSectionPages[DEBRIEF_SECTION_MISSION_RESULT] == g_debriefPageIndex) {
		int16_t height = debrief_SectionMissionResult(&lineRect, 1);
		xrect_Offset_Rect(&lineRect, 0, height);
	}
	if ((uint16_t)g_debriefSectionPages[DEBRIEF_SECTION_PROMOTION] == g_debriefPageIndex) {
		int16_t height = debrief_SectionPromotion(&lineRect, 1);
		xrect_Offset_Rect(&lineRect, 0, height);
	}
	if ((uint16_t)g_debriefSectionPages[DEBRIEF_SECTION_BATTLE_PATCH] == g_debriefPageIndex) {
		int16_t height = debrief_SectionBattlePatch(&lineRect, 1);
		xrect_Offset_Rect(&lineRect, 0, height);
	}
	if ((uint16_t)g_debriefSectionPages[DEBRIEF_SECTION_UNCOMPLETED_GOALS] == g_debriefPageIndex) {
		int16_t height = debrief_SectionUncompletedGoals(&lineRect, 1);
		xrect_Offset_Rect(&lineRect, 0, height);
	}
	if ((uint16_t)g_debriefSectionPages[DEBRIEF_SECTION_COMPLETED_GOALS] == g_debriefPageIndex) {
		int16_t height = debrief_SectionCompletedGoals(&lineRect, 1);
		xrect_Offset_Rect(&lineRect, 0, height);
	}
	if ((uint16_t)g_debriefSectionPages[DEBRIEF_SECTION_OBJECT_GOALS] == g_debriefPageIndex) {
		int16_t height = debrief_SectionObjectGoals(&lineRect, 1);
		xrect_Offset_Rect(&lineRect, 0, height);
	}
	if ((uint16_t)g_debriefSectionPages[DEBRIEF_SECTION_SPACECRAFT_KILLS] == g_debriefPageIndex) {
		int16_t height = debrief_SectionSpacecraftKills(&lineRect, 1);
		xrect_Offset_Rect(&lineRect, 0, height);
	}
	if ((uint16_t)g_debriefSectionPages[DEBRIEF_SECTION_SPACECRAFT_LOSSES] == g_debriefPageIndex) {
		int16_t height = debrief_SectionSpacecraftLosses(&lineRect, 1);
		xrect_Offset_Rect(&lineRect, 0, height);
	}
	if ((uint16_t)g_debriefSectionPages[DEBRIEF_SECTION_SPACE_OBJECT_KILLS] == g_debriefPageIndex) {
		int16_t height = debrief_SectionSpaceObjectKills(&lineRect, 1);
		xrect_Offset_Rect(&lineRect, 0, height);
	}
	if ((uint16_t)g_debriefSectionPages[DEBRIEF_SECTION_DEATH_STAR_BUILDING_KILLS] == g_debriefPageIndex) {
		int16_t height = debrief_SectionDeathStarBuildingKills(&lineRect, 1);
		xrect_Offset_Rect(&lineRect, 0, height);
	}
	if ((uint16_t)g_debriefSectionPages[DEBRIEF_SECTION_LASER_ACCURACY] == g_debriefPageIndex) {
		int16_t height = debrief_SectionLaserAccuracy(&lineRect, 1);
		xrect_Offset_Rect(&lineRect, 0, height);
	}
	if ((uint16_t)g_debriefSectionPages[DEBRIEF_SECTION_ION_ACCURACY] == g_debriefPageIndex) {
		int16_t height = debrief_SectionIonAccuracy(&lineRect, 1);
		xrect_Offset_Rect(&lineRect, 0, height);
	}
	if ((uint16_t)g_debriefSectionPages[DEBRIEF_SECTION_WARHEAD_ACCURACY] == g_debriefPageIndex) {
		int16_t height = debrief_SectionWarheadAccuracy(&lineRect, 1);
		xrect_Offset_Rect(&lineRect, 0, height);
	}
}

// FUNCTION: XW 0x443B00
void debrief_DrawMissionHeader(Rect* headerRect) {
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
	xfont_Print_Centered_Text(text, &lineRect, DEBRIEF_HEADER_FONT, DEBRIEF_HEADER_COLOR);
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
	xrect_Offset_Rect(&lineRect, 0, DEBRIEF_HEADER_LINE_HEIGHT);
	xfont_Print_Centered_Text(text, &lineRect, DEBRIEF_HEADER_FONT, DEBRIEF_HEADER_COLOR);
}

// FUNCTION: XW 0x443E20
int16_t debrief_SectionMissionResult(Rect* sectionRect, int16_t draw) {
	Rect lineRect;
	xrect_Copy_Rect(&lineRect, sectionRect);
	if (draw != 0) {
		if (g_missionRuntimeState.objectivesCompleted != 0) {
			xfont_Print_Centered_Text("The Mission was a Success!", &lineRect, DEBRIEF_MISSION_RESULT_FONT,
									  DEBRIEF_MISSION_SUCCESS_COLOR);
		} else {
			xfont_Print_Centered_Text("The Mission was a Failure!", &lineRect, DEBRIEF_MISSION_RESULT_FONT,
									  DEBRIEF_MISSION_FAILURE_COLOR);
		}
	}
	return DEBRIEF_MISSION_RESULT_HEIGHT;
}

// FUNCTION: XW 0x443E90
int16_t debrief_SectionPromotion(Rect* sectionRect, int16_t draw) {
	Rect lineRect;
	char text[DEBRIEF_AWARD_TEXT_CAPACITY];
	xrect_Copy_Rect(&lineRect, sectionRect);
	if (g_missionRuntimeState.newRank != 0) {
		if (draw != 0) {
			strcpy(text, g_debriefPromotionText);
			strcat(text, g_debriefRankNames[g_missionRuntimeState.newRank]);
			xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_AWARD_LEFT_INSET, lineRect.top,
									 DEBRIEF_AWARD_FONT, DEBRIEF_AWARD_COLOR);
		}
		return DEBRIEF_AWARD_HEIGHT;
	}
	return 0;
}

// FUNCTION: XW 0x443F60
int16_t debrief_SectionBattlePatch(Rect* sectionRect, int16_t draw) {
	Rect lineRect;
	char text[DEBRIEF_AWARD_TEXT_CAPACITY];
	const char* awardText = "Battle Patch Awarded!";
	xrect_Copy_Rect(&lineRect, sectionRect);
	if (g_missionRuntimeState.newBattlePatch != 0) {
		if (draw != 0) {
			strcpy(text, awardText);
			xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_AWARD_LEFT_INSET, lineRect.top,
									 DEBRIEF_AWARD_FONT, DEBRIEF_AWARD_COLOR);
		}
		return DEBRIEF_AWARD_HEIGHT;
	}
	return 0;
}

// FUNCTION: XW 0x443FE0
int16_t debrief_SectionUncompletedGoals(Rect* sectionRect, int16_t draw) {
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
									 lineRect.top, DEBRIEF_GOAL_HEADING_FONT, DEBRIEF_GOAL_HEADING_COLOR);
			xrect_Offset_Rect(&lineRect, 0, DEBRIEF_GOAL_HEADING_HEIGHT);
			flightGroupCount = g_debriefFlightGroupCount;
		}
		height = DEBRIEF_GOAL_HEADING_HEIGHT;
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
					xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_GOAL_LINE_LEFT, lineRect.top,
											 DEBRIEF_GOAL_LINE_FONT, DEBRIEF_GOAL_LINE_COLOR);
					xrect_Offset_Rect(&lineRect, 0, DEBRIEF_GOAL_LINE_HEIGHT);
					flightGroupCount = g_debriefFlightGroupCount;
				}
				height += DEBRIEF_GOAL_LINE_HEIGHT;
			}
		}
		xrect_Offset_Rect(&lineRect, 0, DEBRIEF_GOAL_TRAILING_SPACE);
		height += DEBRIEF_GOAL_TRAILING_SPACE;
	}
	return (int16_t)height;
}

// FUNCTION: XW 0x444260
int16_t debrief_SectionCompletedGoals(Rect* sectionRect, int16_t draw) {
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
									 lineRect.top, DEBRIEF_GOAL_HEADING_FONT, DEBRIEF_GOAL_HEADING_COLOR);
			xrect_Offset_Rect(&lineRect, 0, DEBRIEF_GOAL_HEADING_HEIGHT);
			flightGroupCount = g_debriefFlightGroupCount;
		}
		height = DEBRIEF_GOAL_HEADING_HEIGHT;
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
					xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_GOAL_LINE_LEFT, lineRect.top,
											 DEBRIEF_GOAL_LINE_FONT, DEBRIEF_GOAL_LINE_COLOR);
					xrect_Offset_Rect(&lineRect, 0, DEBRIEF_GOAL_LINE_HEIGHT);
					flightGroupCount = g_debriefFlightGroupCount;
				}
				height += DEBRIEF_GOAL_LINE_HEIGHT;
			}
		}
		xrect_Offset_Rect(&lineRect, 0, DEBRIEF_GOAL_TRAILING_SPACE);
		height += DEBRIEF_GOAL_TRAILING_SPACE;
	}
	return (int16_t)height;
}

// FUNCTION: XW 0x4444D0
int16_t debrief_SectionObjectGoals(Rect* sectionRect, int16_t draw) {
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
										 DEBRIEF_OBJECT_GOAL_FONT, DEBRIEF_OBJECT_GOAL_COLOR);
				xrect_Offset_Rect(&lineRect, 0, DEBRIEF_OBJECT_GOAL_LINE_HEIGHT);
			}
			height += DEBRIEF_OBJECT_GOAL_LINE_HEIGHT;
		}
	}
	return height;
}

// FUNCTION: XW 0x444660
int16_t debrief_SectionSpacecraftKills(Rect* sectionRect, int16_t draw) {
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
			xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_LOSS_HEADER_LEFT, lineRect.top,
									 DEBRIEF_LOSS_HEADER_FONT, DEBRIEF_LOSS_HEADER_COLOR);
			columnX = lineRect.left + DEBRIEF_LOSS_COLUMN_LEFT;
			rowY = lineRect.bottom + DEBRIEF_LOSS_ROWS_TOP;
			if (nonzeroCategories > 0) {
				rowsPerColumn = (nonzeroCategories + 1) >> 1;
				for (rowIndex = 0, remainingRows = nonzeroCategories; remainingRows != 0;
					 ++rowIndex, --remainingRows) {
					int16_t category = categories.order[rowIndex];
					int16_t displayCount;
					if (rowIndex == rowsPerColumn) {
						columnX += DEBRIEF_LOSS_COLUMN_SPACING;
						rowY = lineRect.bottom + DEBRIEF_LOSS_ROWS_TOP;
					}
					displayCount = categories.total[category];
					if (displayCount != 0) {
						int16_t valueOffsetX;
						strcpy(text, g_debriefCraftCategoryNames[category]);
						strcat(text, ": ");
						xfont_Print_Clipped_Text(text, columnX, rowY, DEBRIEF_LOSS_FONT,
												 DEBRIEF_LOSS_LABEL_COLOR);
						valueOffsetX = xfont_Get_String_Width_0(DEBRIEF_LOSS_FONT, text);
						sprintf(text, g_debriefKillsCountFormat, displayCount,
								(int16_t)categories.player[category]);
						xfont_Print_Clipped_Text(text, columnX + valueOffsetX, rowY, DEBRIEF_LOSS_FONT,
												 DEBRIEF_LOSS_VALUE_COLOR);
						rowY += DEBRIEF_LOSS_ROW_HEIGHT;
					}
				}
			}
		}
		return DEBRIEF_LOSS_BUDGET_ROW_HEIGHT * (((nonzeroCategories + 1) >> 1) + 1);
	}
	return 0;
}

// FUNCTION: XW 0x4449A0
int16_t debrief_SectionSpacecraftLosses(Rect* sectionRect, int16_t draw) {
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
			xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_LOSS_HEADER_LEFT, lineRect.top,
									 DEBRIEF_LOSS_HEADER_FONT, DEBRIEF_LOSS_HEADER_COLOR);
			columnX = lineRect.left + DEBRIEF_LOSS_COLUMN_LEFT;
			rowY = lineRect.bottom + DEBRIEF_LOSS_ROWS_TOP;
			if (nonzeroCategories > 0) {
				rowsPerColumn = (nonzeroCategories + 1) >> 1;
				for (rowIndex = 0, remainingRows = nonzeroCategories; remainingRows != 0;
					 ++rowIndex, --remainingRows) {
					int16_t category = categories.order[rowIndex];
					int16_t displayCount;
					if (rowIndex == rowsPerColumn) {
						columnX += DEBRIEF_LOSS_COLUMN_SPACING;
						rowY = lineRect.bottom + DEBRIEF_LOSS_ROWS_TOP;
					}
					displayCount = categories.total[category];
					if (displayCount != 0) {
						int valueOffsetX;
						strcpy(text, g_debriefCraftCategoryNames[category]);
						strcat(text, ":");
						xfont_Print_Clipped_Text(text, columnX, rowY, DEBRIEF_LOSS_FONT,
												 DEBRIEF_LOSS_LABEL_COLOR);
						valueOffsetX = xfont_Get_String_Width_0(DEBRIEF_LOSS_FONT, text);
						valueOffsetX += xfont_Get_String_Width_0(DEBRIEF_LOSS_FONT, " ");
						sprintf(text, g_debriefLossCountFormat, displayCount);
						xfont_Print_Clipped_Text(text, columnX + valueOffsetX, rowY, DEBRIEF_LOSS_FONT,
												 DEBRIEF_LOSS_VALUE_COLOR);
						rowY += DEBRIEF_LOSS_ROW_HEIGHT;
					}
				}
			}
		}
		return DEBRIEF_LOSS_BUDGET_ROW_HEIGHT * (((nonzeroCategories + 1) >> 1) + 1);
	}
	return 0;
}

// FUNCTION: XW 0x444CA0
int16_t debrief_SectionSpaceObjectKills(Rect* sectionRect, int16_t draw) {
	Rect lineRect;
	char numberText[DEBRIEF_KILL_NUMBER_CAPACITY];
	char text[DEBRIEF_KILL_TEXT_CAPACITY];
	xrect_Copy_Rect(&lineRect, sectionRect);
	if (g_playerFlightState.spaceObjectKills != 0) {
		if (draw != 0) {
			strcpy(text, g_debriefSpaceObjectKillsText);
			sprintf(numberText, "%d", g_playerFlightState.spaceObjectKills);
			strcat(text, numberText);
			xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_KILL_LINE_LEFT_INSET, lineRect.top,
									 DEBRIEF_KILL_LINE_FONT, DEBRIEF_KILL_LINE_COLOR);
		}
		return DEBRIEF_KILL_LINE_HEIGHT;
	}
	return 0;
}

// FUNCTION: XW 0x444D70
int16_t debrief_SectionDeathStarBuildingKills(Rect* sectionRect, int16_t draw) {
	Rect lineRect;
	char numberText[DEBRIEF_KILL_NUMBER_CAPACITY];
	char text[DEBRIEF_KILL_TEXT_CAPACITY];
	xrect_Copy_Rect(&lineRect, sectionRect);
	if (g_playerFlightState.deathStarBuildingKills != 0) {
		if (draw != 0) {
			strcpy(text, g_debriefDeathStarBuildingKillsText);
			sprintf(numberText, "%d", g_playerFlightState.deathStarBuildingKills);
			strcat(text, numberText);
			xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_KILL_LINE_LEFT_INSET, lineRect.top,
									 DEBRIEF_KILL_LINE_FONT, DEBRIEF_KILL_LINE_COLOR);
		}
		return DEBRIEF_KILL_LINE_HEIGHT;
	}
	return 0;
}

// FUNCTION: XW 0x444E40
int debrief_SectionLaserAccuracy(Rect* sectionRect, int16_t draw) {
	uint16_t shotsFired = g_playerFlightState.weaponStats.laserShotsFired;
	uint16_t spacecraftHits = g_playerFlightState.weaponStats.laserSpacecraftHits;
	uint16_t surfaceHits = g_playerFlightState.weaponStats.laserSurfaceHits;
	return debrief_SectionWeaponAccuracy(sectionRect, DEBRIEF_WEAPON_LASER, shotsFired, spacecraftHits,
										 surfaceHits, draw);
}

// FUNCTION: XW 0x444E80
int debrief_SectionIonAccuracy(Rect* sectionRect, int16_t draw) {
	uint16_t shotsFired = g_playerFlightState.weaponStats.ionShotsFired;
	uint16_t spacecraftHits = g_playerFlightState.weaponStats.ionSpacecraftHits;
	uint16_t surfaceHits = g_playerFlightState.weaponStats.ionSurfaceHits;
	return debrief_SectionWeaponAccuracy(sectionRect, DEBRIEF_WEAPON_ION, shotsFired, spacecraftHits,
										 surfaceHits, draw);
}

// FUNCTION: XW 0x444ED0
int debrief_SectionWarheadAccuracy(Rect* sectionRect, int16_t draw) {
	return debrief_SectionWeaponAccuracy(sectionRect, DEBRIEF_WEAPON_WARHEAD,
										 g_playerFlightState.weaponStats.warheadsFired,
										 g_playerFlightState.weaponStats.warheadSpacecraftHits,
										 g_playerFlightState.weaponStats.warheadSurfaceHits, draw);
}

// FUNCTION: XW 0x444F00
int debrief_SectionWeaponAccuracy(Rect* sectionRect, int16_t weaponKind, uint16_t shotsFired,
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
			xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_ACCURACY_HEADING_LEFT, lineRect.top,
									 DEBRIEF_ACCURACY_HEADING_FONT, DEBRIEF_ACCURACY_HEADING_COLOR);
		}
		if (surfaceHits == 0) {
			if (draw != 0) {
				xrect_Offset_Rect(&lineRect, 0, DEBRIEF_ACCURACY_HEADING_HEIGHT);
				strcpy(text, g_debriefSpacecraftHitsText);
				sprintf(numberText, "%u (%lu%%)", (unsigned int)spacecraftHits,
						(unsigned long)(DEBRIEF_PERCENT_SCALE * spacecraftHits / shotsFired));
				strcat(text, numberText);
				xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_ACCURACY_DETAIL_LEFT, lineRect.top,
										 DEBRIEF_ACCURACY_DETAIL_FONT, DEBRIEF_ACCURACY_DETAIL_COLOR);
			}
			return DEBRIEF_ACCURACY_SPACE_HEIGHT;
		}
		if (draw != 0) {
			uint16_t totalHits = surfaceHits + spacecraftHits;
			xrect_Offset_Rect(&lineRect, 0, DEBRIEF_ACCURACY_HEADING_HEIGHT);
			strcpy(text, g_debriefTotalHitsText);
			sprintf(numberText, "%u (%lu%%)", (unsigned int)totalHits,
					(unsigned long)(DEBRIEF_PERCENT_SCALE * totalHits / shotsFired));
			strcat(text, numberText);
			xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_ACCURACY_DETAIL_LEFT, lineRect.top,
									 DEBRIEF_ACCURACY_DETAIL_FONT, DEBRIEF_ACCURACY_DETAIL_COLOR);
			xrect_Offset_Rect(&lineRect, 0, DEBRIEF_ACCURACY_DETAIL_HEIGHT);
			strcpy(text, g_debriefSpacecraftHitsText);
			sprintf(numberText, "%u (%lu%%)", (unsigned int)spacecraftHits,
					(unsigned long)(DEBRIEF_PERCENT_SCALE * spacecraftHits / shotsFired));
			strcat(text, numberText);
			xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_ACCURACY_DETAIL_LEFT, lineRect.top,
									 DEBRIEF_ACCURACY_DETAIL_FONT, DEBRIEF_ACCURACY_DETAIL_COLOR);
			xrect_Offset_Rect(&lineRect, 0, DEBRIEF_ACCURACY_DETAIL_HEIGHT);
			strcpy(text, g_debriefSurfaceHitsText);
			sprintf(numberText, "%u (%lu%%)", (unsigned int)surfaceHits,
					(unsigned long)(DEBRIEF_PERCENT_SCALE * surfaceHits / shotsFired));
			strcat(text, numberText);
			xfont_Print_Clipped_Text(text, lineRect.left + DEBRIEF_ACCURACY_DETAIL_LEFT, lineRect.top,
									 DEBRIEF_ACCURACY_DETAIL_FONT, DEBRIEF_ACCURACY_DETAIL_COLOR);
		}
		return DEBRIEF_ACCURACY_SURFACE_HEIGHT;
	}
	return 0;
}

// FUNCTION: XW 0x445300
void debrief_Update_Debrief_Scores(void) {

	int16_t scene = shellext_Get_Cur_Scene();
	if (scene == XW_SCENE_DEBRIEF_PROVING_GROUNDS) {
		LandruFile* scoreFile;
		uint32_t scoreRead;
		uint16_t levelRead;
		int index;
		int remaining;
		int16_t qualifies;
		int16_t rank;
		int insertionRank;
		scoreFile = xfile_Open_File(LANDRU_FILE_ROOT_USER, "X-Wing Data\\train.hgh", "rb");
		if (scoreFile != NULL) {
			for (index = 0, remaining = DEBRIEF_HIGH_SCORE_COUNT; remaining != 0; ++index, --remaining) {
				xfile_Read_Data_From_File(scoreFile, g_trainingHighScoreNames[index],
										  sizeof(g_trainingHighScoreNames[index]));
				xfile_Read_Long_From_File(scoreFile, &scoreRead);
				g_trainingHighScorePoints[index] = (int)scoreRead;
				xfile_Read_Word_From_File(scoreFile, &levelRead);
				g_trainingHighScoreLevels[index] = (int16_t)levelRead;
			}
			xfile_Close_File(scoreFile);
		} else {
			memset(g_trainingHighScoreLevels, 0, sizeof(g_trainingHighScoreLevels));
			memset(g_trainingHighScorePoints, 0, sizeof(g_trainingHighScorePoints));
			for (index = 0, remaining = DEBRIEF_HIGH_SCORE_COUNT; remaining != 0; ++index, --remaining)
				g_trainingHighScoreNames[index][0] = '\0';
		}
		qualifies = 0;
		for (rank = 0; rank < DEBRIEF_HIGH_SCORE_COUNT && qualifies == 0; ++rank) {
			if (g_trainingHighScorePoints[rank] < g_missionRuntimeState.provingGroundsScore) {
				qualifies = 1;
				insertionRank = rank;
			}
		}
		if (qualifies == 0)
			return;
		for (index = DEBRIEF_HIGH_SCORE_COUNT - 1; index > (int16_t)insertionRank; --index) {
			strcpy(g_trainingHighScoreNames[index], g_trainingHighScoreNames[index - 1]);
			g_trainingHighScoreLevels[index] = g_trainingHighScoreLevels[index - 1];
			g_trainingHighScorePoints[index] = g_trainingHighScorePoints[index - 1];
		}
		strcpy(g_trainingHighScoreNames[(int16_t)insertionRank], g_RegisterShellPilot.name);
		g_trainingHighScoreLevels[(int16_t)insertionRank] = g_missionRuntimeState.provingGroundsLevel;
		g_trainingHighScorePoints[(int16_t)insertionRank] = g_missionRuntimeState.provingGroundsScore;
		scoreFile = xfile_Open_File(LANDRU_FILE_ROOT_USER, "X-Wing Data\\train.hgh", "wb");
		if (scoreFile != NULL) {
			for (index = 0, remaining = DEBRIEF_HIGH_SCORE_COUNT; remaining != 0; ++index, --remaining) {
				xfile_Write_Data_To_File(scoreFile, g_trainingHighScoreNames[index],
										 sizeof(g_trainingHighScoreNames[index]));
				xfile_Write_Long_To_File(scoreFile, g_trainingHighScorePoints[index]);
				xfile_Write_Word_To_File(scoreFile, g_trainingHighScoreLevels[index]);
			}
			xfile_Close_File(scoreFile);
		}
	} else if (scene == XW_SCENE_DEBRIEF_COMBAT) {
		int16_t source = shipext_Get_Combat_Source_Index();
		int16_t capacity;
		char filename[DEBRIEF_SCORE_FILENAME_CAPACITY];
		char path[DEBRIEF_SCORE_PATH_CAPACITY];
		LandruHandle handle;
		LandruFile* scoreFile;
		uint16_t recordCount;
		uint16_t kills;
		int index;
		int remaining;
		int16_t missionIndex;
		int16_t rank;
		int insertionRank;
		int16_t qualifies;
		XwMissionHighScores* table;
		XwMissionHighScores* mission;
		if (source >= SHIPEXT_SHIP_COUNT) {
			strcpy(filename, "tourx.hgh");
			capacity = COMBAT_TOUR_SCORE_MISSION_CAPACITY;
			filename[DEBRIEF_TOUR_SCORE_DIGIT] = (char)(source + '1' - SHIPEXT_SHIP_COUNT);
		} else {
			strcpy(filename, "combatx.hgh");
			capacity = COMBAT_SCORE_MISSION_CAPACITY;
			filename[DEBRIEF_COMBAT_SCORE_DIGIT] = (char)(source + '1');
		}
		strcpy(path, "X-Wing Data\\");
		strcat(path, filename);
		handle = xmemhdl_Alloc_Clear_Handle((int16_t)(capacity * sizeof(XwMissionHighScores)),
											LANDRU_MEMORY_RESOURCE);
		scoreFile = xfile_Open_File(LANDRU_FILE_ROOT_USER, path, "rb");
		if (scoreFile != NULL) {
			xfile_Read_Word_From_File(scoreFile, &recordCount);
			if ((int16_t)recordCount > capacity)
				recordCount = 0;
			if (recordCount != 0) {
				void* buffer = xmemhdl_Lock_Handle(handle);
				xres_Resource_Data_To_Buffer(scoreFile, buffer,
											 (int16_t)recordCount * (int)sizeof(XwMissionHighScores));
				xmemhdl_Unlock_Handle(handle);
			}
			xfile_Close_File(scoreFile);
		} else {
			recordCount = 0;
		}
		kills = 0;
		for (index = 0, remaining = sizeof(g_playerFlightState.spacecraftKillsByType) /
									sizeof(g_playerFlightState.spacecraftKillsByType[0]);
			 remaining != 0; ++index, --remaining)
			kills += g_playerFlightState.spacecraftKillsByType[index];
		kills +=
			(uint16_t)(g_playerFlightState.spaceObjectKills + g_playerFlightState.deathStarBuildingKills);
		qualifies = 0;
		missionIndex = 0;
		table = xmemhdl_Lock_Handle(handle);
		for (missionIndex = 0; strcmp(table[missionIndex].missionName, g_shellMissionName) != 0;
			 ++missionIndex) {
			if (table[missionIndex].missionName[0] == '\0')
				break;
			if (missionIndex >= capacity)
				break;
		}
		mission = &table[missionIndex];
		if (mission->missionName[0] == '\0') {
			strcpy(mission->missionName, g_shellMissionName);
			++recordCount;
		}
		if (missionIndex < capacity) {
			insertionRank = 0;
			for (rank = 0; rank < DEBRIEF_HIGH_SCORE_COUNT && qualifies == 0; ++rank) {
				if ((unsigned int)table[missionIndex].entries[rank].score <
					(unsigned int)g_missionRuntimeState.provingGroundsScore) {
					qualifies = 1;
					insertionRank = rank;
				}
			}
			if (qualifies != 0) {
				for (index = DEBRIEF_HIGH_SCORE_COUNT - 1; index > (int16_t)insertionRank; --index) {
					strcpy(mission->entries[index].pilotName, mission->entries[index - 1].pilotName);
					mission->entries[index].score = mission->entries[index - 1].score;
					mission->entries[index].kills = mission->entries[index - 1].kills;
				}
				strcpy(table[missionIndex].entries[(int16_t)insertionRank].pilotName,
					   g_RegisterShellPilot.name);
				table[missionIndex].entries[(int16_t)insertionRank].score =
					g_missionRuntimeState.provingGroundsScore;
				table[missionIndex].entries[(int16_t)insertionRank].kills = (uint16_t)kills;
				scoreFile = xfile_Open_File(LANDRU_FILE_ROOT_USER, path, "wb");
				if (scoreFile != NULL) {
					xfile_Write_Word_To_File(scoreFile, recordCount);
					for (missionIndex = 0; missionIndex < (int16_t)recordCount; ++missionIndex)
						xfile_Write_Data_To_File(scoreFile, &table[missionIndex],
												 sizeof(table[missionIndex]));
					xfile_Close_File(scoreFile);
				}
			}
		}
		xmemhdl_Unlock_Handle(handle);
		xmemhdl_Free_Handle(handle);
	}
}

// FUNCTION: XW 0x445950
int debrief_LoadMissionFlightGroups(void) {
	char resolvedPath[DEBRIEF_MISSION_PATH_CAPACITY];
	XwMissionFlightGroup flightGroup;
	char missionPath[DEBRIEF_MISSION_PATH_CAPACITY];
	XwMissionHeader missionHeader;
	LandruFile* stream;
	int16_t hasPathPrefix;
	int groupIndex = 0;
	int16_t remainingGroups;
	g_debriefLegacyWord = 0;
	g_debriefFlightGroupCount = 0;
	shipext_Get_Mission_Path(missionPath, g_shellMissionName, 1);
	if (missionPath[0] == ';' || missionPath[0] == '+') {
		strcpy(resolvedPath, "X-Wing Data\\");
		strcat(resolvedPath, &missionPath[1]);
		hasPathPrefix = 1;
	} else {
		strcpy(resolvedPath, missionPath);
		/* The original leaves this flag uninitialized for ordinary paths. */
		hasPathPrefix = 0;
	}
	stream = xfile_Open_File(LANDRU_FILE_ROOT_ASSET, resolvedPath, "rb");
	if (stream == NULL) {
		if (hasPathPrefix != 0) {
			if (missionPath[0] == ';') {
				strcpy(resolvedPath, "c:\\XwingCD\\");
				strcat(resolvedPath, &missionPath[1]);
				resolvedPath[0] = (char)g_installDriveLetter;
			} else {
				strcpy(resolvedPath, &missionPath[1]);
			}
			stream = xfile_Open_File(LANDRU_FILE_ROOT_ASSET, resolvedPath, "rb");
		}
	}
	if (stream != NULL) {
		xfile_Read_Data_From_File(stream, &missionHeader, sizeof(missionHeader));
		memcpy(&g_debriefMissionSurfaceWord, &missionHeader.surfaceMode, sizeof(g_debriefMissionSurfaceWord));
		g_debriefFlightGroupCount = missionHeader.flightGroupCount;
		if ((int16_t)missionHeader.flightGroupCount > 0)
			for (remainingGroups = (int16_t)missionHeader.flightGroupCount; remainingGroups != 0;
				 --remainingGroups, ++groupIndex) {
				int16_t characterIndex;
				xfile_Read_Data_From_File(stream, &flightGroup, sizeof(flightGroup));
				strcpy(g_debriefFlightGroupNames[groupIndex], flightGroup.name);
				g_debriefFlightGroupIffOverrides[groupIndex] = flightGroup.iffOverride;
				g_debriefFlightGroupCraftCounts[groupIndex] = flightGroup.numberOfCraft;
				g_debriefFlightGroupGoals[groupIndex] = flightGroup.missionGoal;
				g_debriefFlightGroupCraftTypes[groupIndex] = flightGroup.craftType;
				g_debriefFlightGroupInitialStatus[groupIndex] = flightGroup.initialStatus;
				for (characterIndex = 0; characterIndex < DEBRIEF_FLIGHT_GROUP_NAME_CAPACITY;
					 ++characterIndex) {
					if (g_debriefFlightGroupNames[groupIndex][characterIndex] == 0)
						break;
					g_debriefFlightGroupNames[groupIndex][characterIndex] =
						(toupper)(g_debriefFlightGroupNames[groupIndex][characterIndex]);
				}
			}
		xfile_Close_File(stream);
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x445C30
int16_t debrief_ReloadPilotRecord(void) {
	char pilotFilename[REGISTER_PILOT_PATH_CAPACITY];
	LandruFile* stream;
	strcpy(pilotFilename, g_RegisterShellPilot.name);
	strcat(pilotFilename, ".PLT");
	stream = xfile_Open_File(LANDRU_FILE_ROOT_USER, pilotFilename, "rb");
	if (stream != NULL) {
		register_ReadPilotRecord(stream, &g_debriefPilotRecord);
		xfile_Close_File(stream);
		g_RegisterShellPilot.lost_status = g_debriefPilotRecord.lost_status;
		g_RegisterShellPilot.rank = g_debriefPilotRecord.rank;
		g_RegisterShellPilot.current_tour = g_debriefPilotRecord.current_tour;
		g_RegisterShellPilot.skillValue = g_debriefPilotRecord.skillValue;
		g_RegisterShellPilot.score = g_debriefPilotRecord.score;
		return 1;
	}
	return 0;
}
