#include "xw/flight/flight.h"

#include "xw/audio/frontend_audio.h"
#include "xw/audio/sound.h"
#include "xw/flight/shell_flight.h"
#include "xw/flight/xw.h"
#include "xw/frontend/shell.h"
#include "xw/input/dinput.h"
#include "xw/input/joystick.h"
#include "xw/render/renderer.h"
#include "xw/render/std3d.h"
#include "xw/util/testdrv.h"
#include "xw_runtime/timing/host_clock.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/startup_task.h"
#endif
#include "xw/assets/file.h"
#include "xw/flight/fediskio.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/flight_input.h"
#include "xw/input/win_mouse.h"
#include "xw/landru_config.h"
#include "xw/render/flight_palette.h"
#include "xw/render/flight_screenshot.h"
#include "xw/util/landru_display.h"
#include "xw/util/shared.h"
#include "xw_runtime/input/system_cursor.h"
#include "xw_runtime/platform/main_window.h"

#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/io.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4DECF0
int g_flightStartupField4DECF0 = 1;

// GLOBAL: XW 0x4DED3C
const int g_flightStandaloneArgumentCount = 2;

// GLOBAL: XW 0x4DED60
const char* g_flightStandaloneArguments[FLIGHT_STANDALONE_ARGUMENT_COUNT] = { "xtie", "/trebla", "test.tie" };

// GLOBAL: XW 0x4DEE1C
char g_flightWindowTitle[13] = "X-Wing Win95";

// GLOBAL: XW 0x4DEE2C
char g_flightWindowClassName[9] = "X-WING95";

// GLOBAL: XW 0x566880
int g_windowReactivated = 0;

// GLOBAL: XW 0x566884
int g_flightTrainCourseRequested = 0;

// GLOBAL: XW 0x5668A0
int g_flightHardware3DRequested = 0;

// GLOBAL: XW 0x5678F0
int g_windowActive = 0;

// GLOBAL: XW 0x5678F8
void* g_flightMainWindowHandle = NULL;

// GLOBAL: XW 0x568140
void* g_hInstance = NULL;

// GLOBAL: XW 0x568148
uint32_t g_flightStartupTimeMs = 0;

// GLOBAL: XW 0x568260
int g_quitRequested = 0;

// GLOBAL: XW 0x56869C
void* g_flightWindowCursor = NULL;

// GLOBAL: XW 0x5BEA00
uint8_t g_flightConfVoiceEnabled = 0;

// GLOBAL: XW 0x5BECA4
uint8_t g_flightConfSfxEnabled = 0;

// GLOBAL: XW 0x5BECB0
uint8_t g_flightConfMusicEnabled = 0;

