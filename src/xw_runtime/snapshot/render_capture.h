#ifndef XW_RENDER_CAPTURE_H
#define XW_RENDER_CAPTURE_H
#include "xw_runtime/snapshot/render_types.h"
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef enum XwRenderSurface {
	XW_RENDER_SURFACE_NONE,
	XW_RENDER_SURFACE_FRONT,
	XW_RENDER_SURFACE_BACK,
	XW_RENDER_SURFACE_OFFSCREEN,
	XW_RENDER_SURFACE_AUXILIARY,
	XW_RENDER_SURFACE_DOS,
	XW_RENDER_SURFACE_COUNT
} XwRenderSurface;

/* Structural provenance, separate from the eventual semantic HUD revision. */
typedef struct XwRenderSurfaceState {
	XwSnapViewKey key;
	uint64_t pending_view, write_revision, hud_source_revision;
	bool incomplete, classic_complete;
} XwRenderSurfaceState;

void XwRenderCapture_Init(void);
void XwRenderCapture_Shutdown(void);
void XwRenderCapture_BeginMission(uint8_t version, bool classic_content);
void XwRenderCapture_EndMission(void);
void XwRenderCapture_WorldChanged(void);
/* Reset the intro world while keeping the same flight presentation until its replacement is ready. */
void XwRenderCapture_ContinueFlight(void);
void XwRenderCapture_SetOwner(uint8_t owner);
void XwRenderCapture_Export(XwRenderSnapshot* snapshot);
/* Begin/Seal describe one logical draw, which may wait across host ticks for a flip. */
void XwRenderCapture_BeginView(XwRenderSurface target);
/* Accepted simulation steps, including replay steps without a draw. */
void XwRenderCapture_Simulate(uint16_t ticks);
/* Called once immediately after the original main-camera update. */
void XwRenderCapture_CaptureWorld(void);
XwRenderSnapshot* XwRenderCapture_Pending(void);
void XwRenderCapture_SealView(void);
void XwRenderCapture_CancelView(void);
bool XwRenderCapture_HasPendingView(void);
/* Call only after successful surface operations; a failed operation never publishes. */
/* Suppression during any part of a pending draw disqualifies a fresh-classic handoff. */
void XwRenderCapture_ClassicSuppressed(void);
void XwRenderCapture_SurfaceWrite(XwRenderSurface target);
void XwRenderCapture_ClearSurface(XwRenderSurface target);
void XwRenderCapture_CopySurface(XwRenderSurface target, XwRenderSurface source, bool complete);
void XwRenderCapture_ComposeHud(XwRenderSurface target, XwRenderSurface source);
/* Publish the classic HUD composite only after both surfaces unlock successfully. */
void XwRenderCapture_CompleteHudComposite(bool succeeded);
/* Replay saving edits HUD over the completed world, without issuing a new draw. */
void XwRenderCapture_BeginReplayOverlay(void);
void XwRenderCapture_ResetSurfaces(void);
const XwRenderSurfaceState* XwRenderCapture_Surface(XwRenderSurface surface);
void XwRenderCapture_Presented(XwRenderSurface surface, uint8_t classic_source, bool fresh, bool flip);
/* The live DOS palette is independent of logical view completion. */
void XwRenderCapture_DosPalette(const uint32_t colors[256]);
bool XwRenderCapture_IsCurrentWorld(const XwSnapViewKey* key);
bool XwRenderCapture_IsCurrentPresentation(const XwRenderSnapshot* snapshot);
#ifdef __cplusplus
}
#endif
#endif
