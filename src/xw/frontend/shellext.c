#include "xw/frontend/shellext.h"
#ifdef XW_MODERN
#include "xw_dos94/frontend/sound_preferences.h"
#endif

#include "xw/landru_config.h"

#include "xw/audio/fsfx.h"
#include "xw/audio/hilevel.h"
#include "xw/audio/lolevel.h"
#include "xw/audio/soundext.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/shell_flight.h"
#include "xw/frontend/asl.h"
#include "xw/frontend/computer.h"
#include "xw/frontend/inflight_options.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shipext.h"
#include "xw/input/joystick.h"
#include "xw/render/rtsvga2.h"
#include "xw/util/landru_display.h"
#include "xw/util/testdrv.h"
#include "xw_runtime/compat/landru_modal.h"
#include "xw_runtime/runtime/computer_task.h"
#include "xw_runtime/runtime/options_task.h"

#ifdef XW_MODERN
#include "xw/assets/file.h"
#include "xw_dos94/frontend/dispatch.h"
#include "xw_runtime/audio/imuse_session.h"
#include "xw_runtime/config/preference_apply.h"
#include "xw_runtime/runtime/port.h"
#include "xw_runtime/runtime/profile.h"
#include "xw_runtime/runtime/shell_scene_task.h"
#endif

#include <landru/actanim.h>
#include <landru/actor.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/dlgjoy.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/file.h>
#include <landru/font.h>
#include <landru/inpattr.h>
#include <landru/io.h>
#include <landru/joy.h>
#include <landru/paint.h>
#include <landru/pal.h>
#include <landru/res.h>
#include <landru/sound.h>
#include <landru/style.h>
#include <landru/view.h>
#include <landru/viewadd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4DC6B8
XwSceneTransitionRedirect g_sceneTransitionRedirects[XW_SCENE_TRANSITION_REDIRECT_COUNT] = {
	{ XW_SCENE_TRAINING_DEPART_INDEPENDENCE, XW_SCENE_PROVING_GROUNDS_ROOM },
	{ XW_SCENE_TRAINING_RETURN_SHUTTLE, XW_SCENE_CONCOURSE },
	{ XW_SCENE_COMBAT_DEPART_INDEPENDENCE, XW_SCENE_COMBAT_SIMULATOR_ROOM },
	{ XW_SCENE_COMBAT_RETURN_SHUTTLE, XW_SCENE_CONCOURSE },
	{ XW_SCENE_TOUR_DEPART_INDEPENDENCE, XW_SCENE_BRIEFING_TOUR },
	{ XW_SCENE_TOUR_RETURN_AWING, XW_SCENE_DEBRIEF_TOUR },
	{ XW_SCENE_TOUR_RETURN_XWING, XW_SCENE_DEBRIEF_TOUR },
	{ XW_SCENE_TOUR_RETURN_YWING, XW_SCENE_DEBRIEF_TOUR },
	{ XW_SCENE_TOUR_RETURN_BWING, XW_SCENE_DEBRIEF_TOUR },
	{ XW_SCENE_LEGACY_TRAINING_TRANSITION_260, XW_SCENE_FLIGHT_PROVING_GROUNDS },
	{ XW_SCENE_LEGACY_TRAINING_TRANSITION_261, XW_SCENE_FLIGHT_PROVING_GROUNDS },
	{ XW_SCENE_LEGACY_TRAINING_TRANSITION_262, XW_SCENE_FLIGHT_PROVING_GROUNDS },
	{ XW_SCENE_LEGACY_TRAINING_TRANSITION_266, XW_SCENE_FLIGHT_PROVING_GROUNDS },
	{ XW_SCENE_LEGACY_COMBAT_TRANSITION_270, XW_SCENE_FLIGHT_COMBAT },
	{ XW_SCENE_LEGACY_COMBAT_TRANSITION_271, XW_SCENE_FLIGHT_COMBAT },
	{ XW_SCENE_LEGACY_COMBAT_TRANSITION_272, XW_SCENE_FLIGHT_COMBAT },
	{ XW_SCENE_LEGACY_COMBAT_TRANSITION_276, XW_SCENE_FLIGHT_COMBAT },
	{ XW_SCENE_LEGACY_TOUR_TRANSITION_283, XW_SCENE_FLIGHT_TOUR },
	{ XW_SCENE_LEGACY_TOUR_TRANSITION_284, XW_SCENE_FLIGHT_TOUR },
	{ XW_SCENE_LEGACY_TOUR_TRANSITION_285, XW_SCENE_FLIGHT_TOUR },
	{ XW_SCENE_LEGACY_TOUR_TRANSITION_281, XW_SCENE_FLIGHT_TOUR },
	{ XW_SCENE_TRAINING_RETURN_AWING, XW_SCENE_DEBRIEF_PROVING_GROUNDS },
	{ XW_SCENE_TRAINING_RETURN_XWING, XW_SCENE_DEBRIEF_PROVING_GROUNDS },
	{ XW_SCENE_TRAINING_RETURN_YWING, XW_SCENE_DEBRIEF_PROVING_GROUNDS },
	{ XW_SCENE_TRAINING_RETURN_BWING, XW_SCENE_DEBRIEF_PROVING_GROUNDS },
	{ XW_SCENE_COMBAT_RETURN_AWING, XW_SCENE_DEBRIEF_COMBAT },
	{ XW_SCENE_COMBAT_RETURN_XWING, XW_SCENE_DEBRIEF_COMBAT },
	{ XW_SCENE_COMBAT_RETURN_YWING, XW_SCENE_DEBRIEF_COMBAT },
	{ XW_SCENE_COMBAT_RETURN_BWING, XW_SCENE_DEBRIEF_COMBAT },
	{ XW_SCENE_TOUR_RETURN_INDEPENDENCE, XW_SCENE_CONCOURSE },
	{ XW_SCENE_TOUR_LAUNCH_XWING, XW_SCENE_FLIGHT_TOUR },
	{ XW_SCENE_TOUR_LAUNCH_YWING, XW_SCENE_FLIGHT_TOUR },
	{ XW_SCENE_TOUR_LAUNCH_AWING, XW_SCENE_FLIGHT_TOUR },
	{ XW_SCENE_TOUR_LAUNCH_BWING, XW_SCENE_FLIGHT_TOUR },
	{ XW_SCENE_COMBAT_LAUNCH_XWING, XW_SCENE_FLIGHT_COMBAT },
	{ XW_SCENE_COMBAT_LAUNCH_YWING, XW_SCENE_FLIGHT_COMBAT },
	{ XW_SCENE_COMBAT_LAUNCH_AWING, XW_SCENE_FLIGHT_COMBAT },
	{ XW_SCENE_COMBAT_LAUNCH_BWING, XW_SCENE_FLIGHT_COMBAT },
	{ XW_SCENE_TRAINING_LAUNCH_XWING, XW_SCENE_FLIGHT_PROVING_GROUNDS },
	{ XW_SCENE_TRAINING_LAUNCH_YWING, XW_SCENE_FLIGHT_PROVING_GROUNDS },
	{ XW_SCENE_TRAINING_LAUNCH_AWING, XW_SCENE_FLIGHT_PROVING_GROUNDS },
	{ XW_SCENE_TRAINING_LAUNCH_BWING, XW_SCENE_FLIGHT_PROVING_GROUNDS },
	{ XW_SCENE_TRANSITION_TABLE_END, XW_SCENE_TRANSITION_TABLE_END },
};

// GLOBAL: XW 0x4DC7B8
char g_optionsMusicLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY] = "Music is Off\0Music is On";

// GLOBAL: XW 0x4DC808
char g_optionsSoundLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY] = "Sound is Off\0Sound is On";

// GLOBAL: XW 0x4DC858
char g_optionsSpokenTextLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY] = "Spoken Text Hidden\0Spoken Text Shown";

// GLOBAL: XW 0x4DC8A8
char g_optionsTransitionsLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY] = "Transitions Skipped\0Transitions Shown";

// GLOBAL: XW 0x4DC8F8
char g_optionsOkLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY] = "OK";

// GLOBAL: XW 0x4DC948
char g_optionsExitLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY] = "Exit to Windows";

// GLOBAL: XW 0x4DC998
char g_optionsMissionSetLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY] = "New Missions\0Classic Missions";

// GLOBAL: XW 0x4DC9E8
char g_optionsFlightLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY] = "Flight Options";

// GLOBAL: XW 0x4DCA38
char g_optionsResolutionLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY] =
	"320x200x8bit\0 640x480x8bit\0 640x480x16bit\0 640x480 3D Hardware";

// GLOBAL: XW 0x4DCA88
char g_optionsEngineSoundLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY] = "Engine sfx is Off\0Engine sfx is On";

// GLOBAL: XW 0x4DCAD8
char g_optionsTextureLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY] =
	"Texture Res Low\0Texture Res Med\0Texture Res High";

// GLOBAL: XW 0x4DCB78
char g_optionsJoystickLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY] = "Configure Joystick";

// GLOBAL: XW 0x4DCBC8
char g_optionsBackLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY] = "Back to Options";

// GLOBAL: XW 0x4DCCB8
char g_optionsPreviousPageLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY] = "Last Page";

// GLOBAL: XW 0x4DCD08
char g_optionsNextPageLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY] = "Next Page";

// GLOBAL: XW 0x4DCD58
char g_optionsResetLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY] = "Reset";

// GLOBAL: XW 0x4DCDA8
XwJoystickSlotLabelRecord g_optionsJoystickSlotLabels[JOYSTICK_ACTION_CAPACITY] = { 0 };

// GLOBAL: XW 0x561D20
XwOptionsPage g_optionsPageIndex = XW_OPTIONS_PAGE_GENERAL;

