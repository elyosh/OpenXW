#include "xw/audio/fsfx.h"

#ifdef XW_MODERN
#include "xw_dos94/audio/fsfx.h"
#include "xw_runtime/audio/imuse_session.h"
#include "xw_runtime/audio/player_engine.h"
#include "xw_runtime/runtime/flight_dispatch.h"
#include "xw_runtime/runtime/flight_types.h"
#endif

#include "xw/audio/fmusic.h"
#include "xw/audio/lolevel.h"
#include "xw/audio/sound.h"
#include "xw/flight/death_star.h"
#include "xw/flight/fediskio.h"
#include "xw/flight/hud/panel.h"
#include "xw/flight/object/collide.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/object/create.h"
#include "xw/flight/player/user.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/math/math2.h"
#include "xw/math/transfm2.h"
#include "xw/math/trig2.h"
#include "xw/render/flight_view.h"
#include "xw/util/memory.h"
#include "xw/util/shared.h"

#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4C7760
uint16_t g_sfxAttenuationRangeBySlot[FSFX_RANGE_TABLE_COUNT] = {
	0,     0,     0,    0,    8192, 8192, 8192, 10240, 10240, 10240, 10240, 10240, 49152, 24576, 24576,
	24576, 24576, 8192, 8192, 8192, 8192, 8192, 8192,  8192,  8192,  8192,  8192,  8192,  8192,  8192,
	8192,  8192,  8192, 8192, 8192, 8192, 8192, 8192,  8192,  8192,  8192,  8192,  8192,  0
};

// GLOBAL: XW 0x4C77B8
const uint8_t g_sfxFullVolumeBySlot[FSFX_VOLUME_TABLE_COUNT] = {
	0,   0,  0,   0,   72,  72, 72, 80, 80, 80, 80, 80, 127, 104, 104, 104, 104, 72,  112, 127, 127, 127,
	112, 96, 112, 112, 112, 96, 96, 80, 80, 80, 80, 80, 80,  80,  96,  96,  96,  112, 112, 112, 112
};

// GLOBAL: XW 0x4D4338
uint8_t g_flightSfxEnabled = 1;

// GLOBAL: XW 0x4D4340
uint8_t g_flightVoiceEnabled = 1;

#ifndef XW_MODERN
// GLOBAL: XW 0x4F48D8
int g_playerEngineLoopObjectType = 0;
#endif

// GLOBAL: XW 0x62AFEE
uint8_t g_fsfxVoiceQueueCount = 0;

// GLOBAL: XW 0x62B6E0
uint8_t g_playerEngineLoopSuppressed = 0;

// GLOBAL: XW 0x62C929
uint8_t g_fsfxLoaded = 0;

// GLOBAL: XW 0x637370
uint8_t g_fsfxVoiceQueueSfxSlot[FSFX_VOICE_QUEUE_CAPACITY] = { 0 };

#ifndef XW_MODERN
// GLOBAL: XW 0x63B220
FlightSoundName g_fsfxSfxNameTable[FSFX_NAME_COUNT] = { 0 };
#endif

// GLOBAL: XW 0x63B9A0
uint16_t g_fsfxLoadedSoundHandles[FSFX_SOUND_HANDLE_COUNT] = { 0 };

// GLOBAL: XW 0x63BA3E
uint8_t g_fsfxCurrentVoiceSfxSlot = FSFX_NO_VOICE_SLOT;

// FUNCTION: XW 0x40C3C0
void j_Sound_UnloadAllEffects(void) {
#ifdef XW_MODERN
	XwFlightMode_UnloadSounds();
#else
	Sound_UnloadAllEffects();
#endif
}

