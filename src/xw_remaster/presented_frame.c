#include "xw_remaster/presented_frame.h"
#include "xw_remaster/config.h"
#include "xw_remaster/flight_pipeline.h"
#include <aeron/aeron.h>
#include <string.h>

typedef struct PresentedFrame {
	XwFlightOutput output;
	XwRenderAssetId assets;
	uint64_t config, presentation;
	uint8_t version;
} PresentedFrame;

static PresentedFrame frame;
static bool valid;

const XwFlightOutput* XwPresentedFrame_Get(const XwRenderSnapshot* s) {
	const PresentedFrame* f = &frame;
	return valid && s && f->version == s->flight_version &&
				   f->output.key.mission_generation == s->key.mission_generation &&
				   f->presentation == s->presentation_generation
			   ? &f->output
			   : NULL;
}

bool XwPresentedFrame_Current(const XwRenderSnapshot* s) {
	const XwFlightOutput* out = XwPresentedFrame_Get(s);
	return out && out->key.world_generation == s->key.world_generation &&
		   out->key.view_serial == s->key.view_serial && out->key.hud_revision == s->key.hud_revision &&
		   frame.assets == s->flight_assets && frame.config == XwRemasterConfig_Generation();
}

bool XwPresentedFrame_Store(const XwRenderSnapshot* s, const XwFlightOutput* out) {
	if (!s || !out || (!out->texture && !out->direct) || out->width <= 0 || out->height <= 0)
		return false;
	/* Like OpenXvT, presentation borrows completed renderer output without a GPU copy. */
	frame = (PresentedFrame) { .output = *out,
							   .assets = s->flight_assets,
							   .config = XwRemasterConfig_Generation(),
							   .version = s->flight_version,
							   .presentation = s->presentation_generation };
	valid = true;
	return true;
}

bool XwPresentedFrame_Retain(const XwRenderSnapshot* s) {
	if (!XwPresentedFrame_Get(s)) {
		XwPresentedFrame_Discard();
		return true;
	}
	if (frame.output.texture)
		return true;
	/* Resolve a direct frame once before an idle/transition update changes its HUD or sources. */
	AeronCommandBuffer* cmd = Aeron_AcquireCommandBuffer();
	if (!cmd)
		goto fail;
	if (!XwFlightPipeline_Retain(cmd)) {
		Aeron_CancelCommandBuffer(cmd);
		goto fail;
	}
	bool submitted = Aeron_SubmitCommandBuffer(cmd);
	XwFlightPipeline_Commit(submitted);
	if (!submitted)
		goto fail;
	frame.output.texture = XwFlightPipeline_Output();
	if (!frame.output.texture)
		goto fail;
	frame.output.direct = false;
	return true;
fail:
	Aeron_RequestFatalRendererError("retaining modern flight output");
	return false;
}

void XwPresentedFrame_Discard(void) { valid = false; }

void XwPresentedFrame_Shutdown(void) {
	memset(&frame, 0, sizeof frame);
	valid = false;
}
