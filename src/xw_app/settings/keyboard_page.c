#include "xw_app/settings/keyboard_page.h"
#include <stdio.h>
#include <string.h>

void XwKeyboardSettings_Open(XwKeyboardSettings* settings, const XwSettings* config) {
	memset(settings, 0, sizeof *settings);
	settings->original = settings->draft = config->keyboard;
	XwBindingsEditor_Init(&settings->editor);
}

void XwKeyboardSettings_CancelCapture(XwKeyboardSettings* settings, AeronUiContext* ui) {
	AeronUi_CancelKeyboardCapture(ui);
	settings->editor.binding_modal_open = 0;
	settings->conflict_open = settings->restore_open = 0;
}

static void Changed(XwKeyboardSettings* settings) {
	XwKeyboardMapping_Sort(&settings->draft);
	settings->dirty =
		settings->restore_defaults || !XwKeyboardMapping_Equal(&settings->original, &settings->draft);
	settings->restore_defaults = false;
	settings->editor.binding_selected = SIZE_MAX;
	settings->error[0] = 0;
}

static const char* ReservedShortcutMessage(XwKeyboardShortcut shortcut) {
	switch (shortcut) {
		case XW_KEYBOARD_SHORTCUT_SETTINGS:
			return "Esc opens settings and cannot be reassigned.";
		case XW_KEYBOARD_SHORTCUT_DEBUG:
			return "The grave accent key opens the debug menu and cannot be reassigned.";
		case XW_KEYBOARD_SHORTCUT_MOUSE:
			return "Ctrl+Alt+M captures or releases the mouse and cannot be reassigned.";
		case XW_KEYBOARD_SHORTCUT_RENDERER:
			return "Tab switches graphics and cannot be reassigned.";
		default:
			return NULL;
	}
}

static bool Captured(XwKeyboardSettings* settings, AeronUiContext* ui, const char* label, const char* display,
					 AeronKeyChord* chord) {
	if (AeronUi_KeyboardCapture(ui, label, display, chord) != AERON_UI_KEYBOARD_CAPTURE_CAPTURED)
		return false;
	const char* reserved = ReservedShortcutMessage(XwKeyboardMapping_Shortcut(*chord));
	if (reserved) {
		snprintf(settings->error, sizeof settings->error, "%s", reserved);
		return false;
	}
	if (!XwKeyboardMapping_SourceValid(*chord)) {
		snprintf(settings->error, sizeof settings->error, "This key combination is not supported.");
		return false;
	}
	settings->error[0] = 0;
	return true;
}

static void AddBinding(XwKeyboardSettings* settings, AeronKeyChord source) {
	size_t existing = XwKeyboardMapping_Find(&settings->draft, source);
	if (existing != SIZE_MAX) {
		XwInputAction action = settings->draft.bindings[existing].action;
		if (action == settings->editor.selected_action) {
			size_t position = action == XW_INPUT_ACTION_ESCAPE ? 1 : 0;
			for (size_t i = 0; i < existing; ++i)
				position += settings->draft.bindings[i].action == action;
			settings->editor.binding_selected = position;
		} else {
			settings->pending = source;
			settings->conflicting_action = action;
			settings->conflict_open = 1;
		}
		return;
	}
	if (settings->draft.count == XW_KEYBOARD_BINDING_CAP) {
		snprintf(settings->error, sizeof settings->error, "Remove a binding before adding another.");
		return;
	}
	settings->draft.bindings[settings->draft.count++] =
		(XwKeyboardBinding) { source, settings->editor.selected_action };
	Changed(settings);
}

static void DescribeAction(const XwKeyboardBindings* profile, XwInputAction action, char* text,
						   size_t capacity) {
	snprintf(text, capacity, "%s", action == XW_INPUT_ACTION_ESCAPE ? "Esc (fixed)" : "");
	for (size_t i = 0; i < profile->count; ++i) {
		if (profile->bindings[i].action != action)
			continue;
		char label[128];
		XwKeyboardMapping_FormatSource(label, sizeof label, profile->bindings[i].source);
		size_t length = strlen(text);
		int written =
			snprintf(text + length, capacity - length, "%s%s%s", length ? "   " : "", label,
					 XwKeyboardMapping_Shortcut(profile->bindings[i].source) == XW_KEYBOARD_SHORTCUT_RENDERER
						 ? " (switches graphics)"
						 : "");
		if (written < 0 || (size_t)written >= capacity - length)
			break;
	}
	if (!text[0])
		snprintf(text, capacity, "Not Bound");
}

