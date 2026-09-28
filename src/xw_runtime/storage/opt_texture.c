#include "xw_runtime/storage/opt_texture.h"

#include <string.h>

void XwPort_LoadNativeOptTexture(OptTextureData* destination, const char* fileName) {
	/* Place the file header just before the native payload. The payload stays in place. */
	uint8_t* serialized = (uint8_t*)(destination + 1) - sizeof(OptTextureFileHeader);
	OptTextureFileHeader header;
	OptModel_LoadRgbOrTexFile(serialized, fileName);
	memcpy(&header, serialized, sizeof(header));
	/* The loader emits an inline palette with a color-count marker, not a pointer. */
	destination->palette = NULL;
	destination->paletteType = header.paletteType;
	destination->textureSize = header.textureSize;
	destination->dataSize = header.dataSize;
	destination->width = header.width;
	destination->height = header.height;
}
