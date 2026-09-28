#include "xw/flight/feinput.h"
#include "xw_runtime/snapshot/render_hud.h"

#include "xw/flight/flight.h"
#include "xw/flight/hud/festring.h"
#include "xw/flight/hud/msg.h"
#include "xw/flight/hud/panelrts.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/object/object.h"
#include "xw/flight/player/user.h"
#include "xw/flight/xw.h"
#include "xw/render/flight_view.h"
#include "xw_dos94/flight/hud/panel.h"
#include "xw_dos94/render/display.h"
#include "xw_runtime/runtime/port.h"
#include "xw_runtime/snapshot/render_assets.h"
#include "xw_runtime/storage/file_io.h"
#include "xw_runtime/storage/storage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t* viewData[20];
static size_t viewSizes[20];
static size_t sharedBytes;

static uint16_t word(const uint8_t* p) { return p[0] | (uint16_t)p[1] << 8; }

static uint32_t dword(const uint8_t* p) { return word(p) | (uint32_t)word(p + 2) << 16; }

static void fail(const char* path) {
	char message[160];
	snprintf(message, sizeof message, "X-Wing %d %s: missing or invalid cockpit resource",
			 XwGameVersion_Year(Dos94Assets_Version()), path);
	XwPort_Fail(message);
}

static uint8_t* read_file(const char* path, size_t* size, size_t maximum) {
	AeronFile* file = XwStorage_OpenFlight(path);
	int32_t length = file ? XwFile_Length(file) : -1;
	uint8_t* data = length > 0 && (size_t)length <= maximum ? malloc(length) : NULL;
	bool ok = data && XwFile_Read(data, 1, length, file) == (size_t)length;
	if (file)
		XwFile_Close(file);
	if (!ok) {
		free(data);
		fail(path);
		return NULL;
	}
	XwRenderSourceKind kind = strstr(path, ".INT")   ? XW_SOURCE_COCKPIT_LAYOUT
							  : strstr(path, ".PNL") ? XW_SOURCE_PANEL
													 : XW_SOURCE_COCKPIT;
	XwRenderAssets_RegisterBytes(kind, 0, XwStorage_LastPath(),
								 XwGameVersion_Year(Dos94Assets_Version()) - 1900, data, length);
	*size = length;
	return data;
}

static bool load_sprites(const char* name, unsigned first, unsigned count, size_t start) {
	char path[64];
	size_t size = 0, offset = 0;
	snprintf(path, sizeof path, "CP/%s.PNL", name);
	uint8_t* data = read_file(path, &size, DOS94_SPRITE_BYTES);
	if (!data)
		return false;
	XwCockpitAssets_Panel(XwRenderAssets_Find(XW_SOURCE_PANEL, strrchr(path, '/') + 1), first, count, 0);
	bool ok = first + count <= PANEL_HUD_SPRITE_COUNT && start <= DOS94_SPRITE_BYTES;
	for (unsigned i = 0; ok && i < count; ++i) {
		size_t length = 0;
		ok = Dos94Sprite_Size(data + offset, size - offset, &length) && length <= DOS94_SPRITE_BYTES - start;
		if (!ok)
			break;
		g_hudPanelSpriteDataByIndex[first + i] = Dos94_display->sprites + start;
		memcpy(Dos94_display->sprites + start, data + offset, length);
		start += length;
		offset += length;
	}
	free(data);
	if (!ok) {
		fail(path);
		return false;
	}
	g_hudPanelSpriteDataWriteCursor = Dos94_display->sprites + start;
	return true;
}

void Dos94Panel_Free(void) {
	for (unsigned i = 0; i < 20; ++i) {
		free(viewData[i]);
		viewData[i] = NULL;
		viewSizes[i] = 0;
	}
	memset(g_hudPanelSpriteDataByIndex, 0, sizeof g_hudPanelSpriteDataByIndex);
	g_hudPanelSpriteDataBuffer = NULL;
	g_hudPanelSpriteDataWriteCursor = NULL;
	g_hudPanelSpriteCraftDataStart = NULL;
	g_hudCockpitResourcesLoaded = 0;
	g_hudCockpitResourceWriteCursor = NULL;
	memset(g_hudCockpitResources, 0, sizeof g_hudCockpitResources);
	sharedBytes = 0;
}

