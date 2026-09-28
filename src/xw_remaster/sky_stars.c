/* Captured X-Wing star axes with OpenXvT/OpenTIE rounded shader coverage. */
#include "xw_remaster/sky_stars.h"
#include "aeron/aeron.h"
#include "xw_runtime/snapshot/render_assets.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

enum { STAR_COUNT = 3072, STAR_GRID_SPAN = 32 };

typedef struct StarInstance {
	float axis[3];
	float padding;
	float color[4];
} StarInstance;

typedef struct StarVertexUniform {
	float view_proj[16];
	float pixel_to_clip[2];
	float half_size_px[2];
	float brightness;
	float padding[3];
} StarVertexUniform;

/* Match the two-float4 instance and uniform layouts consumed by HLSL. */
typedef char StarInstanceLayout[(sizeof(StarInstance) == 32) ? 1 : -1];
typedef char StarVertexUniformLayout[(sizeof(StarVertexUniform) == 96) ? 1 : -1];

struct XwRemasterSkyStars {
	AeronShader* vertex_shader;
	AeronShader* fragment_shader;
	AeronGraphicsPipeline* pipeline;
	AeronSampleCount pipeline_samples;
	AeronBuffer* instances;
	StarVertexUniform vertex_uniform;
	XwRenderAssetId source;
	uint32_t instance_count;
	uint16_t density_divisor;
	uint64_t palette_revision;
	int16_t oscillator;
};

static int UploadStarInstances(XwRemasterSkyStars* stars, AeronCommandBuffer* cmd,
							   const XwRemasterSkyStarsParams* params) {
	uint16_t density_divisor = params->density_divisor;
	XwRenderAssetId source_id = params->source;
	if (stars->density_divisor == density_divisor && stars->source == source_id &&
		stars->palette_revision == params->palette_revision && stars->oscillator == params->oscillator)
		return 1;
	const XwRenderSource* source = XwRenderAssets_Source(source_id);
	if (!source || source->kind != XW_SOURCE_STARS)
		return 0;
	bool dos = source->flight_version != 98;
	if (source->size != (dos ? sizeof(XwRenderDosStars) : sizeof(XwRenderStars)) ||
		(dos && !params->palette_argb))
		return 0;
	StarInstance instances[STAR_COUNT];
	uint32_t count = 0;
	static const unsigned column_axis[3] = { 0, 0, 1 }, row_axis[3] = { 1, 2, 2 };
	unsigned grid_size = dos ? 16 / density_divisor : STAR_GRID_SPAN / density_divisor;
	for (unsigned plane = 0; plane < 3; ++plane) {
		for (unsigned row = 0; row < grid_size; ++row) {
			for (unsigned column = 0; column < grid_size; ++column, ++count) {
				StarInstance* star = &instances[count];
				uint32_t color;
				if (dos) {
					const XwRenderDosStars* data = source->data;
					unsigned seed = row * grid_size + column, index = data->jitter[seed];
					static const float jitter[4] = { .5f, .25f, -.25f, -.5f };
					for (unsigned a = 0; a < 3; ++a)
						star->axis[a] = -8 + jitter[(index >> (2 * a)) & 3];
					star->axis[column_axis[plane]] += column * density_divisor + 1;
					if (row)
						star->axis[row_axis[plane]] += (row - 1) * density_divisor + 1;
					unsigned light = (uint8_t)(data->brightness[seed] + params->oscillator);
					color = params->palette_argb[248 + (light > 7 ? 7 : light)];
				} else {
					const XwRenderStars* data = source->data;
					int index = data->indices[count];
					star->axis[0] = -32 + index / 25 - 2;
					star->axis[1] = -32 + (index % 25) / 5 - 2;
					star->axis[2] = -32 + index % 5 - 2;
					star->axis[column_axis[plane]] += (64.0f / grid_size) * column;
					star->axis[row_axis[plane]] += (64.0f / grid_size) * row;
					color = data->colors[count];
				}
				star->padding = 0;
				for (unsigned a = 0; a < 3; ++a) {
					float value = ((color >> (16 - a * 8)) & 255) / 255.0f;
					star->color[a] = value <= .04045f ? value / 12.92f : powf((value + .055f) / 1.055f, 2.4f);
				}
				star->color[3] = 1;
			}
		}
	}
	if (!Aeron_UploadBufferDataCmd(cmd, stars->instances, 0, instances, count * sizeof instances[0]))
		return 0;
	stars->density_divisor = density_divisor;
	stars->palette_revision = params->palette_revision;
	stars->oscillator = params->oscillator;
	stars->source = source_id;
	stars->instance_count = count;
	return 1;
}

