#include "xw_runtime/storage/replay_snapshot.h"
#include "xw_runtime/snapshot/render_capture.h"

#include "xw/audio/fsfx.h"
#include "xw/flight/death_star.h"
#include "xw/flight/fediskio.h"
#include "xw/flight/feinput.h"
#include "xw/flight/gate.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/laser.h"
#include "xw/flight/player/user.h"
#include "xw/flight/replay/replayio.h"
#include "xw/flight/xw.h"
#include "xw/frontend/inflight_options.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/render/flight_view.h"

#include "xw/flight/fview.h"
#include "xw/flight/object/anim.h"
#include "xw/flight/replay/replay.h"
#include "xw/flight/shell_flight.h"
#include "xw/math/math2.h"
#include "xw_runtime/runtime/flight_camera.h"
#include "xw_runtime/runtime/flight_types.h"
#include "xw_runtime/runtime/profile.h"
#include "xw_runtime/timing/flight_integration.h"
#include "xw_runtime/timing/flight_timing.h"
#include "xw_runtime/timing/player_timing.h"
#include "xw_runtime/timing/reference_motion.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

enum {
	SNAPSHOT_REFERENCE_OBJECT = 1,
	SNAPSHOT_REFERENCE_CRAFT = 2,
	SNAPSHOT_REFERENCE_GUIDANCE = 4,
	SNAPSHOT_OBJECT_ADDRESS = 0x628640,
	SNAPSHOT_CRAFT_ADDRESS = 0x634080,
	SNAPSHOT_GUIDANCE_ADDRESS = 0x62B620,
	SNAPSHOT_SURFACE_OBJECT_ADDRESS = 0x63BD00,
	SNAPSHOT_SURFACE_CRAFT_ADDRESS = 0x63BD80
};

typedef struct SnapshotCodec {
	uint8_t* bytes;
	size_t offset;
	int decoding;
	int valid;
	int checkOnly;
	int nativeReferences;
} SnapshotCodec;

typedef struct SnapshotReferenceRegion {
	void* base;
	uint32_t originalAddress;
	size_t count;
	size_t nativeStride;
	uint32_t diskStride;
	unsigned int kind;
} SnapshotReferenceRegion;

/* Original addresses are checkpoint identifiers, never host addresses.
 * Debris copies retain their source instance pointer, so validate storage
 * identity rather than selecting its type from the current object genus. */
static const SnapshotReferenceRegion snapshotReferenceRegions[] = {
	{ g_objectTable, SNAPSHOT_OBJECT_ADDRESS, XW_OBJECT_COUNT, sizeof(g_objectTable[0]),
	  REPLAYIO_DISK_OBJECT_SIZE, SNAPSHOT_REFERENCE_OBJECT },
	{ g_craftTable, SNAPSHOT_CRAFT_ADDRESS, XW_CRAFT_OBJECT_COUNT, sizeof(g_craftTable[0]),
	  REPLAYIO_DISK_CRAFT_SIZE, SNAPSHOT_REFERENCE_CRAFT },
	{ g_warheadGuidanceTable, SNAPSHOT_GUIDANCE_ADDRESS, LASER_WARHEAD_GUIDANCE_COUNT,
	  sizeof(g_warheadGuidanceTable[0]), REPLAYIO_DISK_WARHEAD_GUIDANCE_SIZE, SNAPSHOT_REFERENCE_GUIDANCE },
	{ &g_deathStarRenderObject, SNAPSHOT_SURFACE_OBJECT_ADDRESS, 1, sizeof(g_deathStarRenderObject),
	  REPLAYIO_DISK_OBJECT_SIZE, SNAPSHOT_REFERENCE_OBJECT },
	{ &g_deathStarRenderCraft, SNAPSHOT_SURFACE_CRAFT_ADDRESS, 1, sizeof(g_deathStarRenderCraft),
	  REPLAYIO_DISK_CRAFT_SIZE, SNAPSHOT_REFERENCE_CRAFT }
};

static void Snapshot_Bytes(SnapshotCodec* codec, void* field, size_t size) {
	if (!codec->bytes) {
		codec->offset += size;
		return;
	}
	if (codec->decoding) {
		if (!codec->checkOnly)
			memcpy(field, &codec->bytes[codec->offset], size);
	} else
		memcpy(&codec->bytes[codec->offset], field, size);
	codec->offset += size;
}

/* memcpy preserves signed bit patterns without aliasing or unaligned loads. */
static void Snapshot_Word(SnapshotCodec* codec, void* field) {
	if (!codec->bytes) {
		codec->offset += 2;
		return;
	}
	uint16_t value;
	if (codec->decoding) {
		value =
			(uint16_t)(codec->bytes[codec->offset] | (uint16_t)codec->bytes[codec->offset + 1] << CHAR_BIT);
		if (!codec->checkOnly)
			memcpy(field, &value, sizeof(value));
	} else {
		memcpy(&value, field, sizeof(value));
		codec->bytes[codec->offset] = (uint8_t)value;
		codec->bytes[codec->offset + 1] = (uint8_t)(value >> CHAR_BIT);
	}
	codec->offset += sizeof(value);
}

static void Snapshot_Dword(SnapshotCodec* codec, void* field) {
	if (!codec->bytes) {
		codec->offset += 4;
		return;
	}
	uint32_t value = 0;
	size_t index;
	if (codec->decoding) {
		for (index = 0; index < sizeof(value); ++index)
			value |= (uint32_t)codec->bytes[codec->offset + index] << (CHAR_BIT * index);
		if (!codec->checkOnly)
			memcpy(field, &value, sizeof(value));
	} else {
		memcpy(&value, field, sizeof(value));
		for (index = 0; index < sizeof(value); ++index)
			codec->bytes[codec->offset + index] = (uint8_t)(value >> (CHAR_BIT * index));
	}
	codec->offset += sizeof(value);
}

static void* Snapshot_Pointer(SnapshotCodec* codec, void* pointer, unsigned int allowedKinds) {
	uint32_t address = 0;
	size_t regionIndex;
	size_t index;
	int found = pointer == NULL;
	if (!codec->decoding && pointer != NULL) {
		for (regionIndex = 0;
			 regionIndex < sizeof(snapshotReferenceRegions) / sizeof(snapshotReferenceRegions[0]);
			 ++regionIndex) {
			const SnapshotReferenceRegion* region = &snapshotReferenceRegions[regionIndex];
			if ((region->kind & allowedKinds) != 0) {
				for (index = 0; index < region->count; ++index) {
					if (pointer == (uint8_t*)region->base + index * region->nativeStride) {
						address = codec->nativeReferences
									  ? ((uint32_t)(regionIndex + 1) << 24) | (uint32_t)(index + 1)
									  : region->originalAddress + (uint32_t)index * region->diskStride;
						found = 1;
					}
				}
			}
		}
		if (!found)
			codec->valid = 0;
	}
	SnapshotCodec reference = *codec;
	reference.checkOnly = 0;
	Snapshot_Dword(&reference, &address);
	codec->offset = reference.offset;
	if (!codec->decoding)
		return pointer;
	if (address == 0)
		return codec->checkOnly ? pointer : NULL;
	for (regionIndex = 0;
		 regionIndex < sizeof(snapshotReferenceRegions) / sizeof(snapshotReferenceRegions[0]);
		 ++regionIndex) {
		const SnapshotReferenceRegion* region = &snapshotReferenceRegions[regionIndex];
		if (!(region->kind & allowedKinds))
			continue;
		uint32_t slot;
		if (codec->nativeReferences) {
			if ((address >> 24) != regionIndex + 1 || !(address & 0xFFFFFF))
				continue;
			slot = (address & 0xFFFFFF) - 1;
		} else {
			if (address < region->originalAddress || (address - region->originalAddress) % region->diskStride)
				continue;
			slot = (address - region->originalAddress) / region->diskStride;
		}
		if (slot < region->count)
			return codec->checkOnly ? pointer : (uint8_t*)region->base + slot * region->nativeStride;
	}
	codec->valid = 0;
	return codec->checkOnly ? pointer : NULL;
}

/* Original record: 83 bytes; native padding is omitted. */
static void Snapshot_ObjectRecord(SnapshotCodec* codec, ObjectRecord* value) {
	Snapshot_Bytes(codec, &value->familyId, sizeof(value->familyId));
	Snapshot_Bytes(codec, &value->genusId, sizeof(value->genusId));
	Snapshot_Bytes(codec, &value->objectType, sizeof(value->objectType));
	Snapshot_Bytes(codec, &value->billboardScaleCode, sizeof(value->billboardScaleCode));
	Snapshot_Dword(codec, &value->worldX);
	Snapshot_Dword(codec, &value->worldY);
	Snapshot_Dword(codec, &value->worldZ);
	Snapshot_Dword(codec, &value->prevWorldX);
	Snapshot_Dword(codec, &value->prevWorldY);
	Snapshot_Dword(codec, &value->prevWorldZ);
	Snapshot_Word(codec, &value->yaw);
	Snapshot_Word(codec, &value->pitch);
	Snapshot_Word(codec, &value->roll);
	Snapshot_Word(codec, &value->rollImpulseRate);
	Snapshot_Word(codec, &value->speed);
	Snapshot_Word(codec, &value->speedFractionQ16);
	Snapshot_Word(codec, &value->damageAmount);
	Snapshot_Word(codec, &value->lifetimeTicks);
	Snapshot_Word(codec, &value->ageSeconds);
	Snapshot_Word(codec, &value->sourceObjectRef);
	Snapshot_Bytes(codec, &value->sourceObjectType, sizeof(value->sourceObjectType));
	Snapshot_Bytes(codec, &value->iff, sizeof(value->iff));
	Snapshot_Bytes(codec, &value->markings, sizeof(value->markings));
	Snapshot_Bytes(codec, &value->animationState, sizeof(value->animationState));
	Snapshot_Bytes(codec, &value->secondaryAnimationState, sizeof(value->secondaryAnimationState));
	Snapshot_Bytes(codec, &value->moveVectorDirty, sizeof(value->moveVectorDirty));
	Snapshot_Word(codec, &value->moveX);
	Snapshot_Word(codec, &value->moveY);
	Snapshot_Word(codec, &value->moveZ);
	Snapshot_Bytes(codec, &value->orientMatrixDirty, sizeof(value->orientMatrixDirty));
	Snapshot_Word(codec, &value->cachedForwardX);
	Snapshot_Word(codec, &value->cachedForwardY);
	Snapshot_Word(codec, &value->cachedForwardZ);
	Snapshot_Word(codec, &value->cachedSideX);
	Snapshot_Word(codec, &value->cachedSideY);
	Snapshot_Word(codec, &value->cachedSideZ);
	Snapshot_Word(codec, &value->cachedUpX);
	Snapshot_Word(codec, &value->cachedUpY);
	Snapshot_Word(codec, &value->cachedUpZ);
	value->instanceData =
		Snapshot_Pointer(codec, value->instanceData, SNAPSHOT_REFERENCE_CRAFT | SNAPSHOT_REFERENCE_GUIDANCE);
}

