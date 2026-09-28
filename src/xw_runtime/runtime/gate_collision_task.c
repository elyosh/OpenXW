#include "xw_runtime/runtime/gate_collision_task.h"
#include "xw/flight/flight.h"
#include "xw_runtime/runtime/flight_dispatch.h"
#include "xw_runtime/runtime/flight_types.h"
#include "xw_runtime/runtime/profile.h"
#include "xw_runtime/timing/host_clock.h"

#include "xw/audio/fsfx.h"
#include "xw/flight/gate.h"
#include "xw/flight/hud/msg.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/xw.h"

#include <landru/task.h>
#include <landru/timer.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct XwGateBonusTask {
	bool finished;
	uint64_t nextWakeUs;
} XwGateBonusTask;

static XwGateCollisionResume saved_collision;
static bool bonus_pending;
static bool collision_ready;
static XwCollisionLoopState collision_loop;

static XwGameVersion active_version(void) { return XwProfile_ActiveFlight()->version; }

static void bonus_award_second(void) {
	if (g_missionCountdownClock.seconds != 0)
		--g_missionCountdownClock.seconds;
	else {
		g_missionCountdownClock.seconds = XW_SECONDS_PER_MINUTE - 1;
		--g_missionCountdownClock.minutes;
	}
	g_missionRuntimeState.provingGroundsTimeBonus += GATE_BONUS_POINTS_PER_SECOND;
	g_missionRuntimeState.provingGroundsScore += GATE_BONUS_POINTS_PER_SECOND;
	if (g_missionRuntimeState.provingGroundsScore % GATE_BONUS_SOUND_INTERVAL == 0)
		fsfx_triggersfx(GATE_BONUS_SOUND, FSFX_UNPOSITIONED_OBJECT);
	XwFlightMode_UpdateCourseBonus();
}

static void bonus_complete(void) {
	g_msgArgTable[0] = g_missionRuntimeState.provingGroundsTimeBonus;
	msg_messageprintf(XW_MSG_BONUS_POINTS_AWARDED);
	gate_LoadNextCourse();
}

static void bonus_schedule_wait(XwGateBonusTask* state) {
	if (g_flightAccumulatedTicks < GATE_BONUS_WAIT_TICKS)
		g_flightAccumulatedTicks += xtimer_Time_Elapsed();
	uint32_t remaining = g_flightAccumulatedTicks < GATE_BONUS_WAIT_TICKS
							 ? GATE_BONUS_WAIT_TICKS - g_flightAccumulatedTicks
							 : 0;
	state->nextWakeUs = XwTime_GetElapsedUs() + xtimer_Elapsed_Delay_Us(remaining);
}

static LandruTaskStepResult bonus_step(void* self) {
	XwGateBonusTask* state = self;
	if (saved_collision.version != active_version())
		abort();
	if (g_flightAccumulatedTicks < GATE_BONUS_WAIT_TICKS)
		g_flightAccumulatedTicks += xtimer_Time_Elapsed();
	if (g_flightAccumulatedTicks < GATE_BONUS_WAIT_TICKS) {
		state->nextWakeUs =
			XwTime_GetElapsedUs() + xtimer_Elapsed_Delay_Us(GATE_BONUS_WAIT_TICKS - g_flightAccumulatedTicks);
		return LANDRU_TASK_STEP_YIELD;
	}
	g_flightAccumulatedTicks = 0;
	if (g_missionCountdownClock.minutes == 0 && g_missionCountdownClock.seconds == 0) {
		bonus_complete();
		state->finished = !g_quitRequested;
		return LANDRU_TASK_STEP_DONE;
	}
	bonus_award_second();
	if (g_quitRequested)
		return LANDRU_TASK_STEP_DONE;
	bonus_schedule_wait(state);
	return LANDRU_TASK_STEP_FRAME_COMPLETE;
}

static void bonus_end(void* self) {
	XwGateBonusTask* state = self;
	bonus_pending = false;
	collision_ready = state->finished;
	if (!state->finished)
		collision_loop.phase = XW_COLLISION_LOOP_IDLE;
}

static uint64_t bonus_next_wake(const void* self) {
	const XwGateBonusTask* state = self;
	uint64_t now = XwTime_GetElapsedUs();
	return state->nextWakeUs > now ? state->nextWakeUs - now : 0;
}

static const LandruTaskVtable bonus_vtable = { bonus_step, bonus_end, NULL, bonus_next_wake };

int XwGateCollision_Resume(uint16_t objectIndex, XwGateCollisionResume* resume) {
	if ((bonus_pending || collision_ready) &&
		(objectIndex != saved_collision.objectIndex || saved_collision.version != active_version()))
		abort();
	if (bonus_pending)
		return XW_GATE_COLLISION_PENDING;
	if (!collision_ready)
		return 0;
	*resume = saved_collision;
	collision_ready = false;
	return 1;
}

int XwGateCollision_BeginBonus(const XwGateCollisionResume* resume) {
	if (bonus_pending || collision_ready)
		abort();
	msg_messageprintf(XW_MSG_PROVING_GROUNDS_LEVEL_COMPLETED);
	g_missionRuntimeState.provingGroundsTimeBonus = 0;
	XwFlightMode_UpdateCourseBonus();
	if (g_quitRequested)
		return 0;
	if (g_missionCountdownClock.minutes == 0 && g_missionCountdownClock.seconds == 0) {
		bonus_complete();
		return 0;
	}
	XwGateBonusTask* state = landru_task_push(&bonus_vtable);
	if (state == NULL)
		abort();
	state->finished = false;
	saved_collision = *resume;
	saved_collision.version = active_version();
	bonus_pending = true;
	bonus_award_second();
	bonus_schedule_wait(state);
	return 1;
}

const XwCollisionLoopState* XwCollisionLoop_GetState(void) { return &collision_loop; }

void XwCollisionLoop_Suspend(const XwCollisionLoopState* state) { collision_loop = *state; }

void XwCollisionLoop_Complete(void) { collision_loop.phase = XW_COLLISION_LOOP_IDLE; }

int XwCollisionLoop_IsPending(void) { return collision_loop.phase != XW_COLLISION_LOOP_IDLE; }

void XwCollisionLoop_Cancel(void) {
	if (bonus_pending)
		abort();
	collision_ready = false;
	collision_loop.phase = XW_COLLISION_LOOP_IDLE;
}
