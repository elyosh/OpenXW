#ifndef XW_RENDER_RENDER_SCENE_H
#define XW_RENDER_RENDER_SCENE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/landru_config.h"

#include <aeron/compat/d3d.h>
#include <landru/memhdl.h>
#include <stddef.h>
#include <stdint.h>
#include <xw/assets/opt_model.h>

struct ObjectRecord;
struct OptNode;
struct OptPackedFaceRecord;
struct OptTexCoord;
struct OptVector;
struct OptimizedPolyObject;
struct Std3DRenderTri;
typedef struct DefaultWhiteTexture DefaultWhiteTexture;
typedef struct FaceTextureGradients FaceTextureGradients;

enum { RENDER_SCENE_VIEW_ORIENTATION_INDEX = 3 };

enum {
	RENDER_SCENE_SPAN_CAPACITY = 20000,
	RENDER_SCENE_FACE_CAPACITY = 5000,
	RENDER_SCENE_MESH_CAPACITY = 5000,
	RENDER_SCENE_CLIP_CAPACITY_MULTIPLIER = 4,
	RENDER_SCENE_LEGACY_CODE_REGION_SIZE = 0x80000
};

enum {
	RENDER_DAMAGE_MESH_TYPE = 3,
	RENDER_DAMAGE_FIRST_FRAME = 0x8000,
	RENDER_DAMAGE_SCREEN_HIGH_MASK = -65536,
	RENDER_DAMAGE_SCREEN_SCALE = 256
};

enum {
	RENDER_MESH_MIP_FOOTPRINT_LIMIT = 256,
	RENDER_MESH_MIN_MIP_DIMENSION = 8,
	RENDER_MESH_MIP_AREA_SHIFT = 2,
	RENDER_MESH_OPAQUE_PALETTE_OFFSET = 2048,
	RENDER_MESH_NO_COLOR_KEY_PREFIX = '_'
};

extern const float g_renderSignedBasisToFloat;
extern const float g_modelNodeQ15ToFloat;

extern struct DefaultWhiteTexture g_defaultWhiteTexture;
extern struct OptTextureData* g_defaultWhiteTextureDescPtr;
extern float g_lodDistanceScale;
extern int g_bwingBridgeMeshIndex;
extern int g_forcedLodLevel;
extern int g_nodeSwitchIndex;
extern struct OptTextureData* g_curTextureDesc;
extern float g_textureMipScale;

enum {
	RENDER_NODE_DESCRIPTOR_SELECTOR_5 = 5,
	RENDER_NODE_DESCRIPTOR_SELECTOR_6 = 6,
	RENDER_NODE_DESCRIPTOR_SELECTOR_7 = 7,
	RENDER_NODE_DESCRIPTOR_SELECTOR_8 = 8
};

extern int g_meshReusableProjectedVertexCount;
extern int g_dirLightingEnabled;
extern int g_vertexLightOcclusionEnabled;
extern uint16_t g_billboardObjectOrTypeIndex;
extern uint16_t g_damageBillboardMeshCount;

enum { RENDER_SCENE_AXIS_ANGLE_COMPONENTS = 4, RENDER_SCENE_ROTATION_MATRIX_CAPACITY = 16 };

enum {
	RENDER_SCENE_BACKDROP_EYE_DISTANCE = 100000,
	RENDER_SCENE_PHONG_SLOT_COUNT = 200,
	RENDER_SCENE_FACE_KEY_LAYER_SHIFT = 16
};

extern const float g_hardwareDirectionalLightScale;
extern const float g_hardwarePointLightFacingCutoff;
extern const double g_hardwareDiffuseDistanceScale;
extern const float g_hardwareTriangleVertexCountFloat;
extern const float g_hardwareQuadVertexCountFloat;
extern const float g_hardwareDistantProjectionScale;
extern const float g_hardwareDistantDepthSubtract;
extern const float g_hardwareDepthReciprocalScale;
extern const float g_hardwareShadeIntensityNegativeScale;
extern const float g_renderRadiansPerAngleByte;
extern const float g_modelFacePlanePositiveCrossingLimit;
extern const float g_modelFacePlaneNegativeCrossingLimit;

enum {
	RENDER_HARDWARE_SHADE_BASE = 48,
	RENDER_HARDWARE_SHADE_MAX = 255,
	RENDER_HARDWARE_GRAYSCALE_MULTIPLIER = 0x010101,
	RENDER_HARDWARE_VERTEX_ALPHA = 254,
	RENDER_HARDWARE_OPAQUE_ALPHA = 255,
	RENDER_HARDWARE_ALPHA_SHIFT = 24
};

