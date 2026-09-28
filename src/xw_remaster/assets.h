#ifndef XW_REMASTER_ASSETS_H
#define XW_REMASTER_ASSETS_H
#include "xw_remaster/dos_mesh.h"
#include "xw_remaster/gate_lights.h"
#include "xw_runtime/snapshot/render_assets.h"
#include <aeron/scene/mesh.h>
#include <aeron/scene/runtime_atlas.h>

typedef enum XwAssetPreparation {
	XW_ASSETS_IDLE,
	XW_ASSETS_PREPARING,
	XW_ASSETS_READY,
	XW_ASSETS_FAILED
} XwAssetPreparation;

typedef struct XwMeshAsset {
	AeronSceneMesh* mesh;
	uint32_t component_count;
	int32_t bridge_component;
	const XwGateLightModel* gate_lights;
} XwMeshAsset;

/* Ready describes registered sources, OPT and effect uploads; scene/HUD readiness is separate. */
XwAssetPreparation XwRemasterAssets_Frame(const XwRenderSnapshot* snapshot);
const XwMeshAsset* XwRemasterAssets_Mesh(XwRenderAssetId source);
const XwDosMesh* XwRemasterAssets_DosMesh(XwRenderAssetId source);
const AeronRuntimeAtlas* XwRemasterAssets_Image(XwRenderAssetId source);
void XwRemasterAssets_Shutdown(void);
#endif
