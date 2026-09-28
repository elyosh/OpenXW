/* Surface owners follow the existing composition schedule, independently of dirty caches. */
#include "xw/flight/feinput.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/hud/festring.h"
#include "xw/flight/hud/panel.h"
#include "xw/flight/player/user.h"
#include "xw/flight/replay/replay.h"
#include "xw/render/flight_view.h"
#include "xw/render/rtsrgb.h"
#include "xw/render/rtsvga2.h"
#include "xw_runtime/platform/classic_surfaces.h"
#include "xw_runtime/runtime/flight_types.h"
#include "xw_runtime/snapshot/render_hud_internal.h"
#include <aeron/aeron.h>
#include <stddef.h>
#include <string.h>

static XwHudState owners[XW_RENDER_SURFACE_COUNT];
static uint64_t revision;
static bool enabled, reported;
static int pane = -1;

static void References(const XwSnapCockpit* c, void (*op)(XwRenderAssetId)) {
	op(c->definition);
	for (unsigned i = 0; i < c->glyph_count; ++i)
		op(c->glyphs[i].font);
	for (unsigned i = 0; i < c->sprite_count; ++i)
		op(c->sprites[i].image);
}

void XwHud_Release(XwSnapCockpit* c) {
	References(c, XwRenderAssets_Release);
	memset(c, 0, offsetof(XwSnapCockpit, glyphs));
}

void XwHud_Copy(XwSnapCockpit* dst, const XwSnapCockpit* src) {
	if (dst == src)
		return;
	References(src, XwRenderAssets_Retain);
	XwHud_Release(dst);
	memcpy(dst, src, offsetof(XwSnapCockpit, glyphs));
	memcpy(dst->glyphs, src->glyphs, src->glyph_count * sizeof src->glyphs[0]);
	memcpy(dst->paint, src->paint, src->paint_count * sizeof src->paint[0]);
	memcpy(dst->sprites, src->sprites, src->sprite_count * sizeof src->sprites[0]);
}

void XwHud_CopyState(XwHudState* dst, const XwHudState* src) {
	if (dst == src)
		return;
	XwHud_Copy(&dst->cockpit, &src->cockpit);
	memcpy(dst->glyph_pane, src->glyph_pane, src->cockpit.glyph_count);
	memcpy(dst->paint_pane, src->paint_pane, src->cockpit.paint_count);
	memcpy(dst->sprite_pane, src->sprite_pane, src->cockpit.sprite_count);
	dst->revision = src->revision;
	dst->order = src->order;
	dst->valid = src->valid;
}

void XwHud_ClearSurface(XwRenderSurface surface) {
	if (surface <= XW_RENDER_SURFACE_NONE || surface >= XW_RENDER_SURFACE_COUNT)
		return;
	XwHud_Release(&owners[surface].cockpit);
	owners[surface].valid = false;
	owners[surface].revision = ++revision;
	owners[surface].order = 0;
}

void XwHud_Reset(void) {
	XwHud_ClearSaved();
	for (unsigned i = 1; i < XW_RENDER_SURFACE_COUNT; ++i)
		XwHud_ClearSurface(i);
	pane = -1;
	reported = false;
}

void XwHud_Enable(bool value) {
	enabled = value;
	pane = -1;
}

int XwHud_Push(int next) {
	int old = pane;
	pane = next;
	return old;
}

void XwHud_Pop(int previous) { pane = previous; }

int XwHud_Pane(void) { return pane; }

XwHudState* XwHud_Working(void) {
	if (!enabled)
		return NULL;
	XwRenderSurface surface = XwFlightTypes_Dos() ? XW_RENDER_SURFACE_DOS : XwDisplay_RenderSurface();
	return surface > XW_RENDER_SURFACE_NONE && surface < XW_RENDER_SURFACE_COUNT ? &owners[surface] : NULL;
}

void XwHud_Changed(XwHudState* state) { state->revision = ++revision; }

void XwHud_Fail(XwHudState* state, const char* reason) {
	state->valid = false;
	if (!reported) {
		Aeron_RequestFatalError("Cockpit Capture Error", reason);
		reported = true;
	}
	XwHud_Changed(state);
}

XwSnapRect XwHud_Clip(void) {
	return (XwSnapRect) { g_flightClipLeft, g_flightClipTop, (int)g_flightClipRight - g_flightClipLeft,
						  (int)g_flightClipBottom - g_flightClipTop };
}

uint16_t XwHud_Color(uint8_t index) {
	return g_flightBytesPerPixel == 2 ? g_flightTextPalette[index] : index;
}

void XwHud_View(unsigned view, const char* base, XwSnapRect aperture, bool mirrored) {
	XwHudState* s = XwHud_Working();
	if (!s)
		return;
	/* The message-strip separator belongs to the retained message pane. */
	int clear_height = g_flightScreenWidth == FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH
						   ? FLIGHT_DISPLAY_LOW_HUD_CLEAR_HEIGHT
						   : FLIGHT_DISPLAY_HIGH_HUD_CLEAR_HEIGHT;
	XwHud_Erase(s, (XwSnapRect) { 0, 0, g_flightScreenWidth, clear_height });
	XwRenderAssets_Release(s->cockpit.definition);
	memset(s->cockpit.widgets, 0, sizeof s->cockpit.widgets);
	memset(&s->cockpit.radar, 0, sizeof s->cockpit.radar);
	XwSnapCockpit* c = &s->cockpit;
	c->definition = XwCockpitAssets_Define(base, aperture);
	XwRenderAssets_Retain(c->definition);
	c->view = view;
	c->hud_state = g_flightCamera.hudStateLive;
	c->screen_width = g_flightScreenWidth;
	c->screen_height = g_flightScreenHeight;
	c->viewport = (XwSnapRect) { g_flightVpX, g_flightVpY, g_flightVpWidth, g_flightVpHeight };
	c->mirrored = mirrored;
	c->suppressed = g_playerFlightState.hudSuppressed;
	c->projection_offset_y = g_projOffsetY;
	c->color_mode = g_flightBytesPerPixel == 2
						? (g_pixelFormatCode == 555 ? XW_SNAP_COLOR_RGB555 : XW_SNAP_COLOR_RGB565)
						: XW_SNAP_COLOR_INDEX8;
	XwCockpitAssets_Palette(c->palette_argb);
	s->valid = c->definition != 0;
	XwHud_Changed(s);
}