// FUNCTION: XW 0x40C3D0
uint16_t fsfx_loadsfx(const char* archivePath, uint32_t legacyResourceTag) {
#ifdef XW_MODERN
	(void)legacyResourceTag;
	return Dos94_fsfx_loadsfx(archivePath);
#else
	uint32_t pendingSkipBytes;
	uint16_t opened;
	XwFile* archiveStream;
	CockpitLfdEntryHeader rmapHeader;
	CockpitLfdEntryHeader* directory;
	uint16_t entryCount;
	uint16_t directoryIndex;
	uint16_t soundId;
	(void)legacyResourceTag;
	memset(g_fsfxLoadedSoundHandles, 0, sizeof(g_fsfxLoadedSoundHandles));
	pendingSkipBytes = 0;
	opened = fediskio_tryopenfile(archivePath, "rb", 0);
	if (opened == 0)
		return opened;
	archiveStream = g_stream;
	fediskio_readfileblock(&rmapHeader, 1, sizeof(rmapHeader), archiveStream);
	directory = (CockpitLfdEntryHeader*)g_FileLoadBuffer;
	fmusic_readfiledata(archiveStream, directory, (uint16_t)rmapHeader.payloadSize);
	entryCount = (uint16_t)rmapHeader.payloadSize / sizeof(rmapHeader);
	soundId = FSFX_FIRST_PLAYABLE_SLOT;
	for (directoryIndex = 0; directoryIndex < entryCount; ++directoryIndex) {
		uint32_t typeTag;
		uint16_t soundHandle;
		pendingSkipBytes += sizeof(rmapHeader);
		memcpy(&typeTag, directory[directoryIndex].typeTag, sizeof(typeTag));
		typeTag = fmusic_swapdword(typeTag);
		memcpy(directory[directoryIndex].typeTag, &typeTag, sizeof(typeTag));
		soundHandle = Memory_AllocHandle(directory[directoryIndex].payloadSize, 0);
		directory = (CockpitLfdEntryHeader*)g_FileLoadBuffer;
		g_fsfxLoadedSoundHandles[soundId] = soundHandle;
		if (soundHandle != 0) {
			void* vocData;
			if (pendingSkipBytes != 0) {
				File_RawSeek(archiveStream, (int32_t)pendingSkipBytes, SEEK_CUR);
				pendingSkipBytes = 0;
			}
			vocData = Memory_LockHandle(g_fsfxLoadedSoundHandles[soundId]);
			fediskio_readfileblock(vocData, 1, directory[directoryIndex].payloadSize, archiveStream);
			strcpy(g_fsfxSfxNameTable[soundId].name, directory[directoryIndex].resourceName);
			Sound_LoadEffect(directory[directoryIndex].resourceName, directory[directoryIndex].resourceName,
							 SOUND_SOURCE_VOC, vocData, directory[directoryIndex].payloadSize);
			nullsub_SharedNoOp();
		} else {
			pendingSkipBytes += directory[directoryIndex].payloadSize;
		}
		++soundId;
		g_fsfxLoaded = 1;
	}
	File_RawClose(archiveStream);
	return soundId - FSFX_FIRST_PLAYABLE_SLOT;
#endif
}

// FUNCTION: XW 0x40C5B0
int16_t fsfx_triggersfx(uint16_t soundId, uint16_t objectIndex) {
#ifdef XW_MODERN
	return Dos94_fsfx_triggersfx(soundId, objectIndex);
#else
	uint16_t volume;
	uint16_t priority;
	int16_t pan;
	uint8_t objectType;
	if (!ShellPreferences_GetSfxEnabled())
		return 0;
	if (!g_fsfxLoadedSoundHandles[soundId] || !g_flightSfxEnabled || !g_flightSfxVolume)
		return 0;
	objectType = g_playerFlightState.object->objectType;
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
	if (priority != FSFX_PLAYER_PRIORITY && Sound_GetParam(soundId, SOUND_PARAM_INSTANCE_COUNT)) {
		if (soundId == FSFX_NO_DUPLICATE_SLOT || Sound_GetParam(soundId, SOUND_PARAM_PRIORITY) > priority)
			return 0;
		if (soundId < FSFX_OVERLAP_FIRST_SLOT || soundId > FSFX_OVERLAP_LAST_SLOT)
			Sound_StopOldestInstanceById(soundId);
	}
	Sound_QueueEffect(g_fsfxSfxNameTable[soundId].name, 1, 0, priority, volume, pan, SOUND_GROUP_SFX);
	return 0;
#endif
}

