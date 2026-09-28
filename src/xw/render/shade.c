#include "xw/render/shade.h"

#include "xw/render/flight_palette.h"

#include <landru/canvas.h>
#include <stdlib.h>

#ifndef XW_MODERN
// GLOBAL: XW 0x5BECB8
extern uint8_t* g_draw_buff_gbl;
// GLOBAL: XW 0x5BECCC
extern int32_t g_draw_w_gbl;
#endif

// GLOBAL: XW 0x4D8D90
uint8_t g_shadePalette[SHADE_PALETTE_COLOR_COUNT] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

// FUNCTION: XW 0x4698E0
int16_t shade_Set_Shaded_Palette(const struct RgbTriplet* colors, int blendFactor, uint16_t targetRed,
								 uint16_t targetGreen, uint16_t targetBlue) {
	uint8_t blendDelta[SHADE_PALETTE_COLOR_COUNT];
	int16_t squaredDifference[SHADE_PALETTE_COLOR_COUNT];
	int nextOddNumber;
#ifdef XW_MODERN
	uint32_t blendAccumulator;
#else
	int blendAccumulator;
#endif
	int index;
	int remaining;
	int16_t sourceIndex;
	squaredDifference[0] = 0;
	blendDelta[0] = 0;
	nextOddNumber = 1;
	blendAccumulator = blendFactor;
	for (index = 1, remaining = SHADE_PALETTE_COLOR_COUNT - 1; remaining != 0; ++index, --remaining) {
		squaredDifference[index] = squaredDifference[index - 1] + nextOddNumber;
		nextOddNumber += 2;
		blendDelta[index] = blendAccumulator >> SHADE_BLEND_FRACTION_BITS;
		blendAccumulator += blendFactor;
	}
	for (sourceIndex = 0; sourceIndex < SHADE_PALETTE_COLOR_COUNT; ++sourceIndex) {
		uint16_t red = colors[sourceIndex].r;
		uint16_t blue = colors[sourceIndex].b;
		uint16_t green = colors[sourceIndex].g;
		int16_t bestDistance;
		int16_t bestIndex;
		int16_t candidateIndex;
		if (red > targetRed)
			red -= blendDelta[red - targetRed];
		else
			red += blendDelta[targetRed - red];
		if (green > targetGreen)
			green -= blendDelta[green - targetGreen];
		else
			green += blendDelta[targetGreen - green];
		if (blue > targetBlue)
			blue -= blendDelta[blue - targetBlue];
		else
			blue += blendDelta[targetBlue - blue];
		bestIndex = sourceIndex;
		bestDistance = SHADE_INITIAL_DISTANCE;
		for (candidateIndex = 0; candidateIndex < SHADE_PALETTE_COLOR_COUNT; ++candidateIndex) {
			if (blendDelta[candidateIndex] != 0) {
				int16_t distance = squaredDifference[abs(red - colors[candidateIndex].r)] +
								   squaredDifference[abs(blue - colors[candidateIndex].b)];
				distance += squaredDifference[abs(green - colors[candidateIndex].g)];
				if (distance < bestDistance) {
					bestDistance = distance;
					bestIndex = candidateIndex;
				}
			}
		}
		g_shadePalette[sourceIndex] = bestIndex;
	}
	return 1;
}

// FUNCTION: XW 0x469B20
int16_t shade_RemapClippedRect(const Rect* rect) {
	Rect rasterClip;
	int16_t clipWidth;
	int16_t clipHeight;
	int16_t left;
	int16_t top;
	int16_t width;
	int16_t height;
	int16_t requestedHeight;

	xcanvas_Get_Raster_Clip(&rasterClip);
	clipWidth = rasterClip.right - rasterClip.left;
	clipHeight = rasterClip.bottom - rasterClip.top;
	left = rect->left;
	top = rect->top;
	width = rect->right - left;
	requestedHeight = rect->bottom - top;
	if (left < rasterClip.left) {
		int16_t leftClipAmount = rasterClip.left - left;
		left = rasterClip.left;
		width -= leftClipAmount;
	}
	if (width + left > rasterClip.left + clipWidth) {
		width = clipWidth + rasterClip.left - left;
	}
	if (top < rasterClip.top) {
		requestedHeight += top - rasterClip.top;
		top = rasterClip.top;
	}
	if (requestedHeight + top > rasterClip.top + clipHeight) {
		height = clipHeight + rasterClip.top - top;
	} else {
		height = requestedHeight;
	}
	if (width > 0 && height > 0) {
		shade_Shadow_Line_List(g_shadePalette, left, top, width, height);
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x469C10
void shade_Shadow_Line_List(const uint8_t* remap, int16_t x, int16_t y, int16_t width, int16_t height) {
#ifdef XW_MODERN
	uint8_t* pixels = (uint8_t*)draw_buff_gbl + x + draw_w_gbl * y;
	int16_t rowPadding = (int16_t)(draw_w_gbl - width);
#else
	uint8_t* pixels = g_draw_buff_gbl + x + g_draw_w_gbl * y;
	int16_t rowPadding = (int16_t)(g_draw_w_gbl - width);
#endif
	if (height > 0) {
		int row = height;
		do {
			if (width > 0) {
				int column = width;
				do {
					*pixels = remap[*pixels];
					++pixels;
				} while (--column != 0);
			}
			pixels += rowPadding;
		} while (--row != 0);
	}
}
