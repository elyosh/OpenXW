#include "xw_runtime/snapshot/render_assets.h"
#include "xw/assets/model_mesh.h"
#include "xw/flight/object/create.h"
#include "xw_runtime/runtime/flight_types.h"
#include "xw_runtime/runtime/profile.h"
#include "xw_runtime/snapshot/render_dos_assets.h"
#include <aeron/aeron.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Source {
	XwRenderSource view;
	uint32_t references;
	uint16_t handle;
	bool live;
	void (*destroy)(void*);
} Source;

static Source sources[XW_RENDER_SOURCE_CAPACITY];
static XwSnapType bindings[XW_SNAP_TYPES];
static XwRenderAssetId next_id, current_set;
static uint8_t version;
static bool initialized, active, classic, dirty, failed;

static Source* Find(XwRenderAssetId id) {
	if (id)
		for (unsigned i = 0; i < XW_RENDER_SOURCE_CAPACITY; ++i)
			if (sources[i].view.id == id)
				return &sources[i];
	return NULL;
}

const XwRenderSource* XwRenderAssets_Source(XwRenderAssetId id) {
	Source* s = Find(id);
	return s ? &s->view : NULL;
}

const XwRenderAssetSetView* XwRenderAssets_Set(XwRenderAssetId id) {
	const XwRenderSource* s = XwRenderAssets_Source(id);
	return s && s->kind == XW_SOURCE_SET ? s->data : NULL;
}

void XwRenderAssets_Retain(XwRenderAssetId id) {
	Source* s = Find(id);
	if (s)
		++s->references;
}

static void Collect(Source* s) {
	if (s->live || s->references || !s->view.id)
		return;
	void* data = (void*)s->view.data;
	void (*destroy)(void*) = s->destroy;
	bool set = s->view.kind == XW_SOURCE_SET;
	memset(s, 0, sizeof *s);
	if (set) {
		const XwRenderAssetSetView* view = data;
		for (unsigned i = 0; i < view->source_count; ++i)
			XwRenderAssets_Release(view->sources[i]);
	}
	destroy(data);
}

void XwRenderAssets_Release(XwRenderAssetId id) {
	Source* s = Find(id);
	if (s && s->references) {
		--s->references;
		Collect(s);
	}
}

static void Retire(Source* s) {
	s->live = false;
	s->handle = 0;
	dirty = true;
	Collect(s);
}

void XwRenderAssets_Fail(const char* source, const char* reason) {
	if (!initialized || !active)
		return;
	if (!failed) {
		char message[1536];
		snprintf(message, sizeof message, "Cannot capture rendering resource %s:\n\n%s", source, reason);
		Aeron_RequestFatalError("Renderer Error", message);
	}
	failed = true;
	dirty = true;
}

void XwRenderAssets_Init(void) {
	memset(sources, 0, sizeof sources);
	memset(bindings, 0, sizeof bindings);
	next_id = 1;
	current_set = 0;
	initialized = true;
	active = dirty = failed = false;
}

void XwRenderAssets_EndMission(void) {
	active = false;
	XwRenderAssetId old = current_set;
	current_set = 0;
	/* Death Star TEX/RGB sources follow the original process-lifetime texture cache. */
	for (unsigned i = 0; i < XW_RENDER_SOURCE_CAPACITY; ++i)
		if (sources[i].view.id && sources[i].live && sources[i].view.kind != XW_SOURCE_TEXTURE)
			Retire(&sources[i]);
	XwRenderAssets_Release(old);
	memset(bindings, 0, sizeof bindings);
}

void XwRenderAssets_Shutdown(void) {
	XwRenderAssets_EndMission();
	/* Snapshot and upload owners must have released their sets before shutdown. */
	for (unsigned i = 0; i < XW_RENDER_SOURCE_CAPACITY; ++i) {
		if (sources[i].view.id) {
			sources[i].references = 0;
			Retire(&sources[i]);
		}
	}
	initialized = false;
}

void XwRenderAssets_BeginMission(uint8_t v, bool content) {
	XwRenderAssets_EndMission();
	version = v;
	classic = content;
	active = true;
	dirty = true;
	failed = false;
}

