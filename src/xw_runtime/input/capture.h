#ifndef XW_RUNTIME_INPUT_CAPTURE_H
#define XW_RUNTIME_INPUT_CAPTURE_H
#include <aeron/input.h>
#include <stdbool.h>
void XwInput_BeginCaptureFrame(const AeronInputSnapshot* input, bool captured);
void XwInput_ResetCapture(void);
void XwInput_BlockHeldKeys(void);
void XwInput_SuppressKey(int key);
/* Held renderer Tab survives legacy input resets until its release barrier. */
void XwInput_SuppressRendererTab(bool suppressed);
bool XwInput_KeyBlocked(int key);
bool XwInput_IsCaptured(void);
bool XwInput_MouseMotionAllowed(void);
void XwInput_BlockMouseButtons(uint32_t buttons);
uint32_t XwInput_FilterMouseButtons(uint32_t buttons);
void XwInput_UpdateMouseCapture(const AeronInputSnapshot* input, int32_t delta_us);
bool XwInput_MouseFlightAllowed(void);
#endif
