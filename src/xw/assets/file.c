#include "xw/assets/file.h"

#include <string.h>

// FUNCTION: XW 0x41EDC0
int File_RemoveFromDataDirectory(const char* filename) {
#ifdef XW_MODERN
	/* The storage owner strips the data prefix and selects USER or replay TEMP. */
	return XwStorage_Remove(filename + (filename[0] == '+'));
#else
	enum { DATA_PATH_CAPACITY = 256 };

	char dataPath[DATA_PATH_CAPACITY];
	strcpy(dataPath, "X-Wing Data\\");
	strcat(dataPath, &filename[filename[0] == '+']);
	return File_RawRemove(dataPath);
#endif
}
