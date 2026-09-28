#ifndef XW_REMASTER_HUD_ASSETS_H
#define XW_REMASTER_HUD_ASSETS_H
#include "aeron/asset/decode_types.h"
#include "aeron/scene/runtime_atlas.h"
#include "xw_runtime/snapshot/render_cockpit_assets.h"

enum { XW_HUD_IMAGES = 768 };

enum { XW_HUD_IMAGE_PART, XW_HUD_IMAGE_BASE, XW_HUD_IMAGE_FONT };

typedef struct XwHudImageKey {
	XwRenderAssetId source;
	uint16_t frame, transparent;
	uint8_t kind;
} XwHudImageKey;

typedef struct XwHudImage {
	XwHudImageKey key;
	AeronIndexedFrame bitmap;
	AeronDecodedFont font;
	int atlas_frame;
} XwHudImage;

/* Prepare loaded resource families before selection. View selection only borrows prepared assets. */
bool XwHudAssets_Prepare(const XwRenderSnapshot* snapshot);
bool XwHudAssets_Select(const XwRenderSnapshot* snapshot);
const XwHudImage* XwHudAssets_Image(XwHudImageKey key);
const AeronRuntimeAtlas* XwHudAssets_Atlas(const XwHudImage* image);
uint64_t XwHudAssets_Generation(void);
/* Loaded artwork keeps its filter until its original resources retire. */
bool XwHudAssets_UnditherPending(int requested);
void XwHudAssets_Shutdown(void);
#endif