// FUNCTION: XW 0x4AB990
intptr_t XW_STDCALL Flight_WndProc(void* window, uint32_t message, uintptr_t wParam, intptr_t lParam) {
#ifdef XW_MODERN
	/* Aeron owns event dispatch; this legacy callback is never registered. */
	(void)window;
	(void)message;
	(void)wParam;
	(void)lParam;
	return 0;
#else
	uint16_t translatedChars[XW_WIN32_TRANSLATED_CHAR_CAPACITY];
	XwWin32Paint paint;
	uint8_t keyboardState[FLIGHT_VIRTUAL_KEY_COUNT];
	if (g_frontendDisplayWndProcMode != 0) {
		switch (message) {
			case XW_WIN32_WM_CREATE:
				break;
			case XW_WIN32_WM_DESTROY:
				nullsub_SharedNoOp();
				if (g_windowActive != 0)
					ReleaseCapture();
				PostQuitMessage(0);
				return 0;
			case XW_WIN32_WM_PAINT:
				BeginPaint(window, &paint);
				EndPaint(window, &paint);
				return 1;
			case XW_WIN32_WM_ACTIVATEAPP: {
				int wasActive = g_windowActive;
				g_windowActive = wParam;
				if (wParam == 0) {
					if (wasActive == 1) {
						ReleaseCapture();
						if (g_frontendDisplayWndProcMode == 1) {
							FrontendAudio_Pause();
							LandruDisplay_SaveCanvasBackground();
						}
						if (g_SoftwareCursor == 0) {
							while (ShowCursor(1) < 0) {
							}
						}
					}
				} else {
					g_windowReactivated = 1;
					SetCapture(window);
					if (g_frontendDisplayWndProcMode == 1) {
						FrontendAudio_Resume();
						LandruDisplay_RestoreCanvasBackground();
						FlightDisplay_RestorePaletteAfterActivation();
					}
					xio_Set_Mouse_Position(WIN_MOUSE_DEFAULT_MAX_X / 2, WIN_MOUSE_DEFAULT_MAX_Y / 2);
					if (g_SoftwareCursor == 0) {
						if (xcursor_Get_Display_Count() >= 0) {
							while (ShowCursor(1) < 0) {
							}
						} else {
							while (ShowCursor(0) >= 0) {
							}
						}
					}
				}
				break;
			}
			case XW_WIN32_WM_SETCURSOR:
				SetCursor(g_flightWindowCursor);
				return 1;
			case XW_WIN32_WM_KEYDOWN:
				if (wParam < FLIGHT_VIRTUAL_KEY_COUNT)
					g_windowVirtualKeyDown[wParam] = 1;
				break;
			case XW_WIN32_WM_KEYUP: {
				unsigned int scanCode;
				uint8_t scanCodeByte;
				if (wParam < FLIGHT_VIRTUAL_KEY_COUNT)
					g_windowVirtualKeyDown[wParam] = 0;
				g_keyReady = 1;
				g_lastReleasedVirtualKey = wParam;
				GetKeyboardState(keyboardState);
				scanCode = MapVirtualKeyA(g_lastReleasedVirtualKey, XW_WIN32_MAP_VK_TO_SCAN);
				scanCodeByte = scanCode;
				if (ToAscii(g_lastReleasedVirtualKey, scanCode, keyboardState, translatedChars, 0) == 0)
					g_lastKeyCode = scanCodeByte + FLIGHT_EXTENDED_KEY_BASE;
				else
					g_lastKeyCode = translatedChars[0];
				break;
			}
			case XW_WIN32_WM_SYSKEYUP:
				if (wParam == XW_WIN32_VK_F4) {
					if (g_windowActive != 0)
						ReleaseCapture();
					PostQuitMessage(0);
					return 0;
				}
				if (wParam == 'O')
					FlightScreenshot_Capture();
				break;
			case XW_WIN32_WM_MOUSEMOVE: {
				int clampCursor = 0;
				int mouseX = (uint16_t)lParam;
				int mouseY = (uint32_t)lParam >> 16;
				if (mouseX > WIN_MOUSE_DEFAULT_MAX_X) {
					mouseX = WIN_MOUSE_DEFAULT_MAX_X;
					clampCursor = 1;
				}
				if (mouseY > WIN_MOUSE_DEFAULT_MAX_Y) {
					mouseY = WIN_MOUSE_DEFAULT_MAX_Y;
					clampCursor = 1;
				}
				if (clampCursor != 0)
					xio_Set_Mouse_Position(mouseX, mouseY);
				break;
			}
			case XW_WIN32_WM_LBUTTONDOWN:
				g_winMouseButtonDown[WIN_MOUSE_BUTTON_LEFT] = 1;
				++g_winMouseButtonPressed[WIN_MOUSE_BUTTON_LEFT];
				break;
			case XW_WIN32_WM_LBUTTONUP:
				g_winMouseButtonDown[WIN_MOUSE_BUTTON_LEFT] = 0;
				++g_winMouseButtonReleased[WIN_MOUSE_BUTTON_LEFT];
				break;
			case XW_WIN32_WM_RBUTTONDOWN:
				g_winMouseButtonDown[WIN_MOUSE_BUTTON_RIGHT] = 1;
				++g_winMouseButtonPressed[WIN_MOUSE_BUTTON_RIGHT];
				break;
			case XW_WIN32_WM_RBUTTONUP:
				g_winMouseButtonDown[WIN_MOUSE_BUTTON_RIGHT] = 0;
				++g_winMouseButtonReleased[WIN_MOUSE_BUTTON_RIGHT];
				break;
			case XW_WIN32_WM_MBUTTONDOWN:
				g_winMouseButtonDown[WIN_MOUSE_BUTTON_MIDDLE] = 1;
				++g_winMouseButtonPressed[WIN_MOUSE_BUTTON_MIDDLE];
				break;
			case XW_WIN32_WM_MBUTTONUP:
				g_winMouseButtonDown[WIN_MOUSE_BUTTON_MIDDLE] = 0;
				++g_winMouseButtonReleased[WIN_MOUSE_BUTTON_MIDDLE];
				break;
			default:
				break;
		}
		return DefWindowProcA(window, message, wParam, lParam);
	}
	if (message == XW_WIN32_WM_QUERYNEWPALETTE)
		FlightPalette_ResetIf8Bit();
	if (message == XW_WIN32_WM_PAINT)
		return DefWindowProcA(window, message, wParam, lParam);
	if (message == XW_WIN32_WM_SYSKEYUP && wParam == 'O')
		FlightScreenshot_Capture();
	return 0;
#endif
}

