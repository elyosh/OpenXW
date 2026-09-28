#include "xw_runtime/config/settings.h"
#include "xw_runtime/config/controller_config.h"
#include "xw_runtime/config/keyboard_config.h"
#include "xw_runtime/config/preferences.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static const char* const mouse_modes[] = { "virtual_stick", "classic" };

typedef enum XwSettingType {
	XW_SETTING_BOOL,
	XW_SETTING_INT,
	XW_SETTING_FLOAT,
	XW_SETTING_STRING
} XwSettingType;

typedef struct XwSettingField {
	const char* path;
	size_t offset;
	XwSettingType type;
	double minimum, maximum;
} XwSettingField;

#define SETTING_BOOL(path, member) { path, offsetof(XwSettings, member), XW_SETTING_BOOL, 0, 1 }
#define SETTING_INT(path, member, low, high) { path, offsetof(XwSettings, member), XW_SETTING_INT, low, high }
#define SETTING_FLOAT(path, member, low, high)                                                               \
	{ path, offsetof(XwSettings, member), XW_SETTING_FLOAT, low, high }
#define SETTING_STRING(path, member)                                                                         \
	{ path, offsetof(XwSettings, member), XW_SETTING_STRING, 0, sizeof(((XwSettings*)0)->member) }

static const XwSettingField g_fields[] = {
	SETTING_INT("render.msaa_samples", render.msaa_samples, 1, 8),
	SETTING_BOOL("render.cockpit_undither", render.cockpit_undither),
	SETTING_FLOAT("render.temporal_upscaling.sharpness", render.temporal_sharpness, 0, 1),
	SETTING_BOOL("render.motion_blur.camera_blur", render.motion_blur.camera_blur),
	SETTING_BOOL("render.motion_blur.pause_keep_blur", render.motion_blur.pause_keep_blur),
	SETTING_BOOL("render.motion_blur.velocity_viz", render.motion_blur.velocity_viz),
	SETTING_BOOL("render.motion_blur.fsr_direct_motion", render.motion_blur.fsr_direct_motion),
	SETTING_INT("render.motion_blur.quality", render.motion_blur.quality, 0, 2),
	SETTING_FLOAT("render.motion_blur.shutter", render.motion_blur.shutter, 0, 1),
	SETTING_BOOL("texture_filtering.anisotropic", render.anisotropic),
	SETTING_FLOAT("texture_filtering.max_anisotropy", render.max_anisotropy, 1, 16),
	SETTING_FLOAT("models.smooth_angle_degrees", render.models.smooth_angle_degrees, 0, 180),
	SETTING_FLOAT("models.opt_emissive_strength", render.models.opt_emissive_strength, 0, FLT_MAX),
	SETTING_FLOAT("models.opt_projectile_emissive_strength", render.models.opt_projectile_emissive_strength,
				  0, FLT_MAX),
	SETTING_FLOAT("models.engine_emissive_strength", render.models.engine_emissive_strength, 0, FLT_MAX),
	SETTING_BOOL("point_lights.enabled", render.point_lights.enabled),
	SETTING_BOOL("point_lights.clustered", render.point_lights.clustered),
	SETTING_BOOL("point_lights.cluster_debug", render.point_lights.cluster_debug),
	SETTING_INT("point_lights.cluster_depth_slices", render.point_lights.cluster_depth_slices, 4, 64),
	SETTING_FLOAT("point_lights.scale", render.point_lights.scale, 0, FLT_MAX),
	SETTING_FLOAT("point_lights.range_scale", render.point_lights.range_scale, FLT_MIN, FLT_MAX),
	SETTING_FLOAT("point_lights.min_distance", render.point_lights.min_distance, FLT_MIN, FLT_MAX),
	SETTING_FLOAT("point_lights.spec_weight", render.point_lights.spec_weight, 0, FLT_MAX),
	SETTING_FLOAT("point_lights.diffuse_wrap", render.point_lights.diffuse_wrap, 0, 1),
	SETTING_FLOAT("point_lights.contrib_cap", render.point_lights.contrib_cap, 0, FLT_MAX),
	SETTING_FLOAT("lighting.intensity", render.lighting.intensity, 0, FLT_MAX),
	SETTING_FLOAT("lighting.spec_mul", render.lighting.spec_mul, 0, FLT_MAX),
	SETTING_FLOAT("lighting.wrap", render.lighting.wrap, 0, 1),
	SETTING_BOOL("lighting.spec_geom_adapt", render.lighting.spec_geom_adapt),
	SETTING_FLOAT("lighting.ambient_r", render.lighting.ambient[0], 0, FLT_MAX),
	SETTING_FLOAT("lighting.ambient_g", render.lighting.ambient[1], 0, FLT_MAX),
	SETTING_FLOAT("lighting.ambient_b", render.lighting.ambient[2], 0, FLT_MAX),
	SETTING_BOOL("skybox.enabled", render.sky.enabled),
	SETTING_STRING("skybox.path", render.sky.path),
	SETTING_FLOAT("skybox.exposure", render.sky.exposure, 0, FLT_MAX),
	SETTING_FLOAT("skybox.star_brightness", render.sky.star_brightness, 0, FLT_MAX),
	SETTING_FLOAT("hyperspace_tunnel.travel_speed", render.hyperspace.travel_speed, FLT_MIN, 64),
	SETTING_FLOAT("hyperspace_tunnel.rotation_speed", render.hyperspace.rotation_speed, -8, 8),
	SETTING_FLOAT("hyperspace_tunnel.noise_scale", render.hyperspace.noise_scale, 0.125, 8),
	SETTING_FLOAT("hyperspace_tunnel.brightness", render.hyperspace.brightness, 0, 16),
	SETTING_FLOAT("hyperspace_tunnel.highlight_strength", render.hyperspace.highlight_strength, 0, 16),
	SETTING_FLOAT("hyperspace_tunnel.focal_length", render.hyperspace.focal_length, 0.25, 4),
	SETTING_FLOAT("hyperspace_tunnel.twist", render.hyperspace.twist, -2, 2),
	SETTING_FLOAT("hyperspace_tunnel.cap_radius", render.hyperspace.cap_radius, 0.001, 1),
	SETTING_FLOAT("hyperspace_tunnel.cap_falloff", render.hyperspace.cap_falloff, 0.1, 64),
	SETTING_FLOAT("hyperspace_tunnel.mesh_ambient_strength", render.hyperspace.mesh_ambient_strength, 0, 4),
	SETTING_FLOAT("hyperspace_tunnel.mesh_environment_roughness",
				  render.hyperspace.mesh_environment_roughness, 0, 1),
	SETTING_FLOAT("hyperspace_tunnel.mesh_key_strength", render.hyperspace.mesh_key_strength, 0, 4),
	SETTING_FLOAT("hyperspace_tunnel.dark_color_r", render.hyperspace.dark_color[0], 0, 16),
	SETTING_FLOAT("hyperspace_tunnel.dark_color_g", render.hyperspace.dark_color[1], 0, 16),
	SETTING_FLOAT("hyperspace_tunnel.dark_color_b", render.hyperspace.dark_color[2], 0, 16),
	SETTING_FLOAT("hyperspace_tunnel.body_color_r", render.hyperspace.body_color[0], 0, 16),
	SETTING_FLOAT("hyperspace_tunnel.body_color_g", render.hyperspace.body_color[1], 0, 16),
	SETTING_FLOAT("hyperspace_tunnel.body_color_b", render.hyperspace.body_color[2], 0, 16),
	SETTING_FLOAT("hyperspace_tunnel.highlight_color_r", render.hyperspace.highlight_color[0], 0, 16),
	SETTING_FLOAT("hyperspace_tunnel.highlight_color_g", render.hyperspace.highlight_color[1], 0, 16),
	SETTING_FLOAT("hyperspace_tunnel.highlight_color_b", render.hyperspace.highlight_color[2], 0, 16),
	SETTING_FLOAT("hyperspace_tunnel.cap_color_r", render.hyperspace.cap_color[0], 0, 16),
	SETTING_FLOAT("hyperspace_tunnel.cap_color_g", render.hyperspace.cap_color[1], 0, 16),
	SETTING_FLOAT("hyperspace_tunnel.cap_color_b", render.hyperspace.cap_color[2], 0, 16),
	SETTING_FLOAT("bloom.intensity", render.bloom_intensity, 0, FLT_MAX),
	SETTING_FLOAT("effects.explosion_genus_emissive_strength", render.explosion_emissive_strength, 0,
				  FLT_MAX),
	SETTING_INT("render.dos.msaa_samples", render.dos_msaa_samples, 1, 8),
	SETTING_STRING("frontend.version", frontend_version),
	SETTING_STRING("frontend.inflight_version", inflight_frontend_version),
	SETTING_STRING("flight.version", flight_version),
	SETTING_BOOL("startup.skip_intro", skip_intro),
	SETTING_BOOL("audio.sb16_filter", sb16_filter_enabled),
	SETTING_INT("audio.player_engine_sound_volume_percent", player_engine_sound_volume_percent, 0, 100),
	SETTING_BOOL("game.starfighter_collision_damage", starfighter_collision_damage),
	SETTING_BOOL("game.player_invulnerable", player_invulnerable),
	SETTING_BOOL("game.unlimited_ammunition", unlimited_ammunition),
	SETTING_STRING("music.arrangement", music.arrangement),
	SETTING_STRING("music.backend", music.backend),
	SETTING_STRING("music.soundfont", music.soundfont),
	SETTING_STRING("music.sc55_rom_directory", music.sc55_rom_directory),
	SETTING_STRING("music.mt32_control", music.mt32_control),
	SETTING_STRING("music.mt32_pcm", music.mt32_pcm),
	SETTING_STRING("paths.installations.xw93", xw93_data),
	SETTING_STRING("paths.installations.xw94", xw94_data),
	SETTING_STRING("paths.installations.xw98", xw98_data),
	SETTING_STRING("ui.font", ui_font),
	SETTING_INT("presentation.vsync_divisor", presentation.vsync_divisor, 1, 2),
	SETTING_BOOL("presentation.hdr_output", presentation.hdr_output),
	SETTING_BOOL("input.mouse_flight", mouse_flight),
	SETTING_INT("input.mouse_sensitivity", mouse_sensitivity, 1, 9),
	SETTING_BOOL("input.mouse_invert_y", mouse_invert_y),
};

