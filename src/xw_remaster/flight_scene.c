/* Scene submission adapted from OpenXvT flight_scene.c, using X-Wing snapshot semantics. */
#include "xw_remaster/flight_scene.h"
#include "xw_remaster/component_animation.h"
#include "xw_remaster/config.h"
#include "xw_remaster/dos_scene.h"
#include "xw_remaster/effects.h"
#include "xw_remaster/engine_glows.h"
#include "xw_remaster/flight_pipeline.h"
#include "xw_remaster/hud_renderer.h"
#include "xw_remaster/lighting.h"
#include "xw_remaster/sky.h"
#include "xw_remaster/special_world.h"
#include <aeron/aeron.h>
#include <aeron/asset/opt_model.h>
#include <stdlib.h>
#include <string.h>

typedef struct SceneFailure {
	uint64_t mission, world, config;
	XwRenderAssetId assets;
	int width, height;
	bool valid;
} SceneFailure;

static AeronScene3D* scene;
static AeronSceneMeshTable* tables;
static int width, height, samples;
static XwFlightOutput output;
static uint64_t output_revision;
static SceneFailure failed;

void XwFlightScene_Invalidate(void) {
	memset(&output, 0, sizeof output);
	XwFlightPipeline_Invalidate();
	XwDosScene_Invalidate();
}

static bool Ensure(int w, int h, int msaa) {
	if (!tables)
		tables = calloc(XW_SNAP_OBJECTS * 2, sizeof *tables);
	if (!tables)
		return false;
	if (scene && width == w && height == h && samples == msaa)
		return true;
	AeronScene3D* next =
		AeronScene_Create(&(AeronScene3DDesc) { .rt_width = w,
												.rt_height = h,
												.color_format = AERON_TEXTURE_FORMAT_RGBA16_FLOAT,
												.with_normal_rt = 1,
												.sample_count = (AeronSampleCount)msaa,
												.view_space_to_meters = AERON_OPT_METERS_PER_UNIT });
	if (!next)
		return false;
	AeronScene_SetClearColor(next, (const float[4]) { 0, 0, 0, 1 });
	AeronScene_Destroy(scene);
	scene = next;
	width = w;
	height = h;
	samples = msaa;
	XwFlightScene_Invalidate();
	return true;
}

bool XwFlightScene_PrepareResources(uint8_t version, int w, int h) {
	if (version != 98)
		return XwDosScene_PrepareResources(w, h);
	return Ensure(w, h, XwRemasterConfig_Effective()->msaa_samples) && XwEngineGlows_Prepare() &&
		   XwFlightPipeline_PrepareResources(scene, w, h);
}

static const XwSnapCraft* Craft(const XwSnapObject* object, const XwSnapCraft* crafts, unsigned count) {
	return object->craft_index < count ? &crafts[object->craft_index] : NULL;
}

static bool Previous(const XwPreparedFlight* frame, const XwSnapObject* object, const XwPreparedObject* pose,
					 const XwMeshAsset* asset, const XwShipSelection* selection,
					 AeronSceneMeshInstance* instance, AeronSceneMeshTable* table) {
	if (instance->zero_velocity || pose->previous_index < 0 ||
		(unsigned)pose->previous_index >= frame->previous_object_count)
		return true;
	const XwRenderAssetSetView* old_set = XwRenderAssets_Set(frame->previous_assets);
	const XwSnapObject* old = &frame->previous_objects[pose->previous_index];
	XwShipSelection previous;
	if (!old_set || XwShip_Select(&old_set->bindings, old, &previous) != 1 ||
		previous.asset_id != selection->asset_id || previous.component != selection->component ||
		previous.variant != selection->variant) {
		instance->zero_velocity = 1;
		return true;
	}
	const XwSnapCraft* craft = Craft(old, frame->previous_crafts, frame->previous_craft_count);
	if (!XwShip_BuildMeshTable(asset, old, craft, previous.component, XwComponentAnimation_Angles(old, true),
							   table))
		return false;
	/* A newly exposed component must not inherit the absent component's velocity. */
	for (unsigned i = 0; i < asset->component_count; ++i)
		if (instance->mesh_table->visibility_packed[i >> 2][i & 3] != table->visibility_packed[i >> 2][i & 3])
			instance->zero_velocity = 1;
	instance->prev_mesh_table = table;
	if (XwShip_Projectile(object)) {
		const XwRenderSettings* settings = XwRemasterConfig_Effective();
		const int32_t* camera =
			settings->motion_blur.camera_blur || settings->temporal_mode != AERON_TEMPORAL_OFF
				? frame->previous_camera.world_pos
				: frame->view.origin_world;
		XwShip_ProjectileMatrix(old, camera, frame->view.origin_world, instance->prev_transform);
	}
	return true;
}

