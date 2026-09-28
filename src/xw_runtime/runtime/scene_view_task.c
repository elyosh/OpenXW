#include "xw_runtime/runtime/scene_view_task.h"
#include "xw/frontend/shellext.h"
#include <landru/task.h>
#include <stdlib.h>

typedef struct SceneViewTask {
	void (*complete)(void);
	void (*release)(void);
	int completed;
} SceneViewTask;

static LandruTaskStepResult step(void* self) {
	SceneViewTask* task = self;
	if (!task->completed) {
		task->completed = 1;
		if (task->complete)
			task->complete();
		return LANDRU_TASK_STEP_CONTINUE;
	}
	return LANDRU_TASK_STEP_DONE;
}

static void release(void* self) {
	SceneViewTask* task = self;
	if (task->release)
		task->release();
}

static const LandruTaskVtable vtable = { step, release, NULL, NULL };

void XwScene_RunView(void (*complete)(void), void (*cleanup)(void)) {
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	SceneViewTask* task = landru_task_push(&vtable);
	if (!task)
		abort();
	task->complete = complete;
	task->release = cleanup;
	j_xviewadd_Handle_View();
}
