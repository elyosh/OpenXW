/* Semantic selection from DOS DRAW_drawcraft/ANIM_drawverysimpleobject. No
 * classic renderer globals, scratch matrices or mutable resource bytes are read. */
#include "xw_remaster/dos_ship.h"
#include <math.h>
#include <string.h>

typedef struct Selection {
	const XwRenderSnapshot* snapshot;
	const XwRenderAssetSet* assets;
	const XwSnapObject* object;
	const XwSnapCraft* craft;
	const float* rotations;
	XwDosSelection* out;
	float pose[16], eye[3], original_eye[3], rows[9], angle;
	uint16_t model, parent;
	bool angle_cached;
} Selection;

static const XwRenderDosModel* Model(XwRenderAssetId id) {
	const XwRenderSource* source = XwRenderAssets_Source(id);
	return source && source->kind == XW_SOURCE_DOS_MODEL && source->size == sizeof(XwRenderDosModel)
			   ? source->data
			   : NULL;
}

static const XwRenderDosDescriptors* Descriptors(const XwRenderAssetSet* set, unsigned type) {
	if (type >= set->type_count)
		return NULL;
	const XwRenderSource* source = XwRenderAssets_Source(set->types[type].dos_components);
	if (!source || source->kind != XW_SOURCE_DOS_COMPONENTS || source->size < sizeof(XwRenderDosDescriptors))
		return NULL;
	const XwRenderDosDescriptors* d = source->data;
	return d->count <= 256 && d->variant_count <= (source->size - sizeof *d) / sizeof(uint16_t) ? d : NULL;
}

static bool Variant(const XwRenderDosDescriptors* d, unsigned id, unsigned state, uint16_t* out) {
	if (id >= d->count)
		return false;
	XwSnapRange range = d->descriptors[id].variants;
	if (range.first > d->variant_count || range.count > d->variant_count - range.first)
		return false;
	*out = state < range.count ? d->variants[range.first + state] : UINT16_MAX;
	return true;
}

static int Lod(const XwRenderDosModel* model, unsigned component) {
	if (!model || !model->component_count)
		return -1;
	if (component >= model->component_count) {
		if (!model->clamp_components)
			return -1;
		component = model->component_count - 1;
	}
	XwSnapRange r = model->components[component];
	if (!r.count || r.first > model->lod_count || r.count > model->lod_count - r.first)
		return -1;
	/* Authored component LODs run from highest to lowest detail. */
	return (int)r.first;
}

static bool Geometry(Selection* c, unsigned model_id, unsigned component, unsigned descriptor,
					 bool articulated) {
	if (model_id >= c->assets->type_count || c->out->part_count == XW_DOS_SELECTION_CAPACITY)
		return false;
	XwRenderAssetId id = c->assets->types[model_id].geometry;
	const XwRenderDosModel* model = Model(id);
	float pose[16];
	memcpy(pose, c->pose, sizeof pose);
	for (unsigned a = 0; a < 3; ++a)
		for (unsigned row = 0; row < 3; ++row)
			pose[4 * a + 3] += c->rows[3 * row + a] * (c->eye[row] - c->original_eye[row]);
	if (articulated && c->craft && descriptor < XW_DOS_COMPONENTS) {
		float rotation = c->craft->mesh_rotation[descriptor];
		if (c->rotations && descriptor > 0 &&
			((model_id == 1 && descriptor < 5) || (model_id == 118 && descriptor < 6)))
			rotation = c->rotations[descriptor];
		XwDosShip_Articulate(pose, model_id, descriptor, rotation, c->snapshot->flight_version);
	}
	int lod = Lod(model, component);
	if (lod < 0)
		return false;
	XwDosPart* part = &c->out->parts[c->out->part_count++];
	*part = (XwDosPart) { .geometry = id,
						  .lod = lod,
						  .descriptor = descriptor,
						  .component = component,
						  .parent = c->parent,
						  .markings = c->snapshot->appearance.markings_enabled,
						  .marking_mode = c->craft ? c->object->markings : 0,
						  .projectile = c->object->genus == 5 || c->object->genus == 6 };
	if (model_id == 9 && descriptor == (c->snapshot->flight_version == 93 ? 3u : 6u))
		part->markings = c->object->iff == 0;
	/* Classic FVIEW rotations change geometry but retain the unarticulated light. */
	const XwRenderDosMesh* mesh = &model->lods[lod];
	for (unsigned a = 0; a < 3; ++a) {
		part->light_direction[a] = c->snapshot->appearance.direction_q15[a] / 32768.0f;
		if ((mesh->format & 1) && mesh->format != 0xFF && c->snapshot->appearance.directional_enabled) {
			part->light_direction[a] = 0;
			for (unsigned row = 0; row < 3; ++row)
				part->light_direction[a] +=
					c->pose[4 * row + a] * (c->snapshot->appearance.direction_q15[row] / 32768.0f);
		}
	}
	XwRenderMath_DosMeshMatrix(pose, mesh->format, mesh->coordinate_shift, c->parent >= 0x5000,
							   part->transform);
	return true;
}

