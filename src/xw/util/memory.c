#include "xw/util/memory.h"

#ifdef XW_MODERN
#include "xw_runtime/snapshot/render_assets.h"
#endif

#include "xw_runtime/platform/virtual_memory.h"

#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4FC8D0
MemoryHandleTableState g_handleTables = { { 0 }, { NULL } };

// GLOBAL: XW 0x53C8D0
uint8_t g_handleAllocatorInitialized = 0;

// GLOBAL: XW 0x53C8D4
unsigned int g_handleAllocationAttemptCount = 0;

// FUNCTION: XW 0x47CCA0
uint16_t Memory_AllocHandle(size_t size, int legacyTag) {
	return Memory_AllocHandleInternal(size, legacyTag, 0);
}

// FUNCTION: XW 0x47CCC0
uint16_t Memory_AllocHandleInternal(size_t size, int legacyTag, int clearFlag) {
	uint16_t slotIndex;
	void* block;
	(void)legacyTag;
	++g_handleAllocationAttemptCount;
	if (g_handleAllocatorInitialized == 0) {
		g_handleAllocatorInitialized = 1;
		memset(g_handleTables.sizeTable, 0, sizeof(g_handleTables.sizeTable));
		memset(g_handleTables.ptrTable, 0, sizeof(g_handleTables.ptrTable));
	}
	for (slotIndex = 0; slotIndex < MEMORY_HANDLE_CAPACITY; ++slotIndex) {
		if (g_handleTables.ptrTable[slotIndex] == NULL) {
			break;
		}
	}
	if (slotIndex == MEMORY_HANDLE_CAPACITY) {
		return 0;
	}
	block = malloc(size);
	g_handleTables.ptrTable[slotIndex] = block;
	if (block == NULL) {
		return 0;
	}
	if (clearFlag != 0) {
		memset(block, 0, size);
	}
	g_handleTables.sizeTable[slotIndex] = size;
	return slotIndex + 1;
}

// FUNCTION: XW 0x47CD80
void Memory_FreeHandle(uint16_t handle) {
#ifdef XW_MODERN
	XwRenderAssets_FreeHandle(handle);
#endif
	if (g_handleTables.ptrTable[handle - 1] != NULL)
		free(g_handleTables.ptrTable[handle - 1]);
	g_handleTables.sizeTable[handle - 1] = 0;
	g_handleTables.ptrTable[handle - 1] = NULL;
}

// FUNCTION: XW 0x47CDC0
void* Memory_LockHandle(uint16_t handle) { return g_handleTables.ptrTable[handle - 1]; }

// FUNCTION: XW 0x4AECE0
int32_t Memory_SetRegionExecuteReadWrite(void* lpAddress, size_t dwSize) {
#ifdef XW_MODERN
	/* Recovered C does not patch executable instructions. */
	(void)lpAddress;
	(void)dwSize;
	return 1;
#else
	uint32_t oldProtection;
	return VirtualProtect(lpAddress, dwSize, XW_PAGE_EXECUTE_READWRITE, &oldProtection);
#endif
}
