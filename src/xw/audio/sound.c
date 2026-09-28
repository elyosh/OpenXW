#include "xw/audio/sound.h"

#include "xw/audio/direct_sound.h"
#include "xw/audio/fsfx.h"
#include "xw/audio/lolevel.h"
#include "xw/flight/flight_display.h"

#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4DEAE0
const int g_soundVolumeAttenuationByLevel[SOUND_VOLUME_LEVEL_COUNT] = {
	-10000, -6000, -5415, -5000, -4678, -4415, -4192, -4000, -3830, -3678, -3540, -3415, -3299, -3192, -3093,
	-3000,  -2912, -2830, -2752, -2678, -2607, -2540, -2476, -2415, -2356, -2299, -2245, -2192, -2142, -2093,
	-2045,  -2000, -1955, -1912, -1870, -1830, -1790, -1752, -1714, -1678, -1642, -1607, -1573, -1540, -1508,
	-1476,  -1445, -1415, -1385, -1356, -1327, -1299, -1272, -1245, -1218, -1192, -1167, -1142, -1117, -1093,
	-1069,  -1045, -1022, -1000, -977,  -955,  -933,  -912,  -891,  -870,  -850,  -830,  -810,  -790,  -771,
	-752,   -733,  -714,  -696,  -678,  -660,  -642,  -624,  -607,  -590,  -573,  -557,  -540,  -524,  -508,
	-492,   -476,  -460,  -445,  -430,  -415,  -400,  -385,  -370,  -356,  -341,  -327,  -313,  -299,  -285,
	-272,   -258,  -245,  -231,  -218,  -205,  -192,  -179,  -167,  -154,  -142,  -129,  -117,  -105,  -93,
	-81,    -69,   -57,   -45,   -34,   -22,   -11,   0
};

// GLOBAL: XW 0x5686A4
struct IDirectSound* g_directSound = NULL;

#ifndef XW_MODERN
// GLOBAL: XW 0x5686A8
struct IDirectSoundBuffer* g_soundPrimaryBuffer = NULL;

// GLOBAL: XW 0x5686AC
SoundEffectDef g_soundDefs[SOUND_EFFECT_CAPACITY] = { 0 };

// GLOBAL: XW 0x5B865C
ActiveSoundInstance g_activeSoundInstances[SOUND_ACTIVE_CAPACITY] = { 0 };

// GLOBAL: XW 0x5B871C
SoundQueueEntry g_soundQueue[SOUND_QUEUE_CAPACITY] = { 0 };

// GLOBAL: XW 0x5B8A34
int g_soundQueueCount = 0;

// GLOBAL: XW 0x5B8A38
int g_soundCount = 0;

// GLOBAL: XW 0x5B8A3C
int g_activeSoundCount = 0;

// GLOBAL: XW 0x5B8A40
int g_nextSoundInstanceSeq = 0;

// GLOBAL: XW 0x5B8A44
int g_soundGroup0Volume = 0;

// GLOBAL: XW 0x5B8A48
int g_soundGroup1Volume = 0;

// GLOBAL: XW 0x5B8A4C
int g_soundMasterVolume = 0;

// FUNCTION: XW 0x485010
int32_t Sound_SetParam(int soundId, int param, int value) {
	int locks;
	int i;
	int result;
	if (soundId < FSFX_FIRST_PLAYABLE_SLOT || soundId > FSFX_LAST_PLAYABLE_SLOT)
		return 0;
	locks = FlightDisplay_GetSurfaceLockCount();
	if (locks) {
		for (i = locks; i > 0; --i)
			FlightDisplay_UnlockSurface();
	}
	switch (param) {
		case XW_SOUND_PARAM_PRIORITY:
			result = Sound_SetEffectCurrentPriority(g_fsfxSfxNameTable[soundId].name, value);
			break;
		case XW_SOUND_PARAM_PAN:
			result = Sound_SetOldestInstancePan(g_fsfxSfxNameTable[soundId].name, value);
			break;
		case XW_SOUND_PARAM_VOLUME:
			result = Sound_SetOldestInstanceVolume(g_fsfxSfxNameTable[soundId].name, value);
			break;
		case XW_SOUND_PARAM_FREQUENCY_HZ:
			result = Sound_SetOldestInstanceFrequency(g_fsfxSfxNameTable[soundId].name, value);
			break;
		default:
			result = 0;
			break;
	}
	if (locks) {
		for (i = locks; i > 0; --i)
			FlightDisplay_LockSurface();
	}
	return result;
}

