#include "xw/audio/hilevel.h"

#ifdef XW_MODERN
#include "xw_runtime/audio/imuse_session.h"
#endif

#include "xw/audio/cdaudio.h"
#include "xw/audio/sound.h"
#include "xw/flight/flight_display.h"

#include <stdlib.h>

// GLOBAL: XW 0x55CAF0
int g_musicCdVolumeSetting = 0;

// FUNCTION: XW 0x484F20
int hilevel_ImSetVoiceVol(int volume) {
#ifdef XW_MODERN
	return g_dos94Imuse ? imuse_set_group_volume(g_dos94Imuse, IMUSE_GROUP_VOICE, volume) : 0;
#else
	int locks = FlightDisplay_GetSurfaceLockCount();
	int i;
	if (locks) {
		for (i = 0; i < locks; ++i)
			FlightDisplay_UnlockSurface();
	}
	Sound_SetGroupVolume(HILEVEL_VOICE_GAIN_NUMERATOR * volume / HILEVEL_VOICE_GAIN_DENOMINATOR,
						 SOUND_VOLUME_GROUP_VOICE);
	if (locks) {
		for (i = 0; i < locks; ++i)
			FlightDisplay_LockSurface();
	}
	return 0;
#endif
}

// FUNCTION: XW 0x484F70
int hilevel_ImGetVoiceVol(void) {
#ifdef XW_MODERN
	return g_dos94Imuse ? imuse_set_group_volume(g_dos94Imuse, IMUSE_GROUP_VOICE, -1) : 0;
#else
	return Sound_GetGroupVolume(SOUND_VOLUME_GROUP_VOICE);
#endif
}

// FUNCTION: XW 0x484F80
int hilevel_ImGetSfxVol(void) {
#ifdef XW_MODERN
	return g_dos94Imuse ? imuse_set_group_volume(g_dos94Imuse, IMUSE_GROUP_SFX, -1) : 0;
#else
	return Sound_GetGroupVolume(SOUND_VOLUME_GROUP_SFX);
#endif
}

// FUNCTION: XW 0x484F90
int hilevel_ImGetMusicVol(void) { return g_musicCdVolumeSetting; }

// FUNCTION: XW 0x484FA0
int hilevel_ImSetSfxVol(int volume) {
#ifdef XW_MODERN
	return g_dos94Imuse ? imuse_set_group_volume(g_dos94Imuse, IMUSE_GROUP_SFX, volume) : 0;
#else
	int locks = FlightDisplay_GetSurfaceLockCount();
	int i;
	if (locks) {
		for (i = 0; i < locks; ++i)
			FlightDisplay_UnlockSurface();
	}
	Sound_SetGroupVolume(volume, SOUND_VOLUME_GROUP_SFX);
	if (locks) {
		for (i = 0; i < locks; ++i)
			FlightDisplay_LockSurface();
	}
	return 0;
#endif
}

// FUNCTION: XW 0x484FE0
int hilevel_SetCdAuxVolume(int volume) {
	g_musicCdVolumeSetting = volume;
#ifdef XW_MODERN
	CDAudio_SetAuxVolume((int32_t)((uint32_t)volume * CDAUDIO_AUX_VOLUME_MAX) / HILEVEL_CD_VOLUME_SCALE);
#else
	CDAudio_SetAuxVolume(CDAUDIO_AUX_VOLUME_MAX * volume / HILEVEL_CD_VOLUME_SCALE);
#endif
	return 0;
}

// FUNCTION: XW 0x4852B0
int hilevel_ImSetMasterVol(int volume) {
#ifdef XW_MODERN
	return g_dos94Imuse ? imuse_set_group_volume(g_dos94Imuse, IMUSE_GROUP_MASTER, volume) : 0;
#else
	int locks;
	int i;
	locks = FlightDisplay_GetSurfaceLockCount();
	if (locks) {
		for (i = 0; i < locks; ++i)
			FlightDisplay_UnlockSurface();
	}
	Sound_SetGroupVolume(volume, SOUND_VOLUME_GROUP_MASTER);
	if (locks) {
		for (i = 0; i < locks; ++i)
			FlightDisplay_LockSurface();
	}
	return 0;
#endif
}

// FUNCTION: XW 0x4852F0
int hilevel_ImGetMasterVol(void) {
#ifdef XW_MODERN
	return g_dos94Imuse ? imuse_set_group_volume(g_dos94Imuse, IMUSE_GROUP_MASTER, -1) : 0;
#else
	return Sound_GetGroupVolume(SOUND_VOLUME_GROUP_MASTER);
#endif
}

#ifndef XW_MODERN
// FUNCTION: XW 0x485340
int32_t hilevel_ImStartSfx(const char* soundName, int priority) {
	if (!soundName)
		return 0;
	return Sound_QueueEffect(soundName, 1, 0, priority, SOUND_VOLUME_MAX, SOUND_QUEUE_PAN_CENTER,
							 SOUND_GROUP_SFX);
}
#endif

#ifndef XW_MODERN
// FUNCTION: XW 0x485370
int32_t hilevel_ImStartVoice(const char* soundName, int priority) {
	if (!soundName)
		return 0;
	return Sound_QueueEffect(soundName, 1, 0, priority, SOUND_VOLUME_MAX, SOUND_QUEUE_PAN_CENTER,
							 SOUND_GROUP_VOICE);
}
#endif
