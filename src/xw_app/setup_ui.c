#include "xw_app/setup_ui.h"
#include "xw_app/installation_ui.h"
#include "xw_runtime/audio/midi_resources.h"
#include "xw_runtime/config/config.h"
#include <imuse/midi_nuked_sc55.h>
#include <stdio.h>
#include <string.h>

typedef enum XwSetupPreset {
	XW_SETUP_PRESET_RECOMMENDED,
	XW_SETUP_PRESET_94,
	XW_SETUP_PRESET_98,
	XW_SETUP_PRESET_NONE
} XwSetupPreset;

typedef struct XwSetupPresetDesc {
	const char* label;
	XwGameVersion frontend, flight;
} XwSetupPresetDesc;

static const XwSetupPresetDesc setup_presets[] = {
	[XW_SETUP_PRESET_RECOMMENDED] = { "Recommended", XW_GAME_VERSION_94, XW_GAME_VERSION_98 },
	[XW_SETUP_PRESET_94] = { "X-Wing - Collector's CD-ROM (1994)", XW_GAME_VERSION_94, XW_GAME_VERSION_94 },
	[XW_SETUP_PRESET_98] = { "X-Wing 1998", XW_GAME_VERSION_98, XW_GAME_VERSION_98 },
};

typedef struct XwSetupMusic {
	const char* arrangement;
	const char* backend;
	const char* label;
} XwSetupMusic;

typedef enum XwSetupFrameResult {
	XW_SETUP_FRAME_PENDING,
	XW_SETUP_FRAME_SUCCESS,
	XW_SETUP_FRAME_CANCELLED
} XwSetupFrameResult;

typedef enum XwSetupPickerTarget {
	XW_SETUP_PICKER_93,
	XW_SETUP_PICKER_94,
	XW_SETUP_PICKER_98,
	XW_SETUP_PICKER_SC55,
	XW_SETUP_PICKER_MT32_CONTROL,
	XW_SETUP_PICKER_MT32_PCM
} XwSetupPickerTarget;

typedef struct XwSetupDialog {
	XwInstallationSet* installations;
	XwSettings settings;
	AeronUiFilePicker* picker;
	XwSetupPickerTarget picker_target;
	XwGameVersion picker_version;
	bool override93, override94, override98, preset_touched;
	bool sc55_valid, mt32_valid;
	XwSetupPreset preset;
	char xw93_path[XW_PATH_CAPACITY], xw94_path[XW_PATH_CAPACITY], xw98_path[XW_PATH_CAPACITY],
		sc55_path[XW_PATH_CAPACITY];
	char mt32_control[XW_PATH_CAPACITY], mt32_pcm[XW_PATH_CAPACITY];
	char sc55_error[512], mt32_error[512];
} XwSetupDialog;

static bool PresetAvailable(const XwSetupDialog* setup, XwSetupPreset preset) {
	if ((unsigned)preset >= XW_SETUP_PRESET_NONE)
		return false;
	const XwSetupPresetDesc* desc = &setup_presets[preset];
	return XwInstallation_Get(setup->installations, desc->frontend) &&
		   XwInstallation_Get(setup->installations, desc->flight);
}

static void RefreshPreset(XwSetupDialog* setup) {
	if (setup->preset_touched && PresetAvailable(setup, setup->preset))
		return;
	setup->preset = XW_SETUP_PRESET_NONE;
	for (unsigned i = 0; i < XW_SETUP_PRESET_NONE; ++i) {
		if (PresetAvailable(setup, (XwSetupPreset)i)) {
			setup->preset = (XwSetupPreset)i;
			break;
		}
	}
}

/* Validate saved or edited paths once. Picker selections already validate SC-55 ROMs;
 * MT-32 selections additionally need the two images checked as a compatible pair. */
static void ValidateSc55(XwSetupDialog* setup) {
	setup->sc55_error[0] = 0;
	setup->sc55_valid = setup->sc55_path[0] && imuse_nuked_sc55_backend_available() &&
						XwMidiResources_Sc55RomDirectoryValidate(setup->sc55_path, setup->sc55_error,
																 sizeof setup->sc55_error);
}

