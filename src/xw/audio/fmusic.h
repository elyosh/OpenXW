#ifndef XW_AUDIO_FMUSIC_H
#define XW_AUDIO_FMUSIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/assets/file.h"
#include <stddef.h>
#include <stdint.h>

enum { FMUSIC_FILE_CHUNK_CAPACITY = 64 };

/* Declarations follow ascending original IDB address. */

/* 0x40BD10 */
unsigned int fmusic_swapdword(unsigned int value);

/* 0x40BD40 */
int fmusic_readfiledata(XwFile* stream, void* dest, unsigned int total);

#ifdef __cplusplus
}
#endif

#endif
