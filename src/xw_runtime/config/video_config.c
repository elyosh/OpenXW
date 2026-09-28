/* Enum parsing and changed-field persistence follow OpenXvT's video settings. */
#include "xw_runtime/config/video_config.h"
#include "xw_runtime/config/settings.h"
#include <string.h>

static const char* const temporal_modes[] = { "off", "native_aa", "quality", "balanced", "performance" };
static const char* const sky_modes[] = { "stars", "cube" };
static const char* const shadow_modes[] = { "off", "pcf" };
static const char* const shadow_fits[] = { "stable", "frustum", "scene_dependent" };
static const char* const tonemap_modes[] = { "aces", "agx" };
static const char* const tonemap_looks[] = { "base", "punchy" };

static int ReadChoice(const AeronConfigFile* document, const char* path, const char* const* names,
					  size_t count, int* out, char* error, size_t capacity) {
	const char* value = AeronConfigNode_String(AeronConfigFile_GetNode(document, path), NULL);
	for (size_t i = 0; value && i < count; ++i) {
		if (!strcmp(value, names[i])) {
			*out = (int)i;
			return 1;
		}
	}
	return XwSettings_NodeError(document, path, "missing or unsupported option", error, capacity);
}

int XwRenderSettings_ReadChoices(const AeronConfigFile* document, XwRenderSettings* out, char* error,
								 size_t capacity) {
	int temporal;
	if (!ReadChoice(document, "render.temporal_upscaling.mode", temporal_modes, 5, &temporal, error,
					capacity))
		return 0;
	out->temporal_mode = (AeronTemporalMode)temporal;
	const char* sky = AeronConfigNode_String(AeronConfigFile_GetNode(document, "skybox.mode"), "");
	if (!strcmp(sky, "procedural"))
		out->sky.mode = XW_SKY_STARS;
	else if (!ReadChoice(document, "skybox.mode", sky_modes, 2, &out->sky.mode, error, capacity))
		return 0;
	const int samples[] = { out->msaa_samples, out->dos_msaa_samples };
	const char* paths[] = { "render.msaa_samples", "render.dos.msaa_samples" };
	for (unsigned i = 0; i < 2; ++i)
		if (samples[i] != 1 && samples[i] != 2 && samples[i] != 4 && samples[i] != 8)
			return XwSettings_NodeError(document, paths[i], "expected 1, 2, 4 or 8", error, capacity);
	char normalized[XW_PATH_CAPACITY];
	if ((out->sky.mode == XW_SKY_CUBE && !out->sky.path[0]) ||
		(out->sky.path[0] && !XwStorage_Normalize(out->sky.path, normalized, sizeof normalized)))
		return XwSettings_NodeError(document, "skybox.path", "expected a relative resource path", error,
									capacity);
	return 1;
}

static int WriteChoice(AeronConfigFile* document, const char* path, const char* const* names, size_t count,
					   int value, int previous, int defaults, AeronConfigError* detail) {
	if ((unsigned)value >= count)
		return 0;
	if (value == previous)
		return 1;
	if (value == defaults)
		return !AeronConfigFile_Has(document, path) || AeronConfigFile_Remove(document, path, detail);
	return AeronConfigFile_SetString(document, path, names[value], detail);
}

static int WriteNumber(AeronConfigFile* document, const char* path, double value, double previous,
					   double defaults, int kind, AeronConfigError* detail) {
	if (value == previous)
		return 1;
	if (value == defaults)
		return !AeronConfigFile_Has(document, path) || AeronConfigFile_Remove(document, path, detail);
	if (kind == 0)
		return AeronConfigFile_SetFloat(document, path, value, detail);
	if (kind == 1)
		return AeronConfigFile_SetInt(document, path, (int64_t)value, detail);
	return AeronConfigFile_SetBool(document, path, value != 0, detail);
}

