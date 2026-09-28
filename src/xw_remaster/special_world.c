#include "xw_remaster/config.h"
#include "xw_remaster/dos_draw.h"
#include "xw_remaster/dos_mesh_internal.h"
#include "xw_remaster/ship.h"
#include "xw_remaster/special_world_internal.h"
#include <aeron/asset/opt_model.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static XwWorldBuild build;

const XwSnapDamageCell* XwWorld_Damage(const XwSnapSpecialWorld* state, unsigned start, uint16_t key) {
	for (unsigned i = 0; i < XW_SNAP_DAMAGE_CELLS; ++i) {
		const XwSnapDamageCell* cell = &state->damage[(start + i) & 63];
		if (!cell->key)
			return NULL;
		if (cell->key == key)
			return cell;
	}
	return NULL;
}

float XwWorld_Depth(const XwWorldBuild* b, const int32_t position[3]) {
	float local[3], depth = 0;
	XwRenderMath_Local(b->snapshot->camera.world_pos, position, local);
	for (unsigned a = 0; a < 3; ++a)
		depth += local[a] * b->snapshot->camera.rows[6 + a];
	return depth;
}

bool XwWorld_Model(XwWorldBuild* b, uint64_t key, unsigned type, unsigned component, int32_t x, int32_t y,
				   int32_t z, unsigned parent, float depth) {
	if (type >= b->assets->type_count || b->model_count >= 131072 ||
		!XwDosMesh_Grow((void**)&b->models, &b->model_capacity, b->model_count + 1, sizeof *b->models))
		return false;
	b->models[b->model_count++] = (XwWorldModel) { .key = key,
												   .position = { x, y, z },
												   .type = type,
												   .component = component,
												   .parent = parent,
												   .depth = depth };
	return true;
}

bool XwWorld_Quad(XwWorldBuild* b, unsigned mesh, const float position[3], const float across[3],
				  const float along[3]) {
	if (b->quad_count >= 4096 ||
		!XwDosMesh_Grow((void**)&b->quads, &b->quad_capacity, b->quad_count + 1, sizeof *b->quads))
		return false;
	XwWorldQuad* q = &b->quads[b->quad_count++];
	*q = (XwWorldQuad) { .mesh = mesh };
	for (unsigned a = 0; a < 3; ++a) {
		q->transform[4 * a] = across[a];
		q->transform[4 * a + 1] = along[a];
		q->transform[4 * a + 3] = position[a];
	}
	q->transform[15] = 1;
	return true;
}

bool XwWorld_Tile(XwWorldBuild* b, unsigned mesh, int32_t x, int32_t y) {
	float p[3];
	XwRenderMath_Local(b->view->origin_world, (int32_t[3]) { x, y, 0 }, p);
	if (!XwWorld_Quad(b, mesh, p, (float[3]) { 1, 0, 0 }, (float[3]) { 0, 1, 0 }))
		return false;
	b->quads[b->quad_count - 1].transform[10] = 1;
	return true;
}

static bool Dos(const XwWorldBuild* b) {
	for (unsigned i = 0; i < b->quad_count; ++i) {
		const XwWorldQuad* q = &b->quads[i];
		XwDosPart part = { .lod = q->mesh, .parent = 0x2000 };
		memcpy(part.transform, q->transform, sizeof part.transform);
		if (!XwDosDraw_AddMesh(&part, XwWorld_DosQuad()))
			return false;
	}
	for (unsigned i = 0; i < b->model_count; ++i) {
		const XwWorldModel* m = &b->models[i];
		XwSnapObject object = { .type = m->type, .pitch = 0x4000, .orientation_dirty = 1, .genus = 14 };
		memcpy(object.world_pos, m->position, sizeof object.world_pos);
		XwDosPart part;
		if (!XwDosShip_Component(b->snapshot, b->assets, &object, b->view, m->component, m->parent, &part) ||
			!XwDosDraw_Add(&part))
			return false;
	}
	return true;
}

