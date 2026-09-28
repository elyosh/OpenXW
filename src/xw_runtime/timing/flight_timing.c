/* Reference cadence and scoped clocks adapted from OpenXvT's offline timing owner. */
#include "xw_runtime/timing/flight_timing.h"
#include "xw/flight/xw.h"
#include "xw_runtime/runtime/flight_camera.h"
#include "xw_runtime/timing/flight_integration.h"
#include "xw_runtime/timing/player_timing.h"
#include "xw_runtime/timing/reference_motion.h"
#include <aeron/log.h>

static XwFlightTimingState timing = { .reference_ticks = XW_FRAME_MINIMUM_TICKS };
static bool session_active;
static uint32_t reported_periods;

void XwFlightTiming_ResetWorld(void) {
	timing = (XwFlightTimingState) {
		.update_rate = timing.update_rate,
		.reference_ticks = timing.reference_ticks,
	};
	reported_periods = 0;
	XwFlightIntegration_ClearAll();
	XwPlayerTiming_Reset();
	XwPlayerTiming_ResetRecovery();
	XwFlightCamera_ResetChase();
	XwReferenceMotion_SeedWorld();
}

void XwFlightTiming_BeginSession(void) {
	timing.update_rate = XwProfile_ActiveFlightRate();
	timing.reference_ticks = XwProfile_ActiveFlight()->minimum_frame_ticks;
	session_active = XwProfile_HasActiveFlight();
	XwFlightTiming_ResetWorld();
	Aeron_LogInfo("xw.flight.timing", "flight timing: %s, reference interval %u ticks",
				  XwFlightTiming_IsUnlocked() ? "unlocked" : "native", timing.reference_ticks);
}

void XwFlightTiming_EndSession(void) {
	timing = (XwFlightTimingState) { .reference_ticks = XW_FRAME_MINIMUM_TICKS };
	session_active = false;
	reported_periods = 0;
	XwFlightIntegration_ClearAll();
	XwPlayerTiming_Reset();
	XwPlayerTiming_ResetRecovery();
	XwFlightCamera_ResetChase();
	XwReferenceMotion_Clear();
}

bool XwFlightTiming_IsUnlocked(void) {
	return session_active && timing.update_rate == XW_FLIGHT_UPDATE_RATE_UNLOCKED;
}

uint16_t XwFlightTiming_StepTicks(void) { return XwFlightTiming_IsUnlocked() ? 1 : timing.reference_ticks; }

uint16_t XwFlightTiming_ReferenceTicks(void) { return timing.reference_ticks; }

void XwFlightTiming_BeginAdvance(uint16_t elapsed) {
	XwFlightTiming_EndAdvance();
	if (!session_active || !elapsed)
		return;
	timing.advance_active = true;
	timing.elapsed_ticks = elapsed;
	++timing.advance_serial;
	if (!XwFlightTiming_IsUnlocked()) {
		timing.reference_due = true;
		return;
	}
	uint32_t total = (uint32_t)timing.phase + elapsed;
	timing.phase = (uint16_t)(total % timing.reference_ticks);
	timing.reference_due = total >= timing.reference_ticks;
	if (timing.reference_due) {
		uint32_t dropped = total / timing.reference_ticks - 1;
		timing.dropped_periods +=
			dropped > UINT32_MAX - timing.dropped_periods ? UINT32_MAX - timing.dropped_periods : dropped;
	}
	/* Bound overdue work to one decision point, and report only sustained overload. */
	if (timing.dropped_periods - reported_periods >= 256) {
		Aeron_LogWarn("xw.flight.timing", "dropped %u overdue reference periods", timing.dropped_periods);
		reported_periods = timing.dropped_periods;
	}
}

void XwFlightTiming_MovementCompleted(void) {
	if (!timing.advance_active || timing.movement_completed)
		return;
	timing.completed_movement_ticks += timing.elapsed_ticks;
	timing.movement_completed = true;
}

void XwFlightTiming_EndAdvance(void) {
	timing.advance_active = timing.reference_due = timing.movement_completed = false;
	timing.elapsed_ticks = 0;
}

