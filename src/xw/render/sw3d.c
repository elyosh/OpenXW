#include "xw/render/sw3d.h"

#include "xw/assets/opt_model.h"
#include "xw/flight/flight_display.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/math/math3d.h"
#include "xw/render/flight_light.h"
#include "xw/render/flight_view.h"
#include "xw/render/render_scene.h"
#include "xw/render/renderer.h"

#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4C347C
const float g_softwareDistantDepthSubtract = -100000.0f;

// GLOBAL: XW 0x4C3484
const float g_softwareTriangleVertexCountFloat = 3.0f;

// GLOBAL: XW 0x4C3488
const float g_softwareQuadVertexCountFloat = 4.0f;

// GLOBAL: XW 0x4C348C
const float g_softwareDistantProjectionScale = 100000.0f;

// GLOBAL: XW 0x4DA500
float g_sw3dSpanLengthReciprocal[SW3D_SPAN_RECIPROCAL_COUNT] = {
	1.0f,
	1.0f,
	0.5f,
	0.3333333432674408f,
	0.25f,
	0.20000000298023224f,
	0.1666666716337204f,
	0.1428571492433548f,
	0.125f,
	0.1111111119389534f,
	0.10000000149011612f,
	0.09090909361839294f,
	0.0833333358168602f,
	0.07692307978868484f,
	0.0714285746216774f,
	0.06666667014360428f,
	0.0625f,
	0.05882352963089943f,
	0.0555555559694767f,
	0.05263157933950424f,
	0.05000000074505806f,
	0.0476190485060215f,
	0.04545454680919647f,
	0.043478261679410934f,
	0.0416666679084301f,
	0.03999999910593033f,
	0.03846153989434242f,
	0.03703703731298447f,
	0.0357142873108387f,
	0.03448275849223137f,
	0.03333333507180214f,
	0.032258063554763794f,
	0.03125f,
	0.03030303120613098f,
	0.029411764815449715f,
	0.02857142873108387f,
	0.02777777798473835f,
	0.027027027681469917f,
	0.02631578966975212f,
	0.025641025975346565f,
	0.02500000037252903f,
	0.024390242993831635f,
	0.02380952425301075f,
	0.023255813866853714f,
	0.022727273404598236f,
	0.02222222276031971f,
	0.021739130839705467f,
	0.021276595070958138f,
	0.02083333395421505f,
	0.020408162847161293f,
	0.019999999552965164f,
	0.019607843831181526f,
	0.01923076994717121f,
	0.01886792480945587f,
	0.018518518656492233f,
	0.0181818176060915f,
	0.01785714365541935f,
	0.017543859779834747f,
	0.017241379246115685f,
	0.016949152573943138f,
	0.01666666753590107f,
	0.016393441706895828f,
	0.016129031777381897f,
	0.01587301678955555f,
	0.015625f,
	0.015384615398943424f,
	0.01515151560306549f,
	0.014925372786819935f,
	0.014705882407724857f,
	0.014492753893136978f,
};

// GLOBAL: XW 0x4DA6E8
int g_sw3dShadeDitherInitialByScanlineParity[2] = { 0, 128 };

// GLOBAL: XW 0x4DA6F0
const int g_textureLog2BySizeDiv16[SW3D_TEXTURE_LOG2_TABLE_COUNT] = {
	3, 4, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9,
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10
};

// GLOBAL: XW 0x4DA7F8
const float g_sw3dSpanOneFloat = 1.0f;

// GLOBAL: XW 0x4DA7FC
const float g_sw3dLightIntensityToShadeScale = 15.0f;

// GLOBAL: XW 0x4DA800
const float g_sw3dFloatToIntRoundBias = 12582912.0f;

// GLOBAL: XW 0x4DA820
const float g_sw3dTexCoordBiasByShift[SW3D_TEXCOORD_BIAS_COUNT] = { 49152.0f, 24576.0f, 12288.0f, 6144.0f,
																	3072.0f,  1536.0f,  768.0f,   384.0f,
																	192.0f,   96.0f,    48.0f,    24.0f,
																	12.0f,    6.0f,     3.0f };

// GLOBAL: XW 0x4F4A54
int g_sw3dSkipOddScanlines = 0;

// GLOBAL: XW 0x55CAFC
ProjVertex* g_sw3dGeneratedClipVertex = NULL;

// GLOBAL: XW 0x55CB08
ProjVertex* g_sw3dClipTop = NULL;

// GLOBAL: XW 0x55CB18
ProjVertex* g_sw3dClipBottom = NULL;

// GLOBAL: XW 0x561310
SceneFace* g_sw3dCurrentFace = NULL;

// GLOBAL: XW 0x561314
int g_sw3dLightSampleBlockMask = 0;

// GLOBAL: XW 0x561318
unsigned int g_sw3dSpanShadeDitherAccum = 0;

// GLOBAL: XW 0x56131C
int g_sw3dLightSampleBlockSize = 0;

// GLOBAL: XW 0x561320
uint8_t* g_sw3dSpanCachedTexels = NULL;

// GLOBAL: XW 0x561324
int g_sw3dSpanFramebufferRowOffset = 0;

// GLOBAL: XW 0x56132C
float g_sw3dLightSampleSubrowLerpT = 0.0f;

// GLOBAL: XW 0x561334
SoftwareSpanShadeCounter g_sw3dSpanShadeCounterScratch = { 0 };

// GLOBAL: XW 0x561338
int g_sw3dSpanVQ8 = 0;

// GLOBAL: XW 0x56133C
float g_sw3dLightSampleRowsToNextBlockFloat = 0.0f;

// GLOBAL: XW 0x561340
float g_sw3dLightSampleSubrowFloat = 0.0f;

// GLOBAL: XW 0x561344
float g_sw3dLightSampleInvBlockSize = 0.0f;

// GLOBAL: XW 0x56134A
int g_sw3dSpanUQ8 = 0;

// GLOBAL: XW 0x561358
int g_sw3dSpanShadeStepQ8 = 0;

// GLOBAL: XW 0x56135E
int g_sw3dSpanStepUQ8 = 0;

// GLOBAL: XW 0x561364
int g_sw3dCurrentScanlineY = 0;

// GLOBAL: XW 0x561368
int g_sw3dSpanLength = 0;

// GLOBAL: XW 0x56136C
int g_sw3dSpanStartX = 0;

// GLOBAL: XW 0x561370
int g_sw3dCurrentLightSampleCacheStamp = 0;

// GLOBAL: XW 0x561878
unsigned int g_sw3dSpanPackedUVScratch = 0;

// GLOBAL: XW 0x5618F0
float g_sw3dLightSampleBlockSizeFloat = 0.0f;

// GLOBAL: XW 0x5618F8
SceneMesh* g_sw3dSpanSceneMesh = NULL;

// GLOBAL: XW 0x5618FC
float g_sw3dSpanTextureWidthFloat = 0.0f;

// GLOBAL: XW 0x561900
float g_sw3dSpanTextureHeightFloat = 0.0f;

// GLOBAL: XW 0x561904
int g_sw3dSpanTextureWidthShift = 0;

// GLOBAL: XW 0x561908
int g_sw3dSpanTextureHeightShift = 0;

// GLOBAL: XW 0x56190C
uint8_t* g_sw3dSpanShadeTable = NULL;

// GLOBAL: XW 0x561910
uint8_t* g_sw3dSpanTexels = NULL;

// GLOBAL: XW 0x561914
unsigned int g_sw3dSpanTexelMask = 0;

// GLOBAL: XW 0x561918
uint8_t* g_sw3dSpanCachedShadeTable = NULL;

// GLOBAL: XW 0x56191C
int g_sw3dLightSampleBlockShift = 0;

// GLOBAL: XW 0x561920
int g_sw3dSpanShadeQ8 = 0;

// GLOBAL: XW 0x561924
int g_sw3dSpanStepVQ8 = 0;

// FUNCTION: XW 0x486490
void sw3d_ProjectMeshVertices(struct SceneMesh* mesh) {
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
					output->screenY = (g_projOffsetY + (g_flightVpHeight >> 1)) + output->screenY;
				} else {
					output->depth = position[2] - 1.0f;
					output->screenX = position[0];
					output->screenY = position[1];
					face->nearClipState = OPT_INDEX_NONE;
					depth = (unsigned int)g_projScaleInt;
				}
				RenderScene_ComputeVertexLighting(mesh, output, &mesh->vertexNormals[normalIndex],
												  &mesh->vertices[sourceIndex], &g_meshCullEyePosition);
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
				1.0f / (planeCofactor[1] * face->depthPlane[1] + planeCofactor[2] * face->depthPlane[2] +
						planeCofactor[0] * face->depthPlane[0]);
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
					(sourceFace->vertexIndices[3] == OPT_INDEX_NONE ? g_softwareTriangleVertexCountFloat
																	: g_softwareQuadVertexCountFloat) /
					sumVertexW * (unsigned int)g_projScaleInt;
				face->mipLevel = (int)((int32_t)((uint32_t)(mesh->pMaterial->width * mesh->pMaterial->height)
												 << SW3D_MIP_AREA_SHIFT) *
									   (averageDepth * averageDepth * footprint));
			}
		}
	}
	g_projVertCount += mesh->projVertCursor;
}

// FUNCTION: XW 0x486C00
void sw3d_ProjectMeshVerticesDistant(struct SceneMesh* mesh) {
	float projectionScale = (unsigned int)g_projScaleInt / mesh->viewPosZ;
	SceneFace* visibleFaces = &g_visFaceList[mesh->firstVisibleFace];
	ProjVertex* projectedVertices = &g_projVertList[g_projVertCount];
	int vertexIndex, faceIndex;
	float position[3];
	int outputIndex = 0;
	float viewTransform[12];
	int component;

	mesh->vertBaseIndex = g_projVertCount;
	mesh->projVertCursor = 0;
	projectionScale *= g_softwareDistantProjectionScale;
	for (vertexIndex = 0; vertexIndex < mesh->vertexCount; ++vertexIndex)
		g_vertexRemap[vertexIndex] = OPT_INDEX_NONE;
	for (faceIndex = 0; faceIndex < mesh->visibleFaceCount; ++faceIndex) {
		SceneFace* face = &visibleFaces[faceIndex];
		OptPackedFaceRecord* sourceFace = &mesh->faces[face->faceIndex];
		int corner;
		viewTransform[0] = mesh->viewPosX;
		viewTransform[1] = mesh->viewPosY;
		viewTransform[2] = mesh->viewPosZ;
		for (component = 0; component < (int)(sizeof(mesh->viewOrient) / sizeof(mesh->viewOrient[0]));
			 ++component)
			viewTransform[RENDER_SCENE_VIEW_ORIENTATION_INDEX + component] = mesh->viewOrient[component];
		RenderScene_TransformFaceTextureGradients(face, &mesh->faceTexGradients[face->faceIndex],
												  viewTransform);
		face->maxVertW = 0.0f;
		face->minVertW = (unsigned int)g_projScaleInt;
		for (corner = 0;
			 corner < (int)(sizeof(sourceFace->vertexIndices) / sizeof(sourceFace->vertexIndices[0]));
			 ++corner) {
			int sourceIndex = sourceFace->vertexIndices[corner];
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
				position[2] = position[2] + mesh->viewPosZ - g_softwareDistantDepthSubtract;
				output->depth = projectionScale / position[2];
				depth = output->depth;
				output->screenX = position[0] * output->depth;
				output->screenY = position[1] * output->depth;
				output->screenX = (g_flightVpWidth >> 1) + output->screenX;
				output->screenY = (unsigned int)(g_projOffsetY + (g_flightVpHeight >> 1)) + output->screenY;
				RenderScene_ComputeVertexLighting(mesh, output, &mesh->vertexNormals[normalIndex],
												  &mesh->vertices[sourceIndex], &g_meshCullEyePosition);
				++outputIndex;
			} else {
				depth = g_projVertList[mesh->vertBaseIndex + g_vertexRemap[sourceIndex]].depth;
			}
			if (depth > face->maxVertW)
				face->maxVertW = depth;
			if (depth < face->minVertW)
				face->minVertW = depth;
		}
		if (mesh->uvs != NULL) {
			int texCoordIndex = sourceFace->texCoordIndices[0];
			int sourceIndex = sourceFace->vertexIndices[0];
			float uCofactor[3], vCofactor[3], planeCofactor[3];
			double inverseDeterminant;
			float projectedInverse, footprint;
			position[0] = mesh->vertices[sourceIndex].x;
			position[1] = mesh->vertices[sourceIndex].y;
			position[2] = mesh->vertices[sourceIndex].z;
			Math3D_RotateVec3(position, mesh->viewOrient);
			position[0] += mesh->viewPosX;
			position[1] = mesh->viewPosY + position[1];
			position[2] = position[2] + mesh->viewPosZ - g_softwareDistantDepthSubtract;
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
				1.0f / (planeCofactor[1] * face->depthPlane[1] + planeCofactor[2] * face->depthPlane[2] +
						planeCofactor[0] * face->depthPlane[0]);
			projectedInverse = inverseDeterminant / projectionScale;
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
			footprint = face->gradients[0] * (position[2] * position[2] * face->gradients[4]);
			if (footprint < 0.0f)
				footprint = -footprint;
			face->mipLevel = (int)((int32_t)((uint32_t)(mesh->pMaterial->width * mesh->pMaterial->height)
											 << SW3D_MIP_AREA_SHIFT) *
								   footprint);
			footprint = position[2] * position[2] * face->gradients[1] * face->gradients[3];
			if (footprint < 0.0f)
				footprint = -footprint;
			face->mipLevel += (int)((int32_t)((uint32_t)(mesh->pMaterial->width * mesh->pMaterial->height)
											  << SW3D_MIP_AREA_SHIFT) *
									footprint);
		}
	}
	g_projVertCount += mesh->projVertCursor;
}

// FUNCTION: XW 0x4872C0
void sw3d_RasterizeMeshFaces(struct SceneMesh* mesh) {
	ProjVertex* vertices = &g_projVertList[mesh->vertBaseIndex];
	SceneFace* meshFaces = &g_visFaceList[mesh->firstVisibleFace];
	int sceneEdgeCursor = g_sceneEdgeCursor;
	SceneFace* face;
	SceneEdge* outputEdge;
	SceneEdge* firstEdge;
	int edgeIndex;
	int faceIndex;
	int outputCount;

	mesh->edgeBaseIndex = sceneEdgeCursor;
	mesh->clippedEdgeCount = 0;
	firstEdge = outputEdge = &g_sceneEdgeList[sceneEdgeCursor];
	for (edgeIndex = 0; edgeIndex < mesh->edgeCount; ++edgeIndex)
		g_sceneEdgeFlags[edgeIndex] = SW3D_INVALID_EDGE;

	for (faceIndex = 0; faceIndex < mesh->visibleFaceCount; ++faceIndex) {
		const OptPackedFaceRecord* record;
		int cornerCount;
		int currentCorner;
		int previousCorner;

		outputCount = 0;
		record = &mesh->faces[meshFaces[faceIndex].faceIndex];
		face = &meshFaces[faceIndex];

		if (face->nearClipState == SW3D_INVALID_EDGE) {
			face->nearClipState = g_flightVpHeight;
			g_sw3dClipTop = NULL;
			g_sw3dClipBottom = NULL;
			cornerCount =
				record->edgeIndices[(sizeof(record->edgeIndices) / sizeof(record->edgeIndices[0])) - 1] !=
						SW3D_INVALID_EDGE
					? (int)(sizeof(record->edgeIndices) / sizeof(record->edgeIndices[0]))
					: (int)(sizeof(record->edgeIndices) / sizeof(record->edgeIndices[0])) - 1;
			currentCorner = 0;
			for (previousCorner = cornerCount - 1; previousCorner >= 0; --previousCorner) {
				int sourceEdge;
				int existingEdge;

				sourceEdge = record->edgeIndices[previousCorner];
				existingEdge = g_sceneEdgeFlags[sourceEdge];
				g_sw3dGeneratedClipVertex = NULL;
				if (existingEdge == SW3D_INVALID_EDGE) {
					if (sw3d_SetupClippedEdge(
							mesh, outputEdge, &vertices[g_vertexRemap[record->vertexIndices[previousCorner]]],
							&vertices[g_vertexRemap[record->vertexIndices[currentCorner]]]) >= 0) {
						face->edges[outputCount++] = outputEdge;
						outputEdge->pClipVert = g_sw3dGeneratedClipVertex;
						g_sceneEdgeFlags[sourceEdge] = mesh->clippedEdgeCount;
						++mesh->clippedEdgeCount;
						outputEdge = &firstEdge[mesh->clippedEdgeCount];
					} else if (g_sw3dGeneratedClipVertex == NULL) {
						g_sceneEdgeFlags[sourceEdge] = SW3D_REJECTED_EDGE;
					}
				} else if (existingEdge != SW3D_REJECTED_EDGE) {
					SceneEdge* edge;

					edge = &firstEdge[existingEdge];
					face->edges[outputCount++] = edge;
					if (edge->pClipVert != NULL) {
						g_sw3dClipBottom = g_sw3dClipTop;
						g_sw3dClipTop = edge->pClipVert;
					}
				}
				currentCorner = previousCorner;
			}

			if (g_sw3dClipBottom != NULL) {
				if (sw3d_SetupClippedEdge(mesh, outputEdge, g_sw3dClipTop, g_sw3dClipBottom) >= 0) {
					face->edges[outputCount++] = outputEdge;
					++mesh->clippedEdgeCount;
					outputEdge = &firstEdge[mesh->clippedEdgeCount];
				}
			}
		} else {
			face->nearClipState = g_flightVpHeight;
			cornerCount =
				record->edgeIndices[(sizeof(record->edgeIndices) / sizeof(record->edgeIndices[0])) - 1] !=
						SW3D_INVALID_EDGE
					? (int)(sizeof(record->edgeIndices) / sizeof(record->edgeIndices[0]))
					: (int)(sizeof(record->edgeIndices) / sizeof(record->edgeIndices[0])) - 1;
			currentCorner = 0;
			for (previousCorner = cornerCount - 1; previousCorner >= 0; --previousCorner) {
				int sourceEdge;
				int existingEdge;

				sourceEdge = record->edgeIndices[previousCorner];
				existingEdge = g_sceneEdgeFlags[sourceEdge];
				if (existingEdge == SW3D_INVALID_EDGE) {
					if (sw3d_SetupEdge(outputEdge,
									   &vertices[g_vertexRemap[record->vertexIndices[previousCorner]]],
									   &vertices[g_vertexRemap[record->vertexIndices[currentCorner]]]) >= 0) {
						face->edges[outputCount++] = outputEdge;
						g_sceneEdgeFlags[sourceEdge] = mesh->clippedEdgeCount;
						++mesh->clippedEdgeCount;
						outputEdge = &firstEdge[mesh->clippedEdgeCount];
					} else {
						g_sceneEdgeFlags[sourceEdge] = SW3D_REJECTED_EDGE;
					}
				} else if (existingEdge != SW3D_REJECTED_EDGE) {
					face->edges[outputCount++] = &firstEdge[existingEdge];
				}
				currentCorner = previousCorner;
			}
		}

		face->edgeCount = outputCount;
		if (outputCount == 0) {
			face->yBot = 0;
			face->yTop = 0;
		} else {
			sw3d_ScanConvertFace(face);
		}
	}
	g_sceneEdgeCursor += mesh->clippedEdgeCount;
}

