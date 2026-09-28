/* Pure bitmap selection and geometry follow X-Wing anim/static/render_quad; scene policy follows OpenXvT. */
#include "xw_remaster/effects.h"
#include "xw/flight/object/anim.h"
#include "xw/flight/xw.h"
#include "xw/render/render_quad.h"
#include "xw/render/render_scene.h"
#include "xw/render/rotscale.h"
#include "xw_remaster/assets.h"
#include "xw_remaster/config.h"
#include "xw_remaster/ship.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

static const float tau = 6.283185307179586f;

bool XwEffects_Frame(XwRenderAssetId set_id, unsigned type, unsigned frame, XwEffectFrame* out) {
	const XwRenderAssetSetView* set = XwRenderAssets_Set(set_id);
	if (!set || type >= set->bindings.type_count)
		return false;
	const AeronRuntimeAtlas* a = XwRemasterAssets_Image(set->bindings.types[type].bitmaps);
	if (!a)
		return false;
	for (int i = 0; i < a->layout.frame_count; ++i) {
		if ((unsigned)a->layout.ids[i] != frame)
			continue;
		const AeronRuntimeAtlasPage* page = &a->pages[a->layout.pages[i]];
		const AeronSpriteRect* r = &a->layout.frames[i];
		*out = (XwEffectFrame) { page->texture,
								 r->x / page->width,
								 r->y / page->height,
								 (r->x + r->w) / page->width,
								 (r->y + r->h) / page->height,
								 a->layout.classic_w[i],
								 a->layout.classic_h[i] };
		return true;
	}
	return false;
}

void XwEffects_SetFrame(AeronSceneBillboardDesc* b, const XwEffectFrame* f, float strength, float alpha) {
	b->texture = f->texture;
	b->blend = AERON_SCENE_BILLBOARD_BLEND_PMA;
	const float uv[4][2] = { { f->u0, f->v0 }, { f->u1, f->v0 }, { f->u1, f->v1 }, { f->u0, f->v1 } };
	memcpy(b->uv, uv, sizeof uv);
	for (unsigned i = 0; i < 4; ++i) {
		b->colors[i][0] = b->colors[i][1] = b->colors[i][2] = strength * alpha;
		b->colors[i][3] = alpha;
	}
}

static uint16_t Sequence(XwRenderAssetId id, unsigned index) {
	const XwRenderSource* source = XwRenderAssets_Source(id);
	uint16_t result = UINT16_MAX;
	if (source && source->kind == XW_SOURCE_ANIMATION && index < source->size / 2)
		memcpy(&result, (const uint16_t*)source->data + index, 2);
	return result;
}

static uint16_t Code(XwRenderAssetId id, const XwSnapObject* object) {
	const XwRenderAssetSetView* set = XwRenderAssets_Set(id);
	if (!set || object->type >= set->bindings.type_count)
		return UINT16_MAX;
	if (object->type == XW_OBJ_DETACHED_COMPONENT) {
		/* anim_drawverysimpleobject retests the resolved source type. */
		return object->source_type == XW_OBJ_DETACHED_COMPONENT
				   ? Sequence(set->bindings.fragment_sequence, object->secondary_animation_state)
				   : UINT16_MAX;
	}
	const XwSnapType* type = &set->bindings.types[object->type];
	unsigned state = object->id.kind == XW_SNAP_OBJECT_MISSION ? object->state : object->animation_state;
	return state < type->animation_count ? Sequence(type->animation, type->animation_first + state)
										 : UINT16_MAX;
}

static float Angle(const XwSnapObject* object, const XwSnapCamera* camera) {
	float m[16], rows[2][3];
	XwRenderMath_ObjectMatrix(object, 98, camera->world_pos, m);
	for (unsigned col = 0; col < 2; ++col)
		for (unsigned r = 0; r < 3; ++r)
			rows[col][r] = camera->rows[r * 3] * m[col] + camera->rows[r * 3 + 1] * m[4 + col] +
						   camera->rows[r * 3 + 2] * m[8 + col];
	const float* v = fabsf(rows[0][2]) < fabsf(rows[1][2]) ? rows[0] : rows[1];
	return (v[0] < 0 ? 1.0f : -1.0f) * atan2f(v[1], fabsf(v[0]));
}

static unsigned Base(const XwSnapObject* object) {
	if (object->id.kind == XW_SNAP_OBJECT_MISSION || !object->billboard_scale)
		return 256;
	unsigned base = (unsigned)object->billboard_scale << 6;
	return base >= 256 ? base + 256 : base;
}