static XwRenderAssetId Adopt(XwRenderSourceKind kind, uint16_t handle, const char* path, uint8_t installation,
							 void* data, size_t size, void (*destroy)(void*)) {
	if (!initialized || !active) {
		destroy(data);
		return 0;
	}
	char normalized[XW_PATH_CAPACITY];
	if (!path || !XwStorage_Normalize(path, normalized, sizeof normalized)) {
		destroy(data);
		XwRenderAssets_Fail(path ? path : "source", "invalid source identity");
		return 0;
	}
	for (unsigned i = 0; normalized[i]; ++i)
		if (normalized[i] >= 'A' && normalized[i] <= 'Z')
			normalized[i] += 'a' - 'A';
	Source* old = NULL;
	for (unsigned i = 0; i < XW_RENDER_SOURCE_CAPACITY; ++i) {
		Source* s = &sources[i];
		if (!s->view.id || !s->live || s->view.kind != kind)
			continue;
		if (handle ? s->handle == handle
				   : (!s->handle && s->view.installation_version == installation &&
					  !strcmp(s->view.path, normalized))) {
			if (destroy == free && s->destroy == free && s->view.size == size &&
				!strcmp(s->view.path, normalized) && !memcmp(s->view.data, data, size)) {
				destroy(data);
				return s->view.id;
			}
			old = s;
			break;
		}
	}
	for (unsigned i = 0; i < XW_RENDER_SOURCE_CAPACITY; ++i) {
		Source* s = &sources[i];
		if (s->view.id)
			continue;
		s->view = (XwRenderSource) { .id = next_id++,
									 .kind = kind,
									 .flight_version = version,
									 .installation_version = installation,
									 .mission_classic = classic,
									 .data = data,
									 .size = size };
		strcpy(s->view.path, normalized);
		s->handle = handle;
		s->live = true;
		s->destroy = destroy;
		if (old)
			Retire(old);
		dirty = true;
		return s->view.id;
	}
	destroy(data);
	XwRenderAssets_Fail(path, "source registry capacity exceeded");
	return 0;
}

XwRenderAssetId XwRenderAssets_RegisterOwned(XwRenderSourceKind kind, const char* path, uint8_t installation,
											 void* data, size_t size, void (*destroy)(void*)) {
	return Adopt(kind, 0, path, installation, data, size, destroy);
}

XwRenderAssetId XwRenderAssets_RegisterBytes(XwRenderSourceKind kind, uint16_t handle, const char* path,
											 uint8_t installation, const void* bytes, size_t size) {
	if (!initialized || !active)
		return 0;
	if (!bytes || !size || size > 64u * 1024u * 1024u) {
		XwRenderAssets_Fail(path, "invalid source size");
		return 0;
	}
	char normalized[XW_PATH_CAPACITY];
	if (path && XwStorage_Normalize(path, normalized, sizeof normalized)) {
		for (unsigned i = 0; normalized[i]; ++i)
			if (normalized[i] >= 'A' && normalized[i] <= 'Z')
				normalized[i] += 'a' - 'A';
		for (unsigned i = 0; i < XW_RENDER_SOURCE_CAPACITY; ++i) {
			const Source* s = &sources[i];
			if (s->live && s->view.kind == kind && s->handle == handle &&
				s->view.installation_version == installation && s->view.size == size &&
				!strcmp(s->view.path, normalized) && !memcmp(s->view.data, bytes, size))
				return s->view.id;
		}
	}
	void* data = malloc(size);
	if (!data) {
		XwRenderAssets_Fail(path, "cannot retain source bytes");
		return 0;
	}
	memcpy(data, bytes, size);
	return Adopt(kind, handle, path, installation, data, size, free);
}

XwRenderAssetId XwRenderAssets_RegisterStream(XwRenderSourceKind kind, uint16_t handle, AeronFile* file,
											  const char* path) {
	if (!initialized || !active || !file)
		return 0;
	int64_t size = AeronVfs_GetSize(file), position = AeronVfs_Tell(file);
	if (size <= 0 || size > 64 * 1024 * 1024 || position < 0) {
		XwRenderAssets_Fail(path, "invalid file size");
		return 0;
	}
	void* data = malloc((size_t)size);
	size_t read = 0;
	bool ok = data && AeronVfs_Seek(file, 0, SEEK_SET) && AeronVfs_Read(file, data, (size_t)size, &read) &&
			  read == (size_t)size;
	bool restored = AeronVfs_Seek(file, position, SEEK_SET);
	if (!ok || !restored) {
		free(data);
		XwRenderAssets_Fail(path, "cannot retain original file");
		return 0;
	}
	return Adopt(kind, handle, path, XwGameVersion_Year(XwStorage_LastInstallation()) - 1900, data,
				 (size_t)size, free);
}

XwRenderAssetId XwRenderAssets_HandleId(uint16_t handle) {
	if (handle)
		for (unsigned i = 0; i < XW_RENDER_SOURCE_CAPACITY; ++i)
			if (sources[i].live && sources[i].handle == handle)
				return sources[i].view.id;
	return 0;
}

void XwRenderAssets_AttachHandle(uint16_t handle, XwRenderAssetId id) {
	Source* s = Find(id);
	if (s) {
		s->handle = handle;
		s->live = true;
		dirty = true;
	}
}