// FUNCTION: XW 0x4876B0
void sw3d_ScanConvertFace(struct SceneFace* face) {
	SceneEdge* left;
	SceneEdge* right;
	SceneEdge* edge;
	SceneEdge* swapEdge;
	int edgeIndex;
	int edgeCount;
	int remainingEdges;
	int scanY;
	int runRows;
	int spanCount;
	float runRowsFloat;
	float nextLight;
	float projectedLeftX, projectedLeftLight, spanWidth;
	float leftStartX;
	float rightStartX;
	float leftStartLight;
	float rightStartLight;

	edgeCount = face->edgeCount;
	remainingEdges = edgeCount;
	left = face->edges[0];
	right = left;
	for (edgeIndex = 1; edgeIndex < edgeCount; ++edgeIndex) {
		edge = face->edges[edgeIndex];
		if (edge->yStart < left->yStart)
			left = edge;
		if (edge->yEnd > right->yEnd)
			right = edge;
	}

	face->yTop = left->yStart;
	face->yBot = right->yEnd;
	spanCount = face->yBot - face->yTop;
	if (g_sceneSpanPtrAvail > spanCount) {
		g_sceneSpanPtrAvail -= spanCount;
		face->pSpans = &g_sceneSpanPtrList[g_sceneSpanPtrAvail];

		for (edgeIndex = 0; edgeIndex < face->edgeCount; ++edgeIndex) {
			edge = face->edges[edgeIndex];
			if (left != edge && edge->yStart == left->yStart) {
				right = edge;
				break;
			}
		}
		if (edgeIndex == face->edgeCount) {
			face->yBot = face->yTop;
		} else {
			if (left->x > right->x || (left->x == right->x && left->dxdy > right->dxdy)) {
				swapEdge = left;
				left = right;
				right = swapEdge;
			}

			face->pScanEdge = left;
			scanY = left->yStart;
			if (left->yEnd < right->yEnd)
				runRows = left->yEnd - scanY;
			else
				runRows = right->yEnd - scanY;
			leftStartX = left->x;
			rightStartX = right->x;
			leftStartLight = left->lightIntensity;
			rightStartLight = right->lightIntensity;

			if ((double)rightStartX - leftStartX > 1.0) {
				do {
					face->spanLightIntensityDx =
						((double)right->lightIntensity - left->lightIntensity) / ((double)right->x - left->x);
					sw3d_InsertSpan(left->x, right->x, scanY++, face);
					if (--runRows <= 0)
						break;
					nextLight = left->dLightIntensityDy + left->lightIntensity;
					left->x = left->dxdy + left->x;
					left->lightIntensity = nextLight;
					nextLight = right->dLightIntensityDy + right->lightIntensity;
					right->x = right->dxdy + right->x;
					right->lightIntensity = nextLight;
				} while (1);
			} else {
				runRowsFloat = (float)runRows;
				projectedLeftX = (double)runRowsFloat * left->dxdy + left->x;
				projectedLeftLight = (double)runRowsFloat * left->dLightIntensityDy + left->lightIntensity;
				face->spanLightIntensityDx = ((double)runRows * right->dLightIntensityDy +
											  right->lightIntensity - projectedLeftLight) /
											 ((double)runRowsFloat * right->dxdy + right->x - projectedLeftX);
				do {
					sw3d_InsertSpan(left->x, right->x, scanY++, face);
					if (--runRows <= 0)
						break;
					nextLight = left->dLightIntensityDy + left->lightIntensity;
					left->x = left->dxdy + left->x;
					left->lightIntensity = nextLight;
					nextLight = right->dLightIntensityDy + right->lightIntensity;
					right->x = right->dxdy + right->x;
					right->lightIntensity = nextLight;
				} while (1);
			}

			while (remainingEdges > 0) {
				if (right->yEnd != left->yEnd) {
					--remainingEdges;
					if (scanY == left->yEnd) {
						left->x = leftStartX;
						left->lightIntensity = leftStartLight;
						for (edgeIndex = 0; edgeIndex < face->edgeCount; ++edgeIndex) {
							if (face->edges[edgeIndex]->yStart == scanY)
								break;
						}
						if (edgeIndex == face->edgeCount) {
							face->yBot = scanY;
							break;
						}
						left = face->edges[edgeIndex];
						face->pScanEdge = left;
						leftStartX = left->x;
						leftStartLight = left->lightIntensity;
						nextLight = right->dLightIntensityDy + right->lightIntensity;
						right->x = right->dxdy + right->x;
						right->lightIntensity = nextLight;
					} else {
						right->x = rightStartX;
						right->lightIntensity = rightStartLight;
						for (edgeIndex = 0; edgeIndex < face->edgeCount; ++edgeIndex) {
							if (face->edges[edgeIndex]->yStart == scanY)
								break;
						}
						if (edgeIndex == face->edgeCount) {
							face->yBot = scanY;
							break;
						}
						right = face->edges[edgeIndex];
						rightStartX = right->x;
						rightStartLight = right->lightIntensity;
						nextLight = left->dLightIntensityDy + left->lightIntensity;
						left->x = left->dxdy + left->x;
						left->lightIntensity = nextLight;
					}
				} else {
					remainingEdges -= 2;
					if (remainingEdges == 0)
						break;

					left->x = leftStartX;
					right->x = rightStartX;
					left->lightIntensity = leftStartLight;
					right->lightIntensity = rightStartLight;
					for (edgeIndex = 0; edgeIndex < face->edgeCount; ++edgeIndex) {
						if (face->edges[edgeIndex]->yStart == scanY)
							break;
					}
					if (edgeIndex == face->edgeCount) {
						face->yBot = scanY;
						break;
					}
					left = face->edges[edgeIndex];
					for (++edgeIndex; edgeIndex < face->edgeCount; ++edgeIndex) {
						if (face->edges[edgeIndex]->yStart == scanY)
							break;
					}
					if (edgeIndex == face->edgeCount) {
						face->yBot = scanY;
						break;
					}
					right = face->edges[edgeIndex];
					if (left->x > right->x || (left->x == right->x && left->dxdy > right->dxdy)) {
						swapEdge = left;
						left = right;
						right = swapEdge;
					}
					face->pScanEdge = left;
					leftStartX = left->x;
					rightStartX = right->x;
					leftStartLight = left->lightIntensity;
					rightStartLight = right->lightIntensity;
				}

				if (remainingEdges == 1)
					return;
				if (remainingEdges == 2) {
					if (right->yEnd != left->yEnd)
						return;
					runRows = left->yEnd - scanY;
					projectedLeftX = left->dxdy * (double)runRows + left->x;
					if (right->dxdy * (double)runRows + right->x - projectedLeftX > 1.0) {
						do {
							face->spanLightIntensityDx =
								((double)right->lightIntensity - left->lightIntensity) /
								((double)right->x - left->x);
							sw3d_InsertSpan(left->x, right->x, scanY++, face);
							if (--runRows <= 0)
								break;
							nextLight = left->dLightIntensityDy + left->lightIntensity;
							left->x = left->dxdy + left->x;
							left->lightIntensity = nextLight;
							nextLight = right->dLightIntensityDy + right->lightIntensity;
							right->x = right->dxdy + right->x;
							right->lightIntensity = nextLight;
						} while (1);
					} else {
						spanWidth = right->x - left->x;
						face->spanLightIntensityDx =
							((double)right->lightIntensity - left->lightIntensity) / spanWidth;
						do {
							sw3d_InsertSpan(left->x, right->x, scanY++, face);
							if (--runRows <= 0)
								break;
							nextLight = left->dLightIntensityDy + left->lightIntensity;
							left->x = left->dxdy + left->x;
							left->lightIntensity = nextLight;
							nextLight = right->dLightIntensityDy + right->lightIntensity;
							right->x = right->dxdy + right->x;
							right->lightIntensity = nextLight;
						} while (1);
					}
				} else {
					if (left->yEnd < right->yEnd)
						runRows = left->yEnd - scanY;
					else
						runRows = right->yEnd - scanY;
					do {
						face->spanLightIntensityDx = ((double)right->lightIntensity - left->lightIntensity) /
													 ((double)right->x - left->x);
						sw3d_InsertSpan(left->x, right->x, scanY++, face);
						if (--runRows <= 0)
							break;
						nextLight = left->dLightIntensityDy + left->lightIntensity;
						left->x = left->dxdy + left->x;
						left->lightIntensity = nextLight;
						nextLight = right->dLightIntensityDy + right->lightIntensity;
						right->x = right->dxdy + right->x;
						right->lightIntensity = nextLight;
					} while (1);
				}
			}

			left->x = leftStartX;
			right->x = rightStartX;
			left->lightIntensity = leftStartLight;
			right->lightIntensity = rightStartLight;
		}
	} else {
		face->yBot = face->yTop;
	}
}

// FUNCTION: XW 0x487CE0
int sw3d_SetupClippedEdge(struct SceneMesh* mesh, struct SceneEdge* edge, const struct ProjVertex* vTop,
						  const struct ProjVertex* vBot) {
	int yStart;
	int yEnd;
	float inverseHeight;
	float startOffset;
	if (vBot->depth < 0.0f) {
		const ProjVertex* vertex;
		if (vTop->depth < 0.0f)
			return -1;
		vertex = vTop;
		vTop = vBot;
		vBot = vertex;
	}
	if (vTop->depth < 0.0f) {
		float inverseDepth;
		float scale;
		float clipFraction;
		float cameraX;
		float cameraY;
		g_sw3dClipBottom = g_sw3dClipTop;
		g_sw3dClipTop = &g_projVertList[mesh->projVertCursor + mesh->vertBaseIndex];
		g_sw3dGeneratedClipVertex = g_sw3dClipTop;
		++mesh->projVertCursor;
		++g_projVertCount;
		inverseDepth = 1.0f / vBot->depth;
		scale = (unsigned int)g_projScaleInt;
		clipFraction = vTop->depth / (vTop->depth - -1.0f - inverseDepth * scale);
		cameraX = inverseDepth * (vBot->screenX - (g_flightVpWidth >> 1));
		cameraY = vBot->screenY - (g_projOffsetY + (g_flightVpHeight >> 1));
		cameraY *= inverseDepth;
		cameraX -= vTop->screenX;
		cameraX *= clipFraction;
		cameraX += vTop->screenX;
		cameraY = (cameraY - vTop->screenY) * clipFraction + vTop->screenY;
		g_sw3dClipTop->screenX = cameraX * scale;
		g_sw3dClipTop->screenY = cameraY * (unsigned int)g_projScaleInt;
		g_sw3dClipTop->screenX += g_flightVpWidth >> 1;
		g_sw3dClipTop->screenY += g_projOffsetY + (g_flightVpHeight >> 1);
		g_sw3dClipTop->depth = (unsigned int)g_projScaleInt;
		g_sw3dClipTop->lightIntensity =
			(vBot->lightIntensity - vTop->lightIntensity) * clipFraction + vTop->lightIntensity;
		vTop = g_sw3dClipTop;
	}
	if (vTop->screenY < 0.0f) {
		yStart = 0;
	} else {
		yStart = (int)vTop->screenY;
		if (yStart != vTop->screenY)
			++yStart;
	}
	if (vBot->screenY < 0.0f) {
		yEnd = 0;
	} else {
		yEnd = (int)vBot->screenY;
		if (yEnd != vBot->screenY)
			++yEnd;
	}
	if (yStart == yEnd)
		return -1;
	if (yStart > yEnd) {
		const ProjVertex* vertex = vTop;
		int y;
		vTop = vBot;
		vBot = vertex;
		y = yStart;
		yStart = yEnd;
		yEnd = y;
	}
	if (yEnd <= 0)
		return -1;
	if (yStart >= g_flightVpHeight)
		return -1;
	if (yEnd > g_flightVpHeight)
		yEnd = g_flightVpHeight;
	edge->yEnd = yEnd;
	inverseHeight = 1.0f / (vBot->screenY - vTop->screenY);
	edge->dxdy = (vBot->screenX - vTop->screenX) * inverseHeight;
	edge->dLightIntensityDy = (vBot->lightIntensity - vTop->lightIntensity) * inverseHeight;
	edge->pClipVert = NULL;
	startOffset = yStart - vTop->screenY;
	if (yStart == 0 && startOffset > vBot->screenY) {
		edge->x = vBot->screenX - vBot->screenY * edge->dxdy;
		edge->lightIntensity = vBot->lightIntensity - vBot->screenY * edge->dLightIntensityDy;
	} else {
		edge->x = startOffset * edge->dxdy + vTop->screenX;
		edge->lightIntensity = startOffset * edge->dLightIntensityDy + vTop->lightIntensity;
	}
	edge->yStart = yStart;
	return yStart;
}

// FUNCTION: XW 0x488040
int sw3d_SetupEdge(struct SceneEdge* edge, const struct ProjVertex* vTop, const struct ProjVertex* vBot) {
	int yStart;
	int yEnd;
	float inverseHeight;
	float startOffset;
	if (vTop->screenY < 0.0f) {
		yStart = 0;
	} else {
		yStart = (int)vTop->screenY;
		if (yStart != vTop->screenY)
			++yStart;
	}
	if (vBot->screenY < 0.0f) {
		yEnd = 0;
	} else {
		yEnd = (int)vBot->screenY;
		if (yEnd != vBot->screenY)
			++yEnd;
	}
	if (yStart == yEnd)
		return -1;
	if (yStart > yEnd) {
		const ProjVertex* vertex = vTop;
		int y;
		vTop = vBot;
		vBot = vertex;
		y = yStart;
		yStart = yEnd;
		yEnd = y;
	}
	if (yEnd <= 0)
		return -1;
	if (yStart >= g_flightVpHeight)
		return -1;
	if (yEnd > g_flightVpHeight)
		yEnd = g_flightVpHeight;
	edge->yEnd = yEnd;
	inverseHeight = 1.0f / (vBot->screenY - vTop->screenY);
	edge->dxdy = (vBot->screenX - vTop->screenX) * inverseHeight;
	edge->dLightIntensityDy = (vBot->lightIntensity - vTop->lightIntensity) * inverseHeight;
	edge->pClipVert = NULL;
	startOffset = yStart - vTop->screenY;
	if (yStart == 0 && startOffset > vBot->screenY) {
		edge->x = vBot->screenX - vBot->screenY * edge->dxdy;
		edge->lightIntensity = vBot->lightIntensity - vBot->screenY * edge->dLightIntensityDy;
	} else {
		edge->x = startOffset * edge->dxdy + vTop->screenX;
		edge->lightIntensity = startOffset * edge->dLightIntensityDy + vTop->lightIntensity;
	}
	edge->yStart = yStart;
	return yStart;
}

// FUNCTION: XW 0x492140
void sw3d_DrawVisibleFacesToSurface(void) {
	int visibleFaceIndex;
	SceneEdge scanEdge;
	FlightLight_ResetSoftwareFaceSampleCache();
	if (g_useHardware3D != 0) {
		RenderScene_FlushGeometry();
		return;
	}
	if (g_flightSurfaceAlreadyLocked == 0)
		FlightDisplay_LockSurface();
	g_sw3dSpanSceneMesh = NULL;
	for (visibleFaceIndex = g_visFacePassStart; visibleFaceIndex < g_visFaceCount; ++visibleFaceIndex) {
		SceneFace* face;
		OptTextureData* material;
		int width, height;
		int mipTexelOffset = 0;
		int rowIndex;
		int pitch, rowOffset;
		unsigned int scanY;
		float depthRowBase;
		g_sw3dCurrentFace = &g_visFaceList[visibleFaceIndex];
		g_sw3dCurrentScanlineY = g_sw3dCurrentFace->yTop;
		g_sw3dCurrentFace->pScanEdge = &scanEdge;
		face = g_sw3dCurrentFace;
		material = face->mesh->pMaterial;
		width = material->width;
		height = material->height;
		if (g_textureMipmapsEnabled != 0 && width * height == material->textureSize) {
			int mipArea = (int)((double)face->mipLevel * g_textureMipScale);
			while (mipArea > SW3D_MIP_AREA_LIMIT) {
				if (width == SW3D_MIP_MIN_DIMENSION || height == SW3D_MIP_MIN_DIMENSION)
					break;
				mipArea >>= SW3D_MIP_AREA_REDUCTION_SHIFT;
				mipTexelOffset += width * height;
				width >>= 1;
				height >>= 1;
			}
		}
		g_sw3dSpanTextureWidthShift = g_textureLog2BySizeDiv16[width >> SW3D_TEXTURE_LOG2_INDEX_SHIFT];
		g_sw3dSpanTextureWidthFloat = (float)width;
		g_sw3dSpanTextureHeightFloat = (float)height;
		g_sw3dSpanTexelMask = width * height - 1;
		g_sw3dSpanTextureHeightShift = g_textureLog2BySizeDiv16[height >> SW3D_TEXTURE_LOG2_INDEX_SHIFT];
		pitch = g_surfacePitch;
		g_sw3dSpanShadeTable = face->mesh->pPalette;
		g_sw3dSpanTexels = &face->mesh->pTexels[mipTexelOffset];
		g_sw3dSpanSceneMesh = face->mesh;
		scanY = (unsigned int)g_sw3dCurrentScanlineY;
		depthRowBase = (double)scanY * face->depthPlane[1] + face->depthPlane[2];
		rowOffset = g_flightBytesPerPixel * g_flightVpX + pitch * (scanY + g_flightVpY);
		g_sw3dSpanFramebufferRowOffset = rowOffset;
		for (rowIndex = 0; scanY < (unsigned int)face->yBot; ++rowIndex, g_sw3dCurrentScanlineY = ++scanY) {
			SceneSpan* span = face->pSpans[rowIndex];
			if (span == NULL) {
				depthRowBase = (double)depthRowBase + face->depthPlane[1];
				rowOffset += pitch;
				g_sw3dSpanFramebufferRowOffset = rowOffset;
				continue;
			}
			{
				unsigned int subrow = scanY & g_sw3dLightSampleBlockMask;
				int xStart, xEnd;
				double spanStart;
				SceneSpan* occluder;
				if (subrow != 0) {
					float reciprocal = g_sw3dSpanLengthReciprocal[g_sw3dLightSampleBlockSize];
					g_sw3dLightSampleSubrowFloat = (float)subrow;
					g_sw3dLightSampleRowsToNextBlockFloat =
						(float)((unsigned int)g_sw3dLightSampleBlockSize - subrow);
					g_sw3dLightSampleSubrowLerpT = reciprocal * g_sw3dLightSampleSubrowFloat;
				} else {
					g_sw3dLightSampleSubrowLerpT = 0.0f;
					g_sw3dLightSampleSubrowFloat = 0.0f;
					g_sw3dLightSampleRowsToNextBlockFloat = (float)(unsigned int)g_sw3dLightSampleBlockSize;
				}
				g_sw3dCurrentLightSampleCacheStamp =
					g_sw3dLightSampleCacheSceneStampBase + (scanY >> g_sw3dLightSampleBlockShift);
				xStart = span->xStart;
				xEnd = span->xEnd;
				occluder = span->next;
				spanStart = (double)xStart;
				scanEdge.lightIntensity = span->lightIntensity;
				scanEdge.x = spanStart;
				face->spanLightIntensityDx = span->dLightIntensityDx;
				if (occluder == NULL) {
					float depth = g_sw3dCurrentFace->depthPlane[0] * spanStart + depthRowBase;
					sw3d_DrawTexturedSpan(xStart, xEnd, depth);
				} else {
					if (occluder->xStart < xEnd) {
						do {
							if (occluder->xStart > xStart) {
								float depth =
									(double)xStart * g_sw3dCurrentFace->depthPlane[0] + depthRowBase;
								sw3d_DrawTexturedSpan(xStart, occluder->xStart, depth);
								xStart = occluder->xEnd;
								xEnd = span->xEnd;
								if (xStart >= xEnd)
									break;
							} else if (xStart < occluder->xEnd) {
								xStart = occluder->xEnd;
								if (xStart >= xEnd)
									break;
							}
							occluder = occluder->next;
						} while (occluder != NULL && occluder->xStart < xEnd);
					}
					if (xStart < xEnd) {
						float depth = (double)xStart * g_sw3dCurrentFace->depthPlane[0] + depthRowBase;
						sw3d_DrawTexturedSpan(xStart, xEnd, depth);
					}
				}
				face = g_sw3dCurrentFace;
				depthRowBase = (double)depthRowBase + face->depthPlane[1];
				pitch = g_surfacePitch;
				scanY = (unsigned int)g_sw3dCurrentScanlineY;
				rowOffset = g_sw3dSpanFramebufferRowOffset + pitch;
				g_sw3dSpanFramebufferRowOffset = rowOffset;
			}
		}
	}
	if (g_flightSurfaceAlreadyLocked == 0)
		FlightDisplay_UnlockSurface();
}

