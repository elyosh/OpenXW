#include "xw_dos94/render/dos93_projection.h"
#include "xw_dos94/render/trace_internal.h"

static void classify_slope(Dos94ProjectedEdge* edge) {
	edge->yDominant = (edge->yDifference >> 1) >= edge->xDifference;
	if ((edge->yDifference >> 1) == edge->xDifference) {
		edge->slope = 2;
		edge->fraction = 0;
		edge->slopeCached = true;
	}
}

static uint64_t signed_slope(Dos94ProjectedEdge* edge) {
	if (!edge->slopeCached) {
		Dos94TraceSlope slope =
			Dos94_math2_calcEdgeSlope(edge->yDominant ? edge->yDifference : edge->xDifference,
									  edge->yDominant ? edge->xDifference : edge->yDifference);
		edge->slope = slope.whole;
		edge->fraction = slope.fraction;
		edge->slopeCached = true;
	}
	uint64_t slope = ((uint64_t)edge->slope << 16) | edge->fraction;
	return edge->xDirection != edge->yDirection ? 0u - slope : slope;
}

/* DOS93 0x6A1B6E: return 5 front, 0 back, or 4 when a normal test is needed. */
int Dos93_TRANSFM2_checkfaceorientation(Dos94MeshProjection* p, const Dos94FaceView* face) {
	const uint8_t* vertices = Dos94Assets_Data(face->stream);
	if (!vertices || face->stream.size < 4u + 2u * face->vertexCount)
		return 4;
	++vertices;
	Dos94ProjectedEdge* first = NULL;
	Dos94ProjectedEdge* second = NULL;
	for (unsigned i = 0; i < face->vertexCount; ++i) {
		unsigned a = vertices[2 * i + 1], b = vertices[2 * i + 3];
		if (a >= p->mesh.edgeCount || b >= p->mesh.edgeCount) {
			p->failed = true;
			return 4;
		}
		first = &p->edges[a];
		second = &p->edges[b];
		if (first->second && first->second == second->first && first->flags != DOS94_EDGE_BEHIND_EYE &&
			second->flags != DOS94_EDGE_BEHIND_EYE)
			break;
		first = second = NULL;
	}
	if (!first || (first->xDifference <= 1 && first->yDifference <= 1) ||
		(second->xDifference <= 1 && second->yDifference <= 1))
		return 4;
	int quadrantChange = second->xDirection + second->yDirection - first->xDirection - first->yDirection;
	if (quadrantChange == -2)
		return first->yDirection >= 0 && second->xDirection < 0 ? 0 : 5;
	if (quadrantChange == 2)
		return first->yDirection < 0 && second->xDirection >= 0 ? 0 : 5;
	classify_slope(second);
	classify_slope(first);
	if (first->yDominant != second->yDominant) {
		bool front = first->yDominant ? first->yDirection == second->xDirection
									  : second->yDirection != first->xDirection;
		return front ? 5 : 0;
	}
	/* Subtraction borrows from the fraction, but the zero test ignores that word. */
	uint32_t difference = (uint32_t)((signed_slope(first) - signed_slope(second)) >> 16);
	if (!difference)
		return 4;
	bool reverse = second->yDominant ? first->xDirection != second->xDirection
									 : -first->yDirection != second->yDirection;
	return reverse == ((difference & 0x80000000u) != 0) ? 5 : 0;
}

/* DOS93 0x69D07F: normalize the point, then truncate each normal/matrix product. */
unsigned Dos93_DRAWPOL_checknormal(const Dos94Transform* transform, const Dos94FaceView* face,
								   const Dos94EyePoint* eyeVertices, uint16_t vertexCount) {
	const uint8_t* stream = Dos94Assets_Data(face->stream);
	if (!stream || face->stream.size < 4 || stream[3] >= vertexCount)
		return 2;
	const Dos94EyePoint* point = &eyeVertices[stream[3]];
	int32_t coordinate[3] = { point->x, point->y, point->z };
	while (coordinate[0] < -65536 || coordinate[0] >= 65536 || coordinate[1] < -65536 ||
		   coordinate[1] >= 65536 || coordinate[2] < -65536 || coordinate[2] >= 65536)
		for (unsigned i = 0; i < 3; ++i)
			coordinate[i] >>= 1;
	uint32_t sum = 0;
	for (unsigned axis = 0; axis < 3; ++axis) {
		uint16_t normal = 0;
		for (unsigned i = 0; i < 3; ++i)
			normal = (uint16_t)(normal + (((int32_t)face->normal[i] * transform->matrix[axis][i]) >> 16));
		sum += (uint32_t)((int16_t)normal * (int32_t)(int16_t)(coordinate[axis] >> 1));
	}
	return (int32_t)sum >= 0 ? 2 : 0;
}
