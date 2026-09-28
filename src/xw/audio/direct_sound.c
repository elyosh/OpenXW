#include "xw/audio/direct_sound.h"

#ifdef XW_MODERN
#include "xw_runtime/audio/frontend_stream.h"
#endif

#include "xw/audio/sound.h"
#include "xw/flight/fediskio.h"
#include "xw/landru_config.h"

#include <landru/file.h>
#include <landru/stream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4FC8C8
void* g_waveFileDataBuffer = NULL;

// FUNCTION: XW 0x47C310
struct IDirectSoundBuffer* DirectSound_LoadWaveBuffer(struct IDirectSound* directSound, const char* filename,
													  int alternateCapabilities) {
	IDirectSoundBuffer* buffer = NULL;
	DSBufferDesc descriptor = { 0 };
	uint8_t* samples;
	g_waveFileDataBuffer = NULL;
	if (DirectSound_LoadFileAndFindAudioData(0, filename, &descriptor.lpwfxFormat, &samples,
											 &descriptor.dwBufferBytes)) {
		descriptor.dwSize = sizeof(descriptor);
		descriptor.dwFlags = alternateCapabilities ? XW_SOUND_BUFFER_FLAGS : XW_SOUND_BUFFER_DEFAULT_FLAGS;
		if (directSound->lpVtbl->CreateSoundBuffer(directSound, &descriptor, &buffer, NULL) >= 0) {
			if (!DirectSound_CopyWaveDataToBuffer(buffer, samples, descriptor.dwBufferBytes)) {
				buffer->lpVtbl->Release(buffer);
				buffer = NULL;
			}
		} else
			buffer = NULL;
	}
	if (g_waveFileDataBuffer) {
		free(g_waveFileDataBuffer);
#ifdef XW_MODERN
		g_waveFileDataBuffer = NULL;
#endif
	}
	return buffer;
}

// FUNCTION: XW 0x47C3E0
int32_t DirectSound_ReloadWaveBuffer(struct IDirectSoundBuffer* buffer, const char* filename) {
	int reloadSucceeded = 0;
	uint8_t* sampleData;
	unsigned int sampleBytes;
	g_waveFileDataBuffer = NULL;
	if (DirectSound_LoadFileAndFindAudioData(0, filename, NULL, &sampleData, &sampleBytes) &&
		buffer->lpVtbl->Restore(buffer) >= 0) {
		if (DirectSound_CopyWaveDataToBuffer(buffer, sampleData, sampleBytes))
			reloadSucceeded = 1;
	}
	if (g_waveFileDataBuffer) {
		free(g_waveFileDataBuffer);
#ifdef XW_MODERN
		g_waveFileDataBuffer = NULL;
#endif
	}
	return reloadSucceeded;
}

// FUNCTION: XW 0x47C450
int DirectSound_LoadFileAndFindAudioData(int unusedContext, const char* filename, DSWaveFormat** outFormat,
										 uint8_t** outData, unsigned int* outDataBytes) {
	LandruFile* file = xfile_Open_File(LANDRU_FILE_ROOT_ASSET, filename, "rb");
	(void)unusedContext;
	if (file) {
		int fileBytes;
		RiffWaveHeader* waveFileData;
		xfile_Seek_File(file, 0, SEEK_END);
		fileBytes = xfile_Tell_File(file);
		xfile_Seek_File(file, 0, SEEK_SET);
#ifdef XW_MODERN
		if (fileBytes < (int)sizeof(*waveFileData)) {
			xfile_Close_File(file);
			return 0;
		}
#endif
		g_waveFileDataBuffer = malloc(fileBytes);
		waveFileData = (RiffWaveHeader*)g_waveFileDataBuffer;
		if (waveFileData) {
			if (xfile_Read_Data_From_File(file, waveFileData, fileBytes)) {
#ifdef XW_MODERN
				if (waveFileData->riffBytes > (size_t)fileBytes - sizeof(RiffChunkHeader)) {
					xfile_Close_File(file);
					return 0;
				}
#endif
				if (DirectSound_FindFormatAndDataChunks(waveFileData, outFormat, outData, outDataBytes)) {
#ifdef XW_MODERN
					if (outFormat && *outFormat &&
						(size_t)((uint8_t*)*outFormat - (uint8_t*)waveFileData) + sizeof(DSWaveFormat) -
								sizeof((*outFormat)->cbSize) >
							(size_t)fileBytes) {
						xfile_Close_File(file);
						return 0;
					}
#endif
					xfile_Close_File(file);
					return 1;
				}
			}
		} else
			fediskio_fatalerror(FEDISKIO_ERROR_ALLOCATION);
		xfile_Close_File(file);
	}
	return 0;
}

