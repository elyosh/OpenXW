#ifndef XW_REMASTER_DOS_IMAGES_H
#define XW_REMASTER_DOS_IMAGES_H
#include "xw_remaster/image_assets.h"
/* RGBA8 data atlases: red stores an unchanged nibble index; alpha stores coverage. */
bool XwDosImages_Build(AeronCommandBuffer* cmd, const XwRenderSource* source, AeronRuntimeAtlas* out,
					   char* error, size_t capacity);
#endif
