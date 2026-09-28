#include "xw_dos94/audio/gamesnd.h"
#include "xw/audio/fsfx.h"
#include "xw/landru_config.h"
#include "xw/util/memory.h"
#include "xw_dos94/audio/fmusic.h"
#include "xw_runtime/audio/imuse_host.h"
#include "xw_runtime/audio/imuse_session.h"
#include "xw_runtime/audio/midi_backend.h"
#include "xw_runtime/audio/output.h"
#include "xw_runtime/config/config.h"
#include "xw_runtime/runtime/port.h"
#include <landru/sound.h>

// GLOBAL: DOS94 0x5B056
static uint16_t g_gameSoundMode;

// FUNCTION: DOS94 0x107F0
int16_t Dos94_gamesnd_Open_Pre_iMuse(void) {
	char error[1024];
	ImuseMidiBackend* backend = XwMidiBackend_Create(error, sizeof error);
	if (!backend && XwMidiBackend_UsesImuse()) {
		XwMidiBackend_ReleaseResources();
		XwPort_Fail(error);
		return 0;
	}
	/* Native output and synthesizer replace the DOS driver initialization block. */
	ImuseConfig config = { .outputSampleRate = 44100,
						   .waveSpeed = 1,
						   .waveMixCount = 4,
						   .xwingTiming = 1,
						   .waveOutputFilter = XwConfig_Settings()->sb16_filter_enabled
												   ? IMUSE_WAVE_OUTPUT_FILTER_SB16
												   : IMUSE_WAVE_OUTPUT_FILTER_NONE };
	g_dos94Imuse = imuse_create(&g_xwImuseHost, &config, backend);
	if (!g_dos94Imuse || !XwAudioOutput_Start(44100, 2, XwImuseHost_Render, g_dos94Imuse)) {
		Dos94_gamesnd_Close_Pre_iMuse();
		XwPort_Fail("Cannot initialize iMUSE audio output");
		return 0;
	}
	imuse_set_music_ducking_factor(g_dos94Imuse, 128);
	return 1;
}

// FUNCTION: DOS94 0x10872
void Dos94_gamesnd_Close_Pre_iMuse(void) {
	XwAudioOutput_Stop();
	if (g_dos94Imuse) {
		imuse_stop_all_sounds(g_dos94Imuse);
		imuse_destroy(g_dos94Imuse);
		g_dos94Imuse = NULL;
	}
	XwMidiBackend_ReleaseResources();
}

// FUNCTION: DOS94 0x108C0
void* Dos94_gamesnd_GetSoundAddr(intptr_t soundId) {
	if (g_gameSoundMode == 0) {
		if (soundId >= 500 && soundId < 625)
			return Dos94_fmusic_GetPagedSound((uint16_t)(soundId - 500));
		if (soundId >= FSFX_FIRST_PLAYABLE_SLOT && soundId < FSFX_SOUND_HANDLE_COUNT)
			return Memory_LockHandle(g_fsfxLoadedSoundHandles[soundId]);
		return NULL;
	}
	if (g_gameSoundMode != 1 || !soundId || soundId == -1)
		return NULL;
	/* Native VOC buffers are stable; no DOS range-copy/EMS window is needed. */
	Sound* sound = (Sound*)soundId;
	void* data = xmemhdl_Lock_Handle(sound->data);
	xmemhdl_Unlock_Handle(sound->data);
	return data;
}

// FUNCTION: DOS94 0x10DE0
void Dos94_gamesnd_game_Set_Front_Sound(void) {
	imuse_pause(g_dos94Imuse);
	g_gameSoundMode = 1;
	imuse_resume(g_dos94Imuse);
}

// FUNCTION: DOS94 0x10E1A
void Dos94_gamesnd_game_Set_Flight_Sound(void) {
	imuse_pause(g_dos94Imuse);
	g_gameSoundMode = 0;
	imuse_resume(g_dos94Imuse);
}
