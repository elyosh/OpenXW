/* Document ownership and transactions adapted from OpenXvT. */
#include "xw_runtime/config/config.h"
#include "xw_runtime/config/keyboard_config.h"
#include "xw_runtime/config/preferences.h"
#include "xw_runtime/storage/storage.h"
#include <aeron/log.h>
#include <stdio.h>
#include <string.h>

static AeronConfigFile* g_defaults;
static AeronConfigFile* g_user;
static AeronConfigFile* g_resolved;
static AeronVfs* g_configVfs;
static XwSettings g_settings, g_defaultSettings;
static XwSceneSettings g_sceneDefaults;
static uint64_t g_generation;

const AeronConfigFile* XwConfig_UserDocument(void) { return g_user; }

const AeronConfigFile* XwConfig_ResolvedDocument(void) { return g_resolved; }

const XwSettings* XwConfig_Settings(void) { return g_resolved ? &g_settings : NULL; }

const XwSettings* XwConfig_DefaultSettings(void) { return g_defaults ? &g_defaultSettings : NULL; }

uint64_t XwConfig_Generation(void) { return g_generation; }

int XwConfig_CanReplace(void) { return g_defaults != NULL; }

void XwConfig_Shutdown(void) {
	AeronConfigFile_Destroy(g_resolved);
	AeronConfigFile_Destroy(g_user);
	AeronConfigFile_Destroy(g_defaults);
	g_defaults = g_user = g_resolved = NULL;
	g_configVfs = NULL;
	XwPreferences_ResetPending();
}

static int ConfigError(char* error, size_t capacity, const char* message) {
	snprintf(error, capacity, "USER/config.yaml: %s", message);
	return 0;
}

static int Version(const AeronConfigFile* document, char* error, size_t capacity) {
	const AeronConfigNode* version = AeronConfigFile_GetNode(document, "version");
	if (AeronConfigNode_Type(AeronConfigFile_Root(document)) != AERON_CONFIG_MAP)
		return XwSettings_NodeError(document, "", "expected a configuration mapping", error, capacity);
	if (AeronConfigNode_Type(version) != AERON_CONFIG_INT || AeronConfigNode_Int(version, 0) != 1)
		return XwSettings_NodeError(document, "version", "unsupported configuration version; expected 1",
									error, capacity);
	return 1;
}

static int EmptyUser(AeronConfigFile** out, char* error, size_t capacity) {
	AeronConfigError detail;
	if (!AeronConfigFile_CreateMap(AERON_VFS_ROOT_USER, "config.yaml", out, &detail) ||
		!AeronConfigFile_SetInt(*out, "version", 1, &detail)) {
		AeronConfigFile_Destroy(*out);
		*out = NULL;
		return XwSettings_FileError(&detail, error, capacity);
	}
	return 1;
}

static int MigrateInstallations(AeronConfigFile* user, char* error, size_t capacity) {
	static const char* const old_keys[] = { "paths.game_data", "music.data" };
	static const char* const new_keys[] = { "paths.installations.xw98", "paths.installations.xw94" };
	AeronConfigError detail = { 0 };
	for (size_t i = 0; i < 2; ++i) {
		if (!AeronConfigFile_Has(user, old_keys[i]))
			continue;
		const char* path = AeronConfigNode_String(AeronConfigFile_GetNode(user, old_keys[i]), NULL);
		if (!path)
			return XwSettings_NodeError(user, old_keys[i], "expected an installation path string", error,
										capacity);
		if (!AeronConfigFile_Has(user, new_keys[i]) &&
			!AeronConfigFile_SetString(user, new_keys[i], path, &detail))
			return XwSettings_FileError(&detail, error, capacity);
		if (!AeronConfigFile_Remove(user, old_keys[i], &detail))
			return XwSettings_FileError(&detail, error, capacity);
	}
	return 1;
}