static int WriteScene(AeronConfigFile* document, const XwSceneSettings* v, const XwSceneSettings* p,
					  const XwSceneSettings* d, AeronConfigError* detail) {
#define NUMBER(path, member, kind)                                                                           \
	if (!WriteNumber(document, path, v->member, p->member, d->member, kind, detail))                         \
	return 0
#define CHOICE(path, member, names)                                                                          \
	if (!WriteChoice(document, path, names, sizeof names / sizeof names[0], v->member, p->member, d->member, \
					 detail))                                                                                \
	return 0
	NUMBER("render.ssao.quality", ssao.ssao_quality, 1);
	NUMBER("render.ssao.intensity", ssao.ssao_intensity, 0);
	NUMBER("render.ssao.power", ssao.ssao_power, 0);
	NUMBER("render.ssao.radius_view", ssao.ssao_radius_view, 0);
	NUMBER("render.ssao.bias_view", ssao.ssao_bias_view, 0);
	NUMBER("render.ssao.direct", ssao.ssao_direct, 0);
	NUMBER("render.ssao.debug_viz", ssao.ssao_debug_viz, 2);
	NUMBER("render.ssao.min_screen_frac", ssao.ssao_min_screen_frac, 0);
	NUMBER("render.ssao.max_screen_frac", ssao.ssao_max_screen_frac, 0);
	NUMBER("render.ssao.sample_jitter", ssao.ssao_sample_jitter, 0);
	CHOICE("render.shadows.mode", shadows.enabled, shadow_modes);
	CHOICE("render.shadows.fit_mode", shadows.fit_mode, shadow_fits);
	NUMBER("render.shadows.atlas_size", shadows.atlas_size, 1);
	NUMBER("render.shadows.cascade_count", shadows.cascade_count, 1);
	NUMBER("render.shadows.max_distance", shadows.max_distance, 0);
	NUMBER("render.shadows.split_lambda", shadows.split_lambda, 0);
	NUMBER("render.shadows.explicit_splits", shadows.explicit_splits, 2);
	NUMBER("render.shadows.split_1", shadows.split_positions[0], 0);
	NUMBER("render.shadows.split_2", shadows.split_positions[1], 0);
	NUMBER("render.shadows.split_3", shadows.split_positions[2], 0);
	NUMBER("render.shadows.filter_quality", shadows.filter_quality, 1);
	NUMBER("render.shadows.filter_radius", shadows.filter_radius, 0);
	NUMBER("render.shadows.contact_hardening", shadows.contact_hardening, 2);
	NUMBER("render.shadows.light_angular_radius_degrees", shadows.light_angular_radius_degrees, 0);
	NUMBER("render.shadows.max_filter_radius", shadows.max_filter_radius, 0);
	NUMBER("render.shadows.pcss_min_filter_radius", shadows.pcss_min_filter_radius, 0);
	NUMBER("render.shadows.normal_bias_texels", shadows.normal_bias_texels, 0);
	NUMBER("render.shadows.depth_bias_texels", shadows.depth_bias_texels, 0);
	NUMBER("render.shadows.transition_fraction", shadows.transition_fraction, 0);
	NUMBER("render.shadows.distance_fade_fraction", shadows.distance_fade_fraction, 0);
	NUMBER("render.shadows.debug_cascades", shadows.debug_cascades, 2);
	CHOICE("render.tonemap.operator", tonemap.tonemap_operator, tonemap_modes);
	CHOICE("render.tonemap.agx_look", tonemap.agx_look, tonemap_looks);
	NUMBER("render.tonemap.agx_eotf_exponent", tonemap.agx_eotf_exponent, 0);
	NUMBER("render.tonemap.agx_punchy_power", tonemap.agx_punchy_power, 0);
	NUMBER("render.tonemap.agx_punchy_saturation", tonemap.agx_punchy_saturation, 0);
	NUMBER("render.tonemap.aces_pre_exposure", tonemap.aces_pre_exposure, 0);
#undef NUMBER
#undef CHOICE
	return 1;
}

int XwRenderSettings_WriteChoices(AeronConfigFile* document, const XwRenderSettings* value,
								  const XwRenderSettings* previous, const XwRenderSettings* defaults,
								  char* error, size_t capacity) {
	AeronConfigError detail = { 0 };
	if ((unsigned)value->temporal_mode >= 5 || (unsigned)value->sky.mode >= 2 ||
		(unsigned)value->scene.shadows.enabled >= 2 || value->scene.shadows.fit_mode >= 3 ||
		(unsigned)value->scene.tonemap.tonemap_operator >= 2 || (unsigned)value->scene.tonemap.agx_look >= 2)
		return XwSettings_NodeError(document, "render", "unsupported renderer option", error, capacity);
	if (!WriteChoice(document, "render.temporal_upscaling.mode", temporal_modes, 5, value->temporal_mode,
					 previous->temporal_mode, defaults->temporal_mode, &detail) ||
		!WriteChoice(document, "skybox.mode", sky_modes, 2, value->sky.mode, previous->sky.mode,
					 defaults->sky.mode, &detail) ||
		!WriteScene(document, &value->scene, &previous->scene, &defaults->scene, &detail))
		return XwSettings_FileError(&detail, error, capacity);
	return 1;
}
