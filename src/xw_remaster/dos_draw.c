/* OpenTIE custom-pass pattern: immutable mesh buffers plus uploaded component tables. */
#include "xw_remaster/dos_draw.h"
#include "xw_remaster/assets.h"
#include "xw_remaster/dos_mesh_internal.h"
#include "xw_remaster/ship.h"
#include <aeron/aeron.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct DosTable {
	float transform[12], light[4];
} DosTable;

typedef struct DosDrawUniform {
	float clip_x[4], clip_y[4], depth_row[4], options[4], pixels[4], policy[4];
} DosDrawUniform;

typedef struct DosViewUniform {
	float view_proj[16], pixels[4], camera[4];
} DosViewUniform;

typedef struct DosDraw {
	const XwDosMesh* mesh;
	uint16_t lod;
	bool projectile;

	struct {
		uint32_t index;
		float endpoints[2], color;
	} vertex_uniform;

	DosDrawUniform uniform;
} DosDraw;

static AeronShader *vertex_shader, *fragment_shader;
static AeronGraphicsPipeline *opaque, *bolt;
static AeronSampleCount samples;
static AeronBuffer* tables;
static AeronTexture *palette, *materials;
static AeronSampler* sampler;
static DosTable* cpu_tables;
static DosDraw* draws;
static uint32_t count, table_capacity, draw_capacity, gpu_capacity;
static DosViewUniform view_uniform;
static XwRenderView current_view;
static uint8_t version, material_count;
static uint16_t target;
static bool gouraud;
static uint8_t gates[4];

typedef char DosTableLayout[sizeof(DosTable) == 64 ? 1 : -1];
typedef char DosDrawLayout[sizeof(DosDrawUniform) == 96 ? 1 : -1];

static AeronGraphicsPipeline* Pipeline(AeronSampleCount msaa, bool depth_write) {
	AeronVertexAttributeDesc attributes[8];
	static const uint32_t offsets[] = {
		offsetof(XwDosVertex, position),     offsetof(XwDosVertex, other),
		offsetof(XwDosVertex, normal),       offsetof(XwDosVertex, vertex_normal),
		offsetof(XwDosVertex, other_normal), offsetof(XwDosVertex, uv),
		offsetof(XwDosVertex, info),         offsetof(XwDosVertex, line)
	};
	for (unsigned i = 0; i < 8; ++i)
		attributes[i] = (AeronVertexAttributeDesc) { .location = i,
													 .offset = offsets[i],
													 .format = i < 5    ? AERON_VERTEX_FORMAT_FLOAT3
															   : i == 6 ? AERON_VERTEX_FORMAT_FLOAT4
																		: AERON_VERTEX_FORMAT_FLOAT2 };
	AeronVertexBufferLayoutDesc buffer = { .stride = sizeof(XwDosVertex) };
	AeronColorTargetStateDesc color = { .format = AERON_TEXTURE_FORMAT_RGBA16_FLOAT };
	return Aeron_CreateGraphicsPipeline(&(AeronGraphicsPipelineDesc) {
		.vertex_shader = vertex_shader,
		.fragment_shader = fragment_shader,
		.primitive_type = AERON_PRIMITIVE_TRIANGLES,
		.cull_mode = AERON_CULL_NONE,
		.vertex_buffers = &buffer,
		.vertex_buffer_count = 1,
		.attributes = attributes,
		.attribute_count = 8,
		.sample_count = msaa,
		.depth_format = AERON_TEXTURE_FORMAT_D32_FLOAT,
		.depth = { .depth_test = 1, .depth_write = depth_write, .compare = AERON_COMPARE_GREATER_EQUAL },
		.color_targets = &color,
		.color_target_count = 1 });
}

