/* Cached palette geometry, following OpenTIE classic_mesh.c. Source vertices and
 * normals have already been decoded by X-Wing's bounded model views. */
#include "xw_remaster/dos_mesh_internal.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { MAX_BUFFER_BYTES = 64 * 1024 * 1024 };

typedef char DosVertexLayout[sizeof(XwDosVertex) == 92 ? 1 : -1];
typedef char DosMarkLayout[sizeof(XwDosMark) == 32 ? 1 : -1];

bool XwDosMesh_Grow(void** data, uint32_t* capacity, uint32_t need, size_t stride) {
	if (need <= *capacity)
		return true;
	if (need > MAX_BUFFER_BYTES / stride)
		return false;
	uint32_t next = *capacity ? *capacity : 64;
	while (next < need && next <= MAX_BUFFER_BYTES / stride / 2)
		next *= 2;
	if (next < need)
		next = need;
	void* resized = realloc(*data, next * stride);
	if (!resized)
		return false;
	*data = resized;
	*capacity = next;
	return true;
}

static bool AddLine(XwDosMeshBuild* b, const XwRenderDosMesh* mesh, const XwRenderDosFace* face,
					unsigned first, unsigned second, unsigned width, unsigned color, unsigned flags) {
	if (first >= mesh->vertex_count || second >= mesh->vertex_count ||
		!XwDosMesh_Grow((void**)&b->vertices, &b->vertex_capacity, b->vertex_count + 4,
						sizeof *b->vertices) ||
		!XwDosMesh_Grow((void**)&b->indices, &b->index_capacity, b->index_count + 6, sizeof *b->indices))
		return false;
	static const unsigned corners[6] = { 0, 1, 2, 2, 1, 3 };
	for (unsigned i = 0; i < 6; ++i)
		b->indices[b->index_count++] = b->vertex_count + corners[i];
	for (unsigned i = 0; i < 4; ++i) {
		XwDosVertex v = { .info = { color, flags | XW_DOS_LINE, 0, 0 }, .line = { width, i } };
		for (unsigned a = 0; a < 3; ++a) {
			v.position[a] = mesh->vertices[first][a];
			v.other[a] = mesh->vertices[second][a];
			v.normal[a] = face ? face->normal[a] / 32768.0f : 0;
			v.vertex_normal[a] =
				mesh->authored_normals ? mesh->normals[i / 2 ? second : first][a] / 32768.0f : v.normal[a];
			v.other_normal[a] =
				mesh->authored_normals ? mesh->normals[i / 2 ? first : second][a] / 32768.0f : v.normal[a];
		}
		b->vertices[b->vertex_count++] = v;
	}
	return true;
}

static bool Polygon(XwDosMeshBuild* b, const XwRenderDosMesh* mesh, const XwRenderDosFace* face,
					unsigned id) {
	unsigned count = face ? face->vertex_count : mesh->vertex_count;
	if (count < 3)
		return true;
	if (!XwDosMesh_Grow((void**)&b->vertices, &b->vertex_capacity, b->vertex_count + count,
						sizeof *b->vertices))
		return false;
	/* Dropping the dominant plane axis preserves concavity and authored winding. */
	float normal[3] = { 0, 0, 1 };
	if (face)
		for (unsigned a = 0; a < 3; ++a)
			normal[a] = face->normal[a] / 32768.0f;
	unsigned drop = 0;
	for (unsigned a = 1; a < 3; ++a)
		if (fabsf(normal[a]) > fabsf(normal[drop]))
			drop = a;
	unsigned u = (drop + 1) % 3, v = (drop + 2) % 3;
	XwSnapRange marks = { 0 };
	if (face && !XwDosMesh_Marks(b, mesh, id, u, v, &marks))
		return false;
	unsigned flags = (!(mesh->format & 1) || mesh->format == 0xFF) ? XW_DOS_RAW_COLOR : 0;
	if (!face || face->two_sided)
		flags |= XW_DOS_TWO_SIDED;
	if (face && face->gouraud && mesh->authored_normals)
		flags |= XW_DOS_GOURAUD;
	uint32_t base = b->vertex_count;
	float uv[255][2];
	for (unsigned i = 0; i < count; ++i) {
		unsigned index = face ? face->indices[i] : i;
		if (index >= mesh->vertex_count)
			return false;
		XwDosVertex vertex = { .info = { face ? face->color : mesh->color, flags, marks.first,
										 marks.count } };
		for (unsigned a = 0; a < 3; ++a) {
			vertex.position[a] = mesh->vertices[index][a];
			vertex.normal[a] = normal[a];
			vertex.vertex_normal[a] = mesh->authored_normals ? mesh->normals[index][a] / 32768.0f : normal[a];
		}
		vertex.uv[0] = uv[i][0] = vertex.position[u];
		vertex.uv[1] = uv[i][1] = vertex.position[v];
		b->vertices[b->vertex_count++] = vertex;
	}
	return XwDosMesh_Triangulate(uv, count, base, b);
}

