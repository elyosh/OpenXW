#ifndef XW_RUNTIME_INPUT_BRIDGE_H
#define XW_RUNTIME_INPUT_BRIDGE_H
#include <aeron/input.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum XwKeyboardRoute { XW_KEYBOARD_RAW, XW_KEYBOARD_GAMEPLAY, XW_KEYBOARD_BLOCKED } XwKeyboardRoute;

void XwInput_Init(void);
void XwInput_ApplySettings(const AeronInputSnapshot* input);
void XwInput_Shutdown(void);
XwKeyboardRoute XwInput_ReconcileKeyboard(void);
bool XwInput_GameplayActive(void);
bool XwInput_SettingsShortcutAllowed(void);
bool XwInput_RendererShortcutAllowed(void);
void XwInput_ClearCommands(void);
void XwInput_FlushRawKeyboard(void);
int XwInput_FlightKeyPending(void);
uint16_t XwInput_ReadFlightKey(void);
void XwInput_QueueFlightKey(uint16_t key);
int XwInput_CanReacquireKeyboard(void);
uint32_t XwInput_JoystickRead(int32_t* x, int32_t* y, int32_t* z, int16_t device);
/* Sample once after Aeron_BeginFrame. Raw Landru input is consumed independently of flight commands. */
void XwInput_BeginFrame(const AeronInputSnapshot* input, bool suppressed, int32_t delta_us);
void XwInput_Reset(void);
void XwInput_ResetPointer(void);
void XwInput_SetPointer(int x, int y);
void XwInput_CursorPosition(int* x, int* y);
int XwInput_KeyPending(void);
int XwInput_ReadKey(void);
int XwInput_Modifiers(void);
void XwInput_MousePosition(int16_t* buttons, int16_t* x, int16_t* y);
void XwInput_MouseMovement(int16_t* x, int16_t* y);
#ifdef __cplusplus
}
#endif
#endif