enum {
	RENDER_HARDWARE_BATCH_LIMIT = 256,
	RENDER_HARDWARE_FACE_VERTEX_BUDGET = 8,
	RENDER_HARDWARE_FACE_TRIANGLE_BUDGET = 2,
	RENDER_HARDWARE_BUFFER_BUDGET_DIVISOR = 4,
	RENDER_HARDWARE_VERTEX_EXEC_MULTIPLIER = 2,
	RENDER_HARDWARE_TRIANGLE_EXEC_BYTES = 24,
	RENDER_HARDWARE_BUFFER_PARTS = 2
};
typedef struct ObjectPointLight ObjectPointLight;
typedef struct ProjVertex ProjVertex;
typedef struct RenderObjectListEntry RenderObjectListEntry;
typedef struct SceneBillboardQueueEntry SceneBillboardQueueEntry;
typedef struct SceneEdge SceneEdge;
typedef struct SceneFace SceneFace;
typedef struct SceneMesh SceneMesh;
typedef struct SceneSpan SceneSpan;
typedef struct SoftwareLightSample SoftwareLightSample;

/* Original IDB size: 12376 bytes. */
struct DefaultWhiteTexture {
	/* IDB +0x0: Runtime initialized 8x8 fallback texture header. */
	struct OptTextureData descriptor;
	/* IDB +0x18: 64 indexed texels generated from the all-white RGB source. */
	uint8_t texels[64];
	/* IDB +0x58: 16 shade levels of 256 palette indices. */
	uint8_t indexedShades[4096];
	/* IDB +0x1058: 16 shade levels of 256 packed 16-bit colors. */
	uint16_t packedShades[4096];
};

/* Original IDB size: 24 bytes. */
struct FaceTextureGradients {
	/* IDB +0x0: First per-face texture-gradient vector; rotated into SceneFace.gradients[0..2]. */
	float gradient0[3];
	/* IDB +0xC: Second per-face texture-gradient vector; rotated into SceneFace.gradients[3..5]. */
	float gradient1[3];
};

/* Original IDB size: 16 bytes. */
struct ObjectPointLight {
	/* IDB +0x0 */
	int x;
	/* IDB +0x4 */
	int y;
	/* IDB +0x8 */
	int z;
	/* IDB +0xC: Integer light weight selected by source type/state and multiplied by eight. */
	int intensity;
};

/* Original IDB size: 24 bytes. */
struct ProjVertex {
	/* IDB +0x0 */
	float screenX;
	/* IDB +0x4 */
	float screenY;
	/* IDB +0x8: Projection depth value; normal path stores projectionScale/viewZ and clipped path stores
	 * viewZ-1. */
	float depth;
	/* IDB +0xC: Float lighting intensity at +12, clamped to 1.0 by RenderScene_ComputeVertexLighting. */
	float lightIntensity;
	/* IDB +0x10 */
	float u;
	/* IDB +0x14 */
	float v;
};

/* Original IDB size: 12 bytes. */
struct RenderObjectListEntry {
	/* IDB +0x0: Signed camera-space depth used by the ascending merge sort. */
	int sortDepth;
	/* IDB +0x4: Object index or special scene reference (0x3800 plus index in X-Wing). */
	int objectIdx;
	/* IDB +0x8: Next record in the transient linked render list. */
	struct RenderObjectListEntry* next;
};

/* Original IDB size: 16 bytes. */
struct SceneBillboardQueueEntry {
	/* IDB +0x0: Object index or 0x3800-based special scene reference. */
	uint16_t objectOrTypeIndex;
	/* IDB +0x2: Low 7 bits select sprite frame; bits 7..14 select model type. */
	int16_t frame;
	/* IDB +0x4: Size parameter passed to SceneBillboard_ComputeProjectedSize. */
	int16_t screenSize;
	/* IDB +0x6: Projected screen X. */
	int16_t screenX;
	/* IDB +0x8: Projected screen Y. */
	int16_t screenY;
	/* IDB +0xA: Signed depth-sort key and projected-size input. */
	int depthZ;
	/* IDB +0xE: Angle passed to the sprite rotation renderer. */
	int16_t rotationAngle;
};

/* Original IDB size: 28 bytes. */
struct SceneEdge {
	/* IDB +0x0: Exclusive final scanline, clipped to viewport height. */
	int yEnd;
	/* IDB +0x4: First scanline, ceil of projected Y and clamped to zero. */
	int yStart;
	/* IDB +0x8 */
	float x;
	/* IDB +0xC */
	float lightIntensity;
	/* IDB +0x10 */
	float dxdy;
	/* IDB +0x14 */
	float dLightIntensityDy;
	/* IDB +0x18: Generated near-plane intersection for reuse by adjacent faces; otherwise NULL. */
	struct ProjVertex* pClipVert;
};

