#include "xw_runtime/snapshot/render_dos_assets.h"
#include "xw_dos94/assets/models.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static XwRenderAssetId geometry[DOS94_MODEL_COUNT], bitmaps[DOS94_MODEL_COUNT];

void XwRenderDosAssets_Reset(void) {
	memset(geometry, 0, sizeof geometry);
	memset(bitmaps, 0, sizeof bitmaps);
}

static uint8_t Version(void) { return XwGameVersion_Year(Dos94Assets_Version()) - 1900; }

static void Destroy(void* data) {
	XwRenderDosModel* model = data;
	if (!model)
		return;
	for (unsigned i = 0; i < model->lod_count; ++i)
		free(model->lods[i].faces);
	free(model->lods);
	free(model->components);
	free(model->nodes);
	free(model->image_offsets);
	free(model->bytes);
	free(model);
}

static bool Mesh(XwRenderDosMesh* out, const Dos94Lod* lod) {
	const Dos94MeshView* m = &lod->mesh;
	*out = (XwRenderDosMesh) { .max_depth = lod->maxDepth,
							   .format = m->format,
							   .color = m->color,
							   .coordinate_shift = m->coordinateShift,
							   .vertex_count = m->vertexCount,
							   .face_count = m->faceCount,
							   .edge_count = m->edgeCount,
							   .markings_offset = m->markings,
							   .lines_offset = m->streams,
							   .face_bsp_offset = m->faceBsp,
							   .authored_normals = m->normals != 0 };
	memcpy(out->bounds, m->bounds, sizeof out->bounds);
	const uint8_t* payload = Dos94Assets_Data(m->payload);
	if (!payload)
		return false;
	if (m->offset + 4u < m->payload.size)
		out->detail_count = payload[m->offset + 4];
	for (unsigned v = 0; v < m->vertexCount; ++v) {
		if (!Dos94Models_Vertex(m, v, out->vertices[v]))
			return false;
		if (m->normals)
			for (unsigned a = 0; a < 3; ++a) {
				uint16_t value;
				if (!Dos94Models_ReadWord(m->payload, m->normals + 6 * v + 2 * a, &value))
					return false;
				out->normals[v][a] = (int16_t)value;
			}
	}
	if (!m->faceCount)
		return true;
	out->faces = calloc(m->faceCount, sizeof *out->faces);
	if (!out->faces)
		return false;
	for (unsigned f = 0; f < m->faceCount; ++f) {
		Dos94FaceView face;
		if (!Dos94Models_Face(m, f, &face))
			return false;
		const uint8_t* ring = Dos94Assets_Data(face.stream);
		if (!ring || face.vertexCount > 127)
			return false;
		XwRenderDosFace* dest = &out->faces[f];
		memcpy(dest->normal, face.normal, sizeof dest->normal);
		dest->color = face.color;
		dest->vertex_count = face.vertexCount;
		dest->two_sided = face.twoSided;
		dest->gouraud = face.gouraud;
		dest->stream_offset = face.stream.offset - m->payload.offset;
		if (face.vertexCount == 2)
			dest->line_width = (uint16_t)(ring[1] | (ring[2] << 8));
		for (unsigned v = 0; v < face.vertexCount; ++v) {
			dest->indices[v] = ring[face.vertexCount == 2 ? 3 + v : 1 + 2 * v];
			if (dest->indices[v] >= m->vertexCount)
				return false;
		}
	}
	return true;
}

