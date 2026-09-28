#ifndef XW_REMASTER_VIEW_MODE_H
#define XW_REMASTER_VIEW_MODE_H
#include "xw_remaster/xw_remaster.h"
#include "xw_runtime/snapshot/render_snapshot.h"
#include <aeron/input.h>

void XwRemasterView_Init(void);
void XwRemasterView_Invalidate(void);
void XwRemasterView_BeginFrame(const AeronInputSnapshot* input);
bool XwRemasterView_NeedsWorld(void);
bool XwRemasterView_Direct(const XwRenderSnapshot* snapshot);
void XwRemasterView_Present(const XwRenderSnapshot* snapshot, int32_t delta_us, bool ready);
void XwRemasterView_Status(XwRemasterStatus* status);
void XwRemasterView_Shutdown(void);
#endif
