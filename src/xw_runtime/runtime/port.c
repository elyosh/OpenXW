/* Host lifecycle and pacing follow OpenXvT around X-Wing's existing Landru stack. */
#include "xw_runtime/runtime/port.h"
#include "xw/flight/fediskio.h"
#include "xw/flight/flight.h"
#include "xw/flight/flight_display.h"
#include "xw/input/joystick.h"
#include "xw_dos94/audio/gamesnd.h"
#include "xw_dos94/render/display.h"
#include "xw_runtime/audio/imuse_session.h"
#include "xw_runtime/audio/music_policy.h"
#include "xw_runtime/audio/output.h"
#include "xw_runtime/config/config.h"
#include "xw_runtime/config/preference_apply.h"
#include "xw_runtime/config/preferences.h"
#include "xw_runtime/input/capture.h"
#include "xw_runtime/input/input_bridge.h"
#include "xw_runtime/integration/landru_adapter.h"
#include "xw_runtime/integration/landru_sound.h"
#include "xw_runtime/runtime/flight_frame.h"
#include "xw_runtime/runtime/flight_loading.h"
#include "xw_runtime/runtime/flight_mode.h"
#include "xw_runtime/runtime/flight_sim.h"
#include "xw_runtime/runtime/frontend_task.h"
#include "xw_runtime/runtime/options_task.h"
#include "xw_runtime/runtime/presentation.h"
#include "xw_runtime/runtime/profile.h"
#include "xw_runtime/runtime/replay_save_task.h"
#include "xw_runtime/runtime/startup_task.h"
#include "xw_runtime/snapshot/render_snapshot.h"
#include "xw_runtime/timing/host_clock.h"
#include <aeron/aeron.h>
#include <aeron/compat/host.h>
#include <aeron/log.h>
#include <landru/error.h>
#include <landru/task.h>
#include <string.h>

static int initialized, paused, rebase, exit_code, skip_intro;
static int ever_had_focus;
static int settings_open, settings_requested;
static XwSettingsPage settings_page;
static char startup_options[] = "dinput sfx music voice fullscreen pageflip softwarecursor";

void XwPort_SetSkipIntro(int skip) {
	if (!initialized)
		skip_intro = skip != 0;
}

int XwPort_SkipIntro(void) { return skip_intro; }

void XwPort_RebaseClock(void) { rebase = 1; }

void XwPort_SetSettingsOpen(int open) {
	int closed = settings_open && !open;
	settings_open = open != 0;
	if (closed)
		XwOptions_SettingsClosed();
}

void XwPort_RequestSettingsPage(XwSettingsPage page) {
	settings_page = page;
	settings_requested = 1;
	XwInput_BeginCaptureFrame(Aeron_InputSnapshot(), true);
}

void XwPort_RequestSettings(void) { XwPort_RequestSettingsPage(XW_SETTINGS_GAME); }

int XwPort_SettingsOpen(void) { return settings_open; }

XwSettingsPage XwPort_SettingsPage(void) { return settings_page; }

int XwPort_ConsumeSettingsRequest(void) {
	int request = settings_requested;
	settings_requested = 0;
	return request;
}

void XwPort_Fail(const char* message) {
	Aeron_LogError("xw.port", "%s", message);
	exit_code = 1;
	g_quitRequested = 1;
}

int XwPort_GetExitCode(void) { return Aeron_FatalErrorRequested() ? 1 : exit_code; }

int XwPort_ShouldQuit(void) {
	return !initialized || g_quitRequested || Aeron_QuitRequested() || Aeron_FatalErrorRequested() ||
		   landru_task_stack_empty();
}

int XwPort_Init(int cd_music_available) {
	int width, height;
	char error[1024];
	if (initialized)
		return 1;
	exit_code = 0;
	if (!XwConfig_Settings() || !XwStorage_Vfs() || !Aeron_GetLogicalSize(&width, &height) || width != 640 ||
		height != 480) {
		XwPort_Fail("Game runtime requires a configured 640x480 Aeron host");
		return 0;
	}
	initialized = 1;
	paused = settings_open = settings_requested = 0;
	ever_had_focus = 0;
	rebase = 1;
	g_quitRequested = 0;
	const AeronInputSnapshot* input = Aeron_InputSnapshot();
	g_windowActive = input && input->has_focus;
	g_windowReactivated = 0;
	XwTime_Reset();
	Dos94Display_ResetPaletteCycle();
	if (!XwProfile_Init(error, sizeof error)) {
		XwPort_Fail(error);
		goto failed;
	}
	if (!Dos94_gamesnd_Open_Pre_iMuse()) {
		goto failed;
	}
	XwMusicPolicy_Init(cd_music_available != 0);
	XwPresentation_Init();
	if (!XwLandru_Init()) {
		XwPort_Fail("Cannot initialize Landru host");
		goto failed;
	}
	if (!XwConfig_Apply(&g_savedShellPreferences, error, sizeof error)) {
		XwPort_Fail(error);
		goto failed;
	}
	g_shellPreferences = g_savedShellPreferences.preferences;
	memset(g_joystickCalibrationInitialized, 0, sizeof g_joystickCalibrationInitialized);
	/* The host window mode remains independent of the virtual DirectDraw surfaces. */
	g_flightFullscreen = g_flightPageFlip = 1;
	XwPresentation_SetPointerSuppressed(false);
	XwInput_Init();
	XwInput_BeginCaptureFrame(input, true);
	AeronCompat_Update(1);
	XwInput_BeginFrame(input, true, 0);
	XwRenderSnapshot_BeginTick();
	if (!Flight_Main(startup_options)) {
		XwPort_Fail("Game startup failed");
		goto failed;
	}
	XwRenderSnapshot_Commit(g_windowActive, 0);
	XwPreferences_InitRuntime();
	Aeron_LogInfo("xw.port", "Runtime initialized");
	return 1;
failed:
	XwPort_Shutdown();
	return 0;
}

