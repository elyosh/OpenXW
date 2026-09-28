#include "xw_remaster/hyperspace.h"
#include "aeron/aeron.h"
#include "aeron/asset/opt_model.h"
#include "xw/flight/object/anim.h"
#include "xw_remaster/config.h"
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct HyperStreakVertex {
	float position[3];
	float color[4];
} HyperStreakVertex;

typedef struct HyperTunnelUniform {
	float view[4];
	float projection[4];
	float motion[4];
	float appearance[4];
	float geometry[4];
	float tunnel_right[4];
	float tunnel_up[4];
	float tunnel_forward[4];
	float dark_color[4];
	float body_color[4];
	float highlight_color[4];
	float cap_color[4];
} HyperTunnelUniform;

enum { HYPER_ENVIRONMENT_FACE_SIZE = 32, HYPER_ENVIRONMENT_ATLAS_WIDTH = 192 };

typedef struct HyperEnvironmentUniform {
	float roughness;
	float pad[3];
} HyperEnvironmentUniform;

typedef struct XwHyperspace {
	AeronShader *streak_vs, *streak_fs, *tunnel_vs, *tunnel_fs;
	AeronGraphicsPipeline *streak_pipeline, *tunnel_pipeline;
	AeronComputePipeline* environment_pipeline;
	AeronTexture *environment_atlas, *environment_cube;
	AeronSampler* environment_sampler;
	AeronScene3D* lighting_scene;
	AeronSampleCount pipeline_samples;
	AeronBuffer* streak_vb;
	uint32_t streak_vb_capacity, streak_vertex_count;
	int draw_background;
	float view_proj[16];
	HyperTunnelUniform tunnel_uniform;
} XwHyperspace;

static XwHyperspace g_hyper;

void XwHyperspace_Shutdown(void) {
	XwHyperspace* h = &g_hyper;
	Aeron_DestroyGraphicsPipeline(h->streak_pipeline);
	Aeron_DestroyGraphicsPipeline(h->tunnel_pipeline);
	Aeron_DestroyShader(h->streak_vs);
	Aeron_DestroyShader(h->streak_fs);
	Aeron_DestroyShader(h->tunnel_vs);
	Aeron_DestroyShader(h->tunnel_fs);
	Aeron_DestroyBuffer(h->streak_vb);
	Aeron_DestroyComputePipeline(h->environment_pipeline);
	Aeron_DestroyTexture(h->environment_atlas);
	Aeron_DestroyTexture(h->environment_cube);
	Aeron_DestroySampler(h->environment_sampler);
	memset(h, 0, sizeof *h);
}

static int Init(void) {
	XwHyperspace* h = &g_hyper;
	if (h->streak_vs)
		return 1;
	h->streak_vs = Aeron_CreateShader(&(AeronShaderDesc) {
		.name = "hyperspace_streak.vert", .stage = AERON_SHADER_STAGE_VERTEX, .uniform_buffer_count = 1 });
	h->streak_fs = Aeron_CreateShader(
		&(AeronShaderDesc) { .name = "hyperspace_streak.frag", .stage = AERON_SHADER_STAGE_FRAGMENT });
	h->tunnel_vs = Aeron_CreateShader(
		&(AeronShaderDesc) { .name = "hyperspace_tunnel.vert", .stage = AERON_SHADER_STAGE_VERTEX });
	h->tunnel_fs = Aeron_CreateShader(&(AeronShaderDesc) {
		.name = "hyperspace_tunnel.frag", .stage = AERON_SHADER_STAGE_FRAGMENT, .uniform_buffer_count = 1 });
	h->environment_pipeline = Aeron_CreateComputePipeline(&(AeronComputePipelineDesc) {
		.name = "hyperspace_environment.comp",
		.readwrite_storage_texture_count = 1,
		.uniform_buffer_count = 2,
		.thread_count_x = 8,
		.thread_count_y = 8,
		.thread_count_z = 1,
	});
	h->environment_atlas = Aeron_CreateTexture(&(AeronTextureDesc) {
		.width = HYPER_ENVIRONMENT_ATLAS_WIDTH,
		.height = HYPER_ENVIRONMENT_FACE_SIZE,
		.format = AERON_TEXTURE_FORMAT_RGBA16_FLOAT,
		.usage = AERON_TEXTURE_USAGE_COMPUTE_STORAGE_WRITE | AERON_TEXTURE_USAGE_TRANSFER_SRC,
		.debug_name = "xw.hyperspace.environment_atlas",
	});
	h->environment_cube = Aeron_CreateTexture(&(AeronTextureDesc) {
		.width = HYPER_ENVIRONMENT_FACE_SIZE,
		.height = HYPER_ENVIRONMENT_FACE_SIZE,
		.format = AERON_TEXTURE_FORMAT_RGBA16_FLOAT,
		.usage = AERON_TEXTURE_USAGE_SAMPLED | AERON_TEXTURE_USAGE_TRANSFER_DST,
		.cube = 1,
		.debug_name = "xw.hyperspace.environment",
	});
	h->environment_sampler = Aeron_CreateSampler(&(AeronSamplerDesc) {
		.min_filter = AERON_FILTER_LINEAR,
		.mag_filter = AERON_FILTER_LINEAR,
		.mip_filter = AERON_FILTER_LINEAR,
		.address_u = AERON_ADDRESS_CLAMP_TO_EDGE,
		.address_v = AERON_ADDRESS_CLAMP_TO_EDGE,
		.address_w = AERON_ADDRESS_CLAMP_TO_EDGE,
	});
	if (!h->streak_vs || !h->streak_fs || !h->tunnel_vs || !h->tunnel_fs || !h->environment_pipeline ||
		!h->environment_atlas || !h->environment_cube || !h->environment_sampler) {
		XwHyperspace_Shutdown();
		return 0;
	}
	return 1;
}

