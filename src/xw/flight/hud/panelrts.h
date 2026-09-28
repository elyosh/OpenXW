#ifndef XW_FLIGHT_HUD_PANELRTS_H
#define XW_FLIGHT_HUD_PANELRTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

enum { PANELRTS_DECIMAL_DIVISOR_COUNT = 6, PANELRTS_NUMBER_UNAVAILABLE = 0xFFFF };

enum { PANELRTS_DECIMAL_MAX_DIGIT = 9 };

extern const uint16_t g_flightTextDecimalDivisors[PANELRTS_DECIMAL_DIVISOR_COUNT];

/* Declarations follow ascending original IDB address. */

/* 0x41C6D0 */
void panelrts_setnewpilotview(uint16_t hudViewState);

/* 0x41C730 */
void panelrts_outnum(uint16_t value, uint16_t width, uint16_t minDigits);

#ifdef __cplusplus
}
#endif

#endif