static void ValidateMt32(XwSetupDialog* setup) {
	setup->mt32_error[0] = 0;
	setup->mt32_valid = (setup->mt32_control[0] || setup->mt32_pcm[0]) && XwMidiResources_Mt32Available() &&
						XwMidiResources_Mt32PairValidate(setup->mt32_control, setup->mt32_pcm,
														 setup->mt32_error, sizeof setup->mt32_error);
}

static XwSetupMusic PresetMusic(const XwSetupDialog* setup) {
	if (setup->preset == XW_SETUP_PRESET_98)
		return (XwSetupMusic) { "windows", "auto", "CD Music" };
	if (setup->mt32_valid)
		return (XwSetupMusic) { "rlnd", "mt32", "iMUSE / MT-32" };
	if (setup->sc55_valid)
		return (XwSetupMusic) { "gmid", "sc55", "iMUSE / SC-55" };
	return (XwSetupMusic) { "adlb", "adlib", "iMUSE / AdLib" };
}

static bool Commit(XwSetupDialog* setup, char* error, size_t capacity) {
	if (!PresetAvailable(setup, setup->preset)) {
		snprintf(error, capacity, "Select an available preset.");
		return false;
	}
	const XwSetupPresetDesc* preset = &setup_presets[setup->preset];
	XwSetupMusic music = PresetMusic(setup);
	XwSettings* selected = &setup->settings;
	if (!setup->override93)
		strcpy(selected->xw93_data, setup->installations->xw93.root);
	if (!setup->override94)
		strcpy(selected->xw94_data, setup->installations->xw94.root);
	if (!setup->override98)
		strcpy(selected->xw98_data, setup->installations->xw98.root);
	strcpy(selected->frontend_version, preset->frontend == XW_GAME_VERSION_94 ? "xw94" : "xw98");
	strcpy(selected->inflight_frontend_version, "frontend");
	strcpy(selected->flight_version, preset->flight == XW_GAME_VERSION_94 ? "xw94" : "xw98");
	strcpy(selected->music.arrangement, music.arrangement);
	strcpy(selected->music.backend, music.backend);
	strcpy(selected->music.sc55_rom_directory, setup->sc55_path);
	strcpy(selected->music.mt32_control, setup->mt32_control);
	strcpy(selected->music.mt32_pcm, setup->mt32_pcm);
	return XwSetup_ValidateSelection(setup->installations, selected, error, capacity) &&
		   XwSetup_CommitSettings(selected, 1, error, capacity);
}

static void OpenPicker(XwSetupDialog* setup, XwGameVersion version, char* error, size_t capacity) {
	const char* path;
	switch (version) {
		case XW_GAME_VERSION_93:
			setup->picker_target = XW_SETUP_PICKER_93;
			path = setup->xw93_path;
			break;
		case XW_GAME_VERSION_94:
			setup->picker_target = XW_SETUP_PICKER_94;
			path = setup->xw94_path;
			break;
		case XW_GAME_VERSION_98:
			setup->picker_target = XW_SETUP_PICKER_98;
			path = setup->xw98_path;
			break;
		default:
			return;
	}
	setup->picker_version = version;
	XwInstallation_OpenPicker(setup->picker, &setup->picker_version, path, error, capacity);
}

static int AcceptSc55(const char* path, void* user, char* error, size_t capacity) {
	(void)user;
	return XwMidiResources_Sc55RomDirectoryValidate(path, error, capacity);
}

static void OpenSc55Picker(XwSetupDialog* setup, char* error, size_t capacity) {
	setup->picker_target = XW_SETUP_PICKER_SC55;
	const AeronUiFilePickerDesc desc = {
		.mode = AERON_UI_FILE_PICKER_SELECT_DIRECTORY,
		.title = "SELECT SC-55 ROM DIRECTORY",
		.instructions = "Select the folder containing the original SC-55 ROM dumps.",
		.accept_label = "Use This Folder",
		.cancel_label = "Cancel",
		.initial_path = setup->sc55_path[0] ? setup->sc55_path : NULL,
		.accept_fn = AcceptSc55,
	};
	if (AeronUiFilePicker_Open(setup->picker, &desc, error, capacity))
		error[0] = 0;
}

