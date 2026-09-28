#include "xw/flight/object/laser.h"

#ifdef XW_MODERN
#include "xw_runtime/runtime/flight_math.h"
#include "xw_runtime/runtime/flight_types.h"
#include "xw_runtime/timing/flight_timing.h"
#endif

#include "xw/audio/fsfx.h"
#include "xw/flight/ai/pai.h"
#include "xw/flight/hud/msg.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/mission/spec.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/starship.h"
#include "xw/flight/object/static.h"
#include "xw/flight/player/user.h"
#include "xw/flight/xw.h"
#include "xw/math/math2.h"
#include "xw/math/trig2.h"

#include <stdlib.h>

// GLOBAL: XW 0x4C7850
const int16_t g_projectileBaseDamageByType[LASER_PROJECTILE_TYPE_COUNT] = { 250, 500, 200,   400,
																			200, 400, 10000, 3000 };
// GLOBAL: XW 0x4C7860
const int16_t g_projectileSpeedByType[LASER_PROJECTILE_TYPE_COUNT] = { 1000, 1000, 900, 900,
																	   350,  375,  250, 500 };
// GLOBAL: XW 0x4C7870
const int16_t g_projectileLifetimeSecondsByType[LASER_PROJECTILE_TYPE_COUNT] = { 2, 3, 2, 3, 5, 5, 60, 30 };
// GLOBAL: XW 0x4C7880
const int16_t g_projectileLaunchOffsetByType[LASER_PROJECTILE_TYPE_COUNT] = { 2048, 2048, 2048, 2048,
																			  2048, 2048, 512,  512 };

// GLOBAL: XW 0x62B620
WarheadGuidanceState g_warheadGuidanceTable[LASER_WARHEAD_GUIDANCE_COUNT] = { 0 };

