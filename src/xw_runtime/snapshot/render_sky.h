#ifndef XW_RENDER_SKY_H
#define XW_RENDER_SKY_H
#include "xw_runtime/snapshot/render_types.h"
#include <stdbool.h>

enum { XW_SKY_STAR_COUNT = 3072 };

typedef struct XwRenderStars {
	uint8_t indices[XW_SKY_STAR_COUNT];
	uint32_t colors[XW_SKY_STAR_COUNT]; /* Resolved sRGB 0xAARRGGBB. */
} XwRenderStars;

typedef struct XwRenderDosStars {
	uint8_t jitter[256], brightness[256];
} XwRenderDosStars;

void XwRenderSky_DosHyperstar(unsigned slot, const int32_t relative[3]);
bool XwRenderSky_Capture(XwRenderSnapshot* snapshot);
void XwRenderSky_Stars(const uint8_t* indices, const void* colors);
void XwRenderSky_Hyperstar(unsigned slot, const int32_t position[3], uint16_t roll);
#endif
