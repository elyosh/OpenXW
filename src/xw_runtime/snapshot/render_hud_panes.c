#include "xw/flight/feinput.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/hud/festring.h"
#include "xw/flight/hud/panel.h"
#include "xw_runtime/snapshot/render_hud_internal.h"
#include <string.h>

static XwSnapRect Intersect(XwSnapRect a, XwSnapRect b) {
	int right = a.x + a.width < b.x + b.width ? a.x + a.width : b.x + b.width;
	int bottom = a.y + a.height < b.y + b.height ? a.y + a.height : b.y + b.height;
	if (a.x < b.x)
		a.x = b.x;
	if (a.y < b.y)
		a.y = b.y;
	a.width = right - a.x;
	a.height = bottom - a.y;
	if (a.width < 0)
		a.width = 0;
	if (a.height < 0)
		a.height = 0;
	return a;
}

/* Retained pane paint is bounded visible content, never an accumulating frame command log. */
static unsigned Subtract(XwSnapRect a, XwSnapRect erase, XwSnapRect result[4]) {
	if (a.width <= 0 || a.height <= 0)
		return 0;
	if (erase.width <= 0 || erase.height <= 0) {
		result[0] = a;
		return 1;
	}
	XwSnapRect cut = Intersect(a, erase);
	if (!cut.width || !cut.height) {
		result[0] = a;
		return 1;
	}
	unsigned n = 0;
	if (cut.y > a.y)
		result[n++] = (XwSnapRect) { a.x, a.y, a.width, cut.y - a.y };
	if (cut.y + cut.height < a.y + a.height)
		result[n++] = (XwSnapRect) { a.x, cut.y + cut.height, a.width, a.y + a.height - cut.y - cut.height };
	if (cut.x > a.x)
		result[n++] = (XwSnapRect) { a.x, cut.y, cut.x - a.x, cut.height };
	if (cut.x + cut.width < a.x + a.width)
		result[n++] =
			(XwSnapRect) { cut.x + cut.width, cut.y, a.x + a.width - cut.x - cut.width, cut.height };
	return n;
}

/* Each fragment keeps its original order and resource identity. */
#define ERASE_RECORDS(member, count, tags, capacity, asset)                                                  \
	do {                                                                                                     \
		unsigned original = c->count;                                                                        \
		for (unsigned i = 0; i < original;) {                                                                \
			XwSnapRect pieces[4];                                                                            \
			unsigned n = Subtract(c->member[i].clip, rect, pieces);                                          \
			if (!n) {                                                                                        \
				XwRenderAssets_Release(asset);                                                               \
				--c->count;                                                                                  \
				--original;                                                                                  \
				memmove(c->member + i, c->member + i + 1, (c->count - i) * sizeof c->member[0]);             \
				memmove(s->tags + i, s->tags + i + 1, c->count - i);                                         \
				continue;                                                                                    \
			}                                                                                                \
			if (c->count + n - 1 > capacity) {                                                               \
				XwHud_Fail(s, "pane fragment capacity exceeded");                                            \
				return;                                                                                      \
			}                                                                                                \
			c->member[i].clip = pieces[0];                                                                   \
			for (unsigned j = 1; j < n; ++j) {                                                               \
				unsigned at = c->count++;                                                                    \
				c->member[at] = c->member[i];                                                                \
				c->member[at].clip = pieces[j];                                                              \
				s->tags[at] = s->tags[i];                                                                    \
				XwRenderAssets_Retain(asset);                                                                \
			}                                                                                                \
			++i;                                                                                             \
		}                                                                                                    \
	} while (0)

static void EraseGlyphs(XwHudState* s, XwSnapRect rect) {
	XwSnapCockpit* c = &s->cockpit;
	ERASE_RECORDS(glyphs, glyph_count, glyph_pane, XW_SNAP_HUD_GLYPHS, c->glyphs[i].font);
}

void XwHud_Erase(XwHudState* s, XwSnapRect rect) {
	XwSnapCockpit* c = &s->cockpit;
	ERASE_RECORDS(glyphs, glyph_count, glyph_pane, XW_SNAP_HUD_GLYPHS, c->glyphs[i].font);
	ERASE_RECORDS(paint, paint_count, paint_pane, XW_SNAP_HUD_PAINT, 0);
	ERASE_RECORDS(sprites, sprite_count, sprite_pane, XW_SNAP_HUD_SPRITES, c->sprites[i].image);
}

#undef ERASE_RECORDS

static XwHudState* PaneState(void) {
	XwHudState* s = XwHud_Working();
	return s && XwHud_Pane() >= 0 && XwHud_Pane() < XW_SNAP_HUD_PANES ? s : NULL;
}

static unsigned PaneForRect(XwSnapRect rect) {
	unsigned pane = XwHud_Pane();
	if (pane == XW_SNAP_PANE_TARGET_BODY) {
		int y = g_hudElementLayouts[PANEL_RADAR_TARGET_ELEMENT].y;
		int title = g_flightScreenWidth == 320 ? 6 : 11, height = g_flightScreenWidth == 320 ? 38 : 91;
		if (rect.y >= y + height)
			return XW_SNAP_PANE_TARGET_FOOTER;
		if (rect.y + rect.height <= y + title)
			return XW_SNAP_PANE_TARGET_HEADER;
	}
	return pane;
}