bool XwWorld_Submit(AeronScene3D* scene, const XwRenderView* view, AeronSceneMeshInstance* instance,
					float radius, unsigned counts[2]) {
	if (XwShip_Visible(view, instance->transform, radius)) {
		/* Reserve ordinary object/effect capacity; never accept Aeron's silent drop. */
		if (++counts[0] > 1536)
			return false;
		AeronScene_AddMeshInstance(scene, instance);
	} else {
		const AeronSceneShadowSettings* settings = &XwRemasterConfig_Effective()->scene.shadows;
		if (!settings->enabled)
			return true;
		float distance = 0;
		for (unsigned a = 0; a < 3; ++a) {
			float delta = instance->transform[4 * a + 3] - view->camera.pos[a];
			distance += delta * delta;
		}
		if (sqrtf(distance) > settings->max_distance + radius)
			return true;
		if (++counts[1] > 1536)
			return false;
		AeronScene_AddShadowCaster(scene, instance);
	}
	return true;
}

static bool Windows(AeronScene3D* scene, const XwWorldBuild* b, bool reset, unsigned counts[2]) {
	for (unsigned i = 0; i < b->quad_count; ++i) {
		const XwWorldQuad* q = &b->quads[i];
		AeronSceneMeshInstance instance = { .mesh = XwWorld_WindowsQuad(q->mesh), .zero_velocity = reset };
		if (!instance.mesh)
			return false;
		memcpy(instance.transform, q->transform, sizeof instance.transform);
		memcpy(instance.prev_transform, q->transform, sizeof instance.prev_transform);
		if (!XwWorld_Submit(scene, b->view, &instance, instance.mesh->bound_radius, counts))
			return false;
	}
	for (unsigned i = 0; i < b->model_count; ++i) {
		const XwWorldModel* m = &b->models[i];
		const XwMeshAsset* asset = XwRemasterAssets_Mesh(b->assets->types[m->type].geometry);
		if (!asset || !asset->mesh || !asset->component_count)
			return false;
		unsigned component = m->component;
		if (component != UINT16_MAX && component >= asset->component_count)
			component = component == 3 && asset->component_count > 1 ? 1 : 0;
		XwSnapObject object = { .type = m->type, .pitch = 0x4000, .orientation_dirty = 1 };
		memcpy(object.world_pos, m->position, sizeof object.world_pos);
		AeronSceneMeshInstance instance = { .mesh = asset->mesh,
											.zero_velocity = reset,
											.cull_mode = AERON_CULL_BACK,
											.mesh_table = component == UINT16_MAX
															  ? NULL
															  : XwWorld_Component(component) };
		XwRenderMath_ObjectMatrix(&object, 98, b->view->origin_world, instance.transform);
		memcpy(instance.prev_transform, instance.transform, sizeof instance.prev_transform);
		if (!XwWorld_Submit(scene, b->view, &instance, asset->mesh->bound_radius * AERON_OPT_UNITS_PER_METER,
							counts))
			return false;
	}
	return true;
}

bool XwSpecialWorld_Prepare(AeronCommandBuffer* cmd, AeronScene3D* scene, const XwRenderSnapshot* s,
							const XwPreparedFlight* frame, bool reset) {
	const XwRenderAssetSetView* set = XwRenderAssets_Set(s->flight_assets);
	if (!set || set->failed)
		return false;
	unsigned counts[2] = { 0 };
	if (!XwWorld_Course(scene, s, frame, &set->bindings, reset, counts))
		return false;
	if (!s->special.surface_active || (s->camera.rows[8] >= 0.75f && s->camera.world_pos[2] >= 0))
		return true;
	const XwRenderSource* source = XwRenderAssets_Source(set->bindings.special_layout);
	if (!source || source->kind != XW_SOURCE_SPECIAL_LAYOUT || source->size != sizeof(XwRenderWorldLayout) ||
		source->flight_version != s->flight_version)
		return false;
	build.snapshot = s;
	build.assets = &set->bindings;
	build.layout = source->data;
	build.view = &frame->view;
	const AeronSceneShadowSettings* shadows = &XwRemasterConfig_Effective()->scene.shadows;
	build.shadow_distance = shadows->enabled ? shadows->max_distance : 0;
	build.model_count = build.quad_count = 0;
	if (!XwWorld_Surface(&build) || !XwWorld_Trench(&build))
		return false;
	if (s->flight_version != 98)
		return XwWorld_DosMeshes(cmd) && Dos(&build);
	return XwWorld_WindowsMeshes(cmd, &build) && Windows(scene, &build, reset, counts);
}

void XwSpecialWorld_Shutdown(void) {
	XwWorld_MeshesShutdown();
	free(build.models);
	free(build.quads);
	memset(&build, 0, sizeof build);
}
