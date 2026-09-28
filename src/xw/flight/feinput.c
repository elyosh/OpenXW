#include "xw/flight/feinput.h"

#ifdef XW_MODERN
#include "xw_runtime/input/flight_controls.h"
#endif
#ifdef XW_MODERN
#include "xw_dos94/render/display.h"
#include "xw_runtime/runtime/flight_types.h"
#endif

#include "xw/flight/flight_display.h"
#include "xw/flight/flight_input.h"
#include "xw/input/joystick.h"
#include "xw/landru_config.h"
#include "xw/render/rtsrgb.h"
#include "xw/render/rtsvga2.h"
#include "xw/util/shared.h"

#include <landru/io.h>
#include <landru/joy.h>
#include <stdlib.h>

// GLOBAL: XW 0x4C5550
FlightRenderCallbacks g_flightRenderCallbackTables[FEINPUT_RENDER_MODE_COUNT] = {
	{ rtsvga2_initgraphVGA, rtsvga2_blankVGA, rtsvga2_unblankVGA, rtsvga2_buildpaletteVGA,
	  rtsvga2_savepaletteVGA, rtsvga2_restorepaletteVGA, rtsvga2_calcpositionVGA, rtsvga2_drawshapeVGA,
	  rtsvga2_outcharVGA, rtsvga2_clearwindowVGA, rtsvga2_fillboxVGA, rtsvga2_saveboxVGA,
	  rtsvga2_restoreboxVGA },
	{ rtsvga2_initgraphVGA, rtsvga2_blankVGA, rtsvga2_unblankVGA, rtsvga2_buildpaletteVGA,
	  rtsvga2_savepaletteVGA, rtsvga2_restorepaletteVGA, rtsvga2_calcpositionVGA, rtsvga2_drawshapeVGA,
	  rtsvga2_outchar32VGA, rtsvga2_clearwindowVGA, rtsvga2_fillboxVGA, rtsvga2_saveboxVGA,
	  rtsvga2_restoreboxVGA },
	{ rtsvga2_initgraphVGA, rtsvga2_blankVGA, rtsvga2_unblankVGA, rtsvga2_buildpaletteVGA,
	  rtsvga2_savepaletteVGA, rtsvga2_restorepaletteVGA, rtsvga2_calcpositionVGA, rtsrgb_drawshapeRGB,
	  rtsrgb_outcharRGB, rtsrgb_clearwindowRGB, rtsrgb_fillboxRGB, rtsrgb_saveboxRGB, rtsrgb_restoreboxRGB }
};

// GLOBAL: XW 0x4C5688
const uint8_t g_throttleKeyTable[FEINPUT_THROTTLE_BIN_COUNT] = { 0x5C, 0xDB, 0xDC, 0xDD, 0xDE, 0x5B,
																 0xDF, 0xE0, 0xE1, 0xE2, 0xE3, 0x5D,
																 0xE4, 0xE5, 0xE6, 0xE7, 0x08 };

// GLOBAL: XW 0x4CEF40
uint8_t g_flightGraphicsDetailPreset = FEINPUT_INITIAL_GRAPHICS_DETAIL;

// GLOBAL: XW 0x4F4154
unsigned int g_controlMask = 0;

// GLOBAL: XW 0x4F4158
int g_throttleSmoothed = 0;

// GLOBAL: XW 0x566924
int16_t g_joystickPollingSuppressed = 0;

// GLOBAL: XW 0x62AFF8
void (*g_flightDrawCharFn)(char ch) = NULL;

// GLOBAL: XW 0x62B000
int16_t g_flightMouseX = 0;

// GLOBAL: XW 0x62B002
int16_t g_flightMouseY = 0;

// GLOBAL: XW 0x62B004
int16_t g_flightMouseDeltaX = 0;

// GLOBAL: XW 0x62B006
int16_t g_flightMouseDeltaY = 0;

// GLOBAL: XW 0x62B010
uint8_t g_palettePackedMode = 0;

// GLOBAL: XW 0x62B4FA
uint16_t g_actionKey = 0;

/* The PE loader zero-initializes these callbacks before graphics setup. */
// GLOBAL: XW 0x62B504
void (*g_flightResetPaletteFn)(void) = NULL;

// GLOBAL: XW 0x62B50C
int (*g_flightComputePixelOffsetFn)(uint16_t x, uint16_t y) = NULL;

// GLOBAL: XW 0x62B5E0
void (*g_flightRenderTransitionHook)(void) = NULL;

// GLOBAL: XW 0x62B93C
void (*g_flightFillClipRectFn)(void) = NULL;

// GLOBAL: XW 0x62B94E
uint16_t g_currentActionKey = 0;

// GLOBAL: XW 0x62BABC
void (*g_flightInitLineBufferFn)(void) = NULL;

