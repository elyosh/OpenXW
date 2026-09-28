/* Device lifetime for recorded frontend WAV playback. */
#include "xw_runtime/audio/media_device.h"
#include "xw/audio/direct_sound.h"
#include "xw/audio/frontend_audio.h"
#include "xw/audio/sound.h"

static IDirectSoundBuffer* primary;

bool XwMediaDevice_Init(void* window) {
	if (g_directSound)
		return true;
	if (DirectSoundCreate(NULL, (void**)&g_directSound, NULL) != 0)
		return false;
	DSBufferDesc descriptor = { 0 };
	descriptor.dwSize = sizeof descriptor;
	descriptor.dwFlags = DSBCAPS_PRIMARYBUFFER;
	if (g_directSound->lpVtbl->SetCooperativeLevel(g_directSound, window, DSSCL_PRIORITY) != 0 ||
		g_directSound->lpVtbl->CreateSoundBuffer(g_directSound, &descriptor, &primary, NULL) != 0) {
		XwMediaDevice_Shutdown();
		return false;
	}
	return true;
}

void XwMediaDevice_Shutdown(void) {
	FrontendAudio_StopAndRelease();
	if (primary) {
		primary->lpVtbl->Release(primary);
		primary = NULL;
	}
	if (g_directSound) {
		g_directSound->lpVtbl->Release(g_directSound);
		g_directSound = NULL;
	}
}
