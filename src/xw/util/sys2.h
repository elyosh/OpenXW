#ifndef XW_UTIL_SYS2_H
#define XW_UTIL_SYS2_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

/* Declarations follow ascending original IDB address. */

/* 0x423960 */
int16_t sys2_calclength(const uint8_t* text);

#ifdef __cplusplus
}
#endif

#endif