#undef SETTING_BOOL
#undef SETTING_INT
#undef SETTING_STRING
#undef SETTING_FLOAT

static const char* XwSettings_RootName(AeronVfsRoot root) {
	switch (root) {
		case AERON_VFS_ROOT_RESOURCE:
			return "RESOURCE";
		case AERON_VFS_ROOT_USER:
			return "USER";
		case AERON_VFS_ROOT_TEMP:
			return "TEMP";
		default:
			return "ASSET";
	}
}

int XwSettings_FileError(const AeronConfigError* detail, char* error, size_t capacity) {
	snprintf(error, capacity, "%s/%s:%d:%d: %s", XwSettings_RootName(detail->root), detail->path,
			 detail->line, detail->column, detail->message);
	return 0;
}

int XwSettings_NodeError(const AeronConfigFile* document, const char* path, const char* message, char* error,
						 size_t capacity) {
	const AeronConfigNode* node = AeronConfigFile_GetNode(document, path);
	const char* source;
	if (!node)
		node = AeronConfigFile_Root(document);
	source = AeronConfigNode_SourcePath(node);
	snprintf(error, capacity, "%s/%s:%d:%d: %s: %s", XwSettings_RootName(AeronConfigNode_SourceRoot(node)),
			 source ? source : "config.yaml", AeronConfigNode_Line(node), AeronConfigNode_Column(node), path,
			 message);
	return 0;
}

