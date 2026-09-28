#ifndef XW_RUNTIME_PRESENTATION_H
#define XW_RUNTIME_PRESENTATION_H
#include <aeron/aeron.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum XwPresentationSource {
	XW_PRESENT_FRONTEND,
	XW_PRESENT_WINDOWS_FLIGHT,
	XW_PRESENT_DOS_FLIGHT
} XwPresentationSource;

XwPresentationSource XwPresentation_BaseSource(void);
void XwPresentation_SelectBaseSource(XwPresentationSource source);
void XwPresentation_Init(void);
/* Base layers precede the remaster; cursor follows it in the application loop. */
void XwPresentation_EndFrame(void);
void XwPresentation_SubmitCursor(void);
/* A completed opaque modern flight view is required; this is not a user preference. */
void XwPresentation_SetModernFlightOpaque(bool opaque);
bool XwPresentation_ClassicHardwareSuppressed(void);
void XwPresentation_Shutdown(void);
void XwPresentation_Invalidate(void);
/* Original flight UI keeps its classic surfaces and requires a fresh world draw on return. */
void XwPresentation_BeginFlightUi(void);
/* Sync before input/task work; source scene extents remain independent. */
bool XwPresentation_SyncToWindow(int width, int height);
/* Refresh an application-owned input copy after the logical width changes. */
void XwPresentation_MapHostPointer(AeronInputSnapshot* input);
AeronRectI XwPresentation_Frame(void);
AeronRectI XwPresentation_ClassicRect(void);
/* Scene coordinates share the same 4:3 rectangle as the final presentation. */
int XwPresentation_MouseToScene(const AeronInputSnapshot* input, int* x, int* y);
int XwPresentation_WarpScene(int x, int y);
void XwPresentation_SetSceneExtent(int width, int height);
void XwPresentation_SetPointerSuppressed(bool suppressed);
bool XwPresentation_PointerSuppressed(void);
#ifdef __cplusplus
}
#endif
#endif