static AeronBlendStateDesc hyper_blend_additive(void) {
	return (AeronBlendStateDesc) {
		.enabled = 1,
		.src_color = AERON_BLEND_ONE,
		.dst_color = AERON_BLEND_ONE,
		.color_op = AERON_BLEND_OP_ADD,
		.src_alpha = AERON_BLEND_ZERO,
		.dst_alpha = AERON_BLEND_ONE,
		.alpha_op = AERON_BLEND_OP_ADD,
	};
}

static AeronBlendStateDesc hyper_blend_pma(void) {
	return (AeronBlendStateDesc) {
		.enabled = 1,
		.src_color = AERON_BLEND_ONE,
		.dst_color = AERON_BLEND_ONE_MINUS_SRC_ALPHA,
		.color_op = AERON_BLEND_OP_ADD,
		.src_alpha = AERON_BLEND_ONE,
		.dst_alpha = AERON_BLEND_ONE_MINUS_SRC_ALPHA,
		.alpha_op = AERON_BLEND_OP_ADD,
	};
}

static AeronGraphicsPipeline* hyper_create_pipeline(AeronShader* vs, AeronShader* fs, uint32_t stride,
													const AeronVertexAttributeDesc* attrs,
													uint32_t attr_count, AeronBlendStateDesc blend,
													AeronSampleCount sample_count) {
	const AeronVertexBufferLayoutDesc layout = { .slot = 0, .stride = stride };
	AeronColorTargetStateDesc targets[1] = { 0 };
	targets[0].format = AERON_TEXTURE_FORMAT_RGBA16_FLOAT;
	targets[0].blend = blend;
	return Aeron_CreateGraphicsPipeline(&(AeronGraphicsPipelineDesc) {
		.vertex_shader = vs,
		.fragment_shader = fs,
		.primitive_type = AERON_PRIMITIVE_TRIANGLES,
		.cull_mode = AERON_CULL_NONE,
		.vertex_buffers = &layout,
		.vertex_buffer_count = 1,
		.attributes = attrs,
		.attribute_count = attr_count,
		.depth_format = AERON_TEXTURE_FORMAT_D32_FLOAT,
		.depth = { .depth_test = 0, .depth_write = 0, .compare = AERON_COMPARE_ALWAYS },
		.color_target_count = 1,
		.color_targets = targets,
		.sample_count = sample_count,
	});
}

static AeronGraphicsPipeline* hyper_create_fullscreen_pipeline(AeronShader* vs, AeronShader* fs,
															   AeronSampleCount sample_count) {
	AeronColorTargetStateDesc target = {
		.format = AERON_TEXTURE_FORMAT_RGBA16_FLOAT,
		.blend = hyper_blend_pma(),
	};
	return Aeron_CreateGraphicsPipeline(&(AeronGraphicsPipelineDesc) {
		.vertex_shader = vs,
		.fragment_shader = fs,
		.primitive_type = AERON_PRIMITIVE_TRIANGLES,
		.cull_mode = AERON_CULL_NONE,
		.depth_format = AERON_TEXTURE_FORMAT_D32_FLOAT,
		.depth = { .depth_test = 0, .depth_write = 0, .compare = AERON_COMPARE_ALWAYS },
		.color_target_count = 1,
		.color_targets = &target,
		.sample_count = sample_count,
	});
}

