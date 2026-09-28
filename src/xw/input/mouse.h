#ifndef XW_INPUT_MOUSE_H
#define XW_INPUT_MOUSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

/* Declarations follow ascending original IDB address. */

/* 0x49E580 */
void Mouse_ReadPositionAndButtons(uint16_t* outButtons, int16_t* outX, int16_t* outY);

/* 0x4AB920 */
int32_t Mouse_SetPosition(int16_t x, int16_t y);

#ifdef __cplusplus
}
#endif

#endif
