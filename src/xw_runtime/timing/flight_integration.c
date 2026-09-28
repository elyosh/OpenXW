/* Fractional integration adapted from OpenXvT, with OpenTIE's Q16 velocity carry. */
#include "xw_runtime/timing/flight_integration.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/laser.h"
#include "xw/flight/object/move.h"
#include "xw/flight/player/user.h"
#include "xw/flight/xw.h"
#include "xw/math/math2.h"
#include "xw/math/trig2.h"
#include "xw_runtime/runtime/flight_camera.h"
#include "xw_runtime/timing/flight_timing.h"
#include "xw_runtime/timing/player_timing.h"
#include "xw_runtime/timing/reference_motion.h"
#include <limits.h>
#include <string.h>

typedef XwIntegrationCarry Carry;
static XwMobileIntegration mobile[XW_OBJECT_COUNT];
static Carry craft[XW_CRAFT_OBJECT_COUNT][XW_INTEGRATE_COUNT - XW_INTEGRATE_ROLL];

void XwFlightIntegration_Save(XwFlightIntegrationState* out) {
	memcpy(out->mobile, mobile, sizeof mobile);
	memcpy(out->craft, craft, sizeof craft);
}

void XwFlightIntegration_Restore(const XwFlightIntegrationState* state) {
	memcpy(mobile, state->mobile, sizeof mobile);
	memcpy(craft, state->craft, sizeof craft);
}

static Carry* Channel(unsigned slot, unsigned channel) {
	if (slot >= XW_OBJECT_COUNT || channel >= XW_INTEGRATE_COUNT)
		return NULL;
	if (channel < XW_INTEGRATE_ROLL)
		return &mobile[slot].channels[channel];
	return slot < XW_CRAFT_OBJECT_COUNT ? &craft[slot][channel - XW_INTEGRATE_ROLL] : NULL;
}

void XwFlightIntegration_ClearAll(void) {
	memset(mobile, 0, sizeof mobile);
	memset(craft, 0, sizeof craft);
}

void XwFlightIntegration_Clear(unsigned slot, unsigned channel) {
	Carry* carry = Channel(slot, channel);
	if (carry)
		*carry = (Carry) { 0 };
}

void XwFlightIntegration_Reset(unsigned slot) {
	if (slot >= XW_OBJECT_COUNT)
		return;
	memset(&mobile[slot], 0, sizeof mobile[slot]);
	if (slot < XW_CRAFT_OBJECT_COUNT)
		memset(craft[slot], 0, sizeof craft[slot]);
	/* A new occupant of a tracked target slot must not inherit guidance carry. */
	for (unsigned other = 0; other < XW_OBJECT_COUNT; ++other) {
		if (mobile[other].tier && mobile[other].target == slot) {
			XwFlightIntegration_Clear(other, XW_INTEGRATE_HOME_YAW);
			XwFlightIntegration_Clear(other, XW_INTEGRATE_HOME_PITCH);
		}
	}
	XwReferenceMotion_Reset(slot);
	XwPlayerTiming_ResetObject(slot);
	XwFlightCamera_ResetObject(slot);
}

void XwFlightIntegration_ClearPush(unsigned slot) {
	for (unsigned channel = XW_INTEGRATE_PUSH_X; channel <= XW_INTEGRATE_PUSH_Z; ++channel)
		XwFlightIntegration_Clear(slot, channel);
}

void XwFlightIntegration_Reposition(unsigned slot) {
	if (slot >= XW_OBJECT_COUNT)
		return;
	memset(mobile[slot].position, 0, sizeof mobile[slot].position);
	XwFlightIntegration_ClearPush(slot);
	XwFlightIntegration_Clear(slot, XW_INTEGRATE_HOME_YAW);
	XwFlightIntegration_Clear(slot, XW_INTEGRATE_HOME_PITCH);
	XwReferenceMotion_Reposition(slot);
	XwPlayerTiming_RepositionObject(slot);
	XwFlightCamera_ResetObject(slot);
}

void XwFlightIntegration_ResetManeuver(unsigned slot) {
	XwFlightIntegration_ClearPush(slot);
	for (unsigned channel = XW_INTEGRATE_ROLL; channel <= XW_INTEGRATE_BANK; ++channel)
		XwFlightIntegration_Clear(slot, channel);
}

