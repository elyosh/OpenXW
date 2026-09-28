#ifndef XW_RENDER_SHADE_H
#define XW_RENDER_SHADE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/landru_config.h"

#include <landru/rect.h>
#include <stddef.h>
#include <stdint.h>

enum { SHADE_PALETTE_COLOR_COUNT = 256, SHADE_BLEND_FRACTION_BITS = 8, SHADE_INITIAL_DISTANCE = 4095 };

extern uint8_t g_shadePalette[SHADE_PALETTE_COLOR_COUNT];

struct RgbTriplet;
typedef struct XwGrayscaleEntry XwGrayscaleEntry;

/* Original IDB size: 4 bytes. */
struct XwGrayscaleEntry {
	/* IDB +0x0: Grayscale channel byte; RGB/BGR channel order unproven. */
	uint8_t channel0;
	/* IDB +0x1: Grayscale channel byte; RGB/BGR channel order unproven. */
	uint8_t channel1;
	/* IDB +0x2: Grayscale channel byte; RGB/BGR channel order unproven. */
	uint8_t channel2;
	/* IDB +0x3: Untouched fourth byte; no static reader found for this table. */
	uint8_t field_03;
};

/* Declarations follow ascending original IDB address. */

/* 0x4698E0 */
int16_t shade_Set_Shaded_Palette(const struct RgbTriplet* colors, int blendFactor, uint16_t targetRed,
								 uint16_t targetGreen, uint16_t targetBlue);

/* 0x469B20 */
int16_t shade_RemapClippedRect(const Rect* rect);

/* 0x469C10 */
void shade_Shadow_Line_List(const uint8_t* remap, int16_t x, int16_t y, int16_t width, int16_t height);

#ifdef __cplusplus
}
#endif

#endif
