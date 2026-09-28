#ifndef XW_RENDER_RTSRGB_H
#define XW_RENDER_RTSRGB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/render/rtsvga2.h"

#include <stddef.h>
#include <stdint.h>

struct XwRadarBlip;

enum { RTSRGB_PALETTE_COLOR_COUNT = 256, RTSRGB_RADAR_BACKGROUND_COLOR = 33 };

enum { RTSRGB_BRACKET_PIXEL_COUNT = 10, RTSRGB_CROSS_PIXEL_COUNT = 7 };

enum { RTSRGB_BRACKET_COLOR = 7 };

extern const FlightMarkerOffset g_radarBracketOffsets16[RTSRGB_BRACKET_PIXEL_COUNT];
extern const int8_t g_crossMarkerOffsets16[RTSRGB_CROSS_PIXEL_COUNT * 2];
extern uint16_t g_flightFillRectBottom;
extern uint16_t g_flightFillRectRight;
extern uint16_t g_flightFillRectLeft;
extern uint16_t g_flightFillRectTop;
extern uint16_t g_radarBracketSavedPixels16[RTSRGB_BRACKET_PIXEL_COUNT];
extern uint16_t g_flightFillRectCurrentY;
extern uint16_t g_crossMarkerSavedPixels16[RTSRGB_CROSS_PIXEL_COUNT];
extern unsigned int g_flightFillRectRemainingRows;
extern uint16_t g_flightTextPalette[RTSRGB_PALETTE_COLOR_COUNT];
extern uint16_t g_savedRowPixelsRemaining;
/* Declarations follow ascending original IDB address. */

/* 0x4201F0 */
void rtsrgb_drawshapeRGB(const uint8_t* rleData, int16_t x, int16_t y, int16_t transparentColor,
						 int16_t mirror);

/* 0x420220 */
void rtsrgb__lowdrawshapeRGB(const uint8_t* rleData, int16_t x, int16_t y, int16_t transparentColor,
							 int16_t mirror, uint8_t mode);

/* 0x420450 */
void rtsrgb_outcharRGB(char ch);

/* 0x420750 */
void rtsrgb_clearwindowRGB(void);

/* 0x420790 */
void rtsrgb_fillrectangleRGB(void);

/* 0x420890 */
void rtsrgb_fillboxRGB(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

/* 0x420930 */
void rtsrgb_autofillRGB(void);

/* 0x4209C0 */
void rtsrgb_saveboxRGB(uint8_t* buffer, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

/* 0x420AA0 */
void rtsrgb_restoreboxRGB(const uint8_t* buffer, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

/* 0x420B80 */
void rtsrgb_drawblipsRGB(struct XwRadarBlip* blips, int count);

/* 0x420C50 */
void rtsrgb_removeblipsRGB(struct XwRadarBlip* blips, int count);

/* 0x420CF0 */
void rtsrgb_drawbracketRGB(void);

/* 0x420DD0 */
void rtsrgb_removebracketRGB(void);

/* 0x420E90 */
void rtsrgb_drawcrossRGB(uint16_t x, uint16_t y, uint8_t paletteIndex);

/* 0x420F90 */
void rtsrgb_removecrossRGB(uint16_t x, uint16_t y);

#ifdef __cplusplus
}
#endif

#endif