static int XwSettings_ReadField(const AeronConfigFile* document, const XwSettingField* field,
								XwSettings* settings, char* error, size_t capacity) {
	const AeronConfigNode* node = AeronConfigFile_GetNode(document, field->path);
	AeronConfigNodeType type = AeronConfigNode_Type(node);
	unsigned char* destination = (unsigned char*)settings + field->offset;
	const char* problem = "missing required setting or incorrect value type";
	if (field->type == XW_SETTING_STRING && type == AERON_CONFIG_STRING) {
		const char* value = AeronConfigNode_String(node, "");
		if (strlen(value) < (size_t)field->maximum) {
			memcpy(destination, value, strlen(value) + 1);
			return 1;
		}
		problem = "text exceeds the supported length";
	} else if (field->type == XW_SETTING_BOOL && type == AERON_CONFIG_BOOL) {
		int value = AeronConfigNode_Bool(node, 0);
		memcpy(destination, &value, sizeof(value));
		return 1;
	} else if (field->type == XW_SETTING_INT && type == AERON_CONFIG_INT) {
		int64_t value = AeronConfigNode_Int(node, 0);
		if ((double)value >= field->minimum && (double)value <= field->maximum) {
			int integer = (int)value;
			memcpy(destination, &integer, sizeof(integer));
			return 1;
		}
		problem = "integer outside the supported range";
	} else if (field->type == XW_SETTING_FLOAT && (type == AERON_CONFIG_FLOAT || type == AERON_CONFIG_INT)) {
		double value = AeronConfigNode_Float(node, 0);
		if (isfinite(value) && value >= field->minimum && value <= field->maximum) {
			float number = (float)value;
			memcpy(destination, &number, sizeof(number));
			return 1;
		}
		problem = "number must be finite and within the supported range";
	}
	return XwSettings_NodeError(document, field->path, problem, error, capacity);
}

