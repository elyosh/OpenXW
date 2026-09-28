#include "xw_dos94/assets/models.h"
#include <limits.h>
#include <string.h>

bool Dos94Models_ReadWord(Dos94ByteView view, size_t offset, uint16_t* out) {
	uint8_t* bytes = Dos94Assets_Data(view);
	if (!bytes || offset > view.size || view.size - offset < 2)
		return false;
	*out = (uint16_t)(bytes[offset] | (uint16_t)bytes[offset + 1] << 8);
	return true;
}

bool Dos94Models_ReadDword(Dos94ByteView view, size_t offset, uint32_t* out) {
	uint16_t low, high;
	if (offset > view.size || view.size - offset < 4 || !Dos94Models_ReadWord(view, offset, &low) ||
		!Dos94Models_ReadWord(view, offset + 2, &high))
		return false;
	*out = low | (uint32_t)high << 16;
	return true;
}

static bool span(Dos94ByteView view, size_t offset, size_t size) {
	return offset <= view.size && size <= view.size - offset;
}

/* DOS93 0x69BC58 / DOS94 0x699C6B: CRFT omits the CPLX vertex normals. */
bool Dos94Models_DecodeMesh(Dos94ByteView payload, uint16_t offset, Dos94MeshLayout layout,
							Dos94MeshView* out) {
	const uint8_t* bytes = Dos94Assets_Data(payload);
	if (!out || !bytes || payload.size > UINT16_MAX || !span(payload, offset, 4))
		return false;
	Dos94MeshView mesh = { .payload = payload, .layout = layout, .offset = offset };
	mesh.format = bytes[offset];
	mesh.color = bytes[offset + 1];
	mesh.vertexCount = bytes[offset + 2];
	if (mesh.format == 0xff) {
		mesh.layout = DOS94_MESH_SIMPLE;
		mesh.coordinateShift = bytes[offset + 3];
		size_t vertices = (size_t)offset + 4;
		if (vertices > UINT16_MAX || !span(payload, vertices, 4 * mesh.vertexCount))
			return false;
		mesh.vertices = (uint16_t)vertices;
	} else if (mesh.format == 0x40 || mesh.format == 0x41) {
		mesh.layout = DOS94_MESH_SIMPLE;
		mesh.edgeCount = bytes[offset + 3];
		mesh.vertices = offset + 4;
		size_t lines = (size_t)offset + 4 + 6 * mesh.vertexCount;
		if (lines > UINT16_MAX || !span(payload, lines, 5 * mesh.edgeCount))
			return false;
		mesh.streams = (uint16_t)lines;
	} else if (mesh.format >= 0x80 && mesh.format <= 0x83) {
		if ((layout != DOS94_MESH_CPLX && layout != DOS94_MESH_CRFT) || !span(payload, offset, 5))
			return false;
		mesh.edgeCount = bytes[offset + 3];
		mesh.faceCount = bytes[offset + 4];
		size_t bounds = (size_t)offset + 5 + mesh.faceCount;
		size_t vertices = bounds + 12;
		size_t normals = vertices + 6 * mesh.vertexCount;
		size_t faces = normals + (layout == DOS94_MESH_CPLX ? 6 * mesh.vertexCount : 0);
		size_t stream = faces + 8 * mesh.faceCount;
		if (stream > UINT16_MAX || !span(payload, bounds, stream - bounds))
			return false;
		for (unsigned i = 0; i < 6; ++i) {
			uint16_t value;
			Dos94Models_ReadWord(payload, bounds + 2 * i, &value);
			mesh.bounds[i] = (int16_t)value;
		}
		mesh.vertices = (uint16_t)vertices;
		mesh.normals = layout == DOS94_MESH_CPLX ? (uint16_t)normals : 0;
		mesh.faces = (uint16_t)faces;
		mesh.streams = (uint16_t)stream;
		for (unsigned i = 0; i < mesh.faceCount; ++i) {
			if (!span(payload, stream, 1))
				return false;
			size_t size = 4 + 2 * (bytes[stream] & (layout == DOS94_MESH_CRFT ? 0x7f : 0x3f));
			if (!span(payload, stream, size))
				return false;
			stream += size;
		}
		if (mesh.format & 2) {
			if (stream > UINT16_MAX || !span(payload, stream, 3 * mesh.faceCount))
				return false;
			mesh.faceBsp = (uint16_t)stream;
			stream += 3 * mesh.faceCount;
		}
		if (stream > UINT16_MAX)
			return false;
		mesh.markings = (uint16_t)stream;
	} else
		return false;
	*out = mesh;
	return true;
}

