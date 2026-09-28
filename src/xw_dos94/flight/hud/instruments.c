#include "xw/flight/player/user.h"
#include "xw/flight/xw.h"
#include "xw_dos94/flight/hud/panel.h"
#include "xw_dos94/render/display.h"
#include "xw_runtime/snapshot/render_hud.h"

void Dos94_panel_updatethrottle(void) {
	if (!(g_playerFlightState.craft->activeHudFeatureMask & 4))
		return;
	uint16_t filled = g_playerFlightState.craft->engineThrottle[0] / 1725;
	XwHud_Widget(22, filled, 0, 0, 0, 0, 0, XW_SNAP_WIDGET_THROTTLE);
	if (filled == g_hudElementStateCache[22])
		return;
	g_hudElementStateCache[22] = filled;
	for (unsigned i = 0; i < 38; ++i) {
		uint8_t color = i >= filled ? 0 : (i >= 34 ? 54 : i >= 30 ? 58 : 62) - (i & 1 ? 2 : 0);
		Dos94_rtsvga2_drawdotVGA(g_hudElementLayouts[22].x + i, g_hudElementLayouts[22].y, color);
	}
}