static int XwSettings_ReadChoice(const AeronConfigFile* document, const char* path,
								 const char* const* choices, size_t count, int* out, char* error,
								 size_t capacity) {
	const AeronConfigNode* node = AeronConfigFile_GetNode(document, path);
	const char* value = AeronConfigNode_String(node, NULL);
	if (value) {
		for (size_t i = 0; i < count; ++i) {
			if (!strcmp(value, choices[i])) {
				*out = (int)i;
				return 1;
			}
		}
	}
	return XwSettings_NodeError(document, path, "missing or unsupported option", error, capacity);
}

static int XwSettings_ReadDisplay(const AeronConfigFile* document, XwPresentationSettings* out, char* error,
								  size_t capacity) {
	const char* gamma_path = "presentation.sdr_gamma";
	const char* white_path = "presentation.paper_white_nits";
	const AeronConfigNode* gamma = AeronConfigFile_GetNode(document, gamma_path);
	const AeronConfigNode* white = AeronConfigFile_GetNode(document, white_path);
	const char* name = AeronConfigNode_String(gamma, "");
	double value = AeronConfigNode_Float(gamma, -1);
	if (!strcmp(name, "auto"))
		out->sdr_gamma = -1.0f;
	else if (!strcmp(name, "srgb"))
		out->sdr_gamma = 0.0f;
	else if (value == 2.2 || !strcmp(name, "2.2"))
		out->sdr_gamma = 2.2f;
	else if (value == 2.4 || !strcmp(name, "2.4"))
		out->sdr_gamma = 2.4f;
	else
		return XwSettings_NodeError(document, gamma_path, "expected auto, srgb, 2.2 or 2.4", error, capacity);
	name = AeronConfigNode_String(white, "");
	value = AeronConfigNode_Float(white, -1);
	if (!strcmp(name, "auto"))
		out->paper_white_nits = 0.0f;
	else if (isfinite(value) && value > 0 && value <= FLT_MAX)
		out->paper_white_nits = (float)value;
	else
		return XwSettings_NodeError(document, white_path, "expected auto or a positive finite luminance",
									error, capacity);
	return 1;
}