// FUNCTION: XW 0x485110
int32_t Sound_QueueVoiceBySlot(int soundSlot, int priority) {
	if (soundSlot >= FSFX_FIRST_PLAYABLE_SLOT && soundSlot <= FSFX_LAST_PLAYABLE_SLOT) {
		return Sound_QueueEffect(g_fsfxSfxNameTable[soundSlot].name, 1, 0, priority, SOUND_VOLUME_MAX,
								 SOUND_QUEUE_PAN_CENTER, SOUND_GROUP_VOICE);
	}
	return 0;
}

// FUNCTION: XW 0x485150
int Sound_GetParam(int soundId, int parameter) {
	int locks;
	int index;
	int value;
	if (soundId < FSFX_FIRST_PLAYABLE_SLOT || soundId > FSFX_LAST_PLAYABLE_SLOT) {
		return 0;
	}
	locks = FlightDisplay_GetSurfaceLockCount();
	if (locks != 0) {
		for (index = locks; index > 0; --index) {
			FlightDisplay_UnlockSurface();
		}
	}
	switch (parameter) {
		case XW_SOUND_PARAM_INSTANCE_COUNT:
			value = Sound_CountPlayingInstances(g_fsfxSfxNameTable[soundId].name);
			break;
		case XW_SOUND_PARAM_PRIORITY:
			value = Sound_GetEffectCurrentPriority(g_fsfxSfxNameTable[soundId].name);
			break;
		default:
			value = 0;
			break;
	}
	if (locks != 0) {
		for (index = locks; index > 0; --index) {
			FlightDisplay_LockSurface();
		}
	}
	return value;
}

// FUNCTION: XW 0x4851D0
int32_t Sound_StopOldestInstanceById(int soundId) {
	int locks;
	int i;
	int32_t stopped;
	if (soundId < FSFX_FIRST_PLAYABLE_SLOT || soundId > FSFX_LAST_PLAYABLE_SLOT)
		return 0;
	locks = FlightDisplay_GetSurfaceLockCount();
	if (locks) {
		for (i = locks; i > 0; --i)
			FlightDisplay_UnlockSurface();
	}
	stopped = Sound_StopOldestInstance(g_fsfxSfxNameTable[soundId].name);
	if (locks) {
		for (i = locks; i > 0; --i)
			FlightDisplay_LockSurface();
	}
	return stopped;
}

// FUNCTION: XW 0x4AA4C0
int32_t Sound_Init_Sound_Engine(void* hwnd) {
	int i;
	DSBufferDesc descriptor;
	if (!g_directSound) {
		for (i = 0; i < SOUND_ACTIVE_CAPACITY; ++i) {
			g_activeSoundInstances[i].soundId = -1;
			g_activeSoundInstances[i].sequence = 0;
			g_activeSoundInstances[i].buffer = NULL;
			g_activeSoundInstances[i].volumeGroup = 0;
		}
		g_activeSoundCount = 0;
		g_soundGroup0Volume = SOUND_VOLUME_MAX;
		g_soundGroup1Volume = SOUND_VOLUME_MAX;
		g_soundMasterVolume = SOUND_VOLUME_MAX;
		g_soundCount = 0;
		g_nextSoundInstanceSeq = 0;
		for (i = 0; i < SOUND_EFFECT_CAPACITY; ++i) {
			g_soundDefs[i].buffer = NULL;
			g_soundDefs[i].name[0] = 0;
		}
		if (DirectSoundCreate(NULL, (void**)&g_directSound, NULL) != 0)
			return 0;
		if (g_directSound->lpVtbl->SetCooperativeLevel(g_directSound, hwnd, DSSCL_PRIORITY) != 0) {
			Sound_Shutdown_Sound_Engine();
			return 0;
		}
		memset(&descriptor, 0, sizeof(descriptor));
		descriptor.dwSize = sizeof(descriptor);
		descriptor.dwFlags = DSBCAPS_PRIMARYBUFFER;
		descriptor.dwBufferBytes = 0;
		descriptor.lpwfxFormat = NULL;
		if (g_directSound->lpVtbl->CreateSoundBuffer(g_directSound, &descriptor, &g_soundPrimaryBuffer,
													 NULL) != 0) {
			Sound_Shutdown_Sound_Engine();
			return 0;
		}
	}
	return 1;
}

// FUNCTION: XW 0x4AA5E0
int32_t Sound_Shutdown_Sound_Engine(void) {
	int i;
	if (g_directSound) {
		Sound_UnloadAllEffects();
		g_directSound->lpVtbl->Release(g_directSound);
		g_directSound = NULL;
		for (i = 0; i < SOUND_EFFECT_CAPACITY; ++i) {
			g_soundDefs[i].buffer = NULL;
			g_soundDefs[i].name[0] = 0;
		}
		for (i = 0; i < SOUND_ACTIVE_CAPACITY; ++i) {
			g_activeSoundInstances[i].soundId = -1;
			g_activeSoundInstances[i].sequence = 0;
			g_activeSoundInstances[i].volumeGroup = 0;
			g_activeSoundInstances[i].buffer = NULL;
		}
		g_soundPrimaryBuffer = NULL;
		g_nextSoundInstanceSeq = 0;
	}
	return 1;
}

