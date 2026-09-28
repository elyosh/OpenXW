#include "xw/util/front_image.h"

#include "xw/assets/file.h"

#include <string.h>

// FUNCTION: XW 0x47BAB0
int FrontImage_SaveBmpFile(const char* fileName, const uint8_t* pixels, int width, int height, int pitchBytes,
						   int bitsPerPixel, int is555, const AeronRgbQuad* palette) {
	FrontImageBitmapFileHeader fileHeader;
	FrontImageBitmapInfoHeader infoHeader;
	XwFile* output;
	uint32_t fileBytes;
	uint8_t padding;
	uint8_t blue, green, red;
	uint16_t pixel;
	int rowIndex;
	int outputWidth;
	if (bitsPerPixel == FRONT_IMAGE_INDEXED_BITS && palette == NULL)
		return 0;
	output = File_RawOpen(fileName, "wb");
	if (output == NULL)
		return 0;
	padding = 0;
	File_RawSeek(output, sizeof(fileHeader) + sizeof(infoHeader), SEEK_SET);
	fileBytes = sizeof(fileHeader) + sizeof(infoHeader);
	switch (bitsPerPixel) {
		case FRONT_IMAGE_INDEXED_BITS:
			for (rowIndex = height - 1; rowIndex >= 0; --rowIndex) {
				const uint8_t* row = &pixels[rowIndex * pitchBytes];
				int column;
				for (column = 0; column < width; ++column) {
					if (File_RawWrite(&palette[row[column]].rgbBlue, sizeof(uint8_t), 1, output) == 0) {
						File_RawClose(output);
						return 0;
					}
					if (File_RawWrite(&palette[row[column]].rgbGreen, sizeof(uint8_t), 1, output) == 0) {
						File_RawClose(output);
						return 0;
					}
					if (File_RawWrite(&palette[row[column]].rgbRed, sizeof(uint8_t), 1, output) == 0) {
						File_RawClose(output);
						return 0;
					}
					fileBytes += FRONT_IMAGE_BGR_BYTES;
				}
				if (column & 1) {
					if (File_RawWrite(&padding, sizeof(padding), 1, output) == 0) {
						File_RawClose(output);
						return 0;
					}
					if (File_RawWrite(&padding, sizeof(padding), 1, output) == 0) {
						File_RawClose(output);
						return 0;
					}
					if (File_RawWrite(&padding, sizeof(padding), 1, output) == 0) {
						File_RawClose(output);
						return 0;
					}
					fileBytes += FRONT_IMAGE_BGR_BYTES;
				}
			}
			break;
		case FRONT_IMAGE_PACKED_BITS:
			for (rowIndex = height - 1; rowIndex >= 0; --rowIndex) {
				const uint8_t* row = &pixels[rowIndex * pitchBytes];
				int column;
				for (column = 0; column < width; ++column) {
					memcpy(&pixel, &row[column * sizeof(pixel)], sizeof(pixel));
					blue = pixel << FRONT_IMAGE_CHANNEL5_SHIFT;
					pixel >>= FRONT_IMAGE_CHANNEL5_BITS;
					if (is555) {
						green = pixel << FRONT_IMAGE_CHANNEL5_SHIFT;
						pixel >>= FRONT_IMAGE_CHANNEL5_BITS;
					} else {
						green = pixel << FRONT_IMAGE_CHANNEL6_SHIFT;
						pixel >>= FRONT_IMAGE_CHANNEL6_BITS;
					}
					red = pixel << FRONT_IMAGE_CHANNEL5_SHIFT;
					if (File_RawWrite(&blue, sizeof(blue), 1, output) == 0) {
						File_RawClose(output);
						return 0;
					}
					if (File_RawWrite(&green, sizeof(green), 1, output) == 0) {
						File_RawClose(output);
						return 0;
					}
					if (File_RawWrite(&red, sizeof(red), 1, output) == 0) {
						File_RawClose(output);
						return 0;
					}
					fileBytes += FRONT_IMAGE_BGR_BYTES;
				}
				if (column & 1) {
					if (File_RawWrite(&padding, sizeof(padding), 1, output) == 0) {
						File_RawClose(output);
						return 0;
					}
					if (File_RawWrite(&padding, sizeof(padding), 1, output) == 0) {
						File_RawClose(output);
						return 0;
					}
					if (File_RawWrite(&padding, sizeof(padding), 1, output) == 0) {
						File_RawClose(output);
						return 0;
					}
					fileBytes += FRONT_IMAGE_BGR_BYTES;
				}
			}
			break;
	}
	File_RawSeek(output, 0, SEEK_SET);
	fileHeader.type = FRONT_IMAGE_BMP_SIGNATURE;
	fileHeader.size = fileBytes;
	fileHeader.reserved1 = 0;
	fileHeader.reserved2 = 0;
	fileHeader.pixelOffset = sizeof(fileHeader) + sizeof(infoHeader);
	infoHeader.size = sizeof(infoHeader);
	outputWidth = width;
	if (width & 1)
		++outputWidth;
	infoHeader.width = outputWidth;
	infoHeader.height = height;
	infoHeader.planes = 1;
	infoHeader.bitCount = FRONT_IMAGE_BMP_BITS;
	infoHeader.compression = 0;
	infoHeader.imageSize = fileBytes - sizeof(fileHeader) - sizeof(infoHeader);
	infoHeader.xPixelsPerMeter = 0;
	infoHeader.yPixelsPerMeter = 0;
	infoHeader.colorsUsed = 0;
	infoHeader.colorsImportant = 0;
	if (File_RawWrite(&fileHeader, sizeof(fileHeader), 1, output) == 0) {
		File_RawClose(output);
		return 0;
	}
	if (File_RawWrite(&infoHeader, sizeof(infoHeader), 1, output) == 0) {
		File_RawClose(output);
		return 0;
	}
	File_RawClose(output);
	return 1;
}
