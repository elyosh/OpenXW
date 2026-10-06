/* Articulation and conservative bounds adapted from OpenXvT ship.c. */
#include "xw_remaster/ship.h"
#include "xw/assets/model_mesh.h"
#include "xw/flight/object/anim.h"
#include "xw/flight/xw.h"
#include <math.h>
#include <string.h>

static bool Frame(const XwSnapType* type, unsigned state, uint16_t* frame) {
	const XwRenderSource* source = XwRenderAssets_Source(type->animation);
	if (!source || source->kind != XW_SOURCE_ANIMATION || state >= type->animation_count ||
		type->animation_first > source->size / 2 || state >= source->size / 2 - type->animation_first)
		return false;
	memcpy(frame, (const uint16_t*)source->data + type->animation_first + state, sizeof *frame);
	return true;
}

int XwShip_Select(const XwRenderAssetSet* set, const XwSnapObject* object, XwShipSelection* out) {
	if (!set || object->type >= set->type_count)
		return -1;
	unsigned type = object->type;
	uint16_t component = UINT16_MAX;
	unsigned variant = 0;
	bool projectile = XwShip_Projectile(object);
	if (object->id.kind == XW_SNAP_OBJECT_MISSION) {
		if (object->genus < XW_GENUS_MINE || object->genus > XW_GENUS_DEBRIS)
			return 0;
		const XwSnapType* t = &set->types[type];
		if (!t->animation) {
			if (object->state)
				return 0;
		} else {
			uint16_t code;
			if (!Frame(t, object->state, &code))
				return -1;
			if (code >= ANIM_BITMAP_FIRST_FRAME)
				return 0;
			/* static_drawstaticobject selects root zero for every mesh frame. */
			component = 0;
		}
	} else if (type == XW_OBJ_DETACHED_COMPONENT) {
		type = object->source_type;
		component = object->animation_state >> 1;
		variant = object->markings;
	} else if (object->genus == XW_GENUS_DEBRIS || object->genus == XW_GENUS_EXPLOSION_EFFECT) {
		variant = object->markings;
		if (type != XW_MULTI_MESH_DEBRIS_MODEL) {
			const XwSnapType* t = &set->types[type];
			if (!t->animation)
				return 0;
			if (!Frame(t, object->animation_state, &component))
				return -1;
			if (component >= ANIM_BITMAP_FIRST_FRAME)
				return 0;
		}
	} else if (object->genus <= XW_GENUS_STARSHIP) {
		variant = object->markings;
	} else if (!projectile)
		return 0;
	if (type >= set->type_count)
		return -1;
	*out = (XwShipSelection) { .asset_id = set->types[type].geometry,
							   .component = component,
							   .variant = variant };
	if (!out->asset_id)
		return -1;
	/* collide_damagecraft can detach meshes 1..4 from two-mesh A-Wings;
	 * the classic root walk draws nothing for a missing mesh. */
	if (object->type == XW_OBJ_DETACHED_COMPONENT) {
		const XwMeshAsset* asset = XwRemasterAssets_Mesh(out->asset_id);
		if (asset && component >= asset->component_count)
			return 0;
	}
	return 1;
}

bool XwShip_Projectile(const XwSnapObject* object) {
	return object->genus == XW_GENUS_PLAYER_PROJECTILE || object->genus == XW_GENUS_OTHER_PROJECTILE;
}

bool XwShip_Eligible(const XwRenderSnapshot* s, const XwSnapObject* object) {
	if (object->slot_class == XW_SNAP_SLOT_LOCAL_DEBRIS &&
		(!s->appearance.debris_enabled || s->special.proving_grounds_active))
		return false;
	return object->id.kind != XW_SNAP_OBJECT_MISSION ||
		   (s->hyperspace.phase != ANIM_HYPERSPACE_DEPART && s->hyperspace.phase != ANIM_HYPERSPACE_RETURN);
}

static void ship_mat3x4_identity(float out[3][4]) {
	memset(out, 0, 12 * sizeof(float));
	out[0][0] = out[1][1] = out[2][2] = 1.0f;
}

static void ship_mat3x4_rotation_about_pivot(float out[3][4], const float axis[3], const float pivot[3],
											 float angle) {
	float ax = axis[0], ay = axis[1], az = axis[2];
	const float len = sqrtf(ax * ax + ay * ay + az * az);
	if (len < 1e-4f) {
		ship_mat3x4_identity(out);
		return;
	}
	ax /= len;
	ay /= len;
	az /= len;
	const float c = cosf(angle), s = sinf(angle), omc = 1.0f - c;
	out[0][0] = c + ax * ax * omc;
	out[0][1] = ax * ay * omc - az * s;
	out[0][2] = ax * az * omc + ay * s;
	out[1][0] = ay * ax * omc + az * s;
	out[1][1] = c + ay * ay * omc;
	out[1][2] = ay * az * omc - ax * s;
	out[2][0] = az * ax * omc - ay * s;
	out[2][1] = az * ay * omc + ax * s;
	out[2][2] = c + az * az * omc;
	for (int r = 0; r < 3; r++) {
		out[r][3] = pivot[r] - (out[r][0] * pivot[0] + out[r][1] * pivot[1] + out[r][2] * pivot[2]);
	}
}

/* Compose two affine transforms using the shader's row-major convention:
 * out(v) = lhs(rhs(v)). */
