#include "xw_dos94/audio/hilevel.h"
#include "xw_runtime/audio/imuse_session.h"

// FUNCTION: DOS94 0x119EE
int16_t Dos94_hilevel_ImSetMusicVol(int16_t volume) {
	if (g_dos94Imuse)
		imuse_set_group_volume(g_dos94Imuse, IMUSE_GROUP_MUSIC, volume);
	return 0;
}

// FUNCTION: DOS94 0x11A16
int16_t Dos94_hilevel_ImSetSfxVol(int16_t volume) {
	if (g_dos94Imuse)
		imuse_set_group_volume(g_dos94Imuse, IMUSE_GROUP_SFX, volume);
	return 0;
}

// FUNCTION: DOS94 0x11A3E
int16_t Dos94_hilevel_ImSetVoiceVol(int16_t volume) {
	if (g_dos94Imuse)
		imuse_set_group_volume(g_dos94Imuse, IMUSE_GROUP_VOICE, volume);
	return 0;
}

// FUNCTION: DOS94 0x11A66
int16_t Dos94_hilevel_ImStartSfx(intptr_t soundId) {
	if (!g_dos94Imuse || !soundId || imuse_start_sound(g_dos94Imuse, soundId, 64))
		return -1;
	return imuse_set_param(g_dos94Imuse, soundId, IMUSE_PARAM_SOUND_GROUP, IMUSE_GROUP_SFX) ? -1 : 0;
}

// FUNCTION: DOS94 0x11A98
int16_t Dos94_hilevel_ImStartVoice(intptr_t soundId) {
	if (!g_dos94Imuse || !soundId || imuse_start_sound(g_dos94Imuse, soundId, 64))
		return -1;
	return imuse_set_param(g_dos94Imuse, soundId, IMUSE_PARAM_SOUND_GROUP, IMUSE_GROUP_VOICE) ? -1 : 0;
}

// FUNCTION: DOS94 0x11ACA
int16_t Dos94_hilevel_ImStartMusic(intptr_t soundId) {
	if (!g_dos94Imuse || !soundId || imuse_start_sound(g_dos94Imuse, soundId, 64))
		return -1;
	return imuse_set_param(g_dos94Imuse, soundId, IMUSE_PARAM_SOUND_GROUP, IMUSE_GROUP_MUSIC) ? -1 : 0;
}
