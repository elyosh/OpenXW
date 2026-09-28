/* Storage ownership and VFS helpers adapted from OpenXvT. */
#include "xw_runtime/storage/storage.h"
#include "xw_runtime/runtime/profile.h"
#include "xw_runtime/storage/file_io.h"
#include <aeron/log.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct WritableFile {
	struct WritableFile* next;
	AeronFile* file;
	AeronVfsRoot root;
	char path[XW_PATH_CAPACITY];
} WritableFile;

static AeronVfs* g_vfs;
static AeronVfs* g_xw93;
static AeronVfs* g_xw94;
static AeronVfs* g_xw98;
static char g_lastPath[XW_PATH_CAPACITY];
static XwGameVersion g_lastInstallation = XW_GAME_VERSION_98;
static AeronVfsRoot g_lastRoot;
static WritableFile* g_writableFiles;
static AeronFile* XwStorage_OpenContent(XwGameVersion version, const char* path);

static const char* XwStorage_RootName(AeronVfsRoot root) {
	switch (root) {
		case AERON_VFS_ROOT_ASSET:
			return "ASSET";
		case AERON_VFS_ROOT_RESOURCE:
			return "RESOURCE";
		case AERON_VFS_ROOT_USER:
			return "USER";
		case AERON_VFS_ROOT_TEMP:
			return "TEMP";
		default:
			return "INVALID";
	}
}

void XwStorage_Bind(AeronVfs* vfs) {
	g_vfs = vfs;
	g_xw93 = g_xw94 = g_xw98 = NULL;
	g_lastPath[0] = 0;
	g_lastRoot = AERON_VFS_ROOT_ASSET;
	if (vfs) {
		AeronVfs_SetRootOptions(vfs, AERON_VFS_ROOT_ASSET, AERON_VFS_ROOT_OPTION_CASE_INSENSITIVE_LOOKUP);
		AeronVfs_SetRootOptions(vfs, AERON_VFS_ROOT_USER, AERON_VFS_ROOT_OPTION_CASE_INSENSITIVE_LOOKUP);
		AeronVfs_SetRootOptions(vfs, AERON_VFS_ROOT_TEMP, AERON_VFS_ROOT_OPTION_CASE_INSENSITIVE_LOOKUP);
	}
}

AeronVfs* XwStorage_Vfs(void) { return g_vfs; }

int XwGameVersion_Year(XwGameVersion version) {
	switch (version) {
		case XW_GAME_VERSION_93:
			return 1993;
		case XW_GAME_VERSION_94:
			return 1994;
		case XW_GAME_VERSION_98:
			return 1998;
		default:
			return 0;
	}
}

void XwStorage_BindInstallations(AeronVfs* xw93, AeronVfs* xw94, AeronVfs* xw98) {
	g_xw93 = xw93;
	g_xw94 = xw94;
	g_xw98 = xw98;
}

AeronVfs* XwStorage_InstallationVfs(XwGameVersion version) {
	switch (version) {
		case XW_GAME_VERSION_93:
			return g_xw93;
		case XW_GAME_VERSION_94:
			return g_xw94;
		case XW_GAME_VERSION_98:
			return g_xw98;
		default:
			return NULL;
	}
}

bool XwStorage_HasInstallation(XwGameVersion version) { return XwStorage_InstallationVfs(version) != NULL; }

AeronVfs* XwStorage_RootVfs(AeronVfsRoot root) { return root == AERON_VFS_ROOT_ASSET ? g_xw98 : g_vfs; }

AeronFile* XwStorage_OpenInstallation(XwGameVersion version, const char* path) {
	AeronVfs* vfs = XwStorage_InstallationVfs(version);
	char resolved[XW_PATH_CAPACITY];
	AeronFile* file = vfs ? XwStorage_OpenAssetVfs(vfs, path, resolved, sizeof resolved) : NULL;
	if (file) {
		g_lastInstallation = version;
		g_lastRoot = AERON_VFS_ROOT_ASSET;
		strcpy(g_lastPath, resolved);
	}
	return file;
}

AeronFile* XwStorage_OpenMission(const char* path) {
	char error[256];
	if (!XwProfile_PinRequestedMission(error, sizeof error)) {
		Aeron_LogError("xw.files", "%s", error);
		return NULL;
	}
	return XwStorage_OpenContent(XwProfile_MissionFlight()->mission_version, path);
}

