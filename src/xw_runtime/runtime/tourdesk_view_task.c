#include "xw_runtime/runtime/tourdesk_view_task.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/tourdesk.h"
#include "xw_dos94/frontend/tourdesk.h"
#include "xw_runtime/runtime/profile.h"

#include "xw/util/landru_display.h"

#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/paragrp.h>
#include <landru/task.h>
#include <landru/view.h>
#include <stdlib.h>

typedef struct XwTourDeskView {
	ResFile* deskResource;
	ResFile* extraTextResource;
} XwTourDeskView;

static void finish_view_end(void* self) {
	XwTourDeskView* state = self;
	xview_Clear_View_Update_Function();
	xparagrp_Free_Paragraph(g_tourDeskText);
	g_tourDeskText = 0;
	xres_Close_Resource(state->deskResource);
	if (state->extraTextResource != NULL)
		xres_Close_Resource(state->extraTextResource);
}

static LandruTaskStepResult finish_view(void* self) {
	(void)self;
	if (XwProfile_DosFrontend())
		Dos94_tourdesk_CloseMusic();
	else
		tourdesk_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xview_Enable_All_View_Erase();
	xio_Clear_Key_Buttons();
	xview_Clear_View_Update_Function();
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	LandruDisplay_ForwardLegacyNoOp(0);
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable finish_view_vtable = { finish_view, finish_view_end, NULL, NULL };

void XwTourDesk_RunView(ResFile* deskResource, ResFile* extraTextResource) {
	XwTourDeskView* state;
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	state = landru_task_push(&finish_view_vtable);
	if (state == NULL)
		abort();
	state->deskResource = deskResource;
	state->extraTextResource = extraTextResource;
	j_xviewadd_Handle_View();
}
