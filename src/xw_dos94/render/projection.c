#include "xw_dos94/render/projection.h"
#include "xw/render/flight_view.h"
#include "xw_dos94/render/dos93_projection.h"
#include <string.h>

/* DOS94 DRAW initialization at 0x69A1A1/0x69A739; only active entries reset. */
bool Dos94Projection_BeginMesh(Dos94MeshProjection* p, const Dos94MeshView* mesh, const Dos94EyePoint* eye,
							   bool someZNegative) {
	if (mesh->vertexCount > DOS94_PROJECTION_VERTICES)
		return false;
	p->mesh = *mesh;
	p->pointCount = 0;
	p->linePoint1 = NULL;
	p->failed = false;
	p->someZNegative = someZNegative;
	memset(p->vertexScreen, 0, mesh->vertexCount * sizeof p->vertexScreen[0]);
	memset(p->edges, 0, mesh->edgeCount * sizeof p->edges[0]);
	memcpy(p->eyeVertices, eye, mesh->vertexCount * sizeof p->eyeVertices[0]);
	return true;
}

static Dos94ProjectedPoint* reserve_point(Dos94MeshProjection* p) {
	if (p->pointCount == DOS94_PROJECTION_POINTS) {
		p->failed = true;
		return NULL;
	}
	Dos94ProjectedPoint* point = &p->points[p->pointCount++];
	*point = (Dos94ProjectedPoint) { 0 };
	return point;
}

static Dos94ProjectedPoint* project_point(Dos94MeshProjection* p, Dos94Lighting* l, uint8_t vertex,
										  uint8_t other, const Dos94FaceView* face, bool reverseLightIds) {
	if (vertex >= p->mesh.vertexCount || other >= p->mesh.vertexCount) {
		p->failed = true;
		return NULL;
	}
	if (p->vertexScreen[vertex])
		return p->vertexScreen[vertex];
	Dos94ProjectedPoint* output = reserve_point(p);
	if (!output)
		return NULL;
	const Dos94EyePoint* point = &p->eyeVertices[vertex];
	if (point->z >= 0) {
		output->screen = (Dos94ScreenPoint) { Dos94_TRANSFM2_getscreenx(point->x, (uint32_t)point->z),
											  Dos94_TRANSFM2_getscreeny(point->y, (uint32_t)point->z) };
		p->vertexScreen[vertex] = output;
	} else {
		const Dos94EyePoint* positive = &p->eyeVertices[other];
		if (positive->z < 0)
			return NULL;
		if (Dos94Assets_Version() == XW_GAME_VERSION_93) {
			output->screen = Dos94_TRANSFM2_calczintersect(point, positive);
			return output;
		}
		int16_t negativeLight = 0, positiveLight = 0;
		if (face && face->gouraud) {
			bool retainNegative = face->twoSided && face->vertexCount == 2;
			if (!Dos94_DRAWPOL_vertexlight(l, &p->mesh, vertex, retainNegative, false) ||
				!Dos94_DRAWPOL_vertexlight(l, &p->mesh, other, retainNegative, false)) {
				p->failed = true;
				return NULL;
			}
			negativeLight = l->vertexLight[reverseLightIds ? other : vertex];
			positiveLight = l->vertexLight[reverseLightIds ? vertex : other];
		}
		output->screen =
			Dos94_TRANSFM2_facezintersect(point, positive, negativeLight, positiveLight, &output->light);
	}
	return output;
}

/* DOS94 0x6A140D: both-behind rejection retains the first point allocation. */
Dos94ProjectedPoint* Dos94_TRANSFM2_calclinepts(Dos94MeshProjection* p, Dos94Lighting* l, uint8_t first,
												uint8_t second, const Dos94FaceView* face) {
	Dos94ProjectedPoint* point = project_point(p, l, first, second, face, false);
	if (!point)
		return NULL;
	p->linePoint1 = point;
	/* 0x6A14F9 keeps first/second light IDs when geometric endpoints swap. */
	return project_point(p, l, second, first, face, true);
}

static void reverse_edge(Dos94ProjectedEdge* edge) {
	edge->xDirection = -edge->xDirection;
	edge->yDirection = -edge->yDirection;
	Dos94ProjectedPoint* point = edge->first;
	edge->first = edge->second;
	edge->second = point;
}