AeronFile* XwStorage_OpenFlight(const char* path) {
	if (!XwProfile_HasActiveFlight())
		return NULL;
	return XwStorage_OpenContent(XwProfile_ActiveFlight()->version, path);
}

const char* XwStorage_LastPath(void) { return g_lastPath; }

XwGameVersion XwStorage_LastInstallation(void) { return g_lastInstallation; }

AeronVfsRoot XwStorage_LastRoot(void) { return g_lastRoot; }

void XwStorage_Fatal(const char* message, int exit_code) {
	char detail[XW_PATH_CAPACITY + 1024];
	snprintf(detail, sizeof(detail), "%s\n\n%s/%s", message ? message : "File operation failed",
			 XwStorage_RootName(g_lastRoot), g_lastPath);
	Aeron_LogError("xw.files", "%s", detail);
	Aeron_FatalError("OpenXW", detail);
	exit(exit_code > 0 ? exit_code : EXIT_FAILURE);
}

int XwStorage_Normalize(const char* path, char* output, size_t capacity) {
	size_t used = 0;
	if (!output || !capacity || !path || !path[0] || path[0] == '/' || path[0] == '\\' || strchr(path, ':'))
		return 0;
	while (*path) {
		const char* start = path;
		size_t length;
		while (*path && *path != '/' && *path != '\\')
			++path;
		length = (size_t)(path - start);
		if (length == 2 && start[0] == '.' && start[1] == '.')
			return 0;
		if (length && !(length == 1 && *start == '.')) {
			if (used + length + (used != 0) >= capacity)
				return 0;
			if (used)
				output[used++] = '/';
			memmove(output + used, start, length);
			used += length;
		}
		if (*path)
			++path;
	}
	if (!used)
		return 0;
	output[used] = 0;
	return 1;
}

static const char* XwStorage_SkipPrefix(const char* path, const char* prefix);

static int XwStorage_NormalizeRoot(AeronVfsRoot root, const char* path, char* output) {
	if (!XwStorage_Normalize(path, output, XW_PATH_CAPACITY))
		return 0;
	if (root == AERON_VFS_ROOT_USER || root == AERON_VFS_ROOT_TEMP) {
		const char* relative = XwStorage_SkipPrefix(output, "X-Wing Data");
		memmove(output, relative, strlen(relative) + 1);
	}
	return 1;
}

static int XwStorage_Found(void* context, const AeronVfsEntry* entry) {
	(void)entry;
	*(int*)context = 1;
	return 1;
}

/* Stat alone cannot distinguish absence from failure. A successful parent
 * listing lets setup and USER-file callers distinguish the two. */
int XwStorage_ProbeVfs(AeronVfs* vfs, AeronVfsRoot root, const char* path) {
	AeronFileInfo info;
	char parent[XW_PATH_CAPACITY];
	char* slash;
	const char* name;
	int found = 0;
	if (!vfs || !XwStorage_Normalize(path, parent, sizeof(parent)))
		return -1;
	if (AeronVfs_Stat(vfs, root, parent, &info) && info.exists)
		return info.is_directory ? -1 : 1;
	slash = strrchr(parent, '/');
	name = path;
	if (slash) {
		*slash = 0;
		name = slash + 1;
		if (!AeronVfs_Stat(vfs, root, parent, &info) || !info.exists) {
			/* Only a confirmed missing ancestor proves this path is absent. */
			int status = XwStorage_ProbeVfs(vfs, root, parent);
			return status == 0 ? 0 : -1;
		}
		if (!info.is_directory)
			return -1;
	} else {
		name = parent;
	}
	if (!AeronVfs_Glob(vfs, root, slash ? parent : "", name, AERON_VFS_GLOB_CASE_INSENSITIVE, XwStorage_Found,
					   &found))
		return -1;
	return found ? -1 : 0;
}

int XwStorage_Probe(AeronVfsRoot root, const char* path) {
	return XwStorage_ProbeVfs(XwStorage_RootVfs(root), root, path);
}

static int XwStorage_Equals(const char* a, const char* b) {
	while (*a && *b && tolower((unsigned char)*a) == tolower((unsigned char)*b)) {
		++a;
		++b;
	}
	return !*a && !*b;
}