static int PrepareStarPipeline(XwRemasterSkyStars* stars, AeronSampleCount sample_count) {
	if (stars->pipeline && stars->pipeline_samples == sample_count)
		return 1;
	if (stars->pipeline) {
		Aeron_DestroyGraphicsPipeline(stars->pipeline);
		stars->pipeline = NULL;
	}
	AeronColorTargetStateDesc color_target = {
        .format = AERON_TEXTURE_FORMAT_RGBA16_FLOAT,
        .blend = {
            .enabled = 1,
            .src_color = AERON_BLEND_ONE,
            .dst_color = AERON_BLEND_ONE_MINUS_SRC_ALPHA,
            .color_op = AERON_BLEND_OP_ADD,
            .src_alpha = AERON_BLEND_ONE,
            .dst_alpha = AERON_BLEND_ONE_MINUS_SRC_ALPHA,
            .alpha_op = AERON_BLEND_OP_ADD,
        },
    };
	stars->pipeline = Aeron_CreateGraphicsPipeline(&(AeronGraphicsPipelineDesc) {
		.vertex_shader = stars->vertex_shader,
		.fragment_shader = stars->fragment_shader,
		.primitive_type = AERON_PRIMITIVE_TRIANGLES,
		.cull_mode = AERON_CULL_NONE,
		.depth_format = AERON_TEXTURE_FORMAT_D32_FLOAT,
		.depth = { .depth_test = 1, .depth_write = 0, .compare = AERON_COMPARE_GREATER_EQUAL },
		.color_target_count = 1,
		.color_targets = &color_target,
		.sample_count = sample_count,
	});
	stars->pipeline_samples = stars->pipeline ? sample_count : 0;
	return stars->pipeline != NULL;
}

XwRemasterSkyStars* XwRemasterSkyStars_Create(void) {
	XwRemasterSkyStars* stars = calloc(1, sizeof *stars);
	if (!stars)
		return NULL;
	stars->vertex_shader = Aeron_CreateShader(&(AeronShaderDesc) {
		.name = "sky_stars.vert",
		.stage = AERON_SHADER_STAGE_VERTEX,
		.uniform_buffer_count = 1,
		.storage_buffer_count = 1,
	});
	stars->fragment_shader = Aeron_CreateShader(&(AeronShaderDesc) {
		.name = "sky_stars.frag",
		.stage = AERON_SHADER_STAGE_FRAGMENT,
	});
	stars->instances = Aeron_CreateBuffer(&(AeronBufferDesc) {
		.size = STAR_COUNT * sizeof(StarInstance),
		.usage = AERON_BUFFER_USAGE_STORAGE,
		.memory_usage = AERON_MEMORY_USAGE_DYNAMIC,
		.debug_name = "xw.sky.stars.instances",
	});
	if (!stars->vertex_shader || !stars->fragment_shader || !stars->instances) {
		Aeron_LogError("xw.remaster", "starfield: GPU resource creation failed");
		XwRemasterSkyStars_Destroy(stars);
		return NULL;
	}
	return stars;
}

void XwRemasterSkyStars_Destroy(XwRemasterSkyStars* stars) {
	if (!stars)
		return;
	if (stars->pipeline)
		Aeron_DestroyGraphicsPipeline(stars->pipeline);
	if (stars->vertex_shader)
		Aeron_DestroyShader(stars->vertex_shader);
	if (stars->fragment_shader)
		Aeron_DestroyShader(stars->fragment_shader);
	if (stars->instances)
		Aeron_DestroyBuffer(stars->instances);
	free(stars);
}

int XwRemasterSkyStars_Prepare(XwRemasterSkyStars* stars, AeronCommandBuffer* cmd, const AeronScene3D* scene,
							   const XwRemasterSkyStarsParams* params) {
	if (!stars || !cmd || !scene || !params || !params->density_divisor ||
		params->density_divisor > STAR_GRID_SPAN || params->classic_pixel_scale <= 0)
		return 0;
	const float* view_proj = params->view_proj ? params->view_proj : AeronScene_JitteredViewProj(scene);
	int render_w, render_h, output_w, output_h;
	AeronScene_RenderDims(scene, &render_w, &render_h);
	AeronScene_RtDims(scene, &output_w, &output_h);
	if (!view_proj || render_w <= 0 || render_h <= 0 || output_w <= 0 || output_h <= 0)
		return 0;
	if (!UploadStarInstances(stars, cmd, params))
		return 0;
	StarVertexUniform* uniform = &stars->vertex_uniform;
	memcpy(uniform->view_proj, view_proj, sizeof uniform->view_proj);
	uniform->pixel_to_clip[0] = 2.0f / (float)render_w;
	uniform->pixel_to_clip[1] = 2.0f / (float)render_h;
	/* Fitted classic pixels, scaled independently into the internal target. */
	uniform->half_size_px[0] = 0.5f * params->classic_pixel_scale * render_w / output_w;
	uniform->half_size_px[1] =
		0.5f *
		(params->classic_pixel_scale_y > 0 ? params->classic_pixel_scale_y : params->classic_pixel_scale) *
		render_h / output_h;
	uniform->brightness = params->exposure * params->brightness;
	return 1;
}

void XwRemasterSkyStars_Draw(AeronCommandBuffer* command_buffer, AeronRenderPass* render_pass, int rt_w,
							 int rt_h, void* user) {
	(void)rt_w;
	(void)rt_h;
	XwRemasterSkyStars* stars = user;
	if (!stars || !render_pass || !stars->instance_count)
		return;
	if (!PrepareStarPipeline(stars, Aeron_RenderPassGetSampleCount(render_pass))) {
		Aeron_CommandBufferSetFailure(command_buffer, "Starfield pipeline preparation failed");
		return;
	}
	Aeron_BindGraphicsPipeline(render_pass, stars->pipeline);
	Aeron_BindStorageBuffer(render_pass, AERON_SHADER_STAGE_VERTEX, 0, stars->instances);
	Aeron_BindUniformData(render_pass, AERON_SHADER_STAGE_VERTEX, 0, &stars->vertex_uniform,
						  sizeof stars->vertex_uniform);
	Aeron_DrawInstanced(render_pass, 6, stars->instance_count, 0);
}
