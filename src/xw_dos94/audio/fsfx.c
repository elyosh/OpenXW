/* DOS94 numeric-ID effects policy; shared simulation supplies geometry. */
#include "xw_dos94/audio/fsfx.h"
#include "xw/audio/fsfx.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/object/create.h"
#include "xw/flight/player/user.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/util/memory.h"
#include "xw_dos94/audio/hilevel.h"
#include "xw_runtime/audio/imuse_session.h"
#include "xw_runtime/runtime/flight_dispatch.h"
#include "xw_runtime/runtime/flight_types.h"
#include "xw_runtime/runtime/port.h"
#include "xw_runtime/runtime/profile.h"
#include "xw_runtime/storage/file_io.h"
#include <stdio.h>
#include <string.h>

// FUNCTION: DOS94 0x6965E0
uint16_t Dos94_fsfx_loadsfx(const char* path) {
	XwFlightMode_UnloadSounds();
	XwGameVersion version = XwProfile_ActiveFlight()->version;
	if (version == XW_GAME_VERSION_93)
		version = XwProfile_Frontend()->version;
	char expanded[XW_PATH_CAPACITY];
	/* Preserve fediskio's game-data prefix when opening a specific installation. */
	if (*path == ';' || *path == '+') {
		int length = snprintf(expanded, sizeof expanded, "X-Wing Data/%s", path + 1);
		if (length < 0 || (size_t)length >= sizeof expanded)
			goto failed;
		path = expanded;
	}
	unsigned loaded = 0;
	unsigned banks = XwStorage_HasInstallation(XW_GAME_VERSION_98) ? 2 : 1;
	for (unsigned bank = 0; bank < banks; ++bank) {
		if (bank)
			version = XW_GAME_VERSION_98;
		unsigned first = bank ? FSFX_ENGINE_XWING_SLOT - FSFX_FIRST_PLAYABLE_SLOT : 0;
		unsigned end = bank ? FSFX_SOUND_HANDLE_COUNT - FSFX_FIRST_PLAYABLE_SLOT
							: FSFX_ENGINE_XWING_SLOT - FSFX_FIRST_PLAYABLE_SLOT;
		AeronFile* file = XwStorage_OpenInstallation(version, path);
		if (!file)
			goto failed;
		uint8_t header[16];
		int32_t length = XwFile_Length(file);
		bool ok = length >= 16 && XwFile_Read(header, 1, sizeof header, file) == sizeof header &&
				  !memcmp(header, "RMAP", 4);
		uint32_t map_size = ok ? (header[12] | (uint32_t)header[13] << 8 | (uint32_t)header[14] << 16 |
								  (uint32_t)header[15] << 24)
							   : 0;
		ok = ok && !(map_size % 16) && map_size / 16 >= end &&
			 map_size / 16 <= FSFX_SOUND_HANDLE_COUNT - FSFX_FIRST_PLAYABLE_SLOT &&
			 map_size <= (uint32_t)length - 16 && !XwFile_Seek(file, (int32_t)map_size, SEEK_CUR);
		for (unsigned index = 0; ok && index < end; ++index) {
			ok = XwFile_Read(header, 1, sizeof header, file) == sizeof header;
			int32_t position = XwFile_Tell(file);
			uint32_t size = ok ? (header[12] | (uint32_t)header[13] << 8 | (uint32_t)header[14] << 16 |
								  (uint32_t)header[15] << 24)
							   : 0;
			ok = ok && position >= 0 && position <= length && size && size <= (uint32_t)(length - position) &&
				 (!memcmp(header, "BLAS", 4) || !memcmp(header, "VOIC", 4));
			if (!ok)
				break;
			if (index < first) {
				ok = !XwFile_Seek(file, (int32_t)size, SEEK_CUR);
				continue;
			}
			unsigned slot = FSFX_FIRST_PLAYABLE_SLOT + index;
			if (bank) {
				static const char engines[3][8] = { "xwenglp", "awenglp", "ywenglp" };
				if (memcmp(header + 4, engines[slot - FSFX_ENGINE_XWING_SLOT], 8)) {
					ok = false;
					break;
				}
			}
			uint16_t handle = Memory_AllocHandle(size, 0);
			void* data = handle ? Memory_LockHandle(handle) : NULL;
			if (!data || XwFile_Read(data, 1, size, file) != size) {
				if (handle)
					Memory_FreeHandle(handle);
				ok = false;
				break;
			}
			g_fsfxLoadedSoundHandles[slot] = handle;
			++loaded;
		}
		if (XwFile_Close(file))
			ok = false;
		if (!ok)
			goto failed;
	}
	g_fsfxLoaded = 1;
	return (uint16_t)loaded;
failed: {
	char error[256];
	snprintf(error, sizeof error, "Cannot load X-Wing %d flight sounds from %s.", XwGameVersion_Year(version),
			 path);
	XwFlightMode_UnloadSounds();
	XwPort_Fail(error);
	return 0;
}
}