// FUNCTION: XW 0x4924E0
void sw3d_InsertSpan(float xLeft, float xRight, int scanY, struct SceneFace* face) {
	SceneSpan* current;
	SceneSpan* next;
	SceneSpan* insertionNext;
	SceneSpan* previous;
	SceneSpan* span;
	float newDepth;
	float currentDepth;
	float newRowDepth, currentRowDepth;
	double newEndDepth, currentEndDepth;
	int startX;
	int endX;
	int overlapWidth;
	int crossingFromRight;
	int crossingFromLeft;
	int currentLeftWidth;
	int currentRightWidth;

	face->pSpans[scanY - face->yTop] = NULL;
	if (xLeft < 0.0f) {
		startX = 0;
	} else {
		startX = (int)xLeft;
		if ((double)startX != xLeft) {
			++startX;
		}
	}
	if (xRight < 0.0f) {
		endX = 0;
	} else {
		endX = (int)xRight;
		if ((double)endX != xRight) {
			++endX;
		}
	}
	if (endX >= g_flightVpWidth) {
		endX = g_flightVpWidth;
	}
	if (endX <= startX || startX >= g_flightVpWidth || (g_sw3dSkipOddScanlines && (scanY & 1))) {
		return;
	}

	previous = NULL;
	current = g_scanlineSpanHeads[scanY];
	while (current != NULL) {
		if (current->xEnd <= startX) {
			previous = current;
			current = current->next;
			continue;
		}
		if (current->xStart > startX) {
			break;
		}

		if (face->maxVertW <= current->face->minVertW) {
			startX = current->xEnd;
			if (startX >= endX) {
				return;
			}
			previous = current;
			current = current->next;
			continue;
		}
		if (face->minVertW >= current->face->maxVertW) {
			if (current->xEnd > endX) {
				previous = current;
				current = current->next;
				continue;
			}
			current->xEnd = startX;
			if (current->xEnd == current->xStart) {
				current->face->pSpans[scanY - current->face->yTop] = NULL;
				if (previous == NULL) {
					g_scanlineSpanHeads[scanY] = current->next;
				} else {
					previous->next = current->next;
				}
				current = current->next;
			} else {
				previous = current;
				current = current->next;
			}
			continue;
		}

		newRowDepth = face->depthPlane[1] * (double)scanY + face->depthPlane[2];
		currentRowDepth = current->face->depthPlane[1] * (double)scanY + current->face->depthPlane[2];
		newDepth = face->depthPlane[0] * (double)(float)startX + newRowDepth;
		currentDepth = current->face->depthPlane[0] * (double)(float)startX + currentRowDepth;
		if (newDepth <= currentDepth) {
			if (!(face->depthPlane[0] > current->face->depthPlane[0])) {
				startX = current->xEnd;
				if (startX >= endX) {
					return;
				}
				previous = current;
				current = current->next;
				continue;
			}
			if (current->xEnd < endX) {
				overlapWidth = current->xEnd - startX;
				newEndDepth = (double)overlapWidth * face->depthPlane[0] + newDepth;
				currentEndDepth = (double)overlapWidth * current->face->depthPlane[0] + currentDepth;
				if (newEndDepth <= currentEndDepth) {
					startX = current->xEnd;
					previous = current;
					current = current->next;
					continue;
				}
			} else {
				overlapWidth = endX - startX;
				newEndDepth = (double)overlapWidth * face->depthPlane[0] + newDepth;
				currentEndDepth = (double)overlapWidth * current->face->depthPlane[0] + currentDepth;
				if (newEndDepth <= currentEndDepth) {
					return;
				}
			}
			currentLeftWidth = (int)((double)overlapWidth -
									 (newEndDepth - currentEndDepth) /
										 ((double)face->depthPlane[0] - current->face->depthPlane[0])) +
							   1;
			if (currentLeftWidth < 0) {
				currentLeftWidth = 0;
			}
			startX += currentLeftWidth;
			if (startX >= endX) {
				return;
			}
			previous = current;
			current = current->next;
			continue;
		}

		if (current->face->depthPlane[0] <= face->depthPlane[0]) {
			if (current->xEnd > endX) {
				previous = current;
				current = current->next;
				continue;
			}
			current->xEnd = startX;
			if (current->xEnd == current->xStart) {
				current->face->pSpans[scanY - current->face->yTop] = NULL;
				if (previous == NULL) {
					g_scanlineSpanHeads[scanY] = current->next;
				} else {
					previous->next = current->next;
				}
				current = current->next;
			} else {
				previous = current;
				current = current->next;
			}
			continue;
		}

		if (current->xEnd <= endX) {
			overlapWidth = current->xEnd - startX;
			newEndDepth = (double)overlapWidth * face->depthPlane[0] + newDepth;
			currentEndDepth = (double)overlapWidth * current->face->depthPlane[0] + currentDepth;
			if (newEndDepth >= currentEndDepth) {
				current->xEnd = startX;
				if (current->xEnd == current->xStart) {
					current->face->pSpans[scanY - current->face->yTop] = NULL;
					if (previous == NULL) {
						g_scanlineSpanHeads[scanY] = current->next;
					} else {
						previous->next = current->next;
					}
					current = current->next;
				} else {
					previous = current;
					current = current->next;
				}
				continue;
			}

			crossingFromRight = (int)((newEndDepth - currentEndDepth) /
									  ((double)face->depthPlane[0] - current->face->depthPlane[0]));
			if (crossingFromRight < 0) {
				crossingFromRight = 0;
			}
			if (crossingFromRight > overlapWidth) {
				crossingFromRight = overlapWidth;
			}
			crossingFromLeft = overlapWidth - crossingFromRight;
			currentLeftWidth = startX - current->xStart;
			currentRightWidth = endX - current->xEnd;

			if (currentLeftWidth <= crossingFromRight && currentLeftWidth <= crossingFromLeft &&
				currentRightWidth >= currentLeftWidth) {
				currentLeftWidth = current->xEnd - current->xStart - crossingFromRight;
				current->xStart += currentLeftWidth;
				current->lightIntensity += (double)currentLeftWidth * current->dLightIntensityDx;
				next = current->next;
				if (next == NULL || next->xStart > current->xStart) {
					break;
				}
				if (previous == NULL) {
					g_scanlineSpanHeads[scanY] = next;
				} else {
					previous->next = next;
				}
				if (current->xStart < next->xEnd) {
					currentLeftWidth = next->xEnd - current->xStart;
					current->xStart += currentLeftWidth;
					current->lightIntensity += (double)currentLeftWidth * current->dLightIntensityDx;
				}
				insertionNext = next->next;
				while (current->xStart < current->xEnd && insertionNext != NULL &&
					   insertionNext->xStart < current->xStart) {
					if (current->xStart < insertionNext->xEnd) {
						currentLeftWidth = insertionNext->xEnd - current->xStart;
						current->xStart += currentLeftWidth;
						current->lightIntensity += (double)currentLeftWidth * current->dLightIntensityDx;
					}
					if (current->xStart >= current->xEnd) {
						break;
					}
					next = insertionNext;
					insertionNext = insertionNext->next;
				}
				if (current->xStart < current->xEnd) {
					next->next = current;
					current->next = insertionNext;
				} else {
					current->face->pSpans[scanY - current->face->yTop] = NULL;
				}
				if (previous == NULL) {
					current = g_scanlineSpanHeads[scanY];
				} else {
					current = previous->next;
				}
				continue;
			} else if (currentLeftWidth >= crossingFromRight && crossingFromLeft >= crossingFromRight &&
					   currentRightWidth >= crossingFromRight) {
				current->xEnd = startX;
				if (current->xEnd == current->xStart) {
					current->face->pSpans[scanY - current->face->yTop] = NULL;
					if (previous == NULL) {
						g_scanlineSpanHeads[scanY] = current->next;
					} else {
						previous->next = current->next;
					}
					current = current->next;
				} else {
					previous = current;
					current = current->next;
				}
				continue;
			}
			if (currentLeftWidth >= crossingFromLeft && crossingFromLeft <= crossingFromRight &&
				currentRightWidth >= crossingFromLeft) {
				startX = current->xEnd;
				if (startX >= endX) {
					return;
				}
				previous = current;
				current = current->next;
				continue;
			}
			endX = current->xEnd - crossingFromRight;
			if (startX >= endX) {
				return;
			}
			previous = current;
			current = current->next;
		} else {
			overlapWidth = endX - startX;
			newEndDepth = (double)overlapWidth * face->depthPlane[0] + newDepth;
			currentEndDepth = (double)overlapWidth * current->face->depthPlane[0] + currentDepth;
			if (newEndDepth < currentEndDepth) {
				currentLeftWidth = (int)((double)overlapWidth -
										 (newEndDepth - currentEndDepth) /
											 ((double)face->depthPlane[0] - current->face->depthPlane[0])) +
								   1;
				if (currentLeftWidth < 0) {
					currentLeftWidth = 0;
				}
				currentLeftWidth += startX;
				if (currentLeftWidth > endX) {
					currentLeftWidth = endX;
				}
				endX = currentLeftWidth;
				if (startX >= endX) {
					return;
				}
			}
			previous = current;
			current = current->next;
		}
	}

	span = g_pSceneSpanDataCur++;
	if (g_pSceneSpanDataCur == g_pSceneSpanDataEnd) {
		--g_pSceneSpanDataCur;
	}
	span->xStart = startX;
	span->xEnd = endX;
	span->face = face;
	if (face->pScanEdge == NULL) {
		span->lightIntensity = 0.0f;
	} else {
		span->lightIntensity = (float)startX;
		span->lightIntensity -= face->pScanEdge->x;
		span->lightIntensity *= face->spanLightIntensityDx;
		span->lightIntensity += face->pScanEdge->lightIntensity;
	}
	span->dLightIntensityDx = face->spanLightIntensityDx;
	face->pSpans[scanY - face->yTop] = span;
	if (previous == NULL) {
		g_scanlineSpanHeads[scanY] = span;
	} else {
		previous->next = span;
	}
	span->next = current;

	previous = span;
	while (current != NULL) {
		if (current->xStart >= span->xEnd) {
			return;
		}
		if (face->maxVertW <= current->face->minVertW) {
			if (current->xEnd >= span->xEnd) {
				span->xEnd = current->xStart;
				return;
			}
			previous = current;
			current = current->next;
			continue;
		}
		if (face->minVertW >= current->face->maxVertW) {
			if (current->xEnd <= span->xEnd) {
				current->face->pSpans[scanY - current->face->yTop] = NULL;
				previous->next = current->next;
				current = current->next;
				continue;
			}
			currentLeftWidth = span->xEnd - current->xStart;
			current->xStart += currentLeftWidth;
			current->lightIntensity += (double)currentLeftWidth * current->dLightIntensityDx;
			next = current->next;
			if (next != NULL && next->xStart < current->xStart) {
				previous->next = next;
				if (current->xStart < next->xEnd) {
					currentLeftWidth = next->xEnd - current->xStart;
					current->xStart += currentLeftWidth;
					current->lightIntensity += (double)currentLeftWidth * current->dLightIntensityDx;
				}
				insertionNext = next->next;
				while (current->xStart < current->xEnd && insertionNext != NULL &&
					   insertionNext->xStart < current->xStart) {
					if (current->xStart < insertionNext->xEnd) {
						currentLeftWidth = insertionNext->xEnd - current->xStart;
						current->xStart += currentLeftWidth;
						current->lightIntensity += (double)currentLeftWidth * current->dLightIntensityDx;
					}
					if (current->xStart >= current->xEnd) {
						break;
					}
					next = insertionNext;
					insertionNext = insertionNext->next;
				}
				if (current->xStart < current->xEnd) {
					next->next = current;
					current->next = insertionNext;
				} else {
					current->face->pSpans[scanY - current->face->yTop] = NULL;
				}
				current = previous->next;
				continue;
			}
			previous = current;
			current = current->next;
			continue;
		}

		newRowDepth = face->depthPlane[1] * (double)scanY + face->depthPlane[2];
		currentRowDepth = current->face->depthPlane[1] * (double)scanY + current->face->depthPlane[2];
		newDepth = face->depthPlane[0] * (double)(float)current->xStart + newRowDepth;
		currentDepth = current->face->depthPlane[0] * (double)(float)current->xStart + currentRowDepth;
		if (newDepth <= currentDepth) {
			if (current->face->depthPlane[0] >= face->depthPlane[0]) {
				if (current->xEnd >= span->xEnd) {
					span->xEnd = current->xStart;
					return;
				}
				previous = current;
				current = current->next;
				continue;
			}

			if (current->xEnd < span->xEnd) {
				overlapWidth = current->xEnd - current->xStart;
				newEndDepth = (double)overlapWidth * face->depthPlane[0] + newDepth;
				currentEndDepth = (double)overlapWidth * current->face->depthPlane[0] + currentDepth;
				if (newEndDepth <= currentEndDepth) {
					previous = current;
					current = current->next;
					continue;
				}
				crossingFromRight = (int)((newEndDepth - currentEndDepth) /
										  ((double)face->depthPlane[0] - current->face->depthPlane[0]));
				if (crossingFromRight < 0) {
					crossingFromRight = 0;
				}
				current->xEnd -= crossingFromRight;
				if (current->xEnd <= current->xStart) {
					current->face->pSpans[scanY - current->face->yTop] = NULL;
					previous->next = current->next;
					current = current->next;
				} else {
					previous = current;
					current = current->next;
				}
				continue;
			}

			overlapWidth = span->xEnd - current->xStart;
			newEndDepth = (double)overlapWidth * face->depthPlane[0] + newDepth;
			currentEndDepth = (double)overlapWidth * current->face->depthPlane[0] + currentDepth;
			if (newEndDepth <= currentEndDepth) {
				span->xEnd = current->xStart;
				return;
			}
			crossingFromRight = (int)((newEndDepth - currentEndDepth) /
									  ((double)face->depthPlane[0] - current->face->depthPlane[0]));
			if (crossingFromRight < 0) {
				crossingFromRight = 0;
			}
			if (crossingFromRight > overlapWidth) {
				crossingFromRight = overlapWidth;
			}
			currentLeftWidth = overlapWidth - crossingFromRight;
			if (currentLeftWidth > crossingFromRight && current->xEnd - span->xEnd > crossingFromRight) {
				span->xEnd = current->xStart;
				return;
			}
			if (current->xEnd - span->xEnd > currentLeftWidth) {
				current->xStart += overlapWidth;
				current->lightIntensity += (double)overlapWidth * current->dLightIntensityDx;
				next = current->next;
				if (next != NULL && next->xStart < current->xStart) {
					previous->next = next;
					if (current->xStart < next->xEnd) {
						currentLeftWidth = next->xEnd - current->xStart;
						current->xStart += currentLeftWidth;
						current->lightIntensity += (double)currentLeftWidth * current->dLightIntensityDx;
					}
					insertionNext = next->next;
					while (current->xStart < current->xEnd && insertionNext != NULL &&
						   insertionNext->xStart < current->xStart) {
						if (current->xStart < insertionNext->xEnd) {
							currentLeftWidth = insertionNext->xEnd - current->xStart;
							current->xStart += currentLeftWidth;
							current->lightIntensity += (double)currentLeftWidth * current->dLightIntensityDx;
						}
						if (current->xStart >= current->xEnd) {
							break;
						}
						next = insertionNext;
						insertionNext = insertionNext->next;
					}
					if (current->xStart < current->xEnd) {
						next->next = current;
						current->next = insertionNext;
					} else {
						current->face->pSpans[scanY - current->face->yTop] = NULL;
					}
					current = previous->next;
					continue;
				}
				previous = current;
				current = current->next;
			} else {
				current->xEnd = span->xEnd - crossingFromRight;
				previous = current;
				current = current->next;
			}
			continue;
		}

		if (current->face->depthPlane[0] <= face->depthPlane[0]) {
			if (current->xEnd <= span->xEnd) {
				current->face->pSpans[scanY - current->face->yTop] = NULL;
				previous->next = current->next;
				current = current->next;
				continue;
			}
			currentLeftWidth = span->xEnd - current->xStart;
			current->xStart += currentLeftWidth;
			current->lightIntensity += (double)currentLeftWidth * current->dLightIntensityDx;
			next = current->next;
			if (next != NULL && next->xStart < current->xStart) {
				previous->next = next;
				if (current->xStart < next->xEnd) {
					currentLeftWidth = next->xEnd - current->xStart;
					current->xStart += currentLeftWidth;
					current->lightIntensity += (double)currentLeftWidth * current->dLightIntensityDx;
				}
				insertionNext = next->next;
				while (current->xStart < current->xEnd && insertionNext != NULL &&
					   insertionNext->xStart < current->xStart) {
					if (current->xStart < insertionNext->xEnd) {
						currentLeftWidth = insertionNext->xEnd - current->xStart;
						current->xStart += currentLeftWidth;
						current->lightIntensity += (double)currentLeftWidth * current->dLightIntensityDx;
					}
					if (current->xStart >= current->xEnd) {
						break;
					}
					next = insertionNext;
					insertionNext = insertionNext->next;
				}
				if (current->xStart < current->xEnd) {
					next->next = current;
					current->next = insertionNext;
				} else {
					current->face->pSpans[scanY - current->face->yTop] = NULL;
				}
				current = previous->next;
				continue;
			}
			previous = current;
			current = current->next;
			continue;
		}

		if (span->xEnd >= current->xEnd) {
			overlapWidth = current->xEnd - current->xStart;
			newEndDepth = (double)overlapWidth * face->depthPlane[0] + newDepth;
			currentEndDepth = (double)overlapWidth * current->face->depthPlane[0] + currentDepth;
			if (newEndDepth >= currentEndDepth) {
				current->face->pSpans[scanY - current->face->yTop] = NULL;
				previous->next = current->next;
				current = current->next;
				continue;
			}
			crossingFromRight = (int)((newEndDepth - currentEndDepth) /
									  ((double)face->depthPlane[0] - current->face->depthPlane[0]));
			if (crossingFromRight < 0) {
				crossingFromRight = 0;
			}
			currentLeftWidth = overlapWidth - crossingFromRight;
			if (currentLeftWidth < 0) {
				currentLeftWidth = 0;
			}
			current->xStart += currentLeftWidth;
			current->lightIntensity += (double)currentLeftWidth * current->dLightIntensityDx;
			if (current->xEnd <= current->xStart) {
				current->face->pSpans[scanY - current->face->yTop] = NULL;
				previous->next = current->next;
				current = current->next;
				continue;
			}
			next = current->next;
			if (next != NULL && next->xStart < current->xStart) {
				previous->next = next;
				if (current->xStart < next->xEnd) {
					currentLeftWidth = next->xEnd - current->xStart;
					current->xStart += currentLeftWidth;
					current->lightIntensity += (double)currentLeftWidth * current->dLightIntensityDx;
				}
				insertionNext = next->next;
				while (current->xStart < current->xEnd && insertionNext != NULL &&
					   insertionNext->xStart < current->xStart) {
					if (current->xStart < insertionNext->xEnd) {
						currentLeftWidth = insertionNext->xEnd - current->xStart;
						current->xStart += currentLeftWidth;
						current->lightIntensity += (double)currentLeftWidth * current->dLightIntensityDx;
					}
					if (current->xStart >= current->xEnd) {
						break;
					}
					next = insertionNext;
					insertionNext = insertionNext->next;
				}
				if (current->xStart < current->xEnd) {
					next->next = current;
					current->next = insertionNext;
				} else {
					current->face->pSpans[scanY - current->face->yTop] = NULL;
				}
				current = previous->next;
				continue;
			}
			previous = current;
			current = current->next;
		} else {
			overlapWidth = span->xEnd - current->xStart;
			newEndDepth = (double)overlapWidth * face->depthPlane[0] + newDepth;
			currentEndDepth = (double)overlapWidth * current->face->depthPlane[0] + currentDepth;
			if (newEndDepth >= currentEndDepth) {
				current->xStart += overlapWidth;
				current->lightIntensity += (double)overlapWidth * current->dLightIntensityDx;
				next = current->next;
				if (next != NULL && next->xStart < current->xStart) {
					previous->next = next;
					if (current->xStart < next->xEnd) {
						currentLeftWidth = next->xEnd - current->xStart;
						current->xStart += currentLeftWidth;
						current->lightIntensity += (double)currentLeftWidth * current->dLightIntensityDx;
					}
					insertionNext = next->next;
					while (current->xStart < current->xEnd && insertionNext != NULL &&
						   insertionNext->xStart < current->xStart) {
						if (current->xStart < insertionNext->xEnd) {
							currentLeftWidth = insertionNext->xEnd - current->xStart;
							current->xStart += currentLeftWidth;
							current->lightIntensity += (double)currentLeftWidth * current->dLightIntensityDx;
						}
						if (current->xStart >= current->xEnd) {
							break;
						}
						next = insertionNext;
						insertionNext = insertionNext->next;
					}
					if (current->xStart < current->xEnd) {
						next->next = current;
						current->next = insertionNext;
					} else {
						current->face->pSpans[scanY - current->face->yTop] = NULL;
					}
					current = previous->next;
					continue;
				}
				previous = current;
				current = current->next;
				continue;
			}
			currentLeftWidth = (int)((double)overlapWidth -
									 (newEndDepth - currentEndDepth) /
										 ((double)face->depthPlane[0] - current->face->depthPlane[0]));
			if (currentLeftWidth < 0) {
				currentLeftWidth = 0;
			}
			if (currentLeftWidth > overlapWidth) {
				currentLeftWidth = overlapWidth;
			}
			current->xStart += currentLeftWidth;
			current->lightIntensity += (double)currentLeftWidth * current->dLightIntensityDx;
			next = current->next;
			if (next != NULL && next->xStart < current->xStart) {
				previous->next = next;
				if (current->xStart < next->xEnd) {
					currentLeftWidth = next->xEnd - current->xStart;
					current->xStart += currentLeftWidth;
					current->lightIntensity += (double)currentLeftWidth * current->dLightIntensityDx;
				}
				insertionNext = next->next;
				while (current->xStart < current->xEnd && insertionNext != NULL &&
					   insertionNext->xStart < current->xStart) {
					if (current->xStart < insertionNext->xEnd) {
						currentLeftWidth = insertionNext->xEnd - current->xStart;
						current->xStart += currentLeftWidth;
						current->lightIntensity += (double)currentLeftWidth * current->dLightIntensityDx;
					}
					if (current->xStart >= current->xEnd) {
						break;
					}
					next = insertionNext;
					insertionNext = insertionNext->next;
				}
				if (current->xStart < current->xEnd) {
					next->next = current;
					current->next = insertionNext;
				} else {
					current->face->pSpans[scanY - current->face->yTop] = NULL;
				}
				current = previous->next;
				continue;
			}
			previous = current;
			current = current->next;
		}
	}
}

