/* Dynamic host frame with a fixed classic scene, adapted from OpenXvT. */
#include "xw_runtime/runtime/presentation.h"
#include "xw/flight/flight_display.h"
#include "xw/input/win_mouse.h"
#include "xw/landru_config.h"
#include "xw/render/renderer.h"
#include "xw/util/landru_display.h"
#include "xw_dos94/render/display.h"
#include "xw_runtime/input/input_bridge.h"
#include "xw_runtime/input/system_cursor.h"
#include "xw_runtime/runtime/cursor_layer.h"
#include "xw_runtime/runtime/flight_loading.h"
#include "xw_runtime/runtime/landru_layer.h"
#include "xw_runtime/runtime/profile.h"
#include "xw_runtime/snapshot/render_capture.h"
#include "xw_runtime/snapshot/render_snapshot.h"
#include <aeron/compat/host.h>
#include <landru/surface.h>

static XwPresentationSource base_source = XW_PRESENT_FRONTEND;
static bool modern_opaque;
static AeronRectI logical_frame = { 0, 0, 640, 480 };

static bool ModernFlightReady(void) {
	const XwRenderSnapshot* s = XwRenderSnapshot_Current();
	return modern_opaque && s && s->owner == XW_SNAP_OWNER_FLIGHT && s->world_valid && s->hud_valid &&
		   XwRenderCapture_IsCurrentWorld(&s->key);
}

static void ApplySuppression(void) {
	bool suppress = base_source == XW_PRESENT_DOS_FLIGHT ||
					(base_source == XW_PRESENT_WINDOWS_FLIGHT && g_useHardware3D && ModernFlightReady());
	if (suppress)
		XwRenderCapture_ClassicSuppressed();
	AeronDx5_SetClassicFlightRenderingSuppressed(suppress);
}

bool XwPresentation_ClassicHardwareSuppressed(void) {
	return XwPresentation_BaseSource() == XW_PRESENT_WINDOWS_FLIGHT && g_useHardware3D &&
		   AeronDx5_IsClassicFlightRenderingSuppressed();
}

void XwPresentation_SetModernFlightOpaque(bool opaque) {
	modern_opaque = opaque;
	ApplySuppression();
}

XwPresentationSource XwPresentation_BaseSource(void) {
	if (g_frontendDisplayWndProcMode != 0)
		return XW_PRESENT_FRONTEND;
	return XwProfile_HasActiveFlight() && XwProfile_DosFlight() ? XW_PRESENT_DOS_FLIGHT
																: XW_PRESENT_WINDOWS_FLIGHT;
}

void XwPresentation_SelectBaseSource(XwPresentationSource source) {
	if (source != base_source) {
		base_source = source;
		if (source == XW_PRESENT_DOS_FLIGHT && Dos94_display)
			Dos94_display->fullUpdate = true;
		XwPresentation_Invalidate();
		XwRenderCapture_SetOwner(source == XW_PRESENT_FRONTEND ? XW_SNAP_OWNER_FRONTEND
															   : XW_SNAP_OWNER_FLIGHT_UI);
	}
	if (source == XW_PRESENT_DOS_FLIGHT)
		XwPresentation_SetSceneExtent(320, 200);
	else if (source == XW_PRESENT_WINDOWS_FLIGHT)
		XwPresentation_SetSceneExtent(g_flightScreenWidth, g_flightScreenHeight);
	ApplySuppression();
}

static int scene_width = 640, scene_height = 480;
static bool pointer_suppressed = true;

bool XwPresentation_SyncToWindow(int width, int height) {
	if (width <= 0 || height <= 0)
		return false;
	int64_t requested = ((int64_t)480 * width + height / 2) / height;
	if (requested < 640)
		requested = 640;
	if (requested > 32 * 480 / 9)
		requested = 32 * 480 / 9;
	int w = (int)requested & ~1;
	if (logical_frame.width == w || !Aeron_SetLogicalSize(w, 480))
		return false;
	logical_frame.width = w;
	return true;
}

AeronRectI XwPresentation_Frame(void) { return logical_frame; }