static void hyper_destroy_pipelines(XwHyperspace* h) {
	if (h->streak_pipeline)
		Aeron_DestroyGraphicsPipeline(h->streak_pipeline);
	if (h->tunnel_pipeline)
		Aeron_DestroyGraphicsPipeline(h->tunnel_pipeline);
	h->streak_pipeline = NULL;
	h->tunnel_pipeline = NULL;
	h->pipeline_samples = 0;
}

static int hyper_ensure_pipelines(XwHyperspace* h, AeronSampleCount sample_count) {
	static const AeronVertexAttributeDesc streak_attrs[] = {
		{ .location = 0,
		  .buffer_slot = 0,
		  .format = AERON_VERTEX_FORMAT_FLOAT3,
		  .offset = (uint32_t)offsetof(HyperStreakVertex, position) },
		{ .location = 1,
		  .buffer_slot = 0,
		  .format = AERON_VERTEX_FORMAT_FLOAT4,
		  .offset = (uint32_t)offsetof(HyperStreakVertex, color) },
	};
	if (h->pipeline_samples == sample_count && h->streak_pipeline && h->tunnel_pipeline) {
		return 1;
	}
	hyper_destroy_pipelines(h);
	h->streak_pipeline =
		hyper_create_pipeline(h->streak_vs, h->streak_fs, (uint32_t)sizeof(HyperStreakVertex), streak_attrs,
							  2, hyper_blend_additive(), sample_count);
	h->tunnel_pipeline = hyper_create_fullscreen_pipeline(h->tunnel_vs, h->tunnel_fs, sample_count);
	if (!h->streak_pipeline || !h->tunnel_pipeline) {
		hyper_destroy_pipelines(h);
		return 0;
	}
	h->pipeline_samples = sample_count;
	return 1;
}

static int hyper_ensure_streak_buffer(XwHyperspace* h, uint32_t bytes) {
	if (bytes == 0) {
		return 1;
	}
	if (h->streak_vb && h->streak_vb_capacity >= bytes) {
		return 1;
	}
	uint32_t capacity = h->streak_vb_capacity ? h->streak_vb_capacity : 64u * 1024u;
	while (capacity < bytes) {
		capacity *= 2u;
	}
	if (h->streak_vb) {
		Aeron_DestroyBuffer(h->streak_vb);
	}
	h->streak_vb = Aeron_CreateBuffer(&(AeronBufferDesc) { .size = capacity,
														   .usage = AERON_BUFFER_USAGE_VERTEX,
														   .debug_name = "xw.hyperspace.streak_vertices" });
	h->streak_vb_capacity = h->streak_vb ? capacity : 0;
	return h->streak_vb != NULL;
}

static void hyper_widescreen_remap(float p[3], const float camera_rows[9], float x_scale) {
	if (x_scale <= 1.0f) {
		return;
	}
	float eye[3];
	for (int r = 0; r < 3; r++) {
		eye[r] =
			camera_rows[r * 3 + 0] * p[0] + camera_rows[r * 3 + 1] * p[1] + camera_rows[r * 3 + 2] * p[2];
	}
	eye[0] *= x_scale;
	for (int c = 0; c < 3; c++) {
		p[c] = camera_rows[0 * 3 + c] * eye[0] + camera_rows[1 * 3 + c] * eye[1] +
			   camera_rows[2 * 3 + c] * eye[2];
	}
}

