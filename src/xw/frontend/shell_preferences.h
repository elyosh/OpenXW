#ifndef XW_FRONTEND_SHELL_PREFERENCES_H
#define XW_FRONTEND_SHELL_PREFERENCES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

typedef struct XwShellPreferences XwShellPreferences;

enum { XW_SHELL_VOLUME_STEP = 8 };

enum {
	XW_SHELL_RESOLUTION_320_200 = 0,
	XW_SHELL_RESOLUTION_640_480_INDEXED = 1,
	XW_SHELL_RESOLUTION_640_480_RGB = 2,
	XW_SHELL_RESOLUTION_HARDWARE = 3
};

enum { XW_FLIGHT_INITIAL_VOLUME = 16, XW_FLIGHT_INITIAL_BRIGHTNESS = 4 };

enum {
	XW_SHELL_DEFAULT_VOLUME = 8,
	XW_SHELL_DEFAULT_REPLAY_CACHE_KB = 256,
	XW_SHELL_DEFAULT_RESOLUTION = 2,
	XW_SHELL_DEFAULT_TEXTURE_QUALITY = 1,
	XW_SHELL_DEFAULT_BRIGHTNESS = 2
};

/* Original IDB size: 64 bytes. */
struct XwShellPreferences {
	/* IDB +0x0: Enable flag; gates shell music volume and mirrors g_flightMusicEnabled. */
	int16_t musicEnabled;
	/* IDB +0x2: Enable flag; gates shell SFX/voice volume and mirrors g_flightSfxEnabled. Saved copy is
	 * queried by sub_47E730. */
	int16_t sfxEnabled;
	/* IDB +0x4: Slider 0..16; converted to 8*value-1 when enabled. Default 8. */
	int16_t musicVolume;
	/* IDB +0x6: Slider 0..16; shared by SFX and voice in SHELLEXT_Set_Prefs_Sound. Default 8. */
	int16_t sfxVolume;
	/* IDB +0x8: Options button 8: 0=Spoken Text Hidden, 1=Spoken Text Shown. Timed speech text is forced
	 * visible when sound is unavailable. */
	int16_t spokenTextEnabled;
	/* IDB +0xA: Options button 9: 0=Transitions Skipped, 1=Transitions Shown. Zero enables
	 * SHELLEXT_Convert_Transition redirects; mirrored into flight transitions flag. */
	int16_t transitionsEnabled;
	/* IDB +0xC: Digital sound option; copied to the low byte at 0x4D433C. UI label verified in sub_44F380. */
	uint8_t digitalSoundEnabled;
	/* IDB +0xD: Speech enable option; mirrored to g_flightVoiceEnabled. */
	uint8_t voiceEnabled;
	/* IDB +0xE: Film disk-cache size in KiB; options change it in steps of 32 and display it with K suffix.
	 * Default 256. */
	uint16_t replayDiskCacheKB;
	/* IDB +0x10: Film disk-cache on/off option; copied to low byte at 0x4D4348. */
	uint8_t replayDiskCacheEnabled;
	/* IDB +0x11: Select ;classic\ rather than ;mission\ in SHIPEXT_Get_Mission_Path. Toggled by options
	 * button 10; saved as byte 0x11 of prefs.cfg. */
	uint8_t classicMissions;
	/* IDB +0x12: UI high-detail starfield flag; runtime uses density step 1 when set, otherwise 2. */
	uint8_t highDetailStarfield;
	/* IDB +0x13: Planets and galaxies toggle; surface missions override it off. */
	uint8_t backdropsEnabled;
	/* IDB +0x14: Space debris toggle; surface missions override it off. */
	uint8_t debrisEnabled;
	/* IDB +0x15: DOS polygon-markings preference; nonzero enables markings. */
	uint8_t markingsEnabled;
	/* IDB +0x16: Engine-glow toggle; the in-flight options control is created hidden. */
	uint8_t engineGlowEnabled;
	/* IDB +0x17: Starfighter detail slider 0..12. */
	uint8_t starfighterDetail;
	/* IDB +0x18: Starship detail slider 0..12; also controls explosion spawn threshold. */
	uint8_t starshipDetail;
	/* IDB +0x19: Death Star detail slider 0..12. */
	uint8_t deathStarDetail;
	/* IDB +0x1A: Interlace toggle. */
	uint8_t interlaceEnabled;
	/* IDB +0x1B: 0,1,2,3 map to flight modes 0x13,0x101,0x111,0x1FF respectively. */
	uint8_t resolutionIndex;
	/* IDB +0x1C: Engine sound option; copied to g_flightEngineSoundEnabled. Default 1. */
	uint8_t engineSoundEnabled;
	/* IDB +0x1D: Texture quality: 0 low, 1 medium, 2 high. Button 101 cycles these labels and mirrors
	 * g_modelTextureQuality. */
	uint8_t textureQuality;
	/* IDB +0x1E: Options brightness slider 0..8; low byte mirrors g_flightBrightnessSetting. Flight scale
	 * calculation clamps that byte to 7. */
	int16_t brightness;
	/* IDB +0x20: 32 byte action-code bindings indexed by joystick button. Copied to 0x5B8AA0 for flight use;
	 * options select codes from g_joystickEntries. */
	uint8_t joystickActions[32];
};

