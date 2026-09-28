#ifndef XW_RENDER_RTSVGA2_H
#define XW_RENDER_RTSVGA2_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/render/flight_palette.h"

#include <stddef.h>
#include <stdint.h>

enum { RTSVGA2_SCANLINE_CAPACITY = 480, RTSVGA2_PALETTE_COLOR_COUNT = 256 };

enum { RTSVGA2_SCRATCH_BUFFER_SIZE = 0xC00, RTSVGA2_SCRATCH_EMPTY = 0xFF };

enum { RTSVGA2_PALETTE_BLANKED = 1 };

enum {
	RTSVGA2_BRIGHTNESS_IDENTITY = 256,
	RTSVGA2_BRIGHTNESS_SHIFT = 8,
	RTSVGA2_HSV_CHANNEL_MAX = 63,
	RTSVGA2_HUE_RED_YELLOW = 0,
	RTSVGA2_HUE_YELLOW_GREEN = 1,
	RTSVGA2_HUE_GREEN_CYAN = 2,
	RTSVGA2_HUE_CYAN_BLUE = 3,
	RTSVGA2_HUE_BLUE_MAGENTA = 4,
	RTSVGA2_HUE_MAGENTA_RED = 5
};

enum {
	RTSVGA2_MODEL_PALETTE_FIRST_COLOR = 64,
	RTSVGA2_MODEL_PALETTE_COLOR_COUNT = RTSVGA2_PALETTE_COLOR_COUNT - RTSVGA2_MODEL_PALETTE_FIRST_COLOR,
	RTSVGA2_RGB8_TO_RGB6_SHIFT = 2,
	RTSVGA2_RGB_PALETTE_SCRATCH_COUNT = 1024,
	RTSVGA2_RESERVED_COLOR_TOGGLE = 1
};

enum {
	RTSVGA2_DEFAULT_BRACKET_PIXEL_COUNT = 10,
	RTSVGA2_VESA_BRACKET_PIXEL_COUNT = 12,
	RTSVGA2_BRACKET_SAVED_PIXEL_CAPACITY = 12,
	RTSVGA2_BRACKET_COLOR = 7
};

enum { RTSVGA2_CROSS_PIXEL_COUNT = 7 };

enum { RTSVGA2_RADAR_BACKGROUND_COLOR = 33, RTSVGA2_BLIP_FIRST_PIXEL = 1, RTSVGA2_BLIP_SECOND_PIXEL = 2 };

enum { RTSVGA2_MODE_13H = 0x13 };

enum { RTSVGA2_NARROW_GLYPH_ROW_SIZE = 2, RTSVGA2_NARROW_GLYPH_HIGH_BIT = 0x80 };

enum { RTSVGA2_BANK_MAX_OFFSET = 0xFFFF };

enum {
	RTSVGA2_RLE_PALETTE_SHIFT = 0xFB,
	RTSVGA2_RLE_ALTERNATING_RUN = 0xFC,
	RTSVGA2_RLE_LONG_RUN = 0xFD,
	RTSVGA2_RLE_NEXT_ROW = 0xFE,
	RTSVGA2_RLE_END = 0xFF,
	RTSVGA2_RLE_SHORT_COLOR_SHIFT = 2,
	RTSVGA2_RLE_SHORT_COUNT_MASK = 3
};

enum {
	RTSVGA2_RGB565_LUT_SIZE = 65536,
	RTSVGA2_RGB565_LUT_ROW_SIZE = 256,
	RTSVGA2_RGB565_RED_SHIFT = 11,
	RTSVGA2_RGB565_GREEN_SHIFT = 5,
	RTSVGA2_RGB5_MASK = 0x1F,
	RTSVGA2_RGB6_MASK = 0x3F,
	RTSVGA2_RGB5_TO_RGB6_SCALE = 2
};

extern uint16_t g_flightFillRectBottom8bpp;
extern uint16_t g_flightFillRectRight8bpp;
extern uint16_t g_flightFillRectLeft8bpp;
extern uint16_t g_flightFillRectTop8bpp;
extern uint16_t g_flightFillRectCurrentY8bpp;
extern unsigned int g_flightFillRectRemainingRows8bpp;

extern RgbTriplet g_swPalette[RTSVGA2_PALETTE_COLOR_COUNT];
extern uint8_t g_paletteCycleEnabled;
extern uint8_t g_paletteBlankFlags;
extern int16_t g_flightSwRleSpriteX;
extern int16_t g_flightSwRleSpriteY;
extern unsigned int g_flightLineOffsetTable[RTSVGA2_SCANLINE_CAPACITY];
extern uint8_t g_flightSwRlePaletteShift;
extern uint8_t g_flightSwRleSpriteEndMarker;
extern uint8_t g_flightScratchBufferA[RTSVGA2_SCRATCH_BUFFER_SIZE];
extern uint8_t g_flightScratchBufferB[RTSVGA2_SCRATCH_BUFFER_SIZE];
extern uint16_t g_radarPreviousTargetScreenY;
extern uint16_t g_radarPreviousTargetScreenX;
extern int16_t g_radarSelectedTargetScreenX;
extern int16_t g_radarSelectedTargetScreenY;

struct RgbTriplet;
struct XwBitmapModelPrefix;
typedef struct FlightMarkerOffset FlightMarkerOffset;
typedef struct XwRadarBlip XwRadarBlip;
typedef struct XwRadarBoundarySample XwRadarBoundarySample;

