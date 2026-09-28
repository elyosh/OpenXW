#ifndef XW_RENDER_FLIGHT_STARFIELD_H
#define XW_RENDER_FLIGHT_STARFIELD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

enum {
	FLIGHT_STARFIELD_GRID_SIDE = 32,
	FLIGHT_STARFIELD_PLANE_COUNT = 3,
	FLIGHT_STARFIELD_STAR_COUNT =
		FLIGHT_STARFIELD_GRID_SIDE * FLIGHT_STARFIELD_GRID_SIDE * FLIGHT_STARFIELD_PLANE_COUNT,
	FLIGHT_STARFIELD_JITTER_COUNT = 125,
	FLIGHT_STARFIELD_JITTER_RANDOM_MASK = 127,
	FLIGHT_STARFIELD_INTENSITY_MASK = 15,
	FLIGHT_STARFIELD_MIN_INTENSITY = 8,
	FLIGHT_STARFIELD_PALETTE_FIRST = 64,
	FLIGHT_STARFIELD_RGB555_GRAY = (1 << 10) | (1 << 5) | 1,
	FLIGHT_STARFIELD_RGB565_GRAY = (1 << 11) | (1 << 6) | 1,
	FLIGHT_STARFIELD_RANDOM_ALLOCATION_BYTES = FLIGHT_STARFIELD_STAR_COUNT * 3
};

extern int g_starfieldGridDimension;
extern int g_starfieldColors8Initialized;
extern int g_starfieldColors16Initialized;
extern int g_starfieldRandomVectorIndicesInitialized;
extern uint16_t g_starfieldColors8Handle;
extern uint16_t g_starfieldColors16Handle;
extern uint16_t g_starfieldRandomVectorIndicesHandle;
extern int g_starfieldColorCacheReusable;
extern int g_starfieldJitterY[FLIGHT_STARFIELD_JITTER_COUNT];
extern int g_starfieldJitterZ[FLIGHT_STARFIELD_JITTER_COUNT];
extern int g_starfieldJitterX[FLIGHT_STARFIELD_JITTER_COUNT];

/* Declarations follow ascending original IDB address. */

/* 0x4232F0 */
void FlightStarfield_Render(void);

#ifdef __cplusplus
}
#endif

#endif
