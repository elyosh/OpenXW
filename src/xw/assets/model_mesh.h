#ifndef XW_ASSETS_MODEL_MESH_H
#define XW_ASSETS_MODEL_MESH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <xw/assets/opt_model.h>
#include <xw/flight/object/object.h>

struct OptNode;
struct OptimizedPolyObject;
typedef struct MeshDescriptor MeshDescriptor;
typedef struct ModelMeshObjectTypeCache ModelMeshObjectTypeCache;
typedef struct XwModelTypeRecord XwModelTypeRecord;

enum { MODEL_TYPE_TARGETABLE = 1 };

enum {
	MODEL_TYPE_RECORD_COUNT = 211,
	MODEL_LOADED_HANDLE_COUNT = MODEL_TYPE_RECORD_COUNT,
	MODEL_TYPE_FLAG_OPT_MESH = 1,
	MODEL_TYPE_FLAG_BITMAP = 2,
	MODEL_TYPE_LOAD_FROM_LIST = 2,
	MODEL_TYPE_FLAG_LOAD_COMMON = 0x18,
	MODEL_TYPE_FLAG_LOAD_PROVING_GROUNDS = 0x58,
	MODEL_TYPE_FLAG_LOAD_DEATH_STAR = 0x98,
	MODEL_TYPE_FLAG_MISSION_REQUIRED = 0x10,
	MODEL_TYPE_FLAG_CRAFT_RECORD = 0x40,
	MODEL_MESH_MAX_COUNT = 50,
	MODEL_MESH_CACHE_COUNT = 69,
	MODEL_TYPE139_ANIMATION_OFFSET = 8,
	MODEL_TYPE140_ANIMATION_OFFSET = 10
};

enum {
	MODEL_MESH_TYPE_DAMAGE_ANIMATION = 3,
	MODEL_MESH_TYPE_4 = 4,
	MODEL_MESH_TYPE_5 = 5,
	MODEL_MESH_TYPE_BRIDGE = 7,
	MODEL_MESH_TYPE_SFOIL = 20,
	MODEL_MESH_TYPE_21 = 21,
	MODEL_MESH_INDEX_NOT_FOUND = -1
};

enum { MODEL_BOUNDS_VECTOR_COUNT = 2, MODEL_BOUNDS_INITIAL_LIMIT = 1073741824 };

/* Original IDB size: 72 bytes. */
struct MeshDescriptor {
	/* IDB +0x0: Component kind; bridge lookup tests value 7. */
	int meshType;
	/* IDB +0x4: Preserved dword; semantics not established in this pass. */
	int field_04;
	/* IDB +0x8 */
	struct OptVector span;
	/* IDB +0x14 */
	struct OptVector center;
	/* IDB +0x20 */
	struct OptVector boxMin;
	/* IDB +0x2C */
	struct OptVector boxMax;
	/* IDB +0x38: Preserved tail of the 72-byte record; semantics not established in this pass. */
	uint8_t gap38[16];
};

/* Original IDB size: 404 bytes. */
struct ModelMeshObjectTypeCache {
	/* IDB +0x0: Drawable mesh count returned by ModelMesh_GetObjectTypeMeshCount, capped at 50. */
	int meshCount;
	/* IDB +0x4: MeshDescriptor meshType values for mesh indices below meshCount. */
	int meshTypes[50];
	/* IDB +0xCC: Pointers into loaded OPT model storage, parallel to meshTypes. */
	struct MeshDescriptor* meshDescriptors[50];
};

/* Original IDB size: 22 bytes. */
struct XwModelTypeRecord {
	/* IDB +0x0: Bit 1 enables resource-list loading; nonzero permits clearing mission-required load flags. */
	uint8_t resourceFlags;
	/* IDB +0x1: Bit 0 selects loaded OPT mesh operations; verified in ModelBounds_EnsureCached. Other flags
	 * remain unclassified. */
	uint8_t flags;
	/* IDB +0x2: Copied to ObjectRecord.familyId during craft initialization. */
	uint8_t familyId;
	/* IDB +0x3: Default XwObjectGenus, copied to runtime craft/static records. Player shots override default
	 * OTHER_PROJECTILE with PLAYER_PROJECTILE. Distinct from familyId and objectType. */
	XwObjectGenus genusId;
	/* IDB +0x4: Updated from ModelBounds_GetMaxExtent by FeDiskIo_BuildModelDef; used for size/collision and
	 * formation scaling. */
	uint16_t maxBoundsExtent;
	/* IDB +0x6: Written as maxBoundsExtent >> 1 by FeDiskIo_BuildModelDef. */
	uint16_t halfMaxBoundsExtent;
	/* IDB +0x8: Resource memory handle released once per distinct handle by FeDiskIo_FreeModelResources. */
	uint16_t memoryHandle;
	/* IDB +0xA: Animation words: model/texture frames below 0xFF00; high values control looping/end/jumps.
	 * Read by ANIM_updateanimstate and scene billboard consumers. */
	const uint16_t* animationFrames;
	/* IDB +0xE */
	uint8_t gap0E[4];
	/* IDB +0x12: Bit 0 permits radar/target selection; bit 6 selects the craft instance layout when drawing.
	 */
	uint8_t objectFlags;
	/* IDB +0x13: Index of the 211-byte craft definition; returned by GetModelIndexFromType. */
	uint8_t craftDefinitionIndex;
	/* IDB +0x14: Zero-based SPEC/SPEC2/SPEC3/DSTAR resource-list selector. */
	uint8_t resourceListIndex;
	/* IDB +0x15: Zero-based nonempty entry index in that list. */
	uint8_t resourceEntryIndex;
};

