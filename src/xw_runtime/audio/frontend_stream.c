#include "xw_runtime/audio/frontend_stream.h"

#include "xw/audio/direct_sound.h"
#include "xw/audio/frontend_audio.h"
#include "xw/audio/sound.h"
#include "xw/frontend/shell_preferences.h"

#include <landru/stream.h>
#include <stdbool.h>
#include <string.h>

/* The stream reader supplies a prefix, not the entire RIFF allocation. */
static int find_wave_prefix(const uint8_t* bytes, size_t size, DSWaveFormat* format,
							unsigned int* data_offset, unsigned int* sample_bytes) {
	RiffWaveHeader wave;
	if (size < sizeof wave)
		return 0;
	memcpy(&wave, bytes, sizeof wave);
	if (wave.riffId != RIFF_TAG_RIFF || wave.formType != RIFF_TAG_WAVE || wave.riffBytes < 4)
		return 0;
	uint64_t riff_end = (uint64_t)wave.riffBytes + sizeof(RiffChunkHeader);
	bool have_format = false;
	memset(format, 0, sizeof *format);
	for (size_t offset = sizeof wave; offset <= size && size - offset >= sizeof(RiffChunkHeader);) {
		RiffChunkHeader chunk;
		memcpy(&chunk, bytes + offset, sizeof chunk);
		offset += sizeof chunk;
		if (offset > riff_end || chunk.payloadBytes > riff_end - offset)
			return 0;
		if (chunk.chunkId == RIFF_TAG_DATA) {
			if (!have_format || !chunk.payloadBytes)
				return 0;
			*data_offset = (unsigned int)offset;
			*sample_bytes = (unsigned int)(size - offset);
			if (*sample_bytes > chunk.payloadBytes)
				*sample_bytes = chunk.payloadBytes;
			return 1;
		}
		if (chunk.payloadBytes > size - offset)
			return 0;
		if (chunk.chunkId == RIFF_TAG_FMT) {
			if (chunk.payloadBytes < 16)
				return 0;
			size_t count = chunk.payloadBytes < sizeof *format ? chunk.payloadBytes : sizeof *format;
			memcpy(format, bytes + offset, count);
			have_format = true;
		}
		offset += ((size_t)chunk.payloadBytes + 1) & ~(size_t)1;
	}
	return 0;
}

int XwFrontendAudio_CreateBufferFromStreamHeader(IDirectSoundBuffer** out, unsigned int buffer_bytes,
												 unsigned int* data_offset, unsigned int channel) {
	if (!out)
		return -1;
	*out = NULL;
	LandruHandle header = xmemhdl_Alloc_Handle(XW_STREAM_WAVE_PREFIX_BYTES, 0);
	if (!header)
		return -1;
	int result = -1;
	int read = xstream_Read_From_Stream_Buffer(channel, header, 0, XW_STREAM_WAVE_PREFIX_BYTES, 1);
	if (read == XW_STREAM_WAVE_PREFIX_BYTES) {
		const uint8_t* bytes = xmemhdl_Lock_Handle(header);
		DSWaveFormat format;
		unsigned int offset, samples;
		if (bytes && find_wave_prefix(bytes, (size_t)read, &format, &offset, &samples) &&
			samples <= buffer_bytes) {
			DirectSound_CreateStreamBuffer(out, buffer_bytes, &format, 0);
			if (*out && (!samples || DirectSound_CopyWaveDataToBuffer(*out, bytes + offset, samples))) {
				if (data_offset)
					*data_offset = offset;
				result = (int)samples;
			}
		}
		xmemhdl_Unlock_Handle(header);
	}
	xmemhdl_Free_Handle(header);
	if (result < 0 && *out) {
		(*out)->lpVtbl->Release(*out);
		*out = NULL;
	}
	return result;
}

void XwFrontendAudio_ApplyVolume(void) {
	if (!g_frontendAudioBuffer)
		return;
	int enabled = g_frontendLoopAsSfx ? g_shellPreferences.sfxEnabled : g_shellPreferences.musicEnabled;
	int volume = g_frontendLoopAsSfx ? g_shellPreferences.sfxVolume : g_shellPreferences.musicVolume;
	volume = enabled && volume ? FRONTEND_AUDIO_VOLUME_SCALE * volume - 1 : 0;
	g_frontendAudioBuffer->lpVtbl->SetVolume(g_frontendAudioBuffer, Sound_MapVolumeToAttenuation(volume));
}
