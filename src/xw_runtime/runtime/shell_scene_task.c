#include "xw_runtime/runtime/shell_scene_task.h"

#include "xw/frontend/shellext.h"

#include <landru/canvas.h>
#include <landru/task.h>
#include <stdlib.h>

static LandruTaskStepResult save_scene(void* self) {
	(void)self;
	xcanvas_Copy_Screen_Portion_To_Diff(0, 0, LANDRU_VGA_WIDTH, LANDRU_VGA_HEIGHT);
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable save_scene_vtable = { save_scene, NULL, NULL, NULL };

void XwShell_FadeAndSaveScene(void) {
	/* Reserve one continuation and the shared fade child before changing state. */
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	if (landru_task_push(&save_scene_vtable) == NULL)
		abort();
	shellext_Sudden_Scene_Fade();
}
