#include "xw_runtime/audio/midi_backend.h"
#include "xw_runtime/audio/midi_resources.h"
#include "xw_runtime/config/config.h"
#include "xw_runtime/storage/storage.h"
#include <aeron/aeron.h>
#include <aeron/log.h>
#include <imuse/midi_nuked_sc55.h>
#include <landru/fourcc.h>
#include <stdio.h>
#include <string.h>

static ImuseNukedSc55Romset* sc55_romset;
static uint32_t music_type;
static XwMusicSettings active_settings;

static const char* music_backend(const XwMusicSettings* settings) {
	if (strcmp(settings->backend, "auto"))
		return settings->backend;
	return !strcmp(settings->arrangement, "adlb")   ? "adlib"
		   : !strcmp(settings->arrangement, "rlnd") ? "mt32"
													: "fm4";
}

bool XwMidiBackend_SettingsPending(const XwMusicSettings* requested) {
	if (strcmp(requested->arrangement, active_settings.arrangement))
		return true;
	if (!strcmp(requested->arrangement, "windows"))
		return false;
	const char* backend = music_backend(requested);
	if (strcmp(backend, music_backend(&active_settings)))
		return true;
	/* Only resources used by the selected synthesizer affect the current soundtrack. */
	if (!strcmp(backend, "fluidsynth"))
		return strcmp(requested->soundfont, active_settings.soundfont) != 0;
	if (!strcmp(backend, "sc55"))
		return strcmp(requested->sc55_rom_directory, active_settings.sc55_rom_directory) != 0;
	if (!strcmp(backend, "mt32"))
		return strcmp(requested->mt32_control, active_settings.mt32_control) ||
			   strcmp(requested->mt32_pcm, active_settings.mt32_pcm);
	return false;
}

static void warn_midi_fallback(const char* reason, const char* fallback) {
	static const AeronMessageBoxButton button = {
		.id = 0, .label = "Continue", .is_default = 1, .is_cancel = 1
	};
	char message[1024];
	snprintf(message, sizeof message, "%s\n\n%s will be used for this session.", reason, fallback);
	Aeron_LogWarn("xw.music", "%s", message);
	const AeronMessageBoxOptions options = { .kind = AERON_MESSAGE_BOX_WARNING,
											 .title = "MIDI Synthesizer Warning",
											 .message = message,
											 .buttons = &button,
											 .button_count = 1 };
	if (!Aeron_ShowMessageBox(&options, NULL))
		Aeron_LogError("xw.music", "could not display the MIDI fallback warning");
	else
		Aeron_RaiseWindow();
}

ImuseMidiBackend* XwMidiBackend_Create(char* error, size_t capacity) {
	const XwSettings* settings = XwConfig_Settings();
	if (!settings)
		return NULL;
	active_settings = settings->music;
	bool midi = strcmp(active_settings.arrangement, "windows") != 0;
	music_type = !strcmp(active_settings.arrangement, "adlb")   ? FOURCC_ADLB
				 : !strcmp(active_settings.arrangement, "rlnd") ? FOURCC_RLND
																: FOURCC_GMID;
	const char* backend = midi ? music_backend(&active_settings) : "none";
	bool valid = music_type == FOURCC_ADLB   ? !strcmp(backend, "adlib")
				 : music_type == FOURCC_RLND ? !strcmp(backend, "mt32")
											 : (!strcmp(backend, "fm4") || !strcmp(backend, "fluidsynth") ||
												!strcmp(backend, "sc55"));
	if (midi && !valid) {
		snprintf(error, capacity, "Backend %s cannot render arrangement %s", backend,
				 active_settings.arrangement);
		return NULL;
	}
	if (midi && !XwStorage_HasInstallation(XW_GAME_VERSION_94)) {
		snprintf(error, capacity,
				 "Select your X-Wing (1994) game folder and restart OpenXW to use this soundtrack.");
		return NULL;
	}
	if (!strcmp(backend, "mt32")) {
		char rom_error[768];
		if (!XwMidiResources_Mt32PairValidate(active_settings.mt32_control, active_settings.mt32_pcm,
											  rom_error, sizeof rom_error)) {
			warn_midi_fallback(rom_error, "The 1994 General MIDI / OPL3 soundtrack");
			music_type = FOURCC_GMID;
			backend = "fm4";
		}
	}
	if (!strcmp(backend, "sc55")) {
		char rom_error[768];
		sc55_romset =
			imuse_nuked_sc55_romset_load(active_settings.sc55_rom_directory, rom_error, sizeof rom_error);
		if (!sc55_romset) {
			warn_midi_fallback(rom_error, "The OPL3 MIDI backend");
			backend = "fm4";
		} else
			Aeron_LogInfo("xw.music", "using Nuked SC-55 %s MIDI backend from %s",
						  imuse_nuked_sc55_romset_name(sc55_romset), active_settings.sc55_rom_directory);
	}
	ImuseMidiBackend* synth = NULL;
	if (!strcmp(backend, "adlib"))
		synth = imuse_midi_xwing_adlib_create();
	else if (!strcmp(backend, "mt32"))
		synth = imuse_midi_mt32_create(active_settings.mt32_control, active_settings.mt32_pcm);
	else if (!strcmp(backend, "fm4"))
		synth = imuse_fm4_opl3_xwing_backend_create();
	else if (!strcmp(backend, "sc55"))
		synth = imuse_nuked_sc55_backend_create(sc55_romset);
	else if (midi) {
		ImuseFluidSynthConfig cfg = { .soundfontPath = active_settings.soundfont };
		synth = imuse_fluidsynth_backend_create(&cfg);
	}
	if (midi && !synth) {
		snprintf(error, capacity,
				 "Cannot create %s music backend; check its build support and configured SoundFont/ROM paths",
				 backend);
		XwMidiBackend_ReleaseResources();
		return NULL;
	}
	return synth;
}

bool XwMidiBackend_UsesImuse(void) { return strcmp(active_settings.arrangement, "windows") != 0; }

uint32_t XwMidiBackend_ResourceType(void) { return music_type ? music_type : FOURCC_GMID; }

void XwMidiBackend_ReleaseResources(void) {
	imuse_nuked_sc55_romset_release(sc55_romset);
	sc55_romset = NULL;
}
