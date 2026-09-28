#include "xw/flight/hud/festring.h"

#ifdef XW_MODERN
#include "xw_dos94/render/display.h"
#include "xw_runtime/runtime/flight_types.h"
#endif

#include "xw/flight/fediskio.h"
#include "xw/flight/feinput.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/replay/replay.h"
#include "xw/util/sys2.h"

#include <stdlib.h>

// GLOBAL: XW 0x4C78C0
const uint8_t g_flightCharToColorLut[FLIGHT_TEXT_COLOR_CODE_COUNT] = { 0x00, 0x32, 0x31, 0x36, 0x35, 0x3D,
																	   0x3C, 0x3A, 0x39, 0x02, 0x32, 0x31,
																	   0x36, 0x35, 0x05, 0x08 };

// GLOBAL: XW 0x4C78D0
const uint8_t g_flightReplayCharToColorLut[FLIGHT_TEXT_COLOR_CODE_COUNT] = {
	0x10, 0x09, 0x01, 0x0C, 0x04, 0x0A, 0x02, 0x0E, 0x06, 0x0F, 0x09, 0x01, 0x0C, 0x04, 0x07, 0x08
};

// GLOBAL: XW 0x4DA85C
uint8_t g_flightColorEscapeBypassChar = FLIGHT_TEXT_DEFAULT_COLOR_BYPASS;

// GLOBAL: XW 0x628600
char g_flightTextScratchBuffer[FLIGHT_TEXT_SCRATCH_CAPACITY] = { 0 };

// GLOBAL: XW 0x62AD7C
uint8_t g_flightFontHasLowercase = 0;

// GLOBAL: XW 0x62AFF3
uint8_t g_flightFontLineHeight = 0;

// GLOBAL: XW 0x62AFF6
uint16_t g_flightClipLeft = 0;

// GLOBAL: XW 0x62B4F4
uint8_t g_flightTextColorIndex = 0;

// GLOBAL: XW 0x62B90C
int16_t g_flightWordWrapEnabled = 0;

// GLOBAL: XW 0x62BA94
uint8_t* g_flightFontGlyphTableSw = NULL;

// GLOBAL: XW 0x62BAE0
uint8_t g_flightFontTier = 0;

// GLOBAL: XW 0x62BB10
uint16_t g_flightClipRight = 0;

// GLOBAL: XW 0x62BB12
uint16_t g_flightFontGlyphStrideSw = 0;

// GLOBAL: XW 0x62BC82
uint8_t g_flightTextShadowColor = 0;

// GLOBAL: XW 0x62BD94
uint8_t g_flightTextShadowEnabled = 0;

// GLOBAL: XW 0x62D120
int16_t g_flightClearLineBgEnabled = 0;

// GLOBAL: XW 0x637348
uint16_t g_flightClipBottom = 0;

// GLOBAL: XW 0x63734C
uint8_t g_flightTextBgColor = 0;

// GLOBAL: XW 0x63734E
int16_t g_flightCursorY = 0;

// GLOBAL: XW 0x637354
int16_t g_flightCursorX = 0;

// GLOBAL: XW 0x63736A
uint16_t g_flightClipTop = 0;

// FUNCTION: XW 0x40B950
void festring_setcursor(int16_t x, int16_t y) {
	g_flightCursorY = y;
	g_flightCursorX = x;
}

// FUNCTION: XW 0x40B970
void festring_setbound(uint16_t left, uint16_t top, int16_t right, uint16_t bottom) {
	g_flightClipTop = top;
	g_flightClipBottom = bottom;
	g_flightClipLeft = left;
	g_flightClipRight = right;
}

// FUNCTION: XW 0x40B9A0
void festring_settextcolor(uint16_t charOrIndex) {
#ifdef XW_MODERN
	if (XwFlightTypes_Dos() && g_replayviewmode && !(uint8_t)g_flightDisplaySurfaceMode &&
		charOrIndex == 0x40) {
		g_flightTextColorIndex = 0;
		return;
	}
#endif
	if (charOrIndex < FLIGHT_TEXT_ENCODED_COLOR_BASE) {
		g_flightTextColorIndex = (uint8_t)charOrIndex;
	} else if (g_replayviewmode != 0 && (uint8_t)g_flightDisplaySurfaceMode == 0) {
		g_flightTextColorIndex = g_flightReplayCharToColorLut[charOrIndex - FLIGHT_TEXT_ENCODED_COLOR_BASE];
	} else {
		g_flightTextColorIndex = g_flightCharToColorLut[charOrIndex - FLIGHT_TEXT_ENCODED_COLOR_BASE];
	}
}

// FUNCTION: XW 0x40B9F0
void festring_setbackcolor(uint16_t charOrIndex) {
#ifdef XW_MODERN
	if (XwFlightTypes_Dos() && g_replayviewmode && !(uint8_t)g_flightDisplaySurfaceMode &&
		charOrIndex == 0x40) {
		g_flightTextBgColor = 0;
		return;
	}
#endif
	if (charOrIndex == g_flightColorEscapeBypassChar || charOrIndex < FLIGHT_TEXT_ENCODED_COLOR_BASE) {
		g_flightTextBgColor = (uint8_t)charOrIndex;
	} else if (g_replayviewmode != 0 && (uint8_t)g_flightDisplaySurfaceMode == 0) {
		g_flightTextBgColor = g_flightReplayCharToColorLut[charOrIndex - FLIGHT_TEXT_ENCODED_COLOR_BASE];
	} else {
		g_flightTextBgColor = g_flightCharToColorLut[charOrIndex - FLIGHT_TEXT_ENCODED_COLOR_BASE];
	}
}