// FUNCTION: XW 0x47C500
int DirectSound_CopyWaveDataToBuffer(struct IDirectSoundBuffer* buffer, const uint8_t* sampleData,
									 unsigned int sampleBytes) {
	void* firstRegion;
	void* secondRegion;
	uint32_t firstBytes;
	uint32_t secondBytes;
	if (!buffer || !sampleData || !sampleBytes)
		return 0;
	if (buffer->lpVtbl->Lock(buffer, 0, sampleBytes, &firstRegion, &firstBytes, &secondRegion, &secondBytes,
							 0) < 0)
		return 0;
	memcpy(firstRegion, sampleData, firstBytes);
	if (secondBytes)
		memcpy(secondRegion, sampleData + firstBytes, secondBytes);
	buffer->lpVtbl->Unlock(buffer, firstRegion, firstBytes, secondRegion, secondBytes);
	return 1;
}

// FUNCTION: XW 0x47C5C0
int32_t DirectSound_FindFormatAndDataChunks(const struct RiffWaveHeader* waveData, DSWaveFormat** outFormat,
											uint8_t** outData, unsigned int* outDataBytes) {
	size_t offset;
	size_t endOffset;
	unsigned int payloadBytes;
	if (outFormat)
		*outFormat = NULL;
	if (outData)
		*outData = NULL;
	if (outDataBytes)
		*outDataBytes = 0;
#ifdef XW_MODERN
	if (!waveData || waveData->riffBytes < sizeof(waveData->formType))
		return 0;
#endif
	endOffset = (size_t)waveData->riffBytes + sizeof(RiffChunkHeader);
	if (waveData->riffId != RIFF_TAG_RIFF || waveData->formType != RIFF_TAG_WAVE)
		return 0;
	for (offset = sizeof(*waveData); offset < endOffset;
		 offset += (payloadBytes + 1u) & ~(RIFF_CHUNK_ALIGNMENT - 1u)) {
		RiffChunkHeader chunk;
		uint8_t* payload;
#ifdef XW_MODERN
		if (endOffset - offset < sizeof(chunk))
			return 0;
#endif
		memcpy(&chunk, (const uint8_t*)waveData + offset, sizeof(chunk));
		payloadBytes = chunk.payloadBytes;
		offset += sizeof(chunk);
		payload = (uint8_t*)waveData + offset;
#ifdef XW_MODERN
		if (payloadBytes > endOffset - offset)
			return 0;
#endif
		if (chunk.chunkId != RIFF_TAG_FMT) {
			if (chunk.chunkId == RIFF_TAG_DATA &&
				((outData && !*outData) || (outDataBytes && !*outDataBytes))) {
				if (outData)
					*outData = payload;
				if (outDataBytes)
					*outDataBytes = payloadBytes;
				if (!outFormat || *outFormat)
					return 1;
			}
		} else {
			if (outFormat && !*outFormat) {
				if (payloadBytes < RIFF_MIN_FORMAT_BYTES)
					return 0;
				*outFormat = (DSWaveFormat*)payload;
				if ((!outData || *outData) && (!outDataBytes || *outDataBytes))
					return 1;
			}
		}
	}
	return 0;
}