// FUNCTION: DOS94 0x69674E
int16_t Dos94_fsfx_triggersfx(uint16_t soundId, uint16_t objectIndex) {
	uint16_t volume;
	uint16_t priority;
	int16_t pan;
	uint8_t objectType;
	if (!ShellPreferences_GetSfxEnabled() || !g_dos94Imuse || soundId >= FSFX_SOUND_HANDLE_COUNT)
		return 0;
	if (!g_fsfxLoadedSoundHandles[soundId] || !g_flightSfxEnabled || !g_flightSfxVolume)
		return 0;
	objectType = XwFlightTypes_CanonicalType(g_playerFlightState.object->objectType);
	if (objectType != XW_OBJ_X_WING && objectType != XW_OBJ_Y_WING &&
		soundId >= FSFX_CRAFT_RESTRICTED_FIRST_SLOT && soundId <= FSFX_CRAFT_RESTRICTED_LAST_SLOT)
		return 0;
	volume = fsfx_calcvolume(objectIndex, soundId);
	if (!volume)
		return 0;
	pan = fsfx_calcpan(objectIndex, &volume);
	priority = FSFX_POSITIONAL_MAX_PRIORITY;
	if (volume < FSFX_PLAYER_PRIORITY)
		priority = volume;
	if (objectIndex == XW_OBJECT_SLOT_UNAVAILABLE || objectIndex == g_playerFlightState.objectIndex ||
		(objectIndex < XW_MISSION_OBJECT_REF_BASE &&
		 g_objectTable[objectIndex].sourceObjectRef == g_playerFlightState.objectIndex))
		priority = FSFX_PLAYER_PRIORITY;
	if (imuse_get_param(g_dos94Imuse, soundId, IMUSE_PARAM_SOUND_PLAY_COUNT)) {
		if (soundId == FSFX_NO_DUPLICATE_SLOT ||
			imuse_get_param(g_dos94Imuse, soundId, IMUSE_PARAM_SOUND_PRIORITY) > priority)
			return 0;
		imuse_stop_sound(g_dos94Imuse, soundId);
	}
	Dos94_hilevel_ImStartSfx(soundId);
	imuse_set_param(g_dos94Imuse, soundId, IMUSE_PARAM_SOUND_PRIORITY, priority);
	imuse_set_param(g_dos94Imuse, soundId, IMUSE_PARAM_SOUND_PAN, pan);
	imuse_set_param(g_dos94Imuse, soundId, IMUSE_PARAM_SOUND_VOL, volume);
	return 0;
}

// FUNCTION: DOS94 0x696C06
void Dos94_fsfx_triggergunsightsfx(uint16_t toneState) {
	if (!ShellPreferences_GetSfxEnabled())
		return;
	if (!g_flightSfxEnabled)
		return;
	if (!g_flightSfxVolume)
		return;
	if (toneState == FSFX_TONE_OFF || toneState == FSFX_TONE_IDLE) {
		if (imuse_get_param(g_dos94Imuse, FSFX_LOCKED_TONE_SLOT, IMUSE_PARAM_SOUND_PLAY_COUNT))
			imuse_stop_sound(g_dos94Imuse, FSFX_LOCKED_TONE_SLOT);
		else if (imuse_get_param(g_dos94Imuse, FSFX_ACQUIRING_TONE_SLOT, IMUSE_PARAM_SOUND_PLAY_COUNT))
			imuse_stop_sound(g_dos94Imuse, FSFX_ACQUIRING_TONE_SLOT);
	} else if (toneState == FSFX_TONE_LOCKED) {
		if (imuse_get_param(g_dos94Imuse, FSFX_ACQUIRING_TONE_SLOT, IMUSE_PARAM_SOUND_PLAY_COUNT))
			imuse_stop_sound(g_dos94Imuse, FSFX_ACQUIRING_TONE_SLOT);
		if (!imuse_get_param(g_dos94Imuse, FSFX_LOCKED_TONE_SLOT, IMUSE_PARAM_SOUND_PLAY_COUNT))
			Dos94_fsfx_triggersfx(FSFX_LOCKED_TONE_SLOT, XW_OBJECT_SLOT_UNAVAILABLE);
	} else {
		if (imuse_get_param(g_dos94Imuse, FSFX_LOCKED_TONE_SLOT, IMUSE_PARAM_SOUND_PLAY_COUNT))
			imuse_stop_sound(g_dos94Imuse, FSFX_LOCKED_TONE_SLOT);
		if (!imuse_get_param(g_dos94Imuse, FSFX_ACQUIRING_TONE_SLOT, IMUSE_PARAM_SOUND_PLAY_COUNT))
			Dos94_fsfx_triggersfx(FSFX_ACQUIRING_TONE_SLOT, XW_OBJECT_SLOT_UNAVAILABLE);
	}
}

