#include "xw_remaster/hud_asset_upload.h"
#include "xw_remaster/undither.h"
#include <stdlib.h>
#include <string.h>

static uint8_t* Rgba(const XwHudImage* image, const uint32_t colors[256], bool filter, int* width,
					 int* height) {
	const AeronIndexedFrame* b = &image->bitmap;
	if (image->key.kind == XW_HUD_IMAGE_FONT) {
		const AeronDecodedFont* f = &image->font;
		*width = f->width;
		*height = f->height * 2 + 2;
		uint8_t* out = calloc((size_t)*width * *height, 4);
		if (!out)
			return NULL;
		size_t pixels = (size_t)f->width * f->height;
		for (size_t i = 0; i < pixels * 2; ++i) {
			uint8_t a = i < pixels ? f->foreground[i] : f->shadow[i - pixels];
			memset(out + i * 4, a, 4);
		}
		/* OpenXvT's transparent guard row and opaque strip for batched text backgrounds. */
		memset(out + (pixels * 2 + f->width) * 4, 255, (size_t)f->width * 4);
		return out;
	}
	*width = b->width;
	*height = b->height;
	size_t pixels = (size_t)b->width * b->height;
	uint8_t* out = malloc(pixels * 4);
	if (!out)
		return NULL;
	uint8_t rgba_palette[256][4];
	for (unsigned i = 0; i < 256; ++i) {
		rgba_palette[i][0] = colors[i] >> 16;
		rgba_palette[i][1] = colors[i] >> 8;
		rgba_palette[i][2] = colors[i];
		rgba_palette[i][3] = 255;
	}
	for (size_t i = 0; i < pixels; ++i) {
		memcpy(out + i * 4, rgba_palette[b->indices[i]], 4);
		out[i * 4 + 3] = b->coverage[i];
		if (!b->coverage[i])
			memset(out + i * 4, 0, 4);
	}
	if (filter && !XwUndither_Apply(b, rgba_palette, 256, out)) {
		free(out);
		return NULL;
	}
	return out;
}

bool XwHudAssetUpload(AeronCommandBuffer* cmd, const XwHudImage* images, unsigned count,
					  const uint32_t colors[256], bool filter, AeronRuntimeAtlas* out,
					  AeronImageCoverage* coverage) {
	if (!count || count > XW_HUD_IMAGES || (coverage && count != 1))
		return false;
	AeronRuntimeAtlasFrame* frames = calloc(count, sizeof *frames);
	if (!frames)
		return false;
	size_t bytes = 0;
	bool ok = true;
	for (unsigned i = 0; i < count && ok; ++i) {
		int width = 0, height = 0;
		frames[i].rgba = Rgba(&images[i], colors, filter, &width, &height);
		frames[i].width = width;
		frames[i].height = height;
		frames[i].id = i;
		bytes += (size_t)width * height * 4;
		ok = frames[i].rgba && bytes <= 64u * 1024u * 1024u;
	}
	const AeronRuntimeAtlasOptions options = { .format = AERON_TEXTURE_FORMAT_RGBA8_SRGB,
											   .color_space = AERON_COLOR_SPACE_SRGB,
											   .alpha_mode = AERON_IMAGE_ALPHA_PREMULTIPLIED,
											   .debug_name = "X-Wing resident cockpit" };
	if (ok && coverage)
		ok = Aeron_ImageBuildCoverageRgba8(frames[0].rgba, frames[0].width, frames[0].height, coverage);
	if (ok)
		ok = Aeron_RuntimeAtlasBuild(out, cmd, frames, count, &options);
	if (!ok && coverage)
		Aeron_ImageFreeCoverage(coverage);
	for (unsigned i = 0; i < count; ++i)
		free((void*)frames[i].rgba);
	free(frames);
	return ok;
}