int XwSettings_Parse(const AeronConfigFile* document, const XwSceneSettings* scene_defaults, XwSettings* out,
					 char* error, size_t capacity) {
	XwSettings candidate = { .flight_update_rate = XW_FLIGHT_UPDATE_RATE_UNLOCKED };
	AeronConfigError detail;
	if (!document || !scene_defaults || !out)
		return XwSettings_NodeError(document, "", "configuration and scene defaults are required", error,
									capacity);
	static const char* const window_modes[] = { "windowed", "fullscreen" };
	static const char* const flight_rates[] = { "native", "unlocked" };
	int update_rate = candidate.flight_update_rate;
	if (AeronConfigFile_Has(document, "flight.update_rate") &&
		!XwSettings_ReadChoice(document, "flight.update_rate", flight_rates, 2, &update_rate, error,
							   capacity))
		return 0;
	candidate.flight_update_rate = (XwFlightUpdateRate)update_rate;
	int mouse_mode = XW_MOUSE_VIRTUAL_STICK;
	if (AeronConfigFile_Has(document, "input.mouse_mode") &&
		!XwSettings_ReadChoice(document, "input.mouse_mode", mouse_modes, 2, &mouse_mode, error, capacity))
		return 0;
	candidate.mouse_mode = (XwMouseFlightMode)mouse_mode;
	for (size_t i = 0; i < sizeof g_fields / sizeof g_fields[0]; ++i)
		if (!XwSettings_ReadField(document, &g_fields[i], &candidate, error, capacity))
			return 0;
	if (!XwSettings_ReadChoice(document, "video.window_mode", window_modes, 2, &candidate.fullscreen, error,
							   capacity) ||
		!XwSettings_ReadDisplay(document, &candidate.presentation, error, capacity) ||
		!XwPreferences_Parse(document, &candidate.game, error, capacity))
		return 0;
	if (strcmp(candidate.music.arrangement, "windows") && strcmp(candidate.music.arrangement, "gmid") &&
		strcmp(candidate.music.arrangement, "adlb") && strcmp(candidate.music.arrangement, "rlnd"))
		return XwSettings_NodeError(document, "music.arrangement", "expected windows, gmid, adlb or rlnd",
									error, capacity);
	if (strcmp(candidate.frontend_version, "xw94") && strcmp(candidate.frontend_version, "xw98"))
		return XwSettings_NodeError(document, "frontend.version", "expected xw94 or xw98", error, capacity);
	if (strcmp(candidate.inflight_frontend_version, "frontend") &&
		strcmp(candidate.inflight_frontend_version, "xw94") &&
		strcmp(candidate.inflight_frontend_version, "xw98"))
		return XwSettings_NodeError(document, "frontend.inflight_version", "expected frontend, xw94 or xw98",
									error, capacity);
	if (strcmp(candidate.flight_version, "xw93") && strcmp(candidate.flight_version, "xw94") &&
		strcmp(candidate.flight_version, "xw98"))
		return XwSettings_NodeError(document, "flight.version", "expected xw93, xw94 or xw98", error,
									capacity);
	if (strcmp(candidate.music.backend, "auto") && strcmp(candidate.music.backend, "fm4") &&
		strcmp(candidate.music.backend, "fluidsynth") && strcmp(candidate.music.backend, "adlib") &&
		strcmp(candidate.music.backend, "mt32") && strcmp(candidate.music.backend, "sc55"))
		return XwSettings_NodeError(document, "music.backend",
									"expected auto, fm4, fluidsynth, sc55, adlib or mt32", error, capacity);
	char normalized[XW_PATH_CAPACITY];
	if (!XwStorage_Normalize(candidate.ui_font, normalized, sizeof normalized))
		return XwSettings_NodeError(document, "ui.font", "expected a relative font atlas path", error,
									capacity);
	if (!XwKeyboardConfig_Read(document, &candidate.keyboard, error, capacity) ||
		!XwControllerConfig_Parse(document, &candidate.controller, error, capacity) ||
		!XwControllerConfig_ReadProfile(document, "input.gamepad_defaults", AERON_CONTROLLER_KIND_GAMEPAD,
										&candidate.gamepad_defaults, error, capacity))
		return 0;
	if (!XwRenderSettings_ReadChoices(document, &candidate.render, error, capacity))
		return 0;
	candidate.render.scene = *scene_defaults;
	if (!AeronSceneSettings_Overlay(AeronConfigFile_GetNode(document, "render"), &candidate.render.scene.ssao,
									&candidate.render.scene.shadows, &candidate.render.scene.tonemap,
									&detail))
		return XwSettings_FileError(&detail, error, capacity);
	*out = candidate;
	return 1;
}