bool XwDosDraw_PrepareResources(AeronSampleCount msaa) {
	if (!vertex_shader)
		vertex_shader = Aeron_CreateShader(&(AeronShaderDesc) { .name = "dos_mesh.vert",
																.stage = AERON_SHADER_STAGE_VERTEX,
																.uniform_buffer_count = 2,
																.storage_buffer_count = 1 });
	if (!fragment_shader)
		fragment_shader = Aeron_CreateShader(&(AeronShaderDesc) { .name = "dos_mesh.frag",
																  .stage = AERON_SHADER_STAGE_FRAGMENT,
																  .uniform_buffer_count = 1,
																  .storage_buffer_count = 2,
																  .sampler_count = 2 });
	if (!vertex_shader || !fragment_shader)
		return false;
	if (!opaque || samples != msaa) {
		AeronGraphicsPipeline* next_opaque = Pipeline(msaa, true);
		AeronGraphicsPipeline* next_bolt = Pipeline(msaa, false);
		if (!next_opaque || !next_bolt) {
			Aeron_DestroyGraphicsPipeline(next_opaque);
			Aeron_DestroyGraphicsPipeline(next_bolt);
			return false;
		}
		Aeron_DestroyGraphicsPipeline(opaque);
		Aeron_DestroyGraphicsPipeline(bolt);
		opaque = next_opaque;
		bolt = next_bolt;
		samples = msaa;
	}
	if (!sampler)
		sampler = Aeron_CreateSampler(&(AeronSamplerDesc) { .min_filter = AERON_FILTER_NEAREST,
															.mag_filter = AERON_FILTER_NEAREST,
															.address_u = AERON_ADDRESS_CLAMP_TO_EDGE,
															.address_v = AERON_ADDRESS_CLAMP_TO_EDGE });
	if (!palette)
		palette = Aeron_CreateTexture(
			&(AeronTextureDesc) { .width = 256,
								  .height = 1,
								  .mip_count = 1,
								  .format = AERON_TEXTURE_FORMAT_RGBA8_SRGB,
								  .usage = AERON_TEXTURE_USAGE_SAMPLED | AERON_TEXTURE_USAGE_TRANSFER_DST });
	if (!materials)
		materials = Aeron_CreateTexture(
			&(AeronTextureDesc) { .width = 16,
								  .height = 46,
								  .mip_count = 1,
								  .format = AERON_TEXTURE_FORMAT_R8_UNORM,
								  .usage = AERON_TEXTURE_USAGE_SAMPLED | AERON_TEXTURE_USAGE_TRANSFER_DST });
	return sampler && palette && materials;
}

bool XwDosDraw_Begin(AeronCommandBuffer* cmd, const XwRenderSnapshot* s, const XwRenderView* view,
					 AeronSampleCount msaa) {
	count = 0;
	const XwRenderAssetSetView* set = XwRenderAssets_Set(s->flight_assets);
	const XwRenderSource* source = set ? XwRenderAssets_Source(set->bindings.dos_materials) : NULL;
	if (!source || source->kind != XW_SOURCE_DOS_MATERIALS || source->size != sizeof(XwRenderDosMaterials) ||
		!XwDosDraw_PrepareResources(msaa))
		return false;
	const XwRenderDosMaterials* m = source->data;
	uint8_t colors[256][4], ramps[46][16] = { 0 };
	memcpy(ramps, m->colors, sizeof m->colors);
	memcpy(ramps + 39, m->markings, sizeof m->markings);
	memcpy(ramps[42], m->offsets, 16);
	memcpy(ramps[43], m->target, sizeof m->target);
	for (unsigned i = 0; i < 256; ++i) {
		uint32_t color = s->appearance.palette_argb[i];
		colors[i][0] = color >> 16;
		colors[i][1] = color >> 8;
		colors[i][2] = color;
		colors[i][3] = 255;
	}
	if (!Aeron_UploadTextureDataCmd(cmd, &(AeronTextureUploadDesc) { .texture = palette,
																	 .width = 256,
																	 .height = 1,
																	 .cycle = 1,
																	 .raw_data = colors,
																	 .raw_size = sizeof colors }) ||
		!Aeron_UploadTextureDataCmd(cmd, &(AeronTextureUploadDesc) { .texture = materials,
																	 .width = 16,
																	 .height = 46,
																	 .cycle = 1,
																	 .raw_data = ramps,
																	 .raw_size = sizeof ramps }))
		return false;
	current_view = *view;
	version = s->flight_version;
	material_count = m->material_count;
	target = s->appearance.target_highlight;
	gouraud = version == 94 && s->appearance.gouraud_enabled;
	memcpy(gates, m->gate, sizeof gates);
	memcpy(view_uniform.view_proj, view->view_proj, sizeof view_uniform.view_proj);
	memcpy(view_uniform.camera, view->camera.pos, sizeof view->camera.pos);
	view_uniform.pixels[0] = 2.0f / view->camera.viewport.width;
	view_uniform.pixels[1] = 2.0f / view->camera.viewport.height;
	view_uniform.pixels[2] = view->classic_pixel_scale_x;
	view_uniform.pixels[3] = view->classic_pixel_scale_y;
	return true;
}