static int Build(const AeronConfigFile* defaults, const AeronConfigFile* candidate,
				 const XwSceneSettings* scene_defaults, AeronConfigFile** user, AeronConfigFile** resolved,
				 XwSettings* settings, char* error, size_t capacity) {
	AeronConfigFile* root = NULL;
	AeronConfigError detail;
	if (!Version(candidate, error, capacity))
		return 0;
	if (!AeronConfigFile_CreateMap(AERON_VFS_ROOT_USER, "config.yaml", &root, &detail))
		return XwSettings_FileError(&detail, error, capacity);
	int success = AeronConfigFile_Overlay(root, candidate, user, &detail);
	AeronConfigFile_Destroy(root);
	if (success && !MigrateInstallations(*user, error, capacity))
		return 0;
	if (success && AeronConfigFile_Has(*user, "render.mode"))
		success = AeronConfigFile_Remove(*user, "render.mode", &detail);
	/* The shipped gamepad profile belongs to the application, not user overrides. */
	if (success && AeronConfigFile_Has(*user, "input.gamepad_defaults")) {
		Aeron_LogWarn("xw.config", "Ignoring shipped-only input.gamepad_defaults override");
		success = AeronConfigFile_Remove(*user, "input.gamepad_defaults", &detail);
	}
	if (!success || !AeronConfigFile_Overlay(defaults, *user, resolved, &detail))
		return XwSettings_FileError(&detail, error, capacity);
	XwKeyboardBindings bindings;
	return XwKeyboardConfig_Read(defaults, &bindings, error, capacity) &&
		   XwKeyboardConfig_Resolve(&bindings, *user, *resolved, error, capacity) &&
		   XwSettings_Parse(*resolved, scene_defaults, settings, error, capacity);
}

static void Publish(AeronConfigFile* user, AeronConfigFile* resolved, const XwSettings* settings) {
	AeronConfigFile_Destroy(g_user);
	AeronConfigFile_Destroy(g_resolved);
	g_user = user;
	g_resolved = resolved;
	g_settings = *settings;
	++g_generation;
}

int XwConfig_UpdateUser(const AeronConfigFile* candidate, int save, char* error, size_t capacity) {
	AeronConfigFile* user = NULL;
	AeronConfigFile* resolved = NULL;
	AeronConfigError detail;
	XwSettings settings;
	int success = 0;
	if (!g_defaults || !g_configVfs)
		return ConfigError(error, capacity, "valid shipped defaults are required");
	if (!Build(g_defaults, candidate, &g_sceneDefaults, &user, &resolved, &settings, error, capacity))
		goto done;
	if (save && !AeronConfigFile_SaveYaml(g_configVfs, user, &detail)) {
		XwSettings_FileError(&detail, error, capacity);
		goto done;
	}
	Publish(user, resolved, &settings);
	user = resolved = NULL;
	success = 1;
done:
	AeronConfigFile_Destroy(user);
	AeronConfigFile_Destroy(resolved);
	return success;
}

static int LoadSceneDefaults(AeronVfs* vfs, XwSceneSettings* settings, char* error, size_t capacity) {
	AeronConfigFile* document = NULL;
	AeronConfigError detail;
	if (!AeronConfigFile_LoadYamlEx(vfs, AERON_VFS_ROOT_RESOURCE, "aeron/scene3d_defaults.yaml", &document,
									&detail))
		return XwSettings_FileError(&detail, error, capacity);
	int ok = AeronSceneSettings_Load(AeronConfigFile_Root(document), &settings->ssao, &settings->shadows,
									 &settings->tonemap, &detail);
	AeronConfigFile_Destroy(document);
	return ok || XwSettings_FileError(&detail, error, capacity);
}