/* Original record: 24 bytes; native padding is omitted. */
static void Snapshot_CraftLaserState(SnapshotCodec* codec, CraftLaserState* value) {
	size_t index;
	Snapshot_Bytes(codec, &value->projectileTypeId, sizeof(value->projectileTypeId));
	Snapshot_Bytes(codec, &value->linkMode, sizeof(value->linkMode));
	Snapshot_Bytes(codec, &value->burstShotsRemaining, sizeof(value->burstShotsRemaining));
	Snapshot_Bytes(codec, &value->nextSlot, sizeof(value->nextSlot));
	for (index = 0; index < sizeof(value->fireCooldownTicks) / sizeof(value->fireCooldownTicks[0]); ++index) {
		Snapshot_Word(codec, &value->fireCooldownTicks[index]);
	}
}

/* Original record: 15 bytes; native padding is omitted. */
static void Snapshot_XwWeaponHitStats(SnapshotCodec* codec, XwWeaponHitStats* value) {
	Snapshot_Word(codec, &value->laserShotsFired);
	Snapshot_Word(codec, &value->laserSpacecraftHits);
	Snapshot_Word(codec, &value->laserSurfaceHits);
	Snapshot_Word(codec, &value->ionShotsFired);
	Snapshot_Word(codec, &value->ionSpacecraftHits);
	Snapshot_Word(codec, &value->ionSurfaceHits);
	Snapshot_Bytes(codec, &value->warheadsFired, sizeof(value->warheadsFired));
	Snapshot_Bytes(codec, &value->warheadSpacecraftHits, sizeof(value->warheadSpacecraftHits));
	Snapshot_Bytes(codec, &value->warheadSurfaceHits, sizeof(value->warheadSurfaceHits));
}

/* Original record: 28 bytes; native padding is omitted. */
static void Snapshot_XwCraftKillStats(SnapshotCodec* codec, XwCraftKillStats* value) {
	Snapshot_Bytes(codec, &value->spacecraftByType, sizeof(value->spacecraftByType));
	Snapshot_Word(codec, &value->spaceObjects);
	Snapshot_Word(codec, &value->deathStarBuildings);
}

/* Original record: 4 bytes; native padding is omitted. */
static void Snapshot_CraftWeaponSlot(SnapshotCodec* codec, CraftWeaponSlot* value) {
	Snapshot_Word(codec, &value->firingGate);
	Snapshot_Bytes(codec, &value->laserCharge, sizeof(value->laserCharge));
	Snapshot_Bytes(codec, &value->count, sizeof(value->count));
}

/* Original record: 462 bytes; native padding is omitted. */
static void Snapshot_CraftData(SnapshotCodec* codec, CraftData* value) {
	size_t index;
	Snapshot_Bytes(codec, &value->craftTypeIndex, sizeof(value->craftTypeIndex));
	Snapshot_Bytes(codec, &value->field01, sizeof(value->field01));
	Snapshot_Word(codec, &value->aiSkillQ16);
	Snapshot_Bytes(codec, &value->field04, sizeof(value->field04));
	Snapshot_Bytes(codec, &value->flightGroupIndex, sizeof(value->flightGroupIndex));
	Snapshot_Bytes(codec, &value->aiLeaderObjectIndex, sizeof(value->aiLeaderObjectIndex));
	Snapshot_Bytes(codec, &value->gap07, sizeof(value->gap07));
	Snapshot_Word(codec, &value->pitch);
	Snapshot_Word(codec, &value->yaw);
	Snapshot_Bytes(codec, &value->field0D, sizeof(value->field0D));
	Snapshot_Dword(codec, &value->viewX);
	Snapshot_Dword(codec, &value->viewY);
	Snapshot_Dword(codec, &value->viewZ);
	Snapshot_Bytes(codec, &value->field1D, sizeof(value->field1D));
	Snapshot_Bytes(codec, &value->sFoilState, sizeof(value->sFoilState));
	Snapshot_Bytes(codec, &value->aiCurrentPlanId, sizeof(value->aiCurrentPlanId));
	Snapshot_Bytes(codec, &value->aiOrderPlanId, sizeof(value->aiOrderPlanId));
	Snapshot_Bytes(codec, &value->aiWaypointIndex, sizeof(value->aiWaypointIndex));
	Snapshot_Bytes(codec, &value->aiSavedPlanId, sizeof(value->aiSavedPlanId));
	Snapshot_Word(codec, &value->aiThinkIntervalTicks);
	Snapshot_Word(codec, &value->aiThinkTimerTicks);
	Snapshot_Word(codec, &value->aiTargetRef);
	Snapshot_Dword(codec, &value->aiAimPointX);
	Snapshot_Dword(codec, &value->aiAimPointY);
	Snapshot_Dword(codec, &value->aiAimPointZ);
	Snapshot_Word(codec, &value->aiCandidateTargetOrSavedInterval);
	Snapshot_Bytes(codec, &value->aiEscortTargetFlightGroup, sizeof(value->aiEscortTargetFlightGroup));
	Snapshot_Word(codec, &value->aiOrderParameter);
	Snapshot_Word(codec, &value->lastAttackerObjIdx);
	Snapshot_Bytes(codec, &value->aiOrderProgress, sizeof(value->aiOrderProgress));
	Snapshot_Bytes(codec, &value->aiWarheadsFiredThisRun, sizeof(value->aiWarheadsFiredThisRun));
	Snapshot_Bytes(codec, &value->aiHitsThisManeuver, sizeof(value->aiHitsThisManeuver));
	Snapshot_Bytes(codec, &value->objectKind, sizeof(value->objectKind));
	Snapshot_Bytes(codec, &value->aiManeuverId, sizeof(value->aiManeuverId));
	Snapshot_Bytes(codec, &value->aiManeuverPhase, sizeof(value->aiManeuverPhase));
	Snapshot_Word(codec, &value->aiManeuverTimerTicks);
	Snapshot_Word(codec, &value->aiManeuverAuxTimerTicks);
	Snapshot_Word(codec, &value->maxSpeed);
	Snapshot_Word(codec, &value->field48);
	Snapshot_Bytes(codec, &value->aiClimbState, sizeof(value->aiClimbState));
	Snapshot_Bytes(codec, &value->aiDiveState, sizeof(value->aiDiveState));
	Snapshot_Word(codec, &value->pitchRateLimit);
	Snapshot_Word(codec, &value->aiPitchRateScaleQ16);
	Snapshot_Bytes(codec, &value->aiPitchState, sizeof(value->aiPitchState));
	Snapshot_Bytes(codec, &value->aiPitchForce, sizeof(value->aiPitchForce));
	Snapshot_Word(codec, &value->aiTargetPitch);
	Snapshot_Word(codec, &value->aiPitchStepQ16);
	Snapshot_Word(codec, &value->rollRateLimit);
	Snapshot_Word(codec, &value->aiRollRateScaleQ16);
	Snapshot_Bytes(codec, &value->aiRollState, sizeof(value->aiRollState));
	Snapshot_Word(codec, &value->aiTargetRoll);
	Snapshot_Word(codec, &value->aiRollStepQ16);
	Snapshot_Word(codec, &value->yawRateLimit);
	Snapshot_Word(codec, &value->aiYawRateScaleQ16);
	Snapshot_Bytes(codec, &value->aiYawState, sizeof(value->aiYawState));
	Snapshot_Word(codec, &value->aiTargetYaw);
	Snapshot_Word(codec, &value->aiYawStepQ16);
	Snapshot_Bytes(codec, &value->aiFormationType, sizeof(value->aiFormationType));
	Snapshot_Bytes(codec, &value->aiFormationSpacing, sizeof(value->aiFormationSpacing));
	Snapshot_Bytes(codec, &value->craftIndexInFlightGroup, sizeof(value->craftIndexInFlightGroup));
	Snapshot_Dword(codec, &value->aiDisplacementX);
	Snapshot_Dword(codec, &value->aiDisplacementY);
	Snapshot_Dword(codec, &value->aiDisplacementZ);
	for (index = 0; index < sizeof(value->engineThrottle) / sizeof(value->engineThrottle[0]); ++index) {
		Snapshot_Word(codec, &value->engineThrottle[index]);
	}
	for (index = 0; index < sizeof(value->enginePowerScaleQ16) / sizeof(value->enginePowerScaleQ16[0]);
		 ++index) {
		Snapshot_Word(codec, &value->enginePowerScaleQ16[index]);
	}
	for (index = 0; index < sizeof(value->engineOutputQ16) / sizeof(value->engineOutputQ16[0]); ++index) {
		Snapshot_Word(codec, &value->engineOutputQ16[index]);
	}
	Snapshot_Word(codec, &value->hullDamage);
	Snapshot_Word(codec, &value->systemDamageHullThreshold);
	Snapshot_Word(codec, &value->hullMax);
	Snapshot_Bytes(codec, &value->activeHudFeatureMask, sizeof(value->activeHudFeatureMask));
	Snapshot_Bytes(codec, &value->workingSubsystems, sizeof(value->workingSubsystems));
	Snapshot_Word(codec, &value->cockpitOverlayMask);
	Snapshot_Word(codec, &value->field99);
	Snapshot_Word(codec, &value->field9B);
	Snapshot_Bytes(codec, &value->field9D, sizeof(value->field9D));
	Snapshot_Bytes(codec, &value->captorFlightGroupOverride, sizeof(value->captorFlightGroupOverride));
	Snapshot_Bytes(codec, &value->isInspected, sizeof(value->isInspected));
	Snapshot_Bytes(codec, &value->boardingState, sizeof(value->boardingState));
	Snapshot_Bytes(codec, &value->cargoName, sizeof(value->cargoName));
	for (index = 0; index < sizeof(value->shieldEnergy) / sizeof(value->shieldEnergy[0]); ++index) {
		Snapshot_Word(codec, &value->shieldEnergy[index]);
	}
	Snapshot_Bytes(codec, &value->shieldRedirect, sizeof(value->shieldRedirect));
	Snapshot_Bytes(codec, &value->shieldDistribMode, sizeof(value->shieldDistribMode));
	Snapshot_Bytes(codec, &value->cannonClassCount, sizeof(value->cannonClassCount));
	Snapshot_Bytes(codec, &value->laserRedirect, sizeof(value->laserRedirect));
	Snapshot_Bytes(codec, &value->laserSlotCount, sizeof(value->laserSlotCount));
	Snapshot_CraftLaserState(codec, &value->laserState);
	Snapshot_Bytes(codec, &value->warheadLauncherCount, sizeof(value->warheadLauncherCount));
	Snapshot_Bytes(codec, &value->warheadSlotTypeIds, sizeof(value->warheadSlotTypeIds));
	Snapshot_Bytes(codec, &value->warheadLauncherFlags, sizeof(value->warheadLauncherFlags));
	for (index = 0; index < sizeof(value->warheadFireTimers) / sizeof(value->warheadFireTimers[0]); ++index) {
		Snapshot_Word(codec, &value->warheadFireTimers[index]);
	}
	Snapshot_Word(codec, &value->warheadLockTicks);
	Snapshot_XwWeaponHitStats(codec, &value->weaponStats);
	Snapshot_XwCraftKillStats(codec, &value->killStats);
	for (index = 0; index < sizeof(value->weaponSlots) / sizeof(value->weaponSlots[0]); ++index) {
		Snapshot_CraftWeaponSlot(codec, &value->weaponSlots[index]);
	}
	Snapshot_Bytes(codec, &value->componentState, sizeof(value->componentState));
	Snapshot_Bytes(codec, &value->meshRotation, sizeof(value->meshRotation));
	Snapshot_Bytes(codec, &value->componentHp, sizeof(value->componentHp));
}