// FUNCTION: XW 0x40C710
int16_t fsfx_triggerlasersfx(uint16_t projectileObjectIndex) {
	uint16_t objectType;
	int sfxEnabled = ShellPreferences_GetSfxEnabled();
	if (!sfxEnabled)
		return 0;
	if (!g_flightSfxEnabled)
		return 0;
	if (!g_flightSfxVolume)
		return 0;
	objectType = g_objectTable[projectileObjectIndex].objectType;
#ifdef XW_MODERN
	{
		int sound = XwFlightTypes_ProjectileSound(objectType);
		if (sound >= 0)
			return fsfx_triggersfx(sound, projectileObjectIndex);
	}
	return objectType;
#else
	{
		int projectileType = objectType;
		if (projectileType >= XW_OBJ_LASER_143 && projectileType <= XW_OBJ_TRACKED_WARHEAD)
			return fsfx_triggersfx(objectType - XW_OBJ_LASER_143 + FSFX_FIRST_PLAYABLE_SLOT, projectileObjectIndex);
		/* Unsupported types leave their type value in the legacy return register. */
		return projectileType;
	}
#endif
}

// FUNCTION: XW 0x40C780
int16_t fsfx_calcvolume(uint16_t object_idx, uint16_t sound_id) {
	uint16_t volume;
	uint16_t range;
	int32_t dx;
	int32_t dy;
	int32_t dz;
	uint32_t distance;
	if (object_idx == XW_OBJECT_SLOT_UNAVAILABLE) {
		if (sound_id >= FSFX_POSITIONAL_SLOT_COUNT)
			volume = FSFX_DEFAULT_VOLUME;
		else
			volume = g_sfxFullVolumeBySlot[sound_id];
		return volume;
	}
	if (sound_id >= FSFX_POSITIONAL_SLOT_COUNT) {
		volume = FSFX_DEFAULT_VOLUME;
		range = FSFX_DEFAULT_RANGE;
	} else {
		range = g_sfxAttenuationRangeBySlot[sound_id];
		volume = g_sfxFullVolumeBySlot[sound_id];
	}
	if (object_idx < XW_MISSION_OBJECT_REF_BASE) {
		dx = (int32_t)((uint32_t)g_objectTable[object_idx].prevWorldX -
					   (uint32_t)g_flightCamera.worldPosition.x);
		dy = (int32_t)((uint32_t)g_objectTable[object_idx].prevWorldY -
					   (uint32_t)g_flightCamera.worldPosition.y);
		dz = (int32_t)((uint32_t)g_objectTable[object_idx].prevWorldZ -
					   (uint32_t)g_flightCamera.worldPosition.z);
	} else {
		create_getworldposition(object_idx, 0);
		dx = (int32_t)((uint32_t)g_resolvedWorldX - (uint32_t)g_flightCamera.worldPosition.x);
		dy = (int32_t)((uint32_t)g_resolvedWorldY - (uint32_t)g_flightCamera.worldPosition.y);
		dz = (int32_t)((uint32_t)g_resolvedWorldZ - (uint32_t)g_flightCamera.worldPosition.z);
	}
	distance = collide_roughdistance3d(dx, dy, dz);
	if ((distance >> FSFX_FAR_DISTANCE_SHIFT) >= range)
		return 0;
	if ((distance >> FSFX_MIDDLE_DISTANCE_SHIFT) >= range)
		return volume >> FSFX_FAR_VOLUME_SHIFT;
	if (distance >= range)
		return volume >> FSFX_MIDDLE_VOLUME_SHIFT;
	volume = (range - distance) * (volume - (volume >> FSFX_MIDDLE_VOLUME_SHIFT)) /
				 (range - (range >> FSFX_NEAR_RANGE_SHIFT)) +
			 (volume >> FSFX_MIDDLE_VOLUME_SHIFT);
	if (volume > FSFX_MAX_VOLUME)
		volume = FSFX_MAX_VOLUME;
	return volume;
}