int XwConfig_Load(AeronVfs* vfs, char* error, size_t capacity) {
	XwSceneSettings scene_defaults = { 0 };
	if (!LoadSceneDefaults(vfs, &scene_defaults, error, capacity))
		return 0;
	AeronConfigFile* defaults = NULL;
	AeronConfigFile* candidate = NULL;
	AeronConfigFile* user = NULL;
	AeronConfigFile* resolved = NULL;
	AeronConfigError detail;
	XwSettings settings, initial;
	int valid_defaults = 0, success = 0;
	if (!AeronConfigFile_LoadYamlEx(vfs, AERON_VFS_ROOT_RESOURCE, "config.yaml", &defaults, &detail)) {
		XwSettings_FileError(&detail, error, capacity);
		goto done;
	}
	if (!Version(defaults, error, capacity) ||
		!XwSettings_Parse(defaults, &scene_defaults, &initial, error, capacity))
		goto done;
	valid_defaults = 1;
	int status = XwStorage_ProbeVfs(vfs, AERON_VFS_ROOT_USER, "config.yaml");
	if (status < 0) {
		ConfigError(error, capacity, "cannot inspect configuration; file preserved");
		goto done;
	}
	if (status == 0) {
		if (!EmptyUser(&candidate, error, capacity))
			goto done;
	} else if (!AeronConfigFile_LoadYamlEx(vfs, AERON_VFS_ROOT_USER, "config.yaml", &candidate, &detail)) {
		XwSettings_FileError(&detail, error, capacity);
		goto done;
	}
	if (!Build(defaults, candidate, &scene_defaults, &user, &resolved, &settings, error, capacity))
		goto done;
	AeronConfigFile_Destroy(g_defaults);
	g_defaults = defaults;
	defaults = NULL;
	g_defaultSettings = initial;
	g_sceneDefaults = scene_defaults;
	g_configVfs = vfs;
	Publish(user, resolved, &settings);
	user = resolved = NULL;
	success = 1;
done:
	/* First-launch recovery can reset a bad user file using validated shipped defaults. */
	if (!success && valid_defaults && !g_defaults) {
		g_defaults = defaults;
		defaults = NULL;
		g_defaultSettings = initial;
		g_sceneDefaults = scene_defaults;
		g_configVfs = vfs;
	}
	AeronConfigFile_Destroy(defaults);
	AeronConfigFile_Destroy(candidate);
	AeronConfigFile_Destroy(user);
	AeronConfigFile_Destroy(resolved);
	return success;
}

int XwConfig_Replace(char* error, size_t capacity) {
	AeronConfigFile* candidate = NULL;
	if (!EmptyUser(&candidate, error, capacity))
		return 0;
	int success = XwConfig_UpdateUser(candidate, 1, error, capacity);
	AeronConfigFile_Destroy(candidate);
	return success;
}

int XwConfig_Save(char* error, size_t capacity) {
	if (!g_user)
		return ConfigError(error, capacity, "configuration is not loaded");
	return XwConfig_UpdateUser(g_user, 1, error, capacity);
}

int XwConfig_SetInstallations(const char* xw93, const char* xw94, const char* xw98, const char* arrangement,
							  const char* backend, const char* sc55_rom_directory, const char* mt32_control,
							  const char* mt32_pcm, int save, char* error, size_t capacity) {
	AeronConfigFile* candidate = NULL;
	AeronConfigError detail = { 0 };
	if (!g_user)
		return ConfigError(error, capacity, "configuration is not loaded");
	int success = AeronConfigFile_Clone(g_user, &candidate, &detail);
	if (success && xw93)
		success = AeronConfigFile_SetString(candidate, "paths.installations.xw93", xw93, &detail);
	if (success && xw94)
		success = AeronConfigFile_SetString(candidate, "paths.installations.xw94", xw94, &detail);
	if (success && xw98)
		success = AeronConfigFile_SetString(candidate, "paths.installations.xw98", xw98, &detail);
	if (success && arrangement) {
		success = AeronConfigFile_SetString(candidate, "music.arrangement", arrangement, &detail) &&
				  AeronConfigFile_SetString(candidate, "music.backend", backend ? backend : "auto", &detail);
	}
	if (success && sc55_rom_directory)
		success =
			AeronConfigFile_SetString(candidate, "music.sc55_rom_directory", sc55_rom_directory, &detail);
	if (success && mt32_control)
		success = AeronConfigFile_SetString(candidate, "music.mt32_control", mt32_control, &detail);
	if (success && mt32_pcm)
		success = AeronConfigFile_SetString(candidate, "music.mt32_pcm", mt32_pcm, &detail);
	if (success)
		success = XwConfig_UpdateUser(candidate, save, error, capacity);
	else
		XwSettings_FileError(&detail, error, capacity);
	AeronConfigFile_Destroy(candidate);
	return success;
}

