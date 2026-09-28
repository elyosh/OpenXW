#ifndef XW_REMASTER_SKY_STARS_H
#define XW_REMASTER_SKY_STARS_H

#include "aeron/scene/scene3d.h"

#include "xw_runtime/snapshot/render_sky.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XwRemasterSkyStars XwRemasterSkyStars;

typedef struct XwRemasterSkyStarsParams {
	float exposure;
	float brightness;
	float classic_pixel_scale, classic_pixel_scale_y;
	const float* view_proj; /* Optional profile projection; borrowed during preparation. */
	const uint32_t* palette_argb;
	uint64_t palette_revision;
	int16_t oscillator;
	uint16_t density_divisor;
	XwRenderAssetId source;
} XwRemasterSkyStarsParams;

XwRemasterSkyStars* XwRemasterSkyStars_Create(void);
void XwRemasterSkyStars_Destroy(XwRemasterSkyStars* stars);

/* Captures the current scene transform after AeronScene_Begin. The public
 * jittered view-projection keeps stars aligned with temporally jittered meshes. */
int XwRemasterSkyStars_Prepare(XwRemasterSkyStars* stars, AeronCommandBuffer* cmd, const AeronScene3D* scene,
							   const XwRemasterSkyStarsParams* params);

/* AeronScene BEFORE_OPAQUE hook draw. */
void XwRemasterSkyStars_Draw(AeronCommandBuffer* command_buffer, AeronRenderPass* render_pass, int rt_w,
							 int rt_h, void* user);

#ifdef __cplusplus
}
#endif

#endif /* XW_REMASTER_SKY_STARS_H */
