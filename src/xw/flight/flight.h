#ifndef XW_FLIGHT_FLIGHT_H
#define XW_FLIGHT_FLIGHT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <xw/compiler.h>

enum {
	FLIGHT_STARTUP_RETRY = 0,
	FLIGHT_STARTUP_READY = 1,
	FLIGHT_STARTUP_CANCEL = 2,
	FLIGHT_STARTUP_ERROR_SOUND = 13,
	FLIGHT_STANDALONE_ARGUMENT_COUNT = 3,
	FLIGHT_STANDALONE_MISSION_ARGUMENT = 2
};

extern int g_flightStartupField4DECF0;
extern const int g_flightStandaloneArgumentCount;
extern const char* g_flightStandaloneArguments[FLIGHT_STANDALONE_ARGUMENT_COUNT];
extern int g_flightTrainCourseRequested;
extern int g_flightHardware3DRequested;
extern uint32_t g_flightStartupTimeMs;
extern uint8_t g_flightConfVoiceEnabled;
extern uint8_t g_flightConfSfxEnabled;
extern uint8_t g_flightConfMusicEnabled;

enum { FLIGHT_STARTUP_MESSAGE_CAPACITY = 256 };

enum { FLIGHT_WINDOW_ICON_RESOURCE = 101, FLIGHT_WINDOW_CURSOR_RESOURCE = 104 };

extern int g_windowReactivated;
extern int g_windowActive;
extern void* g_flightMainWindowHandle;
extern void* g_hInstance;
extern int g_quitRequested;
extern void* g_flightWindowCursor;

extern char g_flightWindowTitle[13];
extern char g_flightWindowClassName[9];

/* Declarations follow ascending original IDB address. */

/* 0x4AB990 */
intptr_t XW_STDCALL Flight_WndProc(void* window, uint32_t message, uintptr_t wParam, intptr_t lParam);

/* 0x4ABE30 */
int Flight_Main(char* commandLine);

/* 0x4AC270 */
int Flight_CreateMainWindow(void* instance, int unusedArgument);

/* 0x4AC340 */
intptr_t Flight_PumpWindowMessages(void);

/* 0x4AEE60 */
int32_t Flight_ShowInsertCdPrompt(void);

/* 0x4AEF10 */
int32_t Flight_ShowMissingJoystickMessage(void);

#ifdef __cplusplus
}
#endif

#endif