// FUNCTION: XW 0x410510
void laser_weaponsfire(void) {
	int powerObjectIndex;
	uint16_t objectIndex, mineIndex;
	if (g_playerFlightState.selectedWeaponMode != 0) {
		if (g_playerFlightState.currentTargetObjectIdx != XW_OBJECT_SLOT_UNAVAILABLE) {
			uint16_t maxLockAngleScore;
			unsigned int maxLockRange;
			int targetDistance;
			pai_distancebetween(g_playerFlightState.objectIndex, g_playerFlightState.currentTargetObjectIdx);
			targetDistance = g_trig2PolarDistance;
			if (targetDistance >= LASER_LOCK_ANGLE_DISTANCE_LIMIT)
				maxLockAngleScore = LASER_LOCK_MIN_ANGLE_SCORE;
			else
				maxLockAngleScore =
					(uint16_t)(LASER_LOCK_NEAR_ANGLE_SCORE -
							   LASER_LOCK_ANGLE_SCALE * (targetDistance >> LASER_LOCK_DISTANCE_SHIFT));
			maxLockRange = LASER_LOCK_RANGE;
			if (g_playerFlightState.currentTargetObjectIdx < XW_CRAFT_OBJECT_COUNT) {
				uint8_t genus = g_objectTable[g_playerFlightState.currentTargetObjectIdx].genusId;
				if (genus == XW_GENUS_STARSHIP || genus == XW_GENUS_FREIGHTER)
					maxLockRange = LASER_LOCK_LARGE_RANGE;
			}
			if ((unsigned int)targetDistance < maxLockRange &&
				user_targetincross(g_playerFlightState.currentTargetObjectIdx, maxLockAngleScore) != 0) {
				g_playerFlightState.craft->warheadLockTicks += g_elapsedTicks;
				g_playerFlightState.missileLockState =
					g_playerFlightState.craft->warheadLockTicks >=
							LASER_LOCK_SECONDS * XW_SIMULATION_TICKS_PER_SECOND
						? LASER_LOCK_COMPLETE
						: LASER_LOCK_ACQUIRING;
			} else {
				int16_t lockTicks = g_playerFlightState.craft->warheadLockTicks;
				if (lockTicks > 0) {
					g_playerFlightState.craft->warheadLockTicks = (int16_t)(lockTicks - g_elapsedTicks);
					if (g_playerFlightState.craft->warheadLockTicks < 0)
						g_playerFlightState.craft->warheadLockTicks = 0;
				}
				g_playerFlightState.missileLockState = 0;
			}
		} else {
			g_playerFlightState.missileLockState = 0;
			g_playerFlightState.craft->warheadLockTicks = 0;
		}
	}
	if (g_flightGlobalCountdownTimers.ticks[XW_TIMER_WEAPON_POWER] == 0
#ifdef XW_MODERN
		&& XwFlightTiming_ReferenceDue()
#endif
	) {
		g_flightGlobalCountdownTimers.ticks[XW_TIMER_WEAPON_POWER] = XW_SIMULATION_TICKS_PER_SECOND;
		for (powerObjectIndex = 0; (uint16_t)powerObjectIndex < XW_CRAFT_OBJECT_COUNT; ++powerObjectIndex) {
			ObjectRecord* powerObject = &g_objectTable[powerObjectIndex];
			CraftData* craft;
			if (powerObject->objectType == XW_OBJ_NONE || powerObject->familyId != 0 ||
				(powerObject->genusId != XW_GENUS_STARFIGHTER && powerObject->genusId != XW_GENUS_TRANSPORT))
				continue;
			craft = (CraftData*)powerObject->instanceData;
			g_curCraft = craft;
			if ((uint16_t)powerObjectIndex != g_playerFlightState.objectIndex) {
				int16_t nominalFrontShield =
					g_craftTypeDefs[craft->craftTypeIndex].nominalShieldEnergy[LASER_FRONT_SHIELD];
				int16_t totalLaserCharge;
				uint16_t fixedLaserCount, slotIndex;
				if (nominalFrontShield != 0) {
					uint16_t maxFrontShield =
						(uint16_t)(XW_SHIELD_MAX_CHARGE_MULTIPLIER * nominalFrontShield);
					int16_t frontShield = craft->shieldEnergy[LASER_FRONT_SHIELD];
					uint8_t shieldRechargeLevel;
					if (frontShield <= 0)
						shieldRechargeLevel = LASER_RECHARGE_MAX;
					else {
						uint16_t fraction = (uint16_t)math2_percentage(frontShield, maxFrontShield);
						if (fraction < LASER_SHIELD_LOW_FRACTION)
							shieldRechargeLevel = LASER_RECHARGE_MAX;
						else
							shieldRechargeLevel =
								(fraction < LASER_SHIELD_HIGH_FRACTION) + LASER_RECHARGE_NEUTRAL;
						craft = g_curCraft;
					}
					craft->shieldRedirect = shieldRechargeLevel;
					craft = g_curCraft;
				}
				totalLaserCharge = 0;
				fixedLaserCount = 0;
				for (slotIndex = 0; slotIndex < craft->laserSlotCount; ++slotIndex) {
					if (craft->weaponSlots[slotIndex].firingGate == 0) {
						totalLaserCharge =
							(int16_t)(totalLaserCharge + craft->weaponSlots[slotIndex].laserCharge);
						++fixedLaserCount;
					}
				}
				if (fixedLaserCount != 0) {
					int averageLaserCharge = totalLaserCharge / (int)fixedLaserCount;
					uint8_t laserRechargeLevel;
					if ((int16_t)averageLaserCharge < LASER_CHARGE_LOW)
						laserRechargeLevel = LASER_RECHARGE_MAX;
					else
						laserRechargeLevel =
							((int16_t)averageLaserCharge < LASER_CHARGE_HIGH) + LASER_RECHARGE_NEUTRAL;
					craft->laserRedirect = laserRechargeLevel;
					craft = g_curCraft;
				}
			}
			if (craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_SHIELDS) {
				int16_t shieldEnergyDelta =
					(int16_t)(LASER_SHIELD_RECHARGE_STEP * (craft->shieldRedirect - LASER_RECHARGE_NEUTRAL));
				if (shieldEnergyDelta != 0) {
					uint8_t mode = craft->shieldDistribMode;
					if (mode == LASER_SHIELD_DISTRIBUTE_FRONT)
						laser_chargeshields(LASER_FRONT_SHIELD, shieldEnergyDelta);
					else if (mode == LASER_SHIELD_DISTRIBUTE_REAR)
						laser_chargeshields(LASER_REAR_SHIELD, shieldEnergyDelta);
					else {
						int16_t splitDelta = shieldEnergyDelta / 2;
						laser_chargeshields(LASER_FRONT_SHIELD, splitDelta);
						laser_chargeshields(LASER_REAR_SHIELD, splitDelta);
					}
					craft = g_curCraft;
				}
			}
			if (craft->workingSubsystems & LASER_CANNON_SUBSYSTEM_MASK) {
				uint16_t laserSlotIndex;
				for (laserSlotIndex = 0; laserSlotIndex < craft->laserSlotCount; ++laserSlotIndex) {
					if (craft->weaponSlots[laserSlotIndex].firingGate == 0) {
						unsigned int chargeSlotIndex = laserSlotIndex;
						int16_t laserChargeDelta = (int16_t)(LASER_CHARGE_RECHARGE_STEP *
															 (craft->laserRedirect - LASER_RECHARGE_NEUTRAL));
						craft->weaponSlots[chargeSlotIndex].laserCharge += laserChargeDelta;
						craft = g_curCraft;
						if (laserChargeDelta < 0 && craft->weaponSlots[chargeSlotIndex].laserCharge < 0) {
							craft->weaponSlots[chargeSlotIndex].laserCharge = 0;
							craft = g_curCraft;
						}
						if (laserChargeDelta > 0 && craft->weaponSlots[chargeSlotIndex].laserCharge < 0) {
							craft->weaponSlots[chargeSlotIndex].laserCharge = LASER_CHARGE_MAX;
							craft = g_curCraft;
						}
					}
				}
			}
		}
	}
	for (objectIndex = 0; objectIndex < XW_CRAFT_OBJECT_COUNT; ++objectIndex) {
		ObjectRecord* object = &g_objectTable[objectIndex];
		CraftData* weaponCraft;
		int cannonClassIndex;
		uint16_t turretSlotIndex, launcherIndex;
		if (object->objectType == XW_OBJ_NONE || object->familyId != 0)
			continue;
		weaponCraft = (CraftData*)object->instanceData;
		g_curCraft = weaponCraft;
#ifdef XW_MODERN
		/* Player cooldowns remain live; autonomous bursts retain their reference frame delay. */
		if (objectIndex == g_playerFlightState.objectIndex || XwFlightTiming_ReferenceDue()) {
			XwFlightClock cannonClock = { g_elapsedTicks, g_simStepScale };
			if (objectIndex != g_playerFlightState.objectIndex)
				cannonClock = XwFlightTiming_EnterReference();
#endif
			for (cannonClassIndex = 0; (uint16_t)cannonClassIndex < weaponCraft->cannonClassCount;
				 ++cannonClassIndex) {
				uint16_t group = (uint16_t)cannonClassIndex;
				int16_t cooldown = weaponCraft->laserState.fireCooldownTicks[group];
				if (cooldown != 0) {
					cooldown = (int16_t)(cooldown - g_elapsedTicks);
					if (cooldown < 0)
						cooldown = 0;
					weaponCraft->laserState.fireCooldownTicks[group] = cooldown;
					weaponCraft = g_curCraft;
				}
				if (objectIndex != g_playerFlightState.objectIndex && cooldown < (int16_t)g_elapsedTicks &&
					weaponCraft->laserState.linkMode[group] != 0) {
					if ((weaponCraft->workingSubsystems & LASER_CANNON_SUBSYSTEM_MASK) &&
						weaponCraft->objectKind == 0) {
						laser_firelasersystem(objectIndex, cannonClassIndex);
						weaponCraft = g_curCraft;
					}
					--weaponCraft->laserState.burstShotsRemaining[group];
					g_curCraft->laserState.fireCooldownTicks[group] +=
						LASER_BURST_FRAME_DELAY * g_elapsedTicks;
					weaponCraft = g_curCraft;
					if (weaponCraft->laserState.burstShotsRemaining[group] == 0) {
						weaponCraft->laserState.linkMode[group] = 0;
						weaponCraft = g_curCraft;
					}
				}
			}
#ifdef XW_MODERN
			XwFlightTiming_RestoreClock(cannonClock);
		}
#endif
		for (turretSlotIndex = 0; turretSlotIndex < weaponCraft->laserSlotCount; ++turretSlotIndex) {
			int16_t firingGate = weaponCraft->weaponSlots[turretSlotIndex].firingGate;
			if (firingGate > 0
#ifdef XW_MODERN
				&& XwFlightTiming_ReferenceDue()
#endif
			) {
#ifdef XW_MODERN
				XwFlightClock turretClock = XwFlightTiming_EnterReference();
#endif
				starship_firelasergunner(objectIndex, turretSlotIndex, (uint16_t)(firingGate - 1));
#ifdef XW_MODERN
				XwFlightTiming_RestoreClock(turretClock);
#endif
				weaponCraft = g_curCraft;
			}
		}
		for (launcherIndex = 0; launcherIndex < weaponCraft->warheadLauncherCount; ++launcherIndex) {
			int16_t cooldown = weaponCraft->warheadFireTimers[launcherIndex];
			if (cooldown != 0) {
				cooldown = (int16_t)(cooldown - g_elapsedTicks);
				if (cooldown < 0)
					cooldown = 0;
				weaponCraft->warheadFireTimers[launcherIndex] = cooldown;
				weaponCraft = g_curCraft;
			}
		}
	}
	for (mineIndex = 0; mineIndex < MISSION_OBJECT_COUNT; ++mineIndex) {
		if (g_missionObjects[mineIndex].objectType != XW_OBJ_NONE &&
			g_missionObjects[mineIndex].genusId == XW_GENUS_MINE
#ifdef XW_MODERN
			&& XwFlightTiming_ReferenceDue()
#endif
		) {
#ifdef XW_MODERN
			XwFlightClock mineClock = XwFlightTiming_EnterReference();
#endif
			static_updatemineguns(mineIndex);
#ifdef XW_MODERN
			XwFlightTiming_RestoreClock(mineClock);
#endif
		}
	}
}