/* Animation word counts match the original IDB array extents. */
extern const uint16_t g_modelType133AnimationFrames[13];
extern const uint16_t g_modelType137AnimationFrames[7];
extern uint16_t g_fragmentSecondaryAnimationFrames[28];
extern const uint16_t g_componentDamageAnimationFrames[25];
extern const uint16_t g_modelType110AnimationFrames[9];
extern const uint16_t g_modelType111AnimationFrames[11];
extern const uint16_t g_modelType112AnimationFrames[11];
extern const uint16_t g_modelType113AnimationFrames[9];
extern const uint16_t g_modelType134AnimationFrames[13];
extern const uint16_t g_modelType135AnimationFrames[16];
extern const uint16_t g_modelType136AnimationFrames[15];
extern XwModelTypeRecord g_modelTypeTable[MODEL_TYPE_RECORD_COUNT];
extern uint16_t g_loadedModels[MODEL_LOADED_HANDLE_COUNT];
extern ModelMeshObjectTypeCache g_objectTypeMeshCache[MODEL_MESH_CACHE_COUNT];

extern OptVector g_modelBoundsMin[MODEL_TYPE_RECORD_COUNT];
extern int g_modelBoundsCached[MODEL_TYPE_RECORD_COUNT];
extern OptVector g_modelBoundsMax[MODEL_TYPE_RECORD_COUNT];
extern uint8_t g_objectTypeHudShipIds[MODEL_TYPE_RECORD_COUNT];

/* Declarations follow ascending original IDB address. */

/* 0x490870 */
int ModelMesh_GetObjectTypeMeshCount(int objectType);

/* 0x4908F0 */
struct OptNode* ModelMesh_FindFirstMeshVertsNode(struct OptNode* node);

/* 0x490940 */
void ModelMesh_EnsureModelBoundsCached(int objectType);

/* 0x490B00 */
int ModelMesh_GetModelMaxExtent(int modelType);

/* 0x490BB0 */
int64_t ModelMesh_GetModelSizeX(int objectType);

/* 0x490BF0 */
int64_t ModelMesh_GetModelSizeY(int objectType);

/* 0x490C30 */
int64_t ModelMesh_GetModelSizeZ(int objectType);

/* 0x490C70 */
struct MeshDescriptor* ModelMesh_FindDescriptorNodeRecursive(struct OptNode* node,
															 struct OptimizedPolyObject* model);

/* 0x490CC0 */
struct MeshDescriptor* ModelMesh_GetDescriptor(int objectType, int meshIndex);

/* 0x490D50 */
int ModelMesh_GetObjectTypeMeshType(int objectType, int meshIndex);

/* 0x490DF0 */
int ModelMesh_CountHardpoints(int modelType, int meshIndex);

/* 0x490E70 */
int ModelMesh_GetVertexX(int objectType, int meshIndex, int vertexIndex);

/* 0x490F10 */
int ModelMesh_GetVertexY(int objectType, int meshIndex, int vertexIndex);

/* 0x490FB0 */
int ModelMesh_GetVertexZ(int objectType, int meshIndex, int vertexIndex);

/* 0x491050 */
int ModelMesh_GetCenterX(int objectType, int meshIndex);

/* 0x4910F0 */
int ModelMesh_GetCenterY(int objectType, int meshIndex);

/* 0x491190 */
int ModelMesh_GetCenterZ(int objectType, int meshIndex);

/* 0x491230 */
int ModelMesh_GetBoundsMinX(int modelType, int meshIndex);

/* 0x4912D0 */
int ModelMesh_GetBoundsMinY(int modelType, int meshIndex);

/* 0x491370 */
int ModelMesh_GetBoundsMinZ(int modelType, int meshIndex);

/* 0x491410 */
int ModelMesh_GetBoundsMaxX(int modelType, int meshIndex);

/* 0x4914B0 */
int ModelMesh_GetBoundsMaxY(int modelType, int meshIndex);

/* 0x491550 */
int ModelMesh_GetBoundsMaxZ(int modelType, int meshIndex);

/* 0x4915F0 */
int ModelMesh_GetComponentMaxExtent(int modelType, int meshIndex);

/* 0x4916C0 */
int ModelMesh_FindBridgeIndex(struct OptimizedPolyObject* model);

/* 0x491720 */
void ModelMesh_BuildObjectTypeMeshCache(void);

/* 0x4917A0 */
int ModelMesh_GetCachedObjectTypeMeshCount(int objectType);

/* 0x4917D0 */
int ModelMesh_GetCachedObjectTypeMeshType(int objectType, int meshIndex);

/* 0x491820 */
struct MeshDescriptor* ModelMesh_GetCachedDescriptor(int objectType, int meshIndex);

#ifdef __cplusplus
}
#endif

#endif
