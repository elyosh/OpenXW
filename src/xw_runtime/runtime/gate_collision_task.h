#ifndef XW_RUNTIME_RUNTIME_GATE_COLLISION_TASK_H
#define XW_RUNTIME_RUNTIME_GATE_COLLISION_TASK_H

#include "xw_runtime/storage/storage.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { XW_GATE_COLLISION_PENDING = -1 };

typedef struct XwGateCollisionResume {
	XwGameVersion version;
	uint16_t objectIndex;
	uint16_t gateIndex;
	uint16_t objectType;
	uint16_t checkpointNumber;
	int16_t originalStartGateUp;
	int gateWorldX, gateWorldY, gateWorldZ;
} XwGateCollisionResume;

typedef enum XwCollisionLoopPhase {
	XW_COLLISION_LOOP_IDLE,
	XW_COLLISION_LOOP_PLAYER_GATE,
	XW_COLLISION_LOOP_PROJECTILE_GATE
} XwCollisionLoopPhase;

typedef struct XwCollisionLoopState {
	XwCollisionLoopPhase phase;
	uint16_t objectIndex;
	int16_t objectGenus;
	int16_t projectileIff;
	int16_t projectileSourceIndex;
} XwCollisionLoopState;

/* The frame owner resumes collide_collisions before proceeding with the frame
 * while this state is pending, and dispatches only Landru tasks during waits. */
const XwCollisionLoopState* XwCollisionLoop_GetState(void);
void XwCollisionLoop_Suspend(const XwCollisionLoopState* state);
void XwCollisionLoop_Complete(void);
int XwCollisionLoop_IsPending(void);
/* After clearing the task stack when abandoning a flight. */
void XwCollisionLoop_Cancel(void);

/* A pending collision suspends its owning collision/frame loop. Dispatch only
 * the Landru task until completion, then call gate_ProcessCourseCollision with
 * the same object index to resume after the scored checkpoint. Do not update
 * simulation or shared collision/orientation state while suspended. Clearing
 * the task stack cancels the continuation together with its owning flight. */
int XwGateCollision_Resume(uint16_t objectIndex, XwGateCollisionResume* resume);
int XwGateCollision_BeginBonus(const XwGateCollisionResume* resume);

#ifdef __cplusplus
}
#endif

#endif