static uint8_t classify_new_edge(Dos94ProjectedEdge* edge) {
	Dos94ScreenPoint a = edge->first->screen, b = edge->second->screen;
	/* SBB/JGE tests the signed endpoint comparison, then negates a wrapping delta. */
	uint32_t dx = (uint32_t)b.x - (uint32_t)a.x, dy = (uint32_t)b.y - (uint32_t)a.y;
	int8_t xs = b.x < a.x ? -1 : 1, ys = b.y < a.y ? -1 : 1;
	if (xs < 0)
		dx = 0u - dx;
	if (ys < 0)
		dy = 0u - dy;
	edge->xDirection = xs;
	edge->yDirection = ys;
	edge->xDifference = dx;
	edge->yDifference = dy;
	if ((ys > 0 && (!dy || b.y < 0 || a.y >= g_flightVpHeight)) ||
		(ys < 0 && (a.y < 0 || b.y >= g_flightVpHeight)))
		return DOS94_EDGE_OFF_Y;
	int32_t minimum = xs > 0 ? a.x : b.x, maximum = xs > 0 ? b.x : a.x;
	if (maximum <= 0)
		return DOS94_EDGE_OFF_LEFT;
	if (minimum >= g_flightVpWidth)
		return DOS94_EDGE_OFF_RIGHT;
	return DOS94_EDGE_VISIBLE;
}

static bool count_edge(Dos94MeshProjection* p, uint8_t flags) {
	if (flags & DOS94_EDGE_OFF_RIGHT) {
		++p->offRightCount;
		return ++p->offscreenCount != 0;
	}
	if (flags & DOS94_EDGE_OFF_LEFT)
		return ++p->offLeftCount != 0;
	if (flags & DOS94_EDGE_OFF_Y)
		return ++p->offscreenCount != 0;
	if (flags & DOS94_EDGE_VISIBLE)
		++p->validCount;
	return true;
}

/* DOS94 0x6A15D0: shared-edge endpoints reverse on each subsequent face visit. */
static bool classify_edges(Dos94MeshProjection* p, Dos94Lighting* l, const uint8_t* vertices, uint8_t count,
						   const Dos94FaceView* face) {
	for (unsigned i = 0; i < count; ++i) {
		unsigned index = vertices[2 * i + 1];
		if (index >= p->mesh.edgeCount) {
			p->failed = true;
			return false;
		}
		Dos94ProjectedEdge* edge = &p->edges[index];
		if (edge->flags == DOS94_EDGE_BEHIND_EYE) {
			++p->offRightCount;
			continue;
		}
		if (edge->flags & DOS94_EDGE_CLASSIFIED) {
			reverse_edge(edge);
			if (!count_edge(p, edge->flags))
				return false;
			continue;
		}
		uint8_t first = vertices[2 * i], second = vertices[2 * i + 2];
		if (first >= p->mesh.vertexCount || second >= p->mesh.vertexCount) {
			p->failed = true;
			return false;
		}
		if (p->eyeVertices[first].z < 0 && p->eyeVertices[second].z < 0) {
			edge->flags = DOS94_EDGE_BEHIND_EYE;
			++p->offRightCount;
			continue;
		}
		edge->first = project_point(p, l, first, second, face, false);
		if (!edge->first)
			return false;
		edge->second = project_point(p, l, second, first, face, false);
		if (!edge->second)
			return false;
		edge->flags = classify_new_edge(edge);
		if (!count_edge(p, edge->flags))
			return false;
	}
	return true;
}

static int orientation(Dos94MeshProjection* p, const Dos94FaceView* face) {
	if (Dos94Assets_Version() != XW_GAME_VERSION_93)
		return 4;
	int result = Dos93_TRANSFM2_checkfaceorientation(p, face);
	return result ? result : 0x10;
}

/* DOS93 0x6A17C8 / DOS94 0x6A1559, including the near-plane fallback. */
int Dos94_TRANSFM2_getfacescreenxy(Dos94MeshProjection* p, Dos94Lighting* lighting,
								   const Dos94FaceView* face) {
	const uint8_t* stream = Dos94Assets_Data(face->stream);
	uint8_t count = face->vertexCount;
	if (!stream || !count || face->stream.size < 2u * count + 2) {
		p->failed = true;
		return 0;
	}
	p->validCount = p->offRightCount = 0;
	p->offscreenCount = p->offLeftCount = (uint8_t)-count;
	p->sliverCount = (uint8_t)(2 - count);
	if (classify_edges(p, lighting, stream + 1, count, face) && (p->validCount || p->offRightCount))
		return orientation(p, face);
	if (!p->failed && p->someZNegative)
		for (unsigned i = 0; i < count; ++i) {
			unsigned vertex = stream[1 + 2 * i];
			if (vertex >= p->mesh.vertexCount) {
				p->failed = true;
				return 0;
			}
			if (!p->vertexScreen[vertex])
				return orientation(p, face);
		}
	return 0;
}
