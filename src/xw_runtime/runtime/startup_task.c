#include "xw_runtime/runtime/startup_task.h"
#include "xw/assets/file.h"
#include "xw/flight/fediskio.h"
#include "xw/flight/flight.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/flight_input.h"
#include "xw/flight/shell_flight.h"
#include "xw/flight/xw.h"
#include "xw/frontend/shell.h"
#include "xw/input/dinput.h"
#include "xw/input/joystick.h"
#include "xw/input/win_mouse.h"
#include "xw/render/renderer.h"
#include "xw/render/std3d.h"
#include "xw/util/testdrv.h"
#include "xw_runtime/audio/media_device.h"
#include "xw_runtime/input/system_cursor.h"
#include "xw_runtime/platform/classic_surfaces.h"
#include "xw_runtime/timing/host_clock.h"
#include <landru/task.h>
#include <stdlib.h>
#include <string.h>

typedef struct StartupState {
	int shellStarted;
	int displayStarted, inputStarted, soundStarted;
} StartupState;

static int startupResult;

static LandruTaskStepResult startup_step(void* self) {
	StartupState* state = self;
	if (!state->shellStarted) {
		state->shellStarted = 1;
		ShellFlight_Run(g_flightStandaloneArgumentCount, g_flightStandaloneArguments);
		return LANDRU_TASK_STEP_YIELD;
	}
	startupResult = 1;
	return LANDRU_TASK_STEP_DONE;
}

static void startup_end(void* self) {
	StartupState* state = self;
	if (state->soundStarted) {
		XwMediaDevice_Shutdown();
		state->soundStarted = 0;
	}
	if (state->inputStarted) {
		DInput_Shutdown();
		state->inputStarted = 0;
	}
	if (state->displayStarted) {
		XwDisplay_BeginSurfaceChange();
		if (g_useHardware3D) {
			std3D_DetachAndReleaseZBufferSurface();
			std3D_Close();
			std3D_Shutdown();
		}
		IDirectDrawSurface** surfaces[] = { &g_flightBackBuffer, &g_flightOffscreenSurface,
											&g_flightAuxiliarySurface, &g_flightPrimarySurface };
		for (unsigned i = 0; i < sizeof surfaces / sizeof surfaces[0]; ++i) {
			if (*surfaces[i]) {
				(*surfaces[i])->lpVtbl->Release(*surfaces[i]);
				*surfaces[i] = NULL;
			}
		}
		if (g_ddPalette) {
			g_ddPalette->lpVtbl->Release(g_ddPalette);
			g_ddPalette = NULL;
		}
		if (g_flightDirectDraw) {
			g_flightDirectDraw->lpVtbl->Release(g_flightDirectDraw);
			g_flightDirectDraw = NULL;
		}
		state->displayStarted = 0;
	}
	g_flightMainWindowHandle = NULL;
	g_flightRenderSurface = NULL;
	g_useHardware3D = 0;
}

static const LandruTaskVtable startup_vtable = { startup_step, startup_end, NULL, NULL };

int XwStartup_Begin(char* commandLine) {
	XwFile* flickerFile;
	int axisX, axisY, axisZ;
	unsigned int buttons;
	startupResult = 0;
	if (!commandLine)
		return 0;
	StartupState* state = landru_task_push(&startup_vtable);
	if (!state)
		return 0;
	g_installDriveLetter = testdrv_Get_XWing_CD_Drive();
	if (g_installDriveLetter == 0)
		goto failed;
	Joystick_PollRawAxes(0, &axisX, &axisY, &axisZ, &buttons);
	g_flightStartupField4DECF0 = 0;
	flickerFile = XwStorage_OpenText("flicker.txt", "r");
	if (flickerFile != NULL) {
		XwFile_Close(flickerFile);
		g_flightConfFlicker = 0;
	} else {
		g_flightConfFlicker = 1;
	}
	g_flightTrainCourseRequested = strstr(commandLine, "traincourse") != NULL;
	if (strstr(commandLine, "nodinput") != NULL) {
		g_flightConfDirectInput = 0;
	} else {
		(void)strstr(commandLine, "dinput");
		g_flightConfDirectInput = 1;
	}
	if (strstr(commandLine, "nosfx") != NULL) {
		g_flightConfSfxEnabled = 0;
	} else {
		(void)strstr(commandLine, "sfx");
		g_flightConfSfxEnabled = 1;
	}
	if (strstr(commandLine, "nomusic") != NULL) {
		g_flightConfMusicEnabled = 0;
	} else {
		(void)strstr(commandLine, "music");
		g_flightConfMusicEnabled = 1;
	}
	if (strstr(commandLine, "novoice") != NULL) {
		g_flightConfVoiceEnabled = 0;
	} else {
		(void)strstr(commandLine, "voice");
		g_flightConfVoiceEnabled = 1;
	}
	if (strstr(commandLine, "nofullscreen") != NULL) {
		g_flightFullscreen = 0;
	} else if (strstr(commandLine, "fullscreen") != NULL) {
		g_flightFullscreen = 1;
	}
	if (strstr(commandLine, "nopageflip") != NULL) {
		g_flightPageFlip = 0;
	} else if (strstr(commandLine, "pageflip") != NULL) {
		g_flightPageFlip = 1;
	}
	if (strstr(commandLine, "softwarecursor") != NULL) {
		g_SoftwareCursor = 1;
		XwPort_SetSystemCursorDisplayCount(-1);
	} else {
		g_SoftwareCursor = 0;
	}

	if (!Flight_CreateMainWindow(g_hInstance, 0))
		goto failed;
	g_flightHardware3DRequested = g_useHardware3D;
	state->displayStarted = 1;
	if (!FlightDisplay_Init())
		goto failed;
	if (g_flightConfDirectInput) {
		state->inputStarted = 1;
		if (!DInput_Init())
			goto failed;
	}
	g_flightStartupTimeMs = timeGetTime();
	state->soundStarted = 1;
	if (!XwMediaDevice_Init(g_flightMainWindowHandle))
		goto failed;
	strcpy(g_currentMissionFile, g_flightStandaloneArguments[FLIGHT_STANDALONE_MISSION_ARGUMENT]);
	return 1;
failed:
	landru_task_pop();
	return 0;
}

int XwStartup_Result(void) { return startupResult; }