// FUNCTION: XW 0x4AA650
int32_t Sound_LoadEffect(const char* fileName, const char* name, int sourceMode, const void* vocData,
						 unsigned int vocDataSize) {
	return Sound_LoadEffectEx(fileName, name, 0, sourceMode, vocData, vocDataSize);
}

// FUNCTION: XW 0x4AA680
int32_t Sound_LoadEffectEx(const char* fileName, const char* name, int createFlags, int sourceMode,
						   const void* vocData, unsigned int vocDataSize) {
	SoundEffectDef effect;
	if (!fileName[0] || !name[0] || g_soundCount >= SOUND_EFFECT_CAPACITY || !g_directSound ||
		Sound_FindLoadedEffectByName(name) != -1)
		return 0;

	switch (sourceMode) {
		case SOUND_SOURCE_VOC:
			effect.buffer = DirectSound_LoadVocBuffer(g_directSound, fileName, createFlags,
													  (const XwVocType1Header*)vocData, vocDataSize);
			break;
		default:
			effect.buffer = DirectSound_LoadWaveBuffer(g_directSound, fileName, createFlags);
			break;
	}
	if (!effect.buffer)
		return 0;
	effect.buffer->lpVtbl->SetCurrentPosition(effect.buffer, 0);
	strncpy(effect.name, name, sizeof(effect.name));
	effect.name[sizeof(effect.name) - 1] = 0;
	strncpy(effect.fileName, fileName, sizeof(effect.fileName));
	effect.fileName[sizeof(effect.fileName) - 1] = 0;
	effect.currentPriority = 0;
	Sound_InsertEffectDefSorted(&effect);
	return effect.buffer != NULL;
}

// FUNCTION: XW 0x4AA7A0
void Sound_UnloadAllEffects(void) {
	int i;
	for (i = 0; i < SOUND_EFFECT_CAPACITY; ++i)
		Sound_UnloadEffectByName(g_soundDefs[i].name);
}

// FUNCTION: XW 0x4AA7C0
int32_t Sound_UnloadEffectByName(const char* name) {
	int soundId;
	if (!name[0])
		return 0;
	soundId = Sound_FindLoadedEffectByName(name);
	if (soundId == -1)
		return 0;
	while (Sound_StopOldestInstance(g_soundDefs[soundId].name) == 1) {
	}
	g_soundDefs[soundId].buffer->lpVtbl->Release(g_soundDefs[soundId].buffer);
	Sound_RemoveEffectDef(soundId);
	return 1;
}

// FUNCTION: XW 0x4AA840
void Sound_FlushQueuedEffects(void) {
	int i;
	for (i = 0; i < g_soundQueueCount; ++i)
		Sound_PlayEffectNow(g_soundQueue[i].name, g_soundQueue[i].stopMatchingWhenFull, g_soundQueue[i].loop,
							g_soundQueue[i].priority, g_soundQueue[i].volume, g_soundQueue[i].pan,
							g_soundQueue[i].volumeGroup);
	g_soundQueueCount = 0;
}

// FUNCTION: XW 0x4AA890
int32_t Sound_QueueEffect(const char* soundName, int stopMatchingWhenFull, int loop, int priority, int volume,
						  int pan, int volumeGroup) {
	int insertIndex;
	int scaledVolume;
	if (!g_directSound)
		return 0;
	if (!soundName[0])
		return 0;
	if (Sound_FindLoadedEffectByName(soundName) == -1)
		return 0;
	for (insertIndex = 0; insertIndex < g_soundQueueCount; ++insertIndex) {
		if (g_soundQueue[insertIndex].priority < priority)
			break;
	}
	if (insertIndex == g_soundQueueCount && insertIndex == SOUND_QUEUE_LIMIT)
		return 0;
	memmove(&g_soundQueue[insertIndex + 1], &g_soundQueue[insertIndex],
			g_soundQueueCount * sizeof(g_soundQueue[0]) - insertIndex * sizeof(g_soundQueue[0]));
	strncpy(g_soundQueue[insertIndex].name, soundName, sizeof(g_soundQueue[insertIndex].name));
	g_soundQueue[insertIndex].loop = loop;
	g_soundQueue[insertIndex].stopMatchingWhenFull = stopMatchingWhenFull;
	g_soundQueue[insertIndex].priority = priority;
	if (volumeGroup) {
		scaledVolume = g_soundGroup1Volume * volume;
	} else {
		scaledVolume = g_soundGroup0Volume * volume;
	}
	scaledVolume = g_soundMasterVolume * (scaledVolume / SOUND_VOLUME_MAX) / SOUND_VOLUME_MAX;
	g_soundQueue[insertIndex].volume = scaledVolume;
	g_soundQueue[insertIndex].pan = pan;
	g_soundQueue[insertIndex].volumeGroup = volumeGroup;
	if (++g_soundQueueCount > SOUND_QUEUE_LIMIT)
		g_soundQueueCount = SOUND_QUEUE_LIMIT;
	return 1;
}