/* Original record: 15 bytes; native padding is omitted. */
static void Snapshot_XwMissionObjectRecord(SnapshotCodec* codec, XwMissionObjectRecord* value) {
	Snapshot_Bytes(codec, &value->field_00, sizeof(value->field_00));
	Snapshot_Bytes(codec, &value->genusId, sizeof(value->genusId));
	Snapshot_Bytes(codec, &value->objectType, sizeof(value->objectType));
	Snapshot_Word(codec, &value->worldX);
	Snapshot_Word(codec, &value->worldY);
	Snapshot_Word(codec, &value->worldZ);
	Snapshot_Bytes(codec, &value->yawAngle8, sizeof(value->yawAngle8));
	Snapshot_Bytes(codec, &value->pitchAngle8, sizeof(value->pitchAngle8));
	Snapshot_Bytes(codec, &value->rollAngle8, sizeof(value->rollAngle8));
	Snapshot_Bytes(codec, &value->goalFlags, sizeof(value->goalFlags));
	Snapshot_Bytes(codec, &value->stateByte, sizeof(value->stateByte));
	Snapshot_Bytes(codec, &value->typeSpecificByte, sizeof(value->typeSpecificByte));
}

/* Original record: 3 bytes; native padding is omitted. */
static void Snapshot_WarheadGuidanceState(SnapshotCodec* codec, WarheadGuidanceState* value) {
	Snapshot_Bytes(codec, &value->homingTier, sizeof(value->homingTier));
	Snapshot_Word(codec, &value->targetObjIdx);
}

/* Original record: 8 bytes; native padding is omitted. */
static void Snapshot_XwMissionClock(SnapshotCodec* codec, XwMissionClock* value) {
	Snapshot_Bytes(codec, &value->gap_0, sizeof(value->gap_0));
	Snapshot_Bytes(codec, &value->hours, sizeof(value->hours));
	Snapshot_Bytes(codec, &value->minutes, sizeof(value->minutes));
	Snapshot_Bytes(codec, &value->seconds, sizeof(value->seconds));
	Snapshot_Word(codec, &value->subsecondTicks);
}

/* Original record: 206 bytes; native padding is omitted. */
static void Snapshot_XwMissionHeader(SnapshotCodec* codec, XwMissionHeader* value) {
	Snapshot_Word(codec, &value->field_00);
	Snapshot_Word(codec, &value->timeLimitMinutes);
	Snapshot_Bytes(codec, &value->missionRuleFlags, sizeof(value->missionRuleFlags));
	Snapshot_Bytes(codec, &value->field_05, sizeof(value->field_05));
	Snapshot_Word(codec, &value->backdropSeed);
	Snapshot_Bytes(codec, &value->surfaceMode, sizeof(value->surfaceMode));
	Snapshot_Bytes(codec, &value->field_09, sizeof(value->field_09));
	Snapshot_Bytes(codec, &value->completionMessages, sizeof(value->completionMessages));
	Snapshot_Word(codec, &value->flightGroupCount);
	Snapshot_Word(codec, &value->objectRecordCount);
}

/* Original record: 20 bytes; native padding is omitted. */
static void Snapshot_MissionFlightGroupState(SnapshotCodec* codec, MissionFlightGroupState* value) {
	Snapshot_Bytes(codec, &value->hasArrived, sizeof(value->hasArrived));
	Snapshot_Bytes(codec, &value->wavesRemaining, sizeof(value->wavesRemaining));
	Snapshot_Word(codec, &value->arrivalDelayTimer);
	Snapshot_Bytes(codec, &value->arrivalDelayPending, sizeof(value->arrivalDelayPending));
	Snapshot_Bytes(codec, &value->gap05, sizeof(value->gap05));
	Snapshot_Word(codec, &value->currentWaypointRef);
	Snapshot_Bytes(codec, &value->spawnedCraftCount, sizeof(value->spawnedCraftCount));
	Snapshot_Bytes(codec, &value->outcomes, sizeof(value->outcomes));
	Snapshot_Bytes(codec, &value->destroyedCount, sizeof(value->destroyedCount));
	Snapshot_Bytes(codec, &value->specialCraftDestroyed, sizeof(value->specialCraftDestroyed));
	Snapshot_Bytes(codec, &value->inspectedCount, sizeof(value->inspectedCount));
	Snapshot_Bytes(codec, &value->specialCraftInspected, sizeof(value->specialCraftInspected));
}

/* Original record: 6 bytes; native padding is omitted. */
static void Snapshot_XwMissionObjectGoalResults(SnapshotCodec* codec, XwMissionObjectGoalResults* value) {
	Snapshot_Bytes(codec, &value->protectMines, sizeof(value->protectMines));
	Snapshot_Bytes(codec, &value->destroyMines, sizeof(value->destroyMines));
	Snapshot_Bytes(codec, &value->protectSatellites, sizeof(value->protectSatellites));
	Snapshot_Bytes(codec, &value->destroySatellites, sizeof(value->destroySatellites));
	Snapshot_Bytes(codec, &value->protectProbes, sizeof(value->protectProbes));
	Snapshot_Bytes(codec, &value->destroyProbes, sizeof(value->destroyProbes));
}

/* Original record: 306 bytes; native padding is omitted. */
static void Snapshot_XwMissionRuntimeState(SnapshotCodec* codec, XwMissionRuntimeState* value) {
	size_t index;
	Snapshot_Bytes(codec, &value->provingGroundsActive, sizeof(value->provingGroundsActive));
	Snapshot_Bytes(codec, &value->provingGroundsSelectedCraft, sizeof(value->provingGroundsSelectedCraft));
	Snapshot_Bytes(codec, &value->provingGroundsLevel, sizeof(value->provingGroundsLevel));
	Snapshot_Dword(codec, &value->provingGroundsScore);
	Snapshot_Word(codec, &value->provingGroundsCurrentCheckpointIndex);
	Snapshot_Word(codec, &value->provingGroundsCheckpointsPassed);
	Snapshot_Word(codec, &value->provingGroundsCheckpointsMissed);
	Snapshot_Word(codec, &value->provingGroundsCheckpointsRemaining);
	Snapshot_Word(codec, &value->provingGroundsTargetsDestroyed);
	Snapshot_Word(codec, &value->provingGroundsTimeBonus);
	Snapshot_Bytes(codec, &value->inspectedCountsByCategory, sizeof(value->inspectedCountsByCategory));
	Snapshot_Bytes(codec, &value->captureCountsByIffAndType, sizeof(value->captureCountsByIffAndType));
	Snapshot_Bytes(codec, &value->destroyedCountsByIffAndCategory,
				   sizeof(value->destroyedCountsByIffAndCategory));
	Snapshot_Bytes(codec, &value->flightBadgeAnnouncement, sizeof(value->flightBadgeAnnouncement));
	Snapshot_Bytes(codec, &value->newBattlePatch, sizeof(value->newBattlePatch));
	Snapshot_Bytes(codec, &value->newRank, sizeof(value->newRank));
	Snapshot_Bytes(codec, &value->newMedal, sizeof(value->newMedal));
	Snapshot_Bytes(codec, &value->tourCutsceneIndex, sizeof(value->tourCutsceneIndex));
	Snapshot_Bytes(codec, &value->mode, sizeof(value->mode));
	Snapshot_Bytes(codec, &value->flightExitReason, sizeof(value->flightExitReason));
	Snapshot_Bytes(codec, &value->flightExitRequested, sizeof(value->flightExitRequested));
	Snapshot_Bytes(codec, &value->gap_C3, sizeof(value->gap_C3));
	Snapshot_Bytes(codec, &value->objectivesCompleted, sizeof(value->objectivesCompleted));
	Snapshot_Bytes(codec, &value->gap_C5, sizeof(value->gap_C5));
	Snapshot_Bytes(codec, &value->objectivesUnfinishable, sizeof(value->objectivesUnfinishable));
	Snapshot_Bytes(codec, &value->flightGroupGoalState, sizeof(value->flightGroupGoalState));
	Snapshot_XwMissionObjectGoalResults(codec, &value->objectGoalResults);
	for (index = 0; index < sizeof(value->snapshotCraftDisplayDistances) /
								sizeof(value->snapshotCraftDisplayDistances[0]);
		 ++index) {
		Snapshot_Word(codec, &value->snapshotCraftDisplayDistances[index]);
	}
	Snapshot_Bytes(codec, &value->snapshotCraftStatusCodes, sizeof(value->snapshotCraftStatusCodes));
}

/* Original record: 4 bytes; native padding is omitted. */
static void Snapshot_XwPlayerSmoothedTurnInput(SnapshotCodec* codec, XwPlayerSmoothedTurnInput* value) {
	Snapshot_Word(codec, &value->roll);
	Snapshot_Word(codec, &value->pitch);
}

/* Original record: 12 bytes; native padding is omitted. */
static void Snapshot_XwCameraPosition(SnapshotCodec* codec, XwCameraPosition* value) {
	Snapshot_Dword(codec, &value->x);
	Snapshot_Dword(codec, &value->y);
	Snapshot_Dword(codec, &value->z);
}

