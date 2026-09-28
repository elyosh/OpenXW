#ifndef XW_FLIGHT_MISSION_SPEC_H
#define XW_FLIGHT_MISSION_SPEC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

enum { SPEC_STATISTICS_B_WING = 17, SPEC_STATISTICS_INTERDICTOR = 18 };

/* Declarations follow ascending original IDB address. */

/* 0x4239F0 */
uint16_t spec_getspecnum(uint16_t objectType);

/* 0x423A10 */
uint16_t spec_getstatisticscategory(uint16_t objectType);

#ifdef __cplusplus
}
#endif

#endif