// FUNCTION: XW 0x40C8D0
int16_t fsfx_calcpan(uint16_t objectIndex, uint16_t* volume) {
	int relativeX;
	int relativeY;
	int relativeZ;
	int viewX;
	int viewZ;
	int16_t yaw;
	int16_t panAngle;
	int16_t panOffset;
	int clampedPan;
	if (objectIndex == XW_OBJECT_SLOT_UNAVAILABLE)
		return FSFX_PAN_CENTER;
	relativeX =
		(int16_t)((uint32_t)g_objectTable[objectIndex].prevWorldX - (uint32_t)g_flightCamera.worldPosition.x);
	relativeY =
		(int16_t)((uint32_t)g_objectTable[objectIndex].prevWorldY - (uint32_t)g_flightCamera.worldPosition.y);
	relativeZ =
		(int16_t)((uint32_t)g_objectTable[objectIndex].prevWorldZ - (uint32_t)g_flightCamera.worldPosition.z);
#ifdef XW_MODERN
	viewX = (int32_t)((uint32_t)g_camMatR0_X * (uint32_t)relativeX +
					  (uint32_t)g_camMatR0_Y * (uint32_t)relativeY +
					  (uint32_t)g_camMatR0_Z * (uint32_t)relativeZ);
#else
	viewX = g_camMatR0_X * relativeX + g_camMatR0_Y * relativeY + g_camMatR0_Z * relativeZ;
#endif
	if (viewX >= FSFX_TRANSFORM_CLAMP_LIMIT)
		viewX = FSFX_TRANSFORM_CLAMP_MAX;
	if (viewX <= -FSFX_TRANSFORM_CLAMP_LIMIT)
		viewX = FSFX_TRANSFORM_CLAMP_MIN;
	viewX >>= TRANSFM2_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
	viewZ = (int32_t)((uint32_t)g_camMatR2_X * (uint32_t)relativeX +
					  (uint32_t)g_camMatR2_Y * (uint32_t)relativeY +
					  (uint32_t)g_camMatR2_Z * (uint32_t)relativeZ);
#else
	viewZ = g_camMatR2_X * relativeX + g_camMatR2_Y * relativeY + g_camMatR2_Z * relativeZ;
#endif
	if (viewZ >= FSFX_TRANSFORM_CLAMP_LIMIT)
		viewZ = FSFX_TRANSFORM_CLAMP_MAX;
	if (viewZ <= -FSFX_TRANSFORM_CLAMP_LIMIT)
		viewZ = FSFX_TRANSFORM_CLAMP_MIN;
	viewZ >>= TRANSFM2_MATRIX_FRACTION_BITS;
	yaw = trig2_arctan((int16_t)viewX, (int16_t)viewZ);
	panAngle = yaw;
	if (panAngle >= TRIG2_QUARTER_TURN || panAngle <= -TRIG2_QUARTER_TURN) {
		int viewY;
		int rearPitchMagnitude;
		int rearYawMagnitude;
		int16_t rearFactor;
#ifdef XW_MODERN
		viewY = (int32_t)((uint32_t)g_camMatR1_X * (uint32_t)relativeX +
						  (uint32_t)g_camMatR1_Y * (uint32_t)relativeY +
						  (uint32_t)g_camMatR1_Z * (uint32_t)relativeZ);
#else
		viewY = g_camMatR1_X * relativeX + g_camMatR1_Y * relativeY + g_camMatR1_Z * relativeZ;
#endif
		if (viewY >= FSFX_TRANSFORM_CLAMP_LIMIT)
			viewY = FSFX_TRANSFORM_CLAMP_MAX;
		if (viewY <= -FSFX_TRANSFORM_CLAMP_LIMIT)
			viewY = FSFX_TRANSFORM_CLAMP_MIN;
		viewY >>= TRANSFM2_MATRIX_FRACTION_BITS;
		rearPitchMagnitude = trig2_arctan((int16_t)viewY, (int16_t)viewZ);
		rearPitchMagnitude = -TRIG2_ANGLE_SIGN_BIT - rearPitchMagnitude;
		rearYawMagnitude = -TRIG2_ANGLE_SIGN_BIT - yaw;
		panAngle = rearYawMagnitude;
		if ((int16_t)rearPitchMagnitude < 0)
			rearPitchMagnitude = -rearPitchMagnitude;
		if (panAngle < 0)
			rearYawMagnitude = -rearYawMagnitude;
		rearFactor = ((int16_t)(TRIG2_QUARTER_TURN - rearPitchMagnitude) >> FSFX_REAR_FACTOR_SHIFT) *
					 ((int16_t)(TRIG2_QUARTER_TURN - rearYawMagnitude) >> FSFX_REAR_FACTOR_SHIFT);
		*volume -= (int16_t)(*volume * (rearFactor / FSFX_REAR_FACTOR_DIVISOR)) / FSFX_REAR_VOLUME_DIVISOR;
	}
	panOffset = panAngle >> FSFX_PAN_ANGLE_SHIFT;
	if (panOffset >= FSFX_PAN_MIN_OFFSET) {
		clampedPan = panOffset;
	} else {
		clampedPan = FSFX_PAN_MIN_OFFSET;
		panOffset = FSFX_PAN_MIN_OFFSET;
	}
	if (panOffset > FSFX_PAN_MAX_OFFSET)
		clampedPan = FSFX_PAN_MAX_OFFSET;
	return clampedPan + FSFX_PAN_CENTER;
}

