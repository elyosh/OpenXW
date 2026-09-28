/* Field-table parsing and delta writes follow OpenXvT's configuration bridge. */
#include "xw_runtime/config/preferences.h"
#include "xw_runtime/config/config.h"
#include <aeron/log.h>
#include <stdio.h>
#include <string.h>

typedef struct PreferenceField {
	const char* path;
	size_t offset;
	unsigned size;
	int64_t minimum, maximum, initial;
} PreferenceField;

static const PreferenceField fields[] = {
	{ "game.music_enabled", offsetof(XwSavedShellPreferences, preferences.musicEnabled), 2, 0LL, 1LL, 1 },
	{ "game.sfx_enabled", offsetof(XwSavedShellPreferences, preferences.sfxEnabled), 2, 0LL, 1LL, 1 },
	{ "game.music_volume", offsetof(XwSavedShellPreferences, preferences.musicVolume), 2, 0LL, 16LL, 8 },
	{ "game.sfx_volume", offsetof(XwSavedShellPreferences, preferences.sfxVolume), 2, 0LL, 16LL, 8 },
	{ "game.spoken_text_enabled", offsetof(XwSavedShellPreferences, preferences.spokenTextEnabled), 2, 0LL,
	  1LL, 1 },
	{ "game.transitions_enabled", offsetof(XwSavedShellPreferences, preferences.transitionsEnabled), 2, 0LL,
	  1LL, 1 },
	{ "game.digital_sound_enabled", offsetof(XwSavedShellPreferences, preferences.digitalSoundEnabled), 1,
	  0LL, 1LL, 1 },
	{ "game.voice_enabled", offsetof(XwSavedShellPreferences, preferences.voiceEnabled), 1, 0LL, 1LL, 1 },
	{ "game.replay_disk_cache_kb", offsetof(XwSavedShellPreferences, preferences.replayDiskCacheKB), 2, 32LL,
	  2048LL, 256 },
	{ "game.replay_disk_cache_enabled", offsetof(XwSavedShellPreferences, preferences.replayDiskCacheEnabled),
	  1, 0LL, 1LL, 1 },
	{ "game.classic_missions", offsetof(XwSavedShellPreferences, preferences.classicMissions), 1, 0LL, 1LL,
	  0 },
	{ "game.high_detail_starfield", offsetof(XwSavedShellPreferences, preferences.highDetailStarfield), 1,
	  0LL, 1LL, 1 },
	{ "game.backdrops_enabled", offsetof(XwSavedShellPreferences, preferences.backdropsEnabled), 1, 0LL, 1LL,
	  1 },
	{ "game.debris_enabled", offsetof(XwSavedShellPreferences, preferences.debrisEnabled), 1, 0LL, 1LL, 1 },
	/* Preserve the existing YAML key for the recovered markings preference. */
	{ "game.compatibility.field_15", offsetof(XwSavedShellPreferences, preferences.markingsEnabled), 1, 0LL,
	  255LL, 1 },
	{ "game.engine_glow_enabled", offsetof(XwSavedShellPreferences, preferences.engineGlowEnabled), 1, 0LL,
	  1LL, 1 },
	{ "game.starfighter_detail", offsetof(XwSavedShellPreferences, preferences.starfighterDetail), 1, 0LL,
	  12LL, 12 },
	{ "game.starship_detail", offsetof(XwSavedShellPreferences, preferences.starshipDetail), 1, 0LL, 12LL,
	  12 },
	{ "game.death_star_detail", offsetof(XwSavedShellPreferences, preferences.deathStarDetail), 1, 0LL, 12LL,
	  12 },
	{ "game.interlace_enabled", offsetof(XwSavedShellPreferences, preferences.interlaceEnabled), 1, 0LL, 1LL,
	  0 },
	{ "game.resolution_index", offsetof(XwSavedShellPreferences, preferences.resolutionIndex), 1, 0LL, 3LL,
	  2 },
	{ "game.engine_sound_enabled", offsetof(XwSavedShellPreferences, preferences.engineSoundEnabled), 1, 0LL,
	  1LL, 1 },
	{ "game.texture_quality", offsetof(XwSavedShellPreferences, preferences.textureQuality), 1, 0LL, 2LL, 1 },
	{ "game.brightness", offsetof(XwSavedShellPreferences, preferences.brightness), 2, 0LL, 8LL, 2 },
	{ "game.compatibility.field_40", offsetof(XwSavedShellPreferences, field_40), 4, 0LL, 4294967295LL, 1 },
	{ "game.compatibility.field_44", offsetof(XwSavedShellPreferences, field_44), 4, 0LL, 4294967295LL, 1 },
	{ "game.compatibility.field_48", offsetof(XwSavedShellPreferences, field_48), 4, 0LL, 4294967295LL, 0 },
	{ "game.compatibility.field_4c", offsetof(XwSavedShellPreferences, field_4C), 4, 0LL, 4294967295LL, 0 },
	{ "game.compatibility.intro_playback_mode", offsetof(XwSavedShellPreferences, introPlaybackMode), 4,
	  -2147483648LL, 2147483647LL, 0 },
	{ "game.compatibility.field_5c", offsetof(XwSavedShellPreferences, field_5C), 4, 0LL, 4294967295LL, 0 },
};

