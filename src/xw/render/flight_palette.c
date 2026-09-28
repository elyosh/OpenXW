#include "xw/render/flight_palette.h"

#include "xw/flight/flight_display.h"
#include "xw/flight/player/user.h"
#include "xw/render/rtsrgb.h"
#include "xw/render/rtsvga2.h"

#include <stdlib.h>

// FUNCTION: XW 0x41FC10
void FlightPalette_Build16BppRange(const struct RgbTriplet* sourceRgb6, uint16_t* destinationPixels,
								   int firstIndex, int entryCount) {
	unsigned int entry;
	unsigned int remaining;
	if (g_flightBrightnessScaleQ8 == RTSVGA2_BRIGHTNESS_IDENTITY) {
		if (firstIndex < (int32_t)((uint32_t)firstIndex + (uint32_t)entryCount)) {
			for (entry = 0, remaining = entryCount; remaining != 0; ++entry, --remaining) {
				if (!FlightDisplay_IsPixelFormat555()) {
					RgbTriplet color;
					color.r = sourceRgb6[firstIndex + entry].r;
					color.g = sourceRgb6[firstIndex + entry].g;
					color.b = sourceRgb6[firstIndex + entry].b;
					if (&color == g_swPalette)
						destinationPixels[firstIndex + entry] = g_flightTextPalette[0];
					else if (FlightDisplay_IsPixelFormat555())
						destinationPixels[firstIndex + entry] =
							((color.r >> FLIGHT_PALETTE_RGB6_TO_RGB5_SHIFT)
							 << FLIGHT_PALETTE_RGB555_RED_SHIFT) |
							((color.g >> FLIGHT_PALETTE_RGB6_TO_RGB5_SHIFT) << RTSVGA2_RGB565_GREEN_SHIFT) |
							(color.b >> FLIGHT_PALETTE_RGB6_TO_RGB5_SHIFT);
					else
						destinationPixels[firstIndex + entry] =
							((color.r >> FLIGHT_PALETTE_RGB6_TO_RGB5_SHIFT) << RTSVGA2_RGB565_RED_SHIFT) |
							(color.g << RTSVGA2_RGB565_GREEN_SHIFT) |
							(color.b >> FLIGHT_PALETTE_RGB6_TO_RGB5_SHIFT);
				} else {
					destinationPixels[firstIndex + entry] =
						(sourceRgb6[firstIndex + entry].b >> FLIGHT_PALETTE_RGB6_TO_RGB5_SHIFT) |
						((sourceRgb6[firstIndex + entry].r & FLIGHT_PALETTE_DIRECT_RED_MASK)
						 << FLIGHT_PALETTE_DIRECT_RED_SHIFT) |
						((sourceRgb6[firstIndex + entry].g & FLIGHT_PALETTE_DIRECT_GREEN_MASK)
						 << FLIGHT_PALETTE_DIRECT_GREEN_SHIFT);
				}
			}
		}
	} else {
		for (entry = 0, remaining = entryCount; remaining != 0; ++entry, --remaining) {
			uint8_t red = sourceRgb6[firstIndex + entry].r;
			uint8_t green = sourceRgb6[firstIndex + entry].g;
			uint8_t blue = sourceRgb6[firstIndex + entry].b;
			uint8_t outputRed = red;
			uint8_t outputGreen = green;
			uint8_t outputBlue = blue;
			uint8_t maxChannel, minChannel, saturation, hueFraction;
			uint8_t scaledValue;
			int8_t hueSector;
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
			if (!FlightDisplay_IsPixelFormat555()) {
				RgbTriplet color;
				color.r = outputRed;
				color.g = outputGreen;
				color.b = outputBlue;
				if (&color == g_swPalette)
					destinationPixels[firstIndex + entry] = g_flightTextPalette[0];
				else if (FlightDisplay_IsPixelFormat555())
					destinationPixels[firstIndex + entry] =
						((color.r >> FLIGHT_PALETTE_RGB6_TO_RGB5_SHIFT) << FLIGHT_PALETTE_RGB555_RED_SHIFT) |
						((color.g >> FLIGHT_PALETTE_RGB6_TO_RGB5_SHIFT) << RTSVGA2_RGB565_GREEN_SHIFT) |
						(color.b >> FLIGHT_PALETTE_RGB6_TO_RGB5_SHIFT);
				else
					destinationPixels[firstIndex + entry] =
						((color.r >> FLIGHT_PALETTE_RGB6_TO_RGB5_SHIFT) << RTSVGA2_RGB565_RED_SHIFT) |
						(color.g << RTSVGA2_RGB565_GREEN_SHIFT) |
						(color.b >> FLIGHT_PALETTE_RGB6_TO_RGB5_SHIFT);
			} else {
				destinationPixels[firstIndex + entry] =
					(outputBlue >> FLIGHT_PALETTE_RGB6_TO_RGB5_SHIFT) |
					((outputRed & FLIGHT_PALETTE_DIRECT_RED_MASK) << FLIGHT_PALETTE_DIRECT_RED_SHIFT) |
					((outputGreen & FLIGHT_PALETTE_DIRECT_GREEN_MASK) << FLIGHT_PALETTE_DIRECT_GREEN_SHIFT);
			}
		}
	}
}

// FUNCTION: XW 0x4239E0
void FlightPalette_ResetIf8Bit(void) {
	if (g_flightBytesPerPixel == FLIGHT_DISPLAY_INDEXED_BYTES_PER_PIXEL) {
		rtsvga2_unblankVGA();
	}
}
