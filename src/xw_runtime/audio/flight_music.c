/* Mission-owned native music allocations; script behavior lives in DOS94. */
#include "xw_runtime/audio/flight_music.h"
#include "xw_dos94/audio/fmusic.h"
#include "xw_dos94/audio/fscript.h"
#include "xw_dos94/flight/music.h"
#include "xw_runtime/audio/imuse_session.h"
#include "xw_runtime/audio/midi_backend.h"
#include "xw_runtime/audio/music_policy.h"
#include <aeron/log.h>
#include <landru/fourcc.h>
#include <stdlib.h>
#include <string.h>

XwFlightMusicResource g_flightMusicResources[125];

bool XwFlightMusic_Start(void) {
	if (g_musicEventDispatchGate)
		return true;
	if (!XwMusicPolicy_UsesImuse())
		return false;
	XwFlightMusic_Stop();
	uint32_t type = XwMidiBackend_ResourceType();
	const char* path = type == FOURCC_ADLB   ? "RESOURCE/ADLIB.LFD"
					   : type == FOURCC_RLND ? "RESOURCE/ROLAND.LFD"
											 : "RESOURCE/GMIDI.LFD";
	if (Dos94_fmusic_loadmusic(path) != 124) {
		Aeron_LogError("xw.music", "Cannot load complete selected flight catalog");
		XwFlightMusic_Stop();
		return false;
	}
	Dos94_fscript_MsStartScript();
	g_dos94MusicChangeCooldownTicks = g_dos94DynamicMusicLastState = g_dos94DynamicMusicCombatEntered =
		g_dos94DynamicMusicIntensity = 0;
	g_musicEventDispatchGate = 1;
	Dos94_fscript_ChangeState(3);
	return true;
}

void XwFlightMusic_Stop(void) {
	g_musicEventDispatchGate = 0;
	for (int i = 0; i < 125; ++i) {
		if (g_dos94Imuse && g_flightMusicResources[i].data)
			imuse_forget_sound(g_dos94Imuse, 500 + i);
		free(g_flightMusicResources[i].data);
		memset(&g_flightMusicResources[i], 0, sizeof g_flightMusicResources[i]);
	}
	g_dos94MusicTrackCount = 0;
	g_musicCurrentSoundId = g_musicNextSoundId = g_musicSequenceSoundId = g_musicPreviousSoundId = 0;
	g_musicCurrentState = FSCRIPT_INACTIVE_STATE;
}
