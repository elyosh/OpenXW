#include "xw_runtime/runtime/logo640_task.h"
#include "xw_runtime/runtime/profile.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/logo640.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include <landru/canvas.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/task.h>
#include <landru/view.h>
#include <stdlib.h>

typedef struct XwLogo640View {
	ResFile* resourceFile;
} XwLogo640View;

static void finish_view_end(void* self) {
	XwLogo640View* state = self;
	xview_Clear_View_Update_Function();
	if ((uint16_t)xio_Is_System_Slower_Than(LOGO640_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
		xmemhdl_Free_Handle(g_logo640BackgroundHandle);
		g_logo640BackgroundHandle = 0;
	}
	xres_Close_Resource(state->resourceFile);
}

static LandruTaskStepResult finish_view(void* self) {
	(void)self;
	Rect frame;
	j_ShellPreferences_GetMusicEnabled();
	if (!XwProfile_DosFrontend())
		soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	if ((uint16_t)xio_Is_System_Slower_Than(LOGO640_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
		xview_Enable_All_View_Erase();
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		xview_Set_View_Frame(0, &frame);
		xview_Set_View_Pos(0, frame.left, frame.top);
	}
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable finish_view_vtable = { finish_view, finish_view_end, NULL, NULL };

void XwLogo640_RunView(ResFile* resourceFile) {
	XwLogo640View* state;
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	state = landru_task_push(&finish_view_vtable);
	if (state == NULL)
		abort();
	state->resourceFile = resourceFile;
	j_xviewadd_Handle_View();
}
