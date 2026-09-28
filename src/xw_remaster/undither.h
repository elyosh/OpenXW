#ifndef XW_UNDITHER_H
#define XW_UNDITHER_H
#include "aeron/asset/decode_types.h"
/* Palette-aware RGB filtering. Coverage and the output alpha remain unchanged. */
int XwUndither_Apply(const AeronIndexedFrame* image, const uint8_t palette[256][4], unsigned palette_count,
					 uint8_t* rgba);
#endif
