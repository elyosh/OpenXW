#ifndef XW_RUNTIME_AUDIO_OUTPUT_H
#define XW_RUNTIME_AUDIO_OUTPUT_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef void (*XwAudioRenderFunc)(void* user, int16_t* frames, size_t count);
bool XwAudioOutput_Start(int sample_rate, int channels, XwAudioRenderFunc render, void* user);
void XwAudioOutput_Stop(void);
#endif
