#include "xw/util/sys2.h"

#include "xw/flight/hud/festring.h"

// FUNCTION: XW 0x423960
int16_t sys2_calclength(const uint8_t* text) {
	int16_t width = 0;
	size_t index;
	for (index = 0;;) {
		uint8_t character = text[index++];
		if (character == 0 || character == '\n') {
			break;
		}
		if (character >= ' ') {
			if (character == FLIGHT_TEXT_COLOR_ESCAPE) {
				++index;
			} else {
				if (g_flightFontTier != 0 && g_flightFontHasLowercase == 0 && character >= 'a' &&
					character <= 'z') {
					character -= 'a' - 'A';
				}
				width += g_flightFontGlyphTableSw[g_flightFontGlyphStrideSw * (uint8_t)(character - ' ')];
			}
		}
	}
	return width;
}
