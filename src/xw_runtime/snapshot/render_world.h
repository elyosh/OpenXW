#ifndef XW_RENDER_WORLD_H
#define XW_RENDER_WORLD_H
#include "xw_runtime/snapshot/render_assets.h"

enum { XW_WORLD_LAYOUTS = 14, XW_WORLD_PLACEMENTS = 19 };

typedef struct XwRenderPlacement {
	uint8_t position, type, health;
} XwRenderPlacement;

typedef struct XwRenderPlacementList {
	uint8_t count;
	XwRenderPlacement entries[XW_WORLD_PLACEMENTS];
} XwRenderPlacementList;

typedef struct XwRenderWorldQuad {
	float positions[4][3], uv[4][2], normal[3];
} XwRenderWorldQuad;

typedef struct XwRenderWorldLayout {
	XwRenderPlacementList surface[5], trench[9];
	XwRenderWorldQuad quads[4]; /* Windows surface, right wall, floor, left wall. */
	float detail_scale[4], minimum_size, minimum_large_size;
	uint8_t surface_colors[28], trench_colors[18];
} XwRenderWorldLayout;

bool XwRenderWorld_Capture(XwRenderSnapshot* snapshot);
/* Observe the resolved Windows checkpoint appearance after its original clock advances. */
void XwRenderWorld_Checkpoint(unsigned slot, unsigned component, bool lit);
#endif
