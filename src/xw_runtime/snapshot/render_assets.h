#ifndef XW_RENDER_ASSETS_H
#define XW_RENDER_ASSETS_H
#include "xw_runtime/snapshot/render_types.h"
#include "xw_runtime/storage/storage.h"

/* Registry views are immutable and borrowed for the lifetime of a retained set. */
typedef enum XwRenderSourceKind {
	XW_SOURCE_OPT,
	XW_SOURCE_BITMAP,
	XW_SOURCE_TEXTURE, /* Process-resident Death Star TEX/RGB files. */
	XW_SOURCE_FONT,
	XW_SOURCE_PALETTE,
	XW_SOURCE_PANEL,
	XW_SOURCE_COCKPIT_LAYOUT,
	XW_SOURCE_COCKPIT,
	XW_SOURCE_DOS_MODEL,
	XW_SOURCE_DOS_COMPONENTS,
	XW_SOURCE_DOS_MATERIALS,
	XW_SOURCE_ANIMATION,
	XW_SOURCE_STARS,
	XW_SOURCE_COCKPIT_DEFINITION,
	XW_SOURCE_SPECIAL_LAYOUT,
	XW_SOURCE_SET
} XwRenderSourceKind;

typedef struct XwRenderSource {
	XwRenderAssetId id;
	XwRenderSourceKind kind;
	uint8_t flight_version, installation_version;
	bool mission_classic;
	char path[XW_PATH_CAPACITY];
	const void* data;
	size_t size; /* Bytes at data; typed payloads expose separately bounded owned arrays. */
} XwRenderSource;

enum { XW_RENDER_SOURCE_CAPACITY = 1024 };

typedef struct XwRenderAssetSetView {
	XwRenderAssetSet bindings;
	uint32_t source_count;
	XwRenderAssetId sources[XW_RENDER_SOURCE_CAPACITY];
	bool failed;
} XwRenderAssetSetView;

/* Latest live source for the active flight version, including resident textures. */
XwRenderAssetId XwRenderAssets_Find(XwRenderSourceKind kind, const char* basename);
void XwRenderAssets_Init(void);
void XwRenderAssets_Shutdown(void);
void XwRenderAssets_BeginMission(uint8_t version, bool classic);
void XwRenderAssets_EndMission(void);
void XwRenderAssets_Retain(XwRenderAssetId id);
void XwRenderAssets_Release(XwRenderAssetId id);
const XwRenderSource* XwRenderAssets_Source(XwRenderAssetId id);
const XwRenderAssetSetView* XwRenderAssets_Set(XwRenderAssetId id);
/* Assign a retained immutable set; snapshots release it through Clear. */
void XwRenderAssets_Capture(XwRenderSnapshot* snapshot);
/* Export the last captured resident set without reading suspended simulation state. */
void XwRenderAssets_ExportResident(XwRenderSnapshot* snapshot);
void XwRenderAssets_Fail(const char* source, const char* reason);
/* Copies original bytes. Stream registration preserves the loader's file position. */
XwRenderAssetId XwRenderAssets_RegisterBytes(XwRenderSourceKind kind, uint16_t handle, const char* path,
											 uint8_t installation_version, const void* bytes, size_t size);
XwRenderAssetId XwRenderAssets_RegisterStream(XwRenderSourceKind kind, uint16_t handle, AeronFile* file,
											  const char* resolved_path);
/* Adopt a typed allocation. A destructor must release the entire owned representation. */
XwRenderAssetId XwRenderAssets_RegisterOwned(XwRenderSourceKind kind, const char* identity,
											 uint8_t installation_version, void* data, size_t size,
											 void (*destroy)(void*));
XwRenderAssetId XwRenderAssets_HandleId(uint16_t handle);
void XwRenderAssets_AttachHandle(uint16_t handle, XwRenderAssetId id);
void XwRenderAssets_FreeHandle(uint16_t handle);
void XwRenderAssets_ClearDosModels(void);
void XwRenderAssets_BindDos(uint16_t type, XwRenderAssetId geometry, XwRenderAssetId bitmaps,
							XwRenderAssetId alternate, XwRenderAssetId remap, XwRenderAssetId components);
#endif