void Dos94_panel_loadpaneldata(void) {
	Dos94Panel_Free();
	char path[64];
	size_t size = 0;
	snprintf(path, sizeof path, "CP/%.8s.INT",
			 g_craftTypeDefs[g_playerFlightState.craftTypeIndex].cockpitBaseName);
	uint8_t* data = read_file(path, &size, 1061);
	if (!data)
		return;
	if (size != 1061) {
		free(data);
		fail(path);
		return;
	}
	for (unsigned i = 0; i < 20; ++i) {
		const uint8_t* p = data + 30 * i;
		HudCockpitResourceDescriptor* v = &g_hudCockpitResourceDescriptors[i];
		v->enabled = p[0];
		memcpy(v->lfdName, p + 1, 9);
		v->lfdName[8] = 0;
		v->viewportX = (int16_t)word(p + 10);
		v->viewportY = (int16_t)word(p + 12);
		v->viewportWidth = (int16_t)word(p + 14);
		v->viewportHeight = (int16_t)word(p + 16);
		memcpy(v->viewLabel, p + 18, 12);
		v->viewLabel[11] = 0;
	}
	for (unsigned i = 0; i < 75; ++i) {
		const uint8_t* p = data + 600 + 6 * i;
		g_hudElementLayouts[i] = (HudElementLayout) { (int16_t)word(p), p[2], p[3], p[4] };
	}
	memcpy(&g_hudPanelSpriteFileInfo, data + 1050, 11);
	g_hudPanelSpriteFileInfo.baseName[8] = 0;
	free(data);
	g_hudPanelSpriteDataBuffer = Dos94_display->sprites;
	if (!load_sprites("PARTS", 0, 111, 0)) {
		Dos94Panel_Free();
		return;
	}
	sharedBytes = (size_t)(g_hudPanelSpriteDataWriteCursor - Dos94_display->sprites);
	g_hudPanelSpriteCraftDataStart = Dos94_display->sprites + sharedBytes;
	g_hudPanelSetId = 0;
	g_hudLoadedPanelSetId = 255;
	g_hudLoadedCockpitView = -1;
	/* Native allocations replace EMS handles; aliases resolve to the same allocation. */
	for (unsigned i = 0; i < 20; ++i)
		if (g_hudCockpitResourceDescriptors[i].enabled == 1) {
			snprintf(path, sizeof path, "CP/%s.LFD", g_hudCockpitResourceDescriptors[i].lfdName);
			viewData[i] = read_file(path, &viewSizes[i], DOS94_WORK_BYTES);
			if (!viewData[i]) {
				Dos94Panel_Free();
				return;
			}
		}
	g_hudCockpitResourcesLoaded = 1;
}

void Dos94Panel_RestoreCraftSprites(void) {
	if (!load_sprites(g_hudPanelSpriteFileInfo.baseName, 111,
					  g_hudPanelSpriteFileInfo.spriteCount + g_hudPanelSpriteFileInfo.spriteCountAddend,
					  sharedBytes))
		return;
	g_hudLoadedPanelSetId = g_hudPanelSetId;
}

