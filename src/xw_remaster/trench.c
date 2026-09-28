#include "xw_remaster/special_world_internal.h"
#include <math.h>

/* The end-cell boost persists until a placement passes the original logical
 * visibility gate. This does not cull modern instances or depend on draw output. */
static bool ConsumesEndDetail(const XwWorldBuild* b, const int32_t position[3], unsigned type) {
	if (type >= b->assets->type_count)
		return false;
	float relative[3], eye[3] = { 0 };
	XwRenderMath_Local(b->snapshot->camera.world_pos, position, relative);
	for (unsigned row = 0; row < 3; ++row)
		for (unsigned a = 0; a < 3; ++a)
			eye[row] += relative[a] * b->snapshot->camera.rows[row * 3 + a];
	float expanded = eye[2] + b->assets->types[type].max_extent;
	return expanded >= 0 && fabsf(eye[0]) <= expanded && fabsf(eye[1]) <= expanded;
}

static bool Details(XwWorldBuild* b, int16_t y) {
	const XwRenderSnapshot* s = b->snapshot;
	if (s->camera.world_pos[2] > 32768)
		return true;
	bool dos = s->flight_version != 98;
	uint16_t key = dos ? ((y & 7) << 3) | ((y & 0xf8) << 8) | 0x400 : (y & 7) | ((y & 0xf8) << 8) | 0x400;
	const XwSnapDamageCell* damage = XwWorld_Damage(&s->special, y & 63, key);
	unsigned layout = y ? y & 7 : 8;
	const XwRenderPlacementList* list = &b->layout->trench[layout];
	unsigned limit = dos ? s->appearance.surface_object_limit : s->appearance.trench_object_limit;
	unsigned count = list->count < limit ? list->count : limit;
	if (count < 8)
		count = 8;
	bool special = layout == 8;
	if (special)
		count = 13;
	if (count > list->count)
		return false;
	for (unsigned i = 0; i < count; ++i) {
		if (damage && i < XW_SNAP_DAMAGE_STATES && !damage->state[i])
			continue;
		const XwRenderPlacement* p = &list->entries[i];
		unsigned side = p->position & 3, along = (p->position >> 2) & 15;
		unsigned height = (p->position >> 5) & 14;
		int32_t pos[3] = { side == 0   ? 3072
						   : side == 1 ? -3072
									   : 0,
						   (int32_t)(((uint32_t)(uint16_t)y << 16) | (along << 12)),
						   -6144 + (int)height * 512 };
		float depth = XwWorld_Depth(b, pos);
		float maximum = special ? 98304 : dos ? 49152 : (s->appearance.graphics_detail + 3) * 16384;
		if (depth > maximum)
			continue;
		unsigned component = side == 0;
		if (height)
			component += 2;
		if (dos && side >= 2)
			component = 0;
		if (!dos && p->type == 185) {
			if (side == 2)
				component = 2;
			else if (side == 1)
				component = 0;
		}
		uint64_t identity = ((uint64_t)(uint16_t)y << 16) | 0x8000 | i;
		if (!XwWorld_Model(b, identity, p->type, component, pos[0], pos[1], pos[2], 0x1000, depth))
			return false;
		if (special && ConsumesEndDetail(b, pos, p->type))
			special = false;
	}
	return true;
}

static bool Section(XwWorldBuild* b, int32_t y, unsigned level, unsigned parity) {
	const XwSnapCamera* camera = &b->snapshot->camera;
	int32_t pos[3] = { -3072, y, 0 };
	float p[3];
	XwRenderMath_Local(b->view->origin_world, pos, p);
	float length = level == 5 ? 16777216 : (float)(65536u << (level - 1));
	if (level == 5)
		p[1] -= length / 2;
	float along[3] = { 0, length, 0 }, down[3] = { 0, 0, -6144 }, across[3] = { 6144, 0, 0 };
	const uint8_t* colors = b->layout->trench_colors + 3 * level;
	if (camera->world_pos[0] > -3072) {
		if (!XwWorld_Quad(b, colors[0] + parity, p, along, down))
			return false;
	} else if (level == 5) {
		float cover[3] = { 0, 0, -12288 };
		if (!XwWorld_Quad(b, 0, p, along, cover))
			return false;
	}
	p[2] -= 6144;
	if (!XwWorld_Quad(b, colors[1] + parity, p, along, across))
		return false;
	p[0] += 6144;
	down[2] = 6144;
	if (camera->world_pos[0] < 3072) {
		if (!XwWorld_Quad(b, colors[2] + parity, p, along, down))
			return false;
	} else if (level == 5) {
		p[2] -= 6144;
		down[2] = 12288;
		if (!XwWorld_Quad(b, 0, p, along, down))
			return false;
	}
	return true;
}

bool XwWorld_Trench(XwWorldBuild* b) {
	const XwRenderSnapshot* s = b->snapshot;
	const XwSnapCamera* c = &s->camera;
	bool dos = s->flight_version != 98;
	if (dos) {
		if (!Section(b, c->world_pos[1], 5, 0))
			return false;
	} else {
		int radius = 1;
		for (int32_t altitude = c->world_pos[2]; altitude > 65536; altitude >>= 1)
			++radius;
		for (int i = -radius; i <= radius; ++i) {
			int32_t y = (int32_t)((uint32_t)(c->world_pos[1] & -65536) + (uint32_t)i * 0x200000u);
			if (!XwWorld_Tile(b, 67, 0, y))
				return false;
		}
	}
	if (c->world_pos[0] < -32768 || c->world_pos[0] > 32768 || c->world_pos[2] > 131072)
		return true;
	int16_t mask = -8;
	for (unsigned level = 4; level; --level) {
		int16_t step = c->rows[7] < 0 ? mask : (int16_t)-mask;
		int16_t y = (c->world_pos[1] >> 16) & mask;
		unsigned parity = (((uint16_t)(c->world_pos[1] >> 16) & (uint16_t)~(mask * 2)) << 1) >> level;
		mask >>= 1;
		for (unsigned row = 0; row < (dos ? 3u : 4u); ++row) {
			if (level == 1) {
				if (!Details(b, y))
					return false;
			} else if (dos && 4 - level < s->appearance.surface_detail_level &&
					   !Section(b, (int32_t)((uint32_t)(uint16_t)y << 16), level, parity))
				return false;
			y = (int16_t)(y + step);
			parity ^= 1;
		}
	}
	return true;
}
