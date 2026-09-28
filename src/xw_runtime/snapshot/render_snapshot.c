/* Three owned slots follow OpenXvT; cooperative pending views live outside them. */
#include "xw_runtime/snapshot/render_assets.h"
#include "xw_runtime/snapshot/render_capture.h"
#include "xw_runtime/snapshot/render_hud.h"
#include "xw_runtime/snapshot/render_snapshot_internal.h"
#include <stddef.h>
#include <string.h>

static XwRenderSnapshot slots[3];
static int initialized, tick_open, writer, current = -1, previous = -1;
static uint64_t host_serial;

void XwRenderSnapshot_Clear(XwRenderSnapshot* s) {
	XwRenderAssets_Release(s->sky.stars);
	XwRenderAssets_Release(s->flight_assets);
	XwRenderAssets_Release(s->loaded_cockpit);
	XwHud_Release(&s->cockpit);
	memset(s, 0, offsetof(XwRenderSnapshot, objects));
	memset(&s->cockpit, 0, offsetof(XwSnapCockpit, glyphs));
}

void XwRenderSnapshot_Copy(XwRenderSnapshot* dst, const XwRenderSnapshot* src) {
	XwRenderAssets_Retain(src->sky.stars);
	XwRenderAssets_Release(dst->sky.stars);
	XwRenderAssets_Retain(src->flight_assets);
	XwRenderAssets_Retain(src->loaded_cockpit);
	XwRenderAssets_Release(dst->loaded_cockpit);
	XwRenderAssets_Release(dst->flight_assets);
	memcpy(dst, src, offsetof(XwRenderSnapshot, objects));
	memcpy(dst->objects, src->objects, src->object_count * sizeof src->objects[0]);
	memcpy(dst->crafts, src->crafts, src->craft_count * sizeof src->crafts[0]);
	XwHud_Copy(&dst->cockpit, &src->cockpit);
}

void XwRenderSnapshot_Init(void) {
	if (initialized)
		return;
	XwRenderAssets_Init();
	for (unsigned i = 0; i < 3; ++i)
		XwRenderSnapshot_Clear(&slots[i]);
	writer = 0;
	current = previous = -1;
	host_serial = 0;
	tick_open = 0;
	initialized = 1;
	XwRenderCapture_Init();
}

void XwRenderSnapshot_Shutdown(void) {
	XwRenderCapture_Shutdown();
	for (unsigned i = 0; i < 3; ++i)
		XwRenderSnapshot_Clear(&slots[i]);
	XwRenderAssets_Shutdown();
	initialized = tick_open = 0;
	current = previous = -1;
}

void XwRenderSnapshot_BeginTick(void) {
	if (!initialized || tick_open)
		return;
	if (current >= 0)
		XwRenderSnapshot_Copy(&slots[writer], &slots[current]);
	else
		XwRenderSnapshot_Clear(&slots[writer]);
	tick_open = 1;
	XwRenderCapture_Export(&slots[writer]);
}

void XwRenderSnapshot_Commit(int focused, int paused) {
	if (!initialized || !tick_open)
		return;
	XwRenderSnapshot* s = &slots[writer];
	XwRenderCapture_Export(s);
	s->host_serial = ++host_serial;
	s->focused = focused != 0;
	s->paused = paused != 0;
	previous = current;
	current = writer;
	for (int i = 0; i < 3; ++i)
		if (i != current && i != previous) {
			writer = i;
			break;
		}
	tick_open = 0;
}

void XwRenderSnapshot_CancelTick(void) {
	if (tick_open)
		XwRenderSnapshot_Clear(&slots[writer]);
	tick_open = 0;
}

const XwRenderSnapshot* XwRenderSnapshot_Current(void) { return current >= 0 ? &slots[current] : NULL; }

const XwRenderSnapshot* XwRenderSnapshot_Previous(void) { return previous >= 0 ? &slots[previous] : NULL; }

XwRenderSnapshot* XwRenderSnapshot_Writer(void) { return tick_open ? &slots[writer] : NULL; }
