#include "xw_dos94/assets/models.h"

#include "xw_dos94/assets/resident_models.h"
#include "xw_runtime/snapshot/render_dos_assets.h"
#include "xw_runtime/storage/file_io.h"
#include <stdio.h>
#include <string.h>

/* DOS94 0x6826CA: catalogue indices follow the resident records, not resource names. */
typedef struct Dos94ArchiveEntry {
	uint32_t offset, size;
	bool present;
} Dos94ArchiveEntry;

static AeronFile* species;
static char speciesPath[XW_PATH_CAPACITY];
static uint8_t speciesInstallation;
static Dos94ArchiveEntry catalog[DOS94_MODEL_COUNT];
static bool surfaceMission, courseMission;
static Dos94ByteView projectilePayloads[6], hyperstar;
static Dos94Lod* projectileLods[6];
static uint16_t projectileLodCounts[6];

static const char* mesh_tag(void) { return Dos94Assets_Version() == XW_GAME_VERSION_93 ? "CRFT" : "CPLX"; }

static const char* bitmap_tag(void) { return Dos94Assets_Version() == XW_GAME_VERSION_93 ? "BTMP" : "BMAP"; }

static uint32_t u32(const uint8_t* p) {
	return p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static bool error_model(char* error, size_t capacity, uint16_t model, const char* reason) {
	Dos94Model* record = Dos94Assets_Model(model);
	snprintf(error, capacity, "X-Wing %d %s, model %u (%s): %s", XwGameVersion_Year(Dos94Assets_Version()),
			 model == 118 ? "RESOURCE/BWING.CFT" : "RESOURCE/SPECIES.LFD", model,
			 record ? record->resourceName : "", reason);
	return false;
}

void Dos94Models_CloseCatalog(void) {
	if (species)
		XwFile_Close(species);
	species = NULL;
	memset(catalog, 0, sizeof catalog);
	memset(projectilePayloads, 0, sizeof projectilePayloads);
	memset(projectileLods, 0, sizeof projectileLods);
	memset(projectileLodCounts, 0, sizeof projectileLodCounts);
	hyperstar = (Dos94ByteView) { 0 };
}

static bool selected(uint8_t flags) {
	return (flags & 0x18) && (!(flags & 0x80) || surfaceMission) && (!(flags & 0x20) || !surfaceMission) &&
		   (!(flags & 0x40) || courseMission);
}

static bool read_catalog(char* error, size_t capacity) {
	uint8_t header[16];
	int32_t length = XwFile_Length(species);
	if (length < 16 || XwFile_Read(header, 1, sizeof header, species) != sizeof header ||
		memcmp(header, "RMAP", 4) || u32(header + 12) > (uint32_t)length - 16 || (u32(header + 12) & 15))
		return error_model(error, capacity, 0, "invalid archive directory");
	uint32_t cursor = 16 + u32(header + 12);
	for (uint16_t i = 0; i < DOS94_MODEL_COUNT; ++i) {
		const Dos94ModelMetadata* original = Dos94Assets_Template(i);
		/* Required-bit edits must not insert entries into the original archive association. */
		if (original->isAlias || !original->flags || i == 118)
			continue;
		if (cursor > (uint32_t)length || (uint32_t)length - cursor < sizeof header ||
			XwFile_Seek(species, (int32_t)cursor, SEEK_SET) ||
			XwFile_Read(header, 1, sizeof header, species) != sizeof header)
			return error_model(error, capacity, i, "missing archive entry");
		cursor += sizeof header;
		uint32_t size = u32(header + 12);
		if (size > (uint32_t)length - cursor ||
			memcmp(header, original->flags & 1 ? mesh_tag() : bitmap_tag(), 4))
			return error_model(error, capacity, i, "invalid archive entry");
		catalog[i] = (Dos94ArchiveEntry) { cursor, size, true };
		Dos94Model* model = Dos94Assets_Model(i);
		memcpy(model->resourceType, header, 4);
		memcpy(model->resourceName, header + 4, 8);
		cursor += size;
	}
	return true;
}

static bool load_payload(uint16_t type, char* error, size_t capacity) {
	Dos94Model* model = Dos94Assets_Model(type);
	AeronFile* input = species;
	uint32_t size;
	if (type == 118) {
		input = XwStorage_OpenInstallation(Dos94Assets_Version(), "X-Wing Data/RESOURCE/BWING.CFT");
		uint8_t prefix[2];
		if (!input)
			return error_model(error, capacity, type, "cannot open required file");
		int32_t length = XwFile_Length(input);
		if (length < 2 || XwFile_Read(prefix, 1, 2, input) != 2) {
			XwFile_Close(input);
			return error_model(error, capacity, type, "invalid size prefix");
		}
		size = prefix[0] | (uint16_t)prefix[1] << 8;
		if (size > (uint32_t)length - 2) {
			XwFile_Close(input);
			return error_model(error, capacity, type, "truncated model");
		}
		memcpy(model->resourceType, mesh_tag(), 5);
		memcpy(model->resourceName, "BWING", 6);
	} else {
		const Dos94ArchiveEntry* entry = &catalog[type];
		uint32_t skip = (model->metadata.flags & 1) ? 2 : 0;
		if (!input || !entry->present || entry->size < skip ||
			XwFile_Seek(input, (int32_t)(entry->offset + skip), SEEK_SET))
			return error_model(error, capacity, type, "cannot read model entry");
		size = entry->size - skip;
	}
	bool loaded = size && size <= UINT16_MAX && Dos94Assets_Read(input, size, &model->payload);
	if (type == 118 && XwFile_Close(input))
		loaded = false;
	if (!loaded)
		return error_model(error, capacity, type, "cannot allocate/read bounded model payload");
	if (!Dos94Models_Decode(model))
		return error_model(error, capacity, type, "invalid component, LOD or bitmap data");
	XwRenderDosAssets_Register(type, type == 118 ? XwStorage_LastPath() : speciesPath,
							   type == 118 ? XwGameVersion_Year(Dos94Assets_Version()) - 1900
										   : speciesInstallation);
	return true;
}

static bool ensure_model(uint16_t type, unsigned depth, char* error, size_t capacity) {
	Dos94Model* model = Dos94Assets_Model(type);
	if (!model || depth >= DOS94_MODEL_COUNT)
		return error_model(error, capacity, type, "invalid model alias");
	if (Dos94Assets_Data(model->payload))
		return true;
	if (model->metadata.isAlias) {
		if (!ensure_model(model->metadata.flags, depth + 1, error, capacity))
			return false;
		return Dos94Assets_Alias(type, model->metadata.flags);
	}
	if (!selected(model->metadata.flags))
		return error_model(error, capacity, type, "model is not selected for this mission");
	if (!load_payload(type, error, capacity))
		return false;
	/* Aliases share payload mutations as well as the decoded indices. */
	for (uint16_t i = 0; i < DOS94_MODEL_COUNT; ++i) {
		Dos94Model* alias = Dos94Assets_Model(i);
		if (alias->metadata.isAlias && alias->metadata.flags == type)
			Dos94Assets_Alias(i, type);
	}
	return true;
}

bool Dos94Models_Ensure(uint16_t model, char* error, size_t capacity) {
	if (!species)
		return error_model(error, capacity, model, "model catalogue is not open");
	if (ensure_model(model, 0, error, capacity))
		return true;
	/* Failed startup/lazy acquisition cannot leave borrowed pointers to partial data. */
	Dos94Assets_ReleaseModels();
	return false;
}

static bool load_resident(char* error, size_t capacity) {
	for (unsigned i = 0; i < 6; ++i) {
		const Dos94ResidentModel* source = &Dos94_projectileModels[i];
		if (!Dos94Assets_Allocate(source->size, &projectilePayloads[i]))
			return error_model(error, capacity, 18, "cannot allocate resident projectile data");
		memcpy(Dos94Assets_Data(projectilePayloads[i]), source->bytes, source->size);
		if (!Dos94Models_DecodeLods(projectilePayloads[i], 0, DOS94_MESH_SIMPLE, &projectileLods[i],
									&projectileLodCounts[i]))
			return error_model(error, capacity, 18, "invalid resident projectile data");
	}
	if (!Dos94Assets_Allocate(sizeof Dos94_hyperstarData, &hyperstar))
		return error_model(error, capacity, 0, "cannot allocate hyperspace line data");
	memcpy(Dos94Assets_Data(hyperstar), Dos94_hyperstarData, sizeof Dos94_hyperstarData);
	return true;
}

bool Dos94_fediskio_loadspecies(bool surface, bool provingGrounds, char* error, size_t capacity) {
	Dos94Assets_ReleaseModels();
	surfaceMission = surface;
	courseMission = provingGrounds;
	species = Dos94Assets_OpenSpecies();
	if (!species) {
		error_model(error, capacity, 0,
					Dos94Assets_Version() == XW_GAME_VERSION_93
						? "cannot open required 1993 species archive"
						: "cannot open species archive or documented OLSPECS fallback");
		return false;
	}
	snprintf(speciesPath, sizeof speciesPath, "%s", XwStorage_LastPath());
	speciesInstallation = XwGameVersion_Year(XwStorage_LastInstallation()) - 1900;
	bool loaded = read_catalog(error, capacity) && load_resident(error, capacity);
	if (loaded)
		XwRenderDosAssets_Resident();
	for (uint16_t i = 0; loaded && i < DOS94_MODEL_COUNT; ++i) {
		Dos94Model* model = Dos94Assets_Model(i);
		if (!model->metadata.isAlias && (catalog[i].present || i == 118) && selected(model->metadata.flags))
			loaded = ensure_model(i, 0, error, capacity);
	}
	if (!loaded)
		Dos94Assets_ReleaseModels();
	return loaded;
}

const Dos94Lod* Dos94Models_ProjectileLods(uint16_t model, uint16_t* count) {
	/* DOS94 0x51640: green and ion pairs intentionally share their geometry. */
	static const uint8_t indices[8] = { 0, 1, 2, 2, 3, 3, 4, 5 };
	if (model < 18 || model > 25 || !count)
		return NULL;
	unsigned index = indices[model - 18];
	if (!Dos94Assets_Data(projectilePayloads[index]))
		return NULL;
	*count = projectileLodCounts[index];
	return projectileLods[index];
}

bool Dos94Models_Hyperstar(Dos94MeshView* out) {
	return Dos94Models_DecodeMesh(hyperstar, 0, DOS94_MESH_SIMPLE, out);
}
