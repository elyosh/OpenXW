#ifndef XW_REMASTER_IMAGE_ASSETS_H
#define XW_REMASTER_IMAGE_ASSETS_H
#include "xw_runtime/snapshot/render_assets.h"
#include <aeron/scene/runtime_atlas.h>

/* Decode immutable original sources; the caller commits only after command submission. */
bool XwImageAssets_Build(AeronCommandBuffer* cmd, const XwRenderSource* source, AeronRuntimeAtlas* out,
						 char* error, size_t capacity);
#endif
