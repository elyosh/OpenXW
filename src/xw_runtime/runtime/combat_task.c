#include "xw_runtime/runtime/combat_task.h"
#include "xw/audio/soundext.h"
#include "xw/frontend/combat.h"
#include "xw/frontend/shellext.h"
#include "xw/util/landru_display.h"
#include <landru/cursor.h>
#include <landru/io.h>
#include <landru/paragrp.h>
#include <landru/task.h>
#include <landru/view.h>
#include <stdlib.h>

static void finish_view_end(void* self) {
	(void)self;
	int16_t releaseIndex;
	xview_Clear_View_Update_Function();
	xparagrp_Free_Paragraph(g_combatShipListText);
	g_combatShipListText = 0;
	for (releaseIndex = 0; releaseIndex < g_combatAvailableTourCount + g_combatAvailableShipCount;
		 ++releaseIndex)
		xparagrp_Free_Paragraph(g_combatMissionParagraphs[releaseIndex]);
	xres_Close_Resource(g_combatResourceFile);
	xmemhdl_Free_Handle(g_combatScoreHandle);
	g_combatScoreHandle = 0;
}

static LandruTaskStepResult finish_view(void* self) {
	int16_t selectedMission;
	int16_t selectedShip;

	(void)self;
	xview_Clear_View_Update_Function();
	selectedMission = shipext_Get_Combat_Mission();
	selectedShip = shipext_Get_Combat_Ship();
	xparagrp_Get_Paragraph_String(g_combatMissionParagraphs[selectedShip], g_shellMissionName, 0,
								  selectedMission);
	xview_Enable_All_View_Erase();
	combat_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xio_Clear_Key_Buttons();
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	LandruDisplay_ForwardLegacyNoOp(0);
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable finish_view_vtable = { finish_view, finish_view_end, NULL, NULL };

void XwCombat_RunView(void) {
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	if (landru_task_push(&finish_view_vtable) == NULL)
		abort();
	j_xviewadd_Handle_View();
}
