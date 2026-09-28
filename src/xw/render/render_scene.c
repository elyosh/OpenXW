#include "xw/render/render_scene.h"
#ifdef XW_MODERN
#include "aeron/compat/host.h"
#endif

#include "xw/assets/model_mesh.h"
#include "xw/assets/model_texture.h"
#include "xw/flight/fediskio.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/fview.h"
#include "xw/flight/hud/panel.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/anim.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/object.h"
#include "xw/flight/player/targeting.h"
#include "xw/flight/player/user.h"
#include "xw/flight/xw.h"
#include "xw/math/math.h"
#include "xw/math/math3d.h"
#include "xw/math/transfm2.h"
#include "xw/math/trig2.h"
#include "xw/render/flight_light.h"
#include "xw/render/flight_view.h"
#include "xw/render/render_clip.h"
#include "xw/render/render_quad.h"
#include "xw/render/render_texture.h"
#include "xw/render/renderer.h"
#include "xw/render/rtsrgb.h"
#include "xw/render/std3d.h"
#include "xw/render/sw3d.h"
#include "xw/util/memory.h"
#include "xw/util/shared.h"

#ifndef XW_MODERN
#include <float.h>
#endif
#include <landru/memhdl.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4C3254
const float g_hardwareDirectionalLightScale = 0.8f;

// GLOBAL: XW 0x4C3260
const float g_hardwarePointLightFacingCutoff = -0.3f;

// GLOBAL: XW 0x4C3268
const double g_hardwareDiffuseDistanceScale = 0.5;

// GLOBAL: XW 0x4C32B4
const float g_hardwareTriangleVertexCountFloat = 3.0f;

// GLOBAL: XW 0x4C32B8
const float g_hardwareQuadVertexCountFloat = 4.0f;

// GLOBAL: XW 0x4C32BC
const float g_hardwareDistantProjectionScale = 100000.0f;

// GLOBAL: XW 0x4C32C0
const float g_hardwareDistantDepthSubtract = -100000.0f;

// GLOBAL: XW 0x4C32C8
const float g_hardwareDepthReciprocalScale = 0.00048828125f;

// GLOBAL: XW 0x4C32CC
const float g_hardwareShadeIntensityNegativeScale = -320.0f;

// GLOBAL: XW 0x4C3494
const float g_renderRadiansPerAngleByte = 0.024543672800064087f;

// GLOBAL: XW 0x4C3498
const float g_renderSignedBasisToFloat = 3.0518509447574615e-05f;

// GLOBAL: XW 0x4C349C
const float g_modelNodeQ15ToFloat = 0.000030517578125f;

// GLOBAL: XW 0x4C34A0
const float g_modelFacePlanePositiveCrossingLimit = 40.0f;

// GLOBAL: XW 0x4C34A4
const float g_modelFacePlaneNegativeCrossingLimit = -40.0f;

// GLOBAL: XW 0x4CEF64
float g_lodDistanceScale = 1.0f;

// GLOBAL: XW 0x4CEF6C
float g_textureMipScale = 1.0f;

// GLOBAL: XW 0x4CEF74
int g_dirLightingEnabled = 1;

// GLOBAL: XW 0x4D9DD4
int g_capVertexAlpha = 1;

// GLOBAL: XW 0x4D9DD8
int g_powerVrBeginScenePending = 1;

// GLOBAL: XW 0x4DA6D8
int g_bwingBridgeMeshIndex = -1;

// GLOBAL: XW 0x4F4A48
int g_forcedLodLevel = 0;

// GLOBAL: XW 0x4F4A4C
int g_nodeSwitchIndex = 0;

// GLOBAL: XW 0x53C998
int g_maxBatchTris = 0;

// GLOBAL: XW 0x54CAA0
D3DTLVERTEX* g_flightVertexBuffer = NULL;

// GLOBAL: XW 0x54CAA4
float g_flightVpOriginY = 0.0f;

// GLOBAL: XW 0x55CAB8
Std3DRenderTri* g_triBuffer = NULL;

// GLOBAL: XW 0x55CABC
int g_meshReusableProjectedVertexCount = 0;

// GLOBAL: XW 0x55CAC0
int g_maxBatchVerts = 0;

// GLOBAL: XW 0x55CAC4
int g_d3dVertexCount = 0;

// GLOBAL: XW 0x55CAD0
unsigned int g_d3dIndexCount = 0;

// GLOBAL: XW 0x55CAD4
float g_flightVpOriginX = 0.0f;

// GLOBAL: XW 0x55CAD8
int g_sceneFlushDrawTargetMarkers = 0;

// GLOBAL: XW 0x55CAE8
int g_hardwareFrameField_55CAE8 = 0;

// GLOBAL: XW 0x55CB24
OptTextureData* g_curTextureDesc = NULL;

// GLOBAL: XW 0x55CB48
DefaultWhiteTexture g_defaultWhiteTexture = { 0 };

// GLOBAL: XW 0x55FBB8
uint8_t g_bBackdropMeshMode = 0;

// GLOBAL: XW 0x55FBC0
unsigned int g_curLayerId = 0;

// GLOBAL: XW 0x55FBC4
OptTextureData* g_defaultWhiteTextureDescPtr = NULL;

// GLOBAL: XW 0x55FBC8
int g_vertexLightOcclusionEnabled = 0;

// GLOBAL: XW 0x56187C
unsigned int g_sw3dLightSampleCacheSceneStampBase = 0;

// GLOBAL: XW 0x561880
SceneFace g_sw3dOcclusionSentinelFace = { 0 };

// GLOBAL: XW 0x5BECF0
float g_invProjScale = 0.0f;

// GLOBAL: XW 0x5BED0A
SceneSpan* g_pSceneSpanDataCur = NULL;

// GLOBAL: XW 0x5BED0E
SceneSpan* g_pSceneSpanDataEnd = NULL;

// GLOBAL: XW 0x5BED1C
int g_sceneSpanPtrAvail = 0;

// GLOBAL: XW 0x5BED34
int g_visFacePassStart = 0;

// GLOBAL: XW 0x5BED42
int g_projVertCount = 0;

// GLOBAL: XW 0x5BED82
int g_meshQueueIndex = 0;

// GLOBAL: XW 0x5BECF4
unsigned int g_phongSlotStride = 0;

// GLOBAL: XW 0x5BED00
SceneSpan* g_sceneSpanDataBase = NULL;

// GLOBAL: XW 0x5BED04
LandruHandle g_sceneSpanDataHandle = 0;

// GLOBAL: XW 0x5BED06
int g_sceneSpanDataCapacity = 0;

// GLOBAL: XW 0x5BED12
SceneSpan** g_sceneSpanPtrList = NULL;

// GLOBAL: XW 0x5BED16
LandruHandle g_sceneSpanPtrListHandle = 0;

// GLOBAL: XW 0x5BED18
int g_sceneSpanPtrCapacity = 0;

// GLOBAL: XW 0x5BED20
SoftwareLightSample* g_scenePhongData = NULL;

// GLOBAL: XW 0x5BED24
LandruHandle g_scenePhongDataHandle = 0;

// GLOBAL: XW 0x5BED26
int g_phongSlotIndex = 0;

// GLOBAL: XW 0x5BED2A
SceneFace* g_visFaceList = NULL;

// GLOBAL: XW 0x5BED2E
LandruHandle g_visFaceListHandle = 0;

// GLOBAL: XW 0x5BED30
int g_visFaceCount = 0;

// GLOBAL: XW 0x5BED38
int g_sceneFaceMax = 0;

// GLOBAL: XW 0x5BED3C
ProjVertex* g_projVertList = NULL;

// GLOBAL: XW 0x5BED40
LandruHandle g_projVertListHandle = 0;

// GLOBAL: XW 0x5BED46
int g_projVertMax = 0;

// GLOBAL: XW 0x5BED4A
SceneEdge* g_sceneEdgeList = NULL;

// GLOBAL: XW 0x5BED4E
LandruHandle g_sceneEdgeListHandle = 0;

// GLOBAL: XW 0x5BED50
int g_sceneEdgeCursor = 0;

// GLOBAL: XW 0x5BED54
int g_sceneEdgeMax = 0;

// GLOBAL: XW 0x5BED58
int* g_vertexRemap = NULL;

// GLOBAL: XW 0x5BED5C
LandruHandle g_vertexRemapHandle = 0;

// GLOBAL: XW 0x5BED5E
int g_vertexRemapCapacity = 0;

// GLOBAL: XW 0x5BED62
int* g_sceneEdgeFlags = NULL;

// GLOBAL: XW 0x5BED66
LandruHandle g_sceneEdgeFlagsHandle = 0;

// GLOBAL: XW 0x5BED68
int g_sceneEdgeFlagsCapacity = 0;

// GLOBAL: XW 0x5BED6C
void** g_sceneSclEdgeList = NULL;

// GLOBAL: XW 0x5BED70
LandruHandle g_sceneSclEdgeListHandle = 0;

// GLOBAL: XW 0x5BED72
SceneSpan** g_scanlineSpanHeads = NULL;

// GLOBAL: XW 0x5BED76
LandruHandle g_scanlineSpanHeadsHandle = 0;

// GLOBAL: XW 0x5BED78
SceneMesh* g_meshQueue = NULL;

// GLOBAL: XW 0x5BED7C
LandruHandle g_meshQueueHandle = 0;

// GLOBAL: XW 0x5BED7E
int g_meshQueueMax = 0;

// GLOBAL: XW 0x5BED90
OptVector g_meshCullEyePosition = { 0.0f, 0.0f, 0.0f };

// GLOBAL: XW 0x62CC6A
uint16_t g_billboardObjectOrTypeIndex = 0;

// GLOBAL: XW 0x63BBC8
uint16_t g_damageBillboardMeshCount = 0;

// FUNCTION: XW 0x401000
void RenderScene_DrawAllObjectRootMeshes(uint16_t objectIndex) {
	unsigned int meshCount;
	int16_t savedDetailSetting;
	int meshIndex;

	g_billboardObjectOrTypeIndex = objectIndex;
	g_currentModelObjectGenus = g_objectTable[objectIndex].genusId;
	meshCount = (uint16_t)ModelMesh_GetCachedObjectTypeMeshCount(g_objectTable[objectIndex].objectType);
	savedDetailSetting = g_drawMarkingsFlag;
	if (meshCount > 0) {
		const ObjectRecord* object = &g_objectTable[objectIndex];
		unsigned int remainingMeshCount;
		for (meshIndex = 0, remainingMeshCount = meshCount; remainingMeshCount > 0;
			 ++meshIndex, --remainingMeshCount)
			RenderScene_DrawNoAssetSourceModel(object, meshIndex);
	}
	g_drawMarkingsFlag = savedDetailSetting;
}

// FUNCTION: XW 0x408B90
void RenderScene_QueueCraftDamageBillboards(uint16_t objectIndex) {
	if (objectIndex >= XW_MISSION_OBJECT_REF_BASE)
		RenderScene_QueueCraftDamageBillboardsForObjectType(
			objectIndex, g_missionObjects[objectIndex - XW_MISSION_OBJECT_REF_BASE].objectType);
	else
		RenderScene_QueueCraftDamageBillboardsForObjectType(objectIndex,
															g_objectTable[objectIndex].objectType);
}

// FUNCTION: XW 0x408BE0
void RenderScene_QueueCraftDamageBillboardsForObjectType(uint16_t objectIndex, uint16_t objectType) {
	uint16_t savedHighlight;
	uint16_t meshIndex;
	int16_t rotationAngle;
	int16_t rotationReady = 0;
	g_billboardObjectOrTypeIndex = objectIndex;
	savedHighlight = g_targetHighlightObjectAndBlinkBits;
	g_damageBillboardMeshCount = ModelMesh_GetCachedObjectTypeMeshCount(objectType);
	rotationAngle = savedHighlight;
	for (meshIndex = 0; meshIndex < g_damageBillboardMeshCount; ++meshIndex) {
		int16_t meshType;
		uint16_t componentState = 0;
		meshType = ModelMesh_GetCachedObjectTypeMeshType(objectType, meshIndex);
		if (objectIndex < XW_CRAFT_OBJECT_COUNT) {
			CraftData* craft = (CraftData*)g_objectTable[objectIndex].instanceData;
			if (craft)
				componentState = craft->componentState[meshIndex];
		}
		if (componentState == 0 && meshType == RENDER_DAMAGE_MESH_TYPE &&
			objectIndex < XW_CRAFT_OBJECT_COUNT) {
			CraftData* craft;
			uint16_t animationIndex = 0;
			uint16_t meshCount = g_damageBillboardMeshCount;
			uint16_t frameCode;
			craft = (CraftData*)g_objectTable[objectIndex].instanceData;
			if (craft)
				animationIndex = craft->componentState[meshCount];
			frameCode = g_componentDamageAnimationFrames[animationIndex];
			if (frameCode >= RENDER_DAMAGE_FIRST_FRAME && frameCode < ANIM_FRAME_JUMP_BASE) {
				int screenX;
				int screenY;
				int screenHigh;
				if (!rotationReady) {
					int absRow0Z = g_objViewMat_R0_Z;
					int absRow1Z = g_objViewMat_R1_Z;
					int axisX;
					int axisY;
					int angle;
					if (absRow0Z < 0)
						absRow0Z = (int32_t)(0u - (uint32_t)absRow0Z);
					if (absRow1Z < 0)
						absRow1Z = (int32_t)(0u - (uint32_t)absRow1Z);
					if (absRow0Z < absRow1Z) {
						axisX = g_objViewMat_R0_X;
						axisY = g_objViewMat_R0_Y;
					} else {
						axisX = g_objViewMat_R1_X;
						axisY = g_objViewMat_R1_Y;
					}
					if (axisX < 0)
						angle = trig2_arctan(axisY, (int32_t)(0u - (uint32_t)axisX));
					else
						angle = -trig2_arctan(axisY, axisX);
					rotationAngle = angle;
					rotationReady = 1;
				}
				rotationAngle += g_objectTable[objectIndex].roll;
				screenX = transfm2_getscreencoordx(g_objectViewX, g_objectViewZ);
				screenHigh = screenX & RENDER_DAMAGE_SCREEN_HIGH_MASK;
				if (screenHigh <= 0 && screenHigh >= RENDER_DAMAGE_SCREEN_HIGH_MASK) {
					screenY = transfm2_getscreencoordy(g_objectViewY, g_objectViewZ);
					screenHigh = screenY & RENDER_DAMAGE_SCREEN_HIGH_MASK;
					if (screenHigh <= 0 && screenHigh >= RENDER_DAMAGE_SCREEN_HIGH_MASK)
						anim_add_bitmap_draw(g_billboardObjectOrTypeIndex, frameCode,
											 RENDER_DAMAGE_SCREEN_SCALE, (int16_t)screenX,
											 2 * (g_flightVpHeight >> 1) - screenY, g_objectViewZ,
											 rotationAngle);
				}
			}
		}
	}
	g_targetHighlightObjectAndBlinkBits = savedHighlight;
}

// FUNCTION: XW 0x408E00
void RenderScene_DrawRollAlignedObjectModel(uint16_t objectIndex) {
	ObjectRecord* object;
	int cameraDeltaX, cameraDeltaY, cameraDeltaZ;
	uint32_t sideProjection, upProjection;
	int sideX, sideY, sideZ, upX, upY, upZ;
	int16_t savedRoll;
	g_billboardObjectOrTypeIndex = objectIndex;
	object = &g_objectTable[objectIndex];
	cameraDeltaX = (int32_t)((uint32_t)g_flightCamera.worldPosition.x - (uint32_t)object->worldX);
	cameraDeltaY = (int32_t)((uint32_t)g_flightCamera.worldPosition.y - (uint32_t)object->worldY);
	cameraDeltaZ = (int32_t)((uint32_t)g_flightCamera.worldPosition.z - (uint32_t)object->worldZ);
	sideX = object->cachedSideX;
	sideY = object->cachedSideY;
	sideZ = object->cachedSideZ;
	sideProjection = (uint32_t)((uint64_t)((int64_t)sideX * cameraDeltaX) >> TRANSFM2_MATRIX_FRACTION_BITS);
	sideProjection += (uint32_t)((uint64_t)((int64_t)sideY * cameraDeltaY) >> TRANSFM2_MATRIX_FRACTION_BITS);
	sideProjection += (uint32_t)((uint64_t)((int64_t)sideZ * cameraDeltaZ) >> TRANSFM2_MATRIX_FRACTION_BITS);
	upX = object->cachedUpX;
	upY = object->cachedUpY;
	upZ = object->cachedUpZ;
	upProjection = (uint32_t)((uint64_t)((int64_t)upX * cameraDeltaX) >> TRANSFM2_MATRIX_FRACTION_BITS);
	upProjection += (uint32_t)((uint64_t)((int64_t)upY * cameraDeltaY) >> TRANSFM2_MATRIX_FRACTION_BITS);
	upProjection += (uint32_t)((uint64_t)((int64_t)upZ * cameraDeltaZ) >> TRANSFM2_MATRIX_FRACTION_BITS);
	savedRoll = object->roll;
	object->roll += trig2_arctan((int32_t)upProjection, (int32_t)sideProjection) - TRIG2_QUARTER_TURN;
	object->orientMatrixDirty = 1;
	fview_newcalcrotate(object->roll, object->pitch, object->yaw, 0, object);
	RenderScene_DrawObjectModel(object);
	object->roll = savedRoll;
	object->orientMatrixDirty = 1;
}

