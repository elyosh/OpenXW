#include "xw/flight/death_star.h"
#include "xw_dos94/flight/object/collision.h"
#include "xw_dos94/render/fview.h"

static bool vertex(const Dos94MeshView* mesh, uint8_t index, uint8_t shift, int16_t out[3]) {
	if (!Dos94Models_Vertex(mesh, index, out))
		return false;
	for (int i = 0; i < 3; ++i)
		out[i] >>= shift;
	return true;
}

static bool halfplane(const int16_t a[3], const int16_t b[3], const int16_t p[3], int x, int y) {
	int32_t first = (int16_t)(b[x] - a[x]) * (int16_t)(p[y] - a[y]);
	int32_t second = (int16_t)(b[y] - a[y]) * (int16_t)(p[x] - a[x]);
	return (int32_t)((uint32_t)first - (uint32_t)second) < 0;
}

static bool contains(const Dos94MeshView* mesh, const Dos94FaceView* face, const int16_t point[3],
					 uint8_t shift) {
	int16_t normal[3];
	for (int i = 0; i < 3; ++i)
		normal[i] = face->normal[i] < 0 ? (int16_t)-face->normal[i] : face->normal[i];
	int x = 1, y = 2;
	if (normal[2] >= normal[1] && normal[2] >= normal[0]) {
		x = 0;
		y = 1;
	} else if (normal[1] >= normal[0] && normal[2] <= normal[1]) {
		x = 0;
		y = 2;
	}
	const uint8_t* indices = Dos94Assets_Data(face->stream);
	unsigned count = face->vertexCount;
	int16_t a[3], b[3];
	if (!indices || count < 3 || !vertex(mesh, indices[1], shift, a) || !vertex(mesh, indices[3], shift, b))
		return false;
	bool expected = halfplane(a, b, point, x, y);
	/* Read the authored closing pairs as well as the initial edge. */
	for (unsigned i = 1; i <= count; ++i) {
		for (int axis = 0; axis < 3; ++axis)
			a[axis] = b[axis];
		unsigned index = i + 1;
		if (!vertex(mesh, indices[1 + 2 * index], shift, b) || halfplane(a, b, point, x, y) != expected)
			return false;
	}
	return true;
}

/* DOS94 0x687B6C: COLLIDE_checkhitpolygons. Face order and low-bit encoding are gameplay data. */
uint16_t Dos94_COLLIDE_checkhitpolygons(const Dos94MeshView* mesh, const int16_t start[3],
										const int16_t end[3], uint8_t shift) {
	if (shift > 15)
		return 0;
	for (int i = 0; i < 3; ++i) {
		int16_t lo = mesh->bounds[i] >> shift, hi = mesh->bounds[i + 3] >> shift;
		if ((lo > start[i] && lo > end[i]) || (hi < start[i] && hi < end[i]))
			return 0;
	}
	for (unsigned f = 0; f < mesh->faceCount; ++f) {
		Dos94FaceView face;
		int16_t v[3], a[3], b[3], point[3];
		if (!Dos94Models_Face(mesh, f, &face))
			return 0;
		const uint8_t* stream = Dos94Assets_Data(face.stream);
		if (!stream || face.vertexCount == 2)
			continue;
		if (!vertex(mesh, stream[1], shift, v))
			return 0;
		for (int i = 0; i < 3; ++i) {
			a[i] = (int16_t)(start[i] - v[i]);
			b[i] = (int16_t)(end[i] - v[i]);
		}
		int16_t d1 = Dos94_math2_dot3Q15(face.normal, a), d2 = Dos94_math2_dot3Q15(face.normal, b);
		if (d1 > -10 && d1 < 10)
			d1 = 0;
		if (d2 > -10 && d2 < 10)
			d2 = 0;
		if (d1 && d2 && (int16_t)(d1 ^ d2) >= 0)
			continue;
		uint16_t fraction;
		if (!d1 || !d2) {
			/* Defined native policy for the two uninitialized DOS fraction branches. */
			fraction = !d1 ? 0 : 0x7fff;
			for (int i = 0; i < 3; ++i)
				point[i] = !d1 ? start[i] : end[i];
		} else {
			const int16_t* positive = d1 < 0 ? end : start;
			const int16_t* negative = d1 < 0 ? start : end;
			uint16_t numerator = d1 < 0 ? d2 : d1;
			uint16_t denominator = d1 < 0 ? (uint16_t)(d2 - d1) : (uint16_t)(d1 - d2);
			fraction = (uint16_t)(((uint32_t)numerator << 16) / denominator) >> 1;
			for (int i = 0; i < 3; ++i)
				point[i] = (int16_t)(positive[i] +
									 (((int16_t)(negative[i] - positive[i]) * (int32_t)fraction) >> 15));
		}
		if (contains(mesh, &face, point, shift)) {
			if (g_surfaceSpecialTargetCollisionMode && f == 11)
				g_surfaceSpecialTargetHit = 1;
			return fraction | 1;
		}
	}
	return 0;
}
