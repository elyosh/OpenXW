#include "xw/render/rtsvga2.h"

#ifdef XW_MODERN
#include "xw_runtime/snapshot/render_hud.h"
#endif
#include "xw/assets/bitmap.h"

#include "xw/flight/flight_display.h"
#include "xw/flight/hud/festring.h"
#include "xw/flight/player/user.h"
#include "xw/render/rtsrgb.h"
#include "xw/util/shared.h"
#include "xw_runtime/compat/framebuffer_address.h"

#ifdef XW_MODERN
#include "xw_dos94/render/display.h"
#include "xw_runtime/runtime/flight_types.h"
#endif

#include <limits.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4C9C18
FlightMarkerOffset g_flightBracketOffsets10[RTSVGA2_DEFAULT_BRACKET_PIXEL_COUNT] = {
	{ -1, 1 }, { -2, 1 }, { -2, 0 }, { -2, -1 }, { -1, -1 },
	{ 1, -1 }, { 2, -1 }, { 2, 0 },  { 2, 1 },   { 1, 1 }
};

// GLOBAL: XW 0x4C9C30
FlightMarkerOffset g_flightBracketOffsets12[RTSVGA2_VESA_BRACKET_PIXEL_COUNT] = {
	{ -1, 2 }, { -2, 2 }, { -2, 1 }, { -2, 0 }, { -2, -1 }, { -1, -1 },
	{ 1, -1 }, { 2, -1 }, { 2, 0 },  { 2, 1 },  { 2, 2 },   { 1, 2 }
};

// GLOBAL: XW 0x4C9C48
const FlightMarkerOffset* g_flightBracketOffsets = g_flightBracketOffsets10;

// GLOBAL: XW 0x4C9C4C
unsigned int g_flightBracketOffsetCount = RTSVGA2_DEFAULT_BRACKET_PIXEL_COUNT;

// GLOBAL: XW 0x4C9C50
FlightMarkerOffset g_crossMarkerOffsets8[RTSVGA2_CROSS_PIXEL_COUNT] = { { -2, 0 }, { -1, 0 }, { 0, 0 },
																		{ 1, 0 },  { 2, 0 },  { 0, 1 },
																		{ 0, -1 } };

// GLOBAL: XW 0x4F4980
uint16_t g_flightFillRectBottom8bpp = 0;

// GLOBAL: XW 0x4F4984
uint16_t g_flightFillRectRight8bpp = 0;

// GLOBAL: XW 0x4F4988
uint16_t g_flightFillRectLeft8bpp = 0;

// GLOBAL: XW 0x4F498C
uint16_t g_flightFillRectTop8bpp = 0;

// GLOBAL: XW 0x4F4990
uint8_t g_radarBracketSavedPixels8[RTSVGA2_BRACKET_SAVED_PIXEL_CAPACITY] = { 0 };

// GLOBAL: XW 0x4F49A0
uint16_t g_flightFillRectCurrentY8bpp = 0;

// GLOBAL: XW 0x4F49A8
uint8_t g_crossMarkerSavedPixels8[RTSVGA2_CROSS_PIXEL_COUNT] = { 0 };

// GLOBAL: XW 0x4F49B0
unsigned int g_flightFillRectRemainingRows8bpp = 0;

// GLOBAL: XW 0x62B020
RgbTriplet g_swPalette[RTSVGA2_PALETTE_COLOR_COUNT] = { { 0, 0, 0 } };

// GLOBAL: XW 0x62C928
uint8_t g_paletteCycleEnabled = 0;

// GLOBAL: XW 0x6377AA
uint8_t g_paletteBlankFlags = 0;

// GLOBAL: XW 0x6380A0
int16_t g_flightSwRleSpriteX = 0;

// GLOBAL: XW 0x6380A2
int16_t g_flightSwRleSpriteY = 0;

// GLOBAL: XW 0x6380C0
unsigned int g_flightLineOffsetTable[RTSVGA2_SCANLINE_CAPACITY] = { 0 };

// GLOBAL: XW 0x638940
uint8_t g_flightSwRlePaletteShift = 0;

// GLOBAL: XW 0x638944
uint8_t g_flightSwRleSpriteEndMarker = 0;

// GLOBAL: XW 0x6389A0
uint8_t g_flightScratchBufferA[RTSVGA2_SCRATCH_BUFFER_SIZE] = { 0 };

// GLOBAL: XW 0x6395A0
uint8_t g_flightScratchBufferB[RTSVGA2_SCRATCH_BUFFER_SIZE] = { 0 };

// GLOBAL: XW 0x63A8EA
uint16_t g_radarPreviousTargetScreenY = 0;

// GLOBAL: XW 0x63A8EC
uint16_t g_radarPreviousTargetScreenX = 0;

// GLOBAL: XW 0x63AAC8
int16_t g_radarSelectedTargetScreenX = 0;

// GLOBAL: XW 0x63AACA
int16_t g_radarSelectedTargetScreenY = 0;

// FUNCTION: XW 0x421060
void rtsvga2_initgraphVGA(void) {
	int16_t row;
	memset(g_flightScratchBufferB, RTSVGA2_SCRATCH_EMPTY, sizeof(g_flightScratchBufferB));
	memset(g_flightScratchBufferA, RTSVGA2_SCRATCH_EMPTY, sizeof(g_flightScratchBufferA));
	for (row = 0; (unsigned int)row < g_flightScreenHeight; ++row)
		g_flightLineOffsetTable[row] = (unsigned int)row * g_surfacePitch;
	switch (g_flightResolutionMode) {
		case RTSVGA2_MODE_13H:
			memset(g_flightSwFramebufferBase, 0, g_flightScreenHeight * g_surfacePitch);
			g_flightBracketOffsets = g_flightBracketOffsets10;
			g_flightBracketOffsetCount = RTSVGA2_DEFAULT_BRACKET_PIXEL_COUNT;
			break;
		case FLIGHT_DISPLAY_MODE_101H:
		case FLIGHT_DISPLAY_MODE_111H: {
			int16_t bankIndex;
			for (bankIndex = 0; (unsigned int)bankIndex < g_flightScreenHeight * g_surfacePitch /
															  (unsigned int)g_swFramebufferClearChunkSize;
				 ++bankIndex) {
				nullsub_SharedNoOp();
				memset(g_flightSwFramebufferBase, 0, g_swFramebufferClearChunkSize);
			}
			if (g_flightScreenHeight * g_surfacePitch % (unsigned int)g_swFramebufferClearChunkSize != 0) {
				nullsub_SharedNoOp();
				memset(g_flightSwFramebufferBase, 0,
					   g_flightScreenHeight * g_surfacePitch % (unsigned int)g_swFramebufferClearChunkSize);
			}
			g_flightBracketOffsets = g_flightBracketOffsets12;
			g_flightBracketOffsetCount = RTSVGA2_VESA_BRACKET_PIXEL_COUNT;
			break;
		}
		default:
			memset(g_flightSwFramebufferBase, 0, g_flightScreenHeight * g_surfacePitch);
			g_flightBracketOffsets = g_flightBracketOffsets10;
			g_flightBracketOffsetCount = RTSVGA2_DEFAULT_BRACKET_PIXEL_COUNT;
			break;
	}
}

// FUNCTION: XW 0x4211C0
void rtsvga2_setvgapointers(uint8_t* surface, uint16_t pitchBytes, uint16_t height) {
	int16_t row;
	if (!surface) {
		unsigned int screenHeight = g_flightScreenHeight;
		g_flightSwFramebufferBase = g_surfacePixels;
#ifdef XW_MODERN
		if (screenHeight > RTSVGA2_SCANLINE_CAPACITY)
			screenHeight = RTSVGA2_SCANLINE_CAPACITY;
#endif
		for (row = 0; (unsigned int)row < screenHeight; ++row)
#ifdef XW_MODERN
			g_flightLineOffsetTable[row] = (uint32_t)row * (uint32_t)g_surfacePitch;
#else
			g_flightLineOffsetTable[row] = row * g_surfacePitch;
#endif
	} else {
		g_flightSwFramebufferBase = surface;
#ifdef XW_MODERN
		if (height > RTSVGA2_SCANLINE_CAPACITY)
			height = RTSVGA2_SCANLINE_CAPACITY;
#endif
		for (row = 0; row < (int)height; ++row)
			g_flightLineOffsetTable[row] = row * pitchBytes;
	}
}