/* Original IDB size: 112 bytes. */
struct SceneFace {
	/* IDB +0x0: Source face index within mesh face data. */
	int faceIndex;
	/* IDB +0x4: Mesh containing this face. */
	struct SceneMesh* mesh;
	/* IDB +0x8: Face index combined with a context counter shifted left 16; upper-part identity not yet
	 * established. */
	unsigned int packedFaceKey;
	/* IDB +0xC: -1 requests near-plane edge clipping; rasterization resets to viewport height. */
	int nearClipState;
	/* IDB +0x10: Two three-float texture gradients transformed into view space. */
	float gradients[6];
	/* IDB +0x28: After projection, reciprocal-depth plane: [0]*screenX + [1]*screenY + [2]. Temporarily holds
	 * view-space texture-origin vector while projection coefficients are built. */
	float depthPlane[3];
	/* IDB +0x34 */
	float spanLightIntensityDx;
	/* IDB +0x38 */
	struct SceneEdge* pScanEdge;
	/* IDB +0x3C: Per-face row of 12-byte software lighting samples. Allocated from g_scenePhongData by
	 * RenderScene_CullMeshFacesFromView; indexed by X >> g_sw3dLightSampleBlockShift in
	 * sw3d_DrawTexturedSpan. */
	struct SoftwareLightSample* lightSamples;
	/* IDB +0x40 */
	int yTop;
	/* IDB +0x44 */
	int yBot;
	/* IDB +0x48: Maximum projected reciprocal depth among the face vertices. */
	float maxVertW;
	/* IDB +0x4C: Minimum projected reciprocal depth among the face vertices. */
	float minVertW;
	/* IDB +0x50: Up to five edges after clipping a triangle/quad. */
	struct SceneEdge* edges[5];
	/* IDB +0x64 */
	int edgeCount;
	/* IDB +0x68: Per-face scanline pointers indexed by scanY - yTop. */
	struct SceneSpan** pSpans;
	/* IDB +0x6C: Integer footprint estimate used to choose a mip level, not a direct texture-array index. */
	int mipLevel;
};

/* Original IDB size: 208 bytes. */
struct SceneMesh {
	/* IDB +0x0: Originating runtime game object, using the shared 83-byte ObjectRecord layout. */
	struct ObjectRecord* pObject;
	/* IDB +0x4: Radians supplied by the object mesh rotation byte (2*pi/256). */
	float rotAngle;
	/* IDB +0x8 */
	float viewPosX;
	/* IDB +0xC */
	float viewPosY;
	/* IDB +0x10 */
	float viewPosZ;
	/* IDB +0x14: Nine-float forward view transform used by vertex and gradient projection. */
	float viewOrient[9];
	/* IDB +0x38 */
	float posX;
	/* IDB +0x3C */
	float posY;
	/* IDB +0x40 */
	float posZ;
	/* IDB +0x44: Nine-float inverse/object-oriented transform paired with viewOrient. */
	float orient[9];
	/* IDB +0x68: Opaque node type-19 words. */
	unsigned int nodeFlags[3];
	/* IDB +0x74: Descriptor pointer copied by the default type-10 selector. */
	void* nodeDescriptor;
	/* IDB +0x78 */
	int vertexCount;
	/* IDB +0x7C: Model-space vertex array, 12-byte OptVector stride. */
	struct OptVector* vertices;
	/* IDB +0x80: UV array read by RenderScene_ProjectMeshVertices with 8-byte stride. */
	struct OptTexCoord* uvs;
	/* IDB +0x84: Vertex-normal array supplied by node type 11 or by the packed face-data tail. */
	struct OptVector* vertexNormals;
	/* IDB +0x88: Type-10 descriptor pointer when param1 is 7 or 8. */
	void* nodeType10Descriptor78;
	/* IDB +0x8C */
	int faceCount;
	/* IDB +0x90 */
	int edgeCount;
	/* IDB +0x94: Three-float normal per face. */
	struct OptVector* faceNormals;
	/* IDB +0x98: Two three-float texture gradients per face, 24-byte stride. */
	struct FaceTextureGradients* faceTexGradients;
	/* IDB +0x9C: Type-10 descriptor pointer when param1 is 5 or 6. */
	void* nodeType10Descriptor56;
	/* IDB +0xA0: Points directly to the packed OPT face payload at byte +4 (after edgeCount), with 64-byte
	 * record stride; no distinct renderer face-record format. */
	struct OptPackedFaceRecord* faces;
	/* IDB +0xA4: Name of the active OPT_TEXTURE node. */
	char* pTextureName;
	/* IDB +0xA8 */
	struct OptTextureData* pMaterial;
	/* IDB +0xAC */
	uint8_t* pTexels;
	/* IDB +0xB0 */
	uint8_t* pPalette;
	/* IDB +0xB4: Packed 16-bit shade table, 4096 bytes after the indexed shade table. */
	uint16_t* pColorKeyPalette;
	/* IDB +0xB8: Starting index in the visible-face queue, captured before culling. */
	int firstVisibleFace;
	/* IDB +0xBC */
	int vertBaseIndex;
	/* IDB +0xC0 */
	int edgeBaseIndex;
	/* IDB +0xC4: Number of faces accepted during this mesh cull. */
	int visibleFaceCount;
	/* IDB +0xC8: Number of projected vertices in this mesh, including generated clip vertices. */
	int projVertCursor;
	/* IDB +0xCC: Count of allocated software edges for this mesh, clipped or ordinary. */
	int clippedEdgeCount;
};

