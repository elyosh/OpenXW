/* Renderer settings adapted from OpenXvT f643323. Host output stays in XwSettings. */
#ifndef XW_RUNTIME_VIDEO_CONFIG_H
#define XW_RUNTIME_VIDEO_CONFIG_H
#include "xw_runtime/storage/storage.h"
#include <aeron/scene/settings.h>
#include <aeron/temporal.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct XwModelSettings {
	float smooth_angle_degrees;
	float opt_emissive_strength, opt_projectile_emissive_strength, engine_emissive_strength;
} XwModelSettings;

typedef struct XwPointLightSettings {
	int enabled, clustered, cluster_depth_slices, cluster_debug;
	float scale, range_scale, min_distance, spec_weight, diffuse_wrap, contrib_cap;
} XwPointLightSettings;

typedef struct XwLightingSettings {
	float intensity, spec_mul, wrap;
	int spec_geom_adapt;
	float ambient[3];
} XwLightingSettings;

enum { XW_SKY_STARS, XW_SKY_CUBE };

typedef struct XwSkySettings {
	int enabled, mode;
	char path[XW_PATH_CAPACITY];
	float exposure, star_brightness;
} XwSkySettings;

typedef struct XwHyperspaceSettings {
	float travel_speed, rotation_speed, noise_scale, brightness, highlight_strength;
	float focal_length, twist, cap_radius, cap_falloff;
	float mesh_ambient_strength, mesh_environment_roughness, mesh_key_strength;
	float dark_color[3], body_color[3], highlight_color[3], cap_color[3];
} XwHyperspaceSettings;

typedef struct XwMotionBlurSettings {
	int quality, camera_blur, pause_keep_blur, velocity_viz, fsr_direct_motion;
	float shutter;
} XwMotionBlurSettings;

typedef struct XwSceneSettings {
	AeronSceneSsaoSettings ssao;
	AeronSceneShadowSettings shadows;
	AeronSceneTonemapSettings tonemap;
} XwSceneSettings;

typedef struct XwRenderSettings {
	int dos_msaa_samples;
	int cockpit_undither;
	XwSceneSettings scene;
	int msaa_samples;
	AeronTemporalMode temporal_mode;
	float temporal_sharpness;
	XwMotionBlurSettings motion_blur;
	int anisotropic;
	float max_anisotropy;
	XwModelSettings models;
	XwPointLightSettings point_lights;
	XwLightingSettings lighting;
	XwSkySettings sky;
	XwHyperspaceSettings hyperspace;
	float bloom_intensity, explosion_emissive_strength;
} XwRenderSettings;

int XwRenderSettings_ReadChoices(const AeronConfigFile* document, XwRenderSettings* out, char* error,
								 size_t capacity);
int XwRenderSettings_WriteChoices(AeronConfigFile* document, const XwRenderSettings* value,
								  const XwRenderSettings* previous, const XwRenderSettings* defaults,
								  char* error, size_t capacity);
#ifdef __cplusplus
}
#endif
#endif
