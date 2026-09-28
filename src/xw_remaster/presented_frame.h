#ifndef XW_REMASTER_PRESENTED_FRAME_H
#define XW_REMASTER_PRESENTED_FRAME_H
#include "xw_remaster/flight_scene.h"

/* This owns completed modern GPU output, independently of mutable scene targets. */
bool XwPresentedFrame_Store(const XwRenderSnapshot* snapshot, const XwFlightOutput* output);
const XwFlightOutput* XwPresentedFrame_Get(const XwRenderSnapshot* snapshot);
bool XwPresentedFrame_Current(const XwRenderSnapshot* snapshot);
void XwPresentedFrame_Discard(void);
void XwPresentedFrame_Shutdown(void);
#endif