// FUNCTION: XW 0x4AA9E0
int Sound_PlayEffectNow(const char* soundName, int stopMatchingWhenFull, int loop, int priority, int volume,
						int pan, int volumeGroup) {
	int soundId;
	int slotIndex;
	int i;
	uint32_t bufferStatus;
	IDirectSoundBuffer* buffer;
	int loopFlag;
	int directSoundVolume;
	HRESULT result;
	if (!g_directSound)
		return 0;
	if (!soundName[0])
		return 0;
	soundId = Sound_FindLoadedEffectByName(soundName);
	if (soundId == -1)
		return 0;
	if (g_activeSoundCount == SOUND_ACTIVE_CAPACITY) {
		for (slotIndex = 0; slotIndex < SOUND_ACTIVE_CAPACITY; ++slotIndex) {
			if (g_activeSoundInstances[slotIndex].soundId != -1 &&
				(g_activeSoundInstances[slotIndex].buffer->lpVtbl->GetStatus(
					 g_activeSoundInstances[slotIndex].buffer, &bufferStatus) != 0 ||
				 !(bufferStatus & (DSBSTATUS_PLAYING | DSBSTATUS_LOOPING)))) {
				g_activeSoundInstances[slotIndex].buffer->lpVtbl->Release(
					g_activeSoundInstances[slotIndex].buffer);
				g_activeSoundInstances[slotIndex].soundId = -1;
				g_activeSoundInstances[slotIndex].buffer = NULL;
				--g_activeSoundCount;
				break;
			}
		}
		if (slotIndex == SOUND_ACTIVE_CAPACITY) {
			int lowestPriority = priority;
			for (i = 0; i < SOUND_ACTIVE_CAPACITY; ++i) {
				if (g_activeSoundInstances[i].priority < lowestPriority) {
					slotIndex = i;
					lowestPriority = g_activeSoundInstances[i].priority;
				}
			}
			if (slotIndex != SOUND_ACTIVE_CAPACITY) {
				g_activeSoundInstances[slotIndex].buffer->lpVtbl->Stop(
					g_activeSoundInstances[slotIndex].buffer);
				g_activeSoundInstances[slotIndex].buffer->lpVtbl->Release(
					g_activeSoundInstances[slotIndex].buffer);
				g_activeSoundInstances[slotIndex].soundId = -1;
				g_activeSoundInstances[slotIndex].buffer = NULL;
				--g_activeSoundCount;
			} else {
				if (stopMatchingWhenFull) {
					for (i = 0; i < SOUND_ACTIVE_CAPACITY; ++i) {
						if (g_activeSoundInstances[i].soundId == soundId) {
							g_activeSoundInstances[i].buffer->lpVtbl->Stop(g_activeSoundInstances[i].buffer);
							g_activeSoundInstances[i].buffer->lpVtbl->Release(
								g_activeSoundInstances[i].buffer);
							g_activeSoundInstances[i].soundId = -1;
							g_activeSoundInstances[i].buffer = NULL;
							--g_activeSoundCount;
						}
					}
				}
				return 0;
			}
		}
	} else {
		for (slotIndex = 0; slotIndex < SOUND_ACTIVE_CAPACITY; ++slotIndex) {
			if (g_activeSoundInstances[slotIndex].soundId == -1)
				break;
		}
	}
	g_directSound->lpVtbl->DuplicateSoundBuffer(g_directSound, g_soundDefs[soundId].buffer, &buffer);
	if (!buffer)
		return 0;
	buffer->lpVtbl->SetCurrentPosition(buffer, 0);
	directSoundVolume = Sound_MapVolumeToAttenuation(volume);
	buffer->lpVtbl->SetVolume(buffer, directSoundVolume);
	if (pan > SOUND_PAN_MAX)
		pan = SOUND_PAN_MAX;
	if (pan < SOUND_PAN_MIN)
		pan = SOUND_PAN_MIN;
	buffer->lpVtbl->SetPan(buffer, SOUND_PAN_ATTENUATION * (pan - SOUND_PAN_CENTER) / SOUND_PAN_CENTER);
	loopFlag = loop == 1;
	result = buffer->lpVtbl->Play(buffer, 0, 0, loopFlag);
	if (result == DX_DSERR_BUFFERLOST) {
		result = DirectSound_ReloadWaveBuffer(g_soundDefs[soundId].buffer, g_soundDefs[soundId].fileName);
		if (result == 1) {
			buffer->lpVtbl->SetCurrentPosition(buffer, 0);
			result = buffer->lpVtbl->Play(buffer, 0, 0, loopFlag);
			if (result == 0) {
				g_activeSoundInstances[slotIndex].soundId = soundId;
				g_activeSoundInstances[slotIndex].buffer = buffer;
				g_activeSoundInstances[slotIndex].sequence = g_nextSoundInstanceSeq;
				++g_nextSoundInstanceSeq;
				g_activeSoundInstances[slotIndex].volumeGroup = volumeGroup;
				g_activeSoundInstances[slotIndex].loop = loopFlag;
				g_activeSoundInstances[slotIndex].priority = priority;
				++g_activeSoundCount;
				return result;
			}
			return 0;
		}
	} else if (result == 0) {
		g_activeSoundInstances[slotIndex].soundId = soundId;
		g_activeSoundInstances[slotIndex].buffer = buffer;
		g_activeSoundInstances[slotIndex].volumeGroup = volumeGroup;
		g_activeSoundInstances[slotIndex].loop = loopFlag;
		g_activeSoundInstances[slotIndex].priority = priority;
		g_activeSoundInstances[slotIndex].sequence = g_nextSoundInstanceSeq;
		++g_nextSoundInstanceSeq;
		++g_activeSoundCount;
		return 1;
	}
	return result;
}

