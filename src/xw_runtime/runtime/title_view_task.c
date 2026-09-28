#include "xw_runtime/runtime/title_view_task.h"
#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/title.h"
#include "xw/frontend/shellext.h"
#include "xw/util/landru_display.h"
#include <landru/canvas.h>
#include <landru/memhdl.h>
#include <landru/paragrp.h>
#include <landru/task.h>
#include <landru/view.h>
#include <stdlib.h>

typedef struct XwTitleView {
	ResFile* resourceFile;
	int musicCleanupStarted;
} XwTitleView;

static void finish_view_end(void* self) {
	XwTitleView* state = self;
	if (!state->resourceFile)
		return;
	xview_Clear_View_Update_Function();
	xmemhdl_Free_Handle(g_titleTextBitmapHandle);
	xparagrp_Free_Paragraph(g_titleTextHandle);
	xres_Close_Resource(state->resourceFile);

	state->resourceFile = NULL;
}

static LandruTaskStepResult finish_view(void* self) {
	XwTitleView* state = self;
	Rect frame;
	if (!state->musicCleanupStarted) {
		xview_Enable_Global_View_Erase();
		xview_Clear_View_Update_Function();
		state->musicCleanupStarted = 1;
		title_CloseMusic();
		return LANDRU_TASK_STEP_YIELD;
	}
	soundext_ResetEnabledSfxCache();

	finish_view_end(state);
	LandruDisplay_SetLowResolutionMode(0);
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, 0, 0);
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable finish_view_vtable = { finish_view, finish_view_end, NULL, NULL };

void XwTitle_RunView(ResFile* resourceFile) {
	XwTitleView* state;
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	state = landru_task_push(&finish_view_vtable);
	if (state == NULL)
		abort();
	state->resourceFile = resourceFile;
	state->musicCleanupStarted = 0;
	j_xviewadd_Handle_View();
}
