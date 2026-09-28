#include "xw_runtime/runtime/dfire_task.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/dfire.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include "xw/util/landru_display.h"

#include <landru/canvas.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/task.h>
#include <landru/view.h>
#include <stdlib.h>

typedef struct XwDFireView {
	ResFile* resourceFile;
	int16_t noSavedBackground;
} XwDFireView;

static void finish_view_end(void* self) {
	XwDFireView* state = self;
	xview_Clear_View_Update_Function();
	if ((uint16_t)xio_Is_System_Slower_Than(DFIRE_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0 &&
		!state->noSavedBackground) {
		xmemhdl_Free_Handle(g_dfireBackgroundHandle);
		g_dfireBackgroundHandle = 0;
	}
	xres_Close_Resource(state->resourceFile);
}

static LandruTaskStepResult finish_view(void* self) {
	XwDFireView* state = self;
	Rect frame;
	DFire_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	if ((uint16_t)xio_Is_System_Slower_Than(DFIRE_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0 &&
		!state->noSavedBackground) {
		xview_Enable_All_View_Erase();
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		xview_Set_View_Frame(0, &frame);
		xview_Set_View_Pos(0, frame.left, frame.top);
	}
	LandruDisplay_SetLowResolutionMode(0);
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable finish_view_vtable = { finish_view, finish_view_end, NULL, NULL };

void XwDFire_RunView(ResFile* resourceFile, int16_t noSavedBackground) {
	XwDFireView* state;
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	state = landru_task_push(&finish_view_vtable);
	if (state == NULL)
		abort();
	state->resourceFile = resourceFile;
	state->noSavedBackground = noSavedBackground;
	j_xviewadd_Handle_View();
}
