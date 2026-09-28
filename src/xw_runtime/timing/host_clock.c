#include "xw_runtime/timing/host_clock.h"

static uint64_t elapsed_us;

void XwTime_Reset(void) { elapsed_us = 0; }

void XwTime_AdvanceHostClock(int32_t delta_us) {
	if (delta_us > 0)
		elapsed_us += (uint32_t)delta_us;
}

uint64_t XwTime_GetElapsedUs(void) { return elapsed_us; }

uint32_t XwTime_GetElapsedTicks(void) { return (uint32_t)(elapsed_us / 1000u); }

uint32_t XW_STDCALL timeGetTime(void) { return XwTime_GetElapsedTicks(); }