static const char* XwStorage_SkipPrefix(const char* path, const char* prefix) {
	size_t index = 0;
	while (prefix[index] && path[index] &&
		   tolower((unsigned char)path[index]) == tolower((unsigned char)prefix[index]))
		++index;
	return !prefix[index] && path[index] == '/' ? path + index + 1 : path;
}

/* fediskio has already expanded :, ; and +. Only physical legacy spellings remain. */
static int XwStorage_LegacyPath(const char* path, char* normalized, int* asset_only) {
	const char* relative;
	*asset_only = 0;
	if (!path)
		return 0;
	if (isalpha((unsigned char)path[0]) && path[1] == ':' && (path[2] == '/' || path[2] == '\\')) {
		path += 3;
		*asset_only = 1;
	}
	if (!XwStorage_Normalize(path, normalized, XW_PATH_CAPACITY))
		return 0;
	relative = XwStorage_SkipPrefix(normalized, "XwingCD");
	if (relative != normalized) {
		*asset_only = 1;
		memmove(normalized, relative, strlen(relative) + 1);
	}
	return 1;
}

static int XwStorage_Extension(const char* path, const char* extension) {
	const char* dot = strrchr(path, '.');
	return dot && XwStorage_Equals(dot, extension);
}

static AeronFile* XwStorage_OpenContent(XwGameVersion version, const char* path) {
	char normalized[XW_PATH_CAPACITY], logical[XW_PATH_CAPACITY];
	int asset_only;
	if (!path)
		return NULL;
	if (*path == ':' || *path == ';' || *path == '+')
		++path;
	if (!XwStorage_LegacyPath(path, normalized, &asset_only))
		return NULL;
	int length =
		snprintf(logical, sizeof logical, "X-Wing Data/%s", XwStorage_SkipPrefix(normalized, "X-Wing Data"));
	if (length < 0 || (size_t)length >= sizeof logical)
		return NULL;
	AeronFile* file = XwStorage_OpenInstallation(version, logical);
	if (!file) {
		g_lastRoot = AERON_VFS_ROOT_ASSET;
		snprintf(g_lastPath, sizeof g_lastPath, "xw%02d/%.1000s", XwGameVersion_Year(version) % 100, logical);
	}
	return file;
}

static int XwStorage_IsSave(const char* path) {
	return XwStorage_Extension(path, ".plt") || XwStorage_Extension(path, ".clp") ||
		   XwStorage_Extension(path, ".rpy");
}

static int XwStorage_WritablePath(const char* path, char* output, AeronVfsRoot* root) {
	char normalized[XW_PATH_CAPACITY];
	int asset_only;
	if (!XwStorage_LegacyPath(path, normalized, &asset_only) || asset_only)
		return 0;
	strcpy(output, XwStorage_SkipPrefix(normalized, "X-Wing Data"));
	/* These are the recovered replay spool and checkpoint scratch files. */
	*root = XwStorage_Equals(output, "rpybuff.tmp") || XwStorage_Equals(output, "input.spl")
				? AERON_VFS_ROOT_TEMP
				: AERON_VFS_ROOT_USER;
	return 1;
}

int XwStorage_ResolveInstallation(const char* path, char* resolved, size_t capacity) {
	char candidate[XW_PATH_CAPACITY];
	char* separator;
	size_t length;
	AeronVfs* vfs;
	AeronFileInfo info;
	int success;
	if (!path || !path[0] || !resolved || !capacity || strlen(path) >= sizeof(candidate))
		return 0;
	strcpy(candidate, path);
	for (char* cursor = candidate; *cursor; ++cursor)
		if (*cursor == '\\')
			*cursor = '/';
	if (candidate[0] != '/' &&
		!(isalpha((unsigned char)candidate[0]) && candidate[1] == ':' && candidate[2] == '/'))
		return 0;
	length = strlen(candidate);
	while (length > 1 && candidate[length - 1] == '/' && !(length == 3 && candidate[1] == ':'))
		candidate[--length] = 0;
	/* Native directory selection is separate from the engine's relative-path rules. */
	separator = strrchr(candidate, '/');
	if (separator && XwStorage_Equals(separator + 1, "XWINGCD")) {
		if (separator == candidate || (separator == candidate + 2 && candidate[1] == ':'))
			separator[1] = 0;
		else
			*separator = 0;
	}
	if (strlen(candidate) >= capacity)
		return 0;
	vfs = AeronVfs_Create(
		&(AeronVfsConfig) { .org_name = "TotallyOpen", .app_name = "OpenXW", .asset_root = candidate });
	if (!vfs)
		return 0;
	success = AeronVfs_Stat(vfs, AERON_VFS_ROOT_ASSET, "", &info) && info.exists && info.is_directory;
	AeronVfs_Destroy(vfs);
	if (success)
		strcpy(resolved, candidate);
	return success;
}

