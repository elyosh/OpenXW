#include "xw/render/render_texture.h"

#include "xw/flight/flight_display.h"
#include "xw/render/rotscale.h"
#include "xw/render/rtsrgb.h"
#include "xw/util/shared.h"
#include "xw_runtime/compat/pointer_hash.h"

#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4D9DB8
uint8_t g_bitmapRleRunMask[RENDER_TEXTURE_RUN_MASK_TABLE_SIZE] = { 0,   1, 3, 7, 15, 31, 63, 127,
																   255, 0, 0, 0, 0,  0,  0,  0 };

// GLOBAL: XW 0x4D9DC8
uint8_t g_bitmapRleColorShift[RENDER_TEXTURE_COLOR_SHIFT_TABLE_SIZE] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 0, 0 };

// GLOBAL: XW 0x53C994
int g_renderTextureCacheCursor = 0;

// GLOBAL: XW 0x53CA20
uint8_t g_bitmapDecodeScratchPixels[RENDER_TEXTURE_SCRATCH_CAPACITY] = { 0 };

// GLOBAL: XW 0x54CAA8
uint8_t g_renderTextureColorKeyScratch[RENDER_TEXTURE_SCRATCH_CAPACITY] = { 0 };

// GLOBAL: XW 0x5C0260
const void* g_renderTextureCacheKeys[RENDER_TEXTURE_CACHE_CAPACITY] = { NULL };

// GLOBAL: XW 0x5C1260
Std3DTexCacheNode g_renderTextureCache[RENDER_TEXTURE_CACHE_CAPACITY] = { 0 };

// FUNCTION: XW 0x47E750
struct Std3DTexCacheNode* RenderTexture_FindOrAllocateCacheEntry(const void* cacheKey) {
	int startSlot;
	int probeCount;
	if (g_renderTextureCacheCursor == RENDER_TEXTURE_CACHE_UNINITIALIZED) {
		int index;
		memset(g_renderTextureCacheKeys, 0, sizeof(g_renderTextureCacheKeys));
		for (index = 0; index < RENDER_TEXTURE_CACHE_CAPACITY; ++index) {
			g_renderTextureCache[index].bCached = 0;
		}
	}
	startSlot = (int)XwPointerHash_LowBits(cacheKey, RENDER_TEXTURE_CACHE_CAPACITY - 1);
	g_renderTextureCacheCursor = startSlot;
	for (probeCount = 0; probeCount < RENDER_TEXTURE_CACHE_CAPACITY; ++probeCount) {
		if (g_renderTextureCacheKeys[g_renderTextureCacheCursor] == cacheKey) {
			break;
		}
		if (++g_renderTextureCacheCursor == RENDER_TEXTURE_CACHE_CAPACITY) {
			g_renderTextureCacheCursor = 0;
		}
	}
	if (probeCount == RENDER_TEXTURE_CACHE_CAPACITY) {
		g_renderTextureCacheCursor = startSlot;
		for (probeCount = 0; probeCount < RENDER_TEXTURE_CACHE_CAPACITY; ++probeCount) {
			if (g_renderTextureCache[g_renderTextureCacheCursor].bCached == 0) {
				break;
			}
			if (++g_renderTextureCacheCursor == RENDER_TEXTURE_CACHE_CAPACITY) {
				g_renderTextureCacheCursor = 0;
			}
		}
		if (probeCount == RENDER_TEXTURE_CACHE_CAPACITY) {
			nullsub_SharedNoOp();
			return &g_renderTextureCache[g_renderTextureCacheCursor];
		}
	}
	g_renderTextureCacheKeys[g_renderTextureCacheCursor] = cacheKey;
	return &g_renderTextureCache[g_renderTextureCacheCursor];
}

