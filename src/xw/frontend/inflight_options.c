#include "xw/frontend/inflight_options.h"

#include "xw/audio/fsfx.h"
#include "xw/audio/soundext.h"
#include "xw/flight/player/user.h"
#include "xw/flight/replay/replay.h"
#include "xw/frontend/inflight_ui.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/util/landru_display.h"
#include "xw_runtime/integration/inflight_callbacks.h"

#include <landru/btnpush.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/film.h>
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

// GLOBAL: XW 0x4D4350
uint8_t g_flightHighDetailStarfield = 1;

// GLOBAL: XW 0x4D4354
uint8_t g_flightBackdropsPreference = 1;

// GLOBAL: XW 0x4D4358
uint8_t g_flightDebrisPreference = 1;

// GLOBAL: XW 0x4D435C
int8_t g_flightMarkingsPreference = 1;

// GLOBAL: XW 0x4D4360
uint8_t g_flightEngineGlowPreference = 1;

// GLOBAL: XW 0x4D4364
int8_t g_flightStarfighterDetail = INFLIGHT_OPTIONS_DETAIL_MAX;

// GLOBAL: XW 0x4D4368
int8_t g_flightStarshipDetail = INFLIGHT_OPTIONS_DETAIL_MAX;

// GLOBAL: XW 0x4D436C
int8_t g_flightDeathStarDetail = INFLIGHT_OPTIONS_DETAIL_MAX;

// GLOBAL: XW 0x4D43C0
char g_inflightOptionsPageTitles[INFLIGHT_OPTIONS_PAGE_COUNT][INFLIGHT_OPTIONS_TITLE_CAPACITY] = {
	"Inflight Options", "Inflight Detail"
};

// GLOBAL: XW 0x4D4400
char g_inflightOptionsDiskCacheLabels[INFLIGHT_OPTIONS_TOGGLE_LABEL_COUNT]
									 [INFLIGHT_OPTIONS_CACHE_LABEL_CAPACITY] = { "Film Disk Cache is Off",
																				 "Film Disk Cache is On" };

// GLOBAL: XW 0x4D4450
char g_inflightOptionsCacheOffText[INFLIGHT_OPTIONS_CACHE_LABEL_CAPACITY] = "Film Disk Cache is Off";

// GLOBAL: XW 0x4D4478
char g_inflightOptionsCacheOnText[INFLIGHT_OPTIONS_CACHE_LABEL_CAPACITY] = "Film Disk Cache is On";

// GLOBAL: XW 0x4D44A0
char g_inflightOptionsMusicLabels[INFLIGHT_OPTIONS_TOGGLE_LABEL_CAPACITY] = "Music is Off\0Music is On";

// GLOBAL: XW 0x4D44C0
char g_inflightOptionsSoundLabels[INFLIGHT_OPTIONS_TOGGLE_LABEL_CAPACITY] = "Sound is Off\0Sound is On";

// GLOBAL: XW 0x4D4520
char g_inflightOptionsWeaponsLabels[INFLIGHT_OPTIONS_TOGGLE_LABEL_COUNT]
								   [INFLIGHT_OPTIONS_TOGGLE_LABEL_CAPACITY] = {
									   "You have Limited Weapons", "You have Unlimited Weapons"
								   };

// GLOBAL: XW 0x4D4560
char g_inflightOptionsInvulnerabilityLabels[INFLIGHT_OPTIONS_TOGGLE_LABEL_COUNT]
										   [INFLIGHT_OPTIONS_TOGGLE_LABEL_CAPACITY] = {
											   "You are Vulnerable", "You are Invulnerable"
										   };

// GLOBAL: XW 0x4D45A0
char g_inflightOptionsCollisionsLabels[INFLIGHT_OPTIONS_TOGGLE_LABEL_COUNT]
									  [INFLIGHT_OPTIONS_TOGGLE_LABEL_CAPACITY] = {
										  "Starfighter Collision is Off", "Starfighter Collision is On"
									  };

// GLOBAL: XW 0x4D45E0
char g_inflightOptionsDigitalSoundLabels[INFLIGHT_OPTIONS_TOGGLE_LABEL_COUNT]
										[INFLIGHT_OPTIONS_TOGGLE_LABEL_CAPACITY] = { "Digital Sound is Off",
																					 "Digital Sound is On" };

// GLOBAL: XW 0x4D4620
char g_inflightOptionsSpeechLabels[INFLIGHT_OPTIONS_TOGGLE_LABEL_COUNT]
								  [INFLIGHT_OPTIONS_TOGGLE_LABEL_CAPACITY] = { "Speech is Off",
																			   "Speech is On" };

// GLOBAL: XW 0x4D4660
char g_inflightOptionsBackdropsLabels[INFLIGHT_OPTIONS_TOGGLE_LABEL_COUNT]
									 [INFLIGHT_OPTIONS_TOGGLE_LABEL_CAPACITY] = {
										 "Planets and Galaxies are Off", "Planets and Galaxies are On"
									 };

// GLOBAL: XW 0x4D46A0
char g_inflightOptionsDebrisLabels[INFLIGHT_OPTIONS_TOGGLE_LABEL_COUNT]
								  [INFLIGHT_OPTIONS_TOGGLE_LABEL_CAPACITY] = { "Space Debris is Off",
																			   "Space Debris is On" };

// GLOBAL: XW 0x4D46E0
char g_inflightOptionsStarfieldLabels[INFLIGHT_OPTIONS_TOGGLE_LABEL_COUNT]
									 [INFLIGHT_OPTIONS_TOGGLE_LABEL_CAPACITY] = {
										 "Starfield is Low Detail", "Starfield is High Detail"
									 };

// GLOBAL: XW 0x4D47C0
char g_inflightOptionsEngineGlowLabels[INFLIGHT_OPTIONS_TOGGLE_LABEL_COUNT]
									  [INFLIGHT_OPTIONS_TOGGLE_LABEL_CAPACITY] = { "Engine Glow is Off",
																				   "Engine Glow is On" };

// GLOBAL: XW 0x4D4800
char g_inflightOptionsInterlaceLabels[INFLIGHT_OPTIONS_TOGGLE_LABEL_COUNT]
									 [INFLIGHT_OPTIONS_TOGGLE_LABEL_CAPACITY] = { "Interlace is Off",
																				  "Interlace is On" };

// GLOBAL: XW 0x4D4840
const int16_t g_inflightOptionsFocusPage0X[INFLIGHT_OPTIONS_PAGE0_FOCUS_COUNT] = {
	17,  17,  17,  304, 304, 107, 229, 254, 279, 305, 107, 229, 254, 279, 305,
	82,  82,  82,  238, 238, 112, 112, 236, 286, 286, 160, 160, 160, 160, 160,
	160, 160, 160, 160, 160, 160, 160, 160, 160, 160, 75,  75,  75,  238, 238
};

// GLOBAL: XW 0x4D48A0
const int16_t g_inflightOptionsFocusPage0Y[INFLIGHT_OPTIONS_PAGE0_FOCUS_COUNT] = {
	14,  14,  14,  14,  14,  38,  38,  38,  38,  38,  58,  58,  58,  58,  58,
	79,  79,  79,  79,  79,  100, 100, 100, 100, 100, 124, 124, 124, 124, 124,
	144, 144, 144, 144, 144, 164, 164, 164, 164, 164, 186, 186, 186, 186, 186
};