static void hyper_emit_streak(HyperStreakVertex* out, const XwSnapHyperstar* streak,
							  const XwRenderSnapshot* snapshot, const float rows[9], float x_scale) {
	XwSnapObject synthetic = { .orientation_dirty = 1, .pitch = 0x4000u, .roll = streak->roll };
	memcpy(synthetic.world_pos, streak->world_pos, sizeof synthetic.world_pos);
	float model[16];
	XwRenderMath_ObjectMatrix(&synthetic, 98, snapshot->camera.world_pos, model);
	float half = 64.0f * AERON_OPT_METERS_PER_UNIT;
	float extent = (snapshot->hyperspace.windows_streak_length >> 1) * AERON_OPT_METERS_PER_UNIT;
	const float corners[4][3] = {
		{ half, 0, 0 }, { half, extent, 0 }, { -half, extent, 0 }, { -half, 0, 0 }
	};
	static const uint8_t indices[6] = { 0, 1, 2, 0, 2, 3 };
	for (unsigned v = 0; v < 6; ++v) {
		const float* q = corners[indices[v]];
		float p[3];
		for (unsigned r = 0; r < 3; ++r)
			p[r] = model[r * 4] * q[0] + model[r * 4 + 1] * q[1] + model[r * 4 + 2] * q[2] + model[r * 4 + 3];
		hyper_widescreen_remap(p, rows, x_scale);
		memcpy(out[v].position, p, sizeof p);
		for (unsigned c = 0; c < 4; ++c)
			out[v].color[c] = 1;
	}
}

bool XwHyperspace_Active(const XwRenderSnapshot* s) {
	return s->hyperspace.phase == ANIM_HYPERSPACE_DEPART || s->hyperspace.phase == ANIM_HYPERSPACE_EXTERNAL ||
		   s->hyperspace.phase == ANIM_HYPERSPACE_RETURN;
}

static int hyper_generate_environment(XwHyperspace* h, AeronCommandBuffer* cmd) {
	const AeronComputeTextureBinding output = { .texture = h->environment_atlas };
	AeronComputePass* pass = Aeron_BeginComputePass(&(AeronComputePassDesc) {
		.command_buffer = cmd,
		.write_textures = &output,
		.write_texture_count = 1,
		.debug_label = "Hyperspace diffuse environment",
	});
	if (!pass) {
		return 0;
	}
	const HyperEnvironmentUniform environment = {
		.roughness = XwRemasterConfig_Effective()->hyperspace.mesh_environment_roughness,
	};
	Aeron_BindComputePipeline(pass, h->environment_pipeline);
	Aeron_BindComputeUniformData(pass, 0, &h->tunnel_uniform, sizeof h->tunnel_uniform);
	Aeron_BindComputeUniformData(pass, 1, &environment, sizeof environment);
	Aeron_DispatchCompute(pass, HYPER_ENVIRONMENT_ATLAS_WIDTH / 8, HYPER_ENVIRONMENT_FACE_SIZE / 8, 1);
	Aeron_EndComputePass(pass);

	/* Metal cannot expose one face of a cube as a writable storage view.
	 * Generate into a 2D atlas and copy each completed face into the sampled cube. */
	for (uint32_t face = 0; face < 6; face++) {
		if (!Aeron_CopyTextureCmd(cmd, &(AeronTextureCopyDesc) {
										   .source = h->environment_atlas,
										   .source_x = face * HYPER_ENVIRONMENT_FACE_SIZE,
										   .destination = h->environment_cube,
										   .destination_layer = face,
										   .width = HYPER_ENVIRONMENT_FACE_SIZE,
										   .height = HYPER_ENVIRONMENT_FACE_SIZE,
									   })) {
			return 0;
		}
	}
	return 1;
}

