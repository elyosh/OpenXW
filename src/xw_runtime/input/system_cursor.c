#include "xw_runtime/input/system_cursor.h"
#include "xw/input/win_mouse.h"
#include "xw_runtime/config/config.h"
#include "xw_runtime/input/input_bridge.h"
#include "xw_runtime/runtime/flight_sim.h"
#include "xw_runtime/runtime/presentation.h"
#include <aeron/aeron.h>
#include <stdint.h>

static int32_t displayCount;

void XwPort_RefreshSystemCursorVisibility(void) {
	const XwSettings* settings = XwConfig_Settings();
	Aeron_SetHostCursorVisible(
		XwPresentation_PointerSuppressed() ||
		(settings && settings->mouse_flight && XwFlightSim_IsPlayerControl() && !Aeron_RelativeMouseMode()) ||
		(!g_SoftwareCursor && displayCount >= 0));
}

void XwPort_SetSystemCursorDisplayCount(int count) {
	displayCount = count;
	XwPort_RefreshSystemCursorVisibility();
}

int XwPort_SetSystemCursorPosition(int x, int y) { return XwPresentation_WarpScene(x, y); }

int XwPort_ShowSystemCursor(int show) {
	XwPort_SetSystemCursorDisplayCount((int32_t)((uint32_t)displayCount + (show ? 1u : UINT32_MAX)));
	return displayCount;
}

int XwPort_GetSystemCursorPosition(struct XwMousePosition* position) {
	int16_t x, y;
	XwPort_UpdateMouseButtons();
	XwInput_MousePosition(NULL, &x, &y);
	position->x = x;
	position->y = y;
	return Aeron_InputSnapshot() != NULL;
}

void XwPort_UpdateMouseButtons(void) {
	XwInput_BeginFrame(Aeron_InputSnapshot(), XwPresentation_PointerSuppressed(), 0);
}