/* Original record: 180 bytes; native padding is omitted. */
static void Snapshot_XwPlayerFlightState(SnapshotCodec* codec, XwPlayerFlightState* value) {
	size_t index;
	value->object = Snapshot_Pointer(codec, value->object, SNAPSHOT_REFERENCE_OBJECT);
	value->craft = Snapshot_Pointer(codec, value->craft, SNAPSHOT_REFERENCE_CRAFT);
	Snapshot_Word(codec, &value->objectIndex);
	Snapshot_Bytes(codec, &value->flightGroupIndex, sizeof(value->flightGroupIndex));
	Snapshot_Bytes(codec, &value->hudSuppressed, sizeof(value->hudSuppressed));
	Snapshot_Bytes(codec, &value->missionExitHullDamageQuarter, sizeof(value->missionExitHullDamageQuarter));
	Snapshot_Bytes(codec, &value->craftTypeIndex, sizeof(value->craftTypeIndex));
	Snapshot_Bytes(codec, &value->engineCount, sizeof(value->engineCount));
	Snapshot_Bytes(codec, &value->hudTargetDetailsEnabled, sizeof(value->hudTargetDetailsEnabled));
	Snapshot_Word(codec, &value->currentTargetObjectIdx);
	Snapshot_Word(codec, &value->previousTargetObjectIdx);
	for (index = 0; index < USER_TARGET_MEMORY_COUNT; ++index) {
		Snapshot_Word(codec, &value->savedTargetRefs[index]);
	}
	Snapshot_Bytes(codec, &value->missileLockState, sizeof(value->missileLockState));
	Snapshot_Bytes(codec, &value->selectedWeaponBank, sizeof(value->selectedWeaponBank));
	Snapshot_Bytes(codec, &value->selectedWeaponMode, sizeof(value->selectedWeaponMode));
	Snapshot_Bytes(codec, &value->incomingWarheadAlertState, sizeof(value->incomingWarheadAlertState));
	Snapshot_Word(codec, &value->incomingWarheadObjectIndex);
	Snapshot_Word(codec, &value->previousRollModifierMode);
	Snapshot_XwPlayerSmoothedTurnInput(codec, &value->smoothedTurnInput);
	Snapshot_Word(codec, &value->savedKeyModifiers);
	Snapshot_Word(codec, &value->targetButtonHoldTicks);
	Snapshot_XwWeaponHitStats(codec, &value->weaponStats);
	for (index = 0; index < sizeof(value->spacecraftKillsByType) / sizeof(value->spacecraftKillsByType[0]);
		 ++index) {
		Snapshot_Word(codec, &value->spacecraftKillsByType[index]);
	}
	Snapshot_Word(codec, &value->spaceObjectKills);
	Snapshot_Word(codec, &value->deathStarBuildingKills);
	Snapshot_Bytes(codec, &value->savedSubsystemRepairPriority, sizeof(value->savedSubsystemRepairPriority));
	Snapshot_Bytes(codec, &value->gap_77, sizeof(value->gap_77));
	for (index = 0; index < sizeof(value->subsystemHealth) / sizeof(value->subsystemHealth[0]); ++index) {
		Snapshot_Word(codec, &value->subsystemHealth[index]);
	}
	Snapshot_Bytes(codec, &value->gap_88, sizeof(value->gap_88));
	for (index = 0; index < sizeof(value->subsystemRepairTimers) / sizeof(value->subsystemRepairTimers[0]);
		 ++index) {
		Snapshot_Word(codec, &value->subsystemRepairTimers[index]);
	}
	Snapshot_Bytes(codec, &value->gap_9A, sizeof(value->gap_9A));
	Snapshot_XwCameraPosition(codec, &value->rotatedCockpitOffset);
	Snapshot_XwCameraPosition(codec, &value->previousRotatedCockpitOffset);
}

/* Original record: 362 bytes; native padding is omitted. */
static void Snapshot_XwCameraAngleHistory(SnapshotCodec* codec, XwCameraAngleHistory* value) {
	size_t index;
	for (index = 0; index < sizeof(value->roll) / sizeof(value->roll[0]); ++index) {
		Snapshot_Word(codec, &value->roll[index]);
	}
	for (index = 0; index < sizeof(value->pitch) / sizeof(value->pitch[0]); ++index) {
		Snapshot_Word(codec, &value->pitch[index]);
	}
	for (index = 0; index < sizeof(value->yaw) / sizeof(value->yaw[0]); ++index) {
		Snapshot_Word(codec, &value->yaw[index]);
	}
	Snapshot_Word(codec, &value->writeIndex);
}

/* Original record: 405 bytes; native padding is omitted. */
static void Snapshot_XwFlightCamera(SnapshotCodec* codec, XwFlightCamera* value) {
	Snapshot_XwCameraPosition(codec, &value->worldPosition);
	Snapshot_Word(codec, &value->focusObjectRef);
	Snapshot_Word(codec, &value->viewPitch);
	Snapshot_Word(codec, &value->viewYaw);
	Snapshot_Word(codec, &value->viewRoll);
	Snapshot_Word(codec, &value->viewAngleD);
	Snapshot_Word(codec, &value->hudAimX);
	Snapshot_Word(codec, &value->hudAimY);
	Snapshot_Bytes(codec, &value->hudStateLive, sizeof(value->hudStateLive));
	Snapshot_Bytes(codec, &value->hudStateMirror, sizeof(value->hudStateMirror));
	Snapshot_Bytes(codec, &value->rearViewHudOffset, sizeof(value->rearViewHudOffset));
	Snapshot_Bytes(codec, &value->savedHudState, sizeof(value->savedHudState));
	Snapshot_Bytes(codec, &value->field_1E, sizeof(value->field_1E));
	Snapshot_Word(codec, &value->savedHudAimX);
	Snapshot_Word(codec, &value->savedHudAimY);
	Snapshot_Word(codec, &value->manualControlActive);
	Snapshot_Word(codec, &value->movementStep);
	Snapshot_Word(codec, &value->externalViewActive);
	Snapshot_Word(codec, &value->externalDistance);
	Snapshot_XwCameraAngleHistory(codec, &value->angleHistory);
}

/* Original record: 20 bytes; native padding is omitted. */
static void Snapshot_XwFlightGlobalCountdownTimers(SnapshotCodec* codec,
												   XwFlightGlobalCountdownTimers* value) {
	unsigned int timerIndex;
	for (timerIndex = 0; timerIndex < XW_GLOBAL_COUNTDOWN_TIMER_COUNT; ++timerIndex)
		Snapshot_Word(codec, &value->ticks[timerIndex]);
}

/* Original record: 16 bytes; native padding is omitted. */
static void Snapshot_XwSurfaceCellDamageState(SnapshotCodec* codec, XwSurfaceCellDamageState* value) {
	Snapshot_Word(codec, &value->cellKey);
	Snapshot_Bytes(codec, &value->objectHealthOrEffectState, sizeof(value->objectHealthOrEffectState));
}

/* Original record: 6 bytes; native padding is omitted. */
static void Snapshot_XwSurfaceGunCell(SnapshotCodec* codec, XwSurfaceGunCell* value) {
	Snapshot_Word(codec, &value->cellX);
	Snapshot_Word(codec, &value->cellY);
	Snapshot_Word(codec, &value->fireCountdownTicks);
}

/* Original record: 148 bytes; native padding is omitted. */
static void Snapshot_XwMissionFlightGroup(SnapshotCodec* codec, XwMissionFlightGroup* value) {
	size_t index;
	Snapshot_Bytes(codec, &value->name, sizeof(value->name));
	Snapshot_Bytes(codec, &value->cargo, sizeof(value->cargo));
	Snapshot_Bytes(codec, &value->specialCargo, sizeof(value->specialCargo));
	Snapshot_Word(codec, &value->specialCraftIndex);
	Snapshot_Word(codec, &value->craftType);
	Snapshot_Word(codec, &value->iffOverride);
	Snapshot_Word(codec, &value->initialStatus);
	Snapshot_Word(codec, &value->numberOfCraft);
	Snapshot_Word(codec, &value->additionalWaveCount);
	Snapshot_Word(codec, &value->arrivalCondition);
	Snapshot_Word(codec, &value->arrivalDelay);
	Snapshot_Word(codec, &value->arrivalTriggerFlightGroupIndex);
	Snapshot_Word(codec, &value->mothershipFlightGroupIndex);
	Snapshot_Word(codec, &value->arrivalMethod);
	Snapshot_Word(codec, &value->departureMethod);
	for (index = 0; index < sizeof(value->waypointX) / sizeof(value->waypointX[0]); ++index) {
		Snapshot_Word(codec, &value->waypointX[index]);
	}
	for (index = 0; index < sizeof(value->waypointY) / sizeof(value->waypointY[0]); ++index) {
		Snapshot_Word(codec, &value->waypointY[index]);
	}
	for (index = 0; index < sizeof(value->waypointZ) / sizeof(value->waypointZ[0]); ++index) {
		Snapshot_Word(codec, &value->waypointZ[index]);
	}
	for (index = 0; index < sizeof(value->waypointEnabled) / sizeof(value->waypointEnabled[0]); ++index) {
		Snapshot_Word(codec, &value->waypointEnabled[index]);
	}
	Snapshot_Word(codec, &value->formation);
	Snapshot_Word(codec, &value->playerCraftOrdinalPlusOne);
	Snapshot_Word(codec, &value->aiLevel);
	Snapshot_Word(codec, &value->order);
	Snapshot_Word(codec, &value->throttlePreset);
	Snapshot_Word(codec, &value->markings);
	Snapshot_Word(codec, &value->field_8C);
	Snapshot_Word(codec, &value->missionGoal);
	Snapshot_Word(codec, &value->primaryTarget);
	Snapshot_Word(codec, &value->secondaryTarget);
}

