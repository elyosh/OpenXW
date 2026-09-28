#include "xw_runtime/runtime/register_view_task.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/register.h"
#include "xw/frontend/shellext.h"

#include "xw/util/landru_display.h"

#include <landru/cursor.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/paragrp.h>
#include <landru/task.h>
#include <landru/view.h>
#include <stdlib.h>

typedef struct XwRegisterView {
	ResFile* resource;
} XwRegisterView;

static void finish_view_end(void* self) {
	XwRegisterView* state = self;
	xview_Clear_View_Update_Function();
	xparagrp_Free_Paragraph(g_RegisterTourParagraph);
	g_RegisterTourParagraph = 0;
	xfiledir_Free_Directory(&g_RegisterDirectory);
	if (g_RegisterFastPilotHandle != LANDRU_NULL_HANDLE)
		xmemhdl_Free_Handle(g_RegisterFastPilotHandle);
	g_RegisterFastPilotHandle = 0;
	xres_Close_Resource(state->resource);
}

static LandruTaskStepResult finish_view(void* self) {
	(void)self;
	xio_Clear_Key_Buttons();
	xview_Enable_All_View_Erase();
	register_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	LandruDisplay_ForwardLegacyNoOp(0);
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable finish_view_vtable = { finish_view, finish_view_end, NULL, NULL };

void XwRegister_RunView(ResFile* resource) {
	XwRegisterView* state;
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	state = landru_task_push(&finish_view_vtable);
	if (state == NULL)
		abort();
	state->resource = resource;
	j_xviewadd_Handle_View();
}
