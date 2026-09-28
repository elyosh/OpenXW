#include "xw_runtime/runtime/flight_dispatch.h"
#include "xw/audio/fsfx.h"
#include "xw/flight/death_star.h"
#include "xw/flight/fediskio.h"
#include "xw/flight/flight.h"
#include "xw/flight/gate.h"
#include "xw/flight/hud/panel.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/anim.h"
#include "xw/flight/replay/replay.h"
#include "xw/flight/xw.h"
#include "xw/util/memory.h"
#include "xw_dos94/assets/models.h"
#include "xw_dos94/flight/hud/panel.h"
#include "xw_dos94/flight/hyperspace.h"
#include "xw_dos94/flight/object/effects.h"
#include "xw_dos94/flight/special_world.h"
#include "xw_dos94/render/display.h"
#include "xw_dos94/render/world.h"
#include "xw_runtime/audio/imuse_session.h"
#include "xw_runtime/runtime/flight_mode.h"
#include "xw_runtime/runtime/flight_types.h"
#include "xw_runtime/runtime/port.h"
#include "xw_runtime/runtime/presentation.h"
#include "xw_runtime/snapshot/render_capture.h"
#include "xw_runtime/storage/storage.h"
#include "xw_runtime/timing/flight_timing.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t* dosReplayBuffer;

void XwFlightMode_UnloadSounds(void) {
	for (unsigned i = 0; i < FSFX_SOUND_HANDLE_COUNT; ++i) {
		if (g_dos94Imuse)
			imuse_forget_sound(g_dos94Imuse, i);
		if (g_fsfxLoadedSoundHandles[i])
			Memory_FreeHandle(g_fsfxLoadedSoundHandles[i]);
		g_fsfxLoadedSoundHandles[i] = 0;
	}
	g_fsfxLoaded = 0;
	g_fsfxVoiceQueueCount = 0;
	g_fsfxCurrentVoiceSfxSlot = FSFX_NO_VOICE_SLOT;
}

void XwFlightMode_LoadSounds(void) {
	char path[FEDISKIO_SOUND_PATH_CAPACITY];
	strcpy(path, g_flightResourcePrefix);
	strcat(path, "BLAST.LFD");
	fsfx_loadsfx(path, FSFX_BLAST_RESOURCE_TAG);
}

void XwFlightMode_ResourceError(const char* path, const char* reason) {
	char error[512];
	snprintf(error, sizeof error, "X-Wing %d %s: %s", XwGameVersion_Year(XwProfile_MissionFlight()->version),
			 path, reason);
	XwPort_Fail(error);
}

int16_t XwFlightMode_OpenMission(const char* path) {
	g_stream = XwStorage_OpenMission(path);
	if (!g_stream) {
		XwFlightMode_ResourceError(path, "cannot open mission");
		return 0;
	}
	return 1;
}

void XwFlightMode_InitBuffers(void) {
	if (XwFlightTypes_Dos()) {
		char error[256];
		if (!Dos94Display_Init(error, sizeof error))
			XwPort_Fail(error);
		if (g_quitRequested)
			return;
		dosReplayBuffer = calloc(1, REPLAY_BUFFER_CAPACITY);
		if (!dosReplayBuffer) {
			XwFlightMode_ResourceError("replay", "cannot allocate input buffer");
			return;
		}
		g_ReplayBufferStart = g_ReplayInputPointer = dosReplayBuffer;
		XwFlightMode_LoadSounds();
		return;
	}
	fediskio_Init_Buffers_and_Fonts();
}

void XwFlightMode_FreeResources(void) {
	XwRenderCapture_EndMission();
	XwFlightMode_UnloadSounds();
	if (XwFlightTypes_Dos()) {
		Dos94Panel_Free();
		Dos94Display_Free();
		Dos94Assets_ReleaseModels();
		free(dosReplayBuffer);
		dosReplayBuffer = NULL;
		g_ReplayBufferStart = g_ReplayInputPointer = NULL;
		return;
	}
	fediskio_FreeFlightHandles();
}

void XwFlightMode_LoadResources(void) {
	if (XwFlightTypes_Dos()) {
		char error[256];
		if (!Dos94_fediskio_loadspecies(g_deathStarSurfaceModeActive != 0,
										g_missionRuntimeState.provingGroundsActive != 0, error, sizeof error))
			XwPort_Fail(error);
		else
			Dos94Hyperspace_Restore();
	} else
		fediskio_InitResources();
}

void XwFlightMode_UpdateScreen(void) {
	XwPresentation_SelectBaseSource(XwPresentation_BaseSource());
	XwRenderCapture_BeginView(XwFlightTypes_Dos() ? XW_RENDER_SURFACE_DOS : XW_RENDER_SURFACE_BACK);
	if (XwFlightTypes_Dos())
		Dos94_Xw_updatescreen();
	else
		Xw_updatescreen();
	if (g_quitRequested)
		XwRenderCapture_CancelView();
	else
		XwRenderCapture_SealView();
}

void XwFlightMode_LoadPanel(void) {
	if (XwFlightTypes_Dos()) {
		Dos94_panel_loadpaneldata();
		return;
	}
	panel_loadpaneldata();
}

void XwFlightMode_SetPanelView(uint16_t hudViewState) {
	if (XwFlightTypes_Dos()) {
		Dos94_panel_forcenewviewdir(hudViewState);
		return;
	}
	panel_forcenewviewdir(hudViewState);
}

void XwFlightMode_ApplyPanelView(uint16_t hudViewState) {
	if (XwFlightTypes_Dos()) {
		Dos94_panel_dosetnewpilotview(hudViewState);
		return;
	}
	panel_dosetnewpilotview(hudViewState);
}

void XwFlightMode_UpdateCourseBonus(void) {
	if (XwFlightTypes_Dos()) {
		Dos94_gate_updatebonuspoints();
		return;
	}
	gate_updatebonuspoints();
}

void XwFlightMode_UpdateAnimation(void) {
	if (!XwFlightTiming_ReferenceDue() || g_flightGlobalCountdownTimers.ticks[XW_TIMER_ANIMATION])
		return;
	XwFlightClock clock = XwFlightTiming_EnterReference();
	if (XwFlightTypes_Dos())
		Dos94_anim_updateanimation();
	else
		anim_updateanimation();
	XwFlightTiming_RestoreClock(clock);
	XwFlightTiming_AnimationEvent();
}
