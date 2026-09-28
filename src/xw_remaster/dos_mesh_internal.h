#ifndef XW_REMASTER_DOS_MESH_INTERNAL_H
#define XW_REMASTER_DOS_MESH_INTERNAL_H
#include "xw_remaster/dos_mesh.h"

typedef struct XwDosMeshBuild {
	const XwRenderDosModel* model;
	XwDosVertex* vertices;
	uint32_t* indices;
	XwDosMark* marks;
	float (*mark_vertices)[4]; /* Line: model-space xyz/1; polygon: face-local uv/0/1. */
	uint32_t vertex_count, index_count, mark_count, mark_vertex_count;
	uint32_t vertex_capacity, index_capacity, mark_capacity, mark_vertex_capacity;
	bool dos93;
} XwDosMeshBuild;

bool XwDosMesh_Grow(void** data, uint32_t* capacity, uint32_t need, size_t stride);
bool XwDosMesh_Marks(XwDosMeshBuild* b, const XwRenderDosMesh* mesh, unsigned face, unsigned axis_u,
					 unsigned axis_v, XwSnapRange* range);
/* Concave polygons retain source winding; zero-area rings produce no triangles. */
bool XwDosMesh_Triangulate(const float (*uv)[2], unsigned count, uint32_t base, XwDosMeshBuild* b);
#endif
