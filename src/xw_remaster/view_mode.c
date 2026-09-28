/* Flight-only adaptation of OpenXvT's blend and fresh-classic handoff. */
#include "xw_remaster/view_mode.h"
#include "xw_remaster/flight_pipeline.h"
#include "xw_remaster/presented_frame.h"
#include "xw_runtime/input/capture.h"
#include "xw_runtime/input/input_bridge.h"
#include "xw_runtime/input/keyboard_mapping.h"
#include "xw_runtime/runtime/flight_frame.h"
#include "xw_runtime/runtime/flight_loading.h"
#include "xw_runtime/runtime/port.h"
#include "xw_runtime/runtime/presentation.h"
#include "xw_runtime/snapshot/render_capture.h"
#include <aeron/compat/host.h>
#include <aeron/scene/blend_ramp.h>

static AeronBlendRamp blend;
static bool consumed_tab, world_ready, classic_dirty;
static uint64_t classic_serial, completion_serial;

static bool OwnsFlight(const XwRenderSnapshot* s) {
	return s && s->owner == XW_SNAP_OWNER_FLIGHT && XwPresentation_BaseSource() != XW_PRESENT_FRONTEND &&
		   XwRenderCapture_IsCurrentWorld(&s->key);
}

static bool OwnsPresentation(const XwRenderSnapshot* s) {
	return s && (s->owner == XW_SNAP_OWNER_FLIGHT || s->owner == XW_SNAP_OWNER_FLIGHT_TRANSITION) &&
		   XwPresentation_BaseSource() != XW_PRESENT_FRONTEND && XwRenderCapture_IsCurrentPresentation(s);
}

static bool Flight(const XwRenderSnapshot* s) { return OwnsFlight(s) && s->world_valid && s->hud_valid; }

static void Reset(void) {
	blend.alpha = 0;
	world_ready = classic_dirty = false;
	XwPresentedFrame_Discard();
	XwPresentation_SetModernFlightOpaque(false);
}

void XwRemasterView_Invalidate(void) { Reset(); }

void XwRemasterView_Init(void) {
	Aeron_BlendRampInit(&blend);
	consumed_tab = false;
	XwInput_SuppressRendererTab(false);
	classic_serial = completion_serial = 0;
	Reset();
}

void XwRemasterView_BeginFrame(const AeronInputSnapshot* input) {
	const XwRenderSnapshot* s = XwRenderSnapshot_Current();
	if (consumed_tab && input && !input->key_down[AERON_KEY_TAB] && !input->key_released[AERON_KEY_TAB])
		consumed_tab = false;
	int key = XwKeyboardMapping_Trigger(input, XW_KEYBOARD_SHORTCUT_RENDERER);
	if (!consumed_tab && key >= 0 && !XwInput_KeyBlocked(key) && OwnsFlight(s) &&
		XwInput_RendererShortcutAllowed()) {
		consumed_tab = true;
		Aeron_BlendRampToggle(&blend);
		Aeron_LogInfo("xw.remaster", "Renderer requested: %s", blend.target > 0 ? "modern" : "classic");
	}
	/* Reapply after input/owner resets, through the release event itself. */
	XwInput_SuppressRendererTab(consumed_tab);
	if (!OwnsPresentation(s) || (blend.alpha > 0 && !XwPresentedFrame_Get(s))) {
		Reset();
		return;
	}
	int w = 0, h = 0;
	Aeron_GetPresentationPixelSize(&w, &h);
	const XwFlightOutput* output = XwPresentedFrame_Get(s);
	bool opaque = Flight(s) && input && input->has_focus && !XwPort_SettingsOpen() &&
				  !Aeron_DebugUiVisible() && !s->paused && blend.target > 0 && blend.alpha >= 1 &&
				  world_ready && XwPresentedFrame_Current(s) && output && output->width == w &&
				  output->height == h;
	/* Dirtiness survives reversals and pauses until a wholly unsuppressed view completes. */
	bool was_suppressed = XwPresentation_ClassicHardwareSuppressed();
	XwPresentation_SetModernFlightOpaque(opaque);
	if (!classic_dirty && (was_suppressed || XwPresentation_ClassicHardwareSuppressed())) {
		classic_dirty = true;
		classic_serial = AeronDx5_GetClassicFlightFrameSerial();
		completion_serial = s->classic.completion_serial;
	}
}

bool XwRemasterView_NeedsWorld(void) { return blend.target > 0 || blend.alpha > 0 || classic_dirty; }

