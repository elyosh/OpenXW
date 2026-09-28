#ifdef XW_MODERN
#include "xw_runtime/snapshot/render_hud.h"
#endif
/* DOS94 0x698445–0x698967. VGA writes retain 16-bit effective offsets. */
#include "xw/flight/hud/festring.h"
#include "xw/render/rtsvga2.h"
#include "xw_dos94/render/display.h"
#include <string.h>

static uint8_t glyphHeight;
static uint8_t bracketSaved[10], crossSaved[7];
static const int8_t bracket[10][2] = { { -1, 1 }, { -2, 1 }, { -2, 0 }, { -2, -1 }, { -1, -1 },
									   { 1, -1 }, { 2, -1 }, { 2, 0 },  { 2, 1 },   { 1, 1 } };
static const int8_t cross[7][2] = { { -2, 0 }, { -1, 0 }, { 0, 0 }, { 1, 0 }, { 2, 0 }, { 0, 1 }, { 0, -1 } };

static void put(uint16_t offset, uint8_t color) {
	if (offset < DOS94_SCREEN_BYTES)
		Dos94_display->screen[offset] = color;
}

static uint8_t get(uint16_t offset) {
	return offset < DOS94_SCREEN_BYTES ? Dos94_display->screen[offset] : 0;
}

int Dos94_rtsvga2_calcpositionVGA(uint16_t x, uint16_t y) { return (uint16_t)(y * 320 + x); }

void Dos94_rtsvga2_drawdotVGA(uint16_t x, uint16_t y, uint8_t color) { put((uint16_t)(y * 320 + x), color); }

bool Dos94Sprite_Size(const uint8_t* data, size_t available, size_t* size) {
	size_t i = 0;
	while (i < available) {
		uint8_t op = data[i++];
		if (op == 255) {
			*size = i;
			return true;
		}
		size_t operands = op == 251 ? 1 : (op == 252 || op == 253 ? 2 : 0);
		if (operands > available - i)
			return false;
		i += operands;
	}
	return false;
}

void Dos94_rtsvga2_drawshapeVGA(const uint8_t* source, int16_t x, int16_t y, int16_t transparent,
								int16_t mirror) {
#ifdef XW_MODERN
	XwHud_Sprite(source, x, y, transparent, mirror);
#endif

	uint8_t shift = 0;
	uint16_t offset = (uint16_t)(320 * y + x);
	const int step = mirror ? -1 : 1;
	for (;;) {
		uint8_t op = *source++, color;
		unsigned count;
		if (op == 255)
			return;
		if (op == 254) {
			y = (int16_t)(y + 1);
			offset = (uint16_t)(320 * y + x);
			continue;
		}
		if (op == 251) {
			shift = *source++;
			continue;
		}
		if (op == 252) {
			color = *source++;
			count = (unsigned)*source++ + 1;
			for (unsigned i = 0; i < count; ++i) {
				put(offset, (uint8_t)(color + (i & 1)));
				offset = (uint16_t)(offset + step);
			}
			continue;
		}
		if (op == 253) {
			count = (unsigned)*source++ + 1;
			color = *source++;
		} else {
			count = (op & 3) + 1;
			color = (uint8_t)((op >> 2) + shift);
		}
		for (unsigned i = 0; i < count; ++i) {
			if (color != (uint8_t)transparent)
				put(offset, color);
			offset = (uint16_t)(offset + step);
		}
	}
}

void Dos94_rtsvga2_fillboxVGA(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
#ifdef XW_MODERN
	XwHud_Fill(x1, y1, x2, y2);
#endif

	int16_t width = (int16_t)(x2 - x1);
	if (width <= 0)
		return;
	unsigned rows = (uint8_t)(y2 - y1);
	if (!rows)
		rows = 256;
	for (unsigned row = 0; row < rows; ++row) {
		uint16_t offset = (uint16_t)((uint16_t)(y1 + row) * 320 + x1);
		for (int i = 0; i < width; ++i)
			put((uint16_t)(offset + i), g_flightTextBgColor);
	}
}

void Dos94_rtsvga2_clearwindowVGA(void) {
	Dos94_rtsvga2_fillboxVGA(g_flightClipLeft, g_flightClipTop, g_flightClipRight, g_flightClipBottom);
}

void Dos94_rtsvga2_saveboxVGA(uint8_t* dest, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
#ifdef XW_MODERN
	XwHud_Save(dest, x, y, width, height);
#endif

	for (unsigned row = 0; row < height; ++row)
		for (unsigned col = 0; col < width; ++col)
			*dest++ = get((uint16_t)((y + row) * 320 + x + col));
}

void Dos94_rtsvga2_restoreboxVGA(const uint8_t* source, uint16_t x, uint16_t y, uint16_t width,
								 uint16_t height) {
#ifdef XW_MODERN
	XwHud_Restore(source);
#endif

	for (unsigned row = 0; row < height; ++row)
		for (unsigned col = 0; col < width; ++col)
			put((uint16_t)((y + row) * 320 + x + col), *source++);
}

