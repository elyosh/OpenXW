#ifndef XW_RUNTIME_COMPAT_LEGACY_MEMORY_H
#define XW_RUNTIME_COMPAT_LEGACY_MEMORY_H

#include <stddef.h>
#include <stdint.h>

/* Original success depends on heap address bits, which are not a portable status. */
static __inline int16_t XwLegacyMemory_AllocationResult(const void* allocation) {
#ifdef XW_MODERN
	return allocation != NULL;
#else
	return (int16_t)(uintptr_t)allocation;
#endif
}

#endif