// FUNCTION: XW 0x47C6B0
struct IDirectSoundBuffer* DirectSound_LoadVocBuffer(struct IDirectSound* directSound,
													 const char* unusedFilename, int alternateCapabilities,
													 const struct XwVocType1Header* vocData,
													 unsigned int vocDataSize) {
	IDirectSoundBuffer* buffer = NULL;
	DSBufferDesc descriptor;
	uint8_t* samples;
	memset(&descriptor, 0, sizeof(descriptor));
#ifdef XW_MODERN
	g_waveFileDataBuffer = NULL;
#endif
	if (DirectSound_AllocAndConvertVoc(0, unusedFilename, &descriptor.lpwfxFormat, &samples,
									   &descriptor.dwBufferBytes, vocData, vocDataSize)) {
		descriptor.dwSize = sizeof(descriptor);
		descriptor.dwFlags = alternateCapabilities ? XW_SOUND_BUFFER_FLAGS : XW_SOUND_BUFFER_DEFAULT_FLAGS;
		if (directSound->lpVtbl->CreateSoundBuffer(directSound, &descriptor, &buffer, NULL) >= 0) {
			if (!DirectSound_CopyWaveDataToBuffer(buffer, samples, descriptor.dwBufferBytes)) {
				buffer->lpVtbl->Release(buffer);
				buffer = NULL;
			}
		} else
			buffer = NULL;
	}
	if (g_waveFileDataBuffer) {
		free(g_waveFileDataBuffer);
#ifdef XW_MODERN
		g_waveFileDataBuffer = NULL;
#endif
	}
	return buffer;
}

// FUNCTION: XW 0x47C780
int32_t DirectSound_AllocAndConvertVoc(int unusedContext, const char* unusedFilename,
									   DSWaveFormat** outFormat, uint8_t** outData,
									   unsigned int* outDataBytes, const struct XwVocType1Header* vocData,
									   int vocDataSize) {
	(void)unusedContext;
	(void)unusedFilename;
	if (vocDataSize > 0 && vocData) {
		DSWaveFormat* convertedData = (DSWaveFormat*)malloc(vocDataSize + sizeof(DSWaveFormat));
		g_waveFileDataBuffer = convertedData;
		if (convertedData) {
			if (DirectSound_ConvertVocType1(convertedData, outFormat, outData, outDataBytes, vocData,
											vocDataSize))
				return 1;
		} else
			fediskio_fatalerror(FEDISKIO_ERROR_ALLOCATION);
	}
	return 0;
}

// FUNCTION: XW 0x47C7E0
int32_t DirectSound_ConvertVocType1(DSWaveFormat* convertedData, DSWaveFormat** outFormat, uint8_t** outData,
									unsigned int* outDataBytes, const struct XwVocType1Header* vocData,
									unsigned int maxDataBytes) {
	unsigned int sampleBytes;
	unsigned int sampleRate;
	uint32_t signature;
	int codec;
	uint8_t* samples;
	if (outFormat)
		*outFormat = convertedData;
	if (outData)
		*outData = NULL;
	if (outDataBytes)
		*outDataBytes = 0;
#ifdef XW_MODERN
	if (!vocData || !convertedData || maxDataBytes < sizeof(*vocData))
		return 0;
#endif
	memcpy(&signature, vocData->signature, sizeof(signature));
	if (signature != XW_VOC_SIGNATURE_PREFIX || vocData->terminator != XW_VOC_TERMINATOR ||
		vocData->dataOffset != XW_VOC_DATA_OFFSET || vocData->version != XW_VOC_VERSION ||
		vocData->checksum != XW_VOC_CHECKSUM || vocData->blockType != XW_VOC_SOUND_BLOCK)
		return 0;
	sampleBytes = vocData->blockSizeLow | ((unsigned int)vocData->blockSizeHigh << XW_PCM_SAMPLE_BITS);
#ifdef XW_MODERN
	if (sampleBytes < sizeof(vocData->timeConstant) + sizeof(vocData->codec))
		return 0;
	maxDataBytes -= sizeof(*vocData);
#endif
	sampleBytes -= sizeof(vocData->timeConstant) + sizeof(vocData->codec);
	sampleRate = XW_VOC_RATE_NUMERATOR / (XW_VOC_RATE_STEPS - vocData->timeConstant);
	codec = vocData->codec;
	if (sampleBytes >= maxDataBytes)
		sampleBytes = maxDataBytes;
	samples = (uint8_t*)(convertedData + 1);
	if (sampleBytes)
		memcpy(samples, vocData + 1, sampleBytes);
	convertedData->wFormatTag = XW_PCM_FORMAT;
	convertedData->nChannels = XW_PCM_MONO;
	convertedData->nSamplesPerSec = sampleRate;
	convertedData->nAvgBytesPerSec = sampleRate;
	convertedData->nBlockAlign = sizeof(uint8_t);
	convertedData->wBitsPerSample = codec == XW_VOC_PCM_CODEC ? XW_PCM_SAMPLE_BITS : 0;
	convertedData->cbSize = 0;
	if (outData)
		*outData = samples;
	if (outDataBytes)
		*outDataBytes = sampleBytes;
	return 1;
}