void XwFlightIntegration_Observe(unsigned slot) {
	if (!XwFlightTiming_IsUnlocked() || slot >= XW_OBJECT_COUNT)
		return;
	const ObjectRecord* object = &g_objectTable[slot];
	if (!object->objectType) {
		memset(&mobile[slot], 0, sizeof mobile[slot]);
		if (slot < XW_CRAFT_OBJECT_COUNT)
			memset(craft[slot], 0, sizeof craft[slot]);
		XwReferenceMotion_Reset(slot);
		return;
	}
	if (!object->rollImpulseRate)
		XwFlightIntegration_Clear(slot, XW_INTEGRATE_IMPULSE_ROLL);
	if (object->genusId == XW_GENUS_PLAYER_PROJECTILE || object->genusId == XW_GENUS_OTHER_PROJECTILE) {
		const WarheadGuidanceState* guidance = object->instanceData;
		if (!guidance->homingTier || guidance->targetObjIdx == XW_OBJECT_SLOT_UNAVAILABLE ||
			mobile[slot].target != guidance->targetObjIdx || mobile[slot].tier != guidance->homingTier) {
			XwFlightIntegration_Clear(slot, XW_INTEGRATE_HOME_YAW);
			XwFlightIntegration_Clear(slot, XW_INTEGRATE_HOME_PITCH);
		}
		mobile[slot].target = guidance->targetObjIdx;
		mobile[slot].tier = guidance->homingTier;
	}
	if (slot < XW_CRAFT_OBJECT_COUNT && object->familyId == XW_OBJECT_FAMILY_CRAFT) {
		const CraftData* data = object->instanceData;
		const int push[3] = { data->aiDisplacementX, data->aiDisplacementY, data->aiDisplacementZ };
		for (unsigned axis = 0; axis < 3; ++axis)
			if (!data->workingSubsystems || !push[axis])
				XwFlightIntegration_Clear(slot, XW_INTEGRATE_PUSH_X + axis);
	}
}

void XwFlightIntegration_ObserveCraft(unsigned slot) {
	if (!XwFlightTiming_IsUnlocked() || slot >= XW_CRAFT_OBJECT_COUNT)
		return;
	const ObjectRecord* object = &g_objectTable[slot];
	if (!object->objectType || object->familyId != XW_OBJECT_FAMILY_CRAFT) {
		memset(craft[slot], 0, sizeof craft[slot]);
		return;
	}
	const CraftData* data = object->instanceData;
	bool attitude =
		slot != g_playerFlightState.objectIndex && (data->workingSubsystems & XW_CRAFT_SUBSYSTEM_ATTITUDE);
	bool roll =
		attitude && data->aiRollState >= XW_AI_ROLL_STATE_1 && data->aiRollState <= XW_AI_ROLL_STATE_3;
	bool pitch = attitude &&
				 (data->aiPitchState == XW_AI_PITCH_DECREMENT || data->aiPitchState == XW_AI_PITCH_INCREMENT);
	bool yaw = attitude && data->objectKind != XW_CRAFT_OBJECT_KIND_2 &&
			   data->aiYawState >= XW_AI_YAW_STATE_1 && data->aiTargetYaw != (uint16_t)object->yaw &&
			   data->yawRateLimit && data->aiYawRateScaleQ16 && data->aiYawStepQ16;
	if (!roll)
		XwFlightIntegration_Clear(slot, XW_INTEGRATE_ROLL);
	if (!pitch || (Channel(slot, XW_INTEGRATE_PITCH)->direction &&
				   Channel(slot, XW_INTEGRATE_PITCH)->direction !=
					   (data->aiPitchState == XW_AI_PITCH_DECREMENT ? -1 : 1)))
		XwFlightIntegration_Clear(slot, XW_INTEGRATE_PITCH);
	if (!yaw)
		XwFlightIntegration_Clear(slot, XW_INTEGRATE_YAW);
	if (!yaw || (data->aiRollState != 0 && data->aiRollState != XW_AI_ROLL_TARGET_REACHED))
		XwFlightIntegration_Clear(slot, XW_INTEGRATE_BANK);
}

static int64_t Integrate(Carry* carry, int64_t numerator, int64_t divisor, int direction) {
	if (carry) {
		if (carry->direction != direction) {
			carry->remainder = 0;
			carry->direction = direction;
		}
		numerator += carry->remainder;
		carry->remainder = numerator % divisor;
	}
	return numerator / divisor;
}

int XwFlightIntegration_Rate(unsigned slot, unsigned channel, int rate, unsigned elapsed, int divisor) {
	if (divisor <= 0)
		return 0;
	int64_t value =
		Integrate(Channel(slot, channel), (int64_t)rate * elapsed, divisor, (rate > 0) - (rate < 0));
	return value > INT_MAX ? INT_MAX : value < INT_MIN ? INT_MIN : (int)value;
}

unsigned XwFlightIntegration_Steer(unsigned slot, unsigned channel, uint16_t rate, uint16_t accel,
								   uint16_t factor, int direction) {
	uint64_t a = accel == UINT16_MAX ? 65536u : accel;
	uint64_t f = factor == UINT16_MAX ? 65536u : factor;
	/* Split before adding carry: the complete uint16 product fits unsigned, not signed, 64 bits. */
	uint64_t product = (uint64_t)rate * g_elapsedTicks * a * f;
	const int64_t divisor = (int64_t)XW_SIMULATION_TICKS_PER_SECOND * 65536 * 65536;
	if (!rate || !accel || !factor)
		direction = 0;
	uint64_t whole = product / divisor;
	whole += Integrate(Channel(slot, channel), product % divisor, divisor, direction);
	return whole > UINT16_MAX ? UINT16_MAX : (unsigned)whole;
}

