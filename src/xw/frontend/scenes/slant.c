#include "xw/frontend/scenes/slant.h"

#include <landru/canvas.h>

// FUNCTION: XW 0x4611A0
void slant_Scale_Line(const uint8_t* bitmap, int16_t srcX, int16_t srcY, int16_t skip,
					  uint16_t fractionalSkip, int16_t dstX, int16_t dstY, int16_t width, uint8_t color) {
	uint8_t* destination = (uint8_t*)draw_buff_gbl + dstX + draw_w_gbl * dstY;
	const uint8_t* source = bitmap + SLANT_SOURCE_PITCH * srcY + srcX;
	int sourceIndex = 0;
	int fraction = skip;
	int pixelsLeft;
	for (pixelsLeft = width; pixelsLeft > 0; --pixelsLeft) {
		if (source[sourceIndex] != 0) {
			destination[width - pixelsLeft] = color;
		}
		++sourceIndex;
		fraction += fractionalSkip;
		if (fraction > SLANT_FRACTION_MASK) {
			++sourceIndex;
			fraction &= SLANT_FRACTION_MASK;
		}
		sourceIndex += skip;
	}
}
