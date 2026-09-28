/* Indexed, masked DOS billboards. Palette resolution is deferred to the GPU. */
#include "xw_remaster/dos_sprites.h"
#include "xw_remaster/assets.h"
#include "xw_remaster/dos_draw.h"
#include "xw_remaster/dos_mesh_internal.h"
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct SpriteVertex {
	float position[4], uv[2];
} SpriteVertex;

typedef struct SpriteParams {
	float colors[16];
	float texel_bounds[4];
	float filter[4];
} SpriteParams;

typedef struct Sprite {
	AeronTexture* texture;
	SpriteParams params;
	float depth;
	uint32_t first, order;
	bool sky;
} Sprite;

static Sprite* sprites;
static SpriteVertex* vertices;
static uint32_t count, capacity, vertex_capacity, gpu_capacity;
static AeronBuffer* buffer;
static AeronShader *vs, *fs;
static AeronGraphicsPipeline *world_pipeline, *sky_pipeline;
static AeronSampler* sampler;
static AeronSampleCount samples;

static AeronGraphicsPipeline* Pipeline(AeronSampleCount msaa, bool sky) {
	AeronVertexBufferLayoutDesc layout = { .stride = sizeof(SpriteVertex) };
	AeronVertexAttributeDesc attributes[2] = {
		{ .location = 0, .format = AERON_VERTEX_FORMAT_FLOAT4, .offset = offsetof(SpriteVertex, position) },
		{ .location = 1, .format = AERON_VERTEX_FORMAT_FLOAT2, .offset = offsetof(SpriteVertex, uv) }
	};
	AeronColorTargetStateDesc target = { .format = AERON_TEXTURE_FORMAT_RGBA16_FLOAT,
										 .blend = { .enabled = sky,
													.src_color = AERON_BLEND_ONE,
													.dst_color = AERON_BLEND_ONE_MINUS_SRC_ALPHA,
													.color_op = AERON_BLEND_OP_ADD,
													.src_alpha = AERON_BLEND_ONE,
													.dst_alpha = AERON_BLEND_ONE_MINUS_SRC_ALPHA,
													.alpha_op = AERON_BLEND_OP_ADD } };
	return Aeron_CreateGraphicsPipeline(&(AeronGraphicsPipelineDesc) {
		.vertex_shader = vs,
		.fragment_shader = fs,
		.primitive_type = AERON_PRIMITIVE_TRIANGLES,
		.cull_mode = AERON_CULL_NONE,
		.vertex_buffers = &layout,
		.vertex_buffer_count = 1,
		.attributes = attributes,
		.attribute_count = 2,
		.color_targets = &target,
		.color_target_count = 1,
		.sample_count = msaa,
		.depth_format = AERON_TEXTURE_FORMAT_D32_FLOAT,
		/* Effects are sorted back to front and test only against scene geometry. */
		.depth = { .depth_test = 1, .depth_write = 0, .compare = AERON_COMPARE_GREATER_EQUAL } });
}

bool XwDosSprites_Begin(AeronSampleCount msaa) {
	count = 0;
	if (!vs)
		vs = Aeron_CreateShader(
			&(AeronShaderDesc) { .name = "dos_sprite.vert", .stage = AERON_SHADER_STAGE_VERTEX });
	if (!fs)
		fs = Aeron_CreateShader(&(AeronShaderDesc) { .name = "dos_sprite.frag",
													 .stage = AERON_SHADER_STAGE_FRAGMENT,
													 .sampler_count = 2,
													 .uniform_buffer_count = 1 });
	if (!sampler)
		sampler = Aeron_CreateSampler(&(AeronSamplerDesc) { .min_filter = AERON_FILTER_NEAREST,
															.mag_filter = AERON_FILTER_NEAREST,
															.address_u = AERON_ADDRESS_CLAMP_TO_EDGE,
															.address_v = AERON_ADDRESS_CLAMP_TO_EDGE });
	if (!vs || !fs || !sampler)
		return false;
	if (!world_pipeline || samples != msaa) {
		AeronGraphicsPipeline* world = Pipeline(msaa, false);
		AeronGraphicsPipeline* sky = Pipeline(msaa, true);
		if (!world || !sky) {
			Aeron_DestroyGraphicsPipeline(world);
			Aeron_DestroyGraphicsPipeline(sky);
			return false;
		}
		Aeron_DestroyGraphicsPipeline(world_pipeline);
		Aeron_DestroyGraphicsPipeline(sky_pipeline);
		world_pipeline = world;
		sky_pipeline = sky;
		samples = msaa;
	}
	return true;
}

static uint16_t Scale(float depth, unsigned extent, uint16_t factor) {
	uint32_t z = depth >= 2147483648.0f ? INT32_MAX : (uint32_t)depth;
	uint16_t denominator = z >> 8;
	if (z >> 24)
		denominator |= 0xFF00;
	if (!denominator)
		denominator = 1;
	uint16_t scale = (uint16_t)(((extent / denominator) * factor) >> 8);
	return scale < 1024 ? scale : 1024;
}