bool XwFlightTiming_ReferenceDue(void) {
	return !XwFlightTiming_IsUnlocked() || (timing.advance_active && timing.reference_due);
}

uint16_t XwFlightTiming_ReferenceElapsed(void) {
	return !XwFlightTiming_IsUnlocked()    ? g_elapsedTicks
		   : XwFlightTiming_ReferenceDue() ? timing.reference_ticks
										   : 0;
}

uint16_t XwFlightTiming_MissionPoseElapsed(unsigned slot) {
	if (slot >= MISSION_OBJECT_COUNT)
		return 0;
	if (!XwFlightTiming_IsUnlocked())
		return g_elapsedTicks;
	if (!XwFlightTiming_ReferenceDue() || timing.mission_pose_serial[slot] == timing.advance_serial)
		return 0;
	timing.mission_pose_serial[slot] = timing.advance_serial;
	return timing.reference_ticks;
}

uint64_t XwFlightTiming_AdvanceSerial(void) { return timing.advance_serial; }

uint64_t XwFlightTiming_CompletedMovementTicks(void) { return timing.completed_movement_ticks; }

XwFlightClock XwFlightTiming_EnterReference(void) {
	XwFlightClock saved = { g_elapsedTicks, g_simStepScale };
	if (XwFlightTiming_IsUnlocked()) {
		g_elapsedTicks = timing.reference_ticks;
		g_simStepScale = XW_SIMULATION_TICKS_PER_SECOND / timing.reference_ticks;
	}
	return saved;
}

void XwFlightTiming_RestoreClock(XwFlightClock clock) {
	g_elapsedTicks = clock.elapsed;
	g_simStepScale = clock.scale;
}

void XwFlightTiming_AnimationEvent(void) {
	if (!timing.advance_active || !timing.movement_completed)
		return;
	++timing.animation_serial;
	timing.animation_time_ticks = timing.completed_movement_ticks;
}

uint64_t XwFlightTiming_AnimationSerial(void) { return timing.animation_serial; }

uint64_t XwFlightTiming_AnimationTime(void) { return timing.animation_time_ticks; }

void XwFlightTiming_Save(XwFlightTimingState* out) {
	if (out)
		*out = timing;
}

bool XwFlightTiming_Check(const XwFlightTimingState* state, XwGameVersion version, XwFlightUpdateRate rate) {
	const XwFlightProfile* profile = XwProfile_Flight(version);
	if (!state || !profile ||
		(state->update_rate != XW_FLIGHT_UPDATE_RATE_NATIVE &&
		 state->update_rate != XW_FLIGHT_UPDATE_RATE_UNLOCKED) ||
		state->update_rate != rate || state->reference_ticks != profile->minimum_frame_ticks ||
		!state->reference_ticks || state->phase >= state->reference_ticks ||
		state->advance_active != (state->elapsed_ticks != 0) ||
		(!state->advance_active && (state->reference_due || state->movement_completed)) ||
		(state->advance_active && !state->advance_serial) ||
		state->animation_serial > state->advance_serial ||
		state->animation_time_ticks > state->completed_movement_ticks ||
		(state->update_rate == XW_FLIGHT_UPDATE_RATE_NATIVE &&
		 (state->phase || state->dropped_periods || state->reference_due != state->advance_active)))
		return false;
	for (unsigned slot = 0; slot < MISSION_OBJECT_COUNT; ++slot)
		if (state->mission_pose_serial[slot] > state->advance_serial ||
			(state->update_rate == XW_FLIGHT_UPDATE_RATE_NATIVE && state->mission_pose_serial[slot]))
			return false;
	return true;
}

bool XwFlightTiming_Restore(const XwFlightTimingState* state) {
	if (!session_active || !XwProfile_HasActiveFlight() ||
		!XwFlightTiming_Check(state, XwProfile_ActiveFlight()->version, XwProfile_ActiveFlightRate()))
		return false;
	timing = *state;
	reported_periods = timing.dropped_periods;
	return true;
}