// FUNCTION: XW 0x4934D0
void sw3d_DrawTexturedSpan(int startX, int endX, float depth) {
	SoftwareLightSample* lightSamples;
	SoftwareLightSample* leftSample;
	SoftwareLightSample* rightSample;
	const float* uGradient;
	const float* vGradient;
	float uAtY;
	double initialUAtY, initialVAtY;
	float vAtY;
	float viewDepthAtY;
	float uNumerator;
	float vNumerator;
	float inverseDepth;
	float u;
	float v;
	float rightU;
	float rightV;
	float leftLight;
	float rightLight;
	float lightIntensity;
	float spanLightIntensityDx;
	float lightIntensityAtEnd;
	float lightIntensityBlockStep;
	float uNumeratorBlockStep;
	float vNumeratorBlockStep;
	float depthBlockStep;
	float boundaryDepth;
	float sampleIntensity;
	double advancedIntensity;
	float fixedPointValue;
	/* The original adds an IEEE-754 exponent bias, then subtracts its integer bits. */
	float fixedPointBias;
	int fixedPointBits;
	int fixedPointBiasBits;
	int nextUQ8;
	int nextVQ8;
	int endShadeQ8;
	int shadeDeltaQ8;
	int startBlock;
	int endBlock;
	int block;
	int boundaryX;
	int blockStartX;
	int blockStartY;
	int withinBlockX;
	int withinBlockY;
	int stampDelta;

	lightSamples = g_sw3dCurrentFace->lightSamples;
	viewDepthAtY = (float)((double)(unsigned int)g_sw3dCurrentScanlineY * g_sw3dCurrentFace->depthPlane[1]);
	viewDepthAtY += g_sw3dCurrentFace->depthPlane[2];
	uGradient = &g_sw3dCurrentFace->gradients[0];
	vGradient = &g_sw3dCurrentFace->gradients[3];
	initialUAtY = (double)g_sw3dCurrentScanlineY * uGradient[1] + uGradient[2];
	initialVAtY = (double)g_sw3dCurrentScanlineY * vGradient[1] + vGradient[2];
	uAtY = (float)initialUAtY;
	vAtY = (float)initialVAtY;
	uNumerator = (float)((double)startX * uGradient[0] + initialUAtY);
	vNumerator = (float)((double)startX * vGradient[0] + initialVAtY);
	inverseDepth = g_sw3dSpanOneFloat / depth;
	spanLightIntensityDx = g_sw3dCurrentFace->spanLightIntensityDx;
	lightIntensity = ((float)startX - g_sw3dCurrentFace->pScanEdge->x) * spanLightIntensityDx +
					 g_sw3dCurrentFace->pScanEdge->lightIntensity;

	g_sw3dSpanShadeDitherAccum = g_sw3dShadeDitherInitialByScanlineParity[g_sw3dCurrentScanlineY & 1];
	startBlock = startX >> (g_sw3dLightSampleBlockShift & SW3D_SHIFT_COUNT_MASK);
	endBlock = (endX - 1) >> (g_sw3dLightSampleBlockShift & SW3D_SHIFT_COUNT_MASK);
	g_sw3dSpanStartX = startX;
	withinBlockX = startX & g_sw3dLightSampleBlockMask;
	withinBlockY = g_sw3dCurrentScanlineY & g_sw3dLightSampleBlockMask;
	blockStartX = startX - withinBlockX;
	blockStartY = g_sw3dCurrentScanlineY - withinBlockY;
	leftSample = &lightSamples[startBlock];
	stampDelta = (int32_t)((uint32_t)g_sw3dCurrentLightSampleCacheStamp - (uint32_t)leftSample->stamp);
	if (stampDelta != 0) {
		if (stampDelta != 1) {
			leftSample->stamp = g_sw3dCurrentLightSampleCacheStamp;
			leftSample->intensity = FlightLight_ComputeSoftwareFaceSampleIntensity(
				g_sw3dCurrentFace, blockStartX, blockStartY,
				depth - (float)withinBlockX * g_sw3dCurrentFace->depthPlane[0] -
					g_sw3dLightSampleSubrowFloat * g_sw3dCurrentFace->depthPlane[1]);
		} else {
			leftSample->stamp = (int32_t)((uint32_t)leftSample->stamp + 1u);
			leftSample->intensity += leftSample->intensityDelta;
		}
		sampleIntensity = FlightLight_ComputeSoftwareFaceSampleIntensity(
			g_sw3dCurrentFace, blockStartX, blockStartY + g_sw3dLightSampleBlockSize,
			depth - (float)withinBlockX * g_sw3dCurrentFace->depthPlane[0] +
				g_sw3dLightSampleRowsToNextBlockFloat * g_sw3dCurrentFace->depthPlane[1]);
		leftSample->intensityDelta = sampleIntensity - leftSample->intensity;
	}
	leftLight = leftSample->intensity + g_sw3dLightSampleSubrowLerpT * leftSample->intensityDelta;
	u = inverseDepth * uNumerator;
	v = inverseDepth * vNumerator;

	boundaryX =
		(int32_t)(((uint32_t)startBlock + 1u) << (g_sw3dLightSampleBlockShift & SW3D_SHIFT_COUNT_MASK));
	g_sw3dSpanLength = boundaryX - g_sw3dSpanStartX;
	uNumerator = (float)boundaryX * uGradient[0] + uAtY;
	vNumerator = (float)boundaryX * vGradient[0] + vAtY;
	boundaryDepth = (float)boundaryX * g_sw3dCurrentFace->depthPlane[0] + viewDepthAtY;
	block = startBlock + 1;
	inverseDepth = g_sw3dSpanOneFloat / boundaryDepth;
	rightSample = &lightSamples[block];
	stampDelta = (int32_t)((uint32_t)g_sw3dCurrentLightSampleCacheStamp - (uint32_t)rightSample->stamp);
	if (stampDelta != 0) {
		rightSample->stamp = g_sw3dCurrentLightSampleCacheStamp;
		if (stampDelta != 1) {
			rightSample->intensity = FlightLight_ComputeSoftwareFaceSampleIntensity(
				g_sw3dCurrentFace, boundaryX, blockStartY, boundaryDepth);
			sampleIntensity = FlightLight_ComputeSoftwareFaceSampleIntensity(
				g_sw3dCurrentFace, boundaryX, blockStartY + g_sw3dLightSampleBlockSize, boundaryDepth);
		} else {
			sampleIntensity = FlightLight_ComputeSoftwareFaceSampleIntensity(
				g_sw3dCurrentFace, boundaryX, blockStartY + g_sw3dLightSampleBlockSize, boundaryDepth);
			advancedIntensity = (double)rightSample->intensity + rightSample->intensityDelta;
			rightSample->intensity = (float)advancedIntensity;
		}
		rightSample->intensityDelta =
			(float)((double)sampleIntensity - (stampDelta == 1 ? advancedIntensity : rightSample->intensity));
	}
	rightLight = rightSample->intensity + g_sw3dLightSampleSubrowLerpT * rightSample->intensityDelta;
	leftLight += (rightLight - leftLight) * ((float)withinBlockX * g_sw3dLightSampleInvBlockSize);
	rightU = inverseDepth * uNumerator;
	rightV = inverseDepth * vNumerator;

	fixedPointBias = g_sw3dTexCoordBiasByShift[g_sw3dSpanTextureWidthShift];
	memcpy(&fixedPointBiasBits, &fixedPointBias, sizeof(fixedPointBiasBits));
	fixedPointValue = ((double)rightU - u) * g_sw3dSpanLengthReciprocal[g_sw3dSpanLength] + fixedPointBias;
	memcpy(&fixedPointBits, &fixedPointValue, sizeof(fixedPointBits));
	g_sw3dSpanStepVQ8 = (int32_t)((uint32_t)fixedPointBits - (uint32_t)fixedPointBiasBits);
	fixedPointBias = g_sw3dTexCoordBiasByShift[g_sw3dSpanTextureHeightShift];
	memcpy(&fixedPointBiasBits, &fixedPointBias, sizeof(fixedPointBiasBits));
	fixedPointValue = ((double)rightV - v) * g_sw3dSpanLengthReciprocal[g_sw3dSpanLength] + fixedPointBias;
	memcpy(&fixedPointBits, &fixedPointValue, sizeof(fixedPointBits));
	g_sw3dSpanStepUQ8 = (int32_t)((uint32_t)fixedPointBits - (uint32_t)fixedPointBiasBits);

	if ((unsigned int)block > (unsigned int)endBlock) {
		g_sw3dSpanLength = endX - g_sw3dSpanStartX;
	} else {
		uNumeratorBlockStep = uGradient[0] * g_sw3dLightSampleBlockSizeFloat;
		vNumeratorBlockStep = vGradient[0] * g_sw3dLightSampleBlockSizeFloat;
		depthBlockStep = g_sw3dCurrentFace->depthPlane[0] * g_sw3dLightSampleBlockSizeFloat;
	}

	lightIntensityAtEnd = (float)g_sw3dSpanLength * spanLightIntensityDx + lightIntensity;
	lightIntensityBlockStep = spanLightIntensityDx * g_sw3dLightSampleBlockSizeFloat;
	fixedPointBias = g_sw3dTexCoordBiasByShift[0];
	memcpy(&fixedPointBiasBits, &fixedPointBias, sizeof(fixedPointBiasBits));
	fixedPointValue =
		((double)leftLight + lightIntensity) * g_sw3dLightIntensityToShadeScale + fixedPointBias;
	memcpy(&fixedPointBits, &fixedPointValue, sizeof(fixedPointBits));
	g_sw3dSpanShadeQ8 = (int32_t)((uint32_t)fixedPointBits - (uint32_t)fixedPointBiasBits);
	if (g_sw3dSpanShadeQ8 < 0) {
		g_sw3dSpanShadeQ8 = 0;
	}
	if (g_sw3dSpanShadeQ8 > SW3D_MAX_SHADE_Q8) {
		g_sw3dSpanShadeQ8 = SW3D_MAX_SHADE_Q8;
	}
	fixedPointValue =
		((double)rightLight + lightIntensityAtEnd) * g_sw3dLightIntensityToShadeScale + fixedPointBias;
	memcpy(&fixedPointBits, &fixedPointValue, sizeof(fixedPointBits));
	endShadeQ8 = (int32_t)((uint32_t)fixedPointBits - (uint32_t)fixedPointBiasBits);
	if (endShadeQ8 < 0) {
		endShadeQ8 = 0;
	}
	if (endShadeQ8 > SW3D_MAX_SHADE_Q8) {
		endShadeQ8 = SW3D_MAX_SHADE_Q8;
	}
	shadeDeltaQ8 = endShadeQ8 - g_sw3dSpanShadeQ8;
	if (shadeDeltaQ8 < 0) {
		shadeDeltaQ8 += g_sw3dLightSampleBlockSize;
	}
	fixedPointBias = g_sw3dFloatToIntRoundBias;
	memcpy(&fixedPointBiasBits, &fixedPointBias, sizeof(fixedPointBiasBits));
	fixedPointValue = (double)shadeDeltaQ8 * g_sw3dSpanLengthReciprocal[g_sw3dSpanLength] + fixedPointBias;
	memcpy(&fixedPointBits, &fixedPointValue, sizeof(fixedPointBits));
	g_sw3dSpanShadeStepQ8 = (int32_t)((uint32_t)fixedPointBits - (uint32_t)fixedPointBiasBits);

	fixedPointBias = g_sw3dTexCoordBiasByShift[g_sw3dSpanTextureWidthShift];
	memcpy(&fixedPointBiasBits, &fixedPointBias, sizeof(fixedPointBiasBits));
	fixedPointValue = u + fixedPointBias;
	memcpy(&fixedPointBits, &fixedPointValue, sizeof(fixedPointBits));
	g_sw3dSpanVQ8 = (int32_t)((uint32_t)fixedPointBits - (uint32_t)fixedPointBiasBits);
	fixedPointValue = rightU + fixedPointBias;
	memcpy(&fixedPointBits, &fixedPointValue, sizeof(fixedPointBits));
	nextUQ8 = (int32_t)((uint32_t)fixedPointBits - (uint32_t)fixedPointBiasBits);
	fixedPointBias = g_sw3dTexCoordBiasByShift[g_sw3dSpanTextureHeightShift];
	memcpy(&fixedPointBiasBits, &fixedPointBias, sizeof(fixedPointBiasBits));
	fixedPointValue = v + fixedPointBias;
	memcpy(&fixedPointBits, &fixedPointValue, sizeof(fixedPointBits));
	g_sw3dSpanUQ8 = (int32_t)((uint32_t)fixedPointBits - (uint32_t)fixedPointBiasBits);
	fixedPointValue = rightV + fixedPointBias;
	memcpy(&fixedPointBits, &fixedPointValue, sizeof(fixedPointBits));
	nextVQ8 = (int32_t)((uint32_t)fixedPointBits - (uint32_t)fixedPointBiasBits);

	for (block = startBlock + 1;; ++block) {
		if (block <= endBlock) {
			boundaryDepth += depthBlockStep;
			inverseDepth = g_sw3dSpanOneFloat / boundaryDepth;
		}

		if (g_flightBytesPerPixel == FLIGHT_DISPLAY_DIRECT_BYTES_PER_PIXEL) {
			switch (g_sw3dSpanTextureWidthShift) {
				case SW3D_TEXTURE_SHIFT_8:
					switch (g_sw3dSpanTextureHeightShift) {
						case SW3D_TEXTURE_SHIFT_8:
							sw3d_DrawTexturedShadeSpan16_8x8();
							break;
						case SW3D_TEXTURE_SHIFT_16:
							sw3d_DrawTexturedShadeSpan16_8x16();
							break;
						case SW3D_TEXTURE_SHIFT_32:
							sw3d_DrawTexturedShadeSpan16_8x32();
							break;
						case SW3D_TEXTURE_SHIFT_64:
							sw3d_DrawTexturedShadeSpan16_8x64();
							break;
						case SW3D_TEXTURE_SHIFT_128:
							sw3d_DrawTexturedShadeSpan16_8x128();
							break;
						case SW3D_TEXTURE_SHIFT_256:
							sw3d_DrawTexturedShadeSpan16_8x256();
							break;
						default:
							sw3d_DrawTexturedShadeSpanGeneric();
							break;
					}
					break;
				case SW3D_TEXTURE_SHIFT_16:
					switch (g_sw3dSpanTextureHeightShift) {
						case SW3D_TEXTURE_SHIFT_8:
							sw3d_DrawTexturedShadeSpan16_16x8();
							break;
						case SW3D_TEXTURE_SHIFT_16:
							sw3d_DrawTexturedShadeSpan16_16x16();
							break;
						case SW3D_TEXTURE_SHIFT_32:
							sw3d_DrawTexturedShadeSpan16_16x32();
							break;
						case SW3D_TEXTURE_SHIFT_64:
							sw3d_DrawTexturedShadeSpan16_16x64();
							break;
						case SW3D_TEXTURE_SHIFT_128:
							sw3d_DrawTexturedShadeSpan16_16x128();
							break;
						case SW3D_TEXTURE_SHIFT_256:
							sw3d_DrawTexturedShadeSpan16_16x256();
							break;
						default:
							sw3d_DrawTexturedShadeSpanGeneric();
							break;
					}
					break;
				case SW3D_TEXTURE_SHIFT_32:
					switch (g_sw3dSpanTextureHeightShift) {
						case SW3D_TEXTURE_SHIFT_8:
							sw3d_DrawTexturedShadeSpan16_32x8();
							break;
						case SW3D_TEXTURE_SHIFT_16:
							sw3d_DrawTexturedShadeSpan16_32x16();
							break;
						case SW3D_TEXTURE_SHIFT_32:
							sw3d_DrawTexturedShadeSpan16_32x32();
							break;
						case SW3D_TEXTURE_SHIFT_64:
							sw3d_DrawTexturedShadeSpan16_32x64();
							break;
						case SW3D_TEXTURE_SHIFT_128:
							sw3d_DrawTexturedShadeSpan16_32x128();
							break;
						case SW3D_TEXTURE_SHIFT_256:
							sw3d_DrawTexturedShadeSpan16_32x256();
							break;
						default:
							sw3d_DrawTexturedShadeSpanGeneric();
							break;
					}
					break;
				case SW3D_TEXTURE_SHIFT_64:
					switch (g_sw3dSpanTextureHeightShift) {
						case SW3D_TEXTURE_SHIFT_8:
							sw3d_DrawTexturedShadeSpan16_64x8();
							break;
						case SW3D_TEXTURE_SHIFT_16:
							sw3d_DrawTexturedShadeSpan16_64x16();
							break;
						case SW3D_TEXTURE_SHIFT_32:
							sw3d_DrawTexturedShadeSpan16_64x32();
							break;
						case SW3D_TEXTURE_SHIFT_64:
							sw3d_DrawTexturedShadeSpan16_64x64();
							break;
						case SW3D_TEXTURE_SHIFT_128:
							sw3d_DrawTexturedShadeSpan16_64x128();
							break;
						case SW3D_TEXTURE_SHIFT_256:
							sw3d_DrawTexturedShadeSpan16_64x256();
							break;
						default:
							sw3d_DrawTexturedShadeSpanGeneric();
							break;
					}
					break;
				case SW3D_TEXTURE_SHIFT_128:
					switch (g_sw3dSpanTextureHeightShift) {
						case SW3D_TEXTURE_SHIFT_8:
							sw3d_DrawTexturedShadeSpan16_128x8();
							break;
						case SW3D_TEXTURE_SHIFT_16:
							sw3d_DrawTexturedShadeSpan16_128x16();
							break;
						case SW3D_TEXTURE_SHIFT_32:
							sw3d_DrawTexturedShadeSpan16_128x32();
							break;
						case SW3D_TEXTURE_SHIFT_64:
							sw3d_DrawTexturedShadeSpan16_128x64();
							break;
						case SW3D_TEXTURE_SHIFT_128:
							sw3d_DrawTexturedShadeSpan16_128x128();
							break;
						case SW3D_TEXTURE_SHIFT_256:
							sw3d_DrawTexturedShadeSpan16_128x256();
							break;
						default:
							sw3d_DrawTexturedShadeSpanGeneric();
							break;
					}
					break;
				case SW3D_TEXTURE_SHIFT_256:
					switch (g_sw3dSpanTextureHeightShift) {
						case SW3D_TEXTURE_SHIFT_8:
							sw3d_DrawTexturedShadeSpan16_256x8();
							break;
						case SW3D_TEXTURE_SHIFT_16:
							sw3d_DrawTexturedShadeSpan16_256x16();
							break;
						case SW3D_TEXTURE_SHIFT_32:
							sw3d_DrawTexturedShadeSpan16_256x32();
							break;
						case SW3D_TEXTURE_SHIFT_64:
							sw3d_DrawTexturedShadeSpan16_256x64();
							break;
						case SW3D_TEXTURE_SHIFT_128:
							sw3d_DrawTexturedShadeSpan16_256x128();
							break;
						case SW3D_TEXTURE_SHIFT_256:
							sw3d_DrawTexturedShadeSpan16_256x256();
							break;
						default:
							sw3d_DrawTexturedShadeSpanGeneric();
							break;
					}
					break;
				default:
					sw3d_DrawTexturedShadeSpanGeneric();
					break;
			}
		} else {
			/* The original unshaded branch is unreachable: its selector is always zero. */
			switch (g_sw3dSpanTextureWidthShift) {
				case SW3D_TEXTURE_SHIFT_8:
					switch (g_sw3dSpanTextureHeightShift) {
						case SW3D_TEXTURE_SHIFT_8:
							sw3d_DrawTexturedShadeSpan8_8x8();
							break;
						case SW3D_TEXTURE_SHIFT_16:
							sw3d_DrawTexturedShadeSpan8_8x16();
							break;
						case SW3D_TEXTURE_SHIFT_32:
							sw3d_DrawTexturedShadeSpan8_8x32();
							break;
						case SW3D_TEXTURE_SHIFT_64:
							sw3d_DrawTexturedShadeSpan8_8x64();
							break;
						case SW3D_TEXTURE_SHIFT_128:
							sw3d_DrawTexturedShadeSpan8_8x128();
							break;
						case SW3D_TEXTURE_SHIFT_256:
							sw3d_DrawTexturedShadeSpan8_8x256();
							break;
						default:
							sw3d_DrawTexturedShadeSpanGeneric8();
							break;
					}
					break;
				case SW3D_TEXTURE_SHIFT_16:
					switch (g_sw3dSpanTextureHeightShift) {
						case SW3D_TEXTURE_SHIFT_8:
							sw3d_DrawTexturedShadeSpan8_16x8();
							break;
						case SW3D_TEXTURE_SHIFT_16:
							sw3d_DrawTexturedShadeSpan8_16x16();
							break;
						case SW3D_TEXTURE_SHIFT_32:
							sw3d_DrawTexturedShadeSpan8_16x32();
							break;
						case SW3D_TEXTURE_SHIFT_64:
							sw3d_DrawTexturedShadeSpan8_16x64();
							break;
						case SW3D_TEXTURE_SHIFT_128:
							sw3d_DrawTexturedShadeSpan8_16x128();
							break;
						case SW3D_TEXTURE_SHIFT_256:
							sw3d_DrawTexturedShadeSpan8_16x256();
							break;
						default:
							sw3d_DrawTexturedShadeSpanGeneric8();
							break;
					}
					break;
				case SW3D_TEXTURE_SHIFT_32:
					switch (g_sw3dSpanTextureHeightShift) {
						case SW3D_TEXTURE_SHIFT_8:
							sw3d_DrawTexturedShadeSpan8_32x8();
							break;
						case SW3D_TEXTURE_SHIFT_16:
							sw3d_DrawTexturedShadeSpan8_32x16();
							break;
						case SW3D_TEXTURE_SHIFT_32:
							sw3d_DrawTexturedShadeSpan8_32x32();
							break;
						case SW3D_TEXTURE_SHIFT_64:
							sw3d_DrawTexturedShadeSpan8_32x64();
							break;
						case SW3D_TEXTURE_SHIFT_128:
							sw3d_DrawTexturedShadeSpan8_32x128();
							break;
						case SW3D_TEXTURE_SHIFT_256:
							sw3d_DrawTexturedShadeSpan8_32x256();
							break;
						default:
							sw3d_DrawTexturedShadeSpanGeneric8();
							break;
					}
					break;
				case SW3D_TEXTURE_SHIFT_64:
					switch (g_sw3dSpanTextureHeightShift) {
						case SW3D_TEXTURE_SHIFT_8:
							sw3d_DrawTexturedShadeSpan8_64x8();
							break;
						case SW3D_TEXTURE_SHIFT_16:
							sw3d_DrawTexturedShadeSpan8_64x16();
							break;
						case SW3D_TEXTURE_SHIFT_32:
							sw3d_DrawTexturedShadeSpan8_64x32();
							break;
						case SW3D_TEXTURE_SHIFT_64:
							sw3d_DrawTexturedShadeSpan8_64x64();
							break;
						case SW3D_TEXTURE_SHIFT_128:
							sw3d_DrawTexturedShadeSpan8_64x128();
							break;
						case SW3D_TEXTURE_SHIFT_256:
							sw3d_DrawTexturedShadeSpan8_64x256();
							break;
						default:
							sw3d_DrawTexturedShadeSpanGeneric8();
							break;
					}
					break;
				case SW3D_TEXTURE_SHIFT_128:
					switch (g_sw3dSpanTextureHeightShift) {
						case SW3D_TEXTURE_SHIFT_8:
							sw3d_DrawTexturedShadeSpan8_128x8();
							break;
						case SW3D_TEXTURE_SHIFT_16:
							sw3d_DrawTexturedShadeSpan8_128x16();
							break;
						case SW3D_TEXTURE_SHIFT_32:
							sw3d_DrawTexturedShadeSpan8_128x32();
							break;
						case SW3D_TEXTURE_SHIFT_64:
							sw3d_DrawTexturedShadeSpan8_128x64();
							break;
						case SW3D_TEXTURE_SHIFT_128:
							sw3d_DrawTexturedShadeSpan8_128x128();
							break;
						case SW3D_TEXTURE_SHIFT_256:
							sw3d_DrawTexturedShadeSpan8_128x256();
							break;
						default:
							sw3d_DrawTexturedShadeSpanGeneric8();
							break;
					}
					break;
				case SW3D_TEXTURE_SHIFT_256:
					switch (g_sw3dSpanTextureHeightShift) {
						case SW3D_TEXTURE_SHIFT_8:
							sw3d_DrawTexturedShadeSpan8_256x8();
							break;
						case SW3D_TEXTURE_SHIFT_16:
							sw3d_DrawTexturedShadeSpan8_256x16();
							break;
						case SW3D_TEXTURE_SHIFT_32:
							sw3d_DrawTexturedShadeSpan8_256x32();
							break;
						case SW3D_TEXTURE_SHIFT_64:
							sw3d_DrawTexturedShadeSpan8_256x64();
							break;
						case SW3D_TEXTURE_SHIFT_128:
							sw3d_DrawTexturedShadeSpan8_256x128();
							break;
						case SW3D_TEXTURE_SHIFT_256:
							sw3d_DrawTexturedShadeSpan8_256x256();
							break;
						default:
							sw3d_DrawTexturedShadeSpanGeneric8();
							break;
					}
					break;
				default:
					sw3d_DrawTexturedShadeSpanGeneric8();
					break;
			}
		}
		if (block > endBlock) {
			break;
		}

		uNumerator += uNumeratorBlockStep;
		vNumerator += vNumeratorBlockStep;
		g_sw3dSpanStartX += g_sw3dSpanLength;
		boundaryX += g_sw3dLightSampleBlockSize;
		if (block == endBlock) {
			g_sw3dSpanLength = endX - g_sw3dSpanStartX;
		} else {
			g_sw3dSpanLength = g_sw3dLightSampleBlockSize;
		}
		rightSample = &lightSamples[block + 1];
		stampDelta = (int32_t)((uint32_t)g_sw3dCurrentLightSampleCacheStamp - (uint32_t)rightSample->stamp);
		if (stampDelta != 0) {
			rightSample->stamp = g_sw3dCurrentLightSampleCacheStamp;
			if (stampDelta != 1) {
				rightSample->intensity = FlightLight_ComputeSoftwareFaceSampleIntensity(
					g_sw3dCurrentFace, boundaryX, blockStartY, boundaryDepth);
				sampleIntensity = FlightLight_ComputeSoftwareFaceSampleIntensity(
					g_sw3dCurrentFace, boundaryX, blockStartY + g_sw3dLightSampleBlockSize, boundaryDepth);
			} else {
				sampleIntensity = FlightLight_ComputeSoftwareFaceSampleIntensity(
					g_sw3dCurrentFace, boundaryX, blockStartY + g_sw3dLightSampleBlockSize, boundaryDepth);
				advancedIntensity = (double)rightSample->intensity + rightSample->intensityDelta;
				rightSample->intensity = (float)advancedIntensity;
			}
			rightSample->intensityDelta =
				(float)((double)sampleIntensity -
						(stampDelta == 1 ? advancedIntensity : rightSample->intensity));
		}
		rightU = inverseDepth * uNumerator;
		rightV = inverseDepth * vNumerator;
		rightLight = rightSample->intensity + g_sw3dLightSampleSubrowLerpT * rightSample->intensityDelta;
		lightIntensityAtEnd += lightIntensityBlockStep;
		g_sw3dSpanShadeQ8 = endShadeQ8;
		fixedPointBias = g_sw3dTexCoordBiasByShift[0];
		memcpy(&fixedPointBiasBits, &fixedPointBias, sizeof(fixedPointBiasBits));
		fixedPointValue =
			((double)rightLight + lightIntensityAtEnd) * g_sw3dLightIntensityToShadeScale + fixedPointBias;
		memcpy(&fixedPointBits, &fixedPointValue, sizeof(fixedPointBits));
		endShadeQ8 = (int32_t)((uint32_t)fixedPointBits - (uint32_t)fixedPointBiasBits);
		if (endShadeQ8 < 0) {
			endShadeQ8 = 0;
		}
		if (endShadeQ8 > SW3D_MAX_SHADE_Q8) {
			endShadeQ8 = SW3D_MAX_SHADE_Q8;
		}
		shadeDeltaQ8 = endShadeQ8 - g_sw3dSpanShadeQ8;
		if (shadeDeltaQ8 < 0) {
			shadeDeltaQ8 += g_sw3dLightSampleBlockSize;
		}
		g_sw3dSpanShadeStepQ8 = shadeDeltaQ8 >> (g_sw3dLightSampleBlockShift & SW3D_SHIFT_COUNT_MASK);
		g_sw3dSpanVQ8 = nextUQ8;
		g_sw3dSpanUQ8 = nextVQ8;
		fixedPointBias = g_sw3dTexCoordBiasByShift[g_sw3dSpanTextureWidthShift];
		memcpy(&fixedPointBiasBits, &fixedPointBias, sizeof(fixedPointBiasBits));
		fixedPointValue = rightU + fixedPointBias;
		memcpy(&fixedPointBits, &fixedPointValue, sizeof(fixedPointBits));
		nextUQ8 = (int32_t)((uint32_t)fixedPointBits - (uint32_t)fixedPointBiasBits);
		fixedPointBias = g_sw3dTexCoordBiasByShift[g_sw3dSpanTextureHeightShift];
		memcpy(&fixedPointBiasBits, &fixedPointBias, sizeof(fixedPointBiasBits));
		fixedPointValue = rightV + fixedPointBias;
		memcpy(&fixedPointBits, &fixedPointValue, sizeof(fixedPointBits));
		nextVQ8 = (int32_t)((uint32_t)fixedPointBits - (uint32_t)fixedPointBiasBits);
		g_sw3dSpanStepVQ8 =
			(nextUQ8 - g_sw3dSpanVQ8) >> (g_sw3dLightSampleBlockShift & SW3D_SHIFT_COUNT_MASK);
		g_sw3dSpanStepUQ8 =
			(nextVQ8 - g_sw3dSpanUQ8) >> (g_sw3dLightSampleBlockShift & SW3D_SHIFT_COUNT_MASK);
	}
}

