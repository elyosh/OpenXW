/* Settings lifecycle and editors adapted from OpenXvT. */
#include "xw_app/settings/settings.h"
#include "xw_app/settings/audio_page.h"
#include "xw_app/settings/controller_page.h"
#include "xw_app/settings/game_page.h"
#include "xw_app/settings/installation_page.h"
#include "xw_app/settings/keyboard_page.h"
#include "xw_app/settings/mouse_page.h"
#include "xw_app/settings/video_options.h"
#include "xw_app/settings/video_page.h"
#include "xw_runtime/config/config.h"
#include "xw_runtime/config/preference_apply.h"
#include "xw_runtime/config/preferences.h"
#include "xw_runtime/input/capture.h"
#include "xw_runtime/input/input_bridge.h"
#include "xw_runtime/runtime/flight_sim.h"
#include "xw_runtime/runtime/port.h"
#include <aeron/log.h>
#include <stdio.h>
#include <string.h>

static struct {
	AeronUiContext* ui;
	XwSettings draft;
	XwControllerSettings controller;
	XwKeyboardSettings keyboard;
	bool ready, open, close_requested, exit_requested, captures;
	int page, exit_confirmation, observed_fullscreen;
	char error[1024];

	struct {
		uint32_t instance;
		bool down, armed;
	} start[AERON_CONTROLLER_MAX];
} menu;

XwSettings* XwSettingsMenu_Draft(void) { return &menu.draft; }

float XwSettingsMenu_ScrollHeight(void) {
	const AeronUiTheme* theme = AeronUi_GetTheme(menu.ui);
	/* Reserve the scroll-row gap, 4-unit tab ending, 9-unit separator, and button row. */
	float footer = 4.0f + 9.0f + theme->row_height + 3.0f * theme->item_spacing;
	if (menu.error[0])
		footer += AeronUi_MeasureHelpHeight(menu.ui, menu.error, 0.0f) + theme->row_height +
				  2.0f * theme->item_spacing;
	float height = AeronUi_AvailableHeight(menu.ui) - footer;
	return height > 0.0f ? height : 1.0f;
}

bool XwSettingsMenu_Open(void) { return menu.open; }

bool XwSettingsMenu_CapturesKeyboard(void) { return menu.open && AeronUi_KeyboardCaptureActive(menu.ui); }

void XwSettingsMenu_ReportError(const char* error) {
	snprintf(menu.error, sizeof menu.error, "%s", error);
	menu.close_requested = menu.exit_requested = false;
}

bool XwSettingsMenu_Init(XwAppUi* ui, char* error, size_t capacity) {
	memset(&menu, 0, sizeof menu);
	menu.ui = XwAppUi_Context(ui);
	menu.ready = XwInstallationPage_Init(error, capacity);
	return menu.ready;
}

void XwSettingsMenu_Show(void) {
	if (!menu.ready || menu.open)
		return;
	menu.open = true;
	menu.draft = *XwConfig_Settings();
	menu.observed_fullscreen = menu.draft.fullscreen = Aeron_Fullscreen();
	XwControllerSettings_Open(&menu.controller, &menu.draft);
	XwKeyboardSettings_Open(&menu.keyboard, &menu.draft);
	menu.error[0] = 0;
	XwPort_SetSettingsOpen(1);
	XwInput_BeginCaptureFrame(Aeron_InputSnapshot(), true);
	Aeron_SetHostCursorVisible(1);
}

void XwSettingsMenu_RequestClose(void) {
	if (menu.open)
		menu.close_requested = true;
}

static void CompleteClose(void) {
	XwKeyboardSettings_CancelCapture(&menu.keyboard, menu.ui);
	XwControllerSettings_CancelCapture(&menu.controller, menu.ui);
	XwInstallationPage_CancelPicker();
	menu.open = menu.close_requested = menu.captures = false;
	menu.exit_confirmation = 0;
	XwPort_SetSettingsOpen(0);
}

