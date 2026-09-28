#include "xw_runtime/runtime/flight_loading.h"
#include "xw_runtime/runtime/presentation.h"
#include <aeron/aeron.h>

static bool active, resources_ready, view_completed;

void XwFlightLoading_Begin(void) {
	active = true;
	resources_ready = view_completed = false;
}

void XwFlightLoading_End(void) { active = resources_ready = view_completed = false; }

bool XwFlightLoading_Active(void) { return active; }

bool XwFlightLoading_ResourcesReady(void) { return resources_ready; }

void XwFlightLoading_SetResourcesReady(bool ready) { resources_ready = active && ready; }

void XwFlightLoading_ViewCompleted(void) {
	if (active)
		view_completed = true;
}

bool XwFlightLoading_Waiting(void) { return active && (!resources_ready || view_completed); }

void XwFlightLoading_Submit(void) {
	if (!active)
		return;
	static const uint8_t black[4] = { 0, 0, 0, 255 };
	AeronPixelLayerDesc background = { .frame = { .pixels = black,
												  .width = 1,
												  .height = 1,
												  .pitch = 4,
												  .bpp = 32,
												  .format = AERON_PIXEL_FORMAT_RGBA8888,
												  .generation = 1 },
									   .logical_rect = XwPresentation_Frame(),
									   .blend_mode = AERON_LAYER_BLEND_OPAQUE };
	if (!Aeron_SubmitPixelLayer(&background))
		Aeron_RequestFatalRendererError("flight loading presentation");
}
