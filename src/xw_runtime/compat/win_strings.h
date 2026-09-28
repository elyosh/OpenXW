#ifndef XW_RUNTIME_COMPAT_WIN_STRINGS_H
#define XW_RUNTIME_COMPAT_WIN_STRINGS_H

#include "xw/compiler.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef XW_MODERN
static inline char* XwWinStringCopy(char* destination, const char* source) {
	return strcpy(destination, source);
}
#else
__declspec(dllimport) char* XW_STDCALL lstrcpyA(char* destination, const char* source);
#define XwWinStringCopy lstrcpyA
#endif

#ifdef __cplusplus
}
#endif

#endif
