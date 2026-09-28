#include "xw_runtime/runtime/replay_screen_task.h"
#include "xw_dos94/flight/hud/replay.h"
#include "xw_runtime/runtime/flight_dispatch.h"
#include "xw_runtime/runtime/flight_types.h"
#include "xw_runtime/snapshot/render_capture.h"
#include "xw_runtime/snapshot/render_hud.h"

#include "xw/audio/hilevel.h"
#include "xw/audio/lolevel.h"
#include "xw/flight/feinput.h"
#include "xw/flight/flight.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/hud/festring.h"
#include "xw/flight/hud/panel.h"
#include "xw/flight/hud/panelrts.h"
#include "xw/flight/player/user.h"
#include "xw/flight/replay/replay.h"
#include "xw/flight/shell_flight.h"
#include "xw/flight/xw.h"
#include "xw/math/math2.h"
#include "xw/render/render_scene.h"
#include "xw/render/renderer.h"
#include "xw_runtime/runtime/flight_frame.h"
#include "xw_runtime/runtime/profile.h"
#include "xw_runtime/runtime/replay_save_task.h"
#include "xw_runtime/timing/host_clock.h"
#include <landru/timer.h>

#include <landru/task.h>
#include <stdlib.h>

typedef enum ReplayScreenPhase {
	REPLAY_SCREEN_INPUT,
	REPLAY_SCREEN_RESUME_INPUT,
	REPLAY_SCREEN_FRAME,
	REPLAY_SCREEN_STATUS,
	REPLAY_SCREEN_PROGRESS,
	REPLAY_SCREEN_DONE
} ReplayScreenPhase;

typedef struct ReplayScreenState {
	ReplayScreenPhase phase;
	uint16_t lastChaseStatus;
	uint16_t lastTrackStatus;
	int surfaceLocked;
	uint64_t nextFrameUs, lastFrameUs;
	uint64_t cameraSampleUs, cameraAccumulatedUs;
	bool cameraPlaying;
	XwFlightFrame frame;
} ReplayScreenState;

static ReplayScreenState* activeScreen;

void XwReplayScreen_ResetCameraClock(void) {
	if (!activeScreen)
		return;
	activeScreen->cameraSampleUs = XwTime_GetElapsedUs();
	activeScreen->cameraAccumulatedUs = 0;
	activeScreen->cameraPlaying = g_ReplayPlaybackActive != 0;
}

bool XwReplayScreen_BeginCameraControls(XwFlightClock* saved) {
	*saved = (XwFlightClock) { g_elapsedTicks, g_simStepScale };
	if (!XwFlightTiming_IsUnlocked())
		return true;
	if (!activeScreen)
		return false;
	uint64_t now = XwTime_GetElapsedUs();
	if (activeScreen->cameraPlaying != (g_ReplayPlaybackActive != 0) || now < activeScreen->cameraSampleUs)
		XwReplayScreen_ResetCameraClock();
	activeScreen->cameraAccumulatedUs += now - activeScreen->cameraSampleUs;
	activeScreen->cameraSampleUs = now;
	const XwFlightProfile* profile = XwProfile_ActiveFlight();
	uint64_t ticks = activeScreen->cameraAccumulatedUs / profile->tick_period_us;
	if (ticks < profile->minimum_frame_ticks)
		return false;
	/* One UI update, bounded like live flight; retain only the partial timer tick. */
	activeScreen->cameraAccumulatedUs %= profile->tick_period_us;
	g_elapsedTicks =
		ticks > XW_SIMULATION_TICKS_PER_SECOND ? XW_SIMULATION_TICKS_PER_SECOND : (uint16_t)ticks;
	g_simStepScale = XW_SIMULATION_TICKS_PER_SECOND / g_elapsedTicks;
	return true;
}