void Dos94_festring_setfontsize(uint8_t tier) {
	g_flightFontTier = tier;
	g_flightFontHasLowercase = 0;
	if (tier == 1) {
		g_flightFontGlyphTableSw = Dos94_display->tiny;
		g_flightFontGlyphStrideSw = 16;
		g_flightFontLineHeight = 7;
	} else if (tier == 2) {
		g_flightFontGlyphTableSw = Dos94_display->micro;
		g_flightFontGlyphStrideSw = 12;
		g_flightFontLineHeight = 5;
	}
}

static void newline(void) {
	if (g_flightClearLineBgEnabled && (int16_t)g_flightClipRight > g_flightCursorX)
		Dos94_rtsvga2_fillboxVGA(g_flightCursorX, g_flightCursorY, g_flightClipRight,
								 (uint16_t)(g_flightCursorY + g_flightFontLineHeight));
	g_flightCursorX = g_flightClipLeft;
	g_flightCursorY = (int16_t)(g_flightCursorY + glyphHeight);
}

void Dos94_rtsvga2_outcharVGA(char character) {
	uint8_t ch = (uint8_t)character;
	if (ch == '\n') {
		newline();
		return;
	}
	/* DOS93 compares the character as a signed byte before looking up a glyph. */
	if (ch < 32 || (Dos94Assets_Version() == XW_GAME_VERSION_93 && ch > 127))
		return;
	if (g_flightFontTier && ch >= 'a' && ch <= 'z')
		ch -= 'a' - 'A';
	size_t size = g_flightFontGlyphTableSw == Dos94_display->tiny ? sizeof Dos94_display->tiny
																  : sizeof Dos94_display->micro;
	size_t offset = (ch - 32) * g_flightFontGlyphStrideSw;
	if (offset + g_flightFontGlyphStrideSw > size)
		return;
	const uint8_t* glyph = g_flightFontGlyphTableSw + offset;
	uint8_t width = glyph[0];
	glyphHeight = glyph[1];
	if (!width || !glyphHeight || 2 + 2 * glyphHeight > g_flightFontGlyphStrideSw)
		return;
	if ((int16_t)(g_flightCursorX + width) >= (int16_t)g_flightClipRight) {
		if (!g_flightWordWrapEnabled)
			return;
		newline();
	}
	XwHud_Glyph(ch, width, glyphHeight);
	uint8_t shadow = 0;
	for (unsigned row = 0; row < glyphHeight; ++row) {
		uint8_t bits = glyph[2 + 2 * row];
		for (unsigned col = 0; col < width + (g_flightTextShadowEnabled != 0); ++col) {
			uint8_t color = bits & 128
								? g_flightTextColorIndex
								: (g_flightTextShadowEnabled && (shadow & 128) ? g_flightTextShadowColor
																			   : g_flightTextBgColor);
			put((uint16_t)((g_flightCursorY + row) * 320 + g_flightCursorX + col), color);
			bits <<= 1;
			shadow <<= 1;
		}
		shadow = glyph[2 + 2 * row] >> 1;
	}
	int16_t next = (int16_t)(g_flightCursorX + width);
	if (next < (int16_t)g_flightClipRight)
		g_flightCursorX = next;
	else if (g_flightWordWrapEnabled)
		newline();
}

void Dos94_rtsvga2_drawblips(XwRadarBlip* blips, int count) {
	for (int i = 0; i < count; ++i) {
		uint16_t offset = (uint16_t)(blips[i].y * 320 + blips[i].x);
		if (get(offset) == 33)
			put(offset, (uint8_t)blips[i].colorOrDrawMask);
		else
			blips[i].colorOrDrawMask &= 0xFF00;
	}
}

void Dos94_rtsvga2_removeblips(XwRadarBlip* blips, int count) {
	for (int i = 0; i < count; ++i)
		if ((uint8_t)blips[i].colorOrDrawMask)
			put((uint16_t)(blips[i].y * 320 + blips[i].x), 33);
}

static void marker(int x, int y, const int8_t offsets[][2], uint8_t* saved, unsigned count, uint8_t color,
				   bool restore) {
	for (unsigned i = 0; i < count; ++i) {
		uint16_t offset = (uint16_t)((y + offsets[i][1]) * 320 + x + offsets[i][0]);
		if (restore)
			put(offset, saved[i]);
		else {
			saved[i] = get(offset);
			put(offset, color);
		}
	}
}

void Dos94_rtsvga2_drawbracket(void) {
	marker(g_radarSelectedTargetScreenX, g_radarSelectedTargetScreenY, bracket, bracketSaved, 10, 7, false);
}

void Dos94_rtsvga2_removebracket(void) {
	marker(g_radarPreviousTargetScreenX, g_radarPreviousTargetScreenY, bracket, bracketSaved, 10, 0, true);
}

void Dos94_rtsvga2_drawcross(uint16_t x, uint16_t y, uint8_t color) {
	marker(x, y, cross, crossSaved, 7, color, false);
}

void Dos94_rtsvga2_removecross(uint16_t x, uint16_t y) { marker(x, y, cross, crossSaved, 7, 0, true); }