// FUNCTION: XW 0x40CAF0
void fsfx_triggergunsightsfx(uint16_t toneState) {
#ifdef XW_MODERN
	Dos94_fsfx_triggergunsightsfx(toneState);
#else
	if (!ShellPreferences_GetSfxEnabled())
		return;
	if (!g_flightSfxEnabled)
		return;
	if (!g_flightSfxVolume)
		return;
	if (toneState == FSFX_TONE_OFF || toneState == FSFX_TONE_IDLE) {
		if (Sound_GetParam(FSFX_LOCKED_TONE_SLOT, SOUND_PARAM_INSTANCE_COUNT))
			Sound_StopOldestInstanceById(FSFX_LOCKED_TONE_SLOT);
		else if (Sound_GetParam(FSFX_ACQUIRING_TONE_SLOT, SOUND_PARAM_INSTANCE_COUNT))
			Sound_StopOldestInstanceById(FSFX_ACQUIRING_TONE_SLOT);
	} else if (toneState == FSFX_TONE_LOCKED) {
		if (Sound_GetParam(FSFX_ACQUIRING_TONE_SLOT, SOUND_PARAM_INSTANCE_COUNT))
			Sound_StopOldestInstanceById(FSFX_ACQUIRING_TONE_SLOT);
		if (!Sound_GetParam(FSFX_LOCKED_TONE_SLOT, SOUND_PARAM_INSTANCE_COUNT))
			fsfx_triggersfx(FSFX_LOCKED_TONE_SLOT, XW_OBJECT_SLOT_UNAVAILABLE);
	} else {
		if (Sound_GetParam(FSFX_LOCKED_TONE_SLOT, SOUND_PARAM_INSTANCE_COUNT))
			Sound_StopOldestInstanceById(FSFX_LOCKED_TONE_SLOT);
		if (!Sound_GetParam(FSFX_ACQUIRING_TONE_SLOT, SOUND_PARAM_INSTANCE_COUNT))
			fsfx_triggersfx(FSFX_ACQUIRING_TONE_SLOT, XW_OBJECT_SLOT_UNAVAILABLE);
	}
#endif
}

// FUNCTION: XW 0x40CBE0
int16_t fsfx_triggervoicesfx(uint16_t sfxSlot) {
#ifdef XW_MODERN
	return Dos94_fsfx_triggervoicesfx(sfxSlot);
#else
	if (ShellPreferences_GetSfxEnabled() != 0 && g_flightSfxEnabled != 0 && g_flightVoiceEnabled != 0) {
		uint16_t voiceSlot = sfxSlot;
		if ((voiceSlot == FSFX_VOICE_REMAP_SLOT_1 || voiceSlot == FSFX_VOICE_REMAP_SLOT_2) &&
			g_playerFlightState.craft->craftIndexInFlightGroup == 0) {
			voiceSlot += FSFX_VOICE_LEADER_SLOT_OFFSET;
		}
		if (g_fsfxLoaded != 0 && g_fsfxLoadedSoundHandles[voiceSlot] != 0) {
			if (Sound_GetParam(g_fsfxCurrentVoiceSfxSlot, XW_SOUND_PARAM_INSTANCE_COUNT) != 0) {
				if (g_fsfxVoiceQueueCount != FSFX_VOICE_QUEUE_CAPACITY) {
					g_fsfxVoiceQueueSfxSlot[g_fsfxVoiceQueueCount++] = (uint8_t)voiceSlot;
					return 1;
				}
			} else {
				Sound_QueueVoiceBySlot(voiceSlot, FSFX_VOICE_QUEUE_PRIORITY);
				Sound_SetParam(voiceSlot, XW_SOUND_PARAM_PRIORITY, FSFX_VOICE_PLAYBACK_PRIORITY);
				Sound_SetParam(voiceSlot, XW_SOUND_PARAM_PAN, SOUND_QUEUE_PAN_CENTER);
				Sound_SetParam(voiceSlot, XW_SOUND_PARAM_VOLUME, SOUND_VOLUME_MAX);
				g_fsfxCurrentVoiceSfxSlot = voiceSlot;
				return 1;
			}
		}
	}
	return 0;
#endif
}

