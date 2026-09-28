#include "xw/audio/lolevel.h"

#ifdef XW_MODERN
#include "xw_runtime/audio/imuse_session.h"
#endif

#include "xw/audio/frontend_audio.h"
#include "xw/audio/hilevel.h"
#include "xw/audio/sound.h"
#include "xw/flight/flight_display.h"

#include <stdlib.h>

// FUNCTION: XW 0x485230
int32_t lolevel_ImStopAllSounds(void) {
#ifdef XW_MODERN
	return g_dos94Imuse ? imuse_stop_all_sounds(g_dos94Imuse) : 0;
#else
	int locks;
	int i;
	int result;
	locks = FlightDisplay_GetSurfaceLockCount();
	if (locks) {
		for (i = 0; i < locks; ++i)
			FlightDisplay_UnlockSurface();
	}
	result = Sound_StopAllInstances();
	if (locks) {
		for (i = 0; i < locks; ++i)
			FlightDisplay_LockSurface();
	}
	return result;
#endif
}

// FUNCTION: XW 0x485270
int lolevel_ImResume(void) {
#ifdef XW_MODERN
	if (g_dos94Imuse)
		imuse_resume(g_dos94Imuse);
	FrontendAudio_Resume();
	return 0;
#else
	int locks;
	int i;
	locks = FlightDisplay_GetSurfaceLockCount();
	if (locks) {
		for (i = 0; i < locks; ++i)
			FlightDisplay_UnlockSurface();
	}
	Sound_ResumeActiveInstances();
	if (locks) {
		for (i = 0; i < locks; ++i)
			FlightDisplay_LockSurface();
	}
	FrontendAudio_Resume();
	return 0;
#endif
}

// FUNCTION: XW 0x485300
int lolevel_ImPause(void) {
#ifdef XW_MODERN
	if (g_dos94Imuse)
		imuse_pause(g_dos94Imuse);
	FrontendAudio_Pause();
	return 0;
#else
	int locks;
	int i;
	locks = FlightDisplay_GetSurfaceLockCount();
	if (locks) {
		for (i = 0; i < locks; ++i)
			FlightDisplay_UnlockSurface();
	}
	Sound_PauseActiveInstances();
	FrontendAudio_Pause();
	if (locks) {
		for (i = 0; i < locks; ++i)
			FlightDisplay_LockSurface();
	}
	return 0;
#endif
}

#ifndef XW_MODERN
// FUNCTION: XW 0x4853A0
int32_t lolevel_ImSetParam(const char* soundName, XwNamedSoundParameter parameter, int value) {
	int locks;
	int i;
	int result;
	if (!soundName)
		return 0;
	locks = FlightDisplay_GetSurfaceLockCount();
	if (locks) {
		for (i = 0; i < locks; ++i)
			FlightDisplay_UnlockSurface();
	}
	switch (parameter) {
		case XW_SOUND_PARAM_PRIORITY:
			result = Sound_SetEffectCurrentPriority(soundName, value);
			break;
		case XW_SOUND_PARAM_PAN:
			result = Sound_SetOldestInstancePan(soundName, value);
			break;
		case XW_SOUND_PARAM_VOLUME:
			result = Sound_SetOldestInstanceVolume(soundName, value);
			break;
		case XW_SOUND_PARAM_FREQUENCY_HZ:
			result = Sound_SetOldestInstanceFrequency(soundName, value);
			break;
		default:
			result = 0;
			break;
	}
	if (locks) {
		for (i = 0; i < locks; ++i)
			FlightDisplay_LockSurface();
	}
	return result;
}
#endif

#ifndef XW_MODERN
// FUNCTION: XW 0x485460
int32_t lolevel_ImStopSound(const char* soundName) {
	int locks;
	int i;
	int result;
	if (!soundName)
		return 0;
	locks = FlightDisplay_GetSurfaceLockCount();
	if (locks) {
		for (i = 0; i < locks; ++i)
			FlightDisplay_UnlockSurface();
	}
	result = Sound_StopOldestInstance(soundName);
	if (locks) {
		for (i = 0; i < locks; ++i)
			FlightDisplay_LockSurface();
	}
	return result;
}
#endif

// FUNCTION: XW 0x4854B0
int16_t lolevel_ImSetGroupVol(int16_t group, int16_t volume) {
#ifdef XW_MODERN
	if (group == IMUSE_GROUP_MUSIC && volume >= 0)
		hilevel_SetCdAuxVolume(volume);
	return g_dos94Imuse ? imuse_set_group_volume(g_dos94Imuse, group, volume) : 0;
#else
	switch (group) {
		case XW_SOUND_GROUP_MUSIC:
			hilevel_SetCdAuxVolume(volume);
			break;
		case XW_SOUND_GROUP_ALL:
			hilevel_ImSetSfxVol(volume);
			hilevel_ImSetVoiceVol(volume);
			break;
	}
	return 0;
#endif
}

#ifndef XW_MODERN
// FUNCTION: XW 0x4854F0
int lolevel_ImGetParam(const char* soundName, int16_t parameter) {
	int locks = FlightDisplay_GetSurfaceLockCount();
	int i;
	if (locks) {
		for (i = locks; i > 0; --i)
			FlightDisplay_UnlockSurface();
	}
	if (soundName) {
		switch (parameter) {
			case XW_SOUND_PARAM_INSTANCE_COUNT:
				return Sound_CountPlayingInstances(soundName);
			case XW_SOUND_PARAM_PRIORITY:
				return Sound_GetEffectCurrentPriority(soundName);
		}
	}
	if (locks) {
		for (i = locks; i > 0; --i)
			FlightDisplay_LockSurface();
	}
	return 0;
}
#endif
