#ifndef XW_REMASTER_DOS_DRAW_H
#define XW_REMASTER_DOS_DRAW_H
#include "xw_remaster/dos_ship.h"

bool XwDosDraw_PrepareResources(AeronSampleCount samples);
bool XwDosDraw_Begin(AeronCommandBuffer* cmd, const XwRenderSnapshot* snapshot, const XwRenderView* view,
					 AeronSampleCount samples);
bool XwDosDraw_Add(const XwDosPart* part);
/* Borrow a procedural mesh through completion of this scene submission. */
bool XwDosDraw_AddMesh(const XwDosPart* part, const XwDosMesh* mesh);
bool XwDosDraw_Upload(AeronCommandBuffer* cmd);
void XwDosDraw_Pass(AeronCommandBuffer* cmd, AeronRenderPass* pass, int width, int height, void* user);
AeronTexture* XwDosDraw_Palette(void);
void XwDosDraw_Shutdown(void);
#endif
