#include "xw/frontend/inflight_ui.h"
#ifdef XW_MODERN
#include "xw_dos94/audio/soundext.h"
#endif

#ifdef XW_MODERN
#include "xw_runtime/audio/imuse_session.h"
#endif

#ifdef XW_MODERN
#include "xw_runtime/audio/music_policy.h"
#endif

#include "xw/assets/model_mesh.h"
#include "xw/audio/soundext.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/mission/spec.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/object.h"
#include "xw/flight/player/user.h"
#include "xw/frontend/brief.h"
#include "xw/frontend/player.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include "xw/util/shared.h"
#include "xw_runtime/integration/inflight_callbacks.h"

#ifdef XW_MODERN
#include "xw_runtime/runtime/inflight_music_task.h"
#include "xw_runtime/runtime/inflight_view_task.h"
#endif

#include <landru/actanim.h>
#include <landru/actdelt.h>
#include <landru/actor.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/font.h>
#include <landru/fourcc.h>
#include <landru/inpattr.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/paint.h>
#include <landru/style.h>
#include <landru/view.h>
#include <landru/viewadd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4D4AC4
char g_inflightIconResourceNames[INFLIGHT_ICON_RESOURCE_COUNT][INFLIGHT_ICON_RESOURCE_NAME_CAPACITY] = {
	"iconsg64", "iconsr64", "iconsb64"
};

// GLOBAL: XW 0x4D4C30
char g_inflightViewTitles[INFLIGHT_VIEW_COUNT][INFLIGHT_DETAIL_TEXT_CAPACITY] = { "Inflight Map",
																				  "Inflight Briefing",
																				  "Inflight Damage Control" };

// GLOBAL: XW 0x4D4C98
char g_inflightCraftStatusText[INFLIGHT_CRAFT_STATUS_COUNT][INFLIGHT_CRAFT_STATUS_CAPACITY] = {
	"      OK", " STOPPED", "DISABLED", "CAPTURED", "        ", "  HOMING", "SHLDS DN", "HULL DMG", " WAITING"
};

// GLOBAL: XW 0x4D4CF0
char g_inflightCraftAbbreviations[INFLIGHT_CRAFT_ABBREVIATION_COUNT][INFLIGHT_CRAFT_ABBREVIATION_CAPACITY] = {
	"X-W", "Y-W", "A-W", "T/F", "T/I", "T/B", "GUN", "TRN", "SHU",
	"TUG", "CON", "FRT", "CRS", "FRG", "CRV", "STD", "T/A", "B-W"
};

// GLOBAL: XW 0x4D4E48
int16_t g_inflightMapZoomScales[INFLIGHT_MAP_ZOOM_SCALE_COUNT] = { 4,   5,   6,   7,    8,    9,    10, 11,
																   12,  14,  16,  18,   20,   24,   28, 32,
																   38,  44,  52,  60,   70,   82,   96, 128,
																   192, 256, 512, 1024, 2048, 3072, 0,  0 };

// GLOBAL: XW 0x4D4EA0
int16_t g_inflightMapFocusX[INFLIGHT_MAP_FOCUS_COUNT] = { 52,  128, 128, 200, 200, 200, 280, 280, 280,
														  280, 280, 280, 265, 265, 265, 300, 300, 300,
														  280, 280, 280, 280, 280, 280, 280, 280, 280,
														  280, 280, 280, 54,  126, 202, 285, 285, 285 };

// GLOBAL: XW 0x4D4EE8
int16_t g_inflightMapFocusY[INFLIGHT_MAP_FOCUS_COUNT] = { 8,   8,   8,   8,   8,   8,   105, 105, 105,
														  105, 105, 105, 118, 118, 118, 118, 118, 118,
														  133, 133, 133, 133, 133, 133, 161, 161, 161,
														  161, 161, 161, 188, 188, 188, 192, 192, 192 };

// GLOBAL: XW 0x4D4F30
int16_t g_inflightBriefingFocusX[INFLIGHT_BRIEFING_FOCUS_COUNT] = { 52, 128, 128, 200, 54, 126, 202, 285 };

// GLOBAL: XW 0x4D4F40
int16_t g_inflightBriefingFocusY[INFLIGHT_BRIEFING_FOCUS_COUNT] = { 8, 8, 8, 8, 188, 188, 188, 192 };

// GLOBAL: XW 0x4D4F50
int16_t g_inflightDamageFocusX[INFLIGHT_DAMAGE_FOCUS_COUNT] = {
	52,  128, 128, 200, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123,
	123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 280, 123, 123, 123, 280, 54,  126, 202, 285
};

// GLOBAL: XW 0x4D4FA0
int16_t g_inflightDamageFocusY[INFLIGHT_DAMAGE_FOCUS_COUNT] = {
	8,  8,  8,  8,  50,  50,  50,  50,  62,  62,  62,  62,  74,  74,  74,  74,  86,  86,  86,  86,
	98, 98, 98, 98, 110, 110, 110, 110, 122, 122, 122, 146, 134, 134, 134, 160, 188, 188, 188, 192
};

// GLOBAL: XW 0x4F76A0
XwInflightMusicState g_inflightMusicState = { 0 };

// GLOBAL: XW 0x4F76F8
XwInflightViewMode g_inflightViewMode = INFLIGHT_VIEW_MAP;

// GLOBAL: XW 0x4F77D8
Input* g_inflightShipDetailsInput = NULL;

// GLOBAL: XW 0x4F77DC
Input* g_inflightViewportInput = NULL;

// GLOBAL: XW 0x4F77E8
int16_t g_inflightMapScaleY = 0;

// GLOBAL: XW 0x4F77EC
int16_t g_inflightMapScaleX = 0;

// GLOBAL: XW 0x4F77F0
int16_t g_inflightMapSelectedShipIndex = 0;

// GLOBAL: XW 0x4F77F4
LandruHandle g_inflightMapStateHandle = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4F77F8
Input* g_inflightRootInput = NULL;

// GLOBAL: XW 0x4F77FC
int16_t g_inflightRepairDragActive = 0;

// GLOBAL: XW 0x4F7800
Input* g_inflightPanelInputs[INFLIGHT_VIEW_COUNT] = { NULL, NULL, NULL };

// GLOBAL: XW 0x4F780C
int16_t g_inflightShipDetailsIndexPlusOne = 0;

// GLOBAL: XW 0x4F7860
int16_t g_inflightMapZoomLevel = 0;

// GLOBAL: XW 0x4F7874
int16_t g_inflightMapCenterX = 0;

// GLOBAL: XW 0x4F7878
int16_t g_inflightMapCenterY = 0;

// GLOBAL: XW 0x4F787C
struct BriefingRuntimeState* g_inflightMapState = NULL;

// GLOBAL: XW 0x4F7880
Actor* g_inflightShipDetailsActor = NULL;

// GLOBAL: XW 0x4F7884
Actor* g_inflightBackgroundActor = NULL;

// GLOBAL: XW 0x4F7888
int16_t g_inflightRepairSelectionIndex = 0;

// GLOBAL: XW 0x4F788C
int g_inflightUIMusicContext = 0;

// GLOBAL: XW 0x4F78E0
int16_t g_inflightFocusIndex = 0;

// GLOBAL: XW 0x6285EA
int16_t g_inflightMapPreferencesInitialized = 0;

