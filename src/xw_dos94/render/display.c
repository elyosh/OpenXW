#include "xw_dos94/render/display.h"
#include "xw_runtime/snapshot/render_hud.h"

#include "xw/flight/feinput.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/hud/festring.h"
#include "xw/flight/player/user.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/render/flight_view.h"
#include "xw/render/renderer.h"
#include "xw/render/rtsvga2.h"
#include "xw/render/sw3d.h"
#include "xw_runtime/runtime/presentation.h"
#include "xw_runtime/runtime/profile.h"
#include "xw_runtime/snapshot/render_assets.h"
#include "xw_runtime/storage/file_io.h"
#include "xw_runtime/storage/storage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Dos94Display* Dos94_display;
static uint32_t publishedGeneration;

/* gamesnd_Host_Timer_Tick (0x11370), RGB6 table at 0x1128B. */
static const RgbTriplet engineGlowColors94[5][3] = {
	{ { 0x00, 0x3F, 0x00 }, { 0x3F, 0x20, 0x1F }, { 0x1A, 0x39, 0x3F } },
	{ { 0x00, 0x30, 0x00 }, { 0x3F, 0x1A, 0x15 }, { 0x11, 0x32, 0x3F } },
	{ { 0x00, 0x22, 0x00 }, { 0x3F, 0x14, 0x0E }, { 0x08, 0x29, 0x3F } },
	{ { 0x00, 0x14, 0x00 }, { 0x3F, 0x1A, 0x15 }, { 0x11, 0x32, 0x3F } },
	{ { 0x00, 0x06, 0x00 }, { 0x3F, 0x20, 0x1F }, { 0x1A, 0x39, 0x3F } },
};

/* DOS93 gamesnd_Host_Timer_Tick (0x11470), RGB6 table at 0x1138B. */
static const RgbTriplet engineGlowColors93[5][3] = {
	{ { 0, 31, 0 }, { 31, 15, 7 }, { 7, 15, 31 } }, { { 0, 24, 0 }, { 29, 7, 3 }, { 3, 7, 29 } },
	{ { 0, 17, 0 }, { 16, 3, 0 }, { 0, 3, 16 } },   { { 0, 10, 0 }, { 8, 0, 0 }, { 0, 0, 8 } },
	{ { 0, 3, 0 }, { 16, 0, 0 }, { 0, 0, 16 } },
};

/* Each revision retains its resident timer countdown and phase between missions. */
static struct {
	uint32_t elapsedUs, phase;
} paletteCycles[2];

void Dos94Display_ResetPaletteCycle(void) { memset(paletteCycles, 0, sizeof paletteCycles); }

void Dos94Display_AdvancePalette(int32_t deltaUs) {
	static const XwGameVersion versions[2] = { XW_GAME_VERSION_94, XW_GAME_VERSION_93 };
	if (deltaUs <= 0)
		return;
	for (unsigned i = 0; i < 2; ++i) {
		uint32_t period = 18u * XwProfile_Flight(versions[i])->tick_period_us;
		uint64_t elapsed = (uint64_t)paletteCycles[i].elapsedUs + (uint32_t)deltaUs;
		uint32_t steps = (uint32_t)(elapsed / period);
		paletteCycles[i].elapsedUs = (uint32_t)(elapsed % period);
		/* Countdowns reload even while disabled; only the active color phase advances. */
		if (!steps || !Dos94_display || !Dos94_display->ready || !XwProfile_HasActiveFlight() ||
			XwProfile_ActiveFlight()->version != versions[i] ||
			!(g_engineGlowEnabled & g_paletteCycleEnabled))
			continue;
		paletteCycles[i].phase = (paletteCycles[i].phase + steps) % 5;
		const RgbTriplet(*colors)[3] = i == 0 ? engineGlowColors94 : engineGlowColors93;
		/* Retail writes the DAC directly, leaving the saved base palette intact. */
		memcpy(Dos94_display->effective + 0xF4, colors[paletteCycles[i].phase], sizeof colors[0]);
	}
}

void Dos94Display_Restore(void) {
	g_flightScreenWidth = 320;
	g_flightScreenHeight = 200;
	g_flightResolutionMode = FLIGHT_DISPLAY_MODE_13H;
	g_flightBytesPerPixel = 1;
	g_useHardware3D = 0;
	g_modelTextureQuality = 0;
	g_sw3dSkipOddScanlines = 0;
	g_palettePackedMode = FEINPUT_RENDER_INDEXED_LOW;
	Dos94Display_BindCallbacks();
	Dos94_festring_setfontsize(1);
	Dos94_display->fullUpdate = true;
	XwPresentation_SelectBaseSource(XW_PRESENT_DOS_FLIGHT);
}

