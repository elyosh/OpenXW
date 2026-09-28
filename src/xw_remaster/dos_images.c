#include "xw_remaster/dos_images.h"
#include "xw_runtime/snapshot/render_dos_assets.h"
#include <aeron/render.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned Word(const uint8_t* p) { return p[0] | ((unsigned)p[1] << 8); }

/* ROTSCALE consumes complete runs; header bounds do not clip the encoded pixels. */
static const char* Measure(const uint8_t* data, size_t size, int* width, int* height, size_t* end) {
	if (size < 6 || size > UINT16_MAX)
		return "invalid image size";
	/* Authored bounds use +Y up; encoded rows descend from top to bottom. */
	*width = (int8_t)data[2] - (int8_t)data[0] + 1;
	*height = (int8_t)data[1] - (int8_t)data[3] + 1;
	if (*width <= 0 || *height <= 0)
		return "invalid signed image bounds";
	int rows = 0;
	for (size_t cursor = 5; cursor < size;) {
		unsigned count = data[cursor];
		if (count == 255) {
			*end = cursor;
			if (rows > *height)
				*height = rows;
			return NULL;
		}
		if (size - cursor < 3u + count)
			return "truncated row runs";
		int right = data[cursor + 1];
		for (unsigned i = 0; i < count; ++i)
			right += (data[cursor + 3 + i] & 15) + 1;
		if (right > *width)
			*width = right;
		++rows;
		cursor += 3 + count;
	}
	return "missing image terminator";
}

static const char* Decode(const uint8_t* data, size_t size, unsigned id, AeronRuntimeAtlasFrame* out,
						  size_t* remaining_bytes) {
	int width, height;
	size_t end;
	const char* error = Measure(data, size, &width, &height, &end);
	if (error)
		return error;
	size_t pixels = (size_t)width * height;
	if (pixels > *remaining_bytes / 4)
		return "decoded frames exceed the 64 MiB upload budget";
	uint8_t* rgba = calloc(pixels, 4);
	if (!rgba)
		return "image allocation failed";
	*remaining_bytes -= pixels * 4;
	*out = (AeronRuntimeAtlasFrame) { .rgba = rgba,
									  .width = width,
									  .height = height,
									  .anchor_x = (int8_t)data[0],
									  .anchor_y = (int8_t)data[1],
									  .id = id };
	unsigned y = 0;
	for (size_t cursor = 5; cursor < end;) {
		unsigned count = data[cursor];
		unsigned x = data[cursor + 1];
		for (unsigned i = 0; i < count; ++i) {
			unsigned run = data[cursor + 3 + i], length = (run & 15) + 1, color = run >> 4;
			for (unsigned j = 0; j < length; ++j, ++x) {
				uint8_t* pixel = rgba + 4 * (y * width + x);
				pixel[0] = color;
				pixel[3] = color == data[4] ? 0 : 255;
			}
		}
		++y;
		cursor += 3 + count;
	}
	return NULL;
}

static bool Failure(char* error, size_t capacity, const char* reason) {
	if (error && capacity)
		snprintf(error, capacity, "%s", reason);
	return false;
}

bool XwDosImages_Build(AeronCommandBuffer* cmd, const XwRenderSource* source, AeronRuntimeAtlas* out,
					   char* error, size_t capacity) {
	if (!source || source->flight_version == 98)
		return Failure(error, capacity, "Invalid DOS bitmap source");
	const uint8_t* bytes = source->data;
	size_t size = source->size;
	unsigned count = 0;
	const uint16_t* offsets = NULL;
	if (source->kind == XW_SOURCE_DOS_MODEL && size == sizeof(XwRenderDosModel)) {
		const XwRenderDosModel* model = source->data;
		bytes = model->bytes;
		size = model->size;
		count = model->image_count;
		offsets = model->image_offsets;
	} else if (source->kind == XW_SOURCE_BITMAP && bytes && size >= 2) {
		count = Word(bytes);
		if (count > (size - 2) / 2)
			return Failure(error, capacity, "Truncated DOS bitmap offset table");
	}
	if (!bytes || !count || count > 256)
		return Failure(error, capacity, "Invalid DOS bitmap frame count or payload");
	AeronRuntimeAtlasFrame* frames = calloc(count, sizeof *frames);
	if (!frames)
		return Failure(error, capacity, "DOS bitmap frame allocation failed");
	bool ok = true;
	size_t remaining_bytes = 64u * 1024u * 1024u;
	for (unsigned i = 0; ok && i < count; ++i) {
		size_t start = offsets ? offsets[i] : Word(bytes + 2 + 2 * i), end = size;
		for (unsigned j = 0; j < count; ++j) {
			size_t next = offsets ? offsets[j] : Word(bytes + 2 + 2 * j);
			if (next > start && next < end)
				end = next;
		}
		const char* reason = start < end && end <= size
								 ? Decode(bytes + start, end - start, i, &frames[i], &remaining_bytes)
								 : "invalid image offset";
		ok = reason == NULL;
		if (!ok && error && capacity)
			snprintf(error, capacity, "DOS bitmap frame %u (offset %zu): %s", i, start, reason);
	}
	if (ok) {
		ok = Aeron_RuntimeAtlasBuild(
			out, cmd, frames, (int)count,
			&(AeronRuntimeAtlasOptions) { .format = AERON_TEXTURE_FORMAT_RGBA8_UNORM,
										  .color_space = AERON_COLOR_SPACE_LINEAR_SRGB,
										  .alpha_mode = AERON_IMAGE_ALPHA_STRAIGHT,
										  .debug_name = source->path });
		if (!ok && error && capacity)
			snprintf(error, capacity, "DOS bitmap atlas upload failed: %s", Aeron_RenderLastError());
	}
	for (unsigned i = 0; i < count; ++i)
		free((void*)frames[i].rgba);
	free(frames);
	return ok;
}