// GLOBAL: XW 0x4D4900
const int16_t g_inflightOptionsFocusPage1X[INFLIGHT_OPTIONS_PAGE1_FOCUS_COUNT] = {
	17,  17,  17,  304, 304, 160, 160, 160, 160, 160, 160, 160, 160, 160, 160, 160, 160,
	160, 160, 160, 160, 160, 160, 160, 160, 160, 160, 160, 160, 160, 193, 208, 223, 238,
	248, 193, 208, 223, 238, 248, 193, 208, 223, 238, 248, 75,  75,  75,  238, 238
};

// GLOBAL: XW 0x4D4968
const int16_t g_inflightOptionsFocusPage1Y[INFLIGHT_OPTIONS_PAGE1_FOCUS_COUNT] = {
	14,  14,  14,  14,  14,  38,  38,  38,  38,  38,  58,  58,  58,  58,  58,  78,  78,
	78,  78,  78,  100, 100, 100, 100, 100, 120, 120, 120, 120, 120, 138, 138, 138, 138,
	138, 152, 152, 152, 152, 152, 166, 166, 166, 166, 166, 186, 186, 186, 186, 186
};

// GLOBAL: XW 0x4F76B0
int16_t g_inflightOptionsPage = 0;

// GLOBAL: XW 0x4F76B8
Input* g_inflightOptionsRootInput = NULL;

// GLOBAL: XW 0x4F76C0
Input* g_inflightOptionsPageInputs[INFLIGHT_OPTIONS_PAGE_COUNT] = { NULL, NULL };

// GLOBAL: XW 0x4F76C8
int g_inflightOptionsMusicContext = 0;

// GLOBAL: XW 0x4F76D4
uint8_t g_flightInterlaceEnabled = 0;

// GLOBAL: XW 0x4F76D8
int16_t g_inflightOptionsFocusIndex = 0;