static int Snapshot_Block(SnapshotCodec* codec, unsigned int blockIndex) {
	size_t index;
	switch (blockIndex) {
		case REPLAYIO_SNAPSHOT_OBJECT_TABLE:
			for (index = 0; index < sizeof(g_objectTable) / sizeof(g_objectTable[0]); ++index) {
				Snapshot_ObjectRecord(codec, &g_objectTable[index]);
			}
			break;
		case REPLAYIO_SNAPSHOT_MISSION_OBJECTS:
			for (index = 0; index < sizeof(g_missionObjects) / sizeof(g_missionObjects[0]); ++index) {
				Snapshot_XwMissionObjectRecord(codec, &g_missionObjects[index]);
			}
			break;
		case REPLAYIO_SNAPSHOT_CRAFT_TABLE:
			for (index = 0; index < sizeof(g_craftTable) / sizeof(g_craftTable[0]); ++index) {
				Snapshot_CraftData(codec, &g_craftTable[index]);
			}
			break;
		case REPLAYIO_SNAPSHOT_WARHEAD_GUIDANCE_TABLE:
			for (index = 0; index < sizeof(g_warheadGuidanceTable) / sizeof(g_warheadGuidanceTable[0]);
				 ++index) {
				Snapshot_WarheadGuidanceState(codec, &g_warheadGuidanceTable[index]);
			}
			break;
		case REPLAYIO_SNAPSHOT_MISSION_ELAPSED_CLOCK:
			Snapshot_XwMissionClock(codec, &g_missionElapsedClock);
			break;
		case REPLAYIO_SNAPSHOT_MISSION_COUNTDOWN_CLOCK:
			Snapshot_XwMissionClock(codec, &g_missionCountdownClock);
			break;
		case REPLAYIO_SNAPSHOT_MISSION_HEADER:
			Snapshot_XwMissionHeader(codec, &g_missionHeader);
			break;
		case REPLAYIO_SNAPSHOT_MISSION_FLIGHT_GROUP_STATES:
			for (index = 0;
				 index < sizeof(g_missionFlightGroupStates) / sizeof(g_missionFlightGroupStates[0]);
				 ++index) {
				Snapshot_MissionFlightGroupState(codec, &g_missionFlightGroupStates[index]);
			}
			break;
		case REPLAYIO_SNAPSHOT_MISSION_RUNTIME_STATE:
			Snapshot_XwMissionRuntimeState(codec, &g_missionRuntimeState);
			break;
		case REPLAYIO_SNAPSHOT_PLAYER_FLIGHT_STATE:
			Snapshot_XwPlayerFlightState(codec, &g_playerFlightState);
			break;
		case REPLAYIO_SNAPSHOT_FLIGHT_CAMERA:
			Snapshot_XwFlightCamera(codec, &g_flightCamera);
			break;
		case REPLAYIO_SNAPSHOT_FIELD_62D12A:
			Snapshot_Word(codec, &g_snapshotField62D12A);
			break;
		case REPLAYIO_SNAPSHOT_FIELD_62D0E2:
			Snapshot_Word(codec, &g_snapshotField62D0E2);
			break;
		case REPLAYIO_SNAPSHOT_FIELD_62D118:
			Snapshot_Bytes(codec, &g_snapshotField62D118, sizeof(g_snapshotField62D118));
			break;
		case REPLAYIO_SNAPSHOT_TARGET_HIGHLIGHT_OBJECT_AND_BLINK_BITS:
			Snapshot_Word(codec, &g_targetHighlightObjectAndBlinkBits);
			break;
		case REPLAYIO_SNAPSHOT_TARGET_HIGHLIGHT_BLINK_TICKS:
			Snapshot_Word(codec, &g_targetHighlightBlinkTicks);
			break;
		case REPLAYIO_SNAPSHOT_FLIGHT_GLOBAL_COUNTDOWN_TIMERS:
			Snapshot_XwFlightGlobalCountdownTimers(codec, &g_flightGlobalCountdownTimers);
			break;
		case REPLAYIO_SNAPSHOT_NEXT_DEBRIS_OBJECT_SLOT:
			Snapshot_Word(codec, &g_nextDebrisObjectSlot);
			break;
		case REPLAYIO_SNAPSHOT_SURFACE_CELL_DAMAGE_STATES:
			for (index = 0; index < sizeof(g_surfaceCellDamageStates) / sizeof(g_surfaceCellDamageStates[0]);
				 ++index) {
				Snapshot_XwSurfaceCellDamageState(codec, &g_surfaceCellDamageStates[index]);
			}
			break;
		case REPLAYIO_SNAPSHOT_SURFACE_GUN_CELLS:
			for (index = 0; index < sizeof(g_surfaceGunCells) / sizeof(g_surfaceGunCells[0]); ++index) {
				Snapshot_XwSurfaceGunCell(codec, &g_surfaceGunCells[index]);
			}
			break;
		case REPLAYIO_SNAPSHOT_SURFACE_GUN_CELL_COUNT:
			Snapshot_Word(codec, &g_surfaceGunCellCount);
			break;
		case REPLAYIO_SNAPSHOT_SURFACE_GUN_CELL_REFRESH_TICKS:
			Snapshot_Word(codec, &g_surfaceGunCellRefreshTicks);
			break;
		case REPLAYIO_SNAPSHOT_BACKDROP_PACKED_DIRECTIONS:
			Snapshot_Bytes(codec, &g_backdropPackedDirections, sizeof(g_backdropPackedDirections));
			break;
		case REPLAYIO_SNAPSHOT_BACKDROP_MODEL_TYPES:
			Snapshot_Bytes(codec, &g_backdropModelTypes, sizeof(g_backdropModelTypes));
			break;
		case REPLAYIO_SNAPSHOT_BACKDROP_POSITIVE_Y_COUNT:
			Snapshot_Word(codec, &g_backdropPositiveYCount);
			break;
		case REPLAYIO_SNAPSHOT_BACKDROP_NEGATIVE_Y_COUNT:
			Snapshot_Word(codec, &g_backdropNegativeYCount);
			break;
		case REPLAYIO_SNAPSHOT_BACKDROP_POSITIVE_Z_COUNT:
			Snapshot_Word(codec, &g_backdropPositiveZCount);
			break;
		case REPLAYIO_SNAPSHOT_BACKDROP_NEGATIVE_Z_COUNT:
			Snapshot_Word(codec, &g_backdropNegativeZCount);
			break;
		case REPLAYIO_SNAPSHOT_BACKDROP_POSITIVE_X_COUNT:
			Snapshot_Word(codec, &g_backdropPositiveXCount);
			break;
		case REPLAYIO_SNAPSHOT_BACKDROP_NEGATIVE_X_COUNT:
			Snapshot_Word(codec, &g_backdropNegativeXCount);
			break;
		case REPLAYIO_SNAPSHOT_STAR_DENSITY:
			Snapshot_Word(codec, &g_starDensity);
			break;
		case REPLAYIO_SNAPSHOT_DEATH_STAR_SURFACE_MODE_ACTIVE:
			Snapshot_Bytes(codec, &g_deathStarSurfaceModeActive, sizeof(g_deathStarSurfaceModeActive));
			break;
		case REPLAYIO_SNAPSHOT_SURFACE_GOAL_CELL_HASH_SLOTS:
			for (index = 0;
				 index < sizeof(g_surfaceGoalCellHashSlots) / sizeof(g_surfaceGoalCellHashSlots[0]);
				 ++index) {
				Snapshot_Word(codec, &g_surfaceGoalCellHashSlots[index]);
			}
			break;
		case REPLAYIO_SNAPSHOT_SURFACE_GOAL_CELL_KEYS:
			for (index = 0; index < sizeof(g_surfaceGoalCellKeys) / sizeof(g_surfaceGoalCellKeys[0]);
				 ++index) {
				Snapshot_Word(codec, &g_surfaceGoalCellKeys[index]);
			}
			break;
		case REPLAYIO_SNAPSHOT_BACKDROPS_ENABLED:
			Snapshot_Bytes(codec, &g_backdropsEnabled, sizeof(g_backdropsEnabled));
			break;
		case REPLAYIO_SNAPSHOT_DEBRIS_ENABLED:
			Snapshot_Bytes(codec, &g_debrisEnabled, sizeof(g_debrisEnabled));
			break;
		case REPLAYIO_SNAPSHOT_CRAFT_EXPLOSION_SPAWN_THRESHOLD:
			Snapshot_Word(codec, &g_craftExplosionSpawnThreshold);
			break;
		case REPLAYIO_SNAPSHOT_STARSHIP_DETAIL:
			Snapshot_Word(codec, &g_starshipDetail);
			break;
		case REPLAYIO_SNAPSHOT_SURFACE_OBJECT_DETAIL_LIMIT:
			Snapshot_Word(codec, &g_surfaceObjectDetailLimit);
			break;
		case REPLAYIO_SNAPSHOT_TRENCH_OBJECT_DETAIL_LIMIT:
			Snapshot_Word(codec, &g_trenchObjectDetailLimit);
			break;
		case REPLAYIO_SNAPSHOT_DEATH_STAR_DETAIL_LEVEL:
			Snapshot_Word(codec, &g_deathStarDetailLevel);
			break;
		case REPLAYIO_SNAPSHOT_HYPERSPACE_EFFECT_OBJECT_COUNT:
			Snapshot_Word(codec, &g_hyperspaceEffectObjectCount);
			break;
		case REPLAYIO_SNAPSHOT_SHIP_DETAIL_VALUE:
			Snapshot_Word(codec, &g_shipDetailValue);
			break;
		case REPLAYIO_SNAPSHOT_SHIP_DETAIL_POLY_COUNT:
			Snapshot_Word(codec, &g_shipDetailPolyCount);
			break;
		case REPLAYIO_SNAPSHOT_DRAW_MARKINGS_FLAG:
			Snapshot_Word(codec, &g_drawMarkingsFlag);
			break;
		case REPLAYIO_SNAPSHOT_FLIGHT_GRAPHICS_DETAIL_PRESET:
			Snapshot_Bytes(codec, &g_flightGraphicsDetailPreset, sizeof(g_flightGraphicsDetailPreset));
			break;
		case REPLAYIO_SNAPSHOT_MISSION_CHEAT_OPTIONS_USED:
			Snapshot_Word(codec, &g_missionCheatOptionsUsed);
			break;
		case REPLAYIO_SNAPSHOT_FLIGHT_MUSIC_VOLUME:
			Snapshot_Bytes(codec, &g_flightMusicVolume, sizeof(g_flightMusicVolume));
			break;
		case REPLAYIO_SNAPSHOT_FLIGHT_MUSIC_ENABLED:
			Snapshot_Bytes(codec, &g_flightMusicEnabled, sizeof(g_flightMusicEnabled));
			break;
		case REPLAYIO_SNAPSHOT_FLIGHT_SFX_VOLUME:
			Snapshot_Bytes(codec, &g_flightSfxVolume, sizeof(g_flightSfxVolume));
			break;
		case REPLAYIO_SNAPSHOT_FLIGHT_SFX_ENABLED:
			Snapshot_Bytes(codec, &g_flightSfxEnabled, sizeof(g_flightSfxEnabled));
			break;
		case REPLAYIO_SNAPSHOT_FLIGHT_ENGINE_SOUND_ENABLED:
			Snapshot_Bytes(codec, &g_flightEngineSoundEnabled, sizeof(g_flightEngineSoundEnabled));
			break;
		case REPLAYIO_SNAPSHOT_FLIGHT_DIGITAL_SOUND_ENABLED:
			Snapshot_Bytes(codec, &g_flightDigitalSoundEnabled, sizeof(g_flightDigitalSoundEnabled));
			break;
		case REPLAYIO_SNAPSHOT_FLIGHT_VOICE_ENABLED:
			Snapshot_Bytes(codec, &g_flightVoiceEnabled, sizeof(g_flightVoiceEnabled));
			break;
		case REPLAYIO_SNAPSHOT_FLIGHT_REPLAY_DISK_CACHE_KB:
			Snapshot_Word(codec, &g_flightReplayDiskCacheKB);
			break;
		case REPLAYIO_SNAPSHOT_FLIGHT_REPLAY_DISK_CACHE_ENABLED:
			Snapshot_Bytes(codec, &g_flightReplayDiskCacheEnabled, sizeof(g_flightReplayDiskCacheEnabled));
			break;
		case REPLAYIO_SNAPSHOT_UNLIMITED_WEAPONS_ENABLED:
			Snapshot_Bytes(codec, &g_unlimitedWeaponsEnabled, sizeof(g_unlimitedWeaponsEnabled));
			break;
		case REPLAYIO_SNAPSHOT_FLIGHT_INVULNERABILITY_ENABLED:
			Snapshot_Bytes(codec, &g_flightInvulnerabilityEnabled, sizeof(g_flightInvulnerabilityEnabled));
			break;
		case REPLAYIO_SNAPSHOT_FLIGHT_CRAFT_COLLISIONS_ENABLED:
			Snapshot_Bytes(codec, &g_flightCraftCollisionsEnabled, sizeof(g_flightCraftCollisionsEnabled));
			break;
		case REPLAYIO_SNAPSHOT_FLIGHT_HIGH_DETAIL_STARFIELD:
			Snapshot_Bytes(codec, &g_flightHighDetailStarfield, sizeof(g_flightHighDetailStarfield));
			break;
		case REPLAYIO_SNAPSHOT_FLIGHT_BACKDROPS_PREFERENCE:
			Snapshot_Bytes(codec, &g_flightBackdropsPreference, sizeof(g_flightBackdropsPreference));
			break;
		case REPLAYIO_SNAPSHOT_FLIGHT_DEBRIS_PREFERENCE:
			Snapshot_Bytes(codec, &g_flightDebrisPreference, sizeof(g_flightDebrisPreference));
			break;
		case REPLAYIO_SNAPSHOT_FLIGHT_MARKINGS_PREFERENCE:
			Snapshot_Bytes(codec, &g_flightMarkingsPreference, sizeof(g_flightMarkingsPreference));
			break;
		case REPLAYIO_SNAPSHOT_FLIGHT_STARFIGHTER_DETAIL:
			Snapshot_Bytes(codec, &g_flightStarfighterDetail, sizeof(g_flightStarfighterDetail));
			break;
		case REPLAYIO_SNAPSHOT_FLIGHT_STARSHIP_DETAIL:
			Snapshot_Bytes(codec, &g_flightStarshipDetail, sizeof(g_flightStarshipDetail));
			break;
		case REPLAYIO_SNAPSHOT_FLIGHT_DEATH_STAR_DETAIL:
			Snapshot_Bytes(codec, &g_flightDeathStarDetail, sizeof(g_flightDeathStarDetail));
			break;
		case REPLAYIO_SNAPSHOT_FLIGHT_ENGINE_GLOW_PREFERENCE:
			Snapshot_Bytes(codec, &g_flightEngineGlowPreference, sizeof(g_flightEngineGlowPreference));
			break;
		case REPLAYIO_SNAPSHOT_MODEL_TEXTURE_QUALITY:
			Snapshot_Dword(codec, &g_modelTextureQuality);
			break;
		case REPLAYIO_SNAPSHOT_FLIGHT_BRIGHTNESS_SETTING:
			Snapshot_Bytes(codec, &g_flightBrightnessSetting, sizeof(g_flightBrightnessSetting));
			break;
		case REPLAYIO_SNAPSHOT_GATE_GUN_TIMER:
			Snapshot_Word(codec, &g_gateGunTimer);
			break;
		case REPLAYIO_SNAPSHOT_PROVING_GROUNDS_CHECKPOINT_BLINK_TICKS:
			Snapshot_Word(codec, &g_provingGroundsCheckpointBlinkTicks);
			break;
		case REPLAYIO_SNAPSHOT_DYNAMIC_MUSIC_STATE:
			Snapshot_Bytes(codec, &g_dynamicMusicState, sizeof(g_dynamicMusicState));
			break;
		case REPLAYIO_SNAPSHOT_DYNAMIC_MUSIC_OUTCOME_LATCHED:
			Snapshot_Bytes(codec, &g_dynamicMusicOutcomeLatched, sizeof(g_dynamicMusicOutcomeLatched));
			break;
		case REPLAYIO_SNAPSHOT_DYNAMIC_MUSIC_TRACK_REMAINING_MS:
			Snapshot_Dword(codec, &g_dynamicMusicTrackRemainingMs);
			break;
		case REPLAYIO_SNAPSHOT_SURFACE_VICTORY_EXIT_SECOND:
			Snapshot_Bytes(codec, &g_surfaceVictoryExitSecond, sizeof(g_surfaceVictoryExitSecond));
			break;
		case REPLAYIO_SNAPSHOT_SURFACE_SPECIAL_TARGET_HIT:
			Snapshot_Bytes(codec, &g_surfaceSpecialTargetHit, sizeof(g_surfaceSpecialTargetHit));
			break;
		case REPLAYIO_SNAPSHOT_SURFACE_SPECIAL_TARGET_COLLISION_MODE:
			Snapshot_Bytes(codec, &g_surfaceSpecialTargetCollisionMode,
						   sizeof(g_surfaceSpecialTargetCollisionMode));
			break;
		case REPLAYIO_SNAPSHOT_TRENCH_SPECIAL_CELL_VOICE_PLAYED:
			Snapshot_Bytes(codec, &g_trenchSpecialCellVoicePlayed, sizeof(g_trenchSpecialCellVoicePlayed));
			break;
		case REPLAYIO_SNAPSHOT_DEATH_STAR_RENDER_OBJECT:
			Snapshot_ObjectRecord(codec, &g_deathStarRenderObject);
			break;
		case REPLAYIO_SNAPSHOT_DEATH_STAR_RENDER_CRAFT:
			Snapshot_CraftData(codec, &g_deathStarRenderCraft);
			break;
		default:
			return 0;
	}
	return codec->valid && codec->offset == g_ReplaySnapshotBlockSizes[blockIndex];
}

