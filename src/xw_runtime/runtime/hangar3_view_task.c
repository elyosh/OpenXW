#include "xw_runtime/runtime/hangar3_view_task.h"
#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/hangar3.h"
#include "xw/frontend/shellext.h"
#include "xw_runtime/runtime/profile.h"
#include <landru/canvas.h>
#include <landru/memhdl.h>
#include <landru/task.h>
#include <landru/timer.h>
#include <landru/view.h>
#include <stdlib.h>

typedef struct XwHangar3View {
	ResFile* resourceFile;
	int manageSceneAudio;
	int musicCleanupStarted;
} XwHangar3View;

static void finish_view_end(void* self) {
	XwHangar3View* state = self;
	xview_Clear_View_Update_Function();
	xmemhdl_Free_Handle(g_hangar3BackgroundHandle);
	g_hangar3BackgroundHandle = 0;
	xres_Close_Resource(state->resourceFile);
}

static LandruTaskStepResult finish_view(void* self) {
	XwHangar3View* state = self;
	if (!state->musicCleanupStarted) {
		Rect frame;
		xtimer_Set_FrameRatePreset(LANDRU_FRAME_PRESET_2);
		xview_Clear_View_Update_Function();
		xview_Enable_All_View_Erase();
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		xview_Set_View_Frame(0, &frame);
		xview_Set_View_Pos(0, frame.left, frame.top);
		xmemhdl_Free_Handle(g_hangar3BackgroundHandle);
		g_hangar3BackgroundHandle = 0;
		if (state->manageSceneAudio) {
			state->musicCleanupStarted = 1;
			Hangar3_CloseMusic();
			/* CloseMusic may push its existing transition wait above this task. */
			return LANDRU_TASK_STEP_YIELD;
		}
	}
	if (state->manageSceneAudio && !XwProfile_DosFrontend())
		soundext_ResetEnabledSfxCache();
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable finish_view_vtable = { finish_view, finish_view_end, NULL, NULL };

void XwHangar3_RunView(ResFile* resourceFile, int manageSceneAudio) {
	XwHangar3View* state;
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	state = landru_task_push(&finish_view_vtable);
	if (state == NULL)
		abort();
	state->resourceFile = resourceFile;
	state->manageSceneAudio = manageSceneAudio;
	state->musicCleanupStarted = 0;
	j_xviewadd_Handle_View();
}
