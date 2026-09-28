#ifndef XW_RENDER_COCKPIT_ASSETS_H
#define XW_RENDER_COCKPIT_ASSETS_H
#include "xw_runtime/snapshot/render_assets.h"

typedef struct XwCockpitPart {
	XwRenderAssetId source;
	uint16_t frame, width, height;
} XwCockpitPart;

typedef struct XwCockpitElement {
	int16_t x, y;
	uint8_t sprite, selector;
} XwCockpitElement;

/* Immutable owned bindings. The registry retains every referenced original resource. */
typedef struct XwCockpitView {
	XwRenderAssetId image;
	XwSnapRect aperture;
	uint32_t palette_argb[64]; /* Original PLTT, resolved through the active display profile. */
	char label[12];
	uint8_t resource, mirrored;
} XwCockpitView;

typedef struct XwCockpitDefinition {
	XwRenderAssetId base, fonts[2];
	XwCockpitPart parts[XW_SNAP_PANEL_SPRITES];
	XwCockpitElement elements[XW_SNAP_WIDGETS];
	XwCockpitView views[XW_SNAP_COCKPIT_VIEWS];
	XwSnapRect aperture;
	uint16_t width, height;
	uint8_t version, color_mode;
	int32_t brightness_q8;
	uint32_t palette_argb[256]; /* Loaded display palette for resource preparation. */
} XwCockpitDefinition;

XwRenderAssetId XwCockpitAssets_Loaded(void);
void XwCockpitAssets_Palette(uint32_t colors[256]);
void XwCockpitAssets_Reset(void);
void XwCockpitAssets_Panel(XwRenderAssetId source, unsigned first, unsigned count, unsigned skip);
XwRenderAssetId XwCockpitAssets_Define(const char* base, XwSnapRect aperture);
const XwCockpitDefinition* XwCockpitAssets_Definition(XwRenderAssetId id);
#endif
