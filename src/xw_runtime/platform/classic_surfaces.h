#ifndef XW_RUNTIME_CLASSIC_SURFACES_H
#define XW_RUNTIME_CLASSIC_SURFACES_H
#include "xw/flight/flight_display.h"
#include "xw_runtime/snapshot/render_capture.h"
#include <stdbool.h>
XwRenderSurface XwDisplay_RenderSurface(void);

#ifdef __cplusplus
extern "C" {
#endif

void XwDisplay_LockSurface(void);
void XwDisplay_UnlockSurface(void);
void XwDisplay_LockRaw(IDirectDrawSurface* surface, DDSURFACEDESC* desc);
int XwDisplay_UnlockRaw(IDirectDrawSurface* surface, void* pixels);
int XwDisplay_Blit(IDirectDrawSurface* dst, XwDirectDrawRect* dst_rect, IDirectDrawSurface* src,
				   XwDirectDrawRect* src_rect, uint32_t flags, DDBLTFX* effects);
int XwDisplay_Flip(void);
void XwDisplay_ClearSurface(IDirectDrawSurface* surface);
void XwDisplay_SetPalette(const uint8_t* rgb, int start, unsigned count);
void XwDisplay_RestorePalette(void);
int XwDisplay_BeginSurfaceChange(void);
void XwDisplay_EndSurfaceChange(int locks);
void XwDisplay_BindLandruVideo(void);
#ifdef __cplusplus
}
#endif
#endif
