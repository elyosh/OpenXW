#ifndef XW_ASSETS_OPT_MODEL_H
#define XW_ASSETS_OPT_MODEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

enum {
	OPT_RGB565_BLUE_MASK = 0x1F,
	OPT_RGB565_GREEN_MASK = 0x3F,
	OPT_RGB565_RED_MASK = 0x1F,
	OPT_RGB565_GREEN_SHIFT = 5,
	OPT_RGB565_GREEN_BITS = 6
};

enum { OPT_INDEX_NONE = -1, OPT_MODEL_PATH_CAPACITY = 257 };

enum {
	OPT_CONVERSION_CAPACITY_MULTIPLIER = 2,
	OPT_NODE_GROUP = 0,
	OPT_SHARED_VERTEX_CHILD = 0,
	OPT_SHARED_TEXCOORD_CHILD = 1,
	OPT_SHARED_NORMAL_CHILD = 2,
	OPT_SHARED_GEOMETRY_CHILD_COUNT = 3,
	OPT_BOUNDS_VERTEX_COUNT = 2
};

enum { OPT_POLYGON_TRIANGLE_VERTICES = 3 };

enum { OPT_FILE_VERSION_0 = 1 };

enum {
	OPT_VECTOR_COMPONENT_COUNT = 3,
	OPT_TANGENT_COMPONENT_COUNT = 6,
	OPT_NO_UV_SKIP_FLOAT_COUNT = 24,
	OPT_NORMAL_RECIPROCAL_LIMIT = 65
};

enum {
	OPT_TEXTURE_QUALITY_LOW = 0,
	OPT_TEXTURE_QUALITY_FULL = 2,
	OPT_MIP_FILTER_WIDTH = 2,
	OPT_MIP_FILTER_PIXEL_COUNT = 4,
	OPT_MIP_FILTER_AVERAGE_SHIFT = 2,
	OPT_TEXTURE_PATH_CAPACITY = 256,
	OPT_TEXTURE_EXTENSION_LENGTH = 3,
	OPT_TEXTURE_DEFAULT_WIDTH = 8,
	OPT_TEXTURE_DEFAULT_HEIGHT = 8,
	OPT_TEXTURE_PALETTE_COLOR_COUNT = 256,
	OPT_TEXTURE_SHADE_PALETTE_COUNT = 4096,
	OPT_RGB_CHANNEL_COUNT = 3,
	OPT_TEXTURE_BYTE_BITS = 8,
	OPT_RGB_FILE_HEADER_SIZE = 512,
	OPT_RGB_GREEN_PLANE = 1,
	OPT_RGB_BLUE_PLANE = 2
};

#pragma pack(push, 1)

typedef struct OptModelFileHeader {
	uint32_t selfMarker;
	uint16_t reserved;
	int32_t rootNodeCount;
	uint32_t rootsOffset;
} OptModelFileHeader;

/* Serialized node offsets stay four bytes on native hosts. */
typedef struct OptNodeFileHeader {
	uint32_t nameOffset;
	int32_t nodeType;
	int32_t childCount;
	uint32_t childrenOffset;
	int32_t param1;
	uint32_t dataOffset;
} OptNodeFileHeader;

/* Serialized TEX header; palette holds the original four-byte palette value. */
typedef struct OptTextureFileHeader {
	uint32_t palette;
	int32_t paletteType;
	int32_t textureSize;
	int32_t dataSize;
	int32_t width;
	int32_t height;
} OptTextureFileHeader;

/* The prefix read to obtain big-endian dimensions from an RGB file. */
typedef struct OptRgbSizeHeader {
	uint8_t prefix[6];
	uint8_t width[2];
	uint8_t height[2];
	uint8_t suffix[6];
} OptRgbSizeHeader;

#pragma pack(pop)

typedef char xw_size_OptModelFileHeader[(sizeof(OptModelFileHeader) == 14) ? 1 : -1];
typedef char
	xw_offset_OptModelFileHeader_rootsOffset[(offsetof(OptModelFileHeader, rootsOffset) == 10) ? 1 : -1];

typedef char xw_size_OptNodeFileHeader[(sizeof(OptNodeFileHeader) == 24) ? 1 : -1];
typedef char xw_offset_OptNodeFileHeader_dataOffset[(offsetof(OptNodeFileHeader, dataOffset) == 20) ? 1 : -1];