static bool Commit(char* error, size_t capacity) {
	if (XwPreferences_HasPendingSave()) {
		snprintf(error, capacity, "%s", XwPreferences_SaveError());
		return false;
	}
	if (!XwInstallationPage_Validate(error, capacity))
		return false;
	menu.draft.controller = menu.controller.draft;
	menu.draft.keyboard = menu.keyboard.draft;
	if (!XwControllerOptions_Validate(&menu.draft.controller, error, capacity))
		return false;
	AeronConfigFile* candidate = NULL;
	AeronConfigError detail = { 0 };
	if (!AeronConfigFile_Clone(XwConfig_UserDocument(), &candidate, &detail))
		return XwSettings_FileError(&detail, error, capacity) != 0;
	bool ok = XwSettings_WriteDocument(candidate, &menu.draft, XwConfig_Settings(),
									   XwConfig_DefaultSettings(), error, capacity) != 0;
	XwSettings previous = *XwConfig_Settings();
	previous.fullscreen = Aeron_Fullscreen();
	bool video_changed =
		menu.draft.fullscreen != previous.fullscreen ||
		memcmp(&menu.draft.presentation, &previous.presentation, sizeof previous.presentation) != 0;
	if (ok && video_changed)
		ok = XwVideoOptions_Apply(&previous, &menu.draft, error, capacity);
	if (ok) {
		ok = XwConfig_UpdateUser(candidate, 1, error, capacity) != 0;
		if (!ok && video_changed) {
			char rollback_error[1024];
			if (!XwVideoOptions_Apply(&menu.draft, &previous, rollback_error, sizeof rollback_error))
				Aeron_RequestFatalRendererError(rollback_error);
		}
	}
	AeronConfigFile_Destroy(candidate);
	if (!ok)
		return false;
	XwPreferences_ApplyPending();
	menu.draft = *XwConfig_Settings();
	menu.controller.original = menu.controller.draft = menu.draft.controller;
	menu.controller.dirty = false;
	menu.keyboard.original = menu.keyboard.draft = menu.draft.keyboard;
	menu.keyboard.dirty = menu.keyboard.restore_defaults = false;
	menu.error[0] = 0;
	return true;
}

void XwSettingsMenu_FlushForExit(void) {
	if (!menu.ready)
		return;
	char error[1024];
	if (menu.open && !XwPreferences_HasPendingSave() && !Commit(error, sizeof error))
		Aeron_LogError("xw.settings", "Unsaved settings: %s", error);
	XwPreferences_Flush();
}

void XwSettingsMenu_Shutdown(void) {
	if (menu.ui) {
		XwKeyboardSettings_CancelCapture(&menu.keyboard, menu.ui);
		XwControllerSettings_CancelCapture(&menu.controller, menu.ui);
	}
	XwInstallationPage_Shutdown();
	memset(&menu, 0, sizeof menu);
}

static bool ControllerStart(const AeronInputSnapshot* input) {
	bool pressed = false;
	for (int i = 0; i < AERON_CONTROLLER_MAX; ++i) {
		const AeronControllerSnapshot* d = input ? &input->controllers[i] : NULL;
		bool eligible = d && d->connected && d->kind == AERON_CONTROLLER_KIND_GAMEPAD;
		uint32_t instance = eligible ? d->instance_id : 0;
		bool down = eligible && (d->gamepad_buttons & (1u << AERON_GAMEPAD_BUTTON_START));
		if (instance != menu.start[i].instance || !input || !input->has_focus) {
			menu.start[i].instance = instance;
			menu.start[i].armed = !down;
		} else if (!down)
			menu.start[i].armed = true;
		else if (menu.start[i].armed && !menu.start[i].down)
			pressed = true;
		menu.start[i].down = down;
	}
	return pressed;
}

bool XwSettingsMenu_BeginFrame(const AeronInputSnapshot* input) {
	bool was_open = menu.open;
	int fullscreen = Aeron_Fullscreen();
	if (menu.open && fullscreen != menu.observed_fullscreen &&
		menu.draft.fullscreen == menu.observed_fullscreen)
		menu.draft.fullscreen = fullscreen;
	menu.observed_fullscreen = fullscreen;
	if (menu.open && !XwPreferences_HasPendingSave())
		XwControllerSettings_Discover(&menu.controller, menu.ui, input,
									  &XwConfig_DefaultSettings()->gamepad_defaults);
	if (menu.close_requested || menu.exit_requested) {
		char error[1024];
		if (Commit(error, sizeof error)) {
			if (menu.exit_requested)
				Aeron_RequestQuit();
			menu.exit_requested = false;
			if (menu.close_requested)
				CompleteClose();
		} else
			XwSettingsMenu_ReportError(error);
	}
	bool start = ControllerStart(input);
	if (input && input->has_focus) {
		if (menu.open && start && !menu.captures && !XwSettingsMenu_CapturesKeyboard() &&
			!XwInstallationPage_PickerOpen())
			XwSettingsMenu_RequestClose();
		else if (!menu.open && !was_open && XwInput_SettingsShortcutAllowed()) {
			int key = XwKeyboardMapping_Trigger(input, XW_KEYBOARD_SHORTCUT_SETTINGS);
			if ((key >= 0 && !XwInput_KeyBlocked(key)) || (start && !XwFlightSim_IsActive())) {
				if (key >= 0)
					XwInput_SuppressKey(key);
				menu.page = XW_SETTINGS_GAME;
				XwSettingsMenu_Show();
			}
		}
	}
	return !was_open && menu.open;
}

bool XwSettingsMenu_ConsumeRuntimeRequest(void) {
	if (!XwPort_ConsumeSettingsRequest())
		return false;
	bool was_open = menu.open;
	menu.page = XwPort_SettingsPage();
	XwSettingsMenu_Show();
	return !was_open && menu.open;
}

