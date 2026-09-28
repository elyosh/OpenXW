#include "xw/flight/feinput.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/hud/panel.h"
#include "xw/flight/object/create.h"
#include "xw/render/rtsvga2.h"
#include "xw_runtime/runtime/flight_types.h"
#include "xw_runtime/snapshot/render_hud_internal.h"
#include "xw_runtime/snapshot/render_objects.h"

void XwHud_Radar(XwRadarBlip* blips, unsigned count, bool completed) {
	XwHudState* s = XwHud_Working();
	if (!s)
		return;
	if (count > XW_SNAP_RADAR_BLIPS) {
		XwHud_Fail(s, "radar capacity exceeded");
		return;
	}
	XwSnapRadar* r = &s->cockpit.radar;
	bool front = blips == g_radarFrontBlips;
	XwSnapRadarBlip* out = front ? r->front : r->rear;
	if (front) {
		r->front_count = count;
		r->front_visible = 1;
	} else {
		r->rear_count = count;
		r->rear_visible = 1;
	}
	for (unsigned i = 0; i < count; ++i) {
		if (!completed)
			out[i] = (XwSnapRadarBlip) { .x = blips[i].x,
										 .y = blips[i].y,
										 .color_index = (uint8_t)blips[i].colorOrDrawMask };
		else
			out[i].coverage = XwFlightTypes_Dos() || g_flightBytesPerPixel == 2
								  ? (uint8_t)blips[i].colorOrDrawMask != 0
								  : blips[i].colorOrDrawMask & 3;
	}
	if (completed)
		XwHud_Changed(s);
}

void XwHud_Marker(bool cross, bool visible, int x, int y, unsigned color) {
	XwHudState* s = XwHud_Working();
	if (!s)
		return;
	XwSnapRadar* r = &s->cockpit.radar;
	r->tall_bracket = !XwFlightTypes_Dos() && g_flightBytesPerPixel == 1 && g_flightBracketOffsetCount == 12;
	if (cross) {
		r->cross_x = x;
		r->cross_y = y;
		r->cross_color = color;
		r->cross_visible = visible;
	} else {
		r->target_x = x;
		r->target_y = y;
		r->target_visible = visible;
	}
	XwHud_Changed(s);
}

void XwHud_Target(uint16_t reference, uint16_t component, int extent, uint8_t color, bool overlay) {
	XwRenderSnapshot* s = XwRenderCapture_Pending();
	if (!s)
		return;
	s->target_box = (XwSnapTargetBox) { .object = XwRenderObjects_Id(reference),
										.world_pos = { g_resolvedWorldX, g_resolvedWorldY, g_resolvedWorldZ },
										.extent = extent,
										.component = component,
										.color_index = color,
										.visible = 1,
										.direct_overlay = overlay };
}