// FUNCTION: XW 0x47CDE0
void RenderScene_ComputeVertexLighting(struct SceneMesh* mesh, struct ProjVertex* outVert,
									   const struct OptVector* normal, const struct OptVector* pos,
									   const struct OptVector* eyePos) {
	int genusId = mesh->pObject->genusId;
	int lightIndex;
	OptVector segmentEnd;
	if (genusId == XW_GENUS_OTHER_PROJECTILE || genusId == XW_GENUS_PLAYER_PROJECTILE) {
		outVert->lightIntensity = 1.0f;
		return;
	}
	if (g_dirLightingEnabled) {
		OptVector direction;
		float directionalDot;
		direction.x = g_objectLightDirectionX * g_lightDirectionQ15ToFloat;
		direction.y = g_objectLightDirectionY * g_lightDirectionQ15ToFloat;
		direction.z = g_objectLightDirectionZ * g_lightDirectionQ15ToFloat;
		directionalDot = direction.x * normal->x + direction.y * normal->y;
		outVert->lightIntensity =
			(directionalDot + direction.z * normal->z) * g_hardwareDirectionalLightScale;
		if (outVert->lightIntensity < g_softwareLightZero) {
			outVert->lightIntensity = 0.0f;
		} else {
			segmentEnd.x = g_objectLightDirectionX + pos->x;
			segmentEnd.y = g_objectLightDirectionY + pos->y;
			segmentEnd.z = g_objectLightDirectionZ + pos->z;
			if (RenderScene_IsSegmentOccludedByObjectModel(mesh->pObject, pos, &segmentEnd))
				outVert->lightIntensity = 0.0f;
		}
	} else {
		outVert->lightIntensity = 0.4f;
	}
	for (lightIndex = 0; lightIndex < g_objectPointLightCount; ++lightIndex) {
		OptVector delta;
		float dot, absX, absY, absZ, distance, diffuse, specular, contribution;
		delta.x = g_objectPointLights[lightIndex].x - pos->x;
		delta.y = g_objectPointLights[lightIndex].y - pos->y;
		delta.z = g_objectPointLights[lightIndex].z - pos->z;
		dot = delta.x * normal->x + delta.y * normal->y;
		dot += delta.z * normal->z;
		if (!g_useHardware3D && dot < g_softwareLightZero)
			continue;
		segmentEnd.x = g_objectPointLights[lightIndex].x;
		segmentEnd.y = g_objectPointLights[lightIndex].y;
		segmentEnd.z = g_objectPointLights[lightIndex].z;
		if (RenderScene_IsSegmentOccludedByObjectModel(mesh->pObject, pos, &segmentEnd))
			continue;
		absX = delta.x;
		absY = delta.y;
		absZ = delta.z;
		if (absX < g_softwareLightZero)
			absX = -absX;
		if (absY < g_softwareLightZero)
			absY = -absY;
		if (absZ < g_softwareLightZero)
			absZ = -absZ;
		if (absX >= absY && absX >= absZ)
			distance = (absZ + absY) * g_lightDistanceMinorAxisWeight + absX;
		else if (absY >= absX && absY >= absZ)
			distance = (absZ + absX) * g_lightDistanceMinorAxisWeight + absY;
		else
			distance = (absY + absX) * g_lightDistanceMinorAxisWeight + absZ;
		if (g_useHardware3D) {
			if (dot / distance < g_hardwarePointLightFacingCutoff)
				continue;
			dot = distance * g_hardwareDiffuseDistanceScale;
		}
		diffuse = dot / (distance * distance);
		if (g_specularEnabled) {
			float halfDot, halfLength, cosine;
			delta.x = eyePos->x - pos->x + delta.x;
			delta.y = eyePos->y - pos->y + delta.y;
			delta.z = eyePos->z - pos->z + delta.z;
			halfDot = delta.x * normal->x + delta.y * normal->y;
			halfDot = (halfDot + delta.z * normal->z) * g_softwareLightHalf;
			absX = delta.x;
			absY = delta.y;
			absZ = delta.z;
			if (absX < g_softwareLightZero)
				absX = -absX;
			if (absY < g_softwareLightZero)
				absY = -absY;
			if (absZ < g_softwareLightZero)
				absZ = -absZ;
			if (absX >= absY && absX >= absZ)
				halfLength = absX * g_lightHalfVectorMajorAxisWeight +
							 (absZ + absY) * g_lightHalfVectorMinorAxisWeight;
			else if (absY >= absX && absY >= absZ)
				halfLength = absY * g_lightHalfVectorMajorAxisWeight +
							 (absZ + absX) * g_lightHalfVectorMinorAxisWeight;
			else
				halfLength = absZ * g_lightHalfVectorMajorAxisWeight +
							 (absY + absX) * g_lightHalfVectorMinorAxisWeight;
			cosine = halfDot / halfLength;
			if (cosine >= g_softwareLightHalf) {
				specular = cosine * cosine * cosine;
				specular *= specular;
				specular *= specular;
				specular *= specular;
				specular *= specular;
			} else {
				specular = g_softwareLightZero;
			}
		} else {
			specular = g_softwareLightZero;
		}
		contribution = diffuse + specular;
		if (contribution > g_softwareLightZero) {
			outVert->lightIntensity += contribution * g_objectPointLights[lightIndex].intensity;
			if (outVert->lightIntensity >= g_softwareLightOne) {
				outVert->lightIntensity = 1.0f;
				return;
			}
		}
	}
}

// FUNCTION: XW 0x47D9B0
void RenderScene_TransformFaceTextureGradients(struct SceneFace* face,
											   const struct FaceTextureGradients* faceTexGradients,
											   const float* viewPosAndOrient) {
	face->gradients[0] = faceTexGradients->gradient0[0];
	face->gradients[1] = faceTexGradients->gradient0[1];
	face->gradients[2] = faceTexGradients->gradient0[2];
	Math3D_RotateVec3(face->gradients, &viewPosAndOrient[RENDER_SCENE_VIEW_ORIENTATION_INDEX]);
	face->gradients[3] = faceTexGradients->gradient1[0];
	face->gradients[4] = faceTexGradients->gradient1[1];
	face->gradients[5] = faceTexGradients->gradient1[2];
	Math3D_RotateVec3(&face->gradients[3], &viewPosAndOrient[RENDER_SCENE_VIEW_ORIENTATION_INDEX]);
}

// FUNCTION: XW 0x47F130
int RenderScene_ProjectMeshVertices(struct SceneMesh* mesh) {
	SceneFace* visibleFaces = &g_visFaceList[mesh->firstVisibleFace];
	ProjVertex* projectedVertices = &g_projVertList[g_projVertCount];
	int vertexIndex, faceIndex;
	float position[3];
	float depth;
	int outputIndex = 0;
	float viewTransform[12];

	mesh->vertBaseIndex = g_projVertCount;
	mesh->projVertCursor = 0;
	for (vertexIndex = 0; vertexIndex < mesh->vertexCount; ++vertexIndex)
		g_vertexRemap[vertexIndex] = OPT_INDEX_NONE;
	for (faceIndex = 0; faceIndex < mesh->visibleFaceCount; ++faceIndex) {
		SceneFace* face = &visibleFaces[faceIndex];
		OptPackedFaceRecord* sourceFace = &mesh->faces[face->faceIndex];
		int corner;
		float sumVertexW;
		viewTransform[0] = mesh->viewPosX;
		viewTransform[1] = mesh->viewPosY;
		viewTransform[2] = mesh->viewPosZ;
		memcpy(&viewTransform[RENDER_SCENE_VIEW_ORIENTATION_INDEX], mesh->viewOrient,
			   sizeof(mesh->viewOrient));
		RenderScene_TransformFaceTextureGradients(face, &mesh->faceTexGradients[face->faceIndex],
												  viewTransform);
		face->maxVertW = 0.0f;
		sumVertexW = 0.0f;
		face->minVertW = (unsigned int)g_projScaleInt;
		for (corner = 0;
			 corner < (int)(sizeof(sourceFace->vertexIndices) / sizeof(sourceFace->vertexIndices[0]));
			 ++corner) {
			int sourceIndex = sourceFace->vertexIndices[corner];
			int normalIndex = sourceFace->normalIndices[corner];
			int texCoordIndex = sourceFace->texCoordIndices[corner];
			if (sourceIndex == OPT_INDEX_NONE)
				break;
			if (g_vertexRemap[sourceIndex] == OPT_INDEX_NONE) {
				ProjVertex* output = &projectedVertices[outputIndex];
				g_vertexRemap[sourceIndex] = mesh->projVertCursor;
				++mesh->projVertCursor;
				position[0] = mesh->vertices[sourceIndex].x;
				position[1] = mesh->vertices[sourceIndex].y;
				position[2] = mesh->vertices[sourceIndex].z;
				Math3D_RotateVec3(position, mesh->viewOrient);
				position[0] = mesh->viewPosX + position[0];
				position[1] = mesh->viewPosY + position[1];
				position[2] = mesh->viewPosZ + position[2];
				if (position[2] >= 1.0f) {
					output->depth = (unsigned int)g_projScaleInt / position[2];
					depth = output->depth;
					output->screenX = position[0] * output->depth;
					output->screenY = position[1] * output->depth;
					output->screenX = (g_flightVpWidth >> 1) + output->screenX;
					output->screenY =
						(unsigned int)(g_projOffsetY + (g_flightVpHeight >> 1)) + output->screenY;
				} else {
					output->depth = position[2] - 1.0f;
					output->screenX = position[0];
					output->screenY = position[1];
					face->nearClipState = OPT_INDEX_NONE;
					depth = (unsigned int)g_projScaleInt;
				}
				RenderScene_ComputeVertexLighting(mesh, output, &mesh->vertexNormals[normalIndex],
												  &mesh->vertices[sourceIndex], &g_meshCullEyePosition);
				output->u = mesh->uvs[texCoordIndex].u;
				output->v = mesh->uvs[texCoordIndex].v;
				++outputIndex;
			} else {
				depth = g_projVertList[mesh->vertBaseIndex + g_vertexRemap[sourceIndex]].depth;
				if (!(depth >= 0.0f)) {
					face->nearClipState = OPT_INDEX_NONE;
					depth = (unsigned int)g_projScaleInt;
				}
			}
			sumVertexW += depth;
			if (depth > face->maxVertW)
				face->maxVertW = depth;
			if (depth < face->minVertW)
				face->minVertW = depth;
		}
		if (mesh->uvs != NULL) {
			int texCoordIndex = sourceFace->texCoordIndices[0];
			int sourceIndex = sourceFace->vertexIndices[0];
			float uCofactor[3], vCofactor[3], planeCofactor[3];
			float inverseDeterminant;
			float projectedInverse, footprint;
			position[0] = mesh->vertices[sourceIndex].x;
			position[1] = mesh->vertices[sourceIndex].y;
			position[2] = mesh->vertices[sourceIndex].z;
			Math3D_RotateVec3(position, mesh->viewOrient);
			position[0] += mesh->viewPosX;
			position[1] = mesh->viewPosY + position[1];
			position[2] = mesh->viewPosZ + position[2];
			face->depthPlane[0] = position[0] - face->gradients[0] * mesh->uvs[texCoordIndex].u -
								  mesh->uvs[texCoordIndex].v * face->gradients[3];
			face->depthPlane[1] = position[1] - face->gradients[1] * mesh->uvs[texCoordIndex].u -
								  mesh->uvs[texCoordIndex].v * face->gradients[4];
			face->depthPlane[2] = position[2] - face->gradients[2] * mesh->uvs[texCoordIndex].u -
								  mesh->uvs[texCoordIndex].v * face->gradients[5];
			uCofactor[0] =
				face->depthPlane[2] * face->gradients[4] - face->gradients[5] * face->depthPlane[1];
			uCofactor[1] =
				face->gradients[5] * face->depthPlane[0] - face->depthPlane[2] * face->gradients[3];
			uCofactor[2] =
				face->depthPlane[1] * face->gradients[3] - face->gradients[4] * face->depthPlane[0];
			vCofactor[0] =
				face->depthPlane[1] * face->gradients[2] - face->depthPlane[2] * face->gradients[1];
			vCofactor[1] =
				face->depthPlane[2] * face->gradients[0] - face->gradients[2] * face->depthPlane[0];
			vCofactor[2] =
				face->depthPlane[0] * face->gradients[1] - face->depthPlane[1] * face->gradients[0];
			planeCofactor[0] =
				face->gradients[5] * face->gradients[1] - face->gradients[2] * face->gradients[4];
			planeCofactor[1] =
				face->gradients[2] * face->gradients[3] - face->gradients[5] * face->gradients[0];
			planeCofactor[2] =
				face->gradients[4] * face->gradients[0] - face->gradients[1] * face->gradients[3];
			if (planeCofactor[0] == 0.0f && planeCofactor[1] == 0.0f && planeCofactor[2] == 0.0f)
				planeCofactor[2] = 1.0f;
			inverseDeterminant =
				1.0f / ((planeCofactor[2] * face->depthPlane[2] + planeCofactor[0] * face->depthPlane[0]) +
						planeCofactor[1] * face->depthPlane[1]);
			projectedInverse = inverseDeterminant * g_invProjScale;
			face->gradients[0] = projectedInverse * uCofactor[0];
			face->gradients[1] = projectedInverse * uCofactor[1];
			face->gradients[2] = inverseDeterminant * uCofactor[2];
			face->gradients[3] = projectedInverse * vCofactor[0];
			face->gradients[4] = projectedInverse * vCofactor[1];
			face->gradients[5] = inverseDeterminant * vCofactor[2];
			face->depthPlane[0] = projectedInverse * planeCofactor[0];
			face->depthPlane[1] = projectedInverse * planeCofactor[1];
			face->depthPlane[2] = inverseDeterminant * planeCofactor[2];
			face->gradients[2] -= (g_flightVpWidth >> 1) * face->gradients[0];
			face->gradients[2] -=
				(unsigned int)(g_projOffsetY + (g_flightVpHeight >> 1)) * face->gradients[1];
			face->gradients[5] -= (g_flightVpWidth >> 1) * face->gradients[3];
			face->gradients[5] -=
				(unsigned int)(g_projOffsetY + (g_flightVpHeight >> 1)) * face->gradients[4];
			face->depthPlane[2] -= (g_flightVpWidth >> 1) * face->depthPlane[0];
			face->depthPlane[2] -=
				(unsigned int)(g_projOffsetY + (g_flightVpHeight >> 1)) * face->depthPlane[1];
			{
				float averageDepth;
				footprint = face->gradients[0] * face->gradients[4] - face->gradients[3] * face->gradients[1];
				if (footprint < 0.0f)
					footprint = -footprint;
				averageDepth =
					(sourceFace->vertexIndices[3] == OPT_INDEX_NONE ? g_hardwareTriangleVertexCountFloat
																	: g_hardwareQuadVertexCountFloat) /
					sumVertexW * (unsigned int)g_projScaleInt;
				face->mipLevel = (int)((int32_t)((uint32_t)(mesh->pMaterial->width * mesh->pMaterial->height)
												 << SW3D_MIP_AREA_SHIFT) *
									   (averageDepth * averageDepth * footprint));
			}
		}
	}
	{
		int appendedCount = mesh->projVertCursor;
		g_projVertCount += appendedCount;
		return appendedCount;
	}
}