/* Original IDB size: 24 bytes. */
struct SceneSpan {
	/* IDB +0x0 */
	struct SceneSpan* next;
	/* IDB +0x4 */
	int xStart;
	/* IDB +0x8 */
	int xEnd;
	/* IDB +0xC: Intensity at xStart, staged into current scan edge by visible-face drawing. */
	float lightIntensity;
	/* IDB +0x10: Horizontal light slope staged into SceneFace.spanLightIntensityDx. */
	float dLightIntensityDx;
	/* IDB +0x14 */
	struct SceneFace* face;
};

/* Original IDB size: 12 bytes. */
struct SoftwareLightSample {
	/* IDB +0x0: Scene/vertical-block stamp; matching stamp reuses the sample, difference 1 advances to the
	 * next vertical block. */
	int stamp;
	/* IDB +0x4: Lighting intensity at the top sample row of the cached vertical block. */
	float intensity;
	/* IDB +0x8: Next sample-row intensity minus intensity; multiply by the subrow interpolation factor. */
	float intensityDelta;
};

extern int g_capVertexAlpha;
extern int g_powerVrBeginScenePending;
extern int g_maxBatchTris;
extern D3DTLVERTEX* g_flightVertexBuffer;
extern float g_flightVpOriginY;
extern struct Std3DRenderTri* g_triBuffer;
extern int g_maxBatchVerts;
extern int g_d3dVertexCount;
extern unsigned int g_d3dIndexCount;
extern float g_flightVpOriginX;
extern int g_sceneFlushDrawTargetMarkers;
extern int g_hardwareFrameField_55CAE8;
extern uint8_t g_bBackdropMeshMode;
extern unsigned int g_curLayerId;
extern unsigned int g_phongSlotStride;
extern unsigned int g_sw3dLightSampleCacheSceneStampBase;
extern SceneFace g_sw3dOcclusionSentinelFace;
extern float g_invProjScale;
extern SceneSpan* g_pSceneSpanDataCur;
extern SceneSpan* g_pSceneSpanDataEnd;
extern int g_sceneSpanPtrAvail;
extern int g_visFacePassStart;
extern int g_meshQueueIndex;
extern SceneSpan* g_sceneSpanDataBase;
extern LandruHandle g_sceneSpanDataHandle;
extern int g_sceneSpanDataCapacity;
extern SceneSpan** g_sceneSpanPtrList;
extern LandruHandle g_sceneSpanPtrListHandle;
extern SoftwareLightSample* g_scenePhongData;
extern LandruHandle g_scenePhongDataHandle;
extern int g_phongSlotIndex;
extern SceneFace* g_visFaceList;
extern LandruHandle g_visFaceListHandle;
extern int g_visFaceCount;
extern ProjVertex* g_projVertList;
extern LandruHandle g_projVertListHandle;
extern SceneEdge* g_sceneEdgeList;
extern LandruHandle g_sceneEdgeListHandle;
extern int* g_vertexRemap;
extern LandruHandle g_vertexRemapHandle;
extern int* g_sceneEdgeFlags;
extern LandruHandle g_sceneEdgeFlagsHandle;
extern void** g_sceneSclEdgeList;
extern LandruHandle g_sceneSclEdgeListHandle;
extern SceneSpan** g_scanlineSpanHeads;
extern LandruHandle g_scanlineSpanHeadsHandle;
extern SceneMesh* g_meshQueue;
extern LandruHandle g_meshQueueHandle;
extern int g_sceneSpanPtrCapacity;
extern int g_sceneFaceMax;
extern int g_projVertCount;
extern int g_projVertMax;
extern int g_sceneEdgeCursor;
extern int g_sceneEdgeMax;
extern int g_vertexRemapCapacity;
extern int g_sceneEdgeFlagsCapacity;
extern int g_meshQueueMax;
extern OptVector g_meshCullEyePosition;

