/* Original PNL/LFD/font decoding follows OpenXvT; aperture run rules are version-specific. */
#include "xw_remaster/hud_decode.h"
#include "aeron/asset/bitmap_font.h"
#include "aeron/asset/lfd.h"
#include "aeron/asset/pnl.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool Error(AeronDecodeError* error, const char* format, ...) {
	if (error) {
		error->code = 1;
		va_list args;
		va_start(args, format);
		vsnprintf(error->message, sizeof error->message, format, args);
		va_end(args);
	}
	return false;
}

static int MaskRun(const uint8_t** cursor, const uint8_t* end, const XwCockpitDefinition* d) {
	if (*cursor == end)
		return -1;
	unsigned n = *(*cursor)++;
	if (n)
		return n;
	if (*cursor == end)
		return -1;
	unsigned extension = *(*cursor)++;
	if (d->version != 98)
		return 256 + extension;
	if (d->width == 320)
		return 255 + (uint8_t)(extension + 1);
	if (extension == 255)
		return 511;
	if (extension)
		return 256 + extension;
	if (*cursor == end)
		return -1;
	unsigned last = *(*cursor)++;
	return 511 + (uint8_t)(last + 1);
}

static bool Mask(AeronIndexedFrame* frame, const AeronLfdEntry* mask, const XwCockpitDefinition* d,
				 AeronDecodeError* error) {
	XwSnapRect rect = d->aperture;
	if (rect.x < 0 || rect.y < 0 || rect.width < 0 || rect.height < 0 || rect.x + rect.width > d->width ||
		rect.y + rect.height > d->height)
		return Error(error, "Cockpit aperture %d,%d %dx%d exceeds %ux%u", rect.x, rect.y, rect.width,
					 rect.height, d->width, d->height);
	const uint8_t *p = mask->data, *end = p + mask->size;
	for (int y = 0; y < rect.height; ++y) {
		if (p == end)
			return Error(error, "MASK ends before row %d of %d", y, rect.height);
		int8_t state = (int8_t)*p++;
		for (int x = 0; x < rect.width;) {
			int n = MaskRun(&p, end, d);
			if (n <= 0 || (d->version != 98 && n > rect.width - x))
				return Error(error, "Invalid MASK run at row %d, column %d (length %d, width %d)", y, x, n,
							 rect.width);
			/* Windows panel_copymaskdata accepts a final run past the viewport.
			 * Like OpenXvT, clip coverage without consuming another source run. */
			if (n > rect.width - x)
				n = rect.width - x;
			if (state >= 0)
				memset(frame->coverage + (size_t)(rect.y + y) * frame->width + rect.x + x, 0, n);
			x += n;
			state = (int8_t)-state;
		}
	}
	return true;
}

static bool Base(XwHudImage* image, const XwRenderSource* source, const XwCockpitDefinition* d,
				 AeronDecodeError* error) {
	AeronLfd lfd = { 0 };
	if (!AeronLfd_Parse(source->data, source->size, &lfd, error))
		return false;
	const AeronLfdEntry* panl = AeronLfd_Find(&lfd, AERON_LFD_FOURCC('P', 'A', 'N', 'L'));
	const AeronLfdEntry* mask = AeronLfd_Find(&lfd, AERON_LFD_FOURCC('M', 'A', 'S', 'K'));
	AeronIndexedFrame decoded = { 0 };
	bool ok = panl && mask;
	if (!ok)
		Error(error, "Missing or duplicate PANL/MASK entry");
	else
		ok = AeronPnl_DecodeIndexed(panl->data, panl->size, 0, -1, &decoded, error);
	if (ok && (decoded.width > d->width || decoded.height > d->height))
		ok = Error(error, "PANL %ux%u exceeds cockpit %ux%u", decoded.width, decoded.height, d->width,
				   d->height);
	if (ok) {
		AeronIndexedFrame* frame = &image->bitmap;
		frame->width = d->width;
		frame->height = d->height;
		frame->indices = calloc((size_t)d->width * d->height, 1);
		frame->coverage = calloc((size_t)d->width * d->height, 1);
		ok = (frame->indices && frame->coverage) || Error(error, "Cockpit bitmap allocation failed");
		if (ok) {
			for (unsigned y = 0; y < decoded.height; ++y) {
				memcpy(frame->indices + (size_t)y * d->width, decoded.indices + (size_t)y * decoded.width,
					   decoded.width);
				memcpy(frame->coverage + (size_t)y * d->width, decoded.coverage + (size_t)y * decoded.width,
					   decoded.width);
			}
			ok = Mask(frame, mask, d, error);
		}
	}
	AeronIndexedFrame_Free(&decoded);
	AeronLfd_Free(&lfd);
	return ok;
}

static bool Font(XwHudImage* image, const XwRenderSource* source, const XwCockpitDefinition* d,
				 AeronDecodeError* error) {
	const uint8_t* bytes = source->data;
	if (source->size < 2)
		return Error(error, "Font header is truncated");
	unsigned row_bytes = d->width == 320 ? 1 : 4, stride = 2 + 2 * bytes[1] * row_bytes;
	/* DOS font files can end with alignment/trailer bytes; only complete glyph records are visible. */
	size_t size = source->size / stride * stride;
	if (!AeronBitmapFont_Decode(bytes, size, row_bytes, 32, &image->font, error))
		return false;
	AeronDecodedFont* f = &image->font;
	/* X-Wing derives the shadow from the preceding foreground row instead of reading plane two. */
	memset(f->shadow, 0, (size_t)f->width * f->height);
	for (unsigned i = 0; i < f->glyph_count; ++i) {
		const AeronDecodedGlyph* g = &f->glyphs[i];
		for (unsigned y = 1; y < g->height; ++y)
			for (unsigned x = 1; x <= g->advance; ++x) {
				size_t at = (size_t)(g->y + y) * f->width + g->x + x;
				f->shadow[at] = f->foreground[at - f->width - 1] && !f->foreground[at] ? 255 : 0;
			}
	}
	return true;
}

bool XwHudDecode_Image(XwHudImage* image, const XwCockpitDefinition* definition, AeronDecodeError* error) {
	const XwRenderSource* source = XwRenderAssets_Source(image->key.source);
	if (!source)
		return Error(error, "Registered image source is unavailable");
	if (image->key.kind == XW_HUD_IMAGE_BASE)
		return Base(image, source, definition, error);
	if (image->key.kind == XW_HUD_IMAGE_FONT)
		return Font(image, source, definition, error);
	AeronPnlList list = { 0 };
	bool ok = AeronPnl_Parse(source->data, source->size, image->key.frame + 1, &list, error);
	if (ok && list.count <= image->key.frame)
		ok = Error(error, "PNL frame %u is absent (decoded %u records)", image->key.frame, list.count);
	if (ok) {
		AeronByteSpan span = list.bitmaps[image->key.frame];
		/* Empty authored sprite slots are valid no-op states. */
		if (span.size == 1 && span.data[0] == 255) {
			image->bitmap.width = image->bitmap.height = 1;
			image->bitmap.indices = calloc(1, 1);
			image->bitmap.coverage = calloc(1, 1);
			ok = (image->bitmap.indices && image->bitmap.coverage) ||
				 Error(error, "Empty sprite allocation failed");
		} else
			ok = AeronPnl_DecodeIndexed(span.data, span.size, 0, image->key.transparent, &image->bitmap,
										error);
	}
	AeronPnl_Free(&list);
	return ok;
}
