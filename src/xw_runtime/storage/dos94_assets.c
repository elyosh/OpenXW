#include "xw_runtime/storage/dos94_assets.h"

#include "xw_dos94/assets/models.h"
#include "xw_dos94/render/world.h"
#include "xw_runtime/snapshot/render_dos_assets.h"
#include "xw_runtime/storage/file_io.h"
#include <stdlib.h>
#include <string.h>

enum { DOS94_ALLOCATION_COUNT = DOS94_MODEL_COUNT * 3 };

typedef struct Dos94Allocation {
	uint8_t* data;
	size_t size;
} Dos94Allocation;

static Dos94Allocation allocations[DOS94_ALLOCATION_COUNT];
static Dos94Model models[DOS94_MODEL_COUNT];
static uint32_t generation = 1;
static uint16_t allocationCount;
static XwGameVersion selectedVersion = XW_GAME_VERSION_94;
static const Dos94ModelMetadata* modelTemplates = Dos94_modelTemplates;

void Dos94Assets_Select(XwGameVersion version) {
	selectedVersion = version;
	modelTemplates = version == XW_GAME_VERSION_93 ? Dos93_modelTemplates : Dos94_modelTemplates;
	Dos94Assets_Reset();
}

XwGameVersion Dos94Assets_Version(void) { return selectedVersion; }

const Dos94ModelMetadata* Dos94Assets_Template(uint16_t model) {
	return model < DOS94_MODEL_COUNT ? &modelTemplates[model] : NULL;
}

void Dos94Assets_ReleaseModels(void) {
	XwRenderAssets_ClearDosModels();
	/* Includes failed lazy loads; detach renderer views before freeing their owners. */
	Dos94Renderer_InvalidateAssets();
	Dos94Models_CloseCatalog();
	/* Invalidate derived references before freeing each unique allocation. */
	for (uint16_t i = 0; i < DOS94_MODEL_COUNT; ++i) {
		Dos94ModelMetadata metadata = models[i].metadata;
		models[i] = (Dos94Model) { .metadata = metadata };
	}
	for (uint16_t i = 0; i < allocationCount; ++i)
		free(allocations[i].data);
	memset(allocations, 0, sizeof allocations);
	allocationCount = 0;
	if (++generation == 0)
		++generation;
}

void Dos94Assets_Reset(void) {
	Dos94Assets_ReleaseModels();
	for (uint16_t i = 0; i < DOS94_MODEL_COUNT; ++i)
		models[i].metadata = modelTemplates[i];
}

Dos94Model* Dos94Assets_Model(uint16_t model) { return model < DOS94_MODEL_COUNT ? &models[model] : NULL; }

uint8_t* Dos94Assets_Data(Dos94ByteView view) {
	if (view.generation != generation || view.allocation >= allocationCount)
		return NULL;
	Dos94Allocation* allocation = &allocations[view.allocation];
	if (view.offset > allocation->size || view.size > allocation->size - view.offset)
		return NULL;
	return allocation->data + view.offset;
}

bool Dos94Assets_Allocate(size_t size, Dos94ByteView* out) {
	if (!out || !size || allocationCount == DOS94_ALLOCATION_COUNT)
		return false;
	uint8_t* data = malloc(size);
	if (!data)
		return false;
	allocations[allocationCount] = (Dos94Allocation) { data, size };
	*out = (Dos94ByteView) { generation, allocationCount++, 0, size };
	return true;
}

bool Dos94Assets_Read(AeronFile* file, size_t size, Dos94ByteView* out) {
	Dos94ByteView view;
	if (!file || !out || !Dos94Assets_Allocate(size, &view))
		return false;
	if (XwFile_Read(Dos94Assets_Data(view), 1, size, file) != size) {
		free(allocations[--allocationCount].data);
		allocations[allocationCount] = (Dos94Allocation) { 0 };
		return false;
	}
	*out = view;
	return true;
}

bool Dos94Assets_Subview(Dos94ByteView view, size_t offset, size_t size, Dos94ByteView* out) {
	if (!out || !Dos94Assets_Data(view) || offset > view.size || size > view.size - offset)
		return false;
	view.offset += offset;
	view.size = size;
	*out = view;
	return true;
}

bool Dos94Assets_Alias(uint16_t model, uint16_t source) {
	if (model >= DOS94_MODEL_COUNT || source >= DOS94_MODEL_COUNT || model == source)
		return false;
	models[model].payload = models[source].payload;
	models[model].decoded = models[source].decoded;
	memcpy(models[model].resourceType, models[source].resourceType, sizeof models[model].resourceType);
	memcpy(models[model].resourceName, models[source].resourceName, sizeof models[model].resourceName);
	XwRenderDosAssets_Alias(model, source);
	return true;
}

AeronFile* Dos94Assets_OpenSpecies(void) {
	return XwStorage_OpenInstallation(selectedVersion, "X-Wing Data/RESOURCE/SPECIES.LFD");
}
