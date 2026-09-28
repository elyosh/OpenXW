#include "xw_dos94/frontend/sound_preferences.h"
#include "xw/frontend/shell_preferences.h"
#include "xw_dos94/audio/hilevel.h"

// FUNCTION: DOS94 0x10146E
void Dos94_shellext_Set_Prefs_Sound(void) {
	int16_t volume = 0;
	if (g_shellPreferences.musicEnabled && g_shellPreferences.musicVolume)
		volume = 8 * g_shellPreferences.musicVolume - 1;
	Dos94_hilevel_ImSetMusicVol(volume);
	volume = 0;
	if (g_shellPreferences.sfxEnabled && g_shellPreferences.sfxVolume)
		volume = 8 * g_shellPreferences.sfxVolume - 1;
	Dos94_hilevel_ImSetSfxVol(volume);
	Dos94_hilevel_ImSetVoiceVol(volume);
}