#ifndef XW_MODERN
// FUNCTION: XW 0x44E490
XwShellSceneResult InflightOptions_Show(struct XwShellContext* shell) {
	ResFile* resourceFile;
	Film* film;
	Input* titleInput;
	Input* slider;
	PushButton* previousPageButton;
	PushButton* nextPageButton;
	PushButton* returnButton;
	PushButton* exitButton;
	int16_t pageIndex;
	PushButton* speechToggle;
	PushButton* cacheToggle;
	PushButton* weaponsToggle;
	PushButton* invulnerabilityToggle;
	PushButton* collisionsToggle;
	PushButton* backdropsToggle;
	PushButton* debrisToggle;
	PushButton* starfieldToggle;
	PushButton* interlaceToggle;
	PushButton* musicToggle;
	PushButton* soundToggle;
	PushButton* digitalSoundToggle;
	PushButton* engineGlowToggle;
	Rect controlRect;
	char cacheSizeSuffix[INFLIGHT_OPTIONS_CACHE_SUFFIX_CAPACITY];
	char cacheLabel[INFLIGHT_OPTIONS_CACHE_LABEL_CAPACITY];
	LandruDisplay_SetLowResolutionMode(0);
	sprintf(cacheSizeSuffix, " (%uK)", g_flightReplayDiskCacheKB);
	strcpy(cacheLabel, g_inflightOptionsCacheOffText);
	strcat(cacheLabel, cacheSizeSuffix);
	strcpy(g_inflightOptionsDiskCacheLabels[0], cacheLabel);
	strcpy(cacheLabel, g_inflightOptionsCacheOnText);
	strcat(cacheLabel, cacheSizeSuffix);
	strcpy(g_inflightOptionsDiskCacheLabels[1], cacheLabel);
	g_inflightOptionsPage = 0;
	g_inflightOptionsFocusIndex = INFLIGHT_OPTIONS_INITIAL_FOCUS;
	g_shellPreferences = g_savedShellPreferences.preferences;
	xio_Set_Mouse_Position(INFLIGHT_OPTIONS_INITIAL_MOUSE_X, INFLIGHT_OPTIONS_INITIAL_MOUSE_Y);
	resourceFile = xres_Open_Resource("inflight.lfd");
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xrect_Set_Rect(&controlRect, INFLIGHT_OPTIONS_CANVAS_LEFT, INFLIGHT_OPTIONS_CANVAS_TOP,
				   INFLIGHT_OPTIONS_CANVAS_RIGHT, INFLIGHT_OPTIONS_CANVAS_BOTTOM);
	film = xfilm_Res_Film(resourceFile, "ifoption", &controlRect, 0, 0, 0);
	xfilm_Set_Film_Def_Palette(film, shell->standardPalette);
	xrect_Set_Rect(&controlRect, INFLIGHT_OPTIONS_INPUT_FRAME_LEFT, INFLIGHT_OPTIONS_INPUT_FRAME_TOP,
				   INFLIGHT_OPTIONS_INPUT_FRAME_RIGHT, INFLIGHT_OPTIONS_INPUT_FRAME_BOTTOM);
	g_inflightOptionsRootInput = xinput_Alloc_Input(NULL, &controlRect, 0, 0);
	xinpattr_Set_Input_Draw_Function(g_inflightOptionsRootInput, InflightOptions_DrawPageBackground);
	xinpattr_Set_Input_Allign(g_inflightOptionsRootInput, INFLIGHT_OPTIONS_ALIGN_CENTER,
							  INFLIGHT_OPTIONS_ALIGN_CENTER);
	xrect_Set_Rect(&controlRect, INFLIGHT_OPTIONS_TITLE_FRAME_LEFT, INFLIGHT_OPTIONS_TITLE_FRAME_TOP,
				   INFLIGHT_OPTIONS_TITLE_FRAME_RIGHT, INFLIGHT_OPTIONS_TITLE_FRAME_BOTTOM);
	titleInput = xinput_Alloc_Input(g_inflightOptionsRootInput, &controlRect, 0, 0);
	xinpattr_Set_Input_Draw_Function(titleInput, InflightOptions_DrawNavigation);
	titleInput->id = INFLIGHT_OPTIONS_TITLE_ID;
	xrect_Set_Rect(&controlRect, INFLIGHT_OPTIONS_PREVIOUS_FRAME_LEFT, INFLIGHT_OPTIONS_PREVIOUS_FRAME_TOP,
				   INFLIGHT_OPTIONS_PREVIOUS_FRAME_RIGHT, INFLIGHT_OPTIONS_PREVIOUS_FRAME_BOTTOM);
	previousPageButton =
		xbtnpush_Alloc_Button(g_inflightOptionsRootInput, &controlRect, 0, InflightOptions_HandleNavigation,
							  NULL, INFLIGHT_OPTIONS_PREVIOUS_ID);
	xinpattr_Set_Input_Draw_Function(&previousPageButton->header, InflightOptions_DrawNavigation);
	xrect_Set_Rect(&controlRect, INFLIGHT_OPTIONS_NEXT_FRAME_LEFT, INFLIGHT_OPTIONS_NEXT_FRAME_TOP,
				   INFLIGHT_OPTIONS_NEXT_FRAME_RIGHT, INFLIGHT_OPTIONS_NEXT_FRAME_BOTTOM);
	nextPageButton = xbtnpush_Alloc_Button(g_inflightOptionsRootInput, &controlRect, 0,
										   InflightOptions_HandleNavigation, NULL, INFLIGHT_OPTIONS_NEXT_ID);
	xinpattr_Set_Input_Draw_Function(&nextPageButton->header, InflightOptions_DrawNavigation);
	xrect_Set_Rect(&controlRect, INFLIGHT_OPTIONS_RESUME_FRAME_LEFT, INFLIGHT_OPTIONS_RESUME_FRAME_TOP,
				   INFLIGHT_OPTIONS_RESUME_FRAME_RIGHT, INFLIGHT_OPTIONS_RESUME_FRAME_BOTTOM);
	returnButton =
		xbtnpush_Alloc_Button(g_inflightOptionsRootInput, &controlRect, 0, InflightOptions_HandleNavigation,
							  "Return to Simulator", INFLIGHT_OPTIONS_RESUME_ID);
	xinpattr_Set_Input_Draw_Function(&returnButton->header, XwInflight_DrawTextButton);
	xrect_Set_Rect(&controlRect, INFLIGHT_OPTIONS_ESCAPE_FRAME_LEFT, INFLIGHT_OPTIONS_ESCAPE_FRAME_TOP,
				   INFLIGHT_OPTIONS_ESCAPE_FRAME_RIGHT, INFLIGHT_OPTIONS_ESCAPE_FRAME_BOTTOM);
	exitButton =
		xbtnpush_Alloc_Button(g_inflightOptionsRootInput, &controlRect, 0, InflightOptions_HandleNavigation,
							  "Exit to Windows", INFLIGHT_OPTIONS_ESCAPE_ID);
	xinpattr_Set_Input_Draw_Function(&exitButton->header, XwInflight_DrawTextButton);
	xrect_Set_Rect(&controlRect, INFLIGHT_OPTIONS_PAGE_FRAME_LEFT, INFLIGHT_OPTIONS_PAGE_FRAME_TOP,
				   INFLIGHT_OPTIONS_PAGE_FRAME_RIGHT, INFLIGHT_OPTIONS_PAGE_FRAME_BOTTOM);
	for (pageIndex = 0; pageIndex != INFLIGHT_OPTIONS_PAGE_COUNT; ++pageIndex) {
		g_inflightOptionsPageInputs[pageIndex] =
			xinput_Alloc_Input(g_inflightOptionsRootInput, &controlRect, 0, 0);
		if (pageIndex != INFLIGHT_OPTIONS_GENERAL_PAGE)
			xinpattr_Hide_Input(g_inflightOptionsPageInputs[pageIndex]);
	}
	xrect_Set_Rect(&controlRect, INFLIGHT_OPTIONS_MUSIC_TOGGLE_FRAME_LEFT,
				   INFLIGHT_OPTIONS_MUSIC_TOGGLE_FRAME_TOP, INFLIGHT_OPTIONS_MUSIC_TOGGLE_FRAME_RIGHT,
				   INFLIGHT_OPTIONS_MUSIC_TOGGLE_FRAME_BOTTOM);
	musicToggle = xbtnpush_Alloc_Button(g_inflightOptionsPageInputs[INFLIGHT_OPTIONS_GENERAL_PAGE],
										&controlRect, 0, InflightOptions_HandleGeneralControl,
										g_inflightOptionsMusicLabels, INFLIGHT_OPTIONS_MUSIC_ENABLED_ID);
	xbtnpush_Set_Button_LabelIndex(musicToggle, g_flightMusicEnabled);
	musicToggle->header.var1 = g_flightMusicEnabled;
	xrect_Set_Rect(&controlRect, INFLIGHT_OPTIONS_MUSIC_VOLUME_FRAME_LEFT,
				   INFLIGHT_OPTIONS_MUSIC_VOLUME_FRAME_TOP, INFLIGHT_OPTIONS_MUSIC_VOLUME_FRAME_RIGHT,
				   INFLIGHT_OPTIONS_MUSIC_VOLUME_FRAME_BOTTOM);
	slider = xinput_Alloc_Dialog_Input(g_inflightOptionsPageInputs[INFLIGHT_OPTIONS_GENERAL_PAGE],
									   &controlRect, 0, 0);
	xinpattr_Set_Input_Update_Function(slider, InflightOptions_UpdateVolumeSlider);
	xinpattr_Set_Input_User_Function(slider, InflightOptions_HandleGeneralControl);
	xinpattr_Set_Input_Draw_Function(slider, InflightOptions_DrawVolumeSlider);
	xinpattr_Show_Input(slider);
	slider->mouseUsage = downMoveUpInput;
	slider->var1 = g_flightMusicVolume;
	slider->id = INFLIGHT_OPTIONS_MUSIC_VOLUME_ID;
	xrect_Set_Rect(&controlRect, INFLIGHT_OPTIONS_SOUND_TOGGLE_FRAME_LEFT,
				   INFLIGHT_OPTIONS_SOUND_TOGGLE_FRAME_TOP, INFLIGHT_OPTIONS_SOUND_TOGGLE_FRAME_RIGHT,
				   INFLIGHT_OPTIONS_SOUND_TOGGLE_FRAME_BOTTOM);
	soundToggle = xbtnpush_Alloc_Button(g_inflightOptionsPageInputs[INFLIGHT_OPTIONS_GENERAL_PAGE],
										&controlRect, 0, InflightOptions_HandleGeneralControl,
										g_inflightOptionsSoundLabels, INFLIGHT_OPTIONS_SOUND_ENABLED_ID);
	xbtnpush_Set_Button_LabelIndex(soundToggle, g_flightSfxEnabled);
	soundToggle->header.var1 = g_flightSfxEnabled;
	xrect_Set_Rect(&controlRect, INFLIGHT_OPTIONS_SOUND_VOLUME_FRAME_LEFT,
				   INFLIGHT_OPTIONS_SOUND_VOLUME_FRAME_TOP, INFLIGHT_OPTIONS_SOUND_VOLUME_FRAME_RIGHT,
				   INFLIGHT_OPTIONS_SOUND_VOLUME_FRAME_BOTTOM);
	slider = xinput_Alloc_Dialog_Input(g_inflightOptionsPageInputs[INFLIGHT_OPTIONS_GENERAL_PAGE],
									   &controlRect, 0, 0);
	xinpattr_Set_Input_Update_Function(slider, InflightOptions_UpdateVolumeSlider);
	xinpattr_Set_Input_User_Function(slider, InflightOptions_HandleGeneralControl);
	xinpattr_Set_Input_Draw_Function(slider, InflightOptions_DrawVolumeSlider);
	xinpattr_Show_Input(slider);
	slider->mouseUsage = downMoveUpInput;
	slider->var1 = g_flightSfxVolume;
	slider->id = INFLIGHT_OPTIONS_SOUND_VOLUME_ID;
	xrect_Set_Rect(&controlRect, INFLIGHT_OPTIONS_DIGITAL_SOUND_FRAME_LEFT,
				   INFLIGHT_OPTIONS_DIGITAL_SOUND_FRAME_TOP, INFLIGHT_OPTIONS_DIGITAL_SOUND_FRAME_RIGHT,
				   INFLIGHT_OPTIONS_DIGITAL_SOUND_FRAME_BOTTOM);
	digitalSoundToggle = xbtnpush_Alloc_Button(
		g_inflightOptionsPageInputs[INFLIGHT_OPTIONS_GENERAL_PAGE], &controlRect, 0,
		InflightOptions_HandleGeneralControl,
		g_inflightOptionsDigitalSoundLabels[g_flightDigitalSoundEnabled], INFLIGHT_OPTIONS_DIGITAL_SOUND_ID);
	digitalSoundToggle->header.var1 = (uint8_t)g_flightInterlaceEnabled;
	xinpattr_Hide_Input(&digitalSoundToggle->header);
	xrect_Set_Rect(&controlRect, INFLIGHT_OPTIONS_SPEECH_FRAME_LEFT, INFLIGHT_OPTIONS_SPEECH_FRAME_TOP,
				   INFLIGHT_OPTIONS_SPEECH_FRAME_RIGHT, INFLIGHT_OPTIONS_SPEECH_FRAME_BOTTOM);
	speechToggle = xbtnpush_Alloc_Button(g_inflightOptionsPageInputs[INFLIGHT_OPTIONS_GENERAL_PAGE],
										 &controlRect, 0, InflightOptions_HandleGeneralControl,
										 g_inflightOptionsSpeechLabels[g_flightVoiceEnabled],
										 INFLIGHT_OPTIONS_SPEECH_ID);
	speechToggle->header.var1 = g_flightVoiceEnabled;
	if (g_ReplayRecording == 0 && g_ReplayRecordedFlightAvailable == 0) {
		xrect_Set_Rect(&controlRect, INFLIGHT_OPTIONS_DISK_CACHE_FRAME_LEFT,
					   INFLIGHT_OPTIONS_DISK_CACHE_FRAME_TOP, INFLIGHT_OPTIONS_DISK_CACHE_FRAME_RIGHT,
					   INFLIGHT_OPTIONS_DISK_CACHE_FRAME_BOTTOM);
		cacheToggle = xbtnpush_Alloc_Button(g_inflightOptionsPageInputs[INFLIGHT_OPTIONS_GENERAL_PAGE],
											&controlRect, 0, InflightOptions_HandleGeneralControl,
											g_inflightOptionsDiskCacheLabels[g_flightReplayDiskCacheEnabled],
											INFLIGHT_OPTIONS_DISK_CACHE_ID);
		cacheToggle->header.var1 = g_flightReplayDiskCacheEnabled;
		xrect_Set_Rect(&controlRect, INFLIGHT_OPTIONS_INCREASE_CACHE_FRAME_LEFT,
					   INFLIGHT_OPTIONS_INCREASE_CACHE_FRAME_TOP, INFLIGHT_OPTIONS_INCREASE_CACHE_FRAME_RIGHT,
					   INFLIGHT_OPTIONS_INCREASE_CACHE_FRAME_BOTTOM);
		xbtnpush_Alloc_Button(g_inflightOptionsPageInputs[INFLIGHT_OPTIONS_GENERAL_PAGE], &controlRect, 0,
							  InflightOptions_HandleGeneralControl, "Bigger",
							  INFLIGHT_OPTIONS_INCREASE_CACHE_ID);
		xrect_Set_Rect(&controlRect, INFLIGHT_OPTIONS_DECREASE_CACHE_FRAME_LEFT,
					   INFLIGHT_OPTIONS_DECREASE_CACHE_FRAME_TOP, INFLIGHT_OPTIONS_DECREASE_CACHE_FRAME_RIGHT,
					   INFLIGHT_OPTIONS_DECREASE_CACHE_FRAME_BOTTOM);
		xbtnpush_Alloc_Button(g_inflightOptionsPageInputs[INFLIGHT_OPTIONS_GENERAL_PAGE], &controlRect, 0,
							  InflightOptions_HandleGeneralControl, "Smaller",
							  INFLIGHT_OPTIONS_DECREASE_CACHE_ID);
	}
	xrect_Set_Rect(&controlRect, INFLIGHT_OPTIONS_WEAPONS_FRAME_LEFT, INFLIGHT_OPTIONS_WEAPONS_FRAME_TOP,
				   INFLIGHT_OPTIONS_WEAPONS_FRAME_RIGHT, INFLIGHT_OPTIONS_WEAPONS_FRAME_BOTTOM);
	weaponsToggle = xbtnpush_Alloc_Button(g_inflightOptionsPageInputs[INFLIGHT_OPTIONS_GENERAL_PAGE],
										  &controlRect, 0, InflightOptions_HandleGeneralControl,
										  g_inflightOptionsWeaponsLabels[g_unlimitedWeaponsEnabled],
										  INFLIGHT_OPTIONS_UNLIMITED_WEAPONS_ID);
	weaponsToggle->header.var1 = g_unlimitedWeaponsEnabled;
	xrect_Set_Rect(&controlRect, INFLIGHT_OPTIONS_INVULNERABILITY_FRAME_LEFT,
				   INFLIGHT_OPTIONS_INVULNERABILITY_FRAME_TOP, INFLIGHT_OPTIONS_INVULNERABILITY_FRAME_RIGHT,
				   INFLIGHT_OPTIONS_INVULNERABILITY_FRAME_BOTTOM);
	invulnerabilityToggle =
		xbtnpush_Alloc_Button(g_inflightOptionsPageInputs[INFLIGHT_OPTIONS_GENERAL_PAGE], &controlRect, 0,
							  InflightOptions_HandleGeneralControl,
							  g_inflightOptionsInvulnerabilityLabels[g_flightInvulnerabilityEnabled],
							  INFLIGHT_OPTIONS_INVULNERABILITY_ID);
	invulnerabilityToggle->header.var1 = g_flightInvulnerabilityEnabled;
	xrect_Set_Rect(&controlRect, INFLIGHT_OPTIONS_COLLISIONS_FRAME_LEFT,
				   INFLIGHT_OPTIONS_COLLISIONS_FRAME_TOP, INFLIGHT_OPTIONS_COLLISIONS_FRAME_RIGHT,
				   INFLIGHT_OPTIONS_COLLISIONS_FRAME_BOTTOM);
	collisionsToggle = xbtnpush_Alloc_Button(
		g_inflightOptionsPageInputs[INFLIGHT_OPTIONS_GENERAL_PAGE], &controlRect, 0,
		InflightOptions_HandleGeneralControl,
		g_inflightOptionsCollisionsLabels[g_flightCraftCollisionsEnabled], INFLIGHT_OPTIONS_COLLISIONS_ID);
	collisionsToggle->header.var1 = g_flightCraftCollisionsEnabled;
	xrect_Set_Rect(&controlRect, INFLIGHT_OPTIONS_BACKDROPS_FRAME_LEFT, INFLIGHT_OPTIONS_BACKDROPS_FRAME_TOP,
				   INFLIGHT_OPTIONS_BACKDROPS_FRAME_RIGHT, INFLIGHT_OPTIONS_BACKDROPS_FRAME_BOTTOM);
	backdropsToggle = xbtnpush_Alloc_Button(g_inflightOptionsPageInputs[INFLIGHT_OPTIONS_GRAPHICS_PAGE],
											&controlRect, 0, InflightOptions_HandleGraphicsControl,
											g_inflightOptionsBackdropsLabels[g_flightBackdropsPreference],
											INFLIGHT_OPTIONS_BACKDROPS_ID);
	backdropsToggle->header.var1 = g_flightBackdropsPreference;
	xrect_Set_Rect(&controlRect, INFLIGHT_OPTIONS_DEBRIS_FRAME_LEFT, INFLIGHT_OPTIONS_DEBRIS_FRAME_TOP,
				   INFLIGHT_OPTIONS_DEBRIS_FRAME_RIGHT, INFLIGHT_OPTIONS_DEBRIS_FRAME_BOTTOM);
	debrisToggle = xbtnpush_Alloc_Button(g_inflightOptionsPageInputs[INFLIGHT_OPTIONS_GRAPHICS_PAGE],
										 &controlRect, 0, InflightOptions_HandleGraphicsControl,
										 g_inflightOptionsDebrisLabels[g_flightDebrisPreference],
										 INFLIGHT_OPTIONS_DEBRIS_ID);
	debrisToggle->header.var1 = g_flightDebrisPreference;
	xrect_Set_Rect(&controlRect, INFLIGHT_OPTIONS_STARFIELD_FRAME_LEFT, INFLIGHT_OPTIONS_STARFIELD_FRAME_TOP,
				   INFLIGHT_OPTIONS_STARFIELD_FRAME_RIGHT, INFLIGHT_OPTIONS_STARFIELD_FRAME_BOTTOM);
	starfieldToggle = xbtnpush_Alloc_Button(g_inflightOptionsPageInputs[INFLIGHT_OPTIONS_GRAPHICS_PAGE],
											&controlRect, 0, InflightOptions_HandleGraphicsControl,
											g_inflightOptionsStarfieldLabels[g_flightHighDetailStarfield],
											INFLIGHT_OPTIONS_STARFIELD_ID);
	starfieldToggle->header.var1 = g_flightHighDetailStarfield;
	xrect_Set_Rect(&controlRect, INFLIGHT_OPTIONS_INTERLACE_FRAME_LEFT, INFLIGHT_OPTIONS_INTERLACE_FRAME_TOP,
				   INFLIGHT_OPTIONS_INTERLACE_FRAME_RIGHT, INFLIGHT_OPTIONS_INTERLACE_FRAME_BOTTOM);
	interlaceToggle = xbtnpush_Alloc_Button(
		g_inflightOptionsPageInputs[INFLIGHT_OPTIONS_GRAPHICS_PAGE], &controlRect, 0,
		InflightOptions_HandleGraphicsControl,
		g_inflightOptionsInterlaceLabels[(uint8_t)g_flightInterlaceEnabled], INFLIGHT_OPTIONS_INTERLACE_ID);
	interlaceToggle->header.var1 = (uint8_t)g_flightInterlaceEnabled;
	xrect_Set_Rect(&controlRect, INFLIGHT_OPTIONS_ENGINE_GLOW_FRAME_LEFT,
				   INFLIGHT_OPTIONS_ENGINE_GLOW_FRAME_TOP, INFLIGHT_OPTIONS_ENGINE_GLOW_FRAME_RIGHT,
				   INFLIGHT_OPTIONS_ENGINE_GLOW_FRAME_BOTTOM);
	engineGlowToggle = xbtnpush_Alloc_Button(g_inflightOptionsPageInputs[INFLIGHT_OPTIONS_GRAPHICS_PAGE],
											 &controlRect, 0, InflightOptions_HandleGraphicsControl,
											 g_inflightOptionsEngineGlowLabels[g_flightEngineGlowPreference],
											 INFLIGHT_OPTIONS_ENGINE_GLOW_ID);
	engineGlowToggle->header.var1 = g_flightEngineGlowPreference;
	xinpattr_Hide_Input(&engineGlowToggle->header);
	xrect_Set_Rect(&controlRect, INFLIGHT_OPTIONS_STARFIGHTER_FRAME_LEFT,
				   INFLIGHT_OPTIONS_STARFIGHTER_FRAME_TOP, INFLIGHT_OPTIONS_STARFIGHTER_FRAME_RIGHT,
				   INFLIGHT_OPTIONS_STARFIGHTER_FRAME_BOTTOM);
	slider = xinput_Alloc_Dialog_Input(g_inflightOptionsPageInputs[INFLIGHT_OPTIONS_GRAPHICS_PAGE],
									   &controlRect, 0, 0);
	xinpattr_Set_Input_Update_Function(slider, InflightOptions_UpdateDetailSlider);
	xinpattr_Set_Input_User_Function(slider, InflightOptions_HandleGraphicsControl);
	xinpattr_Set_Input_Draw_Function(slider, InflightOptions_DrawDetailSlider);
	xinpattr_Show_Input(slider);
	slider->mouseUsage = downMoveUpInput;
	slider->var1 = (uint8_t)g_flightStarfighterDetail;
	slider->id = INFLIGHT_OPTIONS_STARFIGHTER_DETAIL_ID;
	xrect_Set_Rect(&controlRect, INFLIGHT_OPTIONS_STARSHIP_FRAME_LEFT, INFLIGHT_OPTIONS_STARSHIP_FRAME_TOP,
				   INFLIGHT_OPTIONS_STARSHIP_FRAME_RIGHT, INFLIGHT_OPTIONS_STARSHIP_FRAME_BOTTOM);
	slider = xinput_Alloc_Dialog_Input(g_inflightOptionsPageInputs[INFLIGHT_OPTIONS_GRAPHICS_PAGE],
									   &controlRect, 0, 0);
	xinpattr_Set_Input_Update_Function(slider, InflightOptions_UpdateDetailSlider);
	xinpattr_Set_Input_User_Function(slider, InflightOptions_HandleGraphicsControl);
	xinpattr_Set_Input_Draw_Function(slider, InflightOptions_DrawDetailSlider);
	xinpattr_Show_Input(slider);
	slider->mouseUsage = downMoveUpInput;
	slider->var1 = (uint8_t)g_flightStarshipDetail;
	slider->id = INFLIGHT_OPTIONS_STARSHIP_DETAIL_ID;
	xrect_Set_Rect(&controlRect, INFLIGHT_OPTIONS_DEATH_STAR_FRAME_LEFT,
				   INFLIGHT_OPTIONS_DEATH_STAR_FRAME_TOP, INFLIGHT_OPTIONS_DEATH_STAR_FRAME_RIGHT,
				   INFLIGHT_OPTIONS_DEATH_STAR_FRAME_BOTTOM);
	slider = xinput_Alloc_Dialog_Input(g_inflightOptionsPageInputs[INFLIGHT_OPTIONS_GRAPHICS_PAGE],
									   &controlRect, 0, 0);
	xinpattr_Set_Input_Update_Function(slider, InflightOptions_UpdateDetailSlider);
	xinpattr_Set_Input_User_Function(slider, InflightOptions_HandleGraphicsControl);
	xinpattr_Set_Input_Draw_Function(slider, InflightOptions_DrawDetailSlider);
	xinpattr_Show_Input(slider);
	slider->mouseUsage = downMoveUpInput;
	slider->var1 = (uint8_t)g_flightDeathStarDetail;
	slider->id = INFLIGHT_OPTIONS_DEATH_STAR_DETAIL_ID;
	xio_Set_Key_Buttons();
	xview_Set_View_Update_Function(InflightOptions_UpdateView);
	InflightUI_OpenMusic(resourceFile, g_inflightOptionsMusicContext);
	soundext_LoadCommonUiSounds();
	j_xviewadd_Handle_View();
	InflightUI_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xio_Clear_Key_Buttons();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	if (xcursor_Is_Cursor_Visible())
		xcursor_Hide_Cursor();
	xres_Close_Resource(resourceFile);
	InflightOptions_SavePreferences();
	if (xerror_Get_Landru_Exit() != 0)
		shellext_Sudden_Scene_Fade();
	LandruDisplay_SetLowResolutionMode(0);
	return xerror_Get_Landru_Exit();
}