// FUNCTION: XW 0x4AAD30
int32_t Sound_StopOldestInstance(const char* name) {
	int instanceIndex;
	if (!g_directSound)
		return 0;
	if (!name[0])
		return 0;
	instanceIndex = Sound_FindOldestActiveOrQueuedInstance(name);
	if (instanceIndex == -1)
		return 0;
	if (instanceIndex < SOUND_ACTIVE_CAPACITY) {
		IDirectSoundBuffer* buffer = g_activeSoundInstances[instanceIndex].buffer;
		HRESULT result;
		if (!buffer)
			return 0;
		result = buffer->lpVtbl->Stop(buffer);
		buffer->lpVtbl->Release(buffer);
		g_activeSoundInstances[instanceIndex].buffer = NULL;
		g_activeSoundInstances[instanceIndex].soundId = -1;
		--g_activeSoundCount;
		return result >= 0;
	}
	g_soundQueue[instanceIndex - SOUND_ACTIVE_CAPACITY] = g_soundQueue[g_soundQueueCount - 1];
	--g_soundQueueCount;
	return 1;
}

// FUNCTION: XW 0x4AAE00
int32_t Sound_StopAllInstances(void) {
	int i;
	int allStopsSucceeded = 1;
	for (i = 0; i < SOUND_ACTIVE_CAPACITY; ++i) {
		if (g_activeSoundInstances[i].soundId != -1)
			allStopsSucceeded &=
				Sound_StopOldestInstance(g_soundDefs[g_activeSoundInstances[i].soundId].name);
	}
	return allStopsSucceeded;
}

// FUNCTION: XW 0x4AAE40
void Sound_PauseActiveInstances(void) {
	int i;
	uint32_t status;
	for (i = 0; i < SOUND_ACTIVE_CAPACITY; ++i) {
		if (g_activeSoundInstances[i].soundId != -1) {
			IDirectSoundBuffer* buffer = g_activeSoundInstances[i].buffer;
			if (buffer) {
				if (buffer->lpVtbl->GetStatus(buffer, &status) == 0 &&
					(status & (DSBSTATUS_PLAYING | DSBSTATUS_LOOPING)))
					buffer->lpVtbl->Stop(buffer);
				else {
					buffer->lpVtbl->Release(buffer);
					g_activeSoundInstances[i].soundId = -1;
					g_activeSoundInstances[i].buffer = NULL;
					--g_activeSoundCount;
				}
			}
		}
	}
}

// FUNCTION: XW 0x4AAEB0
void Sound_ResumeActiveInstances(void) {
	int i;
	for (i = 0; i < SOUND_ACTIVE_CAPACITY; ++i) {
		if (g_activeSoundInstances[i].soundId != -1 && g_activeSoundInstances[i].buffer)
			g_activeSoundInstances[i].buffer->lpVtbl->Play(g_activeSoundInstances[i].buffer, 0, 0,
														   g_activeSoundInstances[i].loop);
	}
}

