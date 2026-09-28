#ifndef XW_APP_INSTALLATION_H
#define XW_APP_INSTALLATION_H
#include "xw_runtime/storage/storage.h"
#include <stdbool.h>

typedef struct XwInstallation {
	XwGameVersion version;
	char root[XW_PATH_CAPACITY];
	AeronVfs* vfs;
} XwInstallation;

typedef struct XwInstallationSet {
	XwInstallation xw93, xw94, xw98;
} XwInstallationSet;

bool XwInstallation_Open(XwInstallation* out, XwGameVersion version, const char* path, char* error,
						 size_t capacity);
void XwInstallation_Close(XwInstallation* installation);
void XwInstallation_CloseSet(XwInstallationSet* installations);
const XwInstallation* XwInstallation_Get(const XwInstallationSet* installations, XwGameVersion version);
bool XwInstallation_ValidatePath(XwGameVersion version, const char* path, char* resolved,
								 size_t resolved_capacity, char* error, size_t capacity);
int XwInstallation_CdMusicAvailable(const XwInstallation* installation);
#endif
