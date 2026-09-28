#ifndef XW_REMASTER_LIGHTING_H
#define XW_REMASTER_LIGHTING_H
#include "xw_remaster/render_math.h"
int XwLighting_Begin(AeronScene3D* scene, const XwRenderSnapshot* snapshot);
void XwLighting_Environment(AeronScene3D* scene, const XwSnapAppearance* light, const float position[3]);
void XwLighting_AddPoint(AeronScene3D* scene, const float position[3], const float color[3], float intensity,
						 float minimum_range);
#endif