// GLOBAL: XW 0x561D28
JoystickEntry g_joystickEntries[SHELLEXT_JOYSTICK_ENTRY_COUNT] = { 0 };

// GLOBAL: XW 0x5667A8
int g_joystickEntryCount = 0;

// GLOBAL: XW 0x566820
int16_t g_contentAvailabilityInitialized = 0;

// FUNCTION: XW 0x4A65F0
void shellext_Open_Landru(struct XwLegacyMemoryConfig* memory) {
	Rect rect;
	char path[SHELLEXT_RESOURCE_PATH_CAPACITY];
	char filename[SHELLEXT_RESOURCE_PATH_CAPACITY];
	ResFile* resourceFile;
	if (g_installDriveLetter == SHELLEXT_CD_DRIVE_UNKNOWN) {
		g_installDriveLetter = (int16_t)testdrv_Get_XWing_CD_Drive();
	}
	if (g_installDriveLetter == 0) {
		ShellFlight_FatalExit("Can't find X-Wing CD!\n");
	}
	strcpy(path, "c:\\XwingCD\\resource\\");
	path[0] = (char)g_installDriveLetter;
	xres_SetResourcePath(path, 1);
	strcpy(path, "X-Wing Data\\RESOURCE\\");
	xres_SetResourcePath(path, 0);
	shellext_DetectInstalledContent();
	xerror_Clear_Landru_Error();
	asl_Open_ASL(memory);
	soundext_Open_Post_iMuse();
	if (xcursor_Is_Cursor_Visible() != 0) {
		xcursor_Hide_Cursor();
	}
	xrect_Max_Rect(&rect);
	path[0] = '\0';
	strcpy(filename, path);
	strcat(filename, "xwing.lfd");
	resourceFile = xres_Open_Resource(filename);
	g_shellContext->resourceFile = resourceFile;
	g_shellContext->standardPalette = xpal_Res_Palette(resourceFile, "standard");
	g_shellContext->icons = xactanim_Res_Anim_Actor(resourceFile, "icons", &rect, 0, 0, 0);
	g_shellContext->cursors = xactanim_Res_Anim_Actor(resourceFile, "cursors", &rect, 0, 0, 0);
	g_shellContext->font8 = xfont_Res_Font(resourceFile, "font8");
	g_shellContext->font6 = xfont_Res_Font(resourceFile, "font6");
#ifdef XW_MODERN
	if (!XwProfile_DosFrontend()) {
#endif
		g_shellContext->font12 = xfont_Res_Font(resourceFile, "font12");
		g_shellContext->font18 = xfont_Res_Font(resourceFile, "font18");
#ifdef XW_MODERN
	}
#endif
	xpal_Free_Palette_From_System(g_shellContext->standardPalette);
	xactor_Free_Actor_From_System(g_shellContext->icons);
	xstyle_Style_Set_Icon_Actor(g_shellContext->icons);
	xactor_Free_Actor_From_System(g_shellContext->cursors);
	xcursor_Set_Default_Cursor_Actor(g_shellContext->cursors);
	xcanvas_Enable_Screen_Diff();
	shellext_Load_Preferences();
}

// FUNCTION: XW 0x4A6850
void shellext_Close_Landru(void) {
	xcanvas_Disable_Screen_Diff();
	xpal_Free_Palette(g_shellContext->standardPalette);
	xactor_Free_Actor(g_shellContext->icons);
	xactor_Free_Actor(g_shellContext->cursors);
	xfont_Destroy_Font_Module();
	xres_Close_Resource(g_shellContext->resourceFile);
	soundext_Close_Post_iMuse();
	asl_Close_ASL();
}

// FUNCTION: XW 0x4A68B0
void shellext_Open_Landru_Scene(XwShellSceneId scene) {
	xerror_Clear_Landru_Escape();
	xerror_Clear_Landru_Exit();
	xerror_Set_Landru_Escape_Function(shellext_TryOpenOptions);
	g_shellContext->currentScene = scene;
	g_shellContext->suddenEnd = 0;
}

// FUNCTION: XW 0x4A68F0
void shellext_Close_Landru_Scene(XwShellSceneId scene) {
	g_shellContext->lastScene = scene;
	xfade_ClearTimedText();
	if (g_shellContext->suddenEnd != 0) {
#ifdef XW_MODERN
		XwShell_FadeAndSaveScene();
		return;
#else
		shellext_Sudden_Scene_Fade();
#endif
	}
	xcanvas_Copy_Screen_Portion_To_Diff(0, 0, LANDRU_VGA_WIDTH, LANDRU_VGA_HEIGHT);
}

// FUNCTION: XW 0x4A6930
int16_t shellext_Get_Cur_Scene(void) { return (int16_t)g_shellContext->currentScene; }

// FUNCTION: XW 0x4A6940
int16_t shellext_Check_Last_Scene(int16_t scene) { return g_shellContext->lastScene == scene; }

// FUNCTION: XW 0x4A6960
int16_t shellext_Get_Last_Scene(void) { return (int16_t)g_shellContext->lastScene; }

// FUNCTION: XW 0x4A6970
int16_t shellext_Check_Scene_Exit(int16_t* exitScene, int16_t nextScene, int16_t nextSection,
								  int16_t sceneComplete) {
	int16_t key = xio_Get_Key();
	int16_t exiting = 1;
	if (xio_Right_Button_Release() != 0 || key == '\r') {
		shellext_Sudden_Scene_End();
		*exitScene = nextSection;
	} else if (xio_Left_Button_Release() != 0 || key == ' ') {
		shellext_Sudden_Scene_End();
#ifdef XW_MODERN
		/* DOS94 0x100d8c distinguishes an individual scene from its section. */
		*exitScene = XwProfile_DosFrontend() ? nextScene : nextSection;
#else
		*exitScene = nextSection;
#endif
	} else if (sceneComplete != 0) {
		*exitScene = nextScene;
	} else {
		exiting = 0;
	}
	return exiting;
}

// FUNCTION: XW 0x4A69F0
void shellext_Sudden_Scene_End(void) {
	xsound_PurgeGmidSounds();
	g_shellContext->suddenEnd = 1;
}

// FUNCTION: XW 0x4A6A10
int16_t shellext_Is_Sudden_Scene_End(void) { return (int16_t)g_shellContext->suddenEnd; }

// FUNCTION: XW 0x4A6A20
void shellext_Sudden_Scene_Fade(void) {
	xviewadd_Clear_View();
	xfade_Start_Full_Fade(FADE_WIPE_SNAP_ON, FADE_COLOR_TWO_PHASE, 0, 0, 1);
#ifdef XW_MODERN
	xcanvas_Push_Fade_Screen_To_Video_Task(0);
#else
	xcanvas_Fade_Screen_To_Video(0);
#endif
}

// FUNCTION: XW 0x4A6A50
XwShellViewResult j_xviewadd_Handle_View(void) {
#ifdef XW_MODERN
	xviewadd_Push_Handle_View_Task();
#else
	return xviewadd_Handle_View();
#endif
}

// FUNCTION: XW 0x4A6A60
int16_t shellext_MoveGridFocus(int16_t* focusIndex, const int16_t* xPositions, const int16_t* yPositions,
							   int rows, int columns, int16_t key) {
	int16_t candidateIndex;
	int16_t finished;
	int16_t startIndex;
	int16_t startX;
	int16_t startY;
	if (key != SHELLEXT_KEY_LEFT && key != SHELLEXT_KEY_RIGHT && key != SHELLEXT_KEY_UP &&
		key != SHELLEXT_KEY_DOWN)
		return 0;
	finished = 0;
	startIndex = *focusIndex;
	candidateIndex = startIndex;
	startX = xPositions[startIndex];
	startY = yPositions[startIndex];
	do {
		switch (key) {
			case SHELLEXT_KEY_LEFT:
				if (candidateIndex % (int16_t)columns != 0)
					--candidateIndex;
				else
					candidateIndex += columns - 1;
				break;
			case SHELLEXT_KEY_RIGHT:
				if (candidateIndex % (int16_t)columns < (int16_t)columns - 1)
					++candidateIndex;
				else
					candidateIndex += 1 - columns;
				break;
			case SHELLEXT_KEY_UP:
				if (candidateIndex < (int16_t)columns)
					candidateIndex += columns * (rows - 1);
				else
					candidateIndex -= columns;
				break;
			case SHELLEXT_KEY_DOWN:
				if (candidateIndex >= (int16_t)columns * ((int16_t)rows - 1))
					candidateIndex = candidateIndex % (int16_t)columns;
				else
					candidateIndex += columns;
				break;
		}
		if (startX != xPositions[candidateIndex] || startY != yPositions[candidateIndex])
			finished = 1;
		if (candidateIndex == startIndex)
			finished = 1;
	} while (finished == 0);
	*focusIndex = candidateIndex;
	return 1;
}

// FUNCTION: XW 0x4A7010
int16_t shellext_TryOpenOptions(void) {
	int16_t previousExit = xerror_Get_Landru_Exit();
	int16_t dialogIdle = xdialog_Is_Active_Dialog() == 0;
	int16_t dialogAndFadeIdle = dialogIdle && xfade_Fade_Active() == 0;
	int16_t viewStarted = dialogAndFadeIdle && xview_Get_View_Time() != 0;
	int16_t optionsAllowed = viewStarted && (shellext_Get_Cur_Scene() < XW_SCENE_INFLIGHT_MAP ||
											 shellext_Get_Cur_Scene() > XW_SCENE_INFLIGHT_OPTIONS);
	if (optionsAllowed)
		previousExit = shellext_OpenOptionsAndSave();
	return previousExit;
}

