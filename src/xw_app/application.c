/* Host lifetime and frame pacing adapted from OpenXvT. */
#include "xw_app/application.h"
#include "xw_app/settings/settings.h"
#include "xw_app/setup.h"
#include "xw_app/ui.h"
#include "xw_remaster/xw_remaster.h"
#include "xw_runtime/config/config.h"
#include "xw_runtime/input/capture.h"
#include "xw_runtime/input/input_bridge.h"
#include "xw_runtime/input/keyboard_mapping.h"
#include "xw_runtime/runtime/port.h"
#include "xw_runtime/runtime/presentation.h"
#include "xw_runtime/snapshot/render_snapshot.h"
#include "xw_runtime/storage/storage.h"
#include <aeron/log.h>
#include <stdio.h>

static uint64_t XwApplication_PresentationIntervalUs(void) {
	double rate = Aeron_PresentationRate();
	if (!(rate >= 1.0 && rate <= 1000.0))
		rate = 60.0;
	return (uint64_t)(1000000.0 / rate + 0.5);
}

static int XwApplication_FrameLoop(void) {
	static AeronInputSnapshot resized_input;
	while (!XwPort_ShouldQuit()) {
		int32_t delta_us = Aeron_BeginFrame();
		if (XwPort_ShouldQuit())
			break;
		const AeronInputSnapshot* input = Aeron_InputSnapshot();
		if (input && XwPresentation_SyncToWindow(input->window_width, input->window_height)) {
			resized_input = *input;
			XwPresentation_MapHostPointer(&resized_input);
			input = &resized_input;
		}
		int menu_opened = XwSettingsMenu_BeginFrame(input);
		int menu_open = XwSettingsMenu_Open();
		if (input && input->has_focus) {
			int debug_key = XwKeyboardMapping_Trigger(input, XW_KEYBOARD_SHORTCUT_DEBUG);
			if (!menu_open && debug_key >= 0 && !XwInput_KeyBlocked(debug_key) &&
				XwInput_SettingsShortcutAllowed()) {
				Aeron_DebugUiToggle();
				XwInput_SuppressKey(debug_key);
			}
		}
		XwPort_SetSettingsOpen(menu_open);
		XwRemaster_BeginFrame(input);
		XwPort_Tick(delta_us);
		menu_opened |= XwSettingsMenu_ConsumeRuntimeRequest();
		if (XwPort_ShouldQuit())
			break;
		XwRemaster_Frame(delta_us);
		if (XwPort_ShouldQuit())
			break;
		XwPresentation_SubmitCursor();
		if (XwSettingsMenu_Open() && !menu_opened)
			XwSettingsMenu_Frame(input, (float)delta_us * 1e-6f);
		if (!Aeron_Present()) {
			Aeron_RequestFatalRendererError("Frame presentation");
			break;
		}
		uint64_t wake = XwApplication_PresentationIntervalUs();
		uint64_t task = XwPort_NextWakeDelayUs();
		if (task < wake)
			wake = task;
		Aeron_WaitForNextFrame(wake);
	}
	return XwPort_GetExitCode();
}

static int XwApplication_ApplyPresentation(char* error, size_t capacity) {
	const XwPresentationSettings* settings = &XwConfig_Settings()->presentation;
	if (!Aeron_SetPresentationVsyncDivisor(settings->vsync_divisor) ||
		!Aeron_SetOutputHdr(settings->hdr_output)) {
		snprintf(error, capacity, "Cannot apply host presentation settings.");
		return 0;
	}
#ifndef __APPLE__
	Aeron_SetOutputSdrContentGamma(settings->sdr_gamma < 0 ? 2.2f : settings->sdr_gamma);
	Aeron_SetOutputPaperWhiteNits(settings->paper_white_nits);
#endif
	return 1;
}

int XwApplication_Run(const XwLaunchOptions* options) {
#ifdef AERON_DEBUG_UI
	XwKeyboardMapping_SetPolicy(true);
#else
	XwKeyboardMapping_SetPolicy(false);
#endif
	AeronConfig config;
	XwAppUi ui = { 0 };
	char error[1024] = { 0 };
	int exit_code = 1;
	if (options->check_installation) {
		char resource_root[XW_PATH_CAPACITY];
		if (!XwHostConfig_ResolveResourceRoot(options, resource_root, sizeof resource_root)) {
			fprintf(stderr, "OpenXW: cannot resolve application resources.\n");
			return 1;
		}
		AeronVfs* vfs = AeronVfs_Create(&(AeronVfsConfig) {
			.org_name = "TotallyOpen", .app_name = "OpenXW", .resource_root = resource_root });
		XwStorage_Bind(vfs);
		int success = vfs && XwSetup_Run(options, NULL, error, sizeof error) == XW_SETUP_SUCCESS;
		if (!success)
			fprintf(stderr, "OpenXW: %s\n", error[0] ? error : "cannot initialize storage");
		else
			fprintf(stdout, "XW93: %s\nXW94: %s\nXW98: %s\nCD music: %s\n",
					XwSetup_InstallationFor(XW_GAME_VERSION_93), XwSetup_InstallationFor(XW_GAME_VERSION_94),
					XwSetup_InstallationFor(XW_GAME_VERSION_98),
					XwSetup_CdMusicAvailable() ? "available"
											   : "unavailable (MUSIC tracks 2, 3 and 7 required)");
		XwSetup_Shutdown();
		XwConfig_Shutdown();
		XwStorage_Bind(NULL);
		AeronVfs_Destroy(vfs);
		return success ? 0 : 1;
	}
	XwHostConfig_InitAeron(options, &config);
	if (!Aeron_Init(&config)) {
		Aeron_Shutdown();
		return 1;
	}
	XwStorage_Bind(Aeron_GetVfs());
	XwSetupResult result = XwSetup_Run(options, &ui, error, sizeof error);
	if (result != XW_SETUP_SUCCESS) {
		if (result == XW_SETUP_CANCELLED)
			exit_code = 0;
		else
			Aeron_LogError("xw.setup", "%s", error);
		goto cleanup;
	}
	if (!XwApplication_ApplyPresentation(error, sizeof error)) {
		Aeron_LogError("xw.app", "%s", error);
		goto cleanup;
	}
	XwRenderSnapshot_Init();
	if (!XwRemaster_Init())
		goto cleanup;
	XwPort_SetSkipIntro(options->skip_intro || XwConfig_Settings()->skip_intro);
	if (!XwPort_Init(XwSetup_CdMusicAvailable())) {
		exit_code = XwPort_GetExitCode();
		goto cleanup;
	}
	if (!XwSettingsMenu_Init(&ui, error, sizeof error)) {
		Aeron_LogError("xw.settings", "%s", error);
		goto cleanup;
	}
	exit_code = XwApplication_FrameLoop();
cleanup:
	XwSettingsMenu_FlushForExit();
	XwSettingsMenu_Shutdown();
	XwPort_Shutdown();
	XwRemaster_Shutdown();
	XwRenderSnapshot_Shutdown();
	if (XwPort_GetExitCode())
		exit_code = XwPort_GetExitCode();
	XwSetup_Shutdown();
	XwAppUi_Shutdown(&ui);
	XwConfig_Shutdown();
	XwStorage_Bind(NULL);
	Aeron_Shutdown();
	return exit_code;
}
