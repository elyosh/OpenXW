/* Output-resolution DOS cursor bitmap; Aeron caches its upload and composes it. */
#include "xw_runtime/runtime/cursor_layer.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/hud/panel.h"
#include "xw/input/win_mouse.h"
#include "xw_runtime/input/input_bridge.h"
#include "xw_runtime/input/mouse_flight.h"
#include "xw_runtime/runtime/flight_sim.h"
#include "xw_runtime/runtime/frontend_task.h"
#include "xw_runtime/runtime/presentation.h"
#include "xw_runtime/snapshot/render_capture.h"
#include "xw_runtime/snapshot/render_snapshot.h"
#include <landru/cursor.h>
#include <landru/surface.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* 9x11 DOS pixels at 640x480, including transparent padding around the tip. */
enum { CURSOR_WIDTH = 21, CURSOR_HEIGHT = 30, CURSOR_PADDING = 1, CURSOR_SAMPLES = 8 };

static struct {
	uint8_t* pixels;
	int width, height;
	uint32_t generation;
} cursor;

/* Explicit RGBA bytes keep the fixed sRGB green independent of host byte order and cockpit palettes. */
static const uint8_t flight_marker_pixels[4][4][4] = {
	{ { 0, 255, 0, 255 }, { 0, 255, 0, 255 }, { 0, 255, 0, 255 }, { 0, 255, 0, 255 } },
	{ { 0, 255, 0, 255 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 255, 0, 255 } },
	{ { 0, 255, 0, 255 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 255, 0, 255 } },
	{ { 0, 255, 0, 255 }, { 0, 255, 0, 255 }, { 0, 255, 0, 255 }, { 0, 255, 0, 255 } },
};

void XwCursorLayer_Shutdown(void) {
	free(cursor.pixels);
	memset(&cursor, 0, sizeof cursor);
}

void XwCursorLayer_Init(void) { XwCursorLayer_Shutdown(); }

static bool Prepare(int width, int height) {
	if (cursor.pixels && cursor.width == width && cursor.height == height)
		return true;
	if ((size_t)width > SIZE_MAX / 4 / (size_t)height)
		return false;
	uint8_t* pixels = malloc((size_t)width * height * 4);
	if (!pixels)
		return false;
	/* Fit to XWING.LFD / ANIM cursors frame 0: (0,0), (9,8), (0,11).
	 * Inset each edge by one DOS pixel for the white fill. Supersampling
	 * happens only on resize; movement reuses both the bitmap and GPU upload. */
	const float border = 0.8f;
	const unsigned samples = CURSOR_SAMPLES * CURSOR_SAMPLES;
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			unsigned covered = 0, white = 0;
			for (int sy = 0; sy < CURSOR_SAMPLES; ++sy) {
				float py =
					((y + (sy + 0.5f) / CURSOR_SAMPLES) * CURSOR_HEIGHT / height - CURSOR_PADDING) / 2.4f;
				for (int sx = 0; sx < CURSOR_SAMPLES; ++sx) {
					float px =
						((x + (sx + 0.5f) / CURSOR_SAMPLES) * CURSOR_WIDTH / width - CURSOR_PADDING) / 2;
					float upper = 9 * py - 8 * px, lower = 99 - 3 * px - 9 * py;
					if (px >= 0 && upper >= 0 && lower >= 0) {
						++covered;
						/* Edge normal lengths are sqrt(145) and sqrt(90). */
						if (px >= border && upper >= border * 12.0415946f && lower >= border * 9.4868330f)
							++white;
					}
				}
			}
			uint8_t* pixel = pixels + ((size_t)y * width + x) * 4;
			/* Premultiply in linear light. VGA blue #0000AA decodes to about 102/255. */
			pixel[0] = pixel[1] = (uint8_t)((white * 255 + samples / 2) / samples);
			pixel[2] = (uint8_t)((white * 255 + (covered - white) * 102 + samples / 2) / samples);
			pixel[3] = (uint8_t)((covered * 255 + samples / 2) / samples);
		}
	}
	free(cursor.pixels);
	cursor.pixels = pixels;
	cursor.width = width;
	cursor.height = height;
	if (++cursor.generation == 0)
		++cursor.generation;
	return true;
}

