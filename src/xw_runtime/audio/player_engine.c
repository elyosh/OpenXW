#include "xw_runtime/audio/player_engine.h"
#include "xw/audio/fsfx.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/object/create.h"
#include "xw/flight/player/user.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/math/math2.h"
#include "xw/util/memory.h"
#include "xw_runtime/audio/imuse_session.h"
#include "xw_runtime/config/config.h"
#include "xw_runtime/runtime/flight_dispatch.h"
#include "xw_runtime/runtime/flight_types.h"
#include "xw_runtime/runtime/port.h"
#include "xw_runtime/runtime/profile.h"
#include "xw_runtime/storage/file_io.h"
#include <stdio.h>
#include <string.h>

void XwPlayerEngine_Update(void) {
	imuse_t* im = g_dos94Imuse;
	if (!im)
		return;
	int slot = FSFX_ENGINE_NO_SOUND;
	int base = FSFX_ENGINE_BASE_HZ;
	CraftData* craft = NULL;
	if (ShellPreferences_GetSfxEnabled() && g_flightSfxEnabled && g_flightEngineSoundEnabled &&
		g_flightSfxVolume && !g_playerEngineLoopSuppressed &&
		g_playerFlightState.objectIndex != XW_OBJECT_SLOT_UNAVAILABLE &&
		g_playerFlightState.hudSuppressed != 1) {
		ObjectRecord* object = &g_objectTable[g_playerFlightState.objectIndex];
		craft = (CraftData*)object->instanceData;
		if (craft && (craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_ENGINE)) {
			switch (XwFlightTypes_CanonicalType(object->objectType)) {
				case XW_OBJ_X_WING:
				case XW_OBJ_B_WING:
					slot = FSFX_ENGINE_XWING_SLOT;
					base = FSFX_ENGINE_XWING_BASE_HZ;
					break;
				case XW_OBJ_Y_WING:
					slot = FSFX_ENGINE_YWING_SLOT;
					break;
				case XW_OBJ_A_WING:
					slot = FSFX_ENGINE_AWING_SLOT;
					break;
			}
		}
	}
	if (slot != FSFX_ENGINE_NO_SOUND && !g_fsfxLoadedSoundHandles[slot])
		slot = FSFX_ENGINE_NO_SOUND;
	for (int i = FSFX_ENGINE_XWING_SLOT; i < FSFX_SOUND_HANDLE_COUNT; ++i)
		if (i != slot)
			imuse_stop_sound(g_dos94Imuse, i);
	if (slot == FSFX_ENGINE_NO_SOUND || !g_fsfxLoadedSoundHandles[slot])
		return;
	int volume = FSFX_ENGINE_VOLUME_MAX * g_flightSfxVolume / FSFX_ENGINE_VOLUME_SCALE;
	volume = volume * XwConfig_Settings()->player_engine_sound_volume_percent / 100;
	int throttle = math2_percentage(craft->engineThrottle[0], XW_CRAFT_THROTTLE_FULL) /
				   FSFX_ENGINE_THROTTLE_PERCENT_DIVISOR;
	if (!(imuse_get_param(g_dos94Imuse, slot, IMUSE_PARAM_SOUND_PLAY_COUNT) > 0)) {
		if (imuse_start_wave(im, slot, 127, IMUSE_WAVE_START_LOOP))
			return;
		imuse_set_param(im, slot, IMUSE_PARAM_SOUND_GROUP, IMUSE_GROUP_SFX);
	}
	imuse_set_param(im, slot, IMUSE_PARAM_SOUND_FREQUENCY, base + FSFX_ENGINE_HZ_PER_PERCENT * throttle);
	imuse_set_param(im, slot, IMUSE_PARAM_SOUND_VOL, volume);
}
