#ifndef XW_RENDER_FLIGHT_HYPERSPACE_H
#define XW_RENDER_FLIGHT_HYPERSPACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/assets/opt_model.h"
#include "xw/render/render_scene.h"

#include <stddef.h>
#include <stdint.h>

enum { FLIGHT_HYPERSPACE_MODEL_TYPE = 143 };

typedef struct XwHyperspaceFaceData {
	int edgeCount;
	OptPackedFaceRecord records[1];
	OptVector faceNormals[1];
	FaceTextureGradients faceTexGradients[1];
} XwHyperspaceFaceData;

extern OptVector g_hyperspaceStreakQuadVertices[4];
extern OptNode g_hyperspaceVertexNode;
extern OptTexCoord g_hyperspaceTexcoords[4];
extern OptNode g_hyperspaceTexcoordNode;
extern OptVector g_hyperspaceVertexNormals[1];
extern OptNode g_hyperspaceVertexNormalNode;
extern XwHyperspaceFaceData g_hyperspaceFaceData;
extern OptNode g_hyperspaceFaceNode;
extern OptNode* g_hyperspaceNodeChildren[4];
extern OptNode g_hyperspaceRootNode;
extern OptNode* g_hyperspaceRootNodes[1];
extern OptimizedPolyObject g_hyperspaceModelHeaderPatch;

/* Declarations follow ascending original IDB address. */

/* 0x484D90 */
void FlightHyperspace_DrawTransitionEffectObject(int missionObjectIndex);

#ifdef __cplusplus
}
#endif

#endif
