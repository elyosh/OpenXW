#include "xw_runtime/runtime/replay_viewer_task.h"
#include "xw/flight/flight.h"
#include "xw/render/flight_view.h"
#include "xw_dos94/flight/hud/panel.h"
#include "xw_dos94/render/display.h"
#include "xw_runtime/runtime/flight_dispatch.h"
#include "xw_runtime/runtime/flight_types.h"
#include "xw_runtime/snapshot/render_hud.h"

#include "xw/audio/fsfx.h"
#include "xw/audio/hilevel.h"
#include "xw/audio/lolevel.h"
#include "xw/flight/fediskio.h"
#include "xw/flight/feinput.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/hud/festring.h"
#include "xw/flight/hud/msg.h"
#include "xw/flight/hud/panel.h"
#include "xw/flight/hud/panelrts.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/player/user.h"
#include "xw/flight/replay/replay.h"
#include "xw/flight/replay/replayio.h"
#include "xw/flight/shell_flight.h"
#include "xw/flight/xw.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/math/transfm2.h"
#include "xw/render/flight_starfield.h"
#include "xw/render/flight_view.h"
#include "xw/render/rtsvga2.h"
#include "xw_runtime/runtime/flight_frame.h"
#include "xw_runtime/runtime/port.h"
#include "xw_runtime/runtime/presentation.h"
#include "xw_runtime/storage/replay_format.h"
#include "xw_runtime/timing/player_timing.h"

#include <landru/task.h>
#include <landru/timer.h>
#include <stdlib.h>
#include <string.h>

typedef enum ReplayViewerPhase {
	VIEWER_START_PLAYBACK,
	VIEWER_AFTER_PLAYBACK,
	VIEWER_FLIGHT,
	VIEWER_RELOAD
} ReplayViewerPhase;

typedef struct ReplayViewerState {
	ReplayViewerPhase phase;
	uint32_t savedResolutionMode;
	XwFlightFrame frame;
} ReplayViewerState;

