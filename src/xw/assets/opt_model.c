#include "xw/assets/opt_model.h"

#ifdef XW_MODERN
#include "xw_runtime/snapshot/render_assets.h"
#endif

#include "xw/assets/model_mesh.h"
#include "xw/assets/model_texture.h"
#include "xw/flight/fediskio.h"
#include "xw/flight/flight_display.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/render/image_quantizer.h"
#include "xw/render/render_scene.h"
#include "xw/render/renderer.h"
#include "xw/render/rtsvga2.h"
#include "xw/render/sw3d.h"
#include "xw/util/memory.h"
#include "xw/util/shared.h"
#ifdef XW_MODERN
#include "xw_runtime/storage/opt_native.h"
#endif
#include "xw_runtime/compat/middleware_crt.h"

#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4DA4F8
int g_cacheResolvedOptNodeRefs = 1;

// GLOBAL: XW 0x4DA618
uint8_t g_defaultWhiteTextureRgb24[OPT_TEXTURE_DEFAULT_WIDTH * OPT_TEXTURE_DEFAULT_HEIGHT *
								   OPT_RGB_CHANNEL_COUNT] = {
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
};

// GLOBAL: XW 0x4DA6E4
char g_extRgb[OPT_TEXTURE_EXTENSION_LENGTH + 1] = "rgb";

// GLOBAL: XW 0x55CAF8
OptVector* g_sourceVectors = NULL;

// GLOBAL: XW 0x55CB00
OptNode* g_optConvertCurrentMesh = NULL;

// GLOBAL: XW 0x55CB04
OptNode* g_optConvertVertexNormalNode = NULL;

// GLOBAL: XW 0x55CB0C
int g_generatedVertexNormalCount = 0;

// GLOBAL: XW 0x55CB10
void* g_curMeshDescriptorData = NULL;

// GLOBAL: XW 0x55CB14
OptNode* g_optConvertSourceTextureNode = NULL;

// GLOBAL: XW 0x55CB1C
int g_optConvertVectorSearchCursor = 0;

// GLOBAL: XW 0x55CB20
OptTexCoord* g_sourceTexCoords = NULL;

// GLOBAL: XW 0x55CB28
void* g_optNodeWalkScratch2 = NULL;

// GLOBAL: XW 0x55CB2C
OptNode* g_optConvertTexCoordNode = NULL;

// GLOBAL: XW 0x55CB30
OptNode* g_optConvertFaceTextureNode = NULL;

// GLOBAL: XW 0x55CB34
int g_optConvertTexCoordSearchCursor = 0;

// GLOBAL: XW 0x55CB38
OptNode* g_optConvertVertexNode = NULL;

// GLOBAL: XW 0x55CB3C
int g_curVertexCount = 0;

// GLOBAL: XW 0x55CB40
OptVector* g_curVertNormals = NULL;

// GLOBAL: XW 0x55FBBC
int g_optModelInvertFaceNormals = 0;

// GLOBAL: XW 0x55FBCC
uint16_t g_loadOptBufHandle = 0;

// GLOBAL: XW 0x55FBD0
unsigned int g_loadOptBufferCapacityBytes = 0;

// GLOBAL: XW 0x55FBD4
int g_optSourceIsVersion0 = 0;

// GLOBAL: XW 0x55FBD8
uint16_t g_optConversionSourceHandle = 0;

// GLOBAL: XW 0x55FBDC
unsigned int g_optConversionSourceCapacityBytes = 0;

// GLOBAL: XW 0x55FBE0
int g_optConvertTargetFaceFound = 0;

// FUNCTION: XW 0x488360
void OptModel_AdjustOptimizedPolyObjectPointers(struct OptimizedPolyObject* model) {
#ifdef XW_MODERN
	XwOpt_Relocate(model);
#else
	ptrdiff_t baseDelta = (uint8_t*)model - (uint8_t*)model->selfMarker;
	model->selfMarker = (uint8_t*)model->selfMarker + baseDelta;
	if (model->rootNodes != NULL) {
		int rootIndex;
		model->rootNodes = (OptNode**)((uint8_t*)model->rootNodes + baseDelta);
		for (rootIndex = 0; rootIndex < model->rootNodeCount; ++rootIndex) {
			if (model->rootNodes[rootIndex] != NULL) {
				model->rootNodes[rootIndex] = (OptNode*)((uint8_t*)model->rootNodes[rootIndex] + baseDelta);
				OptModel_AdjustOptimizedNodePointers(model->rootNodes[rootIndex], baseDelta);
			}
		}
	}
#endif
}

// FUNCTION: XW 0x4883C0
void OptModel_AdjustOptimizedNodePointers(struct OptNode* node, ptrdiff_t baseDelta) {
#ifdef XW_MODERN
	XwOpt_RelocateNode(node, baseDelta);
#else
	if (node->pName != NULL) {
		node->pName += baseDelta;
	}
	if (node->param2 != NULL) {
		node->param2 = (uint8_t*)node->param2 + baseDelta;
	}
	if (node->nodeType == OPT_TEXTURE) {
		OptTextureData* texture = node->param2;
		if (texture->paletteType == 0) {
			texture->palette = (uint16_t*)((uint8_t*)texture->palette + baseDelta);
		}
	}
	if (node->pChildren != NULL) {
		int childIndex;
		node->pChildren = (OptNode**)((uint8_t*)node->pChildren + baseDelta);
		for (childIndex = 0; childIndex < node->childCount; ++childIndex) {
			if (node->pChildren[childIndex] != NULL) {
				node->pChildren[childIndex] = (OptNode*)((uint8_t*)node->pChildren[childIndex] + baseDelta);
				OptModel_AdjustOptimizedNodePointers(node->pChildren[childIndex], baseDelta);
			}
		}
	}
#endif
}

// FUNCTION: XW 0x4886E0
void OptModel_SetSourceVertices(struct OptVector* vertices) { g_sourceVectors = vertices; }

// FUNCTION: XW 0x4886F0
void OptModel_SetSourceTexCoords(struct OptTexCoord* texCoords) { g_sourceTexCoords = texCoords; }

// FUNCTION: XW 0x48AC40
int16_t OptModel_LoadFileToHandle(char* fileName) {
#ifdef XW_MODERN
	unsigned int nativeSize = 0;
	int version = 0;
	SceneMesh parentState;
	OptimizedPolyObject* model;
	int rootIndex;
	XwRenderAssetId sourceId;
	uint16_t handle = XwOpt_Load(fileName, &version, &nativeSize);
	if (!handle)
		XwStorage_Fatal("Cannot load required OPT model", 1);
	if (g_loadOptBufHandle)
		Memory_FreeHandle(g_loadOptBufHandle);
	g_loadOptBufHandle = handle;
	g_loadOptBufferCapacityBytes = nativeSize;
	sourceId = XwRenderAssets_HandleId(handle);
	XwRenderAssets_Retain(sourceId);
	g_optSourceIsVersion0 = version == 0;
	if (version < 2) {
		OptModel_ConvertLegacyModelToOptimized(nativeSize);
		handle = g_loadOptBufHandle;
	}
	XwRenderAssets_AttachHandle(handle, sourceId);
	XwRenderAssets_Release(sourceId);
	model = Memory_LockHandle(handle);
	memset(&parentState, 0, sizeof parentState);
	g_sourceVectors = NULL;
	g_sourceTexCoords = NULL;
	g_curVertNormals = NULL;
	g_optNodeWalkScratch2 = NULL;
	g_curMeshDescriptorData = NULL;
	g_curVertexCount = 0;
	for (rootIndex = 0; rootIndex < model->rootNodeCount; ++rootIndex)
		OptModel_GetSerializedNodeSize(model->rootNodes[rootIndex], &parentState);
	return handle;
#else
	XwFile* stream;
	OptimizedPolyObject* model;
	uint32_t byteCount;
	int32_t version;
	uint16_t resultHandle;
	int rootIndex;
	SceneMesh parentState;
	fediskio_tryopenfile(fileName, "rb", 1);
	stream = g_stream;
	if (stream == NULL)
		return 0;
	g_optSourceIsVersion0 = 0;
	File_RawRead(&version, 1, sizeof(version), stream);
	if (version > 0) {
		byteCount = version;
		version = 0;
		g_optSourceIsVersion0 = 1;
	} else {
		version = -version;
		if (version == OPT_FILE_VERSION_0)
			version = 0;
		g_optSourceIsVersion0 = 0;
		File_RawRead(&byteCount, 1, sizeof(byteCount), stream);
	}
	if ((int)byteCount > (int)g_loadOptBufferCapacityBytes && g_loadOptBufHandle != 0) {
		Memory_FreeHandle(g_loadOptBufHandle);
		g_loadOptBufHandle = 0;
		g_loadOptBufferCapacityBytes = 0;
	}
	if (g_loadOptBufHandle == 0) {
		g_loadOptBufHandle = Memory_AllocHandle(byteCount, 0);
		if (g_loadOptBufHandle == 0)
			fediskio_fatalerror(0);
		g_loadOptBufferCapacityBytes = byteCount;
	}
	resultHandle = g_loadOptBufHandle;
	model = Memory_LockHandle(g_loadOptBufHandle);
	File_RawRead(model, 1, byteCount, stream);
	File_RawClose(stream);

	if (model->selfMarker != model)
		OptModel_AdjustOptimizedPolyObjectPointers(model);
	if (version == 0) {
		nullsub_SharedNoOp();
		OptModel_ConvertLegacyModelToOptimized(byteCount);
		resultHandle = g_loadOptBufHandle;
		model = Memory_LockHandle(g_loadOptBufHandle);
		if (model->selfMarker != model)
			OptModel_AdjustOptimizedPolyObjectPointers(model);
	}
	g_sourceVectors = NULL;
	memset(&parentState, 0, sizeof(parentState));
	g_sourceTexCoords = NULL;
	g_curVertNormals = NULL;
	g_optNodeWalkScratch2 = NULL;
	g_curMeshDescriptorData = NULL;
	g_curVertexCount = 0;
	byteCount = sizeof(OptModelFileHeader) + sizeof(uint32_t) * model->rootNodeCount;
	for (rootIndex = 0; rootIndex < model->rootNodeCount; ++rootIndex)
		byteCount += OptModel_GetSerializedNodeSize(model->rootNodes[rootIndex], &parentState);
	nullsub_SharedNoOp();
	return resultHandle;
#endif
}

// FUNCTION: XW 0x48AED0
int OptModel_ConvertLegacyModelToOptimized(int sourceBytes) {
	OptimizedPolyObject* sourceModel;
	OptimizedPolyObject* destinationModel;
	uint8_t* destinationCursor;
	int outputCapacity;
	int outputBytes;
	int rootIndex;
	SceneMesh conversionState;
	if (sourceBytes > (int)g_optConversionSourceCapacityBytes && g_optConversionSourceHandle != 0) {
		Memory_FreeHandle(g_optConversionSourceHandle);
		g_optConversionSourceHandle = 0;
		g_optConversionSourceCapacityBytes = 0;
	}
	if (g_optConversionSourceHandle == 0) {
		g_optConversionSourceHandle = Memory_AllocHandle(sourceBytes, 0);
		if (g_optConversionSourceHandle == 0)
			fediskio_fatalerror(0);
		g_optConversionSourceCapacityBytes = sourceBytes;
	}
	sourceModel = Memory_LockHandle(g_optConversionSourceHandle);
	memcpy(sourceModel, Memory_LockHandle(g_loadOptBufHandle), sourceBytes);
	if (sourceModel->selfMarker != sourceModel)
		OptModel_AdjustOptimizedPolyObjectPointers(sourceModel);
	outputCapacity = (int32_t)((uint32_t)sourceBytes * OPT_CONVERSION_CAPACITY_MULTIPLIER);
	nullsub_SharedNoOp();
	if (outputCapacity > (int)g_loadOptBufferCapacityBytes && g_loadOptBufHandle != 0) {
		Memory_FreeHandle(g_loadOptBufHandle);
		g_loadOptBufHandle = 0;
		g_loadOptBufferCapacityBytes = 0;
	}
	if (g_loadOptBufHandle == 0) {
		g_loadOptBufHandle = Memory_AllocHandle(outputCapacity, 0);
		if (g_loadOptBufHandle == 0)
			fediskio_fatalerror(0);
		g_loadOptBufferCapacityBytes = outputCapacity;
	}
	destinationModel = Memory_LockHandle(g_loadOptBufHandle);
	*destinationModel = *sourceModel;
	destinationModel->selfMarker = destinationModel;
	destinationModel->rootNodes = (OptNode**)(destinationModel + 1);
	destinationCursor = (uint8_t*)(destinationModel + 1);
	destinationCursor += sizeof(OptNode*) * sourceModel->rootNodeCount;
	outputBytes = sizeof(*destinationModel) + sizeof(OptNode*) * sourceModel->rootNodeCount;
	memset(&conversionState, 0, sizeof(conversionState));
	g_sourceVectors = NULL;
	g_sourceTexCoords = NULL;
	g_curVertNormals = NULL;
	g_optNodeWalkScratch2 = NULL;
	g_curMeshDescriptorData = NULL;
	g_curVertexCount = 0;
	destinationModel->rootNodeCount = 0;
	for (rootIndex = 0; rootIndex < sourceModel->rootNodeCount; ++rootIndex) {
		size_t rootBytes;
#ifdef XW_MODERN
		size_t padding = (sizeof(void*) - (size_t)outputBytes % sizeof(void*)) % sizeof(void*);
		destinationCursor += padding;
		outputBytes += (int)padding;
#endif
		++destinationModel->rootNodeCount;
		destinationModel->rootNodes[rootIndex] = (OptNode*)destinationCursor;
		g_optConvertVertexNode = NULL;
		g_optConvertTexCoordNode = NULL;
		g_optConvertVertexNormalNode = NULL;
		rootBytes =
			OptModel_ConvertLegacyNodeToOptimized(destinationCursor, sourceModel->rootNodes[rootIndex],
												  sourceModel, destinationModel, &conversionState);
		outputBytes += (int)rootBytes;
		destinationCursor += rootBytes;
	}
	return outputBytes;
}

// FUNCTION: XW 0x48B0C0
struct OptShadeTable* OptModel_FindSharedTextureDataInNodeBeforeTarget(
	const struct OptShadeTable* textureData, struct OptNode* node, const struct OptNode* stopNode) {
	const OptShadeTable* searchData;
	int childIndex;
	if (node == NULL) {
		return NULL;
	}
	if (node->nodeType == OPT_TEXTURE) {
		OptTextureData* texture = node->param2;
		int texelBytes = texture->width * texture->height;
		uint8_t* texels = (uint8_t*)texture + sizeof(*texture);
		OptShadeTable* inlineShades;
		if (texelBytes == texture->textureSize) {
			texelBytes = texture->dataSize;
		}
		inlineShades = (OptShadeTable*)(texels + texelBytes);
		if (texture->paletteType == 0) {
			if ((void*)inlineShades == texture->palette) {
				searchData = (const OptShadeTable*)textureData->packedShades;
				if (memcmp(searchData, inlineShades->packedShades, sizeof(inlineShades->packedShades)) == 0) {
					return inlineShades;
				}
			} else {
				searchData = textureData;
			}
		} else {
			searchData = (const OptShadeTable*)textureData->packedShades;
			if (memcmp(searchData, inlineShades->packedShades, sizeof(inlineShades->packedShades)) == 0) {
				return inlineShades;
			}
		}
	} else {
		searchData = textureData;
	}
	/* Preserve the original comparison-pointer advance for recursive searches. */
	for (childIndex = 0; childIndex < node->childCount; ++childIndex) {
		OptNode* child = node->pChildren[childIndex];
		OptShadeTable* result;
		if (child == stopNode) {
			break;
		}
		result = OptModel_FindSharedTextureDataInNodeBeforeTarget(searchData, child, stopNode);
		if (result != NULL) {
			return result;
		}
	}
	return NULL;
}

// FUNCTION: XW 0x48B190
struct OptShadeTable* OptModel_FindEarlierSharedTextureData(const struct OptShadeTable* textureData,
															struct OptimizedPolyObject* model,
															const struct OptNode* stopNode) {
	int rootIndex;
	for (rootIndex = 0; rootIndex < model->rootNodeCount; ++rootIndex) {
		OptNode* rootNode = model->rootNodes[rootIndex];
		OptShadeTable* result;
		if (rootNode == stopNode) {
			break;
		}
		result = OptModel_FindSharedTextureDataInNodeBeforeTarget(textureData, rootNode, stopNode);
		if (result != NULL) {
			return result;
		}
	}
	return NULL;
}

