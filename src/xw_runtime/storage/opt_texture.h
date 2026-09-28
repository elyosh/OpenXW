#ifndef XW_RUNTIME_STORAGE_OPT_TEXTURE_H
#define XW_RUNTIME_STORAGE_OPT_TEXTURE_H

#include "xw/assets/opt_model.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Destination has the serialized size plus the native/file header size difference. */
void XwPort_LoadNativeOptTexture(OptTextureData* destination, const char* fileName);

#ifdef __cplusplus
}
#endif

#endif