static int viewer_initialize(ReplayViewerState* state) {
	unsigned int liveViewportOffset320, liveViewportOffset640, filmViewportOffset320, filmViewportOffset640;
	state->savedResolutionMode = g_flightResolutionMode;
	if (g_flightAudioMode != 0) {
		g_ReplaySavedVolume = hilevel_ImGetMasterVol();
		hilevel_ImSetMasterVol(0);
		lolevel_ImPause();
		g_ReplayMusicActive = 0;
	}
	if ((uint8_t)g_flightDisplaySurfaceMode == 0 || g_ReplayReturnToExistingCheckpoint != 0 ||
		replayio_copytosave("+savegame.rpy") != 0) {
		g_ReplayReturnToExistingCheckpoint = 0;
		g_ReplayRecording = 0;
		g_replayviewmode = 1;
		if (XwFlightTypes_Dos()) {
			bool film = !(uint8_t)g_flightDisplaySurfaceMode;
			if (film) {
				int loaded = replay_loadreplay();
				if (!loaded || g_quitRequested || !replayio_copyfromsave(g_ReplayStartFilename)) {
					XwPort_Fail("Cannot restore the film checkpoint");
					return 0;
				}
				Dos94Display_Restore();
				XwFlightMode_LoadResources();
				if (g_quitRequested)
					return 0;
				XwFlightMode_LoadPanel();
				if (g_quitRequested)
					return 0;
				msg_messageinit();
			} else
				strcpy(g_ReplayClipName, "UNTITLED");
			Dos94Panel_Replay(film);
			if (g_quitRequested)
				return 0;
			replay_rewindreplay();
			return 1;
		}
		FlightDisplay_LockSurface();
		if ((uint8_t)g_flightDisplaySurfaceMode != 0) {
			festring_clearscreen();
			state->savedResolutionMode = g_flightResolutionMode;
			g_hudPanelSpriteDataWriteCursor = g_hudPanelSpriteCraftDataStart;
			strcpy(g_ReplayClipName, "UNTITLED");
			strcpy(g_hudCockpitBasePath, g_hudCockpitResolutionDirectory);
			strcat(g_hudCockpitBasePath, "camerap");
			strcat(g_hudCockpitBasePath, ".PNL");
			fediskio_loadbufferdata(g_hudCockpitBasePath, PANEL_SHARED_SPRITE_COUNT,
									REPLAYIO_CAMERA_SPRITE_COUNT, 0);
			g_hudCockpitResourceWriteCursor = g_flightOffscreenBuffer;
			panel_loadcontrolpanel("camera", g_replayCockpitResources, REPLAYIO_PANEL_RESOURCE_COUNT);
			g_flightSetPaletteRangeFn(
				(const struct RgbTriplet*)g_replayCockpitResources[REPLAYIO_PANEL_PALETTE], 0,
				REPLAYIO_PANEL_PALETTE_COUNT);
			festring_setbackcolor(g_flightColorEscapeBypassChar);
			g_flightFillClipRectFn();
			g_flightBlitSpriteFn(g_replayCockpitResources[REPLAYIO_PANEL_IMAGE], 0, 0, 0, 0);
			if (g_flightResolutionMode == FLIGHT_DISPLAY_MODE_13H) {
				liveViewportOffset320 = g_flightComputePixelOffsetFn(0, REPLAYIO_LIVE_LOW_TOP);
				SetFlightViewport(FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH, REPLAYIO_LIVE_LOW_HEIGHT,
								  g_flightViewportMode, liveViewportOffset320);
			} else {
				liveViewportOffset640 = g_flightComputePixelOffsetFn(0, REPLAYIO_LIVE_HIGH_TOP);
				SetFlightViewport(FLIGHT_DISPLAY_WIDTH, REPLAYIO_LIVE_HIGH_HEIGHT, g_flightViewportMode,
								  liveViewportOffset640);
			}
		} else {
			int loaded = replay_loadreplay();
			if (!loaded || g_quitRequested || !replayio_copyfromsave(g_ReplayStartFilename)) {
				XwPort_Fail("Cannot restore film checkpoint");
				return 0;
			}
			XwFlightMode_LoadResources();
			g_starfieldColorCacheReusable = 0;
			FlightDisplay_LockSurface();
			g_camMatR0_X = 0;
			g_camMatR0_Y = 0;
			g_camMatR0_Z = 0;
			g_camMatR1_X = 0;
			g_camMatR1_Y = 0;
			g_camMatR1_Z = 0;
			g_camMatR2_X = 0;
			g_camMatR2_Y = 0;
			g_camMatR2_Z = 0;
			g_flightVpWidth = 0;
			g_flightVpHeight = 0;
			FlightStarfield_Render();
			FlightDisplay_UnlockSurface();
			g_starfieldColorCacheReusable = 1;
			XwFlightMode_LoadPanel();
			msg_messageinit();
			g_hudPanelSpriteDataWriteCursor = g_hudPanelSpriteCraftDataStart;
			strcpy(g_hudCockpitBasePath, g_hudCockpitResolutionDirectory);
			strcat(g_hudCockpitBasePath, "camerap");
			strcat(g_hudCockpitBasePath, ".PNL");
			fediskio_loadbufferdata(g_hudCockpitBasePath, PANEL_SHARED_SPRITE_COUNT,
									REPLAYIO_CAMERA_SPRITE_COUNT, 0);
			g_hudCockpitResourceWriteCursor = g_flightOffscreenBuffer;
			panel_loadcontrolpanel("film", g_replayCockpitResources, REPLAYIO_PANEL_RESOURCE_COUNT);
			g_flightSetPaletteRangeFn(
				(const struct RgbTriplet*)g_replayCockpitResources[REPLAYIO_PANEL_PALETTE], 0,
				REPLAYIO_PANEL_PALETTE_COUNT);
			if (g_flightResolutionMode == FLIGHT_DISPLAY_MODE_13H)
				festring_setbound(REPLAYIO_FILM_LOW_LEFT, REPLAYIO_FILM_LOW_TOP, REPLAYIO_FILM_LOW_RIGHT,
								  REPLAYIO_FILM_LOW_BOTTOM);
			else
				festring_setbound(0, REPLAYIO_FILM_HIGH_TOP, REPLAYIO_FILM_HIGH_RIGHT,
								  REPLAYIO_FILM_HIGH_BOTTOM);
			festring_setbackcolor(g_flightColorEscapeBypassChar);
			g_flightFillClipRectFn();
			festring_setbound(0, REPLAYIO_FOOTER_TOP, FLIGHT_DISPLAY_WIDTH, FLIGHT_DISPLAY_HEIGHT);
			g_flightBlitSpriteFn(g_replayCockpitResources[REPLAYIO_PANEL_IMAGE], 0, 0, 0, 0);
			if (g_flightResolutionMode == FLIGHT_DISPLAY_MODE_13H) {
				filmViewportOffset320 = g_flightComputePixelOffsetFn(0, REPLAYIO_FILM_LOW_TOP);
				SetFlightViewport(FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH, REPLAYIO_FILM_LOW_HEIGHT,
								  g_flightViewportMode, filmViewportOffset320);
			} else {
				filmViewportOffset640 = g_flightComputePixelOffsetFn(0, REPLAYIO_FILM_HIGH_TOP);
				SetFlightViewport(FLIGHT_DISPLAY_WIDTH, REPLAYIO_FILM_HIGH_HEIGHT, g_flightViewportMode,
								  filmViewportOffset640);
			}
		}
		panel_copymaskdata(g_replayCockpitResources[REPLAYIO_PANEL_MASK], g_flightVpWidth, g_flightVpHeight,
						   0);
		g_projOffsetY = 0;
		XwHud_View(18, (uint8_t)g_flightDisplaySurfaceMode ? "CAMERA.LFD" : "FILM.LFD",
				   (XwSnapRect) { g_flightVpX, g_flightVpY, g_flightVpWidth, g_flightVpHeight }, false);
		replay_rewindreplay();
		FlightDisplay_UnlockSurface();
		return 1;
	}
	return 0;
}