static bool Submit(const XwRenderSnapshot* s, const XwPreparedFlight* frame, bool reset) {
	const XwRenderAssetSetView* set = XwRenderAssets_Set(s->flight_assets);
	if (!set || set->failed || set->bindings.flight_version != 98)
		return false;
	for (unsigned i = 0; i < s->object_count; ++i) {
		const XwSnapObject* object = &s->objects[i];
		if (!XwShip_Eligible(s, object))
			continue;
		XwShipSelection selection;
		int selected = XwShip_Select(&set->bindings, object, &selection);
		if (selected < 0)
			return false;
		if (!selected)
			continue;
		const XwMeshAsset* asset = XwRemasterAssets_Mesh(selection.asset_id);
		if (!asset)
			return false;
		const XwSnapCraft* craft = Craft(object, s->crafts, s->craft_count);
		const XwPreparedObject* pose = &frame->objects[i];
		AeronSceneMeshTable* table = &tables[i * 2];
		if (!XwShip_BuildMeshTable(asset, object, craft, selection.component,
								   XwComponentAnimation_Angles(object, false), table))
			return false;
		AeronSceneMeshInstance instance = { .mesh = asset->mesh,
											.variant = selection.variant,
											.mesh_table = table,
											.prev_mesh_table = table,
											.zero_velocity =
												reset || pose->zero_velocity || !frame->regenerate_motion,
											.cull_mode = AERON_CULL_BACK };
		memcpy(instance.transform, pose->transform, sizeof instance.transform);
		memcpy(instance.prev_transform, pose->previous_transform, sizeof instance.prev_transform);
		if (XwShip_Projectile(object)) {
			XwShip_ProjectileMatrix(object, s->camera.world_pos, s->camera.world_pos, instance.transform);
			memcpy(instance.prev_transform, instance.transform, sizeof instance.transform);
			instance.base_color_emissive_strength =
				XwRemasterConfig_Effective()->models.opt_projectile_emissive_strength;
			instance.no_local_lights = 1;
			instance.shadow_flags =
				AERON_SCENE_INSTANCE_NO_CAST_SHADOW | AERON_SCENE_INSTANCE_NO_RECEIVE_SHADOW;
			instance.cull_mode = AERON_CULL_NONE;
		}
		if (!Previous(frame, object, pose, asset, &selection, &instance, table + 1))
			return false;
		bool hidden = XwRenderMath_SameObject(object->id, s->camera.focus) && !s->camera.external &&
					  !s->camera.replay_mode;
		XwEngineGlows_Lights(scene, asset->mesh, craft, table, instance.transform);
		float radius = XwShip_Radius(asset, table, instance.transform);
		if (radius <= 0)
			continue;
		/* A glow can enter the view while its owning hull is outside the frustum. */
		if (!hidden && !XwEngineGlows_Submit(scene, s, craft, asset, table, instance.transform, &frame->view))
			return false;
		if (!hidden && XwShip_Visible(&frame->view, instance.transform, radius)) {
			AeronScene_AddMeshInstance(scene, &instance);
		} else
			AeronScene_AddShadowCaster(scene, &instance);
	}
	return true;
}

static bool Fail(SceneFailure key) {
	failed = key;
	XwSpecialWorld_Shutdown();
	XwFlightScene_Invalidate();
	XwSky_Shutdown();
	Aeron_RequestFatalRendererError("1998 flight scene preparation");
	return false;
}

static void WorldMarkers(AeronCommandBuffer* cmd, AeronRenderPass* pass, int w, int h, void* user) {
	(void)w;
	(void)h;
	XwHudRenderer_DrawWorldMarkers(cmd, pass, AeronScene_ColorRt(user));
}