// FUNCTION: XW 0x47E840
struct Std3DTexCacheNode* RenderTexture_GetOrCreateBitmap(int textureWidth, int textureHeight,
														  uint16_t* palette, const uint8_t* rlePixels,
														  int rleFormatIndex) {
	Std3DTexCacheNode* cacheEntry;
	Std3DVBuffer bitmapBuffer;
	unsigned int maximumPaletteIndex = 0;
	size_t sourceIndex;
	ptrdiff_t outputIndex;
	unsigned int paletteBase;
	int row;
	int pixelCount;
	int savedAlphaTexture;
#ifdef XW_MODERN
	pixelCount = (int32_t)((uint32_t)textureWidth * (uint32_t)textureHeight);
#else
	pixelCount = textureWidth * textureHeight;
#endif
	if (pixelCount > RENDER_TEXTURE_SCRATCH_CAPACITY) {
		nullsub_SharedNoOp();
		return NULL;
	}
	cacheEntry = RenderTexture_FindOrAllocateCacheEntry(rlePixels);
	if (cacheEntry->bCached != 0) {
		std3D_CacheTextureSurface(cacheEntry);
		return cacheEntry;
	}
	sourceIndex = 0;
	outputIndex = 0;
	paletteBase = 0;
	for (row = 0; row < textureHeight; ++row) {
		int column = 0;
		ptrdiff_t rowEnd = outputIndex + textureWidth;
		if (rlePixels[sourceIndex] == ROTSCALE_RLE_END_SPRITE)
			break;
		for (; rlePixels[sourceIndex] != ROTSCALE_RLE_END_ROW;) {
			uint8_t opcode = rlePixels[sourceIndex];
			if (opcode == ROTSCALE_RLE_PALETTE_BASE) {
				paletteBase = rlePixels[sourceIndex + 1] +
							  ((unsigned int)rlePixels[sourceIndex + 2] << STD3D_PALETTE_BITS);
				sourceIndex += 1 + sizeof(uint16_t);
			} else if (opcode == ROTSCALE_RLE_SKIP) {
				uint8_t runLength = (uint8_t)(rlePixels[sourceIndex + 1] + 1);
				sourceIndex += 1 + sizeof(runLength);
				if (column < textureWidth) {
					column += runLength;
					if (column > textureWidth) {
						runLength = (uint8_t)(textureWidth - (column - runLength));
						column = textureWidth;
					}
					if (runLength != 0) {
						memset(&g_bitmapDecodeScratchPixels[outputIndex], 0, runLength);
						outputIndex += runLength;
					}
				}
			} else {
				uint8_t runLength;
				uint8_t colorIndex;
				if (opcode == ROTSCALE_RLE_EXPLICIT_RUN) {
					colorIndex = rlePixels[sourceIndex + 2];
					runLength = (uint8_t)(rlePixels[sourceIndex + 1] + 1);
					sourceIndex += 1 + sizeof(runLength) + sizeof(colorIndex);
				} else {
					colorIndex = (uint8_t)(paletteBase + (opcode >> g_bitmapRleColorShift[rleFormatIndex]));
					runLength = (uint8_t)((opcode & g_bitmapRleRunMask[rleFormatIndex]) + 1);
					++sourceIndex;
				}
				if (colorIndex > maximumPaletteIndex)
					maximumPaletteIndex = colorIndex;
				if (column < textureWidth) {
					column += runLength;
					if (column > textureWidth) {
						runLength = (uint8_t)(textureWidth - (column - runLength));
						column = textureWidth;
					}
					if (runLength != 0) {
						memset(&g_bitmapDecodeScratchPixels[outputIndex], colorIndex, runLength);
						outputIndex += runLength;
					}
				}
			}
		}
		++sourceIndex;
		if (outputIndex < rowEnd) {
			memset(&g_bitmapDecodeScratchPixels[outputIndex], 0, rowEnd - outputIndex);
			outputIndex = rowEnd;
		}
	}
	if (row < textureHeight)
		memset(&g_bitmapDecodeScratchPixels[outputIndex], 0, textureWidth * (textureHeight - row));
	memset(&bitmapBuffer, 0, sizeof(bitmapBuffer));
	bitmapBuffer.pixels = g_bitmapDecodeScratchPixels;
	bitmapBuffer.storageType = STD3D_STORAGE_SOFTWARE;
	bitmapBuffer.raster.width = textureWidth;
	bitmapBuffer.raster.height = textureHeight;
	bitmapBuffer.raster.pitch = textureWidth;
	bitmapBuffer.raster.colorInfo.colorMode = STDCOLOR_PAL;
	bitmapBuffer.raster.colorInfo.bpp = STD3D_PALETTE_BITS;
	savedAlphaTexture = g_pStd3DCurDevice->caps.bAlphaTexture;
	if (g_pStd3DCurDevice->caps.bColorKeyTexture != 0)
		g_pStd3DCurDevice->caps.bAlphaTexture = 0;
	if (g_pStd3DCurDevice->caps.bAlphaTexture != 0) {
		palette[0] = g_flightTextPalette[g_flightBackgroundPaletteIndex];
		std3D_ConvertTexTo1555(palette, maximumPaletteIndex + 1);
	} else {
		palette[0] = g_flightTextPalette[g_flightBackgroundPaletteIndex];
		std3D_CopyPaletteToScratch16(palette, maximumPaletteIndex + 1);
	}
	if (std3D_CreateMipSurface(&bitmapBuffer, cacheEntry, 1, 0) == 0) {
		nullsub_SharedNoOp();
		if (g_pStd3DCurDevice->caps.bColorKeyTexture != 0)
			g_pStd3DCurDevice->caps.bAlphaTexture = savedAlphaTexture;
		return NULL;
	}
	if (g_pStd3DCurDevice->caps.bColorKeyTexture != 0)
		g_pStd3DCurDevice->caps.bAlphaTexture = savedAlphaTexture;
	return cacheEntry;
}

