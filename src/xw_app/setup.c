/* Application-owned versioned installation flow, following OpenTIE. */
#include "xw_app/setup.h"
#include "xw_app/setup_ui.h"
#include "xw_runtime/config/config.h"
#include <aeron/log.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static XwInstallationSet installations;
static int cd_music_available;

static XwGameVersion Version(const char* name) {
	return !strcmp(name, "xw93")   ? XW_GAME_VERSION_93
		   : !strcmp(name, "xw94") ? XW_GAME_VERSION_94
								   : XW_GAME_VERSION_98;
}

typedef struct PilotImport {
	AeronVfs* source_vfs;
	const char* root;
	const char* directory;
	size_t imported, skipped, failed;
} PilotImport;

static bool PilotFilename(const char* source, char destination[13]) {
	if (!source || strchr(source, '/') || strchr(source, '\\'))
		return false;
	const char* extension = strrchr(source, '.');
	size_t base_length = extension ? (size_t)(extension - source) : 0;
	if (!base_length || base_length > 8 || strlen(extension) != 4 ||
		tolower((unsigned char)extension[1]) != 'p' || tolower((unsigned char)extension[2]) != 'l' ||
		tolower((unsigned char)extension[3]) != 't')
		return false;
	for (size_t i = 0; i <= base_length + 4; ++i)
		destination[i] = (char)toupper((unsigned char)source[i]);
	return true;
}

static int ImportPilot(void* userdata, const AeronVfsEntry* entry) {
	PilotImport* import = userdata;
	char destination[13];
	/* Windows and DOS records, including the briefing launch path's backup copy. */
	if (entry->is_directory ||
		(entry->size != 1705 && entry->size != 1706 && entry->size != 3410 && entry->size != 3412) ||
		!PilotFilename(entry->name, destination) ||
		AeronVfs_Exists(XwStorage_Vfs(), AERON_VFS_ROOT_USER, destination)) {
		++import->skipped;
		return 1;
	}
	char source[64];
	int length = snprintf(source, sizeof source, "%s/%s", import->directory, entry->name);
	if (length < 0 || (size_t)length >= sizeof source) {
		++import->failed;
		return 1;
	}
	uint8_t* data = NULL;
	size_t size = 0;
	if (!AeronVfs_ReadAll(import->source_vfs, AERON_VFS_ROOT_ASSET, source, 3412, &data, &size) ||
		size != (size_t)entry->size) {
		Aeron_LogWarn("xw.setup", "could not read pilot preset %s from %s", source, import->root);
		++import->failed;
	} else if (!AeronVfs_WriteAllAtomic(XwStorage_Vfs(), AERON_VFS_ROOT_USER, destination, data, size)) {
		Aeron_LogWarn("xw.setup", "could not import pilot preset %s from %s", source, import->root);
		++import->failed;
	} else
		++import->imported;
	free(data);
	return 1;
}

static void ImportPilotDirectories(PilotImport* import, AeronVfs* source) {
	static const char* const directories[] = { "SUPPORT", "." };
	import->source_vfs = source;
	for (size_t i = 0; i < sizeof directories / sizeof directories[0]; ++i) {
		AeronFileInfo info;
		import->directory = directories[i];
		if (!AeronVfs_Stat(source, AERON_VFS_ROOT_ASSET, import->directory, &info) || !info.is_directory)
			continue;
		if (!AeronVfs_Glob(source, AERON_VFS_ROOT_ASSET, import->directory, "*.plt",
						   AERON_VFS_GLOB_FILES | AERON_VFS_GLOB_CASE_INSENSITIVE, ImportPilot, import)) {
			Aeron_LogWarn("xw.setup", "could not enumerate pilot presets in %s/%s", import->root,
						  import->directory);
			++import->failed;
		}
	}
}

static void ImportPilotsFrom(PilotImport* import, const XwInstallation* installation) {
	if (!installation)
		return;
	import->root = installation->root;
	/* GOG's DOS CD mount hides the loose pilot presets beside its disc image. */
	AeronVfs* loose = AeronVfs_Create(&(AeronVfsConfig) {
		.org_name = "TotallyOpen", .app_name = "OpenXW", .asset_root = installation->root });
	AeronFileInfo info;
	if (loose &&
		AeronVfs_SetRootOptions(loose, AERON_VFS_ROOT_ASSET, AERON_VFS_ROOT_OPTION_CASE_INSENSITIVE_LOOKUP) &&
		AeronVfs_Stat(loose, AERON_VFS_ROOT_ASSET, "", &info) && info.exists)
		ImportPilotDirectories(import, info.is_directory ? loose : installation->vfs);
	else {
		Aeron_LogWarn("xw.setup", "could not inspect pilot presets in %s", import->root);
		++import->failed;
	}
	AeronVfs_Destroy(loose);
}

