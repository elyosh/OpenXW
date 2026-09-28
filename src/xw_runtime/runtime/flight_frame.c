#include "xw_runtime/runtime/flight_frame.h"
#include "xw/flight/flight.h"
#include "xw_runtime/input/flight_controls.h"
#include "xw_runtime/runtime/flight_dispatch.h"
#include "xw_runtime/runtime/flight_types.h"
#include "xw_runtime/runtime/port.h"
#include "xw_runtime/runtime/profile.h"
#include "xw_runtime/snapshot/render_capture.h"
#include "xw_runtime/snapshot/render_hud.h"
#include "xw_runtime/storage/replay_format.h"
#include "xw_runtime/timing/flight_timing.h"
#include "xw_runtime/timing/reference_motion.h"

#include "xw/audio/fsfx.h"
#include "xw/flight/ai/pai.h"
#include "xw/flight/death_star.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/gate.h"
#include "xw/flight/hud/festring.h"
#include "xw/flight/hud/msg.h"
#include "xw/flight/hud/panel.h"
#include "xw/flight/hud/panelrts.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/anim.h"
#include "xw/flight/object/collide.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/dynamix.h"
#include "xw/flight/object/laser.h"
#include "xw/flight/object/move.h"
#include "xw/flight/player/user.h"
#include "xw/flight/replay/replay.h"
#include "xw/flight/xw.h"
#include "xw/render/render_scene.h"
#include "xw/render/renderer.h"
#include "xw_runtime/runtime/gate_collision_task.h"

#include <landru/timer.h>

static XwFlightFrame s_frame;
static XwFlightFrame* s_inputFrame = &s_frame;

static int FrameQuitRequested(void) {
	if (!g_quitRequested)
		return 0;
	XwFlightTiming_EndAdvance();
	return 1;
}

void XwFlightFrame_ResumeClock(void) {
	XwFlightControls_ResetThrottle();
	g_flightAccumulatedTicks = 0;
	xtimer_Set_Elapsed_Period_Us(XwProfile_ActiveFlight()->tick_period_us);
	XwPort_RebaseClock();
}

void XwFlightFrame_SetInputPending(int pending) { s_inputFrame->inputPending = pending; }

void XwFlightFrame_UpdateInput(void) {
	s_inputFrame->inputPending = !XwFlightInput_Tick(&s_inputFrame->input);
}

int XwFlightFrame_IsPending(void) { return s_frame.phase != XW_FRAME_IDLE; }

void XwFlightFrame_CancelState(XwFlightFrame* frame) {
	if (frame->rendered && frame->phase != XW_FRAME_IDLE)
		XwRenderCapture_CancelView();
	XwFlightInput_Cancel(&frame->input);
	if (frame->phase == XW_FRAME_COLLISION)
		XwCollisionLoop_Cancel();
	/* Cancelling an input-only parent must not end its nested replay's advance. */
	if (frame->phase >= XW_FRAME_SIMULATE)
		XwFlightTiming_EndAdvance();
	frame->phase = XW_FRAME_IDLE;
	frame->inputPending = 0;
	frame->rendered = 0;
}

void XwFlightFrame_Cancel(void) { XwFlightFrame_CancelState(&s_frame); }

int XwFlightFrame_Tick(void) { return XwFlightFrame_TickState(&s_frame); }

uint64_t XwFlightFrame_NextWakeDelayUs(const XwFlightFrame* frame) {
	if (frame->phase != XW_FRAME_IDLE || g_replayviewmode)
		return UINT64_MAX;
	uint16_t minimum = XwFlightTiming_StepTicks();
	return g_flightAccumulatedTicks >= minimum ? 0
											   : xtimer_Elapsed_Delay_Us(minimum - g_flightAccumulatedTicks);
}

