#include "xw/audio/fmusic.h"

// FUNCTION: XW 0x40BD10
unsigned int fmusic_swapdword(unsigned int value) {
	uint16_t highWord = (uint16_t)(value >> 16);
	uint16_t swappedHighWord = (uint16_t)((highWord << 8) | (highWord >> 8));
	return (value << 24) | ((value & 0x0000FF00u) << 8) | swappedHighWord;
}

// FUNCTION: XW 0x40BD40
int fmusic_readfiledata(XwFile* stream, void* dest, unsigned int total) {
#ifdef XW_MODERN
	uint8_t chunkBuffer[FMUSIC_FILE_CHUNK_CAPACITY] = { 0 };
#else
	uint8_t chunkBuffer[FMUSIC_FILE_CHUNK_CAPACITY];
#endif
	uint16_t destOffset = 0;
	int16_t hadReadError = 0;
	while ((uint16_t)total != 0) {
		unsigned int chunkSize = total;
		uint16_t chunkIndex;
		if ((uint16_t)chunkSize > sizeof(chunkBuffer)) {
			chunkSize = sizeof(chunkBuffer);
		}
		hadReadError |=
			(uint16_t)File_RawRead(chunkBuffer, 1, (uint16_t)chunkSize, stream) != (uint16_t)chunkSize;
		for (chunkIndex = 0; chunkIndex < (uint16_t)chunkSize; ++chunkIndex, ++destOffset) {
			((uint8_t*)dest)[destOffset] = chunkBuffer[chunkIndex];
		}
		total -= chunkSize;
	}
	return hadReadError == 0;
}
