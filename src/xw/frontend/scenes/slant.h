#ifndef XW_FRONTEND_SCENES_SLANT_H
#define XW_FRONTEND_SCENES_SLANT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/landru_config.h"

#include <stddef.h>
#include <stdint.h>

enum { SLANT_SOURCE_PITCH = 320, SLANT_FRACTION_MASK = 0xFFFF };

/* Declarations follow ascending original IDB address. */

/* 0x4611A0 */
void slant_Scale_Line(const uint8_t* bitmap, int16_t srcX, int16_t srcY, int16_t skip,
					  uint16_t fractionalSkip, int16_t dstX, int16_t dstY, int16_t width, uint8_t color);

#ifdef __cplusplus
}
#endif

#endif