unsigned XwFlightIntegration_Bank(unsigned slot, uint16_t step, uint16_t factor, int direction) {
	unsigned fraction = factor == UINT16_MAX ? 65536u : factor;
	/* A zero whole yaw step retains bank carry while steering remains active. */
	return (unsigned)Integrate(Channel(slot, XW_INTEGRATE_BANK), (int64_t)step * fraction, 65536,
							   fraction ? direction : 0);
}

void XwFlightIntegration_ChangeVelocity(unsigned slot, uint16_t rate, int direction) {
	ObjectRecord* object = &g_objectTable[slot];
	uint64_t delta =
		(uint64_t)Integrate(Channel(slot, XW_INTEGRATE_VELOCITY), (int64_t)rate * g_elapsedTicks * 65536,
							XW_SIMULATION_TICKS_PER_SECOND, rate ? direction : 0);
	uint16_t previous = object->speedFractionQ16;
	g_mathDivideFractionQ16 = (uint16_t)delta;
	int64_t speed = object->speed;
	if (direction > 0) {
		object->speedFractionQ16 += g_mathDivideFractionQ16;
		speed += (int64_t)(delta >> 16) + (object->speedFractionQ16 < previous);
	} else {
		object->speedFractionQ16 -= g_mathDivideFractionQ16;
		speed -= (int64_t)(delta >> 16) + (object->speedFractionQ16 > previous);
	}
	if ((direction > 0 && speed >= 3600) || (direction < 0 && speed <= 0)) {
		speed = speed >= 3600 ? 3600 : 0;
		XwFlightIntegration_Clear(slot, XW_INTEGRATE_VELOCITY);
	}
	object->speed = (uint16_t)speed;
}

void XwFlightIntegration_Move(unsigned slot) {
	const ObjectRecord* object = &g_objectTable[slot];
	const int axes[3] = { object->moveX, object->moveY, object->moveZ };
	int* outputs[3] = { &g_moveDeltaX, &g_moveDeltaY, &g_moveDeltaZ };
	int64_t speed = ((int64_t)4660 * object->speed + 128) >> 8;
	const int64_t divisor = (int64_t)XW_SIMULATION_TICKS_PER_SECOND * 32768;
	for (unsigned axis = 0; axis < 3; ++axis) {
		int64_t numerator = speed * g_elapsedTicks * axes[axis] + mobile[slot].position[axis];
		*outputs[axis] = (int)(numerator / divisor);
		mobile[slot].position[axis] = numerator % divisor;
	}
}

void XwFlightIntegration_ClampPosition(unsigned slot) {
	const ObjectRecord* object = &g_objectTable[slot];
	const int position[3] = { object->worldX, object->worldY, object->worldZ };
	for (unsigned axis = 0; axis < 3; ++axis)
		if (position[axis] <= -MOVE_WORLD_COORDINATE_LIMIT || position[axis] >= MOVE_WORLD_COORDINATE_LIMIT)
			mobile[slot].position[axis] = 0;
}

void XwFlightIntegration_Push(unsigned slot, unsigned axis, int* remaining, int cap, int* output) {
	int rate = *remaining < -cap ? -cap : *remaining > cap ? cap : *remaining;
	int step = XwFlightIntegration_Rate(slot, XW_INTEGRATE_PUSH_X + axis, rate, g_elapsedTicks,
										XW_SIMULATION_TICKS_PER_SECOND);
	if ((*remaining > 0 && step > *remaining) || (*remaining < 0 && step < *remaining))
		step = *remaining;
	*remaining = (int32_t)((uint32_t)*remaining - (uint32_t)step);
	*output = (int32_t)((uint32_t)*output + (uint32_t)step);
	if (!*remaining)
		XwFlightIntegration_Clear(slot, XW_INTEGRATE_PUSH_X + axis);
}

int16_t XwFlightIntegration_Home(unsigned slot, unsigned channel, int16_t current, int16_t target,
								 uint16_t rate) {
	int16_t delta = (int16_t)(target - current);
	int step = XwFlightIntegration_Rate(slot, channel, delta < 0 ? -(int)rate : rate, g_elapsedTicks,
										XW_SIMULATION_TICKS_PER_SECOND);
	/* Retain X-Wing's signed-word absolute delta, including its half-turn endpoint. */
	int16_t distance = delta < 0 ? (int16_t)-delta : delta;
	if (distance <= (step < 0 ? -step : step)) {
		XwFlightIntegration_Clear(slot, channel);
		return target;
	}
	return (int16_t)(current + step);
}