/* OpenXvT's four-pixel hollow stick marker, composed above either flight renderer. */
static void SubmitFlightMarker(void) {
	const XwRenderSnapshot* s = XwRenderSnapshot_Current();
	int yaw, pitch;
	if (!s || s->owner != XW_SNAP_OWNER_FLIGHT || !s->world_valid || !s->hud_valid || s->paused ||
		!s->focused || !XwRenderCapture_IsCurrentWorld(&s->key) || !XwFlightSim_IsPlayerControl() ||
		XwPresentation_BaseSource() == XW_PRESENT_FRONTEND || s->camera.external || s->cockpit.suppressed ||
		(s->camera.hud_state != 0 && s->camera.hud_state != PANEL_VIEW_FULL_FORWARD) ||
		!XwMouseFlight_GetHudMarker(&yaw, &pitch))
		return;
	const XwSnapCamera* camera = &s->camera;
	if (!camera->screen_width || !camera->screen_height || camera->viewport.height <= 0)
		return;
	AeronRectI classic = XwPresentation_ClassicRect();
	float sx = (float)classic.width / camera->screen_width;
	float sy = (float)classic.height / camera->screen_height;
	float range = camera->viewport.height * sy / 6.0f;
	float x = classic.x + (camera->viewport.x + camera->center_x) * sx + yaw * range / 127.0f;
	float y = classic.y + (camera->viewport.y + camera->center_y + camera->projection_offset_y) * sy -
			  pitch * range / 127.0f;
	AeronPixelLayerDesc layer = { .frame = { .pixels = flight_marker_pixels,
											 .width = 4,
											 .height = 4,
											 .pitch = sizeof flight_marker_pixels[0],
											 .bpp = 32,
											 .format = AERON_PIXEL_FORMAT_RGBA8888,
											 .color_space = AERON_COLOR_SPACE_SRGB,
											 .generation = 1 },
								  .logical_rect = { (int)lroundf(x) - 2, (int)lroundf(y) - 2, 4, 4 },
								  .blend_mode = AERON_LAYER_BLEND_PREMULTIPLIED,
								  .sampling = AERON_PIXEL_SAMPLING_NEAREST,
								  .scissor = classic };
	if (!Aeron_SubmitPixelLayer(&layer))
		Aeron_RequestFatalRendererError("Flight mouse marker submission");
}

void XwCursorLayer_Submit(void) {
	if (XwPresentation_PointerSuppressed() || Aeron_FatalErrorRequested())
		return;
	SubmitFlightMarker();
	if (!XwFrontend_IsActive() || !g_frontendDisplayWndProcMode || !g_SoftwareCursor ||
		XwPresentation_PointerSuppressed() || Aeron_RelativeMouseMode() || Aeron_FatalErrorRequested())
		return;
	LandruSoftwareCursor state;
	if (!xcursor_Get_Software_Cursor(&state))
		return;
	Rect bounds;
	xsurface_Get_Logical_Bounds(&bounds);
	int width, height;
	if (bounds.right <= 0 || bounds.bottom <= 0 || !Aeron_GetPresentationPixelSize(&width, &height))
		return;
	AeronRectI frame = XwPresentation_Frame(), classic = XwPresentation_ClassicRect();
	/* Match Aeron's logical-rectangle projection for a one-to-one pixel upload. */
	width = (int)((int64_t)CURSOR_WIDTH * width / frame.width);
	height = (int)((int64_t)CURSOR_HEIGHT * height / frame.height);
	if (width <= 0 || height <= 0)
		return;
	if (!Prepare(width, height)) {
		Aeron_RequestFatalError("Frontend cursor", "Could not allocate the cursor bitmap.");
		return;
	}
	int x, y;
	XwInput_CursorPosition(&x, &y);
	AeronPixelLayerDesc layer = {
		.frame = { .pixels = cursor.pixels,
				   .width = width,
				   .height = height,
				   .pitch = width * 4,
				   .bpp = 32,
				   .format = AERON_PIXEL_FORMAT_RGBA8888,
				   .color_space = AERON_COLOR_SPACE_LINEAR_SRGB,
				   .generation = cursor.generation },
		.logical_rect = { classic.x + x * classic.width / bounds.right - CURSOR_PADDING,
						  classic.y + y * classic.height / bounds.bottom - CURSOR_PADDING, CURSOR_WIDTH,
						  CURSOR_HEIGHT },
		.blend_mode = AERON_LAYER_BLEND_PREMULTIPLIED,
		.sampling = AERON_PIXEL_SAMPLING_NEAREST,
		.scissor = classic
	};
	if (!Aeron_SubmitPixelLayer(&layer))
		Aeron_RequestFatalRendererError("Frontend cursor submission");
}
