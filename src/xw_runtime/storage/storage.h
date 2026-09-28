#ifndef XW_RUNTIME_STORAGE_H
#define XW_RUNTIME_STORAGE_H

#include <aeron/vfs.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XW_PATH_CAPACITY 1024

typedef enum XwGameVersion { XW_GAME_VERSION_94, XW_GAME_VERSION_98, XW_GAME_VERSION_93 } XwGameVersion;

int XwGameVersion_Year(XwGameVersion version);

static inline bool XwGameVersion_IsDos(XwGameVersion version) {
	return version == XW_GAME_VERSION_93 || version == XW_GAME_VERSION_94;
}

/* Owner-thread only. Bind before opening files; close all consumers before unbinding. */
void XwStorage_Bind(AeronVfs* vfs);
/* Application VFS owns USER/TEMP/RESOURCE; installation handles are borrowed. */
AeronVfs* XwStorage_Vfs(void);
void XwStorage_BindInstallations(AeronVfs* xw93, AeronVfs* xw94, AeronVfs* xw98);
AeronVfs* XwStorage_InstallationVfs(XwGameVersion version);
AeronVfs* XwStorage_RootVfs(AeronVfsRoot root);
AeronFile* XwStorage_OpenInstallation(XwGameVersion version, const char* path);
/* Explicit content domains; these never change general ASSET or media routing. */
AeronFile* XwStorage_OpenMission(const char* path);
AeronFile* XwStorage_OpenFlight(const char* path);
int XwStorage_ResolveAssetVfs(AeronVfs* vfs, const char* path, char* resolved, size_t capacity);
bool XwStorage_HasInstallation(XwGameVersion version);
int XwStorage_Normalize(const char* path, char* output, size_t capacity);
/* Native absolute installation path, accepting the root or its XWINGCD child.
 * Checks the directory only; setup owns installation completeness checks. */
int XwStorage_ResolveInstallation(const char* path, char* resolved, size_t capacity);
/* Probe an exact root-relative path: 1 ordinary file exists, 0 absent, -1 invalid or inaccessible.
 * ResolveAsset additionally verifies that the selected file can be opened. */
int XwStorage_Probe(AeronVfsRoot root, const char* path);
/* Candidate setup uses the same lookup policy without rebinding live storage. */
int XwStorage_ProbeVfs(AeronVfs* vfs, AeronVfsRoot root, const char* path);
AeronFile* XwStorage_OpenAssetVfs(AeronVfs* vfs, const char* path, char* resolved, size_t capacity);
int XwStorage_ResolveAssetDirectory(AeronVfs* vfs, const char* path, char* resolved, size_t capacity);
int XwStorage_ResolveAsset(const char* path, char* resolved, size_t capacity);
int XwStorage_ResolveDirectory(AeronVfsRoot root, const char* path, char* resolved, size_t capacity);
AeronFile* XwStorage_OpenAsset(const char* path);
AeronFile* XwStorage_Open(const char* path, const char* mode);
AeronFile* XwStorage_OpenText(const char* path, const char* mode);
/* Exact root-relative access; ASSET/RESOURCE are always read-only.
 * USER/TEMP share the canonical X-Wing Data prefix stripping used by game I/O.
 * Supported modes: r/rb, wb, ab; text translation belongs to XwFile_GetsText. */
AeronFile* XwStorage_OpenRoot(AeronVfsRoot root, const char* path, const char* mode);
int XwStorage_WriteAtomic(const char* path, const void* data, size_t size);
int XwStorage_Remove(const char* path);
int XwStorage_Glob(AeronVfsRoot root, const char* wildcard, AeronVfsGlobCallback callback, void* context);
uint8_t XwStorage_FindMountedCdDrive(const char* path);
/* Writable-file identities survive temporary replacement of the recovered g_stream. */
int XwStorage_CloseGlobalStream(AeronFile* stream, int remove_on_error);
/* Called by XwFile_Close before releasing the underlying handle. */
void XwStorage_ForgetFile(AeronFile* stream);
const char* XwStorage_LastPath(void);
XwGameVersion XwStorage_LastInstallation(void);
AeronVfsRoot XwStorage_LastRoot(void);
#if defined(_MSC_VER)
__declspec(noreturn)
#else
__attribute__((noreturn))
#endif
void XwStorage_Fatal(const char* message, int exit_code);

#ifdef __cplusplus
}
#endif
#endif