#endif

// FUNCTION: XW 0x44F050
void InflightOptions_UpdateView(int time) {
	int16_t key;
	if (time == 0 && (uint16_t)xcursor_Is_Cursor_Visible() == 0)
		xcursor_Show_Cursor();
	key = xio_Get_Free_Key();
	if (key != 0) {
		if (g_inflightOptionsPage != 0) {
			if (shellext_MoveGridFocus(&g_inflightOptionsFocusIndex, g_inflightOptionsFocusPage1X,
									   g_inflightOptionsFocusPage1Y, INFLIGHT_OPTIONS_PAGE1_FOCUS_ROWS,
									   INFLIGHT_OPTIONS_FOCUS_COLUMNS, key) != 0) {
				xio_Set_Mouse_Position(g_inflightOptionsFocusPage1X[g_inflightOptionsFocusIndex],
									   g_inflightOptionsFocusPage1Y[g_inflightOptionsFocusIndex]);
				xio_Get_Key();
			}
		} else {
			if (shellext_MoveGridFocus(&g_inflightOptionsFocusIndex, g_inflightOptionsFocusPage0X,
									   g_inflightOptionsFocusPage0Y, INFLIGHT_OPTIONS_PAGE0_FOCUS_ROWS,
									   INFLIGHT_OPTIONS_FOCUS_COLUMNS, key) != 0) {
				xio_Set_Mouse_Position(g_inflightOptionsFocusPage0X[g_inflightOptionsFocusIndex],
									   g_inflightOptionsFocusPage0Y[g_inflightOptionsFocusIndex]);
				xio_Get_Key();
			}
		}
	}
}

