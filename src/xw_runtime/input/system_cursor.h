#ifndef XW_RUNTIME_INPUT_SYSTEM_CURSOR_H
#define XW_RUNTIME_INPUT_SYSTEM_CURSOR_H

#include "xw/compiler.h"

#ifdef __cplusplus
extern "C" {
#endif

struct XwMousePosition;

#ifdef XW_MODERN
int XwPort_ShowSystemCursor(int show);
int XwPort_GetSystemCursorPosition(struct XwMousePosition* position);
void XwPort_UpdateMouseButtons(void);
void XwPort_SetSystemCursorDisplayCount(int count);
void XwPort_RefreshSystemCursorVisibility(void);
int XwPort_SetSystemCursorPosition(int x, int y);
#else
__declspec(dllimport) int XW_STDCALL GetCursorPos(struct XwMousePosition* position);
#define XwPort_GetSystemCursorPosition GetCursorPos
__declspec(dllimport) int XW_STDCALL ShowCursor(int show);
__declspec(dllimport) int XW_STDCALL SetCursorPos(int x, int y);
#define XwPort_ShowSystemCursor ShowCursor
#define XwPort_SetSystemCursorPosition SetCursorPos
#endif

#ifdef __cplusplus
}
#endif

#endif
