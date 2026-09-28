/* Mapped steering and keys use recovered controls; absolute throttle travels with each input record. */
#include "xw_runtime/input/flight_controls.h"
#include "xw/flight/feinput.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/anim.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/object.h"
#include "xw/flight/player/user.h"
#include "xw_runtime/input/controller_mapping.h"
#include "xw_runtime/input/input_bridge.h"
#include "xw_runtime/input/keyboard_mapping.h"
#include "xw_runtime/input/mouse_flight.h"
#include "xw_runtime/runtime/port.h"
#include "xw_runtime/timing/player_timing.h"

static struct {
	bool valid;
	uint16_t position, object;
	uint32_t generation;
	const CraftData* craft;
} g_throttleBaseline;

int16_t g_xwInputRoll;

void XwFlightControls_ResetThrottle(void) { g_throttleBaseline.valid = false; }

void XwFlightControls_Reset(void) {
	XwFlightControls_ResetThrottle();
	XwPlayerTiming_ResetControls();
	g_controlMask = 0;
	g_actionKey = g_currentActionKey = g_keyMods = g_flightKeyMods = g_mouseButtons = 0;
	g_flightMouseInputEnabled = 0;
	g_flightJoystickX = g_flightJoystickY = g_flightMouseDeltaX = g_flightMouseDeltaY = 0;
	g_scaledInputYaw = g_scaledInputPitch = 0;
	g_xwInputRoll = 0;
}

void XwFlightControls_CollectCommands(void) {
	if (!XwInput_GameplayActive())
		return;
	uint16_t key;
	while ((key = XwKeyboardMapping_ReadKey()) != 0)
		XwInput_QueueFlightKey(key);
	while ((key = XwControllerMapping_ReadKey()) != 0)
		XwInput_QueueFlightKey(key);
	while ((key = XwMouseFlight_ReadKey()) != 0)
		XwInput_QueueFlightKey(key);
}

bool XwFlightControls_ThrottleEligible(void) {
	if (g_missionRuntimeState.flightExitRequested || g_hyperspaceflag || g_playerFlightState.hudSuppressed ||
		g_playerFlightState.objectIndex >= XW_CRAFT_OBJECT_COUNT)
		return false;
	const ObjectRecord* object = &g_objectTable[g_playerFlightState.objectIndex];
	return object == g_playerFlightState.object && object->objectType != XW_OBJ_NONE &&
		   object->familyId == XW_OBJECT_FAMILY_CRAFT && g_playerFlightState.craft &&
		   object->instanceData == g_playerFlightState.craft;
}

static bool LocalThrottleEligible(void) {
	return XwInput_ReconcileKeyboard() == XW_KEYBOARD_GAMEPLAY && !XwPort_SettingsOpen() &&
		   XwFlightControls_ThrottleEligible();
}

void XwFlightControls_UpdateThrottleContext(void) {
	if (!LocalThrottleEligible())
		XwFlightControls_ResetThrottle();
}

void XwFlightControls_SampleThrottle(XwFlightInput* record) {
	uint16_t position;
	uint32_t generation;
	record->flags = 0;
	record->throttle = 0;
	if (!LocalThrottleEligible() || !XwControllerMapping_ThrottleSample(&position, &generation)) {
		XwFlightControls_ResetThrottle();
		return;
	}
	uint16_t object = g_playerFlightState.objectIndex;
	const CraftData* craft = g_playerFlightState.craft;
	if (!g_throttleBaseline.valid || g_throttleBaseline.generation != generation ||
		g_throttleBaseline.object != object || g_throttleBaseline.craft != craft) {
		g_throttleBaseline.valid = true;
		g_throttleBaseline.position = position;
		g_throttleBaseline.generation = generation;
		g_throttleBaseline.object = object;
		g_throttleBaseline.craft = craft;
		return;
	}
	int delta = (int)position - g_throttleBaseline.position;
	if (delta < 0)
		delta = -delta;
	/* 0.1% jitter threshold; small movements accumulate against the last accepted position. */
	if (delta && (delta >= 66 || position == 0 || position == UINT16_MAX)) {
		g_throttleBaseline.position = position;
		record->flags = XW_INPUT_THROTTLE_PRESENT;
		record->throttle = position;
	}
}

void XwFlightControls_ApplyThrottle(const XwFlightInput* input) {
	if ((input->flags & XW_INPUT_THROTTLE_PRESENT) && XwFlightControls_ThrottleEligible())
		for (unsigned engine = 0; engine < g_playerFlightState.engineCount; ++engine)
			g_playerFlightState.craft->engineThrottle[engine] = input->throttle;
}

uint16_t XwFlightControls_ReadLocal(void) {
	XwKeyboardRoute route = XwInput_ReconcileKeyboard();
	uint16_t command = XwInput_ReadFlightKey();
	int yaw = 0, pitch = 0;
	uint16_t buttons = 0;
	g_xwInputRoll = 0;
	g_flightMouseInputEnabled = 0;
	g_flightMouseDeltaX = g_flightMouseDeltaY = g_flightMouseX = g_flightMouseY = 0;
	g_mouseButtons = 0;
	if (route == XW_KEYBOARD_GAMEPLAY) {
		yaw = XwControllerMapping_Axis(XW_INPUT_AXIS_YAW);
		pitch = XwControllerMapping_Axis(XW_INPUT_AXIS_PITCH);
		g_xwInputRoll = (int16_t)(XwControllerMapping_Axis(XW_INPUT_AXIS_ROLL) * FEINPUT_JOYSTICK_YAW_SCALE);
		buttons = XwKeyboardMapping_ReadButtons() | XwControllerMapping_Modifiers();
		XwMouseFlightSample mouse;
		if (XwMouseFlight_Sample(&mouse)) {
			g_mouseButtons = XwMouseFlight_ReadButtons();
			if (mouse.mode == XW_MOUSE_CLASSIC) {
				/* Recovered scaling preserves DOS authority and per-axis joystick fallback. */
				g_flightMouseInputEnabled = 1;
				g_flightMouseDeltaX = (int16_t)mouse.x;
				g_flightMouseDeltaY = (int16_t)mouse.y;
			} else {
				if (mouse.x)
					yaw = mouse.x;
				if (mouse.y)
					pitch = mouse.y;
			}
			buttons |= g_mouseButtons;
		}
	}
	g_flightJoystickX = (int16_t)yaw;
	g_flightJoystickY = (int16_t)pitch;
	g_keyMods = buttons;
	g_controlMask = buttons;
	g_actionKey = command;
	return command;
}
