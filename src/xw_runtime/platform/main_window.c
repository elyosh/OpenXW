#include "xw_runtime/platform/main_window.h"

#include <aeron/aeron.h>
#include <stddef.h>

/* Opaque identity consumed by compatibility APIs; never a native HWND. */
static int mainWindowIdentity;

int XwPort_WindowQuitRequested(void) { return Aeron_QuitRequested(); }

void* XwPort_ActivateMainWindow(void) {
	int width;
	int height;
	if (!Aeron_GetWindowSize(&width, &height))
		return NULL;
	Aeron_RaiseWindow();
	return &mainWindowIdentity;
}