void XwHud_ViewSelection(unsigned view) {
	XwHudState* s = XwHud_Working();
	if (!s || !s->cockpit.definition)
		return;
	XwSnapCockpit* c = &s->cockpit;
	if (c->view != view || c->hud_state != g_flightCamera.hudStateLive ||
		c->suppressed != g_playerFlightState.hudSuppressed) {
		c->view = view;
		c->hud_state = g_flightCamera.hudStateLive;
		c->suppressed = g_playerFlightState.hudSuppressed;
		XwHud_Changed(s);
	}
}

void XwHud_Widget(unsigned element, unsigned value, unsigned segments, int step_x, int step_y, unsigned empty,
				  unsigned filled, unsigned kind) {
	XwHudState* s = XwHud_Working();
	if (!s || element >= XW_SNAP_WIDGETS || !s->cockpit.definition)
		return;
	XwSnapWidget w = { .value = value,
					   .segments = segments,
					   .step_x = step_x,
					   .step_y = step_y,
					   .empty_state = empty,
					   .filled_state = filled,
					   .kind = kind,
					   .visible = 1 };
	if (memcmp(&w, &s->cockpit.widgets[element], sizeof w)) {
		s->cockpit.widgets[element] = w;
		XwHud_Changed(s);
	}
}

void XwHud_CopySurface(XwRenderSurface dst, XwRenderSurface src, bool complete) {
	if (dst <= XW_RENDER_SURFACE_NONE || dst >= XW_RENDER_SURFACE_COUNT)
		return;
	if (complete && src > XW_RENDER_SURFACE_NONE && src < XW_RENDER_SURFACE_COUNT)
		XwHud_CopyState(&owners[dst], &owners[src]);
	else
		XwHud_ClearSurface(dst);
}

/* Compact by pane only at publication; per-record order preserves overlap between panes. */
static void Export(XwRenderSnapshot* out, const XwHudState* s) {
	XwHud_Copy(&out->cockpit, &s->cockpit);
	XwSnapCockpit* c = &out->cockpit;
	unsigned g = 0, p = 0, b = 0;
	for (unsigned pane_index = 0; pane_index < XW_SNAP_HUD_PANES; ++pane_index) {
		XwSnapHudPane* pane_out = &c->panes[pane_index];
		pane_out->glyphs.first = g;
		pane_out->paint.first = p;
		pane_out->sprites.first = b;
		for (unsigned i = 0; i < s->cockpit.glyph_count; ++i)
			if (s->glyph_pane[i] == pane_index)
				c->glyphs[g++] = s->cockpit.glyphs[i];
		for (unsigned i = 0; i < s->cockpit.paint_count; ++i)
			if (s->paint_pane[i] == pane_index)
				c->paint[p++] = s->cockpit.paint[i];
		for (unsigned i = 0; i < s->cockpit.sprite_count; ++i)
			if (s->sprite_pane[i] == pane_index)
				c->sprites[b++] = s->cockpit.sprites[i];
		pane_out->glyphs.count = g - pane_out->glyphs.first;
		pane_out->paint.count = p - pane_out->paint.first;
		pane_out->sprites.count = b - pane_out->sprites.first;
		pane_out->visible = !!(pane_out->glyphs.count + pane_out->paint.count + pane_out->sprites.count);
		pane_out->clip = (XwSnapRect) { 0, 0, c->screen_width, c->screen_height };
	}
	out->hud_valid = s->valid;
	out->key.hud_revision = s->revision;
}

void XwHud_Present(XwRenderSnapshot* out, XwRenderSurface surface, bool flip) {
	Export(out, &owners[surface]);
	if (surface == XW_RENDER_SURFACE_BACK) {
		if (flip) {
			XwHudState swap = owners[XW_RENDER_SURFACE_FRONT];
			owners[XW_RENDER_SURFACE_FRONT] = owners[surface];
			owners[surface] = swap;
		} else
			XwHud_CopyState(&owners[XW_RENDER_SURFACE_FRONT], &owners[surface]);
	}
}

void XwHud_IndexedPalette(const uint32_t colors[256]) {
	if (!enabled)
		return;
	for (unsigned i = XW_RENDER_SURFACE_FRONT; i < XW_RENDER_SURFACE_DOS; ++i) {
		XwHudState* s = &owners[i];
		if (s->cockpit.definition && s->cockpit.color_mode == XW_SNAP_COLOR_INDEX8 &&
			memcmp(s->cockpit.palette_argb, colors, sizeof s->cockpit.palette_argb)) {
			memcpy(s->cockpit.palette_argb, colors, sizeof s->cockpit.palette_argb);
			XwHud_Changed(s);
		}
	}
}

void XwHud_ExportDirect(XwRenderSnapshot* out) {
	if (!enabled || !out->world_valid || XwRenderCapture_HasPendingView())
		return;
	/* SCREEN writes and indexed FRONT palette changes can affect the visible image between flips. */
	const XwHudState* s =
		&owners[out->flight_version == 98 ? XW_RENDER_SURFACE_FRONT : XW_RENDER_SURFACE_DOS];
	if (s->revision != out->key.hud_revision)
		Export(out, s);
}
