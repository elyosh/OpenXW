#include "xw_remaster/special_world_internal.h"
#include <math.h>
#include <string.h>

static XwDosMesh dos_quad;
static AeronSceneMesh* windows_meshes[68];
static XwRenderAssetId layout_id, texture_ids[2];
static AeronSceneMeshTable components[XW_SNAP_COMPONENTS];
static bool components_ready;

const AeronSceneMeshTable* XwWorld_Component(unsigned component) {
	if (component >= XW_SNAP_COMPONENTS)
		return NULL;
	if (!components_ready) {
		for (unsigned i = 0; i < XW_SNAP_COMPONENTS; ++i) {
			AeronSceneMeshTable* t = &components[i];
			t->rows[i][0][0] = t->rows[i][1][1] = t->rows[i][2][2] = 1;
			t->visibility_packed[i >> 2][i & 3] = t->emissive_packed[i >> 2][i & 3] = 1;
		}
		components_ready = true;
	}
	return &components[component];
}

bool XwWorld_DosMeshes(AeronCommandBuffer* cmd) {
	if (dos_quad.vertices)
		return true;
	XwRenderDosMesh lods[17] = { 0 };
	for (unsigned i = 0; i < 17; ++i) {
		lods[i].format = 0xFF;
		lods[i].color = 0xB0 + (i == 16 ? 1 : i);
		lods[i].vertex_count = 4;
		lods[i].vertices[1][0] = 1;
		lods[i].vertices[2][0] = i == 16 ? 17 : 1;
		lods[i].vertices[2][1] = lods[i].vertices[3][1] = 1;
		lods[i].vertices[3][0] = i == 16 ? -15 : 0;
	}
	uint8_t empty = 0;
	XwRenderDosModel model = { .lod_count = 17, .lods = lods, .bytes = &empty, .size = 1 };
	XwRenderSource source = { .kind = XW_SOURCE_DOS_MODEL, .data = &model, .size = sizeof model };
	char error[128];
	return XwDosMesh_Build(cmd, &source, &dos_quad, error, sizeof error);
}

static void DestroyWindows(void) {
	for (unsigned i = 0; i < 68; ++i) {
		/* Atlas textures are borrowed from the committed source cache. */
		if (windows_meshes[i])
			windows_meshes[i]->atlas[0] = NULL;
		AeronScene_MeshDestroy(windows_meshes[i]);
		windows_meshes[i] = NULL;
	}
	layout_id = texture_ids[0] = texture_ids[1] = 0;
}