/* Original IDB size: 2 bytes. */
struct FlightMarkerOffset {
	/* IDB +0x0: Signed pixel offset relative to the marker origin. */
	int8_t x;
	/* IDB +0x1: Signed pixel offset relative to the marker origin. */
	int8_t y;
};

/* Original IDB size: 6 bytes. */
struct XwRadarBlip {
	/* IDB +0x0 */
	uint16_t x;
	/* IDB +0x2 */
	uint16_t y;
	/* IDB +0x4: On queue insertion: palette index. 8bpp draw replaces with bit0/bit1 mask for pixels actually
	 * drawn; 16bpp retains color on success and clears on failure. */
	uint16_t colorOrDrawMask;
};

/* Original IDB size: 2 bytes. */
struct XwRadarBoundarySample {
	/* IDB +0x0 */
	uint8_t maxX;
	/* IDB +0x1 */
	uint8_t maxY;
};

extern FlightMarkerOffset g_flightBracketOffsets10[RTSVGA2_DEFAULT_BRACKET_PIXEL_COUNT];
extern FlightMarkerOffset g_flightBracketOffsets12[RTSVGA2_VESA_BRACKET_PIXEL_COUNT];
extern const FlightMarkerOffset* g_flightBracketOffsets;
extern unsigned int g_flightBracketOffsetCount;
extern uint8_t g_radarBracketSavedPixels8[RTSVGA2_BRACKET_SAVED_PIXEL_CAPACITY];
extern FlightMarkerOffset g_crossMarkerOffsets8[RTSVGA2_CROSS_PIXEL_COUNT];
extern uint8_t g_crossMarkerSavedPixels8[RTSVGA2_CROSS_PIXEL_COUNT];

/* Declarations follow ascending original IDB address. */

/* 0x421060 */
void rtsvga2_initgraphVGA(void);

/* 0x4211C0 */
void rtsvga2_setvgapointers(uint8_t* surface, uint16_t pitchBytes, uint16_t height);

/* 0x421240 */
void rtsvga2_BuildRgbRange(const struct RgbTriplet* sourcePalette, struct RgbTriplet* destPalette,
						   int startIndex, unsigned int count);

/* 0x4216D0 */
void rtsvga2_blankVGA(void);

/* 0x421720 */
void rtsvga2_unblankVGA(void);

/* 0x421780 */
void rtsvga2_buildpaletteVGA(const struct RgbTriplet* rgbTriples, uint16_t startIndex, uint16_t count);

/* 0x421840 */
void rtsvga2_savepaletteVGA(struct RgbTriplet* dstPalette);

/* 0x421870 */
void rtsvga2_restorepaletteVGA(const struct RgbTriplet* rgbTriples);

/* 0x421890 */
unsigned int rtsvga2_Convert24BppPalettesTo16Bpp(struct XwBitmapModelPrefix* bitmapModel);

/* 0x4219B0 */
void rtsvga2_Convert24BppPalettesTo8Bpp(struct XwBitmapModelPrefix* bitmapModel);

/* 0x421AC0 */
unsigned int rtsvga2_FindNearestRgbTripletIndex(const struct RgbTriplet* targetRgb,
												const struct RgbTriplet* palette, unsigned int startIndex,
												unsigned int endIndex);

/* 0x421B60 */
void rtsvga2_BuildRgb565ToPaletteIndexLut(uint8_t* destinationLut, unsigned int firstPaletteIndex,
										  unsigned int endPaletteIndex);

/* 0x421BC0 */
int rtsvga2_calcpositionVGA(uint16_t x, uint16_t y);

/* 0x421BF0 */
void rtsvga2_drawshapeVGA(const uint8_t* rleData, int16_t x, int16_t y, int16_t transparentColor,
						  int16_t mirror);

/* 0x421C20 */
void rtsvga2__lowdrawshapeVGA(const uint8_t* rleData, int16_t x, int16_t y, int16_t transparentColor,
							  int16_t mirror, uint8_t mode);

/* 0x421E10 */
uint8_t rtsvga2_drawdotVGA(uint16_t x, uint16_t y, uint8_t colorIndex);

/* 0x421E80 */
void rtsvga2_outcharVGA(char character);

/* 0x422350 */
void rtsvga2_outchar32VGA(char character);

/* 0x422800 */
void rtsvga2_clearwindowVGA(void);

/* 0x422840 */
void rtsvga2_fillrectangleVGA(void);

/* 0x4229F0 */
void rtsvga2_fillboxVGA(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

/* 0x422A90 */
void rtsvga2_autofillVGA(void);

/* 0x422B20 */
void rtsvga2_saveboxVGA(uint8_t* buffer, uint16_t xByteOffset, uint16_t y, uint16_t width, uint16_t height);

/* 0x422C00 */
void rtsvga2_restoreboxVGA(const uint8_t* buffer, uint16_t xByteOffset, uint16_t y, uint16_t width,
						   uint16_t height);

/* 0x422CD0 */
void rtsvga2_drawblips(struct XwRadarBlip* blips, int count);

/* 0x422E20 */
void rtsvga2_removeblips(struct XwRadarBlip* blips, int count);

/* 0x422F40 */
void rtsvga2_drawbracket(void);

/* 0x423030 */
void rtsvga2_removebracket(void);

/* 0x423110 */
void rtsvga2_drawcross(uint16_t x, uint16_t y, uint8_t paletteIndex);

/* 0x423210 */
void rtsvga2_removecross(uint16_t x, uint16_t y);

#ifdef __cplusplus
}
#endif

#endif