static int AcceptMt32(const char* path, void* user, char* error, size_t capacity) {
	XwSetupDialog* setup = user;
	return XwMidiResources_Mt32RomValidate(path, setup->picker_target == XW_SETUP_PICKER_MT32_CONTROL, error,
										   capacity);
}

static void OpenMt32Picker(XwSetupDialog* setup, bool control, char* error, size_t capacity) {
	setup->picker_target = control ? XW_SETUP_PICKER_MT32_CONTROL : XW_SETUP_PICKER_MT32_PCM;
	const char* path = control ? setup->mt32_control : setup->mt32_pcm;
	char parent[XW_PATH_CAPACITY];
	const AeronUiFilePickerDesc desc = {
		.mode = AERON_UI_FILE_PICKER_OPEN_FILE,
		.title = control ? "SELECT MT-32 CONTROL ROM" : "SELECT MT-32 PCM ROM",
		.instructions = "Select a complete MT-32 compatible ROM image.",
		.accept_label = "Use This File",
		.cancel_label = "Cancel",
		.initial_path = XwMidiResources_ParentDirectory(path, parent, sizeof parent),
		.accept_fn = AcceptMt32,
		.accept_user = setup,
	};
	if (AeronUiFilePicker_Open(setup->picker, &desc, error, capacity))
		error[0] = 0;
}

static void TakeSelection(XwSetupDialog* setup, const char* path, char* error, size_t capacity) {
	if (setup->picker_target == XW_SETUP_PICKER_SC55) {
		snprintf(setup->sc55_path, sizeof setup->sc55_path, "%s", path);
		setup->sc55_valid = true;
		setup->sc55_error[0] = 0;
		error[0] = 0;
		return;
	}
	if (setup->picker_target == XW_SETUP_PICKER_MT32_CONTROL ||
		setup->picker_target == XW_SETUP_PICKER_MT32_PCM) {
		char* destination =
			setup->picker_target == XW_SETUP_PICKER_MT32_CONTROL ? setup->mt32_control : setup->mt32_pcm;
		snprintf(destination, XW_PATH_CAPACITY, "%s", path);
		error[0] = 0;
		ValidateMt32(setup);
		return;
	}
	XwInstallation candidate = { 0 };
	if (!XwInstallation_Open(&candidate, setup->picker_version, path, error, capacity))
		return;
	XwInstallation* target;
	char* destination;
	switch (setup->picker_version) {
		case XW_GAME_VERSION_93:
			target = &setup->installations->xw93;
			destination = setup->xw93_path;
			setup->override93 = false;
			break;
		case XW_GAME_VERSION_94:
			target = &setup->installations->xw94;
			destination = setup->xw94_path;
			setup->override94 = false;
			break;
		case XW_GAME_VERSION_98:
			target = &setup->installations->xw98;
			destination = setup->xw98_path;
			setup->override98 = false;
			break;
		default:
			XwInstallation_Close(&candidate);
			return;
	}
	XwInstallation_Close(target);
	*target = candidate;
	strcpy(destination, target->root);
	RefreshPreset(setup);
}

static XwSetupFrameResult XwSetupUi_DrawRecovery(AeronUiContext* ui, char* error, size_t capacity) {
	XwSetupFrameResult result = XW_SETUP_FRAME_PENDING;
	const AeronUiWindowDesc window = { .width_ref = 920.0f, .centered = 1 };
	if (!AeronUi_BeginWindow(ui, "OpenXW settings", &window))
		return result;
	AeronUi_Help(ui, "Your saved settings could not be loaded.");
	AeronUi_Error(ui, error);
	AeronUi_Separator(ui);
	AeronUi_BeginColumns(ui, 2, NULL);
	if (AeronUi_Button(ui, "Quit without changes"))
		result = XW_SETUP_FRAME_CANCELLED;
	AeronUi_NextColumn(ui);
	if (AeronUi_Button(ui, "Reset to defaults") && XwConfig_Replace(error, capacity))
		result = XW_SETUP_FRAME_SUCCESS;
	AeronUi_EndColumns(ui);
	AeronUi_EndWindow(ui);
	return result;
}