static void Store(XwSavedShellPreferences* out, const PreferenceField* field, int64_t value) {
	uint8_t* target = (uint8_t*)out + field->offset;
	if (field->size == 1)
		*target = (uint8_t)value;
	else if (field->size == 2) {
		uint16_t v = (uint16_t)value;
		memcpy(target, &v, sizeof v);
	} else {
		uint32_t v = (uint32_t)value;
		memcpy(target, &v, sizeof v);
	}
}

static int64_t Read(const XwSavedShellPreferences* value, const PreferenceField* field) {
	const uint8_t* source = (const uint8_t*)value + field->offset;
	if (field->size == 1)
		return *source;
	if (field->size == 2) {
		uint16_t v;
		memcpy(&v, source, sizeof v);
		return v;
	}
	uint32_t v;
	memcpy(&v, source, sizeof v);
	return field->minimum < 0 && v > INT32_MAX ? (int64_t)v - 4294967296LL : v;
}

static int Valid(const PreferenceField* field, int64_t value) {
	return value >= field->minimum && value <= field->maximum &&
		   (field->offset != offsetof(XwSavedShellPreferences, preferences.replayDiskCacheKB) ||
			value % 32 == 0);
}

void XwPreferences_Defaults(XwSavedShellPreferences* out) {
	static const uint8_t actions[32] = { 156, 157, 'r', '.', 'e', 'i', '[', '\b', '\r', ']' };
	memset(out, 0, sizeof *out);
	for (size_t i = 0; i < sizeof fields / sizeof fields[0]; ++i)
		Store(out, &fields[i], fields[i].initial);
	memcpy(out->preferences.joystickActions, actions, sizeof actions);
}

static int ParseBytes(const AeronConfigFile* document, const char* path, uint8_t* out, size_t count,
					  char* error, size_t capacity) {
	const AeronConfigNode* node = AeronConfigFile_GetNode(document, path);
	if (AeronConfigNode_Type(node) != AERON_CONFIG_SEQUENCE || AeronConfigNode_SequenceCount(node) != count)
		return XwSettings_NodeError(document, path, "expected a fixed-length byte sequence", error, capacity);
	for (size_t i = 0; i < count; ++i) {
		const AeronConfigNode* item = AeronConfigNode_SequenceGet(node, i);
		int64_t value = AeronConfigNode_Int(item, -1);
		if (AeronConfigNode_Type(item) != AERON_CONFIG_INT || value < 0 || value > 255)
			return XwSettings_NodeError(document, path, "expected bytes in 0..255", error, capacity);
		out[i] = (uint8_t)value;
	}
	return 1;
}