static LandruTaskStepResult screen_step(void* self) {
	ReplayScreenState* state = self;
	uint16_t chaseStatus, trackStatus, elapsedTicks, playedFraction;
	int16_t remainingPercent;
	if (g_quitRequested || state->phase == REPLAY_SCREEN_DONE)
		return LANDRU_TASK_STEP_DONE;
	if (state->phase == REPLAY_SCREEN_INPUT) {
		if (state->nextFrameUs > XwTime_GetElapsedUs())
			return LANDRU_TASK_STEP_YIELD;
		FlightDisplay_LockSurface();
		state->surfaceLocked = 1;
		state->phase = REPLAY_SCREEN_RESUME_INPUT;
	}
	if (state->phase == REPLAY_SCREEN_RESUME_INPUT) {
		replay_replayinput();
		if (XwReplayInput_IsSavePending())
			return LANDRU_TASK_STEP_YIELD;
		if (g_flightAudioMode != 0) {
			if (g_ReplayPlaybackActive == 0 || g_ReplayFastForward != 0) {
				if (g_ReplayMusicActive == 1) {
					g_ReplayMusicActive = 0;
					g_ReplaySavedVolume = hilevel_ImGetMasterVol();
					hilevel_ImSetMasterVol(0);
					lolevel_ImPause();
				}
			} else if (g_ReplayMusicActive == 0) {
				g_ReplayMusicActive = 1;
				hilevel_ImSetMasterVol((uint16_t)g_ReplaySavedVolume);
				lolevel_ImResume();
			}
		}

		if (g_ReplayPlaybackActive != 0) {
			FlightDisplay_UnlockSurface();
			state->surfaceLocked = 0;
			state->phase = REPLAY_SCREEN_FRAME;
		} else {
			FlightDisplay_UnlockSurface();
			XwFlightMode_UpdateScreen();
			FlightDisplay_Flip();
			if (!XwFlightTypes_Dos() && g_useHardware3D != 0)
				RenderScene_ClearFrameBuffers();
			else
				FlightDisplay_BlitRenderSurface();
			FlightDisplay_LockSurface();
			elapsedTicks =
				(uint16_t)((XwTime_GetElapsedUs() - state->lastFrameUs) / xtimer_Elapsed_Period_Us());
			g_elapsedTicks = elapsedTicks;
			if (elapsedTicks == 0) {
				elapsedTicks = 1;
				g_elapsedTicks = 1;
			}
			g_simStepScale = REPLAY_ADVANCE_TICKS / elapsedTicks;
			if (g_simStepScale == 0)
				g_simStepScale = 1;

			state->phase = REPLAY_SCREEN_PROGRESS;
		}
	}
	if (state->phase == REPLAY_SCREEN_FRAME) {
		if (!XwFlightFrame_TickState(&state->frame))
			return LANDRU_TASK_STEP_YIELD;
		if (g_quitRequested)
			return LANDRU_TASK_STEP_DONE;
		FlightDisplay_LockSurface();
		state->surfaceLocked = 1;
		state->phase = REPLAY_SCREEN_STATUS;
	}
	int hud_pane = XwHud_Push(XW_SNAP_PANE_REPLAY_STATUS);
	if (state->phase == REPLAY_SCREEN_STATUS) {
		if (g_ReplayChaseStatusVisible != 0) {
			festring_setbackcolor(REPLAY_PANEL_BACKGROUND_COLOR);
			if (XwFlightTypes_Dos())
				Dos94Replay_StatusBounds(false);
			else if ((uint8_t)g_flightDisplaySurfaceMode != 0) {
				festring_setbound(REPLAY_PANEL_STATUS_LEFT, REPLAY_CHASE_TOP, REPLAY_PANEL_STATUS_RIGHT,
								  REPLAY_CHASE_BOTTOM);
				festring_setcursor(REPLAY_PANEL_STATUS_LEFT, REPLAY_CHASE_TOP);
			} else {
				festring_setbound(REPLAY_ALT_PANEL_STATUS_LEFT, REPLAY_ALT_CHASE_TOP,
								  REPLAY_ALT_PANEL_STATUS_RIGHT, REPLAY_CHASE_BOTTOM);
				festring_setcursor(REPLAY_ALT_PANEL_STATUS_LEFT, REPLAY_ALT_CHASE_TOP);
			}
			chaseStatus = replay_getstatusnum(g_replayCamera.focusObjectRef);
			if (chaseStatus != state->lastChaseStatus) {
				g_flightFillClipRectFn();
				festring_settextcolor(REPLAY_PANEL_STATUS_COLOR);
				festring_outstringcenter(g_ReplayStatusStrings[chaseStatus]);
				state->lastChaseStatus = chaseStatus;
			}
		}
		if (g_trackobject != XW_PLAYER_NO_TARGET) {
			festring_setbackcolor(REPLAY_PANEL_BACKGROUND_COLOR);
			if (XwFlightTypes_Dos())
				Dos94Replay_StatusBounds(true);
			else if ((uint8_t)g_flightDisplaySurfaceMode != 0) {
				festring_setbound(REPLAY_PANEL_STATUS_LEFT, REPLAY_TRACKED_TOP, REPLAY_PANEL_STATUS_RIGHT,
								  REPLAY_TRACKED_BOTTOM);
				festring_setcursor(REPLAY_PANEL_STATUS_LEFT, REPLAY_TRACKED_TOP);
			} else {
				festring_setbound(REPLAY_ALT_PANEL_STATUS_LEFT, REPLAY_TRACKED_TOP,
								  REPLAY_ALT_PANEL_STATUS_RIGHT, REPLAY_ALT_TRACKED_BOTTOM);
				festring_setcursor(REPLAY_ALT_PANEL_STATUS_LEFT, REPLAY_TRACKED_TOP);
			}
			trackStatus = replay_getstatusnum(g_trackobject);
			if (trackStatus != state->lastTrackStatus) {
				g_flightFillClipRectFn();
				festring_settextcolor(REPLAY_PANEL_STATUS_COLOR);
				festring_outstringcenter(g_ReplayStatusStrings[trackStatus]);
				state->lastTrackStatus = trackStatus;
			}
		}

		state->phase = REPLAY_SCREEN_PROGRESS;
	}
	festring_setfontsize(REPLAY_PROGRESS_FONT_TIER);
	if (XwFlightTypes_Dos())
		Dos94Replay_ProgressBounds();
	else if ((uint8_t)g_flightDisplaySurfaceMode != 0) {
		festring_setbound(REPLAY_FLIGHT_PROGRESS_LEFT, REPLAY_FLIGHT_PROGRESS_TOP,
						  REPLAY_FLIGHT_PROGRESS_RIGHT, REPLAY_FLIGHT_PROGRESS_BOTTOM);
		festring_setcursor(REPLAY_FLIGHT_PROGRESS_CURSOR_X, REPLAY_FLIGHT_PROGRESS_TOP);
	} else {
		festring_setbound(REPLAY_STANDALONE_PROGRESS_LEFT, REPLAY_STANDALONE_PROGRESS_TOP,
						  REPLAY_STANDALONE_PROGRESS_RIGHT, REPLAY_STANDALONE_PROGRESS_BOTTOM);
		festring_setcursor(REPLAY_STANDALONE_PROGRESS_CURSOR_X, REPLAY_STANDALONE_PROGRESS_TOP);
	}
	festring_setbackcolor(REPLAY_PANEL_BACKGROUND_COLOR);
	playedFraction = math2_longpercentage(g_ReplayPlaybackFrameIndex, g_ReplayCapacityFrames);
	remainingPercent = REPLAY_PROGRESS_FULL - math2_fraction(REPLAY_PROGRESS_FULL, playedFraction);
	if ((uint16_t)remainingPercent > REPLAY_PROGRESS_MAXIMUM)
		remainingPercent = REPLAY_PROGRESS_MAXIMUM;
	if (remainingPercent != g_ReplayProgressPercent) {
		g_ReplayProgressPercent = remainingPercent;
		g_flightFillClipRectFn();
		festring_settextcolor(REPLAY_CLIP_NAME_TEXT_COLOR);
		panelrts_outnum(remainingPercent, REPLAY_PROGRESS_DIGITS, REPLAY_PROGRESS_DIGITS);
	}
	if (g_elapsedTicks >= (uint16_t)g_ReplayMessageTimer) {
		if (g_ReplayMessageTimer != 0) {
			festring_setbackcolor(REPLAY_SAVE_BACKGROUND_COLOR);
			if (g_flightResolutionMode == FLIGHT_DISPLAY_MODE_13H)
				festring_setbound(0, REPLAY_LOW_MESSAGE_TOP, FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH,
								  FLIGHT_DISPLAY_LOW_RESOLUTION_HEIGHT);
			else
				festring_setbound(0, REPLAY_HIGH_MESSAGE_TOP, FLIGHT_DISPLAY_WIDTH, FLIGHT_DISPLAY_HEIGHT);
			g_flightFillClipRectFn();
		}
		g_ReplayMessageTimer = 0;
	} else {
		g_ReplayMessageTimer -= g_elapsedTicks;
	}

	FlightDisplay_UnlockSurface();
	state->surfaceLocked = 0;
	state->lastFrameUs = XwTime_GetElapsedUs();
	uint32_t delayTicks = g_ReplayPlaybackActive ? (g_ReplayFastForward ? 0 : g_elapsedTicks)
												 : XwProfile_ActiveFlight()->minimum_frame_ticks;
	state->nextFrameUs = state->lastFrameUs + (uint64_t)delayTicks * xtimer_Elapsed_Period_Us();
	state->phase = g_ReplayExitRequested ? REPLAY_SCREEN_DONE : REPLAY_SCREEN_INPUT;
	XwHud_Pop(hud_pane);
	return LANDRU_TASK_STEP_FRAME_COMPLETE;
}

