/* Mouse controls follow OpenXvT's editor and X-Wing's targeting behavior. */
#include "xw_app/settings/mouse_page.h"
#include "xw_app/settings/settings.h"
#include "xw_runtime/config/config.h"

void XwMousePage_Draw(AeronUiContext* ui, const AeronInputSnapshot* input) {
	(void)input;
	XwSettings* draft = XwSettingsMenu_Draft();
	if (!AeronUi_BeginScroll(ui, "Mouse settings", XwSettingsMenu_ScrollHeight()))
		return;
	AeronUi_Header(ui, "Mouse Flight Control");
	AeronUi_Toggle(ui, "Mouse flight control", &draft->mouse_flight);
	static const char* const modes[] = { "Virtual stick", "Classic (1994)" };
	int mode = draft->mouse_mode;
	if (AeronUi_Selector(ui, "Control mode", &mode, modes, 2))
		draft->mouse_mode = (XwMouseFlightMode)mode;
	if (draft->mouse_mode == XW_MOUSE_CLASSIC)
		AeronUi_Help(ui, "Move the mouse to turn. Stop moving to let the turn settle back to zero.");
	else
		AeronUi_Help(ui, "Mouse movement deflects a virtual stick. Move it back to center to stop turning.");
	AeronUi_SliderInt(ui, "Sensitivity", &draft->mouse_sensitivity, 1, 9, 1, "%d");
	AeronUi_Toggle(ui, "Invert Y", &draft->mouse_invert_y);
	AeronUi_Help(ui, "Mouse up pitches up; with Invert Y enabled, mouse up pitches down.");
	if (draft->mouse_mode == XW_MOUSE_CLASSIC)
		AeronUi_Help(ui, "Enable Invert Y for the original 1994 mouse direction.");
	AeronUi_Help(ui, "Left button: fire.");
	AeronUi_Help(ui, "Right button: tap to target under the crosshair, hold to roll.");
	AeronUi_Help(ui, "Middle button: target nearest fighter.");
	AeronUi_Help(ui, "Side button: toggle cockpit.");
	AeronUi_Help(ui,
				 "Ctrl+Alt+M releases or captures the pointer. Click in the flight view to recapture it.");
	if (AeronUi_Button(ui, "Restore mouse defaults")) {
		const XwSettings* defaults = XwConfig_DefaultSettings();
		draft->mouse_flight = defaults->mouse_flight;
		draft->mouse_mode = defaults->mouse_mode;
		draft->mouse_sensitivity = defaults->mouse_sensitivity;
		draft->mouse_invert_y = defaults->mouse_invert_y;
	}
	AeronUi_EndScroll(ui);
}
