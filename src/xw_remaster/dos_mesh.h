#ifndef XW_REMASTER_DOS_MESH_H
#define XW_REMASTER_DOS_MESH_H
#include "xw_runtime/snapshot/render_dos_assets.h"
#include <aeron/render.h>

enum { XW_DOS_RAW_COLOR = 1, XW_DOS_TWO_SIDED = 2, XW_DOS_GOURAUD = 4, XW_DOS_LINE = 8 };

/* One layout for polygons and screen-expanded lines, mirroring dos_mesh.vert. */
typedef struct XwDosVertex {
	float position[3], other[3], normal[3], vertex_normal[3], other_normal[3], uv[2];
	float info[4]; /* Original color, flags, first marking, marking count. */
	float line[2]; /* Authored width and quad corner (endpoint * 2 + side). */
} XwDosVertex;

typedef struct XwDosMark {
	uint32_t first, count, color, width;
	float max_depth, anchor[3];
} XwDosMark;

typedef struct XwDosMeshLod {
	XwSnapRange triangles, lines;
	float radius; /* Conservative source-space sphere about the component origin. */
} XwDosMeshLod;

typedef struct XwDosMesh {
	AeronBuffer *vertices, *indices, *marks, *mark_vertices;
	uint16_t lod_count;
	XwDosMeshLod* lods;
} XwDosMesh;

bool XwDosMesh_Build(AeronCommandBuffer* cmd, const XwRenderSource* source, XwDosMesh* out, char* error,
					 size_t capacity);
void XwDosMesh_Destroy(XwDosMesh* mesh);
#endif