static int XwStorage_OpenMode(const char* mode, AeronVfsOpenMode* result) {
	if (!mode)
		return 0;
	if (!strcmp(mode, "rb") || !strcmp(mode, "r"))
		*result = AERON_VFS_READ;
	else if (!strcmp(mode, "wb"))
		*result = AERON_VFS_WRITE;
	else if (!strcmp(mode, "ab"))
		*result = AERON_VFS_APPEND;
	else
		return 0;
	return 1;
}

static int XwStorage_CreateParent(AeronVfsRoot root, const char* path) {
	char parent[XW_PATH_CAPACITY];
	char* slash;
	strcpy(parent, path);
	slash = strrchr(parent, '/');
	if (!slash)
		return 1;
	*slash = 0;
	return AeronVfs_CreateDirectory(g_vfs, root, parent);
}

AeronFile* XwStorage_OpenRoot(AeronVfsRoot root, const char* path, const char* mode) {
	AeronFile* file = NULL;
	AeronVfsOpenMode open_mode;
	AeronFileInfo info;
	WritableFile* record = NULL;
	char normalized[XW_PATH_CAPACITY];
	int valid = XwStorage_NormalizeRoot(root, path, normalized);
	g_lastRoot = root;
	if (root == AERON_VFS_ROOT_ASSET)
		g_lastInstallation = XW_GAME_VERSION_98;
	snprintf(g_lastPath, sizeof(g_lastPath), "%s", valid ? normalized : path ? path : "");
	if (!XwStorage_RootVfs(root) || !valid || !XwStorage_OpenMode(mode, &open_mode))
		return NULL;
	if (AeronVfs_Stat(XwStorage_RootVfs(root), root, g_lastPath, &info) && info.is_directory)
		return NULL;
	if (open_mode != AERON_VFS_READ) {
		if (root != AERON_VFS_ROOT_USER && root != AERON_VFS_ROOT_TEMP)
			return NULL;
		if (!XwStorage_CreateParent(root, g_lastPath))
			return NULL;
		/* Allocate before opening/truncating. Handles themselves remain plain AeronFile*. */
		record = malloc(sizeof(*record));
		if (!record)
			return NULL;
	}
	if (!AeronVfs_Open(XwStorage_RootVfs(root), root, g_lastPath, open_mode, &file)) {
		free(record);
		return NULL;
	}
	if (record) {
		record->file = file;
		record->root = root;
		strcpy(record->path, g_lastPath);
		record->next = g_writableFiles;
		g_writableFiles = record;
	}
	return file;
}

/* A failed open permits fallback only when its candidate is confirmed absent. */
static AeronFile* XwStorage_TryAsset(AeronVfs* vfs, const char* path, char* resolved, size_t capacity,
									 int directory, int* status) {
	char normalized[XW_PATH_CAPACITY];
	char candidate[XW_PATH_CAPACITY];
	const char* data;
	const char* prefixes[4];
	const char* suffixes[4];
	int count, asset_only;
	*status = -1;
	if (!vfs || !resolved || !capacity || !XwStorage_LegacyPath(path, normalized, &asset_only))
		return NULL;
	resolved[0] = 0;
	data = XwStorage_SkipPrefix(normalized, "X-Wing Data");
	if (data != normalized) {
		prefixes[0] = "";
		prefixes[1] = "XWINGCD/";
		prefixes[2] = "XWINGCD/";
		prefixes[3] = "";
		suffixes[0] = normalized;
		suffixes[1] = normalized;
		suffixes[2] = data;
		suffixes[3] = data;
		count = 4;
	} else {
		prefixes[0] = "XWINGCD/";
		prefixes[1] = "";
		suffixes[0] = normalized;
		suffixes[1] = normalized;
		count = 2;
	}
	for (int index = 0; index < count; ++index) {
		AeronFile* file = NULL;
		int length = snprintf(candidate, sizeof(candidate), "%s%s", prefixes[index], suffixes[index]);
		if (length < 0 || (size_t)length >= sizeof(candidate) || (size_t)length >= capacity)
			return NULL;
		strcpy(resolved, candidate);
		AeronFileInfo info;
		if (directory) {
			if (AeronVfs_Stat(vfs, AERON_VFS_ROOT_ASSET, candidate, &info) && info.exists) {
				*status = info.is_directory ? 1 : -1;
				return NULL;
			}
		} else if (vfs == g_xw98) {
			file = XwStorage_OpenRoot(AERON_VFS_ROOT_ASSET, candidate, "rb");
		} else {
			file = NULL;
			if (AeronVfs_Stat(vfs, AERON_VFS_ROOT_ASSET, candidate, &info) && info.is_directory)
				return NULL;
			AeronVfs_Open(vfs, AERON_VFS_ROOT_ASSET, candidate, AERON_VFS_READ, &file);
		}
		if (file) {
			*status = 1;
			return file;
		}
		if (XwStorage_ProbeVfs(vfs, AERON_VFS_ROOT_ASSET, candidate) != 0)
			return NULL;
	}
	*status = 0;
	return NULL;
}