// FUNCTION: XW 0x421240
void rtsvga2_BuildRgbRange(const struct RgbTriplet* sourcePalette, struct RgbTriplet* destPalette,
						   int startIndex, unsigned int count) {
	unsigned int entry;
	unsigned int remaining;
	if (g_flightBrightnessScaleQ8 == RTSVGA2_BRIGHTNESS_IDENTITY) {
		for (entry = 0, remaining = count; remaining != 0; ++entry, --remaining) {
			destPalette[startIndex + entry].r = sourcePalette[startIndex + entry].r;
			destPalette[startIndex + entry].g = sourcePalette[startIndex + entry].g;
			destPalette[startIndex + entry].b = sourcePalette[startIndex + entry].b;
		}
	} else {
		for (entry = 0, remaining = count; remaining != 0; ++entry, --remaining) {
			uint8_t red = sourcePalette[startIndex + entry].r;
			uint8_t green = sourcePalette[startIndex + entry].g;
			uint8_t blue = sourcePalette[startIndex + entry].b;
			uint8_t outputRed = red;
			uint8_t outputGreen = green;
			uint8_t outputBlue = blue;
			uint8_t maxChannel, minChannel, saturation, hueFraction;
			uint8_t scaledValue;
			uint8_t hueSector;
			if (red >= green && red >= blue)
				maxChannel = red;
			else if (green >= red && green >= blue)
				maxChannel = green;
			else
				maxChannel = blue;
			if (red <= green && red <= blue)
				minChannel = red;
			else if (green <= red && green <= blue)
				minChannel = green;
			else
				minChannel = blue;
			if (maxChannel != 0)
				saturation = RTSVGA2_HSV_CHANNEL_MAX * (maxChannel - minChannel) / maxChannel;
			else
				saturation = 0;
			if (saturation != 0) {
				if (red == maxChannel) {
					if (green >= blue) {
						hueSector = RTSVGA2_HUE_RED_YELLOW;
						hueFraction = RTSVGA2_HSV_CHANNEL_MAX * (green - blue) / (maxChannel - minChannel);
					} else {
						hueSector = RTSVGA2_HUE_MAGENTA_RED;
						hueFraction = RTSVGA2_HSV_CHANNEL_MAX * (green - blue) / (maxChannel - minChannel) +
									  RTSVGA2_HSV_CHANNEL_MAX;
					}
				} else if (green == maxChannel) {
					if (blue >= red) {
						hueSector = RTSVGA2_HUE_GREEN_CYAN;
						hueFraction = RTSVGA2_HSV_CHANNEL_MAX * (blue - red) / (maxChannel - minChannel);
					} else {
						hueSector = RTSVGA2_HUE_YELLOW_GREEN;
						hueFraction = RTSVGA2_HSV_CHANNEL_MAX * (blue - red) / (maxChannel - minChannel) +
									  RTSVGA2_HSV_CHANNEL_MAX;
					}
				} else if (red >= green) {
					hueSector = RTSVGA2_HUE_BLUE_MAGENTA;
					hueFraction = RTSVGA2_HSV_CHANNEL_MAX * (red - green) / (maxChannel - minChannel);
				} else {
					hueSector = RTSVGA2_HUE_CYAN_BLUE;
					hueFraction = RTSVGA2_HSV_CHANNEL_MAX * (red - green) / (maxChannel - minChannel) +
								  RTSVGA2_HSV_CHANNEL_MAX;
				}
			}
			scaledValue = ((unsigned int)maxChannel * g_flightBrightnessScaleQ8) >> RTSVGA2_BRIGHTNESS_SHIFT;
			if (scaledValue > RTSVGA2_HSV_CHANNEL_MAX)
				scaledValue = RTSVGA2_HSV_CHANNEL_MAX;
			if (saturation != 0) {
				uint8_t lowChannel =
					scaledValue * (RTSVGA2_HSV_CHANNEL_MAX - saturation) / RTSVGA2_HSV_CHANNEL_MAX;
				uint8_t fallingChannel =
					scaledValue *
					(RTSVGA2_HSV_CHANNEL_MAX - saturation * hueFraction / RTSVGA2_HSV_CHANNEL_MAX) /
					RTSVGA2_HSV_CHANNEL_MAX;
				uint8_t risingChannel =
					scaledValue *
					(RTSVGA2_HSV_CHANNEL_MAX -
					 saturation * (RTSVGA2_HSV_CHANNEL_MAX - hueFraction) / RTSVGA2_HSV_CHANNEL_MAX) /
					RTSVGA2_HSV_CHANNEL_MAX;
				switch (hueSector) {
					case RTSVGA2_HUE_RED_YELLOW:
						outputRed = scaledValue;
						outputGreen = risingChannel;
						outputBlue = lowChannel;
						break;
					case RTSVGA2_HUE_YELLOW_GREEN:
						outputRed = fallingChannel;
						outputGreen = scaledValue;
						outputBlue = lowChannel;
						break;
					case RTSVGA2_HUE_GREEN_CYAN:
						outputRed = lowChannel;
						outputGreen = scaledValue;
						outputBlue = risingChannel;
						break;
					case RTSVGA2_HUE_CYAN_BLUE:
						outputRed = lowChannel;
						outputGreen = fallingChannel;
						outputBlue = scaledValue;
						break;
					case RTSVGA2_HUE_BLUE_MAGENTA:
						outputRed = risingChannel;
						outputGreen = lowChannel;
						outputBlue = scaledValue;
						break;
					case RTSVGA2_HUE_MAGENTA_RED:
						outputRed = scaledValue;
						outputGreen = lowChannel;
						outputBlue = fallingChannel;
						break;
				}
			} else {
				outputRed = scaledValue;
				outputGreen = scaledValue;
				outputBlue = scaledValue;
			}
			destPalette[startIndex + entry].r = outputRed;
			destPalette[startIndex + entry].g = outputGreen;
			destPalette[startIndex + entry].b = outputBlue;
		}
	}
}

// FUNCTION: XW 0x4216D0
void rtsvga2_blankVGA(void) {
	RgbTriplet palette[RTSVGA2_PALETTE_COLOR_COUNT];
	int index;
	g_paletteCycleEnabled = 0;
	for (index = 0; index < RTSVGA2_PALETTE_COLOR_COUNT; ++index) {
		palette[index].r = 0;
		palette[index].g = 0;
		palette[index].b = 0;
	}
	FlightDisplay_SetPaletteEntries((const uint8_t*)palette, 0, RTSVGA2_PALETTE_COLOR_COUNT);
	g_paletteBlankFlags |= RTSVGA2_PALETTE_BLANKED;
}

// FUNCTION: XW 0x421720
void rtsvga2_unblankVGA(void) {
	RgbTriplet palette[RTSVGA2_PALETTE_COLOR_COUNT];
	g_paletteCycleEnabled = 0;
	rtsvga2_BuildRgbRange(g_swPalette, palette, 0, sizeof(palette) / sizeof(palette[0]));
	FlightDisplay_SetPaletteEntries((const uint8_t*)palette, 0, sizeof(palette) / sizeof(palette[0]));
	g_paletteCycleEnabled = 1;
	g_paletteBlankFlags &= ~RTSVGA2_PALETTE_BLANKED;
}

// FUNCTION: XW 0x421780
void rtsvga2_buildpaletteVGA(const struct RgbTriplet* rgbTriples, uint16_t startIndex, uint16_t count) {
	uint16_t paletteIndex;
	int endIndex = startIndex + count;
	unsigned int entry;
	for (paletteIndex = startIndex, entry = 0; paletteIndex < endIndex; ++paletteIndex, ++entry) {
		const RgbTriplet* source = &rgbTriples[entry];
		g_swPalette[paletteIndex].r = source->r;
		g_swPalette[paletteIndex].g = source->g;
		g_swPalette[paletteIndex].b = source->b;
	}
	if (g_flightBytesPerPixel == sizeof(g_flightTextPalette[0])) {
		FlightPalette_Build16BppRange(g_swPalette, g_flightTextPalette, startIndex, count);
		if (count == RTSVGA2_MODEL_PALETTE_FIRST_COLOR && startIndex == 0) {
			int reservedColorIndex = g_flightColorEscapeBypassChar;
			int remaining;
			for (entry = 0, remaining = RTSVGA2_MODEL_PALETTE_FIRST_COLOR; remaining != 0;
				 ++entry, --remaining) {
				uint16_t reservedColor = g_flightTextPalette[reservedColorIndex];
				if (g_flightTextPalette[entry] == reservedColor)
					g_flightTextPalette[entry] = reservedColor ^ RTSVGA2_RESERVED_COLOR_TOGGLE;
			}
		}
	}
}