// FUNCTION: XW 0x4AAEF0
int32_t Sound_SetGroupVolume(int volume, int groupSelector) {
	int clampedVolume;
	int attenuation;
	int i;
	if (!g_directSound)
		return 0;
	clampedVolume = volume;
	if (clampedVolume > SOUND_VOLUME_MAX)
		clampedVolume = SOUND_VOLUME_MAX;
	else if (clampedVolume < SOUND_VOLUME_MIN)
		clampedVolume = SOUND_VOLUME_MIN;
	if (groupSelector == SOUND_VOLUME_GROUP_SFX)
		g_soundGroup0Volume = clampedVolume;
	else if (groupSelector == SOUND_VOLUME_GROUP_VOICE)
		g_soundGroup1Volume = clampedVolume;
	else
		g_soundMasterVolume = clampedVolume;
	for (i = 0; i < g_soundQueueCount; ++i) {
		if (groupSelector == SOUND_VOLUME_GROUP_VOICE) {
			if (g_soundQueue[i].volumeGroup)
				g_soundQueue[i].volume = clampedVolume;
		} else if (groupSelector == SOUND_VOLUME_GROUP_SFX) {
			if (!g_soundQueue[i].volumeGroup)
				g_soundQueue[i].volume = clampedVolume;
		} else {
			g_soundQueue[i].volume = g_soundQueue[i].volume * clampedVolume / SOUND_VOLUME_MAX;
		}
	}
	attenuation = Sound_MapVolumeToAttenuation(clampedVolume);
	for (i = 0; i < SOUND_ACTIVE_CAPACITY; ++i) {
		if (g_activeSoundInstances[i].soundId == -1)
			continue;
		if (groupSelector == SOUND_VOLUME_GROUP_VOICE) {
			if (!g_activeSoundInstances[i].volumeGroup)
				continue;
		} else if (groupSelector == SOUND_VOLUME_GROUP_SFX) {
			if (g_activeSoundInstances[i].volumeGroup)
				continue;
		} else {
			if (g_activeSoundInstances[i].buffer->lpVtbl->GetVolume(g_activeSoundInstances[i].buffer,
																	&volume) != 0)
				continue;
			volume = g_soundMasterVolume * Sound_MapAttenuationToVolume(volume);
			volume /= SOUND_VOLUME_MAX;
			attenuation = Sound_MapVolumeToAttenuation(volume);
		}
		g_activeSoundInstances[i].buffer->lpVtbl->SetVolume(g_activeSoundInstances[i].buffer, attenuation);
	}
	return 1;
}

// FUNCTION: XW 0x4AB040
int Sound_GetGroupVolume(int group) {
	if (!g_directSound)
		return 0;
	if (group == SOUND_VOLUME_GROUP_VOICE)
		return g_soundGroup1Volume;
	if (group == SOUND_VOLUME_GROUP_SFX)
		return g_soundGroup0Volume;
	return g_soundMasterVolume;
}

// FUNCTION: XW 0x4AB070
int32_t Sound_SetOldestInstanceVolume(const char* soundName, int volume) {
	int instanceIndex;
	int groupVolume;
	int scaledVolume;
	if (!g_directSound)
		return 0;
	if (!soundName[0])
		return 0;
	instanceIndex = Sound_FindOldestActiveOrQueuedInstance(soundName);
	if (instanceIndex == -1)
		return 0;
	if (instanceIndex < SOUND_ACTIVE_CAPACITY) {
		int attenuation;
		HRESULT result;
		if (g_activeSoundInstances[instanceIndex].volumeGroup)
			scaledVolume = g_soundGroup1Volume * volume;
		else
			scaledVolume = g_soundGroup0Volume * volume;
		scaledVolume = g_soundMasterVolume * (scaledVolume / SOUND_VOLUME_MAX) / SOUND_VOLUME_MAX;
		attenuation = Sound_MapVolumeToAttenuation(scaledVolume);
		result = g_activeSoundInstances[instanceIndex].buffer->lpVtbl->SetVolume(
			g_activeSoundInstances[instanceIndex].buffer, attenuation);
		return result == DX_S_OK;
	}
	instanceIndex -= SOUND_ACTIVE_CAPACITY;
	if (g_soundQueue[instanceIndex].volumeGroup)
		scaledVolume = g_soundGroup1Volume * volume;
	else
		scaledVolume = g_soundGroup0Volume * volume;
	scaledVolume = g_soundMasterVolume * (scaledVolume / SOUND_VOLUME_MAX) / SOUND_VOLUME_MAX;
	g_soundQueue[instanceIndex].volume = scaledVolume;
	return 1;
}

