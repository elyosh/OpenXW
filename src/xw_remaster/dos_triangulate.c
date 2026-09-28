#include "xw_remaster/dos_mesh_internal.h"
#include <math.h>

static double Cross(const float a[2], const float b[2], const float c[2]) {
	return ((double)b[0] - a[0]) * ((double)c[1] - a[1]) - ((double)b[1] - a[1]) * ((double)c[0] - a[0]);
}

bool XwDosMesh_Triangulate(const float (*uv)[2], unsigned count, uint32_t base, XwDosMeshBuild* b) {
	if (count < 3)
		return true;
	if (count > 255 || !XwDosMesh_Grow((void**)&b->indices, &b->index_capacity,
									   b->index_count + 3 * (count - 2), sizeof *b->indices))
		return false;
	uint16_t ring[255];
	double area = 0;
	for (unsigned i = 0; i < count; ++i) {
		ring[i] = i;
		area += (double)uv[i][0] * uv[(i + 1) % count][1] - (double)uv[i][1] * uv[(i + 1) % count][0];
	}
	if (fabs(area) < 1e-8)
		return true;
	double sign = area < 0 ? -1 : 1;
	while (count > 2) {
		bool removed = false;
		for (unsigned i = 0; i < count; ++i) {
			unsigned a = ring[(i + count - 1) % count], c = ring[(i + 1) % count], v = ring[i];
			double turn = sign * Cross(uv[a], uv[v], uv[c]);
			if (turn < -1e-8)
				continue;
			bool inside = false;
			if (turn > 1e-8)
				for (unsigned j = 0; j < count; ++j) {
					unsigned p = ring[j];
					if (p == a || p == v || p == c)
						continue;
					if (sign * Cross(uv[a], uv[v], uv[p]) >= -1e-8 &&
						sign * Cross(uv[v], uv[c], uv[p]) >= -1e-8 &&
						sign * Cross(uv[c], uv[a], uv[p]) >= -1e-8) {
						inside = true;
						break;
					}
				}
			if (inside)
				continue;
			if (turn > 1e-8) {
				b->indices[b->index_count++] = base + a;
				b->indices[b->index_count++] = base + v;
				b->indices[b->index_count++] = base + c;
			}
			for (unsigned j = i + 1; j < count; ++j)
				ring[j - 1] = ring[j];
			--count;
			removed = true;
			break;
		}
		if (!removed)
			return false;
	}
	return true;
}
