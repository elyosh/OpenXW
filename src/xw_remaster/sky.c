#include "xw_remaster/sky.h"
#include "aeron/aeron.h"
#include "aeron/scene/image_cache.h"
#include "xw_remaster/backdrops.h"
#include "xw_remaster/config.h"
#include "xw_remaster/hyperspace.h"
#include "xw_remaster/sky_stars.h"
#include <stdio.h>
#include <string.h>

static XwRemasterSkyStars* g_stars;
static AeronTexture* g_cube;
static char g_cubePath[XW_PATH_CAPACITY];
static int g_hyper, g_drawStars;

static void Background(AeronCommandBuffer* cmd, AeronRenderPass* pass, int w, int h, void* user) {
	(void)user;
	if (g_hyper)
		XwHyperspace_Draw(cmd, pass, w, h, NULL);
	else if (g_drawStars)
		XwRemasterSkyStars_Draw(cmd, pass, w, h, g_stars);
}

int XwSky_Prepare(AeronCommandBuffer* cmd, AeronScene3D* scene, const XwRenderSnapshot* s,
				  const XwRenderView* view) {
	const XwSkySettings* p = &XwRemasterConfig_Effective()->sky;
	g_hyper = XwHyperspace_Active(s);
	g_drawStars = 0;
	AeronScene_SetSkyCube(scene, NULL, NULL, 1);
	AeronScene_SetPassHook(scene, AERON_SCENE_HOOK_BEFORE_OPAQUE, Background, NULL);
	if (!XwHyperspace_Prepare(cmd, s, scene, view))
		return 0;
	if (g_hyper)
		return 1;
	if (p->enabled && p->mode == XW_SKY_CUBE) {
		if (!g_cube || strcmp(g_cubePath, p->path)) {
			AeronTexture* cube = Aeron_ImageLoadCubemapKtx2Vfs(cmd, Aeron_GetVfs(), AERON_VFS_ROOT_ASSET,
															   p->path, 256u * 1024u * 1024u);
			if (!cube)
				return 0;
			Aeron_DestroyTexture(g_cube);
			g_cube = cube;
			snprintf(g_cubePath, sizeof g_cubePath, "%s", p->path);
		}
		AeronScene_SetSkyCube(scene, g_cube, NULL, p->exposure);
	} else {
		if (!g_stars)
			g_stars = XwRemasterSkyStars_Create();
		if (!g_stars)
			return 0;
		XwRemasterSkyStarsParams params = {
			.exposure = p->exposure,
			.brightness = p->star_brightness,
			.classic_pixel_scale = view->classic_pixel_scale_x,
			.density_divisor = s->sky.density,
			.source = s->sky.stars,
		};
		if (!XwRemasterSkyStars_Prepare(g_stars, cmd, scene, &params))
			return 0;
		g_drawStars = 1;
	}
	return XwBackdrops_Submit(scene, s, view);
}

void XwSky_Shutdown(void) {
	XwRemasterSkyStars_Destroy(g_stars);
	g_stars = NULL;
	Aeron_DestroyTexture(g_cube);
	g_cube = NULL;
	g_cubePath[0] = 0;
	XwHyperspace_Shutdown();
}
