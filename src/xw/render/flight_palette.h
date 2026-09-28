#ifndef XW_RENDER_FLIGHT_PALETTE_H
#define XW_RENDER_FLIGHT_PALETTE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

typedef struct RgbTriplet RgbTriplet;

/* Original IDB size: 3 bytes. */
struct RgbTriplet {
	/* IDB +0x0 */
	uint8_t r;
	/* IDB +0x1 */
	uint8_t g;
	/* IDB +0x2 */
	uint8_t b;
};

enum {
	FLIGHT_PALETTE_RGB6_TO_RGB5_SHIFT = 1,
	FLIGHT_PALETTE_RGB555_RED_SHIFT = 10,
	FLIGHT_PALETTE_DIRECT_RED_MASK = 0x3E,
	FLIGHT_PALETTE_DIRECT_GREEN_MASK = 0xFE,
	FLIGHT_PALETTE_DIRECT_RED_SHIFT = 9,
	FLIGHT_PALETTE_DIRECT_GREEN_SHIFT = 4
};

/* Declarations follow ascending original IDB address. */

/* 0x41FC10 */
void FlightPalette_Build16BppRange(const struct RgbTriplet* sourceRgb6, uint16_t* destinationPixels,
								   int firstIndex, int entryCount);

/* 0x4239E0 */
void FlightPalette_ResetIf8Bit(void);

#ifdef __cplusplus
}
#endif

#endif