/* 0x6C276E: zero extends a run by 256, including a final zero for length 256. */
static bool Dos94_panel_copymaskdata(const uint8_t* data, size_t size, unsigned width, unsigned height,
									 bool mirror) {
	uint8_t* dest = Dos94_display->mask;
	size_t input = 0, output = 0;
	for (unsigned y = 0; y < height; ++y) {
		uint16_t runs[99];
		unsigned count = 0, pixels = 0;
		if (input >= size)
			return false;
		uint8_t state = data[input++];
		while (pixels < width) {
			if (input >= size || count == 99)
				return false;
			unsigned run = data[input++];
			if (!run) {
				if (input >= size)
					return false;
				run = 256 + data[input++];
			}
			if (run > width - pixels)
				return false;
			runs[count++] = run;
			pixels += run;
		}
		if (output + 1 + 2 * count > DOS94_MASK_BYTES)
			return false;
		dest[output++] = mirror && !(count & 1) ? (uint8_t)-state : state;
		for (unsigned i = 0; i < count; ++i) {
			unsigned run = runs[mirror ? count - 1 - i : i];
			if (run >= 256) {
				dest[output++] = 0;
				run -= 256;
			}
			dest[output++] = (uint8_t)run;
		}
	}
	return true;
}

static void Dos94_panel_clearmaskdata(unsigned width, unsigned height) {
	uint8_t* dest = Dos94_display->mask;
	for (unsigned y = 0; y < height; ++y) {
		*dest++ = 1;
		if (width >= 256)
			*dest++ = 0;
		*dest++ = (uint8_t)width;
	}
}

static bool draw_view(const uint8_t* data, size_t size, unsigned width, unsigned height, unsigned offset,
					  bool mirror) {
	const uint8_t* entries[3];
	size_t lengths[3], cursor = 0;
	for (unsigned i = 0; i < 3; ++i) {
		if (size - cursor < 16)
			return false;
		lengths[i] = dword(data + cursor + 12);
		cursor += 16;
		if (lengths[i] > size - cursor)
			return false;
		entries[i] = data + cursor;
		cursor += lengths[i];
	}
	size_t spriteSize;
	if (lengths[2] < 194 || !Dos94Sprite_Size(entries[0], lengths[0], &spriteSize) || !width || width > 320 ||
		!height || height > 190 || offset % 320 + width > 320 || offset / 320 + height > 200)
		return false;
	RgbTriplet palette[64];
	for (unsigned i = 0; i < sizeof palette; ++i)
		((uint8_t*)palette)[i] = entries[2][i + 2] >> 2;
	Dos94_rtsvga2_buildpaletteVGA(palette, 0, 64);
	Dos94_rtsvga2_drawshapeVGA(entries[0], mirror ? 319 : 0, 0, 253, mirror);
	Dos94_SetFlightViewport(width, height, 0, offset);
	return Dos94_panel_copymaskdata(entries[1], lengths[1], width, height, mirror);
}

void Dos94_panel_forcenewviewdir(uint16_t view) {
	msg_messageinit();
	g_hudLoadedCockpitView = -1;
	g_flightCamera.hudStateLive = 255;
	g_hudLoadedPanelSetId = 255;
	panelrts_setnewpilotview(view);
}

