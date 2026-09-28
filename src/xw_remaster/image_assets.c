/* ACT atlas policy follows OpenXvT original_2d.c; TEX/RGB follows X-Wing's loader. */
#include "xw_remaster/image_assets.h"
#include "xw/assets/model_texture.h"
#include <aeron/asset/act.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { IMAGE_BUDGET = 64 * 1024 * 1024 };

static uint32_t U32(const uint8_t* p) {
	return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static bool Error(char* error, size_t capacity, const char* message) {
	if (error && capacity)
		snprintf(error, capacity, "%s", message);
	return false;
}

static bool Texture(const XwRenderSource* source, AeronRuntimeAtlasFrame* out) {
	const uint8_t* bytes = source->data;
	const char* extension = strrchr(source->path, '.');
	bool rgb = extension && !strcmp(extension, ".rgb");
	if (source->size < (rgb ? 512u : 24u))
		return false;
	uint32_t width = rgb ? (unsigned)bytes[6] * 256 + bytes[7] : U32(bytes + 16);
	uint32_t height = rgb ? (unsigned)bytes[8] * 256 + bytes[9] : U32(bytes + 20);
	if (!width || !height || width > 4096 || height > 4096)
		return false;
	size_t pixels = (size_t)width * height;
	const uint8_t* colors = NULL;
	if (rgb) {
		if (pixels > (source->size - 512) / 3)
			return false;
	} else {
		/* The file omits the software shade table. Hardware RGB565 shades
		 * follow the complete texel/mipmap payload; row 8 is full brightness. */
		uint32_t payload = pixels == U32(bytes + 8) ? U32(bytes + 12) : (uint32_t)pixels;
		if (payload < pixels || payload > source->size - 24 ||
			MODEL_TEXTURE_SHADE_LEVELS * 256u * 2 > source->size - 24 - payload)
			return false;
		colors = bytes + 24 + payload + MODEL_TEXTURE_BASE_SHADE * 256 * 2;
	}
	uint8_t* rgba = malloc(pixels * 4);
	if (!rgba)
		return false;
	for (size_t i = 0; i < pixels; ++i) {
		for (unsigned channel = 0; channel < 3; ++channel) {
			if (rgb)
				rgba[i * 4 + channel] = bytes[512 + channel * pixels + i];
			else {
				unsigned index = bytes[24 + i];
				unsigned packed = colors[index * 2] | (unsigned)colors[index * 2 + 1] << 8;
				unsigned shift = channel == 0 ? 11 : (channel == 1 ? 5 : 0);
				unsigned value = (packed >> shift) & (channel == 1 ? 63 : 31);
				rgba[i * 4 + channel] =
					(uint8_t)(channel == 1 ? (value << 2) | (value >> 4) : (value << 3) | (value >> 2));
			}
		}
		rgba[i * 4 + 3] = 255;
	}
	*out = (AeronRuntimeAtlasFrame) { .rgba = rgba, .width = (int)width, .height = (int)height };
	return true;
}

static bool Images(const XwRenderSource* source, AeronRuntimeAtlasFrame** output, unsigned* count,
				   char* error, size_t capacity) {
	/* Bound the decoded working set before the shared decoder allocates its planes. */
	const uint8_t* bytes = source->data;
	if (source->size < 52)
		return Error(error, capacity, "truncated effect image header");
	unsigned table = U32(bytes + 16), frames_count = U32(bytes + 24);
	if (!frames_count || frames_count > 256 || table > source->size ||
		frames_count > (source->size - table) / 4)
		return Error(error, capacity, "invalid effect frame table");
	size_t decoded_bytes = 0;
	for (unsigned i = 0; i < frames_count; ++i) {
		unsigned offset = U32(bytes + table + i * 4);
		if (offset > source->size || source->size - offset < 44)
			return Error(error, capacity, "truncated effect frame");
		unsigned w = U32(bytes + offset + 16), h = U32(bytes + offset + 20);
		if (!w || !h || w > 4096 || h > 4096 || (size_t)w * h * 6 > IMAGE_BUDGET - decoded_bytes)
			return Error(error, capacity, "effect image exceeds the 64 MiB decode budget");
		decoded_bytes += (size_t)w * h * 6; /* Indexed, coverage and RGBA planes coexist. */
	}
	AeronIndexedFrames decoded = { 0 };
	AeronDecodeError detail = { 0 };
	if (!AeronAct_Decode(source->data, source->size, &decoded, &detail))
		return Error(error, capacity, detail.message);
	AeronRuntimeAtlasFrame* frames = calloc(decoded.count, sizeof *frames);
	bool ok = frames != NULL;
	size_t total = 0;
	for (unsigned i = 0; ok && i < decoded.count; ++i) {
		const AeronIndexedFrame* image = &decoded.frames[i];
		size_t bytes = (size_t)image->width * image->height * 4;
		if (bytes > IMAGE_BUDGET - total) {
			ok = false;
			break;
		}
		total += bytes;
		uint8_t* rgba = calloc(bytes, 1);
		if (!rgba) {
			ok = false;
			break;
		}
		frames[i] = (AeronRuntimeAtlasFrame) {
			rgba, image->width, image->height, image->frame_index, image->anchor_x, image->anchor_y
		};
		for (size_t p = 0; p < bytes / 4; ++p) {
			if (!image->coverage[p])
				continue;
			memcpy(rgba + p * 4, image->palette[image->indices[p]], 3);
			rgba[p * 4 + 3] = image->coverage[p];
		}
	}
	*output = frames;
	*count = decoded.count;
	AeronIndexedFrames_Free(&decoded);
	return ok || Error(error, capacity, "effect image allocation or 64 MiB budget exceeded");
}

bool XwImageAssets_Build(AeronCommandBuffer* cmd, const XwRenderSource* source, AeronRuntimeAtlas* out,
						 char* error, size_t capacity) {
	AeronRuntimeAtlasFrame* frames = NULL;
	unsigned count = 0;
	bool ok;
	if (source->kind == XW_SOURCE_BITMAP)
		ok = Images(source, &frames, &count, error, capacity);
	else {
		frames = calloc(1, sizeof *frames);
		count = 1;
		ok = frames && Texture(source, frames);
		if (!ok)
			Error(error, capacity, "invalid TEX/RGB source or allocation failure");
	}
	if (ok) {
		ok = Aeron_RuntimeAtlasBuild(
			out, cmd, frames, (int)count,
			&(AeronRuntimeAtlasOptions) { .format = AERON_TEXTURE_FORMAT_RGBA8_SRGB,
										  .color_space = AERON_COLOR_SPACE_SRGB,
										  .alpha_mode = AERON_IMAGE_ALPHA_PREMULTIPLIED,
										  .generate_mips = true,
										  .debug_name = source->path });
		if (!ok)
			Error(error, capacity, "effect atlas GPU creation failed");
	}
	if (frames)
		for (unsigned i = 0; i < count; ++i)
			free((void*)frames[i].rgba);
	free(frames);
	return ok;
}