typedef char xw_size_OptTextureFileHeader[(sizeof(OptTextureFileHeader) == 24) ? 1 : -1];
typedef char xw_offset_OptTextureFileHeader_width[(offsetof(OptTextureFileHeader, width) == 16) ? 1 : -1];
typedef char xw_size_OptRgbSizeHeader[(sizeof(OptRgbSizeHeader) == 16) ? 1 : -1];
typedef char xw_offset_OptRgbSizeHeader_width[(offsetof(OptRgbSizeHeader, width) == 6) ? 1 : -1];
typedef char xw_offset_OptRgbSizeHeader_height[(offsetof(OptRgbSizeHeader, height) == 8) ? 1 : -1];

struct SceneMesh;
typedef struct OptNode OptNode;
typedef struct OptPackedFaceData OptPackedFaceData;
typedef struct OptPackedFaceRecord OptPackedFaceRecord;
typedef struct OptShadeTable OptShadeTable;
typedef struct OptTexCoord OptTexCoord;
typedef struct OptTextureData OptTextureData;
typedef struct OptVector OptVector;
typedef struct OptimizedPolyObject OptimizedPolyObject;
typedef struct XwOptThreeFaceData XwOptThreeFaceData;

typedef int32_t OptNodeType;

/* Payload semantics for the numbered node types remain unclassified. */
enum {
	OPT_NODE_2_23_PAYLOAD_SIZE = 48,
	OPT_NODE_5_PAYLOAD_SIZE = 36,
	OPT_NODE_9_RECORD_SIZE = 56,
	OPT_NODE_22_PAYLOAD_SIZE = 16
};

enum OptNodeTypeValues {
	OPT_NODE_TYPE_2 = 2,
	OPT_NODE_TYPE_4 = 4,
	OPT_NODE_TYPE_5 = 5,
	OPT_NODE_TYPE_6 = 6,
	OPT_NODE_TYPE_9 = 9,
	OPT_NODE_TYPE_10 = 10,
	OPT_NODE_TYPE_19 = 19,
	OPT_NODE_TYPE_21 = 21,
	OPT_NODE_TYPE_22 = 22,
	OPT_NODE_TYPE_23 = 23,
	OPT_NODE_TYPE_24 = 24,
	OPT_MESHVERTS = 0x3,
	OPT_TEXTURE = 0x14,
	OPT_MESHDESCRIPTOR = 0x19,
	OPT_NODEREF = 0x7,
	OPT_VERTNORMALS = 0xB,
	OPT_TEXCOORDS = 0xD,
	OPT_FACEDATA = 0x1,
	OPT_FACEDATA_15 = 0xF,
	OPT_FACEDATA_16 = 0x10,
	OPT_FACEDATA_17 = 0x11
};

/* Original IDB size: 64 bytes. */
struct OptPackedFaceRecord {
	/* IDB +0x0: Four vertex indices; fourth is -1 for a triangle. */
	int vertexIndices[4];
	/* IDB +0x10 */
	int edgeIndices[4];
	/* IDB +0x20 */
	int texCoordIndices[4];
	/* IDB +0x30 */
	int normalIndices[4];
};

/* Original IDB size: 12288 bytes. */
struct OptShadeTable {
	/* IDB +0x0: 16 shade levels x 256 texel indices; output is an 8-bit palette index. */
	uint8_t indexedShades[4096];
	/* IDB +0x1000: 16 shade levels x 256 texel indices; output is a packed 16-bit color. Texture
	 * deduplication compares these 0x2000 bytes. */
	uint16_t packedShades[4096];
};

/* Original IDB size: 8 bytes. */
struct OptTexCoord {
	/* IDB +0x0 */
	float u;
	/* IDB +0x4 */
	float v;
};

/* Original IDB size: 24 bytes. */
struct OptTextureData {
	/* IDB +0x0: Runtime palette pointer. Serialized/build-time form may hold the palette entry count (256)
	 * until fixup. */
	uint16_t* palette;
	/* IDB +0x4: Nonzero serialized/build-time marker (normally 16 shade levels); runtime fixup clears this to
	 * zero. */
	int paletteType;
	/* IDB +0x8: Compared with width*height to select dataSize rather than the base pixel count. */
	int textureSize;
	/* IDB +0xC: Stored pixel payload size when textureSize matches width*height. */
	int dataSize;
	/* IDB +0x10 */
	int width;
	/* IDB +0x14 */
	int height;
};