/* Fixed native supplement: only state that survives a frame or shell visit.
 * Renderer workspaces and audio device state are rebuilt by their owners. */
static void Snapshot_Extra(SnapshotCodec* codec) {
	Snapshot_Word(codec, &g_gameRandomSeed);
	Snapshot_Word(codec, &g_elapsedTicks);
	Snapshot_Word(codec, &g_simStepScale);
	Snapshot_Bytes(codec, &calcframerate, 1);
	Snapshot_Word(codec, &g_legacyOscillatorFrameCounter);
	Snapshot_Word(codec, &g_legacyOscillatorDirection);
	Snapshot_Bytes(codec, &g_legacyOscillatorValue, 1);
	Snapshot_Bytes(codec, g_flightRandomBytePairs, sizeof g_flightRandomBytePairs);
	Snapshot_Dword(codec, &g_hyperspaceStreakLength);
	Snapshot_Word(codec, &g_playerHyperspaceElapsedTicks);
	Snapshot_Bytes(codec, &g_hyperspaceflag, 1);
	Snapshot_Word(codec, &g_hyperspaceSavedDebrisEnabled);
	Snapshot_Word(codec, &g_hyperspaceSavedBackdropsEnabled);
	Snapshot_Bytes(codec, &g_hyperspaceAbortAndCollisionsAllowed, 1);
	for (unsigned i = 0; i < GATE_POSE_HISTORY_COUNT; ++i) {
		Snapshot_Dword(codec, &g_gatePreviousX[i]);
		Snapshot_Dword(codec, &g_gatePreviousY[i]);
		Snapshot_Dword(codec, &g_gatePreviousZ[i]);
		Snapshot_Word(codec, &g_gatePreviousRoll[i]);
		Snapshot_Word(codec, &g_gatePreviousPitch[i]);
		Snapshot_Word(codec, &g_gatePreviousYaw[i]);
	}
	Snapshot_Dword(codec, &g_modelLightDirectionX);
	Snapshot_Dword(codec, &g_modelLightDirectionY);
	Snapshot_Dword(codec, &g_modelLightDirectionZ);
	if (codec->decoding && !memchr(codec->bytes + codec->offset, 0, sizeof g_currentMissionFile))
		codec->valid = 0;
	Snapshot_Bytes(codec, g_currentMissionFile, sizeof g_currentMissionFile);
	Snapshot_Bytes(codec, g_PilotObjectRefs, sizeof g_PilotObjectRefs);
	if (codec->decoding)
		for (unsigned i = 0; i < XW_FLIGHT_PILOT_SLOT_COUNT; ++i)
			if (!memchr(codec->bytes + codec->offset + i * XW_FLIGHT_PILOT_NAME_CAPACITY, 0,
						XW_FLIGHT_PILOT_NAME_CAPACITY))
				codec->valid = 0;
	Snapshot_Bytes(codec, g_PilotSlotNames, sizeof g_PilotSlotNames);
	Snapshot_Bytes(codec, g_pilotSlotFlightGroupIndices, sizeof g_pilotSlotFlightGroupIndices);
	Snapshot_Bytes(codec, g_pilotSlotCraftIndices, sizeof g_pilotSlotCraftIndices);
	for (unsigned i = 0; i < XW_FLIGHT_PILOT_SLOT_COUNT; ++i)
		Snapshot_Word(codec, &g_pilotSlotSkillValues[i]);
	/* Playback owns its cursors; returning to live flight restores the recorder. */
	uint16_t recording = g_ReplayRecording, available = g_ReplayRecordedFlightAvailable,
			 seed = g_ReplayRandomSeed;
	uint32_t frames = g_ReplayFrameCount, capacity = g_ReplayCapacityFrames, index = g_ReplayBufferIndex;
	uint8_t spool = g_ReplaySpoolEnabled;
	uint32_t cursor = 0;
	if (!codec->decoding && g_ReplayBufferStart && g_ReplayInputPointer) {
		uintptr_t base = (uintptr_t)g_ReplayBufferStart, ptr = (uintptr_t)g_ReplayInputPointer;
		if (ptr < base || ptr - base > REPLAY_BUFFER_CAPACITY)
			codec->valid = 0;
		else
			cursor = (uint32_t)(ptr - base);
	}
	SnapshotCodec recorder = *codec;
	recorder.checkOnly = 0;
	Snapshot_Word(&recorder, &recording);
	Snapshot_Word(&recorder, &available);
	Snapshot_Dword(&recorder, &frames);
	Snapshot_Dword(&recorder, &capacity);
	Snapshot_Dword(&recorder, &index);
	Snapshot_Word(&recorder, &seed);
	Snapshot_Bytes(&recorder, &spool, 1);
	Snapshot_Dword(&recorder, &cursor);
	if (cursor > REPLAY_BUFFER_CAPACITY || cursor % REPLAY_INPUT_RECORD_SIZE ||
		index > REPLAY_BUFFER_CAPACITY / REPLAY_INPUT_RECORD_SIZE || recording > 1 || available > 1 ||
		spool > 1 || frames > UINT16_MAX * 1024u / REPLAY_INPUT_RECORD_SIZE)
		codec->valid = 0;
	if (codec->decoding && !codec->checkOnly && !g_replayviewmode) {
		g_ReplayRecording = recording;
		g_ReplayRecordedFlightAvailable = available;
		g_ReplayFrameCount = frames;
		g_ReplayCapacityFrames = capacity;
		g_ReplayBufferIndex = index;
		g_ReplayRandomSeed = seed;
		g_ReplaySpoolEnabled = spool;
		g_ReplayInputPointer = g_ReplayBufferStart + cursor;
	}
	codec->offset = recorder.offset;
}

