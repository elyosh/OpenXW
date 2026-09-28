#ifndef XW_UTIL_WIN32_H
#define XW_UTIL_WIN32_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <xw/compiler.h>

/* Declarations follow ascending original IDB address. */

/* 0x4ABDF0 */
int XW_STDCALL WinMain(void* hInstance, void* hPrevInstance, char* lpCmdLine, int nShowCmd);

/* 0x4AEFF0 */
int Win32_CheckSingleInstance(void);

#ifdef __cplusplus
}
#endif

#endif