static void viewer_reload(void) {
	int brightness;
	unsigned int returnViewportOffset320, returnViewportOffset640;

	g_ReplaySavedVolume = hilevel_ImGetMasterVol();
	hilevel_ImSetMasterVol(0);
	lolevel_ImPause();
	g_ReplayMusicActive = 0;
	g_flightRenderTransitionHook();
	FlightDisplay_ClearOffscreenSurface();
	g_ReplayRecording = 0;
	g_replayviewmode = 1;
	int loaded = replay_loadreplay();
	if (!loaded || g_quitRequested || !replayio_copyfromsave(g_ReplayStartFilename)) {
		XwPort_Fail("Cannot reload film checkpoint");
		return;
	}
	brightness = g_flightBrightnessSetting;
	if ((unsigned int)brightness > USER_BRIGHTNESS_MAX)
		brightness = USER_BRIGHTNESS_MAX;
	g_flightBrightnessScaleQ8 = (brightness + USER_BRIGHTNESS_BIAS) * USER_BRIGHTNESS_STEP;
	msg_messageinit();
	if (XwFlightTypes_Dos()) {
		Dos94Display_Restore();
		XwFlightMode_LoadResources();
		if (g_quitRequested)
			return;
		XwFlightMode_LoadPanel();
		if (g_quitRequested)
			return;
		Dos94Panel_Replay(true);
		replay_rewindreplay();
		return;
	}
	FlightDisplay_LockSurface();
	g_hudPanelSpriteDataWriteCursor = g_hudPanelSpriteCraftDataStart;
	strcpy(g_hudCockpitBasePath, g_hudCockpitResolutionDirectory);
	strcat(g_hudCockpitBasePath, "camerap");
	strcat(g_hudCockpitBasePath, ".PNL");
	fediskio_loadbufferdata(g_hudCockpitBasePath, PANEL_SHARED_SPRITE_COUNT, REPLAYIO_CAMERA_SPRITE_COUNT, 0);
	g_hudCockpitResourceWriteCursor = g_flightOffscreenBuffer;
	panel_loadcontrolpanel("film", g_replayCockpitResources, REPLAYIO_PANEL_RESOURCE_COUNT);
	g_flightSetPaletteRangeFn((const struct RgbTriplet*)g_replayCockpitResources[REPLAYIO_PANEL_PALETTE], 0,
							  REPLAYIO_PANEL_PALETTE_COUNT);
	if (g_flightResolutionMode == FLIGHT_DISPLAY_MODE_13H)
		festring_setbound(REPLAYIO_FILM_LOW_LEFT, REPLAYIO_FILM_LOW_TOP, REPLAYIO_FILM_LOW_RIGHT,
						  REPLAYIO_FILM_LOW_BOTTOM);
	else
		festring_setbound(0, REPLAYIO_FILM_HIGH_TOP, REPLAYIO_FILM_HIGH_RIGHT, REPLAYIO_FILM_HIGH_BOTTOM);
	festring_setbackcolor(g_flightColorEscapeBypassChar);
	g_flightFillClipRectFn();
	festring_setbound(0, REPLAYIO_FOOTER_TOP, FLIGHT_DISPLAY_WIDTH, FLIGHT_DISPLAY_HEIGHT);
	g_flightBlitSpriteFn(g_replayCockpitResources[REPLAYIO_PANEL_IMAGE], 0, 0, 0, 0);
	if (g_flightResolutionMode == FLIGHT_DISPLAY_MODE_13H) {
		returnViewportOffset320 = g_flightComputePixelOffsetFn(0, REPLAYIO_FILM_LOW_TOP);
		SetFlightViewport(FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH, REPLAYIO_FILM_LOW_HEIGHT, g_flightViewportMode,
						  returnViewportOffset320);
	} else {
		returnViewportOffset640 = g_flightComputePixelOffsetFn(0, REPLAYIO_FILM_HIGH_TOP);
		SetFlightViewport(FLIGHT_DISPLAY_WIDTH, REPLAYIO_FILM_HIGH_HEIGHT, g_flightViewportMode,
						  returnViewportOffset640);
	}
	panel_copymaskdata(g_replayCockpitResources[REPLAYIO_PANEL_MASK], g_flightVpWidth, g_flightVpHeight, 0);
	g_projOffsetY = 0;
	XwHud_View(18, (uint8_t)g_flightDisplaySurfaceMode ? "CAMERA.LFD" : "FILM.LFD",
			   (XwSnapRect) { g_flightVpX, g_flightVpY, g_flightVpWidth, g_flightVpHeight }, false);
	FlightDisplay_UnlockSurface();
	replay_rewindreplay();
}