static bool Queue(const AeronRuntimeAtlasPage* page, const AeronSpriteRect* rect, const XwRenderSource* remap,
				  const SpriteVertex quad[4], float depth, bool sky) {
	if (count >= XW_SNAP_OBJECTS * XW_DOS_SELECTION_CAPACITY + XW_SNAP_BACKDROPS ||
		!XwDosMesh_Grow((void**)&sprites, &capacity, count + 1, sizeof *sprites) ||
		!XwDosMesh_Grow((void**)&vertices, &vertex_capacity, (count + 1) * 6, sizeof *vertices))
		return false;
	Sprite* sprite = &sprites[count];
	*sprite =
		(Sprite) { .texture = page->texture, .depth = depth, .first = count * 6, .order = count, .sky = sky };
	sprite->params.texel_bounds[0] = rect->x;
	sprite->params.texel_bounds[1] = rect->y;
	sprite->params.texel_bounds[2] = rect->x + rect->w - 1;
	sprite->params.texel_bounds[3] = rect->y + rect->h - 1;
	sprite->params.filter[0] = sky;
	for (unsigned i = 0; i < 16; ++i)
		sprite->params.colors[i] = ((const uint8_t*)remap->data)[i];
	static const unsigned indices[6] = { 0, 1, 2, 0, 2, 3 };
	for (unsigned i = 0; i < 6; ++i)
		vertices[count * 6 + i] = quad[indices[i]];
	++count;
	return true;
}

static bool AddEffect(const XwRenderSnapshot* s, const XwRenderView* view, unsigned type, unsigned image,
					  const float eye[3], uint16_t factor, float angle) {
	if (eye[2] <= 0)
		return true;
	const XwRenderAssetSetView* set = XwRenderAssets_Set(s->flight_assets);
	if (!set || type >= set->bindings.type_count)
		return false;
	const XwSnapType* t = &set->bindings.types[type];
	uint16_t scale = Scale(eye[2], t->max_extent, factor);
	bool alternate = (scale < 128 || !t->bitmaps) && t->alternate_bitmaps;
	if (alternate)
		scale *= 2;
	if (!scale)
		return true;
	const AeronRuntimeAtlas* atlas = XwRemasterAssets_Image(alternate ? t->alternate_bitmaps : t->bitmaps);
	const XwRenderSource* remap = XwRenderAssets_Source(t->bitmap_remap);
	if (!atlas || !remap || remap->kind != XW_SOURCE_PALETTE || remap->size != 16)
		return false;
	int frame = Aeron_SpriteAtlasFindById(&atlas->layout, (int)image);
	if (frame < 0)
		return true; /* The source bitmap accessor skips absent animation frames. */
	const AeronSpriteRect* rect = &atlas->layout.frames[frame];
	const AeronRuntimeAtlasPage* page = &atlas->pages[atlas->layout.pages[frame]];
	float left = atlas->layout.origin_x[frame], top = atlas->layout.origin_y[frame];
	float aspect = s->camera.aspect_y_q16 / 65536.0f;
	float cx = s->camera.viewport.x + s->camera.center_x + 256 * eye[0] / eye[2];
	float cy = s->camera.viewport.y + s->camera.viewport.height - 1 - s->camera.center_y +
			   s->camera.projection_offset_y + 256 * aspect * eye[1] / eye[2];
	XwLayoutTransform layout;
	if (!XwRenderMath_Layout(s->camera.screen_width, s->camera.screen_height, view->camera.viewport.width,
							 view->camera.viewport.height, &layout))
		return false;
	SpriteVertex quad[4];
	float sine = sinf(angle), cosine = cosf(angle), min_x = INFINITY, min_y = INFINITY;
	float max_x = -INFINITY, max_y = -INFINITY;
	/* Approximate classic flat-inside-mesh priority: bitmap bounds
	 * use half-scale world units. Bias only depth, clamping at the near plane. */
	float near_z = view->camera.near_z;
	float depth = near_z / fmaxf(near_z, eye[2] - t->max_extent * .5f);
	for (unsigned i = 0; i < 4; ++i) {
		bool right = i == 1 || i == 2, bottom = i >= 2;
		float x = (left + (right ? rect->w : 0)) * scale / 256;
		/* ROTSCALE rows descend from the authored +Y-up anchor. */
		float y = (top - (bottom ? rect->h : 0)) * (282.0f / 256) * scale / 256;
		float px, py;
		XwRenderMath_LayoutPoint(&layout, .5f, .5f, cx + x * cosine + y * sine,
								 cy + (-x * sine + y * cosine) * (233.0f / 256), &px, &py);
		min_x = fminf(min_x, px);
		max_x = fmaxf(max_x, px);
		min_y = fminf(min_y, py);
		max_y = fmaxf(max_y, py);
		quad[i] = (SpriteVertex) { .position = { 2 * px / layout.target_width - 1,
												 1 - 2 * py / layout.target_height, depth, 1 },
								   .uv = { (rect->x + (right ? rect->w : 0)) / page->width,
										   (rect->y + (bottom ? rect->h : 0)) / page->height } };
	}
	if (max_x < 0 || min_x >= layout.target_width || max_y < 0 || min_y >= layout.target_height)
		return true;
	return Queue(page, rect, remap, quad, eye[2], false);
}

