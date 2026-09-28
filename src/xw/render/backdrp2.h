#ifndef XW_RENDER_BACKDRP2_H
#define XW_RENDER_BACKDRP2_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

enum { BACKDRP2_PROJECTION_OVERFLOW = 0x7FFFFF00, BACKDRP2_SHIFT_MASK = 31 };

enum { BACKDRP2_CAMERA_DEPTH = 0x100000, BACKDRP2_VIEW_DEPTH = 0x7FFFFFFF };

enum {
	BACKDRP2_CAMERA_STEP_COUNT = 16,
	BACKDRP2_CAMERA_STEP_SHIFT = 5,
	BACKDRP2_STAR_DIRECTION_SIDE = 5,
	BACKDRP2_STAR_DIRECTION_RADIUS = 2,
	BACKDRP2_STAR_DIRECTION_SHIFT = 7,
	BACKDRP2_DIRECTION_MAGNITUDE_MASK = 7,
	BACKDRP2_DIRECTION_LOW_NEGATIVE = 8,
	BACKDRP2_DIRECTION_HIGH_NEGATIVE = 0x80,
	BACKDRP2_DIRECTION_HIGH_SHIFT = 4,
	BACKDRP2_AXIS_DEPTH_SHIFT = 2
};

extern int g_cameraWorldXToViewYSteps[BACKDRP2_CAMERA_STEP_COUNT];
extern int g_cameraWorldXToViewZSteps[BACKDRP2_CAMERA_STEP_COUNT];
extern int g_cameraWorldXToViewXSteps[BACKDRP2_CAMERA_STEP_COUNT];
extern int g_cameraWorldYToViewYSteps[BACKDRP2_CAMERA_STEP_COUNT];
extern int g_cameraWorldYToViewZSteps[BACKDRP2_CAMERA_STEP_COUNT];
extern int g_cameraWorldYToViewXSteps[BACKDRP2_CAMERA_STEP_COUNT];
extern int g_cameraWorldZToViewYSteps[BACKDRP2_CAMERA_STEP_COUNT];
extern int g_cameraWorldZToViewZSteps[BACKDRP2_CAMERA_STEP_COUNT];
extern int g_cameraWorldZToViewXSteps[BACKDRP2_CAMERA_STEP_COUNT];

/* Declarations follow ascending original IDB address. */

/* 0x402420 */
void backdrp2_backdrop(void);

/* 0x402CC0 */
void backdrp2_backdrawbitmap(int viewX, int viewY, int viewDepth, int rotationAngle,
							 int backdropIndexPlusOne);

/* 0x408F20 */
void backdrp2_DrawModelTexQuadAtScreen(uint16_t objectType, uint16_t screenX, uint16_t screenY,
									   unsigned int rotationAngle);

#ifdef __cplusplus
}
#endif

#endif
