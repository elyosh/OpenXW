#include "xw_remaster/backdrops.h"
#include "xw_remaster/assets.h"
#include "xw_remaster/dos_sprites.h"
#include <math.h>

/* OpenTIE's fixed tangent frame, with X-Wing's one-based direction lattice. */
static bool BuildQuad(const XwRenderView* view, unsigned face, unsigned bits, float hw, float hh,
					  AeronSceneBillboardDesc* out) {
	static const unsigned axes[3][3] = { { 1, 0, 2 }, { 0, 1, 2 }, { 2, 1, 0 } };
	unsigned main = axes[face / 2][0], low = axes[face / 2][1], high = axes[face / 2][2];
	float dir[3] = { 0 };
	dir[main] = face & 1 ? -8 : 8;
	dir[low] = (bits & 8 ? -1 : 1) * (float)((bits & 7) + 1);
	dir[high] = (bits & 128 ? -1 : 1) * (float)(((bits >> 4) & 7) + 1);
	float length = sqrtf(dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2]);
	for (unsigned a = 0; a < 3; ++a)
		dir[a] /= length;
	float reference[3] = { 0 };
	reference[face / 2 == 1 ? 1 : 0] = 1;
	float up[3] = { dir[1] * reference[2] - dir[2] * reference[1],
					dir[2] * reference[0] - dir[0] * reference[2],
					dir[0] * reference[1] - dir[1] * reference[0] };
	length = sqrtf(up[0] * up[0] + up[1] * up[1] + up[2] * up[2]);
	/* The face normal and its reference axis are always distinct. */
	for (unsigned a = 0; a < 3; ++a)
		up[a] /= length;
	float right[3] = { up[1] * dir[2] - up[2] * dir[1], up[2] * dir[0] - up[0] * dir[2],
					   up[0] * dir[1] - up[1] * dir[0] };
	static const int sx[4] = { -1, 1, 1, -1 }, sy[4] = { 1, 1, -1, -1 };
	bool in_front = false;
	for (unsigned c = 0; c < 4; ++c) {
		for (unsigned a = 0; a < 3; ++a)
			out->corners[c][a] = 65536.0f * (dir[a] + right[a] * sx[c] * hw + up[a] * sy[c] * hh);
		const float* m = view->view_proj;
		const float* p = out->corners[c];
		in_front |= m[12] * p[0] + m[13] * p[1] + m[14] * p[2] > 0;
	}
	/* Keep partially visible tiles; hardware clips their homogeneous corners. */
	return in_front;
}

bool XwBackdrops_Submit(AeronScene3D* scene, const XwRenderSnapshot* s, const XwRenderView* view) {
	bool dos = s->flight_version != 98;
	if (!s->appearance.backdrops_enabled || (dos && s->special.surface_active && s->camera.rows[8] <= -0.75f))
		return true;
	const XwRenderAssetSetView* set = XwRenderAssets_Set(s->flight_assets);
	if (!set || view->camera.viewport.width <= 0 || view->camera.viewport.height <= 0)
		return false;
	/* OpenTIE's pixel-to-tangent sizing, using the same view that projects the sky. */
	float half_pixel_x =
		view->classic_pixel_scale_x * tanf(view->camera.h_half_rad) / view->camera.viewport.width;
	float half_pixel_y =
		view->classic_pixel_scale_y * tanf(view->camera.v_half_rad) / view->camera.viewport.height;
	/* Match the captured classic renderer, as OpenTIE does. */
	bool mirror_u = s->camera.legacy_render_convention == XW_SNAP_CLASSIC_WINDOWS_HARDWARE;
	unsigned record = 0;
	for (unsigned face = 0; face < 6; ++face) {
		unsigned count = s->sky.face_counts[face];
		if (count > XW_SNAP_BACKDROPS - record)
			return false;
		for (unsigned i = 0; i < count; ++i, ++record) {
			unsigned type = s->sky.backdrop_type[record];
			if (type >= set->bindings.type_count)
				return false;
			const XwSnapType* t = &set->bindings.types[type];
			const AeronRuntimeAtlas* atlas = XwRemasterAssets_Image(t->bitmaps);
			if (!atlas)
				return false;
			int frame = Aeron_SpriteAtlasFindById(&atlas->layout, 0);
			if (frame < 0) {
				if (dos)
					continue;
				return false;
			}
			const AeronSpriteRect* rect = &atlas->layout.frames[frame];
			const AeronRuntimeAtlasPage* page = &atlas->pages[atlas->layout.pages[frame]];
			AeronSceneBillboardDesc b = { .texture = page->texture,
										  .stage = AERON_SCENE_BILLBOARD_STAGE_SKY,
										  .blend = AERON_SCENE_BILLBOARD_BLEND_PMA };
			if (!BuildQuad(view, face, s->sky.backdrop_direction[record],
						   atlas->layout.classic_w[frame] * half_pixel_x,
						   atlas->layout.classic_h[frame] * half_pixel_y, &b))
				continue;
			for (unsigned c = 0; c < 4; ++c) {
				bool right = c == 1 || c == 2;
				b.uv[c][0] = (rect->x + (right != mirror_u ? rect->w : 0)) / page->width;
				b.uv[c][1] = (rect->y + (c >= 2 ? rect->h : 0)) / page->height;
				for (unsigned a = 0; a < 4; ++a)
					b.colors[c][a] = 1;
			}
			if (dos) {
				const XwRenderSource* remap = XwRenderAssets_Source(t->bitmap_remap);
				if (!remap || remap->kind != XW_SOURCE_PALETTE || remap->size != 16 ||
					!XwDosSprites_Backdrop(view, page, rect, remap, &b))
					return false;
			} else
				AeronScene_AddBillboard(scene, &b);
		}
	}
	return true;
}