// FUNCTION: XW 0x4AB180
int Sound_CompareOldestInstanceVolume(const char* soundName, int desiredVolume) {
	int instanceIndex;
	int currentVolume;
	int scaledDesiredVolume;
	if (g_directSound == NULL) {
		return 0;
	}
	if (soundName[0] == '\0') {
		return 0;
	}
	instanceIndex = Sound_FindOldestActiveOrQueuedInstance(soundName);
	if (instanceIndex == -1) {
		return 0;
	}
	if (instanceIndex < SOUND_ACTIVE_CAPACITY) {
		int32_t attenuation;
		if (g_activeSoundInstances[instanceIndex].buffer->lpVtbl->GetVolume(
				g_activeSoundInstances[instanceIndex].buffer, &attenuation) != DX_S_OK) {
			return 0;
		}
		currentVolume = Sound_MapAttenuationToVolume(attenuation);
	} else {
		currentVolume = g_soundQueue[instanceIndex - SOUND_ACTIVE_CAPACITY].volume;
	}
	scaledDesiredVolume =
		g_soundMasterVolume * (g_soundGroup0Volume * desiredVolume / SOUND_VOLUME_MAX) / SOUND_VOLUME_MAX;
	if (scaledDesiredVolume == currentVolume) {
		return 0;
	}
	return currentVolume < scaledDesiredVolume ? -1 : 1;
}

// FUNCTION: XW 0x4AB240
int32_t Sound_SetOldestInstancePan(const char* soundName, int pan) {
	int instanceIndex;
	if (!g_directSound)
		return 0;
	if (!soundName[0])
		return 0;
	instanceIndex = Sound_FindOldestActiveOrQueuedInstance(soundName);
	if (instanceIndex == -1)
		return 0;
	if (instanceIndex < SOUND_ACTIVE_CAPACITY) {
		HRESULT result;
		if (pan > SOUND_PAN_MAX)
			pan = SOUND_PAN_MAX;
		if (pan < SOUND_PAN_MIN)
			pan = SOUND_PAN_MIN;
		result = g_activeSoundInstances[instanceIndex].buffer->lpVtbl->SetPan(
			g_activeSoundInstances[instanceIndex].buffer,
			SOUND_PAN_ATTENUATION * (pan - SOUND_PAN_CENTER) / SOUND_PAN_CENTER);
		return result == DX_S_OK;
	}
	g_soundQueue[instanceIndex - SOUND_ACTIVE_CAPACITY].pan = pan;
	return 1;
}

// FUNCTION: XW 0x4AB2F0
int32_t Sound_SetOldestInstanceFrequency(const char* soundName, unsigned int frequencyHz) {
	int instanceIndex;
	if (g_directSound && soundName[0] &&
		(instanceIndex = Sound_FindOldestActiveOrQueuedInstance(soundName)) != -1 &&
		instanceIndex < SOUND_ACTIVE_CAPACITY) {
		HRESULT result = g_activeSoundInstances[instanceIndex].buffer->lpVtbl->SetFrequency(
			g_activeSoundInstances[instanceIndex].buffer, frequencyHz);
		return result == DX_S_OK;
	}
	return 0;
}

// FUNCTION: XW 0x4AB340
int32_t Sound_SetEffectCurrentPriority(const char* name, int priority) {
	int soundId;
	int i;
	if (!g_directSound)
		return 0;
	if (!name[0])
		return 0;
	soundId = Sound_FindLoadedEffectByName(name);
	if (soundId == -1)
		return 0;
	if (priority > UINT8_MAX)
		priority = UINT8_MAX;
	if (priority < 0)
		priority = 0;
	g_soundDefs[soundId].currentPriority = priority;
	for (i = 0; i < g_soundQueueCount; ++i) {
		if (strcmp(g_soundQueue[i].name, name) == 0)
			g_soundQueue[i].priority = priority;
	}
	return 1;
}

// FUNCTION: XW 0x4AB400
int Sound_GetEffectCurrentPriority(const char* name) {
	int soundId = Sound_FindLoadedEffectByName(name);
	if (soundId == -1)
		return 0;
	return g_soundDefs[soundId].currentPriority;
}

