#include "xw_runtime/platform/classic_surfaces.h"
#include "xw/flight/flight.h"
#include "xw/flight/replay/replay.h"
#include "xw/input/win_mouse.h"
#include "xw/landru_config.h"
#include "xw/render/renderer.h"
#include "xw/render/rtsvga2.h"
#include "xw/render/std3d.h"
#include "xw_runtime/runtime/frontend_task.h"
#include "xw_runtime/runtime/presentation.h"
#include "xw_runtime/snapshot/render_capture.h"
#include "xw_runtime/snapshot/render_hud.h"
#include <aeron/compat/host.h>
#include <aeron/log.h>
#include <landru/cursor.h>
#include <landru/surface.h>
#include <landru/vesa.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static IDirectDrawSurface* locked_surface;
static void* locked_pixels;

static XwRenderSurface SurfaceId(IDirectDrawSurface* surface) {
	if (!surface)
		return XW_RENDER_SURFACE_NONE;
	if (surface == g_flightBackBuffer)
		return XW_RENDER_SURFACE_BACK;
	if (surface == g_flightPrimarySurface)
		return XW_RENDER_SURFACE_FRONT;
	if (surface == g_flightOffscreenSurface)
		return XW_RENDER_SURFACE_OFFSCREEN;
	if (surface == g_flightAuxiliarySurface)
		return XW_RENDER_SURFACE_AUXILIARY;
	return XW_RENDER_SURFACE_NONE;
}

XwRenderSurface XwDisplay_RenderSurface(void) {
	if (locked_surface)
		return SurfaceId(locked_surface);
	bool auxiliary = !g_flightDisplaySurfaceMode && !g_replayviewmode && !g_ReplayReenterSimulation;
	return auxiliary                      ? XW_RENDER_SURFACE_AUXILIARY
		   : g_flightLockOffscreenSurface ? XW_RENDER_SURFACE_OFFSCREEN
										  : XW_RENDER_SURFACE_BACK;
}

static uint8_t ClassicSource(void) {
	return g_useHardware3D ? XW_SNAP_CLASSIC_WINDOWS_HARDWARE : XW_SNAP_CLASSIC_WINDOWS_SOFTWARE;
}

/* Full logical cockpit copies can occupy a centered rectangle of the physical surface. */
static bool FullLogicalCopy(IDirectDrawSurface* dst, const XwDirectDrawRect* dr, IDirectDrawSurface* src,
							const XwDirectDrawRect* sr) {
	if (!dr && !sr)
		return true;
	int x = (g_flightDisplayWidth - g_surfaceWidth) / 2;
	int y = (g_flightDisplayHeight - g_surfaceHeight) / 2;
	return dst == g_flightBackBuffer && src == g_flightOffscreenSurface && dr && sr && sr->left == 0 &&
		   sr->top == 0 && sr->right == g_surfaceWidth && sr->bottom == g_surfaceHeight && dr->left == x &&
		   dr->top == y && dr->right == x + g_surfaceWidth && dr->bottom == y + g_surfaceHeight;
}

static void LockFailure(const char* operation, HRESULT result) {
	char message[160];
	snprintf(message, sizeof message, "%s failed (0x%08x)", operation, (unsigned)result);
	Aeron_FatalError("OpenXW display", message);
	exit(EXIT_FAILURE);
}

static int Check(HRESULT result, const char* operation) {
	if (result == DX_DD_OK)
		return 1;
	Aeron_LogError("xw.display", "%s failed (0x%08x)", operation, (unsigned)result);
	Aeron_RequestFatalRendererError(operation);
	g_quitRequested = 1;
	XwRenderCapture_CancelView();
	return 0;
}

void XwDisplay_LockRaw(IDirectDrawSurface* surface, DDSURFACEDESC* desc) {
	if (!surface)
		LockFailure("Lock missing surface", -1);
	memset(desc, 0, sizeof *desc);
	desc->dwSize = sizeof *desc;
	HRESULT result = surface->lpVtbl->Lock(surface, NULL, desc, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL);
	if (result == DX_DDERR_SURFACELOST && surface->lpVtbl->Restore(surface) == DX_DD_OK)
		result = surface->lpVtbl->Lock(surface, NULL, desc, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL);
	if (result != DX_DD_OK || !desc->lpSurface || desc->lPitch <= 0)
		LockFailure("Lock surface", result);
}