static void ImportPilotPresets(void) {
	XwGameVersion preferred = Version(XwConfig_Settings()->frontend_version);
	static const XwGameVersion versions[] = { XW_GAME_VERSION_98, XW_GAME_VERSION_94, XW_GAME_VERSION_93 };
	PilotImport import = { 0 };
	ImportPilotsFrom(&import, XwInstallation_Get(&installations, preferred));
	for (size_t i = 0; i < sizeof versions / sizeof versions[0]; ++i)
		if (versions[i] != preferred)
			ImportPilotsFrom(&import, XwInstallation_Get(&installations, versions[i]));
	Aeron_LogInfo("xw.setup", "pilot preset import: %zu copied, %zu skipped, %zu failed", import.imported,
				  import.skipped, import.failed);
}

void XwSetup_SelectDefaults(const XwInstallationSet* installed, XwSettings* settings, bool only_unset) {
	const AeronConfigFile* user = XwConfig_UserDocument();
	const char* version = installed->xw98.vfs ? "xw98" : installed->xw94.vfs ? "xw94" : NULL;
	if (version && !XwInstallation_Get(installed, Version(settings->frontend_version)) &&
		(!only_unset || !AeronConfigFile_Has(user, "frontend.version")))
		strcpy(settings->frontend_version, version);
	if (!version && installed->xw93.vfs)
		version = "xw93";
	if (version && !XwInstallation_Get(installed, Version(settings->flight_version)) &&
		(!only_unset || !AeronConfigFile_Has(user, "flight.version")))
		strcpy(settings->flight_version, version);
	bool windows = !strcmp(settings->music.arrangement, "windows");
	if ((windows ? !installed->xw98.vfs : !installed->xw94.vfs) &&
		(installed->xw94.vfs || installed->xw98.vfs) &&
		(!only_unset || !AeronConfigFile_Has(user, "music.arrangement"))) {
		strcpy(settings->music.arrangement, installed->xw94.vfs ? "gmid" : "windows");
		strcpy(settings->music.backend, "auto");
	}
}

bool XwSetup_ValidateSelection(const XwInstallationSet* installed, const XwSettings* settings, char* error,
							   size_t capacity) {
	XwGameVersion frontend = Version(settings->frontend_version);
	XwGameVersion inflight = !strcmp(settings->inflight_frontend_version, "frontend")
								 ? frontend
								 : Version(settings->inflight_frontend_version);
	XwGameVersion flight = Version(settings->flight_version);
	XwGameVersion music =
		!strcmp(settings->music.arrangement, "windows") ? XW_GAME_VERSION_98 : XW_GAME_VERSION_94;
	const char* component = NULL;
	XwGameVersion missing = frontend;
	if (!XwInstallation_Get(installed, frontend))
		component = "cutscenes and menus";
	else if (!XwInstallation_Get(installed, inflight)) {
		component = "in-flight menus";
		missing = inflight;
	} else if (!XwInstallation_Get(installed, flight)) {
		component = "flight engine";
		missing = flight;
	} else if (!XwInstallation_Get(installed, music)) {
		component = "soundtrack";
		missing = music;
	}
	if (!component)
		return true;
	snprintf(error, capacity, "X-Wing %d is required for the selected %s.", XwGameVersion_Year(missing),
			 component);
	return false;
}

bool XwSetup_CommitSettings(const XwSettings* settings, int save, char* error, size_t capacity) {
	AeronConfigFile* candidate = NULL;
	AeronConfigError detail = { 0 };
	if (!AeronConfigFile_Clone(XwConfig_UserDocument(), &candidate, &detail))
		return XwSettings_FileError(&detail, error, capacity) != 0;
	bool ok = XwSettings_WriteDocument(candidate, settings, XwConfig_Settings(), XwConfig_DefaultSettings(),
									   error, capacity) &&
			  XwConfig_UpdateUser(candidate, save, error, capacity);
	AeronConfigFile_Destroy(candidate);
	return ok;
}

const char* XwSetup_InstallationFor(XwGameVersion version) {
	const XwInstallation* item = XwInstallation_Get(&installations, version);
	return item ? item->root : "";
}

int XwSetup_CdMusicAvailable(void) { return cd_music_available; }

void XwSetup_Shutdown(void) {
	XwStorage_BindInstallations(NULL, NULL, NULL);
	XwInstallation_CloseSet(&installations);
	cd_music_available = 0;
}

