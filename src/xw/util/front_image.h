#ifndef XW_UTIL_FRONT_IMAGE_H
#define XW_UTIL_FRONT_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <aeron/compat/win_types.h>
#include <stddef.h>
#include <stdint.h>

enum {
	FRONT_IMAGE_INDEXED_BITS = 8,
	FRONT_IMAGE_PACKED_BITS = 16,
	FRONT_IMAGE_BMP_BITS = 24,
	FRONT_IMAGE_BMP_SIGNATURE = 0x4D42,
	FRONT_IMAGE_BGR_BYTES = 3,
	FRONT_IMAGE_CHANNEL5_BITS = 5,
	FRONT_IMAGE_CHANNEL6_BITS = 6,
	FRONT_IMAGE_CHANNEL5_SHIFT = 3,
	FRONT_IMAGE_CHANNEL6_SHIFT = 2
};

#pragma pack(push, 1)

typedef struct FrontImageBitmapFileHeader {
	uint16_t type;
	uint32_t size;
	uint16_t reserved1;
	uint16_t reserved2;
	uint32_t pixelOffset;
} FrontImageBitmapFileHeader;

typedef struct FrontImageBitmapInfoHeader {
	uint32_t size;
	int32_t width;
	int32_t height;
	uint16_t planes;
	uint16_t bitCount;
	uint32_t compression;
	uint32_t imageSize;
	int32_t xPixelsPerMeter;
	int32_t yPixelsPerMeter;
	uint32_t colorsUsed;
	uint32_t colorsImportant;
} FrontImageBitmapInfoHeader;

#pragma pack(pop)

typedef char FrontImageBitmapFileHeader_size[(sizeof(FrontImageBitmapFileHeader) == 14) ? 1 : -1];
typedef char FrontImageBitmapInfoHeader_size[(sizeof(FrontImageBitmapInfoHeader) == 40) ? 1 : -1];

/* Declarations follow ascending original IDB address. */

/* 0x47BAB0 */
int FrontImage_SaveBmpFile(const char* fileName, const uint8_t* pixels, int width, int height, int pitchBytes,
						   int bitsPerPixel, int is555, const AeronRgbQuad* palette);

#ifdef __cplusplus
}
#endif

#endif
