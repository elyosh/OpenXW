#ifndef XW_RUNTIME_INPUT_MOUSE_FLIGHT_H
#define XW_RUNTIME_INPUT_MOUSE_FLIGHT_H
#include <stdint.h>

typedef enum XwMouseFlightMode { XW_MOUSE_VIRTUAL_STICK, XW_MOUSE_CLASSIC } XwMouseFlightMode;

typedef struct XwMouseFlightSample {
	XwMouseFlightMode mode;
	/* Joystick axes for virtual stick; DOS motion counts for classic. */
	int x, y;
} XwMouseFlightSample;

void XwMouseFlight_SetOptions(XwMouseFlightMode mode, int sensitivity, int invert_y);
void XwMouseFlight_Reset(void);
void XwMouseFlight_Pump(int32_t delta_us);
int XwMouseFlight_Sample(XwMouseFlightSample* sample);
/* Read the held stick without consuming pending motion or advancing input. */
int XwMouseFlight_GetHudMarker(int* yaw, int* pitch);
uint16_t XwMouseFlight_ReadKey(void);
uint16_t XwMouseFlight_ReadButtons(void);
#endif