// FUNCTION: XW 0x421840
void rtsvga2_savepaletteVGA(struct RgbTriplet* dstPalette) {
	size_t index;
	for (index = 0; index != sizeof(g_swPalette) / sizeof(g_swPalette[0]); ++index) {
		dstPalette[index].r = g_swPalette[index].r;
		dstPalette[index].g = g_swPalette[index].g;
		dstPalette[index].b = g_swPalette[index].b;
	}
}

// FUNCTION: XW 0x421870
void rtsvga2_restorepaletteVGA(const struct RgbTriplet* rgbTriples) {
	rtsvga2_buildpaletteVGA(rgbTriples, 0, sizeof(g_swPalette) / sizeof(g_swPalette[0]));
}

// FUNCTION: XW 0x421890
unsigned int rtsvga2_Convert24BppPalettesTo16Bpp(struct XwBitmapModelPrefix* bitmapModel) {
	uint8_t* modelBytes = (uint8_t*)bitmapModel;
	uint16_t* outputPalette = (uint16_t*)(modelBytes + bitmapModel->serializedSize);
	unsigned int frameIndex;
	unsigned int frameCount;
	RgbTriplet sourceRgb6[RTSVGA2_RGB_PALETTE_SCRATCH_COUNT];
	if (bitmapModel->paletteFormat == XW_BITMAP_PALETTE_RGB24) {
		unsigned int count = bitmapModel->paletteEntryCount;
		const uint8_t (*sourceColors)[XW_BITMAP_PALETTE_RECORD_BYTES] =
			(const uint8_t (*)[XW_BITMAP_PALETTE_RECORD_BYTES])(modelBytes +
																bitmapModel->sourcePaletteOffset);
		bitmapModel->convertedPaletteOffset = (uint8_t*)outputPalette - modelBytes;
		if (count < RTSVGA2_RGB_PALETTE_SCRATCH_COUNT) {
			unsigned int paletteIndex;
			for (paletteIndex = 0; paletteIndex < count; ++paletteIndex) {
				RgbTriplet* color = &sourceRgb6[paletteIndex];
				color->r = sourceColors[paletteIndex][0] >> RTSVGA2_RGB8_TO_RGB6_SHIFT;
				color->g = sourceColors[paletteIndex][1] >> RTSVGA2_RGB8_TO_RGB6_SHIFT;
				color->b = sourceColors[paletteIndex][2] >> RTSVGA2_RGB8_TO_RGB6_SHIFT;
			}
			FlightPalette_Build16BppRange(sourceRgb6, outputPalette, 0, count);
			outputPalette += bitmapModel->paletteEntryCount;
		}
	}
	for (frameIndex = 0; frameIndex < (frameCount = bitmapModel->frameCount); ++frameIndex) {
		const uint32_t* frameOffsets = (const uint32_t*)(modelBytes + bitmapModel->frameOffsetsOffset);
		XwBitmapFramePrefix* frame = (XwBitmapFramePrefix*)(modelBytes + frameOffsets[frameIndex]);
		frame->paletteOffset = (uint8_t*)outputPalette - (uint8_t*)frame;
		if (frame->paletteFormat == XW_BITMAP_PALETTE_RGB24) {
			const uint8_t (*sourceColors)[XW_BITMAP_PALETTE_RECORD_BYTES] =
				(const uint8_t (*)[XW_BITMAP_PALETTE_RECORD_BYTES])((uint8_t*)frame +
																	frame->sourcePaletteOffset);
			if ((unsigned int)frame->paletteEntryCount < RTSVGA2_RGB_PALETTE_SCRATCH_COUNT) {
				unsigned int paletteIndex;
				for (paletteIndex = 0; paletteIndex < (unsigned int)frame->paletteEntryCount;
					 ++paletteIndex) {
					RgbTriplet* color = &sourceRgb6[paletteIndex];
					color->r = sourceColors[paletteIndex][0] >> RTSVGA2_RGB8_TO_RGB6_SHIFT;
					color->g = sourceColors[paletteIndex][1] >> RTSVGA2_RGB8_TO_RGB6_SHIFT;
					color->b = sourceColors[paletteIndex][2] >> RTSVGA2_RGB8_TO_RGB6_SHIFT;
				}
				FlightPalette_Build16BppRange(sourceRgb6, outputPalette, 0, frame->paletteEntryCount);
				outputPalette += frame->paletteEntryCount;
			}
		}
	}
	return frameCount;
}

// FUNCTION: XW 0x4219B0
void rtsvga2_Convert24BppPalettesTo8Bpp(struct XwBitmapModelPrefix* bitmapModel) {
	uint8_t* modelBytes = (uint8_t*)bitmapModel;
	uint8_t* outputPalette = modelBytes + bitmapModel->serializedSize;
	unsigned int frameIndex;
	RgbTriplet targetRgb;
	if (bitmapModel->paletteFormat == XW_BITMAP_PALETTE_RGB24) {
		const uint8_t (*sourceColors)[XW_BITMAP_PALETTE_RECORD_BYTES] =
			(const uint8_t (*)[XW_BITMAP_PALETTE_RECORD_BYTES])(modelBytes +
																bitmapModel->sourcePaletteOffset);
		unsigned int paletteIndex;
		bitmapModel->convertedPaletteOffset = outputPalette - modelBytes;
		for (paletteIndex = 0; paletteIndex < bitmapModel->paletteEntryCount; ++paletteIndex) {
			targetRgb.r = sourceColors[paletteIndex][0] >> RTSVGA2_RGB8_TO_RGB6_SHIFT;
			targetRgb.g = sourceColors[paletteIndex][1] >> RTSVGA2_RGB8_TO_RGB6_SHIFT;
			targetRgb.b = sourceColors[paletteIndex][2] >> RTSVGA2_RGB8_TO_RGB6_SHIFT;
			outputPalette[paletteIndex] = rtsvga2_FindNearestRgbTripletIndex(
				&targetRgb, g_swPalette, RTSVGA2_MODEL_PALETTE_FIRST_COLOR, RTSVGA2_PALETTE_COLOR_COUNT);
		}
		outputPalette += paletteIndex;
	}
	for (frameIndex = 0; frameIndex < bitmapModel->frameCount; ++frameIndex) {
		const uint32_t* frameOffsets = (const uint32_t*)(modelBytes + bitmapModel->frameOffsetsOffset);
		unsigned int frameOffset = frameOffsets[frameIndex];
		XwBitmapFramePrefix* frame = (XwBitmapFramePrefix*)(modelBytes + frameOffset);
		frame->paletteOffset = outputPalette - (uint8_t*)frame;
		if (frame->paletteFormat == XW_BITMAP_PALETTE_RGB24) {
			const uint8_t (*sourceColors)[XW_BITMAP_PALETTE_RECORD_BYTES] =
				(const uint8_t (*)[XW_BITMAP_PALETTE_RECORD_BYTES])((uint8_t*)frame +
																	frame->sourcePaletteOffset);
			unsigned int paletteIndex;
			for (paletteIndex = 0; paletteIndex < (unsigned int)frame->paletteEntryCount; ++paletteIndex) {
				targetRgb.r = sourceColors[paletteIndex][0] >> RTSVGA2_RGB8_TO_RGB6_SHIFT;
				targetRgb.g = sourceColors[paletteIndex][1] >> RTSVGA2_RGB8_TO_RGB6_SHIFT;
				targetRgb.b = sourceColors[paletteIndex][2] >> RTSVGA2_RGB8_TO_RGB6_SHIFT;
				outputPalette[paletteIndex] = rtsvga2_FindNearestRgbTripletIndex(
					&targetRgb, g_swPalette, RTSVGA2_MODEL_PALETTE_FIRST_COLOR, RTSVGA2_PALETTE_COLOR_COUNT);
			}
			outputPalette += paletteIndex;
		}
	}
}

