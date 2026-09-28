/* Two-event component interpolation adapted from OpenXvT component_animation.c. */
#include "xw_remaster/component_animation.h"
#include "xw_runtime/snapshot/render_assets.h"
#include <string.h>

typedef struct ComponentPose {
	uint64_t view, event;
	XwRenderAssetId model;
	uint32_t generation;
	uint8_t type, count;
	bool valid;
	uint8_t from[XW_SNAP_COMPONENTS], to[XW_SNAP_COMPONENTS];
	uint8_t hp[XW_SNAP_COMPONENTS], state[XW_SNAP_COMPONENTS];
	float current[XW_SNAP_COMPONENTS], previous[XW_SNAP_COMPONENTS];
} ComponentPose;

static ComponentPose poses[XW_SNAP_OBJECTS];
static uint64_t mission, world, view, view_time;

void XwComponentAnimation_Reset(void) {
	memset(poses, 0, sizeof poses);
	mission = world = view = view_time = 0;
}

static void PrepareObject(const XwRenderSnapshot* s, const XwSnapObject* object, const XwSnapCraft* craft,
						  XwRenderAssetId model) {
	ComponentPose* pose = &poses[object->id.slot];
	bool reset = !pose->valid || pose->view != view || pose->generation != object->id.generation ||
				 pose->type != object->type || pose->model != model || pose->count != craft->component_count;
	bool event = pose->event != s->component_event_serial;
	if ((event && pose->event + 1 != s->component_event_serial) ||
		s->component_event_time_ticks > s->component_view_time_ticks)
		reset = true;
	uint64_t age = s->component_view_time_ticks >= s->component_event_time_ticks
					   ? s->component_view_time_ticks - s->component_event_time_ticks
					   : 0;
	float alpha =
		age >= s->component_event_interval_ticks ? 1.0f : (float)age / s->component_event_interval_ticks;
	for (unsigned i = 0; i < craft->component_count; ++i) {
		uint8_t rotation = craft->mesh_rotation[i];
		bool discontinuity = reset || !craft->component_hp[i] || craft->component_state[i] ||
							 pose->hp[i] != craft->component_hp[i] ||
							 pose->state[i] != craft->component_state[i] ||
							 (!event && pose->to[i] != rotation);
		/* The DOS closed B-Wing pose has a distinct authored translation/rotation branch. */
		if (s->flight_version != 98 && object->type == 118 && pose->to[i] != rotation &&
			(pose->to[i] == 64 || rotation == 64))
			discontinuity = true;
		pose->previous[i] = pose->current[i];
		if (discontinuity)
			pose->from[i] = pose->to[i] = rotation;
		else if (event) {
			pose->from[i] = pose->to[i];
			pose->to[i] = rotation;
		}
		int delta = (int8_t)(uint8_t)(pose->to[i] - pose->from[i]);
		pose->current[i] = pose->from[i] + delta * alpha;
		if (discontinuity)
			pose->previous[i] = pose->current[i];
		pose->hp[i] = craft->component_hp[i];
		pose->state[i] = craft->component_state[i];
	}
	pose->model = model;
	pose->event = s->component_event_serial;
	pose->view = s->key.view_serial;
	pose->generation = object->id.generation;
	pose->type = object->type;
	pose->count = craft->component_count;
	pose->valid = true;
}

void XwComponentAnimation_Prepare(const XwRenderSnapshot* s, bool reset) {
	if (!s || !s->world_valid || !s->flight_unlocked || !s->component_event_interval_ticks) {
		XwComponentAnimation_Reset();
		return;
	}
	if (reset || mission != s->key.mission_generation || world != s->key.world_generation ||
		s->component_view_time_ticks < view_time)
		XwComponentAnimation_Reset();
	const XwRenderAssetSetView* assets = XwRenderAssets_Set(s->flight_assets);
	if (!assets) {
		XwComponentAnimation_Reset();
		return;
	}
	mission = s->key.mission_generation;
	world = s->key.world_generation;
	if (view == s->key.view_serial)
		return;
	for (unsigned i = 0; i < s->object_count; ++i) {
		const XwSnapObject* object = &s->objects[i];
		if (object->id.kind != XW_SNAP_OBJECT_MOBILE || object->id.slot >= XW_SNAP_OBJECTS ||
			object->genus > 4 || object->craft_index >= s->craft_count ||
			(object->type != 1 && object->type != (s->flight_version == 98 ? 4 : 118)))
			continue;
		const XwSnapCraft* craft = &s->crafts[object->craft_index];
		if (craft->component_count > XW_SNAP_COMPONENTS || object->type >= assets->bindings.type_count)
			continue;
		PrepareObject(s, object, craft, assets->bindings.types[object->type].geometry);
	}
	view = s->key.view_serial;
	view_time = s->component_view_time_ticks;
}

const float* XwComponentAnimation_Angles(const XwSnapObject* object, bool previous) {
	if (!object || object->id.kind != XW_SNAP_OBJECT_MOBILE || object->id.slot >= XW_SNAP_OBJECTS)
		return NULL;
	const ComponentPose* pose = &poses[object->id.slot];
	if (!pose->valid || pose->view != view || pose->generation != object->id.generation ||
		pose->type != object->type)
		return NULL;
	return previous ? pose->previous : pose->current;
}
