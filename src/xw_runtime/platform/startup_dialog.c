#include "xw_runtime/platform/startup_dialog.h"

#include <aeron/dialog.h>
#include <aeron/log.h>

void XwPort_WriteStartupDiagnostic(const char* message) {
	Aeron_LogMessage(AERON_LOG_DEBUG, "startup", "%s", message);
}

int XwPort_ShowStartupDialog(void* owner, const char* text, const char* caption, unsigned int buttons) {
	AeronMessageBoxButton choices[] = { { XW_STARTUP_DIALOG_RESULT_OK, "OK", 1,
										  buttons == XW_STARTUP_DIALOG_OK },
										{ XW_STARTUP_DIALOG_RESULT_CANCEL, "Cancel", 0, 1 } };
	AeronMessageBoxOptions options;
	int selected = XW_STARTUP_DIALOG_ERROR;
	if (owner != NULL || (buttons != XW_STARTUP_DIALOG_OK && buttons != XW_STARTUP_DIALOG_OK_CANCEL)) {
		return XW_STARTUP_DIALOG_ERROR;
	}
	options.kind = AERON_MESSAGE_BOX_INFORMATION;
	options.title = caption;
	options.message = text;
	options.buttons = choices;
	options.button_count = buttons == XW_STARTUP_DIALOG_OK_CANCEL ? sizeof(choices) / sizeof(choices[0]) : 1;
	if (!Aeron_ShowMessageBox(&options, &selected)) {
		return XW_STARTUP_DIALOG_ERROR;
	}
	if (selected == XW_STARTUP_DIALOG_DISMISSED) {
		return buttons == XW_STARTUP_DIALOG_OK_CANCEL ? XW_STARTUP_DIALOG_RESULT_CANCEL
													  : XW_STARTUP_DIALOG_RESULT_OK;
	}
	return selected;
}