// FUNCTION: XW 0x421AC0
unsigned int rtsvga2_FindNearestRgbTripletIndex(const struct RgbTriplet* targetRgb,
												const struct RgbTriplet* palette, unsigned int startIndex,
												unsigned int endIndex) {
	int bestSquaredDistance = INT_MAX;
	unsigned int bestIndex = startIndex;
	unsigned int paletteIndex = startIndex;
	if (startIndex < endIndex) {
		int targetRed = targetRgb->r;
		int targetGreen = targetRgb->g;
		int targetBlue = targetRgb->b;
		for (; paletteIndex < endIndex; ++paletteIndex) {
			int redDifference = targetRed - palette[paletteIndex].r;
			int greenDifference = targetGreen - palette[paletteIndex].g;
			int blueDifference = targetBlue - palette[paletteIndex].b;
			int squaredDistance = redDifference * redDifference + greenDifference * greenDifference +
								  blueDifference * blueDifference;
			if (squaredDistance < bestSquaredDistance) {
				bestSquaredDistance = squaredDistance;
				bestIndex = paletteIndex;
			}
		}
	}
	return bestIndex;
}

// FUNCTION: XW 0x421B60
void rtsvga2_BuildRgb565ToPaletteIndexLut(uint8_t* destinationLut, unsigned int firstPaletteIndex,
										  unsigned int endPaletteIndex) {
	int rgb565;
	RgbTriplet targetRgb;
	for (rgb565 = 0; rgb565 < RTSVGA2_RGB565_LUT_SIZE; ++rgb565) {
		int redGreen = rgb565 >> RTSVGA2_RGB565_GREEN_SHIFT;
		targetRgb.g = redGreen & RTSVGA2_RGB6_MASK;
		targetRgb.r =
			((redGreen >> (RTSVGA2_RGB565_RED_SHIFT - RTSVGA2_RGB565_GREEN_SHIFT)) & RTSVGA2_RGB5_MASK) *
			RTSVGA2_RGB5_TO_RGB6_SCALE;
		targetRgb.b = (rgb565 & RTSVGA2_RGB5_MASK) * RTSVGA2_RGB5_TO_RGB6_SCALE;
		destinationLut[rgb565] = (uint8_t)rtsvga2_FindNearestRgbTripletIndex(
			&targetRgb, g_swPalette, firstPaletteIndex, endPaletteIndex);
	}
}

// FUNCTION: XW 0x421BC0
int rtsvga2_calcpositionVGA(uint16_t x, uint16_t y) {
#ifdef XW_MODERN
	return (int32_t)(x * (uint32_t)g_flightBytesPerPixel + y * (uint32_t)g_surfacePitch);
#else
	return g_flightBytesPerPixel * x + g_surfacePitch * y;
#endif
}

// FUNCTION: XW 0x421BF0
void rtsvga2_drawshapeVGA(const uint8_t* rleData, int16_t x, int16_t y, int16_t transparentColor,
						  int16_t mirror) {
#ifdef XW_MODERN
	XwHud_Sprite(rleData, x, y, transparentColor, mirror);
#endif

	g_flightSwRlePaletteShift = 0;
	rtsvga2__lowdrawshapeVGA(rleData, x, y, transparentColor, mirror, 0);
}

// FUNCTION: XW 0x421C20
void rtsvga2__lowdrawshapeVGA(const uint8_t* rleData, int16_t x, int16_t y, int16_t transparentColor,
							  int16_t mirror, uint8_t mode) {
	size_t sourceOffset = 0;
	g_flightSwRleSpriteEndMarker = transparentColor;
	g_flightSwRleSpriteX = x;
	g_flightSwRleSpriteY = y;
	for (;;) {
		unsigned int framebufferOffset =
			(uint16_t)g_flightSwRleSpriteX + g_flightLineOffsetTable[(uint16_t)g_flightSwRleSpriteY];
		uint8_t* rowPixels;
		ptrdiff_t pixelOffset = 0;
		if (g_flightResolutionMode != RTSVGA2_MODE_13H &&
			XwFramebufferAddress_IsLegacyBase(g_flightSwFramebufferBase)) {
			framebufferOffset %= (unsigned int)g_swFramebufferClearChunkSize;
			nullsub_SharedNoOp();
		}
		rowPixels = &g_flightSwFramebufferBase[framebufferOffset];
		for (;;) {
			uint8_t opcode = rleData[sourceOffset++];
			uint8_t color;
			int runCount;
			if (opcode < RTSVGA2_RLE_PALETTE_SHIFT) {
				color = opcode >> RTSVGA2_RLE_SHORT_COLOR_SHIFT;
				if (mode == 0)
					color += g_flightSwRlePaletteShift;
				runCount = (opcode & RTSVGA2_RLE_SHORT_COUNT_MASK) + 1;
			} else if (opcode == RTSVGA2_RLE_ALTERNATING_RUN && mode == 0) {
				int pixel;
				color = rleData[sourceOffset];
				runCount = rleData[sourceOffset + 1] + 1;
				sourceOffset += 2;
				for (pixel = 0; pixel < runCount; pixel += 2) {
					rowPixels[pixelOffset] = color;
					if (mirror == 0)
						++pixelOffset;
					else
						--pixelOffset;
					if (pixel + 1 < runCount) {
						rowPixels[pixelOffset] = color + 1;
						if (mirror == 0)
							++pixelOffset;
						else
							--pixelOffset;
					}
				}
				continue;
			} else if (opcode == RTSVGA2_RLE_LONG_RUN) {
				runCount = rleData[sourceOffset] + 1;
				color = rleData[sourceOffset + 1];
				sourceOffset += 2;
			} else if (opcode == RTSVGA2_RLE_PALETTE_SHIFT) {
				if (mode == 0)
					g_flightSwRlePaletteShift = rleData[sourceOffset];
				++sourceOffset;
				continue;
			} else if (opcode == RTSVGA2_RLE_END) {
				return;
			} else {
				g_flightSwRleSpriteY = (int16_t)((uint16_t)g_flightSwRleSpriteY + 1);
				break;
			}
			if (color == transparentColor) {
				if (mirror != 0)
					pixelOffset -= runCount;
				else
					pixelOffset += runCount;
			} else {
				if (mode != 0)
					color = g_flightSwRlePaletteShift;
				if (mirror == 0) {
					memset(&rowPixels[pixelOffset], color, runCount);
					pixelOffset += runCount;
				} else {
					int pixel;
					for (pixel = 0; pixel < runCount; ++pixel)
						rowPixels[pixelOffset - (ptrdiff_t)pixel] = color;
					pixelOffset -= runCount;
				}
			}
		}
	}
}

// FUNCTION: XW 0x421E10
uint8_t rtsvga2_drawdotVGA(uint16_t x, uint16_t y, uint8_t colorIndex) {
	unsigned int offset = x + g_flightLineOffsetTable[y];
	if (g_flightResolutionMode != RTSVGA2_MODE_13H &&
		XwFramebufferAddress_IsLegacyBase(g_flightSwFramebufferBase)) {
		unsigned int bankSize = g_swFramebufferClearChunkSize;
		offset %= bankSize;
		nullsub_SharedNoOp();
	}
	g_flightSwFramebufferBase[offset] = colorIndex;
	return colorIndex;
}