/* Edit only changed domains in a cloned override tree; opaque keys survive. */
int XwSettings_WriteDocument(AeronConfigFile* document, const XwSettings* value, const XwSettings* previous,
							 const XwSettings* defaults, char* error, size_t capacity) {
	AeronConfigError detail = { 0 };
	if (value->mouse_mode != XW_MOUSE_VIRTUAL_STICK && value->mouse_mode != XW_MOUSE_CLASSIC)
		return XwSettings_NodeError(document, "input.mouse_mode", "expected virtual_stick or classic", error,
									capacity);
	if (value->mouse_mode != previous->mouse_mode &&
		!(value->mouse_mode == defaults->mouse_mode
			  ? (!AeronConfigFile_Has(document, "input.mouse_mode") ||
				 AeronConfigFile_Remove(document, "input.mouse_mode", &detail))
			  : AeronConfigFile_SetString(document, "input.mouse_mode", mouse_modes[value->mouse_mode],
										  &detail)))
		return XwSettings_FileError(&detail, error, capacity);
	if (value->flight_update_rate != XW_FLIGHT_UPDATE_RATE_NATIVE &&
		value->flight_update_rate != XW_FLIGHT_UPDATE_RATE_UNLOCKED)
		return XwSettings_NodeError(document, "flight.update_rate", "expected native or unlocked", error,
									capacity);
	if (value->flight_update_rate != previous->flight_update_rate &&
		!(value->flight_update_rate == defaults->flight_update_rate
			  ? (!AeronConfigFile_Has(document, "flight.update_rate") ||
				 AeronConfigFile_Remove(document, "flight.update_rate", &detail))
			  : AeronConfigFile_SetString(
					document, "flight.update_rate",
					value->flight_update_rate == XW_FLIGHT_UPDATE_RATE_NATIVE ? "native" : "unlocked",
					&detail)))
		return XwSettings_FileError(&detail, error, capacity);
	for (size_t i = 0; i < sizeof g_fields / sizeof g_fields[0]; ++i) {
		const XwSettingField* field = &g_fields[i];
		const char* v = (const char*)value + field->offset;
		const char* p = (const char*)previous + field->offset;
		const char* d = (const char*)defaults + field->offset;
		size_t size = field->type == XW_SETTING_STRING
						  ? strlen(v) + 1
						  : (field->type == XW_SETTING_FLOAT ? sizeof(float) : sizeof(int));
		if (!memcmp(v, p, size))
			continue;
		int ok;
		if (!memcmp(v, d, size))
			ok = !AeronConfigFile_Has(document, field->path) ||
				 AeronConfigFile_Remove(document, field->path, &detail);
		else if (field->type == XW_SETTING_STRING)
			ok = AeronConfigFile_SetString(document, field->path, v, &detail);
		else if (field->type == XW_SETTING_FLOAT) {
			float number;
			memcpy(&number, v, sizeof number);
			ok = AeronConfigFile_SetFloat(document, field->path, number, &detail);
		} else {
			int integer;
			memcpy(&integer, v, sizeof integer);
			ok = field->type == XW_SETTING_BOOL
					 ? AeronConfigFile_SetBool(document, field->path, integer, &detail)
					 : AeronConfigFile_SetInt(document, field->path, integer, &detail);
		}
		if (!ok)
			return XwSettings_FileError(&detail, error, capacity);
	}
	if (value->fullscreen != previous->fullscreen &&
		!(value->fullscreen == defaults->fullscreen
			  ? (!AeronConfigFile_Has(document, "video.window_mode") ||
				 AeronConfigFile_Remove(document, "video.window_mode", &detail))
			  : AeronConfigFile_SetString(document, "video.window_mode",
										  value->fullscreen ? "fullscreen" : "windowed", &detail)))
		return XwSettings_FileError(&detail, error, capacity);
	const XwPresentationSettings* v = &value->presentation;
	const XwPresentationSettings* p = &previous->presentation;
	const XwPresentationSettings* d = &defaults->presentation;
	if (v->sdr_gamma != p->sdr_gamma) {
		int ok = v->sdr_gamma == d->sdr_gamma
					 ? (!AeronConfigFile_Has(document, "presentation.sdr_gamma") ||
						AeronConfigFile_Remove(document, "presentation.sdr_gamma", &detail))
				 : v->sdr_gamma <= 0 ? AeronConfigFile_SetString(document, "presentation.sdr_gamma",
																 v->sdr_gamma < 0 ? "auto" : "srgb", &detail)
									 : AeronConfigFile_SetFloat(document, "presentation.sdr_gamma",
																v->sdr_gamma == 2.4f ? 2.4 : 2.2, &detail);
		if (!ok)
			return XwSettings_FileError(&detail, error, capacity);
	}
	if (v->paper_white_nits != p->paper_white_nits) {
		int ok = v->paper_white_nits == d->paper_white_nits
					 ? (!AeronConfigFile_Has(document, "presentation.paper_white_nits") ||
						AeronConfigFile_Remove(document, "presentation.paper_white_nits", &detail))
				 : v->paper_white_nits == 0
					 ? AeronConfigFile_SetString(document, "presentation.paper_white_nits", "auto", &detail)
					 : AeronConfigFile_SetFloat(document, "presentation.paper_white_nits",
												v->paper_white_nits, &detail);
		if (!ok)
			return XwSettings_FileError(&detail, error, capacity);
	}
	if (!XwControllerOptions_Equals(&value->controller, &previous->controller) &&
		!XwControllerConfig_Write(document, &value->controller, &detail))
		return XwSettings_FileError(&detail, error, capacity);
	if (!XwKeyboardMapping_Equal(&value->keyboard, &previous->keyboard)) {
		int ok = XwKeyboardMapping_Equal(&value->keyboard, &defaults->keyboard)
					 ? (!AeronConfigFile_Has(document, "input.keyboard") ||
						AeronConfigFile_Remove(document, "input.keyboard", &detail))
					 : XwKeyboardConfig_Write(document, &value->keyboard, &detail);
		if (!ok)
			return XwSettings_FileError(&detail, error, capacity);
	}
	if (!XwRenderSettings_WriteChoices(document, &value->render, &previous->render, &defaults->render, error,
									   capacity))
		return 0;
	return !memcmp(&value->game, &previous->game, sizeof value->game) ||
		   XwPreferences_WriteDocument(document, &value->game, &defaults->game, error, capacity);
}