// GLOBAL: XW 0x62BAC8
void (*g_flightSetPaletteRangeFn)(const struct RgbTriplet* rgbTriples, uint16_t startIndex,
								  uint16_t count) = NULL;

// GLOBAL: XW 0x62BADC
void (*g_flightBlitSpriteFn)(const uint8_t* rleData, int16_t x, int16_t y, int16_t transparentColor,
							 int16_t mirror) = NULL;

// GLOBAL: XW 0x62BB04
void (*g_flightSaveScreenRectFn)(uint8_t* buffer, uint16_t xByteOffset, uint16_t y, uint16_t width,
								 uint16_t height) = NULL;

// GLOBAL: XW 0x62BB16
uint16_t g_mouseButtons = 0;

// GLOBAL: XW 0x62BC8C
int16_t g_flightMouseInputEnabled = 0;

// GLOBAL: XW 0x62BD9C
uint16_t g_flightKeyMods = 0;

// GLOBAL: XW 0x62C710
void (*g_flightFillRectClippedFn)(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) = NULL;

// GLOBAL: XW 0x62C714
int16_t g_scaledInputPitch = 0;

// GLOBAL: XW 0x62C920
int16_t g_scaledInputYaw = 0;

// GLOBAL: XW 0x62D11C
void (*g_flightSetPaletteFn)(const struct RgbTriplet* rgbTriples) = NULL;

// GLOBAL: XW 0x62D13A
int16_t g_flightJoystickX = 0;

// GLOBAL: XW 0x62D140
int16_t g_flightJoystickY = 0;

// GLOBAL: XW 0x637344
void (*g_flightRestoreScreenRectFn)(const uint8_t* buffer, uint16_t xByteOffset, uint16_t y, uint16_t width,
									uint16_t height) = NULL;

// GLOBAL: XW 0x637358
void (*g_flightGetPaletteFn)(struct RgbTriplet* dstPalette) = NULL;

// GLOBAL: XW 0x63735E
int16_t g_joystickDetectResultWord = 0;

// GLOBAL: XW 0x6377AE
int16_t g_joystickAvailable = 0;

// GLOBAL: XW 0x6377B0
uint16_t g_keyMods = 0;

// GLOBAL: XW 0x63BAAC
uint16_t g_flightViewportMode = 0;

// FUNCTION: XW 0x40B400
void feinput_checkinput(void) {
	uint16_t modifiers;
	uint16_t keyModifiers;
	g_currentActionKey = g_actionKey;
	keyModifiers = g_keyMods;
	modifiers = 0;
	g_flightKeyMods = modifiers;
	g_scaledInputPitch = 0;
	g_scaledInputYaw = 0;
	if (g_flightMouseInputEnabled != 0) {
		g_scaledInputYaw = g_flightMouseDeltaX * FEINPUT_MOUSE_YAW_SCALE;
		g_scaledInputPitch = g_flightMouseDeltaY * FEINPUT_MOUSE_PITCH_SCALE;
		modifiers = g_mouseButtons;
		modifiers |= keyModifiers;
		g_flightKeyMods = modifiers;
	}
	/* The original availability check always enables joystick fallback. */
	if (g_joystickAvailable | 1) {
		if (g_scaledInputYaw == 0) {
			g_scaledInputYaw = g_flightJoystickX * FEINPUT_JOYSTICK_YAW_SCALE;
		}
		if (g_scaledInputPitch == 0) {
			g_scaledInputPitch = g_flightJoystickY * FEINPUT_JOYSTICK_PITCH_SCALE;
		}
		g_flightKeyMods = modifiers | keyModifiers;
	}
}

// FUNCTION: XW 0x40B4C0
void feinput_degitterinput(void) {
	int16_t yaw = g_scaledInputYaw;
	int16_t pitch;
	if ((int16_t)((uint16_t)yaw < 0x8000u ? (uint16_t)yaw : (uint16_t)(~yaw + 1)) <= FEINPUT_YAW_DEAD_ZONE) {
		g_scaledInputYaw = 0;
	}
	pitch = g_scaledInputPitch;
	if ((int16_t)((uint16_t)pitch < 0x8000u ? (uint16_t)pitch : (uint16_t)(~pitch + 1)) <= FEINPUT_PITCH_DEAD_ZONE) {
		g_scaledInputPitch = 0;
	}
}

