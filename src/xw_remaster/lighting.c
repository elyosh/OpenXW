#include "xw_remaster/lighting.h"
#include "xw/flight/object/object.h"
#include "xw_remaster/config.h"
#include "xw_remaster/hyperspace.h"
#include "xw_remaster/render_math.h"
#include <math.h>
#include <string.h>

/* FS b1 — mirrors cbuffer PbrLightFS (scene_pbr_lighting.hlsli). */
typedef struct PbrLightFS {
	float light_intensity;
	float global_spec_mul;
	float debug_isolate_term;
	float light_wrap;
	float xvt_flat;
	float ssao_intensity;
	float ssao_power;
	float ssao_rt_w;
	float ssao_rt_h;
	float ssao_direct;
	float spec_geom_adapt;
	float _pad_tuning;
	float camera_pos_world[3], _pad0;
	float directional_dir[3], _pad1; /* surface -> light */
	float sun_color[3], _pad2;
	float amb_pos_x[3], _pad3;
	float amb_neg_x[3], _pad4;
	float amb_pos_y[3], _pad5;
	float amb_neg_y[3], _pad6;
	float amb_pos_z[3], _pad7;
	float amb_neg_z[3], _pad8;
	/* Additional diffuse-only directionals (backdrop suns/planets;
	 * the classic sums plain Lambert per light). */
	float extra_dir[3][4];
	float extra_col[3][4];
	/* Point-light evaluation: min distance, spec weight, diffuse wrap, cap. */
	float point_params[4];
	/* Detailed diffuse environment intensity and world-to-local basis. */
	float environment_params[4];
	float environment_right[4];
	float environment_up[4];
	float environment_forward[4];
} PbrLightFS;

typedef char XwPbrSizeCheck[sizeof(PbrLightFS) == 368 ? 1 : -1];

void XwLighting_AddPoint(AeronScene3D* scene, const float position[3], const float color[3], float intensity,
						 float minimum_range) {
	const XwPointLightSettings* p = &XwRemasterConfig_Effective()->point_lights;
	if (!p->enabled || !(intensity > 0) || !isfinite(intensity) || p->scale <= 0)
		return;
	/* XWA's 1% visibility window for the shared 0.5 / distance response. */
	AeronSceneLight light = { .radius = fmaxf(minimum_range, intensity * 50) * p->range_scale };
	memcpy(light.pos, position, sizeof light.pos);
	for (int c = 0; c < 3; ++c)
		light.color[c] = color[c] * intensity * p->scale;
	if (!(light.radius > 0) || !isfinite(light.radius))
		return;
	for (int c = 0; c < 3; ++c)
		if (!isfinite(light.pos[c]) || !isfinite(light.color[c]))
			return;
	AeronScene_AddLight(scene, &light);
}

static float ExplosionIntensity(const XwRenderSnapshot* snapshot, const XwSnapObject* o) {
	static const uint16_t large[] = { 16, 16, 192, 320, 480, 320, 320, 320, 320, 192, 96, 48 };
	static const uint16_t small[] = { 16, 16, 48, 96, 64, 32 };
	unsigned frame = o->animation_state;
	switch (o->type) {
		case XW_OBJ_EXPLOSION_133:
		case XW_OBJ_EXPLOSION_134:
		case XW_OBJ_EXPLOSION_135:
		case XW_OBJ_EXPLOSION_136:
			return 8.0f * (frame < sizeof large / sizeof *large ? large[frame] : 16);
		case XW_OBJ_ASTEROID_IMPACT:
		case XW_OBJ_EXPLOSION_138:
			return 8.0f * (frame < sizeof small / sizeof *small ? small[frame] : 16);
		default:
			return fmaxf(0, (float)snapshot->appearance.brightness_q8 - 256) * 8;
	}
}

