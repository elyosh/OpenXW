#ifndef XW_AUDIO_SOUND_H
#define XW_AUDIO_SOUND_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

struct IDirectSoundBuffer;
struct IDirectSound;
typedef struct ActiveSoundInstance ActiveSoundInstance;
typedef struct SoundEffectDef SoundEffectDef;
typedef struct SoundQueueEntry SoundQueueEntry;

enum { SOUND_VOLUME_MIN = 0, SOUND_VOLUME_MAX = 127, SOUND_VOLUME_LEVEL_COUNT = 128 };

enum { SOUND_PARAM_INSTANCE_COUNT = 1, SOUND_PARAM_PRIORITY = 5 };

enum { SOUND_SOURCE_VOC = 1 };

enum { SOUND_VOLUME_GROUP_MASTER = 0, SOUND_VOLUME_GROUP_SFX = 1, SOUND_VOLUME_GROUP_VOICE = 2 };

enum { SOUND_GROUP_SFX = 0, SOUND_GROUP_VOICE = 1, SOUND_QUEUE_PAN_CENTER = 64 };

enum { SOUND_PAN_MIN = 0, SOUND_PAN_MAX = 127, SOUND_PAN_CENTER = 63, SOUND_PAN_ATTENUATION = 2000 };

enum { SOUND_EFFECT_CAPACITY = 1000, SOUND_ACTIVE_CAPACITY = 8, SOUND_QUEUE_CAPACITY = 9 };

enum { SOUND_QUEUE_LIMIT = SOUND_QUEUE_CAPACITY - 1 };

extern const int g_soundVolumeAttenuationByLevel[SOUND_VOLUME_LEVEL_COUNT];
extern struct IDirectSound* g_directSound;
extern struct IDirectSoundBuffer* g_soundPrimaryBuffer;
extern int g_soundQueueCount;
extern int g_soundCount;
extern int g_activeSoundCount;
extern int g_nextSoundInstanceSeq;
extern int g_soundGroup0Volume;
extern int g_soundGroup1Volume;
extern int g_soundMasterVolume;

/* Original IDB size: 24 bytes. */
struct ActiveSoundInstance {
	/* IDB +0x0: Index into g_soundDefs; -1 marks an unused active slot. */
	int soundId;
	/* IDB +0x4: Creation sequence; lowest signed value selects the oldest instance. */
	int sequence;
	/* IDB +0x8: Zero selects group0, nonzero group1; group identities intentionally left neutral. */
	int volumeGroup;
	/* IDB +0xC: Stored result of loop argument == 1. */
	int loop;
	/* IDB +0x10: Priority used when selecting a lower-priority active slot to replace. */
	int priority;
	/* IDB +0x14: Duplicate playback buffer owned by this active instance. */
	struct IDirectSoundBuffer* buffer;
};

/* Original IDB size: 325 bytes. */
struct SoundEffectDef {
	/* IDB +0x0: Name key compared over at most 64 bytes; loader writes a terminating zero at offset 63. */
	char name[64];
	/* IDB +0x40: Wave file name or resource label; loader terminates at offset 255 within this field. */
	char fileName[256];
	/* IDB +0x140: Base buffer duplicated for playback and released when the definition is unloaded. */
	struct IDirectSoundBuffer* buffer;
	/* IDB +0x144: Definition priority byte, clamped 0..255 by the setter; active instances carry a separate
	 * priority. */
	uint8_t currentPriority;
};

/* Original IDB size: 88 bytes. */
struct SoundQueueEntry {
	/* IDB +0x0 */
	char name[64];
	/* IDB +0x40: Effective 0..127-style volume after group/master scaling; converted for DirectSound at
	 * playback. */
	int volume;
	/* IDB +0x44: Pan input clamped to 0..127 by playback; center is 63. */
	int pan;
	/* IDB +0x48: Playback loops only when this value is 1. */
	int loop;
	/* IDB +0x4C: If no active slot can be reused, nonzero requests stopping existing instances of the same
	 * effect. */
	int stopMatchingWhenFull;
	/* IDB +0x50: Descending queue sort key and requested active-instance priority. */
	int priority;
	/* IDB +0x54: Zero selects group0, nonzero group1; propagated to active instance. */
	int volumeGroup;
};