// FUNCTION: XW 0x47F8F0
int RenderScene_ProjectDistantMeshVertices(struct SceneMesh* mesh) {
	float projectionScale = (unsigned int)g_projScaleInt / mesh->viewPosZ;
	SceneFace* visibleFaces = &g_visFaceList[mesh->firstVisibleFace];
	ProjVertex* projectedVertices = &g_projVertList[g_projVertCount];
	int vertexIndex, faceIndex;
	float position[3];
	int outputIndex = 0;
	mesh->vertBaseIndex = g_projVertCount;
	mesh->projVertCursor = 0;
	projectionScale *= g_hardwareDistantProjectionScale;
	for (vertexIndex = 0; vertexIndex < mesh->vertexCount; ++vertexIndex)
		g_vertexRemap[vertexIndex] = OPT_INDEX_NONE;
	for (faceIndex = 0; faceIndex < mesh->visibleFaceCount; ++faceIndex) {
		SceneFace* face = &visibleFaces[faceIndex];
		OptPackedFaceRecord* sourceFace = &mesh->faces[face->faceIndex];
		int corner;
		face->maxVertW = 0.0f;
		face->minVertW = (unsigned int)g_projScaleInt;
		for (corner = 0;
			 corner < (int)(sizeof(sourceFace->vertexIndices) / sizeof(sourceFace->vertexIndices[0]));
			 ++corner) {
			int sourceIndex = sourceFace->vertexIndices[corner];
			int texCoordIndex = sourceFace->texCoordIndices[corner];
			int normalIndex = sourceFace->normalIndices[corner];
			float depth;
			if (sourceIndex == OPT_INDEX_NONE)
				break;
			if (g_vertexRemap[sourceIndex] == OPT_INDEX_NONE) {
				ProjVertex* output = &projectedVertices[outputIndex];
				g_vertexRemap[sourceIndex] = mesh->projVertCursor;
				++mesh->projVertCursor;
				position[0] = mesh->vertices[sourceIndex].x;
				position[1] = mesh->vertices[sourceIndex].y;
				position[2] = mesh->vertices[sourceIndex].z;
				Math3D_RotateVec3(position, mesh->viewOrient);
				position[0] = mesh->viewPosX + position[0];
				position[1] = mesh->viewPosY + position[1];
				position[2] = position[2] + mesh->viewPosZ - g_hardwareDistantDepthSubtract;
				output->depth = projectionScale / position[2];
				depth = output->depth;
				output->screenX = position[0] * output->depth;
				output->screenY = position[1] * output->depth;
				output->screenX = (g_flightVpWidth >> 1) + output->screenX;
				output->screenY = (unsigned int)(g_projOffsetY + (g_flightVpHeight >> 1)) + output->screenY;
				RenderScene_ComputeVertexLighting(mesh, output, &mesh->vertexNormals[normalIndex],
												  &mesh->vertices[sourceIndex], &g_meshCullEyePosition);
				output->u = mesh->uvs[texCoordIndex].u;
				output->v = mesh->uvs[texCoordIndex].v;
				++outputIndex;
			} else {
				depth = g_projVertList[mesh->vertBaseIndex + g_vertexRemap[sourceIndex]].depth;
			}
			if (depth > face->maxVertW)
				face->maxVertW = depth;
			if (depth < face->minVertW)
				face->minVertW = depth;
		}
	}
	g_projVertCount += mesh->projVertCursor;
	return g_projVertCount;
}

// FUNCTION: XW 0x47FBA0
void RenderScene_DrawMeshFaces(struct SceneMesh* mesh) {
	ProjVertex* vertices = &g_projVertList[mesh->vertBaseIndex];
	SceneFace* visibleFaces;
	int* emittedVertexMap;
	const uint8_t* previousMipTexels = NULL;
	Std3DTexCacheNode* opaqueTexture;
	Std3DTexCacheNode* colorKeyTexture;
	int faceIndex, index;
#ifdef XW_MODERN
	/* Suppress before texture lookup so hidden classic draws do not refill the cache. */
	if (AeronDx5_IsClassicFlightRenderingSuppressed())
		return;
	opaqueTexture = NULL;
	colorKeyTexture = NULL;
#endif
	g_meshReusableProjectedVertexCount = mesh->vertBaseIndex + mesh->projVertCursor;
	g_clipVertCursor = g_meshReusableProjectedVertexCount;
	emittedVertexMap = (int*)g_sceneEdgeList;
	visibleFaces = &g_visFaceList[mesh->firstVisibleFace];
	for (index = 0; index < g_meshReusableProjectedVertexCount; ++index)
		emittedVertexMap[index] = OPT_INDEX_NONE;
	for (faceIndex = 0; faceIndex < mesh->visibleFaceCount; ++faceIndex) {
		SceneFace* visibleFace = &visibleFaces[faceIndex];
		OptPackedFaceRecord* face = &mesh->faces[visibleFace->faceIndex];
		int cornerCount = (face->edgeIndices[3] != OPT_INDEX_NONE) + 3;
		g_clipCountA = cornerCount;
		if (g_pStd3DCurDevice->caps.bSquareOnlyTexture) {
			int width = mesh->pMaterial->width;
			int height = mesh->pMaterial->height;
			float scale = 1.0f;
			if (width > height) {
				int dimension = height;
				do {
					scale *= 0.5f;
					dimension <<= 1;
				} while (dimension < width);
			} else if (width < height) {
				int dimension = width;
				do {
					scale *= 0.5f;
					dimension <<= 1;
				} while (dimension < height);
			}
			for (index = 0; index < cornerCount; ++index) {
				double u = mesh->uvs[face->texCoordIndices[index]].u;
				float v = mesh->uvs[face->texCoordIndices[index]].v;
				int vertexIndex;
				ProjVertex* source;
				if (width > height)
					v = (float)(scale * v);
				else if (width < height)
					u *= scale;
				vertexIndex = g_vertexRemap[face->vertexIndices[index]];
				g_clipIdxA[index] = vertexIndex;
				source = &vertices[vertexIndex];
				if (u != source->u || source->v != v) {
					int seamIndex = g_clipVertCursor++;
					ProjVertex* seam = &vertices[seamIndex];
					seam->screenX = source->screenX;
					seam->screenY = source->screenY;
					seam->lightIntensity = source->lightIntensity;
					seam->depth = source->depth;
					seam->u = (float)u;
					seam->v = v;
					g_clipIdxA[index] = seamIndex;
				}
			}
		} else {
			for (index = 0; index < cornerCount; ++index) {
				int vertexIndex = g_vertexRemap[face->vertexIndices[index]];
				ProjVertex* source;
				OptTexCoord* uv;
				g_clipIdxA[index] = vertexIndex;
				source = &vertices[vertexIndex];
				uv = &mesh->uvs[face->texCoordIndices[index]];
				if (source->u != uv->u || source->v != uv->v) {
					int seamIndex = g_clipVertCursor++;
					ProjVertex* seam = &vertices[seamIndex];
					seam->screenX = source->screenX;
					seam->screenY = source->screenY;
					seam->lightIntensity = source->lightIntensity;
					seam->depth = source->depth;
					seam->u = mesh->uvs[face->texCoordIndices[index]].u;
					seam->v = mesh->uvs[face->texCoordIndices[index]].v;
					g_clipIdxA[index] = seamIndex;
				}
			}
		}
		if (visibleFace->nearClipState == OPT_INDEX_NONE) {
			if (g_clipCountA > 0)
				memcpy(g_clipIdxB, g_clipIdxA, g_clipCountA * sizeof(g_clipIdxA[0]));
			g_clipCountB = g_clipCountA;
			g_clipCountA = 0;
			if (g_clipCountB > 0) {
				int previous = g_clipIdxB[g_clipCountB - 1];
				for (index = 0; index < g_clipCountB; ++index) {
					int current = g_clipIdxB[index];
					RenderClip_ClipPolyNear(previous, current, vertices);
					previous = current;
				}
			}
		}
		visibleFace->nearClipState = g_flightVpHeight;
		g_clipCountB = 0;
		if (g_clipCountA > 0) {
			int previous = g_clipIdxA[g_clipCountA - 1];
			for (index = 0; index < g_clipCountA; ++index) {
				int current = g_clipIdxA[index];
				RenderClip_ClipPolyTop(previous, current, vertices);
				previous = current;
			}
		}
		g_clipCountA = 0;
		if (g_clipCountB > 0) {
			int previous = g_clipIdxB[g_clipCountB - 1];
			for (index = 0; index < g_clipCountB; ++index) {
				int current = g_clipIdxB[index];
				RenderClip_ClipPolyBottom(previous, current, vertices);
				previous = current;
			}
		}
		g_clipCountB = 0;
		if (g_clipCountA > 0) {
			int previous = g_clipIdxA[g_clipCountA - 1];
			for (index = 0; index < g_clipCountA; ++index) {
				int current = g_clipIdxA[index];
				RenderClip_ClipPolyLeft(previous, current, vertices);
				previous = current;
			}
		}
		g_clipCountA = 0;
		if (g_clipCountB > 0) {
			int previous = g_clipIdxB[g_clipCountB - 1];
			for (index = 0; index < g_clipCountB; ++index) {
				int current = g_clipIdxB[index];
				RenderClip_ClipPolyRight(previous, current, vertices);
				previous = current;
			}
		}
		for (index = 0; index < g_clipCountA; ++index) {
			int vertexIndex = g_clipIdxA[index];
			if (vertexIndex >= g_meshReusableProjectedVertexCount)
				g_clipIdxA[index] = RenderScene_EmitFlightVertex(vertexIndex, vertices);
			else {
				if (emittedVertexMap[vertexIndex] == OPT_INDEX_NONE)
					emittedVertexMap[g_clipIdxA[index]] = RenderScene_EmitFlightVertex(vertexIndex, vertices);
				g_clipIdxA[index] = emittedVertexMap[g_clipIdxA[index]];
			}
		}
		if (g_clipCountA > 2) {
			int mipByteOffset = 0;
			int mipWidth = mesh->pMaterial->width;
			int mipHeight = mesh->pMaterial->height;
			const uint8_t* mipTexels;
			if (mipWidth * mipHeight == mesh->pMaterial->textureSize) {
				int footprint = (int)(int64_t)(visibleFace->mipLevel * (double)g_textureMipScale);
				for (; footprint > RENDER_MESH_MIP_FOOTPRINT_LIMIT; mipHeight >>= 1) {
					if (mipWidth == RENDER_MESH_MIN_MIP_DIMENSION)
						break;
					if (mipHeight == RENDER_MESH_MIN_MIP_DIMENSION)
						break;
					footprint >>= RENDER_MESH_MIP_AREA_SHIFT;
					mipByteOffset += mipWidth * mipHeight;
					mipWidth >>= 1;
				}
			}
			mipTexels = &mesh->pTexels[mipByteOffset];
			if (mipTexels != previousMipTexels) {
				uint16_t* palette = mesh->pColorKeyPalette + RENDER_MESH_OPAQUE_PALETTE_OFFSET;
				previousMipTexels = mipTexels;
				opaqueTexture = RenderTexture_GetOrCreateOpaque(mipWidth, mipHeight, palette, mipTexels);
				colorKeyTexture = NULL;
				if (palette[STD3D_PALETTE_COLOR_COUNT] != 0) {
					uint8_t genus = mesh->pObject->genusId;
					if (genus == XW_GENUS_PLAYER_PROJECTILE || genus == XW_GENUS_OTHER_PROJECTILE)
						palette[STD3D_PALETTE_COLOR_COUNT] = 0;
					else if (mesh->pTextureName != NULL &&
							 mesh->pTextureName[0] != RENDER_MESH_NO_COLOR_KEY_PREFIX) {
						colorKeyTexture = RenderTexture_GetOrCreateColorKey(
							mipWidth, mipHeight, mesh->pColorKeyPalette, mipTexels);
						if (colorKeyTexture == NULL && mipWidth == mesh->pMaterial->width &&
							mipHeight == mesh->pMaterial->height)
							mesh->pTextureName[0] = RENDER_MESH_NO_COLOR_KEY_PREFIX;
					}
				}
			}
		}
		if (colorKeyTexture != NULL) {
			int firstVertex;
			for (index = 0; index < g_clipCountA; ++index) {
				g_flightVertexBuffer[index + g_d3dVertexCount] = g_flightVertexBuffer[g_clipIdxA[index]];
				g_flightVertexBuffer[index + g_d3dVertexCount].color = 0xFFFFFFFFu;
			}
			firstVertex = g_d3dVertexCount;
			g_d3dVertexCount += g_clipCountA;
			for (index = 2; index < g_clipCountA; ++index) {
				g_triBuffer[g_d3dIndexCount].v0 = firstVertex;
				g_triBuffer[g_d3dIndexCount].v1 = firstVertex + index - 1;
				g_triBuffer[g_d3dIndexCount].v2 = firstVertex + index;
				g_triBuffer[g_d3dIndexCount].texture = colorKeyTexture;
				g_triBuffer[g_d3dIndexCount].flags = STD3D_RS_DISABLE_MONO | STD3D_RS_Z_WRITE |
													 STD3D_RS_Z_TEST | STD3D_RS_SUBPIXEL | STD3D_RS_DITHER |
													 STD3D_RS_TEXTURE_PERSPECTIVE;
				if (g_bilinearEnabled)
					g_triBuffer[g_d3dIndexCount].flags += STD3D_RS_MIN_LINEAR | STD3D_RS_MAG_LINEAR;
				g_triBuffer[g_d3dIndexCount].flags += STD3D_RS_ALPHA_BLEND;
				++g_d3dIndexCount;
			}
		}
		for (index = 2; index < g_clipCountA; ++index) {
			g_triBuffer[g_d3dIndexCount].v0 = g_clipIdxA[0];
			g_triBuffer[g_d3dIndexCount].v1 = g_clipIdxA[index - 1];
			g_triBuffer[g_d3dIndexCount].v2 = g_clipIdxA[index];
			g_triBuffer[g_d3dIndexCount].texture = opaqueTexture;
			g_triBuffer[g_d3dIndexCount].flags = STD3D_RS_DISABLE_MONO | STD3D_RS_Z_WRITE | STD3D_RS_Z_TEST |
												 STD3D_RS_SUBPIXEL | STD3D_RS_DITHER |
												 STD3D_RS_TEXTURE_PERSPECTIVE;
			if (g_bilinearEnabled)
				g_triBuffer[g_d3dIndexCount].flags += STD3D_RS_MIN_LINEAR | STD3D_RS_MAG_LINEAR;
			if (g_capVertexAlpha) {
				g_triBuffer[g_d3dIndexCount].flags += STD3D_RS_ALPHA_BLEND;
				g_capVertexAlpha = 0;
			}
			++g_d3dIndexCount;
		}
	}
}

// FUNCTION: XW 0x4820D0
void RenderScene_DrawMesh(const struct SceneMesh* mesh) {
	int savedVisibleFaceCount;
	g_projVertCount = 0;
	g_sceneEdgeCursor = 0;
	savedVisibleFaceCount = g_visFaceCount;
	if (g_meshQueueIndex != g_meshQueueMax && g_visFaceCount + mesh->faceCount <= g_sceneFaceMax &&
		mesh->vertexCount <= g_projVertMax && mesh->edgeCount <= g_sceneEdgeMax) {
		SceneMesh* queuedMesh;
		int visibleFaceCount;
		memcpy(&g_meshQueue[g_meshQueueIndex], mesh, sizeof(*mesh));
		queuedMesh = &g_meshQueue[g_meshQueueIndex];
		RenderScene_CullMeshFacesFromView(queuedMesh);
		visibleFaceCount = queuedMesh->visibleFaceCount;
		if (visibleFaceCount != 0) {
			if (g_d3dVertexCount + RENDER_HARDWARE_FACE_VERTEX_BUDGET * visibleFaceCount > g_maxBatchVerts ||
				(int)(g_d3dIndexCount + RENDER_HARDWARE_FACE_TRIANGLE_BUDGET * visibleFaceCount) >
					g_maxBatchTris) {
				Math_SetFpuExtendedPrecisionMode();
				if (!g_isPowerVr)
					std3D_StartScene();
				std3D_LockExecuteBuffer();
				std3D_AddVertices(g_flightVertexBuffer, g_d3dVertexCount);
				std3D_BeginInstructions();
				std3D_AddTriangles(g_triBuffer, g_d3dIndexCount);
				std3D_ExecuteBuffer();
				if (!g_isPowerVr)
					std3D_EndScene();
				Math_SetFpuSinglePrecisionMode();
				g_d3dIndexCount = 0;
				g_d3dVertexCount = 0;
			}
			if (g_bBackdropMeshMode)
				RenderScene_ProjectDistantMeshVertices(queuedMesh);
			else
				RenderScene_ProjectMeshVertices(queuedMesh);
			RenderScene_DrawMeshFaces(queuedMesh);
			g_visFaceCount = savedVisibleFaceCount;
		}
	}
}