// FUNCTION: XW 0x47EBD0
struct Std3DTexCacheNode* RenderTexture_GetOrCreateOpaque(int width, int height, const uint16_t* palette,
														  const uint8_t* pixels) {
	Std3DTexCacheNode* node;
	Std3DVBuffer source;
	node = RenderTexture_FindOrAllocateCacheEntry(pixels);
	if (node->bCached != 0) {
		std3D_CacheTextureSurface(node);
		return node;
	}
	memset(&source, 0, sizeof(source));
	source.storageType = STD3D_STORAGE_SOFTWARE;
	source.raster.colorInfo.colorMode = STDCOLOR_PAL;
	source.pixels = (uint8_t*)pixels;
	source.raster.width = width;
	source.raster.height = height;
	source.raster.pitch = width;
	source.raster.colorInfo.bpp = STD3D_PALETTE_BITS;
	std3D_CopyPaletteToScratch16(palette, STD3D_PALETTE_COLOR_COUNT);
	if (std3D_CreateMipSurface(&source, node, 0, 0) == 0) {
		nullsub_SharedNoOp();
		return NULL;
	}
	return node;
}

// FUNCTION: XW 0x47ECA0
struct Std3DTexCacheNode* RenderTexture_GetOrCreateColorKey(int width, int height, uint16_t* palette,
															const uint8_t* pixels) {
	Std3DTexCacheNode* node;
	Std3DVBuffer source;
	unsigned int sparePaletteIndex;
	int pixelCount;
	int pixelIndex;
	int hasVisiblePixel;
	int savedAlphaTexture;
	node = RenderTexture_FindOrAllocateCacheEntry(pixels + RENDER_TEXTURE_COLOR_KEY_OFFSET);
	if (node->bCached != 0) {
		std3D_CacheTextureSurface(node);
		return node;
	}
	memset(&source, 0, sizeof(source));
	sparePaletteIndex = palette[STD3D_PALETTE_COLOR_COUNT];
	hasVisiblePixel = 0;
#ifdef XW_MODERN
	pixelCount = (int32_t)((uint32_t)width * (uint32_t)height);
#else
	pixelCount = width * height;
#endif
	for (pixelIndex = 0; pixelIndex < pixelCount; ++pixelIndex) {
		uint8_t colorIndex = pixels[pixelIndex];
		if (palette[colorIndex] == 0) {
			g_renderTextureColorKeyScratch[pixelIndex] = 0;
		} else {
			hasVisiblePixel = 1;
			if (colorIndex != 0)
				g_renderTextureColorKeyScratch[pixelIndex] = colorIndex;
			else
				g_renderTextureColorKeyScratch[pixelIndex] = (uint8_t)sparePaletteIndex;
		}
	}
	if (!hasVisiblePixel)
		return NULL;
	source.pixels = g_renderTextureColorKeyScratch;
	source.storageType = STD3D_STORAGE_SOFTWARE;
	source.raster.width = width;
	source.raster.height = height;
	source.raster.pitch = width;
	source.raster.colorInfo.colorMode = STDCOLOR_PAL;
	source.raster.colorInfo.bpp = STD3D_PALETTE_BITS;
	savedAlphaTexture = g_pStd3DCurDevice->caps.bAlphaTexture;
	if (g_pStd3DCurDevice->caps.bColorKeyTexture != 0)
		g_pStd3DCurDevice->caps.bAlphaTexture = 0;
	if (g_pStd3DCurDevice->caps.bAlphaTexture != 0) {
		palette[sparePaletteIndex] = palette[0];
		palette[0] = g_flightTextPalette[g_flightBackgroundPaletteIndex];
		std3D_ConvertTexTo1555(palette, STD3D_PALETTE_COLOR_COUNT);
		palette[0] = palette[sparePaletteIndex];
	} else {
		palette[sparePaletteIndex] = palette[0];
		palette[0] = g_flightTextPalette[g_flightBackgroundPaletteIndex];
		std3D_CopyPaletteToScratch16(palette, STD3D_PALETTE_COLOR_COUNT);
		palette[0] = palette[sparePaletteIndex];
	}
	palette[sparePaletteIndex] = 0;
	if (std3D_CreateMipSurface(&source, node, 1, 0) == 0) {
		nullsub_SharedNoOp();
		if (g_pStd3DCurDevice->caps.bColorKeyTexture != 0)
			g_pStd3DCurDevice->caps.bAlphaTexture = savedAlphaTexture;
		return NULL;
	}
	if (g_pStd3DCurDevice->caps.bColorKeyTexture != 0)
		g_pStd3DCurDevice->caps.bAlphaTexture = savedAlphaTexture;
	return node;
}
