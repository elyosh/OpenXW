#ifndef XW_REMASTER_EFFECTS_H
#define XW_REMASTER_EFFECTS_H
#include "xw_remaster/flight.h"
#include <aeron/scene/billboard.h>

typedef struct XwEffectFrame {
	AeronTexture* texture;
	float u0, v0, u1, v1;
	int width, height;
} XwEffectFrame;

bool XwEffects_Frame(XwRenderAssetId set, unsigned type, unsigned frame, XwEffectFrame* out);
void XwEffects_SetFrame(AeronSceneBillboardDesc* billboard, const XwEffectFrame* frame, float strength,
						float alpha);
bool XwEffects_Submit(AeronScene3D* scene, const XwRenderSnapshot* snapshot, const XwPreparedFlight* frame);
#endif
