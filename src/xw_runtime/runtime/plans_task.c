#include "xw_runtime/runtime/plans_task.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/plans.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include "xw/util/landru_display.h"

#include <landru/canvas.h>
#include <landru/memhdl.h>
#include <landru/task.h>
#include <landru/view.h>
#include <stdlib.h>

typedef struct XwPlansView {
	ResFile* resourceFile;
	int16_t useNormalErase;
} XwPlansView;

static void finish_view_end(void* self) {
	XwPlansView* state = self;
	xview_Clear_View_Update_Function();
	if (state->useNormalErase == 0) {
		xmemhdl_Free_Handle(g_plansBackgroundHandle);
		g_plansBackgroundHandle = 0;
	}
	xres_Close_Resource(state->resourceFile);
}

static LandruTaskStepResult finish_view(void* self) {
	XwPlansView* state = self;
	Rect frame;
	Plans_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	if (state->useNormalErase == 0) {
		xview_Enable_All_View_Erase();
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		xview_Set_View_Frame(0, &frame);
		xview_Set_View_Pos(0, frame.left, frame.top);
	}
	LandruDisplay_SetLowResolutionMode(0);
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable finish_view_vtable = { finish_view, finish_view_end, NULL, NULL };

void XwPlans_RunView(ResFile* resourceFile, int16_t useNormalErase) {
	XwPlansView* state;
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	state = landru_task_push(&finish_view_vtable);
	if (state == NULL)
		abort();
	state->resourceFile = resourceFile;
	state->useNormalErase = useNormalErase;
	j_xviewadd_Handle_View();
}