// FUNCTION: XW 0x40B510
void feinput_getinput(void) {
	int16_t yawMagnitude;
	int16_t pitchMagnitude;
	feinput_getrawinput();
	feinput_checkinput();
	yawMagnitude = g_scaledInputYaw;
	if ((uint16_t)yawMagnitude >= INT16_MAX + 1u) {
		yawMagnitude = (int16_t)-yawMagnitude;
	}
	if (yawMagnitude <= FEINPUT_TEXT_YAW_DEAD_ZONE) {
		g_scaledInputYaw = 0;
	}
	pitchMagnitude = g_scaledInputPitch;
	if ((uint16_t)pitchMagnitude >= INT16_MAX + 1u) {
		pitchMagnitude = (int16_t)-pitchMagnitude;
	}
	if (pitchMagnitude <= FEINPUT_TEXT_PITCH_DEAD_ZONE) {
		g_scaledInputPitch = 0;
	}
}

// FUNCTION: XW 0x40B570
void feinput_setupinputdevices(void) {
	g_joystickAvailable = 0;
	g_joystickPollingSuppressed = 0;
	g_flightMouseInputEnabled = 0;
	g_mouseButtons = 0;
	g_keyMods = 0;
	g_joystickDetectResultWord = xio_Is_Joystick_Input();
	feinput_ResetControlState();
	g_flightMouseInputEnabled = 0;
	g_joystickAvailable = g_joystickDetectResultWord != 0;
}

// FUNCTION: XW 0x40B5D0
void feinput_ResetControlState(void) {
#ifdef XW_MODERN
	XwFlightControls_Reset();
#endif
	g_throttleSmoothed = FEINPUT_THROTTLE_UNINITIALIZED;
	g_controlMask = 0;
}

// FUNCTION: XW 0x40B5F0
uint16_t feinput_getrawinput(void) {
#ifdef XW_MODERN
	return XwFlightControls_ReadLocal();
#else
	int32_t joystickZ = 0;
	int32_t joystickY = 0;
	int32_t joystickX = 0;
	int mouseDeltaX = 0;
	int mouseDeltaY = 0;
	uint32_t retainedButtons = 0;
	uint32_t previousButtons;
	uint32_t buttonBit;
	int bindingIndex;
	uint16_t actionKey;
	int16_t modifier1Held;
	int16_t modifier2Held;
	uint16_t modifiers;
	if (g_joystickAvailable != 0) {
		retainedButtons = xjoy_Joystick_Read(&joystickX, &joystickY, &joystickZ, 0);
	}
	if (g_flightMouseInputEnabled != 0) {
		/* Both original mouse hooks are stripped RETs. */
		nullsub_SharedNoOp();
		nullsub_SharedNoOp();
		if ((int16_t)mouseDeltaX <= -FEINPUT_MOUSE_X_LIMIT) {
			mouseDeltaX = 1 - FEINPUT_MOUSE_X_LIMIT;
		} else if ((int16_t)mouseDeltaX >= FEINPUT_MOUSE_X_LIMIT) {
			mouseDeltaX = FEINPUT_MOUSE_X_LIMIT - 1;
		}
		if ((int16_t)mouseDeltaY <= -FEINPUT_MOUSE_Y_LIMIT) {
			mouseDeltaY = 1 - FEINPUT_MOUSE_Y_LIMIT;
		} else if ((int16_t)mouseDeltaY >= FEINPUT_MOUSE_Y_LIMIT) {
			mouseDeltaY = FEINPUT_MOUSE_Y_LIMIT - 1;
		}
	}
	actionKey = 0;
	if (FlightInput_HasKeyReady() != 0) {
		actionKey = FlightInput_GetNextKey();
	}
	previousButtons = g_controlMask;
	modifier2Held = 0;
	modifier1Held = 0;
	for (bindingIndex = 0, buttonBit = 1; bindingIndex < JOYSTICK_ACTION_CAPACITY;
		 buttonBit <<= 1, ++bindingIndex) {
		uint16_t boundAction = g_flightJoystickActions[bindingIndex];
		if (boundAction == 0) {
			continue;
		}
		if ((retainedButtons & buttonBit) != 0) {
			switch (boundAction) {
				case JOYSTICK_ACTION_FIRE:
					modifier1Held = 1;
					break;
				case JOYSTICK_ACTION_ROLL_TARGET:
					modifier2Held = 1;
					break;
			}
			if ((previousButtons & buttonBit) == 0) {
				if (actionKey == 0) {
					actionKey = boundAction;
				} else {
					retainedButtons &= ~buttonBit;
				}
			}
		} else if ((previousButtons & buttonBit) != 0) {
			uint16_t releaseAction;
			if (boundAction != JOYSTICK_ACTION_RELEASE_REPEAT) {
				if (boundAction <= JOYSTICK_ACTION_RELEASE_REPEAT ||
					boundAction > JOYSTICK_ACTION_RELEASE_RANGE_LAST) {
					continue;
				}
				releaseAction = JOYSTICK_ACTION_RELEASE_NEUTRAL;
			} else {
				releaseAction = JOYSTICK_ACTION_RELEASE_REPEAT;
			}
			if (actionKey == 0) {
				actionKey = releaseAction;
			} else {
				retainedButtons |= buttonBit;
			}
		}
	}
	g_controlMask = retainedButtons;
	modifiers = (uint16_t)(modifier1Held + 2 * modifier2Held);
	if (actionKey == 0) {
		int sample = (int8_t)joystickZ + FEINPUT_THROTTLE_CENTER;
		if (g_throttleSmoothed == FEINPUT_THROTTLE_UNINITIALIZED) {
			g_throttleSmoothed = sample;
		} else {
			int throttleBin;
			int previousThrottle = g_throttleSmoothed;
			g_throttleSmoothed += (sample - g_throttleSmoothed) / FEINPUT_THROTTLE_FILTER_DIVISOR;
			throttleBin = (g_throttleSmoothed + FEINPUT_THROTTLE_BIN_WIDTH / 2) / FEINPUT_THROTTLE_BIN_WIDTH;
			if ((previousThrottle + FEINPUT_THROTTLE_BIN_WIDTH / 2) / FEINPUT_THROTTLE_BIN_WIDTH !=
				throttleBin) {
				if (throttleBin < 0)
					throttleBin = 0;
				if (throttleBin > FEINPUT_THROTTLE_BIN_COUNT - 1)
					throttleBin = FEINPUT_THROTTLE_BIN_COUNT - 1;
				actionKey = g_throttleKeyTable[FEINPUT_THROTTLE_BIN_COUNT - 1 - throttleBin];
			}
		}
	}
	g_flightMouseDeltaX = (int16_t)mouseDeltaX;
	g_keyMods = modifiers;
	g_flightJoystickY = (int16_t)joystickY;
	g_flightMouseDeltaY = (int16_t)mouseDeltaY;
	g_flightJoystickX = (int16_t)joystickX;
	/* The stripped hooks leave original mouse positions/buttons indeterminate. */
	g_flightMouseY = 0;
	g_actionKey = actionKey;
	g_mouseButtons = 0;
	g_flightMouseX = 0;
	return actionKey;
#endif
}