void XwRenderAssets_FreeHandle(uint16_t handle) {
	if (!handle)
		return;
	for (unsigned i = 0; i < XW_RENDER_SOURCE_CAPACITY; ++i)
		if (sources[i].live && sources[i].handle == handle)
			Retire(&sources[i]);
}

void XwRenderAssets_ClearDosModels(void) {
	XwRenderDosAssets_Reset();
	for (unsigned i = 0; i < XW_RENDER_SOURCE_CAPACITY; ++i) {
		Source* s = &sources[i];
		if (s->live && (s->view.kind == XW_SOURCE_DOS_MODEL || s->view.kind == XW_SOURCE_DOS_COMPONENTS))
			Retire(s);
	}
	memset(bindings, 0, sizeof bindings);
	dirty = true;
}

void XwRenderAssets_BindDos(uint16_t type, XwRenderAssetId geometry, XwRenderAssetId bitmaps,
							XwRenderAssetId alternate, XwRenderAssetId remap, XwRenderAssetId components) {
	if (type >= XW_SNAP_TYPES)
		return;
	bindings[type].geometry = geometry;
	bindings[type].bitmaps = bitmaps;
	bindings[type].alternate_bitmaps = alternate;
	bindings[type].bitmap_remap = remap;
	bindings[type].dos_components = components;
	dirty = true;
}

static void BindAnimation(uint16_t type, XwRenderAssetId id, uint32_t first, uint32_t count) {
	if (type >= XW_SNAP_TYPES)
		return;
	bindings[type].animation = id;
	bindings[type].animation_first = first;
	bindings[type].animation_count = count;
}

static void RegisterAnimations(void) {
	if (XwProfile_DosFlight())
		return;

	struct Sequence {
		const char* name;
		const uint16_t* words;
		size_t count;
	};

	static const struct Sequence sequences[] = {
		{ "animation/damage", g_componentDamageAnimationFrames,
		  sizeof(g_componentDamageAnimationFrames) / sizeof(g_componentDamageAnimationFrames[0]) },
		{ "animation/fragments", g_fragmentSecondaryAnimationFrames,
		  sizeof(g_fragmentSecondaryAnimationFrames) / sizeof(g_fragmentSecondaryAnimationFrames[0]) },
		{ "animation/type133", g_modelType133AnimationFrames,
		  sizeof(g_modelType133AnimationFrames) / sizeof(g_modelType133AnimationFrames[0]) },
		{ "animation/type137", g_modelType137AnimationFrames,
		  sizeof(g_modelType137AnimationFrames) / sizeof(g_modelType137AnimationFrames[0]) },
		{ "animation/type110", g_modelType110AnimationFrames,
		  sizeof(g_modelType110AnimationFrames) / sizeof(g_modelType110AnimationFrames[0]) },
		{ "animation/type111", g_modelType111AnimationFrames,
		  sizeof(g_modelType111AnimationFrames) / sizeof(g_modelType111AnimationFrames[0]) },
		{ "animation/type112", g_modelType112AnimationFrames,
		  sizeof(g_modelType112AnimationFrames) / sizeof(g_modelType112AnimationFrames[0]) },
		{ "animation/type113", g_modelType113AnimationFrames,
		  sizeof(g_modelType113AnimationFrames) / sizeof(g_modelType113AnimationFrames[0]) },
		{ "animation/type134", g_modelType134AnimationFrames,
		  sizeof(g_modelType134AnimationFrames) / sizeof(g_modelType134AnimationFrames[0]) },
		{ "animation/type135", g_modelType135AnimationFrames,
		  sizeof(g_modelType135AnimationFrames) / sizeof(g_modelType135AnimationFrames[0]) },
		{ "animation/type136", g_modelType136AnimationFrames,
		  sizeof(g_modelType136AnimationFrames) / sizeof(g_modelType136AnimationFrames[0]) },
	};
	for (unsigned seq = 0; seq < sizeof sequences / sizeof sequences[0]; ++seq) {
		const struct Sequence* a = &sequences[seq];
		XwRenderAssetId id = XwRenderAssets_RegisterBytes(XW_SOURCE_ANIMATION, 0, a->name, 98, a->words,
														  a->count * sizeof(uint16_t));
		for (unsigned type = 0; type < MODEL_TYPE_RECORD_COUNT; ++type) {
			for (unsigned first = 0; first < a->count; ++first)
				if (g_modelTypeTable[type].animationFrames == a->words + first) {
					BindAnimation(type, id, first, a->count - first);
					break;
				}
		}
	}
}

