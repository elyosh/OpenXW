#include "xw/util/landru_display.h"

#include "xw/flight/flight.h"
#include "xw/flight/flight_display.h"
#include "xw/input/landru_mouse.h"
#include "xw/landru_config.h"
#include "xw/util/shared.h"

#ifdef XW_MODERN
#include "xw_runtime/integration/landru_adapter.h"
#include "xw_runtime/platform/classic_surfaces.h"
#include <landru/surface.h>
#endif
#include <landru/bitmap.h>
#include <landru/canvas.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4DA860
int g_landruLogicalWidth = FLIGHT_DISPLAY_WIDTH;

// GLOBAL: XW 0x4DA864
int g_landruLogicalHeight = FLIGHT_DISPLAY_HEIGHT;

// GLOBAL: XW 0x566850
uint8_t* g_frontendSavedBackground = NULL;

// GLOBAL: XW 0x56691C
int g_landruBorderClearCountdown = 0;

// GLOBAL: XW 0x5BECAC
int g_landruLegacyViewportState = 0;

// GLOBAL: XW 0x5BECC4
int g_landruDoublePixelsEnabled = 0;

// FUNCTION: XW 0x49DF80
void LandruDisplay_SetLowResolutionMode(int enabled) {
	if (enabled != 0) {
		LandruDisplay_SetLogicalViewport(1, LANDRU_LOW_RESOLUTION_WIDTH, LANDRU_LOW_RESOLUTION_HEIGHT);
	} else {
		LandruDisplay_SetLogicalViewport(0, FLIGHT_DISPLAY_WIDTH, FLIGHT_DISPLAY_HEIGHT);
	}
}

// FUNCTION: XW 0x49DFC0
void LandruDisplay_SetLogicalViewport(int doublePixels, int width, int height) {
#ifdef XW_MODERN
	XwLandru_SetLogicalViewport(doublePixels, width, height);
#else
	g_landruDoublePixelsEnabled = doublePixels;
	g_landruLegacyViewportState = LANDRU_LEGACY_VIEWPORT_STATE;
	g_landruLogicalWidth = width;
	g_landruLogicalHeight = height;
	LandruMouse_SetBoundsStub(0, (int16_t)width);
	LandruMouse_SetBoundsStub(0, (int16_t)height);
#endif
}

// FUNCTION: XW 0x49E050
void LandruDisplay_ForwardLegacyNoOp(int value) {
	/* All six original arguments are unused by the folded backend. */
	(void)value;
	nullsub_SharedNoOp();
}

