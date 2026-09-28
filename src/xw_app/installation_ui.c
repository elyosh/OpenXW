#include "xw_app/installation_ui.h"
#include <stdio.h>

static int AcceptInstallation(const char* path, void* user, char* error, size_t capacity) {
	XwInstallation candidate = { 0 };
	bool ok = XwInstallation_Open(&candidate, *(XwGameVersion*)user, path, error, capacity);
	XwInstallation_Close(&candidate);
	return ok;
}

uint32_t XwInstallation_PathRow(AeronUiContext* ui, XwGameVersion version, char* path, size_t capacity,
								uint32_t flags) {
	char label[32];
	snprintf(label, sizeof label, "X-Wing (%d)", XwGameVersion_Year(version));
	return AeronUi_InputTextWithAction(ui, label, path, capacity, flags, "Browse...");
}

int XwInstallation_OpenPicker(AeronUiFilePicker* picker, XwGameVersion* version, const char* path,
							  char* error, size_t capacity) {
	const char* title;
	const char* instructions;
	switch (*version) {
		case XW_GAME_VERSION_93:
			title = "SELECT X-WING (1993)";
			instructions = "Select your extracted X-Wing (1993) B-Wing installation folder.";
			break;
		case XW_GAME_VERSION_94:
			title = "SELECT X-WING (1994)";
			instructions = "Select your X-Wing Collector's CD-ROM (1994) folder.";
			break;
		case XW_GAME_VERSION_98:
			title = "SELECT X-WING (1998)";
			instructions = "Select your X-Wing (1998) folder.";
			break;
		default:
			return 0;
	}
	const AeronUiFilePickerDesc desc = {
		.mode = AERON_UI_FILE_PICKER_SELECT_DIRECTORY,
		.title = title,
		.instructions = instructions,
		.accept_label = "Use This Folder",
		.cancel_label = "Cancel",
		.initial_path = path && *path ? path : NULL,
		.accept_fn = AcceptInstallation,
		.accept_user = version,
	};
	return AeronUiFilePicker_Open(picker, &desc, error, capacity);
}