// FUNCTION: XW 0x47C930
HRESULT DirectSound_CreateStreamBuffer(struct IDirectSoundBuffer** outBuffer, unsigned int bufferBytes,
									   DSWaveFormat* format, int alternateCapabilities) {
	DSBufferDesc descriptor;
	DSWaveFormat defaultFormat;
	HRESULT result;
	memset(&descriptor, 0, sizeof(descriptor));
	descriptor.dwSize = sizeof(descriptor);
	descriptor.dwFlags = alternateCapabilities ? XW_SOUND_BUFFER_FLAGS : XW_SOUND_BUFFER_DEFAULT_FLAGS;
	descriptor.dwBufferBytes = bufferBytes;
	if (format == NULL) {
		memset(&defaultFormat, 0, sizeof(defaultFormat) - sizeof(defaultFormat.cbSize));
		defaultFormat.wFormatTag = XW_PCM_FORMAT;
		defaultFormat.nChannels = XW_PCM_MONO;
		defaultFormat.nSamplesPerSec = XW_PCM_DEFAULT_SAMPLE_RATE;
		defaultFormat.nAvgBytesPerSec = XW_PCM_DEFAULT_SAMPLE_RATE;
		defaultFormat.wBitsPerSample = XW_PCM_SAMPLE_BITS;
		defaultFormat.nBlockAlign = sizeof(uint8_t);
#ifdef XW_MODERN
		defaultFormat.cbSize = 0;
#endif
		descriptor.lpwfxFormat = &defaultFormat;
	} else {
		descriptor.lpwfxFormat = format;
	}
	result = g_directSound->lpVtbl->CreateSoundBuffer(g_directSound, &descriptor, outBuffer, NULL);
	if (result < 0) {
		*outBuffer = NULL;
	}
	return result;
}

// FUNCTION: XW 0x47C9F0
void DirectSound_ReleaseBuffer(struct IDirectSoundBuffer** buffer) {
	if (buffer != NULL && *buffer != NULL) {
		(*buffer)->lpVtbl->Release(*buffer);
		*buffer = NULL;
	}
}

// FUNCTION: XW 0x47CA10
void DirectSound_PlayBufferAtOffset(struct IDirectSoundBuffer* buffer, unsigned int startByteOffset, int loop,
									int volume) {
	if (buffer) {
		int attenuation;
		buffer->lpVtbl->SetCurrentPosition(buffer, startByteOffset);
		attenuation = Sound_MapVolumeToAttenuation(volume);
		buffer->lpVtbl->SetVolume(buffer, attenuation);
		buffer->lpVtbl->SetPan(buffer, 0);
		if (loop)
			loop = DSBPLAY_LOOPING;
		buffer->lpVtbl->Play(buffer, 0, 0, loop);
	}
}

// FUNCTION: XW 0x47CA60
HRESULT DirectSound_StopBuffer(struct IDirectSoundBuffer* buffer) {
	HRESULT result = DX_S_OK;

	if (buffer != NULL) {
		result = buffer->lpVtbl->Stop(buffer);
	}
	return result;
}

// FUNCTION: XW 0x47CA70
int DirectSound_LockBufferRegion(struct IDirectSoundBuffer* buffer, unsigned int byteOffset,
								 unsigned int byteCount, void** outRegion1, unsigned int* outBytes1,
								 void** outRegion2, unsigned int* outBytes2) {
	return buffer != NULL && buffer->lpVtbl->Lock(buffer, byteOffset, byteCount, outRegion1, outBytes1,
												  outRegion2, outBytes2, 0) >= 0;
}

// FUNCTION: XW 0x47CAB0
HRESULT DirectSound_UnlockBufferRegion(struct IDirectSoundBuffer* buffer, void* region1, unsigned int bytes1,
									   void* region2, unsigned int bytes2) {
	HRESULT result = DX_S_OK;
	if (buffer != NULL) {
		result = buffer->lpVtbl->Unlock(buffer, region1, bytes1, region2, bytes2);
	}
	return result;
}

