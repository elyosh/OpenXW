#include "xw_app/settings/audio_page.h"
#include "xw_app/settings/installation_page.h"
#include "xw_app/settings/settings.h"
#include "xw_runtime/audio/imuse_session.h"
#include "xw_runtime/audio/midi_backend.h"
#include "xw_runtime/audio/midi_resources.h"
#include "xw_runtime/audio/music_policy.h"
#include "xw_runtime/storage/storage.h"
#include <imuse/midi_nuked_sc55.h>
#include <string.h>

static void music_settings(AeronUiContext* ui, XwMusicSettings* music) {
	/* A newly selected installation becomes usable after the next startup. */
	if (!XwStorage_HasInstallation(XW_GAME_VERSION_94))
		return;
	static const char* const names[] = { "1998 - CD audio", "1994 - General MIDI", "1994 - AdLib",
										 "1994 - Roland MT-32" };
	static const char* const values[] = { "windows", "gmid", "adlb", "rlnd" };
	int selected = 0;
	for (int i = 0; i < 4; ++i)
		if (!strcmp(music->arrangement, values[i]))
			selected = i;
	int first = XwStorage_HasInstallation(XW_GAME_VERSION_98) ? 0 : 1;
	int choice = selected - first;
	if (AeronUi_Selector(ui, "Soundtrack", &choice, names + first, 4 - first)) {
		selected = choice + first;
		strcpy(music->arrangement, values[selected]);
		if (selected > 1 || (selected == 1 && strcmp(music->backend, "fm4") &&
							 strcmp(music->backend, "fluidsynth") && strcmp(music->backend, "sc55")))
			strcpy(music->backend, "auto");
	}
	if (selected == 1) {
		const char* outputs[] = { "OPL3 (FM synthesis)", "SoundFont", "Roland SC-55" };
		int sc55_available = imuse_nuked_sc55_backend_available();
		int output = !strcmp(music->backend, "sc55") && sc55_available ? 2
					 : !strcmp(music->backend, "fluidsynth")           ? 1
																	   : 0;
		if (AeronUi_Selector(ui, "Synthesizer", &output, outputs, sc55_available ? 3 : 2))
			strcpy(music->backend, output == 2 ? "sc55" : output == 1 ? "fluidsynth" : "fm4");
		if (output == 1)
			AeronUi_InputText(ui, "SoundFont", music->soundfont, sizeof music->soundfont, 0);
		else if (output == 2) {
			uint32_t action = AeronUi_InputTextWithAction(
				ui, "SC-55 ROM Directory", music->sc55_rom_directory, sizeof music->sc55_rom_directory,
				AERON_UI_INPUT_TEXT_NONE, "Browse...");
			if (action & AERON_UI_INPUT_TEXT_ACTION_ACTIVATED)
				XwInstallationPage_OpenSc55Picker(music->sc55_rom_directory);
			if (!music->sc55_rom_directory[0])
				AeronUi_Help(ui, "Select a folder containing SC-55 or SC-55mkII ROM dumps.");
		}
		if (!sc55_available && !strcmp(music->backend, "sc55")) {
			AeronUi_Help(ui, "SC-55 support is unavailable in this build; OPL3 will be used.");
			if (AeronUi_Button(ui, "Select OPL3"))
				strcpy(music->backend, "fm4");
		}
	} else if (selected == 3) {
		uint32_t action =
			AeronUi_InputTextWithAction(ui, "MT-32 Control ROM", music->mt32_control,
										sizeof music->mt32_control, AERON_UI_INPUT_TEXT_NONE, "Browse...");
		if (action & AERON_UI_INPUT_TEXT_ACTION_ACTIVATED)
			XwInstallationPage_OpenMt32Picker(true, music->mt32_control);
		action = AeronUi_InputTextWithAction(ui, "MT-32 PCM ROM", music->mt32_pcm, sizeof music->mt32_pcm,
											 AERON_UI_INPUT_TEXT_NONE, "Browse...");
		if (action & AERON_UI_INPUT_TEXT_ACTION_ACTIVATED)
			XwInstallationPage_OpenMt32Picker(false, music->mt32_pcm);
		if (XwMidiResources_Mt32Available() && (!music->mt32_control[0] || !music->mt32_pcm[0]))
			AeronUi_Help(ui, "Select both MT-32 control and PCM ROM files.");
		if (!XwMidiResources_Mt32Available()) {
			AeronUi_Help(ui, "MT-32 support is unavailable in this build; General MIDI / OPL3 will be used.");
			if (AeronUi_Button(ui, "Select General MIDI / OPL3")) {
				strcpy(music->arrangement, "gmid");
				strcpy(music->backend, "fm4");
			}
		}
	}
	if (XwMidiBackend_SettingsPending(music))
		AeronUi_Help(ui, "Restart OpenXW to apply soundtrack changes.");
}

void XwAudioPage_Draw(AeronUiContext* ui) {
	XwSettings* draft = XwSettingsMenu_Draft();
	XwShellPreferences* p = &draft->game.preferences;
	if (!AeronUi_BeginScroll(ui, "Audio settings", XwSettingsMenu_ScrollHeight()))
		return;
	AeronUi_Header(ui, "Music");
	music_settings(ui, &draft->music);
	int value = p->musicEnabled;
	if (AeronUi_Toggle(ui, "Music", &value))
		p->musicEnabled = (int16_t)value;
	value = p->musicVolume;
	if (AeronUi_SliderInt(ui, "Music volume", &value, 0, 16, 1, "%d"))
		p->musicVolume = (int16_t)value;
	AeronUi_Header(ui, "Effects and Speech");
	AeronUi_Toggle(ui, "SB16 Low-pass Filter", &draft->sb16_filter_enabled);
	value = p->sfxEnabled;
	if (AeronUi_Toggle(ui, "Sound effects", &value))
		p->sfxEnabled = (int16_t)value;
	value = p->sfxVolume;
	if (AeronUi_SliderInt(ui, "Effects and speech volume", &value, 0, 16, 1, "%d"))
		p->sfxVolume = (int16_t)value;
	value = p->voiceEnabled;
	if (AeronUi_Toggle(ui, "Flight speech", &value))
		p->voiceEnabled = (uint8_t)value;
	if (XwStorage_HasInstallation(XW_GAME_VERSION_98)) {
		value = p->engineSoundEnabled;
		if (AeronUi_Toggle(ui, "Engine sound", &value))
			p->engineSoundEnabled = (uint8_t)value;
		AeronUi_SliderInt(ui, "Engine sound volume", &draft->player_engine_sound_volume_percent, 0, 100, 5,
						  "%d%%");
	}
	if (!XwMusicPolicy_UsesImuse() && !XwMusicPolicy_CdAvailable())
		AeronUi_Help(ui, "Music during missions is unavailable.");
	AeronUi_EndScroll(ui);
}
