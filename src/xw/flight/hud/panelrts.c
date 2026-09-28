#include "xw/flight/hud/panelrts.h"

#ifdef XW_MODERN
#include "xw_runtime/runtime/flight_dispatch.h"
#endif

#include "xw/flight/feinput.h"
#include "xw/flight/hud/festring.h"
#include "xw/flight/hud/panel.h"
#include "xw/flight/replay/replay.h"

#include <stdlib.h>

// GLOBAL: XW 0x4C9840
const uint16_t g_flightTextDecimalDivisors[PANELRTS_DECIMAL_DIVISOR_COUNT] = { 1, 1, 10, 100, 1000, 10000 };

// FUNCTION: XW 0x41C6D0
void panelrts_setnewpilotview(uint16_t hudViewState) {
	if (g_hudCockpitResourceDescriptors[hudViewState].enabled != 0 &&
		g_flightCamera.hudStateLive != hudViewState) {
		g_flightCamera.hudStateLive = hudViewState;
		g_flightCamera.hudStateMirror = hudViewState;
		if (g_replayviewmode == 0)
#ifdef XW_MODERN
			XwFlightMode_ApplyPanelView(hudViewState);
#else
			panel_dosetnewpilotview(hudViewState);
#endif
	}
}

// FUNCTION: XW 0x41C730
void panelrts_outnum(uint16_t value, uint16_t width, uint16_t minDigits) {
	uint16_t remainingValue = value;
	if (remainingValue == PANELRTS_NUMBER_UNAVAILABLE) {
		uint16_t savedShadowEnabled = g_flightTextShadowEnabled;
		uint16_t savedTextColor = g_flightTextColorIndex;
		uint16_t zeroIndex;
		g_flightTextShadowEnabled = 0;
		festring_settextcolor(FLIGHT_TEXT_ENCODED_COLOR_BASE);
		for (zeroIndex = 0; zeroIndex < width; ++zeroIndex) {
			g_flightDrawCharFn('0');
		}
		g_flightTextShadowEnabled = savedShadowEnabled;
		g_flightTextColorIndex = savedTextColor;
	} else {
		uint16_t digitsRemaining;
		uint16_t hasSignificantDigit = 0;
		for (digitsRemaining = width; digitsRemaining > 0; --digitsRemaining) {
			uint16_t divisor = g_flightTextDecimalDivisors[digitsRemaining];
			int digit = remainingValue / divisor;
			int digitChar;
			remainingValue -= digit * divisor;
			if (hasSignificantDigit != 0 || digitsRemaining <= minDigits || (uint16_t)digit != 0) {
				hasSignificantDigit = 1;
				if ((uint16_t)digit > PANELRTS_DECIMAL_MAX_DIGIT) {
					digit = PANELRTS_DECIMAL_MAX_DIGIT;
				}
				digitChar = digit + '0';
			} else {
				digitChar = ' ';
			}
			g_flightDrawCharFn((char)digitChar);
		}
	}
}
