#include "xw_runtime/runtime/replay_edit_task.h"
#include "xw/flight/feinput.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/hud/festring.h"
#include "xw/flight/replay/replay.h"
#include "xw/landru_config.h"
#include "xw_runtime/snapshot/render_hud.h"
#include <landru/task.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct ReplayEditState {
	int16_t x, y;
	uint8_t maxLength, backgroundColor, length, writeIndex;
	char* text;
	bool finished;
} ReplayEditState;

static void edit_draw(const ReplayEditState* state, bool caret) {
	int pane = XwHud_Push(XW_SNAP_PANE_REPLAY_STATUS);
	festring_setcursor(state->x, state->y);
	festring_outstring(state->text);
	if (caret) {
		festring_setbackcolor(REPLAY_EDIT_CARET_COLOR);
		g_flightDrawCharFn(' ');
		festring_setbackcolor(state->backgroundColor);
	}
	g_flightDrawCharFn('\n');
	XwHud_Pop(pane);
}

static LandruTaskStepResult edit_step(void* self) {
	ReplayEditState* state = self;

	feinput_getinput();
	if ((int16_t)g_actionKey == REPLAY_EDIT_DELETE_ACTION) {
		g_actionKey = '\b';
	}
	if ((int16_t)g_actionKey == '\b' && state->length > 0)
		state->writeIndex = --state->length;
	if ((int16_t)g_actionKey < '0') {
		if ((int16_t)g_actionKey != '\r' && (int16_t)g_actionKey != '-' && (int16_t)g_actionKey != 0) {
			g_actionKey = REPLAY_EDIT_REJECTED_KEY;
		}
	} else if (((int16_t)g_actionKey < 'A' && (int16_t)g_actionKey >= '9' + 1) ||
			   ((int16_t)g_actionKey < 'a' && (int16_t)g_actionKey >= 'Z' + 1) ||
			   (int16_t)g_actionKey >= 'z' + 1) {
		g_actionKey = REPLAY_EDIT_REJECTED_KEY;
	}
	if ((int16_t)g_actionKey != REPLAY_EDIT_REJECTED_KEY && (int16_t)g_actionKey != '\r' &&
		(int16_t)g_actionKey != 0 && state->length < state->maxLength) {
		uint8_t character = (uint8_t)g_actionKey;
		state->text[state->writeIndex] = (char)character;
		if (character >= 'a' && character <= 'z')
			state->text[state->writeIndex] = (char)(character - ('a' - 'A'));
		state->writeIndex = ++state->length;
	}
	state->text[state->writeIndex] = '\0';
	if (g_actionKey != 0)
		edit_draw(state, true);
	FlightDisplay_UnlockSurface();
	FlightDisplay_PresentBackBuffer();
	FlightDisplay_Flip();
	FlightDisplay_LockSurface();

	if (g_actionKey != '\r')
		return LANDRU_TASK_STEP_FRAME_COMPLETE;

	edit_draw(state, false);
	festring_setautofill(0);
	FlightDisplay_UnlockSurface();
	FlightDisplay_PresentBackBuffer();
	FlightDisplay_Flip();
	FlightDisplay_LockSurface();
	state->finished = true;
	return LANDRU_TASK_STEP_DONE;
}

static void edit_end(void* self) {
	ReplayEditState* state = self;
	if (!state->finished)
		festring_setautofill(0);
}

static const LandruTaskVtable edit_vtable = { edit_step, edit_end, NULL, NULL };

void XwReplayEdit_Push(int16_t x, int16_t y, uint8_t maxLength, char* text, uint8_t backgroundColor) {
	ReplayEditState* state = landru_task_push(&edit_vtable);
	if (state == NULL)
		abort();
	state->x = x;
	state->y = y;
	state->maxLength = maxLength;
	state->backgroundColor = backgroundColor;
	state->text = text;
	state->finished = false;
	festring_setautofill(1);
	for (state->length = 0; state->text[state->length] != '\n' && state->text[state->length] != '\0' &&
							state->length < state->maxLength;
		 ++state->length) {
	}
	state->writeIndex = state->length;
	edit_draw(state, true);
}