// FUNCTION: XW 0x44F110
void InflightOptions_SavePreferences(void) {
	g_savedShellPreferences.preferences = g_shellPreferences;
	ShellPreferences_Save();
	shellext_Set_Prefs_Sound();
}

// FUNCTION: XW 0x44F130
void InflightOptions_DrawPageBackground(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	Rect sectionRect;
	(void)input;
	(void)clip;
	if (refresh != 0) {
		xstyle_Style_Paint_Base(frame, 0);
		xfont_Enable_FontID_Shadow(INFLIGHT_OPTIONS_TITLE_FONT);
		switch (g_inflightOptionsPage) {
			case INFLIGHT_OPTIONS_GENERAL_PAGE:
				xrect_Set_Rect(&sectionRect, frame->left + INFLIGHT_OPTIONS_SECTION_LEFT,
							   frame->top + INFLIGHT_OPTIONS_SECTION_TOP,
							   frame->left + INFLIGHT_OPTIONS_SECTION_RIGHT,
							   frame->top + INFLIGHT_OPTIONS_SECTION_BOTTOM);
				xpaint_Paint_Clipped_Bevel(&sectionRect, INFLIGHT_OPTIONS_UPPER_HIGHLIGHT,
										   INFLIGHT_OPTIONS_UPPER_SHADOW, INFLIGHT_OPTIONS_UPPER_FILL, 0);
				xrect_Offset_Rect(&sectionRect, 0, INFLIGHT_OPTIONS_SECTION_STEP);
				sectionRect.bottom = sectionRect.top + INFLIGHT_OPTIONS_GENERAL_MIDDLE_HEIGHT;
				xpaint_Paint_Clipped_Bevel(&sectionRect, INFLIGHT_OPTIONS_MIDDLE_HIGHLIGHT,
										   INFLIGHT_OPTIONS_MIDDLE_SHADOW, INFLIGHT_OPTIONS_MIDDLE_FILL, 0);
				xrect_Offset_Rect(&sectionRect, 0, INFLIGHT_OPTIONS_GENERAL_BOTTOM_STEP);
				sectionRect.bottom = sectionRect.top + INFLIGHT_OPTIONS_GENERAL_BOTTOM_HEIGHT;
				xpaint_Paint_Clipped_Bevel(&sectionRect, INFLIGHT_OPTIONS_LOWER_HIGHLIGHT,
										   INFLIGHT_OPTIONS_LOWER_SHADOW, INFLIGHT_OPTIONS_LOWER_FILL, 0);
				break;
			case INFLIGHT_OPTIONS_GRAPHICS_PAGE:
				xrect_Set_Rect(&sectionRect, frame->left + INFLIGHT_OPTIONS_SECTION_LEFT,
							   frame->top + INFLIGHT_OPTIONS_SECTION_TOP,
							   frame->left + INFLIGHT_OPTIONS_SECTION_RIGHT,
							   frame->top + INFLIGHT_OPTIONS_SECTION_BOTTOM);
				xpaint_Paint_Clipped_Bevel(&sectionRect, INFLIGHT_OPTIONS_UPPER_HIGHLIGHT,
										   INFLIGHT_OPTIONS_UPPER_SHADOW, INFLIGHT_OPTIONS_UPPER_FILL, 0);
				xrect_Offset_Rect(&sectionRect, 0, INFLIGHT_OPTIONS_SECTION_STEP);
				sectionRect.bottom = sectionRect.top + INFLIGHT_OPTIONS_GRAPHICS_SECTION_HEIGHT;
				xpaint_Paint_Clipped_Bevel(&sectionRect, INFLIGHT_OPTIONS_MIDDLE_HIGHLIGHT,
										   INFLIGHT_OPTIONS_MIDDLE_SHADOW, INFLIGHT_OPTIONS_MIDDLE_FILL, 0);
				xfont_Print_Clipped_Text("Starfighter Detail Level",
										 sectionRect.left + INFLIGHT_OPTIONS_STARFIGHTER_LABEL_X,
										 sectionRect.top + INFLIGHT_OPTIONS_STARFIGHTER_LABEL_Y,
										 INFLIGHT_OPTIONS_TITLE_FONT, INFLIGHT_OPTIONS_TITLE_COLOR);
				xfont_Print_Clipped_Text("Starship Detail Level",
										 sectionRect.left + INFLIGHT_OPTIONS_STARSHIP_LABEL_X,
										 sectionRect.top + INFLIGHT_OPTIONS_STARSHIP_LABEL_Y,
										 INFLIGHT_OPTIONS_TITLE_FONT, INFLIGHT_OPTIONS_TITLE_COLOR);
				xfont_Print_Clipped_Text("Death Star Detail Level",
										 sectionRect.left + INFLIGHT_OPTIONS_DEATH_STAR_LABEL_X,
										 sectionRect.top + INFLIGHT_OPTIONS_DEATH_STAR_LABEL_Y,
										 INFLIGHT_OPTIONS_TITLE_FONT, INFLIGHT_OPTIONS_TITLE_COLOR);
				break;
		}
		xfont_Disable_FontID_Shadow(INFLIGHT_OPTIONS_TITLE_FONT);
	}
}