// FUNCTION: XW 0x40CCD0
void fsfx_checkblastqueue(void) {
#ifdef XW_MODERN
	Dos94_fsfx_checkblastqueue();
#else
	if (ShellPreferences_GetSfxEnabled() != 0 && g_fsfxLoaded != 0 && g_fsfxVoiceQueueCount != 0) {
		uint16_t currentVoice = g_fsfxCurrentVoiceSfxSlot;
		if (currentVoice == FSFX_NO_VOICE_SLOT ||
			Sound_GetParam(currentVoice, XW_SOUND_PARAM_INSTANCE_COUNT) == 0) {
			uint16_t voiceSlot = g_fsfxVoiceQueueSfxSlot[0];
			uint16_t entriesRemaining = --g_fsfxVoiceQueueCount;
			unsigned int queueIndex;
			for (queueIndex = 0; entriesRemaining > 0; ++queueIndex, --entriesRemaining)
				g_fsfxVoiceQueueSfxSlot[queueIndex] = g_fsfxVoiceQueueSfxSlot[queueIndex + 1];
			if (g_flightSfxEnabled != 0 && g_flightVoiceEnabled != 0 &&
				g_fsfxLoadedSoundHandles[voiceSlot] != 0) {
				Sound_QueueVoiceBySlot(voiceSlot, FSFX_VOICE_QUEUE_PRIORITY);
				Sound_SetParam(voiceSlot, XW_SOUND_PARAM_PRIORITY, FSFX_VOICE_PLAYBACK_PRIORITY);
				Sound_SetParam(voiceSlot, XW_SOUND_PARAM_PAN, SOUND_QUEUE_PAN_CENTER);
				Sound_SetParam(voiceSlot, XW_SOUND_PARAM_VOLUME, SOUND_VOLUME_MAX);
				g_fsfxCurrentVoiceSfxSlot = voiceSlot;
			}
		}
	}
#endif
}