/* Flight timing supplement. All decoding uses local state, including the check-only pass. */
typedef struct SnapshotTiming {
	XwFlightTimingState flight;
	XwFlightIntegrationState integration;
	XwReferenceMotionState reference[XW_OBJECT_COUNT];
	XwPlayerTimingState player;
	XwRecoveryTimingState recovery;
	XwChaseTimingState chase;
} SnapshotTiming;

static void Snapshot_Qword(SnapshotCodec* codec, void* field) {
	uint64_t value = 0;
	if (codec->bytes) {
		if (codec->decoding) {
			for (unsigned i = 0; i < 8; ++i)
				value |= (uint64_t)codec->bytes[codec->offset + i] << (8 * i);
			if (!codec->checkOnly)
				memcpy(field, &value, 8);
		} else {
			memcpy(&value, field, 8);
			for (unsigned i = 0; i < 8; ++i)
				codec->bytes[codec->offset + i] = (uint8_t)(value >> (8 * i));
		}
	}
	codec->offset += 8;
}

static void Snapshot_Bool(SnapshotCodec* codec, bool* field) {
	uint8_t value = *field;
	Snapshot_Bytes(codec, &value, 1);
	if (value > 1)
		codec->valid = 0;
	if (codec->decoding)
		*field = value != 0;
}

static void Snapshot_TimingFields(SnapshotCodec* codec, SnapshotTiming* state) {
	XwFlightTimingState* f = &state->flight;
	uint8_t rate = f->update_rate;
	Snapshot_Bytes(codec, &rate, 1);
	f->update_rate = (XwFlightUpdateRate)rate;
	Snapshot_Word(codec, &f->reference_ticks);
	Snapshot_Word(codec, &f->phase);
	Snapshot_Word(codec, &f->elapsed_ticks);
	Snapshot_Bool(codec, &f->advance_active);
	Snapshot_Bool(codec, &f->reference_due);
	Snapshot_Bool(codec, &f->movement_completed);
	Snapshot_Qword(codec, &f->advance_serial);
	Snapshot_Qword(codec, &f->completed_movement_ticks);
	Snapshot_Qword(codec, &f->animation_serial);
	Snapshot_Qword(codec, &f->animation_time_ticks);
	Snapshot_Dword(codec, &f->dropped_periods);
	for (unsigned i = 0; i < MISSION_OBJECT_COUNT; ++i)
		Snapshot_Qword(codec, &f->mission_pose_serial[i]);
	for (unsigned i = 0; i < XW_OBJECT_COUNT; ++i) {
		XwMobileIntegration* m = &state->integration.mobile[i];
		for (unsigned a = 0; a < 3; ++a)
			Snapshot_Qword(codec, &m->position[a]);
		for (unsigned c = 0; c < XW_INTEGRATE_ROLL; ++c) {
			Snapshot_Qword(codec, &m->channels[c].remainder);
			Snapshot_Bytes(codec, &m->channels[c].direction, 1);
		}
		Snapshot_Word(codec, &m->target);
		Snapshot_Bytes(codec, &m->tier, 1);
	}
	for (unsigned i = 0; i < XW_CRAFT_OBJECT_COUNT; ++i)
		for (unsigned c = 0; c < XW_INTEGRATE_COUNT - XW_INTEGRATE_ROLL; ++c) {
			Snapshot_Qword(codec, &state->integration.craft[i][c].remainder);
			Snapshot_Bytes(codec, &state->integration.craft[i][c].direction, 1);
		}
	for (unsigned i = 0; i < XW_OBJECT_COUNT; ++i) {
		for (unsigned a = 0; a < 3; ++a)
			Snapshot_Dword(codec, &state->reference[i].position[a]);
		Snapshot_Qword(codec, &state->reference[i].ticks);
		Snapshot_Bool(codec, &state->reference[i].valid);
	}
	XwPlayerTimingState* p = &state->player;
	for (unsigned c = 0; c < XW_PLAYER_BASE_TIMING_CHANNELS; ++c) {
		Snapshot_Qword(codec, &p->remainder[c]);
		Snapshot_Bytes(codec, &p->direction[c], 1);
	}
	Snapshot_Word(codec, &p->slot);
	Snapshot_Word(codec, &p->craft_type);
	Snapshot_Word(codec, &p->focus);
	Snapshot_Dword(codec, &p->mode);
	Snapshot_Bool(codec, &p->valid);
	for (unsigned a = 0; a < 3; ++a)
		Snapshot_Dword(codec, &state->recovery.position[a]);
	Snapshot_Qword(codec, &state->recovery.serial);
	Snapshot_Word(codec, &state->recovery.slot);
	Snapshot_Bool(codec, &state->recovery.valid);
	for (unsigned i = 0; i < FLIGHT_VIEW_ANGLE_HISTORY_COUNT; ++i)
		Snapshot_Qword(codec, &state->chase.ticks[i]);
	Snapshot_Qword(codec, &state->chase.serial);
	Snapshot_Qword(codec, &state->chase.last_ticks);
	Snapshot_Word(codec, &state->chase.focus);
	Snapshot_Bool(codec, &state->chase.valid);
}

static bool Snapshot_CheckCarry(int64_t remainder, int direction, int64_t divisor, bool signedRemainder) {
	return direction >= -1 && direction <= 1 && remainder > -divisor && remainder < divisor &&
		   (direction || !remainder) &&
		   (signedRemainder ? (!remainder || (remainder < 0 ? direction < 0 : direction > 0))
							: remainder >= 0);
}

static bool Snapshot_CheckTiming(const SnapshotTiming* s, XwGameVersion version, XwFlightUpdateRate rate) {
	const XwFlightTimingState* f = &s->flight;
	if (!XwFlightTiming_Check(f, version, rate))
		return false;
	for (unsigned i = 0; i < XW_OBJECT_COUNT; ++i) {
		const XwMobileIntegration* m = &s->integration.mobile[i];
		for (unsigned a = 0; a < 3; ++a)
			if (m->position[a] <= -(int64_t)236 * 32768 || m->position[a] >= (int64_t)236 * 32768)
				return false;
		for (unsigned c = 0; c < XW_INTEGRATE_ROLL; ++c)
			if (!Snapshot_CheckCarry(m->channels[c].remainder, m->channels[c].direction, 236, true))
				return false;
		if (m->tier > 6 || (m->target >= XW_OBJECT_COUNT && m->target != UINT16_MAX &&
							(m->target < XW_MISSION_OBJECT_REF_BASE ||
							 m->target >= XW_MISSION_OBJECT_REF_BASE + MISSION_OBJECT_COUNT)))
			return false;
		if (s->reference[i].ticks > f->completed_movement_ticks)
			return false;
	}
	for (unsigned i = 0; i < XW_CRAFT_OBJECT_COUNT; ++i)
		for (unsigned c = XW_INTEGRATE_ROLL; c < XW_INTEGRATE_COUNT; ++c) {
			const XwIntegrationCarry* carry = &s->integration.craft[i][c - XW_INTEGRATE_ROLL];
			int64_t divisor = c == XW_INTEGRATE_VELOCITY ? 236
							  : c == XW_INTEGRATE_BANK   ? 65536
														 : (int64_t)236 * 65536 * 65536;
			if (!Snapshot_CheckCarry(carry->remainder, carry->direction, divisor, false))
				return false;
		}
	const XwPlayerTimingState* p = &s->player;
	for (unsigned c = 0; c < XW_PLAYER_BASE_TIMING_CHANNELS; ++c) {
		unsigned divisor =
			c <= XW_PLAYER_SLEW_PITCH || c == XW_PLAYER_ZOOM_ACCELERATION ? f->reference_ticks : 236;
		if (!Snapshot_CheckCarry(p->remainder[c], p->direction[c], divisor, true))
			return false;
	}
	if (p->slot >= XW_CRAFT_OBJECT_COUNT ||
		p->craft_type >= XwProfile_Flight(version)->craft_definition_count ||
		(p->focus != UINT16_MAX && p->focus >= XW_OBJECT_COUNT) || (p->mode & ~0x1ffu) ||
		s->recovery.slot >= XW_OBJECT_COUNT || s->recovery.serial > f->advance_serial ||
		s->chase.focus >= XW_OBJECT_COUNT || s->chase.serial > f->advance_serial ||
		s->chase.last_ticks > f->completed_movement_ticks)
		return false;
	for (unsigned i = 0; i < FLIGHT_VIEW_ANGLE_HISTORY_COUNT; ++i)
		if (s->chase.ticks[i] > s->chase.last_ticks)
			return false;
	return true;
}

static bool Snapshot_Timing(SnapshotCodec* codec, XwGameVersion version, XwFlightUpdateRate rate) {
	SnapshotTiming state = { 0 };
	state.flight.reference_ticks = XwProfile_Flight(version)->minimum_frame_ticks;
	if (codec->bytes && !codec->decoding && rate == XW_FLIGHT_UPDATE_RATE_UNLOCKED) {
		XwFlightTiming_Save(&state.flight);
		XwFlightIntegration_Save(&state.integration);
		XwReferenceMotion_Save(state.reference);
		XwPlayerTiming_Save(&state.player, &state.recovery);
		XwFlightCamera_SaveTiming(&state.chase);
	}
	SnapshotCodec local = *codec;
	local.checkOnly = 0;
	Snapshot_TimingFields(&local, &state);
	bool valid = local.valid;
	if (codec->bytes) {
		valid = valid && Snapshot_CheckTiming(&state, version, rate);
		/* Native timing leaves this supplement inactive: reference interval, then zeros. */
		if (valid && rate == XW_FLIGHT_UPDATE_RATE_NATIVE)
			for (size_t i = codec->offset + 3; i < local.offset; ++i)
				if (codec->bytes[i]) {
					valid = false;
					break;
				}
		if (valid && codec->decoding && !codec->checkOnly) {
			valid = XwFlightTiming_Restore(&state.flight);
			if (valid) {
				XwFlightIntegration_Restore(&state.integration);
				XwReferenceMotion_Restore(state.reference);
				XwPlayerTiming_Restore(&state.player, &state.recovery);
				XwFlightCamera_RestoreTiming(&state.chase);
			}
		}
	}
	codec->offset = local.offset;
	return valid;
}

