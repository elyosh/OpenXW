/* Installation picker adapted from OpenXvT. */
#include "xw_app/settings/installation_page.h"
#include "xw_app/installation_ui.h"
#include "xw_app/settings/settings.h"
#include "xw_app/setup.h"
#include "xw_runtime/audio/midi_resources.h"
#include "xw_runtime/config/config.h"
#include <imuse/midi_nuked_sc55.h>
#include <stdio.h>
#include <string.h>

static AeronUiFilePicker* picker;

static enum { INSTALLATION, SC55_ROMS, MT32_CONTROL, MT32_PCM } purpose;

static XwGameVersion picker_version;

bool XwInstallationPage_Init(char* error, size_t capacity) {
	picker = AeronUiFilePicker_Create();
	if (!picker)
		snprintf(error, capacity, "Could not open the game folder chooser.");
	return picker != NULL;
}

void XwInstallationPage_Shutdown(void) {
	AeronUiFilePicker_Destroy(picker);
	picker = NULL;
}

void XwInstallationPage_CancelPicker(void) { AeronUiFilePicker_Cancel(picker); }

bool XwInstallationPage_PickerOpen(void) { return AeronUiFilePicker_IsOpen(picker); }

static int AcceptSc55(const char* path, void* user, char* error, size_t capacity) {
	(void)user;
	return XwMidiResources_Sc55RomDirectoryValidate(path, error, capacity);
}

static int AcceptMt32Control(const char* path, void* user, char* error, size_t capacity) {
	(void)user;
	return XwMidiResources_Mt32RomValidate(path, true, error, capacity);
}

static int AcceptMt32Pcm(const char* path, void* user, char* error, size_t capacity) {
	(void)user;
	return XwMidiResources_Mt32RomValidate(path, false, error, capacity);
}

void XwInstallationPage_OpenSc55Picker(const char* path) {
	const AeronUiFilePickerDesc desc = {
		.mode = AERON_UI_FILE_PICKER_SELECT_DIRECTORY,
		.title = "SELECT SC-55 ROM DIRECTORY",
		.instructions = "Select the folder containing the original SC-55 ROM dumps.",
		.accept_label = "Use This Folder",
		.cancel_label = "Cancel",
		.initial_path = path && path[0] ? path : NULL,
		.accept_fn = AcceptSc55,
	};
	char error[1024];
	if (!AeronUiFilePicker_Open(picker, &desc, error, sizeof error))
		XwSettingsMenu_ReportError(error);
	else
		purpose = SC55_ROMS;
}

void XwInstallationPage_OpenMt32Picker(bool control, const char* path) {
	char parent[XW_PATH_CAPACITY];
	const AeronUiFilePickerDesc desc = {
		.mode = AERON_UI_FILE_PICKER_OPEN_FILE,
		.title = control ? "SELECT MT-32 CONTROL ROM" : "SELECT MT-32 PCM ROM",
		.instructions = "Select a complete MT-32 compatible ROM image.",
		.accept_label = "Use This File",
		.cancel_label = "Cancel",
		.initial_path = XwMidiResources_ParentDirectory(path, parent, sizeof parent),
		.accept_fn = control ? AcceptMt32Control : AcceptMt32Pcm,
	};
	char error[1024];
	if (!AeronUiFilePicker_Open(picker, &desc, error, sizeof error))
		XwSettingsMenu_ReportError(error);
	else
		purpose = control ? MT32_CONTROL : MT32_PCM;
}

void XwInstallationPage_Draw(AeronUiContext* ui, const AeronInputSnapshot* input) {
	(void)input;
	XwSettings* draft = XwSettingsMenu_Draft();
	AeronUi_Header(ui, "Original Installations");
	char* paths[] = { draft->xw93_data, draft->xw94_data, draft->xw98_data };
	const XwGameVersion versions[] = { XW_GAME_VERSION_93, XW_GAME_VERSION_94, XW_GAME_VERSION_98 };
	const XwSettings* saved = XwConfig_Settings();
	bool restart_needed = strcmp(draft->xw93_data, saved->xw93_data) ||
						  strcmp(draft->xw94_data, saved->xw94_data) ||
						  strcmp(draft->xw98_data, saved->xw98_data);
	for (int i = 0; i < 3; ++i) {
		XwGameVersion version = versions[i];
		uint32_t result = XwInstallation_PathRow(ui, version, paths[i], XW_PATH_CAPACITY, 0);
		if (result & AERON_UI_INPUT_TEXT_ACTION_ACTIVATED) {
			char error[1024];
			purpose = INSTALLATION;
			picker_version = version;
			if (!XwInstallation_OpenPicker(picker, &picker_version, paths[i], error, sizeof error))
				XwSettingsMenu_ReportError(error);
		}
		if (*paths[i] && strcmp(paths[i], XwSetup_InstallationFor(version)))
			restart_needed = true;
	}
	if (restart_needed)
		AeronUi_Help(ui, "Restart OpenXW to use the selected game folders.");
}

