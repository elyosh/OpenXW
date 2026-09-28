#include "xw_runtime/runtime/inflight_view_task.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/inflight_ui.h"
#include "xw/frontend/player.h"
#include "xw/frontend/shellext.h"
#include "xw/util/shared.h"

#include "xw/util/landru_display.h"

#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/task.h>
#include <landru/view.h>
#include <stdlib.h>

enum { INFLIGHT_TASK_OPEN_MUSIC, INFLIGHT_TASK_START_VIEW, INFLIGHT_TASK_CLEANUP, INFLIGHT_TASK_FINISHED };

typedef struct XwInflightView {
	ResFile* resource;
	int phase;
} XwInflightView;

static void finish_view_end(void* self) {
	XwInflightView* state = self;
	if (!state->resource)
		return;
	xview_Clear_View_Update_Function();
	xres_Close_Resource(state->resource);
	player_Free_Display_Map();
	xmemhdl_Free_Handle(g_inflightMapStateHandle);

	state->resource = NULL;
}

static LandruTaskStepResult finish_view(void* self) {
	XwInflightView* state = self;
	if (state->phase == INFLIGHT_TASK_OPEN_MUSIC) {
		state->phase = INFLIGHT_TASK_START_VIEW;
		InflightUI_OpenMusic(state->resource, g_inflightUIMusicContext);
		return LANDRU_TASK_STEP_YIELD;
	}
	if (state->phase == INFLIGHT_TASK_START_VIEW) {
		state->phase = INFLIGHT_TASK_CLEANUP;
		soundext_LoadCommonUiSounds();
		j_xviewadd_Handle_View();
		return LANDRU_TASK_STEP_YIELD;
	}
	if (state->phase == INFLIGHT_TASK_FINISHED)
		return LANDRU_TASK_STEP_DONE;
	InflightUI_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xio_Clear_Key_Buttons();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	nullsub_SharedNoOp();

	finish_view_end(state);
	state->phase = INFLIGHT_TASK_FINISHED;
	if (xerror_Get_Landru_Exit() != 0) {
		shellext_Sudden_Scene_Fade();
		return LANDRU_TASK_STEP_YIELD;
	}
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable finish_view_vtable = { finish_view, finish_view_end, NULL, NULL };

void XwInflight_RunView(ResFile* resource) {
	XwInflightView* state;
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	state = landru_task_push(&finish_view_vtable);
	if (state == NULL)
		abort();
	state->resource = resource;
	state->phase = INFLIGHT_TASK_OPEN_MUSIC;
}
