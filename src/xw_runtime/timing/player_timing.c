/* OpenXvT's single-player control carry with TIE's parameterized slew interval. */
#include "xw_runtime/timing/player_timing.h"
#include "xw/flight/feinput.h"
#include "xw/flight/object/anim.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/player/user.h"
#include "xw/flight/replay/replay.h"
#include "xw/flight/xw.h"
#include "xw/math/math2.h"
#include "xw/render/flight_view.h"
#include "xw_runtime/timing/flight_timing.h"
#include <limits.h>

typedef XwPlayerTimingState PlayerTiming;
typedef XwRecoveryTimingState RecoveryTiming;
static PlayerTiming timing;
static RecoveryTiming recovery;
static bool restored_controls;

void XwPlayerTiming_Save(XwPlayerTimingState* out, XwRecoveryTimingState* pose) {
	*out = timing;
	*pose = recovery;
}

void XwPlayerTiming_Restore(const XwPlayerTimingState* state, const XwRecoveryTimingState* pose) {
	timing = *state;
	recovery = *pose;
	XwPlayerTiming_ResumeControls();
}

void XwPlayerTiming_ResumeControls(void) { restored_controls = true; }

void XwPlayerTiming_RestoreRoll(const XwPlayerTimingState* state) {
	if (!timing.valid)
		XwPlayerTiming_BeginControls();
	timing.analog_roll = state->analog_roll;
	for (unsigned c = XW_PLAYER_SLEW_ANALOG_ROLL; c < XW_PLAYER_TIMING_CHANNELS; ++c) {
		timing.remainder[c] = state->remainder[c];
		timing.direction[c] = state->direction[c];
	}
	XwPlayerTiming_ResumeControls();
}

/* A viewer's physical controls must not reset the recorded player's fractions. */
void XwPlayerTiming_ResetControls(void) {
	if (!restored_controls && !g_replayviewmode)
		XwPlayerTiming_Reset();
}

void XwPlayerTiming_Reset(void) {
	timing = (PlayerTiming) { 0 };
	restored_controls = false;
}

void XwPlayerTiming_ResetRecovery(void) {
	recovery = (RecoveryTiming) { 0 };
	/* Seed the first reference interval at world creation, before any high-rate movement. */
	if (XwFlightTiming_IsUnlocked() && g_playerFlightState.object && g_playerFlightState.object->objectType)
		XwPlayerTiming_RepositionObject(g_playerFlightState.objectIndex);
}

void XwPlayerTiming_ResetObject(unsigned slot) {
	if (slot == g_playerFlightState.objectIndex || slot == g_flightCamera.focusObjectRef ||
		(timing.valid && (slot == timing.slot || slot == timing.focus)))
		XwPlayerTiming_Reset();
	if (slot == recovery.slot)
		recovery.valid = false;
}

void XwPlayerTiming_RepositionObject(unsigned slot) {
	XwPlayerTiming_ResetObject(slot);
	if (!XwFlightTiming_IsUnlocked() || slot != g_playerFlightState.objectIndex)
		return;
	const ObjectRecord* object = g_playerFlightState.object;
	recovery.position[0] = object->worldX;
	recovery.position[1] = object->worldY;
	recovery.position[2] = object->worldZ;
	recovery.slot = (uint16_t)slot;
	recovery.valid = true;
}

bool XwPlayerTiming_RecordRecovery(int32_t position[3]) {
	const ObjectRecord* object = g_playerFlightState.object;
	if (!XwFlightTiming_IsUnlocked()) {
		position[0] = object->prevWorldX;
		position[1] = object->prevWorldY;
		position[2] = object->prevWorldZ;
		return true;
	}
	uint64_t serial = XwFlightTiming_AdvanceSerial();
	if (!XwFlightTiming_ReferenceDue() || recovery.serial == serial)
		return false;
	/* OpenXvT's first sample uses the original previous position; later samples span reference events. */
	if (!recovery.valid || recovery.slot != g_playerFlightState.objectIndex) {
		recovery.position[0] = object->prevWorldX;
		recovery.position[1] = object->prevWorldY;
		recovery.position[2] = object->prevWorldZ;
	}
	for (unsigned axis = 0; axis < 3; ++axis)
		position[axis] = recovery.position[axis];
	recovery.position[0] = object->worldX;
	recovery.position[1] = object->worldY;
	recovery.position[2] = object->worldZ;
	recovery.slot = g_playerFlightState.objectIndex;
	recovery.serial = serial;
	recovery.valid = true;
	return true;
}

void XwPlayerTiming_BeginControls(void) {
	restored_controls = false;
	bool manual = g_flightCamera.manualControlActive != 0;
	unsigned mode =
		(g_flightKeyMods & (manual ? USER_CAMERA_MODIFIER_MASK : USER_TURN_MODIFIER_MASK)) |
		((unsigned)manual << 4) | ((g_flightCamera.externalViewActive != 0) << 5) |
		(((g_playerFlightState.craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_ATTITUDE) == 0) << 6) |
		((g_hyperspaceflag != 0) << 7) | ((g_playerFlightState.hudSuppressed != 0) << 8);
	if (!timing.valid || timing.slot != g_playerFlightState.objectIndex ||
		timing.craft_type != g_playerFlightState.craftTypeIndex ||
		((timing.mode ^ mode) & ~USER_TURN_MODIFIER_MASK) || timing.focus != g_flightCamera.focusObjectRef)
		XwPlayerTiming_Reset();
	else if (timing.mode != mode) {
		/* The button's yaw/roll latch must not reset the independent stick. */
		for (unsigned c = 0; c < XW_PLAYER_BASE_TIMING_CHANNELS; ++c) {
			timing.remainder[c] = 0;
			timing.direction[c] = 0;
		}
	}
	timing.valid = true;
	timing.slot = g_playerFlightState.objectIndex;
	timing.craft_type = g_playerFlightState.craftTypeIndex;
	timing.focus = g_flightCamera.focusObjectRef;
	timing.mode = mode;
}

