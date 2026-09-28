/* SPDX-License-Identifier: GPL-3.0-or-later
 * Adapted from undither 1.0.8 by Kornel Lesinski, https://github.com/kornelski/undither.
 * C adaptation: exhaustive palette search and coverage-aware averaging.
 * See licenses/undither.txt. */
#include "xw_remaster/undither.h"
#include <stdlib.h>
#include <string.h>

static unsigned ColorDistance(const uint8_t a[3], const uint8_t b[3]) {
	unsigned distance = 0;
	for (unsigned channel = 0; channel < 3; ++channel) {
		int delta = a[channel] - b[channel];
		distance += delta * delta;
	}
	return distance;
}

static unsigned PaletteWeight(const uint8_t palette[256][4], unsigned count, uint8_t* weights, unsigned a,
							  unsigned b) {
	if (weights[a * count + b] != 255)
		return weights[a * count + b];
	if (a == b)
		return weights[a * count + b] = 7;
	if (a > b) {
		unsigned swap = a;
		a = b;
		b = swap;
	}
	uint8_t midpoint[3];
	for (unsigned channel = 0; channel < 3; ++channel)
		midpoint[channel] = (palette[a][channel] + palette[b][channel]) / 2;
	unsigned distance = ColorDistance(midpoint, palette[a]);
	unsigned nearest = UINT32_MAX;
	for (unsigned other = 0; other < count; ++other) {
		if (other == a || other == b)
			continue;
		unsigned candidate = ColorDistance(midpoint, palette[other]);
		if (candidate < nearest)
			nearest = candidate;
	}
	unsigned weight = nearest >= distance * 2             ? 8
					  : nearest >= distance               ? 6
					  : nearest * 3ull >= distance * 2ull ? 1
														  : 0;
	weights[a * count + b] = weights[b * count + a] = (uint8_t)weight;
	return weight;
}

static void FilterPixel(const AeronIndexedFrame* image, const uint8_t palette[256][4], unsigned count,
						uint8_t* weights, int x, int y, uint8_t* output) {
	unsigned center = image->indices[(size_t)y * image->width + x];
	unsigned total = 8;
	unsigned sum[3];
	for (unsigned channel = 0; channel < 3; ++channel)
		sum[channel] = palette[center][channel] * total;
	for (int row = 0; row < 3; ++row) {
		int sample_y = y + row - 1;
		if (sample_y < 0)
			sample_y = 0;
		if (sample_y >= image->height)
			sample_y = image->height - 1;
		for (int column = 0; column < 3; ++column) {
			if (row == 1 && column == 1)
				continue;
			int sample_x = x + column - 1;
			if (sample_x < 0)
				sample_x = 0;
			if (sample_x >= image->width)
				sample_x = image->width - 1;
			size_t pixel = (size_t)sample_y * image->width + sample_x;
			if (!image->coverage[pixel])
				continue;
			unsigned index = image->indices[pixel],
					 weight = PaletteWeight(palette, count, weights, center, index);
			total += weight;
			for (unsigned channel = 0; channel < 3; ++channel)
				sum[channel] += palette[index][channel] * weight;
		}
	}
	for (unsigned channel = 0; channel < 3; ++channel)
		output[channel] = (uint8_t)(sum[channel] / total);
}

int XwUndither_Apply(const AeronIndexedFrame* image, const uint8_t palette[256][4], unsigned palette_count,
					 uint8_t* rgba) {
	if (!image || !rgba || !palette || !image->indices || !image->coverage || image->width <= 0 ||
		image->height <= 0 || !palette_count || palette_count > 256)
		return 0;
	for (size_t pixel = 0; pixel < (size_t)image->width * image->height; ++pixel)
		if (image->coverage[pixel] && image->indices[pixel] >= palette_count)
			return 0;
	uint8_t* weights = malloc(palette_count * palette_count);
	if (!weights)
		return 0;
	memset(weights, 255, palette_count * palette_count);
	for (int y = 0; y < image->height; ++y)
		for (int x = 0; x < image->width; ++x) {
			size_t pixel = (size_t)y * image->width + x;
			if (image->coverage[pixel])
				FilterPixel(image, palette, palette_count, weights, x, y, rgba + pixel * 4);
		}
	free(weights);
	return 1;
}
