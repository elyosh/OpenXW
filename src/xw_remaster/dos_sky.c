#include "xw_remaster/dos_sky.h"
#include "xw_remaster/backdrops.h"
#include "xw_remaster/dos_draw.h"
#include "xw_remaster/dos_sprites.h"
#include "xw_remaster/sky_stars.h"
#include <string.h>

static XwRemasterSkyStars* stars;
static bool draw_stars;

static bool Hyperstars(const XwRenderSnapshot* s, const XwRenderView* view) {
	if (!s->hyperspace.count)
		return true;
	const XwRenderAssetSetView* set = XwRenderAssets_Set(s->flight_assets);
	if (!set || s->hyperspace.count > XW_SNAP_HYPERSTARS)
		return false;
	XwRenderAssetId source = 0;
	for (unsigned i = 0; i < set->source_count; ++i) {
		const XwRenderSource* record = XwRenderAssets_Source(set->sources[i]);
		if (record && record->kind == XW_SOURCE_DOS_MODEL &&
			!strcmp(record->path, "dos/resident/hyperstar")) {
			source = record->id;
			break;
		}
	}
	if (!source)
		return false;
	for (unsigned i = 0; i < s->hyperspace.count; ++i) {
		const XwSnapHyperstar* star = &s->hyperspace.stars[i];
		float position[3];
		XwRenderMath_Local(view->origin_world, star->world_pos, position);
		XwDosPart part = { .geometry = source,
						   .projectile = true,
						   .hyperstar = true,
						   .line_color = star->color_index,
						   .line_endpoints = { s->hyperspace.dos_endpoints[0],
											   s->hyperspace.dos_endpoints[1] },
						   .transform = { .5f, 0, 0, position[0], 0, .5f, 0, position[1], 0, 0, .5f,
										  position[2], 0, 0, 0, 1 } };
		if (!XwDosDraw_Add(&part))
			return false;
	}
	return true;
}

bool XwDosSky_Prepare(AeronCommandBuffer* cmd, AeronScene3D* scene, const XwRenderSnapshot* s,
					  const XwRenderView* view) {
	draw_stars = false;
	XwSnapCamera camera = s->camera;
	/* Both DOS sky layers use a 256-pixel focal length on each axis. */
	camera.aspect_y_q16 = UINT16_MAX;
	XwRenderView sky_view;
	if (!XwRenderMath_BuildMainView(&camera, view->origin_world, view->camera.viewport.width,
									view->camera.viewport.height, &sky_view) ||
		!XwBackdrops_Submit(scene, s, &sky_view) || !Hyperstars(s, view))
		return false;
	bool surface_background = s->special.surface_active && s->camera.rows[8] < -0.75f;
	bool hyper = s->hyperspace.phase == 3 || s->hyperspace.phase == 5;
	if (surface_background || (!s->special.surface_active && hyper))
		return true;
	if (!stars)
		stars = XwRemasterSkyStars_Create();
	if (!stars)
		return false;
	XwRemasterSkyStarsParams params = { .source = s->sky.stars,
										.density_divisor = s->sky.density == 2 ? 2 : 1,
										.exposure = 1,
										.brightness = 1,
										.classic_pixel_scale = sky_view.classic_pixel_scale_x,
										.classic_pixel_scale_y = sky_view.classic_pixel_scale_y,
										.view_proj = sky_view.view_proj,
										.palette_argb = s->appearance.palette_argb,
										.palette_revision = s->appearance.palette_revision,
										.oscillator = s->sky.oscillator };
	if (!XwRemasterSkyStars_Prepare(stars, cmd, scene, &params))
		return false;
	draw_stars = true;
	return true;
}

void XwDosSky_Draw(AeronCommandBuffer* cmd, AeronRenderPass* pass, int width, int height, void* user) {
	(void)user;
	if (draw_stars)
		XwRemasterSkyStars_Draw(cmd, pass, width, height, stars);
	XwDosSprites_Draw(pass, true);
	XwDosDraw_Pass(cmd, pass, width, height, NULL);
}

void XwDosSky_Shutdown(void) {
	XwRemasterSkyStars_Destroy(stars);
	stars = NULL;
	draw_stars = false;
}
