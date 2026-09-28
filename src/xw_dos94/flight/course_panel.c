#ifdef XW_MODERN
#include "xw_runtime/snapshot/render_hud.h"
#endif
#include "xw/flight/feinput.h"
#include "xw/flight/gate.h"
#include "xw/flight/hud/festring.h"
#include "xw/flight/hud/panel.h"
#include "xw/flight/hud/panelrts.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/replay/replay.h"
#include "xw/flight/xw.h"
#include "xw_dos94/flight/hud/panel.h"
#include "xw_dos94/flight/special_world.h"

/* DOS94 0x78201C: fixed VGA layout, independent of frontend resolution. */
void Dos94_gate_trainingupdatecrt(int16_t x, int16_t y) {
#ifdef XW_MODERN
	int hud_pane = XwHud_Push(XW_SNAP_PANE_COURSE);
#endif

	x += 4;
	++y;
	festring_setfontsize(2);
	festring_setautofill(1);
	festring_setbackcolor(29);
	if (g_hudFullRedrawInProgress) {
		festring_setbound(x, y, x + 60, y + 41);
		g_flightFillClipRectFn();
		festring_settextcolor(68);
		festring_setcursor(x + 12, y);
		festring_outstring("LEVEL:");
		festring_settextcolor(67);
		festring_setcursor(x + 40, y);
		panelrts_outnum(g_missionRuntimeState.provingGroundsLevel, 2, 2);
		char* labels[5] = { "REMAINING:", "MISSED:", "PASSED:", "TARGETS:", "SCORE:" };
		for (unsigned i = 0; i < 5; ++i) {
			festring_settextcolor(i < 3 ? 66 : i == 3 ? 72 : 70);
			festring_setcursor(x, y + 7 * (i + 1));
			festring_outstring(labels[i]);
		}
	}
	festring_setbound(x, y, x + 66, y + 41);
	uint16_t values[4] = { g_missionRuntimeState.provingGroundsCheckpointsRemaining,
						   g_missionRuntimeState.provingGroundsCheckpointsMissed,
						   g_missionRuntimeState.provingGroundsCheckpointsPassed,
						   g_missionRuntimeState.provingGroundsTargetsDestroyed };
	for (unsigned i = 0; i < 4; ++i) {
		festring_settextcolor(i < 3 ? 65 : 71);
		festring_setcursor(x + 48, y + 7 * (i + 1));
		panelrts_outnum(values[i], 3, 1);
	}
	festring_settextcolor(69);
	festring_setcursor(x + 36, y + 35);
	gate_outdnum(g_missionRuntimeState.provingGroundsScore, 6, 1);

#ifdef XW_MODERN
	XwHud_Pop(hud_pane);
#endif
}

/* DOS94 0x782326: the full nested panel call intentionally updates shared aim/query state. */
void Dos94_gate_updatebonuspoints(void) {
#ifdef XW_MODERN
	int hud_pane = XwHud_Push(XW_SNAP_PANE_COURSE);
#endif

	g_flightTextShadowEnabled = 1;
	festring_setbackcolor(25);
	festring_settextcolor(73);
	festring_setautofill(0);
	festring_setfontsize(1);
	festring_setbound(0, 190, 320, 200);
	festring_setcursor(198, 191);
	panelrts_outnum(g_missionCountdownClock.minutes, 2, 2);
	g_flightDrawCharFn(':');
	panelrts_outnum(g_missionCountdownClock.seconds, 2, 2);
	festring_setcursor(252, 191);
	panelrts_outnum(g_missionRuntimeState.provingGroundsTimeBonus, 5, 5);
	if (!g_replayviewmode)
		panel_updatepanel();

#ifdef XW_MODERN
	XwHud_Pop(hud_pane);
#endif
}