static float Depth(const XwSnapCamera* camera, const int32_t position[3]) {
	float local[3];
	XwRenderMath_Local(camera->world_pos, position, local);
	return local[0] * camera->rows[6] + local[1] * camera->rows[7] + local[2] * camera->rows[8];
}

/* -1 is a missing required image; 0 is a valid offscreen/zero-sized effect. */
static int Corners(XwRenderAssetId id, const XwSnapObject* o, const XwSnapCamera* camera,
				   const int32_t origin[3], uint16_t code, unsigned base, float roll, float corners[4][3],
				   XwEffectFrame* image) {
	if (code < ANIM_BITMAP_FIRST_FRAME || code >= ANIM_FRAME_JUMP_BASE)
		return 0;
	unsigned type = (code & 0x7fff) >> 7, index = code & 127;
	const XwRenderAssetSetView* set = XwRenderAssets_Set(id);
	if (!set || !XwEffects_Frame(id, type, index, image))
		return -1;
	float depth = Depth(camera, o->world_pos);
	if (depth <= 0 || (double)depth > INT32_MAX)
		return 0;
	unsigned distance = (unsigned)depth >> 8;
	unsigned size = (base * (distance ? set->bindings.types[type].max_extent / distance : 0)) >> 8;
	if (size > ROTSCALE_MAX_PROJECTED_SIZE)
		size = ROTSCALE_MAX_PROJECTED_SIZE;
	float aspect =
		!camera->aspect_y_q16 || camera->aspect_y_q16 == UINT16_MAX ? 1 : camera->aspect_y_q16 / 65536.0f;
	float hw = ((size * (unsigned)image->width) >> 9) * depth / camera->focal_x;
	float hh = ((size * (unsigned)image->height) >> 9) * depth / (camera->focal_x * aspect);
	if (hw <= 0 || hh <= 0)
		return 0;
	float center[3];
	XwRenderMath_Local(origin, o->world_pos, center);
	const int sx[4] = { 1, -1, -1, 1 }, sy[4] = { 1, 1, -1, -1 };
	float c = cosf(roll), sn = sinf(roll);
	for (unsigned v = 0; v < 4; ++v) {
		float x = c * sx[v] * hw + sn * sy[v] * hh * aspect;
		float y = c * sy[v] * hh - sn * sx[v] * hw / aspect;
		for (unsigned a = 0; a < 3; ++a)
			corners[v][a] = center[a] + camera->rows[a] * x + camera->rows[3 + a] * y;
	}
	return 1;
}

static const XwSnapCraft* Craft(const XwSnapObject* o, const XwSnapCraft* crafts, unsigned count) {
	return o->craft_index < count ? &crafts[o->craft_index] : NULL;
}

static unsigned FlameOrdinal(const XwMeshAsset* mesh, const XwSnapCraft* craft, unsigned component) {
	if (!craft || !craft->damage_frame_valid || component >= craft->component_count ||
		craft->component_state[component])
		return 0;
	unsigned ordinal = 0;
	for (unsigned i = 0; i <= component; ++i)
		if (!craft->component_state[i] && mesh->mesh->mesh_rot[i].mesh_type == RENDER_DAMAGE_MESH_TYPE)
			++ordinal;
	return mesh->mesh->mesh_rot[component].mesh_type == RENDER_DAMAGE_MESH_TYPE ? ordinal : 0;
}

