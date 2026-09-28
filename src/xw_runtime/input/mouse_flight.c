/* OpenXvT virtual stick and OpenTIE relative motion; X-Wing owns button tap/hold timing. */
#include "xw_runtime/input/mouse_flight.h"
#include "xw/flight/feinput.h"
#include "xw_runtime/input/capture.h"
#include "xw_runtime/timing/host_clock.h"
#include <aeron/aeron.h>
#include <math.h>

static struct {
	XwMouseFlightMode mode;
	int sensitivity, invert_y;
	uint64_t frame, drain_time, interval_us;
	float pending_x, pending_y, stick_x, stick_y;
	float fraction_x, fraction_y;
	uint16_t buttons, pending_buttons, observed_buttons, keys[16];
	unsigned read, write;
} mouse = { .sensitivity = 5, .frame = UINT64_MAX };

/* OpenTIE's host-unit multiplier and DOS94 four-tick reference interval. */
enum { CLASSIC_MOTION_MULTIPLIER = 4, CLASSIC_REFERENCE_US = 16000, CLASSIC_MAX_INTERVAL_US = 250000 };

static const float sensitivity_scale[] = { .0625f, .125f, .25f, .5f, 1, 2, 4, 8, 16 };

void XwMouseFlight_Reset(void) {
	mouse.pending_x = mouse.pending_y = mouse.stick_x = mouse.stick_y = 0;
	mouse.buttons = mouse.pending_buttons = mouse.observed_buttons = 0;
	mouse.read = mouse.write = 0;
	mouse.drain_time = 0;
	mouse.interval_us = 0;
	mouse.fraction_x = mouse.fraction_y = 0;
}

void XwMouseFlight_SetOptions(XwMouseFlightMode mode, int sensitivity, int invert_y) {
	mouse.mode = mode;
	mouse.sensitivity = sensitivity;
	mouse.invert_y = invert_y;
	XwMouseFlight_Reset();
}

static void Queue(uint16_t key) {
	unsigned next = (mouse.write + 1) % 16;
	if (next == mouse.read) {
		Aeron_LogWarn("xw.input", "Mouse command queue is full");
		return;
	}
	mouse.keys[mouse.write] = key;
	mouse.write = next;
}

void XwMouseFlight_Pump(int32_t delta_us) {
	const AeronInputSnapshot* input = Aeron_InputSnapshot();
	if (!input || !XwInput_MouseFlightAllowed() || !Aeron_RelativeMouseMode()) {
		XwMouseFlight_Reset();
		return;
	}
	if (input->frame_id == mouse.frame)
		return;
	mouse.frame = input->frame_id;
	mouse.pending_buttons &= (uint16_t)~mouse.observed_buttons;
	mouse.observed_buttons = 0;
	if (mouse.mode == XW_MOUSE_CLASSIC) {
		/* OpenTIE checks each host interval, then collects motion even after a reset. */
		if (delta_us <= 0 || delta_us > CLASSIC_MAX_INTERVAL_US) {
			mouse.pending_x = mouse.pending_y = 0;
			mouse.fraction_x = mouse.fraction_y = 0;
			mouse.interval_us = 0;
		} else
			mouse.interval_us += (uint32_t)delta_us;
		float gain = CLASSIC_MOTION_MULTIPLIER * sensitivity_scale[mouse.sensitivity - 1];
		mouse.pending_x += input->mouse.relative_x * gain;
		mouse.pending_y += input->mouse.relative_y * gain * (mouse.invert_y ? 1 : -1);
	} else {
		mouse.pending_x += input->mouse.relative_x;
		mouse.pending_y += input->mouse.relative_y;
	}
	uint32_t down = XwInput_FilterMouseButtons(input->mouse.buttons);
	uint32_t pressed = XwInput_FilterMouseButtons(input->mouse.pressed_buttons);
	mouse.buttons = ((down & AERON_MOUSE_BUTTON_LEFT) ? 1 : 0) | ((down & AERON_MOUSE_BUTTON_RIGHT) ? 2 : 0);
	mouse.pending_buttons |=
		((pressed & AERON_MOUSE_BUTTON_LEFT) ? 1 : 0) | ((pressed & AERON_MOUSE_BUTTON_RIGHT) ? 2 : 0);
	if (pressed & AERON_MOUSE_BUTTON_MIDDLE)
		Queue('r');
	if (pressed & AERON_MOUSE_BUTTON_X1)
		Queue('.');
}

static float Clamp(float value) { return fmaxf(-127, fminf(127, value)); }

static int ClassicAxis(float delta, float gain, float* fraction, int limit) {
	float value = delta * gain + *fraction;
	if (!isfinite(value)) {
		*fraction = 0;
		return 0;
	}
	float whole;
	*fraction = modff(value, &whole);
	/* Only sub-count carry survives saturation; clamp before converting to an integer. */
	return (int)fmaxf((float)-limit, fminf((float)limit, whole));
}

int XwMouseFlight_Sample(XwMouseFlightSample* sample) {
	if (!XwInput_MouseFlightAllowed() || !Aeron_RelativeMouseMode()) {
		XwMouseFlight_Reset();
		return 0;
	}
	*sample = (XwMouseFlightSample) { .mode = mouse.mode };
	if (mouse.mode == XW_MOUSE_CLASSIC) {
		if (mouse.interval_us) {
			float gain = (float)CLASSIC_REFERENCE_US / (float)mouse.interval_us;
			sample->x = ClassicAxis(mouse.pending_x, gain, &mouse.fraction_x, FEINPUT_MOUSE_X_LIMIT - 1);
			sample->y = ClassicAxis(mouse.pending_y, gain, &mouse.fraction_y, FEINPUT_MOUSE_Y_LIMIT - 1);
		}
		mouse.pending_x = mouse.pending_y = 0;
		mouse.interval_us = 0;
		return 1;
	}
	uint64_t now = XwTime_GetElapsedUs();
	if (mouse.drain_time && now - mouse.drain_time <= 100000) {
		float gain = 127.0f / 256.0f * sensitivity_scale[mouse.sensitivity - 1];
		mouse.stick_x = Clamp(mouse.stick_x + mouse.pending_x * gain);
		mouse.stick_y = Clamp(mouse.stick_y + mouse.pending_y * gain * (mouse.invert_y ? 1 : -1));
	}
	mouse.drain_time = now;
	mouse.pending_x = mouse.pending_y = 0;
	sample->x = (int)floorf(mouse.stick_x + .5f);
	sample->y = (int)floorf(mouse.stick_y + .5f);
	return 1;
}

uint16_t XwMouseFlight_ReadButtons(void) {
	mouse.observed_buttons |= mouse.pending_buttons;
	return mouse.buttons | mouse.pending_buttons;
}

int XwMouseFlight_GetHudMarker(int* yaw, int* pitch) {
	if (mouse.mode != XW_MOUSE_VIRTUAL_STICK || !XwInput_MouseFlightAllowed() || !Aeron_RelativeMouseMode() ||
		!mouse.drain_time)
		return 0;
	*yaw = (int)floorf(mouse.stick_x + .5f);
	*pitch = (int)floorf(mouse.stick_y + .5f);
	return 1;
}

uint16_t XwMouseFlight_ReadKey(void) {
	if (mouse.read == mouse.write)
		return 0;
	uint16_t key = mouse.keys[mouse.read];
	mouse.read = (mouse.read + 1) % 16;
	return key;
}