// FUNCTION: XW 0x4947F0
void sw3d_DrawTexturedShadeSpan8_8x8(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int secondOffset =
				((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int firstTexel;
			unsigned int secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_8];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_8];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_8;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x494960
void sw3d_DrawTexturedShadeSpan8_8x16(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int secondOffset =
				((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int firstTexel;
			unsigned int secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			unsigned int pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_8];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_8];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_16;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x494AD0
void sw3d_DrawTexturedShadeSpan8_8x32(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int secondOffset =
				((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_8];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_8];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_32;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x494C40
void sw3d_DrawTexturedShadeSpan8_8x64(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int secondOffset =
				((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_8];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_8];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_64;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x494DB0
void sw3d_DrawTexturedShadeSpan8_8x128(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int secondOffset =
				((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_8];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_8];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_128;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x494F20
void sw3d_DrawTexturedShadeSpan8_8x256(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int secondOffset =
				((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_8];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_8];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_256;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x495090
void sw3d_DrawTexturedShadeSpan8_16x8(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_16);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_16);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int secondOffset =
				((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_16];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)(g_sw3dSpanShadeCounterScratch.shadeLevel +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_16];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)(g_sw3dSpanShadeCounterScratch.shadeLevel +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_8;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x495200
void sw3d_DrawTexturedShadeSpan8_16x16(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_16);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_16);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int secondOffset =
				((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_16];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)(g_sw3dSpanShadeCounterScratch.shadeLevel +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_16];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)(g_sw3dSpanShadeCounterScratch.shadeLevel +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_16;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x495370
void sw3d_DrawTexturedShadeSpan8_16x32(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_16);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_16);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int secondOffset =
				((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_16];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)(g_sw3dSpanShadeCounterScratch.shadeLevel +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_16];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)(g_sw3dSpanShadeCounterScratch.shadeLevel +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_32;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x4954E0
void sw3d_DrawTexturedShadeSpan8_16x64(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_16);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_16);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int secondOffset =
				((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_16];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)(g_sw3dSpanShadeCounterScratch.shadeLevel +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_16];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)(g_sw3dSpanShadeCounterScratch.shadeLevel +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_64;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x495650
void sw3d_DrawTexturedShadeSpan8_16x128(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_16);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_16);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int secondOffset =
				((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_16];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)(g_sw3dSpanShadeCounterScratch.shadeLevel +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_16];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)(g_sw3dSpanShadeCounterScratch.shadeLevel +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_128;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x4957C0
void sw3d_DrawTexturedShadeSpan8_16x256(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_16);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_16);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int secondOffset =
				((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_16];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)(g_sw3dSpanShadeCounterScratch.shadeLevel +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_16];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)(g_sw3dSpanShadeCounterScratch.shadeLevel +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_256;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x495930
void sw3d_DrawTexturedShadeSpan8_32x8(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int secondOffset =
				((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_32];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_32];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_8;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x495AA0
void sw3d_DrawTexturedShadeSpan8_32x16(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int secondOffset =
				((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_32];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_32];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_16;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x495C10
void sw3d_DrawTexturedShadeSpan8_32x32(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int secondOffset =
				((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_32];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_32];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_32;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x495D80
void sw3d_DrawTexturedShadeSpan8_32x64(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int secondOffset =
				((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_32];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_32];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_64;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x495EF0
void sw3d_DrawTexturedShadeSpan8_32x128(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_32];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_32];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_128;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x496060
void sw3d_DrawTexturedShadeSpan8_32x256(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int secondOffset =
				((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_32];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_32];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_256;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x4961D0
void sw3d_DrawTexturedShadeSpan8_64x8(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_64];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_64];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_8;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			memcpy(&destination[pixelIndex], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x496340
void sw3d_DrawTexturedShadeSpan8_64x16(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_64];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_64];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_16;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			memcpy(&destination[pixelIndex], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x4964B0
void sw3d_DrawTexturedShadeSpan8_64x32(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_64];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex =
				(uint8_t)((shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) + (ditherSum > SW3D_TEXTURE_COLUMN_MASK));
			firstShadeIndex = (firstShadeIndex << SW3D_SHADE_FRACTION_BITS) | firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_64];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex =
				(uint8_t)((shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) + (ditherSum > SW3D_TEXTURE_COLUMN_MASK));
			secondShadeIndex = (secondShadeIndex << SW3D_SHADE_FRACTION_BITS) | secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_32;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			memcpy(&destination[pixelIndex], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x496620
void sw3d_DrawTexturedShadeSpan8_64x64(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_64];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex =
				(uint8_t)((shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) + (ditherSum > SW3D_TEXTURE_COLUMN_MASK));
			firstShadeIndex = (firstShadeIndex << SW3D_SHADE_FRACTION_BITS) | firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_64];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex =
				(uint8_t)((shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) + (ditherSum > SW3D_TEXTURE_COLUMN_MASK));
			secondShadeIndex = (secondShadeIndex << SW3D_SHADE_FRACTION_BITS) | secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_64;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			memcpy(&destination[pixelIndex], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x496790
void sw3d_DrawTexturedShadeSpan8_64x128(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_64];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex =
				(uint8_t)((shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) + (ditherSum > SW3D_TEXTURE_COLUMN_MASK));
			firstShadeIndex = (firstShadeIndex << SW3D_SHADE_FRACTION_BITS) | firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_64];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex =
				(uint8_t)((shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) + (ditherSum > SW3D_TEXTURE_COLUMN_MASK));
			secondShadeIndex = (secondShadeIndex << SW3D_SHADE_FRACTION_BITS) | secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_128;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			memcpy(&destination[pixelIndex], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x496900
void sw3d_DrawTexturedShadeSpan8_64x256(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset = ((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
										(SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
									   (uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_64];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex =
				(uint8_t)((shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) + (ditherSum > SW3D_TEXTURE_COLUMN_MASK));
			firstShadeIndex = (firstShadeIndex << SW3D_SHADE_FRACTION_BITS) | firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_64];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex =
				(uint8_t)((shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) + (ditherSum > SW3D_TEXTURE_COLUMN_MASK));
			secondShadeIndex = (secondShadeIndex << SW3D_SHADE_FRACTION_BITS) | secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_256;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			memcpy(&destination[pixelIndex], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x496A70
void sw3d_DrawTexturedShadeSpan8_128x8(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			unsigned int pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_128];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum >> SW3D_SHADE_FRACTION_BITS))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_128];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum >> SW3D_SHADE_FRACTION_BITS))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_8;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x496BE0
void sw3d_DrawTexturedShadeSpan8_128x16(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int firstTexel;
			unsigned int secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_128];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_128];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_16;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x496D50
void sw3d_DrawTexturedShadeSpan8_128x32(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int firstTexel;
			unsigned int secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_128];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_128];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_32;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x496EC0
void sw3d_DrawTexturedShadeSpan8_128x64(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int firstTexel;
			unsigned int secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_128];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_128];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_64;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x497030
void sw3d_DrawTexturedShadeSpan8_128x128(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int firstTexel;
			unsigned int secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_128];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_128];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_128;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x4971A0
void sw3d_DrawTexturedShadeSpan8_128x256(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int secondOffset =
				((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_128];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_128];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_256;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x497310
void sw3d_DrawTexturedShadeSpan8_256x8(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)g_sw3dSpanVQ8;
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)g_sw3dSpanStepVQ8;
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int secondOffset =
				((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)(g_sw3dSpanShadeCounterScratch.shadeLevel +
													   (ditherSum >> SW3D_SHADE_FRACTION_BITS))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)(g_sw3dSpanShadeCounterScratch.shadeLevel +
														(ditherSum >> SW3D_SHADE_FRACTION_BITS))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_8;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x497480
