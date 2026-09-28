#ifndef XW_REMASTER_DOS_SCENE_H
#define XW_REMASTER_DOS_SCENE_H
#include "xw_remaster/flight_scene.h"

/* Shared DOS palette/effect scene. Special-world coverage and visible handoff are separate gates. */
bool XwDosScene_PrepareResources(int width, int height);
bool XwDosScene_Frame(const XwRenderSnapshot* snapshot, const XwPreparedFlight* frame);
const XwFlightOutput* XwDosScene_Output(void);
void XwDosScene_Invalidate(void);
void XwDosScene_Shutdown(void);
#endif