/* Original IDB size: 12 bytes. */
struct OptVector {
	/* IDB +0x0 */
	float x;
	/* IDB +0x4 */
	float y;
	/* IDB +0x8 */
	float z;
};

/* Original IDB size: 14 bytes. */
struct OptimizedPolyObject {
	/* IDB +0x0: Current base marker used to compute the relocation delta. */
	void* selfMarker;
	/* IDB +0x4: Two bytes preserved when copying the 14-byte header; semantics not established. */
	uint16_t reserved;
	/* IDB +0x6 */
	int rootNodeCount;
	/* IDB +0xA */
	struct OptNode** rootNodes;
};

/* Original IDB size: 24 bytes. */
struct OptNode {
	/* IDB +0x0 */
	char* pName;
	/* IDB +0x4 */
	OptNodeType nodeType;
	/* IDB +0x8 */
	int childCount;
	/* IDB +0xC */
	struct OptNode** pChildren;
	/* IDB +0x10: Type-dependent count or scalar; type 3 uses a vector count. */
	int param1;
	/* IDB +0x14: Type-dependent payload; type 25 points to a 72-byte MeshDescriptor. */
	void* param2;
};

/* Original IDB size: 68 bytes. */
struct OptPackedFaceData {
	/* IDB +0x0 */
	int edgeCount;
	/* IDB +0x4: Variable-length inline array: enclosing face node supplies faceCount. Face normals follow the
	 * records. */
	struct OptPackedFaceRecord records[1];
};

/* Original IDB size: 196 bytes. */
struct XwOptThreeFaceData {
	/* IDB +0x0: Edge count for the packed face block; this trench block has 12 edges. */
	int edgeCount;
	/* IDB +0x4: Three 64-byte faces: vertex, edge, UV and normal index arrays. Face count is supplied
	 * separately to consumers. */
	struct OptPackedFaceRecord records[3];
};

/* Declarations follow ascending original IDB address. */

extern int g_cacheResolvedOptNodeRefs;
extern uint8_t g_defaultWhiteTextureRgb24[OPT_TEXTURE_DEFAULT_WIDTH * OPT_TEXTURE_DEFAULT_HEIGHT *
										  OPT_RGB_CHANNEL_COUNT];
extern char g_extRgb[OPT_TEXTURE_EXTENSION_LENGTH + 1];
extern int g_generatedVertexNormalCount;
extern int g_optModelInvertFaceNormals;
extern OptVector* g_sourceVectors;
extern OptNode* g_optConvertCurrentMesh;
extern OptNode* g_optConvertVertexNormalNode;
extern void* g_curMeshDescriptorData;
extern OptNode* g_optConvertSourceTextureNode;
extern int g_optConvertVectorSearchCursor;
extern OptTexCoord* g_sourceTexCoords;
extern void* g_optNodeWalkScratch2;
extern OptNode* g_optConvertTexCoordNode;
extern OptNode* g_optConvertFaceTextureNode;
extern int g_optConvertTexCoordSearchCursor;
extern OptNode* g_optConvertVertexNode;
extern int g_curVertexCount;
extern OptVector* g_curVertNormals;
extern uint16_t g_loadOptBufHandle;
extern unsigned int g_loadOptBufferCapacityBytes;
extern int g_optSourceIsVersion0;
extern uint16_t g_optConversionSourceHandle;
extern unsigned int g_optConversionSourceCapacityBytes;
extern int g_optConvertTargetFaceFound;

/* 0x488360 */
void OptModel_AdjustOptimizedPolyObjectPointers(struct OptimizedPolyObject* model);

/* 0x4883C0 */
void OptModel_AdjustOptimizedNodePointers(struct OptNode* node, ptrdiff_t baseDelta);

/* 0x4886E0 */
void OptModel_SetSourceVertices(struct OptVector* vertices);

/* 0x4886F0 */
void OptModel_SetSourceTexCoords(struct OptTexCoord* texCoords);

/* 0x48AC40 */
int16_t OptModel_LoadFileToHandle(char* fileName);

/* 0x48AED0 */
int OptModel_ConvertLegacyModelToOptimized(int sourceBytes);

/* 0x48B0C0 */
struct OptShadeTable* OptModel_FindSharedTextureDataInNodeBeforeTarget(
	const struct OptShadeTable* textureData, struct OptNode* node, const struct OptNode* stopNode);

/* 0x48B190 */
struct OptShadeTable* OptModel_FindEarlierSharedTextureData(const struct OptShadeTable* textureData,
															struct OptimizedPolyObject* model,
															const struct OptNode* stopNode);

