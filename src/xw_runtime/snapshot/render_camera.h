#ifndef XW_RENDER_CAMERA_H
#define XW_RENDER_CAMERA_H
#include "xw_runtime/snapshot/render_types.h"
#include <stdbool.h>
/* Mirror the completed camera rotation chain, tagged with its exact Q15 rows. */
void XwRenderCamera_Build(int16_t roll, int16_t pitch, int16_t yaw, int16_t angle_d, int16_t aim_x,
						  int16_t aim_y);
void XwRenderCamera_CopyRows(float rows[9]);
/* Observe the main camera after its existing update, before any temporary model transforms. */
bool XwRenderCamera_Capture(XwSnapCamera* camera);
void XwRenderCamera_Appearance(XwSnapAppearance* appearance);
#endif