// FUNCTION: XW 0x4ABE30
int Flight_Main(char* commandLine) {
#ifdef XW_MODERN
	return XwStartup_Begin(commandLine);
#else
	int startupStatus;
	int result;
	XwFile* flickerFile;
	unsigned int buttons;
	int axisZ;
	int axisY;
	int axisX;

	do {
		g_installDriveLetter = testdrv_Get_XWing_CD_Drive();
		if (g_installDriveLetter == 0) {
			if (Flight_ShowInsertCdPrompt() != 0)
				startupStatus = FLIGHT_STARTUP_RETRY;
			else
				startupStatus = FLIGHT_STARTUP_CANCEL;
		} else {
			startupStatus = FLIGHT_STARTUP_READY;
		}
	} while (startupStatus == FLIGHT_STARTUP_RETRY);
	Joystick_PollRawAxes(0, &axisX, &axisY, &axisZ, &buttons);
	if (Joystick_GetButtonCount() == 0) {
		Flight_ShowMissingJoystickMessage();
		startupStatus = FLIGHT_STARTUP_CANCEL;
	}
	if (startupStatus == FLIGHT_STARTUP_CANCEL)
		return 0;
	g_flightStartupField4DECF0 = 0;
	if (commandLine == NULL)
		return 0;
	flickerFile = File_RawOpenText("flicker.txt", "r");
	if (flickerFile != NULL) {
		File_RawClose(flickerFile);
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
		while (ShowCursor(0) >= 0)
			;
	} else {
		g_SoftwareCursor = 0;
	}
	result = Flight_CreateMainWindow(g_hInstance, 0);
	if (result == 0)
		return result;
	g_flightHardware3DRequested = g_useHardware3D;
	result = FlightDisplay_Init();
	if (result == 0)
		return result;
	if (g_flightConfDirectInput != 0 && DInput_Init() == 0)
		g_flightConfDirectInput = 0;
	g_flightStartupTimeMs = timeGetTime();
	if (!Sound_Init_Sound_Engine(g_flightMainWindowHandle)) {
		DestroyWindow(g_flightMainWindowHandle);
		FlightDisplay_CleanupAndReportError(FLIGHT_STARTUP_ERROR_SOUND);
		return 0;
	}

	strcpy(g_currentMissionFile, g_flightStandaloneArguments[FLIGHT_STANDALONE_MISSION_ARGUMENT]);
	ShellFlight_Run(g_flightStandaloneArgumentCount, g_flightStandaloneArguments);
	Sound_Shutdown_Sound_Engine();
	if (g_flightConfDirectInput != 0)
		DInput_Shutdown();
	if (g_useHardware3D != 0) {
		std3D_DetachAndReleaseZBufferSurface();
		std3D_Close();
		std3D_Shutdown();
	}
	if (g_flightDirectDraw != NULL) {
		FlightDisplay_ClearSurface(g_flightPrimarySurface);
		FlightDisplay_ClearSurface(g_flightBackBuffer);
		FlightDisplay_ClearSurface(g_flightOffscreenSurface);
		FlightDisplay_ClearSurface(g_flightAuxiliarySurface);
		if (g_flightPrimarySurface != NULL) {
			g_flightPrimarySurface->lpVtbl->Release(g_flightPrimarySurface);
			g_flightPrimarySurface = NULL;
			if (g_flightFullscreen == 0)
				g_flightBackBuffer->lpVtbl->Release(g_flightBackBuffer);
			g_flightBackBuffer = NULL;
			g_flightRenderSurface = NULL;
		}
		if (g_ddPalette != NULL) {
			g_ddPalette->lpVtbl->Release(g_ddPalette);
			g_ddPalette = NULL;
		}
		if (g_flightOffscreenSurface != NULL) {
			g_flightOffscreenSurface->lpVtbl->Release(g_flightOffscreenSurface);
			g_flightOffscreenSurface = NULL;
		}
		if (g_flightAuxiliarySurface != NULL) {
			g_flightAuxiliarySurface->lpVtbl->Release(g_flightAuxiliarySurface);
			g_flightAuxiliarySurface = NULL;
		}
		g_flightDirectDraw->lpVtbl->SetCooperativeLevel(g_flightDirectDraw, g_flightMainWindowHandle,
														DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE |
															DDSCL_ALLOWMODEX);
		g_flightDirectDraw->lpVtbl->FlipToGDISurface(g_flightDirectDraw);
		g_flightDirectDraw->lpVtbl->RestoreDisplayMode(g_flightDirectDraw);
		g_flightDirectDraw->lpVtbl->Release(g_flightDirectDraw);
		g_flightDirectDraw = NULL;
		if (g_flightMainWindowHandle != NULL) {
			DestroyWindow(g_flightMainWindowHandle);
			g_flightMainWindowHandle = NULL;
		}
	}
	g_useHardware3D = 0;
	return 1;
#endif
}