void XwInstallationPage_DrawPicker(AeronUiContext* ui) {
	char selected[XW_PATH_CAPACITY], error[1024];
	AeronUiFilePickerResult result =
		AeronUiFilePicker_Draw(picker, ui, selected, sizeof selected, error, sizeof error);
	if (result == AERON_UI_FILE_PICKER_ERROR)
		XwSettingsMenu_ReportError(error);
	if (result == AERON_UI_FILE_PICKER_SELECTED) {
		XwSettings* draft = XwSettingsMenu_Draft();
		if (purpose == SC55_ROMS) {
			snprintf(draft->music.sc55_rom_directory, sizeof draft->music.sc55_rom_directory, "%s", selected);
		} else if (purpose == MT32_CONTROL || purpose == MT32_PCM) {
			char* destination = purpose == MT32_CONTROL ? draft->music.mt32_control : draft->music.mt32_pcm;
			snprintf(destination, XW_PATH_CAPACITY, "%s", selected);
		} else if (purpose == INSTALLATION) {
			XwGameVersion version = picker_version;
			char* destination = NULL;
			switch (version) {
				case XW_GAME_VERSION_93:
					destination = draft->xw93_data;
					break;
				case XW_GAME_VERSION_94:
					destination = draft->xw94_data;
					break;
				case XW_GAME_VERSION_98:
					destination = draft->xw98_data;
					break;
				default:
					return;
			}
			if (!XwInstallation_ValidatePath(version, selected, destination, XW_PATH_CAPACITY, error,
											 sizeof error))
				XwSettingsMenu_ReportError(error);
		}
	}
}

bool XwInstallationPage_Validate(char* error, size_t capacity) {
	XwSettings* draft = XwSettingsMenu_Draft();
	const XwSettings* current = XwConfig_Settings();
	if (!strcmp(draft->flight_version, "xw98") && !XwStorage_HasInstallation(XW_GAME_VERSION_98)) {
		snprintf(error, capacity,
				 "Add the X-Wing (1998) game folder and restart before selecting its flight engine.");
		return false;
	}
	if (!strcmp(draft->flight_version, "xw94") && !XwStorage_HasInstallation(XW_GAME_VERSION_94)) {
		snprintf(error, capacity,
				 "Add the X-Wing (1994) game folder and restart before selecting its flight engine.");
		return false;
	}
	if (!*draft->xw93_data && !*draft->xw94_data && !*draft->xw98_data &&
		!XwStorage_HasInstallation(XW_GAME_VERSION_93) && !XwStorage_HasInstallation(XW_GAME_VERSION_94) &&
		!XwStorage_HasInstallation(XW_GAME_VERSION_98)) {
		snprintf(error, capacity, "Select at least one X-Wing game folder.");
		return false;
	}
	char* paths[] = { draft->xw93_data, draft->xw94_data, draft->xw98_data };
	const XwGameVersion versions[] = { XW_GAME_VERSION_93, XW_GAME_VERSION_94, XW_GAME_VERSION_98 };
	const char* previous[] = { current->xw93_data, current->xw94_data, current->xw98_data };
	for (int i = 0; i < 3; ++i) {
		if (!*paths[i] || !strcmp(paths[i], previous[i]))
			continue;
		if (!XwInstallation_ValidatePath(versions[i], paths[i], paths[i], XW_PATH_CAPACITY, error, capacity))
			return false;
	}
	if (!strcmp(draft->flight_version, "xw93") && !XwStorage_HasInstallation(XW_GAME_VERSION_93)) {
		snprintf(error, capacity,
				 "Add the X-Wing 1993 B-Wing game folder and restart before selecting 1993 flight.");
		return false;
	}
	if (!strcmp(draft->music.arrangement, "windows") && !XwStorage_HasInstallation(XW_GAME_VERSION_98)) {
		snprintf(error, capacity,
				 "Add the X-Wing (1998) game folder and restart before selecting its soundtrack.");
		return false;
	}
	if (strcmp(draft->music.arrangement, "windows") && !*draft->xw94_data &&
		!XwStorage_HasInstallation(XW_GAME_VERSION_94)) {
		snprintf(error, capacity, "Select your X-Wing (1994) game folder to use its soundtrack.");
		return false;
	}
	if (!strcmp(draft->music.arrangement, "gmid") && !strcmp(draft->music.backend, "sc55") &&
		imuse_nuked_sc55_backend_available() &&
		!XwMidiResources_Sc55RomDirectoryValidate(draft->music.sc55_rom_directory, error, capacity))
		return false;
	if (!strcmp(draft->music.arrangement, "rlnd") && XwMidiResources_Mt32Available() &&
		!XwMidiResources_Mt32PairValidate(draft->music.mt32_control, draft->music.mt32_pcm, error, capacity))
		return false;
	if (!strcmp(draft->frontend_version, "xw98") && !*draft->xw98_data &&
		!XwStorage_HasInstallation(XW_GAME_VERSION_98)) {
		snprintf(error, capacity, "Select your X-Wing (1998) game folder to use its cutscenes and menus.");
		return false;
	}
	if (!strcmp(draft->frontend_version, "xw94") && !*draft->xw94_data &&
		!XwStorage_HasInstallation(XW_GAME_VERSION_94)) {
		snprintf(error, capacity, "Select your X-Wing (1994) game folder to use its cutscenes and menus.");
		return false;
	}
	if (!strcmp(draft->inflight_frontend_version, "xw98") && !XwStorage_HasInstallation(XW_GAME_VERSION_98)) {
		snprintf(error, capacity,
				 "Add the X-Wing (1998) game folder and restart before selecting its in-flight menus.");
		return false;
	}
	if (!strcmp(draft->inflight_frontend_version, "xw94") && !XwStorage_HasInstallation(XW_GAME_VERSION_94)) {
		snprintf(error, capacity,
				 "Add the X-Wing (1994) game folder and restart before selecting its in-flight menus.");
		return false;
	}
	return true;
}
