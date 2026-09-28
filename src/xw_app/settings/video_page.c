/* Host controls follow OpenXvT; classic game rendering is selected for the next mission. */
#include "xw_app/settings/video_page.h"
#include "xw_app/settings/settings.h"
#include "xw_remaster/hud_assets.h"
#include "xw_runtime/config/config.h"
#include "xw_runtime/config/preference_apply.h"
#include "xw_runtime/runtime/profile.h"
#include <stdio.h>
#include <string.h>

static void HostControls(AeronUiContext* ui, XwSettings* draft) {
	AeronUi_Header(ui, "Display");
	AeronUi_Toggle(ui, "Fullscreen", &draft->fullscreen);
	AeronUi_Toggle(ui, "HDR output", &draft->presentation.hdr_output);
	if (draft->presentation.hdr_output && !Aeron_OutputHdrEnabled())
		AeronUi_Help(ui, Aeron_OutputHdrStatusName(Aeron_OutputHdrStatus()));
#ifndef __APPLE__
	static const char* const gamma_names[] = { "Auto", "sRGB", "2.2", "2.4" };
	static const float gamma_values[] = { -1, 0, 2.2f, 2.4f };
	float gamma = draft->presentation.sdr_gamma;
	int index = gamma < 0 ? 0 : gamma == 0 ? 1 : gamma == 2.4f ? 3 : 2;
	if (AeronUi_Selector(ui, "SDR content gamma", &index, gamma_names, 4))
		draft->presentation.sdr_gamma = gamma_values[index];
	int automatic = draft->presentation.paper_white_nits == 0;
	if (AeronUi_Toggle(ui, "Automatic paper white", &automatic))
		draft->presentation.paper_white_nits = automatic ? 0 : 200;
	if (!automatic)
		AeronUi_SliderFloat(ui, "Paper white", &draft->presentation.paper_white_nits, 80, 1000, 10,
							"%.0f nits");
#endif
}

static bool DosControls(const XwSettings* draft) {
	return XwProfile_HasActiveFlight() ? XwProfile_DosFlight() : strcmp(draft->flight_version, "xw98") != 0;
}

static bool MsaaControl(AeronUiContext* ui, int* samples) {
	static const int values[] = { 1, 2, 4, 8 };
	static const char* const labels[] = { "Off", "2x", "4x", "8x" };
	int index = 0;
	while (index < 3 && values[index] != *samples)
		++index;
	if (!AeronUi_Selector(ui, "MSAA", &index, labels, 4))
		return false;
	*samples = values[index];
	return true;
}

static void ShadowControl(AeronUiContext* ui, AeronSceneShadowSettings* shadows) {
	static const uint32_t sizes[] = { 4096, 8192 };
	const char* labels[] = { "Standard", "High", NULL };
	char custom[48];
	int count = 2, quality = shadows->atlas_size >= 8192 ? 1 : 0;
	if (!shadows->enabled || (shadows->atlas_size != 4096 && shadows->atlas_size != 8192)) {
		snprintf(custom, sizeof custom, shadows->enabled ? "%u (configured)" : "Off (configured)",
				 shadows->atlas_size);
		labels[2] = custom;
		count = 3;
		quality = 2;
	}
	if (AeronUi_Selector(ui, "Shadow Quality", &quality, labels, count) && quality < 2) {
		shadows->enabled = 1;
		shadows->atlas_size = sizes[quality];
	}
}

