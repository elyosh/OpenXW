#include "xw/render/flight_hyperspace.h"
#ifdef XW_MODERN
#include "xw_runtime/snapshot/render_sky.h"
#endif

#include "xw/assets/model_mesh.h"
#include "xw/flight/fview.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/anim.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/object.h"
#include "xw/flight/xw.h"
#include "xw/math/trig2.h"
#include "xw/render/flight_view.h"
#include "xw/render/render_quad.h"
#include "xw/util/memory.h"

#include <string.h>

// GLOBAL: XW 0x4DA050
OptVector g_hyperspaceStreakQuadVertices[4] = {
	{ 64.0f, 0.0f, 0.0f }, { 64.0f, -256.0f, 0.0f }, { -64.0f, -256.0f, 0.0f }, { -64.0f, 0.0f, 0.0f }
};
// GLOBAL: XW 0x4DA080
OptNode g_hyperspaceVertexNode = { NULL, OPT_MESHVERTS, 0, NULL, 4, g_hyperspaceStreakQuadVertices };
// GLOBAL: XW 0x4DA098
OptTexCoord g_hyperspaceTexcoords[4] = { { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f } };
// GLOBAL: XW 0x4DA0B8
OptNode g_hyperspaceTexcoordNode = { NULL, OPT_TEXCOORDS, 0, NULL, 4, g_hyperspaceTexcoords };
// GLOBAL: XW 0x4DA0D0
OptVector g_hyperspaceVertexNormals[1] = { { 0.0f, 0.0f, 1.0f } };
// GLOBAL: XW 0x4DA0E0
OptNode g_hyperspaceVertexNormalNode = { NULL, OPT_VERTNORMALS, 0, NULL, 1, g_hyperspaceVertexNormals };
// GLOBAL: XW 0x4DA0F8
XwHyperspaceFaceData g_hyperspaceFaceData = {
	4,
	{ { { 0, 1, 2, 3 }, { 0, 1, 2, 3 }, { 0, 0, 0, 0 }, { 0, 1, 2, 3 } } },
	{ { 0.0f, 0.0f, 1.0f } },
	{ { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } } }
};
// GLOBAL: XW 0x4DA160
OptNode g_hyperspaceFaceNode = { NULL, OPT_FACEDATA, 0, NULL, 1, &g_hyperspaceFaceData };
// GLOBAL: XW 0x4DA178
OptNode* g_hyperspaceNodeChildren[4] = { &g_hyperspaceVertexNode, &g_hyperspaceTexcoordNode,
										 &g_hyperspaceVertexNormalNode, &g_hyperspaceFaceNode };
// GLOBAL: XW 0x4DA188
OptNode g_hyperspaceRootNode = { NULL, 0, 4, g_hyperspaceNodeChildren, 4, g_hyperspaceNodeChildren };
// GLOBAL: XW 0x4DA1A0
OptNode* g_hyperspaceRootNodes[1] = { &g_hyperspaceRootNode };
// GLOBAL: XW 0x4DA1A8
OptimizedPolyObject g_hyperspaceModelHeaderPatch = { &g_hyperspaceModelHeaderPatch, 0, 1,
													 g_hyperspaceRootNodes };

// FUNCTION: XW 0x484D90
void FlightHyperspace_DrawTransitionEffectObject(int missionObjectIndex) {
	int savedBilinearEnabled;
	OptimizedPolyObject* model;
	OptimizedPolyObject savedModelHeader;
	ObjectRecord savedObject;
	int worldY, worldZ;
	g_hyperspaceStreakQuadVertices[1].y = (float)(g_hyperspaceStreakLength >> 1);
	g_hyperspaceStreakQuadVertices[2].y = g_hyperspaceStreakQuadVertices[1].y;
	savedBilinearEnabled = g_bilinearEnabled;
	g_bilinearEnabled = 0;
	model = Memory_LockHandle(g_loadedModels[FLIGHT_HYPERSPACE_MODEL_TYPE]);
	memcpy(&savedModelHeader, model, sizeof(savedModelHeader));
	g_hyperspaceModelHeaderPatch.selfMarker = model;
	memcpy(model, &g_hyperspaceModelHeaderPatch, sizeof(*model));
	memcpy(&savedObject, &g_objectTable[0], sizeof(savedObject));
	worldY = g_missionObjects[missionObjectIndex].worldY;
	worldZ = g_missionObjects[missionObjectIndex].worldZ;
	g_objectTable[0].worldX =
		g_missionObjects[missionObjectIndex].worldX * MISSION_OBJECT_WORLD_COORDINATE_SCALE;
	g_objectTable[0].worldZ = worldZ * MISSION_OBJECT_WORLD_COORDINATE_SCALE;
	g_billboardObjectOrTypeIndex = 0;
	g_objectTable[0].worldY = worldY * MISSION_OBJECT_WORLD_COORDINATE_SCALE;
	g_objectTable[0].objectType = FLIGHT_HYPERSPACE_MODEL_TYPE;
	g_objectTable[0].genusId = XW_GENUS_OTHER_PROJECTILE;
#ifdef XW_MODERN
	g_objectTable[0].roll =
		trig2_arctan(
			(int32_t)((uint32_t)g_objectTable[0].worldZ - (uint32_t)g_flightCamera.worldPosition.z),
			(int32_t)((uint32_t)g_objectTable[0].worldX - (uint32_t)g_flightCamera.worldPosition.x)) +
		TRIG2_QUARTER_TURN;
#else
	g_objectTable[0].roll = trig2_arctan(g_objectTable[0].worldZ - g_flightCamera.worldPosition.z,
										 g_objectTable[0].worldX - g_flightCamera.worldPosition.x) +
							TRIG2_QUARTER_TURN;
#endif
#ifdef XW_MODERN
	XwRenderSky_Hyperstar(
		missionObjectIndex,
		(const int32_t[]) { g_objectTable[0].worldX, g_objectTable[0].worldY, g_objectTable[0].worldZ },
		g_objectTable[0].roll);
#endif
	g_objectTable[0].yaw = 0;
	g_objectTable[0].pitch = TRIG2_QUARTER_TURN;
	g_objectTable[0].orientMatrixDirty = 1;
	fview_newcalcrotate(g_objectTable[0].roll, TRIG2_QUARTER_TURN, 0, 0, &g_objectTable[0]);
	RenderScene_DrawObjectModel(&g_objectTable[0]);
	memcpy(model, &savedModelHeader, sizeof(*model));
	memcpy(&g_objectTable[0], &savedObject, sizeof(savedObject));
	g_bilinearEnabled = savedBilinearEnabled;
}
