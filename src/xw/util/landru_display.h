#ifndef XW_UTIL_LANDRU_DISPLAY_H
#define XW_UTIL_LANDRU_DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

enum {
	LANDRU_LEGACY_VIEWPORT_STATE = 2,
	LANDRU_LOW_RESOLUTION_WIDTH = 320,
	LANDRU_LOW_RESOLUTION_HEIGHT = 200,
	LANDRU_BORDER_ROWS = 40,
	LANDRU_BOTTOM_BORDER_ROW = 440,
	LANDRU_BORDER_CLEAR_FRAMES = 2
};

extern int g_landruLogicalWidth;
extern int g_landruLogicalHeight;
extern uint8_t* g_frontendSavedBackground;
extern int g_landruBorderClearCountdown;
/* Write-only state in the original; no reader or more precise meaning is known. */
extern int g_landruLegacyViewportState;
extern int g_landruDoublePixelsEnabled;

/* Declarations follow ascending original IDB address. */

/* 0x49DF80 */
void LandruDisplay_SetLowResolutionMode(int enabled);

/* 0x49DFC0 */
void LandruDisplay_SetLogicalViewport(int doublePixels, int width, int height);

/* 0x49E050 */
void LandruDisplay_ForwardLegacyNoOp(int value);

/* 0x4AE030 */
void LandruDisplay_CopyCanvasToBackBuffer(void);

/* 0x4AED30 */
void LandruDisplay_SaveCanvasBackground(void);

/* 0x4AED80 */
void LandruDisplay_RestoreCanvasBackground(void);

#ifdef __cplusplus
}
#endif

#endif
