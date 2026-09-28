#ifndef XW_RUNTIME_ORIENTATION_HOOK_H
#define XW_RUNTIME_ORIENTATION_HOOK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XwOrientationAngles {
	uint16_t yaw, pitch, roll;
} XwOrientationAngles;

/* Apply local pitch/yaw using OpenXWA's gimbal-lock-safe rotation path. */
XwOrientationAngles XwOrientation_ApplyPitchYaw(XwOrientationAngles current, int pitchDeltaQ16,
												int negYawDeltaQ16);

#ifdef __cplusplus
}
#endif
#endif