bool XwRemasterView_Direct(const XwRenderSnapshot* s) {
	if (!Flight(s) || s->flight_version != 98 || !s->focused || s->paused || blend.target <= 0 ||
		blend.alpha < 1 || !world_ready)
		return false;
	const XwFlightOutput* output = XwPresentedFrame_Get(s);
	int w = 0, h = 0;
	Aeron_GetPresentationPixelSize(&w, &h);
	return output && output->width == w && output->height == h;
}

static bool FreshClassic(const XwRenderSnapshot* s) {
	const XwSnapClassicFrame* classic = &s->classic;
	return Flight(s) && classic->valid && classic->source != XW_SNAP_CLASSIC_DOS &&
		   classic->completion_serial > completion_serial &&
		   AeronDx5_GetClassicFlightFrameSerial() != classic_serial &&
		   classic->key.mission_generation == s->key.mission_generation &&
		   classic->key.world_generation == s->key.world_generation &&
		   classic->key.view_serial == s->key.view_serial && classic->key.hud_revision == s->key.hud_revision;
}

void XwRemasterView_Present(const XwRenderSnapshot* s, int32_t delta_us, bool ready) {
	if (!OwnsPresentation(s)) {
		Reset();
		return;
	}
	int w = 0, h = 0;
	if (!Aeron_GetPresentationPixelSize(&w, &h) || w <= 0 || h <= 0) {
		world_ready = false;
		XwPresentation_SetModernFlightOpaque(false);
		return;
	}
	const XwFlightOutput* rendered = ready ? XwFlightScene_Output() : NULL;
	world_ready = rendered && rendered->key.mission_generation == s->key.mission_generation &&
				  rendered->key.world_generation == s->key.world_generation &&
				  rendered->key.view_serial == s->key.view_serial &&
				  rendered->key.hud_revision == s->key.hud_revision && XwPresentedFrame_Store(s, rendered);
	if (Aeron_FatalErrorRequested())
		return;
	if (ready && !world_ready) {
		Aeron_RequestFatalError("Renderer Error", "Modern output does not match the completed flight view.");
		return;
	}
	bool resizing = rendered && (rendered->width != w || rendered->height != h);
	world_ready = world_ready && !resizing;
	if (classic_dirty && FreshClassic(s))
		classic_dirty = false;
	if (!world_ready || blend.target <= 0)
		XwPresentation_SetModernFlightOpaque(false);
	const XwFlightOutput* output = XwPresentedFrame_Get(s);
	float target = output && (classic_dirty || (blend.target > 0 && world_ready)) ? 1 : 0;
	/* Preparation gaps are not renderer choices. Keep the established mix until the next view. */
	if (!world_ready && output && blend.target > 0 && !classic_dirty)
		target = blend.alpha;
	if (XwFlightLoading_Active() && XwFlightLoading_ResourcesReady() && Flight(s) &&
		(world_ready || blend.target <= 0)) {
		/* Initial entry reveals the completed image; toggles retain their crossfade. */
		blend.alpha = blend.target > 0 ? 1 : 0;
		XwFlightLoading_End();
		XwFlightFrame_ResumeClock();
	}
	Aeron_BlendRampAdvance(&blend, delta_us, target);
	if (!output || blend.alpha <= 0)
		return;
	if (world_ready && rendered->direct && blend.alpha >= 1 && blend.target > 0) {
		if (XwFlightPipeline_SubmitDirect())
			return;
		Aeron_RequestFatalRendererError("direct flight presentation");
		return;
	}
	AeronTextureLayerDesc layer = { .texture = output->texture,
									.logical_rect = XwPresentation_Frame(),
									.blend_mode = AERON_LAYER_BLEND_PREMULTIPLIED,
									.color_space = output->color_space,
									.tint_enabled = 1,
									.tint_rgba = { blend.alpha, blend.alpha, blend.alpha, blend.alpha } };
	if (!Aeron_SubmitTextureLayer(&layer))
		Aeron_RequestFatalRendererError("Modern flight presentation");
}

void XwRemasterView_Status(XwRemasterStatus* status) {
	status->requested = blend.target > 0 ? XW_RENDERER_MODERN : XW_RENDERER_CLASSIC;
	status->effective = blend.alpha > 0 ? XW_RENDERER_MODERN : XW_RENDERER_CLASSIC;
	status->modern_ready = world_ready;
	status->modern_alpha = blend.alpha;
	status->waiting_classic = classic_dirty && (blend.target <= 0 || !world_ready);
}

void XwRemasterView_Shutdown(void) {
	Reset();
	XwPresentedFrame_Shutdown();
	consumed_tab = false;
	XwInput_SuppressRendererTab(false);
}