// FUNCTION: XW 0x40B840
void feinput_setupgraphics(uint8_t initialGraphicsDetailPreset) {
	switch ((int)g_flightResolutionMode) {
		case RTSVGA2_MODE_13H:
			g_palettePackedMode = FEINPUT_RENDER_INDEXED_LOW;
			break;
		case FLIGHT_DISPLAY_MODE_101H:
			g_palettePackedMode = FEINPUT_RENDER_INDEXED_HIGH;
			break;
		case FLIGHT_DISPLAY_MODE_111H:
		case FLIGHT_DISPLAY_MODE_1FFH:
			g_palettePackedMode = FEINPUT_RENDER_RGB16;
			break;
		default:
			g_palettePackedMode = FEINPUT_RENDER_INDEXED_LOW;
			break;
	}
	g_flightViewportMode = FEINPUT_INITIAL_VIEWPORT_MODE;
	feinput_SetGraphicsPtrs(g_palettePackedMode);
	g_flightGraphicsDetailPreset = initialGraphicsDetailPreset;
}

// FUNCTION: XW 0x40B890
void feinput_SetGraphicsPtrs(uint8_t pixelMode) {
	const FlightRenderCallbacks* callbacks;
#ifdef XW_MODERN
	if (XwFlightTypes_Dos() && Dos94_display && !g_frontendDisplayWndProcMode) {
		Dos94Display_BindCallbacks();
		return;
	}
#endif
	callbacks = &g_flightRenderCallbackTables[pixelMode];
	g_flightInitLineBufferFn = callbacks->initLineBuffer;
	g_flightRenderTransitionHook = callbacks->transitionHook;
	g_flightResetPaletteFn = callbacks->resetPalette;
	g_flightSetPaletteRangeFn = callbacks->setPaletteRange;
	g_flightGetPaletteFn = callbacks->getPalette;
	g_flightSetPaletteFn = callbacks->setPalette;
	g_flightComputePixelOffsetFn = callbacks->computePixelOffset;
	g_flightBlitSpriteFn = callbacks->blitSprite;
	g_flightDrawCharFn = callbacks->drawChar;
	g_flightFillClipRectFn = callbacks->fillClipRect;
	g_flightFillRectClippedFn = callbacks->fillRectClipped;
	{
		void (*saveScreenRect)(uint8_t*, uint16_t, uint16_t, uint16_t, uint16_t) = callbacks->saveScreenRect;
		void (*restoreScreenRect)(const uint8_t*, uint16_t, uint16_t, uint16_t, uint16_t) =
			callbacks->restoreScreenRect;
		g_flightRestoreScreenRectFn = restoreScreenRect;
		g_flightSaveScreenRectFn = saveScreenRect;
	}
	nullsub_SharedNoOp();
}
