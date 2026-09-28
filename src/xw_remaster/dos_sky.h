#ifndef XW_REMASTER_DOS_SKY_H
#define XW_REMASTER_DOS_SKY_H
#include "xw_remaster/render_math.h"
bool XwDosSky_Prepare(AeronCommandBuffer* cmd, AeronScene3D* scene, const XwRenderSnapshot* s,
					  const XwRenderView* view);
void XwDosSky_Draw(AeronCommandBuffer* cmd, AeronRenderPass* pass, int width, int height, void* user);
void XwDosSky_Shutdown(void);
#endif