// FUNCTION: XW 0x421E80
void rtsvga2_outcharVGA(char character) {
	const uint8_t* glyph;
	uint8_t glyphWidth;
	int glyphWidthInt;
	uint8_t glyphHeight;
	int firstRow;
	int usePerRowAddressing;
	unsigned int firstRowOffset;
	int rowY;
	int rowIndex;
	uint8_t shadowBits;
	if (character == '\n') {
		if (g_flightClearLineBgEnabled != 0)
			rtsvga2_autofillVGA();
		g_flightCursorY += g_flightFontLineHeight;
		g_flightCursorX = g_flightClipLeft;
		return;
	}
	if ((int8_t)character < ' ')
		return;
	if (g_flightFontTier != 0 && g_flightFontHasLowercase == 0 && character >= 'a' && character <= 'z')
		character -= 'a' - 'A';
	glyph = &g_flightFontGlyphTableSw[g_flightFontGlyphStrideSw * (int8_t)(character - ' ')];
	glyphWidth = glyph[0];
	glyphHeight = glyph[1];
	glyphWidthInt = glyphWidth;
	if (glyphWidthInt + (uint16_t)g_flightCursorX >= g_flightClipRight && g_flightWordWrapEnabled != 0) {
		if (g_flightClearLineBgEnabled != 0)
			rtsvga2_autofillVGA();
		g_flightCursorY += glyphHeight;
		g_flightCursorX = g_flightClipLeft;
	}
#ifdef XW_MODERN
	XwHud_Glyph(character, glyphWidth, glyphHeight);
#endif
	firstRow = (uint16_t)g_flightCursorY;
	shadowBits = 0;
	usePerRowAddressing = 0;
	if (firstRow < g_flightClipTop)
		firstRow = g_flightClipTop;
	firstRowOffset = g_flightLineOffsetTable[firstRow];
	if (g_flightResolutionMode != RTSVGA2_MODE_13H &&
		XwFramebufferAddress_IsLegacyBase(g_flightSwFramebufferBase)) {
		int bottom;
		firstRowOffset %= (unsigned int)g_swFramebufferClearChunkSize;
		nullsub_SharedNoOp();
		bottom = glyphHeight + (uint16_t)g_flightCursorY + 1;
		if (bottom > g_flightClipBottom)
			bottom = g_flightClipBottom;
		if ((int32_t)(firstRowOffset + (uint32_t)g_surfacePitch * (bottom - firstRow)) >
			RTSVGA2_BANK_MAX_OFFSET)
			usePerRowAddressing = 1;
	}
	if (!XwFramebufferAddress_IsLegacyBase(g_flightSwFramebufferBase))
		usePerRowAddressing = 1;
	if (!usePerRowAddressing) {
		uint8_t* framebuffer = g_flightSwFramebufferBase + firstRowOffset;
		ptrdiff_t framebufferRowOffset = 0;
		for (rowY = (uint16_t)g_flightCursorY, rowIndex = 0; rowY < glyphHeight + (uint16_t)g_flightCursorY;
			 ++rowY, ++rowIndex) {
			uint8_t rowBits;
			int clippedWidth;
			int drawX;
			rowBits = glyph[FLIGHT_FONT_GLYPH_HEADER_SIZE + rowIndex * RTSVGA2_NARROW_GLYPH_ROW_SIZE];
			clippedWidth = glyphWidthInt;
			if (g_flightTextShadowEnabled != 0)
				++clippedWidth;
			drawX = (uint16_t)g_flightCursorX;
			if (drawX < g_flightClipLeft) {
				int clippedPixels = g_flightClipLeft - drawX;
				if (clippedPixels >= clippedWidth)
					break;
				clippedWidth += drawX - g_flightClipLeft;
				drawX = g_flightClipLeft;
				rowBits = (uint32_t)rowBits << (clippedPixels & FLIGHT_FONT_GLYPH_SHIFT_MASK);
			}
			if (rowY >= g_flightClipBottom)
				break;
			if (rowY >= g_flightClipTop) {
				uint8_t* pixels;
				int pixelIndex;
				int remaining;
				if (clippedWidth + drawX > g_flightClipRight) {
					clippedWidth = g_flightClipRight - drawX;
					if (clippedWidth <= 0)
						break;
				}
				pixels = framebuffer + framebufferRowOffset + drawX;
				for (pixelIndex = 0, remaining = clippedWidth; remaining != 0; ++pixelIndex, --remaining) {
					if (rowBits & RTSVGA2_NARROW_GLYPH_HIGH_BIT)
						pixels[pixelIndex] = g_flightTextColorIndex;
					else if (g_flightTextShadowEnabled != 0 && (shadowBits & RTSVGA2_NARROW_GLYPH_HIGH_BIT))
						pixels[pixelIndex] = g_flightTextShadowColor;
					else
						pixels[pixelIndex] = g_flightTextBgColor;
					rowBits <<= 1;
					shadowBits <<= 1;
				}
				framebufferRowOffset += g_surfacePitch;
			}
			shadowBits = glyph[FLIGHT_FONT_GLYPH_HEADER_SIZE + rowIndex * RTSVGA2_NARROW_GLYPH_ROW_SIZE] >> 1;
		}
	} else {
		for (rowY = (uint16_t)g_flightCursorY, rowIndex = 0; rowY < glyphHeight + (uint16_t)g_flightCursorY;
			 ++rowY, ++rowIndex) {
			uint8_t rowBits;
			int clippedWidth;
			int drawX;
			rowBits = glyph[FLIGHT_FONT_GLYPH_HEADER_SIZE + rowIndex * RTSVGA2_NARROW_GLYPH_ROW_SIZE];
			clippedWidth = glyphWidthInt;
			if (g_flightTextShadowEnabled != 0)
				++clippedWidth;
			drawX = (uint16_t)g_flightCursorX;
			if (drawX < g_flightClipLeft) {
				int clippedPixels = g_flightClipLeft - drawX;
				if (clippedPixels >= clippedWidth)
					break;
				clippedWidth += drawX - g_flightClipLeft;
				drawX = g_flightClipLeft;
				rowBits = (uint32_t)rowBits << (clippedPixels & FLIGHT_FONT_GLYPH_SHIFT_MASK);
			}
			if (rowY >= g_flightClipBottom)
				break;
			if (rowY >= g_flightClipTop) {
				unsigned int pixelOffset;
				uint8_t* pixels;
				int pixelIndex;
				int remaining;
				if (clippedWidth + drawX > g_flightClipRight) {
					clippedWidth = g_flightClipRight - drawX;
					if (clippedWidth <= 0)
						break;
				}
				pixelOffset = g_flightLineOffsetTable[rowY] + drawX;
				if (g_flightResolutionMode != RTSVGA2_MODE_13H &&
					XwFramebufferAddress_IsLegacyBase(g_flightSwFramebufferBase)) {
					pixelOffset %= (unsigned int)g_swFramebufferClearChunkSize;
					nullsub_SharedNoOp();
				}
				pixels = g_flightSwFramebufferBase + pixelOffset;
				for (pixelIndex = 0, remaining = clippedWidth; remaining != 0; ++pixelIndex, --remaining) {
					if (rowBits & RTSVGA2_NARROW_GLYPH_HIGH_BIT)
						pixels[pixelIndex] = g_flightTextColorIndex;
					else if (g_flightTextShadowEnabled != 0 && (shadowBits & RTSVGA2_NARROW_GLYPH_HIGH_BIT))
						pixels[pixelIndex] = g_flightTextShadowColor;
					else
						pixels[pixelIndex] = g_flightTextBgColor;
					rowBits <<= 1;
					shadowBits <<= 1;
				}
			}
			shadowBits = glyph[FLIGHT_FONT_GLYPH_HEADER_SIZE + rowIndex * RTSVGA2_NARROW_GLYPH_ROW_SIZE] >> 1;
		}
	}
	g_flightCursorX = (uint16_t)g_flightCursorX + glyphWidth;
	if ((uint16_t)g_flightCursorX >= g_flightClipRight && g_flightWordWrapEnabled != 0) {
		if (g_flightClearLineBgEnabled != 0)
			rtsvga2_autofillVGA();
		g_flightCursorY += glyphHeight;
		g_flightCursorX = g_flightClipLeft;
	}
}

