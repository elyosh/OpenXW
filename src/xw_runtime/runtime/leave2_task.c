#include "xw_runtime/runtime/leave2_task.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/leave2.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include "xw/util/landru_display.h"

#include <landru/canvas.h>
#include <landru/memhdl.h>
#include <landru/task.h>
#include <landru/view.h>
#include <stdlib.h>

typedef struct XwLeave2View {
	ResFile* sceneResource;
	ResFile* yavinResource;
} XwLeave2View;

static void finish_view_end(void* self) {
	XwLeave2View* state = self;
	xview_Clear_View_Update_Function();
	xmemhdl_Free_Handle(g_leave2BackgroundHandle);
	g_leave2BackgroundHandle = 0;
	xres_Close_Resource(state->yavinResource);
	xres_Close_Resource(state->sceneResource);
}

static LandruTaskStepResult finish_view(void* self) {
	(void)self;
	Rect frame;
	j_ShellPreferences_GetMusicEnabled();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	LandruDisplay_SetLowResolutionMode(0);
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable finish_view_vtable = { finish_view, finish_view_end, NULL, NULL };

void XwLeave2_RunView(ResFile* sceneResource, ResFile* yavinResource) {
	XwLeave2View* state;
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	state = landru_task_push(&finish_view_vtable);
	if (state == NULL)
		abort();
	state->sceneResource = sceneResource;
	state->yavinResource = yavinResource;
	j_xviewadd_Handle_View();
}
