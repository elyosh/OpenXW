#ifndef XW_RUNTIME_INPUT_FLIGHT_CONTROLS_H
#define XW_RUNTIME_INPUT_FLIGHT_CONTROLS_H
#include "xw_runtime/runtime/flight_input.h"
#include <stdbool.h>
#include <stdint.h>
/* Independent analog roll, scaled like joystick yaw before player smoothing. */
extern int16_t g_xwInputRoll;
void XwFlightControls_Reset(void);
void XwFlightControls_ResetThrottle(void);
bool XwFlightControls_ThrottleEligible(void);
void XwFlightControls_UpdateThrottleContext(void);
void XwFlightControls_SampleThrottle(XwFlightInput* record);
void XwFlightControls_ApplyThrottle(const XwFlightInput* input);
void XwFlightControls_CollectCommands(void);
uint16_t XwFlightControls_ReadLocal(void);
#endif
