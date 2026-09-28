#include "xw/assets/model_texture.h"

#include "xw/frontend/shell_preferences.h"
#include "xw/render/rtsvga2.h"
#include "xw/render/std3d.h"
#include "xw/util/shared.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

// FUNCTION: XW 0x484BC0
int32_t ModelTexture_IsHardwareFormat555(void) {
	return g_pFmtRGB565->colorInfo.greenBPP == STDCOLOR_RGB555_CHANNEL_BITS;
}

// FUNCTION: XW 0x484BD0
void ModelTexture_FilterHardwarePalette(uint16_t* palette) {
	int firstTransparentIndex;
	int transparentCount;
	int colorIndex;

	if (g_modelTextureQuality != MODEL_TEXTURE_QUALITY_ALPHA_FILTER) {
		palette[MODEL_TEXTURE_TRANSPARENT_COUNT_ROW * MODEL_TEXTURE_PALETTE_COLORS] = 0;
		return;
	}
	firstTransparentIndex = -1;
	transparentCount = 0;
	for (colorIndex = 0; colorIndex < MODEL_TEXTURE_PALETTE_COLORS; ++colorIndex) {
		uint16_t baseColor = palette[colorIndex];
		int blue = baseColor & MODEL_TEXTURE_CHANNEL_MASK;
		uint16_t redGreen = baseColor;
		int green;
		int red;
		int shadeIndex;

		redGreen >>= MODEL_TEXTURE_GREEN_SHIFT;
		green = redGreen & MODEL_TEXTURE_CHANNEL_MASK;
		redGreen >>= MODEL_TEXTURE_RED_SHIFT - MODEL_TEXTURE_GREEN_SHIFT;
		red = redGreen & MODEL_TEXTURE_CHANNEL_MASK;

		if (red * red + green * green + blue * blue < MODEL_TEXTURE_DARK_ENERGY_LIMIT) {
			palette[colorIndex] = 0;
			++transparentCount;
			if (firstTransparentIndex == -1)
				firstTransparentIndex = colorIndex;
		} else {
			for (shadeIndex = 1; shadeIndex < MODEL_TEXTURE_SHADE_COMPARE_END; ++shadeIndex) {
				uint16_t shadeColor = palette[shadeIndex * MODEL_TEXTURE_PALETTE_COLORS + colorIndex];
				uint16_t baseRedGreen = baseColor;
				uint16_t shadeRedGreen = shadeColor;
				int blueDifference =
					(shadeColor & MODEL_TEXTURE_CHANNEL_MASK) - (baseColor & MODEL_TEXTURE_CHANNEL_MASK);
				int greenDifference;
				int redDifference;

				baseRedGreen >>= MODEL_TEXTURE_GREEN_SHIFT;
				shadeRedGreen >>= MODEL_TEXTURE_GREEN_SHIFT;
				greenDifference = (shadeRedGreen & MODEL_TEXTURE_CHANNEL_MASK) -
								  (baseRedGreen & MODEL_TEXTURE_CHANNEL_MASK);
				baseRedGreen >>= MODEL_TEXTURE_RED_SHIFT - MODEL_TEXTURE_GREEN_SHIFT;
				shadeRedGreen >>= MODEL_TEXTURE_RED_SHIFT - MODEL_TEXTURE_GREEN_SHIFT;
				redDifference = (shadeRedGreen & MODEL_TEXTURE_CHANNEL_MASK) -
								(baseRedGreen & MODEL_TEXTURE_CHANNEL_MASK);

				if (redDifference * redDifference + greenDifference * greenDifference +
						blueDifference * blueDifference >
					MODEL_TEXTURE_SHADE_DISTANCE_LIMIT) {
					++transparentCount;
					palette[colorIndex] = 0;
					if (firstTransparentIndex == -1)
						firstTransparentIndex = colorIndex;
					break;
				}
			}
			if (shadeIndex == MODEL_TEXTURE_SHADE_COMPARE_END)
				palette[colorIndex] =
					palette[MODEL_TEXTURE_REPLACEMENT_SHADE * MODEL_TEXTURE_PALETTE_COLORS + colorIndex];
		}
	}
	if (transparentCount < MODEL_TEXTURE_PALETTE_COLORS) {
		nullsub_SharedNoOp();
	} else {
		nullsub_SharedNoOp();
		transparentCount = 0;
	}
	palette[MODEL_TEXTURE_FIRST_TRANSPARENT_ROW * MODEL_TEXTURE_PALETTE_COLORS] = firstTransparentIndex;
	palette[MODEL_TEXTURE_TRANSPARENT_COUNT_ROW * MODEL_TEXTURE_PALETTE_COLORS] = transparentCount;
}