enum { XW_POINT_LIGHT_CAPACITY = 8 };

extern ObjectPointLight g_objectPointLights[XW_POINT_LIGHT_CAPACITY];

/* Declarations follow ascending original IDB address. */

/* 0x401000 */
void RenderScene_DrawAllObjectRootMeshes(uint16_t objectIndex);

/* 0x408B90 */
void RenderScene_QueueCraftDamageBillboards(uint16_t objectIndex);

/* 0x408BE0 */
void RenderScene_QueueCraftDamageBillboardsForObjectType(uint16_t objectIndex, uint16_t objectType);

/* 0x408E00 */
void RenderScene_DrawRollAlignedObjectModel(uint16_t objectIndex);

/* 0x47CDE0 */
void RenderScene_ComputeVertexLighting(struct SceneMesh* mesh, struct ProjVertex* outVert,
									   const struct OptVector* normal, const struct OptVector* pos,
									   const struct OptVector* eyePos);

/* 0x47D9B0 */
void RenderScene_TransformFaceTextureGradients(struct SceneFace* face,
											   const struct FaceTextureGradients* faceTexGradients,
											   const float* viewPosAndOrient);

/* 0x47F130 */
int RenderScene_ProjectMeshVertices(struct SceneMesh* mesh);

/* 0x47F8F0 */
int RenderScene_ProjectDistantMeshVertices(struct SceneMesh* mesh);

/* 0x47FBA0 */
void RenderScene_DrawMeshFaces(struct SceneMesh* mesh);

/* 0x4820D0 */
void RenderScene_DrawMesh(const struct SceneMesh* mesh);

/* 0x482240 */
void RenderScene_InitHardwareFrame(void);

/* 0x482390 */
void RenderScene_FlushGeometry(void);

/* 0x482470 */
int RenderScene_EmitFlightVertex(int vertexIndex, const struct ProjVertex* vertices);

/* 0x482630 */
void RenderScene_ClearFrameBuffers(void);

/* 0x4862D0 */
void RenderScene_CullMeshFacesFromView(struct SceneMesh* mesh);

/* 0x488180 */
void RenderScene_DrawSceneMesh(const struct SceneMesh* mesh);

/* 0x488700 */
void RenderScene_ApplyBwingBridgeRotation(void* unusedModel, const struct ObjectRecord* objectRecord,
										  struct SceneMesh* mesh, int bridgeMeshIndex);

/* 0x488790 */
void RenderScene_DrawObjectModel(const void* objectRecord);

/* 0x488CE0 */
void RenderScene_DrawNoAssetSourceModel(const struct ObjectRecord* objectRecord, int rootMeshIndex);

/* 0x4890E0 */
void RenderScene_DrawObjectRootMeshWithSwitch(const void* objectRecord, int rootMeshIndex,
											  int nodeSwitchIndex);

/* 0x489690 */
void RenderScene_DrawModelNode(struct OptimizedPolyObject* object, struct OptNode* node,
							   struct SceneMesh* mesh);

/* 0x489F20 */
int RenderScene_IsSegmentOccludedByObjectModel(struct ObjectRecord* object,
											   const struct OptVector* segmentStart,
											   const struct OptVector* segmentEnd);

/* 0x48A070 */
int RenderScene_TestSegmentAgainstModelNode(struct OptimizedPolyObject* model, struct OptNode* node,
											struct SceneMesh* mesh, const struct OptVector* segmentStart,
											const struct OptVector* segmentEnd);

/* 0x48A4B0 */
int RenderScene_TestSegmentAgainstMeshFaces(const struct SceneMesh* mesh,
											const struct OptVector* segmentStart,
											const struct OptVector* segmentEnd);

/* 0x491870 */
void RenderScene_AllocateBuffers(void);

/* 0x491AC0 */
void RenderScene_Initialize(int resetSceneState);

/* 0x491DB0 */
void RenderScene_UnlockBuffers(void);

/* 0x491EA0 */
void RenderScene_FreeBuffers(void);

#ifdef __cplusplus
}
#endif

#endif