// FUNCTION: XW 0x4AB430
int Sound_CountPlayingInstances(const char* name) {
	int soundId;
	int count;
	int i;
	uint32_t status;
	if (!g_directSound)
		return 0;
	if (!name[0])
		return 0;
	soundId = Sound_FindLoadedEffectByName(name);
	if (soundId == -1)
		return 0;
	count = 0;
	for (i = 0; i < SOUND_ACTIVE_CAPACITY; ++i) {
		if (g_activeSoundInstances[i].soundId == soundId &&
			g_activeSoundInstances[i].buffer->lpVtbl->GetStatus(g_activeSoundInstances[i].buffer, &status) ==
				0 &&
			(status & (DSBSTATUS_PLAYING | DSBSTATUS_LOOPING)))
			++count;
	}
	if (!count) {
		int queuedCount = g_soundQueueCount;
		for (i = 0; i < queuedCount; ++i) {
			if (strcmp(name, g_soundQueue[i].name) == 0)
				++count;
		}
	}
	return count;
}

// FUNCTION: XW 0x4AB520
void Sound_InsertEffectDefSorted(const struct SoundEffectDef* effect) {
	int insertIndex;
	int i;
	for (insertIndex = 0; insertIndex < g_soundCount; ++insertIndex) {
		if (strncmp(effect->name, g_soundDefs[insertIndex].name, sizeof(effect->name)) < 0)
			break;
	}
	for (i = g_soundCount - 1; i >= insertIndex; --i)
		g_soundDefs[i + 1] = g_soundDefs[i];
	g_soundDefs[insertIndex] = *effect;
	++g_soundCount;
	for (i = 0; i < SOUND_ACTIVE_CAPACITY; ++i) {
		if (g_activeSoundInstances[i].soundId >= insertIndex)
			++g_activeSoundInstances[i].soundId;
	}
}

// FUNCTION: XW 0x4AB5D0
void Sound_RemoveEffectDef(int soundId) {
	int i;
	if (soundId >= 0 && soundId < g_soundCount) {
		for (i = soundId + 1; i < g_soundCount; ++i)
			g_soundDefs[i - 1] = g_soundDefs[i];
		--g_soundCount;
		for (i = 0; i < SOUND_ACTIVE_CAPACITY; ++i) {
			if (g_activeSoundInstances[i].soundId > soundId)
				--g_activeSoundInstances[i].soundId;
		}
	}
}

// FUNCTION: XW 0x4AB650
int Sound_FindLoadedEffectByName(const char* name) {
	return Sound_FindEffectByName(g_soundDefs, g_soundCount - 1, name);
}

// FUNCTION: XW 0x4AB670
int Sound_FindEffectByName(const struct SoundEffectDef* records, int lastIndex, const char* name) {
	int baseIndex = 0;
	while (lastIndex >= 0) {
		int midIndex = lastIndex >> 1;
		int comparison = strncmp(records[baseIndex + midIndex].name, name, sizeof(records[0].name));
		if (comparison == 0)
			return baseIndex + midIndex;
		if (lastIndex <= 0)
			break;
		if (comparison >= 0)
			lastIndex = midIndex - 1;
		else {
			lastIndex += -1 - midIndex;
			baseIndex += midIndex + 1;
		}
	}
	return -1;
}

// FUNCTION: XW 0x4AB6F0
int Sound_FindOldestActiveOrQueuedInstance(const char* name) {
	int soundId = Sound_FindLoadedEffectByName(name);
	if (soundId == -1)
		return soundId;
	{
		int oldestSlot = -1;
		int oldestSequence;
		int i;
		oldestSequence = g_nextSoundInstanceSeq + 1;
		for (i = 0; i < SOUND_ACTIVE_CAPACITY; ++i) {
			if (g_activeSoundInstances[i].soundId == soundId &&
				g_activeSoundInstances[i].sequence < oldestSequence) {
				oldestSequence = g_activeSoundInstances[i].sequence;
				oldestSlot = i;
			}
		}
		if (oldestSlot == -1) {
			for (i = 0; i < g_soundQueueCount; ++i) {
				if (strcmp(g_soundQueue[i].name, name) == 0) {
					oldestSlot = i + SOUND_ACTIVE_CAPACITY;
					break;
				}
			}
		}
		return oldestSlot;
	}
}

// FUNCTION: XW 0x4AB7B0
int Sound_MapAttenuationToVolume(int attenuation) {
	int volume;
	for (volume = SOUND_VOLUME_MIN; volume < SOUND_VOLUME_LEVEL_COUNT; ++volume) {
		if (attenuation <= g_soundVolumeAttenuationByLevel[volume]) {
			return volume;
		}
	}
	return SOUND_VOLUME_MAX;
}

#endif

// FUNCTION: XW 0x4AB7E0
int Sound_MapVolumeToAttenuation(int volume) {
	if (volume > SOUND_VOLUME_MAX) {
		volume = SOUND_VOLUME_MAX;
	}
	if (volume < SOUND_VOLUME_MIN) {
		volume = SOUND_VOLUME_MIN;
	}
	return g_soundVolumeAttenuationByLevel[volume];
}