extern SoundEffectDef g_soundDefs[SOUND_EFFECT_CAPACITY];
extern ActiveSoundInstance g_activeSoundInstances[SOUND_ACTIVE_CAPACITY];
extern SoundQueueEntry g_soundQueue[SOUND_QUEUE_CAPACITY];

/* Declarations follow ascending original IDB address. */

/* 0x485010 */
int32_t Sound_SetParam(int soundId, int param, int value);

/* 0x485110 */
int32_t Sound_QueueVoiceBySlot(int soundSlot, int priority);

/* 0x485150 */
int Sound_GetParam(int soundId, int parameter);

/* 0x4851D0 */
int32_t Sound_StopOldestInstanceById(int soundId);

/* 0x4AA4C0 */
int32_t Sound_Init_Sound_Engine(void* hwnd);

/* 0x4AA5E0 */
int32_t Sound_Shutdown_Sound_Engine(void);

/* 0x4AA650 */
int32_t Sound_LoadEffect(const char* fileName, const char* name, int sourceMode, const void* vocData,
						 unsigned int vocDataSize);

/* 0x4AA680 */
int32_t Sound_LoadEffectEx(const char* fileName, const char* name, int createFlags, int sourceMode,
						   const void* vocData, unsigned int vocDataSize);

/* 0x4AA7A0 */
void Sound_UnloadAllEffects(void);

/* 0x4AA7C0 */
int32_t Sound_UnloadEffectByName(const char* name);

/* 0x4AA840 */
void Sound_FlushQueuedEffects(void);

/* 0x4AA890 */
int32_t Sound_QueueEffect(const char* soundName, int stopMatchingWhenFull, int loop, int priority, int volume,
						  int pan, int volumeGroup);

/* 0x4AA9E0 */
int Sound_PlayEffectNow(const char* soundName, int stopMatchingWhenFull, int loop, int priority, int volume,
						int pan, int volumeGroup);

/* 0x4AAD30 */
int32_t Sound_StopOldestInstance(const char* name);

/* 0x4AAE00 */
int32_t Sound_StopAllInstances(void);

/* 0x4AAE40 */
void Sound_PauseActiveInstances(void);

/* 0x4AAEB0 */
void Sound_ResumeActiveInstances(void);

/* 0x4AAEF0 */
int32_t Sound_SetGroupVolume(int volume, int groupSelector);

/* 0x4AB040 */
int Sound_GetGroupVolume(int group);

/* 0x4AB070 */
int32_t Sound_SetOldestInstanceVolume(const char* soundName, int volume);

/* 0x4AB180 */
int Sound_CompareOldestInstanceVolume(const char* soundName, int desiredVolume);

/* 0x4AB240 */
int32_t Sound_SetOldestInstancePan(const char* soundName, int pan);

/* 0x4AB2F0 */
int32_t Sound_SetOldestInstanceFrequency(const char* soundName, unsigned int frequencyHz);

/* 0x4AB340 */
int32_t Sound_SetEffectCurrentPriority(const char* name, int priority);

/* 0x4AB400 */
int Sound_GetEffectCurrentPriority(const char* name);

/* 0x4AB430 */
int Sound_CountPlayingInstances(const char* name);

/* 0x4AB520 */
void Sound_InsertEffectDefSorted(const struct SoundEffectDef* effect);

/* 0x4AB5D0 */
void Sound_RemoveEffectDef(int soundId);

/* 0x4AB650 */
int Sound_FindLoadedEffectByName(const char* name);

/* 0x4AB670 */
int Sound_FindEffectByName(const struct SoundEffectDef* records, int lastIndex, const char* name);

/* 0x4AB6F0 */
int Sound_FindOldestActiveOrQueuedInstance(const char* name);

/* 0x4AB7B0 */
int Sound_MapAttenuationToVolume(int attenuation);

/* 0x4AB7E0 */
int Sound_MapVolumeToAttenuation(int volume);

#ifdef __cplusplus
}
#endif

#endif