// FUNCTION: XW 0x44F300
int16_t InflightOptions_UpdateVolumeSlider(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
										   int rightEvent, int16_t x, int16_t y) {
	(void)frame;
	(void)clip;
	(void)y;
	if (key != 0) {
		return 0;
	}
	if (leftEvent != INFLIGHT_OPTIONS_MOUSE_RELEASE && rightEvent != INFLIGHT_OPTIONS_MOUSE_RELEASE) {
		int sliderValue = x / INFLIGHT_OPTIONS_VOLUME_STEP_WIDTH;
		if (input->var1 != sliderValue) {
			input->var1 = sliderValue;
			if (input->var1 < 0) {
				input->var1 = 0;
			}
			if (input->var1 > INFLIGHT_OPTIONS_VOLUME_MAX) {
				input->var1 = INFLIGHT_OPTIONS_VOLUME_MAX;
			}
			xinpattr_Selected_Input(input);
			xinpattr_Refresh_Input(input);
		}
	}
	return 1;
}

// FUNCTION: XW 0x44F380
void InflightOptions_HandleGeneralControl(Input* input, int context) {
	(void)context;
	if (xinpattr_Get_Input_Selected(input) != 0) {
		switch (input->id) {
			case INFLIGHT_OPTIONS_MUSIC_ENABLED_ID:
				input->var1 ^= 1;
				xbtnpush_Set_Button_LabelIndex((PushButton*)input, input->var1);
				g_flightMusicEnabled = input->var1;
				g_shellPreferences.musicEnabled = input->var1;
				break;
			case INFLIGHT_OPTIONS_MUSIC_VOLUME_ID:
				g_flightMusicVolume = input->var1;
				g_shellPreferences.musicVolume = input->var1;
				shellext_Set_Prefs_Sound();
				break;
			case INFLIGHT_OPTIONS_SOUND_ENABLED_ID:
				input->var1 ^= 1;
				xbtnpush_Set_Button_LabelIndex((PushButton*)input, input->var1);
				g_flightSfxEnabled = input->var1;
				g_shellPreferences.sfxEnabled = input->var1;
				break;
			case INFLIGHT_OPTIONS_SOUND_VOLUME_ID:
				g_flightSfxVolume = input->var1;
				g_shellPreferences.sfxVolume = input->var1;
				break;
			case INFLIGHT_OPTIONS_DISK_CACHE_ID:
				input->var1 ^= 1;
				xbtnpush_Set_Button_Name((PushButton*)input, g_inflightOptionsDiskCacheLabels[input->var1]);
				g_flightReplayDiskCacheEnabled = input->var1;
				g_shellPreferences.replayDiskCacheEnabled = input->var1;
				break;
			case INFLIGHT_OPTIONS_INCREASE_CACHE_ID: {
				char cacheSizeSuffix[INFLIGHT_OPTIONS_CACHE_SUFFIX_CAPACITY];
				char cacheLabel[INFLIGHT_OPTIONS_CACHE_LABEL_CAPACITY];
				if (g_flightReplayDiskCacheKB < INFLIGHT_OPTIONS_CACHE_MAX_KB)
					g_flightReplayDiskCacheKB += INFLIGHT_OPTIONS_CACHE_STEP_KB;
				g_shellPreferences.replayDiskCacheKB = g_flightReplayDiskCacheKB;
				sprintf(cacheSizeSuffix, " (%uK)", (unsigned int)g_flightReplayDiskCacheKB);
				strcpy(cacheLabel, g_inflightOptionsCacheOffText);
				strcat(cacheLabel, cacheSizeSuffix);
				strcpy(g_inflightOptionsDiskCacheLabels[0], cacheLabel);
				strcpy(cacheLabel, g_inflightOptionsCacheOnText);
				strcat(cacheLabel, cacheSizeSuffix);
				strcpy(g_inflightOptionsDiskCacheLabels[1], cacheLabel);
				xinput_Refresh_System_Inputs();
				break;
			}
			case INFLIGHT_OPTIONS_DECREASE_CACHE_ID: {
				char cacheSizeSuffix[INFLIGHT_OPTIONS_CACHE_SUFFIX_CAPACITY];
				char cacheLabel[INFLIGHT_OPTIONS_CACHE_LABEL_CAPACITY];
				if (g_flightReplayDiskCacheKB > INFLIGHT_OPTIONS_CACHE_MIN_KB)
					g_flightReplayDiskCacheKB -= INFLIGHT_OPTIONS_CACHE_STEP_KB;
				g_shellPreferences.replayDiskCacheKB = g_flightReplayDiskCacheKB;
				sprintf(cacheSizeSuffix, " (%dK)", (int)g_flightReplayDiskCacheKB);
				strcpy(cacheLabel, g_inflightOptionsCacheOffText);
				strcat(cacheLabel, cacheSizeSuffix);
				strcpy(g_inflightOptionsDiskCacheLabels[0], cacheLabel);
				strcpy(cacheLabel, g_inflightOptionsCacheOnText);
				strcat(cacheLabel, cacheSizeSuffix);
				strcpy(g_inflightOptionsDiskCacheLabels[1], cacheLabel);
				xinput_Refresh_System_Inputs();
				break;
			}
			case INFLIGHT_OPTIONS_UNLIMITED_WEAPONS_ID:
				input->var1 ^= 1;
				xbtnpush_Set_Button_Name((PushButton*)input, g_inflightOptionsWeaponsLabels[input->var1]);
				g_unlimitedWeaponsEnabled = input->var1;
				break;
			case INFLIGHT_OPTIONS_INVULNERABILITY_ID:
				input->var1 ^= 1;
				xbtnpush_Set_Button_Name((PushButton*)input,
										 g_inflightOptionsInvulnerabilityLabels[input->var1]);
				g_flightInvulnerabilityEnabled = input->var1;
				break;
			case INFLIGHT_OPTIONS_COLLISIONS_ID:
				input->var1 ^= 1;
				xbtnpush_Set_Button_Name((PushButton*)input, g_inflightOptionsCollisionsLabels[input->var1]);
				g_flightCraftCollisionsEnabled = input->var1;
				break;
			case INFLIGHT_OPTIONS_DIGITAL_SOUND_ID:
				input->var1 ^= 1;
				xbtnpush_Set_Button_Name((PushButton*)input,
										 g_inflightOptionsDigitalSoundLabels[input->var1]);
				g_flightDigitalSoundEnabled = input->var1;
				g_shellPreferences.digitalSoundEnabled = input->var1;
				break;
			case INFLIGHT_OPTIONS_SPEECH_ID:
				input->var1 ^= 1;
				xbtnpush_Set_Button_Name((PushButton*)input, g_inflightOptionsSpeechLabels[input->var1]);
				g_flightVoiceEnabled = input->var1;
				g_shellPreferences.voiceEnabled = input->var1;
				break;
			default:
				break;
		}
	}
}

