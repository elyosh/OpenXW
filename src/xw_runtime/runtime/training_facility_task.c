#include "xw_runtime/runtime/training_facility_task.h"
#include "xw/frontend/shellext.h"
#include <landru/task.h>
#include <landru/timer.h>
#include <landru/view.h>
#include <stdlib.h>

typedef struct XwTrainingFacilityView {
	ResFile* resourceFile;
} XwTrainingFacilityView;

static void finish_view_end(void* self) {
	XwTrainingFacilityView* state = self;
	xview_Clear_View_Update_Function();
	xres_Close_Resource(state->resourceFile);
}

static LandruTaskStepResult finish_view(void* self) {
	(void)self;
	xtimer_Set_FrameRatePreset(LANDRU_FRAME_PRESET_2);
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable finish_view_vtable = { finish_view, finish_view_end, NULL, NULL };

void XwTrainingFacility_RunView(ResFile* resourceFile) {
	XwTrainingFacilityView* state;
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	state = landru_task_push(&finish_view_vtable);
	if (state == NULL)
		abort();
	state->resourceFile = resourceFile;
	j_xviewadd_Handle_View();
}