AeronFile* XwStorage_OpenAsset(const char* path) {
	char resolved[XW_PATH_CAPACITY];
	int status;
	return XwStorage_TryAsset(g_xw98, path, resolved, sizeof(resolved), 0, &status);
}

int XwStorage_ResolveAsset(const char* path, char* resolved, size_t capacity) {
	return XwStorage_ResolveAssetVfs(g_xw98, path, resolved, capacity);
}

int XwStorage_ResolveAssetVfs(AeronVfs* vfs, const char* path, char* resolved, size_t capacity) {
	int status;
	AeronFile* file = XwStorage_TryAsset(vfs, path, resolved, capacity, 0, &status);
	if (file && XwFile_Close(file) != 0)
		return -1;
	return status;
}

AeronFile* XwStorage_OpenAssetVfs(AeronVfs* vfs, const char* path, char* resolved, size_t capacity) {
	int status;
	return XwStorage_TryAsset(vfs, path, resolved, capacity, 0, &status);
}

int XwStorage_ResolveAssetDirectory(AeronVfs* vfs, const char* path, char* resolved, size_t capacity) {
	int status;
	XwStorage_TryAsset(vfs, path, resolved, capacity, 1, &status);
	return status;
}

int XwStorage_ResolveDirectory(AeronVfsRoot root, const char* path, char* resolved, size_t capacity) {
	char normalized[XW_PATH_CAPACITY];
	AeronFileInfo info;
	if (!XwStorage_RootVfs(root) || !path || !resolved || !capacity || root >= AERON_VFS_ROOT_COUNT)
		return 0;
	if (!path[0] || !strcmp(path, ".") ||
		((root == AERON_VFS_ROOT_USER || root == AERON_VFS_ROOT_TEMP) &&
		 XwStorage_Equals(path, "X-Wing Data"))) {
		normalized[0] = 0;
	} else if (!XwStorage_NormalizeRoot(root, path, normalized))
		return 0;
	if (root == AERON_VFS_ROOT_ASSET) {
		if (normalized[0])
			return XwStorage_ResolveAssetDirectory(g_xw98, normalized, resolved, capacity) == 1;
		int status = XwStorage_Probe(AERON_VFS_ROOT_ASSET, "XWINGCD");
		if (AeronVfs_Stat(XwStorage_RootVfs(root), root, "XWINGCD", &info) && info.is_directory)
			strcpy(normalized, "XWINGCD");
		else if (status != 0)
			return 0;
	}
	if (strlen(normalized) >= capacity || !AeronVfs_Stat(XwStorage_RootVfs(root), root, normalized, &info) ||
		!info.is_directory)
		return 0;
	strcpy(resolved, normalized);
	return 1;
}