// FUNCTION: XW 0x40BA40
void festring_setdropcolor(uint16_t charOrIndex) {
#ifdef XW_MODERN
	if (XwFlightTypes_Dos() && g_replayviewmode && !(uint8_t)g_flightDisplaySurfaceMode &&
		charOrIndex == 0x40) {
		g_flightTextShadowColor = 0;
		return;
	}
#endif
	if (charOrIndex < FLIGHT_TEXT_ENCODED_COLOR_BASE) {
		g_flightTextShadowColor = (uint8_t)charOrIndex;
	} else if (g_replayviewmode != 0 && (uint8_t)g_flightDisplaySurfaceMode == 0) {
		g_flightTextShadowColor = g_flightReplayCharToColorLut[charOrIndex - FLIGHT_TEXT_ENCODED_COLOR_BASE];
	} else {
		g_flightTextShadowColor = g_flightCharToColorLut[charOrIndex - FLIGHT_TEXT_ENCODED_COLOR_BASE];
	}
}

// FUNCTION: XW 0x40BA90
void festring_setlinewrap(int16_t enabled) { g_flightWordWrapEnabled = enabled; }

// FUNCTION: XW 0x40BAA0
void festring_setautofill(int16_t enabled) { g_flightClearLineBgEnabled = enabled; }

// FUNCTION: XW 0x40BAB0
void festring_setfontsize(uint8_t tier) {
#ifdef XW_MODERN
	if (XwFlightTypes_Dos() && Dos94_display) {
		Dos94_festring_setfontsize(tier);
		return;
	}
#endif
	g_flightFontTier = tier;
	switch (tier) {
		case FLIGHT_FONT_TINY:
			g_flightFontLineHeight = FLIGHT_FONT_TINY_LINE_HEIGHT;
			g_flightFontGlyphTableSw = gTinyFntBuf;
			g_flightFontGlyphStrideSw = FLIGHT_FONT_TINY_GLYPH_STRIDE;
			g_flightFontHasLowercase = 1;
			break;
		case FLIGHT_FONT_MICRO:
			g_flightFontLineHeight = FLIGHT_FONT_MICRO_LINE_HEIGHT;
			g_flightFontGlyphTableSw = gMicroFntBuf;
			g_flightFontGlyphStrideSw = FLIGHT_FONT_MICRO_GLYPH_STRIDE;
			g_flightFontHasLowercase = 0;
			break;
	}
}

// FUNCTION: XW 0x40BB10
void festring_farstrcpy(const char* text) {
	char* destination = g_flightTextScratchBuffer;
	while (*text != '\0') {
		*destination++ = *text++;
	}
	*destination = '\0';
}

// FUNCTION: XW 0x40BB30
void festring_farstrcat(const char* text) {
	char* buffer = g_flightTextScratchBuffer;
	size_t destinationIndex;
	size_t sourceIndex;
	for (destinationIndex = 0; buffer[destinationIndex] != '\0'; ++destinationIndex) {
	}
	for (sourceIndex = 0; text[sourceIndex] != '\0'; ++destinationIndex, ++sourceIndex) {
		buffer[destinationIndex] = text[sourceIndex];
	}
	buffer[destinationIndex] = '\0';
}

// FUNCTION: XW 0x40BB60
void festring_farstradd(uint8_t character) {
	unsigned int i;
	for (i = 0; g_flightTextScratchBuffer[i] != '\0'; ++i) {
	}
	g_flightTextScratchBuffer[i] = character;
	g_flightTextScratchBuffer[i + 1] = '\0';
}

// FUNCTION: XW 0x40BB90
void festring_outstring(char* str) {
	size_t index;
	uint8_t character;
	for (index = 0; (character = (uint8_t)str[index]) != 0; ++index) {
		if (character == FLIGHT_TEXT_COLOR_ESCAPE) {
			++index;
			festring_settextcolor((uint8_t)str[index]);
		} else if (character < FLIGHT_TEXT_FIRST_DRAWABLE_BYTE) {
			festring_settextcolor(character);
		} else {
			g_flightDrawCharFn(str[index]);
		}
	}
}

// FUNCTION: XW 0x40BBE0
void festring_outstringcenter(char* str) {
	uint16_t halfWidth = (uint16_t)sys2_calclength((const uint8_t*)str) >> 1;
	uint16_t x = (g_flightClipRight + g_flightClipLeft) / 2 - halfWidth;
	if (x < g_flightClipLeft) {
		x = g_flightClipLeft;
	}
	festring_setcursor((int16_t)x, g_flightCursorY);
	festring_outstring(str);
}

// FUNCTION: XW 0x40BC50
void festring_outstringright(char* str) {
	int16_t x = g_flightClipRight - (sys2_calclength((const uint8_t*)str) + FLIGHT_TEXT_RIGHT_MARGIN);
	if (x < 0) {
		x = 0;
	}
	if ((uint16_t)x < g_flightClipLeft) {
		x = g_flightClipLeft;
	}
	festring_setcursor((int16_t)x, g_flightCursorY);
	festring_outstring(str);
}

// FUNCTION: XW 0x40BCC0
void festring_clearscreen(void) {
	festring_setbound(0, 0, (int16_t)g_flightScreenWidth, (uint16_t)g_flightScreenHeight);
	festring_setbackcolor(0);
	g_flightFillClipRectFn();
}

// FUNCTION: XW 0x40BCF0
void festring_hidescreen(void) { g_flightRenderTransitionHook(); }

// FUNCTION: XW 0x40BD00
void festring_showscreen(void) { g_flightResetPaletteFn(); }
