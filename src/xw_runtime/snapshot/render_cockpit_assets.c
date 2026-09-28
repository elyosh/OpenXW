#include "xw_runtime/snapshot/render_cockpit_assets.h"
#include "xw/flight/feinput.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/hud/panel.h"
#include "xw/flight/player/user.h"
#include "xw/render/flight_palette.h"
#include "xw/render/rtsrgb.h"
#include "xw/render/rtsvga2.h"
#include "xw_runtime/runtime/flight_types.h"
#include "xw_runtime/runtime/profile.h"
#include <aeron/asset/lfd.h>
#include <aeron/asset/pnl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static XwCockpitPart parts[XW_SNAP_PANEL_SPRITES];
static XwRenderAssetId definition;

const XwCockpitDefinition* XwCockpitAssets_Definition(XwRenderAssetId id) {
	const XwRenderSource* s = XwRenderAssets_Source(id);
	return s && s->kind == XW_SOURCE_COCKPIT_DEFINITION && s->size == sizeof(XwCockpitDefinition) ? s->data
																								  : NULL;
}

static void References(const XwCockpitDefinition* d, void (*operation)(XwRenderAssetId)) {
	operation(d->base);
	for (unsigned i = 0; i < XW_SNAP_COCKPIT_VIEWS; ++i)
		operation(d->views[i].image);
	for (unsigned i = 0; i < 2; ++i)
		operation(d->fonts[i]);
	for (unsigned i = 0; i < XW_SNAP_PANEL_SPRITES; ++i)
		operation(d->parts[i].source);
}

static void Destroy(void* data) {
	References(data, XwRenderAssets_Release);
	free(data);
}

void XwCockpitAssets_Reset(void) {
	XwRenderAssets_Release(definition);
	definition = 0;
	for (unsigned i = 0; i < XW_SNAP_PANEL_SPRITES; ++i)
		XwRenderAssets_Release(parts[i].source);
	memset(parts, 0, sizeof parts);
}

void XwCockpitAssets_Panel(XwRenderAssetId source, unsigned first, unsigned count, unsigned skip) {
	if (first > XW_SNAP_PANEL_SPRITES || count > XW_SNAP_PANEL_SPRITES - first || skip > UINT16_MAX - count) {
		XwRenderAssets_Fail("cockpit parts", "invalid sprite binding range");
		return;
	}
	const XwRenderSource* bytes = XwRenderAssets_Source(source);
	AeronPnlList list = { 0 };
	if (!bytes || !AeronPnl_Parse(bytes->data, bytes->size, skip + count, &list, NULL) ||
		list.count < skip + count) {
		AeronPnl_Free(&list);
		XwRenderAssets_Fail("cockpit parts", "invalid source records");
		return;
	}
	for (unsigned i = 0; i < count; ++i) {
		XwRenderAssets_Retain(source);
		XwRenderAssets_Release(parts[first + i].source);
		AeronByteSpan span = list.bitmaps[skip + i];
		int width = 0, height = 0;
		size_t consumed = 0;
		bool empty = span.size == 1 && span.data[0] == 255;
		if (!empty && !AeronPnl_Measure(span.data, span.size, &width, &height, &consumed, NULL))
			XwRenderAssets_Fail("cockpit part", "invalid dimensions");
		parts[first + i] =
			(XwCockpitPart) { source, (uint16_t)(skip + i), (uint16_t)width, (uint16_t)height };
	}
	AeronPnl_Free(&list);
}

static uint32_t DirectColor(uint16_t color) {
	unsigned bits = g_pixelFormatCode == 555 ? 5 : 6;
	unsigned r = ((color >> (bits + 5)) & 31) * 255 / 31;
	unsigned g = ((color >> 5) & ((1u << bits) - 1)) * 255 / ((1u << bits) - 1);
	unsigned b = (color & 31) * 255 / 31;
	return 0xff000000u | r << 16 | g << 8 | b;
}

void XwCockpitAssets_Palette(uint32_t colors[256]) {
	for (unsigned i = 0; i < 256; ++i)
		colors[i] = g_flightBytesPerPixel == 2
						? DirectColor(g_flightTextPalette[i])
						: 0xff000000u | (uint32_t)(g_swPalette[i].r * 4) << 16 |
							  (uint32_t)(g_swPalette[i].g * 4) << 8 | (uint32_t)(g_swPalette[i].b * 4);
}

