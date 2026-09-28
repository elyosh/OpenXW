/* Authored OPT emitter geometry and quad construction adapted from OpenXvT. */
#include "xw_remaster/engine_glows.h"
#include "xw_remaster/config.h"
#include "xw_remaster/lighting.h"
#include <aeron/aeron.h>
#include <aeron/asset/opt_model.h>
#include <aeron/scene/billboard.h>
#include <math.h>
#include <string.h>

static AeronTexture* g_mask;

static float Linear(float value) {
	return value <= .04045f ? value / 12.92f : powf((value + .055f) / 1.055f, 2.4f);
}

static int Upload(AeronCommandBuffer* cmd) {
	if (g_mask)
		return 1;
	/* Coverage follows OpenXvT's procedural mask. Only coverage is procedural;
	 * emitter geometry and both colors remain authored OPT data. */
	uint8_t pixels[32 * 32 * 4];
	for (int y = 0; y < 32; ++y)
		for (int x = 0; x < 32; ++x) {
			float dx = ((float)x + .5f - 16) / 16, dy = ((float)y + .5f - 16) / 16;
			float a = fmaxf(0, 1 - dx * dx - dy * dy);
			uint8_t v = (uint8_t)(a * a * 255);
			for (int c = 0; c < 4; ++c)
				pixels[(y * 32 + x) * 4 + c] = v;
		}
	g_mask = Aeron_CreateTexture(
		&(AeronTextureDesc) { .width = 32,
							  .height = 32,
							  .format = AERON_TEXTURE_FORMAT_RGBA8_UNORM,
							  .usage = AERON_TEXTURE_USAGE_SAMPLED | AERON_TEXTURE_USAGE_TRANSFER_DST,
							  .debug_name = "xw.engine_glow.coverage" });
	if (!g_mask)
		return 0;
	if (!Aeron_UploadTextureDataCmd(cmd, &(AeronTextureUploadDesc) { .texture = g_mask,
																	 .width = 32,
																	 .height = 32,
																	 .raw_data = pixels,
																	 .raw_size = sizeof pixels })) {
		Aeron_DestroyTexture(g_mask);
		g_mask = NULL;
		return 0;
	}
	return 1;
}

bool XwEngineGlows_Prepare(void) {
	if (g_mask)
		return true;
	AeronCommandBuffer* cmd = Aeron_AcquireCommandBuffer();
	if (!cmd)
		return false;
	if (!Upload(cmd)) {
		Aeron_CancelCommandBuffer(cmd);
		return false;
	}
	if (!Aeron_SubmitCommandBuffer(cmd)) {
		XwEngineGlows_Shutdown();
		return false;
	}
	return true;
}

