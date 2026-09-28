#ifndef XW_RUNTIME_STORAGE_DIRECTORY_H
#define XW_RUNTIME_STORAGE_DIRECTORY_H
#include <aeron/vfs.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XwDirectory XwDirectory;

typedef struct XwDirectoryEntry {
	char name[256];
	int is_directory;
	uint64_t size;
} XwDirectoryEntry;

XwDirectory* XwStorage_OpenDirectory(AeronVfsRoot root, const char* path);
int XwStorage_NextEntry(XwDirectory* directory, XwDirectoryEntry* entry);
void XwStorage_CloseDirectory(XwDirectory* directory);
int XwStorage_IsDirectory(AeronVfsRoot root, const char* path);
#ifdef __cplusplus
}
#endif
#endif
