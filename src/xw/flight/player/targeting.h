#ifndef XW_FLIGHT_PLAYER_TARGETING_H
#define XW_FLIGHT_PLAYER_TARGETING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

enum { TARGETING_COMPONENT_NONE = 0xFFFF };

enum { TARGETING_CURRENT_TARGET_COLOR = 0x3B };

enum { TARGETING_BOX_MIN_LOW = 4, TARGETING_BOX_MIN_HIGH = 8, TARGETING_BOX_PADDING = 4 };

enum { TARGETING_BOUND_AXIS_COUNT = 3, TARGETING_SHIFT_COUNT_MASK = 31 };

/* Declarations follow ascending original IDB address. */

/* 0x42E1B0 */
void Targeting_w_DrawObjectBox(void);

/* 0x42E1D0 */
void Targeting_DrawObjectBox(int objectOrMissionPointRef, int componentIndex, uint8_t colorIndex);

/* 0x42E350 */
int Targeting_GetObjectBoxExtent(int objectOrMissionPointRef);

/* 0x42E410 */
void Targeting_ProjectObjectOrMissionPoint(int objectOrMissionPointRef, int componentIndex, int* outScreenX,
										   int* outScreenY, int* outViewZ);

#ifdef __cplusplus
}
#endif

#endif