int XwConfig_SetSkipIntro(int enabled, char* error, size_t capacity) {
	AeronConfigFile* candidate = NULL;
	AeronConfigError detail;
	if (!g_user)
		return ConfigError(error, capacity, "configuration is not loaded");
	if (!AeronConfigFile_Clone(g_user, &candidate, &detail))
		return XwSettings_FileError(&detail, error, capacity);
	int success = enabled == g_defaultSettings.skip_intro
					  ? (!AeronConfigFile_Has(candidate, "startup.skip_intro") ||
						 AeronConfigFile_Remove(candidate, "startup.skip_intro", &detail))
					  : AeronConfigFile_SetBool(candidate, "startup.skip_intro", enabled, &detail);
	if (success)
		success = XwConfig_UpdateUser(candidate, 0, error, capacity);
	else
		XwSettings_FileError(&detail, error, capacity);
	AeronConfigFile_Destroy(candidate);
	return success;
}

int XwConfig_Apply(XwSavedShellPreferences* game, char* error, size_t capacity) {
	if (!g_resolved || !game)
		return ConfigError(error, capacity, "configuration is not loaded");
	*game = g_settings.game;
	return 1;
}

int XwConfig_Write(const XwSavedShellPreferences* game, char* error, size_t capacity) {
	AeronConfigFile* candidate = NULL;
	AeronConfigError detail;
	if (!g_user || !game)
		return ConfigError(error, capacity, "configuration is not loaded");
	if (!AeronConfigFile_Clone(g_user, &candidate, &detail))
		return XwSettings_FileError(&detail, error, capacity);
	int success = XwPreferences_WriteDocument(candidate, game, &g_defaultSettings.game, error, capacity) &&
				  XwConfig_UpdateUser(candidate, 1, error, capacity);
	AeronConfigFile_Destroy(candidate);
	return success;
}

bool XwConfig_SetKeyboard(const XwKeyboardBindings* bindings, char* error, size_t capacity) {
	AeronConfigFile* candidate = NULL;
	AeronConfigError detail = { 0 };
	if (!g_user)
		return ConfigError(error, capacity, "configuration is not loaded");
	if (!AeronConfigFile_Clone(g_user, &candidate, &detail) ||
		!XwKeyboardConfig_Write(candidate, bindings, &detail)) {
		AeronConfigFile_Destroy(candidate);
		return XwSettings_FileError(&detail, error, capacity);
	}
	bool ok = XwConfig_UpdateUser(candidate, 0, error, capacity) != 0;
	AeronConfigFile_Destroy(candidate);
	return ok;
}

bool XwConfig_RestoreKeyboard(char* error, size_t capacity) {
	AeronConfigFile* candidate = NULL;
	AeronConfigError detail = { 0 };
	if (!g_user)
		return ConfigError(error, capacity, "configuration is not loaded");
	if (!AeronConfigFile_Clone(g_user, &candidate, &detail) ||
		!AeronConfigFile_Remove(candidate, "input.keyboard", &detail)) {
		AeronConfigFile_Destroy(candidate);
		return XwSettings_FileError(&detail, error, capacity);
	}
	bool ok = XwConfig_UpdateUser(candidate, 0, error, capacity) != 0;
	AeronConfigFile_Destroy(candidate);
	return ok;
}
