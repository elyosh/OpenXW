#include "xw_runtime/runtime/dsboom_task.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/ds_boom.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include "xw/util/landru_display.h"

#include <landru/canvas.h>
#include <landru/task.h>
#include <landru/view.h>
#include <stdlib.h>

typedef struct XwDsBoomView {
	ResFile* resourceFile;
} XwDsBoomView;

static void finish_view_end(void* self) {
	XwDsBoomView* state = self;
	xview_Clear_View_Update_Function();
	xres_Close_Resource(state->resourceFile);
}

static LandruTaskStepResult finish_view(void* self) {
	(void)self;
	Rect frame;
	j_ShellPreferences_GetMusicEnabled();
	DsBoom_CloseSoundEffects();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xview_Clear_View_Update_Function();
	LandruDisplay_SetLowResolutionMode(0);
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable finish_view_vtable = { finish_view, finish_view_end, NULL, NULL };

void XwDsBoom_RunView(ResFile* resourceFile) {
	XwDsBoomView* state;
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	state = landru_task_push(&finish_view_vtable);
	if (state == NULL)
		abort();
	state->resourceFile = resourceFile;
	j_xviewadd_Handle_View();
}
