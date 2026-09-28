#ifndef XW_REMASTER_FLIGHT_H
#define XW_REMASTER_FLIGHT_H
#include "xw_remaster/render_math.h"

typedef struct XwPreparedObject {
	float transform[16], previous_transform[16];
	int32_t previous_index;
	bool zero_velocity;
} XwPreparedObject;

typedef struct XwPreparedFlight {
	XwRenderView view, previous_view;
	XwSnapCamera previous_camera;
	XwLayoutTransform cockpit_layout, frontend_layout;
	AeronRectI content_rect;
	XwPreparedObject objects[XW_SNAP_OBJECTS];
	/* Owned previous visual state; indices never refer to a recycled host snapshot. */
	XwSnapObject previous_objects[XW_SNAP_OBJECTS];
	XwSnapCraft previous_crafts[XW_SNAP_CRAFTS];
	uint32_t object_count, previous_object_count, previous_craft_count;
	XwSnapViewKey key;
	XwRenderAssetId assets, previous_assets;
	uint64_t host_serial, velocity_span_us;
	float delta_seconds;
	bool valid, drawable, reset_history, regenerate_motion, render_needed;
} XwPreparedFlight;

bool XwRemasterFlight_Prepare(const XwRenderSnapshot* current, int pixel_width, int pixel_height);
const XwPreparedFlight* XwRemasterFlight_Current(void);
void XwRemasterFlight_Invalidate(void);
void XwRemasterFlight_Shutdown(void);
#endif