static float ProjectileLight(unsigned type, float color[3]) {
	const float rebel[3] = { 1, .2f, 0 }, imperial[3] = { 0, 1, .2f }, ion[3] = { .2f, .2f, 1 };
	const float torpedo[3] = { .4f, .2f, 1 }, missile[3] = { 1, .4f, .2f };
	const float* rgb;
	float intensity = 200;
	switch (type) {
		case XW_OBJ_LASER_143:
		case XW_OBJ_LASER_144:
			rgb = rebel;
			break;
		case XW_OBJ_LASER_145:
		case XW_OBJ_LASER_146:
			rgb = imperial;
			break;
		case XW_OBJ_ION_147:
		case XW_OBJ_ION_148:
			rgb = ion;
			break;
		case XW_OBJ_WARHEAD_149:
			rgb = torpedo;
			intensity = 250;
			break;
		case XW_OBJ_TRACKED_WARHEAD:
			rgb = missile;
			intensity = 250;
			break;
		default:
			return 0;
	}
	for (unsigned c = 0; c < 3; ++c)
		color[c] = rgb[c] <= .04045f ? rgb[c] / 12.92f : powf((rgb[c] + .055f) / 1.055f, 2.4f);
	return intensity;
}

static void Shadows(AeronScene3D* scene, const XwRenderSnapshot* s) {
	const AeronSceneShadowSettings* p = &XwRemasterConfig_Effective()->scene.shadows;
	AeronSceneDirectionalShadowDesc d = { .enabled = p->enabled && s->appearance.directional_enabled,
										  .atlas_size = p->atlas_size,
										  .cascade_count = p->cascade_count,
										  .fit_mode = p->fit_mode,
										  .max_distance = p->max_distance,
										  .split_lambda = p->split_lambda,
										  .explicit_splits = p->explicit_splits,
										  .filter_quality = p->filter_quality,
										  .filter_radius = p->filter_radius,
										  .contact_hardening = p->contact_hardening,
										  .light_angular_radius_degrees = p->light_angular_radius_degrees,
										  .max_filter_radius = p->max_filter_radius,
										  .pcss_min_filter_radius = p->pcss_min_filter_radius,
										  .normal_bias_texels = p->normal_bias_texels,
										  .depth_bias_texels = p->depth_bias_texels,
										  .transition_fraction = p->transition_fraction,
										  .distance_fade_fraction = p->distance_fade_fraction,
										  .debug_cascades = p->debug_cascades };
	memcpy(d.split_positions, p->split_positions, sizeof d.split_positions);
	float length = 0;
	for (int c = 0; c < 3; ++c) {
		d.world_origin[c] = s->camera.world_pos[c];
		d.light_dir[c] = (float)s->appearance.direction_q15[c] / 32768;
		length += d.light_dir[c] * d.light_dir[c];
	}
	XwHyperLighting hyper;
	if (XwHyperspace_Lighting(scene, &hyper)) {
		memcpy(d.light_dir, hyper.direction, sizeof d.light_dir);
		d.enabled = p->enabled;
		length = 1;
	}
	if (length <= 1e-10f)
		d.enabled = 0;
	else
		for (int c = 0; c < 3; ++c)
			d.light_dir[c] /= sqrtf(length);
	AeronScene_SetDirectionalShadow(scene, &d);
}

