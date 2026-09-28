#include "xw_runtime/runtime/leave1_task.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/leave1.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include "xw/util/landru_display.h"

#include <landru/canvas.h>
#include <landru/task.h>
#include <landru/view.h>
#include <stdlib.h>

typedef struct XwLeave1View {
	ResFile* sceneResource;
	ResFile* yavinResource;
} XwLeave1View;

static void finish_view_end(void* self) {
	XwLeave1View* state = self;
	xview_Clear_View_Update_Function();
	xres_Close_Resource(state->yavinResource);
	xres_Close_Resource(state->sceneResource);
}

static LandruTaskStepResult finish_view(void* self) {
	(void)self;
	Rect frame;
	Leave1_TransitionMusic();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xview_Enable_All_View_Erase();
	LandruDisplay_SetLowResolutionMode(0);
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable finish_view_vtable = { finish_view, finish_view_end, NULL, NULL };

void XwLeave1_RunView(ResFile* sceneResource, ResFile* yavinResource) {
	XwLeave1View* state;
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	state = landru_task_push(&finish_view_vtable);
	if (state == NULL)
		abort();
	state->sceneResource = sceneResource;
	state->yavinResource = yavinResource;
	j_xviewadd_Handle_View();
}
