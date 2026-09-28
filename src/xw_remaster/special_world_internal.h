#ifndef XW_REMASTER_SPECIAL_WORLD_INTERNAL_H
#define XW_REMASTER_SPECIAL_WORLD_INTERNAL_H
#include "xw_remaster/assets.h"
#include "xw_remaster/dos_ship.h"
#include "xw_remaster/special_world.h"

typedef struct XwWorldModel {
	uint64_t key; /* Cell/kind/placement, independent of the visible list. */
	int32_t position[3];
	uint16_t type, component, parent;
	float depth;
} XwWorldModel;

typedef struct XwWorldQuad {
	float transform[16];
	uint16_t mesh; /* DOS palette offset, Windows cached tile shape. */
} XwWorldQuad;

typedef struct XwWorldBuild {
	const XwRenderSnapshot* snapshot;
	const XwRenderAssetSet* assets;
	const XwRenderWorldLayout* layout;
	const XwRenderView* view;
	float shadow_distance;
	XwWorldModel* models;
	XwWorldQuad* quads;
	uint32_t model_count, quad_count, model_capacity, quad_capacity;
} XwWorldBuild;

bool XwWorld_Surface(XwWorldBuild* b);
bool XwWorld_Trench(XwWorldBuild* b);
bool XwWorld_Course(AeronScene3D* scene, const XwRenderSnapshot* s, const XwPreparedFlight* frame,
					const XwRenderAssetSet* set, bool reset, unsigned counts[2]);
/* Counts share the reserved scene budget across scenery and course meshes. */
bool XwWorld_Submit(AeronScene3D* scene, const XwRenderView* view, AeronSceneMeshInstance* instance,
					float radius, unsigned counts[2]);
bool XwWorld_DosCourse(const XwRenderSnapshot* s, const XwRenderAssetSet* set, const XwSnapObject* object,
					   const XwRenderView* view);
const XwSnapDamageCell* XwWorld_Damage(const XwSnapSpecialWorld* state, unsigned start, uint16_t key);
float XwWorld_Depth(const XwWorldBuild* b, const int32_t position[3]);
bool XwWorld_Model(XwWorldBuild* b, uint64_t key, unsigned type, unsigned component, int32_t x, int32_t y,
				   int32_t z, unsigned parent, float depth);
bool XwWorld_Quad(XwWorldBuild* b, unsigned mesh, const float position[3], const float across[3],
				  const float along[3]);
bool XwWorld_Tile(XwWorldBuild* b, unsigned mesh, int32_t x, int32_t y);
bool XwWorld_DosMeshes(AeronCommandBuffer* cmd);
bool XwWorld_WindowsMeshes(AeronCommandBuffer* cmd, const XwWorldBuild* b);
const XwDosMesh* XwWorld_DosQuad(void);
const AeronSceneMesh* XwWorld_WindowsQuad(unsigned index);
const AeronSceneMeshTable* XwWorld_Component(unsigned component);
void XwWorld_MeshesShutdown(void);
#endif