static bool SubmitOne(AeronScene3D* scene, const XwRenderSnapshot* s, const XwPreparedFlight* f,
					  unsigned index, const XwMeshAsset* mesh, unsigned component, unsigned ordinal) {
	const XwSnapObject* o = &s->objects[index];
	const XwSnapCraft* craft = Craft(o, s->crafts, s->craft_count);
	uint16_t code = ordinal ? craft->damage_frame : Code(s->flight_assets, o);
	float roll = Angle(o, &s->camera) + ordinal * o->roll * tau / 65536.0f;
	AeronSceneBillboardDesc b = { .stage = AERON_SCENE_BILLBOARD_STAGE_OVERLAY };
	XwEffectFrame image;
	int result = Corners(s->flight_assets, o, &s->camera, s->camera.world_pos, code, ordinal ? 256 : Base(o),
						 roll, b.corners, &image);
	if (result <= 0)
		return result == 0;
	float alpha = 1;
	if (o->id.kind == XW_SNAP_OBJECT_MOBILE && o->genus == XW_GENUS_EXPLOSION_EFFECT &&
		Depth(&s->camera, o->world_pos) <= RENDER_QUAD_DISTANT_DEPTH_THRESHOLD) {
		/* RenderQuad's X-Wing sequence: frame zero is fully opaque. */
		static const uint8_t envelope[32] = { 255, 224, 240, 240, 224, 208, 176, 144, 112, 80, 48,
											  48,  48,  48,  48,  48,  48,  48,  48,  48,  48, 48,
											  48,  48,  48,  48,  48,  48,  48,  48,  48,  48 };
		alpha = envelope[o->animation_state & 31] / 255.0f;
	}
	float strength =
		o->genus == XW_GENUS_EXPLOSION_EFFECT ? XwRemasterConfig_Effective()->explosion_emissive_strength : 1;
	XwEffects_SetFrame(&b, &image, strength, alpha);
	float previous[4][3];
	const XwPreparedObject* pose = &f->objects[index];
	if (f->regenerate_motion && !f->reset_history && !pose->zero_velocity && pose->previous_index >= 0) {
		const XwSnapObject* old = &f->previous_objects[pose->previous_index];
		const XwSnapCraft* old_craft = Craft(old, f->previous_crafts, f->previous_craft_count);
		unsigned old_ordinal = ordinal ? FlameOrdinal(mesh, old_craft, component) : 0;
		uint16_t old_code =
			ordinal ? (old_ordinal ? old_craft->damage_frame : UINT16_MAX) : Code(f->previous_assets, old);
		const XwSnapCamera* pc = XwRemasterConfig_Effective()->motion_blur.camera_blur ||
										 XwRemasterConfig_Effective()->temporal_mode != AERON_TEMPORAL_OFF
									 ? &f->previous_camera
									 : &s->camera;
		float old_roll = Angle(old, pc) + old_ordinal * old->roll * tau / 65536.0f;
		XwEffectFrame old_image;
		if (Corners(f->previous_assets, old, pc, s->camera.world_pos, old_code, ordinal ? 256 : Base(old),
					old_roll, previous, &old_image) > 0)
			b.prev_corners = previous;
	}
	AeronScene_AddBillboard(scene, &b);
	return true;
}

typedef struct EffectOrder {
	unsigned index;
	float depth;
} EffectOrder;

static int Order(const void* lhs, const void* rhs) {
	const EffectOrder *a = lhs, *b = rhs;
	return a->depth != b->depth ? (a->depth > b->depth ? -1 : 1)
								: (a->index < b->index) - (a->index > b->index);
}

bool XwEffects_Submit(AeronScene3D* scene, const XwRenderSnapshot* s, const XwPreparedFlight* f) {
	EffectOrder order[XW_SNAP_OBJECTS];
	for (unsigned i = 0; i < s->object_count; ++i)
		order[i] = (EffectOrder) { i, Depth(&s->camera, s->objects[i].world_pos) };
	qsort(order, s->object_count, sizeof order[0], Order);
	const XwRenderAssetSetView* set = XwRenderAssets_Set(s->flight_assets);
	if (!set)
		return false;
	for (unsigned n = 0; n < s->object_count; ++n) {
		unsigned i = order[n].index;
		const XwSnapObject* o = &s->objects[i];
		if (!XwShip_Eligible(s, o) || (XwRenderMath_SameObject(o->id, s->camera.focus) &&
									   !s->camera.external && !s->camera.replay_mode))
			continue;
		bool mobile = o->id.kind == XW_SNAP_OBJECT_MOBILE;
		if ((mobile && o->type != XW_MULTI_MESH_DEBRIS_MODEL &&
			 (o->genus == XW_GENUS_DEBRIS || o->genus == XW_GENUS_EXPLOSION_EFFECT)) ||
			(!mobile && o->genus >= XW_GENUS_MINE && o->genus <= XW_GENUS_DEBRIS))
			if (!SubmitOne(scene, s, f, i, NULL, 0, 0))
				return false;
		const XwSnapCraft* craft = Craft(o, s->crafts, s->craft_count);
		if (!craft || !craft->damage_frame_valid)
			continue;
		const XwMeshAsset* mesh = XwRemasterAssets_Mesh(set->bindings.types[o->type].geometry);
		if (!mesh)
			return false;
		for (unsigned m = mesh->component_count; m-- > 0;) {
			unsigned ordinal = FlameOrdinal(mesh, craft, m);
			if (ordinal && !SubmitOne(scene, s, f, i, mesh, m, ordinal))
				return false;
		}
	}
	return true;
}
