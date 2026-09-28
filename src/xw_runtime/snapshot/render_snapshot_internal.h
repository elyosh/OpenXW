#ifndef XW_RENDER_SNAPSHOT_INTERNAL_H
#define XW_RENDER_SNAPSHOT_INTERNAL_H
#include "xw_runtime/snapshot/render_snapshot.h"
/* Copy only populated channels; unused tails are never consumed. */
void XwRenderSnapshot_Copy(XwRenderSnapshot* destination, const XwRenderSnapshot* source);
void XwRenderSnapshot_Clear(XwRenderSnapshot* snapshot);
#endif
