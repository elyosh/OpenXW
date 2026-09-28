#ifndef XW_RUNTIME_AUDIO_IMUSE_HOST_H
#define XW_RUNTIME_AUDIO_IMUSE_HOST_H
#include <imuse.h>
extern const ImuseHost g_xwImuseHost;
void XwImuseHost_Render(void* user, int16_t* frames, size_t count);
void XwImuseHost_MusicMarker(int marker, intptr_t a0, intptr_t a1, intptr_t a2, intptr_t a3, intptr_t a4,
							 intptr_t a5, intptr_t a6, intptr_t a7, intptr_t a8, intptr_t a9);
#endif
