#include "xw_runtime/runtime/debrief_task.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/debrief.h"
#include "xw/frontend/shellext.h"
#include "xw_dos94/frontend/debrief.h"
#include "xw_runtime/runtime/profile.h"

#include "xw/util/landru_display.h"

#include <landru/cursor.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/task.h>
#include <landru/view.h>
#include <stdlib.h>

typedef struct XwDebriefView {
	ResFile* resource;
} XwDebriefView;

static void finish_view_end(void* self) {
	XwDebriefView* state = self;
	xview_Clear_View_Update_Function();
	xmemhdl_Free_Handle(g_debriefBackgroundHandle);
	g_debriefBackgroundHandle = 0;
	xres_Close_Resource(state->resource);
}

static LandruTaskStepResult finish_view(void* self) {
	(void)self;
	if (XwProfile_DosFrontend())
		Dos94_debrief_CloseMusic();
	else
		debrief_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xio_Clear_Key_Buttons();
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	LandruDisplay_ForwardLegacyNoOp(0);
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable finish_view_vtable = { finish_view, finish_view_end, NULL, NULL };

void XwDebrief_RunView(ResFile* resource) {
	XwDebriefView* state;
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	state = landru_task_push(&finish_view_vtable);
	if (state == NULL)
		abort();
	state->resource = resource;
	j_xviewadd_Handle_View();
}
