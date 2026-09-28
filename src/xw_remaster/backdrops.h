#ifndef XW_REMASTER_BACKDROPS_H
#define XW_REMASTER_BACKDROPS_H
#include "xw_remaster/render_math.h"

/* The supplied sky view controls both angular sizing and projection. */
bool XwBackdrops_Submit(AeronScene3D* scene, const XwRenderSnapshot* snapshot, const XwRenderView* view);
#endif
