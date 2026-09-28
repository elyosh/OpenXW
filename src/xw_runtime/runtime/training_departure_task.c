#include "xw_runtime/runtime/training_departure_task.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/training_transit.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include <landru/task.h>
#include <landru/timer.h>
#include <landru/view.h>
#include <stdlib.h>

typedef struct XwTrainingDepartureView {
	ResFile* resourceFile;
	int16_t isHyperspaceScene;
} XwTrainingDepartureView;

static void finish_view_end(void* self) {
	XwTrainingDepartureView* state = self;
	xview_Clear_View_Update_Function();
	xres_Close_Resource(state->resourceFile);
}

static LandruTaskStepResult finish_view(void* self) {
	XwTrainingDepartureView* state = self;
	if (state->isHyperspaceScene)
		xtimer_Set_FrameRatePreset(LANDRU_FRAME_PRESET_3);
	TrainingTransit_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	if (!state->isHyperspaceScene)
		xview_Enable_All_View_Erase();
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable finish_view_vtable = { finish_view, finish_view_end, NULL, NULL };

void XwTrainingDeparture_RunView(ResFile* resourceFile, int16_t isHyperspaceScene) {
	XwTrainingDepartureView* state;
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	state = landru_task_push(&finish_view_vtable);
	if (state == NULL)
		abort();
	state->resourceFile = resourceFile;
	state->isHyperspaceScene = isHyperspaceScene;
	j_xviewadd_Handle_View();
}