static bool ViewPalette(XwCockpitView* view) {
	const XwRenderSource* source = XwRenderAssets_Source(view->image);
	AeronLfd lfd = { 0 };
	if (!source || !AeronLfd_Parse(source->data, source->size, &lfd, NULL))
		return false;
	const AeronLfdEntry* pltt = AeronLfd_Find(&lfd, AERON_LFD_FOURCC('P', 'L', 'T', 'T'));
	bool ok = pltt && pltt->size >= 194;
	if (ok) {
		RgbTriplet rgb[64];
		uint16_t direct[64];
		for (unsigned i = 0; i < 64; ++i)
			rgb[i] = (RgbTriplet) { pltt->data[2 + i * 3] >> 2, pltt->data[3 + i * 3] >> 2,
									pltt->data[4 + i * 3] >> 2 };
		/* Pure original conversion: writes only this local destination. */
		if (g_flightBytesPerPixel == 2)
			FlightPalette_Build16BppRange(rgb, direct, 0, 64);
		for (unsigned i = 0; i < 64; ++i)
			view->palette_argb[i] = g_flightBytesPerPixel == 2
										? DirectColor(direct[i])
										: 0xff000000u | (uint32_t)(rgb[i].r * 4) << 16 |
											  (uint32_t)(rgb[i].g * 4) << 8 | (uint32_t)(rgb[i].b * 4);
	}
	AeronLfd_Free(&lfd);
	return ok;
}

XwRenderAssetId XwCockpitAssets_Define(const char* base, XwSnapRect aperture) {
	XwCockpitDefinition d = { 0 };
	d.width = g_flightScreenWidth;
	d.height = g_flightScreenHeight;
	d.version = XwGameVersion_Year(XwProfile_ActiveFlight()->version) - 1900;
	d.aperture = aperture;
	d.color_mode = g_flightBytesPerPixel == 2
					   ? (g_pixelFormatCode == 555 ? XW_SNAP_COLOR_RGB555 : XW_SNAP_COLOR_RGB565)
					   : XW_SNAP_COLOR_INDEX8;
	d.brightness_q8 = g_flightBrightnessScaleQ8;
	XwCockpitAssets_Palette(d.palette_argb);
	d.base = base ? XwRenderAssets_Find(XW_SOURCE_COCKPIT, base) : 0;
	d.fonts[0] = XwRenderAssets_Find(XW_SOURCE_FONT, d.width == 320 ? "TINY.FNT" : "TINY64.FNT");
	d.fonts[1] = XwRenderAssets_Find(XW_SOURCE_FONT, d.width == 320 ? "MICRO.FNT" : "MICRO64.FNT");
	if ((base && !d.base) || !d.fonts[0] || !d.fonts[1]) {
		XwRenderAssets_Fail("cockpit", "missing registered art or font");
		return 0;
	}
	memcpy(d.parts, parts, sizeof parts);
	for (unsigned i = 0; i < XW_SNAP_COCKPIT_VIEWS; ++i) {
		const HudCockpitResourceDescriptor* v = &g_hudCockpitResourceDescriptors[i];
		unsigned resource = i == 18             ? 18
							: v->enabled >= 192 ? v->enabled - 192
							: v->enabled >= 128 ? v->enabled - 128
												: i;
		if (resource >= XW_SNAP_COCKPIT_VIEWS) {
			XwRenderAssets_Fail("cockpit", "invalid view alias");
			return 0;
		}
		XwCockpitView* out = &d.views[i];
		out->resource = resource;
		out->mirrored = v->enabled >= 192;
		out->aperture = (XwSnapRect) { v->viewportX, v->viewportY, v->viewportWidth, v->viewportHeight };
		memcpy(out->label, v->viewLabel, sizeof out->label);
		out->label[sizeof out->label - 1] = 0;
		char name[16];
		snprintf(name, sizeof name, "%.8s.LFD", g_hudCockpitResourceDescriptors[resource].lfdName);
		if (v->enabled && i != 18)
			out->image = XwRenderAssets_Find(XW_SOURCE_COCKPIT, name);
		if (out->image && !ViewPalette(out)) {
			XwRenderAssets_Fail(name, "invalid resident cockpit palette");
			return 0;
		}
	}
	for (unsigned i = 0; i < XW_SNAP_WIDGETS; ++i) {
		const HudElementLayout* e = &g_hudElementLayouts[i];
		d.elements[i] = (XwCockpitElement) { e->x, e->y, e->spriteIndex, e->selector };
	}
	const XwCockpitDefinition* old = XwCockpitAssets_Definition(definition);
	if (old && !memcmp(old, &d, sizeof d))
		return definition;
	XwCockpitDefinition* owned = malloc(sizeof d);
	if (!owned) {
		XwRenderAssets_Fail("cockpit", "definition allocation failed");
		return 0;
	}
	*owned = d;
	References(owned, XwRenderAssets_Retain);
	XwRenderAssetId next = XwRenderAssets_RegisterOwned(XW_SOURCE_COCKPIT_DEFINITION, "flight/cockpit",
														d.version, owned, sizeof d, Destroy);
	XwRenderAssets_Retain(next);
	XwRenderAssets_Release(definition);
	definition = next;
	return next;
}

XwRenderAssetId XwCockpitAssets_Loaded(void) { return definition; }