static void Submit(AeronScene3D* scene, const AeronSceneMesh* mesh, const float transform[16],
				   float model_scale, const AeronSceneMeshTable* table, float scale, const float crows[9],
				   const float cam_pos[3], float focal, AeronTexture* texture) {
	if (!scene || !mesh || !mesh->engine_glow_count || !transform || scale <= 0.0f ||
		XwRemasterConfig_Effective()->models.engine_emissive_strength <= 0.0f || !texture) {
		return;
	}
	const float k = model_scale > 0.0f ? model_scale : 1.0f;
	/* Classic corner UVs, RenderQuad_DrawGlow rim order; center UV is
	 * the sub-rect middle (scene derives center pos/UV as averages). */
	static const float rim_uv[4][2] = {
		{ 0.0f, 0.0f },
		{ 1.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 0.0f, 1.0f },
	};

	for (uint32_t gi = 0; gi < mesh->engine_glow_count; gi++) {
		const AeronFlightEngineGlow* g = &mesh->engine_glows[gi];
		if (!g->enabled ||
			(table && (g->component_index >= AERON_MAX_MESH_SLOTS ||
					   !table->visibility_packed[g->component_index >> 2][g->component_index & 3]))) {
			continue;
		}

		/* Emitter anchor + axes in model space, articulated by the
		 * instance's mesh table (the same affine the mesh's vertices
		 * get in the VS — glows follow their rotary mesh exactly). */
		float p[3] = { g->position.x, g->position.y, g->position.z };
		float ax_look[3] = { g->look.x, g->look.y, g->look.z };
		float ax_right[3] = { g->right.x, g->right.y, g->right.z };
		float ax_up[3] = { g->up.x, g->up.y, g->up.z };
		if (table && g->component_index < AERON_MAX_MESH_SLOTS) {
			const float (*rw)[4] = table->rows[g->component_index];
			float tp[3], tv[3];
			for (int r = 0; r < 3; r++) {
				tp[r] = rw[r][0] * p[0] + rw[r][1] * p[1] + rw[r][2] * p[2] + rw[r][3];
			}
			memcpy(p, tp, sizeof tp);
			for (int r = 0; r < 3; r++)
				tv[r] = rw[r][0] * ax_look[0] + rw[r][1] * ax_look[1] + rw[r][2] * ax_look[2];
			memcpy(ax_look, tv, sizeof tv);
			for (int r = 0; r < 3; r++)
				tv[r] = rw[r][0] * ax_right[0] + rw[r][1] * ax_right[1] + rw[r][2] * ax_right[2];
			memcpy(ax_right, tv, sizeof tv);
			for (int r = 0; r < 3; r++)
				tv[r] = rw[r][0] * ax_up[0] + rw[r][1] * ax_up[1] + rw[r][2] * ax_up[2];
			memcpy(ax_up, tv, sizeof tv);
		}

		/* Model -> instance space (transform is fl_model_matrix layout:
		 * rows of the 3x4 are the space-axes' model components; scale k
		 * rides the matrix — axes divide it back out). */
		float sp[3], s_look[3], s_right[3], s_up[3];
		for (int r = 0; r < 3; r++) {
			sp[r] = transform[r * 4 + 0] * p[0] + transform[r * 4 + 1] * p[1] + transform[r * 4 + 2] * p[2] +
					transform[r * 4 + 3];
			s_look[r] = (transform[r * 4 + 0] * ax_look[0] + transform[r * 4 + 1] * ax_look[1] +
						 transform[r * 4 + 2] * ax_look[2]) /
						k;
			s_right[r] = (transform[r * 4 + 0] * ax_right[0] + transform[r * 4 + 1] * ax_right[1] +
						  transform[r * 4 + 2] * ax_right[2]) /
						 k;
			s_up[r] = (transform[r * 4 + 0] * ax_up[0] + transform[r * 4 + 1] * ax_up[1] +
					   transform[r * 4 + 2] * ax_up[2]) /
					  k;
		}

		/* Space -> view (identity camera when crows/cam_pos are NULL). */
		float c[3], look_v[3], right_v[3], up_v[3];
		if (crows && cam_pos) {
			const float d0 = sp[0] - cam_pos[0], d1 = sp[1] - cam_pos[1], d2 = sp[2] - cam_pos[2];
			for (int r = 0; r < 3; r++) {
				c[r] = crows[r * 3 + 0] * d0 + crows[r * 3 + 1] * d1 + crows[r * 3 + 2] * d2;
				look_v[r] = crows[r * 3 + 0] * s_look[0] + crows[r * 3 + 1] * s_look[1] +
							crows[r * 3 + 2] * s_look[2];
				right_v[r] = crows[r * 3 + 0] * s_right[0] + crows[r * 3 + 1] * s_right[1] +
							 crows[r * 3 + 2] * s_right[2];
				up_v[r] =
					crows[r * 3 + 0] * s_up[0] + crows[r * 3 + 1] * s_up[1] + crows[r * 3 + 2] * s_up[2];
			}
		} else {
			memcpy(c, sp, sizeof c);
			memcpy(look_v, s_look, sizeof look_v);
			memcpy(right_v, s_right, sizeof right_v);
			memcpy(up_v, s_up, sizeof up_v);
		}

		/* Retain the reference eye-plane/size culls using the native pixel focal length. */
		const float dim_x = k * g->dimensions.x;
		const float dim_y = k * g->dimensions.y;
		const float dim_z = k * g->dimensions.z;
		if (c[2] < 1.0f) {
			continue;
		}
		float maxdim = dim_x > dim_y ? dim_x : dim_y;
		if (dim_z > maxdim) {
			maxdim = dim_z;
		}
		if (maxdim * focal / c[2] < 1.0f) {
			continue;
		}

		/* Corner build — rect for elongated dims, view-aligned diamond
		 * for round ones (ratio in (0.85, 1.2)); the classic clamps the
		 * geometric scale at 0.8. */
		const float clamped = scale >= 0.80000001f ? 0.80000001f : scale;
		float corners[4][3];
		const float ratio = g->dimensions.y != 0.0f ? g->dimensions.x / g->dimensions.y : 1.0f;
		if (ratio <= 0.85000002f || ratio >= 1.2f) {
			/* Classic axis pairing (EngineGlow_BuildRectQuadCorners via
			 * BuildProjectedQuad's call: firstAxisView = the rotated
			 * RIGHT axis rides the `upAxisView` param with dims.x, the
			 * UP axis the `rightAxisView` param with dims.y). */
			const float as = dim_x * clamped * 0.5f;
			const float bs = dim_y * clamped * 0.5f;
			for (int r = 0; r < 3; r++) {
				const float a = right_v[r] * as;
				const float b = up_v[r] * bs;
				corners[0][r] = c[r] - a + b;
				corners[1][r] = c[r] + a + b;
				corners[2][r] = c[r] + a - b;
				corners[3][r] = c[r] - a - b;
			}
		} else {
			const float radius = clamped * dim_x * 1.415f * 0.5f;
			const float dot_f = right_v[0] * c[0] + right_v[1] * c[1] + right_v[2] * c[2];
			const float dot_s = up_v[0] * c[0] + up_v[1] * c[1] + up_v[2] * c[2];
			const float len = sqrtf(dot_f * dot_f + dot_s * dot_s);
			float major[3], minor[3];
			if (len == 0.0f) {
				memcpy(major, right_v, sizeof major);
				memcpy(minor, up_v, sizeof minor);
			} else {
				const float fw = dot_f / len;
				const float sw = dot_s / len;
				for (int r = 0; r < 3; r++) {
					major[r] = up_v[r] * sw + right_v[r] * fw;
					minor[r] = right_v[r] * sw - up_v[r] * fw;
				}
			}
			for (int r = 0; r < 3; r++) {
				corners[0][r] = c[r] + minor[r] * radius;
				corners[1][r] = c[r] + major[r] * radius;
				corners[2][r] = c[r] - minor[r] * radius;
				corners[3][r] = c[r] - major[r] * radius;
			}
		}

		/* Look-axis depth extrusion (EngineGlow_ExtrudeQuadAlongViewNormal):
		 * per corner, push along the look axis by the corner's depth
		 * delta from the center — the facing-dependent sign and the
		 * asymmetric gains reproduce the classic exactly (10000/32768
		 * on the negative side, scale x dim.z on the positive). */
		{
			const float center_dot = look_v[0] * c[0] + look_v[1] * c[1] + look_v[2] * c[2];
			const float pos_gain = scale * dim_z;
			for (int ci = 0; ci < 4; ci++) {
				const float z_delta = center_dot < 0.0f ? corners[ci][2] - c[2] : c[2] - corners[ci][2];
				const float gain = z_delta < 0.0f ? (10000.0f / 32768.0f) : pos_gain;
				for (int r = 0; r < 3; r++) {
					corners[ci][r] += look_v[r] * z_delta * gain;
				}
			}
		}

		/* Authored OPT colors are sRGB; the coverage mask and tint use PMA. */
		float core[4], outer[4];
		core[3] = g->core_rgba[3];
		outer[3] = g->outer_rgba[3];
		for (int ch = 0; ch < 3; ch++) {
			core[ch] = Linear(g->core_rgba[ch]) * core[3] *
					   XwRemasterConfig_Effective()->models.engine_emissive_strength;
			outer[ch] = Linear(g->outer_rgba[ch]) * outer[3] *
						XwRemasterConfig_Effective()->models.engine_emissive_strength;
		}

		AeronSceneBillboardDesc d;
		memset(&d, 0, sizeof d);
		d.texture = texture;
		d.blend = AERON_SCENE_BILLBOARD_BLEND_PMA;
		d.stage = AERON_SCENE_BILLBOARD_STAGE_OVERLAY;
		d.center_color = core;
		for (int v = 0; v < 4; v++) {
			/* View corners -> submission space (inverse of the camera
			 * rotation above; identity camera passes through). */
			if (crows && cam_pos) {
				for (int r = 0; r < 3; r++) {
					d.corners[v][r] = cam_pos[r] + crows[0 * 3 + r] * corners[v][0] +
									  crows[1 * 3 + r] * corners[v][1] + crows[2 * 3 + r] * corners[v][2];
				}
			} else {
				memcpy(d.corners[v], corners[v], sizeof d.corners[v]);
			}
			d.uv[v][0] = rim_uv[v][0];
			d.uv[v][1] = rim_uv[v][1];
			memcpy(d.colors[v], outer, sizeof outer);
		}
		AeronScene_AddBillboard(scene, &d);
	}
}