static XwRenderDosModel* CopyModel(Dos94ByteView payload, const Dos94Lod* lods, unsigned lod_count,
								   const Dos94ModelView* decoded, uint16_t type) {
	const uint8_t* bytes = Dos94Assets_Data(payload);
	if (!bytes || !payload.size || lod_count > 4096)
		return NULL;
	XwRenderDosModel* out = calloc(1, sizeof *out);
	if (!out)
		return NULL;
	out->size = payload.size;
	out->bytes = malloc(payload.size);
	out->lods = calloc(lod_count ? lod_count : 1, sizeof *out->lods);
	if (!out->bytes || !out->lods) {
		Destroy(out);
		return NULL;
	}
	memcpy(out->bytes, bytes, payload.size);
	out->lod_count = lod_count;
	for (unsigned i = 0; i < lod_count; ++i)
		if (!Mesh(&out->lods[i], &lods[i])) {
			Destroy(out);
			return NULL;
		}
	out->component_count = decoded ? decoded->componentCount : 1;
	out->clamp_components = decoded && decoded->layout == DOS94_MESH_CRFT;
	out->image_count = decoded ? decoded->imageCount : 0;
	out->node_count = decoded ? decoded->bspNodeCount : 0;
	out->components = calloc(out->component_count ? out->component_count : 1, sizeof *out->components);
	out->image_offsets = calloc(out->image_count ? out->image_count : 1, sizeof *out->image_offsets);
	out->nodes = calloc(out->node_count ? out->node_count : 1, sizeof *out->nodes);
	if (!out->components || !out->image_offsets || !out->nodes) {
		Destroy(out);
		return NULL;
	}
	for (unsigned i = 0; i < out->component_count; ++i)
		out->components[i] =
			decoded ? (XwSnapRange) { decoded->components[i].firstLod, decoded->components[i].lodCount }
					: (XwSnapRange) { 0, lod_count };
	if (decoded && out->image_count)
		memcpy(out->image_offsets, decoded->imageOffsets, out->image_count * sizeof *out->image_offsets);
	for (unsigned i = 0; i < out->node_count; ++i) {
		Dos94ComponentNode node;
		if (!Dos94Models_ComponentNode(type, i, &node)) {
			Destroy(out);
			return NULL;
		}
		memcpy(out->nodes[i].normal, node.normal, sizeof node.normal);
		memcpy(out->nodes[i].point, node.point, sizeof node.point);
		out->nodes[i].first_child = node.firstChildOffset;
		out->nodes[i].second_child_or_component = node.secondChildOffsetOrComponent;
	}
	return out;
}

static void Bind(uint16_t type) {
	const Dos94Model* model = Dos94Assets_Model(type);
	const Dos94ModelMetadata* m = &model->metadata;
	char name[64];
	unsigned variants = 0;
	for (unsigned i = 0; i < m->descriptorCount; ++i)
		variants += m->components[i].stateCount;
	size_t size = sizeof(XwRenderDosDescriptors) + variants * sizeof(uint16_t);
	XwRenderDosDescriptors* out = calloc(1, size);
	if (!out) {
		XwRenderAssets_Fail("DOS components", "allocation failed");
		return;
	}
	out->count = m->descriptorCount;
	out->component_count = m->componentCount;
	out->variant_count = variants;
	unsigned first = 0;
	for (unsigned i = 0; i < m->descriptorCount; ++i) {
		const Dos94Component* c = &m->components[i];
		out->descriptors[i] =
			(XwRenderDosDescriptor) { .variants = { first, c->stateCount },
									  .bitmap_scale = c->bitmapScale,
									  .eye_offset = { c->eyeOffsetX, c->eyeOffsetY, c->eyeOffsetZ },
									  .first_child = c->nextChildIndex,
									  .child_count = c->childCount };
		if (c->stateCount)
			memcpy(out->variants + first, c->stateVariants, c->stateCount * sizeof(uint16_t));
		first += c->stateCount;
	}
	snprintf(name, sizeof name, "dos/components/%u", type);
	XwRenderAssetId components =
		XwRenderAssets_RegisterBytes(XW_SOURCE_DOS_COMPONENTS, 0, name, Version(), out, size);
	free(out);
	XwRenderAssetId remap = 0, alternate = 0;
	if (m->bitmapPalette) {
		snprintf(name, sizeof name, "dos/remap/%u", type);
		remap = XwRenderAssets_RegisterBytes(XW_SOURCE_PALETTE, 0, name, Version(), m->bitmapPalette, 16);
	}
	const uint8_t* bytes = Dos94Assets_Data(model->alternateBitmap);
	if (bytes && model->alternateBitmap.size) {
		snprintf(name, sizeof name, "dos/alternate/%u", type);
		alternate = XwRenderAssets_RegisterBytes(XW_SOURCE_BITMAP, 0, name, Version(), bytes,
												 model->alternateBitmap.size);
	}
	XwRenderAssets_BindDos(type, geometry[type], bitmaps[type], alternate, remap, components);
}