static void Clear(unsigned channel) {
	timing.remainder[channel] = 0;
	timing.direction[channel] = 0;
}

static int Scale(unsigned channel, int value, unsigned divisor) {
	int direction = (value > 0) - (value < 0);
	if (timing.direction[channel] != direction) {
		Clear(channel);
		timing.direction[channel] = direction;
	}
	int64_t numerator = (int64_t)value * g_elapsedTicks + timing.remainder[channel];
	timing.remainder[channel] = numerator % divisor;
	int64_t result = numerator / divisor;
	return result < INT_MIN ? INT_MIN : result > INT_MAX ? INT_MAX : (int)result;
}

int XwPlayerTiming_Scale(unsigned channel, int value) {
	return Scale(channel, value, XW_SIMULATION_TICKS_PER_SECOND);
}

int16_t XwPlayerTiming_Slew(unsigned channel, int16_t current, int16_t target) {
	int16_t difference = (int16_t)(target - current);
	int magnitude = difference < 0 ? -(int)difference : difference;
	if (magnitude < USER_SMOOTHING_THRESHOLD) {
		Clear(channel);
		return target;
	}
	unsigned reference = XwFlightTiming_ReferenceTicks();
	unsigned rate = XwFlightTiming_IsUnlocked() ? XW_SIMULATION_TICKS_PER_SECOND / reference : g_simStepScale;
	int step = magnitude;
	if (rate > USER_SMOOTHING_SCALE) {
		step /= rate;
		if (!step)
			step = 1;
		step *= USER_SMOOTHING_SCALE;
	}
	step = difference < 0 ? -step : step;
	if (XwFlightTiming_IsUnlocked())
		step = Scale(channel, step, reference);
	if (step >= magnitude || step <= -magnitude) {
		Clear(channel);
		return target;
	}
	return (int16_t)(current + step);
}

/* OpenTIE's third steering channel; X-Wing retains its signed craft-gain arithmetic. */
int16_t XwPlayerTiming_RollStep(int16_t input) {
	bool working = (g_playerFlightState.craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_ATTITUDE) != 0;
	int target =
		(((uint16_t)math2_percentage(g_playerFlightState.craft->rollRateLimit, USER_ROLL_RATE_SCALE) >> 1) *
		 (int32_t)input) >>
		USER_TURN_INPUT_SHIFT;
	timing.analog_roll =
		XwPlayerTiming_Slew(XW_PLAYER_SLEW_ANALOG_ROLL, timing.analog_roll, working ? (int16_t)target : 0);
	int16_t step = XwFlightTiming_IsUnlocked()
					   ? (int16_t)XwPlayerTiming_Scale(XW_PLAYER_ANALOG_ROLL, timing.analog_roll)
					   : user_framerateadjust(timing.analog_roll);
	return working ? step : 0;
}

void XwPlayerTiming_ManualCamera(void) {
	g_flightCamera.hudAimY += (int16_t)XwPlayerTiming_Scale(XW_PLAYER_CAMERA_YAW, g_scaledInputYaw);
	g_flightCamera.hudAimX += (int16_t)XwPlayerTiming_Scale(XW_PLAYER_CAMERA_PITCH, g_scaledInputPitch);
	unsigned modifiers = g_flightKeyMods & USER_CAMERA_MODIFIER_MASK;
	if (modifiers != USER_CAMERA_ZOOM_IN && modifiers != USER_CAMERA_ZOOM_OUT) {
		g_flightCamera.movementStep = USER_CAMERA_STEP;
		Clear(XW_PLAYER_ZOOM_ACCELERATION);
		Clear(XW_PLAYER_ZOOM_DISTANCE);
		return;
	}
	int step = g_flightCamera.movementStep +
			   Scale(XW_PLAYER_ZOOM_ACCELERATION, USER_CAMERA_STEP, XwFlightTiming_ReferenceTicks());
	if (step >= USER_CAMERA_MAX_STEP) {
		step = USER_CAMERA_MAX_STEP;
		Clear(XW_PLAYER_ZOOM_ACCELERATION);
	}
	g_flightCamera.movementStep = (uint16_t)step;
	int distance =
		g_flightCamera.externalDistance +
		XwPlayerTiming_Scale(XW_PLAYER_ZOOM_DISTANCE, modifiers == USER_CAMERA_ZOOM_IN ? -step : step);
	if (distance <= USER_CAMERA_MIN_DISTANCE || distance >= USER_CAMERA_MAX_DISTANCE) {
		distance = distance <= USER_CAMERA_MIN_DISTANCE ? USER_CAMERA_MIN_DISTANCE : USER_CAMERA_MAX_DISTANCE;
		Clear(XW_PLAYER_ZOOM_DISTANCE);
	}
	g_flightCamera.externalDistance = (int16_t)distance;
}
