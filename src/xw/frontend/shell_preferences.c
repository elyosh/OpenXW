#include "xw/frontend/shell_preferences.h"

#include "xw/audio/fsfx.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/player/user.h"
#include "xw/frontend/inflight_options.h"
#include "xw/frontend/shellext.h"
#include "xw/input/joystick.h"
#include "xw/landru_config.h"
#include "xw/render/rtsvga2.h"

#ifdef XW_MODERN
#include "xw_runtime/config/config.h"
#include "xw_runtime/config/preferences.h"
#endif
#include <landru/file.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4CEF5C
int g_modelTextureQuality = XW_SHELL_DEFAULT_TEXTURE_QUALITY;

// GLOBAL: XW 0x4CEF60
uint8_t g_flightBrightnessSetting = XW_FLIGHT_INITIAL_BRIGHTNESS;

// GLOBAL: XW 0x4CEF68
int g_textureMipmapsEnabled = 1;

// GLOBAL: XW 0x4D4328
uint8_t g_flightEngineSoundEnabled = 1;

// GLOBAL: XW 0x4D432C
uint8_t g_flightMusicVolume = XW_FLIGHT_INITIAL_VOLUME;

// GLOBAL: XW 0x4D4330
uint8_t g_flightMusicEnabled = 1;

// GLOBAL: XW 0x4D4334
uint8_t g_flightSfxVolume = XW_FLIGHT_INITIAL_VOLUME;

// GLOBAL: XW 0x4D433C
uint8_t g_flightDigitalSoundEnabled = 1;

// GLOBAL: XW 0x4D4344
uint16_t g_flightReplayDiskCacheKB = XW_SHELL_DEFAULT_REPLAY_CACHE_KB;

// GLOBAL: XW 0x4D4348
uint8_t g_flightReplayDiskCacheEnabled = 1;

// GLOBAL: XW 0x4D434C
uint8_t g_flightCraftCollisionsEnabled = 1;

// GLOBAL: XW 0x5B8AFA
uint8_t g_flightTransitionsEnabled = 0;

// GLOBAL: XW 0x5B8AFB
uint8_t g_classicMissionsMirror = 0;

// GLOBAL: XW 0x5B8B00
XwShellPreferences g_shellPreferences = { 0 };

// GLOBAL: XW 0x5E72A0
XwSavedShellPreferences g_savedShellPreferences = { 0 };

// FUNCTION: XW 0x460FB0
int16_t j_ShellPreferences_GetMusicEnabled(void) { return ShellPreferences_GetMusicEnabled(); }

// FUNCTION: XW 0x47E720
int16_t ShellPreferences_GetMusicEnabled(void) { return g_savedShellPreferences.preferences.musicEnabled; }

// FUNCTION: XW 0x47E730
int16_t ShellPreferences_GetSfxEnabled(void) { return g_savedShellPreferences.preferences.sfxEnabled; }

// FUNCTION: XW 0x49E5E0
void ShellPreferences_Load(void) {
#ifdef XW_MODERN
	char error[1024];
	if (!XwConfig_Apply(&g_savedShellPreferences, error, sizeof error))
		XwStorage_Fatal(error, 1);
	g_shellPreferences = g_savedShellPreferences.preferences;
#else
	LandruFile* file = xfile_Open_File(LANDRU_FILE_ROOT_USER, "prefs.cfg", "rb");
	if (file != NULL) {
		xfile_Read_Data_From_File(file, &g_savedShellPreferences, sizeof(g_savedShellPreferences));
		xfile_Close_File(file);
	} else {
		ShellPreferences_InitDefaults();
		g_savedShellPreferences.introPlaybackMode = 0;
		ShellPreferences_ApplySavedAndSave();
	}
	g_shellPreferences = g_savedShellPreferences.preferences;
#endif
}

