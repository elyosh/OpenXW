#include "xw_runtime/runtime/train_task.h"
#include "xw/audio/soundext.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"
#include "xw/frontend/train.h"
#include "xw/util/landru_display.h"
#include "xw_dos94/frontend/train.h"
#include "xw_runtime/runtime/profile.h"
#include <landru/cursor.h>
#include <landru/io.h>
#include <landru/paragrp.h>
#include <landru/task.h>
#include <landru/view.h>
#include <stdlib.h>

static void finish_view_end(void* self) {
	(void)self;

	xview_Clear_View_Update_Function();
	xparagrp_Free_Paragraph(g_trainingLevelParagraph);
	g_trainingLevelParagraph = 0;
	xres_Close_Resource(g_trainingResourceFile);
}

static LandruTaskStepResult finish_view(void* self) {
	int16_t selectedLevel;
	Rect frame;
	(void)self;
	xview_Clear_View_Update_Function();
	if (XwProfile_DosFrontend())
		Dos94_train_CloseMusic();
	else
		train_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xio_Clear_Key_Buttons();
	selectedLevel = shipext_Get_Train_Level();
	xparagrp_Get_Paragraph_String(g_trainingLevelParagraph, g_shellMissionName, 0, selectedLevel);
	xview_Enable_All_View_Erase();
	xview_Set_View_ZPlane(TRAIN_ROOM_VIEW, TRAIN_FULL_NEAR_Z, TRAIN_FULL_FAR_Z);
	xview_Set_View_ZPlane(TRAIN_SCREEN_VIEW, TRAIN_FULL_NEAR_Z, TRAIN_FULL_FAR_Z);
	xrect_Set_Rect(&frame, 0, 0, 0, 0);
	xview_Set_View_Frame(TRAIN_SCREEN_VIEW, &frame);
	xview_Enable_View_Copy(TRAIN_SCREEN_VIEW);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	LandruDisplay_ForwardLegacyNoOp(0);
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable finish_view_vtable = { finish_view, finish_view_end, NULL, NULL };

void XwTrain_RunView(void) {
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	if (landru_task_push(&finish_view_vtable) == NULL)
		abort();
	j_xviewadd_Handle_View();
}