static void ship_mat3x4_mul(float out[3][4], const float lhs[3][4], const float rhs[3][4]) {
	float result[3][4];
	for (int r = 0; r < 3; r++) {
		for (int c = 0; c < 3; c++) {
			result[r][c] = lhs[r][0] * rhs[0][c] + lhs[r][1] * rhs[1][c] + lhs[r][2] * rhs[2][c];
		}
		result[r][3] = lhs[r][0] * rhs[0][3] + lhs[r][1] * rhs[1][3] + lhs[r][2] * rhs[2][3] + lhs[r][3];
	}
	memcpy(out, result, sizeof result);
}

bool XwShip_BuildMeshTable(const XwMeshAsset* asset, const XwSnapObject* object, const XwSnapCraft* craft,
						   uint16_t component, const float* rotations, AeronSceneMeshTable* out) {
	if (asset->component_count > XW_SNAP_COMPONENTS ||
		(component != UINT16_MAX && component >= asset->component_count))
		return false;
	if (craft && component == UINT16_MAX && craft->component_count != asset->component_count)
		return false;
	memset(out, 0, sizeof *out);
	float bridge[3][4];
	ship_mat3x4_identity(bridge);
	const float byte_angle = -6.2831853071795864769f / 256.0f;
	if (craft && component == UINT16_MAX && object->type == XW_OBJ_B_WING && asset->bridge_component >= 0 &&
		(unsigned)asset->bridge_component < craft->component_count)
		ship_mat3x4_rotation_about_pivot(
			bridge, (const float[3]) { 0, -1, 0 }, (const float[3]) { 0, 0, 0 },
			(rotations ? rotations[asset->bridge_component] : craft->mesh_rotation[asset->bridge_component]) *
				byte_angle);
	for (unsigned i = 0; i < AERON_MAX_MESH_SLOTS; ++i) {
		float local[3][4];
		ship_mat3x4_identity(local);
		bool visible = i < asset->component_count && (component == UINT16_MAX || component == i);
		if (craft && component == UINT16_MAX && i < craft->component_count) {
			visible = visible && !craft->component_state[i];
			const AeronMeshRot* r = &asset->mesh->mesh_rot[i];
			float rotation = craft->mesh_rotation[i];
			if (rotations && ((r->mesh_type == MODEL_MESH_TYPE_SFOIL &&
							   (object->type == XW_OBJ_X_WING || object->type == XW_OBJ_B_WING)) ||
							  (r->mesh_type == MODEL_MESH_TYPE_BRIDGE && object->type == XW_OBJ_B_WING)))
				rotation = rotations[i];
			if (r->has_rotation && rotation)
				ship_mat3x4_rotation_about_pivot(local, r->axis, r->pivot, rotation * byte_angle);
		}
		ship_mat3x4_mul(out->rows[i], bridge, local);
		out->visibility_packed[i >> 2][i & 3] = visible ? 1 : 0;
		out->emissive_packed[i >> 2][i & 3] = 1;
	}
	return true;
}

void XwShip_ProjectileMatrix(const XwSnapObject* object, const int32_t camera[3], const int32_t origin[3],
							 float out[16]) {
	XwSnapObject aligned = *object;
	float m[16], delta[3];
	XwRenderMath_ObjectMatrix(object, 98, origin, m);
	XwRenderMath_Local(object->world_pos, camera, delta);
	float side = m[0] * delta[0] + m[4] * delta[1] + m[8] * delta[2];
	float up = m[2] * delta[0] + m[6] * delta[1] + m[10] * delta[2];
	aligned.roll =
		(uint16_t)(object->roll + (int)(atan2f(up, side) * 65536.0f / 6.283185307179586f) - 0x4000);
	aligned.orientation_dirty = 1;
	XwRenderMath_ObjectMatrix(&aligned, 98, origin, out);
}

float XwShip_Radius(const XwMeshAsset* asset, const AeronSceneMeshTable* table, const float m[16]) {
	float center[3], r2 = 0;
	for (int i = 0; i < 3; ++i) {
		center[i] = (asset->mesh->bound_min[i] + asset->mesh->bound_max[i]) * 0.5f;
		float half = (asset->mesh->bound_max[i] - asset->mesh->bound_min[i]) * 0.5f;
		r2 += half * half;
	}
	float radius = 0, base_radius = sqrtf(r2);
	for (unsigned i = 0; i < asset->component_count && i < AERON_MAX_MESH_SLOTS; ++i) {
		if (!table->visibility_packed[i >> 2][i & 3])
			continue;
		float distance = 0;
		for (int row = 0; row < 3; ++row) {
			const float* t = table->rows[i][row];
			float value = t[0] * center[0] + t[1] * center[1] + t[2] * center[2] + t[3];
			distance += value * value;
		}
		radius = fmaxf(radius, sqrtf(distance) + base_radius);
	}
	float scale = 0;
	for (int i = 0; i < 3; ++i)
		scale = fmaxf(scale, sqrtf(m[i] * m[i] + m[4 + i] * m[4 + i] + m[8 + i] * m[8 + i]));
	return radius * scale;
}

int XwShip_Visible(const XwRenderView* view, const float m[16], float radius) {
	const float* p = view->view_proj;
	/* Left/right/top/bottom, near and infinite reversed-Z far planes. */
	for (int plane = 0; plane < 6; ++plane) {
		float v[4];
		for (int i = 0; i < 4; ++i) {
			if (plane < 4)
				v[i] = p[12 + i] + (plane & 1 ? -1.0f : 1.0f) * p[(plane / 2) * 4 + i];
			else
				v[i] = plane == 4 ? p[12 + i] - p[8 + i] : p[8 + i];
		}
		float length = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
		if (length && v[0] * m[3] + v[1] * m[7] + v[2] * m[11] + v[3] < -radius * length)
			return 0;
	}
	return 1;
}
