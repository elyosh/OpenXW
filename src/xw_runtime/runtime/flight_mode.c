#include "xw_runtime/runtime/flight_mode.h"
#include "xw_runtime/runtime/flight_dispatch.h"
#include "xw_runtime/runtime/flight_types.h"
#include "xw_runtime/snapshot/render_capture.h"
#include "xw_runtime/storage/dos94_assets.h"
#include "xw_runtime/timing/flight_timing.h"
#include <aeron/dialog.h>
#include <aeron/log.h>
#include <landru/timer.h>
#include <stdio.h>

static bool suspended;

bool XwFlightMode_IsSuspended(void) { return suspended; }

void XwFlightMode_Suspend(void) {
	XwRenderCapture_SetOwner(XW_SNAP_OWNER_FRONTEND);
	suspended = true;
	xtimer_Set_Elapsed_Period_Us(LANDRU_TIMER_TICK_US);
}

void XwFlightMode_DiscardSuspended(void) {
	if (!suspended)
		return;
	XwFlightMode_FreeResources();
	XwFlightMode_Deactivate();
}

bool XwFlightMode_PrepareMission(char* error, size_t capacity) {
	return XwProfile_PinRequestedMission(error, capacity);
}

bool XwFlightMode_Activate(char* error, size_t capacity) {
	if (suspended) {
		suspended = false;
		XwRenderCapture_SetOwner(XW_SNAP_OWNER_FLIGHT_UI);
	} else {
		if (!XwFlightMode_PrepareMission(error, capacity) || !XwProfile_ActivateFlight(error, capacity))
			return false;
		XwFlightTiming_BeginSession();
		XwFlightTypes_Select(XwProfile_ActiveFlight()->version);
		XwRenderCapture_BeginMission((uint8_t)(XwGameVersion_Year(XwProfile_ActiveFlight()->version) - 1900),
									 XwProfile_MissionClassic());
	}
	xtimer_Set_Elapsed_Period_Us(XwProfile_ActiveFlight()->tick_period_us);
	return true;
}

void XwFlightMode_Deactivate(void) {
	/* The task owner cancels its children and releases backend handles first. */
	XwFlightTiming_EndSession();
	XwRenderCapture_EndMission();
	Dos94Assets_Reset();
	xtimer_Set_Elapsed_Period_Us(LANDRU_TIMER_TICK_US);
	suspended = false;
	XwProfile_DeactivateFlight();
}

void XwFlightMode_ReleaseMission(void) {
	XwFlightMode_DiscardSuspended();
	if (XwProfile_ReleaseMission() && XwFlightTypes_Dos())
		XwFlightTypes_Select(XW_GAME_VERSION_98);
}

void XwFlightMode_ReportError(const char* error) {
	static const AeronMessageBoxButton button = { 1, "OK", 1, 1 };
	const AeronMessageBoxOptions options = { .kind = AERON_MESSAGE_BOX_ERROR,
											 .title = "OpenXW flight",
											 .message = error,
											 .buttons = &button,
											 .button_count = 1 };
	Aeron_LogError("xw.flight", "%s", error);
	Aeron_ShowMessageBox(&options, NULL);
}