void XwHud_Glyph(unsigned character, unsigned width, unsigned height) {
	XwHudState* s = PaneState();
	if (!s)
		return;
	const XwCockpitDefinition* d = XwCockpitAssets_Definition(s->cockpit.definition);
	if (!d)
		return;
	if (g_flightFontTier < 1 || g_flightFontTier > 2) {
		XwHud_Fail(s, "unregistered flight font");
		return;
	}
	XwSnapRect rect = { g_flightCursorX, g_flightCursorY, width + (g_flightTextShadowEnabled != 0), height };
	/* DOS has already checked wrapping; accepted glyphs draw outside the text bounds. */
	XwSnapRect bounds = d->version == 98 ? XwHud_Clip() : (XwSnapRect) { 0, 0, d->width, d->height };
	XwSnapRect clip = Intersect(rect, bounds);
	if (!clip.width || !clip.height)
		return;
	EraseGlyphs(s, clip);
	XwSnapCockpit* c = &s->cockpit;
	if (c->glyph_count == XW_SNAP_HUD_GLYPHS) {
		XwHud_Fail(s, "glyph capacity exceeded");
		return;
	}
	unsigned i = c->glyph_count++;
	c->glyphs[i] = (XwSnapGlyph) { .font = d->fonts[g_flightFontTier - 1],
								   .clip = clip,
								   .x = rect.x,
								   .y = rect.y,
								   .character = character,
								   .advance = width,
								   .height = height,
								   .order = ++s->order,
								   .foreground = XwHud_Color(g_flightTextColorIndex),
								   .background = XwHud_Color(g_flightTextBgColor),
								   .shadow = XwHud_Color(g_flightTextShadowColor),
								   .background_enabled = 1,
								   .shadow_enabled = g_flightTextShadowEnabled != 0 };
	s->glyph_pane[i] = PaneForRect(rect);
	XwRenderAssets_Retain(c->glyphs[i].font);
	XwHud_Changed(s);
}

void XwHud_Fill(int x0, int y0, int x1, int y1) {
	XwHudState* s = PaneState();
	if (!s || x1 <= x0 || y1 <= y0)
		return;
	XwSnapRect rect = { x0, y0, x1 - x0, y1 - y0 };
	XwHud_Erase(s, rect);
	XwSnapCockpit* c = &s->cockpit;
	if (c->paint_count == XW_SNAP_HUD_PAINT) {
		XwHud_Fail(s, "paint capacity exceeded");
		return;
	}
	unsigned i = c->paint_count++;
	c->paint[i] = (XwSnapHudPaint) { .clip = rect,
									 .x0 = x0,
									 .y0 = y0,
									 .x1 = x1,
									 .y1 = y1,
									 .order = ++s->order,
									 .color = XwHud_Color(g_flightTextBgColor),
									 .kind = XW_SNAP_PAINT_FILL };
	s->paint_pane[i] = PaneForRect(rect);
	XwHud_Changed(s);
}

void XwHud_Sprite(const uint8_t* source, int x, int y, int transparent, int mirror) {
	XwHudState* s = PaneState();
	if (!s)
		return;
	const XwCockpitDefinition* d = XwCockpitAssets_Definition(s->cockpit.definition);
	if (!d)
		return;
	unsigned index = 0;
	while (index < XW_SNAP_PANEL_SPRITES && g_hudPanelSpriteDataByIndex[index] != source)
		++index;
	if (index == XW_SNAP_PANEL_SPRITES)
		return; /* Base artwork has its own authored mask. */
	XwCockpitPart part = d->parts[index];
	int width = part.width, height = part.height;
	if (!part.source) {
		XwHud_Fail(s, "unregistered panel sprite");
		return;
	}
	if (!width || !height)
		return;
	XwSnapRect rect = { mirror ? x - width + 1 : x, y, width, height };
	XwSnapCockpit* c = &s->cockpit;
	/* Sprite state replacement retains transparency over the original pane background. */
	for (unsigned i = 0; i < c->sprite_count; ++i)
		if (c->sprites[i].destination.x == rect.x && c->sprites[i].destination.y == rect.y &&
			s->sprite_pane[i] == PaneForRect(rect)) {
			XwRenderAssets_Release(c->sprites[i].image);
			--c->sprite_count;
			memmove(c->sprites + i, c->sprites + i + 1, (c->sprite_count - i) * sizeof c->sprites[0]);
			memmove(s->sprite_pane + i, s->sprite_pane + i + 1, c->sprite_count - i);
			--i;
		}
	if (c->sprite_count == XW_SNAP_HUD_SPRITES) {
		XwHud_Fail(s, "sprite capacity exceeded");
		return;
	}
	unsigned i = c->sprite_count++;
	c->sprites[i] = (XwSnapHudSprite) { .image = part.source,
										.frame = part.frame,
										.order = ++s->order,
										.destination = rect,
										.clip = rect,
										.transparent_color = (uint8_t)transparent,
										.mirrored = mirror != 0 };
	s->sprite_pane[i] = PaneForRect(rect);
	XwRenderAssets_Retain(part.source);
	XwHud_Changed(s);
}