// FUNCTION: DOS94 0x696D0E
int16_t Dos94_fsfx_triggervoicesfx(uint16_t sfxSlot) {
	if (ShellPreferences_GetSfxEnabled() != 0 && g_flightSfxEnabled != 0 && g_flightVoiceEnabled != 0) {
		uint16_t voiceSlot = sfxSlot;
		if ((voiceSlot == FSFX_VOICE_REMAP_SLOT_1 || voiceSlot == FSFX_VOICE_REMAP_SLOT_2) &&
			g_playerFlightState.craft->craftIndexInFlightGroup == 0) {
			voiceSlot += FSFX_VOICE_LEADER_SLOT_OFFSET;
		}
		if (g_fsfxLoaded != 0 && voiceSlot < FSFX_SOUND_HANDLE_COUNT &&
			g_fsfxLoadedSoundHandles[voiceSlot] != 0) {
			if (imuse_get_param(g_dos94Imuse, g_fsfxCurrentVoiceSfxSlot, IMUSE_PARAM_SOUND_PLAY_COUNT) != 0) {
				if (g_fsfxVoiceQueueCount != FSFX_VOICE_QUEUE_CAPACITY) {
					g_fsfxVoiceQueueSfxSlot[g_fsfxVoiceQueueCount++] = (uint8_t)voiceSlot;
					return 1;
				}
			} else {
				Dos94_hilevel_ImStartVoice(voiceSlot);
				imuse_set_param(g_dos94Imuse, voiceSlot, IMUSE_PARAM_SOUND_PRIORITY, 127);
				imuse_set_param(g_dos94Imuse, voiceSlot, IMUSE_PARAM_SOUND_PAN, 64);
				imuse_set_param(g_dos94Imuse, voiceSlot, IMUSE_PARAM_SOUND_VOL, 127);
				g_fsfxCurrentVoiceSfxSlot = voiceSlot;
				return 1;
			}
		}
	}
	return 0;
}

// FUNCTION: DOS94 0x696E04
void Dos94_fsfx_checkblastqueue(void) {
	if (ShellPreferences_GetSfxEnabled() != 0 && g_fsfxLoaded != 0 && g_fsfxVoiceQueueCount != 0) {
		uint16_t currentVoice = g_fsfxCurrentVoiceSfxSlot;
		if (currentVoice == FSFX_NO_VOICE_SLOT ||
			imuse_get_param(g_dos94Imuse, currentVoice, IMUSE_PARAM_SOUND_PLAY_COUNT) == 0) {
			uint16_t voiceSlot = g_fsfxVoiceQueueSfxSlot[0];
			uint16_t entriesRemaining = --g_fsfxVoiceQueueCount;
			unsigned int queueIndex;
			for (queueIndex = 0; entriesRemaining > 0; ++queueIndex, --entriesRemaining)
				g_fsfxVoiceQueueSfxSlot[queueIndex] = g_fsfxVoiceQueueSfxSlot[queueIndex + 1];
			if (g_flightSfxEnabled != 0 && g_flightVoiceEnabled != 0 &&
				g_fsfxLoadedSoundHandles[voiceSlot] != 0) {
				Dos94_hilevel_ImStartVoice(voiceSlot);
				imuse_set_param(g_dos94Imuse, voiceSlot, IMUSE_PARAM_SOUND_PRIORITY, 127);
				imuse_set_param(g_dos94Imuse, voiceSlot, IMUSE_PARAM_SOUND_PAN, 64);
				imuse_set_param(g_dos94Imuse, voiceSlot, IMUSE_PARAM_SOUND_VOL, 127);
				g_fsfxCurrentVoiceSfxSlot = voiceSlot;
			}
		}
	}
}
