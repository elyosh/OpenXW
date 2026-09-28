#include "xw/flight/flight_input.h"

#ifdef XW_MODERN
#include "xw_runtime/input/input_bridge.h"
#endif

#include "xw/flight/flight.h"
#include "xw/input/dinput.h"
#include "xw/landru_config.h"
#include "xw_runtime/platform/main_window.h"

#include <landru/error.h>

// GLOBAL: XW 0x566894
int g_flightConfDirectInput = 0;

// GLOBAL: XW 0x567D40
int g_windowVirtualKeyDown[FLIGHT_VIRTUAL_KEY_COUNT] = { 0 };

// GLOBAL: XW 0x568144
uint8_t g_lastKeyCode = 0;

// GLOBAL: XW 0x568684
unsigned int g_lastReleasedVirtualKey = 0;

// GLOBAL: XW 0x56868C
int g_keyReady = 0;

// FUNCTION: XW 0x49E550
int j_FlightInput_HasKeyReady(void) { return FlightInput_HasKeyReady(); }

// FUNCTION: XW 0x4AC490
int FlightInput_HasKeyReady(void) {
#ifdef XW_MODERN
	return XwInput_FlightKeyPending();
#else
	XwWin32Message message;
	if (g_flightConfDirectInput != 0)
		return DInput_HasKeyReady();
	if (g_windowActive == 0 || PeekMessageA(&message, NULL, 0, 0, XW_WIN32_MESSAGE_NO_REMOVE)) {
		if (!GetMessageA(&message, NULL, 0, 0)) {
			g_quitRequested = 1;
			xerror_Set_Landru_Exit(0);
			return g_keyReady;
		}
		TranslateMessage(&message);
		DispatchMessageA(&message);
	}
	return g_keyReady;
#endif
}

// FUNCTION: XW 0x4AC520
uint8_t FlightInput_GetNextKey(void) {
#ifdef XW_MODERN
	return (uint8_t)XwInput_ReadFlightKey();
#else
	XwWin32Message message;
	if (g_flightConfDirectInput != 0) {
		return DInput_GetKey();
	}
	for (;;) {
		do {
			if (g_keyReady != 0) {
				g_keyReady = 0;
				return g_lastKeyCode;
			}
		} while (g_windowActive != 0 && !PeekMessageA(&message, NULL, 0, 0, XW_WIN32_MESSAGE_NO_REMOVE));
		if (!GetMessageA(&message, NULL, 0, 0)) {
			break;
		}
		TranslateMessage(&message);
		DispatchMessageA(&message);
	}
	g_quitRequested = 1;
	xerror_Set_Landru_Exit(0);
	return g_lastKeyCode;
#endif
}
