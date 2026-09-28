#include "xw_runtime/runtime/blueprint_task.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/blueprnt.h"
#include "xw/frontend/shellext.h"

#include "xw/util/landru_display.h"

#include <landru/cursor.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/paragrp.h>
#include <landru/task.h>
#include <landru/view.h>
#include <stdlib.h>

typedef struct XwBlueprintView {
	ResFile* resource;
} XwBlueprintView;

static void finish_view_end(void* self) {
	XwBlueprintView* state = self;
	xview_Clear_View_Update_Function();
	g_blueprintShipActor = NULL;
	xparagrp_Free_Paragraph(g_blueprintText);
	g_blueprintText = 0;
	xres_Close_Resource(g_blueprintShipResourceFile);
	xres_Close_Resource(state->resource);
}

static LandruTaskStepResult finish_view(void* self) {
	(void)self;
	blueprnt_CloseMusic();
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

void XwBlueprint_RunView(ResFile* resource) {
	XwBlueprintView* state;
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	state = landru_task_push(&finish_view_vtable);
	if (state == NULL)
		abort();
	state->resource = resource;
	j_xviewadd_Handle_View();
}
