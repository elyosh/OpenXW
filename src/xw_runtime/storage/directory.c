/* Owned VFS enumeration adapted from OpenTIE's storage adapter. */
#include "xw_runtime/storage/directory.h"
#include "xw_runtime/storage/storage.h"
#include <stdlib.h>
#include <string.h>

struct XwDirectory {
	XwDirectoryEntry* entries;
	size_t count, next, capacity;
};

static int Collect(void* context, const AeronVfsEntry* entry) {
	XwDirectory* directory = context;
	if (strlen(entry->name) >= sizeof directory->entries[0].name)
		return 1;
	if (directory->count == directory->capacity) {
		if (directory->capacity > SIZE_MAX / (2 * sizeof *directory->entries))
			return 0;
		size_t capacity = directory->capacity ? directory->capacity * 2 : 16;
		XwDirectoryEntry* entries = realloc(directory->entries, capacity * sizeof *entries);
		if (!entries)
			return 0;
		directory->entries = entries;
		directory->capacity = capacity;
	}
	XwDirectoryEntry* out = &directory->entries[directory->count++];
	*out = (XwDirectoryEntry) { 0 };
	strcpy(out->name, entry->name);
	out->is_directory = entry->is_directory;
	out->size = entry->is_directory || entry->size < 0 ? 0 : (uint64_t)entry->size;
	return 1;
}

XwDirectory* XwStorage_OpenDirectory(AeronVfsRoot root, const char* path) {
	char resolved[XW_PATH_CAPACITY];
	if (!XwStorage_ResolveDirectory(root, path, resolved, sizeof resolved))
		return NULL;
	XwDirectory* directory = calloc(1, sizeof *directory);
	if (!directory)
		return NULL;
	if (!AeronVfs_Glob(XwStorage_RootVfs(root), root, resolved, "*",
					   AERON_VFS_GLOB_FILES | AERON_VFS_GLOB_DIRECTORIES | AERON_VFS_GLOB_CASE_INSENSITIVE,
					   Collect, directory)) {
		XwStorage_CloseDirectory(directory);
		return NULL;
	}
	return directory;
}

int XwStorage_NextEntry(XwDirectory* directory, XwDirectoryEntry* entry) {
	if (!directory || !entry || directory->next == directory->count)
		return 0;
	*entry = directory->entries[directory->next++];
	return 1;
}

void XwStorage_CloseDirectory(XwDirectory* directory) {
	if (!directory)
		return;
	free(directory->entries);
	free(directory);
}

int XwStorage_IsDirectory(AeronVfsRoot root, const char* path) {
	char resolved[XW_PATH_CAPACITY];
	return XwStorage_ResolveDirectory(root, path, resolved, sizeof resolved);
}
