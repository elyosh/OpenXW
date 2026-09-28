#include "xw/render/flight_light.h"

#include "xw/flight/fview.h"
#include "xw/flight/xw.h"
#include "xw/math/math3d.h"
#include "xw/render/flight_view.h"
#include "xw/render/render_scene.h"

// GLOBAL: XW 0x4C3258
const float g_softwareLightZero = 0.0f;

// GLOBAL: XW 0x4C3270
const float g_softwareLightHalf = 0.5f;

// GLOBAL: XW 0x4C327C
const float g_softwareLightOne = 1.0f;

// GLOBAL: XW 0x4C3250
const float g_lightDirectionQ15ToFloat = 0.000030517578125f;

// GLOBAL: XW 0x4C325C
const float g_lightDistanceMinorAxisWeight = 0.2941f;

// GLOBAL: XW 0x4C3274
const float g_lightHalfVectorMinorAxisWeight = 0.1936f;

// GLOBAL: XW 0x4C3278
const float g_lightHalfVectorMajorAxisWeight = 0.4632f;

// GLOBAL: XW 0x4C3280
const float g_softwareViewVectorMinorAxisWeight = 0.3872f;

// GLOBAL: XW 0x4C3284
const float g_softwareViewVectorMajorAxisWeight = 0.9264f;

// GLOBAL: XW 0x4C3288
const float g_softwareDirectionalSpecularScale = 0.7f;

// GLOBAL: XW 0x4CEF78
int g_specularEnabled = 1;

// GLOBAL: XW 0x53C8D8
float g_swFaceLightPointIntensities[XW_POINT_LIGHT_CAPACITY] = { 0 };

// GLOBAL: XW 0x53C8F8
OptVector g_swFaceLightDir = { 0 };

// GLOBAL: XW 0x53C908
OptVector g_swFaceLightPointPositions[XW_POINT_LIGHT_CAPACITY] = { { 0 } };

// GLOBAL: XW 0x53C968
OptVector g_swFaceLightFaceNormal = { 0 };

// GLOBAL: XW 0x53C97C
int g_swFaceLightCachedPointLightCount = 0;

// GLOBAL: XW 0x53C978
struct ObjectRecord* g_swFaceLightCachedObject = NULL;

// GLOBAL: XW 0x53C980
struct SceneFace* g_swFaceLightCachedFace = NULL;

// FUNCTION: XW 0x47D290
void FlightLight_ResetSoftwareFaceSampleCache(void) {
	g_swFaceLightCachedObject = NULL;
	g_swFaceLightCachedFace = NULL;
}

