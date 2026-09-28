#include "xw_runtime/runtime/mainmenu_task.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/mainmenu.h"
#include "xw/frontend/shellext.h"

#include "xw/util/landru_display.h"

#include <landru/cursor.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/task.h>
#include <landru/view.h>
#include <stdlib.h>
#include <time.h>

typedef struct XwMainMenuView {
	ResFile* resource;
} XwMainMenuView;

static void finish_view_end(void* self) {
	XwMainMenuView* state = self;
	xview_Clear_View_Update_Function();
	xres_Close_Resource(state->resource);
}

static LandruTaskStepResult finish_view(void* self) {
	(void)self;
	xview_Enable_All_View_Erase();
	xview_Clear_View_Update_Function();
	mainmenu_CloseMusic();
	soundext_ResetEnabledSfxCache();
	LandruDisplay_ForwardLegacyNoOp(0);
	xio_Clear_Key_Buttons();
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable finish_view_vtable = { finish_view, finish_view_end, NULL, NULL };

void XwMainMenu_RunView(ResFile* resource) {
	XwMainMenuView* state;
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	state = landru_task_push(&finish_view_vtable);
	if (state == NULL)
		abort();
	state->resource = resource;
	j_xviewadd_Handle_View();
}

void XwMainMenu_GetDate(char* date, size_t capacity) {
	time_t now = time(NULL);
	struct tm* local = localtime(&now);
	if (capacity == 0)
		return;
	date[0] = '\0';
	if (local != NULL)
		strftime(date, capacity, "%m/%d/%y", local);
}
