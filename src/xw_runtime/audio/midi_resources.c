#include "xw_runtime/audio/midi_resources.h"

#include <imuse/midi_nuked_sc55.h>
#include <stdio.h>
#include <string.h>

#ifdef XW_HAVE_MT32EMU
#include <mt32emu/mt32emu.h>
#endif

bool XwMidiResources_Sc55RomDirectoryValidate(const char* path, char* error, size_t capacity) {
	ImuseNukedSc55Romset* romset = imuse_nuked_sc55_romset_load(path, error, capacity);
	if (!romset)
		return false;
	imuse_nuked_sc55_romset_release(romset);
	return true;
}

bool XwMidiResources_Mt32Available(void) {
#ifdef XW_HAVE_MT32EMU
	return true;
#else
	return false;
#endif
}

#ifdef XW_HAVE_MT32EMU
static bool mt32_rom_result(mt32emu_return_code result, bool control, const char* path, char* error,
							size_t capacity) {
	if (result == (control ? MT32EMU_RC_ADDED_CONTROL_ROM : MT32EMU_RC_ADDED_PCM_ROM))
		return true;
	const char* kind = control ? "control" : "PCM";
	const char* reason =
		result == MT32EMU_RC_FILE_NOT_FOUND || result == MT32EMU_RC_FILE_NOT_LOADED
			? "is missing or unreadable"
		: result == MT32EMU_RC_ADDED_PARTIAL_CONTROL_ROM || result == MT32EMU_RC_ADDED_PARTIAL_PCM_ROM
			? "is a partial ROM; select a complete image"
		: result == MT32EMU_RC_ADDED_CONTROL_ROM || result == MT32EMU_RC_ADDED_PCM_ROM
			? "has the wrong ROM type"
			: "is not a supported ROM image";
	snprintf(error, capacity, "MT-32 %s ROM %s:\n%s", kind, reason, path);
	return false;
}
#endif

bool XwMidiResources_Mt32RomValidate(const char* path, bool control, char* error, size_t capacity) {
	if (!XwMidiResources_Mt32Available()) {
		snprintf(error, capacity, "MT-32 emulation is unavailable in this build.");
		return false;
	}
	if (!path || !path[0]) {
		snprintf(error, capacity, "Select an MT-32 %s ROM file.", control ? "control" : "PCM");
		return false;
	}
#ifdef XW_HAVE_MT32EMU
	mt32emu_report_handler_i handler = { 0 };
	mt32emu_context context = mt32emu_create_context(handler, NULL);
	if (!context) {
		snprintf(error, capacity, "Could not inspect the MT-32 ROM file.");
		return false;
	}
	bool valid = mt32_rom_result(mt32emu_add_rom_file(context, path), control, path, error, capacity);
	mt32emu_free_context(context);
	if (valid && error && capacity)
		error[0] = 0;
	return valid;
#else
	snprintf(error, capacity, "MT-32 emulation is unavailable in this build.");
	return false;
#endif
}

bool XwMidiResources_Mt32PairValidate(const char* control, const char* pcm, char* error, size_t capacity) {
	if (!XwMidiResources_Mt32Available()) {
		snprintf(error, capacity, "MT-32 emulation is unavailable in this build.");
		return false;
	}
	if (!control || !control[0] || !pcm || !pcm[0]) {
		snprintf(error, capacity, "Select both MT-32 control and PCM ROM files.");
		return false;
	}
#ifdef XW_HAVE_MT32EMU
	mt32emu_report_handler_i handler = { 0 };
	mt32emu_context context = mt32emu_create_context(handler, NULL);
	if (!context) {
		snprintf(error, capacity, "Could not inspect the MT-32 ROM files.");
		return false;
	}
	bool valid = mt32_rom_result(mt32emu_add_rom_file(context, control), true, control, error, capacity);
	if (valid)
		valid = mt32_rom_result(mt32emu_add_rom_file(context, pcm), false, pcm, error, capacity);
	if (valid) {
		valid = mt32emu_open_synth(context) == MT32EMU_RC_OK;
		if (!valid)
			snprintf(error, capacity, "The selected MT-32 control and PCM ROMs are incompatible.");
		else
			mt32emu_close_synth(context);
	}
	mt32emu_free_context(context);
	if (valid && error && capacity)
		error[0] = 0;
	return valid;
#else
	snprintf(error, capacity, "MT-32 emulation is unavailable in this build.");
	return false;
#endif
}

const char* XwMidiResources_ParentDirectory(const char* path, char* out, size_t capacity) {
	if (!path || !path[0])
		return NULL;
	snprintf(out, capacity, "%s", path);
	char* slash = strrchr(out, '/');
	char* backslash = strrchr(out, '\\');
	char* separator = slash;
	if (!separator || (backslash && backslash > separator))
		separator = backslash;
	if (!separator) {
		snprintf(out, capacity, ".");
		return out;
	}
	if (separator == out || (separator == out + 2 && out[1] == ':'))
		separator[1] = 0;
	else
		*separator = 0;
	return out;
}
