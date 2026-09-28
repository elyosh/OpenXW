#include "xw_runtime/runtime/intro1_task.h"
#include "xw_dos94/frontend/intro.h"
#include "xw_runtime/runtime/profile.h"

#include "xw/frontend/scenes/intro1.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/util/landru_display.h"

#include <landru/task.h>
#include <landru/view.h>
#include <stdlib.h>

typedef struct XwIntro1View {
	ResFile* resourceFile;
} XwIntro1View;

static void finish_view_end(void* self) {
	XwIntro1View* state = self;
	xview_Clear_View_Update_Function();
	if (XwProfile_DosFrontend())
		Dos94_Intro1_ReleaseBackground();
	xres_Close_Resource(state->resourceFile);
}

static LandruTaskStepResult finish_view(void* self) {
	(void)self;
	j_ShellPreferences_GetMusicEnabled();
	if (!XwProfile_DosFrontend())
		Intro1_CloseSoundEffects();
	xview_Clear_View_Update_Function();
	LandruDisplay_ForwardLegacyNoOp(0);
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable finish_view_vtable = { finish_view, finish_view_end, NULL, NULL };

void XwIntro1_RunView(ResFile* resourceFile) {
	XwIntro1View* state;
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	state = landru_task_push(&finish_view_vtable);
	if (state == NULL)
		abort();
	state->resourceFile = resourceFile;
	j_xviewadd_Handle_View();
}