// FUNCTION: XW 0x4AE030
void LandruDisplay_CopyCanvasToBackBuffer(void) {
#ifdef XW_MODERN
	const uint8_t* canvas;
	DDSURFACEDESC desc;
	if (g_quitRequested || xsurface_Uses_Native_Vga_Presentation())
		return;
	canvas = xcanvas_Get_Draw_Buffer();
	if (!canvas)
		return;
	XwDisplay_LockRaw(g_flightBackBuffer, &desc);
	Raster_BlitIndexed8Clipped(desc.lpSurface, desc.lPitch, (int)desc.dwHeight, canvas, FLIGHT_DISPLAY_WIDTH,
							   FLIGHT_DISPLAY_HEIGHT, FLIGHT_DISPLAY_WIDTH, 0, 0, LANDRU_RASTER_COPY);
	XwDisplay_UnlockRaw(g_flightBackBuffer, desc.lpSurface);
#else
	DDSURFACEDESC surfaceDesc;
	while (g_windowActive == 0) {
		if (g_quitRequested != 0)
			return;
		Flight_PumpWindowMessages();
	}
	if (g_quitRequested != 0)
		return;
	if (g_landruDoublePixelsEnabled != 0) {
		HRESULT result;
		memset(&surfaceDesc, 0, sizeof(surfaceDesc));
		surfaceDesc.dwSize = sizeof(surfaceDesc);
		result = g_flightBackBuffer->lpVtbl->Lock(g_flightBackBuffer, NULL, &surfaceDesc,
												  DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL);
		if (result == DX_DDERR_SURFACELOST) {
			FlightDisplay_RestorePrimarySurface();
			g_landruBorderClearCountdown = LANDRU_BORDER_CLEAR_FRAMES;
			return;
		}
		if (result == 0) {
			if (g_landruBorderClearCountdown != 0) {
				ptrdiff_t row;
				ptrdiff_t rowOffset;
				--g_landruBorderClearCountdown;
				for (row = 0, rowOffset = 0; row < LANDRU_BORDER_ROWS;
					 ++row, rowOffset += surfaceDesc.lPitch) {
					memset(&((uint8_t*)surfaceDesc.lpSurface)[rowOffset], 0, FLIGHT_DISPLAY_WIDTH);
				}
				for (row = 0, rowOffset = (ptrdiff_t)LANDRU_BOTTOM_BORDER_ROW * surfaceDesc.lPitch;
					 row < LANDRU_BORDER_ROWS; ++row, rowOffset += surfaceDesc.lPitch) {
					memset(&((uint8_t*)surfaceDesc.lpSurface)[rowOffset], 0, FLIGHT_DISPLAY_WIDTH);
				}
			}
			Raster_BlitIndexed8Clipped(surfaceDesc.lpSurface, surfaceDesc.lPitch, FLIGHT_DISPLAY_HEIGHT,
									   xcanvas_Get_Draw_Buffer(), LANDRU_LOW_RESOLUTION_WIDTH,
									   LANDRU_LOW_RESOLUTION_HEIGHT, FLIGHT_DISPLAY_WIDTH, 0, 0,
									   LANDRU_RASTER_DOUBLE);
			g_flightBackBuffer->lpVtbl->Unlock(g_flightBackBuffer, surfaceDesc.lpSurface);
		}
	} else {
		HRESULT result;
		memset(&surfaceDesc, 0, sizeof(surfaceDesc));
		surfaceDesc.dwSize = sizeof(surfaceDesc);
		result = g_flightBackBuffer->lpVtbl->Lock(g_flightBackBuffer, NULL, &surfaceDesc,
												  DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL);
		if (result == DX_DDERR_SURFACELOST) {
			FlightDisplay_RestorePrimarySurface();
			g_landruBorderClearCountdown = LANDRU_BORDER_CLEAR_FRAMES;
			return;
		}
		if (result == 0) {
			if (g_landruBorderClearCountdown != 0)
				--g_landruBorderClearCountdown;
			Raster_BlitIndexed8Clipped(surfaceDesc.lpSurface, surfaceDesc.lPitch, FLIGHT_DISPLAY_HEIGHT,
									   xcanvas_Get_Draw_Buffer(), FLIGHT_DISPLAY_WIDTH, FLIGHT_DISPLAY_HEIGHT,
									   FLIGHT_DISPLAY_WIDTH, 0, 0, LANDRU_RASTER_COPY);
			g_flightBackBuffer->lpVtbl->Unlock(g_flightBackBuffer, surfaceDesc.lpSurface);
		}
	}
#endif
}

// FUNCTION: XW 0x4AED30
void LandruDisplay_SaveCanvasBackground(void) {
	if (g_flightAuxiliarySurface != NULL) {
		g_frontendSavedBackground = malloc((uint32_t)g_flightBytesPerPixel * FLIGHT_DISPLAY_WIDTH *
										   FLIGHT_DISPLAY_HEIGHT * sizeof(*g_frontendSavedBackground));
		if (g_frontendSavedBackground != NULL) {
			const uint8_t* canvasPixels = xcanvas_Get_Draw_Buffer();
			memcpy(g_frontendSavedBackground, canvasPixels,
				   FLIGHT_DISPLAY_WIDTH * FLIGHT_DISPLAY_HEIGHT * sizeof(*g_frontendSavedBackground));
		}
	}
}

// FUNCTION: XW 0x4AED80
void LandruDisplay_RestoreCanvasBackground(void) {
	uint8_t* savedPixels = g_frontendSavedBackground;
	if (savedPixels != NULL) {
		if (g_flightAuxiliarySurface != NULL) {
			memcpy(xcanvas_Get_Draw_Buffer(), savedPixels,
				   FLIGHT_DISPLAY_WIDTH * FLIGHT_DISPLAY_HEIGHT * sizeof(*savedPixels));
			savedPixels = g_frontendSavedBackground;
		}
		free(savedPixels);
		g_frontendSavedBackground = NULL;
	}
}
