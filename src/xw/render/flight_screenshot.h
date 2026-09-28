#ifndef XW_RENDER_FLIGHT_SCREENSHOT_H
#define XW_RENDER_FLIGHT_SCREENSHOT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

enum { FLIGHT_SCREENSHOT_NAME_CAPACITY = 64 };

/* Declarations follow ascending original IDB address. */

/* 0x4AF020 */
void FlightScreenshot_Capture(void);

#ifdef __cplusplus
}
#endif

#endif
