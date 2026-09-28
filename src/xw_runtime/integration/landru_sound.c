#include "xw_runtime/integration/landru_sound.h"
#include "xw_runtime/audio/frontend_music.h"
#include "xw_runtime/audio/imuse_session.h"
#include "xw_runtime/audio/midi_backend.h"

#include "xw/audio/frontend_audio.h"
#include "xw/flight/flight_display.h"
#include "xw/frontend/scenes/rescue64.h"
#include <aeron/aeron.h>

static uint64_t audio_frame = UINT64_MAX;

static void pump_audio(void* userdata) {
	(void)userdata;
	const AeronInputSnapshot* input = Aeron_InputSnapshot();
	if (input && audio_frame == input->frame_id)
		return;
	if (input)
		audio_frame = input->frame_id;
	int locks = FlightDisplay_GetSurfaceLockCount();
	for (int i = 0; i < locks; ++i)
		FlightDisplay_UnlockSurface();
	FrontendAudio_UpdatePlayback();
	for (int i = 0; i < locks; i++)
		FlightDisplay_LockSurface();
}

static uint32_t music_type(void* user) {
	(void)user;
	return XwMidiBackend_ResourceType();
}

static Sound* music_load(void* user, ResFile* resource, const char* name) {
	(void)user;
	return XwFrontendMusic_Load(resource, name);
}

static void music_release(void* user, Sound* sound) {
	(void)user;
	XwFrontendMusic_Release(sound);
}

void XwLandru_ConfigureSoundHost(LandruHost* host) {
	audio_frame = UINT64_MAX;
	host->frontend_audio_pump = pump_audio;
	host->music_type = music_type;
	host->music_load = music_load;
	host->music_release = music_release;
}

void XwLandru_RescueSpeechCallback(Sound* sound, int time) {
	(void)sound;
	(void)time;
	(void)j_shellext_Get_Cur_Scene();
}

void XwLandru_ServiceAudio(void) { pump_audio(NULL); }