static void DrawSummary(AeronUiContext* ui, const XwSetupDialog* setup) {
	const XwSetupPresetDesc* preset =
		PresetAvailable(setup, setup->preset) ? &setup_presets[setup->preset] : NULL;
	AeronUi_Header(ui, "Preset Settings");
	AeronUi_BeginColumns(ui, 3, NULL);
	AeronUi_Label(ui, "Cutscenes and Menus");
	AeronUi_Label(ui, !preset ? "-" : preset->frontend == XW_GAME_VERSION_94 ? "XW94" : "XW98");
	AeronUi_NextColumn(ui);
	AeronUi_Label(ui, "Flight Engine");
	AeronUi_Label(ui, !preset ? "-" : preset->flight == XW_GAME_VERSION_94 ? "XW94" : "XW98");
	AeronUi_NextColumn(ui);
	AeronUi_Label(ui, "Music");
	AeronUi_Label(ui, preset ? PresetMusic(setup).label : "-");
	AeronUi_EndColumns(ui);
}

static void DrawPresets(AeronUiContext* ui, XwSetupDialog* setup) {
	AeronUi_Header(ui, "Preset");
	AeronUiListItem items[XW_SETUP_PRESET_NONE];
	for (unsigned i = 0; i < XW_SETUP_PRESET_NONE; ++i) {
		items[i] = (AeronUiListItem) {
			.id = i,
			.label = setup_presets[i].label,
			.flags = PresetAvailable(setup, (XwSetupPreset)i) ? AERON_UI_LIST_ITEM_NONE
															  : AERON_UI_LIST_ITEM_DISABLED,
		};
	}
	size_t selected = setup->preset < XW_SETUP_PRESET_NONE ? (size_t)setup->preset : SIZE_MAX;
	if (AeronUi_ListBox(ui, "First-launch preset", items, XW_SETUP_PRESET_NONE, &selected, 120.0f) &
		AERON_UI_LIST_CHANGED) {
		setup->preset = selected < XW_SETUP_PRESET_NONE ? (XwSetupPreset)selected : XW_SETUP_PRESET_NONE;
		setup->preset_touched = selected < XW_SETUP_PRESET_NONE;
	}
	DrawSummary(ui, setup);
}

