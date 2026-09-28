#ifndef XW_FLIGHT_HUD_FESTRING_H
#define XW_FLIGHT_HUD_FESTRING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

/* Declarations follow ascending original IDB address. */

enum {
	FLIGHT_TEXT_SCRATCH_CAPACITY = 40,
	FLIGHT_TEXT_ENCODED_COLOR_BASE = 0x40,
	FLIGHT_TEXT_COLOR_CODE_COUNT = 16
};

enum { FLIGHT_TEXT_FIRST_DRAWABLE_BYTE = 0x10, FLIGHT_TEXT_COLOR_ESCAPE = 0xFE };

enum { FLIGHT_TEXT_DEFAULT_COLOR_BYPASS = 0xFB };

enum { FLIGHT_TEXT_RIGHT_MARGIN = 2 };

enum {
	FLIGHT_FONT_TINY = 1,
	FLIGHT_FONT_MICRO = 2,
	FLIGHT_FONT_TINY_LINE_HEIGHT = 21,
	FLIGHT_FONT_MICRO_LINE_HEIGHT = 9,
	FLIGHT_FONT_TINY_GLYPH_STRIDE = 170,
	FLIGHT_FONT_MICRO_GLYPH_STRIDE = 74
};

enum { FLIGHT_FONT_GLYPH_HEADER_SIZE = 2, FLIGHT_FONT_GLYPH_ROW_SIZE = 8, FLIGHT_FONT_GLYPH_SHIFT_MASK = 31 };

extern const uint8_t g_flightCharToColorLut[FLIGHT_TEXT_COLOR_CODE_COUNT];
extern const uint8_t g_flightReplayCharToColorLut[FLIGHT_TEXT_COLOR_CODE_COUNT];
extern uint8_t g_flightColorEscapeBypassChar;

extern char g_flightTextScratchBuffer[FLIGHT_TEXT_SCRATCH_CAPACITY];

extern uint8_t g_flightFontHasLowercase;
extern uint8_t g_flightFontLineHeight;
extern uint16_t g_flightClipLeft;
extern uint8_t g_flightTextColorIndex;
extern int16_t g_flightWordWrapEnabled;
extern uint8_t* g_flightFontGlyphTableSw;
extern uint8_t g_flightFontTier;
extern uint16_t g_flightClipRight;
extern uint16_t g_flightFontGlyphStrideSw;
extern uint8_t g_flightTextShadowColor;
extern uint8_t g_flightTextShadowEnabled;
extern int16_t g_flightClearLineBgEnabled;
extern uint16_t g_flightClipBottom;
extern uint8_t g_flightTextBgColor;
extern int16_t g_flightCursorY;
extern int16_t g_flightCursorX;
extern uint16_t g_flightClipTop;

/* 0x40B950 */
void festring_setcursor(int16_t x, int16_t y);

/* 0x40B970 */
void festring_setbound(uint16_t left, uint16_t top, int16_t right, uint16_t bottom);

/* 0x40B9A0 */
/* Encoded colors must index the 16-entry table. Lower values are literal. */
void festring_settextcolor(uint16_t charOrIndex);

/* 0x40B9F0 */
/* Nonliteral colors other than the bypass byte must index the 16-entry table. */
void festring_setbackcolor(uint16_t charOrIndex);

/* 0x40BA40 */
/* Encoded colors must index the 16-entry table. Lower values are literal. */
void festring_setdropcolor(uint16_t charOrIndex);

/* 0x40BA90 */
void festring_setlinewrap(int16_t enabled);

/* 0x40BAA0 */
void festring_setautofill(int16_t enabled);

/* 0x40BAB0 */
void festring_setfontsize(uint8_t tier);

/* 0x40BB10 */
void festring_farstrcpy(const char* text);

/* 0x40BB30 */
void festring_farstrcat(const char* text);

/* 0x40BB60 */
void festring_farstradd(uint8_t character);

/* 0x40BB90 */
/* Color escapes consume a payload byte, followed by the remaining terminated string. */
void festring_outstring(char* str);

/* 0x40BBE0 */
void festring_outstringcenter(char* str);

/* 0x40BC50 */
void festring_outstringright(char* str);

/* 0x40BCC0 */
void festring_clearscreen(void);

/* 0x40BCF0 */
void festring_hidescreen(void);

/* 0x40BD00 */
void festring_showscreen(void);

#ifdef __cplusplus
}
#endif

#endif
