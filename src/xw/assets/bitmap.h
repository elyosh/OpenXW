#ifndef XW_ASSETS_BITMAP_H
#define XW_ASSETS_BITMAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

typedef struct XwBitmapFramePrefix XwBitmapFramePrefix;
typedef struct XwBitmapModelPrefix XwBitmapModelPrefix;

enum { XW_BITMAP_PALETTE_RGB24 = 24, XW_BITMAP_PALETTE_RECORD_BYTES = 4 };

/* Serialized 16-byte header preceding the rotated sprite's RLE stream. */
#pragma pack(push, 1)

typedef struct XwBitmapSpriteHeader {
	int16_t originX;
	uint8_t gap02[2];
	int16_t originY;
	uint8_t gap06[2];
	int16_t flippedOriginX;
	uint8_t gap0A[6];
} XwBitmapSpriteHeader;

#pragma pack(pop)

typedef char xw_size_XwBitmapSpriteHeader[(sizeof(XwBitmapSpriteHeader) == 16) ? 1 : -1];
typedef char xw_offset_XwBitmapSpriteHeader_originY[(offsetof(XwBitmapSpriteHeader, originY) == 4) ? 1 : -1];
typedef char
	xw_offset_XwBitmapSpriteHeader_flippedOriginX[(offsetof(XwBitmapSpriteHeader, flippedOriginX) == 8) ? 1
																										: -1];

/* Original IDB size: 44 bytes. */
struct XwBitmapFramePrefix {
	/* IDB +0x0 */
	uint8_t gap00[4];
	/* IDB +0x4 */
	unsigned int sourcePaletteOffset;
	/* IDB +0x8 */
	unsigned int spriteOffset;
	/* IDB +0xC */
	unsigned int paletteOffset;
	/* IDB +0x10 */
	int width;
	/* IDB +0x14 */
	int height;
	/* IDB +0x18 */
	uint8_t gap18[8];
	/* IDB +0x20 */
	int rleFormatIndex;
	/* IDB +0x24 */
	int paletteFormat;
	/* IDB +0x28 */
	int paletteEntryCount;
};

typedef char xw_size_XwBitmapFramePrefix[(sizeof(XwBitmapFramePrefix) == 44) ? 1 : -1];
typedef char
	xw_offset_XwBitmapFramePrefix_spriteOffset[(offsetof(XwBitmapFramePrefix, spriteOffset) == 8) ? 1 : -1];
typedef char
	xw_offset_XwBitmapFramePrefix_rleFormatIndex[(offsetof(XwBitmapFramePrefix, rleFormatIndex) == 32) ? 1
																									   : -1];

/* Original IDB size: 52 bytes. */
struct XwBitmapModelPrefix {
	/* IDB +0x0 */
	unsigned int serializedSize;
	/* IDB +0x4 */
	unsigned int runtimePaletteEntryCount;
	/* IDB +0x8 */
	uint8_t gap08[4];
	/* IDB +0xC */
	unsigned int sourcePaletteOffset;
	/* IDB +0x10 */
	unsigned int frameOffsetsOffset;
	/* IDB +0x14 */
	unsigned int convertedPaletteOffset;
	/* IDB +0x18 */
	unsigned int frameCount;
	/* IDB +0x1C */
	uint8_t gap1C[16];
	/* IDB +0x2C */
	int paletteFormat;
	/* IDB +0x30 */
	unsigned int paletteEntryCount;
};

typedef char xw_size_XwBitmapModelPrefix[(sizeof(XwBitmapModelPrefix) == 52) ? 1 : -1];
typedef char xw_offset_XwBitmapModelPrefix_frameOffsetsOffset
	[(offsetof(XwBitmapModelPrefix, frameOffsetsOffset) == 16) ? 1 : -1];
typedef char xw_offset_XwBitmapModelPrefix_paletteFormat[(offsetof(XwBitmapModelPrefix, paletteFormat) == 44)
															 ? 1
															 : -1];

/* Declarations follow ascending original IDB address. */

#ifdef __cplusplus
}
#endif

#endif