int XwPreferences_Parse(const AeronConfigFile* document, XwSavedShellPreferences* out, char* error,
						size_t capacity) {
	XwSavedShellPreferences candidate = { 0 };
	for (size_t i = 0; i < sizeof fields / sizeof fields[0]; ++i) {
		const PreferenceField* field = &fields[i];
		const AeronConfigNode* node = AeronConfigFile_GetNode(document, field->path);
		AeronConfigNodeType type = AeronConfigNode_Type(node);
		int64_t value = type == AERON_CONFIG_BOOL && field->maximum == 1
							? AeronConfigNode_Bool(node, 0)
							: AeronConfigNode_Int(node, INT64_MIN);
		if (!Valid(field, value))
			return XwSettings_NodeError(
				document, field->path, "value outside supported range or replay-cache step", error, capacity);
		Store(&candidate, field, value);
	}
	if (!ParseBytes(document, "game.joystick_actions", candidate.preferences.joystickActions, 32, error,
					capacity) ||
		!ParseBytes(document, "game.compatibility.gap54", candidate.gap54, 8, error, capacity))
		return 0;
	*out = candidate;
	return 1;
}

static int WriteBytes(AeronConfigFile* document, const char* path, const uint8_t* value,
					  const uint8_t* defaults, size_t count, AeronConfigError* error) {
	AeronConfigValue entries[32];
	if (!memcmp(value, defaults, count))
		return !AeronConfigFile_Has(document, path) || AeronConfigFile_Remove(document, path, error);
	for (size_t i = 0; i < count; ++i)
		entries[i] = (AeronConfigValue) { .type = AERON_CONFIG_INT, .value.int_value = value[i] };
	AeronConfigValue sequence = { .type = AERON_CONFIG_SEQUENCE, .value.sequence = { entries, count } };
	return AeronConfigFile_SetValue(document, path, &sequence, error);
}

int XwPreferences_WriteDocument(AeronConfigFile* document, const XwSavedShellPreferences* value,
								const XwSavedShellPreferences* defaults, char* error, size_t capacity) {
	AeronConfigError detail = { 0 };
	for (size_t i = 0; i < sizeof fields / sizeof fields[0]; ++i) {
		const PreferenceField* field = &fields[i];
		int64_t v = Read(value, field);
		if (!Valid(field, v))
			return XwSettings_NodeError(document, field->path,
										"preference outside supported range or replay-cache step", error,
										capacity);
		int success;
		if (v == Read(defaults, field))
			success = !AeronConfigFile_Has(document, field->path) ||
					  AeronConfigFile_Remove(document, field->path, &detail);
		else if (field->maximum == 1)
			success = AeronConfigFile_SetBool(document, field->path, (int)v, &detail);
		else
			success = AeronConfigFile_SetInt(document, field->path, v, &detail);
		if (!success)
			return XwSettings_FileError(&detail, error, capacity);
	}
	if (!WriteBytes(document, "game.joystick_actions", value->preferences.joystickActions,
					defaults->preferences.joystickActions, 32, &detail) ||
		!WriteBytes(document, "game.compatibility.gap54", value->gap54, defaults->gap54, 8, &detail))
		return XwSettings_FileError(&detail, error, capacity);
	return 1;
}

static XwSavedShellPreferences pending;
static int pending_save, error_requested;
static char save_error[1024];

void XwPreferences_Save(const XwSavedShellPreferences* value) {
	if (XwConfig_Write(value, save_error, sizeof save_error)) {
		XwPreferences_ResetPending();
		return;
	}
	pending = *value;
	pending_save = error_requested = 1;
	Aeron_LogError("xw.config", "%s", save_error);
}

int XwPreferences_HasPendingSave(void) { return pending_save; }

const char* XwPreferences_SaveError(void) { return save_error; }

int XwPreferences_RetrySave(char* error, size_t capacity) {
	if (!pending_save)
		return 1;
	if (!XwConfig_Write(&pending, save_error, sizeof save_error)) {
		snprintf(error, capacity, "%s", save_error);
		return 0;
	}
	XwPreferences_ResetPending();
	return 1;
}

int XwPreferences_ConsumeSaveError(char* error, size_t capacity) {
	if (!error_requested)
		return 0;
	snprintf(error, capacity, "%s", save_error);
	error_requested = 0;
	return 1;
}

void XwPreferences_Flush(void) {
	char error[1024];
	if (!XwPreferences_RetrySave(error, sizeof error))
		Aeron_LogError("xw.config", "Unsaved preferences: %s", error);
}

void XwPreferences_ResetPending(void) {
	pending_save = error_requested = 0;
	memset(&pending, 0, sizeof pending);
	save_error[0] = 0;
}