void XwRenderDosAssets_Register(uint16_t type, const char* path, uint8_t installation) {
	const Dos94Model* model = Dos94Assets_Model(type);
	if (!model || !model->decoded)
		return;
	XwRenderDosModel* out =
		CopyModel(model->payload, model->decoded->lods, model->decoded->lodCount, model->decoded, type);
	if (!out) {
		XwRenderAssets_Fail(path, "cannot normalize DOS model");
		return;
	}
	/* Archive members have independent IDs even when their names are reused. */
	char identity[XW_PATH_CAPACITY];
	int written = snprintf(identity, sizeof identity, "%s/model-%u", path, type);
	if (written < 0 || (size_t)written >= sizeof identity) {
		Destroy(out);
		XwRenderAssets_Fail(path, "source path too long");
		return;
	}
	bool bitmap = out->image_count != 0;
	XwRenderAssetId id =
		XwRenderAssets_RegisterOwned(XW_SOURCE_DOS_MODEL, identity, installation, out, sizeof *out, Destroy);
	geometry[type] = bitmap ? 0 : id;
	bitmaps[type] = bitmap ? id : 0;
	Bind(type);
}

void XwRenderDosAssets_Alias(uint16_t type, uint16_t original) {
	if (type >= DOS94_MODEL_COUNT || original >= DOS94_MODEL_COUNT)
		return;
	geometry[type] = geometry[original];
	bitmaps[type] = bitmaps[original];
	Bind(type);
}

void XwRenderDosAssets_Resident(void) {
	/* Resident-only types (notably detached components) still own descriptor programs. */
	for (unsigned type = 0; type < DOS94_MODEL_COUNT; ++type)
		Bind(type);
	for (unsigned type = 18; type <= 25; ++type) {
		/* Green and ion pairs intentionally share geometry. */
		if (type == 21 || type == 23) {
			geometry[type] = geometry[type - 1];
			Bind(type);
			continue;
		}
		uint16_t count = 0;
		const Dos94Lod* lods = Dos94Models_ProjectileLods(type, &count);
		if (!lods || !count)
			continue;
		XwRenderDosModel* out = CopyModel(lods[0].mesh.payload, lods, count, NULL, type);
		char name[48];
		snprintf(name, sizeof name, "dos/resident/projectile-%u", type);
		if (!out) {
			XwRenderAssets_Fail(name, "cannot normalize resident model");
			continue;
		}
		geometry[type] =
			XwRenderAssets_RegisterOwned(XW_SOURCE_DOS_MODEL, name, Version(), out, sizeof *out, Destroy);
		Bind(type);
	}
	Dos94MeshView mesh;
	if (Dos94Models_Hyperstar(&mesh)) {
		Dos94Lod lod = { .maxDepth = INT32_MAX, .mesh = mesh };
		XwRenderDosModel* out = CopyModel(mesh.payload, &lod, 1, NULL, 0);
		if (out)
			XwRenderAssets_RegisterOwned(XW_SOURCE_DOS_MODEL, "dos/resident/hyperstar", Version(), out,
										 sizeof *out, Destroy);
		else
			XwRenderAssets_Fail("hyperstar", "cannot normalize resident model");
	}
}

void XwRenderDosAssets_Materials(const XwRenderDosMaterials* data) {
	XwRenderAssets_RegisterBytes(XW_SOURCE_DOS_MATERIALS, 0, "dos/materials", Version(), data, sizeof *data);
}
