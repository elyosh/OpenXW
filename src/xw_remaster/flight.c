/* Prepared pose ownership and resize coalescing follow OpenXvT flight.c. */
#include "xw_remaster/flight.h"
#include "xw_remaster/component_animation.h"
#include "xw_remaster/config.h"
#include "xw_runtime/snapshot/render_assets.h"
#include <aeron/aeron.h>
#include <string.h>

typedef struct VisualPose {
	XwSnapCamera camera;
	XwSnapViewKey key;
	XwRenderAssetId assets;
	XwSnapObject objects[XW_SNAP_OBJECTS];
	XwSnapCraft crafts[XW_SNAP_CRAFTS];
	uint32_t object_count, craft_count;
	uint64_t simulation_ticks, capture_host_us, presentation;
	uint8_t version, hyperspace_phase;
} VisualPose;

static VisualPose pose;
static XwPreparedFlight frame;
static uint64_t last_config, resize_since, last_palette, last_hud;
static int width, height, requested_width, requested_height, last_hdr;
static float last_headroom;
static bool minimized, last_paused, last_focused;

static bool ViewDiscontinuity(const XwSnapCamera* a, const XwSnapCamera* b) {
	return !XwRenderMath_SameObject(a->player, b->player) || !XwRenderMath_SameObject(a->focus, b->focus) ||
		   a->external != b->external || a->replay_mode != b->replay_mode || a->hud_state != b->hud_state ||
		   a->focal_x != b->focal_x || a->aspect_y_q16 != b->aspect_y_q16 ||
		   a->screen_width != b->screen_width || a->screen_height != b->screen_height ||
		   memcmp(&a->viewport, &b->viewport, sizeof a->viewport) || a->center_x != b->center_x ||
		   a->center_y != b->center_y || a->projection_offset_y != b->projection_offset_y;
}

void XwRemasterFlight_Invalidate(void) {
	XwComponentAnimation_Reset();
	XwRenderAssets_Release(pose.assets);
	pose.assets = 0;
	XwRenderAssets_Release(frame.previous_assets);
	frame.previous_assets = 0;
	frame.assets = 0;
	frame.valid = frame.drawable = false;
	frame.render_needed = frame.regenerate_motion = false;
}

const XwPreparedFlight* XwRemasterFlight_Current(void) { return frame.valid ? &frame : NULL; }

static void SavePose(const XwRenderSnapshot* s) {
	XwRenderAssets_Retain(s->flight_assets);
	XwRenderAssets_Release(pose.assets);
	pose.camera = s->camera;
	pose.key = s->key;
	pose.assets = s->flight_assets;
	pose.object_count = s->object_count;
	pose.craft_count = s->craft_count;
	memcpy(pose.objects, s->objects, s->object_count * sizeof s->objects[0]);
	memcpy(pose.crafts, s->crafts, s->craft_count * sizeof s->crafts[0]);
	pose.capture_host_us = s->capture_host_us;
	pose.simulation_ticks = s->simulation_ticks;
	pose.presentation = s->presentation_generation;
	pose.version = s->flight_version;
	pose.hyperspace_phase = s->hyperspace.phase;
}

static unsigned ObjectOrder(XwSnapObjectId id) { return (unsigned)id.kind * 65536u + id.slot; }

static bool Advance(const XwRenderSnapshot* s, bool reset, int w, int h) {
	frame.previous_camera = reset ? s->camera : pose.camera;
	frame.velocity_span_us =
		!reset && s->capture_host_us > pose.capture_host_us ? s->capture_host_us - pose.capture_host_us : 0;
	frame.delta_seconds = !reset && s->simulation_ticks > pose.simulation_ticks
							  ? (float)(s->simulation_ticks - pose.simulation_ticks) / 236.0f
							  : 0;
	if (reset)
		frame.previous_view = frame.view;
	else if (!XwRenderMath_BuildMainView(&pose.camera, s->camera.world_pos, w, h, &frame.previous_view))
		return false;
	XwRenderAssets_Retain(reset ? s->flight_assets : pose.assets);
	XwRenderAssets_Release(frame.previous_assets);
	frame.previous_assets = reset ? s->flight_assets : pose.assets;
	frame.previous_object_count = reset ? s->object_count : pose.object_count;
	frame.previous_craft_count = reset ? s->craft_count : pose.craft_count;
	memcpy(frame.previous_objects, reset ? s->objects : pose.objects,
		   frame.previous_object_count * sizeof frame.previous_objects[0]);
	memcpy(frame.previous_crafts, reset ? s->crafts : pose.crafts,
		   frame.previous_craft_count * sizeof frame.previous_crafts[0]);
	unsigned previous = 0;
	for (unsigned i = 0; i < s->object_count; ++i) {
		const XwSnapObject* object = &s->objects[i];
		XwPreparedObject* out = &frame.objects[i];
		XwRenderMath_ObjectMatrix(object, s->flight_version, s->camera.world_pos, out->transform);
		memcpy(out->previous_transform, out->transform, sizeof out->transform);
		out->previous_index = -1;
		out->zero_velocity = true;
		if (reset)
			continue;
		while (previous < pose.object_count &&
			   ObjectOrder(pose.objects[previous].id) < ObjectOrder(object->id))
			++previous;
		if (previous == pose.object_count)
			continue;
		const XwSnapObject* old = &pose.objects[previous];
		if (!XwRenderMath_SameObject(old->id, object->id) || old->type != object->type ||
			old->source_type != object->source_type)
			continue;
		out->previous_index = previous;
		out->zero_velocity = false;
		XwRenderMath_ObjectMatrix(old, s->flight_version, s->camera.world_pos, out->previous_transform);
	}
	SavePose(s);
	return true;
}

