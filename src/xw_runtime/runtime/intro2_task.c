#include "xw_runtime/runtime/intro2_task.h"
#include "xw_runtime/runtime/profile.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/intro2.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include "xw/util/landru_display.h"

#include <landru/canvas.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/task.h>
#include <landru/view.h>
#include <stdlib.h>

typedef struct XwIntro2View {
	ResFile* resourceFile;
} XwIntro2View;

static void finish_view_end(void* self) {
	XwIntro2View* state = self;
	xview_Clear_View_Update_Function();
	if ((uint16_t)xio_Is_System_Slower_Than(INTRO2_BACKGROUND_CACHE_SPEED) != 0) {
		xmemhdl_Free_Handle(g_intro2BackgroundBuffers[0]);
		xmemhdl_Free_Handle(g_intro2BackgroundBuffers[1]);
	}
	xres_Close_Resource(state->resourceFile);
}

static LandruTaskStepResult finish_view(void* self) {
	(void)self;
	Rect frame;
	if (!XwProfile_DosFrontend()) {
		Intro2_CloseMusic();
		soundext_ResetEnabledSfxCache();
	}
	xview_Clear_View_Update_Function();
	if ((uint16_t)xio_Is_System_Slower_Than(INTRO2_BACKGROUND_CACHE_SPEED) != 0) {
		xview_Enable_All_View_Erase();
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		xview_Set_View_Frame(0, &frame);
		xview_Set_View_Pos(0, frame.left, frame.top);
	}
	LandruDisplay_ForwardLegacyNoOp(0);
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable finish_view_vtable = { finish_view, finish_view_end, NULL, NULL };

void XwIntro2_RunView(ResFile* resourceFile) {
	XwIntro2View* state;
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	state = landru_task_push(&finish_view_vtable);
	if (state == NULL)
		abort();
	state->resourceFile = resourceFile;
	j_xviewadd_Handle_View();
}
