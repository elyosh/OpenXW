#ifndef XW_RENDER_SNAPSHOT_H
#define XW_RENDER_SNAPSHOT_H
#include "xw_runtime/snapshot/render_types.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Owner thread only. Current and Previous stay immutable until the next commit. */
void XwRenderSnapshot_Init(void);
void XwRenderSnapshot_Shutdown(void);
void XwRenderSnapshot_BeginTick(void);
void XwRenderSnapshot_Commit(int focused, int paused);
void XwRenderSnapshot_CancelTick(void);
const XwRenderSnapshot* XwRenderSnapshot_Current(void);
const XwRenderSnapshot* XwRenderSnapshot_Previous(void);
/* Capture-only access, valid during an open host tick. */
XwRenderSnapshot* XwRenderSnapshot_Writer(void);
#ifdef __cplusplus
}
#endif
#endif
