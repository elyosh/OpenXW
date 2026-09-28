#include "xw_runtime/runtime/brief_view_task.h"
#include "xw/audio/frontend_audio.h"
#include "xw/audio/soundext.h"
#include "xw/frontend/brief.h"
#include "xw/frontend/shellext.h"
#include "xw/util/landru_display.h"
#include "xw/util/shared.h"
#include "xw_dos94/frontend/brief.h"
#include "xw_runtime/runtime/profile.h"
#include <landru/cursor.h>
#include <landru/io.h>
#include <landru/task.h>
#include <landru/view.h>
#include <landru/viewadd.h>
#include <stdlib.h>

enum { BRIEF_TASK_OPEN_MUSIC, BRIEF_TASK_START_VIEW, BRIEF_TASK_CLEANUP };

typedef struct XwBriefView {
	ResFile* resource;
	int phase;
} XwBriefView;

static void finish_view_end(void* self) {
	XwBriefView* state = self;
	if (!state->resource)
		return;
	xview_Clear_View_Update_Function();
	xres_Close_Resource(state->resource);
	brief_Free_Display_Map();
	xmemhdl_Free_Handle(g_briefingRuntimeHandle);
	xmemhdl_Free_Handle(g_briefingOfficerRegionBuffer);

	state->resource = NULL;
}

static LandruTaskStepResult finish_view(void* self) {
	XwBriefView* state = self;
	if (state->phase == BRIEF_TASK_OPEN_MUSIC) {
		state->phase = BRIEF_TASK_START_VIEW;
		brief_OpenMusic(state->resource, g_briefingFilm);
		return LANDRU_TASK_STEP_YIELD;
	}
	if (state->phase == BRIEF_TASK_START_VIEW) {
		state->phase = BRIEF_TASK_CLEANUP;
		brief_LoadUiSounds();
		FrontendAudio_PlayFile("XwingCD\\music\\regbrief.wav", 1);
		j_xviewadd_Handle_View();
		return LANDRU_TASK_STEP_YIELD;
	}
	if (XwProfile_DosFrontend())
		Dos94_brief_CloseMusic();
	else
		brief_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xio_Clear_Key_Buttons();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	LandruDisplay_ForwardLegacyNoOp(0);
	nullsub_SharedNoOp();

	finish_view_end(state);
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable finish_view_vtable = { finish_view, finish_view_end, NULL, NULL };

void XwBrief_RunView(ResFile* resource) {
	XwBriefView* state;
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	state = landru_task_push(&finish_view_vtable);
	if (state == NULL)
		abort();
	state->resource = resource;
	state->phase = BRIEF_TASK_OPEN_MUSIC;
}