static float BitmapAngle(const Selection* c) {
	float m[3][2] = { 0 };
	for (unsigned row = 0; row < 3; ++row)
		for (unsigned col = 0; col < 2; ++col)
			for (unsigned a = 0; a < 3; ++a)
				m[row][col] += c->rows[3 * row + a] * c->pose[4 * a + col];
	unsigned col = fabsf(m[2][1]) > fabsf(m[2][0]) ? 0 : 1;
	float denominator = m[0][col], numerator = m[1][col];
	return denominator < 0 ? atan2f(numerator, -denominator) : -atan2f(numerator, denominator);
}

static bool Bitmap(Selection* c, const XwRenderDosDescriptor* descriptor, unsigned index, uint16_t image,
				   uint16_t scale, bool add_roll) {
	if (c->out->bitmap_count == XW_DOS_SELECTION_CAPACITY)
		return false;
	if (!c->angle_cached) {
		c->angle = BitmapAngle(c);
		c->angle_cached = true;
	}
	if (add_roll)
		c->angle += c->object->roll * (6.283185307179586f / 65536.0f);
	/* These offsets are eye-space and cumulative, including the original zero-sum gate. */
	if ((int32_t)descriptor->eye_offset[0] + descriptor->eye_offset[1] + descriptor->eye_offset[2])
		for (unsigned a = 0; a < 3; ++a)
			c->eye[a] += descriptor->eye_offset[a];
	if (c->eye[2] <= 0)
		return true;
	unsigned bitmap_type = (image & 0x7FFF) >> 8;
	if (bitmap_type >= c->assets->type_count)
		return false;
	XwDosBitmap* bitmap = &c->out->bitmaps[c->out->bitmap_count++];
	*bitmap = (XwDosBitmap) {
		.image = image, .scale = scale, .descriptor = index, .parent = c->parent, .angle = c->angle
	};
	memcpy(bitmap->eye, c->eye, sizeof bitmap->eye);
	return true;
}

static bool Children(Selection* c, const XwRenderDosDescriptors* d, unsigned id) {
	unsigned child = d->descriptors[id].first_child;
	for (unsigned remaining = d->descriptors[id].child_count; remaining; --remaining) {
		if (child >= d->count || child >= XW_DOS_COMPONENTS)
			return false;
		uint16_t image;
		if (!Variant(d, child, c->craft->component_state[child], &image))
			return false;
		const XwRenderDosDescriptor* entry = &d->descriptors[child];
		if (image >= 0x8000 && image < 0xFF00 && !Bitmap(c, entry, child, image, entry->bitmap_scale, true))
			return false;
		child = entry->first_child;
		if (!child)
			break;
	}
	return true;
}

static bool Craft(Selection* c, const XwRenderDosDescriptors* d) {
	bool dos93 = c->snapshot->flight_version == 93;
	if (dos93 && (c->model == 3 || c->model == 8 || c->model == 10 || c->model == 11))
		return Geometry(c, c->model, 0, UINT16_MAX, false);
	if (c->model == 13 || c->model == 16)
		c->parent += 0x7000;
	uint16_t order[16];
	XwSnapObject object = *c->object;
	/* Cached orientation is captured before classic lazily refreshes dirty objects. */
	if (object.orientation_dirty)
		for (unsigned a = 0; a < 3; ++a)
			for (unsigned row = 0; row < 3; ++row)
				object.cached_rows_q15[3 * a + row] =
					(int16_t)fmaxf(-32767, fminf(32767, c->pose[4 * row + a] * (a == 1 ? -32768 : 32768)));
	if (!XwDosShip_Order(c->snapshot, &object, c->craft, c->model, Model(c->assets->types[c->model].geometry),
						 d->component_count, order))
		return false;
	for (unsigned i = 0; i < d->component_count; ++i) {
		unsigned id = order[i];
		if (id >= d->count || id >= XW_DOS_COMPONENTS)
			return false;
		uint16_t selected;
		if (!Variant(d, id, c->craft->component_state[id], &selected))
			return false;
		if (selected >= 0x8000)
			continue;
		if (!Geometry(c, c->model, selected, id, true) || !Children(c, d, id))
			return false;
	}
	return true;
}