// FUNCTION: XW 0x4A7090
int16_t shellext_OpenOptionsAndSave(void) {
#ifdef XW_MODERN
	XwPort_RequestSettings();
	return xerror_Get_Landru_Exit();
#else
	bool calibrationPending = false;
	int16_t previousExit = xerror_Get_Landru_Exit();
	if (xio_Is_Mouse_Input() == 0 && xio_Test_Joystick_Callibrate() != 0) {
		XwOptions_SavePreviousExit(previousExit);
		calibrationPending = xdlgjoy_Schedule_Joystick_Callibrate(XwOptions_AfterCalibration, NULL);
	}
	if (!calibrationPending) {
		XwOptions_SavePreviousExit(previousExit);
		shellext_ShowOptionsDialog(XwOptions_SaveAndFinish, NULL);
	}
	return previousExit;
#endif
}

// FUNCTION: XW 0x4A70F0
void shellext_Load_Preferences(void) {
#ifdef XW_MODERN
	XwPreferences_LoadRuntime();
#else
	ShellPreferences_Load();
	g_flightTransitionsEnabled = g_shellPreferences.transitionsEnabled;
	g_classicMissionsMirror = g_shellPreferences.classicMissions;
	g_flightEngineSoundEnabled = g_shellPreferences.engineSoundEnabled;
	g_flightMusicEnabled = g_shellPreferences.musicEnabled;
	g_flightSfxEnabled = g_shellPreferences.sfxEnabled;
	g_flightMusicVolume = g_shellPreferences.musicVolume;
	g_flightSfxVolume = g_shellPreferences.sfxVolume;
	g_flightDigitalSoundEnabled = g_shellPreferences.digitalSoundEnabled;
	g_flightVoiceEnabled = g_shellPreferences.voiceEnabled;
	g_flightReplayDiskCacheKB = g_shellPreferences.replayDiskCacheKB;
	g_flightReplayDiskCacheEnabled = g_shellPreferences.replayDiskCacheEnabled;
	g_flightHighDetailStarfield = g_shellPreferences.highDetailStarfield;
	g_flightBackdropsPreference = g_shellPreferences.backdropsEnabled;
	g_flightDebrisPreference = g_shellPreferences.debrisEnabled;
	g_flightMarkingsPreference = g_shellPreferences.markingsEnabled;
	g_flightEngineGlowPreference = g_shellPreferences.engineGlowEnabled;
	g_flightStarfighterDetail = g_shellPreferences.starfighterDetail;
	g_flightStarshipDetail = g_shellPreferences.starshipDetail;
	g_flightDeathStarDetail = g_shellPreferences.deathStarDetail;
	g_flightInterlaceEnabled = g_shellPreferences.interlaceEnabled;
	g_modelTextureQuality = g_shellPreferences.textureQuality;
	g_flightBrightnessSetting = g_shellPreferences.brightness;
	switch (g_shellPreferences.resolutionIndex) {
		case XW_SHELL_RESOLUTION_320_200:
			g_flightResolutionMode = RTSVGA2_MODE_13H;
			break;
		case XW_SHELL_RESOLUTION_640_480_INDEXED:
			g_flightResolutionMode = FLIGHT_DISPLAY_MODE_101H;
			break;
		case XW_SHELL_RESOLUTION_640_480_RGB:
			g_flightResolutionMode = FLIGHT_DISPLAY_MODE_111H;
			break;
		case XW_SHELL_RESOLUTION_HARDWARE:
			g_flightResolutionMode = FLIGHT_DISPLAY_MODE_1FFH;
			break;
	}
	if (g_shellPreferences.joystickActions[0] == 0)
		g_shellPreferences.joystickActions[0] = JOYSTICK_ACTION_FIRE;
	if (g_shellPreferences.joystickActions[1] == 0)
		g_shellPreferences.joystickActions[1] = JOYSTICK_ACTION_ROLL_TARGET;
	memcpy(g_flightJoystickActions, g_shellPreferences.joystickActions, sizeof(g_flightJoystickActions));
	shellext_Set_Prefs_Sound();
	xfade_SetSpeechTextEnabled(g_shellPreferences.spokenTextEnabled != 0 ||
							   ShellPreferences_GetSfxEnabled() == 0);
#endif
}

// FUNCTION: XW 0x4A72B0
int16_t shellext_GetTransitionsEnabled(void) { return g_shellPreferences.transitionsEnabled; }

// FUNCTION: XW 0x4A72C0
int16_t shellext_GetClassicMissions(void) { return g_shellPreferences.classicMissions; }

// FUNCTION: XW 0x4A72D0
int16_t shellext_Convert_Transition(int16_t scene, int16_t sudden) {
	int16_t index;
#ifdef XW_MODERN
	if (XwProfile_DosFrontend())
		return Dos94_ConvertTransition(scene, sudden);
#endif
	for (index = 0; g_sceneTransitionRedirects[index].sourceScene != scene; ++index) {
		if (g_sceneTransitionRedirects[index].sourceScene == XW_SCENE_TRANSITION_TABLE_END) {
			break;
		}
	}
	if (g_sceneTransitionRedirects[index].sourceScene != XW_SCENE_TRANSITION_TABLE_END) {
		if (sudden != 0) {
			shellext_Sudden_Scene_End();
		}
		return g_sceneTransitionRedirects[index].destinationScene;
	}
	return scene;
}

// FUNCTION: XW 0x4A7320
void shellext_Set_Prefs_Sound(void) {
#ifdef XW_MODERN
	Dos94_shellext_Set_Prefs_Sound();
	hilevel_SetCdAuxVolume(g_shellPreferences.musicEnabled && g_shellPreferences.musicVolume
							   ? XW_SHELL_VOLUME_STEP * g_shellPreferences.musicVolume - 1
							   : 0);
#else
	uint16_t musicVolume;
	uint16_t sfxVolume;
	if (g_shellPreferences.musicEnabled != 0 && g_shellPreferences.musicVolume != 0) {
		musicVolume = XW_SHELL_VOLUME_STEP * g_shellPreferences.musicVolume - 1;
	} else {
		musicVolume = 0;
	}
	hilevel_SetCdAuxVolume(musicVolume);
	hilevel_ImGetMusicVol();
	if (g_shellPreferences.sfxEnabled != 0 && g_shellPreferences.sfxVolume != 0) {
		sfxVolume = XW_SHELL_VOLUME_STEP * g_shellPreferences.sfxVolume - 1;
	} else {
		sfxVolume = 0;
	}
	hilevel_ImSetSfxVol(sfxVolume);
	hilevel_ImGetSfxVol();
	hilevel_ImSetVoiceVol(sfxVolume);
	hilevel_ImGetVoiceVol();
#endif
}

// FUNCTION: XW 0x4A73A0
void shellext_ShowOptionsDialog(DialogSubResultHandler complete, void* context) {
#ifdef XW_MODERN
	XwOptions_RequestSettings(complete, context);
#else
	int entryIndex;
	Palette* savedPalette;
	Palette* blackPalette;
	Input* dialog;
	uint8_t savedMasterVolume;
	int16_t savedKeyButtons;
	int savedDoublePixels = g_landruDoublePixelsEnabled;
	LandruDisplay_SetLowResolutionMode(0);
	LandruDisplay_SaveCanvasBackground();
	savedKeyButtons = xio_Is_Key_Buttons();
	xio_Clear_Key_Buttons();
	g_optionsField5667D0 = SHELLEXT_OPTIONS_INITIAL_FIELD;
	g_optionsField5667BC = 0;
	g_optionsJoystickSlot = 0;
	shellext_LoadJoystickActionDictionary();
	for (entryIndex = 0; entryIndex < g_joystickEntryCount; ++entryIndex) {
		if (g_joystickEntries[entryIndex].actionCode ==
			g_shellPreferences.joystickActions[g_optionsJoystickSlot])
			g_optionsActionIndex = entryIndex;
	}
	g_optionsPageIndex = XW_OPTIONS_PAGE_GENERAL;
	savedPalette = xpal_Alloc_Palette(0, SHELLEXT_PALETTE_COLORS);
	xpal_Copy_Palette(savedPalette, xpal_Get_Screen_Palette(), 0, SHELLEXT_PALETTE_COLORS, 0);
	blackPalette = xpal_Alloc_Palette(0, SHELLEXT_PALETTE_COLORS);
	xpal_Clear_Palette(blackPalette);
	xpal_Set_Screen_Palette(blackPalette);
	xpal_Free_Palette(blackPalette);
	lolevel_ImPause();
	savedMasterVolume = hilevel_ImGetMasterVol();
	hilevel_ImSetMasterVol(0);
	dialog = shellext_BuildOptionsDialog();
	XwOptions_ScheduleDialog(dialog, savedPalette, savedMasterVolume, savedKeyButtons, savedDoublePixels,
							 complete, context);
#endif
}

