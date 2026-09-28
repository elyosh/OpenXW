#ifndef XW_RENDER_OBJECTS_H
#define XW_RENDER_OBJECTS_H
#include "xw_runtime/snapshot/render_types.h"
#include <stdbool.h>
/* Owner-thread sidecars; never serialized or attached to original object storage. */
void XwRenderObjects_Reset(void);
void XwRenderObjects_ReplaceMobile(uint16_t slot);
void XwRenderObjects_ReplaceMission(uint16_t slot);
XwSnapObjectId XwRenderObjects_Id(uint16_t reference);
bool XwRenderObjects_Capture(XwRenderSnapshot* snapshot);
void XwRenderObjects_MissionPose(uint16_t slot);
#endif
