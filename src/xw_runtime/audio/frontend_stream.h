#ifndef XW_RUNTIME_AUDIO_FRONTEND_STREAM_H
#define XW_RUNTIME_AUDIO_FRONTEND_STREAM_H

#include <aeron/compat/dsound.h>

#ifdef __cplusplus
extern "C" {
#endif

int XwFrontendAudio_CreateBufferFromStreamHeader(IDirectSoundBuffer** out, unsigned int buffer_bytes,
												 unsigned int* data_offset, unsigned int channel);
/* Refresh the current stream gain without restarting playback. */
void XwFrontendAudio_ApplyVolume(void);

#ifdef __cplusplus
}
#endif
#endif