static void paused_frame(void) {
	if (!paused) {
		paused = 1;
		Aeron_AudioSetPaused(1);
	}
	rebase = 1;
	XwPresentation_SetPointerSuppressed(true);
	XwInput_BeginCaptureFrame(Aeron_InputSnapshot(), true);
	AeronCompat_Update(1);
	XwInput_BeginFrame(Aeron_InputSnapshot(), true, 0);
	XwPresentation_EndFrame();
}

void XwPort_Tick(int32_t delta_us) {
	if (XwPort_ShouldQuit())
		return;
	XwRenderSnapshot_BeginTick();
	const AeronInputSnapshot* input = Aeron_InputSnapshot();
	XwInput_ApplySettings(input);
	XwPreferences_ApplyPending();
	int focused = input && input->has_focus;
	if (focused)
		ever_had_focus = 1;
	/* Wayland needs a presented frame before the window can receive its first focus. */
	int active = focused || !ever_had_focus;
	if (active && !g_windowActive)
		g_windowReactivated = 1;
	g_windowActive = active;
	char error[1024];
	if (XwPreferences_ConsumeSaveError(error, sizeof error))
		XwPort_RequestSettings();
	if (!active || settings_open || settings_requested || Aeron_DebugUiVisible()) {
		paused_frame();
		XwRenderSnapshot_Commit(focused, 1);
		return;
	}
	if (paused) {
		paused = 0;
		Aeron_AudioSetPaused(0);
	}
	XwPresentation_SetPointerSuppressed(!focused);
	int suppress =
		!focused || rebase || XwFlightLoading_Active() || (!XwFrontend_IsActive() && !XwFlightSim_IsActive());
	XwInput_BeginCaptureFrame(input, suppress != 0);
	AeronCompat_Update(suppress);
	XwInput_BeginFrame(input, suppress != 0, rebase ? 0 : delta_us);
	int rebased = rebase;
	if (rebase)
		rebase = 0;
	else
		XwTime_AdvanceHostClock(delta_us);
	if (!rebased) {
		Dos94Display_AdvancePalette(delta_us);
		imuse_advance(g_dos94Imuse, delta_us);
	}
	if (g_windowReactivated) {
		FlightDisplay_RestorePaletteAfterActivation();
		g_windowReactivated = 0;
	}
	if (XwFrontend_IsActive())
		XwLandru_ServiceAudio();
	landru_task_service_wait();
	landru_task_run_frame();
	if (xerror_Is_Landru_Error())
		XwPort_Fail("Landru reported a runtime error");
	if (landru_task_stack_empty() && !g_quitRequested && !XwStartup_Result())
		XwPort_Fail("Game startup did not complete");
	XwPresentation_EndFrame();
	XwRenderSnapshot_Commit(focused, XwFlightSim_IsPaused() || XwReplayInput_IsSavePending());
}

uint64_t XwPort_NextWakeDelayUs(void) {
	return !initialized || paused || settings_open || settings_requested ? UINT64_MAX
																		 : landru_task_next_wake_delay_us();
}

void XwPort_Shutdown(void) {
	if (!initialized)
		return;
	/* Stop accepting work; end hooks release children before startup releases the devices. */
	g_quitRequested = 1;
	XwAudioOutput_Stop();
	XwRenderSnapshot_CancelTick();
	XwPreferences_Flush();
	XwOptions_CancelSettings();
	XwFlightMode_DiscardSuspended();
	landru_task_clear_all();
	XwFlightFrame_Cancel();
	XwReplayInput_CancelSave();
	Dos94_gamesnd_Close_Pre_iMuse();
	XwLandru_Shutdown();
	XwInput_Shutdown();
	AeronWinmm_Shutdown();
	XwPresentation_Shutdown();
	Aeron_SetRelativeMouseMode(0);
	XwPresentation_SetPointerSuppressed(true);
	Aeron_AudioSetPaused(0);
	XwInput_Reset();
	XwTime_Reset();
	initialized = paused = rebase = settings_open = settings_requested = 0;
	ever_had_focus = 0;
	Aeron_LogInfo("xw.port", "Runtime stopped (exit %d)", XwPort_GetExitCode());
}