// FUNCTION: XW 0x482240
void RenderScene_InitHardwareFrame(void) {
	unsigned int originX;
	unsigned int originY;
	if (g_isPowerVr != 0 && g_powerVrBeginScenePending != 0) {
		Math_SetFpuExtendedPrecisionMode();
		std3D_StartScene();
		Math_SetFpuSinglePrecisionMode();
		g_powerVrBeginScenePending = 0;
	}
	originX = (unsigned int)(g_flightDisplayWidth - g_surfaceWidth) >> 1;
	originY = (unsigned int)(g_flightDisplayHeight - g_surfaceHeight) >> 1;
	g_flightVpOriginX = (float)(g_flightVpX + originX);
	g_flightVpOriginY = (float)(g_flightVpY + originY);
	g_d3dIndexCount = 0;
	g_d3dVertexCount = 0;
	g_hardwareFrameField_55CAE8 = 0;
	g_capVertexAlpha = 1;
	g_maxBatchVerts = (unsigned int)(g_sceneSpanDataCapacity * sizeof(SceneSpan)) /
					  (RENDER_HARDWARE_BUFFER_BUDGET_DIVISOR * sizeof(D3DTLVERTEX));
	g_maxBatchTris = (unsigned int)(g_sceneSpanDataCapacity * sizeof(SceneSpan)) /
					 RENDER_HARDWARE_BUFFER_BUDGET_DIVISOR / sizeof(Std3DRenderTri);
	if ((unsigned int)g_maxBatchVerts > g_pStd3DCurDevice->caps.maxVertexCount)
		g_maxBatchVerts = g_pStd3DCurDevice->caps.maxVertexCount;
	if (g_maxBatchVerts > RENDER_HARDWARE_BATCH_LIMIT)
		g_maxBatchVerts = RENDER_HARDWARE_BATCH_LIMIT;
	if (g_maxBatchTris > RENDER_HARDWARE_BATCH_LIMIT)
		g_maxBatchTris = RENDER_HARDWARE_BATCH_LIMIT;
	if (g_maxBatchTris > (int)((unsigned int)(g_pStd3DCurDevice->caps.maxBufferSize -
											  g_maxBatchVerts * (RENDER_HARDWARE_VERTEX_EXEC_MULTIPLIER *
																 sizeof(D3DTLVERTEX))) /
							   RENDER_HARDWARE_TRIANGLE_EXEC_BYTES)) {
		g_maxBatchTris =
			(unsigned int)(g_pStd3DCurDevice->caps.maxBufferSize -
						   g_maxBatchVerts * (RENDER_HARDWARE_VERTEX_EXEC_MULTIPLIER * sizeof(D3DTLVERTEX))) /
			RENDER_HARDWARE_TRIANGLE_EXEC_BYTES;
	}
	g_flightVertexBuffer = (D3DTLVERTEX*)g_sceneSpanDataBase;
	g_triBuffer =
		(Std3DRenderTri*)&g_sceneSpanDataBase[g_sceneSpanDataCapacity / RENDER_HARDWARE_BUFFER_PARTS];
}

// FUNCTION: XW 0x482390
void RenderScene_FlushGeometry(void) {
	if (g_sceneFlushDrawTargetMarkers != 0) {
		anim_sort_and_draw_bitmaps(1);
		Targeting_w_DrawObjectBox();
	} else {
		anim_sort_and_draw_bitmaps(0);
	}
	g_sceneBillboardQueueCount = 0;
	if (g_d3dVertexCount != 0 && g_d3dIndexCount != 0) {
		Math_SetFpuExtendedPrecisionMode();
		if (!g_isPowerVr) {
			std3D_StartScene();
		} else if (g_powerVrBeginScenePending != 0) {
			std3D_StartScene();
			g_powerVrBeginScenePending = 0;
		}
		std3D_LockExecuteBuffer();
		std3D_AddVertices(g_flightVertexBuffer, g_d3dVertexCount);
		std3D_BeginInstructions();
		std3D_AddTriangles(g_triBuffer, g_d3dIndexCount);
		std3D_ExecuteBuffer();
		if (!g_isPowerVr)
			std3D_EndScene();
		Math_SetFpuSinglePrecisionMode();
	}
	if (g_isPowerVr) {
		Math_SetFpuExtendedPrecisionMode();
		std3D_EndScene();
		g_powerVrBeginScenePending = 1;
		Math_SetFpuSinglePrecisionMode();
	}
}

// FUNCTION: XW 0x482470
int RenderScene_EmitFlightVertex(int vertexIndex, const struct ProjVertex* vertices) {
	float screenX = vertices[vertexIndex].screenX;
	float screenY = vertices[vertexIndex].screenY;
	float depth = vertices[vertexIndex].depth;
	float lightIntensity = vertices[vertexIndex].lightIntensity;
	float u = vertices[vertexIndex].u;
	float v = vertices[vertexIndex].v;
	float normalizedDepth;
	int shade;

	if (depth < 0.0f)
		depth = (unsigned int)g_projScaleInt;
	normalizedDepth = 1.0f / ((unsigned int)g_projScaleInt / depth * g_hardwareDepthReciprocalScale + 1.0f);
	if (g_std3DZCmpMask == STD3D_ZCMP_LESS)
		normalizedDepth = 1.0f - normalizedDepth;
	g_flightVertexBuffer[g_d3dVertexCount].sx = screenX + g_flightVpOriginX;
	g_flightVertexBuffer[g_d3dVertexCount].sy = screenY + g_flightVpOriginY;
	g_flightVertexBuffer[g_d3dVertexCount].sz = normalizedDepth;
	g_flightVertexBuffer[g_d3dVertexCount].rhw = depth;
	g_flightVertexBuffer[g_d3dVertexCount].tu = u;
	g_flightVertexBuffer[g_d3dVertexCount].tv = v;
	shade = RENDER_HARDWARE_SHADE_BASE - (int)(lightIntensity * g_hardwareShadeIntensityNegativeScale);
	if (shade > RENDER_HARDWARE_SHADE_MAX)
		shade = RENDER_HARDWARE_SHADE_MAX;
	if (g_capVertexAlpha)
		g_flightVertexBuffer[g_d3dVertexCount].color =
			(unsigned int)shade * RENDER_HARDWARE_GRAYSCALE_MULTIPLIER +
			((unsigned int)RENDER_HARDWARE_VERTEX_ALPHA << RENDER_HARDWARE_ALPHA_SHIFT);
	else
		g_flightVertexBuffer[g_d3dVertexCount].color =
			(unsigned int)shade * RENDER_HARDWARE_GRAYSCALE_MULTIPLIER +
			((unsigned int)RENDER_HARDWARE_OPAQUE_ALPHA << RENDER_HARDWARE_ALPHA_SHIFT);
	g_flightVertexBuffer[g_d3dVertexCount].specular = 0;
	return g_d3dVertexCount++;
}

// FUNCTION: XW 0x482630
void RenderScene_ClearFrameBuffers(void) {
	DDBLTFX blitFx;

	memset(&blitFx, 0, sizeof(blitFx));
	blitFx.dwSize = sizeof(blitFx);
	blitFx.dwROP = DDROP_SRCCOPY;
	blitFx.dwFillColor = g_flightTextPalette[g_flightBackgroundPaletteIndex];
	g_flightBackBuffer->lpVtbl->Blt(g_flightBackBuffer, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT,
									&blitFx);
	std3D_ClearZBuffer();
}

// FUNCTION: XW 0x4862D0
void RenderScene_CullMeshFacesFromView(struct SceneMesh* mesh) {
	OptVector* vertices;
	OptVector* faceNormals;
	OptPackedFaceRecord* faces;
	SceneFace* visibleFaces;
	int visibleFaceIndex;
	int faceIndex;
	mesh->firstVisibleFace = g_visFaceCount;
	g_meshCullEyePosition.x = mesh->posX;
	g_meshCullEyePosition.y = mesh->posY;
	g_meshCullEyePosition.z = mesh->posZ;
	if (g_bBackdropMeshMode != 0) {
		g_meshCullEyePosition.x = 0.0f;
		g_meshCullEyePosition.y = 0.0f;
		g_meshCullEyePosition.z = -RENDER_SCENE_BACKDROP_EYE_DISTANCE;
		Math3D_RotateVec3(&g_meshCullEyePosition.x, mesh->orient);
		g_meshCullEyePosition.x += mesh->posX;
		g_meshCullEyePosition.y += mesh->posY;
		g_meshCullEyePosition.z += mesh->posZ;
	}
	faceNormals = mesh->faceNormals;
	faces = mesh->faces;
	vertices = mesh->vertices;
	visibleFaces = &g_visFaceList[g_visFaceCount];
	visibleFaceIndex = 0;
	for (faceIndex = 0; faceIndex < mesh->faceCount; ++faceIndex) {
		OptVector vertexToEye;
		int firstVertexIndex = faces[faceIndex].vertexIndices[0];
		vertexToEye.x = g_meshCullEyePosition.x - vertices[firstVertexIndex].x;
		vertexToEye.y = g_meshCullEyePosition.y - vertices[firstVertexIndex].y;
		vertexToEye.z = g_meshCullEyePosition.z - vertices[firstVertexIndex].z;
		if (Math3D_Dot3(&vertexToEye.x, &faceNormals[faceIndex].x) >= 0.0f) {
			SceneFace* outputFace = &visibleFaces[visibleFaceIndex];
			outputFace->faceIndex = faceIndex;
			outputFace->mesh = mesh;
			outputFace->lightSamples = &g_scenePhongData[g_phongSlotIndex * (g_phongSlotStride + 1)];
			if (g_phongSlotIndex < RENDER_SCENE_PHONG_SLOT_COUNT - 1) {
				++g_phongSlotIndex;
			}
			outputFace->packedFaceKey = faceIndex + (g_curLayerId << RENDER_SCENE_FACE_KEY_LAYER_SHIFT);
			outputFace->pScanEdge = NULL;
			++visibleFaceIndex;
			++g_visFaceCount;
		}
	}
	mesh->visibleFaceCount = g_visFaceCount - mesh->firstVisibleFace;
}

// FUNCTION: XW 0x488180
void RenderScene_DrawSceneMesh(const struct SceneMesh* mesh) {
	if (g_useHardware3D != 0) {
		RenderScene_DrawMesh(mesh);
	} else {
		g_projVertCount = 0;
		g_sceneEdgeCursor = 0;
		if (g_meshQueueIndex != g_meshQueueMax && g_visFaceCount + mesh->faceCount <= g_sceneFaceMax &&
			mesh->vertexCount <= g_projVertMax && mesh->edgeCount <= g_sceneEdgeMax) {
			SceneMesh* queuedMesh;
			g_meshQueue[g_meshQueueIndex] = *mesh;
			queuedMesh = &g_meshQueue[g_meshQueueIndex];
			RenderScene_CullMeshFacesFromView(queuedMesh);
			if (queuedMesh->visibleFaceCount != 0) {
				if (g_bBackdropMeshMode != 0)
					sw3d_ProjectMeshVerticesDistant(queuedMesh);
				else
					sw3d_ProjectMeshVertices(queuedMesh);
				sw3d_RasterizeMeshFaces(queuedMesh);
				++g_meshQueueIndex;
			}
		}
	}
}

// FUNCTION: XW 0x488700
void RenderScene_ApplyBwingBridgeRotation(void* unusedModel, const struct ObjectRecord* objectRecord,
										  struct SceneMesh* mesh, int bridgeMeshIndex) {
	float axisAngle[RENDER_SCENE_AXIS_ANGLE_COMPONENTS];
	float rotationMatrix[RENDER_SCENE_ROTATION_MATRIX_CAPACITY];
	int bridgeRotationByte = ((const CraftData*)objectRecord->instanceData)->meshRotation[bridgeMeshIndex];
	(void)unusedModel;
	axisAngle[0] = 0.0f;
	axisAngle[1] = -1.0f;
	axisAngle[2] = 0.0f;
	axisAngle[3] = bridgeRotationByte * g_renderRadiansPerAngleByte;
	Math3D_BuildAxisAngleMatrix(rotationMatrix, axisAngle);
	Math3D_MulMatrix3x3(mesh->orient, rotationMatrix);
	Math3D_RotateVec3(&mesh->posX, rotationMatrix);
	Math3D_MulMatrix3x3T(mesh->viewOrient, rotationMatrix);
}