AeronRectI XwPresentation_ClassicRect(void) {
	return (AeronRectI) { (logical_frame.width - 640) / 2, 0, 640, 480 };
}

static AeronDx5Rect ClassicRect(void* context, int width, int height) {
	(void)context;
	(void)width;
	(void)height;
	AeronRectI rect = XwPresentation_ClassicRect();
	return (AeronDx5Rect) { rect.x, rect.y, rect.width, rect.height };
}

static AeronRectI WindowFrame(const AeronInputSnapshot* input) {
	if (!input || input->window_width <= 0 || input->window_height <= 0)
		return (AeronRectI) { 0 };
	int w = input->window_width, h = input->window_height;
	if ((int64_t)w * 480 > (int64_t)h * logical_frame.width)
		w = (int)((int64_t)h * logical_frame.width / 480);
	else
		h = (int)((int64_t)w * 480 / logical_frame.width);
	/* Match Aeron's aspect-fit rounding, including its small-shortfall snap. */
	if ((int64_t)(input->window_width - w) * 256 <= input->window_width)
		w = input->window_width;
	if ((int64_t)(input->window_height - h) * 256 <= input->window_height)
		h = input->window_height;
	return (AeronRectI) { (input->window_width - w) / 2, (input->window_height - h) / 2, w, h };
}

void XwPresentation_MapHostPointer(AeronInputSnapshot* input) {
	AeronRectI rect = WindowFrame(input);
	if (!rect.width || !rect.height) {
		input->mouse.x = input->mouse.y = 0;
		input->mouse.inside_content = 0;
		return;
	}
	int x = input->mouse.raw_x - rect.x, y = input->mouse.raw_y - rect.y;
	input->mouse.inside_content = x >= 0 && y >= 0 && x < rect.width && y < rect.height;
	input->mouse.x = (int)((int64_t)x * logical_frame.width / rect.width);
	input->mouse.y = (int)((int64_t)y * logical_frame.height / rect.height);
}

static AeronRectI WindowRect(const AeronInputSnapshot* input) {
	AeronRectI full = WindowFrame(input);
	int w = full.width, h = full.height;
	AeronRectI classic = XwPresentation_ClassicRect();
	int left = (int)((int64_t)classic.x * w / logical_frame.width);
	int right = (int)((int64_t)(classic.x + classic.width) * w / logical_frame.width);
	return (AeronRectI) { full.x + left, full.y, right - left, h };
}

int XwPresentation_MouseToScene(const AeronInputSnapshot* input, int* x, int* y) {
	AeronRectI rect = WindowRect(input);
	if (!rect.width || !rect.height) {
		*x = *y = 0;
		return 0;
	}
	int px = input->mouse.raw_x - rect.x, py = input->mouse.raw_y - rect.y;
	bool inside = px >= 0 && py >= 0 && px < rect.width && py < rect.height;
	if (px < 0)
		px = 0;
	if (py < 0)
		py = 0;
	if (px >= rect.width)
		px = rect.width - 1;
	if (py >= rect.height)
		py = rect.height - 1;
	*x = (int)((int64_t)px * scene_width / rect.width);
	*y = (int)((int64_t)py * scene_height / rect.height);
	return inside;
}

int XwPresentation_WarpScene(int x, int y) {
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;
	if (x >= scene_width)
		x = scene_width - 1;
	if (y >= scene_height)
		y = scene_height - 1;
	/* Logical pixel centers map through Aeron's aspect-fit window transform. */
	AeronRectI classic = XwPresentation_ClassicRect();
	int host_x = classic.x + (int)(((int64_t)x * 2 + 1) * classic.width / (2 * scene_width));
	int host_y = classic.y + (int)(((int64_t)y * 2 + 1) * classic.height / (2 * scene_height));
	g_winMousePrevPos = g_winMouseCursorPos = g_winMousePos = (XwMousePosition) { x, y };
	g_softwareCursorX = x;
	g_softwareCursorY = y;
	XwInput_SetPointer(x, y);
	return Aeron_WarpMouseLogical(host_x, host_y);
}

