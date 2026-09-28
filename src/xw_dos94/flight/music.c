#include "xw_dos94/flight/music.h"
#include "xw/flight/ai/pai.h"
#include "xw/flight/death_star.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/create.h"
#include "xw/flight/player/user.h"
#include "xw/flight/shell_flight.h"
#include "xw/flight/xw.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/math/trig2.h"
#include "xw_dos94/audio/fscript.h"
#include "xw_runtime/timing/flight_timing.h"
#include <limits.h>
// GLOBAL: DOS94 0x5BFDE
int16_t g_dos94DynamicMusicState;
// GLOBAL: DOS94 0x5BFE4
int16_t g_dos94DynamicMusicIntensity;
// GLOBAL: DOS94 0x5C004
int16_t g_dos94MusicChangeCooldownTicks;
// GLOBAL: DOS94 0x5D7E8
uint8_t g_dos94DynamicMusicCombatEntered;
// GLOBAL: DOS94 0x5D7FB
uint8_t g_dos94DynamicMusicLastState;

// FUNCTION: DOS94 0x681804
void Dos94_Xw_UpdateDynamicMusicState(void) {
	if (!XwFlightTiming_ReferenceDue() || !g_musicEventDispatchGate || !g_playerFlightState.craft ||
		!g_playerFlightState.object)
		return;
	g_dos94MusicChangeCooldownTicks -= XwFlightTiming_ReferenceElapsed();
	if (g_dos94MusicChangeCooldownTicks > 0)
		return;
	if (!g_flightAudioMode || !g_flightMusicEnabled)
		return;
	g_dos94MusicChangeCooldownTicks = 59;
	CraftData* craft = g_playerFlightState.craft;
	ObjectRecord* player = g_playerFlightState.object;
	int state = -1, intensity = g_dos94DynamicMusicIntensity;
	if (g_missionRuntimeState.provingGroundsActive) {
		state = g_missionRuntimeState.provingGroundsCheckpointsRemaining < 15 ? 8 : 7;
		intensity = craft->engineThrottle[0] >> 14;
	} else {
		if (g_missionElapsedClock.seconds < 15 && craft->shieldEnergy[0] + craft->shieldEnergy[1] == 0)
			state = 3;
		unsigned int target = g_playerFlightState.currentTargetObjectIdx;
		if (target < XW_OBJECT_COUNT && g_objectTable[target].iff != player->iff) {
			unsigned int cone = g_dos94DynamicMusicLastState == 2 ? 0x2800 : 0x800;
			pai_distancebetween(g_playerFlightState.objectIndex, target);
			uint16_t yaw_delta = (uint16_t)(g_trig2Yaw - player->yaw);
			uint16_t pitch_delta = (uint16_t)(g_trig2Pitch - craft->pitch);
			if (yaw_delta >= 0x8000)
				yaw_delta = (uint16_t)(0u - yaw_delta);
			if (pitch_delta >= 0x8000)
				pitch_delta = (uint16_t)(0u - pitch_delta);
			if (g_trig2PolarDistance < 0x4000 && yaw_delta < cone && pitch_delta < cone)
				state = 2;
		}
		if (state < 0 && g_deathStarSurfaceModeActive)
			state = g_missionRuntimeState.objectivesCompleted ? 6 : 5;
		if (state < 0) {
			uint32_t nearest = UINT32_MAX;
			for (int i = 0; i < XW_CRAFT_OBJECT_COUNT; ++i) {
				ObjectRecord* o = &g_objectTable[i];
				if (!o->objectType || o->iff != (player->iff ^ 1) || !o->instanceData ||
					!((CraftData*)o->instanceData)->workingSubsystems)
					continue;
				pai_roughdistancebetween(i, g_playerFlightState.objectIndex);
				uint32_t distance = g_targetRangeScore;
				if (o->genusId == 4)
					distance >>= 2;
				else if (o->genusId == 3)
					distance >>= 1;
				if (distance < nearest)
					nearest = distance;
			}
			unsigned int groups = g_missionHeader.flightGroupCount;
			if (groups > MISSION_FLIGHT_GROUP_COUNT)
				groups = MISSION_FLIGHT_GROUP_COUNT;
			bool pending = false;
			unsigned int arrival = 65535;
			for (unsigned int i = 0; i < groups; ++i) {
				MissionFlightGroupState* g = &g_missionFlightGroupStates[i];
				if (!g->hasArrived || g->wavesRemaining != 0)
					pending = true;
				if (g->arrivalDelayPending && g->arrivalDelayTimer < arrival)
					arrival = g->arrivalDelayTimer;
			}
			uint32_t threshold = g_dos94DynamicMusicCombatEntered ? 0x40000u : 0x10000u;
			if (nearest == UINT32_MAX) {
				if (pending) {
					state = 4;
					intensity = (int)(arrival / 30);
					if (intensity > 3)
						intensity = 3;
				} else {
					state = g_missionRuntimeState.objectivesCompleted ? 6 : 0;
					intensity = 5;
				}
			} else if (nearest > threshold) {
				state = 0;
				intensity = 5 - (int)((nearest - threshold) >> 15);
				if (intensity < 0)
					intensity = 0;
			} else {
				g_dos94DynamicMusicCombatEntered = 1;
				state = 1;
				intensity = 5;
			}
		}
	}
	if (intensity > 5)
		intensity = 5;
	g_dos94DynamicMusicLastState = state;
	g_dos94DynamicMusicIntensity = intensity;
	g_dos94DynamicMusicState = (int16_t)state;
	Dos94_fscript_DispatchMusicEvent(state);
	Dos94_fscript_SetAttributeValue(0, intensity);
	Dos94_fscript_MsRefreshScript();
}