int XwHyperspace_Prepare(AeronCommandBuffer* cmd, const XwRenderSnapshot* s, AeronScene3D* scene,
						 const XwRenderView* view) {
	XwHyperspace* h = &g_hyper;
	h->streak_vertex_count = 0;
	h->draw_background = 0;
	h->lighting_scene = NULL;
	if (!XwHyperspace_Active(s))
		return 1;
	if (!Init())
		return 0;
	int width, height;
	AeronScene_RenderDims(scene, &width, &height);
	memcpy(h->view_proj, AeronScene_JitteredViewProj(scene), sizeof h->view_proj);
	float rows[9];
	XwRenderMath_ViewRows(&view->camera, rows);
	unsigned ticks = s->hyperspace.elapsed_ticks;
	if (s->hyperspace.phase == ANIM_HYPERSPACE_EXTERNAL) {
		const XwHyperspaceSettings* p = &XwRemasterConfig_Effective()->hyperspace;
		HyperTunnelUniform* u = &h->tunnel_uniform;
		memset(u, 0, sizeof *u);
		u->view[0] = (float)width;
		u->view[1] = (float)height;
		u->projection[0] = tanf(view->camera.h_half_rad);
		u->projection[1] = tanf(view->camera.v_half_rad);
		u->projection[2] = view->camera.proj_x_offset;
		u->projection[3] = view->camera.proj_y_offset;
		u->motion[0] = (float)((int)ticks - ANIM_HYPERSPACE_EXTERNAL_TICK) / 236.0f;
		u->motion[1] = p->travel_speed;
		u->motion[2] = p->rotation_speed;
		u->motion[3] = p->noise_scale;
		u->appearance[0] = p->brightness;
		u->appearance[1] = p->highlight_strength;
		u->geometry[0] = p->focal_length;
		u->geometry[1] = p->twist;
		u->geometry[2] = p->cap_radius;
		u->geometry[3] = p->cap_falloff;
		for (int i = 0; i < 3; ++i) {
			u->tunnel_right[i] = rows[i * 3];
			u->tunnel_up[i] = rows[i * 3 + 2];
			u->tunnel_forward[i] = rows[i * 3 + 1];
		}
		memcpy(u->dark_color, p->dark_color, sizeof p->dark_color);
		memcpy(u->body_color, p->body_color, sizeof p->body_color);
		memcpy(u->highlight_color, p->highlight_color, sizeof p->highlight_color);
		memcpy(u->cap_color, p->cap_color, sizeof p->cap_color);
		h->draw_background = 1;
		if (!hyper_generate_environment(h, cmd))
			return 0;
		h->lighting_scene = scene;
		return 1;
	}
	unsigned count = s->hyperspace.count;
	if (!count)
		return 1;
	uint32_t bytes = count * 6u * (uint32_t)sizeof(HyperStreakVertex);
	if (!hyper_ensure_streak_buffer(h, bytes))
		return 0;
	if (count > XW_SNAP_HYPERSTARS)
		return 0;
	HyperStreakVertex vertices[XW_SNAP_HYPERSTARS * 6];
	float aspect = (float)width / height, x_scale = fmaxf(1, aspect / (4.0f / 3.0f));
	for (unsigned i = 0; i < count; ++i)
		hyper_emit_streak(vertices + i * 6, &s->hyperspace.stars[i], s, rows, x_scale);
	int ok = Aeron_UploadBufferDataCmd(cmd, h->streak_vb, 0, vertices, bytes);
	if (ok)
		h->streak_vertex_count = count * 6;
	return ok;
}

void XwHyperspace_Draw(AeronCommandBuffer* command_buffer, AeronRenderPass* pass, int rt_w, int rt_h,
					   void* user) {
	(void)user;
	XwHyperspace* h = &g_hyper;
	if (!h || !pass) {
		return;
	}
	if (!hyper_ensure_pipelines(h, Aeron_RenderPassGetSampleCount(pass))) {
		Aeron_CommandBufferSetFailure(command_buffer, "Hyperspace pipeline preparation failed");
		return;
	}
	Aeron_SetViewport(pass, &(AeronRectI) { 0, 0, rt_w, rt_h });
	if (h->draw_background) {
		Aeron_BindGraphicsPipeline(pass, h->tunnel_pipeline);
		Aeron_BindUniformData(pass, AERON_SHADER_STAGE_FRAGMENT, 0, &h->tunnel_uniform,
							  (uint32_t)sizeof h->tunnel_uniform);
		Aeron_Draw(pass, 3, 0);
	}
	if (h->streak_vertex_count && h->streak_vb) {
		Aeron_BindGraphicsPipeline(pass, h->streak_pipeline);
		Aeron_BindVertexBuffer(pass, 0, h->streak_vb, 0);
		Aeron_BindUniformData(pass, AERON_SHADER_STAGE_VERTEX, 0, h->view_proj, sizeof h->view_proj);
		Aeron_Draw(pass, h->streak_vertex_count, 0);
	}
}

int XwHyperspace_Lighting(AeronScene3D* scene, XwHyperLighting* out) {
	const XwHyperspace* h = &g_hyper;
	if (!h->draw_background || h->lighting_scene != scene)
		return 0;
	const XwHyperspaceSettings* p = &XwRemasterConfig_Effective()->hyperspace;
	*out = (XwHyperLighting) { .texture = h->environment_cube,
							   .sampler = h->environment_sampler,
							   .direction = { 0, 1, 0 } };
	for (int c = 0; c < 3; ++c)
		out->color[c] = p->cap_color[c] * p->brightness * p->highlight_strength * p->mesh_key_strength;
	return 1;
}