bool XwDosDraw_Add(const XwDosPart* part) {
	return XwDosDraw_AddMesh(part, XwRemasterAssets_DosMesh(part->geometry));
}

bool XwDosDraw_AddMesh(const XwDosPart* part, const XwDosMesh* mesh) {
	if (!mesh || part->lod >= mesh->lod_count)
		return false;
	float scale = 0;
	for (unsigned a = 0; a < 3; ++a) {
		const float* m = part->transform + a;
		scale = fmaxf(scale, sqrtf(m[0] * m[0] + m[4] * m[4] + m[8] * m[8]));
	}
	/* Include a source-pixel margin for line floors at the frustum boundary. */
	float depth = 0;
	for (unsigned a = 0; a < 3; ++a)
		depth += current_view.view_proj[12 + a] * part->transform[4 * a + 3];
	float margin = fmaxf(1, fabsf(depth) / 128);
	if (!XwShip_Visible(
			&current_view, part->transform,
			(part->hyperstar
				 ? fmaxf(fabsf((float)part->line_endpoints[0]), fabsf((float)part->line_endpoints[1])) + 128
				 : mesh->lods[part->lod].radius) *
					scale +
				margin))
		return true;
	if (count >= XW_SNAP_OBJECTS * XW_DOS_SELECTION_CAPACITY ||
		!XwDosMesh_Grow((void**)&draws, &draw_capacity, count + 1, sizeof *draws) ||
		!XwDosMesh_Grow((void**)&cpu_tables, &table_capacity, count + 1, sizeof *cpu_tables))
		return false;
	DosTable* table = &cpu_tables[count];
	memcpy(table->transform, part->transform, sizeof table->transform);
	memcpy(table->light, part->light_direction, sizeof part->light_direction);
	table->light[3] = gouraud;
	DosDraw* draw = &draws[count++];
	*draw = (DosDraw) { .mesh = mesh, .lod = part->lod, .projectile = part->projectile };
	draw->vertex_uniform.index = count - 1;
	draw->vertex_uniform.endpoints[0] = part->line_endpoints[0];
	draw->vertex_uniform.endpoints[1] = part->line_endpoints[1];
	draw->vertex_uniform.color = part->hyperstar ? part->line_color : -1;
	for (unsigned col = 0; col < 4; ++col)
		for (unsigned row = 0; row < 4; ++row) {
			draw->uniform.clip_x[col] += view_uniform.view_proj[row] * part->transform[4 * row + col];
			draw->uniform.clip_y[col] += view_uniform.view_proj[4 + row] * part->transform[4 * row + col];
			draw->uniform.depth_row[col] += view_uniform.view_proj[12 + row] * part->transform[4 * row + col];
		}
	unsigned mode =
		version == 93 ? (part->marking_mode > 2 ? 0 : part->marking_mode) : part->marking_mode & 3;
	draw->uniform.options[0] = part->markings;
	draw->uniform.options[1] = mode;
	draw->uniform.options[2] = material_count;
	draw->uniform.options[3] = version == 93;
	draw->uniform.policy[0] = part->parent == target;
	draw->uniform.policy[1] = (part->parent >> 8) == 0x40 ? gates[part->gate_tint & 3] : 0;
	draw->uniform.policy[2] = version == 94 && mode ? (int)(2 * mode) - 3 : 0;
	draw->uniform.pixels[0] = view_uniform.pixels[2];
	draw->uniform.pixels[1] = view_uniform.pixels[3];
	draw->uniform.pixels[2] = current_view.camera.viewport.width;
	draw->uniform.pixels[3] = current_view.camera.viewport.height;
	return true;
}