int XwLighting_Begin(AeronScene3D* scene, const XwRenderSnapshot* s) {
	const XwPointLightSettings* p = &XwRemasterConfig_Effective()->point_lights;
	Shadows(scene, s);
	if (!AeronScene_SetClusteredLights(
			scene, &(AeronSceneClusteredLightDesc) { .enabled = p->enabled && p->clustered,
													 .depth_slices = (uint32_t)p->cluster_depth_slices,
													 .min_distance = p->min_distance,
													 .contribution_cap = p->contrib_cap,
													 .debug_view = p->cluster_debug }))
		return 0;
	if (!p->enabled)
		return 1;
	for (unsigned i = 0; i < s->object_count; ++i) {
		const XwSnapObject* o = &s->objects[i];
		if (o->slot_class == XW_SNAP_SLOT_MISSION)
			continue;
		float intensity = 0, color[3] = { 1, 1, 1 }, minimum_range = 1024, position[3];
		if (o->genus == XW_GENUS_EXPLOSION_EFFECT) {
			intensity = ExplosionIntensity(s, o);
			/* OpenTIE's sRGB explosion color (0.9, 0.5, 0.2), converted to linear. */
			color[0] = .7874123f;
			color[1] = .2140411f;
			color[2] = .03310477f;
			minimum_range = 0;
		} else if (o->genus == XW_GENUS_PLAYER_PROJECTILE || o->genus == XW_GENUS_OTHER_PROJECTILE)
			intensity = ProjectileLight(o->type, color);
		XwRenderMath_Local(s->camera.world_pos, o->world_pos, position);
		XwLighting_AddPoint(scene, position, color, intensity, minimum_range);
	}
	return 1;
}

void XwLighting_Environment(AeronScene3D* scene, const XwSnapAppearance* light, const float position[3]) {
	const XwLightingSettings* settings = &XwRemasterConfig_Effective()->lighting;
	PbrLightFS env = { 0 };
	const XwRenderSettings* config = XwRemasterConfig_Effective();
	int width, height;
	AeronScene_RenderDims(scene, &width, &height);
	env.ssao_intensity = config->scene.ssao.ssao_quality ? config->scene.ssao.ssao_intensity : 0;
	env.ssao_power = config->scene.ssao.ssao_power;
	env.ssao_direct = config->scene.ssao.ssao_direct;
	env.ssao_rt_w = (float)width;
	env.ssao_rt_h = (float)height;
	env.point_params[0] = config->point_lights.min_distance;
	env.point_params[1] = config->point_lights.spec_weight;
	env.point_params[2] = config->point_lights.diffuse_wrap;
	env.point_params[3] = config->point_lights.contrib_cap;
	AeronScene_SetPbrDebugViews(scene, !settings->spec_geom_adapt);
	AeronScene_SetPbrEnvironmentMap(scene, NULL, NULL);
	env.light_intensity = settings->intensity;
	env.global_spec_mul = settings->spec_mul;
	env.light_wrap = settings->wrap;
	env.spec_geom_adapt = settings->spec_geom_adapt;
	if (position)
		memcpy(env.camera_pos_world, position, sizeof env.camera_pos_world);
	float direction[3];
	/* XvT dots this direction with the normal and traces toward it for occlusion. */
	for (int i = 0; i < 3; ++i)
		direction[i] = (float)light->direction_q15[i] / 32768.0f;
	for (int i = 0; i < 3; ++i) {
		env.directional_dir[i] = direction[i];
		env.sun_color[i] = light->directional_enabled ? 1 : 0;
		env.amb_pos_x[i] = env.amb_neg_x[i] = env.amb_pos_y[i] = env.amb_neg_y[i] = env.amb_pos_z[i] =
			env.amb_neg_z[i] = settings->ambient[i];
	}
	float length = sqrtf(env.directional_dir[0] * env.directional_dir[0] +
						 env.directional_dir[1] * env.directional_dir[1] +
						 env.directional_dir[2] * env.directional_dir[2]);
	if (length > 1e-5f)
		for (int i = 0; i < 3; ++i)
			env.directional_dir[i] /= length;
	XwHyperLighting hyper;
	if (XwHyperspace_Lighting(scene, &hyper)) {
		memcpy(env.directional_dir, hyper.direction, sizeof hyper.direction);
		memcpy(env.sun_color, hyper.color, sizeof hyper.color);
		env.environment_params[0] = config->hyperspace.mesh_ambient_strength;
		env.environment_right[0] = env.environment_up[2] = env.environment_forward[1] = 1;
		AeronScene_SetPbrEnvironmentMap(scene, hyper.texture, hyper.sampler);
	}
	AeronScene_SetFrameUniformData(scene, AERON_SHADER_STAGE_FRAGMENT, 1, &env, sizeof env);
}
