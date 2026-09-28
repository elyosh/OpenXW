/* Staged cache replacement follows OpenXvT ship_assets.c. One batch per host frame. */
#include "xw_remaster/assets.h"
#include "xw_remaster/config.h"
#include "xw_remaster/dos_images.h"
#include "xw_remaster/image_assets.h"
#include "xw_remaster/opt_mesh.h"
#include <aeron/aeron.h>
#include <stdio.h>
#include <string.h>

typedef struct AssetEntry {
	XwRenderAssetId id;
	XwMeshAsset asset;
	XwDosMesh dos;
	AeronRuntimeAtlas image;
	bool added;
} AssetEntry;

static AssetEntry entries[XW_RENDER_SOURCE_CAPACITY], pending[XW_RENDER_SOURCE_CAPACITY];
static unsigned count, pending_count;
static XwRenderAssetId committed_set, pending_set, failed_set, failed_source, preparing_source;
static XwModelSettings policy, pending_policy;
static uint64_t failed_config;
static bool policy_loaded;

static bool SamePolicy(const XwModelSettings* a, const XwModelSettings* b) {
	return a->smooth_angle_degrees == b->smooth_angle_degrees &&
		   a->opt_emissive_strength == b->opt_emissive_strength;
}

static void Destroy(AssetEntry* entry) {
	AeronScene_MeshDestroy(entry->asset.mesh);
	XwDosMesh_Destroy(&entry->dos);
	Aeron_RuntimeAtlasRelease(&entry->image);
	memset(entry, 0, sizeof *entry);
}

static bool Supported(const XwRenderSource* source) {
	if (source && source->flight_version != 98)
		return source->kind == XW_SOURCE_DOS_MODEL || source->kind == XW_SOURCE_BITMAP;
	return source && source->flight_version == 98 &&
		   (source->kind == XW_SOURCE_OPT || source->kind == XW_SOURCE_BITMAP ||
			source->kind == XW_SOURCE_TEXTURE);
}

static void Abort(void) {
	for (unsigned i = 0; i < pending_count; ++i)
		if (pending[i].added)
			Destroy(&pending[i]);
	memset(pending, 0, sizeof pending);
	pending_count = 0;
	XwRenderAssets_Release(pending_set);
	pending_set = 0;
}

static XwAssetPreparation Fail(XwRenderAssetId id, const char* message) {
	const XwRenderSource* source = XwRenderAssets_Source(preparing_source);
	char diagnostic[1536];
	snprintf(diagnostic, sizeof diagnostic,
			 "Unable to prepare modern rendering resource %s (set %llu):\n\n%s",
			 source ? source->path : "<resource set>", (unsigned long long)id, message);
	Abort();
	failed_set = id;
	failed_source = preparing_source;
	failed_config = XwRemasterConfig_Generation();
	Aeron_RequestFatalError("Renderer Error", diagnostic);
	return XW_ASSETS_FAILED;
}

static void Commit(void) {
	for (unsigned i = 0; i < count; ++i) {
		unsigned j = 0;
		for (; j < pending_count; ++j)
			if (entries[i].id == pending[j].id && !pending[j].added)
				break;
		if (j == pending_count)
			Destroy(&entries[i]);
	}
	memcpy(entries, pending, sizeof entries);
	count = pending_count;
	for (unsigned i = 0; i < count; ++i)
		entries[i].added = false;
	memset(pending, 0, sizeof pending);
	pending_count = 0;
	XwRenderAssets_Release(committed_set);
	committed_set = pending_set;
	pending_set = 0;
	policy = pending_policy;
}

static bool Contains(XwRenderAssetId id) {
	for (unsigned i = 0; i < pending_count; ++i)
		if (pending[i].id == id)
			return true;
	return false;
}

static bool Reuse(XwRenderAssetId id) {
	const XwRenderSource* source = XwRenderAssets_Source(id);
	if (source && source->kind == XW_SOURCE_OPT && !SamePolicy(&policy, &pending_policy))
		return false;
	for (unsigned i = 0; i < count; ++i)
		if (entries[i].id == id) {
			pending[pending_count++] = entries[i];
			return true;
		}
	return false;
}

