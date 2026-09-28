/* Pure cell expansion from DeathStar_DrawSurfaceAndTrench and DOS surface_draw. */
#include "xw_remaster/special_world_internal.h"
#include <math.h>

static bool Details(XwWorldBuild* b, int16_t x, int16_t y) {
	const XwRenderSnapshot* s = b->snapshot;
	bool dos = s->flight_version != 98;
	unsigned layout = (x & 1) | ((y & 1) << 1);
	if (x == 0 || x == -1)
		layout = 4;
	bool raised = layout != 1 && layout != 4;
	int32_t wx = (int32_t)((uint32_t)(uint16_t)x << 16);
	int32_t wy = (int32_t)((uint32_t)(uint16_t)y << 16);
	uint64_t key = ((uint64_t)(uint16_t)x << 32) | ((uint64_t)(uint16_t)y << 16);
	int32_t center[3] = { (int32_t)((uint32_t)wx + 32768), (int32_t)((uint32_t)wy + 32768), 0 };
	float depth = XwWorld_Depth(b, center);
	if (raised &&
		!XwWorld_Model(b, key, dos ? 89 : 182, dos ? 0 : UINT16_MAX, center[0], center[1], 0, 0x7800, depth))
		return false;
	if (dos ? s->camera.world_pos[2] > 65536 : depth > (s->appearance.graphics_detail + 1) * 65536.0f)
		return true;
	unsigned start = (x & 7) | ((y & 7) << 3);
	uint16_t cell_key = start | ((x & 0xf8) << 3) | ((y & 0xf8) << 8);
	const XwSnapDamageCell* damage = XwWorld_Damage(&s->special, start, cell_key);
	const XwRenderPlacementList* list = &b->layout->surface[layout];
	unsigned count =
		list->count < s->appearance.surface_object_limit ? list->count : s->appearance.surface_object_limit;
	if (count < 8)
		count = 8;
	if (count > list->count)
		return false;
	for (unsigned i = 0; i < count; ++i) {
		if (damage && i < XW_SNAP_DAMAGE_STATES && !damage->state[i])
			continue;
		const XwRenderPlacement* p = &list->entries[i];
		int32_t pos[3] = { (int32_t)((uint32_t)wx | ((p->position & 15u) << 12)),
						   (int32_t)((uint32_t)wy | ((p->position >> 4) << 12)), raised ? 4096 : 0 };
		float z = XwWorld_Depth(b, pos);
		if (dos && z > 65536)
			continue;
		if (p->type >= b->assets->type_count)
			return false;
		if (!dos) {
			float delta[3];
			XwRenderMath_Local(s->camera.world_pos, pos, delta);
			float a = fabsf(delta[0]), c = fabsf(delta[1]), d = fabsf(delta[2]);
			float distance = a > c && a > d   ? a + floorf(c / 4) + floorf(d / 4)
							 : c > a && c > d ? c + floorf(a / 4) + floorf(d / 4)
											  : d + floorf(a / 4) + floorf(c / 4);
			float size = s->camera.focal_x * (float)b->assets->types[p->type].max_extent /
						 fmaxf(1, distance) * b->layout->detail_scale[s->appearance.graphics_detail];
			float minimum =
				p->type >= 167 && p->type <= 172 ? b->layout->minimum_large_size : b->layout->minimum_size;
			if (size < minimum)
				continue;
		}
		if (!XwWorld_Model(b, key | (i + 1), p->type, dos ? 0 : UINT16_MAX, pos[0], pos[1], pos[2], 0x2000,
						   z))
			return false;
	}
	return true;
}

static bool Horizon(XwWorldBuild* b) {
	const XwSnapCamera* c = &b->snapshot->camera;
	if (c->rows[8] <= -0.75f || c->rows[8] >= 0.75f)
		return true;
	float fx = c->rows[6] * 262144.0f, fy = c->rows[7] * 262144.0f;
	float p[3] = { fy - c->rows[6] * 131072.0f, -fx - c->rows[7] * 131072.0f, -(float)c->world_pos[2] };
	float across[3] = { -fy * 2, fx * 2, 0 }, along[3] = { fx * 32, fy * 32, 0 };
	for (unsigned a = 0; a < 3; ++a)
		p[a] += b->view->camera.pos[a];
	return XwWorld_Quad(b, 16, p, across, along);
}

