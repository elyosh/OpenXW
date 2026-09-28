#ifndef XW_REMASTER_PRESENTED_FRAME_H
#define XW_REMASTER_PRESENTED_FRAME_H
#include "xw_remaster/flight_scene.h"

/* Track completed output owned by the flight renderer, as in OpenXvT. */
bool XwPresentedFrame_Store(const XwRenderSnapshot* snapshot, const XwFlightOutput* output);
/* Resolve direct output before changing its sources when no replacement frame will be rendered. */
bool XwPresentedFrame_Retain(const XwRenderSnapshot* snapshot);
const XwFlightOutput* XwPresentedFrame_Get(const XwRenderSnapshot* snapshot);
bool XwPresentedFrame_Current(const XwRenderSnapshot* snapshot);
void XwPresentedFrame_Discard(void);
void XwPresentedFrame_Shutdown(void);
#endif