// FUNCTION: XW 0x4A7530
Input* shellext_BuildOptionsDialog(void) {
	Input* dialog;
	Input* generalPage;
	PushButton* musicButton;
	Input* musicSlider;
	PushButton* sfxButton;
	Input* sfxSlider;
	PushButton* missionSetButton;
	PushButton* transitionsButton;
	PushButton* spokenTextButton;
	PushButton* okButton;
	PushButton* exitButton;
	Input* flightPage;
	PushButton* resolutionButton;
	PushButton* textureButton;
	Input* brightnessSlider;
	PushButton* engineSoundButton;
	PushButton* flightBackButton;
	Input* joystickPage;
	PushButton* previousSlotButton;
	PushButton* nextSlotButton;
	PushButton* previousActionPageButton;
	PushButton* nextActionPageButton;
	PushButton* resetButton;
	PushButton* joystickBackButton;
	Rect rect;
	Rect sliderRect;

	xinput_Get_System_Input_Frame(&rect);
	dialog = xinput_Alloc_Dialog_Input(NULL, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(dialog, shellext_UpdateOptionsInput);
	xinpattr_Set_Input_Draw_Function(dialog, shellext_DrawOptionsInput);
	xinpattr_Show_Input(dialog);
	dialog->var1 = SHELLEXT_OPTIONS_PALETTE_DELAY;
	dialog->id = XW_OPTIONS_ROOT;
	g_optionsDialog = dialog;
	xrect_Set_Rect(&rect, 0, 0, 140, 200);
	generalPage = xinput_Alloc_Dialog_Input(dialog, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(generalPage, shellext_UpdateOptionsInput);
	xinpattr_Set_Input_Draw_Function(generalPage, shellext_DrawOptionsInput);
	xinpattr_Set_Input_Allign(generalPage, SHELLEXT_ALIGN_CENTER, SHELLEXT_ALIGN_CENTER);
	xinpattr_Show_Input(generalPage);
	generalPage->id = XW_OPTIONS_GENERAL_PAGE;
	g_optionsGeneralPage = generalPage;
	xrect_Set_Rect(&rect, 8, 17, 132, 33);
	musicButton = xbtnpush_Alloc_Button(generalPage, &rect, 0, shellext_OptionsButtonUser,
										g_optionsMusicLabels, XW_OPTIONS_MUSIC);
	xbtnpush_Set_Button_LabelIndex(musicButton, g_shellPreferences.musicEnabled);
	xrect_Set_Rect(&rect, 49, 35, 132, 47);
	musicSlider = xinput_Alloc_Dialog_Input(generalPage, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(musicSlider, shellext_UpdateOptionsInput);
	xinpattr_Set_Input_Draw_Function(musicSlider, shellext_DrawOptionsInput);
	xinpattr_Show_Input(musicSlider);
	musicSlider->mouseUsage = downMoveUpInput;
	musicSlider->id = XW_OPTIONS_MUSIC_VOLUME;
	xrect_Set_Rect(&rect, 8, 55, 132, 71);
	sfxButton = xbtnpush_Alloc_Button(generalPage, &rect, 0, shellext_OptionsButtonUser, g_optionsSoundLabels,
									  XW_OPTIONS_SFX);
	xbtnpush_Set_Button_LabelIndex(sfxButton, g_shellPreferences.sfxEnabled);
	xrect_Set_Rect(&rect, 49, 73, 132, 85);
	sfxSlider = xinput_Alloc_Dialog_Input(generalPage, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(sfxSlider, shellext_UpdateOptionsInput);
	xinpattr_Set_Input_Draw_Function(sfxSlider, shellext_DrawOptionsInput);
	xinpattr_Show_Input(sfxSlider);
	sfxSlider->mouseUsage = downMoveUpInput;
	sfxSlider->id = XW_OPTIONS_SFX_VOLUME;
	xrect_Set_Rect(&rect, 4, 87, 136, 103);
	missionSetButton = xbtnpush_Alloc_Button(generalPage, &rect, 0, shellext_OptionsButtonUser,
											 g_optionsMissionSetLabels, XW_OPTIONS_MISSION_SET);
	xbtnpush_Set_Button_LabelIndex(missionSetButton, g_shellPreferences.classicMissions);
#ifdef XW_MODERN
	if (XwProfile_MissionFlight()->version == XW_GAME_VERSION_93)
		xinpattr_Hide_Input(&missionSetButton->header);
#endif
	xrect_Set_Rect(&rect, 4, 105, 136, 121);
	transitionsButton = xbtnpush_Alloc_Button(generalPage, &rect, 0, shellext_OptionsButtonUser,
											  g_optionsTransitionsLabels, XW_OPTIONS_TRANSITIONS);
	xbtnpush_Set_Button_LabelIndex(transitionsButton, g_shellPreferences.transitionsEnabled);
	xrect_Set_Rect(&rect, 4, 123, 136, 139);
	spokenTextButton = xbtnpush_Alloc_Button(generalPage, &rect, 0, shellext_OptionsButtonUser,
											 g_optionsSpokenTextLabels, XW_OPTIONS_SPOKEN_TEXT);
	xbtnpush_Set_Button_LabelIndex(spokenTextButton, g_shellPreferences.spokenTextEnabled);
	xrect_Offset_Rect(&rect, 0, 18);
	xbtnpush_Alloc_Button(generalPage, &rect, 0, shellext_OptionsButtonUser, g_optionsFlightLabels,
						  XW_OPTIONS_OPEN_FLIGHT);
	xrect_Offset_Rect(&rect, 0, 18);
	xbtnpush_Alloc_Button(generalPage, &rect, 0, shellext_OptionsButtonUser, g_optionsJoystickLabels,
						  XW_OPTIONS_OPEN_JOYSTICK);
	xrect_Set_Rect(&rect, 4, 4, 40, 20);
	okButton = xbtnpush_Alloc_Button(generalPage, &rect, 0, shellext_OptionsButtonUser, g_optionsOkLabels,
									 XW_OPTIONS_OK);
	xinpattr_Set_Input_Allign(&okButton->header, SHELLEXT_ALIGN_START, SHELLEXT_ALIGN_END);
	xrect_Set_Rect(&rect, 4, 4, 95, 20);
	exitButton = xbtnpush_Alloc_Button(generalPage, &rect, 0, shellext_OptionsButtonUser, g_optionsExitLabels,
									   XW_OPTIONS_EXIT);
	xinpattr_Set_Input_Allign(&exitButton->header, SHELLEXT_ALIGN_END, SHELLEXT_ALIGN_END);
	xrect_Set_Rect(&rect, 0, 0, 140, 200);
	flightPage = xinput_Alloc_Dialog_Input(dialog, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(flightPage, shellext_UpdateOptionsInput);
	xinpattr_Set_Input_Draw_Function(flightPage, shellext_DrawOptionsInput);
	xinpattr_Set_Input_Allign(flightPage, SHELLEXT_ALIGN_CENTER, SHELLEXT_ALIGN_CENTER);
	xinpattr_Show_Input(flightPage);
	flightPage->id = XW_OPTIONS_FLIGHT_PAGE;
	g_optionsFlightPage = flightPage;
	xinpattr_Hide_Input(flightPage);
	xrect_Set_Rect(&rect, 4, 17, 136, 33);
	resolutionButton = xbtnpush_Alloc_Button(flightPage, &rect, 0, shellext_OptionsButtonUser,
											 g_optionsResolutionLabels, XW_OPTIONS_RESOLUTION);
	xbtnpush_Set_Button_LabelIndex(resolutionButton, g_shellPreferences.resolutionIndex);
	xrect_Offset_Rect(&rect, 0, 18);
	textureButton = xbtnpush_Alloc_Button(flightPage, &rect, 0, shellext_OptionsButtonUser,
										  g_optionsTextureLabels, XW_OPTIONS_TEXTURE_QUALITY);
	xbtnpush_Set_Button_LabelIndex(textureButton, g_shellPreferences.textureQuality);
	xrect_Copy_Rect(&sliderRect, &rect);
	xrect_Offset_Rect(&sliderRect, 0, 36);
	sliderRect.left = 25;
	sliderRect.right = 108;
	brightnessSlider = xinput_Alloc_Dialog_Input(flightPage, &sliderRect, 0, 0);
	xinpattr_Set_Input_Update_Function(brightnessSlider, shellext_UpdateOptionsInput);
	xinpattr_Set_Input_Draw_Function(brightnessSlider, shellext_DrawOptionsInput);
	xinpattr_Show_Input(brightnessSlider);
	brightnessSlider->mouseUsage = downMoveUpInput;
	brightnessSlider->id = XW_OPTIONS_BRIGHTNESS;
	xrect_Offset_Rect(&rect, 0, 56);
	engineSoundButton = xbtnpush_Alloc_Button(flightPage, &rect, 0, shellext_OptionsButtonUser,
											  g_optionsEngineSoundLabels, XW_OPTIONS_ENGINE_SOUND);
	xbtnpush_Set_Button_LabelIndex(engineSoundButton, g_shellPreferences.engineSoundEnabled);
	xrect_Set_Rect(&rect, 4, 4, 136, 20);
	flightBackButton = xbtnpush_Alloc_Button(flightPage, &rect, 0, shellext_OptionsButtonUser,
											 g_optionsBackLabels, XW_OPTIONS_BACK_FROM_FLIGHT);
	xinpattr_Set_Input_Allign(&flightBackButton->header, SHELLEXT_ALIGN_START, SHELLEXT_ALIGN_END);
	xrect_Set_Rect(&rect, 0, 0, 320, 200);
	joystickPage = xinput_Alloc_Dialog_Input(dialog, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(joystickPage, shellext_UpdateOptionsInput);
	xinpattr_Set_Input_Draw_Function(joystickPage, shellext_DrawOptionsInput);
	xinpattr_Set_Input_Allign(joystickPage, SHELLEXT_ALIGN_CENTER, SHELLEXT_ALIGN_CENTER);
	xinpattr_Show_Input(joystickPage);
	joystickPage->id = XW_OPTIONS_JOYSTICK_PAGE;
	g_optionsJoystickPage = joystickPage;
	xinpattr_Hide_Input(joystickPage);
	xrect_Set_Rect(&rect, 4, 32, 20, 48);
	previousSlotButton = xbtnpush_Alloc_Button(joystickPage, &rect, 0, shellext_OptionsButtonUser, NULL,
											   XW_OPTIONS_PREVIOUS_JOYSTICK_SLOT);
	xinpattr_Set_Input_Draw_Function(&previousSlotButton->header, shellext_DrawJoystickArrow);
	xrect_Set_Rect(&rect, 4, 32, 20, 48);
	nextSlotButton = xbtnpush_Alloc_Button(joystickPage, &rect, 0, shellext_OptionsButtonUser, NULL,
										   XW_OPTIONS_NEXT_JOYSTICK_SLOT);
	xinpattr_Set_Input_Draw_Function(&nextSlotButton->header, shellext_DrawJoystickArrow);
	xinpattr_Set_Input_Allign(&nextSlotButton->header, SHELLEXT_ALIGN_END, SHELLEXT_ALIGN_START);
	xrect_Set_Rect(&rect, 4, 4, 78, 20);
	previousActionPageButton =
		xbtnpush_Alloc_Button(joystickPage, &rect, 0, shellext_OptionsButtonUser, g_optionsPreviousPageLabels,
							  XW_OPTIONS_PREVIOUS_ACTION_PAGE);
	xinpattr_Set_Input_Allign(&previousActionPageButton->header, SHELLEXT_ALIGN_START, SHELLEXT_ALIGN_END);
	xrect_Set_Rect(&rect, 82, 4, 156, 20);
	nextActionPageButton = xbtnpush_Alloc_Button(joystickPage, &rect, 0, shellext_OptionsButtonUser,
												 g_optionsNextPageLabels, XW_OPTIONS_NEXT_ACTION_PAGE);
	xinpattr_Set_Input_Allign(&nextActionPageButton->header, SHELLEXT_ALIGN_START, SHELLEXT_ALIGN_END);
	xrect_Set_Rect(&rect, 160, 4, 206, 20);
	resetButton = xbtnpush_Alloc_Button(joystickPage, &rect, 0, shellext_OptionsButtonUser,
										g_optionsResetLabels, XW_OPTIONS_RESET_BINDINGS);
	xinpattr_Set_Input_Allign(&resetButton->header, SHELLEXT_ALIGN_START, SHELLEXT_ALIGN_END);
	xrect_Set_Rect(&rect, 210, 4, 316, 20);
	joystickBackButton = xbtnpush_Alloc_Button(joystickPage, &rect, 0, shellext_OptionsButtonUser,
											   g_optionsBackLabels, XW_OPTIONS_BACK_FROM_JOYSTICK);
	xinpattr_Set_Input_Allign(&joystickBackButton->header, SHELLEXT_ALIGN_START, SHELLEXT_ALIGN_END);
	return dialog;
}

// FUNCTION: XW 0x4A7D00
void shellext_DrawJoystickArrow(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	PushButton* button = (PushButton*)input;
	if (refresh != 0) {
		xstyle_Style_Paint_Border(frame, button->pressed);
		xstyle_Style_Draw_Centered_Icon(
			button->header.id == XW_OPTIONS_PREVIOUS_JOYSTICK_SLOT ? iconLeftArrow : iconRightArrow, frame,
			clip, button->pressed);
	}
}

// FUNCTION: XW 0x4A7D50
void shellext_OptionsButtonUser(Input* input, int context) {
	PushButton* button = (PushButton*)input;
	int32_t outZ, outY, outX;
	uint32_t pressedButtons;
	uint32_t buttonMask;
	int buttonIndex;
	(void)context;
	pressedButtons = xjoy_Joystick_Read(&outX, &outY, &outZ, 0);
	buttonMask = 1;
	for (buttonIndex = 0; buttonIndex < JOYSTICK_ACTION_CAPACITY; ++buttonIndex) {
		if ((buttonMask & pressedButtons) != 0) {
			int entryIndex;
			g_optionsJoystickSlot = buttonIndex;
			xview_Refresh_View();
			for (entryIndex = 0; entryIndex < g_joystickEntryCount; ++entryIndex) {
				if (g_joystickEntries[entryIndex].actionCode ==
					g_shellPreferences.joystickActions[g_optionsJoystickSlot])
					g_optionsActionIndex = entryIndex;
			}
		}
		buttonMask <<= 1;
	}
	if (xinpattr_Get_Input_Selected(&button->header) != 0) {
		int16_t controlId = button->header.id;
		switch (controlId) {
			case XW_OPTIONS_OK:
			case XW_OPTIONS_EXIT:
				xdialog_Set_Dialog_Exit(controlId);
				break;
			case XW_OPTIONS_MUSIC:
				g_shellPreferences.musicEnabled ^= 1;
				g_flightMusicEnabled ^= 1;
				xbtnpush_Set_Button_LabelIndex(button, g_shellPreferences.musicEnabled);
				break;
			case XW_OPTIONS_SFX:
				g_shellPreferences.sfxEnabled ^= 1;
				g_flightSfxEnabled ^= 1;
				xbtnpush_Set_Button_LabelIndex(button, g_shellPreferences.sfxEnabled);
				break;
			case XW_OPTIONS_SPOKEN_TEXT:
				g_shellPreferences.spokenTextEnabled ^= 1;
				xbtnpush_Set_Button_LabelIndex(button, g_shellPreferences.spokenTextEnabled);
				xfade_SetSpeechTextEnabled(g_shellPreferences.spokenTextEnabled != 0 ||
										   ShellPreferences_GetSfxEnabled() == 0);
				break;
			case XW_OPTIONS_TRANSITIONS:
				g_shellPreferences.transitionsEnabled ^= 1;
				g_flightTransitionsEnabled ^= 1;
				xbtnpush_Set_Button_LabelIndex(button, g_shellPreferences.transitionsEnabled);
				break;
			case XW_OPTIONS_MISSION_SET:
#ifdef XW_MODERN
				if (XwProfile_MissionFlight()->version == XW_GAME_VERSION_93)
					break;
#endif
				g_shellPreferences.classicMissions ^= 1;
				g_classicMissionsMirror ^= 1;
				xbtnpush_Set_Button_LabelIndex(button, g_shellPreferences.classicMissions);
				break;
			case XW_OPTIONS_OPEN_FLIGHT:
				g_optionsPageIndex = XW_OPTIONS_PAGE_FLIGHT;
				xinpattr_Hide_Input(g_optionsGeneralPage);
				xinpattr_Show_Input(g_optionsFlightPage);
				xinpattr_Refresh_Input(g_optionsFlightPage);
				break;
			case XW_OPTIONS_OPEN_JOYSTICK:
				g_optionsPageIndex = XW_OPTIONS_PAGE_JOYSTICK;
				xinpattr_Hide_Input(g_optionsGeneralPage);
				xinpattr_Show_Input(g_optionsJoystickPage);
				xinpattr_Refresh_Input(g_optionsJoystickPage);
				break;
			case XW_OPTIONS_TEXTURE_QUALITY:
				++g_shellPreferences.textureQuality;
				if (g_shellPreferences.textureQuality > SHELLEXT_TEXTURE_QUALITY_MAX)
					g_shellPreferences.textureQuality = 0;
				g_modelTextureQuality = g_shellPreferences.textureQuality;
				xbtnpush_Set_Button_LabelIndex(button, g_shellPreferences.textureQuality);
				break;
			case XW_OPTIONS_BACK_FROM_FLIGHT:
				g_optionsPageIndex = XW_OPTIONS_PAGE_GENERAL;
				xinpattr_Hide_Input(g_optionsFlightPage);
				xinpattr_Show_Input(g_optionsGeneralPage);
				xinpattr_Refresh_Input(g_optionsGeneralPage);
				break;
			case XW_OPTIONS_BACK_FROM_JOYSTICK:
				g_optionsPageIndex = XW_OPTIONS_PAGE_GENERAL;
				xinpattr_Hide_Input(g_optionsJoystickPage);
				xinpattr_Show_Input(g_optionsGeneralPage);
				xinpattr_Refresh_Input(g_optionsDialog);
				break;
			case XW_OPTIONS_ENGINE_SOUND:
				g_shellPreferences.engineSoundEnabled ^= 1;
				g_flightEngineSoundEnabled ^= 1;
				xbtnpush_Set_Button_LabelIndex(button, g_shellPreferences.engineSoundEnabled);
				break;
			case XW_OPTIONS_RESOLUTION:
				++g_shellPreferences.resolutionIndex;
				if (g_shellPreferences.resolutionIndex > XW_SHELL_RESOLUTION_HARDWARE)
					g_shellPreferences.resolutionIndex = XW_SHELL_RESOLUTION_640_480_INDEXED;
				switch (g_shellPreferences.resolutionIndex) {
					case XW_SHELL_RESOLUTION_320_200:
						g_flightResolutionMode = RTSVGA2_MODE_13H;
						break;
					case XW_SHELL_RESOLUTION_640_480_INDEXED:
						g_flightResolutionMode = FLIGHT_DISPLAY_MODE_101H;
						break;
					case XW_SHELL_RESOLUTION_640_480_RGB:
						g_flightResolutionMode = FLIGHT_DISPLAY_MODE_111H;
						break;
					case XW_SHELL_RESOLUTION_HARDWARE:
						g_flightResolutionMode = FLIGHT_DISPLAY_MODE_1FFH;
						break;
				}
				xbtnpush_Set_Button_LabelIndex(button, g_shellPreferences.resolutionIndex);
				break;
			case XW_OPTIONS_PREVIOUS_ACTION_PAGE: {
				unsigned int pageSelection = g_optionsActionIndex;
				if (pageSelection >= SHELLEXT_ACTION_ROWS)
					pageSelection -= SHELLEXT_ACTION_ROWS;
				else
					pageSelection += g_joystickEntryCount - SHELLEXT_ACTION_ROWS;
				g_optionsActionIndex = SHELLEXT_ACTION_ROWS * (pageSelection / SHELLEXT_ACTION_ROWS);
				xview_Refresh_View();
				break;
			}
			case XW_OPTIONS_NEXT_ACTION_PAGE: {
				unsigned int pageSelection = g_optionsActionIndex + SHELLEXT_ACTION_ROWS;
				if (pageSelection >= (unsigned int)g_joystickEntryCount)
					pageSelection -= g_joystickEntryCount;
				g_optionsActionIndex = SHELLEXT_ACTION_ROWS * (pageSelection / SHELLEXT_ACTION_ROWS);
				xview_Refresh_View();
				break;
			}
			case XW_OPTIONS_RESET_BINDINGS:
				computer_ConfirmJoystickBindingReset(XwComputer_ApplyBindingReset, NULL);
				break;
			case XW_OPTIONS_PREVIOUS_JOYSTICK_SLOT: {
				unsigned int physicalButtonCount = Joystick_GetButtonCount();
				int entryIndex;
				if (g_optionsJoystickSlot == JOYSTICK_DEFAULT_ASSIGNMENT_LIMIT)
					g_optionsJoystickSlot = physicalButtonCount - 1;
				else if (g_optionsJoystickSlot == 0) {
					if (Joystick_HasPov() != 0)
						g_optionsJoystickSlot = JOYSTICK_ACTION_CAPACITY - 1;
					else
						g_optionsJoystickSlot = physicalButtonCount - 1;
				} else
					--g_optionsJoystickSlot;
				xview_Refresh_View();
				for (entryIndex = 0; entryIndex < g_joystickEntryCount; ++entryIndex) {
					if (g_joystickEntries[entryIndex].actionCode ==
						g_shellPreferences.joystickActions[g_optionsJoystickSlot])
						g_optionsActionIndex = entryIndex;
				}
				break;
			}
			case XW_OPTIONS_NEXT_JOYSTICK_SLOT: {
				int entryIndex;
				unsigned int lastPhysicalButton = Joystick_GetButtonCount() - 1;
				if ((unsigned int)g_optionsJoystickSlot == lastPhysicalButton) {
					if (Joystick_HasPov() != 0)
						g_optionsJoystickSlot = JOYSTICK_DEFAULT_ASSIGNMENT_LIMIT;
				} else if (g_optionsJoystickSlot == JOYSTICK_ACTION_CAPACITY - 1)
					g_optionsJoystickSlot = 0;
				else
					++g_optionsJoystickSlot;
				xview_Refresh_View();
				for (entryIndex = 0; entryIndex < g_joystickEntryCount; ++entryIndex) {
					if (g_joystickEntries[entryIndex].actionCode ==
						g_shellPreferences.joystickActions[g_optionsJoystickSlot])
						g_optionsActionIndex = entryIndex;
				}
				break;
			}
		}
	}
}

// GLOBAL: XW 0x5667BC
int g_optionsField5667BC = -1;

// GLOBAL: XW 0x5667C0
int g_optionsJoystickSlot = 0;

// GLOBAL: XW 0x5667C4
unsigned int g_optionsActionIndex = 0;

// GLOBAL: XW 0x5667CC
Input* g_optionsDialog = NULL;

// GLOBAL: XW 0x5667D0
int16_t g_optionsField5667D0 = -1;

// GLOBAL: XW 0x5667D8
Input* g_optionsGeneralPage = NULL;

// GLOBAL: XW 0x5667DC
Input* g_optionsFlightPage = NULL;

// GLOBAL: XW 0x5667E0
Input* g_optionsJoystickPage = NULL;

// FUNCTION: XW 0x4A84B0
int16_t shellext_UpdateOptionsInput(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
									int rightEvent, int16_t x, int16_t y) {
	(void)frame;
	(void)clip;
	if (key != 0) {
		uint8_t actionCode = shellext_KeyToJoystickAction(key);
		if (actionCode != 0) {
			int16_t actionIndex;
			for (actionIndex = 0; actionIndex < g_joystickEntryCount; ++actionIndex) {
				if (actionCode == g_joystickEntries[actionIndex].actionCode) {
					g_optionsActionIndex = actionIndex;
					g_shellPreferences.joystickActions[g_optionsJoystickSlot] =
						g_joystickEntries[actionIndex].actionCode;
					xview_Refresh_View();
					break;
				}
			}
			return 1;
		}
		return 0;
	}
	switch (input->id) {
		case XW_OPTIONS_MUSIC_VOLUME: {
			int value;
			if (leftEvent == SHELLEXT_SLIDER_IGNORED_EVENT || rightEvent == SHELLEXT_SLIDER_IGNORED_EVENT)
				return 1;
			value = x / SHELLEXT_VOLUME_SLIDER_STEP;
			if (g_shellPreferences.musicVolume != value) {
				g_shellPreferences.musicVolume = value;
				if (g_shellPreferences.musicVolume < 0)
					g_shellPreferences.musicVolume = 0;
				if (g_shellPreferences.musicVolume > INFLIGHT_OPTIONS_VOLUME_MAX)
					g_shellPreferences.musicVolume = INFLIGHT_OPTIONS_VOLUME_MAX;
				g_flightMusicVolume = g_shellPreferences.musicVolume;
				xinpattr_Refresh_Input(input);
			}
			return 1;
		}
		case XW_OPTIONS_SFX_VOLUME: {
			int value;
			if (leftEvent == SHELLEXT_SLIDER_IGNORED_EVENT || rightEvent == SHELLEXT_SLIDER_IGNORED_EVENT)
				return 1;
			value = x / SHELLEXT_VOLUME_SLIDER_STEP;
			if (g_shellPreferences.sfxVolume != value) {
				g_shellPreferences.sfxVolume = value;
				if (g_shellPreferences.sfxVolume < 0)
					g_shellPreferences.sfxVolume = 0;
				if (g_shellPreferences.sfxVolume > INFLIGHT_OPTIONS_VOLUME_MAX)
					g_shellPreferences.sfxVolume = INFLIGHT_OPTIONS_VOLUME_MAX;
				g_flightSfxVolume = g_shellPreferences.sfxVolume;
				xinpattr_Refresh_Input(input);
			}
			return 1;
		}
		case XW_OPTIONS_BRIGHTNESS: {
			int value;
			if (leftEvent == SHELLEXT_SLIDER_IGNORED_EVENT || rightEvent == SHELLEXT_SLIDER_IGNORED_EVENT)
				return 1;
			value = x / SHELLEXT_BRIGHTNESS_SLIDER_STEP;
			if (g_shellPreferences.brightness != value) {
				g_shellPreferences.brightness = value;
				if (g_shellPreferences.brightness < 0)
					g_shellPreferences.brightness = 0;
				if (g_shellPreferences.brightness > SHELLEXT_BRIGHTNESS_MAX)
					g_shellPreferences.brightness = SHELLEXT_BRIGHTNESS_MAX;
				g_flightBrightnessSetting = g_shellPreferences.brightness;
				xinpattr_Refresh_Input(input);
			}
			return 1;
		}
		case XW_OPTIONS_JOYSTICK_PAGE: {
			unsigned int pageStart = SHELLEXT_ACTION_ROWS * (g_optionsActionIndex / SHELLEXT_ACTION_ROWS);
			Rect rowRect;
			int16_t rowIndex;
			xrect_Set_Rect(&rowRect, SHELLEXT_ACTION_LIST_LEFT, SHELLEXT_ACTION_LIST_TOP,
						   SHELLEXT_ACTION_LIST_RIGHT, SHELLEXT_ACTION_LIST_BOTTOM);
			xrect_Inset_Rect(&rowRect, SHELLEXT_ACTION_LIST_INSET, 0);
			rowRect.bottom = rowRect.top + SHELLEXT_ACTION_ROW_HEIGHT;
			for (rowIndex = 0; rowIndex < SHELLEXT_ACTION_ROWS; ++rowIndex) {
				unsigned int entryIndex = rowIndex + pageStart;
				if (entryIndex >= (unsigned int)g_joystickEntryCount)
					break;
				if (xrect_Point_In_Rect(&rowRect, x, y) != 0) {
					g_optionsActionIndex = rowIndex + pageStart;
					g_shellPreferences.joystickActions[g_optionsJoystickSlot] =
						g_joystickEntries[entryIndex].actionCode;
					xview_Refresh_View();
				}
				xrect_Offset_Rect(&rowRect, 0, SHELLEXT_ACTION_ROW_HEIGHT);
			}
			return 1;
		}
		default:
			return 0;
	}
}

// FUNCTION: XW 0x4A88A0
void shellext_DrawOptionsInput(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	int16_t assignedActionIndex;
	int16_t paletteDelay;
	int audioPanelsRemaining;
	int musicBarIndex;
	int brightnessBarIndex;
	int sfxBarIndex;
	int16_t matchingActionIndex;

	unsigned int pageStart;
	int16_t rowIndex;
	unsigned int entryIndexForRow;
	Rect drawRect;
	char rowText[SHELLEXT_OPTIONS_TEXT_CAPACITY];

	(void)clip;
	assignedActionIndex = 0;
	if (input->id == XW_OPTIONS_ROOT) {
		paletteDelay = input->var1;
		if (paletteDelay != 0) {
			if (paletteDelay == 1) {
				xpal_Set_Screen_Palette(g_shellContext->standardPalette);
				input->var1 = 0;
			} else {
				input->var1 = paletteDelay - 1;
			}
		}
	}
	if (refresh != 0) {
		switch (input->id) {
			case XW_OPTIONS_ROOT:
				xpaint_Paint_Clipped_Rect(frame, 0);
				break;
			case XW_OPTIONS_GENERAL_PAGE:
				xrect_Copy_Rect(&drawRect, frame);
				xpaint_Frame_Clipped_Rect(&drawRect, SHELLEXT_OPTIONS_FRAME_COLOR);
				xrect_Inset_Rect(&drawRect, 1, 1);
				xstyle_Style_Paint_Border(&drawRect, 0);
				drawRect.bottom = drawRect.top + SHELLEXT_ACTION_ROW_HEIGHT;
				xfont_Enable_FontID_Shadow(0);
				xfont_Print_Centered_Text("Options Dialog", &drawRect, 0, SHELLEXT_OPTIONS_TEXT_COLOR);
				xrect_Set_Rect(&drawRect, SHELLEXT_OPTIONS_PANEL_LEFT, SHELLEXT_OPTIONS_AUDIO_TOP,
							   SHELLEXT_OPTIONS_PANEL_RIGHT, SHELLEXT_OPTIONS_AUDIO_BOTTOM);
				xrect_Offset_Rect(&drawRect, frame->left, frame->top);
				for (audioPanelsRemaining = SHELLEXT_OPTIONS_AUDIO_PANELS; audioPanelsRemaining != 0;
					 --audioPanelsRemaining) {
					xstyle_Style_Paint_Border(&drawRect, 0);
					xfont_Print_Clipped_Text("Volume", drawRect.left + SHELLEXT_OPTIONS_VOLUME_LABEL_X,
											 drawRect.top + SHELLEXT_OPTIONS_VOLUME_LABEL_Y, 0,
											 SHELLEXT_OPTIONS_TEXT_COLOR);
					xrect_Offset_Rect(&drawRect, 0, SHELLEXT_OPTIONS_AUDIO_SPACING);
				}
				xfont_Disable_FontID_Shadow(0);
				break;
			case XW_OPTIONS_MUSIC_VOLUME:
				xstyle_Style_Paint_TextField(frame);
				xrect_Copy_Rect(&drawRect, frame);
				xrect_Inset_Rect(&drawRect, 2, 2);
				drawRect.right = drawRect.left + SHELLEXT_VOLUME_SLIDER_STEP - 1;
				for (musicBarIndex = 0; (int16_t)musicBarIndex < g_shellPreferences.musicVolume;
					 ++musicBarIndex) {
					xpaint_Paint_Clipped_Rect(&drawRect, SHELLEXT_OPTIONS_BAR_COLOR);
					xrect_Offset_Rect(&drawRect, SHELLEXT_VOLUME_SLIDER_STEP, 0);
				}
				break;
			case XW_OPTIONS_BRIGHTNESS:
				xstyle_Style_Paint_TextField(frame);
				xrect_Copy_Rect(&drawRect, frame);
				xrect_Inset_Rect(&drawRect, 2, 2);
				drawRect.right = drawRect.left + SHELLEXT_BRIGHTNESS_SLIDER_STEP - 1;
				for (brightnessBarIndex = 0; (int16_t)brightnessBarIndex < g_shellPreferences.brightness;
					 ++brightnessBarIndex) {
					xpaint_Paint_Clipped_Rect(&drawRect, SHELLEXT_OPTIONS_BAR_COLOR);
					xrect_Offset_Rect(&drawRect, SHELLEXT_BRIGHTNESS_SLIDER_STEP, 0);
				}
				break;
			case XW_OPTIONS_SFX_VOLUME:
				xstyle_Style_Paint_TextField(frame);
				xrect_Copy_Rect(&drawRect, frame);
				xrect_Inset_Rect(&drawRect, 2, 2);
				drawRect.right = drawRect.left + SHELLEXT_VOLUME_SLIDER_STEP - 1;
				for (sfxBarIndex = 0; (int16_t)sfxBarIndex < g_shellPreferences.sfxVolume; ++sfxBarIndex) {
					xpaint_Paint_Clipped_Rect(&drawRect, SHELLEXT_OPTIONS_BAR_COLOR);
					xrect_Offset_Rect(&drawRect, SHELLEXT_VOLUME_SLIDER_STEP, 0);
				}
				break;
			case XW_OPTIONS_FLIGHT_PAGE:
				xrect_Copy_Rect(&drawRect, frame);
				xpaint_Frame_Clipped_Rect(&drawRect, SHELLEXT_OPTIONS_FRAME_COLOR);
				xrect_Inset_Rect(&drawRect, 1, 1);
				xstyle_Style_Paint_Border(&drawRect, 0);
				drawRect.bottom = drawRect.top + SHELLEXT_ACTION_ROW_HEIGHT;
				xfont_Enable_FontID_Shadow(0);
				xfont_Print_Centered_Text("Flight Options", &drawRect, 0, SHELLEXT_OPTIONS_TEXT_COLOR);
				xrect_Set_Rect(&drawRect, SHELLEXT_OPTIONS_PANEL_LEFT, SHELLEXT_OPTIONS_BRIGHTNESS_TOP,
							   SHELLEXT_OPTIONS_PANEL_RIGHT, SHELLEXT_OPTIONS_BRIGHTNESS_BOTTOM);
				xrect_Offset_Rect(&drawRect, frame->left, frame->top);
				xstyle_Style_Paint_Border(&drawRect, 0);
				drawRect.bottom = drawRect.top + SHELLEXT_OPTIONS_HEADER_HEIGHT;
				xfont_Print_Centered_Text("Brightness", &drawRect, 0, SHELLEXT_OPTIONS_TEXT_COLOR);
				xfont_Disable_FontID_Shadow(0);
				break;
			case XW_OPTIONS_JOYSTICK_PAGE:
				xrect_Copy_Rect(&drawRect, frame);
				xpaint_Frame_Clipped_Rect(&drawRect, SHELLEXT_OPTIONS_FRAME_COLOR);
				xrect_Inset_Rect(&drawRect, 1, 1);
				xstyle_Style_Paint_Border(&drawRect, 0);
				drawRect.bottom = drawRect.top + SHELLEXT_ACTION_ROW_HEIGHT;
				xfont_Enable_FontID_Shadow(0);
				xfont_Print_Centered_Text("Configure Joystick", &drawRect, 0, SHELLEXT_OPTIONS_TEXT_COLOR);
				xrect_Offset_Rect(&drawRect, 0, SHELLEXT_OPTIONS_HEADER_HEIGHT);
				xfont_Print_Centered_Text("Press a joystick button to view/change its configuration",
										  &drawRect, 0, SHELLEXT_OPTIONS_TEXT_COLOR);
				xrect_Offset_Rect(&drawRect, 0, SHELLEXT_OPTIONS_HEADER_HEIGHT);
				xfont_Disable_FontID_Shadow(0);
				xrect_Set_Rect(&drawRect, SHELLEXT_ACTION_LIST_LEFT, SHELLEXT_ACTION_LIST_TOP,
							   SHELLEXT_ACTION_LIST_RIGHT, SHELLEXT_ACTION_LIST_BOTTOM);
				xrect_Offset_Rect(&drawRect, frame->left, frame->top);
				xpaint_Paint_Clipped_Rect(&drawRect, 0);
				for (matchingActionIndex = 0;
					 (unsigned int)matchingActionIndex < (unsigned int)g_joystickEntryCount;
					 ++matchingActionIndex) {
					if (g_joystickEntries[matchingActionIndex].actionCode ==
						g_shellPreferences.joystickActions[g_optionsJoystickSlot])
						break;
				}
				xrect_Offset_Rect(&drawRect, 0, -SHELLEXT_OPTIONS_HEADER_HEIGHT);
				drawRect.bottom = drawRect.top + SHELLEXT_OPTIONS_HEADER_HEIGHT;
				if ((unsigned int)matchingActionIndex < (unsigned int)g_joystickEntryCount) {
					sprintf(rowText, "%s: %s", g_optionsJoystickSlotLabels[g_optionsJoystickSlot].text,
							g_joystickEntries[matchingActionIndex].name);
					xfont_Print_Centered_Text(rowText, &drawRect, 0, SHELLEXT_OPTIONS_TEXT_COLOR);
					assignedActionIndex = matchingActionIndex;
				}
				xrect_Set_Rect(&drawRect, SHELLEXT_ACTION_LIST_LEFT, SHELLEXT_ACTION_LIST_TOP,
							   SHELLEXT_ACTION_LIST_RIGHT, SHELLEXT_ACTION_LIST_BOTTOM);
				xrect_Offset_Rect(&drawRect, frame->left, frame->top);
				pageStart = SHELLEXT_ACTION_ROWS * (g_optionsActionIndex / SHELLEXT_ACTION_ROWS);
				xrect_Inset_Rect(&drawRect, SHELLEXT_ACTION_LIST_INSET, 0);
				drawRect.bottom = drawRect.top + SHELLEXT_ACTION_ROW_HEIGHT;
				for (rowIndex = 0; rowIndex < SHELLEXT_ACTION_ROWS; ++rowIndex) {
					entryIndexForRow = pageStart + rowIndex;
					if (entryIndexForRow >= (unsigned int)g_joystickEntryCount)
						break;
					sprintf(rowText, "%s: %s", g_joystickEntries[entryIndexForRow].name,
							g_joystickEntries[entryIndexForRow].description);
					if (entryIndexForRow == (unsigned int)assignedActionIndex)
						xfont_Print_Clipped_Text(rowText, drawRect.left, drawRect.top + 2, 0,
												 SHELLEXT_OPTIONS_TEXT_COLOR);
					else
						xfont_Print_Clipped_Text(rowText, drawRect.left, drawRect.top + 2, 0,
												 SHELLEXT_OPTIONS_ACTION_COLOR);
					xrect_Offset_Rect(&drawRect, 0, SHELLEXT_ACTION_ROW_HEIGHT);
				}
				break;
			default:
				return;
		}
	}
}

// FUNCTION: XW 0x4A8FE0
void shellext_DetectInstalledContent(void) {
	char markerPath[XW_CONTENT_MARKER_PATH_CAPACITY];
#ifdef XW_MODERN
	static XwGameVersion detectedVersion;
	XwGameVersion version = XwProfile_MissionFlight()->mission_version;
	if (detectedVersion != version)
		g_contentAvailabilityInitialized = 0;
	detectedVersion = version;
#endif
	if (!g_contentAvailabilityInitialized) {
		int markerNumber;
		int markerIndex;
		int remaining;
		g_contentAvailabilityInitialized = 1;
		for (markerIndex = 0, markerNumber = 1, remaining = SHIPEXT_SHIP_COUNT; remaining != 0;
			 ++markerNumber, ++markerIndex, --remaining) {
			LandruFile* marker;
			sprintf(markerPath, "X-Wing Data\\ship%d.xid", markerNumber);
#ifdef XW_MODERN
			marker = XwStorage_OpenInstallation(version, markerPath);
#else
			marker = xfile_Open_File(LANDRU_FILE_ROOT_ASSET, markerPath, "r");
#endif
			if (marker) {
				g_availableShips[markerIndex] = 1;
				xfile_Close_File(marker);
			} else {
				g_availableShips[markerIndex] = 0;
			}
		}
		for (markerIndex = 0, markerNumber = 1, remaining = SHIPEXT_TOUR_COUNT; remaining != 0;
			 ++markerNumber, ++markerIndex, --remaining) {
			LandruFile* marker;
			sprintf(markerPath, "X-Wing Data\\tour%d.xid", markerNumber);
#ifdef XW_MODERN
			marker = XwStorage_OpenInstallation(version, markerPath);
#else
			marker = xfile_Open_File(LANDRU_FILE_ROOT_ASSET, markerPath, "r");
#endif
			if (marker) {
				g_availableTours[markerIndex] = 1;
				xfile_Close_File(marker);
			} else {
				g_availableTours[markerIndex] = 0;
			}
		}
	}
}

// FUNCTION: XW 0x4A90C0
int shellext_LoadJoystickActionDictionary(void) {
	if (g_joystickEntryCount == 0) {
		char token[SHELLEXT_JOYSTICK_PARSE_CAPACITY];
		char lineBuffer[SHELLEXT_JOYSTICK_PARSE_CAPACITY];
		char* actionText;
#ifdef XW_MODERN
		XwFile* stream = File_RawOpenText("X-Wing Data\\joystick.txt", "r");
#else
		LandruFile* stream = xfile_Open_File(LANDRU_FILE_ROOT_ASSET, "X-Wing Data\\joystick.txt", "r");
#endif
		if (stream == NULL)
			return 0;
		for (g_joystickEntryCount = 0;; ++g_joystickEntryCount) {
			int tokenLength;
			int entryIndex;
			char* nameText;
			char character;
#ifdef XW_MODERN
			actionText = File_RawReadTextLine(lineBuffer, SHELLEXT_JOYSTICK_READ_CAPACITY, stream);
#else
			actionText = fgets(lineBuffer, SHELLEXT_JOYSTICK_READ_CAPACITY, (FILE*)stream);
#endif
			if (actionText == NULL)
				break;
			if (lineBuffer[strlen(lineBuffer) - 1] == '\n')
				lineBuffer[strlen(lineBuffer) - 1] = '\0';
			for (tokenLength = 0, character = actionText[0];
				 character != ' ' && character != '\0' && tokenLength < (int)sizeof(token);
				 ++tokenLength, character = actionText[tokenLength])
				token[tokenLength] = character;
			token[tokenLength] = '\0';
			nameText = actionText + tokenLength + 1;
			entryIndex = g_joystickEntryCount;
			g_joystickEntries[entryIndex].actionCode = atoi(token);
			for (tokenLength = 0, character = nameText[0];
				 character != ' ' && character != '\0' && tokenLength < (int)sizeof(token);
				 ++tokenLength, character = nameText[tokenLength])
				token[tokenLength] = character;
			token[tokenLength] = '\0';
			strcpy(g_joystickEntries[entryIndex].name, token);
			strcpy(g_joystickEntries[entryIndex].description, nameText + tokenLength + 1);
		}
#ifdef XW_MODERN
		File_RawClose(stream);
#else
		xfile_Close_File(stream);
#endif
	}
	return 1;
}

// FUNCTION: XW 0x4A9250
uint8_t shellext_KeyToJoystickAction(int16_t key) {
	int extendedKey;
	if (key == 0)
		return JOYSTICK_ACTION_NONE;
	if (key < SHELLEXT_KEY_EXTENDED)
		return (uint8_t)key;
	/* Preserve the original left shift and low-byte narrowing of function keys. */
	if (key >= SHELLEXT_KEY_F1 && key <= SHELLEXT_KEY_F10)
		return (uint8_t)((key << SHELLEXT_KEY_SCAN_SHIFT) - (SHELLEXT_KEY_F1 - JOYSTICK_ACTION_F1));
	if (key >= SHELLEXT_KEY_SHIFT_F1 && key <= SHELLEXT_KEY_SHIFT_F10)
		return (uint8_t)((key << SHELLEXT_KEY_SCAN_SHIFT) -
						 (SHELLEXT_KEY_SHIFT_F1 - JOYSTICK_ACTION_SHIFT_F1));
	extendedKey = key;
	switch (extendedKey) {
		case SHELLEXT_KEY_ALT_Q:
			return JOYSTICK_ACTION_KEY_ALT_Q;
		case SHELLEXT_KEY_ALT_W:
			return JOYSTICK_ACTION_KEY_ALT_W;
		case SHELLEXT_KEY_ALT_E:
			return JOYSTICK_ACTION_KEY_ALT_E;
		case SHELLEXT_KEY_ALT_R:
			return JOYSTICK_ACTION_KEY_ALT_R;
		case SHELLEXT_KEY_ALT_T:
			return JOYSTICK_ACTION_KEY_ALT_T;
		case SHELLEXT_KEY_ALT_Y:
			return JOYSTICK_ACTION_KEY_ALT_Y;
		case SHELLEXT_KEY_ALT_U:
			return JOYSTICK_ACTION_KEY_ALT_U;
		case SHELLEXT_KEY_ALT_I:
			return JOYSTICK_ACTION_KEY_ALT_I;
		case SHELLEXT_KEY_ALT_O:
			return JOYSTICK_ACTION_KEY_ALT_O;
		case SHELLEXT_KEY_ALT_P:
			return JOYSTICK_ACTION_KEY_ALT_P;
		case SHELLEXT_KEY_ALT_A:
			return JOYSTICK_ACTION_KEY_ALT_A;
		case SHELLEXT_KEY_ALT_S:
			return JOYSTICK_ACTION_KEY_ALT_S;
		case SHELLEXT_KEY_ALT_D:
			return JOYSTICK_ACTION_KEY_ALT_D;
		case SHELLEXT_KEY_ALT_F:
			return JOYSTICK_ACTION_KEY_ALT_F;
		case SHELLEXT_KEY_ALT_G:
			return JOYSTICK_ACTION_KEY_ALT_G;
		case SHELLEXT_KEY_ALT_H:
			return JOYSTICK_ACTION_KEY_ALT_H;
		case SHELLEXT_KEY_ALT_J:
			return JOYSTICK_ACTION_KEY_ALT_J;
		case SHELLEXT_KEY_ALT_K:
			return JOYSTICK_ACTION_KEY_ALT_K;
		case SHELLEXT_KEY_ALT_L:
			return JOYSTICK_ACTION_KEY_ALT_L;
		case SHELLEXT_KEY_ALT_Z:
			return JOYSTICK_ACTION_KEY_ALT_Z;
		case SHELLEXT_KEY_ALT_X:
			return JOYSTICK_ACTION_KEY_ALT_X;
		case SHELLEXT_KEY_ALT_C:
			return JOYSTICK_ACTION_KEY_ALT_C;
		case SHELLEXT_KEY_ALT_V:
			return JOYSTICK_ACTION_KEY_ALT_V;
		case SHELLEXT_KEY_ALT_B:
			return JOYSTICK_ACTION_KEY_ALT_B;
		case SHELLEXT_KEY_ALT_N:
			return JOYSTICK_ACTION_KEY_ALT_N;
		case SHELLEXT_KEY_ALT_M:
			return JOYSTICK_ACTION_KEY_ALT_M;
		case SHELLEXT_KEY_HOME:
			return JOYSTICK_ACTION_KEY_HOME;
		case SHELLEXT_KEY_UP:
			return JOYSTICK_ACTION_KEY_UP;
		case SHELLEXT_KEY_PAGE_UP:
			return JOYSTICK_ACTION_KEY_PAGE_UP;
		case SHELLEXT_KEY_LEFT:
			return JOYSTICK_ACTION_KEY_LEFT;
		case SHELLEXT_KEY_RIGHT:
			return JOYSTICK_ACTION_KEY_RIGHT;
		case SHELLEXT_KEY_END:
			return JOYSTICK_ACTION_KEY_END;
		case SHELLEXT_KEY_DOWN:
			return JOYSTICK_ACTION_KEY_DOWN;
		case SHELLEXT_KEY_PAGE_DOWN:
			return JOYSTICK_ACTION_KEY_PAGE_DOWN;
		case SHELLEXT_KEY_INSERT:
			return JOYSTICK_ACTION_KEY_INSERT;
		case SHELLEXT_KEY_DELETE:
			return JOYSTICK_ACTION_KEY_DELETE;
		case SHELLEXT_KEY_ALT_1:
			return JOYSTICK_ACTION_KEY_ALT_1;
		case SHELLEXT_KEY_ALT_2:
			return JOYSTICK_ACTION_KEY_ALT_2;
		case SHELLEXT_KEY_ALT_3:
			return JOYSTICK_ACTION_KEY_ALT_3;
		case SHELLEXT_KEY_ALT_4:
			return JOYSTICK_ACTION_KEY_ALT_4;
		case SHELLEXT_KEY_ALT_5:
			return JOYSTICK_ACTION_KEY_ALT_5;
		case SHELLEXT_KEY_ALT_6:
			return JOYSTICK_ACTION_KEY_ALT_6;
		case SHELLEXT_KEY_ALT_7:
			return JOYSTICK_ACTION_KEY_ALT_7;
		case SHELLEXT_KEY_ALT_8:
			return JOYSTICK_ACTION_KEY_ALT_8;
		case SHELLEXT_KEY_ALT_9:
			return JOYSTICK_ACTION_KEY_ALT_9;
		case SHELLEXT_KEY_ALT_0:
			return JOYSTICK_ACTION_KEY_ALT_0;
		default:
			return JOYSTICK_ACTION_NONE;
	}
}