static void FlightControls(AeronUiContext* ui, XwSettings* draft) {
	XwRenderSettings* r = &draft->render;
	AeronUi_Header(ui, "Flight Rendering");
	if (DosControls(draft)) {
		MsaaControl(ui, &r->dos_msaa_samples);
		return;
	}
	AeronUi_Toggle(ui, "Undither Cockpit Artwork", &r->cockpit_undither);
	if (XwHudAssets_UnditherPending(r->cockpit_undither))
		AeronUi_Help(ui, "This change will take effect on the next cockpit load.");
	static const char* const quality[] = { "Off", "Low", "High" };
	AeronUi_Selector(ui, "SSAO Quality", &r->scene.ssao.ssao_quality, quality, 3);
	ShadowControl(ui, &r->scene.shadows);
	static const AeronTemporalMode modes[] = { AERON_TEMPORAL_OFF, AERON_TEMPORAL_PERFORMANCE,
											   AERON_TEMPORAL_BALANCED, AERON_TEMPORAL_QUALITY,
											   AERON_TEMPORAL_NATIVE_AA };
	static const char* const labels[] = { "Off", "Performance", "Balanced", "Quality", "Native AA" };
	int index = 0;
	while (index < 4 && modes[index] != r->temporal_mode)
		++index;
	if (AeronUi_Selector(ui, "FSR Upscaling", &index, labels, 5)) {
		r->temporal_mode = modes[index];
		if (r->temporal_mode != AERON_TEMPORAL_OFF)
			r->msaa_samples = 1;
	}
	int samples = r->temporal_mode == AERON_TEMPORAL_OFF ? r->msaa_samples : 1;
	if (MsaaControl(ui, &samples)) {
		r->msaa_samples = samples;
		if (samples > 1)
			r->temporal_mode = AERON_TEMPORAL_OFF;
	}
	static const char* const blur[] = { "Off", "Low Quality", "High Quality" };
	AeronUi_Selector(ui, "Motion Blur", &r->motion_blur.quality, blur, 3);
	if (r->motion_blur.quality > 0) {
		int amount = (int)(r->motion_blur.shutter * 100.0f + 0.5f);
		if (AeronUi_SliderInt(ui, "Motion Blur Amount", &amount, 0, 100, 10, "%d%%"))
			r->motion_blur.shutter = amount / 100.0f;
	}
}

static void RestoreFlightDefaults(XwSettings* draft, const XwRenderSettings* defaults) {
	XwRenderSettings* r = &draft->render;
	if (DosControls(draft)) {
		r->dos_msaa_samples = defaults->dos_msaa_samples;
		return;
	}
	r->cockpit_undither = defaults->cockpit_undither;
	r->scene.ssao.ssao_quality = defaults->scene.ssao.ssao_quality;
	r->scene.shadows.enabled = defaults->scene.shadows.enabled;
	r->scene.shadows.atlas_size = defaults->scene.shadows.atlas_size;
	r->temporal_mode = defaults->temporal_mode;
	r->msaa_samples = defaults->temporal_mode == AERON_TEMPORAL_OFF ? defaults->msaa_samples : 1;
	r->motion_blur.quality = defaults->motion_blur.quality;
	r->motion_blur.shutter = defaults->motion_blur.shutter;
}

static void GameControls(AeronUiContext* ui, XwSettings* draft) {
	if (strcmp(draft->flight_version, "xw98"))
		return;
	static const char* const renderers[] = { "640x480 software", "640x480 hardware" };
	int renderer = draft->game.preferences.resolutionIndex == 3;
	AeronUi_Header(ui, "Original flight graphics");
	if (AeronUi_Selector(ui, "Renderer", &renderer, renderers, 2))
		draft->game.preferences.resolutionIndex = renderer ? 3 : 2;
	if (XwPreferences_RendererPending(draft->game.preferences.resolutionIndex))
		AeronUi_Help(ui, "Renderer changes apply next mission.");
}

void XwVideoPage_Draw(AeronUiContext* ui, const AeronInputSnapshot* input) {
	(void)input;
	XwSettings* draft = XwSettingsMenu_Draft();
	if (!AeronUi_BeginScroll(ui, "Video settings", XwSettingsMenu_ScrollHeight()))
		return;
	HostControls(ui, draft);
	FlightControls(ui, draft);
	GameControls(ui, draft);
	if (AeronUi_Button(ui, "Restore video defaults")) {
		const XwSettings* defaults = XwConfig_DefaultSettings();
		draft->fullscreen = defaults->fullscreen;
		draft->presentation.hdr_output = defaults->presentation.hdr_output;
		draft->presentation.sdr_gamma = defaults->presentation.sdr_gamma;
		draft->presentation.paper_white_nits = defaults->presentation.paper_white_nits;
		RestoreFlightDefaults(draft, &defaults->render);
		if (!strcmp(draft->flight_version, "xw98"))
			draft->game.preferences.resolutionIndex = defaults->game.preferences.resolutionIndex == 3 ? 3 : 2;
	}
	AeronUi_EndScroll(ui);
}
