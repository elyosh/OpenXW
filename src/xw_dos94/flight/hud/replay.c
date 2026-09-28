#include "xw_dos94/flight/hud/replay.h"
#include "xw/flight/feinput.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/hud/festring.h"
#include "xw/flight/hud/panel.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/object.h"
#include "xw/flight/replay/replay.h"
#include "xw/flight/xw.h"

/* DOS94 0x6AE980, 0x5517A, 0x5512E. */
char Dos94_panelStatusStrings[11][9] = { "      OK", " STOPPED", "DISABLED", "CAPTURED",
										 "        ", "  HOMING", "SHLDS DN", "HULL DMG",
										 " WAITING", " CLOSING", "IN RANGE" };
static const uint16_t buttonX[38] = { 47,  47,  82, 82, 117, 117, 210, 210, 245, 245, 31,  31,  32,
									  32,  75,  75, 75, 75,  37,  37,  62,  62,  89,  89,  174, 174,
									  258, 258, 42, 42, 43,  43,  86,  86,  86,  86,  205, 205 };
static const uint16_t buttonY[38] = { 1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   159, 159, 178,
									  178, 178, 178, 164, 164, 147, 147, 147, 147, 147, 147, 147, 147,
									  147, 147, 160, 160, 178, 178, 178, 178, 165, 165, 147, 147 };

static void bounds(bool film, bool tracked, bool name) {
	int left = name ? (film ? 139 : 127) : (film ? 241 : 230);
	int right = name ? (film ? 211 : 201) : (film ? 279 : 268);
	int top = tracked ? 181 : 167;
	festring_setbound(left, top, right, top + 5);
	festring_setcursor(left, top);
}

void Dos94Replay_StatusBounds(bool tracked) { bounds(!(uint8_t)g_flightDisplaySurfaceMode, tracked, false); }

void Dos94Replay_ProgressBounds(void) {
	if ((uint8_t)g_flightDisplaySurfaceMode) {
		festring_setbound(154, 5, 165, 11);
		festring_setcursor(156, 5);
	} else {
		festring_setbound(115, 150, 126, 155);
		festring_setcursor(117, 150);
	}
}

void Dos94_replay_outputclipname(void) {
	festring_setbackcolor(0x40);
	if ((uint8_t)g_flightDisplaySurfaceMode) {
		festring_setbound(169, 5, 206, 11);
		festring_setcursor(169, 5);
	} else {
		festring_setbound(130, 150, 169, 155);
		festring_setcursor(130, 150);
	}
	g_flightFillClipRectFn();
	festring_settextcolor(0x49);
	festring_outstringcenter(g_ReplayClipName);
}

void Dos94_replay_drawreplaybutton(uint16_t button) {
	if (!(uint8_t)g_flightDisplaySurfaceMode && button < 18)
		button += 18;
	if (button >= 38)
		return;
	g_flightBlitSpriteFn(g_hudPanelSpriteDataByIndex[111 + button], buttonX[button], buttonY[button], 0, 0);
	bool film = button >= 18;
	unsigned operation = film ? button - 18 : button;
	if (operation < 14 || operation > 17)
		return;
	bool tracked = operation < 16, show = operation & 1;
	bounds(film, tracked, true);
	festring_setbackcolor(0x40);
	g_flightFillClipRectFn();
	uint16_t ref = tracked ? g_trackobject : g_replayCamera.focusObjectRef;
	if (show) {
		bool valid = ref < 116 || (ref >= 0x3800 && ref - 0x3800 < 64);
		if (!valid)
			return;
		replay_outputobjectname(ref);
		uint16_t type = ref < 116 ? g_objectTable[ref].objectType : g_missionObjects[ref - 0x3800].objectType;
		if (tracked)
			g_replayTrackedObjectType = type;
		else
			g_replayChaseObjectType = type;
	}
	bounds(film, tracked, false);
	g_flightFillClipRectFn();
	if (show) {
		festring_settextcolor(0x47);
		unsigned status = replay_getstatusnum(ref);
		if (status < 11)
			festring_outstringcenter(Dos94_panelStatusStrings[status]);
	}
	if (!tracked)
		g_ReplayChaseStatusVisible = show;
}
