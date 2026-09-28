#include "xw_runtime/runtime/outpost_task.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/outpost.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include "xw/util/landru_display.h"

#include <landru/canvas.h>
#include <landru/memhdl.h>
#include <landru/task.h>
#include <landru/view.h>
#include <stdlib.h>

typedef struct XwOutpostView {
	ResFile* resourceFile;
	int16_t useNormalErase;
} XwOutpostView;

static void finish_view_end(void* self) {
	XwOutpostView* state = self;
	xview_Clear_View_Update_Function();
	if (state->useNormalErase == 0) {
		xmemhdl_Free_Handle(g_outpostBackgroundHandle);
		g_outpostBackgroundHandle = 0;
	}
	xres_Close_Resource(state->resourceFile);
}

static LandruTaskStepResult finish_view(void* self) {
	XwOutpostView* state = self;
	Rect frame;
	Outpost_CloseMusic();
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

void XwOutpost_RunView(ResFile* resourceFile, int16_t useNormalErase) {
	XwOutpostView* state;
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	state = landru_task_push(&finish_view_vtable);
	if (state == NULL)
		abort();
	state->resourceFile = resourceFile;
	state->useNormalErase = useNormalErase;
	j_xviewadd_Handle_View();
}