static bool Expanded(const XwSnapCamera* camera) {
	/* Replay supplies an authored bitmap/control frame, even with an external camera. */
	return !camera->replay_mode &&
		   (camera->hud_state == XW_SNAP_VIEW_NO_COCKPIT || camera->hud_state == XW_SNAP_VIEW_FULL_FORWARD);
}

bool XwRemasterFlight_Prepare(const XwRenderSnapshot* s, int w, int h) {
	if (!s || !s->world_valid || s->owner != XW_SNAP_OWNER_FLIGHT || s->object_count > XW_SNAP_OBJECTS ||
		s->craft_count > XW_SNAP_CRAFTS) {
		XwRemasterFlight_Invalidate();
		return true;
	}
	if (w <= 0 || h <= 0) {
		/* Keep owned poses/extents; no target creation or motion regeneration while minimized. */
		minimized = true;
		frame.drawable = frame.render_needed = frame.regenerate_motion = false;
		return true;
	}
	uint64_t now = Aeron_NowUs();
	if (w != requested_width || h != requested_height) {
		requested_width = w;
		requested_height = h;
		resize_since = now;
	}
	if (frame.valid && !minimized && (w != width || h != height) && now - resize_since < 150000) {
		w = width;
		h = height;
	}
	uint64_t config = XwRemasterConfig_Generation();
	bool reset = !frame.valid || minimized || width != w || height != h || last_config != config ||
				 pose.key.mission_generation != s->key.mission_generation ||
				 pose.key.world_generation != s->key.world_generation || pose.version != s->flight_version ||
				 pose.presentation != s->presentation_generation || pose.assets != s->flight_assets ||
				 pose.hyperspace_phase != s->hyperspace.phase ||
				 s->simulation_ticks < pose.simulation_ticks || s->capture_host_us < pose.capture_host_us ||
				 ViewDiscontinuity(&s->camera, &pose.camera) || (last_paused && !s->paused) ||
				 (!last_focused && s->focused);
	bool advance = reset || pose.key.view_serial != s->key.view_serial;
	if (!XwRenderMath_BuildMainView(&s->camera, s->camera.world_pos, w, h, &frame.view) ||
		!XwRenderMath_Layout(s->camera.screen_width, s->camera.screen_height, w, h, &frame.cockpit_layout) ||
		!XwRenderMath_Layout(640, 480, w, h, &frame.frontend_layout) ||
		(advance && !Advance(s, reset, w, h))) {
		XwRemasterFlight_Invalidate();
		return false;
	}
	if (advance)
		XwComponentAnimation_Prepare(s, reset);
	frame.content_rect =
		Expanded(&s->camera) ? (AeronRectI) { 0, 0, w, h } : XwRenderMath_LayoutRect(&frame.cockpit_layout);
	bool hdr = Aeron_OutputHdrEnabled();
	float headroom = Aeron_OutputHdrHeadroom();
	const XwRenderSettings* settings = XwRemasterConfig_Effective();
	bool temporal = s->flight_version == 98 && settings->temporal_mode != AERON_TEMPORAL_OFF;
	bool motion_changed = frame.regenerate_motion != advance;
	frame.render_needed = advance || temporal || last_palette != s->appearance.palette_revision ||
						  last_hud != s->key.hud_revision || last_hdr != hdr || last_headroom != headroom ||
						  last_paused != s->paused ||
						  (!settings->motion_blur.pause_keep_blur && motion_changed);
	frame.regenerate_motion = advance;
	frame.reset_history = reset;
	frame.object_count = s->object_count;
	frame.host_serial = s->host_serial;
	frame.key = s->key;
	frame.assets = s->flight_assets;
	frame.valid = frame.drawable = true;
	width = w;
	height = h;
	minimized = false;
	last_config = config;
	last_palette = s->appearance.palette_revision;
	last_hud = s->key.hud_revision;
	last_hdr = hdr;
	last_headroom = headroom;
	last_paused = s->paused;
	last_focused = s->focused;
	return true;
}

void XwRemasterFlight_Shutdown(void) {
	XwRemasterFlight_Invalidate();
	memset(&pose, 0, sizeof pose);
	memset(&frame, 0, sizeof frame);
	width = height = requested_width = requested_height = last_hdr = 0;
	last_config = resize_since = last_palette = last_hud = 0;
	last_headroom = 0;
	minimized = last_paused = last_focused = false;
}