// FUNCTION: XW 0x488790
void RenderScene_DrawObjectModel(const void* objectRecord) {
	const ObjectRecord* object = (const ObjectRecord*)objectRecord;
	uint16_t modelHandle = g_loadedModels[object->objectType];
	OptimizedPolyObject* model;
	float objectViewR0X, objectViewR0Y, objectViewR0Z;
	float objectViewR1X, objectViewR1Y, objectViewR1Z;
	float objectViewR2X, objectViewR2Y, objectViewR2Z;
	SceneMesh mesh, savedMesh;
	int rootIndex, meshOrdinal;
	int restoreBridgeTransform;
	if ((g_modelTypeTable[object->objectType].objectFlags & MODEL_TYPE_FLAG_CRAFT_RECORD) != 0)
		g_nodeSwitchIndex = object->markings;
	else
		g_nodeSwitchIndex = 0;
	model = (OptimizedPolyObject*)Memory_LockHandle(modelHandle);
	if (model->selfMarker != model)
		OptModel_AdjustOptimizedPolyObjectPointers(model);
	memset(&mesh, 0, sizeof(mesh));
	mesh.pObject = (ObjectRecord*)objectRecord;
	if (object->objectType != XW_OBJ_NONE &&
		(g_modelTypeTable[object->objectType].objectFlags & MODEL_TYPE_FLAG_CRAFT_RECORD) == 0) {
		const XwMissionObjectRecord* missionObject = (const XwMissionObjectRecord*)objectRecord;
		mesh.viewPosX = (float)(missionObject->worldX * MISSION_OBJECT_WORLD_COORDINATE_SCALE -
								g_flightCamera.worldPosition.x);
		mesh.viewPosY = (float)(missionObject->worldY * MISSION_OBJECT_WORLD_COORDINATE_SCALE -
								g_flightCamera.worldPosition.y);
		mesh.viewPosZ = (float)(missionObject->worldZ * MISSION_OBJECT_WORLD_COORDINATE_SCALE -
								g_flightCamera.worldPosition.z);
	} else {
		mesh.viewPosX = (float)(object->worldX - g_flightCamera.worldPosition.x);
		mesh.viewPosY = (float)(object->worldY - g_flightCamera.worldPosition.y);
		mesh.viewPosZ = (float)(object->worldZ - g_flightCamera.worldPosition.z);
	}
	mesh.viewOrient[0] = (double)g_camMatR0_X * g_renderSignedBasisToFloat;
	mesh.viewOrient[1] = (double)g_camMatR1_X * g_renderSignedBasisToFloat;
	mesh.viewOrient[2] = (double)g_camMatR2_X * g_renderSignedBasisToFloat;
	mesh.viewOrient[3] = (double)g_camMatR0_Y * g_renderSignedBasisToFloat;
	mesh.viewOrient[4] = (double)g_camMatR1_Y * g_renderSignedBasisToFloat;
	mesh.viewOrient[5] = (double)g_camMatR2_Y * g_renderSignedBasisToFloat;
	mesh.viewOrient[6] = (double)g_camMatR0_Z * g_renderSignedBasisToFloat;
	mesh.viewOrient[7] = (double)g_camMatR1_Z * g_renderSignedBasisToFloat;
	mesh.viewOrient[8] = (double)g_camMatR2_Z * g_renderSignedBasisToFloat;
	Math3D_RotateVec3(&mesh.viewPosX, mesh.viewOrient);

	objectViewR0X = (double)g_objViewMat_R0_X * g_renderSignedBasisToFloat;
	objectViewR0Y = (double)g_objViewMat_R0_Y * g_renderSignedBasisToFloat;
	objectViewR0Z = (double)g_objViewMat_R0_Z * g_renderSignedBasisToFloat;
	objectViewR1X = (double)g_objViewMat_R1_X * g_renderSignedBasisToFloat;
	objectViewR1Y = (double)g_objViewMat_R1_Y * g_renderSignedBasisToFloat;
	mesh.viewOrient[0] = objectViewR0X;
	mesh.viewOrient[1] = objectViewR0Y;
	mesh.viewOrient[2] = objectViewR0Z;
	mesh.viewOrient[3] = objectViewR1X;
	mesh.viewOrient[4] = objectViewR1Y;
	objectViewR1Z = (double)g_objViewMat_R1_Z * g_renderSignedBasisToFloat;
	mesh.viewOrient[5] = objectViewR1Z;
	objectViewR2X = (double)g_objViewMat_R2_X * g_renderSignedBasisToFloat;
	objectViewR2Y = (double)g_objViewMat_R2_Y * g_renderSignedBasisToFloat;
	objectViewR2Z = (double)g_objViewMat_R2_Z * g_renderSignedBasisToFloat;
	mesh.posX = -mesh.viewPosX;
	mesh.posY = -mesh.viewPosY;
	mesh.posZ = -mesh.viewPosZ;
	mesh.orient[0] = objectViewR0X;
	mesh.orient[1] = objectViewR1X;
	mesh.orient[2] = objectViewR2X;
	mesh.orient[3] = objectViewR0Y;
	mesh.viewOrient[6] = objectViewR2X;
	mesh.viewOrient[7] = objectViewR2Y;
	mesh.viewOrient[8] = objectViewR2Z;
	mesh.orient[4] = objectViewR1Y;
	mesh.orient[5] = objectViewR2Y;
	mesh.orient[6] = objectViewR0Z;
	mesh.orient[7] = objectViewR1Z;
	mesh.orient[8] = objectViewR2Z;
	Math3D_RotateVec3(&mesh.posX, mesh.orient);

	g_sourceVectors = NULL;
	if (g_defaultWhiteTextureDescPtr == NULL) {
		g_defaultWhiteTextureDescPtr = &g_defaultWhiteTexture.descriptor;
		g_defaultWhiteTexture.descriptor.height = OPT_TEXTURE_DEFAULT_HEIGHT;
		g_defaultWhiteTexture.descriptor.width = OPT_TEXTURE_DEFAULT_WIDTH;
		g_defaultWhiteTexture.descriptor.paletteType = MODEL_TEXTURE_SHADE_LEVELS;
#ifdef XW_MODERN
		g_defaultWhiteTexture.descriptor.palette = g_defaultWhiteTexture.packedShades;
#else
		g_defaultWhiteTexture.descriptor.palette = (uint16_t*)OPT_TEXTURE_PALETTE_COLOR_COUNT;
#endif
		ModelTexture_BuildPalettedShadeTable(g_defaultWhiteTexture.texels, g_defaultWhiteTextureRgb24,
											 OPT_TEXTURE_DEFAULT_WIDTH, OPT_TEXTURE_DEFAULT_WIDTH);
	}
	g_curTextureDesc = g_defaultWhiteTextureDescPtr;
	g_sourceTexCoords = NULL;
	g_curVertNormals = NULL;
	g_optNodeWalkScratch2 = NULL;
	g_curMeshDescriptorData = NULL;
	g_curVertexCount = 0;

	meshOrdinal = 0;
	restoreBridgeTransform = 0;
	for (rootIndex = 0; rootIndex < model->rootNodeCount; ++rootIndex) {
		OptNode* rootNode;
		mesh.rotAngle = 0.0f;
		rootNode = model->rootNodes[rootIndex];
		if (rootNode->nodeType != OPT_TEXTURE) {
			int meshRotationByte;
			++meshOrdinal;
			if (object->genusId == XW_GENUS_PLAYER_PROJECTILE ||
				object->genusId == XW_GENUS_OTHER_PROJECTILE) {
				meshRotationByte = 0;
				g_nodeSwitchIndex = 0;
			} else {
				if ((g_modelTypeTable[object->objectType].objectFlags & MODEL_TYPE_FLAG_CRAFT_RECORD) != 0) {
					const CraftData* craft = (const CraftData*)object->instanceData;
					if (craft->componentState[meshOrdinal - 1] != 0)
						continue;
					meshRotationByte = craft->meshRotation[meshOrdinal - 1];
				} else {
					meshRotationByte = 0;
				}
				if (object->objectType == XW_OBJ_B_WING) {
					int bridgeIndex = g_bwingBridgeMeshIndex;
					if (bridgeIndex == -1) {
						bridgeIndex = ModelMesh_FindBridgeIndex(model);
						g_bwingBridgeMeshIndex = bridgeIndex;
					}
					if (bridgeIndex != -1 &&
						((const CraftData*)object->instanceData)->meshRotation[bridgeIndex] != 0) {
						savedMesh = mesh;
						restoreBridgeTransform = 1;
						RenderScene_ApplyBwingBridgeRotation(model, object, &mesh, bridgeIndex);
					}
				}
			}
			mesh.rotAngle = meshRotationByte * g_renderRadiansPerAngleByte;
		}
		++g_curLayerId;
		RenderScene_DrawModelNode(model, rootNode, &mesh);
		if (restoreBridgeTransform != 0) {
			mesh = savedMesh;
			restoreBridgeTransform = 0;
		}
	}
	nullsub_SharedNoOp();
}

// FUNCTION: XW 0x488CE0
void RenderScene_DrawNoAssetSourceModel(const struct ObjectRecord* objectRecord, int rootMeshIndex) {
	int objectType;
	uint16_t modelHandle;
	OptimizedPolyObject* model;
	float objectViewR0X;
	float objectViewR0Y;
	float objectViewR0Z;
	float objectViewR1X;
	float objectViewR1Y;
	float objectViewR1Z;
	float objectViewR2X;
	float objectViewR2Y;
	float objectViewR2Z;
	SceneMesh mesh;
	int rootIndex;

	objectType = objectRecord->objectType;
	if (objectType == XW_OBJ_DETACHED_COMPONENT)
		objectType = objectRecord->sourceObjectType;
	g_nodeSwitchIndex = objectRecord->markings;

	modelHandle = g_loadedModels[objectType];
	model = (OptimizedPolyObject*)Memory_LockHandle(modelHandle);
	if (model->selfMarker != model) {
		OptModel_AdjustOptimizedPolyObjectPointers(model);
	}

	memset(&mesh, 0, sizeof(mesh));
	mesh.pObject = (ObjectRecord*)objectRecord;
	mesh.viewPosX = (float)(objectRecord->worldX - g_flightCamera.worldPosition.x);
	mesh.viewPosY = (float)(objectRecord->worldY - g_flightCamera.worldPosition.y);
	mesh.viewPosZ = (float)(objectRecord->worldZ - g_flightCamera.worldPosition.z);
	mesh.viewOrient[0] = (float)g_camMatR0_X * g_renderSignedBasisToFloat;
	mesh.viewOrient[1] = (float)g_camMatR1_X * g_renderSignedBasisToFloat;
	mesh.viewOrient[2] = (float)g_camMatR2_X * g_renderSignedBasisToFloat;
	mesh.viewOrient[3] = (float)g_camMatR0_Y * g_renderSignedBasisToFloat;
	mesh.viewOrient[4] = (float)g_camMatR1_Y * g_renderSignedBasisToFloat;
	mesh.viewOrient[5] = (float)g_camMatR2_Y * g_renderSignedBasisToFloat;
	mesh.viewOrient[6] = (float)g_camMatR0_Z * g_renderSignedBasisToFloat;
	mesh.viewOrient[7] = (float)g_camMatR1_Z * g_renderSignedBasisToFloat;
	mesh.viewOrient[8] = (float)g_camMatR2_Z * g_renderSignedBasisToFloat;
	Math3D_RotateVec3(&mesh.viewPosX, mesh.viewOrient);

	objectViewR0X = (float)g_objViewMat_R0_X * g_renderSignedBasisToFloat;
	objectViewR0Y = (float)g_objViewMat_R0_Y * g_renderSignedBasisToFloat;
	mesh.viewOrient[0] = objectViewR0X;
	mesh.viewOrient[1] = objectViewR0Y;
	objectViewR0Z = (float)g_objViewMat_R0_Z * g_renderSignedBasisToFloat;
	mesh.viewOrient[2] = objectViewR0Z;
	objectViewR1X = (float)g_objViewMat_R1_X * g_renderSignedBasisToFloat;
	objectViewR1Y = (float)g_objViewMat_R1_Y * g_renderSignedBasisToFloat;
	objectViewR1Z = (float)g_objViewMat_R1_Z * g_renderSignedBasisToFloat;
	mesh.viewOrient[3] = objectViewR1X;
	mesh.viewOrient[4] = objectViewR1Y;
	mesh.viewOrient[5] = objectViewR1Z;
	objectViewR2X = (float)g_objViewMat_R2_X * g_renderSignedBasisToFloat;
	mesh.viewOrient[6] = objectViewR2X;
	objectViewR2Y = (float)g_objViewMat_R2_Y * g_renderSignedBasisToFloat;
	objectViewR2Z = (float)g_objViewMat_R2_Z * g_renderSignedBasisToFloat;
	mesh.posX = -mesh.viewPosX;
	mesh.posY = -mesh.viewPosY;
	mesh.posZ = -mesh.viewPosZ;
	mesh.orient[0] = objectViewR0X;
	mesh.viewOrient[7] = objectViewR2Y;
	mesh.orient[1] = objectViewR1X;
	mesh.orient[2] = objectViewR2X;
	mesh.viewOrient[8] = objectViewR2Z;
	mesh.orient[3] = objectViewR0Y;
	mesh.orient[4] = objectViewR1Y;
	mesh.orient[5] = objectViewR2Y;
	mesh.orient[6] = objectViewR0Z;
	mesh.orient[7] = objectViewR1Z;
	mesh.orient[8] = objectViewR2Z;
	Math3D_RotateVec3(&mesh.posX, mesh.orient);

	g_sourceVectors = NULL;
	if (g_defaultWhiteTextureDescPtr == NULL) {
		g_defaultWhiteTextureDescPtr = &g_defaultWhiteTexture.descriptor;
		g_defaultWhiteTexture.descriptor.height = OPT_TEXTURE_DEFAULT_HEIGHT;
		g_defaultWhiteTexture.descriptor.width = OPT_TEXTURE_DEFAULT_WIDTH;
		g_defaultWhiteTexture.descriptor.paletteType = MODEL_TEXTURE_SHADE_LEVELS;
#ifdef XW_MODERN
		g_defaultWhiteTexture.descriptor.palette = g_defaultWhiteTexture.packedShades;
#else
		g_defaultWhiteTexture.descriptor.palette = (uint16_t*)OPT_TEXTURE_PALETTE_COLOR_COUNT;
#endif
		ModelTexture_BuildPalettedShadeTable(g_defaultWhiteTexture.texels, g_defaultWhiteTextureRgb24,
											 OPT_TEXTURE_DEFAULT_WIDTH, OPT_TEXTURE_DEFAULT_WIDTH);
	}
	g_curTextureDesc = g_defaultWhiteTextureDescPtr;
	g_sourceTexCoords = NULL;
	g_curVertNormals = NULL;
	g_optNodeWalkScratch2 = NULL;
	g_curMeshDescriptorData = NULL;
	g_curVertexCount = 0;

	for (rootIndex = 0; rootIndex < model->rootNodeCount; ++rootIndex) {
		OptNode* rootNode = model->rootNodes[rootIndex];

		if (rootNode->nodeType == OPT_TEXTURE) {
			++g_curLayerId;
			RenderScene_DrawModelNode(model, rootNode, &mesh);
			++rootMeshIndex;
		} else if (rootIndex == rootMeshIndex) {
			++g_curLayerId;
			RenderScene_DrawModelNode(model, rootNode, &mesh);
		}
	}
	nullsub_SharedNoOp();
}

// FUNCTION: XW 0x4890E0
void RenderScene_DrawObjectRootMeshWithSwitch(const void* objectRecord, int rootMeshIndex,
											  int nodeSwitchIndex) {
	const ObjectRecord* object = (const ObjectRecord*)objectRecord;
	uint16_t modelHandle = g_loadedModels[object->objectType];
	OptimizedPolyObject* model;
	float objectViewR0X, objectViewR0Y, objectViewR0Z;
	float objectViewR1X, objectViewR1Y, objectViewR1Z;
	float objectViewR2X, objectViewR2Y, objectViewR2Z;
	SceneMesh mesh, savedMesh;
	int rootIndex, meshOrdinal;
	int restoreBridgeTransform;
	g_nodeSwitchIndex = nodeSwitchIndex;
	model = (OptimizedPolyObject*)Memory_LockHandle(modelHandle);
	if (model->selfMarker != model)
		OptModel_AdjustOptimizedPolyObjectPointers(model);
	memset(&mesh, 0, sizeof(mesh));
	mesh.pObject = (ObjectRecord*)objectRecord;
	if (object->objectType != XW_OBJ_NONE &&
		(g_modelTypeTable[object->objectType].objectFlags & MODEL_TYPE_FLAG_CRAFT_RECORD) == 0) {
		const XwMissionObjectRecord* missionObject = (const XwMissionObjectRecord*)objectRecord;
		mesh.viewPosX =
			(float)(int32_t)((uint32_t)(missionObject->worldX * MISSION_OBJECT_WORLD_COORDINATE_SCALE) -
							 (uint32_t)g_flightCamera.worldPosition.x);
		mesh.viewPosY =
			(float)(int32_t)((uint32_t)(missionObject->worldY * MISSION_OBJECT_WORLD_COORDINATE_SCALE) -
							 (uint32_t)g_flightCamera.worldPosition.y);
		mesh.viewPosZ =
			(float)(int32_t)((uint32_t)(missionObject->worldZ * MISSION_OBJECT_WORLD_COORDINATE_SCALE) -
							 (uint32_t)g_flightCamera.worldPosition.z);
	} else {
		mesh.viewPosX = (float)(int32_t)((uint32_t)object->worldX - (uint32_t)g_flightCamera.worldPosition.x);
		mesh.viewPosY = (float)(int32_t)((uint32_t)object->worldY - (uint32_t)g_flightCamera.worldPosition.y);
		mesh.viewPosZ = (float)(int32_t)((uint32_t)object->worldZ - (uint32_t)g_flightCamera.worldPosition.z);
	}
	mesh.viewOrient[0] = (double)g_camMatR0_X * g_renderSignedBasisToFloat;
	mesh.viewOrient[1] = (double)g_camMatR1_X * g_renderSignedBasisToFloat;
	mesh.viewOrient[2] = (double)g_camMatR2_X * g_renderSignedBasisToFloat;
	mesh.viewOrient[3] = (double)g_camMatR0_Y * g_renderSignedBasisToFloat;
	mesh.viewOrient[4] = (double)g_camMatR1_Y * g_renderSignedBasisToFloat;
	mesh.viewOrient[5] = (double)g_camMatR2_Y * g_renderSignedBasisToFloat;
	mesh.viewOrient[6] = (double)g_camMatR0_Z * g_renderSignedBasisToFloat;
	mesh.viewOrient[7] = (double)g_camMatR1_Z * g_renderSignedBasisToFloat;
	mesh.viewOrient[8] = (double)g_camMatR2_Z * g_renderSignedBasisToFloat;
	Math3D_RotateVec3(&mesh.viewPosX, mesh.viewOrient);

	objectViewR0X = (double)g_objViewMat_R0_X * g_renderSignedBasisToFloat;
	objectViewR0Y = (double)g_objViewMat_R0_Y * g_renderSignedBasisToFloat;
	objectViewR0Z = (double)g_objViewMat_R0_Z * g_renderSignedBasisToFloat;
	objectViewR1X = (double)g_objViewMat_R1_X * g_renderSignedBasisToFloat;
	objectViewR1Y = (double)g_objViewMat_R1_Y * g_renderSignedBasisToFloat;
	mesh.viewOrient[0] = objectViewR0X;
	mesh.viewOrient[1] = objectViewR0Y;
	mesh.viewOrient[2] = objectViewR0Z;
	mesh.viewOrient[4] = objectViewR1Y;
	mesh.viewOrient[3] = objectViewR1X;
	objectViewR1Z = (double)g_objViewMat_R1_Z * g_renderSignedBasisToFloat;
	mesh.viewOrient[5] = objectViewR1Z;
	objectViewR2X = (double)g_objViewMat_R2_X * g_renderSignedBasisToFloat;
	objectViewR2Y = (double)g_objViewMat_R2_Y * g_renderSignedBasisToFloat;
	objectViewR2Z = (double)g_objViewMat_R2_Z * g_renderSignedBasisToFloat;
	mesh.posX = -mesh.viewPosX;
	mesh.posY = -mesh.viewPosY;
	mesh.posZ = -mesh.viewPosZ;
	mesh.orient[0] = objectViewR0X;
	mesh.orient[1] = objectViewR1X;
	mesh.orient[2] = objectViewR2X;
	mesh.orient[3] = objectViewR0Y;
	mesh.viewOrient[6] = objectViewR2X;
	mesh.viewOrient[7] = objectViewR2Y;
	mesh.viewOrient[8] = objectViewR2Z;
	mesh.orient[4] = objectViewR1Y;
	mesh.orient[5] = objectViewR2Y;
	mesh.orient[6] = objectViewR0Z;
	mesh.orient[7] = objectViewR1Z;
	mesh.orient[8] = objectViewR2Z;
	Math3D_RotateVec3(&mesh.posX, mesh.orient);

	g_sourceVectors = NULL;
	if (g_defaultWhiteTextureDescPtr == NULL) {
		g_defaultWhiteTextureDescPtr = &g_defaultWhiteTexture.descriptor;
		g_defaultWhiteTexture.descriptor.height = OPT_TEXTURE_DEFAULT_HEIGHT;
		g_defaultWhiteTexture.descriptor.width = OPT_TEXTURE_DEFAULT_WIDTH;
		g_defaultWhiteTexture.descriptor.paletteType = MODEL_TEXTURE_SHADE_LEVELS;
#ifdef XW_MODERN
		g_defaultWhiteTexture.descriptor.palette = g_defaultWhiteTexture.packedShades;
#else
		g_defaultWhiteTexture.descriptor.palette = (uint16_t*)OPT_TEXTURE_PALETTE_COLOR_COUNT;
#endif
		ModelTexture_BuildPalettedShadeTable(g_defaultWhiteTexture.texels, g_defaultWhiteTextureRgb24,
											 OPT_TEXTURE_DEFAULT_WIDTH, OPT_TEXTURE_DEFAULT_WIDTH);
	}
	g_curTextureDesc = g_defaultWhiteTextureDescPtr;
	g_sourceTexCoords = NULL;
	g_curVertNormals = NULL;
	g_optNodeWalkScratch2 = NULL;
	g_curMeshDescriptorData = NULL;
	g_curVertexCount = 0;

	meshOrdinal = 0;
	restoreBridgeTransform = 0;
	for (rootIndex = 0; rootIndex < model->rootNodeCount; ++rootIndex) {
		OptNode* rootNode;
		mesh.rotAngle = 0.0f;
		rootNode = model->rootNodes[rootIndex];
		if (rootNode->nodeType != OPT_TEXTURE) {
			int meshRotationByte;
			++meshOrdinal;
			if (object->genusId == XW_GENUS_PLAYER_PROJECTILE ||
				object->genusId == XW_GENUS_OTHER_PROJECTILE) {
				meshRotationByte = 0;
				g_nodeSwitchIndex = 0;
			} else {
				if ((g_modelTypeTable[object->objectType].objectFlags & MODEL_TYPE_FLAG_CRAFT_RECORD) != 0) {
					const CraftData* craft = (const CraftData*)object->instanceData;
					if (craft->componentState[meshOrdinal - 1] != 0)
						continue;
					meshRotationByte = craft->meshRotation[meshOrdinal - 1];
				} else {
					meshRotationByte = 0;
				}
				if (object->objectType == XW_OBJ_B_WING) {
					int bridgeIndex = g_bwingBridgeMeshIndex;
					if (bridgeIndex == -1) {
						bridgeIndex = ModelMesh_FindBridgeIndex(model);
						g_bwingBridgeMeshIndex = bridgeIndex;
					}
					if (bridgeIndex != -1 &&
						((const CraftData*)object->instanceData)->meshRotation[bridgeIndex] != 0) {
						memcpy(&savedMesh, &mesh, sizeof(savedMesh));
						restoreBridgeTransform = 1;
						RenderScene_ApplyBwingBridgeRotation(model, object, &mesh, bridgeIndex);
					}
				}
			}
			mesh.rotAngle = meshRotationByte * g_renderRadiansPerAngleByte;
		}
		if (rootNode->nodeType == OPT_TEXTURE || rootMeshIndex < 0) {
			++g_curLayerId;
			RenderScene_DrawModelNode(model, rootNode, &mesh);
		} else if (rootMeshIndex == meshOrdinal - 1) {
			++g_curLayerId;
			RenderScene_DrawModelNode(model, rootNode, &mesh);
			if (restoreBridgeTransform != 0)
				memcpy(&mesh, &savedMesh, sizeof(mesh));
			break;
		}
		if (restoreBridgeTransform != 0) {
			restoreBridgeTransform = 0;
			memcpy(&mesh, &savedMesh, sizeof(mesh));
		}
	}
	nullsub_SharedNoOp();
}