void XwPresentation_SetSceneExtent(int width, int height) {
	if (width <= 0 || height <= 0)
		return;
	bool changed = width != scene_width || height != scene_height;
	scene_width = width;
	scene_height = height;
	g_landruLogicalWidth = width;
	g_landruLogicalHeight = height;
	g_landruDoublePixelsEnabled = width == 320 && height == 200;
	g_landruLegacyViewportState = LANDRU_LEGACY_VIEWPORT_STATE;
	g_winMouseMinX = g_winMouseMinY = 0;
	g_winMouseMaxX = width - 1;
	g_winMouseMaxY = height - 1;
	if (changed) {
		XwInput_ResetPointer();
		XwPresentation_Invalidate();
	}
}

void XwPresentation_SetPointerSuppressed(bool suppressed) {
	if (suppressed != pointer_suppressed)
		XwInput_ResetPointer();
	pointer_suppressed = suppressed;
	XwPort_RefreshSystemCursorVisibility();
}

bool XwPresentation_PointerSuppressed(void) { return pointer_suppressed; }

void XwPresentation_Invalidate(void) {
	modern_opaque = false;
	XwRenderCapture_ResetSurfaces();
	ApplySuppression();
	AeronDx5_ResetPresentationState();
	xsurface_Invalidate_Presentation();
}

void XwPresentation_BeginFlightUi(void) {
	XwPresentation_SetModernFlightOpaque(false);
	XwRenderCapture_SetOwner(XW_SNAP_OWNER_FLIGHT_UI);
}

void XwPresentation_Init(void) {
	logical_frame = (AeronRectI) { 0, 0, 640, 480 };
	Aeron_SetLogicalSize(640, 480);
	base_source = XW_PRESENT_FRONTEND;
	modern_opaque = false;
	XwRenderCapture_SetOwner(XW_SNAP_OWNER_FRONTEND);
	scene_width = 640;
	scene_height = 480;
	pointer_suppressed = true;
	AeronDx5Config config = { .presentation_rect = ClassicRect };
	AeronDx5_Configure(&config);
	AeronDx5_SetClassicFlightRenderingSuppressed(0);
	AeronDx5_ResetPresentationState();
	XwFlightLoading_End();
	XwLandruLayer_Init();
	XwCursorLayer_Init();
}

void XwPresentation_EndFrame(void) {
	XwPresentationSource source = XwPresentation_BaseSource();
	XwPresentation_SelectBaseSource(source);
	if (source == XW_PRESENT_DOS_FLIGHT) {
		Dos94Display_Publish();
		if (Dos94_display && Dos94_display->ready) {
			uint32_t colors[256];
			for (unsigned i = 0; i < 256; ++i) {
				const AeronPaletteEntry* p = &Dos94_display->palette[i];
				colors[i] = 0xFF000000u | (uint32_t)p->r << 16 | (uint32_t)p->g << 8 | p->b;
			}
			XwRenderCapture_DosPalette(colors);
		}
		/* Keep the host-published CPU layer available for same-frame fallback. */
		Dos94Display_Submit();
	} else if (source == XW_PRESENT_FRONTEND && xsurface_Uses_Native_Vga_Presentation()) {
		XwLandruLayer_Submit();
	} else if (source != XW_PRESENT_WINDOWS_FLIGHT || !ModernFlightReady()) {
		/* Software flight still captures its CPU surfaces; the opaque modern layer covers presentation. */
		AeronDx5_EndFrame();
	}
}

void XwPresentation_SubmitCursor(void) {
	if (XwFlightLoading_Active())
		XwFlightLoading_Submit();
	else
		XwCursorLayer_Submit();
}

void XwPresentation_Shutdown(void) {
	XwFlightLoading_End();
	modern_opaque = false;
	XwRenderCapture_SetOwner(XW_SNAP_OWNER_NONE);
	XwCursorLayer_Shutdown();
	XwLandruLayer_Shutdown();
	AeronDx5_SetClassicFlightRenderingSuppressed(0);
	AeronDx5_Shutdown();
	AeronDx5_Configure(NULL);
}
