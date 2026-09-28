#ifndef XW_FLIGHT_FEINPUT_H
#define XW_FLIGHT_FEINPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/render/renderer.h"

#include <stddef.h>
#include <stdint.h>

enum {
	FEINPUT_RENDER_INDEXED_LOW = 0,
	FEINPUT_RENDER_INDEXED_HIGH = 1,
	FEINPUT_RENDER_RGB16 = 2,
	FEINPUT_RENDER_MODE_COUNT = 3
};

enum { FEINPUT_INITIAL_GRAPHICS_DETAIL = 3, FEINPUT_INITIAL_VIEWPORT_MODE = 1 };

enum {
	FEINPUT_THROTTLE_UNINITIALIZED = -1,
	FEINPUT_THROTTLE_CENTER = 128,
	FEINPUT_THROTTLE_FILTER_DIVISOR = 4,
	FEINPUT_THROTTLE_BIN_WIDTH = 16,
	FEINPUT_THROTTLE_BIN_COUNT = 17,
	FEINPUT_MOUSE_X_LIMIT = 192,
	FEINPUT_MOUSE_Y_LIMIT = 128
};

enum {
	FEINPUT_YAW_DEAD_ZONE = 64,
	FEINPUT_PITCH_DEAD_ZONE = 24,
	FEINPUT_TEXT_YAW_DEAD_ZONE = 2048,
	FEINPUT_TEXT_PITCH_DEAD_ZONE = 1536
};

enum {
	FEINPUT_MOUSE_YAW_SCALE = 128,
	FEINPUT_MOUSE_PITCH_SCALE = 64,
	FEINPUT_JOYSTICK_YAW_SCALE = 120,
	FEINPUT_JOYSTICK_PITCH_SCALE = 50
};

extern FlightRenderCallbacks g_flightRenderCallbackTables[FEINPUT_RENDER_MODE_COUNT];
extern const uint8_t g_throttleKeyTable[FEINPUT_THROTTLE_BIN_COUNT];
extern uint8_t g_flightGraphicsDetailPreset;
extern unsigned int g_controlMask;
extern int g_throttleSmoothed;
extern int16_t g_joystickPollingSuppressed;
extern void (*g_flightDrawCharFn)(char ch);
extern int16_t g_flightMouseX;
extern int16_t g_flightMouseY;
extern int16_t g_flightMouseDeltaX;
extern int16_t g_flightMouseDeltaY;
extern uint8_t g_palettePackedMode;
extern uint16_t g_actionKey;
extern void (*g_flightResetPaletteFn)(void);
extern int (*g_flightComputePixelOffsetFn)(uint16_t x, uint16_t y);
extern void (*g_flightRenderTransitionHook)(void);
extern void (*g_flightFillClipRectFn)(void);
extern uint16_t g_currentActionKey;
extern void (*g_flightInitLineBufferFn)(void);
extern void (*g_flightSetPaletteRangeFn)(const struct RgbTriplet* rgbTriples, uint16_t startIndex,
										 uint16_t count);
extern void (*g_flightBlitSpriteFn)(const uint8_t* rleData, int16_t x, int16_t y, int16_t transparentColor,
									int16_t mirror);
extern void (*g_flightSaveScreenRectFn)(uint8_t* buffer, uint16_t xByteOffset, uint16_t y, uint16_t width,
										uint16_t height);
extern uint16_t g_mouseButtons;
extern int16_t g_flightMouseInputEnabled;
extern uint16_t g_flightKeyMods;
extern void (*g_flightFillRectClippedFn)(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
extern int16_t g_scaledInputPitch;
extern int16_t g_scaledInputYaw;
extern void (*g_flightSetPaletteFn)(const struct RgbTriplet* rgbTriples);
extern int16_t g_flightJoystickX;
extern int16_t g_flightJoystickY;
extern void (*g_flightRestoreScreenRectFn)(const uint8_t* buffer, uint16_t xByteOffset, uint16_t y,
										   uint16_t width, uint16_t height);
extern void (*g_flightGetPaletteFn)(struct RgbTriplet* dstPalette);
extern int16_t g_joystickDetectResultWord;
extern int16_t g_joystickAvailable;
extern uint16_t g_keyMods;
extern uint16_t g_flightViewportMode;

/* Declarations follow ascending original IDB address. */

/* 0x40B400 */
void feinput_checkinput(void);

/* 0x40B4C0 */
/* INT16_MIN wraps during magnitude conversion and is also cleared. */
void feinput_degitterinput(void);

/* 0x40B510 */
void feinput_getinput(void);

/* 0x40B570 */
void feinput_setupinputdevices(void);

/* 0x40B5D0 */
void feinput_ResetControlState(void);

/* 0x40B5F0 */
uint16_t feinput_getrawinput(void);

/* 0x40B840 */
void feinput_setupgraphics(uint8_t initialGraphicsDetailPreset);

/* 0x40B890 */
void feinput_SetGraphicsPtrs(uint8_t pixelMode);

#ifdef __cplusplus
}
#endif

#endif