static bool read_asset(const char* name, uint8_t* data, size_t capacity, size_t minimum, char* error,
					   size_t errorSize) {
	AeronFile* file = XwStorage_OpenFlight(name);
	int32_t size = file ? XwFile_Length(file) : -1;
	bool ok = size > 0 && (size_t)size >= minimum && (size_t)size <= capacity &&
			  XwFile_Read(data, 1, size, file) == (size_t)size;
	if (file)
		XwFile_Close(file);
	if (ok)
		XwRenderAssets_RegisterBytes(strstr(name, "FNT") ? XW_SOURCE_FONT : XW_SOURCE_PALETTE, 0,
									 XwStorage_LastPath(), XwGameVersion_Year(Dos94Assets_Version()) - 1900,
									 data, size);
	if (!ok)
		snprintf(error, errorSize, "X-Wing %d %s: missing or invalid resource",
				 XwGameVersion_Year(Dos94Assets_Version()), name);
	return ok;
}

bool Dos94Display_Init(char* error, size_t capacity) {
	Dos94Display_Free();
	Dos94_display = calloc(1, sizeof *Dos94_display);
	if (!Dos94_display) {
		snprintf(error, capacity, "X-Wing %d: cannot allocate display workspaces",
				 XwGameVersion_Year(Dos94Assets_Version()));
		return false;
	}
	Dos94Raster_Init(&Dos94_display->raster);
	uint8_t palette[577] = { 0 };
	/* DOS93 MICRO.FNT adds eight unused bytes to the shared 748-byte font. */
	if (!read_asset("TINY.FNT", Dos94_display->tiny, sizeof Dos94_display->tiny, 1024, error, capacity) ||
		!read_asset("MICRO.FNT", Dos94_display->micro, sizeof Dos94_display->micro, 748, error, capacity) ||
		!read_asset("VGA.PAC", palette, sizeof palette, 576, error, capacity)) {
		Dos94Display_Free();
		return false;
	}
	memcpy(Dos94_display->stored + 64, palette, 576);
	Dos94_display->ready = true;
	Dos94_display->fullUpdate = true;
	Dos94Display_Restore();
	Dos94_rtsvga2_blankVGA();
	return true;
}

void Dos94Display_Free(void) {
	XwHud_ClearSurface(XW_RENDER_SURFACE_DOS);
	if (Dos94_display)
		XwPresentation_Invalidate();
	if (Dos94_display &&
		(g_flightFontGlyphTableSw == Dos94_display->tiny || g_flightFontGlyphTableSw == Dos94_display->micro))
		g_flightFontGlyphTableSw = NULL;
	bool hadDisplay = Dos94_display != NULL;
	Dos94Display* display = Dos94_display;
	Dos94_display = NULL;
	if (hadDisplay)
		feinput_SetGraphicsPtrs(g_palettePackedMode);
	free(display);
}

void Dos94Display_Publish(void) {
	Dos94Display* d = Dos94_display;
	if (!d || !d->ready)
		return;
	bool changed = !d->generation || memcmp(d->screen, d->published, sizeof d->screen);
	for (unsigned i = 0; i < 256; ++i) {
		const uint8_t* c = (const uint8_t*)&d->effective[i];
		AeronPaletteEntry p = { (uint8_t)((c[0] << 2) | (c[0] >> 4)), (uint8_t)((c[1] << 2) | (c[1] >> 4)),
								(uint8_t)((c[2] << 2) | (c[2] >> 4)), 255 };
		if (memcmp(&p, &d->palette[i], sizeof p))
			changed = true;
		d->palette[i] = p;
	}
	if (changed) {
		memcpy(d->published, d->screen, sizeof d->screen);
		if (!++publishedGeneration)
			++publishedGeneration;
		d->generation = publishedGeneration;
	}
}