static XwSetupFrameResult DrawInstallation(XwSetupDialog* setup, AeronUiContext* ui, char* error,
										   size_t capacity) {
	XwSetupFrameResult result = XW_SETUP_FRAME_PENDING;
	const AeronUiWindowDesc window = { .width_ref = 920.0f, .height_ref = 1040.0f, .centered = 1 };
	if (!AeronUi_BeginWindow(ui, "Welcome to OpenXW", &window))
		return result;
	const AeronUiTheme* theme = AeronUi_GetTheme(ui);
	float height = AeronUi_AvailableHeight(ui) - 9.0f - theme->row_height - 3.0f * theme->item_spacing;
	if (!AeronUi_BeginScroll(ui, "First-launch settings", height > 0 ? height : 1)) {
		AeronUi_EndWindow(ui);
		return result;
	}
	AeronUi_Help(ui,
				 "Select your original installations. For the best experience, select both 1994 and 1998.");
	AeronUi_Spacer(ui, 8.0f);
	AeronUi_Header(ui, "Original Installations");
	uint32_t action = XwInstallation_PathRow(ui, XW_GAME_VERSION_93, setup->xw93_path,
											 sizeof setup->xw93_path, AERON_UI_INPUT_TEXT_READ_ONLY);
	if (action & AERON_UI_INPUT_TEXT_ACTION_ACTIVATED)
		OpenPicker(setup, XW_GAME_VERSION_93, error, capacity);
	action = XwInstallation_PathRow(ui, XW_GAME_VERSION_94, setup->xw94_path, sizeof setup->xw94_path,
									AERON_UI_INPUT_TEXT_READ_ONLY);
	if (action & AERON_UI_INPUT_TEXT_ACTION_ACTIVATED)
		OpenPicker(setup, XW_GAME_VERSION_94, error, capacity);
	action = XwInstallation_PathRow(ui, XW_GAME_VERSION_98, setup->xw98_path, sizeof setup->xw98_path,
									AERON_UI_INPUT_TEXT_READ_ONLY);
	if (action & AERON_UI_INPUT_TEXT_ACTION_ACTIVATED)
		OpenPicker(setup, XW_GAME_VERSION_98, error, capacity);
	if (imuse_nuked_sc55_backend_available() || XwMidiResources_Mt32Available()) {
		AeronUi_Spacer(ui, 8.0f);
		AeronUi_Header(ui, "MIDI Music");
		AeronUi_Help(ui,
					 "Have Roland MT-32 or SC-55 ROMs? Select them for the recommended music experience.");
	}
	if (imuse_nuked_sc55_backend_available()) {
		action = AeronUi_InputTextWithAction(ui, "SC-55 ROM Directory", setup->sc55_path,
											 sizeof setup->sc55_path, AERON_UI_INPUT_TEXT_NONE, "Browse...");
		if (action & AERON_UI_INPUT_TEXT_CHANGED)
			ValidateSc55(setup);
		if (action & AERON_UI_INPUT_TEXT_ACTION_ACTIVATED)
			OpenSc55Picker(setup, error, capacity);
		if (setup->sc55_error[0])
			AeronUi_Error(ui, setup->sc55_error);
	}
	if (XwMidiResources_Mt32Available()) {
		action =
			AeronUi_InputTextWithAction(ui, "MT-32 Control ROM", setup->mt32_control,
										sizeof setup->mt32_control, AERON_UI_INPUT_TEXT_NONE, "Browse...");
		bool changed = (action & AERON_UI_INPUT_TEXT_CHANGED) != 0;
		if (action & AERON_UI_INPUT_TEXT_ACTION_ACTIVATED)
			OpenMt32Picker(setup, true, error, capacity);
		action = AeronUi_InputTextWithAction(ui, "MT-32 PCM ROM", setup->mt32_pcm, sizeof setup->mt32_pcm,
											 AERON_UI_INPUT_TEXT_NONE, "Browse...");
		changed |= (action & AERON_UI_INPUT_TEXT_CHANGED) != 0;
		if (changed)
			ValidateMt32(setup);
		if (action & AERON_UI_INPUT_TEXT_ACTION_ACTIVATED)
			OpenMt32Picker(setup, false, error, capacity);
		if (setup->mt32_error[0])
			AeronUi_Error(ui, setup->mt32_error);
	}
	AeronUi_Spacer(ui, 8.0f);
	DrawPresets(ui, setup);
	if (error[0])
		AeronUi_Error(ui, error);
	AeronUi_EndScroll(ui);
	AeronUi_Separator(ui);
	AeronUi_BeginColumns(ui, 2, NULL);
	if (AeronUi_Button(ui, "Quit"))
		result = XW_SETUP_FRAME_CANCELLED;
	AeronUi_NextColumn(ui);
	if (AeronUi_ButtonEnabled(ui, "Continue", PresetAvailable(setup, setup->preset)) &&
		Commit(setup, error, capacity))
		result = XW_SETUP_FRAME_SUCCESS;
	AeronUi_EndColumns(ui);
	AeronUi_EndWindow(ui);
	return result;
}

