#include "xw_remaster/ship.h"
#include "xw_remaster/special_world_internal.h"
#include <aeron/asset/opt_model.h>
#include <string.h>

static bool Windows(AeronScene3D* scene, const XwRenderSnapshot* s, const XwPreparedFlight* frame,
					const XwRenderAssetSet* set, unsigned index, bool reset, unsigned counts[2]) {
	const XwSnapObject* object = &s->objects[index];
	float relative[3], depth = 0;
	XwRenderMath_Local(s->camera.world_pos, object->world_pos, relative);
	for (unsigned a = 0; a < 3; ++a)
		depth += relative[a] * s->camera.rows[6 + a];
	if ((s->appearance.graphics_detail == 0 && depth > 0x20000) ||
		(s->appearance.graphics_detail == 1 && depth > 0x30000))
		return true;
	if (object->type >= set->type_count)
		return false;
	const XwMeshAsset* asset = XwRemasterAssets_Mesh(set->types[object->type].geometry);
	if (!asset || !asset->mesh)
		return false;
	unsigned count = depth > 65536 ? 1 : object->type == 202 ? 2 : 10;
	if (count > asset->component_count)
		return false;
	const XwPreparedObject* pose = &frame->objects[index];
	bool zero = reset || pose->zero_velocity || !frame->regenerate_motion;
	if (!zero && pose->previous_index >= 0 && (unsigned)pose->previous_index < frame->previous_object_count) {
		const XwSnapObject* old = &frame->previous_objects[pose->previous_index];
		zero = old->type != object->type || old->state != object->state ||
			   old->type_specific != object->type_specific || old->checkpoint_lit != object->checkpoint_lit;
	}
	for (unsigned component = 0; component < count; ++component) {
		if (component > 3 && !(object->state & (1u << (component - 4))))
			continue;
		unsigned variant = component && component < 4 && object->type != 202 &&
								   (object->checkpoint_lit & (1u << (component - 1)))
							   ? (object->type_specific >> (2 * (component - 1))) & 3
							   : 0;
		AeronSceneMeshInstance instance = { .mesh = asset->mesh,
											.mesh_table = XwWorld_Component(component),
											.variant = variant,
											.zero_velocity = zero,
											.cull_mode = AERON_CULL_BACK };
		memcpy(instance.transform, pose->transform, sizeof instance.transform);
		memcpy(instance.prev_transform, pose->previous_transform, sizeof instance.prev_transform);
		XwGateLights_Submit(scene, asset->gate_lights, component, variant, instance.transform);
		if (!XwWorld_Submit(scene, &frame->view, &instance,
							asset->mesh->bound_radius * AERON_OPT_UNITS_PER_METER, counts))
			return false;
	}
	return true;
}

bool XwWorld_Course(AeronScene3D* scene, const XwRenderSnapshot* s, const XwPreparedFlight* frame,
					const XwRenderAssetSet* set, bool reset, unsigned counts[2]) {
	if (s->hyperspace.phase == 3 || s->hyperspace.phase == 5)
		return true;
	for (unsigned i = 0; i < s->object_count; ++i) {
		const XwSnapObject* object = &s->objects[i];
		if (object->id.kind != XW_SNAP_OBJECT_MISSION || object->genus != 14)
			continue;
		if (s->flight_version == 98) {
			if (!Windows(scene, s, frame, set, i, reset, counts))
				return false;
		} else if (!XwWorld_DosCourse(s, set, object, &frame->view))
			return false;
	}
	return true;
}