/* 0x48B1E0 */
size_t OptModel_ConvertLegacyNodeToOptimized(uint8_t* destination, struct OptNode* sourceNode,
											 struct OptimizedPolyObject* sourceModel,
											 struct OptimizedPolyObject* destinationModel,
											 struct SceneMesh* conversionState);

/* 0x48BE30 */
void OptModel_CollectUniqueVertices(struct OptNode* dstVertexNode, struct OptNode* srcNode,
									struct OptimizedPolyObject* srcModel, void* meshState);

/* 0x48BF30 */
void OptModel_CollectUniqueTexCoords(struct OptNode* dstTexCoordNode, struct OptNode* srcNode,
									 struct OptimizedPolyObject* srcModel, void* meshState);

/* 0x48C010 */
void OptModel_CollectUniqueVertexNormals(struct OptNode* dstNormalNode, struct OptNode* srcNode,
										 struct OptimizedPolyObject* srcModel, struct SceneMesh* meshState);

/* 0x48C240 */
int OptModel_RemapVectorIndex(const struct OptNode* uniqueVectorNode, const struct OptVector* sourceVectors,
							  int sourceIndex);

/* 0x48C320 */
int OptModel_RemapTexCoordIndex(const struct OptNode* uniqueTexCoordNode,
								const struct OptTexCoord* sourceTexCoords, int sourceIndex);

/* 0x48C3F0 */
void OptModel_AppendConvertedFacesForNode(struct OptNode* destinationFaceNode,
										  struct OptNode* firstSourceFaceNode, struct OptNode* node,
										  struct OptimizedPolyObject* sourceModel,
										  struct SceneMesh* conversionState);

/* 0x48C8B0 */
void OptModel_AppendConvertedFacesForCurrentMesh(struct OptNode* destinationFaceNode,
												 struct OptNode* firstSourceFaceNode,
												 struct OptimizedPolyObject* sourceModel,
												 struct SceneMesh* conversionState);

/* 0x48C910 */
uint16_t OptModel_CreateRuntimeHandle(uint16_t packedModelHandle);

/* 0x48CA90 */
void OptModel_FixupRuntimeTexturePointers(struct OptNode* node, struct OptimizedPolyObject* dstModel,
										  struct OptimizedPolyObject* srcModel);

/* 0x48CBB0 */
struct OptNode* OptModel_FindCorrespondingTextureNode(struct OptNode* srcNode, struct OptNode* dstNode,
													  const uint16_t* sourcePalette);

/* 0x48CC50 */
struct OptNode* OptModel_FindCorrespondingTextureNodeInModel(struct OptimizedPolyObject* dstModel,
															 struct OptimizedPolyObject* srcModel,
															 const uint16_t* sourcePalette);

/* 0x48CDD0 */
size_t OptModel_GetSerializedNodeSize(const struct OptNode* node, struct SceneMesh* parentState);

/* 0x48D090 */
uint8_t OptModel_FindNearestRgb565Index(const uint16_t* palette, int targetRed, int targetGreen,
										int targetBlue, int startIndex, int endIndex);

/* 0x48D150 */
void OptModel_PrepareTexturePalette(uint16_t* paletteRgb565, int entryCount);

/* 0x48D1E0 */
size_t OptModel_BuildRuntimeNode(const struct OptNode* sourceNode, struct SceneMesh* buildState,
								 uint8_t* destination);

/* 0x48F580 */
void OptModel_BuildFaceNormalTangentData(float* dest, const struct OptPackedFaceData* faceData, int faceCount,
										 const struct SceneMesh* conversionState);

/* 0x490010 */
void OptModel_BuildVertexNormalsFromFaces(float* dest, const struct OptPackedFaceData* faceData,
										  int faceCount);

/* 0x490110 */
struct OptNode* OptModel_ResolveNodeRef(const struct OptimizedPolyObject* model, const char* name);

/* 0x490150 */
struct OptNode* OptModel_FindNodeByName(struct OptNode* node, const char* name);

/* 0x4901B0 */
size_t OptModel_LoadRgbOrTexFile(uint8_t* dst, const char* fileName);

/* 0x4906C0 */
int OptModel_GetExternalTextureSerializedSize(const char* sourceFileName);

/* 0x4A0B80 */
uint16_t OptModel_LoadHandle(const char* path);

#ifdef __cplusplus
}
#endif

#endif
