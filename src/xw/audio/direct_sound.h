#ifndef XW_AUDIO_DIRECT_SOUND_H
#define XW_AUDIO_DIRECT_SOUND_H

#ifdef __cplusplus
extern "C" {
#endif

#include <aeron/compat/dsound.h>
#include <aeron/compat/win_types.h>
#include <stddef.h>
#include <stdint.h>

struct IDirectSound;
struct IDirectSoundBuffer;
typedef struct DirectSoundBufferDescV1 DirectSoundBufferDescV1;
typedef struct RiffChunkHeader RiffChunkHeader;
typedef struct RiffWaveHeader RiffWaveHeader;
typedef struct XwVocType1Header XwVocType1Header;

extern void* g_waveFileDataBuffer;

enum {
	XW_VOC_SIGNATURE_PREFIX = 0x61657243,
	XW_VOC_TERMINATOR = 26,
	XW_VOC_DATA_OFFSET = 26,
	XW_VOC_VERSION = 0x010a,
	XW_VOC_CHECKSUM = 0x1129,
	XW_VOC_SOUND_BLOCK = 1,
	XW_VOC_PCM_CODEC = 0,
	XW_VOC_RATE_NUMERATOR = 1000000,
	XW_VOC_RATE_STEPS = 256,
	XW_PCM_FORMAT = 1,
	XW_PCM_MONO = 1,
	XW_PCM_SAMPLE_BITS = 8,
	XW_PCM_DEFAULT_SAMPLE_RATE = 22050
};

enum { XW_STREAM_WAVE_PREFIX_BYTES = 90, XW_STREAM_READ_PENDING = -1 };

enum {
	XW_SOUND_BUFFER_FLAGS = DSBCAPS_STATIC | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME,
	XW_SOUND_BUFFER_DEFAULT_FLAGS = XW_SOUND_BUFFER_FLAGS | DSBCAPS_LOCSOFTWARE | DSBCAPS_CTRLFREQUENCY
};

/* Original IDB size: 20 bytes. */
struct DirectSoundBufferDescV1 {
	/* IDB +0x0 */
	unsigned int dwSize;
	/* IDB +0x4 */
	unsigned int dwFlags;
	/* IDB +0x8 */
	unsigned int dwBufferBytes;
	/* IDB +0xC */
	unsigned int dwReserved;
	/* IDB +0x10 */
	DSWaveFormat* lpwfxFormat;
};

typedef int32_t RiffWaveTag;

enum { RIFF_CHUNK_ALIGNMENT = 2, RIFF_MIN_FORMAT_BYTES = 14 };

enum RiffWaveTagValues {
	RIFF_TAG_RIFF = 0x46464952,
	RIFF_TAG_WAVE = 0x45564157,
	RIFF_TAG_FMT = 0x20746D66,
	RIFF_TAG_DATA = 0x61746164
};

/* Original IDB size: 32 bytes. */
struct XwVocType1Header {
	/* IDB +0x0 */
	char signature[19];
	/* IDB +0x13 */
	uint8_t terminator;
	/* IDB +0x14: Parser requires 26 and assumes first block starts there. */
	uint16_t dataOffset;
	/* IDB +0x16 */
	uint16_t version;
	/* IDB +0x18 */
	uint16_t checksum;
	/* IDB +0x1A: First block type; parser accepts only type 1. */
	uint8_t blockType;
	/* IDB +0x1B: Low byte of the 24-bit block size including timeConstant and codec. */
	uint8_t blockSizeLow;
	/* IDB +0x1C: Upper two bytes of the 24-bit block size. */
	uint16_t blockSizeHigh;
	/* IDB +0x1E: Sample rate = 1000000 / (256 - timeConstant). */
	uint8_t timeConstant;
	/* IDB +0x1F: Codec 0 produces 8-bit PCM; other values yield zero bits/sample without decoding. */
	uint8_t codec;
};

typedef char xw_size_XwVocType1Header[(sizeof(XwVocType1Header) == 32) ? 1 : -1];
typedef char xw_offset_XwVocType1Header_dataOffset[(offsetof(XwVocType1Header, dataOffset) == 20) ? 1 : -1];
typedef char xw_offset_XwVocType1Header_blockType[(offsetof(XwVocType1Header, blockType) == 26) ? 1 : -1];
typedef char
	xw_offset_XwVocType1Header_timeConstant[(offsetof(XwVocType1Header, timeConstant) == 30) ? 1 : -1];

/* Original IDB size: 8 bytes. */
struct RiffChunkHeader {
	/* IDB +0x0: Chunk fourcc; parser handles fmt and data and skips other identifiers. */
	RiffWaveTag chunkId;
	/* IDB +0x4: Payload length excluding this 8-byte header; next chunk follows payload rounded up to an even
	 * byte count. */
	unsigned int payloadBytes;
};

typedef char xw_size_RiffChunkHeader[(sizeof(RiffChunkHeader) == 8) ? 1 : -1];
typedef char xw_offset_RiffChunkHeader_payloadBytes[(offsetof(RiffChunkHeader, payloadBytes) == 4) ? 1 : -1];

/* Original IDB size: 12 bytes. */
struct RiffWaveHeader {
	/* IDB +0x0: Little-endian RIFF fourcc, required to equal RIFF_TAG_RIFF. */
	RiffWaveTag riffId;
	/* IDB +0x4: Declared byte count after the first 8 bytes; includes the four-byte WAVE form tag. Parser end
	 * is base+8+riffBytes; not checked against allocation length. */
	unsigned int riffBytes;
	/* IDB +0x8: Required WAVE form fourcc. */
	RiffWaveTag formType;
};

typedef char xw_size_RiffWaveHeader[(sizeof(RiffWaveHeader) == 12) ? 1 : -1];
typedef char xw_offset_RiffWaveHeader_formType[(offsetof(RiffWaveHeader, formType) == 8) ? 1 : -1];

/* Declarations follow ascending original IDB address. */

/* 0x47C310 */
struct IDirectSoundBuffer* DirectSound_LoadWaveBuffer(struct IDirectSound* directSound, const char* filename,
													  int alternateCapabilities);

/* 0x47C3E0 */
int32_t DirectSound_ReloadWaveBuffer(struct IDirectSoundBuffer* buffer, const char* filename);

/* 0x47C450 */
int DirectSound_LoadFileAndFindAudioData(int unusedContext, const char* filename, DSWaveFormat** outFormat,
										 uint8_t** outData, unsigned int* outDataBytes);

/* 0x47C500 */
int DirectSound_CopyWaveDataToBuffer(struct IDirectSoundBuffer* buffer, const uint8_t* sampleData,
									 unsigned int sampleBytes);

/* 0x47C5C0 */
int32_t DirectSound_FindFormatAndDataChunks(const struct RiffWaveHeader* waveData, DSWaveFormat** outFormat,
											uint8_t** outData, unsigned int* outDataBytes);

/* 0x47C6B0 */
struct IDirectSoundBuffer* DirectSound_LoadVocBuffer(struct IDirectSound* directSound,
													 const char* unusedFilename, int alternateCapabilities,
													 const struct XwVocType1Header* vocData,
													 unsigned int vocDataSize);

/* 0x47C780 */
int32_t DirectSound_AllocAndConvertVoc(int unusedContext, const char* unusedFilename,
									   DSWaveFormat** outFormat, uint8_t** outData,
									   unsigned int* outDataBytes, const struct XwVocType1Header* vocData,
									   int vocDataSize);

/* 0x47C7E0 */
int32_t DirectSound_ConvertVocType1(DSWaveFormat* convertedData, DSWaveFormat** outFormat, uint8_t** outData,
									unsigned int* outDataBytes, const struct XwVocType1Header* vocData,
									unsigned int maxDataBytes);

/* 0x47C930 */
HRESULT DirectSound_CreateStreamBuffer(struct IDirectSoundBuffer** outBuffer, unsigned int bufferBytes,
									   DSWaveFormat* format, int alternateCapabilities);

/* 0x47C9F0 */
void DirectSound_ReleaseBuffer(struct IDirectSoundBuffer** buffer);

/* 0x47CA10 */
void DirectSound_PlayBufferAtOffset(struct IDirectSoundBuffer* buffer, unsigned int startByteOffset, int loop,
									int volume);

/* 0x47CA60 */
HRESULT DirectSound_StopBuffer(struct IDirectSoundBuffer* buffer);

/* 0x47CA70 */
int DirectSound_LockBufferRegion(struct IDirectSoundBuffer* buffer, unsigned int byteOffset,
								 unsigned int byteCount, void** outRegion1, unsigned int* outBytes1,
								 void** outRegion2, unsigned int* outBytes2);

/* 0x47CAB0 */
HRESULT DirectSound_UnlockBufferRegion(struct IDirectSoundBuffer* buffer, void* region1, unsigned int bytes1,
									   void* region2, unsigned int bytes2);

/* 0x47CAE0 */
unsigned int DirectSound_GetPlayCursor(struct IDirectSoundBuffer* buffer);

/* 0x47CB10 */
int DirectSound_CreateBufferFromStreamHeader(struct IDirectSoundBuffer** outBuffer, unsigned int bufferBytes,
											 unsigned int* outDataOffset, unsigned int channel);

/* 0x47CC80 */
struct IDirectSoundBuffer* DirectSound_LoadFileBuffer(struct IDirectSoundBuffer** outBuffer,
													  const char* filename, int alternateCapabilities);

#ifdef __cplusplus
}
#endif

#endif