// FUNCTION: XW 0x44F800
void InflightOptions_DrawVolumeSlider(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	(void)clip;
	if (refresh != 0) {
		switch (input->id) {
			case INFLIGHT_OPTIONS_MUSIC_VOLUME_ID:
			case INFLIGHT_OPTIONS_SOUND_VOLUME_ID: {
				Rect segmentRect;
				int16_t segmentIndex;
				xstyle_Style_Paint_TextField(frame);
				xrect_Copy_Rect(&segmentRect, frame);
				xrect_Inset_Rect(&segmentRect, INFLIGHT_OPTIONS_SLIDER_INSET, INFLIGHT_OPTIONS_SLIDER_INSET);
				segmentRect.right = segmentRect.left + INFLIGHT_OPTIONS_SLIDER_SEGMENT_WIDTH;
				for (segmentIndex = 0; segmentIndex < input->var1; ++segmentIndex) {
					xpaint_Paint_Clipped_Rect(&segmentRect, INFLIGHT_OPTIONS_SLIDER_COLOR);
					xrect_Offset_Rect(&segmentRect, INFLIGHT_OPTIONS_VOLUME_STEP_WIDTH, 0);
				}
				break;
			}
		}
	}
}

// FUNCTION: XW 0x44F890
int16_t InflightOptions_UpdateDetailSlider(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
										   int rightEvent, int16_t x, int16_t y) {
	(void)frame;
	(void)clip;
	(void)y;
	if (key != 0) {
		return 0;
	}
	if (leftEvent != INFLIGHT_OPTIONS_MOUSE_RELEASE && rightEvent != INFLIGHT_OPTIONS_MOUSE_RELEASE) {
		int sliderValue = x / INFLIGHT_OPTIONS_DETAIL_STEP_WIDTH;
		if (input->var1 != sliderValue) {
			input->var1 = sliderValue;
			if (input->var1 < 0) {
				input->var1 = 0;
			}
			if (input->var1 > INFLIGHT_OPTIONS_DETAIL_MAX) {
				input->var1 = INFLIGHT_OPTIONS_DETAIL_MAX;
			}
			xinpattr_Selected_Input(input);
			xinpattr_Refresh_Input(input);
		}
	}
	return 1;
}

