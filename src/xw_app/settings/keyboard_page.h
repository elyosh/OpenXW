#ifndef XW_KEYBOARD_SETTINGS_H
#define XW_KEYBOARD_SETTINGS_H
#include "xw_app/settings/bindings_editor.h"
#include "xw_runtime/config/config.h"

typedef struct XwKeyboardSettings {
	XwKeyboardBindings original;
	XwKeyboardBindings draft;
	XwBindingsEditor editor;
	AeronKeyChord pending;
	XwInputAction conflicting_action;
	int conflict_open;
	int restore_open;
	bool dirty;
	bool restore_defaults;
	char error[512];
} XwKeyboardSettings;

void XwKeyboardSettings_Open(XwKeyboardSettings* settings, const XwSettings* config);
void XwKeyboardSettings_Draw(XwKeyboardSettings* settings, AeronUiContext* ui);
void XwKeyboardSettings_DrawModals(XwKeyboardSettings* settings, AeronUiContext* ui);
void XwKeyboardSettings_CancelCapture(XwKeyboardSettings* settings, AeronUiContext* ui);
#endif
