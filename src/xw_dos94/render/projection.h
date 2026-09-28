#ifndef XW_DOS94_PROJECTION_H
#define XW_DOS94_PROJECTION_H
#include "xw_dos94/render/lighting.h"

enum {
	DOS94_PROJECTION_VERTICES = 128,
	DOS94_PROJECTION_EDGES = 256,
	/* Original vertices plus at most two clipped endpoints per byte-indexed edge. */
	DOS94_PROJECTION_POINTS = DOS94_PROJECTION_VERTICES + 2 * DOS94_PROJECTION_EDGES
};

enum {
	DOS94_EDGE_OFF_RIGHT = 1,
	DOS94_EDGE_OFF_LEFT = 2,
	DOS94_EDGE_OFF_Y = 4,
	DOS94_EDGE_VISIBLE = 8,
	DOS94_EDGE_CLASSIFIED =
		DOS94_EDGE_OFF_RIGHT | DOS94_EDGE_OFF_LEFT | DOS94_EDGE_OFF_Y | DOS94_EDGE_VISIBLE,
	DOS94_EDGE_TRACED = 0x20,
	DOS94_EDGE_BEHIND_EYE = 0x80
};

typedef struct Dos94ProjectedPoint {
	Dos94ScreenPoint screen;
	int16_t light;
} Dos94ProjectedPoint;

typedef struct Dos94ProjectedEdge {
	Dos94ProjectedPoint* first;
	Dos94ProjectedPoint* second;
	uint32_t xDifference, yDifference;
	uint32_t slope;
	uint16_t fraction;
	bool slopeCached, yDominant;
	int8_t xDirection, yDirection;
	uint8_t flags;
} Dos94ProjectedEdge;

typedef struct Dos94MeshProjection {
	Dos94MeshView mesh;
	Dos94EyePoint eyeVertices[DOS94_PROJECTION_VERTICES];
	Dos94ProjectedPoint points[DOS94_PROJECTION_POINTS];
	Dos94ProjectedPoint* vertexScreen[DOS94_PROJECTION_VERTICES];
	Dos94ProjectedEdge edges[DOS94_PROJECTION_EDGES];
	Dos94ProjectedPoint* linePoint1;
	uint16_t pointCount;
	uint8_t validCount, offRightCount, offscreenCount, offLeftCount, sliverCount;
	bool someZNegative, failed;
} Dos94MeshProjection;

bool Dos94Projection_BeginMesh(Dos94MeshProjection* projection, const Dos94MeshView* mesh,
							   const Dos94EyePoint* eye, bool someZNegative);
/* A NULL face projects standalone lines without vertex lighting. */
Dos94ProjectedPoint* Dos94_TRANSFM2_calclinepts(Dos94MeshProjection* projection, Dos94Lighting* lighting,
												uint8_t first, uint8_t second, const Dos94FaceView* face);
int Dos94_TRANSFM2_getfacescreenxy(Dos94MeshProjection* projection, Dos94Lighting* lighting,
								   const Dos94FaceView* face);
#endif