int XwDisplay_UnlockRaw(IDirectDrawSurface* surface, void* pixels) {
	return surface && Check(surface->lpVtbl->Unlock(surface, pixels), "Unlock surface");
}

void XwDisplay_BindLandruVideo(void) {
	if (g_flightBytesPerPixel == 1 && g_surfacePixels && g_surfacePitch > 0 && g_surfacePitch <= INT16_MAX) {
		xvesa_Set_Video_Buffer(g_surfacePixels);
		xvesa_Set_Platform_Pitch((int16_t)g_surfacePitch);
	}
}

void XwDisplay_LockSurface(void) {
	if (XwPresentation_BaseSource() == XW_PRESENT_DOS_FLIGHT)
		return;
	if (g_surfaceLockCount > 0) {
		if (g_surfaceLockCount == INT_MAX)
			LockFailure("Surface lock nesting overflow", -1);
		++g_surfaceLockCount;
		XwDisplay_BindLandruVideo();
		return;
	}
	bool auxiliary = !g_flightDisplaySurfaceMode && !g_replayviewmode && !g_ReplayReenterSimulation;
	locked_surface = auxiliary                      ? g_flightAuxiliarySurface
					 : g_flightLockOffscreenSurface ? g_flightOffscreenSurface
													: g_flightBackBuffer;
	DDSURFACEDESC desc;
	XwDisplay_LockRaw(locked_surface, &desc);
	locked_pixels = desc.lpSurface;
	if (desc.dwWidth < (uint32_t)g_surfaceWidth || desc.dwHeight < (uint32_t)g_surfaceHeight ||
		desc.lPitch < (int64_t)g_surfaceWidth * g_flightBytesPerPixel)
		LockFailure("Invalid surface extent", -1);
	uint8_t* previous = g_flightSwFramebufferBase;
	int old_pitch = g_surfacePitch;
	g_swFramebufferBase = desc.lpSurface;
	g_flightSwFramebufferBase = desc.lpSurface;
	if (auxiliary || !g_flightLockOffscreenSurface)
		g_flightSwFramebufferBase +=
			((desc.dwHeight - (uint32_t)g_surfaceHeight) / 2) * (size_t)desc.lPitch +
			((desc.dwWidth - (uint32_t)g_surfaceWidth) / 2) * (size_t)g_flightBytesPerPixel;
	g_surfacePixels = g_flightSwFramebufferBase;
	g_surfacePitch = g_flightSurfacePitchBytes = desc.lPitch;
	g_surfaceLockCount = 1;
	FlightDisplay_SetViewport480ByteSpan((int32_t)((uint32_t)desc.lPitch * FLIGHT_DISPLAY_HEIGHT));
	if (old_pitch != desc.lPitch || previous != g_flightSwFramebufferBase)
		rtsvga2_setvgapointers(g_flightSwFramebufferBase, desc.lPitch, FLIGHT_DISPLAY_HEIGHT);
	XwDisplay_BindLandruVideo();
}

void XwDisplay_UnlockSurface(void) {
	if (XwPresentation_BaseSource() == XW_PRESENT_DOS_FLIGHT)
		return;
	if (g_surfaceLockCount <= 0) {
		g_surfaceLockCount = 0;
		return;
	}
	if (--g_surfaceLockCount)
		return;
	if (!locked_surface ||
		!Check(locked_surface->lpVtbl->Unlock(locked_surface, locked_pixels), "Unlock surface")) {
		locked_surface = NULL;
		locked_pixels = NULL;
		return;
	}
	XwRenderCapture_SurfaceWrite(SurfaceId(locked_surface));
	locked_surface = NULL;
	locked_pixels = NULL;
}