void sw3d_DrawTexturedShadeSpan8_256x16(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)g_sw3dSpanVQ8;
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)g_sw3dSpanStepVQ8;
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			uint16_t ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum >> SW3D_SHADE_FRACTION_BITS))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum >> SW3D_SHADE_FRACTION_BITS))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_16;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x4975F0
void sw3d_DrawTexturedShadeSpan8_256x32(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)g_sw3dSpanVQ8;
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)g_sw3dSpanStepVQ8;
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum >> SW3D_SHADE_FRACTION_BITS))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum >> SW3D_SHADE_FRACTION_BITS))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_32;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x497760
void sw3d_DrawTexturedShadeSpan8_256x64(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)g_sw3dSpanVQ8;
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)g_sw3dSpanStepVQ8;
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum >> SW3D_SHADE_FRACTION_BITS))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum >> SW3D_SHADE_FRACTION_BITS))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_64;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x4978D0
void sw3d_DrawTexturedShadeSpan8_256x128(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)g_sw3dSpanVQ8;
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)g_sw3dSpanStepVQ8;
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum >> SW3D_SHADE_FRACTION_BITS))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum >> SW3D_SHADE_FRACTION_BITS))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_128;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x497A40
void sw3d_DrawTexturedShadeSpan8_256x256(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)g_sw3dSpanVQ8;
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)g_sw3dSpanStepVQ8;
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint16_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum >> SW3D_SHADE_FRACTION_BITS))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				destination[pixelIndex] = shadeTable[firstShadeIndex];
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			pixelPair = shadeTable[firstShadeIndex];
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum >> SW3D_SHADE_FRACTION_BITS))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_256;
			g_sw3dSpanPackedUVScratch = packedUV;
			pixelPair |= shadeTable[secondShadeIndex] << SW3D_TEXTURE_FRACTION_BITS;
			*(uint16_t*)&destination[pixelIndex] = pixelPair;
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x497BB0
void sw3d_DrawTexturedShadeSpan16_8x8(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelOffset;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
		for (pixelOffset = 0;; pixelOffset += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint32_t firstPixel;
			uint32_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_8];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
							   ~SW3D_TEXTURE_COLUMN_MASK) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_8];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
							 ((unsigned int)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
							  << SW3D_TEXTURE_FRACTION_BITS);
				memcpy(&destination[pixelOffset], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
						 ((unsigned int)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
						  << SW3D_TEXTURE_FRACTION_BITS);
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
								~SW3D_TEXTURE_COLUMN_MASK) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_8;
			g_sw3dSpanPackedUVScratch = packedUV;
			secondPixel = shadeTable[secondShadeIndex * sizeof(uint16_t)] |
						  ((unsigned int)shadeTable[secondShadeIndex * sizeof(uint16_t) + 1]
						   << SW3D_TEXTURE_FRACTION_BITS);
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelOffset], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x497D50
void sw3d_DrawTexturedShadeSpan16_8x16(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelOffset;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
		for (pixelOffset = 0;; pixelOffset += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint32_t firstPixel;
			uint32_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_8];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
							   ~SW3D_TEXTURE_COLUMN_MASK) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_8];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
							 ((unsigned int)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
							  << SW3D_TEXTURE_FRACTION_BITS);
				memcpy(&destination[pixelOffset], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
						 ((unsigned int)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
						  << SW3D_TEXTURE_FRACTION_BITS);
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
								~SW3D_TEXTURE_COLUMN_MASK) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_16;
			g_sw3dSpanPackedUVScratch = packedUV;
			secondPixel = shadeTable[secondShadeIndex * sizeof(uint16_t)] |
						  ((unsigned int)shadeTable[secondShadeIndex * sizeof(uint16_t) + 1]
						   << SW3D_TEXTURE_FRACTION_BITS);
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelOffset], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x497EF0
void sw3d_DrawTexturedShadeSpan16_8x32(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelOffset;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
		for (pixelOffset = 0;; pixelOffset += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint32_t firstPixel;
			uint32_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_8];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
							   ~SW3D_TEXTURE_COLUMN_MASK) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_8];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
							 ((unsigned int)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
							  << SW3D_TEXTURE_FRACTION_BITS);
				memcpy(&destination[pixelOffset], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
						 ((unsigned int)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
						  << SW3D_TEXTURE_FRACTION_BITS);
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
								~SW3D_TEXTURE_COLUMN_MASK) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_32;
			g_sw3dSpanPackedUVScratch = packedUV;
			secondPixel = shadeTable[secondShadeIndex * sizeof(uint16_t)] |
						  ((unsigned int)shadeTable[secondShadeIndex * sizeof(uint16_t) + 1]
						   << SW3D_TEXTURE_FRACTION_BITS);
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelOffset], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x498090
void sw3d_DrawTexturedShadeSpan16_8x64(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelOffset;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
		for (pixelOffset = 0;; pixelOffset += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint32_t firstPixel;
			uint32_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_8];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
							   ~SW3D_TEXTURE_COLUMN_MASK) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_8];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
							 ((unsigned int)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
							  << SW3D_TEXTURE_FRACTION_BITS);
				memcpy(&destination[pixelOffset], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
						 ((unsigned int)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
						  << SW3D_TEXTURE_FRACTION_BITS);
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
								~SW3D_TEXTURE_COLUMN_MASK) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_64;
			g_sw3dSpanPackedUVScratch = packedUV;
			secondPixel = shadeTable[secondShadeIndex * sizeof(uint16_t)] |
						  ((unsigned int)shadeTable[secondShadeIndex * sizeof(uint16_t) + 1]
						   << SW3D_TEXTURE_FRACTION_BITS);
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelOffset], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x498230
void sw3d_DrawTexturedShadeSpan16_8x128(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelOffset;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
		for (pixelOffset = 0;; pixelOffset += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint32_t firstPixel;
			uint32_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_8];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
							   ~SW3D_TEXTURE_COLUMN_MASK) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_8];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
							 ((unsigned int)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
							  << SW3D_TEXTURE_FRACTION_BITS);
				memcpy(&destination[pixelOffset], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
						 ((unsigned int)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
						  << SW3D_TEXTURE_FRACTION_BITS);
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
								~SW3D_TEXTURE_COLUMN_MASK) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_128;
			g_sw3dSpanPackedUVScratch = packedUV;
			secondPixel = shadeTable[secondShadeIndex * sizeof(uint16_t)] |
						  ((unsigned int)shadeTable[secondShadeIndex * sizeof(uint16_t) + 1]
						   << SW3D_TEXTURE_FRACTION_BITS);
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelOffset], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x4983D0
void sw3d_DrawTexturedShadeSpan16_8x256(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelOffset;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
		for (pixelOffset = 0;; pixelOffset += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint16_t firstPixel;
			uint16_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_8];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
							   ~SW3D_TEXTURE_COLUMN_MASK) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_8];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
				memcpy(&destination[pixelOffset], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
								~SW3D_TEXTURE_COLUMN_MASK) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_256;
			g_sw3dSpanPackedUVScratch = packedUV;
			memcpy(&secondPixel, &shadeTable[secondShadeIndex * sizeof(secondPixel)], sizeof(secondPixel));
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelOffset], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x498570
void sw3d_DrawTexturedShadeSpan16_16x8(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_16);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_16);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelOffset;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
		for (pixelOffset = 0;; pixelOffset += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint32_t firstPixel;
			uint32_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_16];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
							   ~SW3D_TEXTURE_COLUMN_MASK) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_16];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
							 ((unsigned int)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
							  << SW3D_TEXTURE_FRACTION_BITS);
				memcpy(&destination[pixelOffset], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
						 ((unsigned int)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
						  << SW3D_TEXTURE_FRACTION_BITS);
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
								~SW3D_TEXTURE_COLUMN_MASK) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_8;
			g_sw3dSpanPackedUVScratch = packedUV;
			secondPixel = shadeTable[secondShadeIndex * sizeof(uint16_t)] |
						  ((unsigned int)shadeTable[secondShadeIndex * sizeof(uint16_t) + 1]
						   << SW3D_TEXTURE_FRACTION_BITS);
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelOffset], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x498710
void sw3d_DrawTexturedShadeSpan16_16x16(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_16);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_16);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelOffset;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
		for (pixelOffset = 0;; pixelOffset += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint16_t firstPixel;
			uint16_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_16];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
							   ~SW3D_TEXTURE_COLUMN_MASK) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_16];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
				memcpy(&destination[pixelOffset], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
								~SW3D_TEXTURE_COLUMN_MASK) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_16;
			g_sw3dSpanPackedUVScratch = packedUV;
			memcpy(&secondPixel, &shadeTable[secondShadeIndex * sizeof(secondPixel)], sizeof(secondPixel));
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelOffset], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x4988B0
void sw3d_DrawTexturedShadeSpan16_16x32(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_16);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_16);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelOffset;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
		for (pixelOffset = 0;; pixelOffset += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint16_t firstPixel;
			uint16_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_16];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
							   ~SW3D_TEXTURE_COLUMN_MASK) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_16];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
				memcpy(&destination[pixelOffset], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
								~SW3D_TEXTURE_COLUMN_MASK) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_32;
			g_sw3dSpanPackedUVScratch = packedUV;
			memcpy(&secondPixel, &shadeTable[secondShadeIndex * sizeof(secondPixel)], sizeof(secondPixel));
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelOffset], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x498A50
void sw3d_DrawTexturedShadeSpan16_16x64(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_16);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_16);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelOffset;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
		for (pixelOffset = 0;; pixelOffset += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint16_t firstPixel;
			uint16_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_16];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
							   ~SW3D_TEXTURE_COLUMN_MASK) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_16];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
				memcpy(&destination[pixelOffset], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
								~SW3D_TEXTURE_COLUMN_MASK) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_64;
			g_sw3dSpanPackedUVScratch = packedUV;
			memcpy(&secondPixel, &shadeTable[secondShadeIndex * sizeof(secondPixel)], sizeof(secondPixel));
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelOffset], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x498BF0
void sw3d_DrawTexturedShadeSpan16_16x128(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_16);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_16);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelOffset;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
		for (pixelOffset = 0;; pixelOffset += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint16_t firstPixel;
			uint16_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_16];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
							   ~SW3D_TEXTURE_COLUMN_MASK) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_16];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
				memcpy(&destination[pixelOffset], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
								~SW3D_TEXTURE_COLUMN_MASK) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_128;
			g_sw3dSpanPackedUVScratch = packedUV;
			memcpy(&secondPixel, &shadeTable[secondShadeIndex * sizeof(secondPixel)], sizeof(secondPixel));
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelOffset], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x498D90
void sw3d_DrawTexturedShadeSpan16_16x256(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_16);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_16);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelOffset;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
		for (pixelOffset = 0;; pixelOffset += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint16_t firstPixel;
			uint16_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_16];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
							   ~SW3D_TEXTURE_COLUMN_MASK) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_16];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
				memcpy(&destination[pixelOffset], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
								~SW3D_TEXTURE_COLUMN_MASK) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_256;
			g_sw3dSpanPackedUVScratch = packedUV;
			memcpy(&secondPixel, &shadeTable[secondShadeIndex * sizeof(secondPixel)], sizeof(secondPixel));
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelOffset], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x498F30
void sw3d_DrawTexturedShadeSpan16_32x8(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelOffset;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
		for (pixelOffset = 0;; pixelOffset += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint16_t firstPixel;
			uint16_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_32];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
							   ~SW3D_TEXTURE_COLUMN_MASK) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_32];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
				memcpy(&destination[pixelOffset], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
								~SW3D_TEXTURE_COLUMN_MASK) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_8;
			g_sw3dSpanPackedUVScratch = packedUV;
			memcpy(&secondPixel, &shadeTable[secondShadeIndex * sizeof(secondPixel)], sizeof(secondPixel));
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelOffset], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x4990D0
void sw3d_DrawTexturedShadeSpan16_32x16(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelOffset;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
		for (pixelOffset = 0;; pixelOffset += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint16_t firstPixel;
			uint16_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_32];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
							   ~SW3D_TEXTURE_COLUMN_MASK) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_32];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
				memcpy(&destination[pixelOffset], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
								~SW3D_TEXTURE_COLUMN_MASK) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_16;
			g_sw3dSpanPackedUVScratch = packedUV;
			memcpy(&secondPixel, &shadeTable[secondShadeIndex * sizeof(secondPixel)], sizeof(secondPixel));
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelOffset], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x499270
void sw3d_DrawTexturedShadeSpan16_32x32(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelOffset;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
		for (pixelOffset = 0;; pixelOffset += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint16_t firstPixel;
			uint16_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_32];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
							   ~SW3D_TEXTURE_COLUMN_MASK) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_32];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
				memcpy(&destination[pixelOffset], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
								~SW3D_TEXTURE_COLUMN_MASK) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_32;
			g_sw3dSpanPackedUVScratch = packedUV;
			memcpy(&secondPixel, &shadeTable[secondShadeIndex * sizeof(secondPixel)], sizeof(secondPixel));
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelOffset], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x499410
void sw3d_DrawTexturedShadeSpan16_32x64(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelOffset;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
		for (pixelOffset = 0;; pixelOffset += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint16_t firstPixel;
			uint16_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_32];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
							   ~SW3D_TEXTURE_COLUMN_MASK) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_32];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
				memcpy(&destination[pixelOffset], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
								~SW3D_TEXTURE_COLUMN_MASK) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_64;
			g_sw3dSpanPackedUVScratch = packedUV;
			memcpy(&secondPixel, &shadeTable[secondShadeIndex * sizeof(secondPixel)], sizeof(secondPixel));
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelOffset], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x4995B0
void sw3d_DrawTexturedShadeSpan16_32x128(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelOffset;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
		for (pixelOffset = 0;; pixelOffset += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint16_t firstPixel;
			uint16_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_32];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
							   ~SW3D_TEXTURE_COLUMN_MASK) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_32];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
				memcpy(&destination[pixelOffset], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
								~SW3D_TEXTURE_COLUMN_MASK) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_128;
			g_sw3dSpanPackedUVScratch = packedUV;
			memcpy(&secondPixel, &shadeTable[secondShadeIndex * sizeof(secondPixel)], sizeof(secondPixel));
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelOffset], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x499750
void sw3d_DrawTexturedShadeSpan16_32x256(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelOffset;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
		for (pixelOffset = 0;; pixelOffset += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint16_t firstPixel;
			uint16_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_32];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
							   ~SW3D_TEXTURE_COLUMN_MASK) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_32];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
				memcpy(&destination[pixelOffset], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
								~SW3D_TEXTURE_COLUMN_MASK) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_256;
			g_sw3dSpanPackedUVScratch = packedUV;
			memcpy(&secondPixel, &shadeTable[secondShadeIndex * sizeof(secondPixel)], sizeof(secondPixel));
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelOffset], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x4998F0
void sw3d_DrawTexturedShadeSpan16_64x8(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelOffset;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
		for (pixelOffset = 0;; pixelOffset += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint32_t firstPixel;
			uint32_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_64];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
							   ~SW3D_TEXTURE_COLUMN_MASK) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_64];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
							 ((uint32_t)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
							  << SW3D_TEXTURE_FRACTION_BITS);
				memcpy(&destination[pixelOffset], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstPixel =
				shadeTable[firstShadeIndex * sizeof(uint16_t)] |
				((uint32_t)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1] << SW3D_TEXTURE_FRACTION_BITS);
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
								~SW3D_TEXTURE_COLUMN_MASK) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_8;
			g_sw3dSpanPackedUVScratch = packedUV;
			secondPixel =
				shadeTable[secondShadeIndex * sizeof(uint16_t)] |
				((uint32_t)shadeTable[secondShadeIndex * sizeof(uint16_t) + 1] << SW3D_TEXTURE_FRACTION_BITS);
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelOffset], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x499A90
void sw3d_DrawTexturedShadeSpan16_64x16(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelOffset;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
		for (pixelOffset = 0;; pixelOffset += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint32_t firstPixel;
			uint32_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_64];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
							   ~SW3D_TEXTURE_COLUMN_MASK) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_64];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
							 ((uint32_t)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
							  << SW3D_TEXTURE_FRACTION_BITS);
				memcpy(&destination[pixelOffset], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstPixel =
				shadeTable[firstShadeIndex * sizeof(uint16_t)] |
				((uint32_t)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1] << SW3D_TEXTURE_FRACTION_BITS);
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
								~SW3D_TEXTURE_COLUMN_MASK) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_16;
			g_sw3dSpanPackedUVScratch = packedUV;
			secondPixel =
				shadeTable[secondShadeIndex * sizeof(uint16_t)] |
				((uint32_t)shadeTable[secondShadeIndex * sizeof(uint16_t) + 1] << SW3D_TEXTURE_FRACTION_BITS);
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelOffset], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x499C30
void sw3d_DrawTexturedShadeSpan16_64x32(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelOffset;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
		for (pixelOffset = 0;; pixelOffset += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint32_t firstPixel;
			uint32_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_64];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
							   ~SW3D_TEXTURE_COLUMN_MASK) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_64];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
							 ((uint32_t)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
							  << SW3D_TEXTURE_FRACTION_BITS);
				memcpy(&destination[pixelOffset], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstPixel =
				shadeTable[firstShadeIndex * sizeof(uint16_t)] |
				((uint32_t)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1] << SW3D_TEXTURE_FRACTION_BITS);
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
								~SW3D_TEXTURE_COLUMN_MASK) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_32;
			g_sw3dSpanPackedUVScratch = packedUV;
			secondPixel =
				shadeTable[secondShadeIndex * sizeof(uint16_t)] |
				((uint32_t)shadeTable[secondShadeIndex * sizeof(uint16_t) + 1] << SW3D_TEXTURE_FRACTION_BITS);
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelOffset], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x499DD0
void sw3d_DrawTexturedShadeSpan16_64x64(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelOffset;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
		for (pixelOffset = 0;; pixelOffset += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint32_t firstPixel;
			uint32_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_64];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
							   ~SW3D_TEXTURE_COLUMN_MASK) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_64];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
							 ((uint32_t)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
							  << SW3D_TEXTURE_FRACTION_BITS);
				memcpy(&destination[pixelOffset], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstPixel =
				shadeTable[firstShadeIndex * sizeof(uint16_t)] |
				((uint32_t)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1] << SW3D_TEXTURE_FRACTION_BITS);
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
								~SW3D_TEXTURE_COLUMN_MASK) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_64;
			g_sw3dSpanPackedUVScratch = packedUV;
			secondPixel =
				shadeTable[secondShadeIndex * sizeof(uint16_t)] |
				((uint32_t)shadeTable[secondShadeIndex * sizeof(uint16_t) + 1] << SW3D_TEXTURE_FRACTION_BITS);
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelOffset], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x499F70
void sw3d_DrawTexturedShadeSpan16_64x128(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelOffset;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
		for (pixelOffset = 0;; pixelOffset += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint32_t firstPixel;
			uint32_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_64];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
							   ~SW3D_TEXTURE_COLUMN_MASK) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_64];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
							 ((uint32_t)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
							  << SW3D_TEXTURE_FRACTION_BITS);
				memcpy(&destination[pixelOffset], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstPixel =
				shadeTable[firstShadeIndex * sizeof(uint16_t)] |
				((uint32_t)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1] << SW3D_TEXTURE_FRACTION_BITS);
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
								~SW3D_TEXTURE_COLUMN_MASK) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_128;
			g_sw3dSpanPackedUVScratch = packedUV;
			secondPixel =
				shadeTable[secondShadeIndex * sizeof(uint16_t)] |
				((uint32_t)shadeTable[secondShadeIndex * sizeof(uint16_t) + 1] << SW3D_TEXTURE_FRACTION_BITS);
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelOffset], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x49A110
void sw3d_DrawTexturedShadeSpan16_64x256(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelOffset;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
		for (pixelOffset = 0;; pixelOffset += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint16_t firstPixel;
			uint16_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_64];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
							   ~SW3D_TEXTURE_COLUMN_MASK) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_64];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
				memcpy(&destination[pixelOffset], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
								~SW3D_TEXTURE_COLUMN_MASK) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_256;
			g_sw3dSpanPackedUVScratch = packedUV;
			memcpy(&secondPixel, &shadeTable[secondShadeIndex * sizeof(secondPixel)], sizeof(secondPixel));
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelOffset], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x49A2B0
void sw3d_DrawTexturedShadeSpan16_128x8(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint32_t firstPixel;
			uint32_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_128];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_128];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
							 ((unsigned int)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
							  << SW3D_TEXTURE_FRACTION_BITS);
				memcpy(&destination[pixelIndex], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
						 ((unsigned int)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
						  << SW3D_TEXTURE_FRACTION_BITS);
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_8;
			g_sw3dSpanPackedUVScratch = packedUV;
			secondPixel = shadeTable[secondShadeIndex * sizeof(uint16_t)] |
						  ((unsigned int)shadeTable[secondShadeIndex * sizeof(uint16_t) + 1]
						   << SW3D_TEXTURE_FRACTION_BITS);
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelIndex], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x49A440
void sw3d_DrawTexturedShadeSpan16_128x16(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint32_t firstPixel;
			uint32_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_128];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_128];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
							 ((unsigned int)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
							  << SW3D_TEXTURE_FRACTION_BITS);
				memcpy(&destination[pixelIndex], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
						 ((unsigned int)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
						  << SW3D_TEXTURE_FRACTION_BITS);
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_16;
			g_sw3dSpanPackedUVScratch = packedUV;
			secondPixel = shadeTable[secondShadeIndex * sizeof(uint16_t)] |
						  ((unsigned int)shadeTable[secondShadeIndex * sizeof(uint16_t) + 1]
						   << SW3D_TEXTURE_FRACTION_BITS);
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelIndex], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x49A5D0
void sw3d_DrawTexturedShadeSpan16_128x32(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint32_t firstPixel;
			uint32_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_128];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_128];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
							 ((unsigned int)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
							  << SW3D_TEXTURE_FRACTION_BITS);
				memcpy(&destination[pixelIndex], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
						 ((unsigned int)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
						  << SW3D_TEXTURE_FRACTION_BITS);
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_32;
			g_sw3dSpanPackedUVScratch = packedUV;
			secondPixel = shadeTable[secondShadeIndex * sizeof(uint16_t)] |
						  ((unsigned int)shadeTable[secondShadeIndex * sizeof(uint16_t) + 1]
						   << SW3D_TEXTURE_FRACTION_BITS);
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelIndex], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x49A760
void sw3d_DrawTexturedShadeSpan16_128x64(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint16_t firstPixel;
			uint16_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_128];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex =
				(((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (ditherSum & ~SW3D_TEXTURE_COLUMN_MASK)) &
				 ~SW3D_TEXTURE_COLUMN_MASK) |
				firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_128];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
				memcpy(&destination[pixelIndex], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex =
				(((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (ditherSum & ~SW3D_TEXTURE_COLUMN_MASK)) &
				 ~SW3D_TEXTURE_COLUMN_MASK) |
				secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_64;
			g_sw3dSpanPackedUVScratch = packedUV;
			memcpy(&secondPixel, &shadeTable[secondShadeIndex * sizeof(secondPixel)], sizeof(secondPixel));
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelIndex], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x49A8F0
void sw3d_DrawTexturedShadeSpan16_128x128(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pairIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
		for (pairIndex = 0;; ++pairIndex) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint16_t firstPixel;
			uint16_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_128];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
							   ~SW3D_TEXTURE_COLUMN_MASK) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_128];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
				memcpy(&destination[pairIndex * sizeof(uint32_t)], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
								~SW3D_TEXTURE_COLUMN_MASK) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_128;
			g_sw3dSpanPackedUVScratch = packedUV;
			memcpy(&secondPixel, &shadeTable[secondShadeIndex * sizeof(secondPixel)], sizeof(secondPixel));
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pairIndex * sizeof(uint32_t)], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x49AA80
void sw3d_DrawTexturedShadeSpan16_128x256(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pairIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
		for (pairIndex = 0;; ++pairIndex) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			uint16_t firstShadeIndex;
			uint16_t secondShadeIndex;
			uint16_t firstPixel;
			uint16_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset >> SW3D_PACKED_V_SHIFT_128];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
							   ~SW3D_TEXTURE_COLUMN_MASK) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset >> SW3D_PACKED_V_SHIFT_128];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
				memcpy(&destination[pairIndex * sizeof(uint32_t)], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(firstPixel)], sizeof(firstPixel));
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = (((shadeAndCount >> SW3D_SHADE_FRACTION_BITS) + (uint8_t)shadeAndCount) &
								~SW3D_TEXTURE_COLUMN_MASK) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_256;
			g_sw3dSpanPackedUVScratch = packedUV;
			memcpy(&secondPixel, &shadeTable[secondShadeIndex * sizeof(secondPixel)], sizeof(secondPixel));
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pairIndex * sizeof(uint32_t)], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x49AC10
void sw3d_DrawTexturedShadeSpan16_256x8(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)g_sw3dSpanVQ8;
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)g_sw3dSpanStepVQ8;
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint32_t firstPixel;
			uint32_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
							 ((unsigned int)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
							  << SW3D_TEXTURE_FRACTION_BITS);
				memcpy(&destination[pixelIndex], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
						 ((unsigned int)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
						  << SW3D_TEXTURE_FRACTION_BITS);
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_8;
			g_sw3dSpanPackedUVScratch = packedUV;
			secondPixel = shadeTable[secondShadeIndex * sizeof(uint16_t)] |
						  ((unsigned int)shadeTable[secondShadeIndex * sizeof(uint16_t) + 1]
						   << SW3D_TEXTURE_FRACTION_BITS);
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelIndex], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x49ADA0
void sw3d_DrawTexturedShadeSpan16_256x16(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)g_sw3dSpanVQ8;
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)g_sw3dSpanStepVQ8;
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint32_t firstPixel;
			uint32_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
							 ((unsigned int)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
							  << SW3D_TEXTURE_FRACTION_BITS);
				memcpy(&destination[pixelIndex], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
						 ((unsigned int)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
						  << SW3D_TEXTURE_FRACTION_BITS);
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_16;
			g_sw3dSpanPackedUVScratch = packedUV;
			secondPixel = shadeTable[secondShadeIndex * sizeof(uint16_t)] |
						  ((unsigned int)shadeTable[secondShadeIndex * sizeof(uint16_t) + 1]
						   << SW3D_TEXTURE_FRACTION_BITS);
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelIndex], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x49AF30
void sw3d_DrawTexturedShadeSpan16_256x32(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)g_sw3dSpanVQ8;
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)g_sw3dSpanStepVQ8;
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint32_t firstPixel;
			uint32_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
							 ((unsigned int)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
							  << SW3D_TEXTURE_FRACTION_BITS);
				memcpy(&destination[pixelIndex], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
						 ((unsigned int)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
						  << SW3D_TEXTURE_FRACTION_BITS);
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_32;
			g_sw3dSpanPackedUVScratch = packedUV;
			secondPixel = shadeTable[secondShadeIndex * sizeof(uint16_t)] |
						  ((unsigned int)shadeTable[secondShadeIndex * sizeof(uint16_t) + 1]
						   << SW3D_TEXTURE_FRACTION_BITS);
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelIndex], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x49B0C0
void sw3d_DrawTexturedShadeSpan16_256x64(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)g_sw3dSpanVQ8;
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)g_sw3dSpanStepVQ8;
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint32_t firstPixel;
			uint32_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
							 ((unsigned int)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
							  << SW3D_TEXTURE_FRACTION_BITS);
				memcpy(&destination[pixelIndex], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
						 ((unsigned int)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
						  << SW3D_TEXTURE_FRACTION_BITS);
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_64;
			g_sw3dSpanPackedUVScratch = packedUV;
			secondPixel = shadeTable[secondShadeIndex * sizeof(uint16_t)] |
						  ((unsigned int)shadeTable[secondShadeIndex * sizeof(uint16_t) + 1]
						   << SW3D_TEXTURE_FRACTION_BITS);
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelIndex], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x49B250
void sw3d_DrawTexturedShadeSpan16_256x128(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)g_sw3dSpanVQ8;
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)g_sw3dSpanStepVQ8;
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint32_t)) {
			uint16_t firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t secondOffset = ((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
									 << SW3D_TEXTURE_FRACTION_BITS) |
									(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			unsigned int ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint32_t firstPixel;
			uint32_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum > SW3D_TEXTURE_COLUMN_MASK))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
							 ((unsigned int)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
							  << SW3D_TEXTURE_FRACTION_BITS);
				memcpy(&destination[pixelIndex], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstPixel = shadeTable[firstShadeIndex * sizeof(uint16_t)] |
						 ((unsigned int)shadeTable[firstShadeIndex * sizeof(uint16_t) + 1]
						  << SW3D_TEXTURE_FRACTION_BITS);
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum > SW3D_TEXTURE_COLUMN_MASK))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_128;
			g_sw3dSpanPackedUVScratch = packedUV;
			secondPixel = shadeTable[secondShadeIndex * sizeof(uint16_t)] |
						  ((unsigned int)shadeTable[secondShadeIndex * sizeof(uint16_t) + 1]
						   << SW3D_TEXTURE_FRACTION_BITS);
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelIndex], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x49B3E0
void sw3d_DrawTexturedShadeSpan16_256x256(void) {
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	if ((uint8_t)g_sw3dSpanLength != 0) {
		uint8_t* destination =
			g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX * (ptrdiff_t)sizeof(uint16_t);
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)g_sw3dSpanVQ8;
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)g_sw3dSpanStepVQ8;
		unsigned int shadeAndCount =
			((unsigned int)(uint8_t)((uint8_t)g_sw3dSpanLength - 1) << SW3D_SPAN_COUNT_SHIFT) |
			((unsigned int)g_sw3dSpanShadeQ8 << SW3D_SHADE_FRACTION_BITS) |
			(uint8_t)g_sw3dSpanShadeDitherAccum;
		unsigned int shadeAndCountStep =
			(((unsigned int)g_sw3dSpanShadeStepQ8 << SW3D_SHADE_FRACTION_BITS) ^ SW3D_SPAN_COUNT_MASK) |
			SW3D_SPAN_COUNT_STEP_MASK;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		uint8_t* shadeTable = g_sw3dSpanCachedShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET;
		unsigned int pixelIndex;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint32_t)) {
			unsigned int firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int secondOffset =
				((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint8_t firstTexel;
			uint8_t secondTexel;
			uint16_t ditherSum;
			unsigned int firstShadeIndex;
			unsigned int secondShadeIndex;
			uint16_t firstPixel;
			uint16_t secondPixel;
			uint32_t pixelPair;
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			firstTexel = texels[firstOffset];
			packedUV += packedStep;
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			firstShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
													   (ditherSum >> SW3D_SHADE_FRACTION_BITS))
							   << SW3D_SHADE_FRACTION_BITS) |
							  firstTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			secondTexel = texels[secondOffset];
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0) {
				memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(uint16_t)], sizeof(firstPixel));
				memcpy(&destination[pixelIndex], &firstPixel, sizeof(uint16_t));
				break;
			}
			memcpy(&g_sw3dSpanShadeCounterScratch, &shadeAndCount, sizeof(g_sw3dSpanShadeCounterScratch));
			memcpy(&firstPixel, &shadeTable[firstShadeIndex * sizeof(uint16_t)], sizeof(firstPixel));
			ditherSum = (uint8_t)shadeAndCount + (uint8_t)(shadeAndCount >> SW3D_SHADE_FRACTION_BITS);
			secondShadeIndex = ((unsigned int)(uint8_t)((uint8_t)(shadeAndCount >> SW3D_SHADE_LEVEL_SHIFT) +
														(ditherSum >> SW3D_SHADE_FRACTION_BITS))
								<< SW3D_SHADE_FRACTION_BITS) |
							   secondTexel;
			shadeAndCount = (shadeAndCount & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)ditherSum;
			packedUV &= SW3D_PACKED_UV_MASK_256;
			g_sw3dSpanPackedUVScratch = packedUV;
			memcpy(&secondPixel, &shadeTable[secondShadeIndex * sizeof(uint16_t)], sizeof(secondPixel));
			pixelPair = firstPixel | ((uint32_t)secondPixel << SW3D_PIXEL16_BITS);
			memcpy(&destination[pixelIndex], &pixelPair, sizeof(pixelPair));
			packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
			shadeAndCount += shadeAndCountStep;
			if ((int32_t)shadeAndCount < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)shadeAndCount;
	}
}

