#ifndef XW_RUNTIME_PLATFORM_LEGACY_HARDWARE_H
#define XW_RUNTIME_PLATFORM_LEGACY_HARDWARE_H

#include "xw/compiler.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XW_MODERN
/* Fixed Win32 version-resource block returned by VerQueryValueA("\\"). */
typedef struct XwLegacyFixedFileInfo {
	uint32_t dwSignature;
	uint32_t dwStrucVersion;
	uint32_t dwFileVersionMS;
	uint32_t dwFileVersionLS;
	uint32_t dwProductVersionMS;
	uint32_t dwProductVersionLS;
	uint32_t dwFileFlagsMask;
	uint32_t dwFileFlags;
	uint32_t dwFileOS;
	uint32_t dwFileType;
	uint32_t dwFileSubtype;
	uint32_t dwFileDateMS;
	uint32_t dwFileDateLS;
} XwLegacyFixedFileInfo;

uint32_t XW_STDCALL GetFileVersionInfoSizeA(const char* filename, uint32_t* handle);
int XW_STDCALL GetFileVersionInfoA(const char* filename, uint32_t handle, uint32_t size, void* data);
int XW_STDCALL VerQueryValueA(const void* block, const char* subBlock, void** value, unsigned int* length);
__declspec(dllimport) void* XW_STDCALL GlobalAlloc(unsigned int flags, size_t size);
__declspec(dllimport) void* XW_STDCALL GlobalLock(void* handle);
__declspec(dllimport) int XW_STDCALL GlobalUnlock(void* handle);
__declspec(dllimport) void* XW_STDCALL GlobalFree(void* handle);

/* Win32's invalid kernel handle is the pointer-sized all-ones sentinel. */
static __inline int XwLegacyHardware_IsInvalidHandle(const void* handle) {
	return handle == (void*)(intptr_t)-1;
}

/* MMIO must perform each access even when a register was just written. */
static __inline uint32_t XwLegacyHardware_ReadRegister32(const void* address) {
	return *(const volatile uint32_t*)address;
}

static __inline void XwLegacyHardware_WriteRegister32(void* address, uint32_t value) {
	*(volatile uint32_t*)address = value;
}

/* The Win9x diagnostic prints a 32-bit physical register address with %X. */
static __inline uintptr_t XwLegacyHardware_RegisterAddress(const volatile void* address) {
	return (uintptr_t)address;
}

__declspec(dllimport) int XW_STDCALL GetKeyboardType(int typeFlag);
__declspec(dllimport) void* XW_STDCALL GetActiveWindow(void);
__declspec(dllimport) void XW_STDCALL OutputDebugStringA(const char* message);
__declspec(dllimport) void* XW_STDCALL CreateFileA(const char* name, uint32_t access, uint32_t sharing,
												   void* security, uint32_t disposition, uint32_t flags,
												   void* templateFile);
__declspec(dllimport) int XW_STDCALL CloseHandle(void* handle);
__declspec(dllimport) int XW_STDCALL DeviceIoControl(void* device, uint32_t code, void* input,
													 uint32_t inputSize, void* output, uint32_t outputSize,
													 uint32_t* bytesReturned, void* overlapped);
__declspec(dllimport) void XW_STDCALL Sleep(uint32_t milliseconds);
__declspec(dllimport) uint32_t XW_STDCALL GetLogicalDriveStringsA(uint32_t capacity, char* buffer);
__declspec(dllimport) unsigned int XW_STDCALL GetDriveTypeA(const char* rootPath);
#endif

#ifdef __cplusplus
}
#endif

#endif