static AeronSceneMesh* WindowsMesh(AeronCommandBuffer* cmd, const XwRenderWorldLayout* layout, unsigned index,
								   const AeronRuntimeAtlas* atlas) {
	if (!atlas || atlas->layout.frame_count != 1 || !atlas->pages)
		return NULL;
	AeronGltfVertex vertices[12] = { 0 };
	uint16_t indices[18];
	unsigned faces = index == 67 ? 3 : 1;
	for (unsigned face = 0; face < faces; ++face) {
		XwRenderWorldQuad quad = layout->quads[index == 67 ? face + 1 : 0];
		if (index > 0 && index < 67) {
			bool left = index < 34;
			int center = ((int)(left ? index - 1 : index - 34) - 16) * 65536;
			float edge = (left ? -3072 : 3072) - center;
			for (unsigned v = 0; v < 4; ++v) {
				if ((left && v < 2) || (!left && v >= 2)) {
					quad.uv[v][0] += (edge - quad.positions[v][0]) / 65536.0f;
					quad.positions[v][0] = edge;
				}
			}
		}
		for (unsigned v = 0; v < 4; ++v) {
			AeronGltfVertex* out = &vertices[4 * face + v];
			memcpy(out->pos, quad.positions[v], sizeof out->pos);
			memcpy(out->normal, quad.normal, sizeof out->normal);
			memcpy(out->uv, quad.uv[v], sizeof out->uv);
			out->tangent[quad.normal[2] ? 0 : 1] = 1;
			out->tangent[3] = 1;
		}
		static const uint16_t ring[6] = { 0, 1, 2, 0, 2, 3 };
		for (unsigned i = 0; i < 6; ++i)
			indices[face * 6 + i] = face * 4 + ring[i];
	}
	const AeronSpriteRect* rect = &atlas->layout.frames[0];
	unsigned page = atlas->layout.pages ? atlas->layout.pages[0] : 0;
	if (page >= (unsigned)atlas->layout.page_count || !atlas->pages[page].texture)
		return NULL;
	AeronGltfMaterial material = { .base_color_factor = { 1, 1, 1, 1 },
								   .roughness_factor = 1,
								   .double_sided = 1 };
	material.uv_xform[0][0] = rect->x / atlas->pages[page].width;
	material.uv_xform[0][1] = rect->y / atlas->pages[page].height;
	material.uv_xform[0][2] = rect->w / atlas->pages[page].width;
	material.uv_xform[0][3] = rect->h / atlas->pages[page].height;
	uint32_t variant = 0;
	AeronFlightModel model = { .render = { .vertices = vertices,
										   .vertex_count = faces * 4,
										   .indices = indices,
										   .index_count = faces * 6,
										   .opaque_index_count = faces * 6,
										   .mask_index_offset = faces * 6,
										   .blend_index_offset = faces * 6,
										   .materials = &material,
										   .material_count = 1,
										   .total_prim_count = 1,
										   .variant_slots = 1,
										   .prim_variant_material = &variant } };
	float minimum[3], maximum[3];
	memcpy(minimum, vertices[0].pos, sizeof minimum);
	memcpy(maximum, minimum, sizeof maximum);
	for (unsigned v = 1; v < faces * 4; ++v)
		for (unsigned a = 0; a < 3; ++a) {
			minimum[a] = fminf(minimum[a], vertices[v].pos[a]);
			maximum[a] = fmaxf(maximum[a], vertices[v].pos[a]);
		}
	model.bounds.min = (AeronFlightVec3) { minimum[0], minimum[1], minimum[2] };
	model.bounds.max = (AeronFlightVec3) { maximum[0], maximum[1], maximum[2] };
	AeronSceneMesh* mesh = AeronScene_MeshCreate(cmd, &model, "X-Wing special world", NULL);
	if (mesh)
		mesh->atlas[0] = atlas->pages[page].texture;
	return mesh;
}

bool XwWorld_WindowsMeshes(AeronCommandBuffer* cmd, const XwWorldBuild* b) {
	XwRenderAssetId surface = b->assets->surface_texture, trench = b->assets->trench_texture;
	const AeronRuntimeAtlas* images[2] = { XwRemasterAssets_Image(surface), XwRemasterAssets_Image(trench) };
	if (!surface || !trench || !images[0] || !images[1])
		return false;
	if (layout_id != b->assets->special_layout || texture_ids[0] != surface || texture_ids[1] != trench)
		DestroyWindows();
	/* Refresh borrowed textures when a staged cache replacement rebuilds the same source ID. */
	for (unsigned i = 0; i < b->quad_count; ++i) {
		unsigned index = b->quads[i].mesh;
		if (index >= 68)
			return false;
		const AeronRuntimeAtlas* image = images[index == 67];
		if (!windows_meshes[index])
			windows_meshes[index] = WindowsMesh(cmd, b->layout, index, image);
		if (!windows_meshes[index])
			return false;
		unsigned page = image->layout.pages ? image->layout.pages[0] : 0;
		windows_meshes[index]->atlas[0] = image->pages[page].texture;
	}
	layout_id = b->assets->special_layout;
	texture_ids[0] = surface;
	texture_ids[1] = trench;
	return true;
}

const XwDosMesh* XwWorld_DosQuad(void) { return &dos_quad; }

const AeronSceneMesh* XwWorld_WindowsQuad(unsigned index) {
	return index < 68 ? windows_meshes[index] : NULL;
}

void XwWorld_MeshesShutdown(void) {
	DestroyWindows();
	XwDosMesh_Destroy(&dos_quad);
}