// FUNCTION: XW 0x44DF60
void InflightUI_OpenMusic(ResFile* unusedResourceFile, int unusedSceneContext) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		const char* musicName;
		g_inflightMusicState.previousMusic = NULL;
		switch (shellext_Get_Cur_Scene()) {
			case XW_SCENE_BRIEFING_COMBAT: {
				ResFile* transitionResource;
				musicName = "patrol";
				transitionResource = xres_Open_Resource("ifmusic.lfd");
				g_inflightMusicState.music = xsound_Res_Music(transitionResource, musicName);
				xres_Close_Resource(transitionResource);
				g_inflightMusicState.previousMusic = xsound_Find_Gmid("adrift");
				if (g_inflightMusicState.previousMusic != NULL) {
#ifdef XW_MODERN
					if (XwMusicPolicy_UsesImuse()) {
#endif
						Sound* currentMusic;
						Sound* previousMusic;
						int previousGroup;
						unsigned int previousBeat;
						int previousTick;
#ifndef XW_MODERN
						int currentTick;
#endif
						soundext_Start_Resource_Sound(g_inflightMusicState.music);
#ifdef XW_MODERN
						Dos94_soundext_SetVolume(g_inflightMusicState.music, INFLIGHT_MUSIC_OPEN_VOLUME);
#else
					soundext_SetVolume(0, INFLIGHT_MUSIC_OPEN_VOLUME);
#endif
						previousMusic = g_inflightMusicState.previousMusic;
						currentMusic = g_inflightMusicState.music;
						soundext_ShareParts(previousMusic, currentMusic);
#ifdef XW_MODERN
						if (XwInflightMusic_WaitForTick(unusedSceneContext))
							return;
#else
					do {
						currentTick =
							soundext_GetMusicParam(g_inflightMusicState.music, XW_SOUND_QUERY_TICK, 0);
					} while (currentTick == 0);
#endif
						j_lolevel_ImPause();
						previousGroup = soundext_GetMusicParam(g_inflightMusicState.previousMusic,
															   XW_SOUND_QUERY_CHUNK, 0);
						previousBeat = soundext_GetMusicParam(g_inflightMusicState.previousMusic,
															  XW_SOUND_QUERY_BEAT, 0);
						previousTick = soundext_GetMusicParam(g_inflightMusicState.previousMusic,
															  XW_SOUND_QUERY_TICK, 0);
						j_lolevel_ImResume();
						soundext_ScanMidi(g_inflightMusicState.music, previousGroup, previousBeat,
										  previousTick);
						soundext_JumpMidi(g_inflightMusicState.previousMusic, INFLIGHT_MUSIC_PREVIOUS_GROUP,
										  0, 0);
#ifdef XW_MODERN
					}
#endif
				} else {
					g_inflightMusicState.previousMusic = xsound_Find_Gmid("mission");
					if (g_inflightMusicState.previousMusic != NULL) {
						soundext_ClearTriggers();
						soundext_SetPartEnabled(g_inflightMusicState.previousMusic,
												INFLIGHT_MUSIC_MISSION_CHANNEL, 0);
						soundext_SetTriggerContext(g_inflightMusicState.previousMusic,
												   INFLIGHT_MUSIC_TRANSITION_MARKER);
						soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_START, g_inflightMusicState.music->id,
													 0, 0, 0, 0, 0);
						soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_SET_VOLUME,
													 g_inflightMusicState.music->id,
													 INFLIGHT_MUSIC_OPEN_VOLUME, 0, 0, 0, 0);
						soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_END, 0, 0, 0, 0, 0, 0);
						soundext_SetTriggerContext(g_inflightMusicState.previousMusic,
												   INFLIGHT_MUSIC_TRANSITION_MARKER);
						soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_STOP,
													 g_inflightMusicState.previousMusic->id, 0, 0, 0, 0, 0);
						soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_END, 0, 0, 0, 0, 0, 0);
					}
				}
				break;
			}
			case XW_SCENE_BRIEFING_TOUR:
				musicName = "wrkmarch";
				break;
			default:
				musicName = "patrol";
				break;
		}
		g_inflightMusicState.unusedSceneContext = unusedSceneContext;
		g_inflightMusicState.music = xsound_Find_Gmid(musicName);
		g_inflightMusicState.legacyLevel = INFLIGHT_MUSIC_INITIAL_LEVEL;
		if (g_inflightMusicState.music == NULL) {
			ResFile* musicResource = xres_Open_Resource("ifmusic.lfd");
			g_inflightMusicState.music = xsound_Res_Music(musicResource, musicName);
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_inflightMusicState.music);
			soundext_FadeVolume(g_inflightMusicState.music, INFLIGHT_MUSIC_OPEN_VOLUME,
								INFLIGHT_MUSIC_OPEN_DURATION);
		}
		xsound_Set_Sound_Keep(g_inflightMusicState.music);
		if (g_inflightMusicState.previousMusic != NULL)
			xsound_Set_Sound_Keep(g_inflightMusicState.previousMusic);
		xsound_Set_Sound_User_Function(g_inflightMusicState.music, InflightUI_user_Music);
	}
}

// FUNCTION: XW 0x44E240
void InflightUI_CloseMusic(void) {
	int exitScene;
	if (ShellPreferences_GetMusicEnabled() == 0)
		return;
	soundext_ClearTriggers();
	exitScene = xerror_Get_Landru_Exit();
	if (exitScene == XW_SCENE_COMBAT_SIMULATOR_ROOM && g_inflightMusicState.previousMusic != NULL &&
		xsound_Find_Gmid("mission") == g_inflightMusicState.previousMusic &&
		soundext_Count_Resource_Instances(g_inflightMusicState.previousMusic) == 1 &&
		soundext_Count_Resource_Instances(g_inflightMusicState.music) != 1)
		return;
	switch (exitScene) {
		case XW_SCENE_REQUEST_COMBAT_LAUNCH:
			if (xsound_Find_Gmid("mission") == g_inflightMusicState.previousMusic)
				soundext_FadeVolume(g_inflightMusicState.previousMusic, 0, INFLIGHT_MUSIC_CLOSE_DURATION);
			break;
		case XW_SCENE_REQUEST_TOUR_LAUNCH:
			if (g_inflightMusicState.previousMusic != NULL)
				soundext_FadeVolume(g_inflightMusicState.previousMusic, 0, INFLIGHT_MUSIC_CLOSE_DURATION);
			soundext_FadeVolume(g_inflightMusicState.music, 0, INFLIGHT_MUSIC_TOUR_CLOSE_DURATION);
			break;
		case XW_SCENE_COMBAT_SIMULATOR_ROOM:
		case XW_SCENE_TOUR_DESK:
#ifdef XW_MODERN
			Dos94_soundext_SetPriority(g_inflightMusicState.music, 0);
#else
			soundext_SetPriority(0, 0);
#endif
			soundext_FadeVolume(g_inflightMusicState.music, 0, INFLIGHT_MUSIC_CLOSE_DURATION);
			/* Fall through to release the previous music. */
		default:
			if (g_inflightMusicState.previousMusic != NULL) {
				xsound_Clear_Sound_Keep(g_inflightMusicState.previousMusic);
				xsound_Free_Sound(g_inflightMusicState.previousMusic);
			}
			break;
	}
}

// FUNCTION: XW 0x44E3C0
void InflightUI_user_Music(Sound* sound, int time) {
	(void)sound;
	(void)time;
	if (g_inflightMusicState.previousMusic == NULL) {
		return;
	}
	if (shellext_Get_Cur_Scene() != XW_SCENE_BRIEFING_COMBAT) {
		return;
	}
	if (soundext_Count_Resource_Instances(g_inflightMusicState.previousMusic) == 1) {
		int musicPosition =
			soundext_GetMusicParam(g_inflightMusicState.previousMusic, XW_SOUND_QUERY_BEAT, 0);
		if (musicPosition > XW_BRIEF_MUSIC_FADE_POSITION &&
			g_inflightMusicState.legacyLevel > XW_BRIEF_MUSIC_MIN_LEVEL) {
#ifdef XW_MODERN
			Dos94_soundext_SetGroup(g_inflightMusicState.previousMusic, --g_inflightMusicState.legacyLevel);
#else
			soundext_SetGroup(0, --g_inflightMusicState.legacyLevel);
#endif
			return;
		}
		if (musicPosition > XW_BRIEF_MUSIC_POSITION_4) {
			soundext_SetHook(g_inflightMusicState.previousMusic, XW_SOUND_CONTROL_DIRECT,
							 XW_BRIEF_MUSIC_CONTROL_4, 0);
		} else if (musicPosition > XW_BRIEF_MUSIC_POSITION_3) {
			soundext_SetHook(g_inflightMusicState.previousMusic, XW_SOUND_CONTROL_DIRECT,
							 XW_BRIEF_MUSIC_CONTROL_3, 0);
		} else if (musicPosition > XW_BRIEF_MUSIC_POSITION_2) {
			soundext_SetHook(g_inflightMusicState.previousMusic, XW_SOUND_CONTROL_DIRECT,
							 XW_BRIEF_MUSIC_CONTROL_2, 0);
		}
	}
}

