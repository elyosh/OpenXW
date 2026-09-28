#ifndef XW_REMASTER_ENGINE_GLOWS_H
#define XW_REMASTER_ENGINE_GLOWS_H
#include "xw_remaster/ship.h"
bool XwEngineGlows_Prepare(void);
bool XwEngineGlows_Submit(AeronScene3D* scene, const XwRenderSnapshot* snapshot, const XwSnapCraft* craft,
						  const XwMeshAsset* asset, const AeronSceneMeshTable* table,
						  const float transform[16], const XwRenderView* view);
void XwEngineGlows_Lights(AeronScene3D* scene, const AeronSceneMesh* mesh, const XwSnapCraft* craft,
						  const AeronSceneMeshTable* table, const float transform[16]);
void XwEngineGlows_Shutdown(void);
#endif