void Dos94Display_Submit(void) {
	Dos94Display* d = Dos94_display;
	if (!d || !d->generation || Aeron_FatalErrorRequested())
		return;
	const AeronPixelLayerDesc layer = { .frame = { .pixels = d->published,
												   .width = 320,
												   .height = 200,
												   .pitch = 320,
												   .bpp = 8,
												   .format = AERON_PIXEL_FORMAT_INDEX8,
												   .color_space = AERON_COLOR_SPACE_SRGB,
												   .palette = d->palette,
												   .generation = d->generation },
										.logical_rect = XwPresentation_ClassicRect(),
										.blend_mode = AERON_LAYER_BLEND_OPAQUE,
										.sampling = AERON_PIXEL_SAMPLING_SHARP_BILINEAR };
	if (!Aeron_SubmitPixelLayer(&layer)) {
		char error[80];
		snprintf(error, sizeof error, "X-Wing %d VGA frame submission",
				 XwGameVersion_Year(Dos94Assets_Version()));
		Aeron_RequestFatalRendererError(error);
	}
}

void Dos94_rtsvga2_blankVGA(void) {
	g_paletteCycleEnabled = 0;
	g_paletteBlankFlags |= 1;
	memset(Dos94_display->effective, 0, sizeof Dos94_display->effective);
}

void Dos94_rtsvga2_unblankVGA(void) {
	g_paletteCycleEnabled = 1;
	g_paletteBlankFlags &= ~1;
	memcpy(Dos94_display->effective, Dos94_display->stored, sizeof Dos94_display->stored);
}

void Dos94_rtsvga2_buildpaletteVGA(const RgbTriplet* source, uint16_t start, uint16_t count) {
	if (start <= 256 && count <= 256 - start)
		memcpy(Dos94_display->stored + start, source, count * sizeof *source);
}

void Dos94_rtsvga2_savepaletteVGA(RgbTriplet* dest) {
	memcpy(dest, Dos94_display->stored, sizeof Dos94_display->stored);
}

void Dos94_rtsvga2_restorepaletteVGA(const RgbTriplet* source) {
	memcpy(Dos94_display->stored, source, sizeof Dos94_display->stored);
}

/* 0x6A6AEF: fixed VGA pitch and packed logical pitch are distinct. */
void Dos94_SetFlightViewport(uint16_t width, uint16_t height, int unused, unsigned int offset) {
	(void)unused;
	if (!width || width > 320 || !height || height > 190 || offset >= DOS94_SCREEN_BYTES ||
		offset % 320 + width > 320 || offset / 320 + height > 200)
		return;
	Dos94Display* d = Dos94_display;
	g_flightVpWidth = width;
	g_flightVpHeight = height;
	g_flightVpMaxX = width - 1;
	g_flightVpMaxY = height - 1;
	g_flightVpCenterX = width / 2;
	g_flightVpCenterY = height / 2;
	g_flightVpBaseOffset = offset;
	g_flightVpX = offset % 320;
	g_flightVpY = offset / 320;
	d->rowStride = 2 * (width / 2);
	d->byteArea = (uint16_t)(height * d->rowStride);
	if (!++d->viewportGeneration)
		++d->viewportGeneration;
	d->fullUpdate = true;
}

void Dos94_LOGBUF2_clearbuffer(uint8_t color) {
	memset(Dos94_display->logical, color, Dos94_display->byteArea);
}

static void initgraph(void) { memset(Dos94_display->screen, 0, DOS94_SCREEN_BYTES); }

void Dos94Display_BindCallbacks(void) {
	g_flightInitLineBufferFn = initgraph;
	g_flightRenderTransitionHook = Dos94_rtsvga2_blankVGA;
	g_flightResetPaletteFn = Dos94_rtsvga2_unblankVGA;
	g_flightSetPaletteRangeFn = Dos94_rtsvga2_buildpaletteVGA;
	g_flightGetPaletteFn = Dos94_rtsvga2_savepaletteVGA;
	g_flightSetPaletteFn = Dos94_rtsvga2_restorepaletteVGA;
	g_flightComputePixelOffsetFn = Dos94_rtsvga2_calcpositionVGA;
	g_flightBlitSpriteFn = Dos94_rtsvga2_drawshapeVGA;
	g_flightDrawCharFn = Dos94_rtsvga2_outcharVGA;
	g_flightFillClipRectFn = Dos94_rtsvga2_clearwindowVGA;
	g_flightFillRectClippedFn = Dos94_rtsvga2_fillboxVGA;
	g_flightSaveScreenRectFn = Dos94_rtsvga2_saveboxVGA;
	g_flightRestoreScreenRectFn = Dos94_rtsvga2_restoreboxVGA;
}