int XwFlightFrame_TickState(XwFlightFrame* frame) {
	int16_t measuredTicks;
	uint16_t minimum = XwFlightTiming_StepTicks();
	if (FrameQuitRequested())
		return 1;
	if (frame->phase == XW_FRAME_IDLE) {
		if (g_replayviewmode != 0) {
			if (!XwReplayFormat_RequireInputRecords(1)) {
				XwPort_Fail(XwReplayFormat_Error());
				return 1;
			}
			g_elapsedTicks = g_ReplayInputPointer[REPLAY_INPUT_ELAPSED_OFFSET];
			if (!g_elapsedTicks) {
				XwPort_Fail("Flight recording contains a zero-duration frame");
				return 1;
			}
			g_simStepScale = XW_SIMULATION_TICKS_PER_SECOND / g_elapsedTicks;
			if (g_simStepScale == 0)
				g_simStepScale = 1;
		} else {
			if (g_flightAccumulatedTicks < minimum) {
				g_flightAccumulatedTicks += xtimer_Time_Elapsed();
				if (g_flightAccumulatedTicks < minimum)
					return 0;
			}
			measuredTicks = g_flightAccumulatedTicks;
			g_previousFrameMeasuredTicks = g_flightAccumulatedTicks;
			g_flightAccumulatedTicks = 0;
			if (calcframerate != 0) {
				g_elapsedTicks = measuredTicks;
				g_simStepScale = XW_SIMULATION_TICKS_PER_SECOND / measuredTicks;
				if (g_simStepScale == 0) {
					g_simStepScale = 1;
					g_elapsedTicks = XW_SIMULATION_TICKS_PER_SECOND;
				}
			}
			calcframerate = 1;
		}
		if (g_showSimStepScale != 0) {
			festring_setfontsize(XW_FRAME_RATE_FONT_TIER);
			festring_setbound(0, 0, g_flightScreenWidth, g_flightScreenHeight);
			festring_setbackcolor(XW_FRAME_RATE_BACKGROUND);
			festring_settextcolor(XW_FRAME_RATE_FOREGROUND);
			g_flightTextShadowEnabled = 0;
			festring_setcursor(g_flightScreenWidth - XW_FRAME_RATE_RIGHT_MARGIN,
							   g_flightScreenHeight - XW_FRAME_RATE_BOTTOM_LINES * g_flightFontLineHeight);
			int hud_pane = XwHud_Push(XW_SNAP_PANE_FRAME_RATE);
			panelrts_outnum(g_simStepScale, XW_FRAME_RATE_DIGITS, 1);
			XwHud_Pop(hud_pane);
		}
		g_replayUiEventConsumed = 0;

		frame->rendered = 0;
		frame->phase = XW_FRAME_INPUT;
	}
	if (frame->phase == XW_FRAME_INPUT) {
		XwFlightFrame* previousInputFrame = s_inputFrame;
		s_inputFrame = frame;
		user_userinterface();
		s_inputFrame = previousInputFrame;
		if (FrameQuitRequested())
			return 1;
		if (frame->inputPending)
			return 0;
		if (g_missionRuntimeState.flightExitRequested != 0 || g_replayUiEventConsumed != 0) {
			XwFlightTiming_EndAdvance();
			frame->phase = XW_FRAME_IDLE;
			return 1;
		}
		XwRenderCapture_Simulate(g_elapsedTicks);
		XwFlightTiming_BeginAdvance(g_elapsedTicks);
		frame->phase = XW_FRAME_SIMULATE;
	}
	if (frame->phase == XW_FRAME_SIMULATE) {
		Xw_updatetime();
		if (XwFlightTiming_ReferenceDue()) {
			XwFlightClock clock = XwFlightTiming_EnterReference();
			create_updatefgstatus();
			if (!g_quitRequested) {
				pai_updateplaneai();
				if (g_deathStarSurfaceModeActive != 0)
					DeathStar_UpdateSurfaceGuns();
				if (g_missionRuntimeState.provingGroundsActive != 0)
					gate_updategateguns();
			}
			XwFlightTiming_RestoreClock(clock);
		}
		if (FrameQuitRequested())
			return 1;
		laser_weaponsfire();
		XwReferenceMotion_CommitBoundary();
		dynamix_planedynamics();
		if (FrameQuitRequested())
			return 1;
		if (g_replayviewmode != 0) {
			if (g_ReplayFastForward != 0) {
				if (g_elapsedTicks > (uint16_t)g_ReplayFastForwardTimer) {
					g_ReplayFastForwardTimer += XW_SIMULATION_TICKS_PER_SECOND;
					XwFlightMode_UpdateScreen();
					frame->rendered = 1;
				}
				g_ReplayFastForwardTimer -= g_elapsedTicks;
			} else {
				XwFlightMode_UpdateScreen();
				frame->rendered = 1;
			}
		} else {
			XwFlightMode_UpdateScreen();
			frame->rendered = 1;
			if (FrameQuitRequested())
				return 1;
			FlightDisplay_LockSurface();
			panel_updatepanel();
			FlightDisplay_UnlockSurface();
		}
		if (g_debrisEnabled != 0 && XwFlightTiming_ReferenceDue())
			create_checkdebris();
		if (FrameQuitRequested())
			return 1;

		frame->phase = XW_FRAME_COLLISION;
	}
	if (frame->phase == XW_FRAME_COLLISION) {
		collide_collisions();
		if (FrameQuitRequested())
			return 1;
		if (XwCollisionLoop_IsPending())
			return 0;
		frame->phase = XW_FRAME_FINISH;
	}
	move_moveobjects();
	if (FrameQuitRequested())
		return 1;
	XwFlightTiming_MovementCompleted();
	XwFlightMode_UpdateAnimation();
	if (XwFlightTiming_ReferenceDue() &&
		(int16_t)g_legacyOscillatorFrameCounter++ > XW_OSCILLATOR_FRAME_LIMIT) {
		if (g_legacyOscillatorValue == XW_OSCILLATOR_MAXIMUM)
			g_legacyOscillatorDirection = -1;
		if (g_legacyOscillatorValue == 0)
			g_legacyOscillatorDirection = 1;
		g_legacyOscillatorFrameCounter = 0;
		g_legacyOscillatorValue += g_legacyOscillatorDirection;
	}
	if (XwFlightTiming_ReferenceDue() || g_missionRuntimeState.flightExitRequested != 0)
		Mission_UpdateLogic();
	msg_messageupdate();
	Xw_UpdateDynamicMusicState();
	if (g_fsfxLoaded != 0) {
		if (g_fsfxVoiceQueueCount != 0)
			fsfx_checkblastqueue();
		fsfx_checktieflyby();
	}
	fsfx_UpdatePlayerEngineLoop();
	if (frame->rendered != 0) {
		FlightDisplay_Flip();
		if (!XwFlightTypes_Dos() && g_useHardware3D != 0)
			RenderScene_ClearFrameBuffers();
		else
			FlightDisplay_BlitRenderSurface();
	}
	XwFlightTiming_EndAdvance();
	frame->phase = XW_FRAME_IDLE;
	return 1;
}