static bool OpenSaved(XwInstallation* out, XwGameVersion version, const char* path, char* error,
					  size_t capacity) {
	if (!path || !*path)
		return true;
	char detail[1024] = { 0 };
	if (XwInstallation_Open(out, version, path, detail, sizeof detail))
		return true;
	snprintf(error, capacity, "%s", detail);
	Aeron_LogWarn("xw.setup", "XW%d installation is invalid: %s", XwGameVersion_Year(version) % 100, error);
	return false;
}

static XwSetupResult LoadConfiguration(const XwLaunchOptions* options, XwAppUi* ui, char* error,
									   size_t capacity) {
	int loaded = XwConfig_Load(XwStorage_Vfs(), error, capacity);
	if (!loaded && !XwConfig_CanReplace())
		return XW_SETUP_ERROR;
	if (options->reset_config) {
		if (!XwConfig_Replace(error, capacity))
			return XW_SETUP_ERROR;
		loaded = 1;
		error[0] = 0;
	}
	if (ui) {
		const XwSettings* settings = loaded ? XwConfig_Settings() : XwConfig_DefaultSettings();
		if (!XwAppUi_Init(ui, settings->ui_font, error, capacity))
			return XW_SETUP_ERROR;
	}
	if (!loaded)
		return ui ? XwSetupUi_Recover(ui, error, capacity) : XW_SETUP_ERROR;
	return XW_SETUP_SUCCESS;
}

XwSetupResult XwSetup_Run(const XwLaunchOptions* options, XwAppUi* ui, char* error, size_t capacity) {
	XwSetup_Shutdown();
	XwSetupResult result = LoadConfiguration(options, ui, error, capacity);
	if (result != XW_SETUP_SUCCESS)
		return result;
	const XwSettings* settings = XwConfig_Settings();
	bool ok93 = OpenSaved(&installations.xw93, XW_GAME_VERSION_93,
						  options->xw93_data ? options->xw93_data : settings->xw93_data, error, capacity);
	if (!ok93 && options->xw93_data)
		return XW_SETUP_ERROR;
	bool ok94 = OpenSaved(&installations.xw94, XW_GAME_VERSION_94,
						  options->xw94_data ? options->xw94_data : settings->xw94_data, error, capacity);
	if (!ok94 && options->xw94_data)
		return XW_SETUP_ERROR;
	bool ok98 = OpenSaved(&installations.xw98, XW_GAME_VERSION_98,
						  options->xw98_data ? options->xw98_data : settings->xw98_data, error, capacity);
	if (!ok98 && options->xw98_data)
		return XW_SETUP_ERROR;
	XwSettings selected = *settings;
	XwSetup_SelectDefaults(&installations, &selected, true);
	if (memcmp(&selected, settings, sizeof selected) &&
		!XwSetup_CommitSettings(&selected, 0, error, capacity))
		return XW_SETUP_ERROR;
	bool valid_selection = XwSetup_ValidateSelection(&installations, settings, error, capacity);
	bool needs_setup = options->setup || !ok93 || !ok94 || !ok98 || !valid_selection;
	if (needs_setup) {
		if (!ui) {
			return XW_SETUP_ERROR;
		}
		result = XwSetupUi_Run(ui, &installations, options->xw93_data != NULL, options->xw94_data != NULL,
							   options->xw98_data != NULL, error, capacity);
		if (result != XW_SETUP_SUCCESS)
			return result;
	}
	XwStorage_BindInstallations(installations.xw93.vfs, installations.xw94.vfs, installations.xw98.vfs);
	cd_music_available = XwInstallation_CdMusicAvailable(&installations.xw98);
	if (installations.xw98.vfs && !cd_music_available)
		Aeron_LogWarn("xw.setup", "In-flight CD music is unavailable: MUSIC tracks 2, 3 and 7 are required.");
	if (options->save_config &&
		!XwConfig_SetInstallations(installations.xw93.root, installations.xw94.root, installations.xw98.root,
								   NULL, NULL, NULL, NULL, NULL, 1, error, capacity))
		return XW_SETUP_ERROR;
	if (ui && !Aeron_SetFullscreen(XwConfig_Settings()->fullscreen)) {
		snprintf(error, capacity, "Could not change the display mode.");
		return XW_SETUP_ERROR;
	}
	if (installations.xw93.vfs)
		Aeron_LogInfo("xw.setup", "Validated XW93 installation: %s", installations.xw93.root);
	if (installations.xw94.vfs)
		Aeron_LogInfo("xw.setup", "Validated XW94 installation: %s", installations.xw94.root);
	if (installations.xw98.vfs)
		Aeron_LogInfo("xw.setup", "Validated XW98 installation: %s", installations.xw98.root);
	if (needs_setup)
		ImportPilotPresets();
	if (capacity)
		error[0] = 0;
	return XW_SETUP_SUCCESS;
}