static XwSetupResult RunDialog(XwAppUi* ui, XwSetupDialog* setup, char* error, size_t capacity) {
	AeronUiContext* context = XwAppUi_Context(ui);
	if (setup) {
		setup->picker = AeronUiFilePicker_Create();
		if (!setup->picker) {
			snprintf(error, capacity, "Could not open the game folder chooser.");
			return XW_SETUP_ERROR;
		}
	}
	XwSetupResult outcome = XW_SETUP_ERROR;
	Aeron_SetHostCursorVisible(1);
	while (!Aeron_QuitRequested() && !Aeron_FatalErrorRequested()) {
		bool picker_was_open = setup && AeronUiFilePicker_IsOpen(setup->picker);
		int32_t delta_us = Aeron_BeginFrame();
		if (Aeron_QuitRequested() || Aeron_FatalErrorRequested())
			break;
		AeronUi_BeginFrame(context, &(AeronUiFrameDesc) { .input = Aeron_InputSnapshot(),
														  .dt_seconds = (float)delta_us * 1e-6f });
		XwSetupFrameResult frame = setup ? DrawInstallation(setup, context, error, capacity)
										 : XwSetupUi_DrawRecovery(context, error, capacity);
		if (setup) {
			char selected[XW_PATH_CAPACITY], picker_error[1024] = { 0 };
			AeronUiFilePickerResult picked = AeronUiFilePicker_Draw(
				setup->picker, context, selected, sizeof selected, picker_error, sizeof picker_error);
			if (picked == AERON_UI_FILE_PICKER_SELECTED)
				TakeSelection(setup, selected, error, capacity);
			else if (picked == AERON_UI_FILE_PICKER_ERROR)
				snprintf(error, capacity, "%s", picker_error);
		}
		AeronUiOutput output = AeronUi_EndFrame(context);
		if (!AeronUi_Submit(context) || !Aeron_Present()) {
			snprintf(error, capacity, "Could not display setup. Please restart OpenXW.");
			break;
		}
		if (frame != XW_SETUP_FRAME_PENDING) {
			outcome = frame == XW_SETUP_FRAME_SUCCESS ? XW_SETUP_SUCCESS : XW_SETUP_CANCELLED;
			break;
		}
		if (output.cancel_pressed && !picker_was_open) {
			outcome = XW_SETUP_CANCELLED;
			break;
		}
		Aeron_WaitForNextFrame(16667);
	}
	if (Aeron_FatalErrorRequested()) {
		snprintf(error, capacity, "Setup could not continue. Please restart OpenXW.");
		outcome = XW_SETUP_ERROR;
	} else if (Aeron_QuitRequested())
		outcome = XW_SETUP_CANCELLED;
	if (setup)
		AeronUiFilePicker_Destroy(setup->picker);
	if (outcome == XW_SETUP_SUCCESS)
		error[0] = 0;
	return outcome;
}

XwSetupResult XwSetupUi_Recover(XwAppUi* ui, char* error, size_t capacity) {
	return RunDialog(ui, NULL, error, capacity);
}

XwSetupResult XwSetupUi_Run(XwAppUi* ui, XwInstallationSet* installations, bool override93, bool override94,
							bool override98, char* error, size_t capacity) {
	XwSetupDialog setup = { .installations = installations,
							.override93 = override93,
							.override94 = override94,
							.override98 = override98,
							.preset = XW_SETUP_PRESET_NONE };
	const XwSettings* settings = XwConfig_Settings();
	setup.settings = *settings;
	snprintf(setup.xw93_path, sizeof setup.xw93_path, "%s",
			 installations->xw93.vfs ? installations->xw93.root : settings->xw93_data);
	snprintf(setup.xw94_path, sizeof setup.xw94_path, "%s",
			 installations->xw94.vfs ? installations->xw94.root : settings->xw94_data);
	snprintf(setup.xw98_path, sizeof setup.xw98_path, "%s",
			 installations->xw98.vfs ? installations->xw98.root : settings->xw98_data);
	snprintf(setup.sc55_path, sizeof setup.sc55_path, "%s", settings->music.sc55_rom_directory);
	snprintf(setup.mt32_control, sizeof setup.mt32_control, "%s", settings->music.mt32_control);
	snprintf(setup.mt32_pcm, sizeof setup.mt32_pcm, "%s", settings->music.mt32_pcm);
	ValidateSc55(&setup);
	ValidateMt32(&setup);
	RefreshPreset(&setup);
	return RunDialog(ui, &setup, error, capacity);
}
