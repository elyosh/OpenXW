#include "xw_remaster/hud_draw.h"
#include <math.h>

void XwHudDraw_Color(const XwHudDraw* draw, unsigned color, bool indexed, float rgba[4]) {
	const XwRenderSnapshot* s = draw->snapshot;
	uint32_t argb;
	if (indexed || s->cockpit.color_mode == XW_SNAP_COLOR_INDEX8) {
		argb = s->flight_version == 98 ? s->cockpit.palette_argb[color & 255]
									   : s->appearance.palette_argb[color & 255];
	} else {
		unsigned bits = s->cockpit.color_mode == XW_SNAP_COLOR_RGB555 ? 5 : 6;
		unsigned r = ((color >> (bits + 5)) & 31) * 255 / 31,
				 g = ((color >> 5) & ((1u << bits) - 1)) * 255 / ((1u << bits) - 1),
				 b = (color & 31) * 255 / 31;
		argb = 0xff000000u | (r << 16) | (g << 8) | b;
	}
	for (unsigned i = 0; i < 3; ++i) {
		float v = ((argb >> (16 - 8 * i)) & 255) / 255.f;
		rgba[i] = v <= 0.04045f ? v / 12.92f : powf((v + 0.055f) / 1.055f, 2.4f);
	}
	rgba[3] = 1;
}

static bool Clip(const XwHudDraw* d, XwSnapRect rect, AeronRectI* out) {
	if (rect.width <= 0 || rect.height <= 0)
		return false;
	/* Clamp rounded clipping bounds to the target while preserving fractional artwork positions. */
	float width = d->layout.target_width, height = d->layout.target_height;
	int x = (int)fminf(width, fmaxf(0, floorf(d->offset_x + rect.x * d->layout.scale_x))),
		y = (int)fminf(height, fmaxf(0, floorf(d->offset_y + rect.y * d->layout.scale_y)));
	int right = (int)fminf(width,
						   fmaxf(0, ceilf(d->offset_x + ((float)rect.x + rect.width) * d->layout.scale_x))),
		bottom = (int)fminf(height,
							fmaxf(0, ceilf(d->offset_y + ((float)rect.y + rect.height) * d->layout.scale_y)));
	*out = (AeronRectI) { x, y, right - x, bottom - y };
	return right > x && bottom > y;
}

void XwHudDraw_Fill(const XwHudDraw* d, XwSnapRect rect, XwSnapRect clip, unsigned color, bool indexed) {
	if (rect.width <= 0 || rect.height <= 0 || clip.width <= 0 || clip.height <= 0)
		return;
	float rgba[4];
	XwHudDraw_Color(d, color, indexed, rgba);
	AeronRectI scissor;
	if (!Clip(d, clip, &scissor))
		return;
	AeronDrawList_AddFill(d->list, d->offset_x + rect.x * d->layout.scale_x,
						  d->offset_y + rect.y * d->layout.scale_y, rect.width * d->layout.scale_x,
						  rect.height * d->layout.scale_y, rgba, AERON_BLIT2D_BLEND_PMA, &scissor);
}

static bool Sprite(const XwHudDraw* d, const XwHudImage* image, XwSnapRect source, XwSnapRect rect,
				   XwSnapRect clip, bool mirrored, const float* tint) {
	if (!image)
		return false;
	AeronRectI scissor;
	/* A zero scissor means unrestricted drawing to Aeron, so omit empty clips. */
	if (!Clip(d, clip, &scissor))
		return true;
	const AeronRuntimeAtlas* a = XwHudAssets_Atlas(image);
	if (!a)
		return false;
	unsigned index = image->atlas_frame;
	if (index >= (unsigned)a->layout.frame_count)
		return false;
	unsigned page = a->layout.pages ? a->layout.pages[index] : 0;
	AeronSpriteRect frame = a->layout.frames[index];
	AeronRuntimeAtlasPage p = a->pages[page];
	float u0 = (frame.x + source.x) / p.width, u1 = (frame.x + source.x + source.width) / p.width;
	AeronDrawList2DSprite sprite = { .texture = p.texture,
									 .src_u0 = mirrored ? u1 : u0,
									 .src_v0 = (frame.y + source.y) / p.height,
									 .src_u1 = mirrored ? u0 : u1,
									 .src_v1 = (frame.y + source.y + source.height) / p.height,
									 .dst_x = d->offset_x + rect.x * d->layout.scale_x,
									 .dst_y = d->offset_y + rect.y * d->layout.scale_y,
									 .dst_w = rect.width * d->layout.scale_x,
									 .dst_h = rect.height * d->layout.scale_y,
									 .tint = { 1, 1, 1, 1 },
									 .blend = AERON_BLIT2D_BLEND_PMA,
									 .filter = AERON_BLIT2D_FILTER_NEAREST,
									 .scissor = scissor };
	if (tint)
		for (unsigned i = 0; i < 4; ++i)
			sprite.tint[i] = tint[i];
	AeronDrawList_AddSprite(d->list, &sprite);
	return true;
}

bool XwHudDraw_Image(const XwHudDraw* d, XwHudImageKey key, XwSnapRect rect, XwSnapRect clip, bool mirrored) {
	const XwHudImage* image = XwHudAssets_Image(key);
	if (!image)
		return false;
	return Sprite(d, image, (XwSnapRect) { 0, 0, image->bitmap.width, image->bitmap.height }, rect, clip,
				  mirrored, NULL);
}

bool XwHudDraw_Glyph(const XwHudDraw* d, const XwSnapGlyph* g) {
	const XwHudImage* image =
		XwHudAssets_Image((XwHudImageKey) { .source = g->font, .kind = XW_HUD_IMAGE_FONT });
	if (!image || g->character < image->font.first_char ||
		g->character - image->font.first_char >= image->font.glyph_count)
		return false;
	const AeronDecodedGlyph* glyph = &image->font.glyphs[g->character - image->font.first_char];
	XwSnapRect rect = { g->x, g->y, g->advance + (g->shadow_enabled != 0), g->height };
	if (g->background_enabled)
		XwHudDraw_Fill(d, rect, g->clip, g->background, false);
	float rgba[4];
	XwSnapRect source = { glyph->x, glyph->y, rect.width, rect.height };
	if (g->shadow_enabled) {
		source.y += image->font.height;
		XwHudDraw_Color(d, g->shadow, false, rgba);
		if (!Sprite(d, image, source, rect, g->clip, false, rgba))
			return false;
		source.y -= image->font.height;
	}
	XwHudDraw_Color(d, g->foreground, false, rgba);
	return Sprite(d, image, source, rect, g->clip, false, rgba);
}
