#include "xw_runtime/runtime/options_task.h"
#include "xw_runtime/runtime/port.h"

#include "xw/audio/hilevel.h"
#include "xw/audio/lolevel.h"
#include "xw/flight/flight.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/input/joystick.h"
#include "xw/util/landru_display.h"
#include <landru/error.h>
#include <landru/io.h>
#include <landru/task.h>
#include <stdlib.h>
#include <string.h>

static struct {
	Input* dialog;
	Palette* savedPalette;
	uint8_t savedMasterVolume;
	int16_t savedKeyButtons;
	int savedDoublePixels;
	int16_t accepted;
	int settings_pending;
	DialogSubResultHandler complete;
	void* context;
} options_state;

static int16_t previous_exit;

void XwOptions_SavePreviousExit(int16_t previousExit) { previous_exit = previousExit; }

void XwOptions_SaveAndFinish(int16_t accepted, void* context) {
	int16_t exitCode = previous_exit;
	(void)context;
	if (accepted == 0) {
		exitCode = xerror_Get_Landru_Escape();
	} else {
		g_savedShellPreferences.preferences = g_shellPreferences;
		ShellPreferences_Save();
	}
	xerror_Set_Landru_Exit(exitCode);
}

void XwOptions_AfterCalibration(int16_t result, void* context) {
	(void)result;
	(void)context;
	shellext_ShowOptionsDialog(XwOptions_SaveAndFinish, NULL);
}

static void options_cleanup(Input* dialog) {
	int16_t result = xdialog_Get_Dialog_Exit();
	memcpy(g_flightJoystickActions, g_shellPreferences.joystickActions, sizeof(g_flightJoystickActions));
	xdialog_Clear_Dialog_Exit();
	xinput_Free_Inputs(dialog);
	if (g_quitRequested) {
		free(g_frontendSavedBackground);
		g_frontendSavedBackground = NULL;
		options_state.complete = NULL;
		options_state.context = NULL;
	} else {
		LandruDisplay_RestoreCanvasBackground();
		shellext_Set_Prefs_Sound();
	}
	hilevel_ImSetMasterVol(options_state.savedMasterVolume);
	lolevel_ImResume();
	if (!g_quitRequested)
		xpal_Set_Screen_Palette(options_state.savedPalette);
	xpal_Free_Palette(options_state.savedPalette);
	if (options_state.savedKeyButtons == 0)
		xio_Clear_Key_Buttons();
	if (!g_quitRequested)
		LandruDisplay_SetLowResolutionMode(options_state.savedDoublePixels);
	if (g_quitRequested != 0)
		result = XW_OPTIONS_EXIT;
	options_state.accepted = result != XW_OPTIONS_EXIT;
	options_state.dialog = NULL;
}

static void options_complete(int16_t unusedResult, void* unusedContext) {
	DialogSubResultHandler complete = options_state.complete;
	void* context = options_state.context;
	int16_t accepted = options_state.accepted;
	(void)unusedResult;
	(void)unusedContext;
	options_state.complete = NULL;
	options_state.context = NULL;
	if (complete != NULL)
		complete(accepted, context);
}

void XwOptions_ScheduleDialog(Input* dialog, Palette* savedPalette, uint8_t savedMasterVolume,
							  int16_t savedKeyButtons, int savedDoublePixels, DialogSubResultHandler complete,
							  void* context) {
	if (options_state.dialog != NULL)
		abort();
	options_state.dialog = dialog;
	options_state.savedPalette = savedPalette;
	options_state.savedMasterVolume = savedMasterVolume;
	options_state.savedKeyButtons = savedKeyButtons;
	options_state.savedDoublePixels = savedDoublePixels;
	options_state.complete = complete;
	options_state.context = context;
	if (landru_task_depth() >= LANDRU_TASK_STACK_DEPTH) {
		options_cleanup(dialog);
		abort();
	}
	if (!xdialog_Schedule_Owned_Sub_Dialog(dialog, options_cleanup, options_complete, NULL))
		abort();
}

void XwOptions_RequestSettings(DialogSubResultHandler complete, void* context) {
	options_state.settings_pending = 1;
	options_state.complete = complete;
	options_state.context = context;
	XwPort_RequestSettings();
}

void XwOptions_SettingsClosed(void) {
	if (!options_state.settings_pending)
		return;
	options_state.settings_pending = 0;
	options_state.accepted = 1;
	options_complete(0, NULL);
}

void XwOptions_CancelSettings(void) {
	if (options_state.settings_pending) {
		options_state.settings_pending = 0;
		options_state.complete = NULL;
		options_state.context = NULL;
	}
}