// FUNCTION: XW 0x47D2A0
float FlightLight_ComputeSoftwareFaceSampleIntensity(struct SceneFace* face, int screenX, int screenY,
													 float projectedDepth) {
	SceneMesh* mesh;
	OptVector normal;
	OptVector vector;
	double inverseDepth;
	float sampleX;
	float sampleY;
	float sampleZ;
	float intensity;
	float dx;
	float dy;
	float dz;
	float specular;
	float contribution;
	float componentX;
	float componentY;
	float componentZ;
	float distance;
	double inverseLength;
	float reciprocal;
	float lightDot;
	int lightIndex;

	mesh = face->mesh;
	if (mesh == NULL) {
		return g_softwareLightZero;
	}
	if (g_swFaceLightCachedObject != mesh->pObject) {
		Xw_MakeLocalLights(mesh->pObject);
		g_swFaceLightCachedPointLightCount = g_objectPointLightCount;
		g_swFaceLightCachedObject = mesh->pObject;
		for (lightIndex = 0; lightIndex < g_objectPointLightCount; ++lightIndex) {
			OptVector* position = &g_swFaceLightPointPositions[lightIndex];

			position->x = (float)g_objectPointLights[lightIndex].x;
			position->y = (float)g_objectPointLights[lightIndex].y;
			position->z = (float)g_objectPointLights[lightIndex].z;
			Math3D_RotateVec3(&position->x, mesh->viewOrient);
			position->x += mesh->viewPosX;
			position->y += mesh->viewPosY;
			position->z += mesh->viewPosZ;
			g_swFaceLightPointIntensities[lightIndex] = (float)g_objectPointLights[lightIndex].intensity;
		}
		g_swFaceLightDir.x = (float)g_objectLightDirectionX * g_lightDirectionQ15ToFloat;
		g_swFaceLightDir.y = (float)g_objectLightDirectionY * g_lightDirectionQ15ToFloat;
		g_swFaceLightDir.z = (float)g_objectLightDirectionZ * g_lightDirectionQ15ToFloat;
		Math3D_RotateVec3(&g_swFaceLightDir.x, mesh->viewOrient);
	}

	inverseDepth = g_softwareLightOne / projectedDepth;
	sampleZ = inverseDepth;
	inverseDepth *= g_invProjScale;
	sampleX = (float)(screenX - (g_flightVpWidth >> 1)) * inverseDepth;
	sampleY = (float)(screenY - (g_flightVpHeight >> 1) - g_projOffsetY) * inverseDepth;
	if (face != g_swFaceLightCachedFace) {
		g_swFaceLightCachedFace = face;
		vector = mesh->faceNormals[face->faceIndex];
		Math3D_RotateVec3(&vector.x, mesh->viewOrient);
		normal = vector;
		g_swFaceLightFaceNormal = vector;
	} else {
		normal = g_swFaceLightFaceNormal;
	}
	intensity = g_softwareLightZero;

	if (g_specularEnabled != 0) {
		vector.x = -sampleX;
		componentX = vector.x;
		vector.y = -sampleY;
		componentY = vector.y;
		vector.z = -sampleZ;
		componentZ = vector.z;
		if (vector.x < g_softwareLightZero) {
			componentX = -vector.x;
		}
		if (vector.y < g_softwareLightZero) {
			componentY = -vector.y;
		}
		if (vector.z < g_softwareLightZero) {
			componentZ = -vector.z;
		}
		if (componentX >= componentY && componentX >= componentZ) {
			distance = componentX * g_softwareViewVectorMajorAxisWeight +
					   (componentZ + componentY) * g_softwareViewVectorMinorAxisWeight;
		} else if (componentY >= componentX && componentY >= componentZ) {
			distance = componentY * g_softwareViewVectorMajorAxisWeight +
					   (componentZ + componentX) * g_softwareViewVectorMinorAxisWeight;
		} else {
			distance = componentZ * g_softwareViewVectorMajorAxisWeight +
					   (componentY + componentX) * g_softwareViewVectorMinorAxisWeight;
		}
		inverseLength = g_softwareLightOne / distance;
		dx = vector.x * inverseLength + g_swFaceLightDir.x;
		dy = vector.y * inverseLength + g_swFaceLightDir.y;
		dz = vector.z * inverseLength + g_swFaceLightDir.z;
		{
			float xyDot = normal.x * dx + normal.y * dy;
			float normalDot = normal.z * dz + xyDot;
			specular = normalDot;
		}
		if (specular > g_softwareLightZero) {
			specular *= g_softwareLightHalf;
			specular = specular * specular * specular;
			specular *= specular;
			specular *= specular;
			specular *= specular;
			specular *= specular;
		} else {
			specular = g_softwareLightZero;
		}
		contribution = (float)(specular * g_softwareDirectionalSpecularScale);
		if (contribution > g_softwareLightZero) {
			intensity = contribution;
			if (intensity >= g_softwareLightOne) {
				return g_softwareLightOne;
			}
		}
	}

	for (lightIndex = 0; lightIndex < g_swFaceLightCachedPointLightCount; ++lightIndex) {
		const OptVector* position = &g_swFaceLightPointPositions[lightIndex];

		dx = position->x - sampleX;
		dy = position->y - sampleY;
		dz = position->z - sampleZ;
		lightDot = normal.x * dx + normal.y * dy + normal.z * dz;
		if (lightDot > g_softwareLightZero) {
			componentX = dx;
			componentY = dy;
			componentZ = dz;
			if (dx < g_softwareLightZero) {
				componentX = -dx;
			}
			if (dy < g_softwareLightZero) {
				componentY = -dy;
			}
			if (dz < g_softwareLightZero) {
				componentZ = -dz;
			}
			if (componentX >= componentY && componentX >= componentZ) {
				distance = componentX + (componentZ + componentY) * g_lightDistanceMinorAxisWeight;
			} else if (componentY >= componentX && componentY >= componentZ) {
				distance = componentY + (componentZ + componentX) * g_lightDistanceMinorAxisWeight;
			} else {
				distance = componentZ + (componentY + componentX) * g_lightDistanceMinorAxisWeight;
			}
			lightDot = lightDot / (distance * distance);
			if (g_specularEnabled != 0) {
				double halfDot;
				double cosine;

				dx -= sampleX;
				dy -= sampleY;
				dz -= sampleZ;
				{
					float xyDot = normal.x * dx + normal.y * dy;
					halfDot = (normal.z * dz + xyDot) * g_softwareLightHalf;
				}
				componentX = dx;
				componentY = dy;
				componentZ = dz;
				if (dx < g_softwareLightZero) {
					componentX = -dx;
				}
				if (dy < g_softwareLightZero) {
					componentY = -dy;
				}
				if (dz < g_softwareLightZero) {
					componentZ = -dz;
				}
				if (componentX >= componentY && componentX >= componentZ) {
					distance = componentX * g_lightHalfVectorMajorAxisWeight +
							   (componentZ + componentY) * g_lightHalfVectorMinorAxisWeight;
				} else if (componentY >= componentX && componentY >= componentZ) {
					distance = componentY * g_lightHalfVectorMajorAxisWeight +
							   (componentZ + componentX) * g_lightHalfVectorMinorAxisWeight;
				} else {
					distance = componentZ * g_lightHalfVectorMajorAxisWeight +
							   (componentY + componentX) * g_lightHalfVectorMinorAxisWeight;
				}
				reciprocal = g_softwareLightOne / distance;
				cosine = halfDot * reciprocal;
				if (cosine >= g_softwareLightZero) {
					specular = cosine * cosine * cosine;
					specular *= specular;
					specular *= specular;
					specular *= specular;
					specular *= specular;
					specular *= reciprocal;
				} else {
					specular = g_softwareLightZero;
				}
			} else {
				specular = g_softwareLightZero;
			}
			contribution = lightDot + specular;
			if (contribution > g_softwareLightZero) {
				intensity += g_swFaceLightPointIntensities[lightIndex] * contribution;
				if (intensity >= g_softwareLightOne) {
					intensity = g_softwareLightOne;
					break;
				}
			}
		}
	}
	return intensity;
}
