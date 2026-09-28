#ifndef XW_FLIGHT_FLIGHT_INPUT_H
#define XW_FLIGHT_FLIGHT_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

extern int g_flightConfDirectInput;

enum { FLIGHT_VIRTUAL_KEY_COUNT = 256, FLIGHT_EXTENDED_KEY_BASE = 0x80 };

extern int g_windowVirtualKeyDown[FLIGHT_VIRTUAL_KEY_COUNT];
extern uint8_t g_lastKeyCode;
extern unsigned int g_lastReleasedVirtualKey;
extern int g_keyReady;

/* Declarations follow ascending original IDB address. */

/* 0x49E550 */
int j_FlightInput_HasKeyReady(void);

/* 0x4AC490 */
int FlightInput_HasKeyReady(void);

/* 0x4AC520 */
uint8_t FlightInput_GetNextKey(void);

#ifdef __cplusplus
}
#endif

#endif