bool XwFlightScene_Frame(const XwRenderSnapshot* s, const XwPreparedFlight* frame, bool direct) {
	if (s && s->flight_version != 98) {
		memset(&output, 0, sizeof output);
		XwFlightPipeline_Invalidate();
		return XwDosScene_Frame(s, frame);
	}
	XwDosScene_Invalidate();
	if (!s || s->flight_version != 98 || !s->world_valid || !s->hud_valid ||
		s->owner != XW_SNAP_OWNER_FLIGHT || !frame || !frame->valid || !frame->drawable ||
		frame->assets != s->flight_assets || s->object_count > XW_SNAP_OBJECTS ||
		s->craft_count > XW_SNAP_CRAFTS || frame->object_count != s->object_count ||
		frame->host_serial != s->host_serial) {
		XwFlightScene_Invalidate();
		return false;
	}
	int w = frame->view.camera.viewport.width, h = frame->view.camera.viewport.height;
	SceneFailure key = { .mission = s->key.mission_generation,
						 .world = s->key.world_generation,
						 .config = XwRemasterConfig_Generation(),
						 .assets = s->flight_assets,
						 .width = w,
						 .height = h,
						 .valid = true };
	if (failed.valid && failed.mission == key.mission && failed.world == key.world &&
		failed.config == key.config && failed.assets == key.assets && failed.width == w && failed.height == h)
		return false;
	int msaa = XwRemasterConfig_Effective()->msaa_samples;
	bool recreate = !scene || width != w || height != h || samples != msaa;
	int physical_w = 0, physical_h = 0;
	Aeron_GetPresentationPixelSize(&physical_w, &physical_h);
	bool wanted = XwFlightPipeline_SetDirect(direct && !s->paused && s->focused && w == physical_w &&
												 h == physical_h && !recreate && !frame->reset_history,
											 w, h);
	if (Aeron_FatalErrorRequested())
		return false;
	bool reset = frame->reset_history || recreate || !output.key.view_serial;
	if (!frame->render_needed && !reset && wanted == output.direct)
		return true;
	if (!Ensure(w, h, msaa) || !XwEngineGlows_Prepare() || !XwFlightPipeline_Begin(scene, s, frame, reset))
		return Fail(key);
	AeronCommandBuffer* cmd = Aeron_AcquireCommandBuffer();
	if (!cmd)
		return Fail(key);
	if (!XwSky_Prepare(cmd, scene, s, &frame->view) || !XwLighting_Begin(scene, s)) {
		Aeron_CancelCommandBuffer(cmd);
		return Fail(key);
	}
	XwLighting_Environment(scene, &s->appearance, frame->view.camera.pos);
	if (!Submit(s, frame, reset) || !XwEffects_Submit(scene, s, frame) ||
		!XwSpecialWorld_Prepare(cmd, scene, s, frame, reset || !frame->regenerate_motion) ||
		!XwHudRenderer_PrepareWorldMarkers(cmd, s, &frame->view, scene)) {
		Aeron_CancelCommandBuffer(cmd);
		return Fail(key);
	}
	AeronScene_SetPassHook(scene, AERON_SCENE_HOOK_AFTER_TRANSPARENT, WorldMarkers, scene);
	if (!XwFlightPipeline_Finish(cmd, scene, frame)) {
		Aeron_CancelCommandBuffer(cmd);
		return Fail(key);
	}
	bool submitted = Aeron_SubmitCommandBuffer(cmd);
	XwFlightPipeline_Commit(submitted);
	if (!submitted)
		return Fail(key);
	output = (XwFlightOutput) { .texture = XwFlightPipeline_Output(),
								.key = s->key,
								.width = w,
								.height = h,
								.direct = wanted,
								.revision = ++output_revision,
								.color_space = AERON_COLOR_SPACE_LINEAR_SRGB };
	failed.valid = false;
	return output.texture || output.direct;
}

const XwFlightOutput* XwFlightScene_Output(void) {
	return output.texture || output.direct ? &output : XwDosScene_Output();
}

void XwFlightScene_Shutdown(void) {
	XwSpecialWorld_Shutdown();
	XwDosScene_Shutdown();
	XwSky_Shutdown();
	XwFlightPipeline_Shutdown();
	AeronScene_Destroy(scene);
	scene = NULL;
	XwEngineGlows_Shutdown();
	free(tables);
	tables = NULL;
	width = height = samples = 0;
	memset(&failed, 0, sizeof failed);
	XwFlightScene_Invalidate();
}
