#include "xw_remaster/gate_lights.h"
#include "xw_remaster/config.h"
#include <aeron/asset/opt_model.h>
#include <aeron/config_file.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

enum { GATE_LIGHT_MODELS = 8, GATE_LIGHT_COMPONENTS = 3 };

typedef struct GateLamp {
	float position[3]; /* Model metres, ready for the prepared object transform. */
	unsigned variants;
} GateLamp;

struct XwGateLightModel {
	char name[32];
	float color[3]; /* Linear RGB. */
	GateLamp lamps[GATE_LIGHT_COMPONENTS];
};

static XwGateLightModel models[GATE_LIGHT_MODELS];
static unsigned model_count;
static float intensity, radius; /* Radius is converted from metres to scene units at load time. */
static bool enabled;

static bool Number(const AeronConfigNode* node, float* out) {
	double value = AeronConfigNode_Float(node, NAN);
	if (!isfinite(value) || value < -FLT_MAX || value > FLT_MAX)
		return false;
	*out = (float)value;
	return true;
}

static bool Vector(const AeronConfigNode* node, float out[3]) {
	if (AeronConfigNode_Type(node) != AERON_CONFIG_SEQUENCE || AeronConfigNode_SequenceCount(node) != 3)
		return false;
	for (unsigned i = 0; i < 3; ++i)
		if (!Number(AeronConfigNode_SequenceGet(node, i), &out[i]))
			return false;
	return true;
}

static bool Model(const AeronConfigNode* node, XwGateLightModel* out) {
	const char* name = AeronConfigNode_String(AeronConfigNode_MapGet(node, "model"), NULL);
	const AeronConfigNode* lamps = AeronConfigNode_MapGet(node, "lamps");
	if (!name || !name[0] || strlen(name) >= sizeof out->name || strpbrk(name, "/\\") ||
		!Vector(AeronConfigNode_MapGet(node, "color"), out->color) ||
		AeronConfigNode_Type(lamps) != AERON_CONFIG_SEQUENCE ||
		AeronConfigNode_SequenceCount(lamps) > GATE_LIGHT_COMPONENTS)
		return false;
	for (unsigned i = 0; name[i]; ++i)
		out->name[i] = name[i] >= 'a' && name[i] <= 'z' ? name[i] - ('a' - 'A') : name[i];
	for (unsigned c = 0; c < 3; ++c) {
		float v = out->color[c];
		if (v < 0 || v > 1)
			return false;
		out->color[c] = v <= .04045f ? v / 12.92f : powf((v + .055f) / 1.055f, 2.4f);
	}
	unsigned components = 0;
	for (size_t i = 0; i < AeronConfigNode_SequenceCount(lamps); ++i) {
		const AeronConfigNode* lamp = AeronConfigNode_SequenceGet(lamps, i);
		int64_t component = AeronConfigNode_Int(AeronConfigNode_MapGet(lamp, "component"), -1);
		const AeronConfigNode* variants = AeronConfigNode_MapGet(lamp, "variants");
		if (component < 1 || component > GATE_LIGHT_COMPONENTS || (components & (1u << component)) ||
			AeronConfigNode_Type(variants) != AERON_CONFIG_SEQUENCE ||
			AeronConfigNode_SequenceCount(variants) > 4)
			return false;
		components |= 1u << component;
		GateLamp* destination = &out->lamps[component - 1];
		if (!Vector(AeronConfigNode_MapGet(lamp, "position"), destination->position))
			return false;
		for (size_t v = 0; v < AeronConfigNode_SequenceCount(variants); ++v) {
			int64_t variant = AeronConfigNode_Int(AeronConfigNode_SequenceGet(variants, v), -1);
			if (variant < 0 || variant > 3 || (destination->variants & (1u << variant)))
				return false;
			destination->variants |= 1u << variant;
		}
	}
	return true;
}

