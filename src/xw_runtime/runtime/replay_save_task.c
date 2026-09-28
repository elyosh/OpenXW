#include "xw_runtime/runtime/replay_save_task.h"
#include "xw/assets/model_mesh.h"
#include "xw/flight/fediskio.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/flight_input.h"
#include "xw/flight/hud/festring.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/replay/replay.h"
#include "xw/flight/replay/replayio.h"
#include "xw/landru_config.h"
#include "xw_runtime/runtime/replay_edit_task.h"
#include "xw_runtime/storage/replay_format.h"
#include <landru/task.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef enum ReplaySavePhase { REPLAY_SAVE_EDIT, REPLAY_SAVE_CONFIRM, REPLAY_SAVE_WRITE } ReplaySavePhase;

typedef struct ReplaySaveState {
	ReplaySavePhase phase;
	char clipName[REPLAY_SAVE_TEXT_CAPACITY];
	char clipPath[REPLAY_SAVE_TEXT_CAPACITY];
	bool finished;
} ReplaySaveState;

static bool input_save_pending;
static bool save_pending;
static bool result_ready;
static XwFlightMessageId save_result;

static LandruTaskStepResult save_step(void* self) {
	ReplaySaveState* state = self;
	if (state->phase == REPLAY_SAVE_EDIT) {
		festring_setfontsize(FLIGHT_FONT_MICRO);
		if (state->clipName[0] == 0) {
			save_result = XW_MSG_REPLAY_CLIP_NOT_SAVED;
			state->finished = true;
			return LANDRU_TASK_STEP_DONE;
		}
		strcpy(state->clipPath, state->clipName);
		strcat(state->clipPath, ".clp");
		g_stream = XwStorage_Open(state->clipPath, "rb");
		if (g_stream != NULL) {
			XwFile_Close(g_stream);
			g_stream = NULL;
			replay_replaymessage(XW_MSG_FILE_EXISTS_REPLACE_PROMPT);
			FlightDisplay_UnlockSurface();
			FlightDisplay_PresentBackBuffer();
			FlightDisplay_Flip();
			FlightDisplay_LockSurface();
			state->phase = REPLAY_SAVE_CONFIRM;
		} else {
			state->phase = REPLAY_SAVE_WRITE;
		}
	}
	if (state->phase == REPLAY_SAVE_CONFIRM) {
		uint8_t key;
		if (!FlightInput_HasKeyReady())
			return LANDRU_TASK_STEP_FRAME_COMPLETE;
		key = FlightInput_GetNextKey();
		if (key != 'y' && key != 'Y') {
			save_result = XW_MSG_REPLAY_CLIP_NOT_SAVED;
			state->finished = true;
			return LANDRU_TASK_STEP_DONE;
		}
		state->phase = REPLAY_SAVE_WRITE;
	}
	save_result = XwReplayFormat_SaveFilm(state->clipPath, state->clipName);
	state->finished = true;
	return LANDRU_TASK_STEP_DONE;
}

static void save_end(void* self) {
	ReplaySaveState* state = self;
	save_pending = false;
	result_ready = state->finished;
	if (!state->finished)
		festring_setfontsize(FLIGHT_FONT_MICRO);
}

static const LandruTaskVtable save_vtable = { save_step, save_end, NULL, NULL };

XwFlightMessageId XwReplaySave_Run(void) {
	ReplaySaveState* state;
	if (result_ready) {
		result_ready = false;
		return save_result;
	}
	if (save_pending)
		return 0;
	state = landru_task_push(&save_vtable);
	if (state == NULL)
		abort();
	save_pending = true;
	state->phase = REPLAY_SAVE_EDIT;
	state->finished = false;
	state->clipName[0] = 0;
	replay_replaymessage(XW_MSG_ENTER_FILENAME_PROMPT);
	festring_setfontsize(FLIGHT_FONT_TINY);
	if (g_flightResolutionMode == REPLAY_SAVE_LOW_RESOLUTION_MODE)
		XwReplayEdit_Push(REPLAY_SAVE_LOW_LEFT, REPLAY_SAVE_LOW_TOP, REPLAY_CLIP_NAME_CAPACITY - 1,
						  state->clipName, REPLAY_SAVE_BACKGROUND_COLOR);
	else
		XwReplayEdit_Push(REPLAY_SAVE_HIGH_LEFT, REPLAY_SAVE_HIGH_TOP, REPLAY_CLIP_NAME_CAPACITY - 1,
						  state->clipName, REPLAY_SAVE_BACKGROUND_COLOR);
	return 0;
}

void XwReplayInput_WaitForSave(void) { input_save_pending = true; }

int XwReplayInput_IsSavePending(void) { return input_save_pending; }

void XwReplayInput_CancelSave(void) {
	input_save_pending = false;
	result_ready = false;
	g_flightLockOffscreenSurface = 1;
}

XwReplayInputSaveState XwReplayInput_ResumeSave(void) {
	XwFlightMessageId result;
	if (!input_save_pending)
		return XW_REPLAY_INPUT_SAVE_IDLE;
	if (save_pending)
		return XW_REPLAY_INPUT_SAVE_PENDING;
	result = result_ready ? XwReplaySave_Run() : XW_MSG_REPLAY_CLIP_NOT_SAVED;
	input_save_pending = false;
	FlightDisplay_UnlockSurface();
	g_flightLockOffscreenSurface = 1;
	FlightDisplay_LockSurface();
	replay_replaymessage(result);
	replay_outputclipname();
	return XW_REPLAY_INPUT_SAVE_FINISHED;
}
