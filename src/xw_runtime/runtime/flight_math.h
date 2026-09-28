#ifndef XW_RUNTIME_RUNTIME_FLIGHT_MATH_H
#define XW_RUNTIME_RUNTIME_FLIGHT_MATH_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void XwFlightMath_MoveHyperspace(int rate);
int32_t XwFlightMath_PredictionStep(int16_t distance, int32_t direction);

typedef struct XwLaserAim {
	int16_t pitch, yaw;
	int16_t moveX, moveY, moveZ;
} XwLaserAim;

/* Cannon-only, owner-thread helper. The caller supplies the rotated muzzle position. */
bool XwFlightMath_ConvergeLaser(uint16_t sourceIndex, int32_t muzzleX, int32_t muzzleY, int32_t muzzleZ,
								XwLaserAim* aim);

#ifdef __cplusplus
}
#endif
#endif
