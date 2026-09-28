#ifndef XW_CONTROLLER_SETTINGS_H
#define XW_CONTROLLER_SETTINGS_H

#include <stdbool.h>
#include <stddef.h>

#include "aeron/scene/ui.h"
#include "xw_app/settings/bindings_editor.h"
#include "xw_runtime/config/config.h"

#define XW_CONTROLLER_SETTINGS_ERROR_CAPACITY 512

typedef struct XwControllerSettings {
	XwBindingsEditor editor;
	XwControllerOptions original;
	XwControllerOptions draft;
	XwControllerProfile unconfigured;
	AeronControllerSnapshot disconnected;
	char selected_guid[33];
	uint32_t selected_instance;
	AeronControllerKind active_kind;
	char conflict_text[512];
	bool capacity_warned;
	int page;
	int axis;
	XwInputAxis pending_axis;
	int pending_axis_source;
	AeronControllerDigitalSource pending_digital;
	XwInputAction conflicting_action;
	uint32_t active_instance;
	int axis_conflict_open;
	int binding_conflict_open;
	int restore_modal_open;
	bool dirty;
	char error[XW_CONTROLLER_SETTINGS_ERROR_CAPACITY];
} XwControllerSettings;

void XwControllerSettings_Discover(XwControllerSettings* settings, AeronUiContext* ui,
								   const AeronInputSnapshot* input, const XwControllerProfile* defaults);
void XwControllerSettings_Open(XwControllerSettings* settings, const XwSettings* config);
void XwControllerSettings_Draw(XwControllerSettings* settings, AeronUiContext* ui,
							   const AeronInputSnapshot* input);
void XwControllerSettings_DrawModals(XwControllerSettings* settings, AeronUiContext* ui,
									 const AeronInputSnapshot* input, const XwSettings* config);
void XwControllerSettings_CancelCapture(XwControllerSettings* settings, AeronUiContext* ui);

#endif