static bool SetContains(const XwRenderAssetSetView* set, XwRenderAssetId id) {
	for (unsigned i = 0; i < set->source_count; ++i)
		if (set->sources[i] == id)
			return true;
	return false;
}

static void Retarget(XwRenderAssetId id, const XwRenderAssetSetView* set, const XwModelSettings* requested) {
	if (!SamePolicy(&pending_policy, requested))
		Abort();
	/* HUD or animation revisions must not restart unchanged mesh preparation. */
	for (unsigned i = 0; i < pending_count;) {
		if (SetContains(set, pending[i].id)) {
			++i;
			continue;
		}
		if (pending[i].added)
			Destroy(&pending[i]);
		pending[i] = pending[--pending_count];
		memset(&pending[pending_count], 0, sizeof pending[0]);
	}
	XwRenderAssets_Retain(id);
	XwRenderAssets_Release(pending_set);
	pending_set = id;
	pending_policy = *requested;
}

XwAssetPreparation XwRemasterAssets_Frame(const XwRenderSnapshot* snapshot) {
	XwRenderAssetId id = snapshot ? snapshot->flight_assets : 0;
	if (!id) {
		XwRemasterAssets_Shutdown();
		return XW_ASSETS_IDLE;
	}
	const XwRenderAssetSetView* set = XwRenderAssets_Set(id);
	if (failed_config == XwRemasterConfig_Generation() &&
		(failed_set == id || (set && failed_source && SetContains(set, failed_source))))
		return XW_ASSETS_FAILED;
	preparing_source = 0;
	if (!set || set->failed)
		return Fail(id, "source capture failed");
	const XwModelSettings* requested = &XwRemasterConfig_Effective()->models;
	if (id == committed_set && SamePolicy(&policy, requested))
		return XW_ASSETS_READY;
	if (pending_set != id || !SamePolicy(&pending_policy, requested)) {
		Retarget(id, set, requested);
	}
	AeronCommandBuffer* cmd = NULL;
	uint64_t started = Aeron_NowUs();
	unsigned uploaded = 0;
	AeronCommandBufferUploadUsage usage = { 0 };
	bool complete = true;
	for (unsigned i = 0; i < set->source_count; ++i) {
		const XwRenderSource* source = XwRenderAssets_Source(set->sources[i]);
		if (!source) {
			if (cmd)
				Aeron_CancelCommandBuffer(cmd);
			return Fail(id, "retained source is missing");
		}
		if (!Supported(source) || Contains(source->id))
			continue;
		if (pending_count == XW_RENDER_SOURCE_CAPACITY) {
			if (cmd)
				Aeron_CancelCommandBuffer(cmd);
			return Fail(id, "asset cache capacity exceeded");
		}
		if (Reuse(source->id))
			continue;
		preparing_source = source->id;
		if (!cmd)
			cmd = Aeron_AcquireCommandBuffer();
		if (!cmd)
			return Fail(id, "asset upload command buffer unavailable");
		char error[256] = { 0 };
		AssetEntry entry = { .id = source->id, .added = true };
		if (source->kind == XW_SOURCE_OPT) {
			if (!policy_loaded) {
				XwRemasterOptMesh_Shutdown();
				if (!XwRemasterOptMesh_Init(Aeron_GetVfs(), error, sizeof error) ||
					!XwGateLights_Init(Aeron_GetVfs(), error, sizeof error)) {
					Aeron_CancelCommandBuffer(cmd);
					return Fail(id, error);
				}
				policy_loaded = true;
			}
			AeronFlightModel model = { 0 };
			if (!XwRemasterOptMesh_Build(source, &pending_policy, &model, error, sizeof error)) {
				Aeron_CancelCommandBuffer(cmd);
				return Fail(id, error);
			}
			if (model.component_count > XW_SNAP_COMPONENTS) {
				Aeron_FlightModelFree(&model);
				Aeron_CancelCommandBuffer(cmd);
				return Fail(id, "OPT component count exceeds the snapshot contract");
			}
			AeronSceneMeshCreateStatus status;
			entry.asset.mesh = AeronScene_MeshCreate(cmd, &model, source->path, &status);
			entry.asset.component_count = model.component_count;
			entry.asset.bridge_component = model.bridge_component;
			entry.asset.gate_lights = XwGateLights_Model(source->path);
			Aeron_FlightModelFree(&model);
			if (!entry.asset.mesh) {
				Aeron_CancelCommandBuffer(cmd);
				return Fail(id, "OPT GPU creation failed");
			}
		} else if (source->kind == XW_SOURCE_DOS_MODEL && source->size == sizeof(XwRenderDosModel) &&
				   !((const XwRenderDosModel*)source->data)->image_count) {
			if (!XwDosMesh_Build(cmd, source, &entry.dos, error, sizeof error)) {
				Aeron_CancelCommandBuffer(cmd);
				Destroy(&entry);
				return Fail(id, error[0] ? error : "DOS mesh upload unavailable");
			}
		} else {
			if (!(source->flight_version == 98
					  ? XwImageAssets_Build(cmd, source, &entry.image, error, sizeof error)
					  : XwDosImages_Build(cmd, source, &entry.image, error, sizeof error))) {
				Aeron_CancelCommandBuffer(cmd);
				Destroy(&entry);
				return Fail(id, error[0] ? error : "image upload command buffer unavailable");
			}
		}
		pending[pending_count++] = entry;
		++uploaded;
		if (!Aeron_CommandBufferGetUploadUsage(cmd, &usage)) {
			Aeron_CancelCommandBuffer(cmd);
			return Fail(id, "upload usage query failed");
		}
		/* One source is atomic, as in OpenXvT; check budgets between complete uploads. */
		if (usage.staged_bytes >= 64u * 1024u * 1024u || usage.copy_count >= 4096) {
			for (unsigned j = i + 1; j < set->source_count; ++j) {
				const XwRenderSource* next = XwRenderAssets_Source(set->sources[j]);
				if (Supported(next) && !Contains(next->id))
					complete = false;
			}
			break;
		}
	}
	if (cmd && !Aeron_SubmitCommandBuffer(cmd))
		return Fail(id, "upload submission failed");
	if (uploaded)
		Aeron_LogDebug("xw.remaster", "Asset batch: %u sources, %llu bytes, %u copies in %.1f ms", uploaded,
					   (unsigned long long)usage.staged_bytes, usage.copy_count,
					   (double)(Aeron_NowUs() - started) / 1000.0);
	if (!complete)
		return XW_ASSETS_PREPARING;
	Commit();
	failed_set = failed_source = 0;
	return XW_ASSETS_READY;
}

