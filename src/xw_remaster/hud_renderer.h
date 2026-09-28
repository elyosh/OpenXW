#ifndef XW_REMASTER_HUD_RENDERER_H
#define XW_REMASTER_HUD_RENDERER_H
#include "xw_remaster/render_math.h"
/* Complete standalone HUD preparation; drawing belongs to the selected world's final composition. */
bool XwHudRenderer_Prepare(const XwRenderSnapshot* snapshot, const XwRenderView* view, int width, int height);
bool XwHudRenderer_PrepareWorldMarkers(AeronCommandBuffer* cmd, const XwRenderSnapshot* snapshot,
									   const XwRenderView* view, AeronScene3D* scene);
/* Draw depth-tested target corners while the world depth attachment is available. */
void XwHudRenderer_DrawWorldMarkers(AeronCommandBuffer* cmd, AeronRenderPass* pass,
									AeronRenderTarget* target);
void XwHudRenderer_Draw(AeronCommandBuffer* cmd, AeronRenderPass* pass, AeronRenderTarget* target);
void XwHudRenderer_Shutdown(void);
#endif
