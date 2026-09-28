#ifndef XW_RUNTIME_PLATFORM_VIRTUAL_MEMORY_H
#define XW_RUNTIME_PLATFORM_VIRTUAL_MEMORY_H

#include "xw/compiler.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { XW_PAGE_EXECUTE_READWRITE = 0x40 };

#ifndef XW_MODERN
__declspec(dllimport) int32_t XW_STDCALL VirtualProtect(void* address, size_t size, uint32_t newProtection,
														uint32_t* oldProtection);
#endif

#ifdef __cplusplus
}
#endif

#endif