// FUNCTION: XW 0x44F910
void InflightOptions_HandleGraphicsControl(Input* input, int context) {
	(void)context;
	if (xinpattr_Get_Input_Selected(input) != 0) {
		switch (input->id) {
			case INFLIGHT_OPTIONS_BACKDROPS_ID:
				input->var1 ^= 1;
				xbtnpush_Set_Button_Name((PushButton*)input, g_inflightOptionsBackdropsLabels[input->var1]);
				g_flightBackdropsPreference = input->var1;
				g_shellPreferences.backdropsEnabled = input->var1;
				break;
			case INFLIGHT_OPTIONS_DEBRIS_ID:
				input->var1 ^= 1;
				xbtnpush_Set_Button_Name((PushButton*)input, g_inflightOptionsDebrisLabels[input->var1]);
				g_flightDebrisPreference = input->var1;
				g_shellPreferences.debrisEnabled = input->var1;
				break;
			case INFLIGHT_OPTIONS_STARFIELD_ID:
				input->var1 ^= 1;
				xbtnpush_Set_Button_Name((PushButton*)input, g_inflightOptionsStarfieldLabels[input->var1]);
				g_flightHighDetailStarfield = input->var1;
				g_shellPreferences.highDetailStarfield = input->var1;
				break;
			case INFLIGHT_OPTIONS_INTERLACE_ID:
				input->var1 ^= 1;
				xbtnpush_Set_Button_Name((PushButton*)input, g_inflightOptionsInterlaceLabels[input->var1]);
				g_flightInterlaceEnabled = input->var1;
				g_shellPreferences.interlaceEnabled = input->var1;
				break;
			case INFLIGHT_OPTIONS_ENGINE_GLOW_ID:
				input->var1 ^= 1;
				xbtnpush_Set_Button_Name((PushButton*)input, g_inflightOptionsEngineGlowLabels[input->var1]);
				g_flightEngineGlowPreference = input->var1;
				g_shellPreferences.engineGlowEnabled = input->var1;
				break;
			case INFLIGHT_OPTIONS_STARFIGHTER_DETAIL_ID:
				g_flightStarfighterDetail = input->var1;
				g_shellPreferences.starfighterDetail = input->var1;
				break;
			case INFLIGHT_OPTIONS_STARSHIP_DETAIL_ID:
				g_flightStarshipDetail = input->var1;
				g_shellPreferences.starshipDetail = input->var1;
				break;
			case INFLIGHT_OPTIONS_DEATH_STAR_DETAIL_ID:
				g_flightDeathStarDetail = input->var1;
				g_shellPreferences.deathStarDetail = input->var1;
				break;
		}
	}
}

// FUNCTION: XW 0x44FA80
void InflightOptions_DrawDetailSlider(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	(void)clip;
	if (refresh != 0) {
		int id = input->id;
		if (id >= INFLIGHT_OPTIONS_FIRST_DETAIL_ID && id <= INFLIGHT_OPTIONS_LAST_DETAIL_ID) {
			Rect segmentRect;
			int16_t segmentIndex;
			xstyle_Style_Paint_TextField(frame);
			xrect_Copy_Rect(&segmentRect, frame);
			xrect_Inset_Rect(&segmentRect, INFLIGHT_OPTIONS_SLIDER_INSET, INFLIGHT_OPTIONS_SLIDER_INSET);
			segmentRect.right = segmentRect.left + INFLIGHT_OPTIONS_SLIDER_SEGMENT_WIDTH;
			for (segmentIndex = 0; segmentIndex < input->var1; ++segmentIndex) {
				xpaint_Paint_Clipped_Rect(&segmentRect, INFLIGHT_OPTIONS_SLIDER_COLOR);
				xrect_Offset_Rect(&segmentRect, INFLIGHT_OPTIONS_DETAIL_STEP_WIDTH, 0);
			}
		}
	}
}

// FUNCTION: XW 0x44FB10
void InflightOptions_HandleNavigation(Input* input, int context) {
	(void)context;
	if (xinpattr_Get_Input_Selected(input) != 0) {
		switch (input->id) {
			case INFLIGHT_OPTIONS_PREVIOUS_ID:
				xinpattr_Hide_Input(g_inflightOptionsPageInputs[g_inflightOptionsPage]);
				if (g_inflightOptionsPage == 0) {
					g_inflightOptionsPage = INFLIGHT_OPTIONS_PAGE_COUNT - 1;
				} else {
					--g_inflightOptionsPage;
				}
				xinpattr_Show_Input(g_inflightOptionsPageInputs[g_inflightOptionsPage]);
				xview_Refresh_View();
				break;
			case INFLIGHT_OPTIONS_NEXT_ID:
				xinpattr_Hide_Input(g_inflightOptionsPageInputs[g_inflightOptionsPage]);
				if (g_inflightOptionsPage >= INFLIGHT_OPTIONS_PAGE_COUNT - 1) {
					g_inflightOptionsPage = 0;
				} else {
					++g_inflightOptionsPage;
				}
				xinpattr_Show_Input(g_inflightOptionsPageInputs[g_inflightOptionsPage]);
				xview_Refresh_View();
				break;
			case INFLIGHT_OPTIONS_RESUME_ID:
				xerror_Set_Landru_Exit(XW_SCENE_FLIGHT_RESUME);
				break;
			case INFLIGHT_OPTIONS_ESCAPE_ID:
				xerror_Set_Landru_Exit(xerror_Get_Landru_Escape());
				break;
		}
	}
}

// FUNCTION: XW 0x44FC00
void InflightOptions_DrawNavigation(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	if (refresh) {
		int id = input->id;
		if (id != INFLIGHT_OPTIONS_TITLE_ID) {
			if (id > INFLIGHT_OPTIONS_TITLE_ID && id <= INFLIGHT_OPTIONS_NEXT_ID) {
				PushButton* button = (PushButton*)input;
				int16_t iconId;
				xstyle_Style_Paint_Border(frame, button->pressed);
				if (input->id == INFLIGHT_OPTIONS_PREVIOUS_ID || input->id == INFLIGHT_OPTIONS_ESCAPE_ID) {
					iconId = iconLeftArrow;
				} else {
					iconId = iconRightArrow;
				}
				xstyle_Style_Draw_Centered_Icon(iconId, frame, clip, button->pressed);
			}
		} else {
			xstyle_Style_Paint_TextField(frame);
			xfont_Print_Centered_Text(g_inflightOptionsPageTitles[g_inflightOptionsPage], frame,
									  INFLIGHT_OPTIONS_TITLE_FONT, INFLIGHT_OPTIONS_TITLE_COLOR);
		}
	}
}
