#ifndef XW_RUNTIME_CONFIG_SETTINGS_H
#define XW_RUNTIME_CONFIG_SETTINGS_H
#include "xw/frontend/shell_preferences.h"
#include "xw_runtime/config/video_config.h"
#include "xw_runtime/input/controller_options.h"
#include "xw_runtime/input/keyboard_mapping.h"
#include "xw_runtime/input/mouse_flight.h"
#include "xw_runtime/runtime/profile.h"
#include <aeron/config_file.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XwPresentationSettings {
	int vsync_divisor, hdr_output;
	/* Negative gamma uses the platform default; zero is sRGB. Zero white is automatic. */
	float sdr_gamma, paper_white_nits;
} XwPresentationSettings;

typedef struct XwMusicSettings {
	char arrangement[16], backend[16];
	char soundfont[XW_PATH_CAPACITY];
	char sc55_rom_directory[XW_PATH_CAPACITY];
	char mt32_control[XW_PATH_CAPACITY], mt32_pcm[XW_PATH_CAPACITY];
} XwMusicSettings;

typedef struct XwSettings {
	XwMusicSettings music;
	char frontend_version[8];
	char inflight_frontend_version[9];
	char flight_version[8];
	XwFlightUpdateRate flight_update_rate;
	int skip_intro, fullscreen;
	int sb16_filter_enabled;
	int player_engine_sound_volume_percent;
	int starfighter_collision_damage, player_invulnerable, unlimited_ammunition;
	int laser_convergence;
	char xw93_data[XW_PATH_CAPACITY], xw94_data[XW_PATH_CAPACITY], xw98_data[XW_PATH_CAPACITY];
	char ui_font[XW_PATH_CAPACITY];
	int mouse_flight, mouse_sensitivity, mouse_invert_y;
	XwMouseFlightMode mouse_mode;
	XwControllerOptions controller;
	XwControllerProfile gamepad_defaults;
	XwKeyboardBindings keyboard;
	XwPresentationSettings presentation;
	XwRenderSettings render;
	XwSavedShellPreferences game;
} XwSettings;

int XwSettings_WriteDocument(AeronConfigFile* document, const XwSettings* value, const XwSettings* previous,
							 const XwSettings* defaults, char* error, size_t capacity);
int XwSettings_Parse(const AeronConfigFile* document, const XwSceneSettings* scene_defaults, XwSettings* out,
					 char* error, size_t capacity);
int XwSettings_NodeError(const AeronConfigFile* document, const char* path, const char* message, char* error,
						 size_t capacity);
int XwSettings_FileError(const AeronConfigError* detail, char* error, size_t capacity);
#ifdef __cplusplus
}
#endif
#endif