static void RefreshBindings(void) {
	RegisterAnimations();
	const XwFlightProfile* p = XwProfile_ActiveFlight();
	for (unsigned i = 0; i < p->model_count; ++i) {
		const XwModelTypeRecord* model = &g_modelTypeTable[i];
		XwSnapType* b = &bindings[i];
		b->max_extent = model->maxBoundsExtent;
		b->half_extent = model->halfMaxBoundsExtent;
		b->family = model->familyId;
		b->genus = model->genusId;
		b->flags = XwFlightTypes_ModelFlags(i);
		b->object_flags = model->objectFlags;
		if (version == 98) {
			XwRenderAssetId id = XwRenderAssets_HandleId(model->memoryHandle);
			const XwRenderSource* source = XwRenderAssets_Source(id);
			b->geometry = source && source->kind == XW_SOURCE_OPT ? id : 0;
			b->bitmaps = source && source->kind == XW_SOURCE_BITMAP ? id : 0;
		}
	}
}

void XwRenderAssets_Capture(XwRenderSnapshot* out) {
	if (!initialized || !active)
		return;
	RefreshBindings();
	const XwRenderAssetSetView* previous = XwRenderAssets_Set(current_set);
	if (!previous || dirty || memcmp(bindings, previous->bindings.types, sizeof bindings)) {
		XwRenderAssetSetView* set = calloc(1, sizeof *set);
		if (!set) {
			XwRenderAssets_Fail("resource set", "allocation failed");
			out->world_valid = 0;
			XwRenderAssets_Release(out->flight_assets);
			out->flight_assets = 0;
			return;
		}
		set->bindings.flight_version = version;
		set->bindings.type_count = XwProfile_ActiveFlight()->model_count;
		memcpy(set->bindings.types, bindings, sizeof bindings);
		set->failed = failed;
		for (unsigned i = 0; i < XW_RENDER_SOURCE_CAPACITY; ++i) {
			const Source* s = &sources[i];
			if (!s->live || s->view.flight_version != version || s->view.kind == XW_SOURCE_SET)
				continue;
			set->sources[set->source_count++] = s->view.id;
			if (s->view.kind == XW_SOURCE_SPECIAL_LAYOUT)
				set->bindings.special_layout = s->view.id;
			if (s->view.kind == XW_SOURCE_DOS_MATERIALS)
				set->bindings.dos_materials = s->view.id;
			if (s->view.kind == XW_SOURCE_ANIMATION && !strcmp(s->view.path, "animation/damage"))
				set->bindings.component_damage_sequence = s->view.id;
			if (s->view.kind == XW_SOURCE_ANIMATION && !strcmp(s->view.path, "animation/fragments"))
				set->bindings.fragment_sequence = s->view.id;
			if (s->view.kind == XW_SOURCE_TEXTURE && strstr(s->view.path, "deathstartexture"))
				set->bindings.surface_texture = s->view.id;
			if (s->view.kind == XW_SOURCE_TEXTURE && strstr(s->view.path, "trenchtexture"))
				set->bindings.trench_texture = s->view.id;
		}
		XwRenderAssetId id = Adopt(XW_SOURCE_SET, 0, "flight/set", version, set, sizeof *set, free);
		if (!id) {
			out->world_valid = 0;
			XwRenderAssets_Release(out->flight_assets);
			out->flight_assets = 0;
			return;
		}
		for (unsigned i = 0; i < set->source_count; ++i)
			XwRenderAssets_Retain(set->sources[i]);
		Source* source = Find(id);
		source->live = false;
		XwRenderAssets_Retain(id);
		XwRenderAssets_Release(current_set);
		current_set = id;
		dirty = false;
	}
	XwRenderAssets_ExportResident(out);
}

void XwRenderAssets_ExportResident(XwRenderSnapshot* out) {
	XwRenderAssetId id = active ? current_set : 0;
	XwRenderAssets_Retain(id);
	XwRenderAssets_Release(out->flight_assets);
	out->flight_assets = id;
}

XwRenderAssetId XwRenderAssets_Find(XwRenderSourceKind kind, const char* basename) {
	char name[XW_PATH_CAPACITY];
	if (!initialized || !active || !basename || strlen(basename) >= sizeof name)
		return 0;
	strcpy(name, basename);
	for (unsigned i = 0; name[i]; ++i)
		if (name[i] >= 'A' && name[i] <= 'Z')
			name[i] += 'a' - 'A';
	XwRenderAssetId id = 0;
	for (unsigned i = 0; i < XW_RENDER_SOURCE_CAPACITY; ++i) {
		const Source* s = &sources[i];
		const char* tail = strrchr(s->view.path, '/');
		if (s->live && s->view.flight_version == version && s->view.kind == kind &&
			!strcmp(tail ? tail + 1 : s->view.path, name) && s->view.id > id)
			id = s->view.id;
	}
	return id;
}
