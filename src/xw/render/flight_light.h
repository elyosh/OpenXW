#ifndef XW_RENDER_FLIGHT_LIGHT_H
#define XW_RENDER_FLIGHT_LIGHT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/assets/opt_model.h"
#include "xw/render/render_scene.h"
#include <stddef.h>
#include <stdint.h>

extern const float g_lightDirectionQ15ToFloat;
extern const float g_lightDistanceMinorAxisWeight;
extern const float g_lightHalfVectorMinorAxisWeight;
extern const float g_lightHalfVectorMajorAxisWeight;
extern const float g_softwareViewVectorMinorAxisWeight;
extern const float g_softwareViewVectorMajorAxisWeight;
extern const float g_softwareDirectionalSpecularScale;
extern int g_specularEnabled;
extern float g_swFaceLightPointIntensities[XW_POINT_LIGHT_CAPACITY];
extern OptVector g_swFaceLightDir;
extern OptVector g_swFaceLightPointPositions[XW_POINT_LIGHT_CAPACITY];
extern OptVector g_swFaceLightFaceNormal;
extern int g_swFaceLightCachedPointLightCount;

extern const float g_softwareLightZero;
extern const float g_softwareLightHalf;
extern const float g_softwareLightOne;

struct ObjectRecord;
struct SceneFace;

extern struct ObjectRecord* g_swFaceLightCachedObject;
extern struct SceneFace* g_swFaceLightCachedFace;

/* Declarations follow ascending original IDB address. */

/* 0x47D290 */
void FlightLight_ResetSoftwareFaceSampleCache(void);

/* 0x47D2A0 */
float FlightLight_ComputeSoftwareFaceSampleIntensity(struct SceneFace* face, int screenX, int screenY,
													 float projectedDepth);

#ifdef __cplusplus
}
#endif

#endif
