#include "xw/render/flight_screenshot.h"

#include "xw/assets/file.h"
#include "xw/flight/flight_display.h"
#include "xw/util/front_image.h"

#ifdef XW_MODERN
#include "xw_dos94/render/display.h"
#include "xw_runtime/runtime/presentation.h"
#endif
#include <limits.h>
#include <stdio.h>

// FUNCTION: XW 0x4AF020
void FlightScreenshot_Capture(void) {
	char fileName[FLIGHT_SCREENSHOT_NAME_CAPACITY];
	AeronRgbQuad palette[FLIGHT_DISPLAY_PALETTE_ENTRIES];
	int paletteIndex;
	int lockCount;
	int lockIndex;
	int savedOffscreenFlag;
	unsigned int savedDisplayMode;
	{
		int screenshotIndex = 0;
		XwFile* existingFile;
		do {
			sprintf(fileName, "tiescreen%d.bmp", screenshotIndex);
			existingFile = File_RawOpen(fileName, "rb");
			if (existingFile != NULL) {
				File_RawClose(existingFile);
				++screenshotIndex;
			}
		} while (existingFile != NULL);
	}
#ifdef XW_MODERN
	if (XwPresentation_BaseSource() == XW_PRESENT_DOS_FLIGHT) {
		const Dos94Display* d = Dos94_display;
		unsigned i;
		if (!d || !d->generation)
			return;
		for (i = 0; i < 256; ++i) {
			const uint8_t* rgb = (const uint8_t*)&d->palette[i];
			palette[i] = (AeronRgbQuad) { rgb[2], rgb[1], rgb[0], 0 };
		}
		FrontImage_SaveBmpFile(fileName, d->published, 320, 200, 320, 8, 0, palette);
		return;
	}
#endif
	for (paletteIndex = 0; paletteIndex < FLIGHT_DISPLAY_PALETTE_ENTRIES; ++paletteIndex) {
		palette[paletteIndex].rgbBlue = g_directDrawPaletteEntries[paletteIndex].blue;
		palette[paletteIndex].rgbGreen = g_directDrawPaletteEntries[paletteIndex].green;
		palette[paletteIndex].rgbRed = g_directDrawPaletteEntries[paletteIndex].red;
		palette[paletteIndex].rgbReserved = 0;
	}
	lockCount = FlightDisplay_GetSurfaceLockCount();
	for (lockIndex = lockCount; lockIndex > 0; --lockIndex) {
		FlightDisplay_UnlockSurface();
	}
	FlightDisplay_Flip();
	savedOffscreenFlag = g_flightLockOffscreenSurface;
	savedDisplayMode = (uint16_t)g_flightDisplaySurfaceMode;
	g_flightLockOffscreenSurface = 0;
	g_flightDisplaySurfaceMode = 1;
	FlightDisplay_LockSurface();
	FrontImage_SaveBmpFile(fileName, g_surfacePixels, g_surfaceWidth, g_surfaceHeight, g_surfacePitch,
						   CHAR_BIT * g_flightBytesPerPixel, FlightDisplay_IsPixelFormat555(), palette);
	FlightDisplay_UnlockSurface();
	g_flightLockOffscreenSurface = savedOffscreenFlag;
	g_flightDisplaySurfaceMode = savedDisplayMode;
	FlightDisplay_Flip();
	for (lockIndex = lockCount; lockIndex > 0; --lockIndex) {
		FlightDisplay_LockSurface();
	}
}
