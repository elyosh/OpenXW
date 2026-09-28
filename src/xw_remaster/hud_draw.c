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

/* OpenXvT clips geometry/UVs at integer scissor bounds so adjacent records can batch. */
static bool ClipSprite(AeronDrawList2DSprite* sprite, const AeronRectI* clip) {
	float left = fmaxf(sprite->dst_x, clip->x);
	float top = fmaxf(sprite->dst_y, clip->y);
	float right = fminf(sprite->dst_x + sprite->dst_w, clip->x + clip->width);
	float bottom = fminf(sprite->dst_y + sprite->dst_h, clip->y + clip->height);
	if (right <= left || bottom <= top)
		return false;
	float du = (sprite->src_u1 - sprite->src_u0) / sprite->dst_w;
	float dv = (sprite->src_v1 - sprite->src_v0) / sprite->dst_h;
	sprite->src_u1 = sprite->src_u0 + (right - sprite->dst_x) * du;
	sprite->src_v1 = sprite->src_v0 + (bottom - sprite->dst_y) * dv;
	sprite->src_u0 += (left - sprite->dst_x) * du;
	sprite->src_v0 += (top - sprite->dst_y) * dv;
	sprite->dst_x = left;
	sprite->dst_y = top;
	sprite->dst_w = right - left;
	sprite->dst_h = bottom - top;
	return true;
}

void XwHudDraw_Fill(const XwHudDraw* d, XwSnapRect rect, XwSnapRect clip, unsigned color, bool indexed) {
	if (rect.width <= 0 || rect.height <= 0 || clip.width <= 0 || clip.height <= 0)
		return;
	float rgba[4];
	XwHudDraw_Color(d, color, indexed, rgba);
	AeronRectI scissor;
	if (!Clip(d, clip, &scissor))
		return;
	AeronDrawList2DSprite sprite = { .dst_x = d->offset_x + rect.x * d->layout.scale_x,
									 .dst_y = d->offset_y + rect.y * d->layout.scale_y,
									 .dst_w = rect.width * d->layout.scale_x,
									 .dst_h = rect.height * d->layout.scale_y };
	if (ClipSprite(&sprite, &scissor))
		AeronDrawList_AddFill(d->list, sprite.dst_x, sprite.dst_y, sprite.dst_w, sprite.dst_h, rgba,
							  AERON_BLIT2D_BLEND_PMA, NULL);
}

static bool Sprite(const XwHudDraw* d, const XwHudImage* image, AeronSpriteRect source, XwSnapRect rect,
				   XwSnapRect clip, bool mirrored, const float* tint) {
	if (!image)
		return false;
	AeronRectI scissor;
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
	float u0 = (frame.x + source.x) / p.width, u1 = (frame.x + source.x + source.w) / p.width;
	AeronDrawList2DSprite sprite = { .texture = p.texture,
									 .src_u0 = mirrored ? u1 : u0,
									 .src_v0 = (frame.y + source.y) / p.height,
									 .src_u1 = mirrored ? u0 : u1,
									 .src_v1 = (frame.y + source.y + source.h) / p.height,
									 .dst_x = d->offset_x + rect.x * d->layout.scale_x,
									 .dst_y = d->offset_y + rect.y * d->layout.scale_y,
									 .dst_w = rect.width * d->layout.scale_x,
									 .dst_h = rect.height * d->layout.scale_y,
									 .tint = { 1, 1, 1, 1 },
									 .blend = AERON_BLIT2D_BLEND_PMA,
									 .filter = AERON_BLIT2D_FILTER_NEAREST };
	if (tint)
		for (unsigned i = 0; i < 4; ++i)
			sprite.tint[i] = tint[i];
	if (ClipSprite(&sprite, &scissor))
		AeronDrawList_AddSprite(d->list, &sprite);
	return true;
}

bool XwHudDraw_Image(const XwHudDraw* d, XwHudImageKey key, XwSnapRect rect, XwSnapRect clip, bool mirrored) {
	const XwHudImage* image = XwHudAssets_Image(key);
	if (!image)
		return false;
	return Sprite(d, image, (AeronSpriteRect) { 0, 0, image->bitmap.width, image->bitmap.height }, rect, clip,
				  mirrored, NULL);
}

bool XwHudDraw_Base(const XwHudDraw* d) {
	if (!d->definition->base)
		return true;
	const XwHudImage* image =
		XwHudAssets_Image((XwHudImageKey) { .source = d->definition->base, .kind = XW_HUD_IMAGE_BASE });
	const AeronImageCoverage* coverage = XwHudAssets_BaseCoverage();
	if (!image || !coverage)
		return false;
	bool mirrored = d->snapshot->cockpit.mirrored;
	XwSnapRect full = { 0, 0, d->definition->width, d->definition->height };
	/* OpenXvT's coverage rectangles share one texture and batch into one draw. */
	for (unsigned i = 0; i < coverage->count; ++i) {
		const AeronImageCoverageRect* r = &coverage->rects[i];
		int x = mirrored ? d->definition->width - r->x - r->width : r->x;
		if (!Sprite(d, image, (AeronSpriteRect) { r->x, r->y, r->width, r->height },
					(XwSnapRect) { x, r->y, r->width, r->height }, full, mirrored, NULL))
			return false;
	}
	return true;
}

bool XwHudDraw_Glyph(const XwHudDraw* d, const XwSnapGlyph* g) {
	const XwHudImage* image =
		XwHudAssets_Image((XwHudImageKey) { .source = g->font, .kind = XW_HUD_IMAGE_FONT });
	if (!image || g->character < image->font.first_char ||
		g->character - image->font.first_char >= image->font.glyph_count)
		return false;
	const AeronDecodedGlyph* glyph = &image->font.glyphs[g->character - image->font.first_char];
	XwSnapRect rect = { g->x, g->y, g->advance + (g->shadow_enabled != 0), g->height };
	float rgba[4];
	if (g->background_enabled) {
		XwHudDraw_Color(d, g->background, false, rgba);
		/* Sample the font's solid strip, keeping background/shadow/foreground in one batch. */
		AeronSpriteRect white = { .x = 0.5f, .y = image->font.height * 2 + 1.5f };
		if (!Sprite(d, image, white, rect, g->clip, false, rgba))
			return false;
	}
	AeronSpriteRect source = { glyph->x, glyph->y, rect.width, rect.height };
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
