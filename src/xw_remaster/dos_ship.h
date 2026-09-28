#ifndef XW_REMASTER_DOS_SHIP_H
#define XW_REMASTER_DOS_SHIP_H
#include "xw_remaster/dos_mesh.h"
#include "xw_remaster/render_math.h"

enum { XW_DOS_SELECTION_CAPACITY = 256, XW_DOS_COMPONENTS = 16 };

typedef struct XwDosPart {
	XwRenderAssetId geometry;
	uint16_t lod, descriptor, component, parent;
	float transform[16], light_direction[3];
	uint8_t markings, marking_mode, gate_tint;
	bool projectile, hyperstar;
	int16_t line_endpoints[2];
	uint8_t line_color;
} XwDosPart;

/* Descriptor selection only; bitmap decoding, palette lookup and composition are
 * shared with the DOS appearance pass. Offsets and rolls are already accumulated. */
typedef struct XwDosBitmap {
	uint16_t image, scale, descriptor, parent;
	float eye[3], angle;
} XwDosBitmap;

typedef struct XwDosSelection {
	unsigned part_count, bitmap_count;
	XwDosPart parts[XW_DOS_SELECTION_CAPACITY];
	XwDosBitmap bitmaps[XW_DOS_SELECTION_CAPACITY];
} XwDosSelection;

bool XwDosShip_Eligible(const XwRenderSnapshot* s, const XwSnapObject* object);
bool XwDosShip_Select(const XwRenderSnapshot* s, const XwRenderAssetSet* set, const XwSnapObject* object,
					  const XwRenderView* view, const float* rotations, XwDosSelection* out);
/* Select a direct scenery/course component at its highest-detail LOD. */
bool XwDosShip_Component(const XwRenderSnapshot* s, const XwRenderAssetSet* set, const XwSnapObject* object,
						 const XwRenderView* view, unsigned component, unsigned parent, XwDosPart* out);
/* DOS93 explicit assembly order and DOS94 authored component BSP. */
bool XwDosShip_Order(const XwRenderSnapshot* s, const XwSnapObject* object, const XwSnapCraft* craft,
					 unsigned model, const XwRenderDosModel* geometry, unsigned count, uint16_t out[16]);
void XwDosShip_Articulate(float pose[16], unsigned model, unsigned part, float rotation, uint8_t version);
#endif