static bool Lod(XwDosMeshBuild* b, const XwRenderDosMesh* mesh, XwDosMeshLod* out) {
	uint32_t first_vertex = b->vertex_count;
	out->triangles.first = b->index_count;
	if (mesh->format == 0xFF) {
		if (!Polygon(b, mesh, NULL, 0))
			return false;
	} else
		for (unsigned i = 0; i < mesh->face_count; ++i)
			if (mesh->faces[i].vertex_count != 2 && !Polygon(b, mesh, &mesh->faces[i], i))
				return false;
	out->triangles.count = b->index_count - out->triangles.first;
	out->lines.first = b->index_count;
	for (unsigned i = 0; i < mesh->face_count; ++i) {
		const XwRenderDosFace* face = &mesh->faces[i];
		if (face->vertex_count != 2)
			continue;
		unsigned flags = face->two_sided ? XW_DOS_TWO_SIDED : 0;
		if (!(mesh->format & 1))
			flags |= XW_DOS_RAW_COLOR;
		if (face->gouraud && mesh->authored_normals)
			flags |= XW_DOS_GOURAUD;
		if (!AddLine(b, mesh, face, face->indices[0], face->indices[1], face->line_width, face->color, flags))
			return false;
	}
	if (mesh->format == 0x40 || mesh->format == 0x41) {
		if (mesh->lines_offset > b->model->size ||
			5u * mesh->edge_count > b->model->size - mesh->lines_offset)
			return false;
		for (unsigned i = 0; i < mesh->edge_count; ++i) {
			const uint8_t* p = b->model->bytes + mesh->lines_offset + 5 * i;
			if (!AddLine(b, mesh, NULL, p[2], p[3], p[0] | (p[1] << 8), p[4],
						 XW_DOS_RAW_COLOR | XW_DOS_TWO_SIDED))
				return false;
		}
	}
	out->lines.count = b->index_count - out->lines.first;
	for (unsigned i = first_vertex; i < b->vertex_count; ++i) {
		const XwDosVertex* vertex = &b->vertices[i];
		for (unsigned endpoint = 0; endpoint < ((unsigned)vertex->info[1] & XW_DOS_LINE ? 2u : 1u);
			 ++endpoint) {
			const float* point = endpoint ? vertex->other : vertex->position;
			float radius = sqrtf(point[0] * point[0] + point[1] * point[1] + point[2] * point[2]);
			out->radius = fmaxf(out->radius, radius + 2 * vertex->line[0]);
		}
	}
	return true;
}

static bool Upload(AeronCommandBuffer* cmd, AeronBuffer** out, const void* data, size_t size,
				   unsigned usage) {
	static const uint8_t empty[32] = { 0 };
	if (!size) {
		size = sizeof empty;
		data = empty;
	}
	*out = Aeron_CreateBuffer(&(AeronBufferDesc) {
		.usage = usage, .size = (uint32_t)size, .memory_usage = AERON_MEMORY_USAGE_GPU_ONLY });
	return *out && Aeron_UploadBufferDataCmd(cmd, *out, 0, data, (uint32_t)size);
}

bool XwDosMesh_Build(AeronCommandBuffer* cmd, const XwRenderSource* source, XwDosMesh* out, char* error,
					 size_t capacity) {
	if (!cmd || !source || source->kind != XW_SOURCE_DOS_MODEL || source->size != sizeof(XwRenderDosModel))
		return false;
	XwDosMeshBuild b = { .model = source->data, .dos93 = source->flight_version == 93 };
	if (b.model->image_count)
		return true;
	bool ok = b.model->lod_count && b.model->lods && b.model->bytes;
	out->lod_count = b.model->lod_count;
	out->lods = calloc(out->lod_count, sizeof *out->lods);
	ok = ok && out->lods;
	for (unsigned i = 0; ok && i < out->lod_count; ++i)
		ok = Lod(&b, &b.model->lods[i], &out->lods[i]);
	size_t vb = b.vertex_count * sizeof *b.vertices, ib = b.index_count * sizeof *b.indices;
	size_t mb = b.mark_count * sizeof *b.marks, mv = b.mark_vertex_count * sizeof *b.mark_vertices;
	ok = ok && vb + ib + mb + mv <= MAX_BUFFER_BYTES;
	if (ok)
		ok = Upload(cmd, &out->vertices, b.vertices, vb, AERON_BUFFER_USAGE_VERTEX) &&
			 Upload(cmd, &out->indices, b.indices, ib, AERON_BUFFER_USAGE_INDEX) &&
			 Upload(cmd, &out->marks, b.marks, mb, AERON_BUFFER_USAGE_STORAGE) &&
			 Upload(cmd, &out->mark_vertices, b.mark_vertices, mv, AERON_BUFFER_USAGE_STORAGE);
	free(b.vertices);
	free(b.indices);
	free(b.marks);
	free(b.mark_vertices);
	if (!ok) {
		XwDosMesh_Destroy(out);
		if (error && capacity)
			snprintf(error, capacity, "DOS mesh conversion/upload failed: %s", source->path);
	}
	return ok;
}

void XwDosMesh_Destroy(XwDosMesh* mesh) {
	Aeron_DestroyBuffer(mesh->vertices);
	Aeron_DestroyBuffer(mesh->indices);
	Aeron_DestroyBuffer(mesh->marks);
	Aeron_DestroyBuffer(mesh->mark_vertices);
	free(mesh->lods);
	memset(mesh, 0, sizeof *mesh);
}
