#include "xw/render/flight_view.h"
#include "xw_dos94/render/display.h"
#include "xw_dos94/render/raster.h"
#include <string.h>

static uint16_t next_run(uint16_t index) { return index + 1 == DOS94_SPAN_RUN_CAPACITY ? 0 : index + 1; }

/* DOS94 0x6A1980. Initial runs use 256 pixels; emitted flat spans split at 255. */
void Dos94_xtrans2_clearruntable(void) {
	Dos94Raster* r = &Dos94_display->raster;
	Dos94SpanRun* cache = Dos94_display->runs;
	uint16_t cursor = 0;
	for (unsigned row = 0; row < g_flightVpHeight; ++row) {
		unsigned width = g_flightVpWidth;
		if (width > 256) {
			cache[cursor] = (Dos94SpanRun) { 255, 255 };
			++cursor;
			width -= 256;
		}
		cache[cursor] = (Dos94SpanRun) { (uint8_t)(width - 1), 255 };
		++cursor;
	}
	r->runRead = 0;
	r->runWrite = cursor;
}

void Dos94_XTRANS2_beginoutput(void) { Dos94_display->raster.frameRuns = Dos94_display->raster.runWrite; }

void Dos94_XTRANS2_endoutput(void) { Dos94_display->raster.runRead = Dos94_display->raster.frameRuns; }

static void append_run(uint16_t count, uint8_t color) {
	Dos94Raster* r = &Dos94_display->raster;
	Dos94_display->runs[r->runWrite] = (Dos94SpanRun) { (uint8_t)(count - 1), color };
	r->runWrite = next_run(r->runWrite);
}

/* DOS94 0x6A2F95: only a partially consumed run changes its stored length. */
static void consume_runs(uint16_t count) {
	Dos94Raster* r = &Dos94_display->raster;
	Dos94SpanRun* cache = Dos94_display->runs;
	while (count) {
		unsigned available = cache[r->runRead].lengthMinusOne + 1u;
		if (available > count) {
			cache[r->runRead].lengthMinusOne -= count;
			return;
		}
		count -= available;
		r->runRead = next_run(r->runRead);
	}
}

static void cached_span(uint16_t row, uint16_t start, uint16_t length, uint8_t color) {
	Dos94Display* d = Dos94_display;
	Dos94Raster* r = &d->raster;
	uint16_t x = start, remaining = length;
	while (remaining) {
		unsigned available = d->runs[r->runRead].lengthMinusOne + 1u;
		unsigned count = available < remaining ? available : remaining;
		uint8_t previous = d->runs[r->runRead].color;
		if (!color || color != previous) {
			uint8_t* destination = d->screen + g_flightVpBaseOffset + row * 320u + x;
			if (color)
				memset(destination, color, count);
			else
				memcpy(destination, d->logical + row * d->rowStride + x, count);
		}
		d->runs[r->runRead].lengthMinusOne = (uint8_t)(d->runs[r->runRead].lengthMinusOne - remaining);
		if (available <= remaining)
			r->runRead = next_run(r->runRead);
		x += count;
		remaining -= count;
	}
	append_run(length, color);
}

/* DOS94 0x6A3043 and the 0x6A3018 split entry. */
void Dos94_XTRANS2_outputCachedSpan(uint16_t row, uint16_t start, uint16_t end, uint8_t color) {
	if (row >= g_flightVpHeight || end > g_flightVpWidth || start >= end)
		return;
	uint16_t length = end - start;
	if (length > 255) {
		uint16_t first = length - 255;
		cached_span(row, start, first, color);
		start += first;
	}
	cached_span(row, start, end - start, color);
}

/* DOS94 0x6A2E63–0x6A300F. The ramp endpoint extension is local: no host write
 * past the last material is needed, and aliases see no persistent palette edit. */
void Dos94_XTRANS2_outputGouraudSpan(uint16_t row, uint16_t start, uint16_t end, uint8_t material,
									 uint16_t leftX, uint8_t leftLight, uint16_t rightX, uint8_t rightLight) {
	if (row >= g_flightVpHeight || end > g_flightVpWidth || start >= end || !material || material > 39)
		return;
	Dos94Display* d = Dos94_display;
	uint8_t* output = d->screen + g_flightVpBaseOffset + row * 320u + start;
	const uint8_t* ramp = d->raster.materialColors + 16 * (material - 1);
	uint16_t light = 63 - (leftLight & 63), last = 63 - (rightLight & 63);
	int difference = (int)last - light;
	if (difference) {
		uint16_t magnitude = (uint16_t)((difference < 0 ? -difference : difference) << 9);
		uint16_t width = (uint16_t)(rightX - leftX);
		int16_t step = (int16_t)(width ? magnitude / width : magnitude);
		if (difference < 0)
			step = (int16_t)-step;
		uint16_t shade = (uint16_t)((light << 9) + (uint16_t)(step * (int16_t)(start - leftX)));
		uint16_t dither = (row & 1) << 10;
		for (uint16_t x = start; x < end; ++x) {
			uint16_t sum = (uint16_t)(shade + dither);
			unsigned index = sum >> 11;
			*output++ = ramp[index < 16 ? index : 15];
			shade = (uint16_t)(shade + step);
			dither = sum & 0x7FF;
		}
	} else {
		unsigned shade = light >> 2;
		unsigned fraction = shade == 15 ? 0 : (light & 3) << 6;
		unsigned accumulator = (row & 1) << 7;
		for (uint16_t x = start; x < end; ++x) {
			accumulator += fraction;
			*output++ = ramp[shade + (accumulator >> 8)];
			accumulator &= 255;
		}
	}
	uint16_t length = end - start;
	consume_runs(length);
	/* Shaded output stores the remainder first, then a full 256-pixel run. */
	if (length > 256) {
		append_run(length - 256, 0);
		append_run(256, 0);
	} else
		append_run(length, 0);
}