// FUNCTION: XW 0x48B1E0
size_t OptModel_ConvertLegacyNodeToOptimized(uint8_t* destination, struct OptNode* sourceNode,
											 struct OptimizedPolyObject* sourceModel,
											 struct OptimizedPolyObject* destinationModel,
											 struct SceneMesh* conversionState) {
	uint8_t* cursor;
	size_t payloadBytes;
	OptNode* outputNode;
	int copyOriginalNode;
	SceneMesh childState;
	if (sourceNode == NULL)
		return 0;
	cursor = destination;
	payloadBytes = 0;
	outputNode = NULL;
	copyOriginalNode = 1;
	switch (sourceNode->nodeType) {
		case OPT_NODE_TYPE_21:
			g_optConvertCurrentMesh = sourceNode;
			if (g_optConvertVertexNode == NULL) {
				OptNode* wrapper = (OptNode*)cursor;
				OptNode* vertexNode;
				OptNode* texCoordNode;
				OptNode* normalNode;
				OptVector* vertices;
				OptVector* savedNormals;
				int savedVertexCount;
				int vertexIndex;
				float minX, minY, minZ, maxX, maxY;
				float maxZ;
				cursor += sizeof(OptNode);
				wrapper->nodeType = OPT_NODE_GROUP;
				if (sourceNode->pName != NULL) {
					strcpy((char*)cursor, sourceNode->pName);
					wrapper->pName = (char*)cursor;
					cursor += strlen(sourceNode->pName) + 1;
				} else {
					wrapper->pName = NULL;
				}
#ifdef XW_MODERN
				cursor += (sizeof(void*) - (size_t)(cursor - destination) % sizeof(void*)) % sizeof(void*);
#endif
				wrapper->pChildren = (OptNode**)cursor;
				wrapper->param2 = NULL;
				wrapper->param1 = 0;
				wrapper->childCount = OPT_SHARED_GEOMETRY_CHILD_COUNT + 1;
				cursor += sizeof(OptNode*) * wrapper->childCount;
#ifdef XW_MODERN
				cursor += (sizeof(void*) - (size_t)(cursor - destination) % sizeof(void*)) % sizeof(void*);
#endif
				vertexNode = (OptNode*)cursor;
				wrapper->pChildren[OPT_SHARED_VERTEX_CHILD] = vertexNode;
				vertexNode->pName = NULL;
				vertexNode->nodeType = OPT_MESHVERTS;
				vertexNode->param1 = 0;
				vertexNode->childCount = 0;
				vertexNode->pChildren = NULL;
				cursor += sizeof(OptNode);
				vertexNode->param2 = cursor;
				OptModel_CollectUniqueVertices(vertexNode, sourceNode, sourceModel, conversionState);
				g_optConvertVertexNode = vertexNode;
				vertices = vertexNode->param2;
#ifdef XW_MODERN
				if (vertexNode->param1 == 0) {
					minX = maxX = 0.0f;
					minY = maxY = 0.0f;
					minZ = maxZ = 0.0f;
				} else
#endif
				{
					maxX = vertices[0].x;
					minX = maxX;
					maxY = vertices[0].y;
					minY = maxY;
					maxZ = vertices[0].z;
					minZ = vertices[0].z;
				}
				for (vertexIndex = 0; vertexIndex < vertexNode->param1; ++vertexIndex) {
					if (
#ifdef XW_MODERN
						!(vertices[vertexIndex].x >= minX)
#else
						vertices[vertexIndex].x < minX
#endif
					)
						minX = vertices[vertexIndex].x;
					if (
#ifdef XW_MODERN
						!(vertices[vertexIndex].y >= minY)
#else
						vertices[vertexIndex].y < minY
#endif
					)
						minY = vertices[vertexIndex].y;
					if (
#ifdef XW_MODERN
						!(vertices[vertexIndex].z >= minZ)
#else
						vertices[vertexIndex].z < minZ
#endif
					)
						minZ = vertices[vertexIndex].z;
					if (vertices[vertexIndex].x > maxX)
						maxX = vertices[vertexIndex].x;
					if (vertices[vertexIndex].y > maxY)
						maxY = vertices[vertexIndex].y;
					if (
#ifdef XW_MODERN
						!(maxZ >= vertices[vertexIndex].z)
#else
						maxZ < vertices[vertexIndex].z
#endif
					)
						maxZ = vertices[vertexIndex].z;
				}
				if (
#ifdef XW_MODERN
					vertexIndex < OPT_BOUNDS_VERTEX_COUNT ||
					(vertices[vertexIndex - OPT_BOUNDS_VERTEX_COUNT].x < minX ||
					 vertices[vertexIndex - OPT_BOUNDS_VERTEX_COUNT].x > minX) ||
					(vertices[vertexIndex - OPT_BOUNDS_VERTEX_COUNT].y < minY ||
					 vertices[vertexIndex - OPT_BOUNDS_VERTEX_COUNT].y > minY) ||
					(vertices[vertexIndex - OPT_BOUNDS_VERTEX_COUNT].z < minZ ||
					 vertices[vertexIndex - OPT_BOUNDS_VERTEX_COUNT].z > minZ) ||
					(vertices[vertexIndex - 1].x < maxX || vertices[vertexIndex - 1].x > maxX) ||
					(vertices[vertexIndex - 1].y < maxY || vertices[vertexIndex - 1].y > maxY) ||
					(vertices[vertexIndex - 1].z < maxZ || vertices[vertexIndex - 1].z > maxZ)
#else
					vertices[vertexIndex - OPT_BOUNDS_VERTEX_COUNT].x != minX ||
					vertices[vertexIndex - OPT_BOUNDS_VERTEX_COUNT].y != minY ||
					vertices[vertexIndex - OPT_BOUNDS_VERTEX_COUNT].z != minZ ||
					vertices[vertexIndex - 1].x != maxX || vertices[vertexIndex - 1].y != maxY ||
					maxZ != vertices[vertexIndex - 1].z
#endif
				) {
					vertices[vertexIndex].x = minX;
					vertices[vertexIndex].y = minY;
					vertices[vertexIndex].z = minZ;
					vertices[vertexIndex + 1].x = maxX;
					vertices[vertexIndex + 1].y = maxY;
					vertices[vertexIndex + 1].z = (float)maxZ;
					vertexNode->param1 += OPT_BOUNDS_VERTEX_COUNT;
				}
				cursor += sizeof(OptVector) * vertexNode->param1;
#ifdef XW_MODERN
				cursor += (sizeof(void*) - (size_t)(cursor - destination) % sizeof(void*)) % sizeof(void*);
#endif
				texCoordNode = (OptNode*)cursor;
				wrapper->pChildren[OPT_SHARED_TEXCOORD_CHILD] = texCoordNode;
				texCoordNode->nodeType = OPT_TEXCOORDS;
				texCoordNode->pName = NULL;
				texCoordNode->param1 = 0;
				texCoordNode->childCount = 0;
				texCoordNode->pChildren = NULL;
				cursor += sizeof(OptNode);
				texCoordNode->param2 = cursor;
				OptModel_CollectUniqueTexCoords(texCoordNode, sourceNode, sourceModel, conversionState);
				g_optConvertTexCoordNode = texCoordNode;
				cursor += sizeof(OptTexCoord) * texCoordNode->param1;
#ifdef XW_MODERN
				cursor += (sizeof(void*) - (size_t)(cursor - destination) % sizeof(void*)) % sizeof(void*);
#endif
				normalNode = (OptNode*)cursor;
				wrapper->pChildren[OPT_SHARED_NORMAL_CHILD] = normalNode;
				normalNode->nodeType = OPT_VERTNORMALS;
				normalNode->pName = NULL;
				normalNode->param1 = 0;
				normalNode->childCount = 0;
				normalNode->pChildren = NULL;
				cursor += sizeof(OptNode);
				normalNode->param2 = cursor;
				savedNormals = conversionState->vertexNormals;
				savedVertexCount = g_curVertexCount;
				conversionState->vertexNormals = NULL;
				OptModel_CollectUniqueVertexNormals(normalNode, sourceNode, sourceModel, conversionState);
				conversionState->vertexNormals = savedNormals;
				g_curVertexCount = savedVertexCount;
				g_optConvertVertexNormalNode = normalNode;
				cursor += sizeof(OptVector) * normalNode->param1;
#ifdef XW_MODERN
				cursor += (sizeof(void*) - (size_t)(cursor - destination) % sizeof(void*)) % sizeof(void*);
#endif
				outputNode = (OptNode*)cursor;
				wrapper->pChildren[OPT_SHARED_GEOMETRY_CHILD_COUNT] = outputNode;

				outputNode->nodeType = OPT_NODE_TYPE_21;
				outputNode->pName = NULL;
				cursor += sizeof(OptNode);
				outputNode->param2 = cursor;
				outputNode->param1 = sourceNode->param1;
				memcpy(cursor, sourceNode->param2, sizeof(int32_t) * sourceNode->param1);
				cursor += sizeof(int32_t) * sourceNode->param1;
				if (sourceNode->param1 < sourceNode->childCount) {
					outputNode->param1 = sourceNode->childCount;
					memset(cursor, 0, sizeof(int32_t) * (sourceNode->childCount - sourceNode->param1));
					cursor += sizeof(int32_t) * (sourceNode->childCount - sourceNode->param1);
				} else if (sourceNode->param1 > sourceNode->childCount) {
					outputNode->param1 = sourceNode->childCount;
					cursor -= sizeof(int32_t) * (sourceNode->param1 - sourceNode->childCount);
				}
				copyOriginalNode = 0;
			} else {
				outputNode = (OptNode*)cursor;
				outputNode->nodeType = OPT_NODE_TYPE_21;
				outputNode->pName = NULL;
				cursor += sizeof(OptNode);
				outputNode->param2 = cursor;
				outputNode->param1 = sourceNode->param1;
				memcpy(cursor, sourceNode->param2, sizeof(int32_t) * sourceNode->param1);
				cursor += sizeof(int32_t) * sourceNode->param1;
				if (sourceNode->param1 < sourceNode->childCount) {
					outputNode->param1 = sourceNode->childCount;
					memset(cursor, 0, sizeof(int32_t) * (sourceNode->childCount - sourceNode->param1));
					cursor += sizeof(int32_t) * (sourceNode->childCount - sourceNode->param1);
				} else if (sourceNode->param1 > sourceNode->childCount) {
					outputNode->param1 = sourceNode->childCount;
					cursor -= sizeof(int32_t) * (sourceNode->param1 - sourceNode->childCount);
				}
				copyOriginalNode = 0;
			}
			break;
		case OPT_NODEREF: {
			OptNode* resolvedNode = sourceNode;
			payloadBytes = strlen(sourceNode->param2) + 1;
			do {
				resolvedNode = OptModel_ResolveNodeRef(sourceModel, resolvedNode->param2);
			} while (resolvedNode != NULL && resolvedNode->nodeType == OPT_NODEREF);
			if (resolvedNode != NULL && resolvedNode->nodeType == OPT_TEXTURE)
				g_optConvertSourceTextureNode = resolvedNode;
			break;
		}
		case OPT_NODE_TYPE_22:
			payloadBytes = OPT_NODE_22_PAYLOAD_SIZE;
			break;
		case OPT_MESHDESCRIPTOR:
			payloadBytes = sizeof(MeshDescriptor);
			break;
		case OPT_NODE_TYPE_2:
		case OPT_NODE_TYPE_23:
			payloadBytes = OPT_NODE_2_23_PAYLOAD_SIZE;
			break;
		case OPT_NODE_TYPE_5:
			payloadBytes = OPT_NODE_5_PAYLOAD_SIZE;
			break;
		case OPT_NODE_TYPE_4:
		case OPT_NODE_TYPE_6:
		case OPT_NODE_TYPE_19:
			payloadBytes = sizeof(OptVector);
			break;
		case OPT_MESHVERTS:
			g_sourceVectors = sourceNode->param2;
			g_curVertexCount = sourceNode->param1;
			if (g_optConvertVertexNode != NULL)
				copyOriginalNode = 0;
			else
				payloadBytes = sizeof(OptVector) * sourceNode->param1;
			break;
		case OPT_NODE_TYPE_9:
			payloadBytes = OPT_NODE_9_RECORD_SIZE * sourceNode->param1;
			g_curMeshDescriptorData = sourceNode->param2;
			break;
		case OPT_VERTNORMALS:
			g_curVertNormals = sourceNode->param2;
			conversionState->vertexNormals = g_curVertNormals;
			if (g_optConvertVertexNormalNode != NULL)
				copyOriginalNode = 0;
			else
				payloadBytes = sizeof(OptVector) * sourceNode->param1;
			break;
		case OPT_TEXCOORDS:
			g_sourceTexCoords = sourceNode->param2;
			if (g_optConvertTexCoordNode != NULL)
				copyOriginalNode = 0;
			else
				payloadBytes = sizeof(OptTexCoord) * sourceNode->param1;
			break;
		case OPT_FACEDATA:
		case OPT_FACEDATA_15:
		case OPT_FACEDATA_16:
		case OPT_FACEDATA_17:
			copyOriginalNode = 0;
			if (((OptPackedFaceData*)sourceNode->param2)->edgeCount >= 0) {
				outputNode = (OptNode*)cursor;
				outputNode->nodeType = sourceNode->nodeType;
				cursor += sizeof(OptNode);
				if (sourceNode->pName != NULL) {
					outputNode->pName = (char*)cursor;
					strcpy((char*)cursor, sourceNode->pName);
					cursor += strlen(sourceNode->pName) + 1;
				} else {
					outputNode->pName = NULL;
				}
#ifdef XW_MODERN
				cursor += (sizeof(void*) - (size_t)(cursor - destination) % sizeof(void*)) % sizeof(void*);
#endif
				outputNode->param2 = cursor;
				if (g_optConvertVertexNode != NULL) {
					outputNode->param1 = 0;
					((OptPackedFaceData*)cursor)->edgeCount = 0;
					OptModel_AppendConvertedFacesForCurrentMesh(outputNode, sourceNode, sourceModel,
																conversionState);
					cursor +=
						sizeof(((OptPackedFaceData*)0)->edgeCount) +
						(sizeof(OptPackedFaceRecord) + sizeof(OptVector) + sizeof(FaceTextureGradients)) *
							outputNode->param1;
				} else {
					size_t faceBytes;
					const uint8_t* sourceDerived;
					outputNode->param1 = sourceNode->param1;
					if (g_optSourceIsVersion0)
						faceBytes =
							(sizeof(OptPackedFaceRecord) - sizeof(((OptPackedFaceRecord*)0)->normalIndices)) *
							sourceNode->param1;
					else
						faceBytes = sizeof(OptPackedFaceRecord) * sourceNode->param1;
					faceBytes += sizeof(((OptPackedFaceData*)0)->edgeCount);
					memcpy(cursor, sourceNode->param2, faceBytes);
					cursor += faceBytes;
					payloadBytes = sizeof(((OptPackedFaceRecord*)0)->normalIndices) * sourceNode->param1;
					memcpy(cursor, sourceNode->param2, payloadBytes);
					cursor += payloadBytes;
					sourceDerived = (const uint8_t*)sourceNode->param2 + faceBytes;
					payloadBytes = (sizeof(OptVector) + sizeof(FaceTextureGradients)) * sourceNode->param1;
					memcpy(cursor, sourceDerived, payloadBytes);
					cursor += payloadBytes;
					sourceDerived += payloadBytes;
					if (conversionState->vertexNormals == NULL) {
						payloadBytes = sizeof(OptVector) * g_curVertexCount;
						memcpy(cursor, sourceDerived, payloadBytes);
						cursor += payloadBytes;
					}
				}
			}
			break;
		case OPT_TEXTURE: {
			OptTextureData* texture = sourceNode->param2;
			int pixelBytes = texture->width * texture->height;
			g_optConvertSourceTextureNode = sourceNode;
			if (pixelBytes == texture->textureSize)
				pixelBytes = texture->dataSize;
			payloadBytes = sizeof(OptTextureData) + pixelBytes;
			if (texture->paletteType != 0)
				payloadBytes +=
					OPT_TEXTURE_PALETTE_COLOR_COUNT * OPT_RGB_CHANNEL_COUNT * texture->paletteType;
			else if ((uint8_t*)(texture + 1) + pixelBytes == (uint8_t*)texture->palette)
				payloadBytes += sizeof(OptShadeTable);
			break;
		}
		default:
			break;
	}
	if (copyOriginalNode == 1) {
		outputNode = (OptNode*)cursor;
		outputNode->nodeType = sourceNode->nodeType;
		cursor += sizeof(OptNode);
		if (sourceNode->pName != NULL) {
			outputNode->pName = (char*)cursor;
			strcpy((char*)cursor, sourceNode->pName);
			cursor += strlen(sourceNode->pName) + 1;
		} else {
			outputNode->pName = NULL;
		}
#ifdef XW_MODERN
		cursor += (sizeof(void*) - (size_t)(cursor - destination) % sizeof(void*)) % sizeof(void*);
#endif
		outputNode->param1 = sourceNode->param1;
		outputNode->param2 = cursor;
		memcpy(cursor, sourceNode->param2, payloadBytes);
		cursor += payloadBytes;
		if (sourceNode->nodeType == OPT_TEXTURE) {
			OptTextureData* texture = sourceNode->param2;
			OptTextureData* outputTexture = outputNode->param2;
			if (texture->paletteType == 0) {
				int pixelBytes = texture->width * texture->height;
				if (pixelBytes == texture->textureSize)
					pixelBytes = texture->dataSize;
				if ((uint8_t*)(texture + 1) + pixelBytes == (uint8_t*)texture->palette) {
					int outputBytes = outputTexture->width * outputTexture->height;
					if (outputBytes == outputTexture->textureSize)
						outputBytes = outputTexture->dataSize;
					outputTexture->palette = (uint16_t*)((uint8_t*)(outputTexture + 1) + outputBytes);
				} else {
					OptShadeTable* shared = OptModel_FindEarlierSharedTextureData(
						(const OptShadeTable*)texture->palette, destinationModel, outputNode);
					if (shared != NULL) {
						outputTexture->palette = (uint16_t*)shared;
					} else {
						outputTexture->palette = (uint16_t*)cursor;
						memcpy(cursor, texture->palette, sizeof(OptShadeTable));
						cursor += sizeof(OptShadeTable);
					}
				}
			}
		}
	}
	if (outputNode == NULL) {
		if (sourceNode->childCount == 0)
			return 0;
		outputNode = (OptNode*)cursor;
		cursor += sizeof(OptNode);
		outputNode->pName = NULL;
		outputNode->nodeType = OPT_NODE_GROUP;
		outputNode->param1 = 0;
		outputNode->param2 = NULL;
	}
	outputNode->childCount = 0;
	outputNode->pChildren = NULL;
	if (sourceNode->childCount != 0) {
		int firstOriginalChildSlot;
		int childIndex;
#ifdef XW_MODERN
		cursor += (sizeof(void*) - (size_t)(cursor - destination) % sizeof(void*)) % sizeof(void*);
#endif
		if (g_optConvertVertexNode != NULL) {
			outputNode->childCount = sourceNode->childCount;
			outputNode->pChildren = (OptNode**)cursor;
			cursor += sizeof(OptNode*) * sourceNode->childCount;
			firstOriginalChildSlot = 0;
		} else {
			OptNode* vertexNode;
			OptNode* texCoordNode;
			OptNode* normalNode;
			OptVector* vertices;
			OptVector* savedNormals;
			int savedVertexCount;
			int vertexIndex;
			float minX, minY, minZ, maxX, maxY;
			float maxZ;
			outputNode->childCount = sourceNode->childCount + OPT_SHARED_GEOMETRY_CHILD_COUNT;
			outputNode->pChildren = (OptNode**)cursor;
			cursor += sizeof(OptNode*) * outputNode->childCount;
#ifdef XW_MODERN
			cursor += (sizeof(void*) - (size_t)(cursor - destination) % sizeof(void*)) % sizeof(void*);
#endif
			vertexNode = (OptNode*)cursor;
			outputNode->pChildren[OPT_SHARED_VERTEX_CHILD] = vertexNode;
			vertexNode->pName = NULL;
			vertexNode->nodeType = OPT_MESHVERTS;
			vertexNode->param1 = 0;
			vertexNode->childCount = 0;
			vertexNode->pChildren = NULL;
			cursor += sizeof(OptNode);
			vertexNode->param2 = cursor;
			OptModel_CollectUniqueVertices(vertexNode, sourceNode, sourceModel, conversionState);
			g_optConvertVertexNode = vertexNode;
			vertices = vertexNode->param2;
#ifdef XW_MODERN
			if (vertexNode->param1 == 0) {
				minX = maxX = 0.0f;
				minY = maxY = 0.0f;
				minZ = maxZ = 0.0f;
			} else
#endif
			{
				maxX = vertices[0].x;
				minX = maxX;
				maxY = vertices[0].y;
				minY = maxY;
				maxZ = vertices[0].z;
				minZ = vertices[0].z;
			}
			for (vertexIndex = 0; vertexIndex < vertexNode->param1; ++vertexIndex) {
				if (
#ifdef XW_MODERN
					!(vertices[vertexIndex].x >= minX)
#else
					vertices[vertexIndex].x < minX
#endif
				)
					minX = vertices[vertexIndex].x;
				if (
#ifdef XW_MODERN
					!(vertices[vertexIndex].y >= minY)
#else
					vertices[vertexIndex].y < minY
#endif
				)
					minY = vertices[vertexIndex].y;
				if (
#ifdef XW_MODERN
					!(vertices[vertexIndex].z >= minZ)
#else
					vertices[vertexIndex].z < minZ
#endif
				)
					minZ = vertices[vertexIndex].z;
				if (vertices[vertexIndex].x > maxX)
					maxX = vertices[vertexIndex].x;
				if (vertices[vertexIndex].y > maxY)
					maxY = vertices[vertexIndex].y;
				if (
#ifdef XW_MODERN
					!(maxZ >= vertices[vertexIndex].z)
#else
					maxZ < vertices[vertexIndex].z
#endif
				)
					maxZ = vertices[vertexIndex].z;
			}
			if (
#ifdef XW_MODERN
				vertexIndex < OPT_BOUNDS_VERTEX_COUNT ||
				(vertices[vertexIndex - OPT_BOUNDS_VERTEX_COUNT].x < minX ||
				 vertices[vertexIndex - OPT_BOUNDS_VERTEX_COUNT].x > minX) ||
				(vertices[vertexIndex - OPT_BOUNDS_VERTEX_COUNT].y < minY ||
				 vertices[vertexIndex - OPT_BOUNDS_VERTEX_COUNT].y > minY) ||
				(vertices[vertexIndex - OPT_BOUNDS_VERTEX_COUNT].z < minZ ||
				 vertices[vertexIndex - OPT_BOUNDS_VERTEX_COUNT].z > minZ) ||
				(vertices[vertexIndex - 1].x < maxX || vertices[vertexIndex - 1].x > maxX) ||
				(vertices[vertexIndex - 1].y < maxY || vertices[vertexIndex - 1].y > maxY) ||
				(vertices[vertexIndex - 1].z < maxZ || vertices[vertexIndex - 1].z > maxZ)
#else
				vertices[vertexIndex - OPT_BOUNDS_VERTEX_COUNT].x != minX ||
				vertices[vertexIndex - OPT_BOUNDS_VERTEX_COUNT].y != minY ||
				vertices[vertexIndex - OPT_BOUNDS_VERTEX_COUNT].z != minZ ||
				vertices[vertexIndex - 1].x != maxX || vertices[vertexIndex - 1].y != maxY ||
				maxZ != vertices[vertexIndex - 1].z
#endif
			) {
				vertices[vertexIndex].x = minX;
				vertices[vertexIndex].y = minY;
				vertices[vertexIndex].z = minZ;
				vertices[vertexIndex + 1].x = maxX;
				vertices[vertexIndex + 1].y = maxY;
				vertices[vertexIndex + 1].z = (float)maxZ;
				vertexNode->param1 += OPT_BOUNDS_VERTEX_COUNT;
			}
			cursor += sizeof(OptVector) * vertexNode->param1;
#ifdef XW_MODERN
			cursor += (sizeof(void*) - (size_t)(cursor - destination) % sizeof(void*)) % sizeof(void*);
#endif
			texCoordNode = (OptNode*)cursor;
			outputNode->pChildren[OPT_SHARED_TEXCOORD_CHILD] = texCoordNode;
			texCoordNode->nodeType = OPT_TEXCOORDS;
			texCoordNode->pName = NULL;
			texCoordNode->param1 = 0;
			texCoordNode->childCount = 0;
			texCoordNode->pChildren = NULL;
			cursor += sizeof(OptNode);
			texCoordNode->param2 = cursor;
			OptModel_CollectUniqueTexCoords(texCoordNode, sourceNode, sourceModel, conversionState);
			g_optConvertTexCoordNode = texCoordNode;
			cursor += sizeof(OptTexCoord) * texCoordNode->param1;
#ifdef XW_MODERN
			cursor += (sizeof(void*) - (size_t)(cursor - destination) % sizeof(void*)) % sizeof(void*);
#endif
			normalNode = (OptNode*)cursor;
			outputNode->pChildren[OPT_SHARED_NORMAL_CHILD] = normalNode;
			normalNode->nodeType = OPT_VERTNORMALS;
			normalNode->pName = NULL;
			normalNode->param1 = 0;
			normalNode->childCount = 0;
			normalNode->pChildren = NULL;
			cursor += sizeof(OptNode);
			normalNode->param2 = cursor;
			savedNormals = conversionState->vertexNormals;
			savedVertexCount = g_curVertexCount;
			conversionState->vertexNormals = NULL;
			OptModel_CollectUniqueVertexNormals(normalNode, sourceNode, sourceModel, conversionState);
			conversionState->vertexNormals = savedNormals;
			g_curVertexCount = savedVertexCount;
			g_optConvertVertexNormalNode = normalNode;
			cursor += sizeof(OptVector) * normalNode->param1;
			firstOriginalChildSlot = OPT_SHARED_GEOMETRY_CHILD_COUNT;
		}
		childState = *conversionState;
		for (childIndex = 0; childIndex < sourceNode->childCount; ++childIndex) {
			size_t childBytes;
#ifdef XW_MODERN
			cursor += (sizeof(void*) - (size_t)(cursor - destination) % sizeof(void*)) % sizeof(void*);
#endif
			outputNode->pChildren[firstOriginalChildSlot + childIndex] = (OptNode*)cursor;
			childBytes = OptModel_ConvertLegacyNodeToOptimized(cursor, sourceNode->pChildren[childIndex],
															   sourceModel, destinationModel, &childState);
			if (childBytes == 0)
				outputNode->pChildren[firstOriginalChildSlot + childIndex] = NULL;
			cursor += childBytes;
		}
	}
	return (size_t)(cursor - destination);
}