int XwDisplay_Blit(IDirectDrawSurface* dst, XwDirectDrawRect* dst_rect, IDirectDrawSurface* src,
				   XwDirectDrawRect* src_rect, uint32_t flags, DDBLTFX* effects) {
	if (!dst)
		return Check(-1, "Blit missing surface");
	XwPresentation_SelectBaseSource(XwPresentation_BaseSource());
	uint64_t before = AeronDx5_GetClassicFlightFrameSerial();
	HRESULT result = dst->lpVtbl->Blt(dst, dst_rect, src, src_rect, flags, effects);
	if (result == DX_DDERR_SURFACELOST && dst->lpVtbl->Restore(dst) == DX_DD_OK &&
		(!src || src == dst || src->lpVtbl->Restore(src) == DX_DD_OK))
		result = dst->lpVtbl->Blt(dst, dst_rect, src, src_rect, flags, effects);
	if (!Check(result, "Blit surface") || Aeron_FatalErrorRequested())
		return 0;
	/* DOS may clear dormant Windows surfaces; they do not own its presentation. */
	if (XwPresentation_BaseSource() != XW_PRESENT_WINDOWS_FLIGHT)
		return 1;
	bool complete = FullLogicalCopy(dst, dst_rect, src, src_rect);
	bool fresh = AeronDx5_GetClassicFlightFrameSerial() != before;
	bool presented = fresh || (g_useHardware3D && AeronDx5_IsClassicFlightRenderingSuppressed());
	if (dst == g_flightPrimarySurface && src == g_flightBackBuffer && complete) {
		if (presented)
			XwRenderCapture_Presented(XW_RENDER_SURFACE_BACK, ClassicSource(), fresh, false);
	} else {
		XwRenderCapture_CopySurface(SurfaceId(dst), SurfaceId(src), complete && src != NULL);
		if (dst == g_flightPrimarySurface && presented)
			XwRenderCapture_Presented(XW_RENDER_SURFACE_FRONT, ClassicSource(), fresh, false);
	}
	return 1;
}

void XwDisplay_ClearSurface(IDirectDrawSurface* surface) {
	if (!surface)
		return;
	DDBLTFX effects = { 0 };
	effects.dwSize = sizeof effects;
	XwDisplay_Blit(surface, NULL, NULL, NULL, DDBLT_COLORFILL, &effects);
}

int XwDisplay_Flip(void) {
	XwPresentation_SelectBaseSource(XwPresentation_BaseSource());
	if (XwPresentation_BaseSource() == XW_PRESENT_DOS_FLIGHT) {
		if (!g_quitRequested && g_windowActive)
			XwRenderCapture_Presented(XW_RENDER_SURFACE_DOS, XW_SNAP_CLASSIC_DOS, false, false);
		return 0;
	}
	if (g_quitRequested || !g_windowActive)
		return g_quitRequested;
	if (!g_flightPrimarySurface || !g_flightBackBuffer) {
		Check(-1, "Present missing surface");
		return -1;
	}
	if (!XwFrontend_IsActive() && g_SoftwareCursor && g_flightBytesPerPixel == 1 &&
		xcursor_Get_Display_Count() >= 0 && !xsurface_Uses_Native_Vga_Presentation() &&
		!XwPresentation_PointerSuppressed()) {
		DDSURFACEDESC desc;
		XwDisplay_LockRaw(g_flightBackBuffer, &desc);
		xcursor_Draw_Software_Cursor_To_Surface(desc.lpSurface, desc.lPitch, (int)desc.dwHeight);
		if (!Check(g_flightBackBuffer->lpVtbl->Unlock(g_flightBackBuffer, desc.lpSurface),
				   "Unlock cursor surface"))
			return -1;
	}
	if (!g_flightPageFlip)
		return XwDisplay_Blit(g_flightPrimarySurface, NULL, g_flightBackBuffer, NULL, DDBLT_WAIT, NULL) ? 0
																										: -1;
	uint64_t before = AeronDx5_GetClassicFlightFrameSerial();
	HRESULT result = g_flightPrimarySurface->lpVtbl->Flip(g_flightPrimarySurface, NULL, DDFLIP_WAIT);
	if (result == DX_DDERR_SURFACELOST) {
		if (g_flightPrimarySurface->lpVtbl->Restore(g_flightPrimarySurface) == DX_DD_OK &&
			g_flightBackBuffer->lpVtbl->Restore(g_flightBackBuffer) == DX_DD_OK &&
			(!g_std3DZBufferSurface ||
			 g_std3DZBufferSurface->lpVtbl->Restore(g_std3DZBufferSurface) == DX_DD_OK))
			result = g_flightPrimarySurface->lpVtbl->Flip(g_flightPrimarySurface, NULL, DDFLIP_WAIT);
	} else if (result == DX_DDERR_NOEXCLUSIVEMODE) {
		result = g_flightDirectDraw->lpVtbl->SetCooperativeLevel(g_flightDirectDraw, g_flightMainWindowHandle,
																 DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE |
																	 DDSCL_ALLOWMODEX);
		if (result == DX_DD_OK)
			result = g_flightPrimarySurface->lpVtbl->Flip(g_flightPrimarySurface, NULL, DDFLIP_WAIT);
	}
	if (!Check(result, "Flip surface") || Aeron_FatalErrorRequested())
		return -1;
	bool fresh = AeronDx5_GetClassicFlightFrameSerial() != before;
	if (fresh || (g_useHardware3D && AeronDx5_IsClassicFlightRenderingSuppressed()))
		XwRenderCapture_Presented(XW_RENDER_SURFACE_BACK, ClassicSource(), fresh, g_useHardware3D != 0);
	return 0;
}