static bool lod_table(Dos94ByteView payload, uint16_t offset, Dos94MeshLayout layout, Dos94Lod* out,
					  uint16_t* count) {
	size_t cursor = offset;
	*count = 0;
	for (;;) {
		uint32_t depth;
		uint16_t displacement;
		if (!Dos94Models_ReadDword(payload, cursor, &depth) ||
			!Dos94Models_ReadWord(payload, cursor + 4, &displacement))
			return false;
		/* Original addition wraps the offset within its segment. */
		uint16_t data = (uint16_t)(cursor + displacement);
		Dos94MeshView mesh;
		if (!Dos94Models_DecodeMesh(payload, data, layout, &mesh))
			return false;
		if (out)
			out[*count] = (Dos94Lod) { (int32_t)depth, (uint16_t)cursor, mesh };
		++*count;
		if (depth == INT32_MAX)
			return true;
		cursor += 6;
		if (cursor > UINT16_MAX)
			return false;
	}
}

static bool component_offset(Dos94ByteView payload, unsigned bspCount, unsigned component, uint16_t* offset) {
	size_t entry = 2 + 16 * bspCount + 2 * component;
	uint16_t displacement;
	if (!Dos94Models_ReadWord(payload, entry, &displacement))
		return false;
	*offset = (uint16_t)(entry + displacement);
	return true;
}

bool Dos94Models_DecodeLods(Dos94ByteView payload, uint16_t offset, Dos94MeshLayout layout, Dos94Lod** out,
							uint16_t* count) {
	uint16_t entries;
	Dos94ByteView storage;
	if (!out || !count || !lod_table(payload, offset, layout, NULL, &entries) ||
		!Dos94Assets_Allocate(entries * sizeof(Dos94Lod), &storage))
		return false;
	Dos94Lod* lods = (Dos94Lod*)Dos94Assets_Data(storage);
	if (!lod_table(payload, offset, layout, lods, &entries))
		return false;
	*out = lods;
	*count = entries;
	return true;
}

/* DOS93 0x69BC36 / DOS94 0x699C45: both component tables are self-relative;
 * only CPLX has a byte count and component BSP preceding that table. */
bool Dos94Models_Decode(Dos94Model* model) {
	if (!model)
		return false;
	const uint8_t* bytes = Dos94Assets_Data(model->payload);
	if (!bytes || model->payload.size < 2 || model->payload.size > UINT16_MAX)
		return false;
	bool bitmap = !memcmp(model->resourceType, "BTMP", 4) || !memcmp(model->resourceType, "BMAP", 4);
	Dos94MeshLayout layout;
	if (bitmap)
		layout = DOS94_MESH_SIMPLE;
	else if (!memcmp(model->resourceType, "CRFT", 4))
		layout = DOS94_MESH_CRFT;
	else if (!memcmp(model->resourceType, "CPLX", 4))
		layout = DOS94_MESH_CPLX;
	else
		return false;
	uint16_t images = 0;
	unsigned components = bitmap ? 0 : bytes[0];
	unsigned nodes = layout == DOS94_MESH_CPLX ? bytes[1] : 0;
	if (layout == DOS94_MESH_CRFT)
		components |= (unsigned)bytes[1] << 8;
	size_t totalLods = 0;
	if (bitmap) {
		Dos94Models_ReadWord(model->payload, 0, &images);
		if (!span(model->payload, 2, 2 * (size_t)images))
			return false;
	} else {
		if (!components || !span(model->payload, 2, 16 * nodes + 2 * components))
			return false;
		for (unsigned i = 0; i < components; ++i) {
			uint16_t offset, count;
			if (!component_offset(model->payload, nodes, i, &offset) ||
				!lod_table(model->payload, offset, layout, NULL, &count))
				return false;
			totalLods += count;
			if (totalLods > UINT16_MAX)
				return false;
		}
	}
	/* Both headers contain a byte view; place the word-only tables after the aligned LODs. */
	size_t allocationSize = sizeof(Dos94ModelView) + totalLods * sizeof(Dos94Lod) +
							components * sizeof(Dos94ComponentView) + images * sizeof(uint16_t);
	Dos94ByteView storage;
	if (!Dos94Assets_Allocate(allocationSize, &storage))
		return false;
	Dos94ModelView* decoded = (Dos94ModelView*)Dos94Assets_Data(storage);
	memset(decoded, 0, allocationSize);
	decoded->payload = model->payload;
	decoded->layout = layout;
	decoded->componentCount = components;
	decoded->bspNodeCount = nodes;
	decoded->imageCount = images;
	decoded->lodCount = totalLods;
	decoded->lods = (Dos94Lod*)(decoded + 1);
	decoded->components = (Dos94ComponentView*)(decoded->lods + totalLods);
	decoded->imageOffsets = (uint16_t*)(decoded->components + components);
	uint16_t first = 0;
	for (unsigned i = 0; i < components; ++i) {
		uint16_t offset, count;
		if (!component_offset(model->payload, nodes, i, &offset) ||
			!lod_table(model->payload, offset, layout, decoded->lods + first, &count))
			return false;
		decoded->components[i] = (Dos94ComponentView) { first, count };
		first += count;
	}
	for (unsigned i = 0; i < images; ++i) {
		uint16_t offset;
		if (!Dos94Models_ReadWord(model->payload, 2 + 2 * i, &offset) || !span(model->payload, offset, 5))
			return false;
		decoded->imageOffsets[i] = offset;
	}
	model->decoded = decoded;
	return true;
}