// FUNCTION: XW 0x40CDB0
void fsfx_checktieflyby(void) {
	uint16_t objectIndex;
	if (!ShellPreferences_GetSfxEnabled())
		return;
	for (objectIndex = 0; objectIndex < XW_CRAFT_OBJECT_COUNT; ++objectIndex) {
		uint8_t objectType;
		CraftData* craft;
		objectType = g_objectTable[objectIndex].objectType;
#ifdef XW_MODERN
		if (XwFlightTypes_IsFlybyCraft(objectType)) {
#else
		if (objectType == XW_OBJ_TIE_FIGHTER || objectType == XW_OBJ_TIE_INTERCEPTOR ||
			objectType == XW_OBJ_TIE_BOMBER || objectType == XW_OBJ_TIE_ADVANCED) {
#endif
			craft = (CraftData*)g_objectTable[objectIndex].instanceData;
			if (craft->objectKind == XW_CRAFT_OBJECT_KIND_0 && craft->workingSubsystems &&
				g_objectTable[objectIndex].speed &&
				(uint32_t)collide_roughdistance3d((int32_t)((uint32_t)g_objectTable[objectIndex].worldX -
															(uint32_t)g_playerFlightState.object->worldX),
												  (int32_t)((uint32_t)g_objectTable[objectIndex].worldY -
															(uint32_t)g_playerFlightState.object->worldY),
												  (int32_t)((uint32_t)g_objectTable[objectIndex].worldZ -
															(uint32_t)g_playerFlightState.object->worldZ)) <
					FSFX_TIE_FLYBY_RANGE)
				fsfx_triggersfx(FSFX_TIE_FLYBY_SLOT, objectIndex);
		}
	}
}

// FUNCTION: XW 0x40CE40
int16_t fsfx_speakeravailable(void) {
	uint16_t craftIndex;
	if (ShellPreferences_GetSfxEnabled() != 0 && g_fsfxLoaded != 0) {
		if (g_deathStarSurfaceModeActive != 0) {
			return 1;
		}
		for (craftIndex = 0; craftIndex < XW_RADIO_CRAFT_SLOT_LIMIT; ++craftIndex) {
			if (craftIndex != g_playerFlightState.objectIndex &&
				g_objectTable[craftIndex].objectType != XW_OBJ_NONE &&
				g_objectTable[craftIndex].iff == g_playerFlightState.object->iff) {
				return 1;
			}
		}
	}
	return 0;
}

// FUNCTION: XW 0x40CEC0
void fsfx_UpdatePlayerEngineLoop(void) {
#ifdef XW_MODERN
	XwPlayerEngine_Update();
#else
	int soundId;
	int baseFrequency;
	uint8_t objectType;
	if (!ShellPreferences_GetSfxEnabled() || !g_flightEngineSoundEnabled)
		return;
	soundId = FSFX_ENGINE_NO_SOUND;
	if (g_playerFlightState.objectIndex != XW_OBJECT_SLOT_UNAVAILABLE && !g_playerEngineLoopSuppressed) {
		objectType = g_objectTable[g_playerFlightState.objectIndex].objectType;
		switch (objectType) {
			case XW_OBJ_X_WING:
			case XW_OBJ_B_WING:
				soundId = FSFX_ENGINE_XWING_SLOT;
				baseFrequency = FSFX_ENGINE_XWING_BASE_HZ;
				break;
			case XW_OBJ_Y_WING:
				soundId = FSFX_ENGINE_YWING_SLOT;
				baseFrequency = FSFX_ENGINE_BASE_HZ;
				break;
			case XW_OBJ_A_WING:
				soundId = FSFX_ENGINE_AWING_SLOT;
				baseFrequency = FSFX_ENGINE_BASE_HZ;
				break;
		}
	}
	if (soundId != FSFX_ENGINE_NO_SOUND) {
		CraftData* craft;
		g_playerEngineLoopObjectType = objectType;
		craft = (CraftData*)g_objectTable[g_playerFlightState.objectIndex].instanceData;
		if (g_playerFlightState.hudSuppressed != 1 &&
			(craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_ENGINE)) {
			int volume;
			uint16_t throttleFraction;
			uint16_t throttlePercent;
			int frequency;
			const char* soundName;
			volume = (uint16_t)(FSFX_ENGINE_VOLUME_MAX * g_flightSfxVolume / FSFX_ENGINE_VOLUME_SCALE);
			throttleFraction = math2_percentage(craft->engineThrottle[0], XW_CRAFT_THROTTLE_FULL);
			throttlePercent = throttleFraction / FSFX_ENGINE_THROTTLE_PERCENT_DIVISOR;
			frequency = baseFrequency + FSFX_ENGINE_HZ_PER_PERCENT * throttlePercent;
			if (!Sound_GetParam(soundId, SOUND_PARAM_INSTANCE_COUNT)) {
				soundName = g_fsfxSfxNameTable[soundId].name;
				lolevel_ImSetParam(soundName, XW_SOUND_PARAM_FREQUENCY_HZ, frequency);
				Sound_QueueEffect(soundName, 1, 1, SOUND_VOLUME_MAX, volume, FSFX_PAN_CENTER,
								  SOUND_GROUP_SFX);
			} else {
				soundName = g_fsfxSfxNameTable[soundId].name;
				lolevel_ImSetParam(soundName, XW_SOUND_PARAM_FREQUENCY_HZ, frequency);
				if (Sound_CompareOldestInstanceVolume(soundName, volume))
					Sound_SetOldestInstanceVolume(soundName, volume);
			}
		} else if (Sound_GetParam(soundId, SOUND_PARAM_INSTANCE_COUNT)) {
			Sound_StopOldestInstanceById(soundId);
		}
	} else {
		switch (g_playerEngineLoopObjectType) {
			case XW_OBJ_X_WING:
			case XW_OBJ_B_WING:
				soundId = FSFX_ENGINE_XWING_SLOT;
				break;
			case XW_OBJ_Y_WING:
				soundId = FSFX_ENGINE_YWING_SLOT;
				break;
			case XW_OBJ_A_WING:
				soundId = FSFX_ENGINE_AWING_SLOT;
				break;
		}
		if (Sound_GetParam(soundId, SOUND_PARAM_INSTANCE_COUNT))
			Sound_StopOldestInstanceById(soundId);
	}
#endif
}