// FUNCTION: XW 0x422350
void rtsvga2_outchar32VGA(char character) {
	const uint8_t* glyph;
	uint8_t glyphWidth;
	uint8_t glyphHeight;
	int firstRow;
	int usePerRowAddressing;
	unsigned int firstRowOffset;
	int rowY;
	int rowIndex;
	uint32_t shadowBits;
	if (character == '\n') {
		if (g_flightClearLineBgEnabled != 0)
			rtsvga2_autofillVGA();
		g_flightCursorY += g_flightFontLineHeight;
		g_flightCursorX = g_flightClipLeft;
		return;
	}
	if ((int8_t)character < ' ')
		return;
	if (g_flightFontTier != 0 && g_flightFontHasLowercase == 0 && character >= 'a' && character <= 'z')
		character -= 'a' - 'A';
	glyph = &g_flightFontGlyphTableSw[g_flightFontGlyphStrideSw * (int8_t)(character - ' ')];
	glyphWidth = glyph[0];
	glyphHeight = glyph[1];
	if (glyphWidth + (uint16_t)g_flightCursorX >= g_flightClipRight && g_flightWordWrapEnabled != 0) {
		if (g_flightClearLineBgEnabled != 0)
			rtsvga2_autofillVGA();
		g_flightCursorY += glyphHeight;
		g_flightCursorX = g_flightClipLeft;
	}
#ifdef XW_MODERN
	XwHud_Glyph(character, glyphWidth, glyphHeight);
#endif
	firstRow = (uint16_t)g_flightCursorY;
	usePerRowAddressing = 0;
	shadowBits = 0;
	if (firstRow < g_flightClipTop)
		firstRow = g_flightClipTop;
	firstRowOffset = g_flightLineOffsetTable[firstRow];
	if (g_flightResolutionMode != RTSVGA2_MODE_13H &&
		XwFramebufferAddress_IsLegacyBase(g_flightSwFramebufferBase)) {
		int bottom;
		firstRowOffset %= (unsigned int)g_swFramebufferClearChunkSize;
		nullsub_SharedNoOp();
		bottom = glyphHeight + (uint16_t)g_flightCursorY + 1;
		if (bottom > g_flightClipBottom)
			bottom = g_flightClipBottom;
		if ((int32_t)(firstRowOffset + (uint32_t)g_surfacePitch * (bottom - firstRow)) >
			RTSVGA2_BANK_MAX_OFFSET)
			usePerRowAddressing = 1;
	}
	if (!XwFramebufferAddress_IsLegacyBase(g_flightSwFramebufferBase))
		usePerRowAddressing = 1;
	if (!usePerRowAddressing) {
		uint8_t* framebuffer = g_flightSwFramebufferBase + firstRowOffset;
		ptrdiff_t framebufferRowOffset = 0;
		for (rowY = (uint16_t)g_flightCursorY, rowIndex = 0; rowY < glyphHeight + (uint16_t)g_flightCursorY;
			 ++rowY, ++rowIndex) {
			uint32_t rowBits;
			uint32_t originalRowBits;
			int clippedWidth;
			int drawX;
			memcpy(&rowBits, &glyph[FLIGHT_FONT_GLYPH_HEADER_SIZE + rowIndex * FLIGHT_FONT_GLYPH_ROW_SIZE],
				   sizeof(rowBits));
			clippedWidth = glyphWidth;
			if (g_flightTextShadowEnabled != 0)
				++clippedWidth;
			drawX = (uint16_t)g_flightCursorX;
			if (drawX < g_flightClipLeft) {
				int clippedPixels = g_flightClipLeft - drawX;
				if (clippedPixels >= clippedWidth)
					break;
				clippedWidth += drawX - g_flightClipLeft;
				drawX = g_flightClipLeft;
				rowBits <<= clippedPixels & FLIGHT_FONT_GLYPH_SHIFT_MASK;
			}
			if (rowY >= g_flightClipBottom)
				break;
			if (rowY >= g_flightClipTop) {
				uint8_t* pixels;
				int pixelIndex;
				if (clippedWidth + drawX > g_flightClipRight) {
					clippedWidth = g_flightClipRight - drawX;
					if (clippedWidth <= 0)
						break;
				}
				pixels = framebuffer + framebufferRowOffset + drawX;
				for (pixelIndex = 0; clippedWidth-- != 0; ++pixelIndex) {
					if (rowBits & (1UL << FLIGHT_FONT_GLYPH_SHIFT_MASK))
						pixels[pixelIndex] = g_flightTextColorIndex;
					else if (g_flightTextShadowEnabled != 0 &&
							 (shadowBits & (1UL << FLIGHT_FONT_GLYPH_SHIFT_MASK)))
						pixels[pixelIndex] = g_flightTextShadowColor;
					else
						pixels[pixelIndex] = g_flightTextBgColor;
					rowBits <<= 1;
					shadowBits <<= 1;
				}
				framebufferRowOffset += g_surfacePitch;
			}
			memcpy(&originalRowBits,
				   &glyph[FLIGHT_FONT_GLYPH_HEADER_SIZE + rowIndex * FLIGHT_FONT_GLYPH_ROW_SIZE],
				   sizeof(originalRowBits));
			shadowBits = originalRowBits >> 1;
		}
	} else {
		for (rowY = (uint16_t)g_flightCursorY, rowIndex = 0; rowY < glyphHeight + (uint16_t)g_flightCursorY;
			 ++rowY, ++rowIndex) {
			uint32_t rowBits;
			uint32_t originalRowBits;
			int clippedWidth;
			int drawX;
			memcpy(&rowBits, &glyph[FLIGHT_FONT_GLYPH_HEADER_SIZE + rowIndex * FLIGHT_FONT_GLYPH_ROW_SIZE],
				   sizeof(rowBits));
			clippedWidth = glyphWidth;
			if (g_flightTextShadowEnabled != 0)
				++clippedWidth;
			drawX = (uint16_t)g_flightCursorX;
			if (drawX < g_flightClipLeft) {
				int clippedPixels = g_flightClipLeft - drawX;
				if (clippedPixels >= clippedWidth)
					break;
				clippedWidth += drawX - g_flightClipLeft;
				drawX = g_flightClipLeft;
				rowBits <<= clippedPixels & FLIGHT_FONT_GLYPH_SHIFT_MASK;
			}
			if (rowY >= g_flightClipBottom)
				break;
			if (rowY >= g_flightClipTop) {
				unsigned int pixelOffset;
				uint8_t* pixels;
				int pixelIndex;
				if (clippedWidth + drawX > g_flightClipRight) {
					clippedWidth = g_flightClipRight - drawX;
					if (clippedWidth <= 0)
						break;
				}
				pixelOffset = g_flightLineOffsetTable[rowY] + drawX;
				if (g_flightResolutionMode != RTSVGA2_MODE_13H &&
					XwFramebufferAddress_IsLegacyBase(g_flightSwFramebufferBase)) {
					pixelOffset %= (unsigned int)g_swFramebufferClearChunkSize;
					nullsub_SharedNoOp();
				}
				pixels = g_flightSwFramebufferBase + pixelOffset;
				for (pixelIndex = 0; clippedWidth-- != 0; ++pixelIndex) {
					if (rowBits & (1UL << FLIGHT_FONT_GLYPH_SHIFT_MASK))
						pixels[pixelIndex] = g_flightTextColorIndex;
					else if (g_flightTextShadowEnabled != 0 &&
							 (shadowBits & (1UL << FLIGHT_FONT_GLYPH_SHIFT_MASK)))
						pixels[pixelIndex] = g_flightTextShadowColor;
					else
						pixels[pixelIndex] = g_flightTextBgColor;
					rowBits <<= 1;
					shadowBits <<= 1;
				}
			}
			memcpy(&originalRowBits,
				   &glyph[FLIGHT_FONT_GLYPH_HEADER_SIZE + rowIndex * FLIGHT_FONT_GLYPH_ROW_SIZE],
				   sizeof(originalRowBits));
			shadowBits = originalRowBits >> 1;
		}
	}
	g_flightCursorX = (uint16_t)g_flightCursorX + glyphWidth;
	if ((uint16_t)g_flightCursorX >= g_flightClipRight && g_flightWordWrapEnabled != 0) {
		if (g_flightClearLineBgEnabled != 0)
			rtsvga2_autofillVGA();
		g_flightCursorY += glyphHeight;
		g_flightCursorX = g_flightClipLeft;
	}
}

// FUNCTION: XW 0x422800
void rtsvga2_clearwindowVGA(void) {
	g_flightFillRectBottom8bpp = g_flightClipBottom;
	g_flightFillRectTop8bpp = g_flightClipTop;
	g_flightFillRectLeft8bpp = g_flightClipLeft;
	g_flightFillRectRight8bpp = g_flightClipRight;
	rtsvga2_fillrectangleVGA();
}