bool XwDosDraw_Upload(AeronCommandBuffer* cmd) {
	if (!count)
		return true;
	if (gpu_capacity < count) {
		uint32_t capacity = table_capacity;
		AeronBuffer* next =
			Aeron_CreateBuffer(&(AeronBufferDesc) { .size = capacity * sizeof(DosTable),
													.usage = AERON_BUFFER_USAGE_STORAGE,
													.memory_usage = AERON_MEMORY_USAGE_DYNAMIC });
		if (!next)
			return false;
		Aeron_DestroyBuffer(tables);
		tables = next;
		gpu_capacity = capacity;
	}
	return Aeron_UploadBufferDataCmd(cmd, tables, 0, cpu_tables, count * sizeof *cpu_tables);
}

void XwDosDraw_Pass(AeronCommandBuffer* cmd, AeronRenderPass* pass, int width, int height, void* user) {
	(void)cmd;
	(void)width;
	(void)height;
	(void)user;
	if (!count)
		return;
	/* Hull depth is complete before non-writing projectile segments are drawn. */
	for (unsigned bolts = 0; bolts < 2; ++bolts) {
		Aeron_BindGraphicsPipeline(pass, bolts ? bolt : opaque);
		Aeron_BindUniformData(pass, AERON_SHADER_STAGE_VERTEX, 0, &view_uniform, sizeof view_uniform);
		Aeron_BindStorageBuffer(pass, AERON_SHADER_STAGE_VERTEX, 0, tables);
		Aeron_BindTextureSampler(pass, AERON_SHADER_STAGE_FRAGMENT, 0, palette, sampler);
		Aeron_BindTextureSampler(pass, AERON_SHADER_STAGE_FRAGMENT, 1, materials, sampler);

		for (unsigned i = 0; i < count; ++i) {
			const DosDraw* draw = &draws[i];
			if (draw->projectile != (bolts != 0))
				continue;
			Aeron_BindUniformData(pass, AERON_SHADER_STAGE_VERTEX, 1, &draw->vertex_uniform,
								  sizeof draw->vertex_uniform);
			Aeron_BindUniformData(pass, AERON_SHADER_STAGE_FRAGMENT, 0, &draw->uniform, sizeof draw->uniform);
			Aeron_BindVertexBuffer(pass, 0, draw->mesh->vertices, 0);
			Aeron_BindIndexBuffer(pass, draw->mesh->indices, AERON_INDEX_FORMAT_UINT32, 0);
			Aeron_BindStorageBuffer(pass, AERON_SHADER_STAGE_FRAGMENT, 0, draw->mesh->marks);
			Aeron_BindStorageBuffer(pass, AERON_SHADER_STAGE_FRAGMENT, 1, draw->mesh->mark_vertices);
			const XwDosMeshLod* lod = &draw->mesh->lods[draw->lod];
			if (lod->triangles.count)
				Aeron_DrawIndexed(pass, lod->triangles.count, lod->triangles.first, 0);
			if (lod->lines.count)
				Aeron_DrawIndexed(pass, lod->lines.count, lod->lines.first, 0);
		}
	}
}

void XwDosDraw_Shutdown(void) {
	Aeron_DestroyGraphicsPipeline(opaque);
	Aeron_DestroyGraphicsPipeline(bolt);
	Aeron_DestroyShader(vertex_shader);
	Aeron_DestroyShader(fragment_shader);
	Aeron_DestroyBuffer(tables);
	Aeron_DestroyTexture(palette);
	Aeron_DestroyTexture(materials);
	Aeron_DestroySampler(sampler);
	free(cpu_tables);
	free(draws);
	opaque = bolt = NULL;
	vertex_shader = fragment_shader = NULL;
	tables = NULL;
	palette = materials = NULL;
	sampler = NULL;
	cpu_tables = NULL;
	draws = NULL;
	count = table_capacity = draw_capacity = gpu_capacity = 0;
	samples = 0;
}

AeronTexture* XwDosDraw_Palette(void) { return palette; }