// FUNCTION: XW 0x4501E0
XwShellSceneResult InflightUI_Show(struct XwShellContext* context) {
	ResFile* resource;
	Film* film;
	int16_t objectIndex;
	int iconIndex;
	int panelIndex;
	Input* titleLabel;
	Input* selectionLabel;
	PushButton* button;
	Rect controlRect;
	Rect viewportRect;
	g_inflightMapSelectedShipIndex = INFLIGHT_SELECTION_CLEAR;
	g_inflightShipDetailsIndexPlusOne = 0;
	g_inflightRepairDragActive = 0;
	g_inflightViewMode = shellext_Get_Cur_Scene() - XW_SCENE_INFLIGHT_MAP;
	if (g_inflightMapPreferencesInitialized == 0) {
		g_inflightMapPreferencesInitialized = 1;
		g_inflightRepairSelectionIndex = 0;
		g_inflightMapZoomLevel = INFLIGHT_INITIAL_ZOOM_LEVEL;
		g_inflightMapScaleX = INFLIGHT_INITIAL_MAP_SCALE;
		g_inflightMapScaleY = INFLIGHT_INITIAL_MAP_SCALE;
		g_inflightMapCenterX = 0;
		g_inflightMapCenterY = 0;
	}
	g_inflightFocusIndex = INFLIGHT_INITIAL_FOCUS;
	xio_Set_Mouse_Position(INFLIGHT_MAP_PAN_MOUSE_X, INFLIGHT_MAP_PAN_MOUSE_Y);
	g_inflightMapStateHandle =
		xmemhdl_Alloc_Clear_Handle(sizeof(*g_inflightMapState), LANDRU_MEMORY_RESOURCE);
	g_inflightMapState = xmemhdl_Lock_Handle(g_inflightMapStateHandle);
	nullsub_SharedNoOp();
	Shared_ReturnOne();
	player_Init_Display_Map();
	player_LoadMissionRecords(g_shellMissionName);
	player_Load_Display_Map(g_shellMissionName);
	player_Rewind_Page(g_inflightMapState->activeScriptIndex);
	g_inflightMapShipCount = 0;
	for (objectIndex = 0; objectIndex < PLAYER_MAP_CRAFT_LIMIT; ++objectIndex) {
		if (player_IsObjectVisible(objectIndex) != 0)
			++g_inflightMapShipCount;
	}
	player_SelectPlayerShip();
	player_OrderPendingRepairsFirst();
	resource = xres_Open_Resource(":X-Wing Data\\RESOURCE\\inflight.lfd");
	if (resource == NULL)
		resource = xres_Open_Resource("inflight.lfd");
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xrect_Set_Rect(&controlRect, 0, 0, INFLIGHT_WIDTH, INFLIGHT_HEIGHT);
	film = xfilm_Res_Film(resource, "inflight", &controlRect, 0, 0, 0);
	xfilm_Set_Film_Def_Palette(film, context->standardPalette);
	g_inflightBackgroundActor = xactor_Find_Actor(FOURCC_DELT, "inflight");
	xactor_Non_Refreshable_Actor(g_inflightBackgroundActor);
	for (iconIndex = 0; iconIndex != PLAYER_MAP_ICON_ACTOR_COUNT; ++iconIndex) {
		g_inflightMapIconActors[iconIndex] =
			xactanim_Res_Anim_Actor(resource, g_inflightIconResourceNames[iconIndex], &controlRect, 0, 0, 0);
		xactor_Set_Actor_Time(g_inflightMapIconActors[iconIndex], 0, 0);
	}
	g_inflightShipDetailsActor = xactanim_Res_Anim_Actor(resource, "radar640", &controlRect, 0, 0, 0);
	xactor_Set_Actor_Time(g_inflightShipDetailsActor, 0, 0);
	if (Shared_ReturnZero() == 0) {
		g_inflightMapStarsActor = xactdelt_Res_Delta_Actor(resource, "stars640", &controlRect, 0, 0, 0);
		xactor_Set_Actor_Time(g_inflightMapStarsActor, 0, 0);
	} else
		g_inflightMapStarsActor = NULL;
	xrect_Set_Rect(&controlRect, 0, 0, INFLIGHT_WIDTH, INFLIGHT_HEIGHT);
	g_inflightRootInput = xinput_Alloc_Input(NULL, &controlRect, 0, 0);
	xrect_Set_Rect(&viewportRect, INFLIGHT_VIEWPORT_LEFT, INFLIGHT_VIEWPORT_TOP, INFLIGHT_VIEWPORT_RIGHT,
				   INFLIGHT_VIEWPORT_BOTTOM);
	g_inflightViewportInput = xinput_Alloc_Input(g_inflightRootInput, &viewportRect, 0, 0);
	xinpattr_Set_Input_Update_Function(g_inflightViewportInput, InflightUI_UpdateViewport);
	xinpattr_Set_Input_User_Function(g_inflightViewportInput, XwInflight_ApplyShipSelection);
	xinpattr_Set_Input_Draw_Function(g_inflightViewportInput, InflightUI_DrawViewport);
	g_inflightViewportInput->mouseUsage = downMoveUpInput;
	xrect_Set_Rect(&controlRect, INFLIGHT_TITLE_LEFT, INFLIGHT_TITLE_TOP, INFLIGHT_TITLE_RIGHT,
				   INFLIGHT_TITLE_BOTTOM);
	titleLabel = xinput_Alloc_Input(g_inflightRootInput, &controlRect, 0, 0);
	xinpattr_Set_Input_Draw_Function(titleLabel, InflightUI_DrawLabel);
	titleLabel->id = INFLIGHT_LABEL_TITLE;
	xrect_Set_Rect(&controlRect, INFLIGHT_PREVIOUS_VIEW_LEFT, INFLIGHT_PREVIOUS_VIEW_TOP,
				   INFLIGHT_PREVIOUS_VIEW_RIGHT, INFLIGHT_PREVIOUS_VIEW_BOTTOM);
	button = xbtnpush_Alloc_Button(g_inflightRootInput, &controlRect, 0, XwInflight_HandleButton, NULL,
								   INFLIGHT_BUTTON_PREVIOUS_VIEW);
	xinpattr_Set_Input_Draw_Function(&button->header, InflightUI_DrawLabel);
	xrect_Set_Rect(&controlRect, INFLIGHT_NEXT_VIEW_LEFT, INFLIGHT_NEXT_VIEW_TOP, INFLIGHT_NEXT_VIEW_RIGHT,
				   INFLIGHT_NEXT_VIEW_BOTTOM);
	button = xbtnpush_Alloc_Button(g_inflightRootInput, &controlRect, 0, XwInflight_HandleButton, NULL,
								   INFLIGHT_BUTTON_NEXT_VIEW);
	xinpattr_Set_Input_Draw_Function(&button->header, InflightUI_DrawLabel);
	xrect_Set_Rect(&controlRect, INFLIGHT_EXIT_LEFT, INFLIGHT_EXIT_TOP, INFLIGHT_EXIT_RIGHT,
				   INFLIGHT_EXIT_BOTTOM);
	button = xbtnpush_Alloc_Button(g_inflightRootInput, &controlRect, 0, XwInflight_HandleButton, "Exit",
								   INFLIGHT_BUTTON_RESUME);
	xinpattr_Set_Input_Draw_Function(&button->header, XwInflight_DrawTextButton);
	xrect_Set_Rect(&controlRect, INFLIGHT_PANEL_LEFT, INFLIGHT_PANEL_TOP, INFLIGHT_PANEL_RIGHT,
				   INFLIGHT_PANEL_BOTTOM);
	for (panelIndex = 0; panelIndex != INFLIGHT_VIEW_COUNT; ++panelIndex) {
		g_inflightPanelInputs[panelIndex] = xinput_Alloc_Input(g_inflightRootInput, &controlRect, 0, 0);
		xinpattr_Hide_Input(g_inflightPanelInputs[panelIndex]);
	}
	xrect_Set_Rect(&controlRect, INFLIGHT_SHIP_DETAILS_LEFT, INFLIGHT_SHIP_DETAILS_TOP,
				   INFLIGHT_SHIP_DETAILS_RIGHT, INFLIGHT_SHIP_DETAILS_BOTTOM);
	xrect_Offset_Rect(&controlRect, -INFLIGHT_PANEL_LEFT, -INFLIGHT_PANEL_TOP);
	g_inflightShipDetailsInput =
		xinput_Alloc_Input(g_inflightPanelInputs[INFLIGHT_VIEW_MAP], &controlRect, 0, 0);
	xinpattr_Set_Input_Draw_Function(g_inflightShipDetailsInput, InflightUI_DrawLabel);
	g_inflightShipDetailsInput->id = INFLIGHT_LABEL_SHIP_DETAILS;
	xrect_Set_Rect(&controlRect, INFLIGHT_PAN_UP_LEFT, INFLIGHT_PAN_UP_TOP, INFLIGHT_PAN_UP_RIGHT,
				   INFLIGHT_PAN_UP_BOTTOM);
	xrect_Offset_Rect(&controlRect, -INFLIGHT_PANEL_LEFT, -INFLIGHT_PANEL_TOP);
	button = xbtnpush_Alloc_Button(g_inflightPanelInputs[INFLIGHT_VIEW_MAP], &controlRect, 0,
								   XwInflight_HandleButton, "Up", INFLIGHT_BUTTON_PAN_UP);
	xinpattr_Set_Input_Update_Function(&button->header, XwInflight_UpdateRepeatingButton);
	xinpattr_Set_Input_Draw_Function(&button->header, XwInflight_DrawTextButton);
	xrect_Set_Rect(&controlRect, INFLIGHT_PAN_LEFT_LEFT, INFLIGHT_PAN_LEFT_TOP, INFLIGHT_PAN_LEFT_RIGHT,
				   INFLIGHT_PAN_LEFT_BOTTOM);
	xrect_Offset_Rect(&controlRect, -INFLIGHT_PANEL_LEFT, -INFLIGHT_PANEL_TOP);
	button = xbtnpush_Alloc_Button(g_inflightPanelInputs[INFLIGHT_VIEW_MAP], &controlRect, 0,
								   XwInflight_HandleButton, "Left", INFLIGHT_BUTTON_PAN_LEFT);
	xinpattr_Set_Input_Update_Function(&button->header, XwInflight_UpdateRepeatingButton);
	xinpattr_Set_Input_Draw_Function(&button->header, XwInflight_DrawTextButton);
	xrect_Set_Rect(&controlRect, INFLIGHT_PAN_RIGHT_LEFT, INFLIGHT_PAN_RIGHT_TOP, INFLIGHT_PAN_RIGHT_RIGHT,
				   INFLIGHT_PAN_RIGHT_BOTTOM);
	xrect_Offset_Rect(&controlRect, -INFLIGHT_PANEL_LEFT, -INFLIGHT_PANEL_TOP);
	button = xbtnpush_Alloc_Button(g_inflightPanelInputs[INFLIGHT_VIEW_MAP], &controlRect, 0,
								   XwInflight_HandleButton, "Right", INFLIGHT_BUTTON_PAN_RIGHT);
	xinpattr_Set_Input_Update_Function(&button->header, XwInflight_UpdateRepeatingButton);
	xinpattr_Set_Input_Draw_Function(&button->header, XwInflight_DrawTextButton);
	xrect_Set_Rect(&controlRect, INFLIGHT_PAN_DOWN_LEFT, INFLIGHT_PAN_DOWN_TOP, INFLIGHT_PAN_DOWN_RIGHT,
				   INFLIGHT_PAN_DOWN_BOTTOM);
	xrect_Offset_Rect(&controlRect, -INFLIGHT_PANEL_LEFT, -INFLIGHT_PANEL_TOP);
	button = xbtnpush_Alloc_Button(g_inflightPanelInputs[INFLIGHT_VIEW_MAP], &controlRect, 0,
								   XwInflight_HandleButton, "Down", INFLIGHT_BUTTON_PAN_DOWN);
	xinpattr_Set_Input_Update_Function(&button->header, XwInflight_UpdateRepeatingButton);
	xinpattr_Set_Input_Draw_Function(&button->header, XwInflight_DrawTextButton);
	xrect_Set_Rect(&controlRect, INFLIGHT_ZOOM_LEFT, INFLIGHT_ZOOM_TOP, INFLIGHT_ZOOM_RIGHT,
				   INFLIGHT_ZOOM_BOTTOM);
	xrect_Offset_Rect(&controlRect, -INFLIGHT_PANEL_LEFT, -INFLIGHT_PANEL_TOP);
	button = xbtnpush_Alloc_Button(g_inflightPanelInputs[INFLIGHT_VIEW_MAP], &controlRect, 0,
								   XwInflight_HandleButton, "Zoom", INFLIGHT_BUTTON_ZOOM);
	xinpattr_Set_Input_Update_Function(&button->header, XwInflight_UpdateRepeatingButton);
	xinpattr_Set_Input_Draw_Function(&button->header, XwInflight_DrawTextButton);
	xrect_Set_Rect(&controlRect, INFLIGHT_PAN_UP_LEFT, INFLIGHT_PAN_UP_TOP, INFLIGHT_PAN_UP_RIGHT,
				   INFLIGHT_PAN_UP_BOTTOM);
	xrect_Offset_Rect(&controlRect, -INFLIGHT_PANEL_LEFT, -INFLIGHT_PANEL_TOP);
	button = xbtnpush_Alloc_Button(g_inflightPanelInputs[INFLIGHT_VIEW_DAMAGE_CONTROL], &controlRect, 0,
								   XwInflight_HandleButton, "Sooner", INFLIGHT_BUTTON_REPAIR_EARLIER);
	xinpattr_Set_Input_Draw_Function(&button->header, XwInflight_DrawTextButton);
	xrect_Set_Rect(&controlRect, INFLIGHT_PAN_DOWN_LEFT, INFLIGHT_PAN_DOWN_TOP, INFLIGHT_PAN_DOWN_RIGHT,
				   INFLIGHT_PAN_DOWN_BOTTOM);
	xrect_Offset_Rect(&controlRect, -INFLIGHT_PANEL_LEFT, -INFLIGHT_PANEL_TOP);
	button = xbtnpush_Alloc_Button(g_inflightPanelInputs[INFLIGHT_VIEW_DAMAGE_CONTROL], &controlRect, 0,
								   XwInflight_HandleButton, "Later", INFLIGHT_BUTTON_REPAIR_LATER);
	xinpattr_Set_Input_Draw_Function(&button->header, XwInflight_DrawTextButton);
	xrect_Set_Rect(&controlRect, INFLIGHT_SELECTION_LEFT, INFLIGHT_SELECTION_TOP, INFLIGHT_SELECTION_RIGHT,
				   INFLIGHT_SELECTION_BOTTOM);
	selectionLabel = xinput_Alloc_Input(g_inflightRootInput, &controlRect, 0, 0);
	xinpattr_Set_Input_Update_Function(selectionLabel, InflightUI_UpdateSelectionLabel);
	xinpattr_Set_Input_Draw_Function(selectionLabel, InflightUI_DrawLabel);
	xinpattr_Set_Input_Allign(selectionLabel, INFLIGHT_ALIGN_NEAR, INFLIGHT_ALIGN_FAR);
	selectionLabel->id = INFLIGHT_LABEL_CENTER_SELECTED;
	xrect_Set_Rect(&controlRect, INFLIGHT_PREVIOUS_ITEM_LEFT, INFLIGHT_PREVIOUS_ITEM_TOP,
				   INFLIGHT_PREVIOUS_ITEM_RIGHT, INFLIGHT_PREVIOUS_ITEM_BOTTOM);
	button = xbtnpush_Alloc_Button(g_inflightRootInput, &controlRect, 0, XwInflight_HandleButton, NULL,
								   INFLIGHT_BUTTON_PREVIOUS_ITEM);
	xinpattr_Set_Input_Draw_Function(&button->header, InflightUI_DrawLabel);
	xinpattr_Set_Input_Allign(&button->header, INFLIGHT_ALIGN_NEAR, INFLIGHT_ALIGN_FAR);
	xrect_Set_Rect(&controlRect, INFLIGHT_NEXT_ITEM_LEFT, INFLIGHT_NEXT_ITEM_TOP, INFLIGHT_NEXT_ITEM_RIGHT,
				   INFLIGHT_NEXT_ITEM_BOTTOM);
	button = xbtnpush_Alloc_Button(g_inflightRootInput, &controlRect, 0, XwInflight_HandleButton, NULL,
								   INFLIGHT_BUTTON_NEXT_ITEM);
	xinpattr_Set_Input_Draw_Function(&button->header, InflightUI_DrawLabel);
	xinpattr_Set_Input_Allign(&button->header, INFLIGHT_ALIGN_NEAR, INFLIGHT_ALIGN_FAR);
	xinpattr_Show_Input(g_inflightPanelInputs[g_inflightViewMode]);
	if (g_inflightViewMode < INFLIGHT_VIEW_DAMAGE_CONTROL)
		InflightUI_SelectBriefingPage(g_inflightViewMode);
	xio_Set_Key_Buttons();
	xview_Set_View_Update_Function(InflightUI_UpdateView);
#ifdef XW_MODERN
	XwInflight_RunView(resource);
#else
	InflightUI_OpenMusic(resource, g_inflightUIMusicContext);
	soundext_LoadCommonUiSounds();
	j_xviewadd_Handle_View();
	InflightUI_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xio_Clear_Key_Buttons();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	if ((uint16_t)xcursor_Is_Cursor_Visible())
		xcursor_Hide_Cursor();
	xres_Close_Resource(resource);
	player_Free_Display_Map();
	nullsub_SharedNoOp();
	xmemhdl_Free_Handle(g_inflightMapStateHandle);
	if (xerror_Get_Landru_Exit() != 0)
		shellext_Sudden_Scene_Fade();
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x450BA0
void InflightUI_UpdateView(int time) {
	int16_t key;
	if (time == 0 && (uint16_t)xcursor_Is_Cursor_Visible() == 0)
		xcursor_Show_Cursor();
	key = xio_Get_Free_Key();
	if (key != 0) {
		switch (g_inflightViewMode) {
			case INFLIGHT_VIEW_MAP:
				if (shellext_MoveGridFocus(&g_inflightFocusIndex, g_inflightMapFocusX, g_inflightMapFocusY,
										   INFLIGHT_MAP_FOCUS_ROWS, INFLIGHT_MAP_FOCUS_COLUMNS, key) != 0) {
					xio_Set_Mouse_Position(g_inflightMapFocusX[g_inflightFocusIndex],
										   g_inflightMapFocusY[g_inflightFocusIndex]);
					xio_Get_Key();
				}
				break;
			case INFLIGHT_VIEW_BRIEFING:
				if (shellext_MoveGridFocus(&g_inflightFocusIndex, g_inflightBriefingFocusX,
										   g_inflightBriefingFocusY, INFLIGHT_BRIEFING_FOCUS_ROWS,
										   INFLIGHT_BRIEFING_FOCUS_COLUMNS, key) != 0) {
					xio_Set_Mouse_Position(g_inflightBriefingFocusX[g_inflightFocusIndex],
										   g_inflightBriefingFocusY[g_inflightFocusIndex]);
					xio_Get_Key();
				}
				break;
			case INFLIGHT_VIEW_DAMAGE_CONTROL:
				if (shellext_MoveGridFocus(&g_inflightFocusIndex, g_inflightDamageFocusX,
										   g_inflightDamageFocusY, INFLIGHT_DAMAGE_FOCUS_ROWS,
										   INFLIGHT_DAMAGE_FOCUS_COLUMNS, key) != 0) {
					xio_Set_Mouse_Position(g_inflightDamageFocusX[g_inflightFocusIndex],
										   g_inflightDamageFocusY[g_inflightFocusIndex]);
					xio_Get_Key();
				}
				break;
		}
	}
}

// FUNCTION: XW 0x450CC0
void InflightUI_RelockState(int16_t relock) {
	if (relock != 0) {
		g_inflightMapState = xmemhdl_Lock_Handle(g_inflightMapStateHandle);
		xmemhdl_Unlock_Handle(g_inflightMapStateHandle);
	}
}

// FUNCTION: XW 0x450CF0
int16_t InflightUI_UpdateViewport(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
								  int rightEvent, int16_t x, int16_t y) {
	Rect localViewport;
	int16_t selectedIndex;
	int16_t handled;
	int16_t mouseY;
	(void)clip;
	if (key != 0)
		return 0;
	mouseY = y;
	if (g_inflightViewMode == INFLIGHT_VIEW_MAP) {
		if (leftEvent == INFLIGHT_MOUSE_PRESS) {
			xrect_Copy_Rect(&localViewport, frame);
			xrect_Origin_Rect(&localViewport);
			if ((uint16_t)player_Find_Ship_On_Screen(&localViewport, x, mouseY, &selectedIndex))
				input->var1 = selectedIndex + 1;
			else
				input->var1 = INFLIGHT_SELECTION_CLEAR;
		}
		if (rightEvent != 0) {
			switch (rightEvent) {
				case INFLIGHT_MOUSE_PRESS:
					xcursor_Disable_Cursor();
					break;
				case INFLIGHT_MOUSE_HOLD: {
					int16_t mapX, mapY;
					player_Screen_To_Map_Pos(frame, (int16_t)(x + frame->left),
											 (int16_t)(frame->top + mouseY), &mapX, &mapY);
					g_inflightMapCenterX = mapX;
					g_inflightMapCenterY = mapY;
					xio_Set_Mouse_Position(INFLIGHT_MAP_PAN_MOUSE_X, INFLIGHT_MAP_PAN_MOUSE_Y);
					xinpattr_Refresh_Input(input);
					break;
				}
				case INFLIGHT_MOUSE_RELEASE:
					if (!xcursor_Is_Cursor_Visible())
						xcursor_Enable_Cursor();
					break;
			}
		}
		handled = 1;
	} else
		handled = key;
	if (g_inflightViewMode == INFLIGHT_VIEW_DAMAGE_CONTROL) {
		int mouseEvent = leftEvent;
		handled = 1;
		if (mouseEvent == 0)
			mouseEvent = rightEvent;
		switch (mouseEvent) {
			case INFLIGHT_MOUSE_PRESS:
				if (mouseY >= INFLIGHT_REPAIR_ROWS_TOP && mouseY < INFLIGHT_REPAIR_ROWS_BOTTOM) {
					g_inflightRepairDragActive = 1;
					g_inflightRepairSelectionIndex =
						(mouseY - INFLIGHT_REPAIR_ROWS_TOP) / INFLIGHT_REPAIR_ROW_HEIGHT;
					xinput_Refresh_System_Inputs();
				} else
					handled = 0;
				break;
			case INFLIGHT_MOUSE_HOLD: {
				int16_t targetRow = (mouseY - INFLIGHT_REPAIR_ROWS_TOP) / INFLIGHT_REPAIR_ROW_HEIGHT;
				int16_t previousRow;
				if (targetRow < 0)
					targetRow = 0;
				if (targetRow >= XW_PLAYER_SUBSYSTEM_COUNT)
					targetRow = XW_PLAYER_SUBSYSTEM_COUNT - 1;
				previousRow = g_inflightRepairSelectionIndex;
				if (targetRow < previousRow) {
					int16_t movedSubsystem = g_playerSubsystemRepairPriority[previousRow];
					int16_t row;
					for (row = g_inflightRepairSelectionIndex - 1; row >= targetRow; --row)
						g_playerSubsystemRepairPriority[row + 1] = g_playerSubsystemRepairPriority[row];
					g_playerSubsystemRepairPriority[targetRow] = movedSubsystem;
				} else if (targetRow > previousRow) {
					int16_t movedSubsystem = g_playerSubsystemRepairPriority[previousRow];
					int16_t row;
					for (row = g_inflightRepairSelectionIndex + 1; row <= targetRow; ++row)
						g_playerSubsystemRepairPriority[row - 1] = g_playerSubsystemRepairPriority[row];
					g_playerSubsystemRepairPriority[targetRow] = movedSubsystem;
				}
				if (targetRow != previousRow) {
					g_inflightRepairSelectionIndex = targetRow;
					xinput_Refresh_System_Inputs();
				}
				break;
			}
			case INFLIGHT_MOUSE_RELEASE:
				g_inflightRepairDragActive = 0;
				xinput_Refresh_System_Inputs();
				break;
		}
	}
	return handled;
}

// FUNCTION: XW 0x450F80
void InflightUI_ApplyShipSelection(Input* input) {
	if (g_inflightViewMode == INFLIGHT_VIEW_MAP) {
		int16_t selectionRequest = input->var1;
		if (selectionRequest != INFLIGHT_SELECTION_IDLE) {
			if (selectionRequest == INFLIGHT_SELECTION_CLEAR) {
				g_inflightMapSelectedShipIndex = INFLIGHT_SELECTION_CLEAR;
				g_inflightShipDetailsIndexPlusOne = INFLIGHT_SELECTION_IDLE;
			} else {
				g_inflightMapSelectedShipIndex = selectionRequest - 1;
				g_inflightShipDetailsIndexPlusOne = input->var1;
			}
			input->var1 = INFLIGHT_SELECTION_IDLE;
			xinput_Refresh_System_Inputs();
		}
	}
}

// FUNCTION: XW 0x450FD0
void InflightUI_DrawViewport(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	(void)input;
	if (refresh != 0) {
		switch (g_inflightViewMode) {
			case INFLIGHT_VIEW_MAP:
				player_DrawInflightMap(frame, clip, refresh);
				break;
			case INFLIGHT_VIEW_BRIEFING:
				player_DrawBriefingText(frame, clip, refresh);
				break;
			case INFLIGHT_VIEW_DAMAGE_CONTROL:
				player_DrawDamageControl(frame, clip, refresh);
				break;
		}
	}
}

// FUNCTION: XW 0x451030
int16_t InflightUI_UpdateSelectionLabel(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
										int rightEvent, int16_t x, int16_t y) {
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (key != 0)
		return 0;
	if (g_inflightViewMode == INFLIGHT_VIEW_MAP && input->id == INFLIGHT_LABEL_CENTER_SELECTED &&
		(leftEvent == INFLIGHT_MOUSE_PRESS || rightEvent == INFLIGHT_MOUSE_PRESS) &&
		g_inflightMapSelectedShipIndex != INFLIGHT_SELECTION_CLEAR) {
		int objectIndex = player_VisibleIndexToObject(g_inflightMapSelectedShipIndex);
		g_inflightMapCenterX = g_objectTable[objectIndex].worldX / INFLIGHT_MAP_WORLD_UNITS_PER_PIXEL;
		g_inflightMapCenterY = g_objectTable[objectIndex].worldY / -INFLIGHT_MAP_WORLD_UNITS_PER_PIXEL;
		xinpattr_Refresh_Input(g_inflightViewportInput);
	}
	return 1;
}

// FUNCTION: XW 0x4510D0
void InflightUI_HandleButton(Input* input) {
	if (xinpattr_Get_Input_Selected(input) != 0) {
		int16_t panNumerator = input->var2 != 0 ? INFLIGHT_MAP_FAST_PAN_STEP : INFLIGHT_MAP_PAN_STEP;
		switch (input->id) {
			case INFLIGHT_BUTTON_PREVIOUS_VIEW:
				xinpattr_Hide_Input(g_inflightPanelInputs[g_inflightViewMode]);
				if (g_inflightViewMode == INFLIGHT_VIEW_MAP)
					g_inflightViewMode = INFLIGHT_VIEW_DAMAGE_CONTROL;
				else
					--g_inflightViewMode;
				xinpattr_Show_Input(g_inflightPanelInputs[g_inflightViewMode]);
				switch (g_inflightViewMode) {
					case INFLIGHT_VIEW_MAP:
						InflightUI_SelectBriefingPage(0);
						break;
					case INFLIGHT_VIEW_BRIEFING:
						InflightUI_SelectBriefingPage(1);
						break;
				}
				xview_Refresh_View();
				break;
			case INFLIGHT_BUTTON_NEXT_VIEW:
				xinpattr_Hide_Input(g_inflightPanelInputs[g_inflightViewMode]);
				if (g_inflightViewMode >= INFLIGHT_VIEW_DAMAGE_CONTROL)
					g_inflightViewMode = INFLIGHT_VIEW_MAP;
				else
					++g_inflightViewMode;
				xinpattr_Show_Input(g_inflightPanelInputs[g_inflightViewMode]);
				switch (g_inflightViewMode) {
					case INFLIGHT_VIEW_MAP:
						InflightUI_SelectBriefingPage(0);
						break;
					case INFLIGHT_VIEW_BRIEFING:
						InflightUI_SelectBriefingPage(1);
						break;
				}
				xview_Refresh_View();
				break;
			case INFLIGHT_BUTTON_PREVIOUS_ITEM:
				switch (g_inflightViewMode) {
					case INFLIGHT_VIEW_MAP:
						if ((int16_t)g_inflightMapShipCount != 0) {
							if (g_inflightMapSelectedShipIndex <= 0)
								g_inflightMapSelectedShipIndex = g_inflightMapShipCount - 1;
							else
								--g_inflightMapSelectedShipIndex;
							g_inflightShipDetailsIndexPlusOne = g_inflightMapSelectedShipIndex + 1;
							xinput_Refresh_System_Inputs();
						}
						break;
					case INFLIGHT_VIEW_BRIEFING: {
						int16_t scriptCount = g_inflightMapState->scriptCount;
						if (scriptCount > 1) {
							int16_t currentPage = g_inflightMapState->activeScriptIndex;
							int16_t nextPage;
							if (currentPage == 1)
								nextPage = scriptCount - 1;
							else
								nextPage = currentPage - 1;
							InflightUI_SelectBriefingPage(nextPage);
						} else {
							/* No briefing pages: select the map instead of incidental pointer bits. */
							InflightUI_SelectBriefingPage(0);
						}
						break;
					}
					case INFLIGHT_VIEW_DAMAGE_CONTROL:
						if (g_inflightRepairSelectionIndex != 0)
							--g_inflightRepairSelectionIndex;
						else
							g_inflightRepairSelectionIndex = XW_PLAYER_SUBSYSTEM_COUNT - 1;
						xinput_Refresh_System_Inputs();
						break;
				}
				break;
			case INFLIGHT_BUTTON_NEXT_ITEM:
				switch (g_inflightViewMode) {
					case INFLIGHT_VIEW_MAP:
						if ((int16_t)g_inflightMapShipCount != 0) {
							if (g_inflightMapSelectedShipIndex == (int16_t)g_inflightMapShipCount - 1)
								g_inflightMapSelectedShipIndex = 0;
							else
								++g_inflightMapSelectedShipIndex;
							g_inflightShipDetailsIndexPlusOne = g_inflightMapSelectedShipIndex + 1;
							xinput_Refresh_System_Inputs();
						}
						break;
					case INFLIGHT_VIEW_BRIEFING: {
						int16_t scriptCount = g_inflightMapState->scriptCount;
						if (scriptCount > 1) {
							int16_t currentPage = g_inflightMapState->activeScriptIndex;
							if (currentPage == scriptCount - 1)
								InflightUI_SelectBriefingPage(1);
							else
								InflightUI_SelectBriefingPage(currentPage + 1);
						} else {
							/* No briefing pages: select the map instead of incidental pointer bits. */
							InflightUI_SelectBriefingPage(0);
						}
						break;
					}
					case INFLIGHT_VIEW_DAMAGE_CONTROL:
						if (g_inflightRepairSelectionIndex < XW_PLAYER_SUBSYSTEM_COUNT - 1)
							++g_inflightRepairSelectionIndex;
						else
							g_inflightRepairSelectionIndex = 0;
						xinput_Refresh_System_Inputs();
						break;
				}
				break;
			case INFLIGHT_BUTTON_PAN_UP:
				g_inflightMapCenterY += -1 - panNumerator / g_inflightMapScaleY;
				xinpattr_Refresh_Input(g_inflightViewportInput);
				break;
			case INFLIGHT_BUTTON_PAN_LEFT:
				g_inflightMapCenterX += -1 - panNumerator / g_inflightMapScaleX;
				xinpattr_Refresh_Input(g_inflightViewportInput);
				break;
			case INFLIGHT_BUTTON_PAN_RIGHT:
				g_inflightMapCenterX += panNumerator / g_inflightMapScaleX + 1;
				xinpattr_Refresh_Input(g_inflightViewportInput);
				break;
			case INFLIGHT_BUTTON_PAN_DOWN:
				g_inflightMapCenterY += panNumerator / g_inflightMapScaleY + 1;
				xinpattr_Refresh_Input(g_inflightViewportInput);
				break;
			case INFLIGHT_BUTTON_ZOOM:
				if (input->var2 != 0) {
					if (g_inflightMapZoomLevel > 0) {
						int16_t zoomScale = g_inflightMapZoomScales[--g_inflightMapZoomLevel];
						g_inflightMapScaleX = zoomScale;
						g_inflightMapScaleY = zoomScale;
					}
				} else if (g_inflightMapZoomLevel < INFLIGHT_MAP_MAX_ZOOM_LEVEL) {
					int16_t zoomScale = g_inflightMapZoomScales[++g_inflightMapZoomLevel];
					g_inflightMapScaleX = zoomScale;
					g_inflightMapScaleY = zoomScale;
				}
				xinpattr_Refresh_Input(g_inflightViewportInput);
				break;
			case INFLIGHT_BUTTON_RESUME:
				xerror_Set_Landru_Exit(XW_SCENE_FLIGHT_RESUME);
				break;
			case INFLIGHT_BUTTON_REPAIR_EARLIER:
				if (g_inflightRepairSelectionIndex != 0) {
					int row = g_inflightRepairSelectionIndex;
					uint16_t selectedSubsystem = g_playerSubsystemRepairPriority[row];
					uint8_t adjacentSubsystem = g_playerSubsystemRepairPriority[row - 1];
					g_playerSubsystemRepairPriority[row] = adjacentSubsystem;
					g_playerSubsystemRepairPriority[row - 1] = selectedSubsystem;
					--g_inflightRepairSelectionIndex;
				}
				xinput_Refresh_System_Inputs();
				break;
			case INFLIGHT_BUTTON_REPAIR_LATER:
				if (g_inflightRepairSelectionIndex < XW_PLAYER_SUBSYSTEM_COUNT - 1) {
					int row = g_inflightRepairSelectionIndex;
					uint16_t selectedSubsystem = g_playerSubsystemRepairPriority[row];
					uint8_t adjacentSubsystem = g_playerSubsystemRepairPriority[row + 1];
					g_playerSubsystemRepairPriority[row] = adjacentSubsystem;
					g_playerSubsystemRepairPriority[row + 1] = selectedSubsystem;
					++g_inflightRepairSelectionIndex;
				}
				xinput_Refresh_System_Inputs();
				break;
			default:
				break;
		}
	}
}

// FUNCTION: XW 0x451550
void InflightUI_DrawLabel(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	char label[INFLIGHT_DETAIL_TEXT_CAPACITY];
	const char* labelText;
	if (refresh == 0)
		return;
	switch (input->id) {
		case INFLIGHT_BUTTON_PREVIOUS_VIEW:
		case INFLIGHT_BUTTON_NEXT_VIEW:
		case INFLIGHT_BUTTON_PREVIOUS_ITEM:
		case INFLIGHT_BUTTON_NEXT_ITEM: {
			int16_t arrowIcon;
			xstyle_Style_Paint_Border(frame, ((PushButton*)input)->pressed);
			if (input->id == INFLIGHT_BUTTON_PREVIOUS_VIEW || input->id == INFLIGHT_BUTTON_PREVIOUS_ITEM)
				arrowIcon = INFLIGHT_LABEL_ARROW_LEFT;
			else
				arrowIcon = INFLIGHT_LABEL_ARROW_RIGHT;
			xstyle_Style_Draw_Centered_Icon(arrowIcon, frame, clip, ((PushButton*)input)->pressed);
			return;
		}
		case INFLIGHT_LABEL_TITLE:
			xstyle_Style_Paint_TextField(frame);
			xfont_Print_Centered_Text(g_inflightViewTitles[g_inflightViewMode], frame, 0,
									  INFLIGHT_LABEL_COLOR);
			return;
		case INFLIGHT_LABEL_CENTER_SELECTED:
			xstyle_Style_Paint_TextField(frame);
			label[0] = 0;
			labelText = "";
			switch (g_inflightViewMode) {
				case INFLIGHT_VIEW_MAP:
					if (g_inflightMapSelectedShipIndex != INFLIGHT_SELECTION_CLEAR)
						sprintf(label, "Spacecraft %d of %d", g_inflightMapSelectedShipIndex + 1,
								(int16_t)g_inflightMapShipCount);
					else
						labelText = "No Ship Selected";
					break;
				case INFLIGHT_VIEW_BRIEFING:
					if (g_inflightMapState->scriptCount > 1)
						sprintf(label, "Page %d of %d", g_inflightMapState->activeScriptIndex,
								g_inflightMapState->scriptCount - 1);
					else
						labelText = "No Briefing";
					break;
				case INFLIGHT_VIEW_DAMAGE_CONTROL:
					labelText = g_inflightSubsystemNames
						[g_playerSubsystemRepairPriority[g_inflightRepairSelectionIndex]];
					break;
			}
			if (labelText[0] != 0)
				strcpy(label, labelText);
			xfont_Print_Centered_Text(label, frame, 0, INFLIGHT_LABEL_COLOR);
			break;
		case INFLIGHT_LABEL_SHIP_DETAILS:
			if (g_inflightShipDetailsIndexPlusOne != 0)
				InflightUI_DrawShipDetails(frame, clip);
			else
				xpaint_Paint_Clipped_Rect(frame, INFLIGHT_DETAIL_BACKGROUND);
			return;
	}
}

// FUNCTION: XW 0x451750
int16_t InflightUI_UpdateRepeatingButton(PushButton* button, Rect* frame, Rect* clip, int16_t key,
										 int leftEvent, int rightEvent, int16_t x, int16_t y) {
	int mouseEvent;
	(void)clip;
	if (key != 0) {
		return 0;
	}
	mouseEvent = leftEvent;
	if (leftEvent == 0) {
		mouseEvent = key;
	}
	if (rightEvent != 0) {
		mouseEvent = rightEvent;
	}
	switch (mouseEvent) {
		case INFLIGHT_MOUSE_PRESS:
			button->pressed = 1;
			button->header.var1 = 0;
			button->header.var2 = rightEvent != 0;
			break;
		case INFLIGHT_MOUSE_HOLD:
			y += frame->top;
			x += frame->left;
			button->pressed = xrect_Point_In_Rect(frame, x, y);
			if (button->pressed == 0) {
				button->header.var1 = 0;
			} else if (++button->header.var1 >= INFLIGHT_BUTTON_REPEAT_DELAY) {
				xinpattr_Selected_Input(&button->header);
			}
			break;
		case INFLIGHT_MOUSE_RELEASE:
			if (button->pressed != 0) {
				if (button->header.var1 <= INFLIGHT_BUTTON_REPEAT_DELAY) {
					xinpattr_Selected_Input(&button->header);
				}
				button->pressed = 0;
			}
			break;
	}
	xinpattr_Refresh_Input(&button->header);
	return 1;
}

// FUNCTION: XW 0x451840
void InflightUI_DrawTextButton(PushButton* button, Rect* frame, Rect* clip, int16_t refresh) {
	(void)clip;
	if (refresh != 0) {
		const char* labels = button->labels;
		xstyle_Style_Paint_Border(frame, button->pressed);
		xstyle_Style_Button_Text(labels, frame, button->pressed);
	}
}

// FUNCTION: XW 0x451880
void InflightUI_DrawShipDetails(Rect* frame, Rect* clip) {
	Rect innerFrame;
	Rect nameFrame;
	int16_t objectIndex;
	uint8_t hudShipId;
	int16_t fallbackImage;
	int16_t iff;
	CraftData* craft;
	int16_t identityVisible;
	LandruHandle flightGroupHandle;
	int distanceHundredths;
	int16_t distanceWhole;
	int16_t distanceFraction;
	int textWidth;
	char statusText[INFLIGHT_DETAIL_STATUS_CAPACITY];
	char cargoText[INFLIGHT_DETAIL_TEXT_CAPACITY];
	char displayName[INFLIGHT_DETAIL_TEXT_CAPACITY];
	char distanceText[INFLIGHT_DETAIL_TEXT_CAPACITY];
	xrect_Copy_Rect(&innerFrame, frame);
	xrect_Inset_Rect(&innerFrame, 1, 1);
	xpaint_Paint_Clipped_Rect(&innerFrame, INFLIGHT_DETAIL_BACKGROUND);
	objectIndex = player_VisibleIndexToObject(g_inflightShipDetailsIndexPlusOne - 1);
	hudShipId = g_objectTypeHudShipIds[g_objectTable[objectIndex].objectType];
	if (hudShipId > INFLIGHT_DETAIL_LAST_SHIP_IMAGE) {
		fallbackImage = 1;
		xactor_Set_Actor_State(g_inflightShipDetailsActor, g_inflightShipDetailsActor->arraySize - 1, 0);
	} else {
		xactor_Set_Actor_State(g_inflightShipDetailsActor, hudShipId - 1, 0);
		fallbackImage = 0;
	}
	iff = g_objectTable[objectIndex].iff;
	craft = (CraftData*)g_objectTable[objectIndex].instanceData;
	if (iff == 0 ||
		g_objectTypeHudShipIds[g_objectTable[objectIndex].objectType] < INFLIGHT_DETAIL_HIDDEN_ID_FIRST ||
		g_objectTypeHudShipIds[g_objectTable[objectIndex].objectType] > INFLIGHT_DETAIL_HIDDEN_ID_LAST)
		identityVisible = 1;
	else
		identityVisible = 0;
	identityVisible |= craft->isInspected;
#ifdef XW_MODERN
	{
		/* Interdictor category 18 has no entry in the original abbreviation table. */
		uint16_t category = spec_getstatisticscategory(g_objectTable[objectIndex].objectType);
		const char* abbreviation = "???";
		if (category < INFLIGHT_CRAFT_ABBREVIATION_COUNT)
			abbreviation = g_inflightCraftAbbreviations[category];
		else if (category == SPEC_STATISTICS_INTERDICTOR)
			abbreviation = "INT";
		strcpy(displayName, abbreviation);
	}
#else
	strcpy(displayName,
		   g_inflightCraftAbbreviations[spec_getstatisticscategory(g_objectTable[objectIndex].objectType)]);
#endif
	strcat(displayName, ": ");
	flightGroupHandle = g_inflightMapFlightGroupHandles[craft->flightGroupIndex];
	if (flightGroupHandle != LANDRU_NULL_HANDLE && identityVisible != 0) {
		XwMissionFlightGroup* flightGroup = (XwMissionFlightGroup*)xmemhdl_Lock_Handle(flightGroupHandle);
		strcat(displayName, flightGroup->name);
		if ((int16_t)flightGroup->numberOfCraft > 1) {
			sprintf(cargoText, " %d", craft->craftIndexInFlightGroup + 1);
			strcat(displayName, cargoText);
		}
		nullsub_SharedNoOp();
	}
	if (identityVisible == 0)
		strcat(displayName, "UNKNOWN");
	if (player_HasCargoReadout(objectIndex) != 0) {
		if (craft->isInspected != 0) {
			if (craft->cargoName[0] != 0)
				strcpy(cargoText, craft->cargoName);
			else
				strcpy(cargoText, "NONE");
		} else {
			strcpy(cargoText, "UNKNOWN");
		}
	} else {
		cargoText[0] = 0;
	}
	distanceHundredths = g_missionRuntimeState.snapshotCraftDisplayDistances[objectIndex];
	distanceWhole = distanceHundredths / INFLIGHT_DISTANCE_FRACTION_SCALE;
	distanceFraction = distanceHundredths % INFLIGHT_DISTANCE_FRACTION_SCALE;
	if (distanceFraction < INFLIGHT_DISTANCE_LEADING_ZERO_LIMIT)
		sprintf(distanceText, "DIS:\002%d\001.\0020%d", distanceWhole, distanceFraction);
	else
		sprintf(distanceText, "DIS:\002%d\001.\002%d", distanceWhole, distanceFraction);
	strcpy(statusText,
		   g_inflightCraftStatusText[g_missionRuntimeState.snapshotCraftStatusCodes[objectIndex]]);
	if (fallbackImage != 0)
		xactanim_Draw_Anim_Actor(g_inflightShipDetailsActor, frame, clip, innerFrame.left + 1,
								 innerFrame.top + INFLIGHT_DETAIL_FALLBACK_Y, 1);
	else
		xactanim_Draw_Anim_Actor(g_inflightShipDetailsActor, frame, clip, innerFrame.left + 1,
								 innerFrame.top + INFLIGHT_DETAIL_LINE_HEIGHT, 1);
	xrect_Copy_Rect(&nameFrame, &innerFrame);
	nameFrame.bottom = nameFrame.top + INFLIGHT_DETAIL_LINE_HEIGHT;
	xfont_Set_FontID_Bold_Color(0, g_inflightPanelBoldColors[iff]);
	xfont_Print_Centered_Text(displayName, &nameFrame, 0, g_inflightPanelTextColors[iff]);
	if (cargoText[0] != 0) {
		textWidth = xfont_Get_String_Width_0(0, cargoText) + INFLIGHT_DETAIL_TEXT_INSET;
		xfont_Print_Clipped_Text(cargoText, innerFrame.right - textWidth,
								 innerFrame.bottom - 2 * INFLIGHT_DETAIL_LINE_HEIGHT, 0,
								 g_inflightPanelBoldColors[INFLIGHT_DETAIL_CARGO_COLOR_INDEX]);
	}
	xfont_Set_FontID_Bold_Color(0, g_inflightPanelBoldColors[INFLIGHT_DETAIL_DISTANCE_COLOR_INDEX]);
	xfont_Print_Clipped_Text(distanceText, innerFrame.left + INFLIGHT_DETAIL_TEXT_INSET,
							 innerFrame.bottom - INFLIGHT_DETAIL_LINE_HEIGHT, 0,
							 g_inflightPanelTextColors[INFLIGHT_DETAIL_DISTANCE_COLOR_INDEX]);
	textWidth = xfont_Get_String_Width_0(0, statusText) + INFLIGHT_DETAIL_TEXT_INSET;
	xfont_Print_Clipped_Text(statusText, innerFrame.right - textWidth,
							 innerFrame.bottom - INFLIGHT_DETAIL_LINE_HEIGHT, 0,
							 INFLIGHT_DETAIL_STATUS_COLOR);
}

// FUNCTION: XW 0x451D70
void InflightUI_SelectBriefingPage(int16_t scriptIndex) {
	int16_t scriptCount = g_inflightMapState->scriptCount;
	if (scriptIndex >= scriptCount)
		scriptIndex -= scriptCount;
	if (scriptIndex < 0)
		scriptIndex += scriptCount;
	g_inflightMapState->activeScriptIndex = scriptIndex;
	player_Reseek_Page(scriptIndex);
	player_ApplyBriefingLayout(g_inflightMapState->scriptLayoutIndex[scriptIndex]);
}
