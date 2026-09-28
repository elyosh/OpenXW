#ifndef XW_RUNTIME_TIMING_HOST_CLOCK_H
#define XW_RUNTIME_TIMING_HOST_CLOCK_H

#include "xw/compiler.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(XW_MODERN) && defined(_MSC_VER)
__declspec(dllimport)
#endif
uint32_t XW_STDCALL timeGetTime(void);

#ifdef XW_MODERN
/* Advance the virtual clock once at the port tick boundary. */
void XwTime_Reset(void);
void XwTime_AdvanceHostClock(int32_t delta_us);
uint64_t XwTime_GetElapsedUs(void);
uint32_t XwTime_GetElapsedTicks(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