// FUNCTION: XW 0x488440
void ModelTexture_BuildPalettedShadeTable(uint8_t* dst, const uint8_t* rgb24, int width, int height) {
	RgbTriplet target;
	RgbTriplet palette[MODEL_TEXTURE_PALETTE_COLORS];
	unsigned int pixelCount = (unsigned int)width * (unsigned int)height;
	unsigned int uniqueColorCount = 1;
	unsigned int pixelIndex, colorIndex, shadeLevel;
	uint8_t* indexedShades;
	uint8_t* hardwareShades;
#ifdef XW_MODERN
	/* Unused palette entries were uninitialized stack data in the original. */
	memset(palette, 0, sizeof(palette));
#endif
	palette[0].r = rgb24[MODEL_TEXTURE_SOURCE_RED] >> MODEL_TEXTURE_SOURCE_CHANNEL_SHIFT;
	palette[0].g = rgb24[MODEL_TEXTURE_SOURCE_GREEN] >> MODEL_TEXTURE_SOURCE_CHANNEL_SHIFT;
	palette[0].b = rgb24[MODEL_TEXTURE_SOURCE_BLUE] >> MODEL_TEXTURE_SOURCE_CHANNEL_SHIFT;
	dst[0] = 0;
	for (pixelIndex = 1; pixelIndex < pixelCount; ++pixelIndex) {
		uint8_t nearest;
		target.r = rgb24[pixelIndex * sizeof(RgbTriplet) + MODEL_TEXTURE_SOURCE_RED] >>
				   MODEL_TEXTURE_SOURCE_CHANNEL_SHIFT;
		target.g = rgb24[pixelIndex * sizeof(RgbTriplet) + MODEL_TEXTURE_SOURCE_GREEN] >>
				   MODEL_TEXTURE_SOURCE_CHANNEL_SHIFT;
		target.b = rgb24[pixelIndex * sizeof(RgbTriplet) + MODEL_TEXTURE_SOURCE_BLUE] >>
				   MODEL_TEXTURE_SOURCE_CHANNEL_SHIFT;
		nearest = rtsvga2_FindNearestRgbTripletIndex(&target, palette, 0, uniqueColorCount);
		if ((palette[nearest].r != target.r || palette[nearest].g != target.g ||
			 palette[nearest].b != target.b) &&
			uniqueColorCount < MODEL_TEXTURE_PALETTE_COLORS) {
			nearest = uniqueColorCount++;
			palette[nearest].r = target.r;
			palette[nearest].g = target.g;
			palette[nearest].b = target.b;
		}
		dst[pixelIndex] = nearest;
	}
	indexedShades = dst + pixelCount;
	hardwareShades = indexedShades + MODEL_TEXTURE_SHADE_LEVELS * MODEL_TEXTURE_PALETTE_COLORS;
	for (colorIndex = 0; colorIndex < MODEL_TEXTURE_PALETTE_COLORS; ++colorIndex) {
		for (shadeLevel = 0; shadeLevel < MODEL_TEXTURE_SHADE_LEVELS; ++shadeLevel) {
			uint16_t packed;
			unsigned int shadeIndex = shadeLevel * MODEL_TEXTURE_PALETTE_COLORS + colorIndex;
			if (shadeLevel < MODEL_TEXTURE_BASE_SHADE) {
				target.r =
					(uint16_t)((
						(((unsigned int)palette[colorIndex].r << MODEL_TEXTURE_FRACTION_BITS) * shadeLevel >>
						 MODEL_TEXTURE_DARK_SCALE_SHIFT) +
						((unsigned int)palette[colorIndex].r << (MODEL_TEXTURE_FRACTION_BITS - 1)))) >>
					MODEL_TEXTURE_FRACTION_BITS;
				target.g =
					(uint16_t)((
						(((unsigned int)palette[colorIndex].g << MODEL_TEXTURE_FRACTION_BITS) * shadeLevel >>
						 MODEL_TEXTURE_DARK_SCALE_SHIFT) +
						((unsigned int)palette[colorIndex].g << (MODEL_TEXTURE_FRACTION_BITS - 1)))) >>
					MODEL_TEXTURE_FRACTION_BITS;
				target.b =
					(uint16_t)((
						(((unsigned int)palette[colorIndex].b << MODEL_TEXTURE_FRACTION_BITS) * shadeLevel >>
						 MODEL_TEXTURE_DARK_SCALE_SHIFT) +
						((unsigned int)palette[colorIndex].b << (MODEL_TEXTURE_FRACTION_BITS - 1)))) >>
					MODEL_TEXTURE_FRACTION_BITS;
			} else {
				target.r =
					(uint16_t)(((((MODEL_TEXTURE_CHANNEL_MASK - (unsigned int)palette[colorIndex].r) *
									  (shadeLevel - MODEL_TEXTURE_BASE_SHADE)
								  << MODEL_TEXTURE_FRACTION_BITS) >>
								 MODEL_TEXTURE_BRIGHT_SCALE_SHIFT) +
								((unsigned int)palette[colorIndex].r << MODEL_TEXTURE_FRACTION_BITS))) >>
					MODEL_TEXTURE_FRACTION_BITS;
				target.g =
					(uint16_t)(((((MODEL_TEXTURE_CHANNEL_MASK - (unsigned int)palette[colorIndex].g) *
									  (shadeLevel - MODEL_TEXTURE_BASE_SHADE)
								  << MODEL_TEXTURE_FRACTION_BITS) >>
								 MODEL_TEXTURE_BRIGHT_SCALE_SHIFT) +
								((unsigned int)palette[colorIndex].g << MODEL_TEXTURE_FRACTION_BITS))) >>
					MODEL_TEXTURE_FRACTION_BITS;
				target.b =
					(uint16_t)(((((MODEL_TEXTURE_CHANNEL_MASK - (unsigned int)palette[colorIndex].b) *
									  (shadeLevel - MODEL_TEXTURE_BASE_SHADE)
								  << MODEL_TEXTURE_FRACTION_BITS) >>
								 MODEL_TEXTURE_BRIGHT_SCALE_SHIFT) +
								((unsigned int)palette[colorIndex].b << MODEL_TEXTURE_FRACTION_BITS))) >>
					MODEL_TEXTURE_FRACTION_BITS;
			}
			packed = (((target.r << (MODEL_TEXTURE_RED_SHIFT - MODEL_TEXTURE_GREEN_SHIFT)) + target.g)
					  << MODEL_TEXTURE_GREEN_SHIFT) +
					 target.b;
#ifdef XW_MODERN
			hardwareShades[shadeIndex * sizeof(packed)] = (uint8_t)packed;
			hardwareShades[shadeIndex * sizeof(packed) + 1] = (uint8_t)(packed >> CHAR_BIT);
#else
			((uint16_t*)hardwareShades)[shadeIndex] = packed;
#endif
			target.r <<= 1;
			target.g <<= 1;
			target.b <<= 1;
			indexedShades[shadeIndex] = rtsvga2_FindNearestRgbTripletIndex(
				&target, g_swPalette, MODEL_TEXTURE_FIRST_SW_COLOR, MODEL_TEXTURE_PALETTE_COLORS);
		}
	}
}