typedef char xw_size_XwShellPreferences[(sizeof(XwShellPreferences) == 64) ? 1 : -1];
typedef char
	xw_offset_XwShellPreferences_classicMissions[(offsetof(XwShellPreferences, classicMissions) == 17) ? 1
																									   : -1];
typedef char
	xw_offset_XwShellPreferences_joystickActions[(offsetof(XwShellPreferences, joystickActions) == 32) ? 1
																									   : -1];

/* Complete prefs.cfg disk record; runtime UI copies only the preferences prefix. */
#ifdef XW_MODERN
#pragma pack(push, 1)
#endif
typedef struct XwSavedShellPreferences {
	XwShellPreferences preferences;
	uint32_t field_40;
	uint32_t field_44;
	uint32_t field_48;
	uint32_t field_4C;
	int32_t introPlaybackMode;
	uint8_t gap54[8];
	uint32_t field_5C;
} XwSavedShellPreferences;
#ifdef XW_MODERN
#pragma pack(pop)
#endif
typedef char xw_size_XwSavedShellPreferences[(sizeof(XwSavedShellPreferences) == 96) ? 1 : -1];
typedef char
	xw_offset_XwSavedShellPreferences_field_40[(offsetof(XwSavedShellPreferences, field_40) == 64) ? 1 : -1];
typedef char xw_offset_XwSavedShellPreferences_introPlaybackMode
	[(offsetof(XwSavedShellPreferences, introPlaybackMode) == 80) ? 1 : -1];
typedef char
	xw_offset_XwSavedShellPreferences_field_5C[(offsetof(XwSavedShellPreferences, field_5C) == 92) ? 1 : -1];

extern int g_modelTextureQuality;
extern uint8_t g_flightBrightnessSetting;
extern int g_textureMipmapsEnabled;
extern uint8_t g_flightEngineSoundEnabled;
extern uint8_t g_flightMusicVolume;
extern uint8_t g_flightMusicEnabled;
extern uint8_t g_flightSfxVolume;
extern uint8_t g_flightDigitalSoundEnabled;
extern uint16_t g_flightReplayDiskCacheKB;
extern uint8_t g_flightReplayDiskCacheEnabled;
extern uint8_t g_flightCraftCollisionsEnabled;
extern uint8_t g_flightTransitionsEnabled;
extern uint8_t g_classicMissionsMirror;
extern XwShellPreferences g_shellPreferences;
extern XwSavedShellPreferences g_savedShellPreferences;

/* Declarations follow ascending original IDB address. */

/* 0x460FB0 */
int16_t j_ShellPreferences_GetMusicEnabled(void);

/* 0x47E720 */
int16_t ShellPreferences_GetMusicEnabled(void);

/* 0x47E730 */
int16_t ShellPreferences_GetSfxEnabled(void);

/* 0x49E5E0 */
void ShellPreferences_Load(void);

/* 0x49E650 */
void ShellPreferences_Save(void);

/* 0x49E690 */
void ShellPreferences_ApplySavedAndSave(void);

/* 0x49E820 */
void ShellPreferences_InitDefaults(void);

#ifdef __cplusplus
}
#endif

#endif