// FUNCTION: XW 0x489690
void RenderScene_DrawModelNode(struct OptimizedPolyObject* object, struct OptNode* node,
							   struct SceneMesh* mesh) {

	OptNode* currentNode;
	void* nodeData;
	int lodChildSelection;
	int switchChildSelection;
	float axisAngle[RENDER_SCENE_AXIS_ANGLE_COMPONENTS];
	float rotationMatrix[RENDER_SCENE_ROTATION_MATRIX_CAPACITY];
	SceneMesh childMesh;
	int childIndex;

	currentNode = node;
	if (currentNode == NULL)
		return;
	lodChildSelection = 0;
	switchChildSelection = 0;
	while (currentNode->nodeType == OPT_NODEREF) {
		if (g_cacheResolvedOptNodeRefs != 0) {
			if (*(char*)currentNode->param2 == '\0') {
				currentNode = (OptNode*)currentNode->pName;
			} else {
				currentNode->pName = (char*)OptModel_ResolveNodeRef(object, (const char*)currentNode->param2);
				*(char*)currentNode->param2 = '\0';
				currentNode = (OptNode*)currentNode->pName;
			}
		} else {
			currentNode = OptModel_ResolveNodeRef(object, (const char*)currentNode->param2);
		}
		if (currentNode == NULL)
			return;
	}

	nodeData = currentNode->param2;
	if (nodeData != NULL) {
		OptVector* parameters;

		parameters = (OptVector*)nodeData;
		switch (currentNode->nodeType) {
			case OPT_NODE_TYPE_23:
				if (mesh->rotAngle != 0.0f) {
					OptVector* pivot;
					OptVector* axis;

					pivot = parameters;
					axis = pivot + 1;
					mesh->posX -= pivot->x;
					mesh->posY -= pivot->y;
					mesh->posZ -= pivot->z;
					mesh->viewPosX += Math3D_RotateVec3X(&pivot->x, mesh->viewOrient);
					mesh->viewPosY += Math3D_RotateVec3Y(&pivot->x, mesh->viewOrient);
					mesh->viewPosZ += Math3D_RotateVec3Z(&pivot->x, mesh->viewOrient);
					axisAngle[0] = axis->x * g_modelNodeQ15ToFloat;
					axisAngle[1] = axis->y * g_modelNodeQ15ToFloat;
					axisAngle[2] = axis->z * g_modelNodeQ15ToFloat;
					axisAngle[3] = mesh->rotAngle;
					Math3D_BuildAxisAngleMatrix(rotationMatrix, axisAngle);
					Math3D_MulMatrix3x3(mesh->orient, rotationMatrix);
					Math3D_RotateVec3(&mesh->posX, rotationMatrix);
					Math3D_MulMatrix3x3T(mesh->viewOrient, rotationMatrix);
					mesh->posX += pivot->x;
					mesh->posY += pivot->y;
					mesh->posZ += pivot->z;
					mesh->viewPosX = mesh->viewPosX - Math3D_RotateVec3X(&pivot->x, mesh->viewOrient);
					mesh->viewPosY = mesh->viewPosY - Math3D_RotateVec3Y(&pivot->x, mesh->viewOrient);
					mesh->viewPosZ = mesh->viewPosZ - Math3D_RotateVec3Z(&pivot->x, mesh->viewOrient);
				}
				break;
			case OPT_NODE_TYPE_24:
				switchChildSelection = g_nodeSwitchIndex + 1;
				if (switchChildSelection > currentNode->childCount)
					switchChildSelection = currentNode->childCount;
				break;
			case OPT_NODE_TYPE_21:
				if (g_objectViewZ <= 0 || g_forcedLodLevel != 0) {
					lodChildSelection = g_forcedLodLevel;
					if (g_forcedLodLevel == 0) {
						lodChildSelection = 1;
					} else if (currentNode->childCount < g_forcedLodLevel) {
						lodChildSelection = -1;
					}
				} else {
					float lodThreshold;

					lodThreshold = 1.0f;
					if (g_lodDistanceScale > 0.0f)
						lodThreshold = 1.0f / ((double)g_objectViewZ * g_lodDistanceScale);
					for (lodChildSelection = 1; lodChildSelection <= currentNode->childCount;
						 ++lodChildSelection) {
						if (((float*)nodeData)[lodChildSelection - 1] <= lodThreshold)
							break;
					}
					if (lodChildSelection > currentNode->childCount)
						lodChildSelection = -1;
				}
				break;
			case OPT_NODE_TYPE_19:
				mesh->nodeFlags[0] = ((int*)nodeData)[0];
				mesh->nodeFlags[1] = ((int*)nodeData)[1];
				mesh->nodeFlags[2] = ((int*)nodeData)[2];
				break;
			case OPT_NODE_TYPE_2:
				Math3D_MulMatrix3x3(mesh->viewOrient, &parameters[1].x);
				Math3D_RotateVec3(&mesh->viewPosX, &parameters[1].x);
				mesh->viewPosX += parameters->x;
				mesh->viewPosY += parameters->y;
				mesh->viewPosZ += parameters->z;
				Math3D_MulMatrix3x3T(mesh->orient, &parameters[1].x);
				mesh->posX = mesh->posX - Math3D_RotateVec3X(&parameters->x, mesh->orient);
				mesh->posY = mesh->posY - Math3D_RotateVec3Y(&parameters->x, mesh->orient);
				mesh->posZ = mesh->posZ - Math3D_RotateVec3Z(&parameters->x, mesh->orient);
				break;
			case OPT_NODE_TYPE_4:
				mesh->viewPosX += parameters->x;
				mesh->viewPosY += parameters->y;
				mesh->viewPosZ += parameters->z;
				mesh->posX = mesh->posX - Math3D_RotateVec3X(&parameters->x, mesh->orient);
				mesh->posY = mesh->posY - Math3D_RotateVec3Y(&parameters->x, mesh->orient);
				mesh->posZ = mesh->posZ - Math3D_RotateVec3Z(&parameters->x, mesh->orient);
				break;
			case OPT_NODE_TYPE_5:
				Math3D_MulMatrix3x3(mesh->viewOrient, (const float*)nodeData);
				Math3D_RotateVec3(&mesh->viewPosX, (const float*)nodeData);
				Math3D_MulMatrix3x3T(mesh->orient, (const float*)nodeData);
				break;
			case OPT_NODE_TYPE_6: {
				float inverseScale;

				mesh->viewOrient[0] = mesh->viewOrient[0] * parameters->x;
				mesh->viewOrient[1] = mesh->viewOrient[1] * parameters->y;
				mesh->viewOrient[2] = mesh->viewOrient[2] * parameters->z;
				mesh->viewOrient[3] = mesh->viewOrient[3] * parameters->x;
				mesh->viewOrient[4] = mesh->viewOrient[4] * parameters->y;
				mesh->viewOrient[5] = mesh->viewOrient[5] * parameters->z;
				mesh->viewOrient[6] = mesh->viewOrient[6] * parameters->x;
				mesh->viewOrient[7] = mesh->viewOrient[7] * parameters->y;
				mesh->viewOrient[8] = mesh->viewOrient[8] * parameters->z;
				mesh->viewPosX = mesh->viewPosX * parameters->x;
				mesh->viewPosY = mesh->viewPosY * parameters->y;
				mesh->viewPosZ = mesh->viewPosZ * parameters->z;

				inverseScale = 1.0f / parameters->x;
				mesh->orient[0] = mesh->orient[0] * inverseScale;
				mesh->orient[1] = mesh->orient[1] * inverseScale;
				mesh->orient[2] = mesh->orient[2] * inverseScale;
				inverseScale = 1.0f / parameters->y;
				mesh->orient[3] = mesh->orient[3] * inverseScale;
				mesh->orient[4] = mesh->orient[4] * inverseScale;
				mesh->orient[5] = mesh->orient[5] * inverseScale;
				inverseScale = 1.0f / parameters->z;
				mesh->orient[6] = mesh->orient[6] * inverseScale;
				mesh->orient[7] = mesh->orient[7] * inverseScale;
				mesh->orient[8] = mesh->orient[8] * inverseScale;
				break;
			}
			case OPT_MESHVERTS:
				mesh->vertexCount = currentNode->param1;
				mesh->vertices = parameters;
				break;
			case OPT_VERTNORMALS:
				g_curVertNormals = parameters;
				mesh->vertexNormals = parameters;
				break;
			case OPT_TEXCOORDS:
				mesh->uvs = (OptTexCoord*)nodeData;
				break;
			case OPT_NODE_TYPE_10:
				if (currentNode->param1 == RENDER_NODE_DESCRIPTOR_SELECTOR_8 ||
					currentNode->param1 == RENDER_NODE_DESCRIPTOR_SELECTOR_7)
					mesh->nodeType10Descriptor78 = g_curMeshDescriptorData;
				else if (currentNode->param1 == RENDER_NODE_DESCRIPTOR_SELECTOR_6 ||
						 currentNode->param1 == RENDER_NODE_DESCRIPTOR_SELECTOR_5)
					mesh->nodeType10Descriptor56 = g_curMeshDescriptorData;
				else
					mesh->nodeDescriptor = g_curMeshDescriptorData;
				break;
			case OPT_FACEDATA:
			case OPT_FACEDATA_15:
			case OPT_FACEDATA_16:
			case OPT_FACEDATA_17: {
				OptPackedFaceData* faceData;
				OptPackedFaceRecord* faceGeometry;
				OptVector* faceNormals;
				FaceTextureGradients* faceTexturing;
				OptVector* generatedNormals;

				faceData = (OptPackedFaceData*)nodeData;
				mesh->faceCount = currentNode->param1;
				mesh->edgeCount = faceData->edgeCount;
				faceGeometry = (OptPackedFaceRecord*)faceData->records;
				mesh->faces = faceGeometry;
				faceNormals = (OptVector*)&faceGeometry[currentNode->param1];
				mesh->faceNormals = faceNormals;
				faceTexturing = (FaceTextureGradients*)&faceNormals[currentNode->param1];
				mesh->faceTexGradients = faceTexturing;
				generatedNormals = (OptVector*)&faceTexturing[currentNode->param1];
				if (mesh->pMaterial == NULL) {
					int paletteOffset;

					mesh->pMaterial = g_curTextureDesc;
					mesh->pTexels = (uint8_t*)(mesh->pMaterial + 1);
					if (g_curTextureDesc->paletteType != 0) {
						mesh->pPalette = mesh->pTexels;
						paletteOffset = mesh->pMaterial->width * mesh->pMaterial->height;
						if (paletteOffset == mesh->pMaterial->textureSize)
							mesh->pPalette = (uint8_t*)mesh->pTexels + mesh->pMaterial->dataSize;
						else
							mesh->pPalette = (uint8_t*)mesh->pTexels + paletteOffset;
					} else {
						mesh->pPalette = (uint8_t*)g_curTextureDesc->palette;
					}
					mesh->pColorKeyPalette =
						(uint16_t*)(mesh->pPalette + sizeof(((OptShadeTable*)0)->indexedShades));
				}
				if (mesh->vertexNormals == NULL) {
					mesh->vertexNormals = generatedNormals;
					RenderScene_DrawSceneMesh(mesh);
					mesh->vertexNormals = NULL;
				} else {
					RenderScene_DrawSceneMesh(mesh);
				}
				break;
			}
			case OPT_TEXTURE: {
				int paletteOffset;

				mesh->pTextureName = currentNode->pName;
				mesh->pMaterial = currentNode->param2;
				g_curTextureDesc = mesh->pMaterial;
				mesh->pTexels = (uint8_t*)(mesh->pMaterial + 1);
				if (g_curTextureDesc->paletteType != 0) {
					mesh->pPalette = mesh->pTexels;
					paletteOffset = mesh->pMaterial->width * mesh->pMaterial->height;
					if (paletteOffset == mesh->pMaterial->textureSize)
						paletteOffset = mesh->pMaterial->dataSize;
					mesh->pPalette = (uint8_t*)mesh->pTexels + paletteOffset;
				} else {
					mesh->pPalette = (uint8_t*)g_curTextureDesc->palette;
				}
				mesh->pColorKeyPalette =
					(uint16_t*)(mesh->pPalette + sizeof(((OptShadeTable*)0)->indexedShades));
				break;
			}
			default:
				break;
		}
	} else {
		switch (currentNode->nodeType) {
			case OPT_NODE_TYPE_10:
				if (currentNode->param1 == RENDER_NODE_DESCRIPTOR_SELECTOR_8 ||
					currentNode->param1 == RENDER_NODE_DESCRIPTOR_SELECTOR_7)
					mesh->nodeType10Descriptor78 = g_curMeshDescriptorData;
				else if (currentNode->param1 == RENDER_NODE_DESCRIPTOR_SELECTOR_6 ||
						 currentNode->param1 == RENDER_NODE_DESCRIPTOR_SELECTOR_5)
					mesh->nodeType10Descriptor56 = g_curMeshDescriptorData;
				else
					mesh->nodeDescriptor = g_curMeshDescriptorData;
				break;
			case OPT_TEXTURE: {
				int paletteOffset;

				mesh->pTextureName = currentNode->pName;
				mesh->pMaterial = currentNode->param2;
				g_curTextureDesc = mesh->pMaterial;
				mesh->pTexels = (uint8_t*)(mesh->pMaterial + 1);
				if (g_curTextureDesc->paletteType != 0) {
					mesh->pPalette = mesh->pTexels;
					paletteOffset = mesh->pMaterial->width * mesh->pMaterial->height;
					if (paletteOffset == mesh->pMaterial->textureSize)
						paletteOffset = mesh->pMaterial->dataSize;
					mesh->pPalette = (uint8_t*)mesh->pTexels + paletteOffset;
				} else {
					mesh->pPalette = (uint8_t*)g_curTextureDesc->palette;
				}
				mesh->pColorKeyPalette =
					(uint16_t*)(mesh->pPalette + sizeof(((OptShadeTable*)0)->indexedShades));
				break;
			}
			case OPT_NODE_TYPE_24:
				switchChildSelection = g_nodeSwitchIndex + 1;
				if (switchChildSelection > currentNode->childCount)
					switchChildSelection = currentNode->childCount;
				break;
			default:
				break;
		}
	}

	if (currentNode->childCount == 0)
		return;
	if (switchChildSelection != 0) {
		++g_curLayerId;
		RenderScene_DrawModelNode(object, currentNode->pChildren[switchChildSelection - 1], mesh);
	} else if (lodChildSelection != 0) {
		if (lodChildSelection != -1) {
			++g_curLayerId;
			RenderScene_DrawModelNode(object, currentNode->pChildren[lodChildSelection - 1], mesh);
		}
	} else {
		childMesh = *mesh;
		g_sourceVectors = NULL;
		g_sourceTexCoords = NULL;
		g_curVertNormals = NULL;
		g_optNodeWalkScratch2 = NULL;
		g_curMeshDescriptorData = NULL;
		g_curVertexCount = 0;
		for (childIndex = 0; childIndex < currentNode->childCount; ++childIndex) {
			++g_curLayerId;
			RenderScene_DrawModelNode(object, currentNode->pChildren[childIndex], &childMesh);
		}
	}
}