static bool DosSurface(XwWorldBuild* b) {
	if (!Horizon(b))
		return false;
	static const int8_t offsets[9][2] = { { 0, 0 },  { 1, 0 },   { 1, 1 },  { 0, 1 }, { -1, 1 },
										  { -1, 0 }, { -1, -1 }, { 0, -1 }, { 1, -1 } };
	const XwRenderSnapshot* s = b->snapshot;
	int32_t z = s->camera.world_pos[2];
	int16_t mask = -32;
	for (unsigned level = 6; level; --level) {
		int16_t x = (s->camera.world_pos[0] >> 16) & mask, y = (s->camera.world_pos[1] >> 16) & mask;
		unsigned parity = ((x >> (level - 1)) & 1) | (((y >> (level - 1)) & 1) << 1);
		mask >>= 1;
		if (level > 1 &&
			(6 - level > s->appearance.surface_detail_level || (level == 6 && z < 32768) ||
			 (level == 5 && z < 24576) || (level == 4 && z < 20480) || (level == 3 && z < 16384)))
			continue;
		int32_t width = 65536 << (level - 1);
		for (unsigned i = 0; i < 9; ++i) {
			int dx = offsets[i][0], dy = offsets[i][1];
			if (level == 1) {
				if (!Details(b, (int16_t)(x + dx), (int16_t)(y + dy)))
					return false;
				continue;
			}
			int32_t pos[3] = { (int32_t)(((uint32_t)(uint16_t)x << 16) + (uint32_t)dx * width),
							   (int32_t)(((uint32_t)(uint16_t)y << 16) + (uint32_t)dy * width), 0 };
			int32_t center[3] = { (int32_t)((uint32_t)pos[0] + width / 2),
								  (int32_t)((uint32_t)pos[1] + width / 2), 0 };
			if (XwWorld_Depth(b, center) > (float)z * 16 + 3 * (width / 4) + 0x40000)
				continue;
			unsigned color = parity ^ (dx & 1) ^ ((dy & 1) << 1);
			float p[3], across[3] = { width, 0, 0 }, along[3] = { 0, width, 0 };
			XwRenderMath_Local(b->view->origin_world, pos, p);
			if (color == 0) {
				p[0] += width / 8;
				across[0] = 0;
				across[1] = width;
				along[0] = width * .75f;
				along[1] = 0;
			} else {
				p[1] += color == 1 ? width / 8 : color == 2 ? width / 4 : 0;
				along[1] *= .75f;
			}
			if (!XwWorld_Quad(b, b->layout->surface_colors[4 * level + color], p, across, along))
				return false;
		}
	}
	return true;
}

static bool WindowsSurface(XwWorldBuild* b) {
	const XwRenderSnapshot* s = b->snapshot;
	int radius = 1;
	for (int32_t altitude = s->camera.world_pos[2]; altitude > 65536; altitude >>= 1)
		++radius;
	int32_t cx = s->camera.world_pos[0] & -65536, cy = s->camera.world_pos[1] & -65536;
	for (int row = -radius; row <= radius; ++row)
		for (int col = -radius; col <= radius; ++col) {
			int32_t x = (int32_t)((uint32_t)cx + (uint32_t)col * 0x200000u);
			int32_t y = (int32_t)((uint32_t)cy + (uint32_t)row * 0x200000u);
			if (x >= -1048576 && x <= 1048576) {
				unsigned slice = (x + 1048576) / 65536;
				if (x > -1048576 && !XwWorld_Tile(b, 1 + slice, x, y))
					return false;
				if (x < 1048576 && !XwWorld_Tile(b, 34 + slice, x, y))
					return false;
			} else if (!XwWorld_Tile(b, 0, x, y))
				return false;
		}
	if (s->camera.world_pos[2] < 0)
		return true;
	unsigned detail = s->appearance.graphics_detail, levels = 0;
	for (int32_t altitude = s->camera.world_pos[2]; altitude > 16384; altitude >>= 1)
		++levels;
	radius = levels * (detail + 1) / 2 + (detail <= 1 ? 1 : detail);
	/* Directional shadow distance uses the same world units as the scene. */
	if (b->shadow_distance > 0)
		radius = (int)fmaxf(radius, fminf(32, ceilf(b->shadow_distance / 65536.0f)));
	for (int row = -radius; row <= radius; ++row)
		for (int col = -radius; col <= radius; ++col)
			if (!Details(b, (int16_t)((cx >> 16) + col), (int16_t)((cy >> 16) + row)))
				return false;
	return true;
}

bool XwWorld_Surface(XwWorldBuild* b) {
	if (b->snapshot->appearance.graphics_detail > 3)
		return false;
	if (b->snapshot->flight_version == 98)
		return WindowsSurface(b);
	return b->snapshot->camera.world_pos[2] < 0 || DosSurface(b);
}