static void ExitConfirmation(AeronUiContext* ui) {
	if (!AeronUi_BeginModal(ui, "EXIT GAME", &menu.exit_confirmation, NULL))
		return;
	AeronUi_Help(ui, "Save settings and exit OpenXW?");
	AeronUi_BeginColumns(ui, 2, NULL);
	if (AeronUi_Button(ui, "Cancel"))
		menu.exit_confirmation = 0;
	AeronUi_NextColumn(ui);
	if (AeronUi_Button(ui, "Exit game##confirm")) {
		menu.exit_confirmation = 0;
		menu.exit_requested = true;
	}
	AeronUi_EndColumns(ui);
	AeronUi_EndModal(ui);
}

static void PendingSave(AeronUiContext* ui) {
	AeronUi_Error(ui, XwPreferences_SaveError());
	if (AeronUi_Button(ui, "Retry saving preferences")) {
		char error[1024];
		if (!XwPreferences_RetrySave(error, sizeof error))
			XwSettingsMenu_ReportError(error);
		else {
			menu.draft = *XwConfig_Settings();
			XwControllerSettings_Open(&menu.controller, &menu.draft);
			XwKeyboardSettings_Open(&menu.keyboard, &menu.draft);
			menu.error[0] = 0;
		}
	}
}

void XwSettingsMenu_Frame(const AeronInputSnapshot* input, float seconds) {
	if (!menu.open || !input)
		return;
	AeronUiContext* ui = menu.ui;
	static const char* const pages[] = { "Game", "Video", "Audio", "Controller", "Keyboard", "Mouse" };
	AeronUi_BeginFrame(ui, &(AeronUiFrameDesc) { .input = input, .dt_seconds = seconds });
	if (AeronUi_BeginWindow(ui, "OpenXW SETTINGS",
							&(AeronUiWindowDesc) { .width_ref = 980, .height_ref = 1000, .centered = 1 })) {
		if (XwPreferences_HasPendingSave())
			PendingSave(ui);
		else {
			AeronUi_BeginTabBar(ui, "pages", pages, 6, &menu.page);
			if (menu.page != XW_SETTINGS_CONTROLLER) {
				XwControllerSettings_CancelCapture(&menu.controller, ui);
				menu.controller.restore_modal_open = 0;
			}
			if (menu.page != XW_SETTINGS_KEYBOARD)
				XwKeyboardSettings_CancelCapture(&menu.keyboard, ui);
			switch (menu.page) {
				case XW_SETTINGS_GAME:
					XwGamePage_Draw(ui, input);
					break;
				case XW_SETTINGS_VIDEO:
					XwVideoPage_Draw(ui, input);
					break;
				case XW_SETTINGS_AUDIO:
					XwAudioPage_Draw(ui);
					break;
				case XW_SETTINGS_CONTROLLER:
					XwControllerSettings_Draw(&menu.controller, ui, input);
					break;
				case XW_SETTINGS_KEYBOARD:
					XwKeyboardSettings_Draw(&menu.keyboard, ui);
					break;
				case XW_SETTINGS_MOUSE:
					XwMousePage_Draw(ui, input);
					break;
			}
			AeronUi_EndTabBar(ui);
		}
		if (menu.error[0]) {
			AeronUi_Error(ui, menu.error);
			if (AeronUi_Button(ui, "Dismiss error"))
				menu.error[0] = 0;
		}
		AeronUi_Separator(ui);
		if (menu.page == XW_SETTINGS_GAME) {
			AeronUi_BeginColumns(ui, 2, NULL);
			if (AeronUi_Button(ui, "Exit game"))
				menu.exit_confirmation = 1;
			AeronUi_NextColumn(ui);
		}
		if (AeronUi_Button(ui, "Close"))
			XwSettingsMenu_RequestClose();
		if (menu.page == XW_SETTINGS_GAME)
			AeronUi_EndColumns(ui);
		if (menu.page == XW_SETTINGS_CONTROLLER)
			XwControllerSettings_DrawModals(&menu.controller, ui, input, XwConfig_Settings());
		if (menu.page == XW_SETTINGS_KEYBOARD)
			XwKeyboardSettings_DrawModals(&menu.keyboard, ui);
		if (menu.exit_confirmation)
			ExitConfirmation(ui);
		AeronUi_EndWindow(ui);
	}
	XwInstallationPage_DrawPicker(ui);
	AeronUiOutput output = AeronUi_EndFrame(ui);
	menu.captures = output.capture_all != 0;
	if (output.cancel_pressed)
		XwSettingsMenu_RequestClose();
	if (!AeronUi_Submit(ui))
		Aeron_RequestFatalRendererError("Settings submission");
}