AeronFile* XwStorage_Open(const char* path, const char* mode) {
	char normalized[XW_PATH_CAPACITY], writable[XW_PATH_CAPACITY];
	AeronVfsRoot root;
	AeronVfsOpenMode open_mode;
	AeronFile* file;
	int asset_only;
	g_lastRoot = AERON_VFS_ROOT_ASSET;
	snprintf(g_lastPath, sizeof(g_lastPath), "%s", path ? path : "");
	if (!XwStorage_OpenMode(mode, &open_mode) || !XwStorage_LegacyPath(path, normalized, &asset_only))
		return NULL;
	if (asset_only)
		return open_mode == AERON_VFS_READ ? XwStorage_OpenAsset(normalized) : NULL;
	if (!XwStorage_WritablePath(normalized, writable, &root))
		return NULL;
	file = XwStorage_OpenRoot(root, normalized, mode);
	if (file || open_mode != AERON_VFS_READ || XwStorage_IsSave(writable) || root == AERON_VFS_ROOT_TEMP)
		return file;
	if (XwStorage_Probe(root, writable) != 0)
		return NULL;
	return XwStorage_OpenAsset(normalized);
}

AeronFile* XwStorage_OpenText(const char* path, const char* mode) {
	return mode && !strcmp(mode, "r") ? XwStorage_Open(path, "rb") : NULL;
}

static int XwStorage_SelectWritable(const char* path) {
	char resolved[XW_PATH_CAPACITY];
	AeronVfsRoot root;
	if (!g_vfs || !XwStorage_WritablePath(path, resolved, &root))
		return 0;
	strcpy(g_lastPath, resolved);
	g_lastRoot = root;
	return 1;
}

int XwStorage_Remove(const char* path) {
	if (!XwStorage_SelectWritable(path) || XwStorage_Probe(g_lastRoot, g_lastPath) != 1)
		return -1;
	return AeronVfs_Remove(g_vfs, g_lastRoot, g_lastPath) ? 0 : -1;
}

int XwStorage_WriteAtomic(const char* path, const void* data, size_t size) {
	if (!XwStorage_SelectWritable(path) || !XwStorage_CreateParent(g_lastRoot, g_lastPath))
		return 0;
	int result = AeronVfs_WriteAllAtomic(g_vfs, g_lastRoot, g_lastPath, data, size);
	if (!result)
		Aeron_LogError("xw.files", "Cannot save %s/%s", XwStorage_RootName(g_lastRoot), g_lastPath);
	return result;
}

int XwStorage_Glob(AeronVfsRoot root, const char* wildcard, AeronVfsGlobCallback callback, void* context) {
	char directory[XW_PATH_CAPACITY];
	char* slash;
	if (!XwStorage_RootVfs(root) || !callback || !XwStorage_NormalizeRoot(root, wildcard, directory))
		return 0;
	slash = strrchr(directory, '/');
	if (slash)
		*slash = 0;
	return AeronVfs_Glob(XwStorage_RootVfs(root), root, slash ? directory : "", slash ? slash + 1 : directory,
						 AERON_VFS_GLOB_FILES | AERON_VFS_GLOB_CASE_INSENSITIVE, callback, context);
}

uint8_t XwStorage_FindMountedCdDrive(const char* path) {
	AeronFile* file;
	if (!path)
		return 0;
	if (*path == ':' || *path == '\\')
		++path;
	file = XwStorage_OpenInstallation(XwProfile_ActiveFrontend()->version, path);
	return file && XwFile_Close(file) == 0 ? 'D' : 0;
}

void XwStorage_ForgetFile(AeronFile* stream) {
	WritableFile** cursor = &g_writableFiles;
	while (*cursor) {
		WritableFile* record = *cursor;
		if (record->file == stream) {
			*cursor = record->next;
			free(record);
			return;
		}
		cursor = &record->next;
	}
}

int XwStorage_CloseGlobalStream(AeronFile* stream, int remove_on_error) {
	char path[XW_PATH_CAPACITY] = { 0 };
	AeronVfsRoot root = AERON_VFS_ROOT_USER;
	int failed, close_failed;
	if (!stream)
		return 0;
	for (WritableFile* record = g_writableFiles; record; record = record->next) {
		if (record->file == stream) {
			strcpy(path, record->path);
			root = record->root;
			break;
		}
	}
	failed = AeronVfs_HasError(stream);
	close_failed = XwFile_Close(stream) != 0;
	if ((failed || close_failed) && path[0]) {
		strcpy(g_lastPath, path);
		g_lastRoot = root;
		Aeron_LogError("xw.files", "Cannot close output: %s/%s", XwStorage_RootName(root), path);
		if (remove_on_error && !AeronVfs_Remove(g_vfs, root, path))
			Aeron_LogError("xw.files", "Cannot remove failed output: %s/%s", XwStorage_RootName(root), path);
	}
	return failed || close_failed;
}
