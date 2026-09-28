#ifndef XW_INPUT_DINPUT_H
#define XW_INPUT_DINPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <aeron/compat/dinput.h>
#include <stddef.h>
#include <stdint.h>

enum {
	DINPUT_VERSION_5 = 0x500,
	DINPUT_VERSION_3 = 0x300,
	DINPUT_KEYBOARD_BUFFER_SIZE = 32,
	DINPUT_PROPERTY_DEVICE = 0,
	DINPUT_KEYBOARD_STATE_SIZE = 256,
	DINPUT_KEY_DOWN = 0x80,
	DINPUT_SHIFT_MARKER = 0xFD,
	DINPUT_CTRL_MARKER = 0xFE,
	DINPUT_ALT_MARKER = 0xFF,
	DINPUT_KEY_LEFT_SHIFT = 0x2A,
	DINPUT_KEY_RIGHT_SHIFT = 0x36,
	DINPUT_KEY_LEFT_CTRL = 0x1D,
	DINPUT_KEY_RIGHT_CTRL = 0x9D,
	DINPUT_KEY_LEFT_ALT = 0x38,
	DINPUT_KEY_RIGHT_ALT = 0xB8
};

extern const DxGuid g_directInputSystemKeyboardGuid;
extern const uint8_t g_dinputKeyCodeTable[DINPUT_KEYBOARD_STATE_SIZE];
extern const uint8_t g_dinputShiftKeyCodeTable[DINPUT_KEYBOARD_STATE_SIZE];
extern const uint8_t g_dinputCtrlKeyCodeTable[DINPUT_KEYBOARD_STATE_SIZE];
extern const uint8_t g_dinputAltKeyCodeTable[DINPUT_KEYBOARD_STATE_SIZE];

extern IDirectInputA* g_directInput;
extern IDirectInputDeviceA* g_dinputKeyboardDevice;
extern int g_dinputKeyboardAcquired;
extern int g_dinputCtrlDown;
extern int g_dinputShiftDown;
extern int g_dinputAltDown;

/* Declarations follow ascending original IDB address. */

/* 0x47BE60 */
int DInput_Init(void);

/* 0x47C010 */
int DInput_HasKeyReady(void);

/* 0x47C130 */
uint8_t DInput_GetKey(void);

/* 0x47C230 */
void DInput_UpdateKeyboardModifierState(void);

/* 0x47C2B0 */
void DInput_Shutdown(void);

/* 0x47C2F0 */
int DInput_ReacquireKeyboard(void);

#ifdef __cplusplus
}
#endif

#endif