// FUNCTION: XW 0x410A40
void laser_chargeshields(uint16_t shieldIndex, int16_t delta) {
	int16_t maxShieldEnergy = g_craftTypeDefs[g_curCraft->craftTypeIndex].nominalShieldEnergy[shieldIndex] *
							  XW_SHIELD_MAX_CHARGE_MULTIPLIER;
	g_curCraft->shieldEnergy[shieldIndex] += delta;
	if (g_curCraft->shieldEnergy[shieldIndex] < 0) {
		g_curCraft->shieldEnergy[shieldIndex] = 0;
	}
	if (g_curCraft->shieldEnergy[shieldIndex] > maxShieldEnergy) {
		g_curCraft->shieldEnergy[shieldIndex] = maxShieldEnergy;
	}
}

// FUNCTION: XW 0x410AC0
void laser_fireplayerweapon(void) {
	int16_t* cooldown;
	int16_t adjustedCooldown;
	if (g_playerFlightState.selectedWeaponMode == 0) {
		cooldown =
			&g_playerFlightState.craft->laserState.fireCooldownTicks[g_playerFlightState.selectedWeaponBank];
		adjustedCooldown = *cooldown;
		if (adjustedCooldown != 0) {
			adjustedCooldown -= g_elapsedTicks;
			if (adjustedCooldown < 0)
				adjustedCooldown = 0;
		}
		if (adjustedCooldown < (int16_t)g_elapsedTicks) {
			if (adjustedCooldown != 0)
				*cooldown -= g_elapsedTicks;
			else
				*cooldown = 0;
			if ((g_playerFlightState.craft->workingSubsystems & LASER_CANNON_SUBSYSTEM_MASK) != 0) {
				laser_firelasersystem(g_playerFlightState.objectIndex,
									  g_playerFlightState.selectedWeaponBank);
				g_playerFlightState.craft->laserState
					.fireCooldownTicks[g_playerFlightState.selectedWeaponBank] += g_elapsedTicks;
			} else {
				g_msgArgTable[0] = g_playerFlightState.selectedWeaponBank + XW_MSG_SUBSYSTEM_LASER;
				g_msgArgTable[1] = XW_MSG_SUBSYSTEM_DAMAGED_AND_INOPERATIVE;
				msg_messageprintf(XW_MSG_SUBSYSTEM_STATUS);
			}
		}
	} else {
		cooldown = &g_playerFlightState.craft->warheadFireTimers[g_playerFlightState.selectedWeaponBank];
		adjustedCooldown = *cooldown;
		if (adjustedCooldown != 0) {
			adjustedCooldown -= g_elapsedTicks;
			if (adjustedCooldown < 0)
				adjustedCooldown = 0;
		}
		if (adjustedCooldown < (int16_t)g_elapsedTicks) {
			if (adjustedCooldown != 0)
				*cooldown -= g_elapsedTicks;
			else
				*cooldown = 0;
			if ((g_playerFlightState.craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_LAUNCHER) != 0) {
				laser_firerocketsystem(g_playerFlightState.objectIndex,
									   g_playerFlightState.selectedWeaponBank);
				g_playerFlightState.craft->warheadFireTimers[g_playerFlightState.selectedWeaponBank] +=
					g_elapsedTicks;
			} else {
				g_msgArgTable[1] = XW_MSG_SUBSYSTEM_DAMAGED_AND_INOPERATIVE;
#ifdef XW_MODERN
				g_msgArgTable[0] = (g_craftTypeDefs[g_playerFlightState.craftTypeIndex]
										.warheadProjectileType[g_playerFlightState.selectedWeaponBank] !=
									XwFlightTypes_ObjectType(XW_OBJ_WARHEAD_149)) +
								   XW_MSG_SUBSYSTEM_TORPEDO_LAUNCHER;
#else
				g_msgArgTable[0] = (g_craftTypeDefs[g_playerFlightState.craftTypeIndex]
										.warheadProjectileType[g_playerFlightState.selectedWeaponBank] !=
									XW_OBJ_WARHEAD_149) +
								   XW_MSG_SUBSYSTEM_TORPEDO_LAUNCHER;
#endif
				msg_messageprintf(XW_MSG_SUBSYSTEM_STATUS);
			}
		}
	}
}

