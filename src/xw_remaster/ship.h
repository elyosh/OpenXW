#ifndef XW_REMASTER_SHIP_H
#define XW_REMASTER_SHIP_H
#include "xw_remaster/assets.h"
#include "xw_remaster/render_math.h"

typedef struct XwShipSelection {
	XwRenderAssetId asset_id;
	uint16_t component;
	uint32_t variant;
} XwShipSelection;

/* 1 selects a model, 0 belongs to another pass, -1 is an invalid/missing required source. */
int XwShip_Select(const XwRenderAssetSet* set, const XwSnapObject* object, XwShipSelection* out);
bool XwShip_Eligible(const XwRenderSnapshot* snapshot, const XwSnapObject* object);
bool XwShip_Projectile(const XwSnapObject* object);
bool XwShip_BuildMeshTable(const XwMeshAsset* asset, const XwSnapObject* object, const XwSnapCraft* craft,
						   uint16_t component, const float* rotations, AeronSceneMeshTable* out);
void XwShip_ProjectileMatrix(const XwSnapObject* object, const int32_t camera[3], const int32_t origin[3],
							 float out[16]);
float XwShip_Radius(const XwMeshAsset* asset, const AeronSceneMeshTable* table, const float transform[16]);
int XwShip_Visible(const XwRenderView* view, const float transform[16], float radius);
#endif
