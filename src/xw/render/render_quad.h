#ifndef XW_RENDER_RENDER_QUAD_H
#define XW_RENDER_RENDER_QUAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

enum {
	RENDER_QUAD_VERTEX_CAPACITY = 40,
	RENDER_QUAD_VERTEX_COUNT = 4,
	RENDER_QUAD_COLOR_COUNT = 32,
	RENDER_QUAD_TEXTURE_SIZE_BITS = 8,
	RENDER_QUAD_HALF_EXTENT_SHIFT = 9,
	RENDER_QUAD_DISTANT_DEPTH_THRESHOLD = 0x1000000
};

extern const float g_hardwareDistantBillboardDepth;
extern const float g_hardwareDistantBillboardReversedDepth;
extern unsigned int g_billboardGenus13FrameColors[RENDER_QUAD_COLOR_COUNT];
extern int g_bilinearEnabled;

struct XwBitmapFramePrefix;
/* Declarations follow ascending original IDB address. */

/* 0x482AB0 */
void RenderQuad_DrawRotatedSprite(unsigned int angle, int screenCenterX, int screenCenterY, uint16_t scaleQ8,
								  struct XwBitmapFramePrefix* frame);

#ifdef __cplusplus
}
#endif

#endif