// FUNCTION: XW 0x410CA0
void laser_firelasersystem(uint16_t sourceObjectIndex, int laserGroup) {
	uint16_t craftTypeIndex;
	g_curCraft = (CraftData*)g_objectTable[sourceObjectIndex].instanceData;
	craftTypeIndex = g_curCraft->craftTypeIndex;
	if (g_curCraft->sFoilState == 0) {
		uint16_t groupIndex;
		uint16_t firstSlot, lastSlot, slotStep, slotsRemaining;
		uint16_t weaponSlot, projectileType;
		uint16_t shotsFired;
		int invalidLinkMode = 0;
		groupIndex = (uint16_t)laserGroup;
		shotsFired = 0;
		switch (g_curCraft->laserState.linkMode[groupIndex]) {
			case LASER_LINK_SINGLE:
				firstSlot = g_curCraft->laserState.nextSlot[groupIndex];
				lastSlot = firstSlot;
				++g_curCraft->laserState.nextSlot[groupIndex];
				if (g_curCraft->laserState.nextSlot[groupIndex] >
					g_craftTypeDefs[craftTypeIndex].laserGroupLastSlot[groupIndex])
					g_curCraft->laserState.nextSlot[groupIndex] =
						g_craftTypeDefs[craftTypeIndex].laserGroupFirstSlot[groupIndex];
				slotsRemaining = 1;
				slotStep = 1;
				break;
			case LASER_LINK_PAIR:
				firstSlot = g_curCraft->laserState.nextSlot[groupIndex];
				g_curCraft->laserState.nextSlot[groupIndex] ^= LASER_PAIR_SLOT_MASK;
				if (g_curCraft->laserState.nextSlot[groupIndex] >
					g_craftTypeDefs[craftTypeIndex].laserGroupLastSlot[groupIndex])
					g_curCraft->laserState.nextSlot[groupIndex] =
						g_craftTypeDefs[craftTypeIndex].laserGroupFirstSlot[groupIndex];
				lastSlot = g_craftTypeDefs[craftTypeIndex].laserGroupLastSlot[groupIndex];
				slotStep = LASER_PAIR_SLOT_STEP;
				slotsRemaining =
					(lastSlot - g_craftTypeDefs[craftTypeIndex].laserGroupFirstSlot[groupIndex] + 1) /
					LASER_PAIR_SLOT_STEP;
				break;
			case LASER_LINK_ALL:
				slotStep = 1;
				firstSlot = g_craftTypeDefs[craftTypeIndex].laserGroupFirstSlot[groupIndex];
				lastSlot = g_craftTypeDefs[craftTypeIndex].laserGroupLastSlot[groupIndex];
				slotsRemaining = lastSlot - firstSlot + 1;
				break;
			default:
#ifdef XW_MODERN
				if (!XwFlightTypes_Dos())
					return;
				firstSlot = 1;
				lastSlot = 0;
				slotStep = 1;
				slotsRemaining = 0;
				break;
#else
				invalidLinkMode = 1;
				break;
#endif
		}
		if (!invalidLinkMode) {
#ifdef XW_MODERN
			projectileType = XwFlightTypes_Dos() ? 0 : sourceObjectIndex;
#else
			projectileType = sourceObjectIndex;
#endif

			for (weaponSlot = firstSlot; weaponSlot <= lastSlot; weaponSlot += slotStep) {
				if (g_curCraft->weaponSlots[weaponSlot].firingGate == 0) {
					int8_t charge = g_curCraft->weaponSlots[weaponSlot].laserCharge;
					if (charge > 0) {
						uint16_t projectileIndex;
						projectileType = g_craftTypeDefs[craftTypeIndex].laserGroupWeaponType[groupIndex];
						if (g_objectTable[sourceObjectIndex].iff == 0 && charge >= LASER_HIGH_CHARGE_THRESHOLD)
							++projectileType;
						projectileIndex = laser_createprojectile(sourceObjectIndex, weaponSlot, projectileType);
						if (projectileIndex != XW_OBJECT_SLOT_UNAVAILABLE) {
							if (g_playerFlightState.objectIndex == sourceObjectIndex) {
								if (g_unlimitedWeaponsEnabled == 0)
									g_curCraft->weaponSlots[weaponSlot].laserCharge -= LASER_PLAYER_CHARGE_COST;
							} else {
								--g_curCraft->weaponSlots[weaponSlot].laserCharge;
							}
							if (shotsFired < LASER_SHOT_SOUND_LIMIT)
								fsfx_triggerlasersfx(projectileIndex);
							if (g_curCraft->weaponSlots[weaponSlot].laserCharge < 0)
								g_curCraft->weaponSlots[weaponSlot].laserCharge = 0;
							++shotsFired;
						}
					}
				}
				--slotsRemaining;
				if (slotsRemaining == 0)
					break;
			}
#ifdef XW_MODERN
			if (projectileType == XwFlightTypes_ObjectType(XW_OBJ_ION_147) ||
				projectileType == XwFlightTypes_ObjectType(XW_OBJ_ION_148)) {
#else
			if (projectileType == XW_OBJ_ION_147 || projectileType == XW_OBJ_ION_148) {
#endif
				g_curCraft->weaponStats.ionShotsFired += shotsFired;
				if (sourceObjectIndex == g_playerFlightState.objectIndex)
					g_playerFlightState.weaponStats.ionShotsFired += shotsFired;
			} else {
				g_curCraft->weaponStats.laserShotsFired += shotsFired;
				if (sourceObjectIndex == g_playerFlightState.objectIndex)
					g_playerFlightState.weaponStats.laserShotsFired += shotsFired;
			}
			g_curCraft->laserState.fireCooldownTicks[groupIndex] =
				LASER_COOLDOWN_PER_SHOT * shotsFired + LASER_COOLDOWN_BASE;
		}
	}
}

// FUNCTION: XW 0x410FD0
void laser_firerocketsystem(uint16_t objectIndex, uint16_t launcherIndex) {
	uint16_t shotsFired = 0;
	uint16_t modelIndex;
	uint16_t firstWeaponSlot;
	uint16_t launcherFlags;
	g_curCraft = (CraftData*)g_objectTable[objectIndex].instanceData;
	launcherFlags = g_curCraft->warheadLauncherFlags[launcherIndex];
	modelIndex = g_curCraft->craftTypeIndex;
	firstWeaponSlot = g_craftTypeDefs[modelIndex].warheadFirstSlot[launcherIndex];
	if ((launcherFlags & XW_LAUNCHER_FIRE_MODE_MASK) == XW_LAUNCHER_FIRE_LINKED) {
		if (laser_firemissile(objectIndex, firstWeaponSlot,
							  g_craftTypeDefs[modelIndex].warheadProjectileType[launcherIndex],
							  launcherIndex) != XW_OBJECT_SLOT_UNAVAILABLE)
			shotsFired = 1;
		if (laser_firemissile(objectIndex, firstWeaponSlot + 1,
							  g_craftTypeDefs[modelIndex].warheadProjectileType[launcherIndex],
							  launcherIndex) != XW_OBJECT_SLOT_UNAVAILABLE)
			++shotsFired;
	} else {
		if ((launcherFlags & LASER_LAUNCHER_SIDE_MASK) != 0) {
			if (laser_firemissile(objectIndex, firstWeaponSlot + 1,
								  g_craftTypeDefs[modelIndex].warheadProjectileType[launcherIndex],
								  launcherIndex) != XW_OBJECT_SLOT_UNAVAILABLE)
				shotsFired = 1;
		} else {
			if (laser_firemissile(objectIndex, firstWeaponSlot,
								  g_craftTypeDefs[modelIndex].warheadProjectileType[launcherIndex],
								  launcherIndex) != XW_OBJECT_SLOT_UNAVAILABLE)
				shotsFired = 1;
		}
	}
	g_curCraft->warheadFireTimers[launcherIndex] =
		LASER_WARHEAD_COOLDOWN_SECONDS * XW_SIMULATION_TICKS_PER_SECOND;
	if (objectIndex == g_playerFlightState.objectIndex) {
#ifdef XW_MODERN
		g_msgArgTable[0] =
			XwFlightTypes_CanonicalType(g_craftTypeDefs[g_playerFlightState.craftTypeIndex]
											.warheadProjectileType[g_playerFlightState.selectedWeaponBank]) -
			LASER_WARHEAD_MESSAGE_TYPE_BASE;
#else
		g_msgArgTable[0] = g_craftTypeDefs[g_playerFlightState.craftTypeIndex]
							   .warheadProjectileType[g_playerFlightState.selectedWeaponBank] -
						   LASER_WARHEAD_MESSAGE_TYPE_BASE;
#endif
		msg_messageprintf(XW_MSG_WARHEAD_LAUNCHER_EMPTY + shotsFired);
	}
}

// FUNCTION: XW 0x411130
uint16_t laser_firemissile(uint16_t objectIndex, uint16_t weaponSlotIndex, uint16_t projectileTypeId,
						   uint16_t launcherIndex) {
	uint16_t guidanceIndex = XW_OBJECT_SLOT_UNAVAILABLE;
	uint16_t playerObjectIndex;

	if (g_curCraft->weaponSlots[weaponSlotIndex].firingGate == 0 &&
		g_curCraft->weaponSlots[weaponSlotIndex].count > 0) {
		guidanceIndex = laser_createprojectile(objectIndex, weaponSlotIndex, projectileTypeId);
		if (guidanceIndex != XW_OBJECT_SLOT_UNAVAILABLE) {
			g_curCraft->warheadLauncherFlags[launcherIndex] ^= LASER_LAUNCHER_SIDE_MASK;
			++g_curCraft->weaponStats.warheadsFired;
			if (objectIndex == g_playerFlightState.objectIndex)
				++g_playerFlightState.weaponStats.warheadsFired;
			fsfx_triggerlasersfx(guidanceIndex);
			if (g_unlimitedWeaponsEnabled == 0 ||
				objectIndex != (playerObjectIndex = g_playerFlightState.objectIndex)) {
				--g_curCraft->weaponSlots[weaponSlotIndex].count;
				playerObjectIndex = g_playerFlightState.objectIndex;
			}

			guidanceIndex -= XW_CRAFT_OBJECT_COUNT;
			g_warheadGuidanceTable[guidanceIndex].homingTier =
				g_curCraft->warheadLockTicks / XW_SIMULATION_TICKS_PER_SECOND;
			if (g_warheadGuidanceTable[guidanceIndex].homingTier > LASER_MAX_HOMING_TIER)
				g_warheadGuidanceTable[guidanceIndex].homingTier = LASER_MAX_HOMING_TIER;
			if (playerObjectIndex == objectIndex)
				g_warheadGuidanceTable[guidanceIndex].targetObjIdx =
					g_playerFlightState.currentTargetObjectIdx;
			else
				g_warheadGuidanceTable[guidanceIndex].targetObjIdx = g_curCraft->aiTargetRef;
			if (g_warheadGuidanceTable[guidanceIndex].targetObjIdx == playerObjectIndex) {
				msg_messageprintf(XW_MSG_INCOMING_MISSILE_TARGET_PROMPT);
				fsfx_triggersfx(LASER_INCOMING_MISSILE_SOUND, XW_OBJECT_SLOT_UNAVAILABLE);
				if (fsfx_speakeravailable() != 0) {
					fsfx_triggervoicesfx(LASER_INCOMING_MISSILE_VOICE_FIRST);
					fsfx_triggervoicesfx(LASER_INCOMING_MISSILE_VOICE_SECOND);
				}
				g_playerFlightState.incomingWarheadAlertState = LASER_INCOMING_ALERT_ACTIVE;
				g_playerFlightState.incomingWarheadObjectIndex = guidanceIndex + XW_CRAFT_OBJECT_COUNT;
				g_flightGlobalCountdownTimers.ticks[XW_TIMER_INCOMING_WARHEAD] =
					LASER_INCOMING_ALERT_SECONDS * XW_SIMULATION_TICKS_PER_SECOND;
			}
		}
	}
	return guidanceIndex;
}

// FUNCTION: XW 0x4112D0
uint16_t laser_createprojectile(uint16_t sourceObjectIndex, uint16_t weaponSlotIndex,
								uint16_t projectileObjectType) {
	uint16_t projectileGenus =
		(g_playerFlightState.objectIndex != sourceObjectIndex) + XW_GENUS_PLAYER_PROJECTILE;
	uint16_t projectileIndex = create_findslot(projectileGenus);
	ObjectRecord* sourceObject;
	uint16_t modelIndex;
	uint16_t sourceType;
	int propertyIndex;
	int16_t launchOffset;
	int64_t product;
	int32_t worldX, worldY, worldZ;
	int32_t launchX, launchY, launchZ;
	uint16_t guidanceIndex;
#ifdef XW_MODERN
	propertyIndex = XwFlightTypes_ProjectileIndex(projectileObjectType);
	if (propertyIndex < 0)
		return XW_OBJECT_SLOT_UNAVAILABLE;
#endif
	if (projectileIndex != XW_OBJECT_SLOT_UNAVAILABLE) {
#ifdef XW_MODERN
		XwLaserAim aim;
#endif
		sourceObject = &g_objectTable[sourceObjectIndex];
		sourceType = sourceObject->objectType;
		g_objectTable[projectileIndex].familyId = LASER_PROJECTILE_FAMILY;
		g_objectTable[projectileIndex].genusId = projectileGenus;
		g_objectTable[projectileIndex].objectType = projectileObjectType;
		g_objectTable[projectileIndex].ageSeconds = LASER_PROJECTILE_INITIAL_AGE;
		g_objectTable[projectileIndex].sourceObjectRef = sourceObjectIndex;
		g_objectTable[projectileIndex].sourceObjectType = sourceType;
		modelIndex = spec_getspecnum(sourceType);
		g_objectTable[projectileIndex].iff = sourceObject->iff;
		g_objectTable[projectileIndex].pitch = sourceObject->pitch;
		g_objectTable[projectileIndex].roll = sourceObject->roll;
		g_objectTable[projectileIndex].yaw = sourceObject->yaw;
#ifndef XW_MODERN
		propertyIndex = projectileObjectType - LASER_PROJECTILE_FIRST_TYPE;
#endif
		g_objectTable[projectileIndex].speed = sourceObject->speed + g_projectileSpeedByType[propertyIndex];
		g_objectTable[projectileIndex].damageAmount =
			sourceObject->speed + g_projectileBaseDamageByType[propertyIndex];
		g_objectTable[projectileIndex].lifetimeTicks =
			XW_SIMULATION_TICKS_PER_SECOND * g_projectileLifetimeSecondsByType[propertyIndex];
		worldY = sourceObject->worldY;
		worldX = sourceObject->worldX;
		worldZ = sourceObject->worldZ;
		pai_calcrotatedpoint(sourceObject, g_craftTypeDefs[modelIndex].weaponHardpoints[weaponSlotIndex].x,
							 g_craftTypeDefs[modelIndex].weaponHardpoints[weaponSlotIndex].z,
							 g_craftTypeDefs[modelIndex].weaponHardpoints[weaponSlotIndex].y);
#ifdef XW_MODERN
		worldX = (int32_t)((uint32_t)worldX + (uint32_t)g_rotatedX);
#else
		worldX += g_rotatedX;
#endif
#ifdef XW_MODERN
		worldY = (int32_t)((uint32_t)worldY + (uint32_t)g_rotatedY);
#else
		worldY += g_rotatedY;
#endif
#ifdef XW_MODERN
		worldZ = (int32_t)((uint32_t)worldZ + (uint32_t)g_rotatedZ);
#else
		worldZ += g_rotatedZ;
#endif
		g_objectTable[projectileIndex].prevWorldX = worldX;
		g_objectTable[projectileIndex].prevWorldY = worldY;
		g_objectTable[projectileIndex].prevWorldZ = worldZ;
		launchOffset = g_projectileLaunchOffsetByType[propertyIndex];
		product = (int64_t)sourceObject->cachedForwardX * launchOffset;
		launchX = (int32_t)(product >> LASER_LAUNCH_BASIS_SHIFT);
		product = (int64_t)sourceObject->cachedForwardY * launchOffset;
		launchY = (int32_t)(product >> LASER_LAUNCH_BASIS_SHIFT);
		product = (int64_t)sourceObject->cachedForwardZ * launchOffset;
		launchZ = (int32_t)(product >> LASER_LAUNCH_BASIS_SHIFT);
#ifdef XW_MODERN
		g_objectTable[projectileIndex].worldX = (int32_t)((uint32_t)worldX + (uint32_t)launchX);
#else
		g_objectTable[projectileIndex].worldX = worldX + launchX;
#endif
#ifdef XW_MODERN
		g_objectTable[projectileIndex].worldY = (int32_t)((uint32_t)worldY + (uint32_t)launchY);
#else
		g_objectTable[projectileIndex].worldY = worldY + launchY;
#endif
#ifdef XW_MODERN
		g_objectTable[projectileIndex].worldZ = (int32_t)((uint32_t)worldZ + (uint32_t)launchZ);
#else
		g_objectTable[projectileIndex].worldZ = worldZ + launchZ;
#endif
		g_objectTable[projectileIndex].moveX = sourceObject->moveX;
		g_objectTable[projectileIndex].moveY = sourceObject->moveY;
		g_objectTable[projectileIndex].moveZ = sourceObject->moveZ;
		g_objectTable[projectileIndex].cachedSideX = sourceObject->cachedSideX;
		g_objectTable[projectileIndex].cachedSideY = sourceObject->cachedSideY;
		g_objectTable[projectileIndex].cachedSideZ = sourceObject->cachedSideZ;
		g_objectTable[projectileIndex].cachedUpX = sourceObject->cachedUpX;
		g_objectTable[projectileIndex].cachedUpY = sourceObject->cachedUpY;
		g_objectTable[projectileIndex].cachedUpZ = sourceObject->cachedUpZ;
		g_objectTable[projectileIndex].cachedForwardX = sourceObject->cachedForwardX;
		g_objectTable[projectileIndex].cachedForwardY = sourceObject->cachedForwardY;
		g_objectTable[projectileIndex].cachedForwardZ = sourceObject->cachedForwardZ;
		g_objectTable[projectileIndex].orientMatrixDirty = 0;
		g_objectTable[projectileIndex].moveVectorDirty = 0;
#ifdef XW_MODERN
		if (!XwFlightTypes_IsWarhead(projectileObjectType) &&
			XwFlightMath_ConvergeLaser(sourceObjectIndex, worldX, worldY, worldZ, &aim)) {
			ObjectRecord* projectile = &g_objectTable[projectileIndex];
			projectile->pitch = aim.pitch;
			projectile->yaw = aim.yaw;
			projectile->roll = 0;
			projectile->moveX = aim.moveX;
			projectile->moveY = aim.moveY;
			projectile->moveZ = aim.moveZ;
			projectile->orientMatrixDirty = 1;
			projectile->moveVectorDirty = 1;
			/* The initial collision segment must follow the converged shot from its muzzle. */
			projectile->worldX =
				(int32_t)((uint32_t)worldX +
						  (uint32_t)(((int64_t)aim.moveX * launchOffset) >> LASER_LAUNCH_BASIS_SHIFT));
			projectile->worldY =
				(int32_t)((uint32_t)worldY +
						  (uint32_t)(((int64_t)aim.moveY * launchOffset) >> LASER_LAUNCH_BASIS_SHIFT));
			projectile->worldZ =
				(int32_t)((uint32_t)worldZ +
						  (uint32_t)(((int64_t)aim.moveZ * launchOffset) >> LASER_LAUNCH_BASIS_SHIFT));
		}
#endif
		guidanceIndex = (uint16_t)(projectileIndex - XW_CRAFT_OBJECT_COUNT);
		g_objectTable[projectileIndex].instanceData = &g_warheadGuidanceTable[guidanceIndex];
		g_warheadGuidanceTable[guidanceIndex].homingTier = 0;
		g_warheadGuidanceTable[guidanceIndex].targetObjIdx = XW_OBJECT_SLOT_UNAVAILABLE;
	}
	return projectileIndex;
}
