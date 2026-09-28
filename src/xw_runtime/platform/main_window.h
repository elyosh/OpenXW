#ifndef XW_RUNTIME_PLATFORM_MAIN_WINDOW_H
#define XW_RUNTIME_PLATFORM_MAIN_WINDOW_H

#include "xw/compiler.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef XW_MODERN
/* Returns a port-owned identity for the existing Aeron window, or NULL. */
void* XwPort_ActivateMainWindow(void);
int XwPort_WindowQuitRequested(void);
#else
typedef struct XwWin32Message {
	void* window;
	uint32_t message;
	uintptr_t wParam;
	intptr_t lParam;
	uint32_t time;
	int32_t pointX;
	int32_t pointY;
} XwWin32Message;

enum {
	XW_WIN32_MESSAGE_NO_REMOVE = 0,
	XW_WIN32_MESSAGE_REMOVE = 1,
	XW_WIN32_WM_CREATE = 0x01,
	XW_WIN32_WM_DESTROY = 0x02,
	XW_WIN32_WM_PAINT = 0x0F,
	XW_WIN32_WM_ACTIVATEAPP = 0x1C,
	XW_WIN32_WM_SETCURSOR = 0x20,
	XW_WIN32_WM_KEYDOWN = 0x100,
	XW_WIN32_WM_KEYUP = 0x101,
	XW_WIN32_WM_SYSKEYUP = 0x105,
	XW_WIN32_WM_MOUSEMOVE = 0x200,
	XW_WIN32_WM_LBUTTONDOWN = 0x201,
	XW_WIN32_WM_LBUTTONUP = 0x202,
	XW_WIN32_WM_RBUTTONDOWN = 0x204,
	XW_WIN32_WM_RBUTTONUP = 0x205,
	XW_WIN32_WM_MBUTTONDOWN = 0x207,
	XW_WIN32_WM_MBUTTONUP = 0x208,
	XW_WIN32_WM_QUERYNEWPALETTE = 0x311,
	XW_WIN32_VK_F4 = 0x73,
	XW_WIN32_MAP_VK_TO_SCAN = 0,
	XW_WIN32_TRANSLATED_CHAR_CAPACITY = 8,
	XW_WIN32_PAINT_RESERVED_BYTES = 32,
	XW_WIN32_WM_KILLFOCUS = 0x08,
	XW_WIN32_WM_ACTIVATE = 0x06,
	XW_WIN32_WM_CANCELMODE = 0x1F,
	XW_WIN32_WM_NCACTIVATE = 0x86
};

__declspec(dllimport) int XW_STDCALL PeekMessageA(XwWin32Message* message, void* window, unsigned int minimum,
												  unsigned int maximum, unsigned int remove);
__declspec(dllimport) int XW_STDCALL GetMessageA(XwWin32Message* message, void* window, unsigned int minimum,
												 unsigned int maximum);
__declspec(dllimport) int XW_STDCALL TranslateMessage(const XwWin32Message* message);
__declspec(dllimport) intptr_t XW_STDCALL DispatchMessageA(const XwWin32Message* message);

typedef struct XwWin32Paint {
	void* dc;
	int erase;
	int32_t left;
	int32_t top;
	int32_t right;
	int32_t bottom;
	int restore;
	int incrementalUpdate;
	uint8_t reserved[XW_WIN32_PAINT_RESERVED_BYTES];
} XwWin32Paint;

__declspec(dllimport) void* XW_STDCALL BeginPaint(void* window, XwWin32Paint* paint);
__declspec(dllimport) int XW_STDCALL EndPaint(void* window, const XwWin32Paint* paint);
__declspec(dllimport) void* XW_STDCALL SetCapture(void* window);
__declspec(dllimport) int XW_STDCALL ReleaseCapture(void);
__declspec(dllimport) void XW_STDCALL PostQuitMessage(int exitCode);
__declspec(dllimport) intptr_t XW_STDCALL DefWindowProcA(void* window, uint32_t message, uintptr_t wParam,
														 intptr_t lParam);
__declspec(dllimport) int XW_STDCALL GetKeyboardState(uint8_t* state);
__declspec(dllimport) unsigned int XW_STDCALL MapVirtualKeyA(unsigned int key, unsigned int mapType);
__declspec(dllimport) int XW_STDCALL ToAscii(unsigned int key, unsigned int scanCode, const uint8_t* state,
											 uint16_t* characters, unsigned int flags);

typedef struct XwWin32WindowClass {
	unsigned int style;
	intptr_t(XW_STDCALL* lpfnWndProc)(void*, uint32_t, uintptr_t, intptr_t);
	int cbClsExtra;
	int cbWndExtra;
	void* hInstance;
	void* hIcon;
	void* hCursor;
	void* hbrBackground;
	const char* lpszMenuName;
	const char* lpszClassName;
} XwWin32WindowClass;

enum {
	XW_WIN32_CLASS_DOUBLE_CLICKS = 0x0008,
	XW_WIN32_WINDOW_POPUP = 0x80000000u,
	XW_WIN32_WINDOW_VISIBLE = 0x10000000,
	XW_WIN32_WINDOW_SYSTEM_MENU = 0x00080000,
	XW_WIN32_SCREEN_WIDTH = 0,
	XW_WIN32_SCREEN_HEIGHT = 1
};

static __inline const char* XwWin32_ResourceName(unsigned int resourceId) {
	return (const char*)(uintptr_t)resourceId;
}

__declspec(dllimport) void* XW_STDCALL LoadIconA(void* instance, const char* name);
__declspec(dllimport) void* XW_STDCALL LoadCursorA(void* instance, const char* name);
__declspec(dllimport) uint16_t XW_STDCALL RegisterClassA(const XwWin32WindowClass* windowClass);
__declspec(dllimport) int XW_STDCALL GetSystemMetrics(int index);
__declspec(dllimport) void* XW_STDCALL CreateWindowExA(unsigned int extendedStyle, const char* className,
													   const char* title, unsigned int style, int x, int y,
													   int width, int height, void* parent, void* menu,
													   void* instance, void* parameter);
__declspec(dllimport) int XW_STDCALL DestroyWindow(void* window);
__declspec(dllimport) int XW_STDCALL UpdateWindow(void* window);
__declspec(dllimport) void* XW_STDCALL SetFocus(void* window);
__declspec(dllimport) void* XW_STDCALL GetForegroundWindow(void);
__declspec(dllimport) int XW_STDCALL SetForegroundWindow(void* window);
__declspec(dllimport) void* XW_STDCALL SetCursor(void* cursor);
#endif

#ifdef __cplusplus
}
#endif

#endif
