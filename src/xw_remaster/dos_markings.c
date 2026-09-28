/* Face-local overlays follow OpenTIE's ordered fragment lookup. X-Wing operands
 * and depth cutoffs come from xw_dos94/render/markings.c, not TIE's blob parser. */
#include "xw_remaster/dos_mesh_internal.h"
#include <limits.h>

static bool Span(const XwRenderDosModel* model, size_t first, size_t size) {
	return first <= model->size && size <= model->size - first;
}

static unsigned Word(const uint8_t* p) { return p[0] | ((unsigned)p[1] << 8); }

static bool Source(const XwRenderDosModel* model, const XwRenderDosMesh* mesh, unsigned face, unsigned* out) {
	*out = 0;
	if (!mesh->markings_offset || !Span(model, mesh->markings_offset, 2))
		return true;
	unsigned count = Word(model->bytes + mesh->markings_offset);
	if (!count || (int16_t)count < 0)
		return true;
	if (count >= 127 || !Span(model, mesh->markings_offset + 2, 3 * count))
		return false;
	for (unsigned i = 0; i < count; ++i) {
		unsigned offset = mesh->markings_offset + 2 + 3 * i;
		if (model->bytes[offset] == face)
			*out = (uint16_t)(offset + Word(model->bytes + offset + 1));
	}
	return true;
}

bool XwDosMesh_Marks(XwDosMeshBuild* b, const XwRenderDosMesh* mesh, unsigned id, unsigned axis_u,
					 unsigned axis_v, XwSnapRange* range) {
	*range = (XwSnapRange) { b->mark_count, 0 };
	unsigned source;
	if (!Source(b->model, mesh, id, &source))
		return false;
	if (!source)
		return true;
	if (!Span(b->model, source, 1))
		return false;
	const uint8_t* bytes = b->model->bytes;
	unsigned total = bytes[source];
	if (!total)
		return true;
	if (total > (b->dos93 ? 7u : 16u) || !Span(b->model, source + 1, total))
		return false;
	const XwRenderDosFace* face = &mesh->faces[id];
	unsigned cursor = source + 1 + total;
	float max_depth = (float)INT32_MAX;
	for (unsigned i = 0; i < total; ++i) {
		if (!Span(b->model, cursor, 1))
			return false;
		if (bytes[cursor] == 255) {
			if (!Span(b->model, cursor, 5))
				return false;
			int32_t limit = (int32_t)(Word(bytes + cursor + 1) | (Word(bytes + cursor + 3) << 16));
			if (limit < max_depth)
				max_depth = limit;
			cursor += 5;
		}
		if (!Span(b->model, cursor, 1))
			return false;
		unsigned count = bytes[cursor++], width = count > 16 ? count - 16 : 0;
		if (width)
			count = 2;
		if (!count || !Span(b->model, cursor, 3 * count) ||
			!XwDosMesh_Grow((void**)&b->marks, &b->mark_capacity, b->mark_count + 1, sizeof *b->marks) ||
			!XwDosMesh_Grow((void**)&b->mark_vertices, &b->mark_vertex_capacity, b->mark_vertex_count + count,
							sizeof *b->mark_vertices))
			return false;
		XwDosMark mark = { .first = b->mark_vertex_count,
						   .count = count,
						   .color = bytes[source + 1 + i],
						   .width = width,
						   .max_depth = max_depth };
		for (unsigned a = 0; a < 3; ++a)
			mark.anchor[a] = mesh->vertices[face->indices[0]][a];
		for (unsigned v = 0; v < count; ++v) {
			const uint8_t* operand = bytes + cursor + 3 * v;
			unsigned offset = operand[0] + 1;
			if (offset < 2 || offset + 2 >= 4u + 2u * face->vertex_count ||
				!Span(b->model, face->stream_offset + offset - 2, 5) || ((operand[1] | operand[2]) & 1) ||
				operand[1] > 30 || operand[2] > 30)
				return false;
			const uint8_t* ring = bytes + face->stream_offset;
			unsigned prev = ring[offset - 2], base = ring[offset], next = ring[offset + 2];
			if (prev >= mesh->vertex_count || base >= mesh->vertex_count || next >= mesh->vertex_count)
				return false;
			float* point = b->mark_vertices[b->mark_vertex_count];
			point[2] = 0;
			point[3] = 1;
			/* Line coverage projects the complete endpoints, including the dropped face axis. */
			for (unsigned a = 0; a < (count == 2 ? 3u : 2u); ++a) {
				unsigned axis = count == 2 ? a : a ? axis_v : axis_u;
				float p = mesh->vertices[base][axis];
				point[a] = p + (mesh->vertices[prev][axis] - p) * (operand[1] / 32.0f) +
						   (mesh->vertices[next][axis] - p) * (operand[2] / 32.0f);
			}
			++b->mark_vertex_count;
		}
		cursor += 3 * count;
		b->marks[b->mark_count++] = mark;
		++range->count;
	}
	return true;
}