// FUNCTION: XW 0x49B570
void sw3d_DrawUnshadedSpan8_8x8(void) {
	int remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (int8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_8);
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		remaining = (int8_t)(remaining - 1);
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_8] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_8] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if ((remaining = (int8_t)(remaining - 1)) < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_8;
			g_sw3dSpanPackedUVScratch = packedUV;
			*(uint16_t*)&destination[pixelIndex] = texelPair;
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_8;
			if ((remaining = (int8_t)(remaining - 1)) < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49B650
void sw3d_DrawUnshadedSpan8_8x16(void) {
	int remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (int8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_8);
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		remaining = (int8_t)(remaining - 1);
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_8] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_8] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if ((remaining = (int8_t)(remaining - 1)) < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_16;
			g_sw3dSpanPackedUVScratch = packedUV;
			*(uint16_t*)&destination[pixelIndex] = texelPair;
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_16;
			if ((remaining = (int8_t)(remaining - 1)) < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49B730
void sw3d_DrawUnshadedSpan8_8x32(void) {
	int remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (int8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_8);
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		remaining = (int8_t)(remaining - 1);
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_8] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_8] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if ((remaining = (int8_t)(remaining - 1)) < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_32;
			g_sw3dSpanPackedUVScratch = packedUV;
			*(uint16_t*)&destination[pixelIndex] = texelPair;
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_32;
			if ((remaining = (int8_t)(remaining - 1)) < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49B810
void sw3d_DrawUnshadedSpan8_8x64(void) {
	int remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (int8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_8);
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		remaining = (int8_t)(remaining - 1);
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_8] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_8] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if ((remaining = (int8_t)(remaining - 1)) < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_64;
			g_sw3dSpanPackedUVScratch = packedUV;
			*(uint16_t*)&destination[pixelIndex] = texelPair;
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_64;
			if ((remaining = (int8_t)(remaining - 1)) < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49B8F0
void sw3d_DrawUnshadedSpan8_8x128(void) {
	int remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (int8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_8);
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		remaining = (int8_t)(remaining - 1);
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_8] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_8] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if ((remaining = (int8_t)(remaining - 1)) < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_128;
			g_sw3dSpanPackedUVScratch = packedUV;
			*(uint16_t*)&destination[pixelIndex] = texelPair;
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_128;
			if ((remaining = (int8_t)(remaining - 1)) < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49B9D0
void sw3d_DrawUnshadedSpan8_8x256(void) {
	int remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (int8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_8);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_8);
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		remaining = (int8_t)(remaining - 1);
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_8] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_8] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if ((remaining = (int8_t)(remaining - 1)) < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_256;
			g_sw3dSpanPackedUVScratch = packedUV;
			*(uint16_t*)&destination[pixelIndex] = texelPair;
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_256;
			if ((remaining = (int8_t)(remaining - 1)) < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49BAB0
void sw3d_DrawUnshadedSpan8_16x8(void) {
	int remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (int8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_16);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_16);
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		remaining = (int8_t)(remaining - 1);
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_16] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_16] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if ((remaining = (int8_t)(remaining - 1)) < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_8;
			g_sw3dSpanPackedUVScratch = packedUV;
			*(uint16_t*)&destination[pixelIndex] = texelPair;
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_8;
			if ((remaining = (int8_t)(remaining - 1)) < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49BB90
void sw3d_DrawUnshadedSpan8_16x16(void) {
	int remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (int8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_16);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_16);
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		remaining = (int8_t)(remaining - 1);
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_16] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_16] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if ((remaining = (int8_t)(remaining - 1)) < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_16;
			g_sw3dSpanPackedUVScratch = packedUV;
			*(uint16_t*)&destination[pixelIndex] = texelPair;
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_16;
			if ((remaining = (int8_t)(remaining - 1)) < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49BC70
void sw3d_DrawUnshadedSpan8_16x32(void) {
	int remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (int8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_16);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_16);
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		remaining = (int8_t)(remaining - 1);
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_16] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_16] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if ((remaining = (int8_t)(remaining - 1)) < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_32;
			g_sw3dSpanPackedUVScratch = packedUV;
			*(uint16_t*)&destination[pixelIndex] = texelPair;
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_32;
			if ((remaining = (int8_t)(remaining - 1)) < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49BD50
void sw3d_DrawUnshadedSpan8_16x64(void) {
	int remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (int8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_16);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_16);
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		remaining = (int8_t)(remaining - 1);
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_16] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_16] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if ((remaining = (int8_t)(remaining - 1)) < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_64;
			g_sw3dSpanPackedUVScratch = packedUV;
			*(uint16_t*)&destination[pixelIndex] = texelPair;
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_64;
			if ((remaining = (int8_t)(remaining - 1)) < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49BE30
void sw3d_DrawUnshadedSpan8_16x128(void) {
	int remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (int8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV = (((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) |
								 (uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_16)) &
								SW3D_PACKED_UV_MASK_128;
		unsigned int packedStep = (((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) |
								   (uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_16)) &
								  SW3D_PACKED_UV_MASK_128;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		remaining = (int8_t)(remaining - 1);
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_16] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_16] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if ((remaining = (int8_t)(remaining - 1)) < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_128;
			g_sw3dSpanPackedUVScratch = packedUV;
			*(uint16_t*)&destination[pixelIndex] = texelPair;
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_128;
			if ((remaining = (int8_t)(remaining - 1)) < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49BF10
void sw3d_DrawUnshadedSpan8_16x256(void) {
	int remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (int8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV = (((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) |
								 (uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_16)) &
								SW3D_PACKED_UV_MASK_256;
		unsigned int packedStep = (((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) |
								   (uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_16)) &
								  SW3D_PACKED_UV_MASK_256;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		remaining = (int8_t)(remaining - 1);
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_16] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_16] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if ((remaining = (int8_t)(remaining - 1)) < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_256;
			g_sw3dSpanPackedUVScratch = packedUV;
			*(uint16_t*)&destination[pixelIndex] = texelPair;
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_256;
			if ((remaining = (int8_t)(remaining - 1)) < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49BFF0
void sw3d_DrawUnshadedSpan8_32x8(void) {
	int8_t remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (int8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_32);
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		--remaining;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_32] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_32] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if (--remaining < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_8;
			g_sw3dSpanPackedUVScratch = packedUV;
			*(uint16_t*)&destination[pixelIndex] = texelPair;
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_8;
			if (--remaining < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49C0D0
void sw3d_DrawUnshadedSpan8_32x16(void) {
	int8_t remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (int8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_32);
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		--remaining;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_32] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_32] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if (--remaining < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_16;
			g_sw3dSpanPackedUVScratch = packedUV;
			*(uint16_t*)&destination[pixelIndex] = texelPair;
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_16;
			if (--remaining < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49C1B0
void sw3d_DrawUnshadedSpan8_32x32(void) {
	int8_t remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (int8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_32);
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		--remaining;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_32] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_32] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if (--remaining < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_32;
			g_sw3dSpanPackedUVScratch = packedUV;
			*(uint16_t*)&destination[pixelIndex] = texelPair;
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_32;
			if (--remaining < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49C290
void sw3d_DrawUnshadedSpan8_32x64(void) {
	int remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (int8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_32);
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		remaining = (int8_t)(remaining - 1);
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_32] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_32] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if ((remaining = (int8_t)(remaining - 1)) < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_64;
			g_sw3dSpanPackedUVScratch = packedUV;
			destination[pixelIndex] = (uint8_t)texelPair;
			destination[pixelIndex + 1] = (uint8_t)(texelPair >> SW3D_TEXTURE_FRACTION_BITS);
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_64;
			if ((remaining = (int8_t)(remaining - 1)) < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49C370
void sw3d_DrawUnshadedSpan8_32x128(void) {
	int remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (int8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_32);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_32);
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		remaining = (int8_t)(remaining - 1);
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_32] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_32] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if ((remaining = (int8_t)(remaining - 1)) < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_128;
			g_sw3dSpanPackedUVScratch = packedUV;
			destination[pixelIndex] = (uint8_t)texelPair;
			destination[pixelIndex + 1] = (uint8_t)(texelPair >> SW3D_TEXTURE_FRACTION_BITS);
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_128;
			if ((remaining = (int8_t)(remaining - 1)) < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49C450
void sw3d_DrawUnshadedSpan8_32x256(void) {
	int remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (int8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV = (((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) |
								 (uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_32)) &
								SW3D_PACKED_UV_MASK_256;
		unsigned int packedStep = (((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) |
								   (uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_32)) &
								  SW3D_PACKED_UV_MASK_256;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		remaining = (int8_t)(remaining - 1);
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_32] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_32] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if ((remaining = (int8_t)(remaining - 1)) < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_256;
			g_sw3dSpanPackedUVScratch = packedUV;
			destination[pixelIndex] = (uint8_t)texelPair;
			destination[pixelIndex + 1] = (uint8_t)(texelPair >> SW3D_TEXTURE_FRACTION_BITS);
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_256;
			if ((remaining = (int8_t)(remaining - 1)) < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49C530
void sw3d_DrawUnshadedSpan8_64x8(void) {
	int remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (int8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_64);
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		remaining = (int8_t)(remaining - 1);
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_64] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_64] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if ((remaining = (int8_t)(remaining - 1)) < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_8;
			g_sw3dSpanPackedUVScratch = packedUV;
			destination[pixelIndex] = (uint8_t)texelPair;
			destination[pixelIndex + 1] = (uint8_t)(texelPair >> SW3D_TEXTURE_FRACTION_BITS);
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_8;
			if ((remaining = (int8_t)(remaining - 1)) < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49C610
void sw3d_DrawUnshadedSpan8_64x16(void) {
	int remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (int8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_64);
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		remaining = (int8_t)(remaining - 1);
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_64] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_64] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if ((remaining = (int8_t)(remaining - 1)) < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_16;
			g_sw3dSpanPackedUVScratch = packedUV;
			destination[pixelIndex] = (uint8_t)texelPair;
			destination[pixelIndex + 1] = (uint8_t)(texelPair >> SW3D_TEXTURE_FRACTION_BITS);
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_16;
			if ((remaining = (int8_t)(remaining - 1)) < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49C6F0
void sw3d_DrawUnshadedSpan8_64x32(void) {
	int remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (int8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_64);
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		remaining = (int8_t)(remaining - 1);
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_64] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_64] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if ((remaining = (int8_t)(remaining - 1)) < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_32;
			g_sw3dSpanPackedUVScratch = packedUV;
			destination[pixelIndex] = (uint8_t)texelPair;
			destination[pixelIndex + 1] = (uint8_t)(texelPair >> SW3D_TEXTURE_FRACTION_BITS);
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_32;
			if ((remaining = (int8_t)(remaining - 1)) < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49C7D0
void sw3d_DrawUnshadedSpan8_64x64(void) {
	int remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (int8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_64);
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		remaining = (int8_t)(remaining - 1);
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_64] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_64] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if ((remaining = (int8_t)(remaining - 1)) < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_64;
			g_sw3dSpanPackedUVScratch = packedUV;
			destination[pixelIndex] = (uint8_t)texelPair;
			destination[pixelIndex + 1] = (uint8_t)(texelPair >> SW3D_TEXTURE_FRACTION_BITS);
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_64;
			if ((remaining = (int8_t)(remaining - 1)) < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49C8B0
void sw3d_DrawUnshadedSpan8_64x128(void) {
	int remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (int8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_64);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_64);
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		remaining = (int8_t)(remaining - 1);
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_64] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_64] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if ((remaining = (int8_t)(remaining - 1)) < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_128;
			g_sw3dSpanPackedUVScratch = packedUV;
			destination[pixelIndex] = (uint8_t)texelPair;
			destination[pixelIndex + 1] = (uint8_t)(texelPair >> SW3D_TEXTURE_FRACTION_BITS);
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_128;
			if ((remaining = (int8_t)(remaining - 1)) < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49C990
void sw3d_DrawUnshadedSpan8_64x256(void) {
	int remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (int8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV = (((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) |
								 (uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_64)) &
								SW3D_PACKED_UV_MASK_256;
		unsigned int packedStep = (((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) |
								   (uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_64)) &
								  SW3D_PACKED_UV_MASK_256;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		remaining = (int8_t)(remaining - 1);
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_64] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_64] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if ((remaining = (int8_t)(remaining - 1)) < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_256;
			g_sw3dSpanPackedUVScratch = packedUV;
			destination[pixelIndex] = (uint8_t)texelPair;
			destination[pixelIndex + 1] = (uint8_t)(texelPair >> SW3D_TEXTURE_FRACTION_BITS);
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_256;
			if ((remaining = (int8_t)(remaining - 1)) < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49CA70
void sw3d_DrawUnshadedSpan8_128x8(void) {
	int8_t remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (uint8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_128);
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		--remaining;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_128] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_128] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if (--remaining < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_8;
			g_sw3dSpanPackedUVScratch = packedUV;
			*(uint16_t*)&destination[pixelIndex] = texelPair;
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_8;
			if (--remaining < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49CB50
void sw3d_DrawUnshadedSpan8_128x16(void) {
	int8_t remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (uint8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_128);
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		--remaining;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_128] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_128] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if (--remaining < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_16;
			g_sw3dSpanPackedUVScratch = packedUV;
			*(uint16_t*)&destination[pixelIndex] = texelPair;
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_16;
			if (--remaining < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49CC30
void sw3d_DrawUnshadedSpan8_128x32(void) {
	int8_t remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (uint8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_128);
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		--remaining;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_128] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_128] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if (--remaining < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_32;
			g_sw3dSpanPackedUVScratch = packedUV;
			*(uint16_t*)&destination[pixelIndex] = texelPair;
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_32;
			if (--remaining < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49CD10
void sw3d_DrawUnshadedSpan8_128x64(void) {
	int8_t remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (uint8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_128);
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		--remaining;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_128] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_128] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if (--remaining < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_64;
			g_sw3dSpanPackedUVScratch = packedUV;
			*(uint16_t*)&destination[pixelIndex] = texelPair;
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_64;
			if (--remaining < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49CDF0
void sw3d_DrawUnshadedSpan8_128x128(void) {
	int8_t remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (uint8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_128);
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		--remaining;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_128] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_128] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if (--remaining < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_128;
			g_sw3dSpanPackedUVScratch = packedUV;
			*(uint16_t*)&destination[pixelIndex] = texelPair;
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_128;
			if (--remaining < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49CED0
void sw3d_DrawUnshadedSpan8_128x256(void) {
	int8_t remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (uint8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)((unsigned int)g_sw3dSpanVQ8 << SW3D_PACKED_V_SHIFT_128);
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)((unsigned int)g_sw3dSpanStepVQ8 << SW3D_PACKED_V_SHIFT_128);
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		--remaining;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((g_sw3dSpanPackedUVScratch >> SW3D_PACKED_U_SHIFT) &
				 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
				((g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			unsigned int secondOffset = ((packedUV >> SW3D_PACKED_U_SHIFT) &
										 (SW3D_TEXTURE_COLUMN_MASK << SW3D_TEXTURE_FRACTION_BITS)) |
										((packedUV >> SW3D_TEXTURE_FRACTION_BITS) & SW3D_TEXTURE_COLUMN_MASK);
			uint16_t texelPair =
				texels[firstOffset >> SW3D_PACKED_V_SHIFT_128] |
				(texels[secondOffset >> SW3D_PACKED_V_SHIFT_128] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if (--remaining < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_256;
			g_sw3dSpanPackedUVScratch = packedUV;
			*(uint16_t*)&destination[pixelIndex] = texelPair;
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_256;
			if (--remaining < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49CFB0
void sw3d_DrawUnshadedSpan8_256x8(void) {
	int8_t remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (uint8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)g_sw3dSpanVQ8;
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_8) |
			(uint16_t)g_sw3dSpanStepVQ8;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		--remaining;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_8;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int secondOffset =
				((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t texelPair = texels[firstOffset] | (texels[secondOffset] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if (--remaining < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_8;
			g_sw3dSpanPackedUVScratch = packedUV;
			*(uint16_t*)&destination[pixelIndex] = texelPair;
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_8;
			if (--remaining < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49D090
void sw3d_DrawUnshadedSpan8_256x16(void) {
	int8_t remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (uint8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)g_sw3dSpanVQ8;
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_16) |
			(uint16_t)g_sw3dSpanStepVQ8;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		--remaining;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_16;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int secondOffset =
				((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t texelPair = texels[firstOffset] | (texels[secondOffset] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if (--remaining < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_16;
			g_sw3dSpanPackedUVScratch = packedUV;
			*(uint16_t*)&destination[pixelIndex] = texelPair;
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_16;
			if (--remaining < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49D170
void sw3d_DrawUnshadedSpan8_256x32(void) {
	int8_t remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (uint8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)g_sw3dSpanVQ8;
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_32) |
			(uint16_t)g_sw3dSpanStepVQ8;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		--remaining;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_32;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int secondOffset =
				((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t texelPair = texels[firstOffset] | (texels[secondOffset] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if (--remaining < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_32;
			g_sw3dSpanPackedUVScratch = packedUV;
			*(uint16_t*)&destination[pixelIndex] = texelPair;
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_32;
			if (--remaining < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49D250
void sw3d_DrawUnshadedSpan8_256x64(void) {
	int8_t remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (uint8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)g_sw3dSpanVQ8;
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_64) |
			(uint16_t)g_sw3dSpanStepVQ8;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		--remaining;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_64;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int secondOffset =
				((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t texelPair = texels[firstOffset] | (texels[secondOffset] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if (--remaining < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_64;
			g_sw3dSpanPackedUVScratch = packedUV;
			*(uint16_t*)&destination[pixelIndex] = texelPair;
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_64;
			if (--remaining < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49D330
void sw3d_DrawUnshadedSpan8_256x128(void) {
	int8_t remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (uint8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)g_sw3dSpanVQ8;
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_128) |
			(uint16_t)g_sw3dSpanStepVQ8;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		--remaining;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_128;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int secondOffset =
				((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t texelPair = texels[firstOffset] | (texels[secondOffset] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if (--remaining < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_128;
			g_sw3dSpanPackedUVScratch = packedUV;
			*(uint16_t*)&destination[pixelIndex] = texelPair;
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_128;
			if (--remaining < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49D410
void sw3d_DrawUnshadedSpan8_256x256(void) {
	int8_t remaining;
	g_sw3dSpanCachedTexels = g_sw3dSpanTexels;
	g_sw3dSpanCachedShadeTable = g_sw3dSpanShadeTable;
	remaining = (uint8_t)g_sw3dSpanLength;
	if (remaining != 0) {
		uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + g_sw3dSpanStartX;
		unsigned int packedUV =
			(((unsigned int)g_sw3dSpanUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)g_sw3dSpanVQ8;
		unsigned int packedStep =
			(((unsigned int)g_sw3dSpanStepUQ8 << SW3D_PACKED_U_SHIFT) & SW3D_PACKED_UV_MASK_256) |
			(uint16_t)g_sw3dSpanStepVQ8;
		uint8_t* texels = g_sw3dSpanCachedTexels;
		unsigned int pixelIndex;
		--remaining;
		g_sw3dSpanPackedUVScratch = packedUV;
		packedUV = (packedUV + packedStep) & SW3D_PACKED_UV_MASK_256;
		for (pixelIndex = 0;; pixelIndex += sizeof(uint16_t)) {
			unsigned int firstOffset =
				((uint8_t)(g_sw3dSpanPackedUVScratch >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(g_sw3dSpanPackedUVScratch >> SW3D_TEXTURE_FRACTION_BITS);
			unsigned int secondOffset =
				((uint8_t)(packedUV >> (SW3D_PACKED_U_SHIFT + SW3D_TEXTURE_FRACTION_BITS))
				 << SW3D_TEXTURE_FRACTION_BITS) |
				(uint8_t)(packedUV >> SW3D_TEXTURE_FRACTION_BITS);
			uint16_t texelPair = texels[firstOffset] | (texels[secondOffset] << SW3D_TEXTURE_FRACTION_BITS);
			packedUV += packedStep;
			if (--remaining < 0) {
				destination[pixelIndex] = (uint8_t)texelPair;
				break;
			}
			packedUV &= SW3D_PACKED_UV_MASK_256;
			g_sw3dSpanPackedUVScratch = packedUV;
			*(uint16_t*)&destination[pixelIndex] = texelPair;
			packedUV = (packedStep + packedUV) & SW3D_PACKED_UV_MASK_256;
			if (--remaining < 0)
				break;
		}
		g_sw3dSpanShadeDitherAccum =
			(g_sw3dSpanShadeDitherAccum & ~SW3D_TEXTURE_COLUMN_MASK) | (uint8_t)remaining;
	}
}

// FUNCTION: XW 0x49D4F0
void sw3d_DrawTexturedShadeSpanGeneric8(void) {
	uint8_t* destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset;
	int endX = (int32_t)((unsigned int)g_sw3dSpanStartX + (unsigned int)g_sw3dSpanLength);
	int pixelIndex;
	for (pixelIndex = g_sw3dSpanStartX; pixelIndex < endX; ++pixelIndex) {
		int widthShift = g_sw3dSpanTextureWidthShift;
		unsigned int texelOffset;
		uint8_t texelIndex;
		unsigned int ditheredShade;
#ifdef XW_MODERN
		widthShift &= SW3D_SHIFT_COUNT_MASK;
#endif
		texelOffset = g_sw3dSpanTexelMask &
					  ((unsigned int)(g_sw3dSpanVQ8 >> SW3D_TEXTURE_FRACTION_BITS) +
					   ((unsigned int)(g_sw3dSpanUQ8 >> SW3D_TEXTURE_FRACTION_BITS) << widthShift));
		texelIndex = g_sw3dSpanTexels[texelOffset];
		ditheredShade = g_sw3dSpanShadeDitherAccum + (unsigned int)g_sw3dSpanShadeQ8;
		g_sw3dSpanShadeDitherAccum = (uint8_t)ditheredShade;
		destination[pixelIndex] =
			g_sw3dSpanShadeTable[SW3D_SHADE_TABLE_STRIDE *
									 ((ditheredShade >> SW3D_SHADE_FRACTION_BITS) & SW3D_SHADE_LEVEL_MASK) +
								 texelIndex];
		g_sw3dSpanShadeQ8 = (int32_t)((unsigned int)g_sw3dSpanShadeQ8 + (unsigned int)g_sw3dSpanShadeStepQ8);
		g_sw3dSpanVQ8 = (int32_t)((unsigned int)g_sw3dSpanVQ8 + (unsigned int)g_sw3dSpanStepVQ8);
		g_sw3dSpanUQ8 = (int32_t)((unsigned int)g_sw3dSpanUQ8 + (unsigned int)g_sw3dSpanStepUQ8);
	}
}

// FUNCTION: XW 0x49D5E0
void sw3d_DrawTexturedShadeSpanGeneric(void) {
	const uint16_t* packedShadeTable =
		(const uint16_t*)(g_sw3dSpanShadeTable + SW3D_PACKED_SHADE_TABLE_OFFSET);
	uint16_t* destination = (uint16_t*)(g_surfacePixels + g_sw3dSpanFramebufferRowOffset) + g_sw3dSpanStartX;
	int spanLength = g_sw3dSpanLength;
	int pixelIndex;
	for (pixelIndex = 0; pixelIndex < spanLength; ++pixelIndex) {
		int widthShift = g_sw3dSpanTextureWidthShift;
		unsigned int texelOffset;
		uint16_t texelIndex;
		unsigned int ditheredShade;
#ifdef XW_MODERN
		widthShift &= SW3D_SHIFT_COUNT_MASK;
#endif
		texelOffset = g_sw3dSpanTexelMask &
					  ((unsigned int)(g_sw3dSpanVQ8 >> SW3D_TEXTURE_FRACTION_BITS) +
					   ((unsigned int)(g_sw3dSpanUQ8 >> SW3D_TEXTURE_FRACTION_BITS) << widthShift));
		texelIndex = g_sw3dSpanTexels[texelOffset];
		ditheredShade = g_sw3dSpanShadeDitherAccum + (unsigned int)g_sw3dSpanShadeQ8;
		g_sw3dSpanShadeDitherAccum = (uint8_t)ditheredShade;
		destination[pixelIndex] =
			packedShadeTable[SW3D_SHADE_TABLE_STRIDE *
								 ((ditheredShade >> SW3D_SHADE_FRACTION_BITS) & SW3D_SHADE_LEVEL_MASK) +
							 texelIndex];
		g_sw3dSpanShadeQ8 = (int32_t)((unsigned int)g_sw3dSpanShadeQ8 + (unsigned int)g_sw3dSpanShadeStepQ8);
		g_sw3dSpanVQ8 = (int32_t)((unsigned int)g_sw3dSpanVQ8 + (unsigned int)g_sw3dSpanStepVQ8);
		g_sw3dSpanUQ8 = (int32_t)((unsigned int)g_sw3dSpanUQ8 + (unsigned int)g_sw3dSpanStepUQ8);
	}
}

// FUNCTION: XW 0x49D6D0
void sw3d_BlitOccludedSpan(const uint8_t* sourcePixels, int startX, int endX, int scanY, float inverseDepth) {
	int visibleStartX = startX;
	SceneSpan* occluder;
	const uint8_t* sourceRasterBase;
	if (g_flightBytesPerPixel == sizeof(uint16_t))
		sourceRasterBase = sourcePixels + (-startX) * (int)sizeof(uint16_t);
	else
		sourceRasterBase = sourcePixels - startX;
	g_sw3dSpanFramebufferRowOffset =
		g_flightBytesPerPixel * g_flightVpX + g_surfacePitch * (scanY + g_flightVpY);
	for (occluder = g_scanlineSpanHeads[scanY]; occluder != NULL; occluder = occluder->next) {
		int occluderEndX = occluder->xEnd;
		if (occluderEndX > visibleStartX) {
			SceneFace* face;
			if (occluder->xStart > visibleStartX)
				break;
			face = occluder->face;
			if (inverseDepth <= face->minVertW) {
				visibleStartX = occluder->xEnd;
				if (visibleStartX >= endX)
					return;
			} else if (inverseDepth < face->maxVertW) {
				float faceDepthAtStart = (float)visibleStartX * face->depthPlane[0] +
										 ((float)scanY * face->depthPlane[1] + face->depthPlane[2]);
				if (inverseDepth <= faceDepthAtStart) {
					if (0.0f <= face->depthPlane[0]) {
						visibleStartX = occluder->xEnd;
						if (visibleStartX >= endX)
							return;
					} else if (occluderEndX < endX) {
						float length = occluderEndX - visibleStartX;
						float endDepth = length * face->depthPlane[0] + faceDepthAtStart;
						if (inverseDepth > endDepth) {
							visibleStartX += (int)(length - (inverseDepth - endDepth) / -face->depthPlane[0]);
							if (visibleStartX >= endX)
								return;
						} else {
							visibleStartX = occluder->xEnd;
						}
					} else {
						float length = endX - visibleStartX;
						float endDepth = length * face->depthPlane[0] + faceDepthAtStart;
						if (inverseDepth <= endDepth)
							return;
						visibleStartX += (int)(length - (inverseDepth - endDepth) / -face->depthPlane[0]);
						if (visibleStartX >= endX)
							return;
					}
				} else if (0.0f < face->depthPlane[0]) {
					float length;
					float endDepth;
					if (occluderEndX <= endX)
						length = occluderEndX - visibleStartX;
					else
						length = endX - visibleStartX;
					endDepth = length * face->depthPlane[0] + faceDepthAtStart;
					if (inverseDepth < endDepth) {
						endX =
							visibleStartX + (int)(length - (inverseDepth - endDepth) / -face->depthPlane[0]);
						if (visibleStartX >= endX)
							return;
					}
				}
			}
		}
	}
	for (; occluder != NULL; occluder = occluder->next) {
		int occluderStartX = occluder->xStart;
		SceneFace* face;
		if (occluderStartX >= endX)
			break;
		face = occluder->face;
		if (inverseDepth <= face->minVertW) {
			sw3d_CopySpanToFramebuffer(sourceRasterBase, visibleStartX, occluderStartX - visibleStartX);
			visibleStartX = occluder->xEnd;
			if (visibleStartX >= endX)
				return;
		} else if (inverseDepth < face->maxVertW) {
			float faceDepthAtStart = (float)occluderStartX * face->depthPlane[0] +
									 ((float)scanY * face->depthPlane[1] + face->depthPlane[2]);
			if (inverseDepth <= faceDepthAtStart) {
				sw3d_CopySpanToFramebuffer(sourceRasterBase, visibleStartX, occluderStartX - visibleStartX);
				face = occluder->face;
				visibleStartX = occluder->xEnd;
				if (0.0f <= face->depthPlane[0]) {
					if (visibleStartX >= endX)
						return;
				} else if (visibleStartX < endX) {
					float endDepth =
						(float)(visibleStartX - occluder->xStart) * face->depthPlane[0] + faceDepthAtStart;
					if (inverseDepth > endDepth)
						visibleStartX -= (int)((inverseDepth - endDepth) / -face->depthPlane[0]);
				} else {
					float endDepth =
						(float)(endX - occluder->xStart) * face->depthPlane[0] + faceDepthAtStart;
					if (inverseDepth <= endDepth)
						return;
					visibleStartX = endX - (int)((inverseDepth - endDepth) / -face->depthPlane[0]);
					if (visibleStartX >= endX)
						return;
				}
			} else if (0.0f < face->depthPlane[0]) {
				int occluderEndX = occluder->xEnd;
				if (occluderEndX < endX) {
					float endDepth =
						(float)(occluderEndX - occluderStartX) * face->depthPlane[0] + faceDepthAtStart;
					if (inverseDepth < endDepth) {
						sw3d_CopySpanToFramebuffer(
							sourceRasterBase, visibleStartX,
							occluderEndX - (int)((inverseDepth - endDepth) / -face->depthPlane[0]) -
								visibleStartX);
						visibleStartX = occluder->xEnd;
					}
				} else {
					float length = endX - occluderStartX;
					float endDepth = length * face->depthPlane[0] + faceDepthAtStart;
					if (inverseDepth < endDepth) {
						sw3d_CopySpanToFramebuffer(
							sourceRasterBase, visibleStartX,
							occluder->xStart +
								(int)(length - (inverseDepth - endDepth) / -occluder->face->depthPlane[0]) -
								visibleStartX);
						return;
					}
				}
			}
		}
	}
	sw3d_CopySpanToFramebuffer(sourceRasterBase, visibleStartX, endX - visibleStartX);
}

// FUNCTION: XW 0x49DB10
void sw3d_CopySpanToFramebuffer(const uint8_t* sourceRasterBase, int startX, int pixelCount) {
	if (pixelCount > 0) {
		unsigned int byteIndex;
		if (g_flightBytesPerPixel == sizeof(uint16_t)) {
			uint8_t* destination;
			sourceRasterBase += startX * (int)sizeof(uint16_t);
			destination = g_surfacePixels + g_sw3dSpanFramebufferRowOffset + startX + startX;
			for (byteIndex = 0; pixelCount != 0; byteIndex += sizeof(uint16_t), --pixelCount) {
				uint8_t lowByte = sourceRasterBase[byteIndex];
				uint8_t highByte = sourceRasterBase[byteIndex + 1];
				destination[byteIndex] = lowByte;
				destination[byteIndex + 1] = highByte;
			}
		} else {
			const uint8_t* source = sourceRasterBase + startX;
			uint8_t* destination = g_surfacePixels + startX + g_sw3dSpanFramebufferRowOffset;
			for (byteIndex = 0; pixelCount != 0; ++byteIndex, --pixelCount) {
				destination[byteIndex] = source[byteIndex];
			}
		}
	}
}
