#ifndef XW_FLIGHT_HUD_HUD_H
#define XW_FLIGHT_HUD_HUD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

enum {
	HUD_BOX_DOT_SIZE = 4,
	HUD_BOX_NEAR_DEPTH = 1,
	HUD_BOX_CORNER_SHIFT = 3,
	HUD_BOX_MIN_CORNER_LENGTH = 3,
	HUD_BOX_VERTEX_BUDGET = 32,
	HUD_BOX_TRIANGLE_BUDGET = 16,
	HUD_BOX_SEGMENT_VERTEX_COUNT = 4,
	HUD_BOX_PALETTE_ALPHA_BIAS = 64,
	HUD_BOX_COLOR_CHANNEL_SHIFT = 8,
	HUD_BOX_RGB6_EXPAND_SHIFT = 2
};

enum { HUD_BOX_SCRATCH_PIXEL_COUNT = 640 };

extern uint16_t g_panelBoxSpanScratch[HUD_BOX_SCRATCH_PIXEL_COUNT];

/* Declarations follow ascending original IDB address. */

/* 0x483280 */
void Hud_DrawBoxOverlayHW(int x, int y, int width, int height, int colorIdx, int depth);

/* 0x49DB90 */
void Hud_DrawBoxInXTrans(int x, int y, int width, int height, int colorIndex, int viewDepth);

#ifdef __cplusplus
}
#endif

#endif
