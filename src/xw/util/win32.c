#include "xw/util/win32.h"

#include "xw/flight/flight.h"

#include "xw/util/shared.h"

#include <locale.h>

#ifndef XW_MODERN
__declspec(dllimport) void* XW_STDCALL FindWindowA(const char* className, const char* windowName);
__declspec(dllimport) int XW_STDCALL ShowWindowAsync(void* window, int command);
#endif

// FUNCTION: XW 0x4ABDF0
int XW_STDCALL WinMain(void* hInstance, void* hPrevInstance, char* lpCmdLine, int nShowCmd) {
	(void)hPrevInstance;
	(void)nShowCmd;
	if (Win32_CheckSingleInstance() != 0)
		return 0;
	setlocale(LC_ALL, g_sharedEmptyString);
	g_hInstance = hInstance;
	return Flight_Main(lpCmdLine);
}

// FUNCTION: XW 0x4AEFF0
int Win32_CheckSingleInstance(void) {
#ifdef XW_MODERN
	/* Follow OpenXvT: the modern host permits multiple instances. */
	return 0;
#else
	void* window = FindWindowA(g_flightWindowClassName, g_flightWindowTitle);
	if (window != NULL) {
		ShowWindowAsync(window, 9);
		return 1;
	}
	return 0;
#endif
}