// FUNCTION: XW 0x48BE30
void OptModel_CollectUniqueVertices(struct OptNode* dstVertexNode, struct OptNode* srcNode,
									struct OptimizedPolyObject* srcModel, void* meshState) {
	int childIndex;
	if (srcNode == NULL)
		return;
	while (srcNode->nodeType == OPT_NODEREF) {
		srcNode = OptModel_ResolveNodeRef(srcModel, srcNode->param2);
		if (srcNode == NULL)
			return;
	}
	if (srcNode->nodeType == OPT_MESHVERTS) {
		const OptVector* sourceVertices = srcNode->param2;
		int sourceIndex;
		for (sourceIndex = 0; sourceIndex < srcNode->param1; ++sourceIndex) {
			int uniqueCount = dstVertexNode->param1;
			OptVector* uniqueVertices = dstVertexNode->param2;
			int uniqueIndex;
			for (uniqueIndex = 0; uniqueIndex < uniqueCount; ++uniqueIndex) {
#ifdef XW_MODERN
				if (!(sourceVertices[sourceIndex].x < uniqueVertices[uniqueIndex].x ||
					  sourceVertices[sourceIndex].x > uniqueVertices[uniqueIndex].x) &&
					!(sourceVertices[sourceIndex].y < uniqueVertices[uniqueIndex].y ||
					  sourceVertices[sourceIndex].y > uniqueVertices[uniqueIndex].y) &&
					!(sourceVertices[sourceIndex].z < uniqueVertices[uniqueIndex].z ||
					  sourceVertices[sourceIndex].z > uniqueVertices[uniqueIndex].z)) {
#else
				if (sourceVertices[sourceIndex].x == uniqueVertices[uniqueIndex].x &&
					sourceVertices[sourceIndex].y == uniqueVertices[uniqueIndex].y &&
					sourceVertices[sourceIndex].z == uniqueVertices[uniqueIndex].z) {
#endif
					break;
				}
			}
			if (uniqueIndex == uniqueCount) {
				uniqueVertices[uniqueIndex] = sourceVertices[sourceIndex];
				++dstVertexNode->param1;
			}
		}
	}
	for (childIndex = 0; childIndex < srcNode->childCount; ++childIndex)
		OptModel_CollectUniqueVertices(dstVertexNode, srcNode->pChildren[childIndex], srcModel, meshState);
}

// FUNCTION: XW 0x48BF30
void OptModel_CollectUniqueTexCoords(struct OptNode* dstTexCoordNode, struct OptNode* srcNode,
									 struct OptimizedPolyObject* srcModel, void* meshState) {
	int childIndex;
	if (srcNode == NULL) {
		return;
	}
	while (srcNode->nodeType == OPT_NODEREF) {
		srcNode = OptModel_ResolveNodeRef(srcModel, srcNode->param2);
		if (srcNode == NULL) {
			return;
		}
	}
	if (srcNode->nodeType == OPT_TEXCOORDS) {
		const OptTexCoord* sourceTexCoords = srcNode->param2;
		int sourceIndex;
		for (sourceIndex = 0; sourceIndex < srcNode->param1; ++sourceIndex) {
			int uniqueCount = dstTexCoordNode->param1;
			OptTexCoord* uniqueTexCoords = dstTexCoordNode->param2;
			int uniqueIndex;
			for (uniqueIndex = 0; uniqueIndex < uniqueCount; ++uniqueIndex) {
#ifdef XW_MODERN
				if (!(sourceTexCoords[sourceIndex].u < uniqueTexCoords[uniqueIndex].u ||
					  sourceTexCoords[sourceIndex].u > uniqueTexCoords[uniqueIndex].u) &&
					!(sourceTexCoords[sourceIndex].v < uniqueTexCoords[uniqueIndex].v ||
					  sourceTexCoords[sourceIndex].v > uniqueTexCoords[uniqueIndex].v)) {
#else
				if (sourceTexCoords[sourceIndex].u == uniqueTexCoords[uniqueIndex].u &&
					sourceTexCoords[sourceIndex].v == uniqueTexCoords[uniqueIndex].v) {
#endif
					break;
				}
			}
			if (uniqueIndex == uniqueCount) {
				uniqueTexCoords[uniqueIndex] = sourceTexCoords[sourceIndex];
				++dstTexCoordNode->param1;
			}
		}
	}
	for (childIndex = 0; childIndex < srcNode->childCount; ++childIndex) {
		OptModel_CollectUniqueTexCoords(dstTexCoordNode, srcNode->pChildren[childIndex], srcModel, meshState);
	}
}

// FUNCTION: XW 0x48C010
void OptModel_CollectUniqueVertexNormals(struct OptNode* dstNormalNode, struct OptNode* srcNode,
										 struct OptimizedPolyObject* srcModel, struct SceneMesh* meshState) {
	int childIndex;
	if (srcNode == NULL)
		return;
	while (srcNode->nodeType == OPT_NODEREF) {
		srcNode = OptModel_ResolveNodeRef(srcModel, srcNode->param2);
		if (srcNode == NULL)
			return;
	}
	switch (srcNode->nodeType) {
		case OPT_MESHVERTS:
			g_curVertexCount = srcNode->param1;
			break;
		case OPT_VERTNORMALS: {
			const OptVector* sourceNormals;
			int sourceIndex;
			meshState->vertexNormals = srcNode->param2;
			sourceNormals = srcNode->param2;
			for (sourceIndex = 0; sourceIndex < srcNode->param1; ++sourceIndex) {
				int uniqueCount = dstNormalNode->param1;
				OptVector* uniqueNormals = dstNormalNode->param2;
				int uniqueIndex;
				OptVector* candidate;
				for (uniqueIndex = 0;; ++uniqueIndex) {
					candidate = &uniqueNormals[uniqueIndex];
					if (uniqueIndex >= uniqueCount)
						break;
#ifdef XW_MODERN
					if (!(sourceNormals[sourceIndex].x < candidate->x ||
						  sourceNormals[sourceIndex].x > candidate->x) &&
						!(sourceNormals[sourceIndex].y < candidate->y ||
						  sourceNormals[sourceIndex].y > candidate->y) &&
						!(sourceNormals[sourceIndex].z < candidate->z ||
						  sourceNormals[sourceIndex].z > candidate->z)) {
#else
					if (sourceNormals[sourceIndex].x == candidate->x &&
						sourceNormals[sourceIndex].y == candidate->y &&
						sourceNormals[sourceIndex].z == candidate->z) {
#endif
						break;
					}
				}
				if (uniqueIndex == uniqueCount) {
					*candidate = sourceNormals[sourceIndex];
					++dstNormalNode->param1;
				}
			}
			break;
		}
		case OPT_FACEDATA:
		case OPT_FACEDATA_15:
		case OPT_FACEDATA_16:
		case OPT_FACEDATA_17:
			if (meshState->vertexNormals == NULL) {
				const OptPackedFaceData* faceData = srcNode->param2;
				const OptVector* sourceNormals;
				int sourceIndex;
				/* Version zero omits the normal-index array from each face record.
				 * Both formats append one normal and two texture gradients per face. */
				if (g_optSourceIsVersion0) {
					sourceNormals =
						(const OptVector*)((const uint8_t*)faceData + sizeof(faceData->edgeCount) +
										   srcNode->param1 *
											   ((sizeof(OptPackedFaceRecord) -
												 sizeof(((OptPackedFaceRecord*)0)->normalIndices)) +
												sizeof(OptVector) + sizeof(struct FaceTextureGradients)));
				} else {
					sourceNormals =
						(const OptVector*)((const uint8_t*)faceData + sizeof(faceData->edgeCount) +
										   srcNode->param1 *
											   (sizeof(OptPackedFaceRecord) + sizeof(OptVector) +
												sizeof(struct FaceTextureGradients)));
				}
				for (sourceIndex = 0; sourceIndex < g_curVertexCount; ++sourceIndex) {
					int uniqueCount = dstNormalNode->param1;
					OptVector* uniqueNormals = dstNormalNode->param2;
					int uniqueIndex;
					OptVector* candidate;
					for (uniqueIndex = 0;; ++uniqueIndex) {
						candidate = &uniqueNormals[uniqueIndex];
						if (uniqueIndex >= uniqueCount)
							break;
#ifdef XW_MODERN
						if (!(sourceNormals[sourceIndex].x < candidate->x ||
							  sourceNormals[sourceIndex].x > candidate->x) &&
							!(sourceNormals[sourceIndex].y < candidate->y ||
							  sourceNormals[sourceIndex].y > candidate->y) &&
							!(sourceNormals[sourceIndex].z < candidate->z ||
							  sourceNormals[sourceIndex].z > candidate->z)) {
#else
						if (sourceNormals[sourceIndex].x == candidate->x &&
							sourceNormals[sourceIndex].y == candidate->y &&
							sourceNormals[sourceIndex].z == candidate->z) {
#endif
							break;
						}
					}
					if (uniqueIndex == uniqueCount) {
						*candidate = sourceNormals[sourceIndex];
						++dstNormalNode->param1;
					}
				}
			}
			break;
	}
	if (g_optSourceIsVersion0)
		meshState->vertexNormals = NULL;
	for (childIndex = 0; childIndex < srcNode->childCount; ++childIndex)
		OptModel_CollectUniqueVertexNormals(dstNormalNode, srcNode->pChildren[childIndex], srcModel,
											meshState);
}

// FUNCTION: XW 0x48C240
int OptModel_RemapVectorIndex(const struct OptNode* uniqueVectorNode, const struct OptVector* sourceVectors,
							  int sourceIndex) {
	const OptVector* uniqueVectors;
	const OptVector* source;
	int searchIndex;
	if (sourceIndex < 0) {
		return OPT_INDEX_NONE;
	}
	source = &sourceVectors[sourceIndex];
	uniqueVectors = uniqueVectorNode->param2;
	searchIndex = g_optConvertVectorSearchCursor - (sourceIndex >> 1);
	g_optConvertVectorSearchCursor = searchIndex;
	if (searchIndex < 0 || searchIndex > uniqueVectorNode->param1) {
		searchIndex = 0;
		g_optConvertVectorSearchCursor = searchIndex;
	}
	for (; searchIndex < uniqueVectorNode->param1;
		 ++searchIndex, g_optConvertVectorSearchCursor = searchIndex) {
		const OptVector* candidate = &uniqueVectors[searchIndex];
#ifdef XW_MODERN
		if (!(candidate->x < source->x || candidate->x > source->x) &&
			!(candidate->y < source->y || candidate->y > source->y) &&
			!(candidate->z < source->z || candidate->z > source->z)) {
#else
		if (candidate->x == source->x && candidate->y == source->y && candidate->z == source->z) {
#endif
			return searchIndex;
		}
	}
	searchIndex = 0;
	g_optConvertVectorSearchCursor = searchIndex;
	uniqueVectors = uniqueVectorNode->param2;
	for (; searchIndex < uniqueVectorNode->param1;
		 ++searchIndex, g_optConvertVectorSearchCursor = searchIndex) {
		const OptVector* candidate = &uniqueVectors[searchIndex];
#ifdef XW_MODERN
		if (!(candidate->x < source->x || candidate->x > source->x) &&
			!(candidate->y < source->y || candidate->y > source->y) &&
			!(candidate->z < source->z || candidate->z > source->z)) {
#else
		if (candidate->x == source->x && candidate->y == source->y && candidate->z == source->z) {
#endif
			return searchIndex;
		}
	}
	return 0;
}

// FUNCTION: XW 0x48C320
int OptModel_RemapTexCoordIndex(const struct OptNode* uniqueTexCoordNode,
								const struct OptTexCoord* sourceTexCoords, int sourceIndex) {
	const OptTexCoord* uniqueTexCoords;
	const OptTexCoord* source;
	if (sourceIndex < 0) {
		return OPT_INDEX_NONE;
	}
	source = &sourceTexCoords[sourceIndex];
	uniqueTexCoords = uniqueTexCoordNode->param2;
	g_optConvertTexCoordSearchCursor -= sourceIndex >> 1;
	if (g_optConvertTexCoordSearchCursor < 0 ||
		g_optConvertTexCoordSearchCursor > uniqueTexCoordNode->param1) {
		g_optConvertTexCoordSearchCursor = 0;
	}
	for (; g_optConvertTexCoordSearchCursor < uniqueTexCoordNode->param1;
		 ++g_optConvertTexCoordSearchCursor) {
		const OptTexCoord* candidate = &uniqueTexCoords[g_optConvertTexCoordSearchCursor];
#ifdef XW_MODERN
		if (!(candidate->u < source->u || candidate->u > source->u) &&
			!(candidate->v < source->v || candidate->v > source->v)) {
#else
		if (candidate->u == source->u && candidate->v == source->v) {
#endif
			return g_optConvertTexCoordSearchCursor;
		}
	}
	g_optConvertTexCoordSearchCursor = 0;
	uniqueTexCoords = uniqueTexCoordNode->param2;
	for (; g_optConvertTexCoordSearchCursor < uniqueTexCoordNode->param1;
		 ++g_optConvertTexCoordSearchCursor) {
		const OptTexCoord* candidate = &uniqueTexCoords[g_optConvertTexCoordSearchCursor];
#ifdef XW_MODERN
		if (!(candidate->u < source->u || candidate->u > source->u) &&
			!(candidate->v < source->v || candidate->v > source->v)) {
#else
		if (candidate->u == source->u && candidate->v == source->v) {
#endif
			return g_optConvertTexCoordSearchCursor;
		}
	}
	return 0;
}

// FUNCTION: XW 0x48C3F0
void OptModel_AppendConvertedFacesForNode(struct OptNode* destinationFaceNode,
										  struct OptNode* firstSourceFaceNode, struct OptNode* node,
										  struct OptimizedPolyObject* sourceModel,
										  struct SceneMesh* conversionState) {
	int childIndex;
	if (node != NULL) {
		while (node->nodeType == OPT_NODEREF) {
			node = OptModel_ResolveNodeRef(sourceModel, node->param2);
			if (node == NULL)
				return;
		}
		if (g_optConvertTargetFaceFound == 0 && node == firstSourceFaceNode)
			g_optConvertTargetFaceFound = 1;
		switch (node->nodeType) {
			case OPT_TEXTURE:
				g_optConvertFaceTextureNode = node;
				break;
			default:
				break;
			case OPT_MESHVERTS:
				g_sourceVectors = node->param2;
				break;
			case OPT_TEXCOORDS:
				g_sourceTexCoords = node->param2;
				break;
			case OPT_VERTNORMALS:
				conversionState->vertexNormals = node->param2;
				break;
			case OPT_FACEDATA:
			case OPT_FACEDATA_15:
			case OPT_FACEDATA_16:
			case OPT_FACEDATA_17:
				if (g_optConvertTargetFaceFound != 0 &&
					g_optConvertFaceTextureNode == g_optConvertSourceTextureNode &&
					((OptPackedFaceData*)node->param2)->edgeCount > 0) {
					OptPackedFaceData* destinationData = destinationFaceNode->param2;
					OptPackedFaceRecord* destinationFaces = destinationData->records;
					const uint8_t* sourceFaces;
					const OptVector* sourceVertexNormals;
					OptVector* destinationNormals;
					const OptVector* sourceNormals;
					FaceTextureGradients* destinationGradients;
					const FaceTextureGradients* sourceGradients;
					size_t sourceOffset;
					int edgeOffset = destinationData->edgeCount;
					int faceIndex;
					memmove(&destinationFaces[node->param1], destinationFaces,
							destinationFaceNode->param1 *
								(int)(sizeof(OptPackedFaceRecord) + sizeof(OptVector) +
									  sizeof(FaceTextureGradients)));
					sourceFaces = (const uint8_t*)node->param2 + sizeof(destinationData->edgeCount);
					sourceVertexNormals = conversionState->vertexNormals;
					if (sourceVertexNormals == NULL) {
						if (g_optSourceIsVersion0)
							sourceVertexNormals =
								(const OptVector*)(sourceFaces +
												   node->param1 *
													   ((sizeof(OptPackedFaceRecord) -
														 sizeof(destinationFaces->normalIndices)) +
														sizeof(OptVector) + sizeof(FaceTextureGradients)));
						else
							sourceVertexNormals =
								(const OptVector*)(sourceFaces +
												   node->param1 *
													   (sizeof(OptPackedFaceRecord) + sizeof(OptVector) +
														sizeof(FaceTextureGradients)));
					}
					for (faceIndex = 0, sourceOffset = 0; faceIndex < node->param1; ++faceIndex,
						sourceOffset += (g_optSourceIsVersion0 ? sizeof(OptPackedFaceRecord) -
																	 sizeof(destinationFaces->normalIndices)
															   : sizeof(OptPackedFaceRecord))) {
						const OptPackedFaceRecord* sourceFace =
							(const OptPackedFaceRecord*)(sourceFaces + sourceOffset);
						OptPackedFaceRecord* destinationFace = &destinationFaces[faceIndex];
						const int* normalIndices;
						destinationFace->vertexIndices[0] = OptModel_RemapVectorIndex(
							g_optConvertVertexNode, g_sourceVectors, sourceFace->vertexIndices[0]);
						destinationFace->vertexIndices[1] = OptModel_RemapVectorIndex(
							g_optConvertVertexNode, g_sourceVectors, sourceFace->vertexIndices[1]);
						destinationFace->vertexIndices[2] = OptModel_RemapVectorIndex(
							g_optConvertVertexNode, g_sourceVectors, sourceFace->vertexIndices[2]);
						destinationFace->vertexIndices[3] = OptModel_RemapVectorIndex(
							g_optConvertVertexNode, g_sourceVectors, sourceFace->vertexIndices[3]);
						destinationFace->edgeIndices[0] = sourceFace->edgeIndices[0] + edgeOffset;
						destinationFace->edgeIndices[1] = sourceFace->edgeIndices[1] + edgeOffset;
						destinationFace->edgeIndices[2] = sourceFace->edgeIndices[2] + edgeOffset;
						destinationFace->edgeIndices[3] = sourceFace->edgeIndices[3] == OPT_INDEX_NONE
															  ? OPT_INDEX_NONE
															  : sourceFace->edgeIndices[3] + edgeOffset;
						destinationFace->texCoordIndices[0] = OptModel_RemapTexCoordIndex(
							g_optConvertTexCoordNode, g_sourceTexCoords, sourceFace->texCoordIndices[0]);
						destinationFace->texCoordIndices[1] = OptModel_RemapTexCoordIndex(
							g_optConvertTexCoordNode, g_sourceTexCoords, sourceFace->texCoordIndices[1]);
						destinationFace->texCoordIndices[2] = OptModel_RemapTexCoordIndex(
							g_optConvertTexCoordNode, g_sourceTexCoords, sourceFace->texCoordIndices[2]);
						destinationFace->texCoordIndices[3] = OptModel_RemapTexCoordIndex(
							g_optConvertTexCoordNode, g_sourceTexCoords, sourceFace->texCoordIndices[3]);
						normalIndices =
							g_optSourceIsVersion0 ? sourceFace->vertexIndices : sourceFace->normalIndices;
						destinationFace->normalIndices[0] = OptModel_RemapVectorIndex(
							g_optConvertVertexNormalNode, sourceVertexNormals, normalIndices[0]);
						destinationFace->normalIndices[1] = OptModel_RemapVectorIndex(
							g_optConvertVertexNormalNode, sourceVertexNormals, normalIndices[1]);
						destinationFace->normalIndices[2] = OptModel_RemapVectorIndex(
							g_optConvertVertexNormalNode, sourceVertexNormals, normalIndices[2]);
						destinationFace->normalIndices[3] = OptModel_RemapVectorIndex(
							g_optConvertVertexNormalNode, sourceVertexNormals, normalIndices[3]);
					}
					destinationNormals =
						(OptVector*)&destinationFaces[destinationFaceNode->param1 + node->param1];
					memmove(&destinationNormals[node->param1], destinationNormals,
							destinationFaceNode->param1 *
								(int)(sizeof(OptVector) + sizeof(FaceTextureGradients)));
					if (g_optSourceIsVersion0)
						sourceNormals =
							(const OptVector*)((const uint8_t*)node->param2 +
											   sizeof(destinationData->edgeCount) +
											   node->param1 * (sizeof(OptPackedFaceRecord) -
															   sizeof(destinationFaces->normalIndices)));
					else
						sourceNormals = (const OptVector*)((const uint8_t*)node->param2 +
														   sizeof(destinationData->edgeCount) +
														   node->param1 * sizeof(OptPackedFaceRecord));
					for (faceIndex = 0; faceIndex < node->param1; ++faceIndex)
						destinationNormals[faceIndex] = sourceNormals[faceIndex];
					destinationGradients =
						(FaceTextureGradients*)&destinationNormals[destinationFaceNode->param1 +
																   node->param1];
					memmove(&destinationGradients[node->param1], destinationGradients,
							destinationFaceNode->param1 * (int)sizeof(FaceTextureGradients));
					if (g_optSourceIsVersion0)
						sourceGradients =
							(const FaceTextureGradients*)((const uint8_t*)node->param2 +
														  sizeof(destinationData->edgeCount) +
														  node->param1 *
															  ((sizeof(OptPackedFaceRecord) -
																sizeof(destinationFaces->normalIndices)) +
															   sizeof(OptVector)));
					else
						sourceGradients =
							(const FaceTextureGradients*)((const uint8_t*)node->param2 +
														  sizeof(destinationData->edgeCount) +
														  node->param1 * (sizeof(OptPackedFaceRecord) +
																		  sizeof(OptVector)));
					for (faceIndex = 0; faceIndex < node->param1; ++faceIndex) {
						memcpy(destinationGradients[faceIndex].gradient0,
							   sourceGradients[faceIndex].gradient0,
							   sizeof(destinationGradients[faceIndex].gradient0));
						memcpy(destinationGradients[faceIndex].gradient1,
							   sourceGradients[faceIndex].gradient1,
							   sizeof(destinationGradients[faceIndex].gradient1));
					}
					destinationData = destinationFaceNode->param2;
					destinationFaceNode->param1 += node->param1;
					destinationData->edgeCount += ((OptPackedFaceData*)node->param2)->edgeCount;
					((OptPackedFaceData*)node->param2)->edgeCount = OPT_INDEX_NONE;
				}
				break;
		}
		if (node->childCount != 0)
			for (childIndex = 0; childIndex < node->childCount; ++childIndex)
				OptModel_AppendConvertedFacesForNode(destinationFaceNode, firstSourceFaceNode,
													 node->pChildren[childIndex], sourceModel,
													 conversionState);
	}
}

// FUNCTION: XW 0x48C8B0
void OptModel_AppendConvertedFacesForCurrentMesh(struct OptNode* destinationFaceNode,
												 struct OptNode* firstSourceFaceNode,
												 struct OptimizedPolyObject* sourceModel,
												 struct SceneMesh* conversionState) {
	int childIndex;
	g_optConvertFaceTextureNode = g_optConvertSourceTextureNode;
	g_optConvertTargetFaceFound = 0;
	for (childIndex = 0; childIndex < g_optConvertCurrentMesh->childCount; ++childIndex) {
		OptModel_AppendConvertedFacesForNode(destinationFaceNode, firstSourceFaceNode,
											 g_optConvertCurrentMesh->pChildren[childIndex], sourceModel,
											 conversionState);
		if (g_optConvertTargetFaceFound != 0)
			break;
	}
}

// FUNCTION: XW 0x48C910
uint16_t OptModel_CreateRuntimeHandle(uint16_t packedModelHandle) {
	OptimizedPolyObject* sourceModel;
	OptimizedPolyObject* relockedSource;
	OptimizedPolyObject* runtimeModel;
	SceneMesh buildState;
	size_t allocationBytes;
	uint8_t* cursor;
	uint16_t runtimeHandle;
	int sizeRootIndex;
	int buildRootIndex;
	int fixupRootIndex;
	sourceModel = Memory_LockHandle(packedModelHandle);
	if (sourceModel->selfMarker != sourceModel)
		OptModel_AdjustOptimizedPolyObjectPointers(sourceModel);
	memset(&buildState, 0, sizeof(buildState));
	g_sourceVectors = NULL;
	g_sourceTexCoords = NULL;
	g_curVertNormals = NULL;
	g_optNodeWalkScratch2 = NULL;
	g_curMeshDescriptorData = NULL;
	g_curVertexCount = 0;
	allocationBytes = sizeof(OptimizedPolyObject) + sizeof(OptNode*) * sourceModel->rootNodeCount;
	for (sizeRootIndex = 0; sizeRootIndex < sourceModel->rootNodeCount; ++sizeRootIndex)
		allocationBytes +=
			OptModel_BuildRuntimeNode(sourceModel->rootNodes[sizeRootIndex], &buildState, NULL);
	nullsub_SharedNoOp();
	runtimeHandle = Memory_AllocHandle(allocationBytes, 0);
	if (runtimeHandle == 0)
		fediskio_fatalerror(FEDISKIO_ERROR_ALLOCATION);
	relockedSource = Memory_LockHandle(packedModelHandle);
	if (relockedSource->selfMarker != relockedSource)
		OptModel_AdjustOptimizedPolyObjectPointers(relockedSource);
	runtimeModel = Memory_LockHandle(runtimeHandle);
	memcpy(runtimeModel, relockedSource, sizeof(*runtimeModel));
	runtimeModel->selfMarker = runtimeModel;
	runtimeModel->rootNodes = (OptNode**)(runtimeModel + 1);
	cursor = (uint8_t*)(runtimeModel->rootNodes + relockedSource->rootNodeCount);
	for (buildRootIndex = 0; buildRootIndex < relockedSource->rootNodeCount; ++buildRootIndex) {
		runtimeModel->rootNodes[buildRootIndex] = (OptNode*)cursor;
		cursor += OptModel_BuildRuntimeNode(relockedSource->rootNodes[buildRootIndex], &buildState, cursor);
	}
	for (fixupRootIndex = 0; fixupRootIndex < runtimeModel->rootNodeCount; ++fixupRootIndex)
		OptModel_FixupRuntimeTexturePointers(runtimeModel->rootNodes[fixupRootIndex], runtimeModel,
											 relockedSource);
	nullsub_SharedNoOp();
	nullsub_SharedNoOp();
#ifdef XW_MODERN
	XwRenderAssets_AttachHandle(runtimeHandle, XwRenderAssets_HandleId(packedModelHandle));
#endif
	return runtimeHandle;
}

// FUNCTION: XW 0x48CA90
void OptModel_FixupRuntimeTexturePointers(struct OptNode* node, struct OptimizedPolyObject* dstModel,
										  struct OptimizedPolyObject* srcModel) {
	int childIndex;
	if (node == NULL)
		return;
	while (node->nodeType == OPT_NODEREF) {
		node = OptModel_ResolveNodeRef(dstModel, node->param2);
		if (node == NULL)
			return;
	}
	if (node->nodeType == OPT_TEXTURE) {
		OptTextureData* texture = node->param2;
		if (texture->paletteType != 0) {
#ifdef XW_MODERN
			int payloadSize = (int32_t)((uint32_t)texture->height * (uint32_t)texture->width);
#else
			int payloadSize = texture->height * texture->width;
#endif
			uint8_t* palette;
			texture->paletteType = 0;
			palette = (uint8_t*)(texture + 1);
			if (payloadSize == texture->textureSize)
				payloadSize = texture->dataSize;
			palette += payloadSize;
			if (g_flightBytesPerPixel == sizeof(uint16_t))
				palette -= sizeof(((OptShadeTable*)0)->indexedShades);
			texture->palette = (uint16_t*)palette;
		} else {
			uint8_t* palette = (uint8_t*)texture->palette;
			int payloadSize;
			if (g_flightBytesPerPixel == sizeof(uint16_t))
				palette += sizeof(((OptShadeTable*)0)->indexedShades);
			palette -= sizeof(*texture);
#ifdef XW_MODERN
			payloadSize = (int32_t)((uint32_t)texture->height * (uint32_t)texture->width);
#else
			payloadSize = texture->height * texture->width;
#endif
			if (payloadSize == texture->textureSize)
				payloadSize = texture->dataSize;
			if ((OptTextureData*)(palette - payloadSize) != texture) {
				OptNode* matchedNode =
					OptModel_FindCorrespondingTextureNodeInModel(dstModel, srcModel, texture->palette);
				if (matchedNode != NULL) {
					OptTextureData* matchedTexture = matchedNode->param2;
#ifdef XW_MODERN
					int matchedPayloadSize =
						(int32_t)((uint32_t)matchedTexture->height * (uint32_t)matchedTexture->width);
#else
					int matchedPayloadSize = matchedTexture->height * matchedTexture->width;
#endif
					uint8_t* matchedPalette = (uint8_t*)(matchedTexture + 1);
					if (matchedPayloadSize == matchedTexture->textureSize)
						matchedPayloadSize = matchedTexture->dataSize;
					matchedPalette += matchedPayloadSize;
					if (g_flightBytesPerPixel == sizeof(uint16_t))
						matchedPalette -= sizeof(((OptShadeTable*)0)->indexedShades);
					texture->palette = (uint16_t*)matchedPalette;
				}
			}
		}
	}
	for (childIndex = 0; childIndex < node->childCount; ++childIndex)
		OptModel_FixupRuntimeTexturePointers(node->pChildren[childIndex], dstModel, srcModel);
}

// FUNCTION: XW 0x48CBB0
struct OptNode* OptModel_FindCorrespondingTextureNode(struct OptNode* srcNode, struct OptNode* dstNode,
													  const uint16_t* sourcePalette) {
	int childIndex;
	if (srcNode == NULL) {
		return NULL;
	}
	if (dstNode == NULL) {
		return NULL;
	}
	if (srcNode->nodeType != dstNode->nodeType) {
		return NULL;
	}
	if (srcNode->nodeType == OPT_TEXTURE) {
		OptTextureData* texture = srcNode->param2;
#ifdef XW_MODERN
		int payloadSize = (int32_t)((uint32_t)texture->width * (uint32_t)texture->height);
#else
		int payloadSize = texture->width * texture->height;
#endif
		uint8_t* payload = (uint8_t*)(texture + 1);
		uint16_t* inlinePalette;
		if (payloadSize == texture->textureSize) {
			inlinePalette = (uint16_t*)(payload + texture->dataSize);
		} else {
			inlinePalette = (uint16_t*)(payload + payloadSize);
		}
		if ((inlinePalette == texture->palette || texture->paletteType != 0) &&
			inlinePalette == sourcePalette) {
			return dstNode;
		}
	}
	if (srcNode->childCount != dstNode->childCount) {
		return NULL;
	}
	for (childIndex = 0; childIndex < srcNode->childCount; ++childIndex) {
		OptNode* result = OptModel_FindCorrespondingTextureNode(
			srcNode->pChildren[childIndex], dstNode->pChildren[childIndex], sourcePalette);
		if (result != NULL) {
			return result;
		}
	}
	return NULL;
}

// FUNCTION: XW 0x48CC50
struct OptNode* OptModel_FindCorrespondingTextureNodeInModel(struct OptimizedPolyObject* dstModel,
															 struct OptimizedPolyObject* srcModel,
															 const uint16_t* sourcePalette) {
	int rootIndex;
	for (rootIndex = 0; rootIndex < srcModel->rootNodeCount; ++rootIndex) {
		OptNode* result = OptModel_FindCorrespondingTextureNode(
			srcModel->rootNodes[rootIndex], dstModel->rootNodes[rootIndex], sourcePalette);
		if (result != NULL) {
			return result;
		}
	}
	return NULL;
}

// FUNCTION: XW 0x48CDD0
size_t OptModel_GetSerializedNodeSize(const struct OptNode* node, struct SceneMesh* parentState) {
	uint32_t sizeBytes;
	void* payload;
	OptNodeType nodeType;
	SceneMesh childState;
	if (node == NULL)
		return 0;
	sizeBytes = sizeof(OptNodeFileHeader);
	if (node->pName != NULL)
		sizeBytes += strlen(node->pName) + 1;
	payload = node->param2;
	nodeType = node->nodeType;
#ifdef XW_MODERN
	if (payload == NULL && node->nodeType == OPT_TEXTURE)
		return 0;
#endif
	if (payload != NULL) {
		switch (nodeType) {
			case OPT_NODE_TYPE_22:
				sizeBytes += OPT_NODE_22_PAYLOAD_SIZE;
				break;
			case OPT_MESHDESCRIPTOR:
				sizeBytes += sizeof(MeshDescriptor);
				break;
			case OPT_NODE_TYPE_21:
				sizeBytes += sizeof(int32_t) * node->param1;
				break;
			case OPT_NODEREF:
				sizeBytes += strlen(payload) + 1;
				break;
			case OPT_NODE_TYPE_2:
			case OPT_NODE_TYPE_23:
				sizeBytes += OPT_NODE_2_23_PAYLOAD_SIZE;
				break;
			case OPT_NODE_TYPE_5:
				sizeBytes += OPT_NODE_5_PAYLOAD_SIZE;
				break;
			case OPT_NODE_TYPE_4:
			case OPT_NODE_TYPE_6:
			case OPT_NODE_TYPE_19:
				sizeBytes += sizeof(OptVector);
				break;
			case OPT_MESHVERTS:
				g_curVertexCount = node->param1;
				sizeBytes += sizeof(OptVector) * g_curVertexCount;
				if (node->param1 > g_vertexRemapCapacity)
					g_vertexRemapCapacity = node->param1;
				break;
			case OPT_NODE_TYPE_9: {
				int count = node->param1;
				g_curMeshDescriptorData = payload;
				sizeBytes += (uint32_t)OPT_NODE_9_RECORD_SIZE * count;
				break;
			}
			case OPT_VERTNORMALS: {
				int count = node->param1;
				g_curVertNormals = payload;
				parentState->vertexNormals = payload;
				sizeBytes += sizeof(OptVector) * count;
				break;
			}
			case OPT_TEXCOORDS: {
				int count = node->param1;
				sizeBytes += sizeof(OptTexCoord) * count;
				break;
			}
			case OPT_FACEDATA:
			case OPT_FACEDATA_15:
			case OPT_FACEDATA_16:
			case OPT_FACEDATA_17: {
				const OptPackedFaceData* faces = payload;
				int faceCount = node->param1;
				if (faces->edgeCount > g_sceneEdgeFlagsCapacity)
					g_sceneEdgeFlagsCapacity = faces->edgeCount;
				sizeBytes +=
					(sizeof(OptPackedFaceRecord) + sizeof(OptVector) + sizeof(FaceTextureGradients)) *
						faceCount +
					sizeof(faces->edgeCount);
				if (parentState->vertexNormals == NULL)
					sizeBytes += sizeof(OptVector) * g_curVertexCount;
				break;
			}
			case OPT_TEXTURE: {
				const OptTextureData* texture = payload;
				int textureSize = texture->textureSize;
				int pixelCount;
				int paletteCount;
#ifdef XW_MODERN
				pixelCount = (int32_t)((uint32_t)texture->height * (uint32_t)texture->width);
#else
				pixelCount = texture->height * texture->width;
#endif
				sizeBytes += sizeof(OptTextureFileHeader);
				if (pixelCount == textureSize)
					sizeBytes += texture->dataSize;
				else
					sizeBytes += pixelCount;
				paletteCount = texture->paletteType;
				if (paletteCount != 0) {
					sizeBytes +=
						(uint32_t)OPT_TEXTURE_PALETTE_COLOR_COUNT * OPT_RGB_CHANNEL_COUNT * paletteCount;
				} else {
					const uint8_t* textureData = (const uint8_t*)(texture + 1);
					if (pixelCount == textureSize)
						pixelCount = texture->dataSize;
					textureData += pixelCount;
					if (textureData == (const uint8_t*)texture->palette)
						sizeBytes += sizeof(OptShadeTable);
				}
				break;
			}
			default:
				break;
		}
	} else {
		switch (nodeType) {
			case OPT_NODE_TYPE_10:
				break;
			case OPT_TEXTURE: {
				const OptTextureData* texture = payload;
				int textureSize = texture->textureSize;
				int pixelCount;
				int paletteCount;
#ifdef XW_MODERN
				pixelCount = (int32_t)((uint32_t)texture->height * (uint32_t)texture->width);
#else
				pixelCount = texture->height * texture->width;
#endif
				sizeBytes += sizeof(OptTextureFileHeader);
				if (pixelCount == textureSize)
					sizeBytes += texture->dataSize;
				else
					sizeBytes += pixelCount;
				paletteCount = texture->paletteType;
				if (paletteCount != 0) {
					sizeBytes +=
						(uint32_t)OPT_TEXTURE_PALETTE_COLOR_COUNT * OPT_RGB_CHANNEL_COUNT * paletteCount;
				} else {
					const uint8_t* textureData = (const uint8_t*)(texture + 1);
					if (pixelCount == textureSize)
						pixelCount = texture->dataSize;
					textureData += pixelCount;
					if (textureData == (const uint8_t*)texture->palette)
						sizeBytes += sizeof(OptShadeTable);
				}
				break;
			}
			default:
				break;
		}
	}
	if (node->childCount != 0) {
		int childIndex;
		childState = *parentState;
		g_sourceVectors = NULL;
		g_sourceTexCoords = NULL;
		g_curVertNormals = NULL;
		g_optNodeWalkScratch2 = NULL;
		g_curMeshDescriptorData = NULL;
		sizeBytes += sizeof(uint32_t) * node->childCount;
		for (childIndex = 0; childIndex < node->childCount; ++childIndex) {
			sizeBytes += OptModel_GetSerializedNodeSize(node->pChildren[childIndex], &childState);
		}
	}
	return sizeBytes;
}

// FUNCTION: XW 0x48D090
uint8_t OptModel_FindNearestRgb565Index(const uint16_t* palette, int targetRed, int targetGreen,
										int targetBlue, int startIndex, int endIndex) {
	int bestDistance = INT_MAX;
	uint8_t bestIndex = (uint8_t)targetBlue;
	for (; startIndex < endIndex; ++startIndex) {
		uint16_t color = palette[startIndex];
#ifdef XW_MODERN
		uint32_t blueDelta = (color & OPT_RGB565_BLUE_MASK) - (uint32_t)targetBlue;
		uint32_t greenDelta;
		uint32_t redDelta;
#else
		int blueDelta = (color & OPT_RGB565_BLUE_MASK) - targetBlue;
		int greenDelta;
		int redDelta;
#endif
		int distance;
		color >>= OPT_RGB565_GREEN_SHIFT;
#ifdef XW_MODERN
		greenDelta = (color & OPT_RGB565_GREEN_MASK) - (uint32_t)targetGreen;
		redDelta = ((color >> OPT_RGB565_GREEN_BITS) & OPT_RGB565_RED_MASK) - (uint32_t)targetRed;
		distance = (int32_t)(redDelta * redDelta + greenDelta * greenDelta + blueDelta * blueDelta);
#else
		greenDelta = (color & OPT_RGB565_GREEN_MASK) - targetGreen;
		redDelta = ((color >> OPT_RGB565_GREEN_BITS) & OPT_RGB565_RED_MASK) - targetRed;
		distance = redDelta * redDelta + greenDelta * greenDelta + blueDelta * blueDelta;
#endif
		if (distance == 0) {
			return (uint8_t)startIndex;
		}
		if (distance < bestDistance) {
			bestDistance = distance;
			bestIndex = (uint8_t)startIndex;
		}
	}
	return (uint8_t)bestIndex;
}

// FUNCTION: XW 0x48D150
void OptModel_PrepareTexturePalette(uint16_t* paletteRgb565, int entryCount) {
	RgbTriplet sourceRgb6[OPT_TEXTURE_SHADE_PALETTE_COUNT];
	int index;
	if (g_useHardware3D != 0)
		ModelTexture_FilterHardwarePalette(paletteRgb565);
	for (index = 0; index < entryCount; ++index) {
		unsigned int rgb565 = paletteRgb565[index];
		sourceRgb6[index].b = (rgb565 & RTSVGA2_RGB5_MASK) * RTSVGA2_RGB5_TO_RGB6_SCALE;
		rgb565 >>= OPT_RGB565_GREEN_SHIFT;
		sourceRgb6[index].g = rgb565 & RTSVGA2_RGB6_MASK;
		sourceRgb6[index].r =
			((rgb565 >> OPT_RGB565_GREEN_BITS) & RTSVGA2_RGB5_MASK) * RTSVGA2_RGB5_TO_RGB6_SCALE;
	}
	FlightPalette_Build16BppRange(sourceRgb6, paletteRgb565, 0, entryCount);
}

// FUNCTION: XW 0x48D1E0
size_t OptModel_BuildRuntimeNode(const struct OptNode* sourceNode, struct SceneMesh* buildState,
								 uint8_t* destination) {
	uint8_t* cursor;
	OptNode* outputNode;
	void* payload;
	size_t totalBytes;
	SceneMesh childState;
	if (sourceNode == NULL)
		return 0;
	cursor = destination;
	outputNode = NULL;
	if (cursor != NULL) {
		*(OptNode*)cursor = *sourceNode;
		outputNode = (OptNode*)cursor;
		cursor += sizeof(OptNode);
	}
	totalBytes = sizeof(OptNode);
	if (sourceNode->pName != NULL) {
		if (cursor != NULL) {
			outputNode->pName = (char*)cursor;
			strcpy((char*)cursor, sourceNode->pName);
			cursor += strlen(sourceNode->pName) + 1;
		}
		totalBytes += strlen(sourceNode->pName) + 1;
	}
#ifdef XW_MODERN
	{
		size_t padding = (sizeof(void*) - totalBytes % sizeof(void*)) % sizeof(void*);
		totalBytes += padding;
		if (cursor != NULL)
			cursor += padding;
	}
#endif
	if (sourceNode->childCount != 0) {
		if (cursor != NULL) {
			outputNode->pChildren = (OptNode**)cursor;
			cursor += sizeof(OptNode*) * sourceNode->childCount;
		}
		totalBytes += sizeof(OptNode*) * sourceNode->childCount;
	}
	payload = sourceNode->param2;
	switch (sourceNode->nodeType) {
		case OPT_NODE_TYPE_22:
			if (cursor != NULL) {
				outputNode->param2 = cursor;
				memcpy(cursor, sourceNode->param2, OPT_NODE_22_PAYLOAD_SIZE);
				cursor += OPT_NODE_22_PAYLOAD_SIZE;
			}
			totalBytes += OPT_NODE_22_PAYLOAD_SIZE;
			break;
		case OPT_MESHDESCRIPTOR:
			if (cursor != NULL) {
				outputNode->param2 = cursor;
				memcpy(cursor, sourceNode->param2, sizeof(MeshDescriptor));
				cursor += sizeof(MeshDescriptor);
			}
			totalBytes += sizeof(MeshDescriptor);
			break;
		case OPT_NODE_TYPE_23:
			if (cursor != NULL) {
				outputNode->param2 = cursor;
				memcpy(cursor, sourceNode->param2, OPT_NODE_2_23_PAYLOAD_SIZE);
				cursor += OPT_NODE_2_23_PAYLOAD_SIZE;
			}
			totalBytes += OPT_NODE_2_23_PAYLOAD_SIZE;
			break;
		case OPT_NODE_TYPE_21: {
			size_t bytes = sizeof(int32_t) * sourceNode->param1;
			if (cursor != NULL) {
				outputNode->param2 = cursor;
				memcpy(cursor, sourceNode->param2, bytes);
				cursor += bytes;
			}
			totalBytes += bytes;
			break;
		}
		case OPT_NODEREF: {
			size_t bytes = strlen(sourceNode->param2) + 1;
			if (cursor != NULL) {
				outputNode->param2 = cursor;
				memcpy(cursor, sourceNode->param2, bytes);
				cursor += bytes;
			}
			totalBytes += bytes;
			break;
		}
		case OPT_NODE_TYPE_19:
			if (cursor != NULL) {
				outputNode->param2 = cursor;
				memcpy(cursor, sourceNode->param2, sizeof(OptVector));
				cursor += sizeof(OptVector);
			}
			totalBytes += sizeof(OptVector);
			break;
		case OPT_NODE_TYPE_2:
			if (cursor != NULL) {
				outputNode->param2 = cursor;
				memcpy(cursor, sourceNode->param2, OPT_NODE_2_23_PAYLOAD_SIZE);
				cursor += OPT_NODE_2_23_PAYLOAD_SIZE;
			}
			totalBytes += OPT_NODE_2_23_PAYLOAD_SIZE;
			break;
		case OPT_NODE_TYPE_4:
			if (cursor != NULL) {
				outputNode->param2 = cursor;
				memcpy(cursor, sourceNode->param2, sizeof(OptVector));
				cursor += sizeof(OptVector);
			}
			totalBytes += sizeof(OptVector);
			break;
		case OPT_NODE_TYPE_5:
			if (cursor != NULL) {
				outputNode->param2 = cursor;
				memcpy(cursor, sourceNode->param2, OPT_NODE_5_PAYLOAD_SIZE);
				cursor += OPT_NODE_5_PAYLOAD_SIZE;
			}
			totalBytes += OPT_NODE_5_PAYLOAD_SIZE;
			break;
		case OPT_NODE_TYPE_6:
			if (cursor != NULL) {
				outputNode->param2 = cursor;
				memcpy(cursor, sourceNode->param2, sizeof(OptVector));
				cursor += sizeof(OptVector);
			}
			totalBytes += sizeof(OptVector);
			break;
		case OPT_MESHVERTS: {
			size_t bytes = sizeof(OptVector) * sourceNode->param1;
			if (cursor != NULL) {
				outputNode->param2 = cursor;
				memcpy(cursor, sourceNode->param2, bytes);
				cursor += bytes;
			}
			g_curVertexCount = sourceNode->param1;
			totalBytes += bytes;
			if (sourceNode->param1 > g_vertexRemapCapacity)
				g_vertexRemapCapacity = sourceNode->param1;
			break;
		}
		case OPT_NODE_TYPE_9: {
			size_t bytes = (size_t)OPT_NODE_9_RECORD_SIZE * sourceNode->param1;
			if (cursor != NULL) {
				outputNode->param2 = cursor;
				memcpy(cursor, sourceNode->param2, bytes);
				cursor += bytes;
			}
			g_curMeshDescriptorData = payload;
			totalBytes += bytes;
			break;
		}
		case OPT_VERTNORMALS: {
			size_t bytes = sizeof(OptVector) * sourceNode->param1;
			if (cursor != NULL) {
				outputNode->param2 = cursor;
				memcpy(cursor, sourceNode->param2, bytes);
				cursor += bytes;
			}
			g_curVertNormals = payload;
			totalBytes += bytes;
			buildState->vertexNormals = payload;
			break;
		}
		case OPT_TEXCOORDS: {
			size_t bytes = sizeof(OptTexCoord) * sourceNode->param1;
			if (cursor != NULL) {
				outputNode->param2 = cursor;
				memcpy(cursor, sourceNode->param2, bytes);
				cursor += bytes;
			}
			totalBytes += bytes;
			break;
		}
		case OPT_FACEDATA:
		case OPT_FACEDATA_15:
		case OPT_FACEDATA_16:
		case OPT_FACEDATA_17: {
			const OptPackedFaceData* faces = payload;
			size_t bytes;
			if (faces->edgeCount > g_sceneEdgeFlagsCapacity)
				g_sceneEdgeFlagsCapacity = faces->edgeCount;
			bytes = (sizeof(OptPackedFaceRecord) + sizeof(OptVector) + sizeof(FaceTextureGradients)) *
						sourceNode->param1 +
					sizeof(faces->edgeCount);
			if (buildState->vertexNormals == NULL)
				bytes += sizeof(OptVector) * g_curVertexCount;
			if (cursor != NULL) {
				outputNode->param2 = cursor;
				memcpy(cursor, sourceNode->param2, bytes);
				cursor += bytes;
			}
			totalBytes += bytes;
			break;
		}
		case OPT_TEXTURE: {
			const OptTextureData* sourceTexture = payload;
			int width;
			int height;
			int pixelCount;
			size_t textureBytes;
			int bytesPerPixel;
			if (cursor != NULL && g_generateMissionPalette != 0 &&
				g_flightBytesPerPixel == FLIGHT_DISPLAY_INDEXED_BYTES_PER_PIXEL) {
				int sourceBytes = sourceTexture->height * sourceTexture->width;
				const uint16_t* shadePalette;
				int shade;
				if (sourceBytes == sourceTexture->textureSize)
					sourceBytes = sourceTexture->dataSize;
				shadePalette = (const uint16_t*)((const uint8_t*)(sourceTexture + 1) + sourceBytes +
												 OPT_TEXTURE_SHADE_PALETTE_COUNT);
				for (shade = 0; shade < MODEL_TEXTURE_SHADE_LEVELS; ++shade)
					ImageQuantizer_ClassifyIndexed16BppImage(
						(uint8_t*)(sourceTexture + 1), &shadePalette[shade * OPT_TEXTURE_PALETTE_COLOR_COUNT],
						sourceTexture->width, sourceTexture->height);
			}
			width = sourceTexture->width;
			height = sourceTexture->height;
			pixelCount = height * width;
			if (pixelCount == sourceTexture->textureSize) {
				if (cursor != NULL) {
					OptTextureData* outputTexture;
					const uint8_t* pixels = (const uint8_t*)(sourceTexture + 1);
					int qualityStep;
					outputNode->param2 = cursor;
					memcpy(cursor, sourceNode->param2, sizeof(OptTextureData));
					outputTexture = outputNode->param2;
					cursor += sizeof(OptTextureData);
					if (g_modelTextureQuality != OPT_TEXTURE_QUALITY_FULL) {
						for (qualityStep = OPT_TEXTURE_QUALITY_FULL - g_modelTextureQuality; qualityStep > 0;
							 --qualityStep) {
							if (outputTexture->width > OPT_TEXTURE_DEFAULT_WIDTH &&
								outputTexture->height > OPT_TEXTURE_DEFAULT_HEIGHT) {
								int skippedPixels = outputTexture->width * outputTexture->height;
								outputTexture->dataSize -= skippedPixels;
								pixels += skippedPixels;
								outputTexture->height >>= 1;
								outputTexture->width >>= 1;
								outputTexture->textureSize = outputTexture->width * outputTexture->height;
							}
						}
					}
					memcpy(cursor, pixels, outputTexture->dataSize);
					cursor += outputTexture->dataSize;
					textureBytes = outputTexture->dataSize + sizeof(OptTextureData);
				} else {
					textureBytes = sourceTexture->dataSize + sizeof(OptTextureData);
					if (g_modelTextureQuality == OPT_TEXTURE_QUALITY_LOW &&
						width > OPT_TEXTURE_DEFAULT_WIDTH && height > OPT_TEXTURE_DEFAULT_HEIGHT)
						textureBytes -= pixelCount;
				}
			} else {
				textureBytes = pixelCount + sizeof(OptTextureData);
				if (cursor != NULL) {
					OptTextureData* outputTexture;
					const uint8_t* pixels = (const uint8_t*)(sourceTexture + 1);
					const uint8_t* paletteBytes;
					const uint16_t* filterPalette;
					outputNode->param2 = cursor;
					memcpy(cursor, sourceNode->param2, textureBytes);
					cursor += textureBytes;
					paletteBytes = sourceTexture->paletteType != 0 ? pixels + pixelCount
																   : (const uint8_t*)sourceTexture->palette;
					filterPalette = (const uint16_t*)(paletteBytes + OPT_TEXTURE_SHADE_PALETTE_COUNT +
													  MODEL_TEXTURE_BASE_SHADE *
														  OPT_TEXTURE_PALETTE_COLOR_COUNT * sizeof(uint16_t));
					outputTexture = outputNode->param2;
					outputTexture->textureSize = width * height;
					outputTexture->dataSize = width * height;
					for (; width > 1 && height > 1;) {
						int mipPixels;
						width >>= 1;
						height >>= 1;
						mipPixels = width * height;
						outputTexture->dataSize += mipPixels;
						if (g_textureMipmapsEnabled != 0) {
							int row;
							for (row = 0; row < height; ++row) {
								int column;
								const uint8_t* upperRow = pixels + row * width * OPT_MIP_FILTER_PIXEL_COUNT;
								const uint8_t* lowerRow = upperRow + width * OPT_MIP_FILTER_WIDTH;
								for (column = 0; column < width; ++column) {
									uint16_t color = filterPalette[upperRow[column * OPT_MIP_FILTER_WIDTH]];
									int blue = color & OPT_RGB565_BLUE_MASK;
									int green;
									int red;
									color >>= OPT_RGB565_GREEN_SHIFT;
									green = color & OPT_RGB565_GREEN_MASK;
									red = (color >> OPT_RGB565_GREEN_BITS) & OPT_RGB565_RED_MASK;
									color = filterPalette[upperRow[column * OPT_MIP_FILTER_WIDTH + 1]];
									blue += color & OPT_RGB565_BLUE_MASK;
									color >>= OPT_RGB565_GREEN_SHIFT;
									green += color & OPT_RGB565_GREEN_MASK;
									red += (color >> OPT_RGB565_GREEN_BITS) & OPT_RGB565_RED_MASK;
									color = filterPalette[lowerRow[column * OPT_MIP_FILTER_WIDTH + 1]];
									blue += color & OPT_RGB565_BLUE_MASK;
									color >>= OPT_RGB565_GREEN_SHIFT;
									green += color & OPT_RGB565_GREEN_MASK;
									red += (color >> OPT_RGB565_GREEN_BITS) & OPT_RGB565_RED_MASK;
									color = filterPalette[lowerRow[column * OPT_MIP_FILTER_WIDTH]];
									blue += color & OPT_RGB565_BLUE_MASK;
									color >>= OPT_RGB565_GREEN_SHIFT;
									green += color & OPT_RGB565_GREEN_MASK;
									red += (color >> OPT_RGB565_GREEN_BITS) & OPT_RGB565_RED_MASK;
									cursor[row * width + column] = OptModel_FindNearestRgb565Index(
										filterPalette, red >> OPT_MIP_FILTER_AVERAGE_SHIFT,
										green >> OPT_MIP_FILTER_AVERAGE_SHIFT,
										blue >> OPT_MIP_FILTER_AVERAGE_SHIFT, 0,
										OPT_TEXTURE_PALETTE_COLOR_COUNT);
								}
							}
						}
						pixels = cursor;
						cursor += mipPixels;
						textureBytes += mipPixels;
					}
				} else {
					for (; width > 1 && height > 1;) {
						height >>= 1;
						width >>= 1;
						textureBytes += width * height;
					}
				}
			}
			sourceTexture = sourceNode->param2;
			bytesPerPixel = g_flightBytesPerPixel;
			if (sourceTexture->paletteType != 0) {
				textureBytes += OPT_TEXTURE_PALETTE_COLOR_COUNT * bytesPerPixel * sourceTexture->paletteType;
				if (cursor != NULL) {
					int sourceBytes = sourceTexture->width * sourceTexture->height;
					const uint8_t* paletteBytes;
					if (sourceBytes == sourceTexture->textureSize)
						sourceBytes = sourceTexture->dataSize;
					paletteBytes = (const uint8_t*)(sourceTexture + 1) + sourceBytes;
					if (bytesPerPixel == FLIGHT_DISPLAY_DIRECT_BYTES_PER_PIXEL)
						paletteBytes += OPT_TEXTURE_PALETTE_COLOR_COUNT * sourceTexture->paletteType;
					if (bytesPerPixel == FLIGHT_DISPLAY_INDEXED_BYTES_PER_PIXEL) {
						const uint16_t* shades =
							(const uint16_t*)(paletteBytes + OPT_TEXTURE_SHADE_PALETTE_COUNT);
						int index;
						for (index = 0; index < OPT_TEXTURE_SHADE_PALETTE_COUNT; ++index)
							cursor[index] = g_rgb565ToPaletteIndexLut[shades[index]];
					} else {
						memcpy(cursor, paletteBytes, bytesPerPixel * OPT_TEXTURE_SHADE_PALETTE_COUNT);
					}
					if (g_flightBytesPerPixel == FLIGHT_DISPLAY_DIRECT_BYTES_PER_PIXEL) {
						if (sourceNode->pName != NULL)
							nullsub_SharedNoOp();
						OptModel_PrepareTexturePalette((uint16_t*)cursor, OPT_TEXTURE_SHADE_PALETTE_COUNT);
					}
					cursor +=
						OPT_TEXTURE_PALETTE_COLOR_COUNT * g_flightBytesPerPixel * sourceTexture->paletteType;
				}
			} else {
				int sourceBytes = sourceTexture->width * sourceTexture->height;
				const uint8_t* paletteBytes;
				if (sourceBytes == sourceTexture->textureSize)
					sourceBytes = sourceTexture->dataSize;
				paletteBytes = (const uint8_t*)(sourceTexture + 1) + sourceBytes;
				if (paletteBytes == (const uint8_t*)sourceTexture->palette) {
					textureBytes += bytesPerPixel * OPT_TEXTURE_SHADE_PALETTE_COUNT;
					if (cursor != NULL) {
						if (bytesPerPixel == FLIGHT_DISPLAY_DIRECT_BYTES_PER_PIXEL)
							paletteBytes += OPT_TEXTURE_SHADE_PALETTE_COUNT;
						if (bytesPerPixel == FLIGHT_DISPLAY_INDEXED_BYTES_PER_PIXEL) {
							const uint16_t* shades =
								(const uint16_t*)(paletteBytes + OPT_TEXTURE_SHADE_PALETTE_COUNT);
							int index;
							for (index = 0; index < OPT_TEXTURE_SHADE_PALETTE_COUNT; ++index)
								cursor[index] = g_rgb565ToPaletteIndexLut[shades[index]];
						} else {
							memcpy(cursor, paletteBytes, bytesPerPixel * OPT_TEXTURE_SHADE_PALETTE_COUNT);
						}
						if (g_flightBytesPerPixel == FLIGHT_DISPLAY_DIRECT_BYTES_PER_PIXEL) {
							if (sourceNode->pName != NULL)
								nullsub_SharedNoOp();
							OptModel_PrepareTexturePalette((uint16_t*)cursor,
														   OPT_TEXTURE_SHADE_PALETTE_COUNT);
						}
						cursor += OPT_TEXTURE_SHADE_PALETTE_COUNT * g_flightBytesPerPixel;
					}
				}
			}
			totalBytes += textureBytes;
			break;
		}
		default:
			break;
	}
#ifdef XW_MODERN
	{
		size_t padding = (sizeof(void*) - totalBytes % sizeof(void*)) % sizeof(void*);
		totalBytes += padding;
		if (cursor != NULL)
			cursor += padding;
	}
#endif
	if (sourceNode->childCount != 0) {
		int childIndex;
		childState = *buildState;
		g_sourceVectors = NULL;
		g_sourceTexCoords = NULL;
		g_curVertNormals = NULL;
		g_optNodeWalkScratch2 = NULL;
		g_curMeshDescriptorData = NULL;
		for (childIndex = 0; childIndex < sourceNode->childCount; ++childIndex) {
			size_t childBytes;
			if (cursor != NULL)
				outputNode->pChildren[childIndex] = (OptNode*)cursor;
			childBytes = OptModel_BuildRuntimeNode(sourceNode->pChildren[childIndex], &childState, cursor);
			if (cursor != NULL) {
				if (childBytes == 0)
					outputNode->pChildren[childIndex] = NULL;
				cursor += childBytes;
			}
			totalBytes += childBytes;
		}
	}
	return totalBytes;
}

// FUNCTION: XW 0x48F580
void OptModel_BuildFaceNormalTangentData(float* dest, const struct OptPackedFaceData* faceData, int faceCount,
										 const struct SceneMesh* conversionState) {
	const OptPackedFaceRecord* records;
	int faceIndex;
	int outputFloatCount = 0;
	float edgeAx, edgeAy, edgeAz, edgeBx, edgeBy, edgeBz;
	float duA, dvA, duB, dvB, determinant;
	float normalLengthSquared;

	if (g_sourceVectors == NULL) {
		return;
	}
	records = faceData->records;
	if (faceCount > 0) {
		for (faceIndex = 0; faceIndex < faceCount; ++faceIndex) {
			const OptPackedFaceRecord* face = &records[faceIndex];
			float* output = &dest[faceIndex * OPT_VECTOR_COMPONENT_COUNT];
			edgeAx = g_sourceVectors[face->vertexIndices[1]].x - g_sourceVectors[face->vertexIndices[0]].x;
			edgeAy = g_sourceVectors[face->vertexIndices[1]].y - g_sourceVectors[face->vertexIndices[0]].y;
			edgeAz = g_sourceVectors[face->vertexIndices[1]].z - g_sourceVectors[face->vertexIndices[0]].z;
			edgeBx = g_sourceVectors[face->vertexIndices[1]].x - g_sourceVectors[face->vertexIndices[2]].x;
			edgeBy = g_sourceVectors[face->vertexIndices[1]].y - g_sourceVectors[face->vertexIndices[2]].y;
			edgeBz = g_sourceVectors[face->vertexIndices[1]].z - g_sourceVectors[face->vertexIndices[2]].z;
			output[0] = edgeBz * edgeAy - edgeBy * edgeAz;
			output[1] = edgeBx * edgeAz - edgeBz * edgeAx;
			output[2] = edgeBy * edgeAx - edgeBx * edgeAy;
			normalLengthSquared = output[1] * output[1] + output[2] * output[2] + output[0] * output[0];
			if (normalLengthSquared == 0.0f) {
				if (face->vertexIndices[3] != OPT_INDEX_NONE) {
					edgeAx =
						g_sourceVectors[face->vertexIndices[3]].x - g_sourceVectors[face->vertexIndices[0]].x;
					edgeAy =
						g_sourceVectors[face->vertexIndices[3]].y - g_sourceVectors[face->vertexIndices[0]].y;
					edgeAz =
						g_sourceVectors[face->vertexIndices[3]].z - g_sourceVectors[face->vertexIndices[0]].z;
					edgeBx =
						g_sourceVectors[face->vertexIndices[3]].x - g_sourceVectors[face->vertexIndices[2]].x;
					edgeBy =
						g_sourceVectors[face->vertexIndices[3]].y - g_sourceVectors[face->vertexIndices[2]].y;
					edgeBz =
						g_sourceVectors[face->vertexIndices[3]].z - g_sourceVectors[face->vertexIndices[2]].z;
					output[0] = edgeBz * edgeAy - edgeBy * edgeAz;
					output[1] = edgeBx * edgeAz - edgeBz * edgeAx;
					output[2] = edgeBy * edgeAx - edgeBx * edgeAy;
					normalLengthSquared =
						output[1] * output[1] + output[2] * output[2] + output[0] * output[0];
					if (normalLengthSquared != 0.0f) {
						const double scale = 1.0 / sqrt(normalLengthSquared);
						output[0] *= scale;
						output[1] *= scale;
						output[2] *= scale;
					}
				}
			} else {
				const double scale = 1.0 / sqrt(normalLengthSquared);
				output[0] *= scale;
				output[1] *= scale;
				output[2] *= scale;
			}
			if (g_optModelInvertFaceNormals != 0) {
				output[0] = -output[0];
				output[1] = -output[1];
				output[2] = -output[2];
			}
		}
	}
	if (faceCount > 0) {
		outputFloatCount = faceCount * OPT_VECTOR_COMPONENT_COUNT;
	}

	records = faceData->records;
	if (g_sourceTexCoords != NULL) {
		int tangentFaceIndex;
		if (faceCount > 0)
			for (tangentFaceIndex = 0; tangentFaceIndex < faceCount; ++tangentFaceIndex) {
				const OptPackedFaceRecord* face = &records[tangentFaceIndex];
				float* output = &dest[outputFloatCount + tangentFaceIndex * OPT_TANGENT_COMPONENT_COUNT];
				float lengthSquared;
				const OptVector* vertices;
				const OptTexCoord* texCoords;
				vertices = g_sourceVectors;
				edgeAx = vertices[face->vertexIndices[0]].x - vertices[face->vertexIndices[1]].x;
				edgeAy = vertices[face->vertexIndices[0]].y - vertices[face->vertexIndices[1]].y;
				edgeAz = vertices[face->vertexIndices[0]].z - vertices[face->vertexIndices[1]].z;
				edgeBx = vertices[face->vertexIndices[0]].x - vertices[face->vertexIndices[2]].x;
				edgeBy = vertices[face->vertexIndices[0]].y - vertices[face->vertexIndices[2]].y;
				edgeBz = vertices[face->vertexIndices[0]].z - vertices[face->vertexIndices[2]].z;
				texCoords = g_sourceTexCoords;
				duA = texCoords[face->texCoordIndices[0]].u - texCoords[face->texCoordIndices[1]].u;
				dvA = texCoords[face->texCoordIndices[0]].v - texCoords[face->texCoordIndices[1]].v;
				duB = texCoords[face->texCoordIndices[0]].u - texCoords[face->texCoordIndices[2]].u;
				dvB = texCoords[face->texCoordIndices[0]].v - texCoords[face->texCoordIndices[2]].v;
				determinant = duB * dvA - duA * dvB;
				output[0] = edgeBx * dvA - dvB * edgeAx;
				output[1] = edgeBy * dvA - dvB * edgeAy;
				output[2] = edgeBz * dvA - dvB * edgeAz;
				lengthSquared = output[1] * output[1] + output[2] * output[2] + output[0] * output[0];
				if (determinant == 0.0f || lengthSquared == 0.0f) {
					if (face->vertexIndices[3] != OPT_INDEX_NONE) {
						vertices = g_sourceVectors;
						edgeAx = vertices[face->vertexIndices[0]].x - vertices[face->vertexIndices[1]].x;
						edgeAy = vertices[face->vertexIndices[0]].y - vertices[face->vertexIndices[1]].y;
						edgeAz = vertices[face->vertexIndices[0]].z - vertices[face->vertexIndices[1]].z;
						edgeBx = vertices[face->vertexIndices[0]].x - vertices[face->vertexIndices[3]].x;
						edgeBy = vertices[face->vertexIndices[0]].y - vertices[face->vertexIndices[3]].y;
						edgeBz = vertices[face->vertexIndices[0]].z - vertices[face->vertexIndices[3]].z;
						texCoords = g_sourceTexCoords;
						duA = texCoords[face->texCoordIndices[0]].u - texCoords[face->texCoordIndices[1]].u;
						dvA = texCoords[face->texCoordIndices[0]].v - texCoords[face->texCoordIndices[1]].v;
						duB = texCoords[face->texCoordIndices[0]].u - texCoords[face->texCoordIndices[3]].u;
						dvB = texCoords[face->texCoordIndices[0]].v - texCoords[face->texCoordIndices[3]].v;
						determinant = duB * dvA - duA * dvB;
						output[0] = edgeBx * dvA - dvB * edgeAx;
						output[1] = edgeBy * dvA - dvB * edgeAy;
						output[2] = edgeBz * dvA - dvB * edgeAz;
						lengthSquared = output[1] * output[1] + output[2] * output[2] + output[0] * output[0];
						if (determinant == 0.0f || lengthSquared == 0.0f) {
							vertices = g_sourceVectors;
							edgeAx = vertices[face->vertexIndices[0]].x - vertices[face->vertexIndices[2]].x;
							edgeAy = vertices[face->vertexIndices[0]].y - vertices[face->vertexIndices[2]].y;
							edgeAz = vertices[face->vertexIndices[0]].z - vertices[face->vertexIndices[2]].z;
							edgeBx = vertices[face->vertexIndices[0]].x - vertices[face->vertexIndices[3]].x;
							edgeBy = vertices[face->vertexIndices[0]].y - vertices[face->vertexIndices[3]].y;
							edgeBz = vertices[face->vertexIndices[0]].z - vertices[face->vertexIndices[3]].z;
							texCoords = g_sourceTexCoords;
							duA =
								texCoords[face->texCoordIndices[0]].u - texCoords[face->texCoordIndices[2]].u;
							dvA =
								texCoords[face->texCoordIndices[0]].v - texCoords[face->texCoordIndices[2]].v;
							duB =
								texCoords[face->texCoordIndices[0]].u - texCoords[face->texCoordIndices[3]].u;
							dvB =
								texCoords[face->texCoordIndices[0]].v - texCoords[face->texCoordIndices[3]].v;
							determinant = duB * dvA - duA * dvB;
							output[0] = edgeBx * dvA - dvB * edgeAx;
							output[1] = edgeBy * dvA - dvB * edgeAy;
							output[2] = edgeBz * dvA - dvB * edgeAz;
							lengthSquared =
								output[1] * output[1] + output[2] * output[2] + output[0] * output[0];
							if (determinant == 0.0f || lengthSquared == 0.0f) {
								vertices = g_sourceVectors;
								edgeAx =
									vertices[face->vertexIndices[1]].x - vertices[face->vertexIndices[2]].x;
								edgeAy =
									vertices[face->vertexIndices[1]].y - vertices[face->vertexIndices[2]].y;
								edgeAz =
									vertices[face->vertexIndices[1]].z - vertices[face->vertexIndices[2]].z;
								edgeBx =
									vertices[face->vertexIndices[1]].x - vertices[face->vertexIndices[3]].x;
								edgeBy =
									vertices[face->vertexIndices[1]].y - vertices[face->vertexIndices[3]].y;
								edgeBz =
									vertices[face->vertexIndices[1]].z - vertices[face->vertexIndices[3]].z;
								texCoords = g_sourceTexCoords;
								duA = texCoords[face->texCoordIndices[1]].u -
									  texCoords[face->texCoordIndices[2]].u;
								dvA = texCoords[face->texCoordIndices[1]].v -
									  texCoords[face->texCoordIndices[2]].v;
								duB = texCoords[face->texCoordIndices[1]].u -
									  texCoords[face->texCoordIndices[3]].u;
								dvB = texCoords[face->texCoordIndices[1]].v -
									  texCoords[face->texCoordIndices[3]].v;
								determinant = duB * dvA - duA * dvB;
								output[0] = edgeBx * dvA - dvB * edgeAx;
								output[1] = edgeBy * dvA - dvB * edgeAy;
								output[2] = edgeBz * dvA - dvB * edgeAz;
								lengthSquared =
									output[1] * output[1] + output[2] * output[2] + output[0] * output[0];
								if (determinant == 0.0f || lengthSquared == 0.0f) {
									output[0] = edgeBx;
									output[1] = edgeBy;
									output[2] = edgeBz;
									determinant = 1.0f;
									duA = 0.0f;
									duB = 1.0f;
								}
							}
						}
					} else {
						output[0] = edgeBx;
						output[1] = edgeBy;
						output[2] = edgeBz;
						determinant = 1.0f;
						duA = 0.0f;
						duB = 1.0f;
					}
				}
				{
					float reciprocal = 1.0f / determinant;
					output[0] *= reciprocal;
					output[1] *= reciprocal;
					output[2] *= reciprocal;
					reciprocal = -reciprocal;
					output[3] = (duA * edgeBx - duB * edgeAx) * reciprocal;
					output[4] = (duA * edgeBy - duB * edgeAy) * reciprocal;
					output[5] = (duA * edgeBz - duB * edgeAz) * reciprocal;
				}
				if (duA == 0.0f) {
					if (duB == 0.0f) {
						output[4] = 1.0f;
					}
				}
			}
		if (faceCount > 0) {
			outputFloatCount += OPT_TANGENT_COMPONENT_COUNT * faceCount;
		}
	} else {
		/* The original skips 96 bytes per face when UV data is absent. */
		outputFloatCount += OPT_NO_UV_SKIP_FLOAT_COUNT * faceCount;
	}

	if (conversionState->vertexNormals != NULL) {
		g_generatedVertexNormalCount = 0;
	} else {
		g_generatedVertexNormalCount = g_curVertexCount;
		OptModel_BuildVertexNormalsFromFaces(&dest[outputFloatCount], faceData, faceCount);
	}
}

// FUNCTION: XW 0x490010
void OptModel_BuildVertexNormalsFromFaces(float* dest, const struct OptPackedFaceData* faceData,
										  int faceCount) {
	int vertexIndex;
	for (vertexIndex = 0; vertexIndex < g_curVertexCount; ++vertexIndex) {
		const OptPackedFaceRecord* records = faceData->records;
		const float* faceNormals = (const float*)&records[faceCount];
		float* vertexNormal = &dest[vertexIndex * OPT_VECTOR_COMPONENT_COUNT];
		int incidentFaceCount = 0;
		int faceIndex;
		for (faceIndex = faceCount; faceIndex > 0; --faceIndex) {
			const OptPackedFaceRecord* face = &records[faceCount - faceIndex];
			if (face->vertexIndices[0] == vertexIndex || face->vertexIndices[1] == vertexIndex ||
				face->vertexIndices[2] == vertexIndex || face->vertexIndices[3] == vertexIndex) {
				++incidentFaceCount;
				if (incidentFaceCount == 1) {
					vertexNormal[0] = faceNormals[(faceCount - faceIndex) * OPT_VECTOR_COMPONENT_COUNT];
					vertexNormal[1] = faceNormals[(faceCount - faceIndex) * OPT_VECTOR_COMPONENT_COUNT + 1];
					vertexNormal[2] = faceNormals[(faceCount - faceIndex) * OPT_VECTOR_COMPONENT_COUNT + 2];
				} else {
					vertexNormal[0] =
						faceNormals[(faceCount - faceIndex) * OPT_VECTOR_COMPONENT_COUNT] + vertexNormal[0];
					vertexNormal[1] = faceNormals[(faceCount - faceIndex) * OPT_VECTOR_COMPONENT_COUNT + 1] +
									  vertexNormal[1];
					vertexNormal[2] = faceNormals[(faceCount - faceIndex) * OPT_VECTOR_COMPONENT_COUNT + 2] +
									  vertexNormal[2];
				}
			}
		}
		if (incidentFaceCount > 1) {
			float inverseFaceCount;
			if (incidentFaceCount < OPT_NORMAL_RECIPROCAL_LIMIT) {
				inverseFaceCount = g_sw3dSpanLengthReciprocal[incidentFaceCount];
			} else {
				inverseFaceCount = 1.0f / incidentFaceCount;
			}
			vertexNormal[0] *= inverseFaceCount;
			vertexNormal[1] *= inverseFaceCount;
			vertexNormal[2] *= inverseFaceCount;
		}
	}
}

// FUNCTION: XW 0x490110
struct OptNode* OptModel_ResolveNodeRef(const struct OptimizedPolyObject* model, const char* name) {
	int rootIndex;
	OptNode* match;
	for (rootIndex = 0; rootIndex < model->rootNodeCount; ++rootIndex) {
		match = OptModel_FindNodeByName(model->rootNodes[rootIndex], name);
		if (match != NULL) {
			return match;
		}
	}
	return NULL;
}

// FUNCTION: XW 0x490150
struct OptNode* OptModel_FindNodeByName(struct OptNode* node, const char* name) {
	int childIndex;
	OptNode* match;
	if (node == NULL) {
		return NULL;
	}
	if (node->pName != NULL && XwStrCaseCmp(node->pName, name) == 0) {
		return node;
	}
	for (childIndex = 0; childIndex < node->childCount; ++childIndex) {
		match = OptModel_FindNodeByName(node->pChildren[childIndex], name);
		if (match != NULL) {
			return match;
		}
	}
	return NULL;
}

// FUNCTION: XW 0x4901B0
size_t OptModel_LoadRgbOrTexFile(uint8_t* dst, const char* fileName) {
	char resolvedFileName[OPT_TEXTURE_PATH_CAPACITY];
	char* extension;
	XwFile* textureStream = NULL;
	XwFile* rgbStream = NULL;
	OptTextureFileHeader* textureHeader = (OptTextureFileHeader*)dst;
	uint8_t* pixels;
	strcpy(resolvedFileName, fileName);
	extension = resolvedFileName + strlen(resolvedFileName) - OPT_TEXTURE_EXTENSION_LENGTH;
	if (XwStrCaseCmp(extension, g_extRgb) == 0) {
		extension[0] = 't';
		extension[1] = 'e';
		extension[2] = 'x';
		fediskio_tryopenfile(resolvedFileName, "rb", 0);
		textureStream = g_stream;
		extension[0] = 'r';
		extension[1] = 'g';
		extension[2] = 'b';
		if (textureStream == NULL) {
			fediskio_tryopenfile(resolvedFileName, "rb", 0);
			rgbStream = g_stream;
			if (rgbStream != NULL) {
				RgbTriplet palette[OPT_TEXTURE_PALETTE_COLOR_COUNT];
				RgbTriplet target;
				const OptRgbSizeHeader* rgbHeader = (const OptRgbSizeHeader*)dst;
				int width, height, pixelCount;
				int pixelIndex, colorIndex, shadeLevel;
				int colorsRemaining;
				int uniqueColorCount;
				uint8_t* indexedShades;
				uint8_t* hardwareShades;
#ifdef XW_MODERN
				/* Unused palette entries were unspecified stack bytes in the original. */
				memset(palette, 0, sizeof(palette));
#endif
#ifdef XW_MODERN
				XwRenderAssets_RegisterStream(XW_SOURCE_TEXTURE, 0, rgbStream, XwStorage_LastPath());
#endif
				File_RawRead(dst, OPT_RGB_FILE_HEADER_SIZE, 1, rgbStream);
				width = (rgbHeader->width[0] << OPT_TEXTURE_BYTE_BITS) + rgbHeader->width[1];
				height = (rgbHeader->height[0] << OPT_TEXTURE_BYTE_BITS) + rgbHeader->height[1];
				textureHeader->width = width;
				textureHeader->height = height;
#ifdef XW_MODERN
				pixelCount = (int32_t)((uint32_t)width * (uint32_t)height);
#else
				pixelCount = width * height;
#endif
				pixels = dst + sizeof(*textureHeader);
				File_RawRead(pixels, pixelCount, OPT_RGB_CHANNEL_COUNT, rgbStream);
				File_RawClose(rgbStream);
				uniqueColorCount = 1;
				palette[0].r = pixels[0] >> MODEL_TEXTURE_SOURCE_CHANNEL_SHIFT;
				palette[0].g = pixels[pixelCount] >> MODEL_TEXTURE_SOURCE_CHANNEL_SHIFT;
				palette[0].b = pixels[OPT_RGB_BLUE_PLANE * pixelCount] >> MODEL_TEXTURE_SOURCE_CHANNEL_SHIFT;
				pixels[0] = 0;
				for (pixelIndex = 1; pixelIndex < pixelCount; ++pixelIndex) {
					uint8_t nearest;
					target.r = pixels[pixelIndex] >> MODEL_TEXTURE_SOURCE_CHANNEL_SHIFT;
					target.g = pixels[pixelCount + pixelIndex] >> MODEL_TEXTURE_SOURCE_CHANNEL_SHIFT;
					target.b = pixels[OPT_RGB_BLUE_PLANE * pixelCount + pixelIndex] >>
							   MODEL_TEXTURE_SOURCE_CHANNEL_SHIFT;
					nearest = rtsvga2_FindNearestRgbTripletIndex(&target, palette, 0, uniqueColorCount);
					if ((palette[nearest].r != target.r || palette[nearest].g != target.g ||
						 palette[nearest].b != target.b) &&
						uniqueColorCount < OPT_TEXTURE_PALETTE_COLOR_COUNT) {
						nearest = uniqueColorCount++;
						palette[nearest].r = target.r;
						palette[nearest].g = target.g;
						palette[nearest].b = target.b;
					}
					pixels[pixelIndex] = nearest;
				}
				indexedShades = pixels + pixelCount;
				hardwareShades = indexedShades + sizeof(((OptShadeTable*)0)->indexedShades);
				textureHeader->paletteType = MODEL_TEXTURE_SHADE_LEVELS;
				textureHeader->palette = OPT_TEXTURE_PALETTE_COLOR_COUNT;
				for (colorIndex = 0, colorsRemaining = OPT_TEXTURE_PALETTE_COLOR_COUNT; colorsRemaining != 0;
					 ++colorIndex, --colorsRemaining) {
					for (shadeLevel = 0; shadeLevel < MODEL_TEXTURE_SHADE_LEVELS; ++shadeLevel) {
						uint16_t packed;
						int shadeIndex = shadeLevel * OPT_TEXTURE_PALETTE_COLOR_COUNT + colorIndex;
						if (shadeLevel < MODEL_TEXTURE_BASE_SHADE) {
							target.r =
								(uint16_t)((palette[colorIndex].r << (MODEL_TEXTURE_FRACTION_BITS - 1)) +
										   shadeLevel *
											   (palette[colorIndex].r << MODEL_TEXTURE_FRACTION_BITS) /
											   MODEL_TEXTURE_SHADE_LEVELS) >>
								MODEL_TEXTURE_FRACTION_BITS;
							target.g =
								(uint16_t)((palette[colorIndex].g << (MODEL_TEXTURE_FRACTION_BITS - 1)) +
										   shadeLevel *
											   (palette[colorIndex].g << MODEL_TEXTURE_FRACTION_BITS) /
											   MODEL_TEXTURE_SHADE_LEVELS) >>
								MODEL_TEXTURE_FRACTION_BITS;
							target.b =
								(uint16_t)((palette[colorIndex].b << (MODEL_TEXTURE_FRACTION_BITS - 1)) +
										   shadeLevel *
											   (palette[colorIndex].b << MODEL_TEXTURE_FRACTION_BITS) /
											   MODEL_TEXTURE_SHADE_LEVELS) >>
								MODEL_TEXTURE_FRACTION_BITS;
						} else {
							target.r = (uint16_t)((palette[colorIndex].r << MODEL_TEXTURE_FRACTION_BITS) +
												  (((shadeLevel - MODEL_TEXTURE_BASE_SHADE) *
													(MODEL_TEXTURE_CHANNEL_MASK - palette[colorIndex].r))
												   << MODEL_TEXTURE_FRACTION_BITS) /
													  MODEL_TEXTURE_BASE_SHADE) >>
									   MODEL_TEXTURE_FRACTION_BITS;
							target.g = (uint16_t)((palette[colorIndex].g << MODEL_TEXTURE_FRACTION_BITS) +
												  (((shadeLevel - MODEL_TEXTURE_BASE_SHADE) *
													(MODEL_TEXTURE_CHANNEL_MASK - palette[colorIndex].g))
												   << MODEL_TEXTURE_FRACTION_BITS) /
													  MODEL_TEXTURE_BASE_SHADE) >>
									   MODEL_TEXTURE_FRACTION_BITS;
							target.b = (uint16_t)((palette[colorIndex].b << MODEL_TEXTURE_FRACTION_BITS) +
												  (((shadeLevel - MODEL_TEXTURE_BASE_SHADE) *
													(MODEL_TEXTURE_CHANNEL_MASK - palette[colorIndex].b))
												   << MODEL_TEXTURE_FRACTION_BITS) /
													  MODEL_TEXTURE_BASE_SHADE) >>
									   MODEL_TEXTURE_FRACTION_BITS;
						}
						packed =
							target.b +
							((target.g + (target.r << (MODEL_TEXTURE_RED_SHIFT - MODEL_TEXTURE_GREEN_SHIFT)))
							 << MODEL_TEXTURE_GREEN_SHIFT);
#ifdef XW_MODERN
						hardwareShades[shadeIndex * sizeof(packed)] = (uint8_t)packed;
						hardwareShades[shadeIndex * sizeof(packed) + 1] = (uint8_t)(packed >> CHAR_BIT);
#else
						((uint16_t*)hardwareShades)[shadeIndex] = packed;
#endif
						target.r *= 2;
						target.g *= 2;
						target.b *= 2;
						indexedShades[shadeIndex] = rtsvga2_FindNearestRgbTripletIndex(
							&target, g_swPalette, MODEL_TEXTURE_FIRST_SW_COLOR,
							OPT_TEXTURE_PALETTE_COLOR_COUNT);
					}
				}
				return sizeof(*textureHeader) + pixelCount + sizeof(OptShadeTable);
			}
		}
	} else if (XwStrCaseCmp(extension, "tex") == 0) {
		fediskio_tryopenfile(resolvedFileName, "rb", 0);
		textureStream = g_stream;
	}

	if (textureStream == NULL) {
		textureHeader->width = OPT_TEXTURE_DEFAULT_WIDTH;
		textureHeader->height = OPT_TEXTURE_DEFAULT_HEIGHT;
		textureHeader->paletteType = MODEL_TEXTURE_SHADE_LEVELS;
		textureHeader->palette = OPT_TEXTURE_PALETTE_COLOR_COUNT;
		pixels = dst + sizeof(*textureHeader);
		ModelTexture_BuildPalettedShadeTable(pixels, g_defaultWhiteTextureRgb24, OPT_TEXTURE_DEFAULT_WIDTH,
											 OPT_TEXTURE_DEFAULT_HEIGHT);
		return sizeof(*textureHeader) + OPT_TEXTURE_DEFAULT_WIDTH * OPT_TEXTURE_DEFAULT_HEIGHT +
			   sizeof(OptShadeTable);
	} else {
		size_t payloadSize;
		pixels = dst + sizeof(*textureHeader);
#ifdef XW_MODERN
		XwRenderAssets_RegisterStream(XW_SOURCE_TEXTURE, 0, textureStream, XwStorage_LastPath());
#endif
		File_RawRead(dst, sizeof(*textureHeader), 1, textureStream);
		payloadSize = (uint32_t)textureHeader->width * (uint32_t)textureHeader->height;
		if (payloadSize == (uint32_t)textureHeader->textureSize)
			payloadSize = textureHeader->dataSize;
		textureHeader->palette = OPT_TEXTURE_PALETTE_COLOR_COUNT;
		textureHeader->paletteType = MODEL_TEXTURE_SHADE_LEVELS;
		File_RawRead(pixels, payloadSize, 1, textureStream);
		File_RawRead(pixels + payloadSize + sizeof(((OptShadeTable*)0)->indexedShades),
					 sizeof(((OptShadeTable*)0)->packedShades), 1, textureStream);
		File_RawClose(textureStream);
		return sizeof(*textureHeader) + payloadSize + sizeof(OptShadeTable);
	}
}

// FUNCTION: XW 0x4906C0
int OptModel_GetExternalTextureSerializedSize(const char* sourceFileName) {
	OptTextureFileHeader textureHeader;
	OptRgbSizeHeader rgbHeader;
	char resolvedFileName[OPT_TEXTURE_PATH_CAPACITY];
	char* extension;
	XwFile* textureStream;
	uint32_t payloadSize;
	strcpy(resolvedFileName, sourceFileName);
	extension = resolvedFileName + strlen(resolvedFileName) - OPT_TEXTURE_EXTENSION_LENGTH;
	if (XwStrCaseCmp(extension, "rgb") == 0) {
		extension[0] = 't';
		extension[1] = 'e';
		extension[2] = 'x';
		fediskio_tryopenfile(resolvedFileName, "rb", 0);
		textureStream = g_stream;
		extension[0] = 'r';
		extension[1] = 'g';
		extension[2] = 'b';
		if (textureStream == NULL) {
			XwFile* rgbStream;
			fediskio_tryopenfile(resolvedFileName, "rb", 0);
			rgbStream = g_stream;
			if (rgbStream != NULL) {
				uint32_t width, height;
				int sizeBudget;
				File_RawRead(&rgbHeader, sizeof(rgbHeader), 1, rgbStream);
				width = (rgbHeader.width[0] << OPT_TEXTURE_BYTE_BITS) + rgbHeader.width[1];
				height = (rgbHeader.height[0] << OPT_TEXTURE_BYTE_BITS) + rgbHeader.height[1];
				sizeBudget =
					(int32_t)(OPT_RGB_CHANNEL_COUNT * (width * height + OPT_TEXTURE_PALETTE_COLOR_COUNT) +
							  sizeof(OptShadeTable));
				File_RawClose(rgbStream);
				return sizeBudget;
			}
			return sizeof(OptTextureFileHeader) + sizeof(OptShadeTable) +
				   OPT_TEXTURE_DEFAULT_WIDTH * OPT_TEXTURE_DEFAULT_HEIGHT;
		}
	} else {
		if (XwStrCaseCmp(extension, "tex") != 0) {
			return sizeof(OptTextureFileHeader) + sizeof(OptShadeTable) +
				   OPT_TEXTURE_DEFAULT_WIDTH * OPT_TEXTURE_DEFAULT_HEIGHT;
		}
		fediskio_tryopenfile(resolvedFileName, "rb", 0);
		textureStream = g_stream;
	}
	if (textureStream == NULL) {
		return sizeof(OptTextureFileHeader) + sizeof(OptShadeTable) +
			   OPT_TEXTURE_DEFAULT_WIDTH * OPT_TEXTURE_DEFAULT_HEIGHT;
	}
	File_RawRead(&textureHeader, sizeof(textureHeader), 1, textureStream);
	File_RawClose(textureStream);
	payloadSize = (uint32_t)textureHeader.width * (uint32_t)textureHeader.height;
	if (payloadSize == (uint32_t)textureHeader.textureSize) {
		payloadSize = textureHeader.dataSize;
	}
	return (int32_t)(payloadSize + sizeof(OptTextureFileHeader) + sizeof(OptShadeTable));
}

// FUNCTION: XW 0x4A0B80
uint16_t OptModel_LoadHandle(const char* path) {
	char fileName[OPT_MODEL_PATH_CAPACITY];
	uint16_t fileHandle;
	if (path == NULL || strlen(path) >= sizeof(fileName))
		return 0;
	strcpy(fileName, path);
	fileHandle = OptModel_LoadFileToHandle(fileName);
	if (fileHandle == 0)
		return 0;
	return OptModel_CreateRuntimeHandle(fileHandle);
}