// FUNCTION: XW 0x47CAE0
unsigned int DirectSound_GetPlayCursor(struct IDirectSoundBuffer* buffer) {
	uint32_t playCursor = 0;
	uint32_t writeCursor;
	if (buffer != NULL) {
		buffer->lpVtbl->GetCurrentPosition(buffer, &playCursor, &writeCursor);
	}
	return playCursor;
}

// FUNCTION: XW 0x47CB10
int DirectSound_CreateBufferFromStreamHeader(struct IDirectSoundBuffer** outBuffer, unsigned int bufferBytes,
											 unsigned int* outDataOffset, unsigned int channel) {
#ifdef XW_MODERN
	return XwFrontendAudio_CreateBufferFromStreamHeader(outBuffer, bufferBytes, outDataOffset, channel);
#else
	LandruHandle headerHandle;
	int bytesRead;
	int result;
	if (outBuffer == NULL)
		return -1;
	headerHandle = xmemhdl_Alloc_Handle(XW_STREAM_WAVE_PREFIX_BYTES, 0);
	do {
		bytesRead = xstream_Read_From_Stream_Buffer(channel, headerHandle, 0, XW_STREAM_WAVE_PREFIX_BYTES, 1);
	} while (bytesRead == XW_STREAM_READ_PENDING);
	result = -1;
	if (bytesRead == XW_STREAM_WAVE_PREFIX_BYTES) {
		uint8_t* headerBytes = xmemhdl_Lock_Handle(headerHandle);
		uint8_t* sampleData;
		DSWaveFormat* waveFormat;
		unsigned int waveDataBytes;
		unsigned int prefixSampleBytes;
		DirectSound_FindFormatAndDataChunks((const RiffWaveHeader*)headerBytes, &waveFormat, &sampleData,
											&waveDataBytes);
		DirectSound_CreateStreamBuffer(outBuffer, bufferBytes, waveFormat, 0);
		if (*outBuffer == NULL) {
			xmemhdl_Unlock_Handle(headerHandle);
			xmemhdl_Free_Handle(headerHandle);
			return -1;
		}
		if (outDataOffset != NULL)
			*outDataOffset = (unsigned int)(sampleData - headerBytes);
		prefixSampleBytes = (unsigned int)(headerBytes - sampleData) + XW_STREAM_WAVE_PREFIX_BYTES;
		result = prefixSampleBytes;
		if (prefixSampleBytes != 0) {
			void* firstRegion;
			void* secondRegion;
			uint32_t firstRegionBytes;
			uint32_t secondRegionBytes;
			if ((*outBuffer)
					->lpVtbl->Lock(*outBuffer, 0, prefixSampleBytes, &firstRegion, &firstRegionBytes,
								   &secondRegion, &secondRegionBytes, 0) >= 0) {
				unsigned int copyBytes = firstRegionBytes;
				unsigned int unlockBytes;
				if (copyBytes >= prefixSampleBytes)
					copyBytes = prefixSampleBytes;
				/* The original copies sound-buffer bytes into the temporary WAV prefix. */
				memcpy(sampleData, firstRegion, copyBytes);
				unlockBytes = firstRegionBytes;
				if (unlockBytes >= prefixSampleBytes)
					unlockBytes = prefixSampleBytes;
				(*outBuffer)->lpVtbl->Unlock(*outBuffer, firstRegion, unlockBytes, secondRegion, 0);
			} else {
				result = -1;
			}
		}
		xmemhdl_Unlock_Handle(headerHandle);
	}
	xmemhdl_Free_Handle(headerHandle);
	return result;
#endif
}

// FUNCTION: XW 0x47CC80
struct IDirectSoundBuffer* DirectSound_LoadFileBuffer(struct IDirectSoundBuffer** outBuffer,
													  const char* filename, int alternateCapabilities) {
	struct IDirectSoundBuffer* buffer =
		DirectSound_LoadWaveBuffer(g_directSound, filename, alternateCapabilities);
	*outBuffer = buffer;
	return buffer;
}