void Dos94_panel_dosetnewpilotview(uint16_t view) {
#ifdef XW_MODERN
	int hud_pane = XwHud_Push(XW_SNAP_PANE_VIEW_LABEL);
#endif

	if (view >= 20 || !Dos94_display || !g_hudCockpitResourcesLoaded) {
#ifdef XW_MODERN
		XwHud_Pop(hud_pane);
#endif
		return;
	}
	unsigned enabled = g_hudCockpitResourceDescriptors[view].enabled;
	bool mirror = view != 18 && enabled >= 192;
	unsigned resource = view == 18       ? 18
						: enabled >= 192 ? enabled - 192
						: enabled >= 128 ? enabled - 128
										 : view;
	if (resource >= 20) {
#ifdef XW_MODERN
		XwHud_Pop(hud_pane);
#endif
		return;
	}
	if (mirror)
		g_hudLoadedCockpitView = -1;
	g_hudCockpitMirrorHorizontal = mirror;
	if (g_hudLoadedCockpitView != (int)resource) {
		Dos94_rtsvga2_blankVGA();
		if (g_hudLoadedPanelSetId != g_hudPanelSetId)
			Dos94Panel_RestoreCraftSprites();
		if (g_quitRequested) {
#ifdef XW_MODERN
			XwHud_Pop(hud_pane);
#endif
			return;
		}
		if (view == 18) {
			festring_setbound(0, 0, 320, 189);
			festring_setbackcolor(0);
			Dos94_rtsvga2_clearwindowVGA();
			Dos94_SetFlightViewport(320, 189, 0, 0);
			Dos94_panel_clearmaskdata(320, 189);
		} else {
			HudCockpitResourceDescriptor* v = &g_hudCockpitResourceDescriptors[mirror ? view : resource];
			uint8_t* data = viewData[resource];
			size_t size = viewSizes[resource];
			bool borrowed = !data;
			char path[64];
			snprintf(path, sizeof path, "CP/%s.LFD", g_hudCockpitResourceDescriptors[resource].lfdName);
			if (borrowed) {
				uint8_t* loaded = read_file(path, &size, DOS94_WORK_BYTES);
				if (!loaded) {
#ifdef XW_MODERN
					XwHud_Pop(hud_pane);
#endif
					return;
				}
				memcpy(Dos94_display->logical, loaded, size);
				free(loaded);
				data = Dos94_display->logical;
			}
			if (!draw_view(data, size, v->viewportWidth, v->viewportHeight, v->viewportY * 320 + v->viewportX,
						   mirror)) {
				fail(path);
				{
#ifdef XW_MODERN
					XwHud_Pop(hud_pane);
#endif
					return;
				}
			}
		}
		g_projOffsetY = view == 19 ? (g_playerFlightState.craftTypeIndex == 1 ? -38 : -30)
								   : (g_playerFlightState.object->objectType == 2 && view == 0 ? -5 : 0);
		g_hudLoadedCockpitView = mirror ? -1 : (int16_t)resource;

		char base[16];
		snprintf(base, sizeof base, "%.8s.LFD", g_hudCockpitResourceDescriptors[resource].lfdName);
		const HudCockpitResourceDescriptor* descriptor = &g_hudCockpitResourceDescriptors[resource];
		XwHud_View(view, view == 18 ? NULL : base,
				   (XwSnapRect) { descriptor->viewportX, descriptor->viewportY, descriptor->viewportWidth,
								  descriptor->viewportHeight },
				   mirror);
		panel_initpanel();
		Dos94_display->fullUpdate = true;
		Dos94_rtsvga2_unblankVGA();
		calcframerate = 0;
	}
	if (resource == 17) {
		festring_setbound(139, 6, 182, 12);
		festring_setbackcolor(0x40);
		Dos94_rtsvga2_clearwindowVGA();
		festring_settextcolor(0x49);
		festring_setcursor(0, 6);
		festring_outstringcenter(g_hudCockpitResourceDescriptors[view].viewLabel);
	}

#ifdef XW_MODERN
	XwHud_Pop(hud_pane);
#endif
#ifdef XW_MODERN
	XwHud_ViewSelection(view);
#endif
}

void Dos94Panel_Replay(bool standalone) {
	if (!Dos94_display || !g_hudCockpitResourcesLoaded || !load_sprites("CAMERAP", 111, 38, sharedBytes))
		return;
	g_hudLoadedPanelSetId = 255;
	size_t size = 0;
	const char* path = standalone ? "CP/FILM.LFD" : "CP/CAMERA.LFD";
	uint8_t* data = read_file(path, &size, DOS94_WORK_BYTES);
	if (!data)
		return;
	memcpy(Dos94_display->logical, data, size);
	free(data);
	if (!draw_view(Dos94_display->logical, size, 320, standalone ? 123 : 135, 320 * (standalone ? 8 : 17),
				   false))
		fail(path);
	g_projOffsetY = 0;
	XwHud_View(18, standalone ? "FILM.LFD" : "CAMERA.LFD",
			   (XwSnapRect) { 0, standalone ? 8 : 17, 320, standalone ? 123 : 135 }, false);
}