static LandruTaskStepResult viewer_step(void* self) {
	ReplayViewerState* state = self;
	if (g_quitRequested)
		return LANDRU_TASK_STEP_DONE;
	if (state->phase == VIEWER_START_PLAYBACK) {
		state->phase = VIEWER_AFTER_PLAYBACK;
		g_ReplayReenterSimulation = 0;
		festring_showscreen();
		FlightDisplay_BlitRenderSurface();
		FlightDisplay_Flip();
		FlightDisplay_BlitRenderSurface();
		replay_doreplayscreen();
		return LANDRU_TASK_STEP_YIELD;
	}
	if (state->phase == VIEWER_AFTER_PLAYBACK) {
		XwPresentation_BeginFlightUi();
		if (g_ReplayMusicActive != 0) {
			g_ReplaySavedVolume = hilevel_ImGetMasterVol();
			hilevel_ImSetMasterVol(0);
			lolevel_ImPause();
			g_ReplayMusicActive = 0;
		}
		g_replayviewmode = 0;
		if ((uint8_t)g_flightDisplaySurfaceMode != 0) {
			if (!XwFlightTypes_Dos() && state->savedResolutionMode != g_flightResolutionMode) {
				g_flightResolutionMode = state->savedResolutionMode;
				Xw_InitFlightResolution();
				rtsvga2_initgraphVGA();
				feinput_setupgraphics(g_flightGraphicsDetailPreset);
				fediskio_readfiletofarmemory("TINY64.FNT", gTinyFntBuf);
				fediskio_readfiletofarmemory("MICRO64.FNT", gMicroFntBuf);
				festring_setfontsize(FLIGHT_FONT_TINY);
			}
			if (!replayio_copyfromsave("+savegame.rpy")) {
				XwPort_Fail("Cannot restore the live flight checkpoint");
				return LANDRU_TASK_STEP_DONE;
			}
			if (XwFlightTypes_Dos())
				Dos94Display_Restore();
			if (g_ReplayReturnToExistingCheckpoint == 0)
				replayio_setreturnview();
			if (g_quitRequested)
				return LANDRU_TASK_STEP_DONE;
			if (g_ReplayMusicActive == 0) {
				hilevel_ImSetMasterVol((uint16_t)g_ReplaySavedVolume);
				lolevel_ImResume();
				g_ReplayMusicActive = 1;
			}
			if (g_ReplayReturnToExistingCheckpoint != 0 && g_flightAudioMode != 0)
				lolevel_ImStopAllSounds();

		} else if (g_ReplayReenterSimulation != 0) {
			XwPlayerTiming_ResumeControls();
			g_flightRenderTransitionHook();
			g_missionRuntimeState.flightExitRequested = 0;
			g_ReplayRecording = 0;
			g_flightCamera.focusObjectRef = g_playerFlightState.objectIndex;
			g_flightCamera.externalViewActive = 0;
			g_flightCamera.hudStateLive = 0;
			g_flightCamera.hudAimY = 0;
			g_flightCamera.hudAimX = 0;
			g_flightCamera.rearViewHudOffset = 0;
			replayio_setreturnview();
			if (g_ReplayMusicActive == 0) {
				hilevel_ImSetMasterVol((uint16_t)g_ReplaySavedVolume);
				lolevel_ImResume();
				g_ReplayMusicActive = 1;
			}
			g_flightAccumulatedTicks = 0;
			xtimer_Time_Elapsed();

			state->phase = VIEWER_FLIGHT;
		} else {
			g_flightRenderTransitionHook();
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_ClearOffscreenSurface();
			FlightDisplay_Flip();
			if (g_ReplayMusicActive == 0) {
				hilevel_ImSetMasterVol((uint16_t)g_ReplaySavedVolume);
				lolevel_ImResume();
				g_ReplayMusicActive = 1;
			}
		}
		if (state->phase != VIEWER_FLIGHT) {
			if (g_ReplayReenterSimulation != 0) {
				state->phase = VIEWER_START_PLAYBACK;
				return LANDRU_TASK_STEP_CONTINUE;
			}
			fsfx_UpdatePlayerEngineLoop();
			return LANDRU_TASK_STEP_DONE;
		}
	}
	if (state->phase == VIEWER_FLIGHT) {
		if (g_missionRuntimeState.flightExitRequested == 0 || state->frame.phase != XW_FRAME_IDLE) {
			if (!XwFlightFrame_TickState(&state->frame))
				return LANDRU_TASK_STEP_YIELD;
			if (g_missionRuntimeState.flightExitRequested != 0)
				state->phase = VIEWER_RELOAD;
			return LANDRU_TASK_STEP_FRAME_COMPLETE;
		}
		state->phase = VIEWER_RELOAD;
	}
	viewer_reload();
	if (g_quitRequested)
		return LANDRU_TASK_STEP_DONE;
	XwPort_RebaseClock();
	if (g_ReplayReenterSimulation != 0) {
		state->phase = VIEWER_START_PLAYBACK;
		return LANDRU_TASK_STEP_CONTINUE;
	}
	fsfx_UpdatePlayerEngineLoop();
	return LANDRU_TASK_STEP_DONE;
}

