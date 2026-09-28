#ifndef XW_RENDER_RENDERER_H
#define XW_RENDER_RENDERER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <aeron/compat/ddraw.h>
#include <stddef.h>
#include <stdint.h>

struct RgbTriplet;
typedef struct FlightRenderCallbacks FlightRenderCallbacks;

/* Original IDB size: 52 bytes. */
struct FlightRenderCallbacks {
	/* IDB +0x0 */
	void (*initLineBuffer)(void);
	/* IDB +0x4 */
	void (*transitionHook)(void);
	/* IDB +0x8 */
	void (*resetPalette)(void);
	/* IDB +0xC */
	void (*setPaletteRange)(const struct RgbTriplet* rgbTriples, uint16_t startIndex, uint16_t count);
	/* IDB +0x10 */
	void (*getPalette)(struct RgbTriplet* dstPalette);
	/* IDB +0x14 */
	void (*setPalette)(const struct RgbTriplet* rgbTriples);
	/* IDB +0x18 */
	int (*computePixelOffset)(uint16_t x, uint16_t y);
	/* IDB +0x1C */
	void (*blitSprite)(const uint8_t* rleData, int16_t x, int16_t y, int16_t transparentColor,
					   int16_t mirror);
	/* IDB +0x20 */
	void (*drawChar)(char ch);
	/* IDB +0x24 */
	void (*fillClipRect)(void);
	/* IDB +0x28 */
	void (*fillRectClipped)(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
	/* IDB +0x2C */
	void (*saveScreenRect)(uint8_t* buffer, uint16_t xByteOffset, uint16_t y, uint16_t width,
						   uint16_t height);
	/* IDB +0x30 */
	void (*restoreScreenRect)(const uint8_t* buffer, uint16_t xByteOffset, uint16_t y, uint16_t width,
							  uint16_t height);
};

/* Declarations follow ascending original IDB address. */

extern int g_useHardware3D;
extern int g_isPowerVr;

/* 0x47E740 */
IDirectDraw* Renderer_GetDirectDraw(void);

/* 0x47EEF0 */
void Renderer_InitD3DDevice(void);

#ifdef __cplusplus
}
#endif

#endif