static void CaptureIndexedPalette(void) {
	uint32_t colors[256];
	for (unsigned i = 0; i < 256; ++i) {
		const XwDirectDrawPaletteEntry* p = &g_directDrawPaletteEntries[i];
		colors[i] = 0xff000000u | ((uint32_t)p->red << 16) | ((uint32_t)p->green << 8) | p->blue;
	}
	XwHud_IndexedPalette(colors);
}

void XwDisplay_SetPalette(const uint8_t* rgb, int start, unsigned count) {
	if (!rgb || start < 0 || start >= 256 || !count)
		return;
	if (count > (unsigned)(256 - start))
		count = (unsigned)(256 - start);
	int locks = g_surfaceLockCount;
	for (int i = 0; i < locks; ++i)
		XwDisplay_UnlockSurface();
	for (unsigned i = (unsigned)start; i < (unsigned)start + count; ++i) {
		g_directDrawPaletteEntries[i].red = rgb[i * 3] << 2;
		g_directDrawPaletteEntries[i].green = rgb[i * 3 + 1] << 2;
		g_directDrawPaletteEntries[i].blue = rgb[i * 3 + 2] << 2;
		g_directDrawPaletteEntries[i].flags = 0;
	}
	if (g_SoftwareCursor)
		xcursor_Select_Contrast_Colors(rgb);
	if (g_ddPalette)
		Check(
			g_ddPalette->lpVtbl->SetEntries(g_ddPalette, 0, start, count, g_directDrawPaletteEntries + start),
			"Update palette");
	CaptureIndexedPalette();
	for (int i = 0; i < locks; ++i)
		XwDisplay_LockSurface();
}

void XwDisplay_RestorePalette(void) {
	int locks = g_surfaceLockCount;
	for (int i = 0; i < locks; ++i)
		XwDisplay_UnlockSurface();
	if (g_ddPalette)
		Check(g_ddPalette->lpVtbl->SetEntries(g_ddPalette, 0, 0, 256, g_directDrawPaletteEntries),
			  "Restore palette");
	CaptureIndexedPalette();
	for (int i = 0; i < locks; ++i)
		XwDisplay_LockSurface();
}

int XwDisplay_BeginSurfaceChange(void) {
	XwHud_Reset();
	int locks = g_surfaceLockCount;
	for (int i = 0; i < locks; ++i)
		XwDisplay_UnlockSurface();
	xvesa_Set_Video_Buffer(NULL);
	g_surfacePixels = g_flightSwFramebufferBase = g_swFramebufferBase = NULL;
	g_flightRenderSurface = NULL;
	XwPresentation_Invalidate();
	return locks;
}

void XwDisplay_EndSurfaceChange(int locks) {
	IDirectDrawSurface* surfaces[] = { g_flightPrimarySurface, g_flightBackBuffer, g_flightAuxiliarySurface,
									   g_flightOffscreenSurface };
	if (g_ddPalette && g_flightBytesPerPixel == 1)
		for (unsigned i = 0; i < sizeof surfaces / sizeof surfaces[0]; ++i)
			if (surfaces[i])
				Check(surfaces[i]->lpVtbl->SetPalette(surfaces[i], g_ddPalette), "Attach palette");
	XwDisplay_LockSurface();
	if (locks > 0)
		g_surfaceLockCount = locks;
	else
		XwDisplay_UnlockSurface();
}
