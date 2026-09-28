#ifndef XW_HUD_ASSET_UPLOAD_H
#define XW_HUD_ASSET_UPLOAD_H
#include "xw_remaster/hud_assets.h"
bool XwHudAssetUpload(AeronCommandBuffer* cmd, const XwHudImage* images, unsigned count,
					  const uint32_t colors[256], bool filter, AeronRuntimeAtlas* out);
#endif
