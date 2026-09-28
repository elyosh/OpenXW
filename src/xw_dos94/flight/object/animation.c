#ifdef XW_MODERN
#include "xw_dos94/audio/fsfx.h"
#include "xw_runtime/audio/imuse_session.h"
#include "xw_runtime/audio/player_engine.h"
#endif
#include "xw/audio/fsfx.h"
#include "xw/flight/death_star.h"
#include "xw/flight/hud/msg.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/anim.h"
#include "xw/flight/object/create.h"
#include "xw/flight/player/user.h"
#include "xw/flight/shell_flight.h"
#include "xw/flight/xw.h"
#include "xw/math/math2.h"
#include "xw_dos94/assets/models.h"
#include "xw_dos94/flight/object/effects.h"

static uint8_t advance(uint16_t ref, const Dos94Component* descriptor, uint8_t state) {
	if (!descriptor->stateVariants || (unsigned)state + 1 >= descriptor->stateCount)
		return state;
	g_animationFrames = descriptor->stateVariants;
	g_animationFrameIndex = state;
	anim_updateanimstate(ref);
	return g_animationFrameIndex;
}

static void foils(ObjectRecord* object) {
	if (object->objectType != 1 && object->objectType != 118)
		return;
	g_curCraft = object->instanceData;
	if (!(g_curCraft->sFoilState & XW_SFOIL_MOVING))
		return;
	bool closed = (g_curCraft->sFoilState & XW_SFOIL_CLOSED) != 0, changed = false;
	const uint8_t xwing[5] = { 0, 0xf8, 0x0c, 8, 0xf4 };
	unsigned count = object->objectType == 1 ? 5 : 6;
	for (unsigned i = 1; i < count; ++i) {
		uint8_t target = closed ? (object->objectType == 1 ? xwing[i] : 64) : 0;
		uint8_t value = g_curCraft->meshRotation[i];
		if (value == target)
			continue;
		changed = true;
		if (object->objectType == 118) {
			for (int step = 0; step < 3 && value != target; ++step)
				value += closed ? 1 : -1;
		} else {
			int direction = (i == 1 || i == 4) ? -1 : 1;
			value += closed ? direction : -direction;
		}
		g_curCraft->meshRotation[i] = value;
	}
	if (changed)
		return;
	g_curCraft->sFoilState = closed ? XW_SFOIL_CLOSED : 0;
	if (g_flightAudioMode) {
		imuse_stop_sound(g_dos94Imuse, FSFX_SFOIL_MOVEMENT_SLOT);
		fsfx_triggersfx(FSFX_SFOIL_FINISHED_SLOT, g_playerFlightState.objectIndex);
	}
	g_msgArgTable[0] = closed ? XW_MSG_SFOILS_CLOSED_POSITION : XW_MSG_SFOILS_OPEN_POSITION;
	msg_messageprintf(XW_MSG_SFOILS_POSITION_REACHED);
}

/* DOS94 0x6957E0: descriptor animation and polygon component counts are separate. */
void Dos94_anim_updateanimation(void) {
	if (g_flightGlobalCountdownTimers.ticks[XW_TIMER_ANIMATION])
		return;
	g_flightGlobalCountdownTimers.ticks[XW_TIMER_ANIMATION] = 29;
	if (g_deathStarSurfaceModeActive)
		anim_AdvanceSurfaceObjectStateCounters();
	for (uint16_t index = 0; index < XW_OBJECT_COUNT; ++index) {
		ObjectRecord* object = &g_objectTable[index];
		if (!object->objectType)
			continue;
		const Dos94Model* model = Dos94Assets_Model(object->objectType);
		if (!model)
			continue;
		g_currentModelObjectGenus = object->genusId;
		CraftData* craft = object->instanceData;
		for (unsigned i = 0; i < model->metadata.descriptorCount; ++i) {
			const Dos94Component* descriptor = &model->metadata.components[i];
			if (object->genusId <= XW_GENUS_STARSHIP) {
				craft->componentState[i] = advance(index, descriptor, craft->componentState[i]);
				g_curCraft = object->instanceData;
				if (g_curCraft->objectKind == 3) {
					if (!g_deathStarSurfaceModeActive && model->metadata.maxBoundsExtent > 2800) {
						Dos94_starship_createstarshipexplo__partial(index);
						i += 3;
					} else {
						if ((uint16_t)math2_getrandom() < 0x100)
							create_blowoffcomponent(index, 0);
						if ((uint16_t)math2_getrandom() < 0x1800)
							create_createember(index);
					}
				}
			} else if (object->genusId == 10 || object->genusId == 13) {
				uint8_t* state = i == 0 ? &object->animationState : &object->secondaryAnimationState;
				*state = advance(index, descriptor, *state);
				if (object->objectType == 43 && (uint16_t)math2_getrandom() < 0x800)
					create_createember(index);
			}
		}
		foils(object);
	}
	for (uint16_t index = 0; index < MISSION_OBJECT_COUNT; ++index) {
		XwMissionObjectRecord* object = &g_missionObjects[index];
		if (!object->objectType || object->genusId < 7 || object->genusId > 10)
			continue;
		const Dos94Model* model = Dos94Assets_Model(object->objectType);
		if (!model)
			continue;
		g_currentModelObjectGenus = object->genusId;
		for (unsigned i = 0; i < model->metadata.descriptorCount; ++i) {
			uint8_t* state = i == 0 ? &object->stateByte : &object->typeSpecificByte;
			*state = advance(index + XW_MISSION_OBJECT_REF_BASE, &model->metadata.components[i], *state);
		}
	}
}