void XwKeyboardSettings_Draw(XwKeyboardSettings* settings, AeronUiContext* ui) {
	AeronUi_Help(ui, "Tab switches between original and modern graphics.");
	XwBindingsEditor_Category(&settings->editor, ui);
	AeronKeyChord source;
	if (Captured(settings, ui, "Find Binding...", "Press to identify", &source)) {
		size_t index = XwKeyboardMapping_Find(&settings->draft, source);
		if (index == SIZE_MAX)
			snprintf(settings->error, sizeof settings->error, "This key combination is not bound.");
		else
			XwBindingsEditor_Select(&settings->editor, settings->draft.bindings[index].action, false);
	}
	AeronUi_Spacer(ui, 8.0f);
	AeronUiListItem items[XW_INPUT_ACTION_COUNT - 1];
	char details[XW_INPUT_ACTION_COUNT - 1][256];
	size_t count = 0;
	for (int action = XW_INPUT_ACTION_NONE + 1; action < XW_INPUT_ACTION_COUNT; ++action) {
		if (!XwInputActions_Visible((XwInputAction)action) ||
			!XwInputActions_KeyboardBindable((XwInputAction)action) ||
			(int)XwInputActions_Category((XwInputAction)action) != settings->editor.category)
			continue;
		DescribeAction(&settings->draft, (XwInputAction)action, details[count], sizeof details[count]);
		items[count] = (AeronUiListItem) { .id = (uint64_t)action,
										   .label = XwInputActions_DisplayName((XwInputAction)action),
										   .detail = details[count] };
		++count;
	}
	float trailing_height = 143.0f;
	if (settings->error[0])
		trailing_height += AeronUi_MeasureHelpHeight(ui, settings->error, 0.0f) + 60.0f;
	XwBindingsEditor_Actions(&settings->editor, ui, items, count, trailing_height);
	if (settings->error[0]) {
		AeronUi_Error(ui, settings->error);
		if (AeronUi_Button(ui, "Dismiss Error"))
			settings->error[0] = 0;
	}
	if (AeronUi_Button(ui, "Restore Defaults")) {
		AeronUi_CancelKeyboardCapture(ui);
		settings->restore_open = 1;
	}
}

static void Detail(XwKeyboardSettings* settings, AeronUiContext* ui) {
	if (!XwBindingsEditor_BeginDetail(&settings->editor, ui))
		return;
	AeronUiListItem items[XW_KEYBOARD_BINDING_CAP + 1];
	char labels[XW_KEYBOARD_BINDING_CAP + 1][128];
	size_t count = 0;
	if (settings->editor.selected_action == XW_INPUT_ACTION_ESCAPE)
		items[count++] = (AeronUiListItem) { .id = SIZE_MAX, .label = "Esc", .detail = "Fixed shortcut" };
	for (size_t i = 0; i < settings->draft.count; ++i) {
		if (settings->draft.bindings[i].action != settings->editor.selected_action)
			continue;
		XwKeyboardMapping_FormatSource(labels[count], sizeof labels[count],
									   settings->draft.bindings[i].source);
		items[count] = (AeronUiListItem) { .id = i,
										   .label = labels[count],
										   .detail = ReservedShortcutMessage(XwKeyboardMapping_Shortcut(
											   settings->draft.bindings[i].source)) };
		++count;
	}
	XwBindingsEditor_List(&settings->editor, ui, items, count, "This action has no keyboard binding.");
	if (settings->editor.binding_selected < count &&
		items[settings->editor.binding_selected].id != SIZE_MAX && XwBindingsEditor_Remove(ui)) {
		XwKeyboardMapping_Remove(&settings->draft, (size_t)items[settings->editor.binding_selected].id);
		Changed(settings);
	}
	AeronUi_Separator(ui);
	AeronKeyChord source;
	if (Captured(settings, ui, "Add Binding...", "Press to add", &source))
		AddBinding(settings, source);
	if (settings->error[0])
		AeronUi_Error(ui, settings->error);
	XwBindingsEditor_EndDetail(&settings->editor, ui);
}

static void Restore(XwKeyboardSettings* settings, AeronUiContext* ui) {
	if (!AeronUi_BeginModal(ui, "RESTORE KEYBOARD DEFAULTS", &settings->restore_open, NULL))
		return;
	AeronUi_Help(ui, "Replace all keyboard bindings with the defaults?");
	AeronUi_BeginColumns(ui, 2, NULL);
	if (AeronUi_Button(ui, "Restore")) {
		settings->draft = XwConfig_DefaultSettings()->keyboard;
		settings->restore_defaults = settings->dirty = true;
		settings->restore_open = 0;
		settings->error[0] = 0;
		XwBindingsEditor_Init(&settings->editor);
	}
	AeronUi_NextColumn(ui);
	if (AeronUi_Button(ui, "Cancel"))
		settings->restore_open = 0;
	AeronUi_EndColumns(ui);
	AeronUi_EndModal(ui);
}

void XwKeyboardSettings_DrawModals(XwKeyboardSettings* settings, AeronUiContext* ui) {
	if (settings->conflict_open) {
		char label[128];
		XwKeyboardMapping_FormatSource(label, sizeof label, settings->pending);
		if (XwBindingsEditor_Conflict(ui, &settings->conflict_open, label, settings->conflicting_action,
									  settings->editor.selected_action)) {
			size_t existing = XwKeyboardMapping_Find(&settings->draft, settings->pending);
			if (existing != SIZE_MAX) {
				settings->draft.bindings[existing].action = settings->editor.selected_action;
				Changed(settings);
			}
		}
	} else if (settings->restore_open) {
		Restore(settings, ui);
	} else {
		Detail(settings, ui);
	}
}
