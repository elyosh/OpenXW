#ifndef XW_RENDER_RENDER_TEXTURE_H
#define XW_RENDER_RENDER_TEXTURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/render/std3d.h"

#include <stddef.h>
#include <stdint.h>

enum { RENDER_TEXTURE_CACHE_CAPACITY = 1024, RENDER_TEXTURE_CACHE_UNINITIALIZED = -1 };

enum {
	RENDER_TEXTURE_COLOR_KEY_OFFSET = 1,
	RENDER_TEXTURE_RUN_MASK_TABLE_SIZE = 16,
	RENDER_TEXTURE_COLOR_SHIFT_TABLE_SIZE = 12,
	RENDER_TEXTURE_SCRATCH_CAPACITY = STD3D_DEVICE_MAX_TEXTURE_SIZE * STD3D_DEVICE_MAX_TEXTURE_SIZE
};

extern uint8_t g_bitmapRleRunMask[RENDER_TEXTURE_RUN_MASK_TABLE_SIZE];
extern uint8_t g_bitmapRleColorShift[RENDER_TEXTURE_COLOR_SHIFT_TABLE_SIZE];
extern int g_renderTextureCacheCursor;
extern uint8_t g_bitmapDecodeScratchPixels[RENDER_TEXTURE_SCRATCH_CAPACITY];
extern uint8_t g_renderTextureColorKeyScratch[RENDER_TEXTURE_SCRATCH_CAPACITY];
extern const void* g_renderTextureCacheKeys[RENDER_TEXTURE_CACHE_CAPACITY];
extern Std3DTexCacheNode g_renderTextureCache[RENDER_TEXTURE_CACHE_CAPACITY];
/* Declarations follow ascending original IDB address. */

/* 0x47E750 */
struct Std3DTexCacheNode* RenderTexture_FindOrAllocateCacheEntry(const void* cacheKey);

/* 0x47E840 */
struct Std3DTexCacheNode* RenderTexture_GetOrCreateBitmap(int textureWidth, int textureHeight,
														  uint16_t* palette, const uint8_t* rlePixels,
														  int rleFormatIndex);

/* 0x47EBD0 */
struct Std3DTexCacheNode* RenderTexture_GetOrCreateOpaque(int width, int height, const uint16_t* palette,
														  const uint8_t* pixels);

/* 0x47ECA0 */
struct Std3DTexCacheNode* RenderTexture_GetOrCreateColorKey(int width, int height, uint16_t* palette,
															const uint8_t* pixels);

#ifdef __cplusplus
}
#endif

#endif