bool XwDosShip_Eligible(const XwRenderSnapshot* s, const XwSnapObject* o) {
	if (o->id.kind == XW_SNAP_OBJECT_MISSION)
		return s->hyperspace.phase != 3 && s->hyperspace.phase != 5 && o->genus >= 7 && o->genus <= 10;
	if (o->slot_class == XW_SNAP_SLOT_LOCAL_DEBRIS && (!s->appearance.debris_enabled || s->hyperspace.phase))
		return false;
	if (XwRenderMath_SameObject(o->id, s->camera.focus) && !s->camera.external && !s->camera.replay_mode)
		return false;
	return o->genus <= 6 || o->genus == 10 || o->genus == 13;
}

bool XwDosShip_Select(const XwRenderSnapshot* s, const XwRenderAssetSet* set, const XwSnapObject* object,
					  const XwRenderView* view, const float* rotations, XwDosSelection* out) {
	memset(out, 0, sizeof *out);
	if (!s || !set || !object || !view || !XwDosShip_Eligible(s, object))
		return true;
	Selection c = { .snapshot = s,
					.rotations = rotations,
					.assets = set,
					.object = object,
					.out = out,
					.model = object->type,
					.parent = object->id.kind == XW_SNAP_OBJECT_MISSION ? 0x3800 + object->id.slot
																		: object->id.slot,
					.craft = object->craft_index < s->craft_count ? &s->crafts[object->craft_index] : NULL };
	if (s->flight_version == 93 && c.model == 119)
		c.model = 16;
	XwRenderMath_ObjectMatrix(object, s->flight_version, view->origin_world, c.pose);
	XwRenderMath_ViewRows(&view->camera, c.rows);
	float local[3];
	XwRenderMath_Local(s->camera.world_pos, object->world_pos, local);
	for (unsigned row = 0; row < 3; ++row)
		for (unsigned a = 0; a < 3; ++a)
			c.eye[row] += c.rows[3 * row + a] * local[a];
	memcpy(c.original_eye, c.eye, sizeof c.eye);
	if (object->genus == 5 || object->genus == 6)
		return Geometry(&c, c.model, 0, 0, false);
	const XwRenderDosDescriptors* d = Descriptors(set, c.model);
	if (!d)
		return false;
	if (object->id.kind == XW_SNAP_OBJECT_MOBILE && object->genus <= 4)
		return c.craft && Craft(&c, d);
	for (unsigned i = 0; i < d->count; ++i) {
		unsigned state = object->id.kind == XW_SNAP_OBJECT_MISSION ? object->state
						 : i                                       ? object->secondary_animation_state
																   : object->animation_state;
		uint16_t image;
		if (!Variant(d, i, state, &image))
			return false;
		if (image >= 0xFF00)
			continue;
		if (image < 0x8000) {
			if (c.model == 43 && object->id.kind == XW_SNAP_OBJECT_MOBILE)
				c.model = object->source_type;
			if (!Geometry(&c, c.model, image, i, false))
				return false;
			continue;
		}
		if (c.eye[2] < 0)
			break;
		const XwRenderDosDescriptor* descriptor = &d->descriptors[i];
		bool mobile = object->id.kind == XW_SNAP_OBJECT_MOBILE;
		uint16_t scale = mobile ? (uint16_t)(object->billboard_scale << 6) : 0;
		if (!scale)
			scale = descriptor->bitmap_scale;
		else if (scale >= 256)
			scale += descriptor->bitmap_scale;
		if (!Bitmap(&c, descriptor, i, image, scale, mobile))
			return false;
	}
	return true;
}

bool XwDosShip_Component(const XwRenderSnapshot* s, const XwRenderAssetSet* set, const XwSnapObject* object,
						 const XwRenderView* view, unsigned component, unsigned parent, XwDosPart* out) {
	XwDosSelection selected;
	selected.part_count = selected.bitmap_count = 0;
	Selection c = { .snapshot = s,
					.assets = set,
					.object = object,
					.out = &selected,
					.model = object->type,
					.parent = parent };
	XwRenderMath_ObjectMatrix(object, s->flight_version, view->origin_world, c.pose);
	XwRenderMath_ViewRows(&view->camera, c.rows);
	float local[3];
	XwRenderMath_Local(s->camera.world_pos, object->world_pos, local);
	for (unsigned row = 0; row < 3; ++row)
		for (unsigned a = 0; a < 3; ++a)
			c.eye[row] += c.rows[3 * row + a] * local[a];
	memcpy(c.original_eye, c.eye, sizeof c.eye);
	if (!Geometry(&c, object->type, component, component, false))
		return false;
	*out = selected.parts[0];
	/* Surface and gates explicitly retain the world light in the original FVIEW path. */
	for (unsigned a = 0; a < 3; ++a)
		out->light_direction[a] = s->appearance.direction_q15[a] / 32768.0f;
	return true;
}
