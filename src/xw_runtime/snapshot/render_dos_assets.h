#ifndef XW_RENDER_DOS_ASSETS_H
#define XW_RENDER_DOS_ASSETS_H
#include "xw_runtime/snapshot/render_assets.h"

typedef struct XwRenderDosFace {
	int16_t normal[3];
	uint8_t color, vertex_count, indices[127];
	bool two_sided, gouraud;
	uint16_t stream_offset, line_width;
} XwRenderDosFace;

typedef struct XwRenderDosMesh {
	int32_t max_depth;
	int16_t bounds[6];
	uint8_t format, color, coordinate_shift, vertex_count, face_count, edge_count, detail_count;
	uint16_t markings_offset, lines_offset, face_bsp_offset;
	bool authored_normals;
	int16_t vertices[255][3], normals[255][3];
	XwRenderDosFace* faces;
} XwRenderDosMesh;

typedef struct XwRenderDosNode {
	int16_t normal[3], point[3];
	uint16_t first_child, second_child_or_component;
} XwRenderDosNode;

typedef struct XwRenderDosModel {
	uint16_t component_count, lod_count, image_count, node_count;
	bool clamp_components; /* CRFT aliases clamp geometry while retaining descriptor identity. */
	XwSnapRange* components;
	XwRenderDosMesh* lods;
	XwRenderDosNode* nodes;
	uint16_t* image_offsets;
	/* Owned bounded data for bitmap commands, face markings and line attributes. */
	uint8_t* bytes;
	size_t size;
} XwRenderDosModel;

typedef struct XwRenderDosDescriptor {
	XwSnapRange variants;
	uint16_t bitmap_scale;
	int16_t eye_offset[3];
	uint8_t first_child, child_count;
} XwRenderDosDescriptor;

typedef struct XwRenderDosDescriptors {
	uint16_t count, variant_count, component_count;
	XwRenderDosDescriptor descriptors[256];
	uint16_t variants[];
} XwRenderDosDescriptors;

typedef struct XwRenderDosMaterials {
	uint8_t colors[624], target[39], offsets[64], markings[48], gate[4];
	uint8_t material_count;
} XwRenderDosMaterials;

void XwRenderDosAssets_Reset(void);
void XwRenderDosAssets_Register(uint16_t type, const char* path, uint8_t installation);
void XwRenderDosAssets_Alias(uint16_t type, uint16_t original);
void XwRenderDosAssets_Resident(void);
/* Called beside immutable resident definitions, before per-object mutations. */
void XwRenderDosAssets_Materials(const XwRenderDosMaterials* materials);
#endif