bool XwGateLights_Init(AeronVfs* vfs, char* error, size_t capacity) {
	XwGateLights_Shutdown();
	AeronConfigFile* file = NULL;
	AeronConfigError diagnostic = { 0 };
	if (!AeronConfigFile_LoadYamlEx(vfs, AERON_VFS_ROOT_RESOURCE, "gate_lights.yaml", &file, &diagnostic)) {
		snprintf(error, capacity, "gate_lights.yaml:%d: %.160s", diagnostic.line, diagnostic.message);
		return false;
	}
	const AeronConfigNode* root = AeronConfigFile_Root(file);
	const AeronConfigNode* entries = AeronConfigNode_MapGet(root, "models");
	const AeronConfigNode* enable = AeronConfigNode_MapGet(root, "enabled");
	bool valid = AeronConfigNode_Int(AeronConfigNode_MapGet(root, "version"), 0) == 1 &&
				 AeronConfigNode_Type(enable) == AERON_CONFIG_BOOL &&
				 Number(AeronConfigNode_MapGet(root, "intensity"), &intensity) && intensity >= 0 &&
				 Number(AeronConfigNode_MapGet(root, "radius"), &radius) && radius > 0 &&
				 radius <= FLT_MAX / AERON_OPT_UNITS_PER_METER &&
				 AeronConfigNode_Type(entries) == AERON_CONFIG_SEQUENCE &&
				 AeronConfigNode_SequenceCount(entries) <= GATE_LIGHT_MODELS;
	int line = AeronConfigNode_Line(root);
	for (size_t i = 0; valid && i < AeronConfigNode_SequenceCount(entries); ++i) {
		const AeronConfigNode* node = AeronConfigNode_SequenceGet(entries, i);
		line = AeronConfigNode_Line(node);
		valid = Model(node, &models[i]);
		for (size_t previous = 0; valid && previous < i; ++previous)
			valid = strcmp(models[previous].name, models[i].name) != 0;
	}
	if (valid) {
		radius *= AERON_OPT_UNITS_PER_METER;
		model_count = (unsigned)AeronConfigNode_SequenceCount(entries);
		enabled = AeronConfigNode_Bool(enable, 0) != 0;
	} else {
		snprintf(error, capacity,
				 "gate_lights.yaml:%d: invalid settings or duplicate model/component/variant", line);
		XwGateLights_Shutdown();
	}
	AeronConfigFile_Destroy(file);
	return valid;
}

const XwGateLightModel* XwGateLights_Model(const char* path) {
	const char* name = path;
	for (const char* p = path; *p; ++p)
		if (*p == '/' || *p == '\\')
			name = p + 1;
	char normalized[sizeof models[0].name] = { 0 };
	if (strlen(name) >= sizeof normalized)
		return NULL;
	for (unsigned i = 0; name[i]; ++i)
		normalized[i] = name[i] >= 'a' && name[i] <= 'z' ? name[i] - ('a' - 'A') : name[i];
	for (unsigned i = 0; i < model_count; ++i)
		if (strcmp(models[i].name, normalized) == 0)
			return &models[i];
	return NULL;
}

void XwGateLights_Submit(AeronScene3D* scene, const XwGateLightModel* model, unsigned component,
						 unsigned variant, const float transform[16]) {
	const XwPointLightSettings* settings = &XwRemasterConfig_Effective()->point_lights;
	if (!enabled || !model || !settings->enabled || intensity <= 0 || settings->scale <= 0 || component < 1 ||
		component > GATE_LIGHT_COMPONENTS || variant > 3)
		return;
	const GateLamp* lamp = &model->lamps[component - 1];
	if (!(lamp->variants & (1u << variant)))
		return;
	/* An explicit radius keeps brightness tuning independent of the light's reach. */
	AeronSceneLight light = { .radius = radius * settings->range_scale };
	if (!(light.radius > 0) || !isfinite(light.radius))
		return;
	for (unsigned c = 0; c < 3; ++c) {
		light.pos[c] = transform[c * 4] * lamp->position[0] + transform[c * 4 + 1] * lamp->position[1] +
					   transform[c * 4 + 2] * lamp->position[2] + transform[c * 4 + 3];
		light.color[c] = model->color[c] * intensity * settings->scale;
		if (!isfinite(light.pos[c]) || !isfinite(light.color[c]))
			return;
	}
	AeronScene_AddLight(scene, &light);
}

void XwGateLights_Shutdown(void) {
	memset(models, 0, sizeof models);
	model_count = 0;
	intensity = radius = 0;
	enabled = false;
}