bool XwDosSprites_Backdrop(const XwRenderView* view, const AeronRuntimeAtlasPage* page,
						   const AeronSpriteRect* rect, const XwRenderSource* remap,
						   const AeronSceneBillboardDesc* backdrop) {
	SpriteVertex quad[4];
	for (unsigned i = 0; i < 4; ++i) {
		const float* axis = backdrop->corners[i];
		const float* m = view->view_proj;
		for (unsigned row = 0; row < 4; ++row)
			quad[i].position[row] =
				m[4 * row] * axis[0] + m[4 * row + 1] * axis[1] + m[4 * row + 2] * axis[2];
		quad[i].position[2] = 0; /* Reverse-Z sky depth; position is a direction. */
		memcpy(quad[i].uv, backdrop->uv[i], sizeof quad[i].uv);
	}
	return Queue(page, rect, remap, quad, 0, true);
}

bool XwDosSprites_Effects(const XwRenderSnapshot* s, const XwRenderView* view,
						  const XwDosSelection* selection) {
	for (unsigned i = 0; i < selection->bitmap_count; ++i) {
		const XwDosBitmap* bitmap = &selection->bitmaps[i];
		if (!AddEffect(s, view, (bitmap->image & 0x7FFF) >> 8, bitmap->image & 255, bitmap->eye,
					   bitmap->scale, bitmap->angle))
			return false;
	}
	return true;
}

static int Compare(const void* a, const void* b) {
	const Sprite* x = a;
	const Sprite* y = b;
	if (x->sky != y->sky)
		return x->sky ? -1 : 1;
	if (!x->sky && x->depth != y->depth)
		return x->depth > y->depth ? -1 : 1;
	/* Classic consumes the stable depth-sort tail; equal-depth effects reverse insertion. */
	if (x->order == y->order)
		return 0;
	return x->sky ? (x->order < y->order ? -1 : 1) : (x->order > y->order ? -1 : 1);
}

bool XwDosSprites_Upload(AeronCommandBuffer* cmd) {
	if (!count)
		return true;
	if (gpu_capacity < count * 6) {
		AeronBuffer* next =
			Aeron_CreateBuffer(&(AeronBufferDesc) { .size = vertex_capacity * sizeof *vertices,
													.usage = AERON_BUFFER_USAGE_VERTEX,
													.memory_usage = AERON_MEMORY_USAGE_DYNAMIC });
		if (!next)
			return false;
		Aeron_DestroyBuffer(buffer);
		buffer = next;
		gpu_capacity = vertex_capacity;
	}
	qsort(sprites, count, sizeof *sprites, Compare);
	return Aeron_UploadBufferDataCmd(cmd, buffer, 0, vertices, count * 6 * sizeof *vertices);
}

void XwDosSprites_Draw(AeronRenderPass* pass, bool sky) {
	if (!count)
		return;
	Aeron_BindGraphicsPipeline(pass, sky ? sky_pipeline : world_pipeline);
	Aeron_BindVertexBuffer(pass, 0, buffer, 0);
	Aeron_BindTextureSampler(pass, AERON_SHADER_STAGE_FRAGMENT, 1, XwDosDraw_Palette(), sampler);
	for (unsigned i = 0; i < count; ++i) {
		const Sprite* sprite = &sprites[i];
		if (sprite->sky != sky)
			continue;
		Aeron_BindTextureSampler(pass, AERON_SHADER_STAGE_FRAGMENT, 0, sprite->texture, sampler);
		Aeron_BindUniformData(pass, AERON_SHADER_STAGE_FRAGMENT, 0, &sprite->params, sizeof sprite->params);
		Aeron_Draw(pass, 6, sprite->first);
	}
}

void XwDosSprites_Shutdown(void) {
	Aeron_DestroyGraphicsPipeline(world_pipeline);
	Aeron_DestroyGraphicsPipeline(sky_pipeline);
	Aeron_DestroyShader(vs);
	Aeron_DestroyShader(fs);
	Aeron_DestroySampler(sampler);
	Aeron_DestroyBuffer(buffer);
	free(sprites);
	free(vertices);
	world_pipeline = sky_pipeline = NULL;
	vs = fs = NULL;
	sampler = NULL;
	buffer = NULL;
	sprites = NULL;
	vertices = NULL;
	count = capacity = vertex_capacity = gpu_capacity = 0;
	samples = 0;
}
