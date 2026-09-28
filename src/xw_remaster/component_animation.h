#ifndef XW_REMASTER_COMPONENT_ANIMATION_H
#define XW_REMASTER_COMPONENT_ANIMATION_H
#include "xw_runtime/snapshot/render_types.h"
#include <stdbool.h>

/* Render-owner state; only completed snapshots and immutable asset identities are consumed. */
void XwComponentAnimation_Reset(void);
void XwComponentAnimation_Prepare(const XwRenderSnapshot* snapshot, bool reset);
/* Previous is the visual pose retained at the preceding prepared view. NULL selects game bytes. */
const float* XwComponentAnimation_Angles(const XwSnapObject* object, bool previous);
#endif