// FUNCTION: XW 0x422840
void rtsvga2_fillrectangleVGA(void) {
	unsigned int firstRowOffset = g_flightLineOffsetTable[g_flightFillRectTop8bpp];
	int useScanlineTable = 0;
#ifdef XW_MODERN
	XwHud_Fill(g_flightFillRectLeft8bpp, g_flightFillRectTop8bpp, g_flightFillRectRight8bpp,
			   g_flightFillRectBottom8bpp);
#endif
	g_flightFillRectRemainingRows8bpp = g_flightFillRectBottom8bpp - g_flightFillRectTop8bpp;
	g_flightFillRectCurrentY8bpp = g_flightFillRectTop8bpp;
	if (g_flightResolutionMode != RTSVGA2_MODE_13H &&
		XwFramebufferAddress_IsLegacyBase(g_flightSwFramebufferBase)) {
		firstRowOffset %= (unsigned int)g_swFramebufferClearChunkSize;
		nullsub_SharedNoOp();
		if (firstRowOffset + (unsigned int)g_surfacePitch * g_flightFillRectRemainingRows8bpp >
			RTSVGA2_BANK_MAX_OFFSET)
			useScanlineTable = 1;
	}
	if (!XwFramebufferAddress_IsLegacyBase(g_flightSwFramebufferBase))
		useScanlineTable = 1;
	if (!useScanlineTable) {
		uint8_t* framebuffer = g_flightSwFramebufferBase + firstRowOffset;
		unsigned int rowOffset;
		for (rowOffset = 0; g_flightFillRectRemainingRows8bpp > 0; rowOffset += g_surfacePitch,
			--g_flightFillRectRemainingRows8bpp, ++g_flightFillRectCurrentY8bpp) {
			uint8_t* pixels = framebuffer + rowOffset + g_flightFillRectLeft8bpp;
			int16_t width = g_flightFillRectRight8bpp - g_flightFillRectLeft8bpp;
			int pixelIndex;
			if (width <= 0)
				break;
			for (pixelIndex = 0; width-- > 0; ++pixelIndex)
				pixels[pixelIndex] = g_flightTextBgColor;
		}
	} else {
		for (; g_flightFillRectRemainingRows8bpp > 0;
			 --g_flightFillRectRemainingRows8bpp, ++g_flightFillRectCurrentY8bpp) {
			unsigned int rowOffset =
				g_flightFillRectLeft8bpp + g_flightLineOffsetTable[g_flightFillRectCurrentY8bpp];
			uint8_t* pixels;
			int16_t width;
			int pixelIndex;
			if (g_flightResolutionMode != RTSVGA2_MODE_13H &&
				XwFramebufferAddress_IsLegacyBase(g_flightSwFramebufferBase)) {
				rowOffset %= (unsigned int)g_swFramebufferClearChunkSize;
				nullsub_SharedNoOp();
			}
			width = g_flightFillRectRight8bpp - g_flightFillRectLeft8bpp;
			pixels = g_flightSwFramebufferBase + rowOffset;
			if (width <= 0)
				break;
			for (pixelIndex = 0; width-- > 0; ++pixelIndex)
				pixels[pixelIndex] = g_flightTextBgColor;
		}
	}
}

// FUNCTION: XW 0x4229F0
void rtsvga2_fillboxVGA(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
	g_flightFillRectLeft8bpp = x1;
	g_flightFillRectTop8bpp = y1;
	g_flightFillRectRight8bpp = x2;
	g_flightFillRectBottom8bpp = y2;
	if (x1 < g_flightClipLeft) {
		x1 = g_flightClipLeft;
		g_flightFillRectLeft8bpp = x1;
	}
	if (x2 > g_flightClipRight) {
		x2 = g_flightClipRight;
		g_flightFillRectRight8bpp = x2;
	}
	if (y1 < g_flightClipTop) {
		y1 = g_flightClipTop;
		g_flightFillRectTop8bpp = y1;
	}
	if (y2 > g_flightClipBottom) {
		y2 = g_flightClipBottom;
		g_flightFillRectBottom8bpp = y2;
	}
	if (y2 > y1 && x2 > x1)
		rtsvga2_fillrectangleVGA();
}

// FUNCTION: XW 0x422A90
void rtsvga2_autofillVGA(void) {
	if (g_flightClipRight > (uint16_t)g_flightCursorX) {
		g_flightFillRectRight8bpp = g_flightClipRight;
		g_flightFillRectLeft8bpp = g_flightCursorX;
		g_flightFillRectTop8bpp = g_flightCursorY;
		g_flightFillRectBottom8bpp = g_flightCursorY + g_flightFontLineHeight;
		if (g_flightFillRectLeft8bpp < g_flightClipLeft)
			g_flightFillRectLeft8bpp = g_flightClipLeft;
		if (g_flightFillRectTop8bpp < g_flightClipTop)
			g_flightFillRectTop8bpp = g_flightClipTop;
		if (g_flightFillRectBottom8bpp > g_flightClipBottom)
			g_flightFillRectBottom8bpp = g_flightClipBottom;
		if (g_flightFillRectBottom8bpp > g_flightFillRectTop8bpp)
			rtsvga2_fillrectangleVGA();
	}
}

// FUNCTION: XW 0x422B20
void rtsvga2_saveboxVGA(uint8_t* buffer, uint16_t xByteOffset, uint16_t y, uint16_t width, uint16_t height) {
	uint16_t rowsRemaining;
	ptrdiff_t destinationOffset = 0;
#ifdef XW_MODERN
	XwHud_Save(buffer, xByteOffset, y, width, height);
#endif
	for (rowsRemaining = height; rowsRemaining != 0; --rowsRemaining, ++y) {
		unsigned int framebufferOffset = xByteOffset + g_flightLineOffsetTable[y];
		if (g_flightResolutionMode != RTSVGA2_MODE_13H &&
			XwFramebufferAddress_IsLegacyBase(g_flightSwFramebufferBase)) {
			framebufferOffset %= (unsigned int)g_swFramebufferClearChunkSize;
			nullsub_SharedNoOp();
			nullsub_SharedNoOp();
		}
		memcpy(buffer + destinationOffset, g_flightSwFramebufferBase + framebufferOffset,
			   width * (unsigned int)g_flightBytesPerPixel);
		destinationOffset += g_surfacePitch;
	}
}

// FUNCTION: XW 0x422C00
void rtsvga2_restoreboxVGA(const uint8_t* buffer, uint16_t xByteOffset, uint16_t y, uint16_t width,
						   uint16_t height) {
	uint16_t row;
	ptrdiff_t sourceOffset;
#ifdef XW_MODERN
	XwHud_Restore(buffer);
#endif
	for (row = height, sourceOffset = 0; row != 0; --row, ++y) {
		unsigned int framebufferOffset = xByteOffset + g_flightLineOffsetTable[y];
		if (g_flightResolutionMode != RTSVGA2_MODE_13H &&
			XwFramebufferAddress_IsLegacyBase(g_flightSwFramebufferBase)) {
			framebufferOffset %= (unsigned int)g_swFramebufferClearChunkSize;
			nullsub_SharedNoOp();
		}
		memcpy(g_flightSwFramebufferBase + framebufferOffset, buffer + sourceOffset,
			   width * (unsigned int)g_flightBytesPerPixel);
		sourceOffset += g_surfacePitch;
	}
}

// FUNCTION: XW 0x422CD0
void rtsvga2_drawblips(struct XwRadarBlip* blips, int count) {
#ifdef XW_MODERN
	if (XwFlightTypes_Dos()) {
		Dos94_rtsvga2_drawblips(blips, count);
		return;
	}
#endif
	if (g_flightBytesPerPixel == sizeof(uint16_t)) {
		rtsrgb_drawblipsRGB(blips, count);
	} else {
		uint16_t index;
		for (index = 0; index < (uint16_t)count; ++index) {
			uint8_t color = (uint8_t)blips[index].colorOrDrawMask;
			unsigned int offset = blips[index].x + g_flightLineOffsetTable[blips[index].y];
			uint8_t* pixel;
			if (g_flightResolutionMode != RTSVGA2_MODE_13H &&
				XwFramebufferAddress_IsLegacyBase(g_flightSwFramebufferBase)) {
				offset %= (unsigned int)g_swFramebufferClearChunkSize;
				nullsub_SharedNoOp();
				nullsub_SharedNoOp();
			}
			pixel = &g_flightSwFramebufferBase[offset];
			if (*pixel != RTSVGA2_RADAR_BACKGROUND_COLOR) {
				blips[index].colorOrDrawMask = 0;
			} else {
				*pixel = color;
				blips[index].colorOrDrawMask = RTSVGA2_BLIP_FIRST_PIXEL;
			}
			if (g_flightResolutionMode != RTSVGA2_MODE_13H) {
				offset = blips[index].x + g_flightLineOffsetTable[(uint16_t)(blips[index].y + 1)];
				if (XwFramebufferAddress_IsLegacyBase(g_flightSwFramebufferBase)) {
					offset %= (unsigned int)g_swFramebufferClearChunkSize;
					nullsub_SharedNoOp();
					nullsub_SharedNoOp();
				}
				pixel = &g_flightSwFramebufferBase[offset];
				if (*pixel != RTSVGA2_RADAR_BACKGROUND_COLOR) {
					blips[index].colorOrDrawMask &= RTSVGA2_BLIP_FIRST_PIXEL;
				} else {
					*pixel = color;
					blips[index].colorOrDrawMask |= RTSVGA2_BLIP_SECOND_PIXEL;
				}
			}
		}
	}
}

