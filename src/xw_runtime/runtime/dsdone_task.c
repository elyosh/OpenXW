#include "xw_runtime/runtime/dsdone_task.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/ds_done.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include "xw/util/landru_display.h"

#include <landru/canvas.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/task.h>
#include <landru/view.h>
#include <stdlib.h>

typedef struct XwDsDoneView {
	ResFile* resourceFile;
} XwDsDoneView;

static void finish_view_end(void* self) {
	XwDsDoneView* state = self;
	xview_Clear_View_Update_Function();
	if ((uint16_t)xio_Is_System_Slower_Than(DSDONE_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
		xmemhdl_Free_Handle(g_dsdoneBackgroundHandle);
		g_dsdoneBackgroundHandle = 0;
	}
	xres_Close_Resource(state->resourceFile);
}

static LandruTaskStepResult finish_view(void* self) {
	(void)self;
	Rect frame;
	j_ShellPreferences_GetMusicEnabled();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	if ((uint16_t)xio_Is_System_Slower_Than(DSDONE_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
		xview_Enable_All_View_Erase();
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		xview_Set_View_Frame(0, &frame);
		xview_Set_View_Pos(0, frame.left, frame.top);
	}
	LandruDisplay_SetLowResolutionMode(0);
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable finish_view_vtable = { finish_view, finish_view_end, NULL, NULL };

void XwDsDone_RunView(ResFile* resourceFile) {
	XwDsDoneView* state;
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	state = landru_task_push(&finish_view_vtable);
	if (state == NULL)
		abort();
	state->resourceFile = resourceFile;
	j_xviewadd_Handle_View();
}