static void viewer_end(void* self) {
	ReplayViewerState* state = self;
	XwFlightFrame_CancelState(&state->frame);
	XwPresentation_BeginFlightUi();
	if (g_flightAudioMode && !g_ReplayMusicActive) {
		hilevel_ImSetMasterVol((uint16_t)g_ReplaySavedVolume);
		lolevel_ImResume();
		g_ReplayMusicActive = 1;
	}
	g_replayviewmode = 0;
	g_ReplayPlaybackActive = g_ReplayReenterSimulation = 0;
	XwFlightFrame_ResumeClock();
}

static uint64_t viewer_next_wake(const void* self) {
	const ReplayViewerState* state = self;
	return state->phase == VIEWER_FLIGHT ? XwFlightFrame_NextWakeDelayUs(&state->frame) : UINT64_MAX;
}

static const LandruTaskVtable viewer_vtable = { viewer_step, viewer_end, NULL, viewer_next_wake };

void XwReplayViewer_Begin(void) {
	XwPresentation_BeginFlightUi();
	XwFlightFrame_ResumeClock();
	ReplayViewerState* state = landru_task_push(&viewer_vtable);
	if (state == NULL)
		abort();
	*state = (ReplayViewerState) { 0 };
	if (!viewer_initialize(state))
		landru_task_pop();
	XwPort_RebaseClock();
}