/* Independent roll retains its accumulator in both native and unlocked timing. */
static bool Snapshot_Roll(SnapshotCodec* codec, XwGameVersion version, XwFlightUpdateRate rate) {
	XwPlayerTimingState state = { 0 };
	XwRecoveryTimingState unused;
	if (codec->bytes && !codec->decoding)
		XwPlayerTiming_Save(&state, &unused);
	SnapshotCodec local = *codec;
	local.checkOnly = 0;
	Snapshot_Word(&local, &state.analog_roll);
	bool valid = true;
	for (unsigned c = XW_PLAYER_SLEW_ANALOG_ROLL; c < XW_PLAYER_TIMING_CHANNELS; ++c) {
		Snapshot_Qword(&local, &state.remainder[c]);
		Snapshot_Bytes(&local, &state.direction[c], 1);
		unsigned divisor =
			c == XW_PLAYER_SLEW_ANALOG_ROLL ? XwProfile_Flight(version)->minimum_frame_ticks : 236;
		valid = valid && Snapshot_CheckCarry(state.remainder[c], state.direction[c], divisor, true);
		if (rate == XW_FLIGHT_UPDATE_RATE_NATIVE && (state.remainder[c] || state.direction[c]))
			valid = false;
	}
	if (valid && codec->bytes && codec->decoding && !codec->checkOnly)
		XwPlayerTiming_RestoreRoll(&state);
	codec->offset = local.offset;
	return valid && local.valid;
}

static size_t Snapshot_CommonSize(void) {
	size_t size = REPLAYIO_DISK_FLIGHT_GROUP_SIZE * MISSION_FLIGHT_GROUP_COUNT;
	for (unsigned i = 0; i < REPLAYIO_SNAPSHOT_BLOCK_COUNT; ++i)
		size += g_ReplaySnapshotBlockSizes[i];
	return size;
}

size_t XwReplaySnapshot_Size(XwGameVersion version, XwReplayLayout layout) {
	const XwFlightProfile* profile = XwProfile_Flight(version);
	if (!profile || (layout != XW_REPLAY_LAYOUT_LEGACY && layout != XW_REPLAY_LAYOUT_CURRENT) ||
		(layout == XW_REPLAY_LAYOUT_LEGACY && version != XW_GAME_VERSION_98))
		return 0;
	SnapshotCodec count = { .valid = 1 };
	if (layout == XW_REPLAY_LAYOUT_CURRENT) {
		Snapshot_Extra(&count);
		Snapshot_Timing(&count, version, XW_FLIGHT_UPDATE_RATE_NATIVE);
		Snapshot_Roll(&count, version, XW_FLIGHT_UPDATE_RATE_NATIVE);
	}
	return Snapshot_CommonSize() + profile->model_count + count.offset;
}

static bool Snapshot_Image(uint8_t* bytes, size_t size, XwGameVersion version, XwReplayLayout layout,
						   XwFlightUpdateRate rate, bool decoding, bool checkOnly) {
	bool legacy = layout == XW_REPLAY_LAYOUT_LEGACY;
	if ((rate != XW_FLIGHT_UPDATE_RATE_NATIVE && rate != XW_FLIGHT_UPDATE_RATE_UNLOCKED) ||
		(legacy && rate != XW_FLIGHT_UPDATE_RATE_NATIVE) || !size ||
		size != XwReplaySnapshot_Size(version, layout))
		return false;
	size_t cursor = 0;
	for (unsigned i = 0; i < REPLAYIO_SNAPSHOT_BLOCK_COUNT; ++i) {
		SnapshotCodec codec = { .bytes = bytes + cursor,
								.decoding = decoding,
								.valid = 1,
								.checkOnly = checkOnly,
								.nativeReferences = !legacy };
		if (!Snapshot_Block(&codec, i))
			return false;
		cursor += codec.offset;
	}
	SnapshotCodec groups = { .bytes = bytes + cursor,
							 .decoding = decoding,
							 .valid = 1,
							 .checkOnly = checkOnly,
							 .nativeReferences = !legacy };
	for (unsigned i = 0; i < MISSION_FLIGHT_GROUP_COUNT; ++i)
		Snapshot_XwMissionFlightGroup(&groups, &g_missionFlightGroups[i]);
	cursor += groups.offset;
	unsigned models = XwProfile_Flight(version)->model_count;
	for (unsigned i = 0; i < models; ++i) {
		if (!decoding)
			bytes[cursor + i] = XwFlightTypes_ModelFlags(i);
		else if (!checkOnly)
			XwFlightTypes_SetModelFlags(i, bytes[cursor + i]);
	}
	cursor += models;
	if (!legacy) {
		SnapshotCodec extra = { .bytes = bytes + cursor,
								.decoding = decoding,
								.valid = 1,
								.checkOnly = checkOnly,
								.nativeReferences = 1 };
		Snapshot_Extra(&extra);
		if (!extra.valid || !Snapshot_Timing(&extra, version, rate) || !Snapshot_Roll(&extra, version, rate))
			return false;
		cursor += extra.offset;
	}
	return cursor == size;
}

bool XwReplaySnapshot_Capture(uint8_t* bytes, size_t size, XwGameVersion version) {
	return XwProfile_HasActiveFlight() && XwProfile_ActiveFlight()->version == version &&
		   Snapshot_Image(bytes, size, version, XW_REPLAY_LAYOUT_CURRENT, XwProfile_ActiveFlightRate(), false,
						  false);
}

static bool Snapshot_CheckRecords(uint8_t* bytes, XwGameVersion version, bool legacy) {
	size_t offsets[REPLAYIO_SNAPSHOT_BLOCK_COUNT + 1] = { 0 };
	for (unsigned i = 0; i < REPLAYIO_SNAPSHOT_BLOCK_COUNT; ++i)
		offsets[i + 1] = offsets[i] + g_ReplaySnapshotBlockSizes[i];
	const XwFlightProfile* profile = XwProfile_Flight(version);
	unsigned models = profile->model_count;
	unsigned definitions = profile->craft_definition_count;
	ObjectRecord objects[XW_OBJECT_COUNT] = { 0 };
	SnapshotCodec codec = { .bytes = bytes, .decoding = 1, .valid = 1, .nativeReferences = !legacy };
	for (unsigned i = 0; i < XW_OBJECT_COUNT; ++i) {
		Snapshot_ObjectRecord(&codec, &objects[i]);
		ObjectRecord* o = &objects[i];
		if (o->objectType >= models || (o->objectType && o->genusId > XW_GENUS_SCENERY))
			return false;
		if (o->objectType && o->genusId <= XW_GENUS_STARSHIP) {
			uintptr_t ptr = (uintptr_t)o->instanceData, base = (uintptr_t)g_craftTable;
			if (ptr < base || ptr - base >= sizeof g_craftTable || (ptr - base) % sizeof(CraftData))
				return false;
			unsigned slot = (unsigned)((ptr - base) / sizeof(CraftData));
			CraftData craft = { 0 };
			SnapshotCodec c = { .bytes = bytes + offsets[REPLAYIO_SNAPSHOT_CRAFT_TABLE] +
										 slot * REPLAYIO_DISK_CRAFT_SIZE,
								.decoding = 1,
								.valid = 1 };
			Snapshot_CraftData(&c, &craft);
			if (craft.craftTypeIndex >= definitions || craft.flightGroupIndex >= MISSION_FLIGHT_GROUP_COUNT)
				return false;
		}
	}
	codec.bytes = bytes + offsets[REPLAYIO_SNAPSHOT_PLAYER_FLIGHT_STATE];
	codec.offset = 0;
	XwPlayerFlightState player = { 0 };
	Snapshot_XwPlayerFlightState(&codec, &player);
	if (!codec.valid || player.objectIndex >= XW_CRAFT_OBJECT_COUNT ||
		player.object != &g_objectTable[player.objectIndex] || !player.craft ||
		player.craft != objects[player.objectIndex].instanceData ||
		player.flightGroupIndex >= MISSION_FLIGHT_GROUP_COUNT || player.craftTypeIndex >= definitions)
		return false;
	codec.bytes = bytes + offsets[REPLAYIO_SNAPSHOT_MISSION_HEADER];
	codec.offset = 0;
	XwMissionHeader header = { 0 };
	Snapshot_XwMissionHeader(&codec, &header);
	if (header.flightGroupCount > MISSION_FLIGHT_GROUP_COUNT ||
		header.objectRecordCount > MISSION_OBJECT_COUNT)
		return false;
	codec.bytes = bytes + offsets[REPLAYIO_SNAPSHOT_FLIGHT_CAMERA];
	codec.offset = 0;
	XwFlightCamera camera = { 0 };
	Snapshot_XwFlightCamera(&codec, &camera);
	if ((unsigned)camera.angleHistory.writeIndex >= FLIGHT_VIEW_ANGLE_HISTORY_COUNT ||
		(camera.focusObjectRef != UINT16_MAX && camera.focusObjectRef >= XW_OBJECT_COUNT))
		return false;
	codec.bytes = bytes + offsets[REPLAYIO_SNAPSHOT_MISSION_OBJECTS];
	codec.offset = 0;
	for (unsigned i = 0; i < MISSION_OBJECT_COUNT; ++i) {
		XwMissionObjectRecord object = { 0 };
		Snapshot_XwMissionObjectRecord(&codec, &object);
		if (object.objectType >= models || (object.objectType && object.genusId > XW_GENUS_SCENERY))
			return false;
	}
	const unsigned details[] = { REPLAYIO_SNAPSHOT_FLIGHT_STARFIGHTER_DETAIL,
								 REPLAYIO_SNAPSHOT_FLIGHT_STARSHIP_DETAIL,
								 REPLAYIO_SNAPSHOT_FLIGHT_DEATH_STAR_DETAIL };
	for (unsigned i = 0; i < 3; ++i)
		if (bytes[offsets[details[i]]] > 13)
			return false;
	uint8_t* count = bytes + offsets[REPLAYIO_SNAPSHOT_SURFACE_GUN_CELL_COUNT];
	if ((unsigned)(count[0] | count[1] << 8) > DEATH_STAR_SURFACE_GUN_CELL_CAPACITY)
		return false;
	return true;
}

bool XwReplaySnapshot_Check(uint8_t* bytes, size_t size, XwGameVersion version, XwReplayLayout layout,
							XwFlightUpdateRate rate) {
	return Snapshot_Image(bytes, size, version, layout, rate, true, true) &&
		   Snapshot_CheckRecords(bytes, version, layout == XW_REPLAY_LAYOUT_LEGACY);
}

bool XwReplaySnapshot_Apply(uint8_t* bytes, size_t size, XwGameVersion version, XwReplayLayout layout,
							XwFlightUpdateRate rate) {
	if (!XwProfile_HasActiveFlight() || XwProfile_ActiveFlight()->version != version ||
		!XwReplaySnapshot_Check(bytes, size, version, layout, rate))
		return false;
	if (!XwProfile_RestoreMissionTiming(rate) ||
		!Snapshot_Image(bytes, size, version, layout, rate, true, false))
		return false;
	if (layout == XW_REPLAY_LAYOUT_LEGACY)
		XwFlightTiming_BeginSession();
	XwRenderCapture_WorldChanged();
	return true;
}