// FUNCTION: XW 0x422E20
void rtsvga2_removeblips(struct XwRadarBlip* blips, int count) {
#ifdef XW_MODERN
	if (XwFlightTypes_Dos()) {
		Dos94_rtsvga2_removeblips(blips, count);
		return;
	}
#endif
	if (g_flightBytesPerPixel == sizeof(uint16_t)) {
		rtsrgb_removeblipsRGB(blips, count);
	} else {
		uint16_t index;
		for (index = 0; index < (uint16_t)count; ++index) {
			unsigned int offset = blips[index].x + g_flightLineOffsetTable[blips[index].y];
			uint8_t* pixel;
			if (g_flightResolutionMode != RTSVGA2_MODE_13H &&
				XwFramebufferAddress_IsLegacyBase(g_flightSwFramebufferBase)) {
				offset %= (unsigned int)g_swFramebufferClearChunkSize;
				nullsub_SharedNoOp();
			}
			pixel = &g_flightSwFramebufferBase[offset];
			if ((blips[index].colorOrDrawMask & RTSVGA2_BLIP_FIRST_PIXEL) != 0) {
				*pixel = RTSVGA2_RADAR_BACKGROUND_COLOR;
			}
			if (g_flightResolutionMode != RTSVGA2_MODE_13H) {
				offset = blips[index].x + g_flightLineOffsetTable[(uint16_t)(blips[index].y + 1)];
				if (XwFramebufferAddress_IsLegacyBase(g_flightSwFramebufferBase)) {
					offset %= (unsigned int)g_swFramebufferClearChunkSize;
					nullsub_SharedNoOp();
				}
				pixel = &g_flightSwFramebufferBase[offset];
				if ((blips[index].colorOrDrawMask & RTSVGA2_BLIP_SECOND_PIXEL) != 0) {
					*pixel = RTSVGA2_RADAR_BACKGROUND_COLOR;
				}
			}
		}
	}
}

// FUNCTION: XW 0x422F40
void rtsvga2_drawbracket(void) {
#ifdef XW_MODERN
	XwHud_Marker(false, true, g_radarSelectedTargetScreenX, g_radarSelectedTargetScreenY, 7);
#endif

#ifdef XW_MODERN
	if (XwFlightTypes_Dos()) {
		Dos94_rtsvga2_drawbracket();
		return;
	}
#endif
	if (g_flightBytesPerPixel == sizeof(uint16_t)) {
		rtsrgb_drawbracketRGB();
	} else {
		uint16_t pixelIndex;
		uint16_t markerByteOffset = 0;
		uint16_t remaining;
		for (pixelIndex = 0, remaining = (uint16_t)g_flightBracketOffsetCount; remaining != 0;
			 ++pixelIndex, markerByteOffset += sizeof(FlightMarkerOffset), --remaining) {
			const FlightMarkerOffset* markerOffset =
				&g_flightBracketOffsets[markerByteOffset / sizeof(FlightMarkerOffset)];
			unsigned int framebufferOffset =
				g_flightLineOffsetTable[(uint16_t)g_radarSelectedTargetScreenY + markerOffset->y] +
				(uint16_t)g_radarSelectedTargetScreenX + markerOffset->x;
			uint8_t* pixel;
			if (g_flightResolutionMode != RTSVGA2_MODE_13H &&
				XwFramebufferAddress_IsLegacyBase(g_flightSwFramebufferBase)) {
				framebufferOffset %= (unsigned int)g_swFramebufferClearChunkSize;
				nullsub_SharedNoOp();
				nullsub_SharedNoOp();
			}
			pixel = &g_flightSwFramebufferBase[framebufferOffset];
			g_radarBracketSavedPixels8[pixelIndex] = *pixel;
			*pixel = RTSVGA2_BRACKET_COLOR;
		}
	}
}

// FUNCTION: XW 0x423030
void rtsvga2_removebracket(void) {
#ifdef XW_MODERN
	XwHud_Marker(false, false, 0, 0, 7);
#endif

#ifdef XW_MODERN
	if (XwFlightTypes_Dos()) {
		Dos94_rtsvga2_removebracket();
		return;
	}
#endif
	if (g_flightBytesPerPixel == sizeof(uint16_t)) {
		rtsrgb_removebracketRGB();
	} else {
		unsigned int pixelIndex;
		uint16_t remaining;
		for (pixelIndex = 0, remaining = (uint16_t)g_flightBracketOffsetCount; remaining != 0;
			 ++pixelIndex, --remaining) {
			const FlightMarkerOffset* markerOffset = &g_flightBracketOffsets[pixelIndex];
			unsigned int framebufferOffset =
				g_flightLineOffsetTable[g_radarPreviousTargetScreenY + markerOffset->y] +
				g_radarPreviousTargetScreenX + markerOffset->x;
			if (g_flightResolutionMode != RTSVGA2_MODE_13H &&
				XwFramebufferAddress_IsLegacyBase(g_flightSwFramebufferBase)) {
				framebufferOffset %= (unsigned int)g_swFramebufferClearChunkSize;
				nullsub_SharedNoOp();
			}
			g_flightSwFramebufferBase[framebufferOffset] = g_radarBracketSavedPixels8[pixelIndex];
		}
	}
}

// FUNCTION: XW 0x423110
void rtsvga2_drawcross(uint16_t x, uint16_t y, uint8_t paletteIndex) {
#ifdef XW_MODERN
	XwHud_Marker(true, true, x, y, paletteIndex);
#endif

#ifdef XW_MODERN
	if (XwFlightTypes_Dos()) {
		Dos94_rtsvga2_drawcross(x, y, paletteIndex);
		return;
	}
#endif
	if (g_flightBytesPerPixel == sizeof(uint16_t)) {
		rtsrgb_drawcrossRGB(x, y, paletteIndex);
	} else {
		uint16_t index;
		int remaining;
		for (index = 0, remaining = RTSVGA2_CROSS_PIXEL_COUNT; remaining != 0; ++index, --remaining) {
			unsigned int framebufferOffset = g_flightLineOffsetTable[y + g_crossMarkerOffsets8[index].y] +
											 g_crossMarkerOffsets8[index].x + x;
			uint8_t* pixel;
			if (g_flightResolutionMode != RTSVGA2_MODE_13H &&
				XwFramebufferAddress_IsLegacyBase(g_flightSwFramebufferBase)) {
				framebufferOffset %= (unsigned int)g_swFramebufferClearChunkSize;
				nullsub_SharedNoOp();
				nullsub_SharedNoOp();
			}
			pixel = &g_flightSwFramebufferBase[framebufferOffset];
			g_crossMarkerSavedPixels8[index] = *pixel;
			*pixel = paletteIndex;
		}
	}
}

// FUNCTION: XW 0x423210
void rtsvga2_removecross(uint16_t x, uint16_t y) {
#ifdef XW_MODERN
	XwHud_Marker(true, false, 0, 0, 0);
#endif

#ifdef XW_MODERN
	if (XwFlightTypes_Dos()) {
		Dos94_rtsvga2_removecross(x, y);
		return;
	}
#endif
	if (g_flightBytesPerPixel == sizeof(uint16_t)) {
		rtsrgb_removecrossRGB(x, y);
	} else {
		uint16_t index;
		int remaining;
		for (index = 0, remaining = RTSVGA2_CROSS_PIXEL_COUNT; remaining != 0; ++index, --remaining) {
			unsigned int framebufferOffset = g_flightLineOffsetTable[y + g_crossMarkerOffsets8[index].y] +
											 g_crossMarkerOffsets8[index].x + x;
			if (g_flightResolutionMode != RTSVGA2_MODE_13H &&
				XwFramebufferAddress_IsLegacyBase(g_flightSwFramebufferBase)) {
				framebufferOffset %= (unsigned int)g_swFramebufferClearChunkSize;
				nullsub_SharedNoOp();
			}
			g_flightSwFramebufferBase[framebufferOffset] = g_crossMarkerSavedPixels8[index];
		}
	}
}
