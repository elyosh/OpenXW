#ifndef XW_INPUT_LANDRU_MOUSE_H
#define XW_INPUT_LANDRU_MOUSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

/* Declarations follow ascending original IDB address. */

/* 0x49E5A0 */
int32_t LandruMouse_SetPosition(int16_t x, int16_t y);

/* 0x4AB940 */
void LandruMouse_SetBoundsStub(int16_t minimum, int16_t maximum);

#ifdef __cplusplus
}
#endif

#endif
