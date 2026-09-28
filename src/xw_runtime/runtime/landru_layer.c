/* Indexed palette conversion and submission adapted from OpenTIE's classic layer. */
#include "xw_runtime/runtime/landru_layer.h"
#include "xw/flight/flight_display.h"
#include "xw/landru_config.h"
#include "xw_runtime/runtime/presentation.h"
#include <landru/surface.h>

static AeronPaletteEntry palette[256];
static uint32_t generation;
static uint64_t previous_generation;

void XwLandruLayer_Init(void) { generation = 0; }

void XwLandruLayer_Submit(void) {
	LandruPresentedFrame frame;
	if (g_frontendDisplayWndProcMode == 0 || Aeron_FatalErrorRequested() ||
		!xsurface_Get_Presented_Frame(&frame))
		return;
	if (!generation || frame.generation != previous_generation) {
		previous_generation = frame.generation;
		for (int i = 0; i < 256; ++i) {
			uint8_t r = frame.palette[i * 3], g = frame.palette[i * 3 + 1], b = frame.palette[i * 3 + 2];
			palette[i] = (AeronPaletteEntry) { (uint8_t)((r << 2) | (r >> 4)), (uint8_t)((g << 2) | (g >> 4)),
											   (uint8_t)((b << 2) | (b >> 4)), 255 };
		}
		if (++generation == 0)
			++generation;
	}
	const AeronPixelLayerDesc layer = { .frame = { .pixels = frame.pixels,
												   .width = 320,
												   .height = 200,
												   .pitch = 320,
												   .bpp = 8,
												   .format = AERON_PIXEL_FORMAT_INDEX8,
												   .color_space = AERON_COLOR_SPACE_SRGB,
												   .palette = palette,
												   .generation = generation },
										.logical_rect = XwPresentation_ClassicRect(),
										.blend_mode = AERON_LAYER_BLEND_OPAQUE,
										.sampling = AERON_PIXEL_SAMPLING_SHARP_BILINEAR };
	if (!Aeron_SubmitPixelLayer(&layer))
		Aeron_RequestFatalRendererError("Landru VGA frame submission");
}

void XwLandruLayer_Shutdown(void) { generation = 0; }