const Dos94Lod* Dos94Models_ComponentLods(uint16_t model, uint16_t component, uint16_t* count) {
	Dos94Model* source = Dos94Assets_Model(model);
	if (!source || !Dos94Assets_Data(source->payload) || !source->decoded ||
		!source->decoded->componentCount || !count)
		return NULL;
	if (component >= source->decoded->componentCount) {
		if (source->decoded->layout != DOS94_MESH_CRFT)
			return NULL;
		/* DOS93 DRAW_getcomponentptr clamps geometry, not the published solid index. */
		component = source->decoded->componentCount - 1;
	}
	const Dos94ComponentView* entry = &source->decoded->components[component];
	*count = entry->lodCount;
	return source->decoded->lods + entry->firstLod;
}

bool Dos94Models_ComponentNode(uint16_t model, uint16_t node, Dos94ComponentNode* out) {
	Dos94Model* source = Dos94Assets_Model(model);
	if (!source || !source->decoded || node >= source->decoded->bspNodeCount || !out)
		return false;
	uint16_t words[8];
	for (unsigned i = 0; i < 8; ++i)
		if (!Dos94Models_ReadWord(source->payload, 2 + 16 * node + 2 * i, &words[i]))
			return false;
	for (unsigned i = 0; i < 3; ++i) {
		out->normal[i] = (int16_t)words[i];
		out->point[i] = (int16_t)words[i + 3];
	}
	out->firstChildOffset = words[6];
	out->secondChildOffsetOrComponent = words[7];
	return true;
}

bool Dos94Models_Bitmap(uint16_t model, uint16_t image, Dos94ByteView* out) {
	Dos94Model* source = Dos94Assets_Model(model);
	if (!source || !Dos94Assets_Data(source->payload) || !source->decoded ||
		image >= source->decoded->imageCount)
		return false;
	uint16_t start = source->decoded->imageOffsets[image];
	size_t end = source->payload.size;
	for (unsigned i = 0; i < source->decoded->imageCount; ++i) {
		uint16_t candidate = source->decoded->imageOffsets[i];
		if (candidate > start && candidate < end)
			end = candidate;
	}
	return Dos94Assets_Subview(source->payload, start, end - start, out);
}

/* DOS94 0x687CAD: resolve each coordinate independently; keep payload backlinks intact. */
bool Dos94Models_Vertex(const Dos94MeshView* mesh, uint16_t vertex, int16_t out[3]) {
	if (!mesh || !out || vertex >= mesh->vertexCount)
		return false;
	unsigned axes = mesh->format == 0xff ? 2 : 3;
	for (unsigned axis = 0; axis < axes; ++axis) {
		size_t offset = mesh->vertices + (size_t)vertex * axes * 2 + axis * 2;
		uint16_t value;
		for (;;) {
			if (!Dos94Models_ReadWord(mesh->payload, offset, &value))
				return false;
			if (axes == 2 || (value >> 8) != 0x7f)
				break;
			size_t distance = 3 * (value & 0xff);
			if (!distance || distance > offset)
				return false;
			offset -= distance;
		}
		out[axis] = (int16_t)value;
	}
	if (axes == 2)
		out[2] = 0;
	return true;
}

bool Dos94Models_Face(const Dos94MeshView* mesh, uint16_t face, Dos94FaceView* out) {
	if (!mesh || !out || face >= mesh->faceCount)
		return false;
	const uint8_t* bytes = Dos94Assets_Data(mesh->payload);
	if (!bytes)
		return false;
	size_t record = mesh->faces + 8 * face;
	uint16_t value;
	for (unsigned i = 0; i < 3; ++i) {
		if (!Dos94Models_ReadWord(mesh->payload, record + 2 * i, &value))
			return false;
		out->normal[i] = (int16_t)value;
	}
	if (!Dos94Models_ReadWord(mesh->payload, record + 6, &value))
		return false;
	uint16_t stream = (uint16_t)(record + value);
	if (!span(mesh->payload, stream, 1))
		return false;
	out->color = bytes[mesh->offset + 5 + face];
	out->vertexCount = bytes[stream] & (mesh->layout == DOS94_MESH_CRFT ? 0x7f : 0x3f);
	out->twoSided = (bytes[stream] & 0x80) != 0;
	out->gouraud = mesh->layout == DOS94_MESH_CPLX && (bytes[stream] & 0x40) != 0;
	return Dos94Assets_Subview(mesh->payload, stream, 4 + 2 * out->vertexCount, &out->stream);
}