// FUNCTION: XW 0x489F20
int RenderScene_IsSegmentOccludedByObjectModel(struct ObjectRecord* object,
											   const struct OptVector* segmentStart,
											   const struct OptVector* segmentEnd) {
	uint16_t modelHandle;
	OptimizedPolyObject* model;
	SceneMesh mesh;
	int rootIndex;
	if (g_vertexLightOcclusionEnabled == 0)
		return 0;
	modelHandle = g_loadedModels[object->objectType];
	/* The folded no-op originally received an unused model-handle argument. */
	nullsub_SharedNoOp();
	model = Memory_LockHandle(modelHandle);
	if (model->selfMarker != model)
		OptModel_AdjustOptimizedPolyObjectPointers(model);
	memset(&mesh, 0, sizeof(mesh));
	mesh.pObject = object;
	mesh.viewOrient[0] = 1.0f;
	mesh.viewOrient[1] = 0.0f;
	mesh.viewOrient[2] = 0.0f;
	mesh.viewOrient[3] = 0.0f;
	mesh.viewOrient[4] = 1.0f;
	mesh.viewOrient[5] = 0.0f;
	mesh.viewOrient[6] = 0.0f;
	mesh.viewOrient[7] = 0.0f;
	mesh.viewOrient[8] = 1.0f;
	mesh.orient[0] = 1.0f;
	mesh.orient[1] = 0.0f;
	mesh.orient[2] = 0.0f;
	mesh.orient[3] = 0.0f;
	mesh.orient[4] = 1.0f;
	mesh.orient[5] = 0.0f;
	mesh.orient[6] = 0.0f;
	mesh.orient[7] = 0.0f;
	mesh.orient[8] = 1.0f;
	g_sourceVectors = NULL;
	g_sourceTexCoords = NULL;
	g_curVertNormals = NULL;
	g_optNodeWalkScratch2 = NULL;
	g_curMeshDescriptorData = NULL;
	g_curVertexCount = 0;
	for (rootIndex = 0; rootIndex < model->rootNodeCount; ++rootIndex) {
		if (RenderScene_TestSegmentAgainstModelNode(model, model->rootNodes[rootIndex], &mesh, segmentStart,
													segmentEnd))
			return 1;
	}
	return 0;
}

// FUNCTION: XW 0x48A070
int RenderScene_TestSegmentAgainstModelNode(struct OptimizedPolyObject* model, struct OptNode* node,
											struct SceneMesh* mesh, const struct OptVector* segmentStart,
											const struct OptVector* segmentEnd) {
	OptNode* resolvedNode = node;
	void* nodeData;
	SceneMesh childMesh;
	int childIndex;
	if (resolvedNode == NULL)
		return 0;
	while (resolvedNode->nodeType == OPT_NODEREF) {
		resolvedNode = OptModel_ResolveNodeRef(model, resolvedNode->param2);
		if (resolvedNode == NULL)
			return 0;
	}
	nodeData = resolvedNode->param2;
	if (nodeData != NULL) {
		switch (resolvedNode->nodeType) {
			case OPT_NODE_TYPE_2: {
				const float* transform = nodeData;
				float viewPosition[3];
				Math3D_MulMatrix3x3(mesh->viewOrient, &transform[RENDER_SCENE_VIEW_ORIENTATION_INDEX]);
				viewPosition[0] = mesh->viewPosX;
				viewPosition[1] = mesh->viewPosY;
				viewPosition[2] = mesh->viewPosZ;
				Math3D_RotateVec3(viewPosition, &transform[RENDER_SCENE_VIEW_ORIENTATION_INDEX]);
				mesh->viewPosX = transform[0] + viewPosition[0];
				mesh->viewPosY = transform[1] + viewPosition[1];
				mesh->viewPosZ = transform[2] + viewPosition[2];
				Math3D_MulMatrix3x3T(mesh->orient, &transform[RENDER_SCENE_VIEW_ORIENTATION_INDEX]);
				mesh->posX -= Math3D_RotateVec3X(transform, mesh->orient);
				mesh->posY -= Math3D_RotateVec3Y(transform, mesh->orient);
				mesh->posZ -= Math3D_RotateVec3Z(transform, mesh->orient);
				break;
			}
			case OPT_NODE_TYPE_4: {
				const float* translation = nodeData;
				mesh->viewPosX = translation[0] + mesh->viewPosX;
				mesh->viewPosY = translation[1] + mesh->viewPosY;
				mesh->viewPosZ = translation[2] + mesh->viewPosZ;
				mesh->posX -= Math3D_RotateVec3X(translation, mesh->orient);
				mesh->posY -= Math3D_RotateVec3Y(translation, mesh->orient);
				mesh->posZ -= Math3D_RotateVec3Z(translation, mesh->orient);
				break;
			}
			case OPT_NODE_TYPE_5: {
				const float* rotation = nodeData;
				float viewPosition[3];
				Math3D_MulMatrix3x3(mesh->viewOrient, rotation);
				viewPosition[0] = mesh->viewPosX;
				viewPosition[1] = mesh->viewPosY;
				viewPosition[2] = mesh->viewPosZ;
				Math3D_RotateVec3(viewPosition, rotation);
				mesh->viewPosX = viewPosition[0];
				mesh->viewPosY = viewPosition[1];
				mesh->viewPosZ = viewPosition[2];
				Math3D_MulMatrix3x3T(mesh->orient, rotation);
				break;
			}
			case OPT_NODE_TYPE_6: {
				const float* scale = nodeData;
				float inverseScale;
				mesh->viewOrient[0] = mesh->viewOrient[0] * scale[0];
				mesh->viewOrient[1] = mesh->viewOrient[1] * scale[1];
				mesh->viewOrient[2] = mesh->viewOrient[2] * scale[2];
				mesh->viewOrient[3] = mesh->viewOrient[3] * scale[0];
				mesh->viewOrient[4] = mesh->viewOrient[4] * scale[1];
				mesh->viewOrient[5] = mesh->viewOrient[5] * scale[2];
				mesh->viewOrient[6] = mesh->viewOrient[6] * scale[0];
				mesh->viewOrient[7] = mesh->viewOrient[7] * scale[1];
				mesh->viewOrient[8] = mesh->viewOrient[8] * scale[2];
				mesh->viewPosX *= scale[0];
				mesh->viewPosY *= scale[1];
				mesh->viewPosZ *= scale[2];
				inverseScale = 1.0f / scale[0];
				mesh->orient[0] *= inverseScale;
				mesh->orient[1] *= inverseScale;
				mesh->orient[2] *= inverseScale;
				inverseScale = 1.0f / scale[1];
				mesh->orient[3] *= inverseScale;
				mesh->orient[4] *= inverseScale;
				mesh->orient[5] *= inverseScale;
				inverseScale = 1.0f / scale[2];
				mesh->orient[6] *= inverseScale;
				mesh->orient[7] *= inverseScale;
				mesh->orient[8] *= inverseScale;
				break;
			}
			case OPT_MESHVERTS: {
				int vertexCount = resolvedNode->param1;
				mesh->vertices = nodeData;
				mesh->vertexCount = vertexCount;
				break;
			}
			case OPT_VERTNORMALS:
				g_curVertNormals = nodeData;
				mesh->vertexNormals = nodeData;
				break;
			case OPT_FACEDATA:
			case OPT_FACEDATA_15:
			case OPT_FACEDATA_16:
			case OPT_FACEDATA_17: {
				int32_t* edgeCount = nodeData;
				OptPackedFaceRecord* faces;
				OptVector* faceNormals;
				FaceTextureGradients* gradients;
				OptVector* fallbackNormals;
				mesh->faceCount = resolvedNode->param1;
				mesh->edgeCount = *edgeCount;
				faces = (OptPackedFaceRecord*)(edgeCount + 1);
				mesh->faces = faces;
				faceNormals = (OptVector*)(faces + resolvedNode->param1);
				mesh->faceNormals = faceNormals;
				gradients = (FaceTextureGradients*)(faceNormals + resolvedNode->param1);
				mesh->faceTexGradients = gradients;
				fallbackNormals = (OptVector*)(gradients + resolvedNode->param1);
				if (mesh->vertexNormals == NULL) {
					mesh->vertexNormals = fallbackNormals;
					if (RenderScene_TestSegmentAgainstMeshFaces(mesh, segmentStart, segmentEnd))
						return 1;
					mesh->vertexNormals = NULL;
				} else if (RenderScene_TestSegmentAgainstMeshFaces(mesh, segmentStart, segmentEnd)) {
					return 1;
				}
				break;
			}
		}
	}
	if (resolvedNode->childCount == 0)
		return 0;
	childMesh = *mesh;
	g_sourceVectors = NULL;
	g_sourceTexCoords = NULL;
	g_curVertNormals = NULL;
	g_optNodeWalkScratch2 = NULL;
	g_curMeshDescriptorData = NULL;
	g_curVertexCount = 0;
	for (childIndex = 0; childIndex < resolvedNode->childCount; ++childIndex) {
		if (RenderScene_TestSegmentAgainstModelNode(model, resolvedNode->pChildren[childIndex], &childMesh,
													segmentStart, segmentEnd))
			return 1;
	}
	return 0;
}

// FUNCTION: XW 0x48A4B0
int RenderScene_TestSegmentAgainstMeshFaces(const struct SceneMesh* mesh,
											const struct OptVector* segmentStart,
											const struct OptVector* segmentEnd) {
	OptVector start = *segmentStart;
	const OptPackedFaceRecord* faces = mesh->faces;
	const OptVector* normals = mesh->faceNormals;
	const OptVector* vertices = mesh->vertices;
	OptVector end = *segmentEnd;
	int faceCount = mesh->faceCount;
	int faceIndex;

	for (faceIndex = 0; faceIndex < faceCount; ++faceIndex) {
		const OptPackedFaceRecord* face = &faces[faceIndex];
		const OptVector* normal = &normals[faceIndex];
		int first = face->vertexIndices[0];
		int second = face->vertexIndices[1];
		int third = face->vertexIndices[2];
		int fourth = face->vertexIndices[3];
		float startDistance, endDistance;
		float scale, projectedU, projectedV, firstEdge, edge;
		OptTexCoord projected[4];
		int vertexIndex;
		int vertexCount =
			fourth < 0 ? OPT_POLYGON_TRIANGLE_VERTICES : sizeof(projected) / sizeof(projected[0]);

		if (vertices[first].x <= start.x && vertices[first].x <= end.x) {
			if (vertices[second].x <= start.x && vertices[second].x <= end.x &&
				vertices[third].x <= start.x && vertices[third].x <= end.x &&
				(fourth == OPT_INDEX_NONE || (vertices[fourth].x <= start.x && vertices[fourth].x <= end.x)))
				continue;
		} else if (vertices[first].x >= start.x && vertices[first].x >= end.x &&
				   vertices[second].x >= start.x && vertices[second].x >= end.x &&
				   vertices[third].x >= start.x && vertices[third].x >= end.x &&
				   (fourth == OPT_INDEX_NONE ||
					(vertices[fourth].x >= start.x && vertices[fourth].x >= end.x))) {
			continue;
		}

		if (vertices[first].y <= start.y && vertices[first].y <= end.y) {
			if (vertices[second].y <= start.y && vertices[second].y <= end.y &&
				vertices[third].y <= start.y && vertices[third].y <= end.y &&
				(fourth == OPT_INDEX_NONE || (vertices[fourth].y <= start.y && vertices[fourth].y <= end.y)))
				continue;
		} else if (vertices[first].y >= start.y && vertices[first].y >= end.y &&
				   vertices[second].y >= start.y && vertices[second].y >= end.y &&
				   vertices[third].y >= start.y && vertices[third].y >= end.y &&
				   (fourth == OPT_INDEX_NONE ||
					(vertices[fourth].y >= start.y && vertices[fourth].y >= end.y))) {
			continue;
		}

		if (vertices[first].z <= start.z && vertices[first].z <= end.z) {
			if (vertices[second].z <= start.z && vertices[second].z <= end.z &&
				vertices[third].z <= start.z && vertices[third].z <= end.z &&
				(fourth == OPT_INDEX_NONE || (vertices[fourth].z <= start.z && vertices[fourth].z <= end.z)))
				continue;
		} else if (vertices[first].z >= start.z && vertices[first].z >= end.z &&
				   vertices[second].z >= start.z && vertices[second].z >= end.z &&
				   vertices[third].z >= start.z && vertices[third].z >= end.z &&
				   (fourth == OPT_INDEX_NONE ||
					(vertices[fourth].z >= start.z && vertices[fourth].z >= end.z))) {
			continue;
		}

		startDistance = (start.x - vertices[first].x) * normal->x +
						(start.y - vertices[first].y) * normal->y + (start.z - vertices[first].z) * normal->z;
		endDistance = (end.x - vertices[first].x) * normal->x + (end.y - vertices[first].y) * normal->y +
					  (end.z - vertices[first].z) * normal->z;
		if (startDistance >= 0.0f) {
			if (startDistance < g_modelFacePlanePositiveCrossingLimit || endDistance >= 0.0f)
				continue;
		} else if (startDistance > g_modelFacePlaneNegativeCrossingLimit || endDistance <= 0.0f) {
			continue;
		}
		/* Preserve the original interpolation and signed projection selection. */
		scale = -startDistance / endDistance;
		if (normal->z > normal->x && normal->z > normal->y) {
			projectedU = (end.x - start.x) * scale + start.x;
			projectedV = (end.y - start.y) * scale + start.y;
			for (vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex) {
				projected[vertexIndex].u = vertices[face->vertexIndices[vertexIndex]].x;
				projected[vertexIndex].v = vertices[face->vertexIndices[vertexIndex]].y;
			}
		} else if (normal->y > normal->x && normal->y > normal->z) {
			projectedU = (end.x - start.x) * scale + start.x;
			projectedV = (end.z - start.z) * scale + start.z;
			for (vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex) {
				projected[vertexIndex].u = vertices[face->vertexIndices[vertexIndex]].x;
				projected[vertexIndex].v = vertices[face->vertexIndices[vertexIndex]].z;
			}
		} else {
			projectedU = (end.y - start.y) * scale + start.y;
			projectedV = (end.z - start.z) * scale + start.z;
			for (vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex) {
				projected[vertexIndex].u = vertices[face->vertexIndices[vertexIndex]].y;
				projected[vertexIndex].v = vertices[face->vertexIndices[vertexIndex]].z;
			}
		}
		firstEdge = (projectedU - projected[0].u) * (projected[1].v - projected[0].v) -
					(projectedV - projected[0].v) * (projected[1].u - projected[0].u);
		edge = (projectedU - projected[1].u) * (projected[2].v - projected[1].v) -
			   (projectedV - projected[1].v) * (projected[2].u - projected[1].u);
		if (firstEdge < 0.0f ? edge >= 0.0f : edge < 0.0f)
			continue;
		if (fourth < 0) {
			edge = (projectedU - projected[2].u) * (projected[0].v - projected[2].v) -
				   (projectedV - projected[2].v) * (projected[0].u - projected[2].u);
		} else {
			edge = (projectedU - projected[2].u) * (projected[3].v - projected[2].v) -
				   (projectedV - projected[2].v) * (projected[3].u - projected[2].u);
			if (firstEdge < 0.0f ? edge >= 0.0f : edge < 0.0f)
				continue;
			edge = (projectedU - projected[3].u) * (projected[0].v - projected[3].v) -
				   (projectedV - projected[3].v) * (projected[0].u - projected[3].u);
		}
		if (firstEdge < 0.0f ? edge < 0.0f : edge >= 0.0f)
			return 1;
	}
	return 0;
}

