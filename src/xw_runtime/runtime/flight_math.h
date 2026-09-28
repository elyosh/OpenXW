#ifndef XW_RUNTIME_RUNTIME_FLIGHT_MATH_H
#define XW_RUNTIME_RUNTIME_FLIGHT_MATH_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void XwFlightMath_MoveHyperspace(int rate);
int32_t XwFlightMath_PredictionStep(int16_t distance, int32_t direction);

#ifdef __cplusplus
}
#endif
#endif