static void screen_end(void* self) {
	ReplayScreenState* state = self;
	if (activeScreen == state)
		activeScreen = NULL;
	if (state->surfaceLocked)
		FlightDisplay_UnlockSurface();
	if (XwReplayInput_IsSavePending())
		XwReplayInput_CancelSave();
	XwFlightFrame_CancelState(&state->frame);
	/* The stopped viewer can own a pending draw outside its simulation frame. */
	XwRenderCapture_CancelView();
}

static uint64_t screen_next_wake(const void* self) {
	const ReplayScreenState* state = self;
	if (state->phase == REPLAY_SCREEN_INPUT) {
		uint64_t now = XwTime_GetElapsedUs();
		return state->nextFrameUs > now ? state->nextFrameUs - now : 0;
	}
	return state->phase == REPLAY_SCREEN_FRAME ? XwFlightFrame_NextWakeDelayUs(&state->frame) : UINT64_MAX;
}

static const LandruTaskVtable screen_vtable = { screen_step, screen_end, NULL, screen_next_wake };

void XwReplayScreen_Begin(void) {
	ReplayScreenState* state = landru_task_push(&screen_vtable);
	if (state == NULL)
		abort();
	*state = (ReplayScreenState) { 0 };
	activeScreen = state;
	state->nextFrameUs = state->lastFrameUs = XwTime_GetElapsedUs();
	g_replayCamera.focusObjectRef = g_playerFlightState.objectIndex;
	g_replayCamera.externalViewActive = 1;
	g_replayCamera.externalDistance = REPLAY_DEFAULT_ZOOM;
	g_replayCamera.manualControlActive = 0;
	g_replayCamera.hudAimY = 0;
	g_replayCamera.hudAimX = 0;
	g_trackobject = XW_PLAYER_NO_TARGET;
	g_textureCacheFlushPending = 1;
	XwFlightMode_UpdateScreen();
	FlightDisplay_LockSurface();
	g_ReplayPlaybackActive = 0;
	g_ReplayFastForward = 0;
	g_ReplayFastForwardTimer = 0;
	XwReplayScreen_ResetCameraClock();
	replay_drawreplaybutton(REPLAY_BUTTON_FOLLOW);
	replay_drawreplaybutton(REPLAY_BUTTON_CHASE_SHOW);
	replay_drawreplaybutton(REPLAY_BUTTON_TRACKING_OFF);
	replay_drawreplaybutton(REPLAY_BUTTON_TRACKED_CLEAR);
	replay_outputclipname();
	FlightDisplay_UnlockSurface();
	state->lastChaseStatus = REPLAY_STATUS_INVALID;
	state->lastTrackStatus = REPLAY_STATUS_INVALID;
	g_ReplayExitRequested = 0;
}
