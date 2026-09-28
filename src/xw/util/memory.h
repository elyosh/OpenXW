#ifndef XW_UTIL_MEMORY_H
#define XW_UTIL_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

enum { MEMORY_HANDLE_CAPACITY = 32768 };

/* Original size: 0x40000 bytes. Native sizes and pointers retain their host width. */
typedef struct MemoryHandleTableState {
	/* IDB +0x0 */
	size_t sizeTable[MEMORY_HANDLE_CAPACITY];
	/* IDB +0x20000 */
	void* ptrTable[MEMORY_HANDLE_CAPACITY];
} MemoryHandleTableState;

extern MemoryHandleTableState g_handleTables;
extern uint8_t g_handleAllocatorInitialized;
extern unsigned int g_handleAllocationAttemptCount;

/* Declarations follow ascending original IDB address. */

/* 0x47CCA0 */
uint16_t Memory_AllocHandle(size_t size, int legacyTag);

/* 0x47CCC0 */
uint16_t Memory_AllocHandleInternal(size_t size, int legacyTag, int clearFlag);

/* 0x47CD80 */
void Memory_FreeHandle(uint16_t handle);

/* 0x47CDC0 */
void* Memory_LockHandle(uint16_t handle);

/* 0x4AECE0 */
int32_t Memory_SetRegionExecuteReadWrite(void* lpAddress, size_t dwSize);

#ifdef __cplusplus
}
#endif

#endif
