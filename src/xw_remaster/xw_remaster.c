/* Host lifecycle adapted from OpenXvT f643323; presentation remains game-owned. */
#include "xw_remaster/xw_remaster.h"
#include "xw_remaster/assets.h"
#include "xw_remaster/config.h"
#include "xw_remaster/flight.h"
#include "xw_remaster/flight_scene.h"
#include "xw_remaster/hud_assets.h"
#include "xw_remaster/hud_renderer.h"
#include "xw_remaster/presented_frame.h"
#include "xw_remaster/view_mode.h"
#include "xw_runtime/runtime/flight_loading.h"
#include "xw_runtime/snapshot/render_snapshot.h"
#include <aeron/aeron.h>

static int initialized;
static XwRemasterStatus status;
static uint64_t mission;
static uint8_t flight_version;

static void ReleaseFlight(void) {
	XwRemasterView_Invalidate();
	XwPresentedFrame_Shutdown();
	XwFlightScene_Shutdown();
	XwHudRenderer_Shutdown();
	XwRemasterFlight_Shutdown();
	XwRemasterAssets_Shutdown();
	status.assets_prepared = status.asset_failed = false;
	status.view_prepared = status.hud_prepared = status.scene_prepared = false;
}

int XwRemaster_Init(void) {
	if (initialized)
		return 1;
	int width, height;
	if (!Aeron_GetLogicalSize(&width, &height) || width <= 0 || height <= 0 || !XwRemasterConfig_Sync()) {
		Aeron_RequestFatalRendererError("modern flight renderer initialization");
		return 0;
	}
	status = (XwRemasterStatus) { .requested = XW_RENDERER_MODERN,
								  .effective = XW_RENDERER_CLASSIC,
								  .modern_ready = false };
	XwRemasterView_Init();
	initialized = 1;
	Aeron_LogInfo("xw.remaster", "Driver initialized; classic presentation active");
	return 1;
}

void XwRemaster_BeginFrame(const AeronInputSnapshot* input) {
	if (!initialized || Aeron_FatalErrorRequested())
		return;
	if (!XwRemasterConfig_Sync()) {
		Aeron_RequestFatalRendererError("Renderer configuration update");
		return;
	}
	XwRemasterView_BeginFrame(input);
	XwRemasterView_Status(&status);
}

void XwRemaster_Frame(int32_t delta_us) {
	if (!initialized || Aeron_FatalErrorRequested())
		return;
	const XwRenderSnapshot* snapshot = XwRenderSnapshot_Current();
	uint64_t next_mission = snapshot ? snapshot->key.mission_generation : 0;
	uint8_t next_version = snapshot ? snapshot->flight_version : 0;
	if (mission != next_mission || flight_version != next_version) {
		/* Retire borrowers before their caches, even when the window is minimized. */
		ReleaseFlight();
		mission = next_mission;
		flight_version = next_version;
	}
	int width = 0, height = 0;
	if (!Aeron_GetPresentationPixelSize(&width, &height) || width <= 0 || height <= 0) {
		XwRemasterFlight_Prepare(snapshot, 0, 0);
		status.view_prepared = status.hud_prepared = false;
		status.scene_prepared = false;
		XwFlightScene_Invalidate();
		XwHudRenderer_Prepare(NULL, NULL, 0, 0);
		XwRemasterView_Present(snapshot, delta_us, false);
		XwRemasterView_Status(&status);
		return;
	}
	XwAssetPreparation result = XwRemasterAssets_Frame(snapshot);
	status.assets_prepared = result == XW_ASSETS_READY;
	status.asset_failed = result == XW_ASSETS_FAILED;
	if (Aeron_FatalErrorRequested())
		return;
	if (result == XW_ASSETS_FAILED) {
		Aeron_RequestFatalRendererError("modern flight asset preparation");
		return;
	}
	if (!XwHudAssets_Prepare(snapshot))
		return;
	if (XwFlightLoading_Active() && !XwFlightLoading_ResourcesReady() && status.assets_prepared) {
		uint64_t started = Aeron_NowUs();
		if (!XwFlightScene_PrepareResources(snapshot->flight_version, width, height)) {
			Aeron_RequestFatalRendererError("flight renderer preparation during loading");
			return;
		}
		XwFlightLoading_SetResourcesReady(true);
		Aeron_LogDebug("xw.remaster", "Flight renderer prepared in %.1f ms",
					   (double)(Aeron_NowUs() - started) / 1000.0);
	}
	if (!XwRemasterFlight_Prepare(snapshot, width, height)) {
		Aeron_RequestFatalError("Renderer Error",
								"The modern flight camera or projection could not be prepared.");
		return;
	}
	const XwPreparedFlight* frame = XwRemasterFlight_Current();
	status.view_prepared = frame && frame->drawable;
	if (status.view_prepared) {
		width = (int)frame->cockpit_layout.target_width;
		height = (int)frame->cockpit_layout.target_height;
	}
	status.hud_prepared = XwHudRenderer_Prepare(status.view_prepared ? snapshot : NULL,
												status.view_prepared ? &frame->view : NULL, width, height);
	if (Aeron_FatalErrorRequested())
		return;
	if (status.view_prepared && snapshot->hud_valid && !status.hud_prepared) {
		Aeron_RequestFatalRendererError("modern cockpit preparation");
		return;
	}
	if (XwFlightLoading_Active() && status.view_prepared && status.hud_prepared)
		XwFlightLoading_ViewCompleted();
	status.scene_prepared = false;
	if (status.assets_prepared && status.view_prepared && status.hud_prepared &&
		XwRemasterView_NeedsWorld()) {
		status.scene_prepared = XwFlightScene_Frame(snapshot, frame, XwRemasterView_Direct(snapshot));
		if (!status.scene_prepared) {
			Aeron_RequestFatalRendererError("modern flight scene preparation");
			return;
		}
	} else
		XwFlightScene_Invalidate();
	XwRemasterView_Present(snapshot, delta_us, status.scene_prepared);
	XwRemasterView_Status(&status);
}

XwRemasterStatus XwRemaster_Status(void) { return status; }

void XwRemaster_Shutdown(void) {
	XwRemasterView_Shutdown();
	ReleaseFlight();
	XwRemasterConfig_Shutdown();
	status = (XwRemasterStatus) { .requested = XW_RENDERER_CLASSIC, .effective = XW_RENDERER_CLASSIC };
	initialized = 0;
	mission = flight_version = 0;
}