const XwMeshAsset* XwRemasterAssets_Mesh(XwRenderAssetId id) {
	for (unsigned i = 0; i < count; ++i)
		if (entries[i].id == id)
			return entries[i].asset.mesh ? &entries[i].asset : NULL;
	return NULL;
}

const XwDosMesh* XwRemasterAssets_DosMesh(XwRenderAssetId id) {
	for (unsigned i = 0; i < count; ++i)
		if (entries[i].id == id)
			return entries[i].dos.lods ? &entries[i].dos : NULL;
	return NULL;
}

const AeronRuntimeAtlas* XwRemasterAssets_Image(XwRenderAssetId id) {
	for (unsigned i = 0; i < count; ++i)
		if (entries[i].id == id)
			return entries[i].image.layout.frame_count ? &entries[i].image : NULL;
	return NULL;
}

void XwRemasterAssets_Shutdown(void) {
	Abort();
	for (unsigned i = 0; i < count; ++i)
		Destroy(&entries[i]);
	memset(entries, 0, sizeof entries);
	count = 0;
	XwRenderAssets_Release(committed_set);
	committed_set = 0;
	failed_set = failed_source = preparing_source = failed_config = 0;
	policy_loaded = false;
	XwRemasterOptMesh_Shutdown();
	XwGateLights_Shutdown();
}