// FUNCTION: XW 0x49E650
void ShellPreferences_Save(void) {
#ifdef XW_MODERN
	XwPreferences_Save(&g_savedShellPreferences);
#else
	LandruFile* file = xfile_Open_File(LANDRU_FILE_ROOT_USER, "prefs.cfg", "wb");
	if (file != NULL) {
		xfile_Write_Data_To_File(file, &g_savedShellPreferences, sizeof(g_savedShellPreferences));
		xfile_Close_File(file);
	}
#endif
}

// FUNCTION: XW 0x49E690
void ShellPreferences_ApplySavedAndSave(void) {
	g_shellPreferences = g_savedShellPreferences.preferences;
	g_flightTransitionsEnabled = g_savedShellPreferences.preferences.transitionsEnabled;
	g_classicMissionsMirror = g_savedShellPreferences.preferences.classicMissions;
	g_flightMusicEnabled = g_savedShellPreferences.preferences.musicEnabled;
	g_flightSfxEnabled = g_savedShellPreferences.preferences.sfxEnabled;
	g_flightMusicVolume = g_savedShellPreferences.preferences.musicVolume;
	g_flightSfxVolume = g_savedShellPreferences.preferences.sfxVolume;
	g_flightDigitalSoundEnabled = g_savedShellPreferences.preferences.digitalSoundEnabled;
	g_flightVoiceEnabled = g_savedShellPreferences.preferences.voiceEnabled;
	g_flightReplayDiskCacheKB = g_savedShellPreferences.preferences.replayDiskCacheKB;
	g_flightReplayDiskCacheEnabled = g_savedShellPreferences.preferences.replayDiskCacheEnabled;
	g_flightHighDetailStarfield = g_savedShellPreferences.preferences.highDetailStarfield;
	g_flightBackdropsPreference = g_savedShellPreferences.preferences.backdropsEnabled;
	g_flightDebrisPreference = g_savedShellPreferences.preferences.debrisEnabled;
	g_flightMarkingsPreference = g_savedShellPreferences.preferences.markingsEnabled;
	g_flightEngineGlowPreference = g_savedShellPreferences.preferences.engineGlowEnabled;
	g_flightStarfighterDetail = g_savedShellPreferences.preferences.starfighterDetail;
	g_flightStarshipDetail = g_savedShellPreferences.preferences.starshipDetail;
	g_flightDeathStarDetail = g_savedShellPreferences.preferences.deathStarDetail;
	g_flightInterlaceEnabled = g_savedShellPreferences.preferences.interlaceEnabled;
	g_flightEngineSoundEnabled = g_savedShellPreferences.preferences.engineSoundEnabled;
	switch (g_savedShellPreferences.preferences.resolutionIndex) {
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
	memcpy(g_flightJoystickActions, g_savedShellPreferences.preferences.joystickActions,
		   sizeof(g_flightJoystickActions));
	g_modelTextureQuality = g_savedShellPreferences.preferences.textureQuality;
	g_flightBrightnessSetting = g_savedShellPreferences.preferences.brightness;
	shellext_Set_Prefs_Sound();
	user_ApplyPreferences();
	ShellPreferences_Save();
}

// FUNCTION: XW 0x49E820
void ShellPreferences_InitDefaults(void) {
#ifdef XW_MODERN
	const XwSettings* defaults = XwConfig_DefaultSettings();
	if (defaults)
		g_savedShellPreferences = defaults->game;
	else
		XwPreferences_Defaults(&g_savedShellPreferences);
	memcpy(g_flightJoystickActions, g_savedShellPreferences.preferences.joystickActions,
		   sizeof(g_flightJoystickActions));
#else
	int buttonCount;
	int buttonIndex;
	g_savedShellPreferences.preferences.musicEnabled = 1;
	g_savedShellPreferences.preferences.sfxEnabled = 1;
	g_savedShellPreferences.preferences.musicVolume = XW_SHELL_DEFAULT_VOLUME;
	g_savedShellPreferences.preferences.sfxVolume = XW_SHELL_DEFAULT_VOLUME;
	g_savedShellPreferences.preferences.spokenTextEnabled = 1;
	g_savedShellPreferences.preferences.transitionsEnabled = 1;
	g_savedShellPreferences.preferences.classicMissions = 0;
	g_savedShellPreferences.preferences.digitalSoundEnabled = 1;
	g_savedShellPreferences.preferences.voiceEnabled = 1;
	g_savedShellPreferences.preferences.replayDiskCacheKB = XW_SHELL_DEFAULT_REPLAY_CACHE_KB;
	g_savedShellPreferences.preferences.replayDiskCacheEnabled = 1;
	g_savedShellPreferences.preferences.highDetailStarfield = 1;
	g_savedShellPreferences.preferences.backdropsEnabled = 1;
	g_savedShellPreferences.preferences.debrisEnabled = 1;
	g_savedShellPreferences.preferences.markingsEnabled = 1;
	g_savedShellPreferences.preferences.engineGlowEnabled = 1;
	g_savedShellPreferences.preferences.starfighterDetail = INFLIGHT_OPTIONS_DETAIL_MAX;
	g_savedShellPreferences.preferences.starshipDetail = INFLIGHT_OPTIONS_DETAIL_MAX;
	g_savedShellPreferences.preferences.deathStarDetail = INFLIGHT_OPTIONS_DETAIL_MAX;
	g_savedShellPreferences.field_4C = 0;
	g_savedShellPreferences.preferences.interlaceEnabled = 0;
	g_savedShellPreferences.preferences.resolutionIndex = XW_SHELL_DEFAULT_RESOLUTION;
	g_savedShellPreferences.preferences.engineSoundEnabled = 1;
	buttonCount = (int)Joystick_GetButtonCount();
	for (buttonIndex = 0; buttonIndex < buttonCount; ++buttonIndex) {
		if (buttonIndex >= JOYSTICK_DEFAULT_ASSIGNMENT_LIMIT)
			break;
		switch (buttonIndex) {
			case 0:
				g_savedShellPreferences.preferences.joystickActions[0] = JOYSTICK_ACTION_FIRE;
				break;
			case 1:
				g_savedShellPreferences.preferences.joystickActions[1] = JOYSTICK_ACTION_ROLL_TARGET;
				break;
			case 2:
				g_savedShellPreferences.preferences.joystickActions[2] = JOYSTICK_ACTION_NEAREST_FIGHTER;
				break;
			case 3:
				g_savedShellPreferences.preferences.joystickActions[3] = JOYSTICK_ACTION_TOGGLE_COCKPIT;
				break;
			case 4:
				g_savedShellPreferences.preferences.joystickActions[4] = JOYSTICK_ACTION_NEAREST_ATTACKER;
				break;
			case 5:
				g_savedShellPreferences.preferences.joystickActions[5] = JOYSTICK_ACTION_IDENTIFY;
				break;
			case 6:
				g_savedShellPreferences.preferences.joystickActions[6] = JOYSTICK_ACTION_THIRD_THROTTLE;
				break;
			case 7:
				g_savedShellPreferences.preferences.joystickActions[7] = JOYSTICK_ACTION_FULL_THROTTLE;
				break;
			case 8:
				g_savedShellPreferences.preferences.joystickActions[8] = JOYSTICK_ACTION_MATCH_SPEED;
				break;
			case 9:
				g_savedShellPreferences.preferences.joystickActions[9] = JOYSTICK_ACTION_TWO_THIRDS_THROTTLE;
				break;
		}
	}
	memcpy(g_flightJoystickActions, g_savedShellPreferences.preferences.joystickActions,
		   sizeof(g_flightJoystickActions));
	g_savedShellPreferences.preferences.textureQuality = XW_SHELL_DEFAULT_TEXTURE_QUALITY;
	g_savedShellPreferences.field_40 = 1;
	g_savedShellPreferences.field_44 = 1;
	g_savedShellPreferences.preferences.brightness = XW_SHELL_DEFAULT_BRIGHTNESS;
	g_savedShellPreferences.field_48 = 0;
	g_savedShellPreferences.field_5C = 0;
#endif
}
