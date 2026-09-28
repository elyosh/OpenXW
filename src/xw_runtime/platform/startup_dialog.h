#ifndef XW_RUNTIME_PLATFORM_STARTUP_DIALOG_H
#define XW_RUNTIME_PLATFORM_STARTUP_DIALOG_H

#include "xw/compiler.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
	XW_STARTUP_DIALOG_OK = 0,
	XW_STARTUP_DIALOG_OK_CANCEL = 1,
	XW_STARTUP_DIALOG_DISMISSED = -1,
	XW_STARTUP_DIALOG_ERROR = 0,
	XW_STARTUP_DIALOG_RESULT_OK = 1,
	XW_STARTUP_DIALOG_RESULT_CANCEL = 2
};

#ifdef XW_MODERN
/* Startup-only native dialog; supports a null owner and the two button modes above. */
int XwPort_ShowStartupDialog(void* owner, const char* text, const char* caption, unsigned int buttons);
void XwPort_WriteStartupDiagnostic(const char* message);
#else
__declspec(dllimport) int wsprintfA(char* destination, const char* format, ...);
__declspec(dllimport) void XW_STDCALL OutputDebugStringA(const char* message);
__declspec(dllimport) int XW_STDCALL MessageBoxA(void* owner, const char* text, const char* caption,
												 unsigned int buttons);
#define XwPort_ShowStartupDialog MessageBoxA
#define XwPort_WriteStartupDiagnostic OutputDebugStringA
#endif

#ifdef __cplusplus
}
#endif

#endif