// FUNCTION: XW 0x491870
void RenderScene_AllocateBuffers(void) {
	g_sceneSpanDataCapacity = RENDER_SCENE_SPAN_CAPACITY;
	g_sceneSpanDataHandle = Memory_AllocHandle(g_sceneSpanDataCapacity * sizeof(*g_sceneSpanDataBase), 0);
	if (g_sceneSpanDataHandle == LANDRU_NULL_HANDLE) {
		fediskio_fatalerror(FEDISKIO_ERROR_ALLOCATION);
	}
	g_sceneSpanPtrCapacity = RENDER_SCENE_SPAN_CAPACITY;
	g_sceneSpanPtrListHandle = Memory_AllocHandle(g_sceneSpanPtrCapacity * sizeof(*g_sceneSpanPtrList), 0);
	if (g_sceneSpanPtrListHandle == LANDRU_NULL_HANDLE) {
		fediskio_fatalerror(FEDISKIO_ERROR_ALLOCATION);
	}
	g_sceneFaceMax = RENDER_SCENE_FACE_CAPACITY;
	g_visFaceListHandle = Memory_AllocHandle(g_sceneFaceMax * sizeof(*g_visFaceList), 0);
	if (g_visFaceListHandle == LANDRU_NULL_HANDLE) {
		fediskio_fatalerror(FEDISKIO_ERROR_ALLOCATION);
	}
	g_projVertMax = RENDER_SCENE_CLIP_CAPACITY_MULTIPLIER * g_vertexRemapCapacity;
	g_projVertListHandle = Memory_AllocHandle(g_projVertMax * sizeof(*g_projVertList), 0);
	if (g_projVertListHandle == LANDRU_NULL_HANDLE) {
		fediskio_fatalerror(FEDISKIO_ERROR_ALLOCATION);
	}
	g_sceneEdgeMax = RENDER_SCENE_CLIP_CAPACITY_MULTIPLIER * g_sceneEdgeFlagsCapacity;
	g_sceneEdgeListHandle = Memory_AllocHandle(g_sceneEdgeMax * sizeof(*g_sceneEdgeList), 0);
	if (g_sceneEdgeListHandle == LANDRU_NULL_HANDLE) {
		fediskio_fatalerror(FEDISKIO_ERROR_ALLOCATION);
	}

	g_vertexRemapHandle = Memory_AllocHandle(g_vertexRemapCapacity * sizeof(*g_vertexRemap), 0);
	if (g_vertexRemapHandle == LANDRU_NULL_HANDLE) {
		fediskio_fatalerror(FEDISKIO_ERROR_ALLOCATION);
	}

	g_sceneEdgeFlagsHandle = Memory_AllocHandle(g_sceneEdgeFlagsCapacity * sizeof(*g_sceneEdgeFlags), 0);
	if (g_sceneEdgeFlagsHandle == LANDRU_NULL_HANDLE) {
		fediskio_fatalerror(FEDISKIO_ERROR_ALLOCATION);
	}

	g_sceneSclEdgeListHandle = Memory_AllocHandle(FLIGHT_DISPLAY_HEIGHT * sizeof(*g_sceneSclEdgeList), 0);
	if (g_sceneSclEdgeListHandle == LANDRU_NULL_HANDLE) {
		fediskio_fatalerror(FEDISKIO_ERROR_ALLOCATION);
	}

	g_scanlineSpanHeadsHandle = Memory_AllocHandle(FLIGHT_DISPLAY_HEIGHT * sizeof(*g_scanlineSpanHeads), 0);
	if (g_scanlineSpanHeadsHandle == LANDRU_NULL_HANDLE) {
		fediskio_fatalerror(FEDISKIO_ERROR_ALLOCATION);
	}
	g_sw3dLightSampleBlockSize = SW3D_LIGHT_SAMPLE_BLOCK_SIZE;
	g_sw3dLightSampleBlockShift = SW3D_LIGHT_SAMPLE_BLOCK_SHIFT;
	g_sw3dLightSampleBlockMask = SW3D_LIGHT_SAMPLE_BLOCK_SIZE - 1;
	g_sw3dLightSampleInvBlockSize = g_sw3dSpanLengthReciprocal[SW3D_LIGHT_SAMPLE_BLOCK_SIZE];
	g_sw3dLightSampleBlockSizeFloat = (float)SW3D_LIGHT_SAMPLE_BLOCK_SIZE;
	g_scenePhongDataHandle = Memory_AllocHandle(
		((FLIGHT_DISPLAY_WIDTH / SW3D_LIGHT_SAMPLE_BLOCK_SIZE + 1) * RENDER_SCENE_PHONG_SLOT_COUNT + 1) *
			sizeof(*g_scenePhongData),
		0);
	if (g_scenePhongDataHandle == LANDRU_NULL_HANDLE) {
		fediskio_fatalerror(FEDISKIO_ERROR_ALLOCATION);
	}
	g_meshQueueMax = RENDER_SCENE_MESH_CAPACITY;
	g_meshQueueHandle = Memory_AllocHandle(g_meshQueueMax * sizeof(*g_meshQueue), 0);
	if (g_meshQueueHandle == LANDRU_NULL_HANDLE) {
		fediskio_fatalerror(FEDISKIO_ERROR_ALLOCATION);
	}
#ifdef XW_MODERN
	Memory_SetRegionExecuteReadWrite(NULL, RENDER_SCENE_LEGACY_CODE_REGION_SIZE);
#else
	/* C has no address for the original interior instruction label. */
	Memory_SetRegionExecuteReadWrite((void*)RenderScene_AllocateBuffers,
									 RENDER_SCENE_LEGACY_CODE_REGION_SIZE);
#endif
}

// FUNCTION: XW 0x491AC0
void RenderScene_Initialize(int resetSceneState) {
#ifndef XW_MODERN
	_control87(_PC_24, _MCW_PC);
#endif
	g_sw3dOcclusionSentinelFace.maxVertW = 1.0e32f;
	g_sw3dOcclusionSentinelFace.minVertW = 1.0e32f;
	g_sw3dOcclusionSentinelFace.depthPlane[0] = 0.0f;
	g_sw3dOcclusionSentinelFace.depthPlane[1] = 0.0f;
	g_sw3dOcclusionSentinelFace.depthPlane[2] = 1.0e32f;
	g_sceneSpanDataBase = (SceneSpan*)Memory_LockHandle(g_sceneSpanDataHandle);
	g_sceneSpanPtrList = (SceneSpan**)Memory_LockHandle(g_sceneSpanPtrListHandle);
	g_visFaceList = (SceneFace*)Memory_LockHandle(g_visFaceListHandle);
	g_projVertList = (ProjVertex*)Memory_LockHandle(g_projVertListHandle);
	g_sceneEdgeList = (SceneEdge*)Memory_LockHandle(g_sceneEdgeListHandle);
	g_vertexRemap = (int*)Memory_LockHandle(g_vertexRemapHandle);
	g_sceneEdgeFlags = (int*)Memory_LockHandle(g_sceneEdgeFlagsHandle);
	g_sceneSclEdgeList = (void**)Memory_LockHandle(g_sceneSclEdgeListHandle);
	g_scanlineSpanHeads = (SceneSpan**)Memory_LockHandle(g_scanlineSpanHeadsHandle);
	g_scenePhongData = (SoftwareLightSample*)Memory_LockHandle(g_scenePhongDataHandle);
	g_meshQueue = (SceneMesh*)Memory_LockHandle(g_meshQueueHandle);
	if (resetSceneState != 0) {
		const uint8_t* mask;
		size_t maskIndex;
		unsigned int row;
		g_sceneSpanPtrAvail = g_sceneSpanPtrCapacity;
		g_pSceneSpanDataCur = g_sceneSpanDataBase;
		g_visFacePassStart = 0;
		g_visFaceCount = 0;
		g_phongSlotIndex = 0;
		g_pSceneSpanDataEnd = &g_sceneSpanDataBase[g_sceneSpanDataCapacity - 1];
		mask = (const uint8_t*)g_flightAuxBuffer;
		maskIndex = (uint16_t)g_viewportSpanMaskOffset;
		g_meshQueueIndex = 0;
		for (row = 0; row < g_flightVpHeight; ++row) {
			unsigned int spanX;
			int8_t runSign;
			SceneSpan* previousSpan = NULL;
			g_scanlineSpanHeads[row] = NULL;
			runSign = (int8_t)mask[maskIndex++];
			for (spanX = 0; spanX < g_flightVpWidth;) {
				int runLength = mask[maskIndex++];
				if (runLength == PANEL_SPAN_MASK_EXTENDED_RUN) {
					runLength = mask[maskIndex++];
					if (runLength == PANEL_SPAN_MASK_EXTENDED_RUN) {
						runLength = mask[maskIndex++] + PANEL_SPAN_MASK_BYTE_RANGE;
					}
					runLength += PANEL_SPAN_MASK_FIRST_EXTENSION;
				}
				if (runSign < 0) {
					if (previousSpan == NULL)
						g_scanlineSpanHeads[row] = g_pSceneSpanDataCur;
					else
						previousSpan->next = g_pSceneSpanDataCur;
					previousSpan = g_pSceneSpanDataCur++;
					previousSpan->xEnd = runLength + spanX;
					previousSpan->xStart = spanX;
					previousSpan->face = &g_sw3dOcclusionSentinelFace;
					previousSpan->next = NULL;
				}
				spanX += runLength;
				runSign = -runSign;
			}
		}
	} else {
		g_visFacePassStart = g_visFaceCount;
	}
	g_phongSlotStride =
		(g_sw3dLightSampleBlockSize + (unsigned int)g_flightVpWidth - 1) / g_sw3dLightSampleBlockSize;
	g_sw3dLightSampleCacheSceneStampBase += g_flightVpHeight;
	g_invProjScale = 1.0f / (double)(unsigned int)g_projScaleInt;
	if (g_useHardware3D != 0)
		RenderScene_InitHardwareFrame();
}

// FUNCTION: XW 0x491DB0
void RenderScene_UnlockBuffers(void) {
	xmemhdl_Unlock_Handle(g_sceneSpanDataHandle);
	xmemhdl_Unlock_Handle(g_sceneSpanPtrListHandle);
	xmemhdl_Unlock_Handle(g_visFaceListHandle);
	xmemhdl_Unlock_Handle(g_projVertListHandle);
	xmemhdl_Unlock_Handle(g_sceneEdgeListHandle);
	xmemhdl_Unlock_Handle(g_vertexRemapHandle);
	xmemhdl_Unlock_Handle(g_sceneEdgeFlagsHandle);
	xmemhdl_Unlock_Handle(g_sceneSclEdgeListHandle);
	xmemhdl_Unlock_Handle(g_scanlineSpanHeadsHandle);
	xmemhdl_Unlock_Handle(g_scenePhongDataHandle);
	xmemhdl_Unlock_Handle(g_meshQueueHandle);
	g_sceneSpanDataBase = NULL;
	g_sceneSpanPtrList = NULL;
	g_visFaceList = NULL;
	g_projVertList = NULL;
	g_sceneEdgeList = NULL;
	g_vertexRemap = NULL;
	g_sceneEdgeFlags = NULL;
	g_sceneSclEdgeList = NULL;
	g_scanlineSpanHeads = NULL;
	g_scenePhongData = NULL;
	g_meshQueue = NULL;
}

// FUNCTION: XW 0x491EA0
void RenderScene_FreeBuffers(void) {
	if (g_sceneSpanDataHandle != LANDRU_NULL_HANDLE)
		Memory_FreeHandle(g_sceneSpanDataHandle);
	g_sceneSpanDataHandle = LANDRU_NULL_HANDLE;
	if (g_sceneSpanPtrListHandle != LANDRU_NULL_HANDLE)
		Memory_FreeHandle(g_sceneSpanPtrListHandle);
	g_sceneSpanPtrListHandle = LANDRU_NULL_HANDLE;
	if (g_visFaceListHandle != LANDRU_NULL_HANDLE)
		Memory_FreeHandle(g_visFaceListHandle);
	g_visFaceListHandle = LANDRU_NULL_HANDLE;
	if (g_projVertListHandle != LANDRU_NULL_HANDLE)
		Memory_FreeHandle(g_projVertListHandle);
	g_projVertListHandle = LANDRU_NULL_HANDLE;
	if (g_sceneEdgeListHandle != LANDRU_NULL_HANDLE)
		Memory_FreeHandle(g_sceneEdgeListHandle);
	g_sceneEdgeListHandle = LANDRU_NULL_HANDLE;
	if (g_vertexRemapHandle != LANDRU_NULL_HANDLE)
		Memory_FreeHandle(g_vertexRemapHandle);
	g_vertexRemapHandle = LANDRU_NULL_HANDLE;
	if (g_sceneEdgeFlagsHandle != LANDRU_NULL_HANDLE)
		Memory_FreeHandle(g_sceneEdgeFlagsHandle);
	g_sceneEdgeFlagsHandle = LANDRU_NULL_HANDLE;
	if (g_sceneSclEdgeListHandle != LANDRU_NULL_HANDLE)
		Memory_FreeHandle(g_sceneSclEdgeListHandle);
	g_sceneSclEdgeListHandle = LANDRU_NULL_HANDLE;
	if (g_scanlineSpanHeadsHandle != LANDRU_NULL_HANDLE)
		Memory_FreeHandle(g_scanlineSpanHeadsHandle);
	g_scanlineSpanHeadsHandle = LANDRU_NULL_HANDLE;
	if (g_scenePhongDataHandle != LANDRU_NULL_HANDLE)
		Memory_FreeHandle(g_scenePhongDataHandle);
	g_scenePhongDataHandle = LANDRU_NULL_HANDLE;
	if (g_meshQueueHandle != LANDRU_NULL_HANDLE)
		Memory_FreeHandle(g_meshQueueHandle);
	g_meshQueueHandle = LANDRU_NULL_HANDLE;
}