// FUNCTION: XW 0x4AC270
int Flight_CreateMainWindow(void* instance, int unusedArgument) {
#ifdef XW_MODERN
	(void)instance;
	(void)unusedArgument;
	g_flightMainWindowHandle = XwPort_ActivateMainWindow();
	return g_flightMainWindowHandle != NULL;
#else
	XwWin32WindowClass windowClass;
	unsigned int classAtom;
	(void)unusedArgument;
	windowClass.style = XW_WIN32_CLASS_DOUBLE_CLICKS;
	windowClass.lpfnWndProc = Flight_WndProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = instance;
	windowClass.hIcon = LoadIconA(instance, XwWin32_ResourceName(FLIGHT_WINDOW_ICON_RESOURCE));
	g_flightWindowCursor = LoadCursorA(instance, XwWin32_ResourceName(FLIGHT_WINDOW_CURSOR_RESOURCE));
	windowClass.hCursor = g_flightWindowCursor;
	windowClass.hbrBackground = NULL;
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = g_flightWindowClassName;
	classAtom = RegisterClassA(&windowClass);
	if (classAtom == 0)
		return 0;
	g_flightMainWindowHandle =
		CreateWindowExA(0, g_flightWindowClassName, g_flightWindowTitle,
						XW_WIN32_WINDOW_POPUP | XW_WIN32_WINDOW_VISIBLE | XW_WIN32_WINDOW_SYSTEM_MENU, 0, 0,
						GetSystemMetrics(XW_WIN32_SCREEN_WIDTH), GetSystemMetrics(XW_WIN32_SCREEN_HEIGHT),
						NULL, NULL, instance, NULL);
	if (g_flightMainWindowHandle == NULL)
		return 0;
	UpdateWindow(g_flightMainWindowHandle);
	SetFocus(g_flightMainWindowHandle);
	return 1;
#endif
}