static float Drive(const XwSnapCraft* craft) {
	if (!craft || !craft->engine_count || craft->engine_count > 4)
		return 0;
	float power = 0;
	for (unsigned i = 0; i < craft->engine_count; ++i)
		power += craft->engine_output_q16[i] / 65535.0f;
	return power / craft->engine_count;
}

void XwEngineGlows_Lights(AeronScene3D* scene, const AeronSceneMesh* mesh, const XwSnapCraft* craft,
						  const AeronSceneMeshTable* table, const float transform[16]) {
	float power = Drive(craft);
	if (power <= 0)
		return;
	for (unsigned i = 0; i < mesh->engine_glow_count; ++i) {
		const AeronFlightEngineGlow* g = &mesh->engine_glows[i];
		unsigned c = g->component_index;
		if (!g->enabled || c >= AERON_MAX_MESH_SLOTS || !table->visibility_packed[c >> 2][c & 3])
			continue;
		float dx = g->dimensions.x * AERON_OPT_UNITS_PER_METER;
		float dy = g->dimensions.y * AERON_OPT_UNITS_PER_METER;
		if (dx <= 2000 && dy <= 2000)
			continue;
		float local[3], world[3], color[3];
		for (int r = 0; r < 3; ++r) {
			const float* row = table->rows[c][r];
			local[r] = row[0] * g->position.x + row[1] * g->position.y + row[2] * g->position.z + row[3];
			color[r] = Linear(g->core_rgba[r]);
		}
		for (int r = 0; r < 3; ++r)
			world[r] = transform[r * 4] * local[0] + transform[r * 4 + 1] * local[1] +
					   transform[r * 4 + 2] * local[2] + transform[r * 4 + 3];
		XwLighting_AddPoint(scene, world, color, g->dimensions.z * AERON_OPT_UNITS_PER_METER * power * 300,
							power * 16384);
	}
}

bool XwEngineGlows_Submit(AeronScene3D* scene, const XwRenderSnapshot* snapshot, const XwSnapCraft* craft,
						  const XwMeshAsset* asset, const AeronSceneMeshTable* table,
						  const float transform[16], const XwRenderView* view) {
	if (!snapshot->appearance.engine_glow_enabled || !craft || !craft->engine_count ||
		!asset->mesh->engine_glow_count || XwRemasterConfig_Effective()->models.engine_emissive_strength <= 0)
		return true;
	/* Outputs already include engine throttle and power allocation. The OPT has
	 * component ownership, but no mapping to X-Wing's four logical engines. */
	float scale = Drive(craft);
	if (scale <= 0)
		return true;
	if (!g_mask)
		return false;
	float rows[9];
	XwRenderMath_ViewRows(&view->camera, rows);
	float focal = view->camera.viewport.height / (2 * tanf(view->camera.v_half_rad));
	Submit(scene, asset->mesh, transform, AERON_OPT_UNITS_PER_METER, table, scale, rows, view->camera.pos,
		   focal, g_mask);
	return true;
}

void XwEngineGlows_Shutdown(void) {
	Aeron_DestroyTexture(g_mask);
	g_mask = NULL;
}
