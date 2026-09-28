#ifndef XW_REMASTER_RENDER_MATH_H
#define XW_REMASTER_RENDER_MATH_H
#include "aeron/scene/scene3d.h"
#include "xw_runtime/snapshot/render_types.h"
#include <stdbool.h>

typedef struct XwRenderView {
	AeronSceneCamera camera;
	int32_t origin_world[3];
	float view_proj[16], classic_pixel_scale_x, classic_pixel_scale_y;
} XwRenderView;

typedef struct XwLayoutTransform {
	float scale_x, scale_y, source_width, source_height, target_width, target_height;
} XwLayoutTransform;

/* Source-screen coordinates map into a centered 4:3 display, including 320x200 pixels. */
bool XwRenderMath_Layout(float source_width, float source_height, float target_width, float target_height,
						 XwLayoutTransform* out);
void XwRenderMath_LayoutPoint(const XwLayoutTransform* layout, float anchor_x, float anchor_y, float x,
							  float y, float* out_x, float* out_y);
void XwRenderMath_LayoutInverse(const XwLayoutTransform* layout, float anchor_x, float anchor_y, float x,
								float y, float* out_x, float* out_y);
AeronRectI XwRenderMath_LayoutRect(const XwLayoutTransform* layout);
/* Wrapped integer subtraction precedes float conversion, matching X-Wing world arithmetic. */
void XwRenderMath_Local(const int32_t origin[3], const int32_t world[3], float out[3]);
bool XwRenderMath_BuildMainView(const XwSnapCamera* camera, const int32_t origin[3], int width, int height,
								XwRenderView* out);
bool XwRenderMath_ProjectWorld(const XwRenderView* view, const int32_t world[3], float* x, float* y,
							   float* depth);
/* Windows matrices undo the OPT converter's metres scale. DOS matrices are rigid bases. */
void XwRenderMath_ObjectMatrix(const XwSnapObject* object, uint8_t version, const int32_t origin[3],
							   float out[16]);
/* Apply decoded DOS coordinates to a pose. Enlarged selects the original >=0x5000 parent path,
 * not a type-wide rule: DOS93 capital ships can use different paths for different LODs. */
void XwRenderMath_DosMeshMatrix(const float pose[16], uint8_t format, uint8_t coordinate_shift, bool enlarged,
								float out[16]);
void XwRenderMath_ViewRows(const AeronSceneCamera* camera, float rows[9]);
bool XwRenderMath_SameObject(XwSnapObjectId a, XwSnapObjectId b);
#endif
