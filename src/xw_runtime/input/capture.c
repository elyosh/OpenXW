/* Release barriers and relative-mouse ownership follow OpenXvT. */
#include "xw_runtime/input/capture.h"
#include "xw_runtime/config/config.h"
#include "xw_runtime/input/input_bridge.h"
#include "xw_runtime/input/keyboard_mapping.h"
#include "xw_runtime/input/mouse_flight.h"
#include "xw_runtime/runtime/flight_sim.h"
#include <aeron/aeron.h>
#include <aeron/compat/host.h>
#include <string.h>

static bool captured, renderer_tab, blocked_keys[AERON_KEY_COUNT];
static uint32_t blocked_mouse;
static uint64_t mouse_resume_frame = UINT64_MAX;
static bool mouse_released, mouse_failed, mouse_session;
static int mouse_enabled, mouse_sensitivity, mouse_invert;
static XwMouseFlightMode mouse_mode;

static void ApplySuppression(void) {
	for (int key = 0; key < AERON_KEY_COUNT; ++key)
		AeronCompat_SetKeySuppressed(key,
									 captured || blocked_keys[key] || (renderer_tab && key == AERON_KEY_TAB));
}

bool XwInput_KeyBlocked(int key) {
	return (unsigned)key >= AERON_KEY_COUNT || captured || blocked_keys[key] ||
		   (renderer_tab && key == AERON_KEY_TAB);
}

void XwInput_SuppressRendererTab(bool suppressed) {
	renderer_tab = suppressed;
	AeronCompat_SetKeySuppressed(AERON_KEY_TAB, XwInput_KeyBlocked(AERON_KEY_TAB));
}

void XwInput_SuppressKey(int key) {
	if ((unsigned)key < AERON_KEY_COUNT) {
		blocked_keys[key] = true;
		AeronCompat_SetKeySuppressed(key, 1);
	}
}

void XwInput_BlockHeldKeys(void) {
	const AeronInputSnapshot* input = Aeron_InputSnapshot();
	if (input) {
		for (int key = 0; key < AERON_KEY_COUNT; ++key)
			blocked_keys[key] |= input->key_down[key] != 0;
		blocked_mouse |= input->mouse.buttons | input->mouse.pressed_buttons;
		mouse_resume_frame = input->frame_id;
	}
	ApplySuppression();
}

void XwInput_BeginCaptureFrame(const AeronInputSnapshot* input, bool capture) {
	if (input) {
		for (int key = 0; key < AERON_KEY_COUNT; ++key)
			if (!input->key_down[key] && !input->key_released[key])
				blocked_keys[key] = false;
		blocked_mouse &= input->mouse.buttons | input->mouse.released_buttons;
	}
	if (capture != captured) {
		captured = capture;
		XwInput_BlockHeldKeys();
		XwInput_ClearCommands();
		XwMouseFlight_Reset();
		if (capture)
			Aeron_SetRelativeMouseMode(0);
	}
	ApplySuppression();
	if (capture)
		XwInput_FlushRawKeyboard();
}

bool XwInput_IsCaptured(void) { return captured; }

bool XwInput_MouseMotionAllowed(void) {
	const AeronInputSnapshot* input = Aeron_InputSnapshot();
	return !captured && input && input->has_focus && input->frame_id != mouse_resume_frame;
}

void XwInput_BlockMouseButtons(uint32_t buttons) { blocked_mouse |= buttons; }

uint32_t XwInput_FilterMouseButtons(uint32_t buttons) { return captured ? 0 : buttons & ~blocked_mouse; }

bool XwInput_MouseFlightAllowed(void) {
	return mouse_enabled && XwInput_GameplayActive() && !XwFlightSim_IsPaused() &&
		   XwInput_MouseMotionAllowed() && !mouse_released && !mouse_failed;
}

void XwInput_UpdateMouseCapture(const AeronInputSnapshot* input, int32_t delta_us) {
	const XwSettings* settings = XwConfig_Settings();
	if (!settings)
		return;
	if (mouse_enabled != settings->mouse_flight || mouse_sensitivity != settings->mouse_sensitivity ||
		mouse_invert != settings->mouse_invert_y || mouse_mode != settings->mouse_mode) {
		mouse_enabled = settings->mouse_flight;
		mouse_sensitivity = settings->mouse_sensitivity;
		mouse_invert = settings->mouse_invert_y;
		mouse_mode = settings->mouse_mode;
		XwMouseFlight_SetOptions(mouse_mode, mouse_sensitivity, mouse_invert);
		mouse_failed = false;
		blocked_mouse |= input ? input->mouse.buttons : 0;
		mouse_resume_frame = input ? input->frame_id : UINT64_MAX;
	}
	bool session = XwFlightSim_IsActive();
	if (session != mouse_session) {
		mouse_session = session;
		mouse_released = mouse_failed = false;
		XwMouseFlight_Reset();
	}
	if (session && mouse_enabled && input && input->has_focus && !captured && XwInput_GameplayActive()) {
		int key = XwKeyboardMapping_Trigger(input, XW_KEYBOARD_SHORTCUT_MOUSE);
		bool chord = key >= 0 && !XwInput_KeyBlocked(key);
		bool click = mouse_released && input->mouse.inside_content && input->mouse.pressed_buttons;
		if (chord || click) {
			mouse_released = chord ? !mouse_released : false;
			mouse_failed = false;
			if (chord)
				XwInput_SuppressKey(key);
			blocked_mouse |= input->mouse.buttons | input->mouse.pressed_buttons;
			mouse_resume_frame = input->frame_id;
			XwMouseFlight_Reset();
		}
	}
	bool relative = XwInput_MouseFlightAllowed();
	if (relative != (Aeron_RelativeMouseMode() != 0)) {
		if (!Aeron_SetRelativeMouseMode(relative) && relative) {
			mouse_failed = mouse_released = true;
			Aeron_LogError("xw.input", "Cannot capture flight mouse; retry with Ctrl+Alt+M");
		}
		blocked_mouse |= input ? input->mouse.buttons : 0;
		XwMouseFlight_Reset();
	}
	XwMouseFlight_Pump(delta_us);
}

void XwInput_ResetCapture(void) {
	captured = false;
	memset(blocked_keys, 0, sizeof blocked_keys);
	blocked_mouse = 0;
	mouse_resume_frame = UINT64_MAX;
	mouse_released = mouse_failed = mouse_session = false;
	mouse_enabled = mouse_sensitivity = mouse_invert = 0;
	mouse_mode = XW_MOUSE_VIRTUAL_STICK;
	ApplySuppression();
	XwMouseFlight_Reset();
	Aeron_SetRelativeMouseMode(0);
}