// FUNCTION: XW 0x4AC340
intptr_t Flight_PumpWindowMessages(void) {
#ifdef XW_MODERN
	if (g_quitRequested != 0)
		return g_quitRequested;
	if (g_SoftwareCursor == 0)
		XwPort_SetSystemCursorDisplayCount(xcursor_Get_Display_Count());
	if (XwPort_WindowQuitRequested()) {
		g_quitRequested = 1;
		xerror_Set_Landru_Exit(0);
	}
	return 0;
#else
	XwWin32Message message;
	intptr_t result = g_quitRequested;
	if (result == 0) {
		if (g_SoftwareCursor == 0) {
			int desiredCount = xcursor_Get_Display_Count();
			int cursorCount;
			if (desiredCount < 0)
				cursorCount = ShowCursor(0);
			else
				cursorCount = ShowCursor(1);
			while (cursorCount != desiredCount) {
				if (cursorCount < desiredCount)
					cursorCount = ShowCursor(1);
				else
					cursorCount = ShowCursor(0);
			}
			if (g_windowReactivated != 0)
				SetCursor(g_flightWindowCursor);
		}
		if (g_frontendDisplayWndProcMode == 0) {
			if (GetForegroundWindow() != g_flightMainWindowHandle) {
				SetForegroundWindow(g_flightMainWindowHandle);
				FlightPalette_ResetIf8Bit();
			}
		}
		if (g_frontendDisplayWndProcMode == 0) {
			result = PeekMessageA(&message, NULL, 0, 0, XW_WIN32_MESSAGE_REMOVE);
			if (result != 0) {
				result = message.message;
				if (message.message != XW_WIN32_WM_ACTIVATEAPP && message.message != XW_WIN32_WM_KILLFOCUS &&
					message.message != XW_WIN32_WM_ACTIVATE && message.message != XW_WIN32_WM_CANCELMODE &&
					message.message != XW_WIN32_WM_NCACTIVATE) {
					TranslateMessage(&message);
					return DispatchMessageA(&message);
				}
			}
		} else {
			result = PeekMessageA(&message, NULL, 0, 0, XW_WIN32_MESSAGE_NO_REMOVE);
			if (result != 0) {
				if (!GetMessageA(&message, NULL, 0, 0)) {
					g_quitRequested = 1;
					xerror_Set_Landru_Exit(0);
				}
				TranslateMessage(&message);
				return DispatchMessageA(&message);
			}
		}
	}
	return result;
#endif
}

// FUNCTION: XW 0x4AEE60
int32_t Flight_ShowInsertCdPrompt(void) {
#ifdef XW_MODERN
	char caption[FLIGHT_STARTUP_MESSAGE_CAPACITY] = "Could not find X-Wing CD.";
	char message[FLIGHT_STARTUP_MESSAGE_CAPACITY] = "Please insert X-Wing CD into your CD-ROM drive.";
#else
	char caption[FLIGHT_STARTUP_MESSAGE_CAPACITY];
	char message[FLIGHT_STARTUP_MESSAGE_CAPACITY];
#endif
	XwFile* errorFile = File_RawOpenText("xwing95err.txt", "r");
	if (errorFile != NULL) {
		File_RawReadTextLine(message, sizeof(message), errorFile);
#ifdef XW_MODERN
		if (File_RawReadTextLine(caption, sizeof(caption), errorFile) != NULL && caption[0] != '\0') {
			caption[strlen(caption) - 1] = '\0';
		}
#else
		File_RawReadTextLine(caption, sizeof(caption), errorFile);
		caption[strlen(caption) - 1] = '\0';
#endif
		File_RawClose(errorFile);
	} else {
		strcpy(message, "Please insert X-Wing CD into your CD-ROM drive.");
		strcpy(caption, "Could not find X-Wing CD.");
	}
	return FlightDisplay_ShowStartupMessageBox(message, caption, 1);
}

// FUNCTION: XW 0x4AEF10
int32_t Flight_ShowMissingJoystickMessage(void) {
#ifdef XW_MODERN
	char caption[FLIGHT_STARTUP_MESSAGE_CAPACITY] = "You need a joystick to play X-Wing CD.";
	char message[FLIGHT_STARTUP_MESSAGE_CAPACITY] =
		"Please check your joystick and make sure it's plugged in.";
#else
	char caption[FLIGHT_STARTUP_MESSAGE_CAPACITY];
	char message[FLIGHT_STARTUP_MESSAGE_CAPACITY];
#endif
	XwFile* errorFile = File_RawOpenText("xwing95err.txt", "r");
	if (errorFile != NULL) {
		File_RawReadTextLine(message, sizeof(message), errorFile);
		File_RawReadTextLine(caption, sizeof(caption), errorFile);
		File_RawReadTextLine(message, sizeof(message), errorFile);
		File_RawReadTextLine(caption, sizeof(caption), errorFile);
#ifdef XW_MODERN
		if (caption[0] != '\0') {
			caption[strlen(caption) - 1] = '\0';
		}
#else
		caption[strlen(caption) - 1] = '\0';
#endif
		File_RawClose(errorFile);
	} else {
		strcpy(message, "Please check your joystick and make sure it's plugged in.");
		strcpy(caption, "You need a joystick to play X-Wing CD.");
	}
	return FlightDisplay_ShowStartupMessageBox(message, caption, 0);
}
