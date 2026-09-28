#include "xw_remaster/presented_frame.h"
#include "xw_remaster/config.h"
#include "xw_remaster/flight_pipeline.h"
#include <aeron/aeron.h>
#include <string.h>

typedef struct PresentedFrame {
	AeronRenderTarget* target;
	XwFlightOutput output;
	XwRenderAssetId assets;
	uint64_t config, presentation;
	uint8_t version;
} PresentedFrame;

static PresentedFrame frames[2], failed;
static unsigned current;
static bool valid, failure;

static bool Epoch(const PresentedFrame* f, const XwRenderSnapshot* s) {
	return s && f->version == s->flight_version &&
		   f->output.key.mission_generation == s->key.mission_generation &&
		   f->output.key.world_generation == s->key.world_generation;
}

const XwFlightOutput* XwPresentedFrame_Get(const XwRenderSnapshot* s) {
	const PresentedFrame* f = &frames[current];
	return valid && s && f->version == s->flight_version &&
				   f->output.key.mission_generation == s->key.mission_generation &&
				   f->presentation == s->presentation_generation
			   ? &f->output
			   : NULL;
}

bool XwPresentedFrame_Current(const XwRenderSnapshot* s) {
	const XwFlightOutput* out = XwPresentedFrame_Get(s);
	return out && Epoch(&frames[current], s) && out->key.view_serial == s->key.view_serial &&
		   out->key.hud_revision == s->key.hud_revision && frames[current].assets == s->flight_assets &&
		   frames[current].config == XwRemasterConfig_Generation();
}

bool XwPresentedFrame_Store(const XwRenderSnapshot* s, const XwFlightOutput* out) {
	if (!s || !out || (!out->texture && !out->direct) || out->width <= 0 || out->height <= 0)
		return false;
	if (XwPresentedFrame_Current(s) && frames[current].output.revision == out->revision)
		return true;
	if (failure && Epoch(&failed, s) && failed.assets == s->flight_assets &&
		failed.config == XwRemasterConfig_Generation() && failed.output.width == out->width &&
		failed.output.height == out->height)
		return false;
	unsigned next = current ^ 1;
	PresentedFrame* f = &frames[next];
	if (!f->target || f->output.width != out->width || f->output.height != out->height) {
		AeronRenderTarget* target = Aeron_CreateRenderTarget(
			&(AeronRenderTargetDesc) { .width = out->width,
									   .height = out->height,
									   .format = AERON_TEXTURE_FORMAT_RGBA16_FLOAT,
									   .debug_name = "X-Wing retained presentation" });
		if (!target)
			goto fail;
		Aeron_DestroyRenderTarget(f->target);
		f->target = target;
		f->output.width = out->width;
		f->output.height = out->height;
	}
	AeronCommandBuffer* cmd = Aeron_AcquireCommandBuffer();
	if (!cmd)
		goto fail;
	bool recorded =
		out->texture
			? Aeron_CopyTextureCmd(
				  cmd, &(AeronTextureCopyDesc) { .source = out->texture,
												 .destination = Aeron_RenderTargetGetTexture(f->target),
												 .width = out->width,
												 .height = out->height,
												 .cycle = 1 })
			: XwFlightPipeline_DrawRetained(cmd, f->target);
	if (!recorded) {
		Aeron_CancelCommandBuffer(cmd);
		goto fail;
	}
	if (!Aeron_SubmitCommandBuffer(cmd))
		goto fail;
	f->output = *out;
	f->output.texture = Aeron_RenderTargetGetTexture(f->target);
	f->output.direct = false;
	f->assets = s->flight_assets;
	f->config = XwRemasterConfig_Generation();
	f->version = s->flight_version;
	f->presentation = s->presentation_generation;
	current = next;
	valid = true;
	failure = false;
	return true;
fail:
	failed = (PresentedFrame) { .output = *out,
								.assets = s->flight_assets,
								.config = XwRemasterConfig_Generation(),
								.version = s->flight_version };
	failure = true;
	Aeron_RequestFatalRendererError("retaining modern flight output");
	return false;
}

void XwPresentedFrame_Discard(void) { valid = false; }

void XwPresentedFrame_Shutdown(void) {
	for (unsigned i = 0; i < 2; ++i)
		Aeron_DestroyRenderTarget(frames[i].target);
	memset(frames, 0, sizeof frames);
	memset(&failed, 0, sizeof failed);
	current = 0;
	valid = failure = false;
}
