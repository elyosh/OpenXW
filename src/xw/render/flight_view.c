#include "xw/render/flight_view.h"

#include "xw/flight/fediskio.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/hud/panel.h"
#include "xw/render/std3d.h"
#ifdef XW_MODERN
#include "xw_runtime/snapshot/render_capture.h"
#endif

#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x5BECD0
uint16_t g_flightVpHeight = 0;

// GLOBAL: XW 0x5BECD2
uint16_t g_flightVpCenterY = 0;

// GLOBAL: XW 0x5BECD4
unsigned int g_flightVpY = 0;

// GLOBAL: XW 0x5BECD8
unsigned int g_flightVpX = 0;

// GLOBAL: XW 0x5BECDC
uint16_t g_flightVpMaxX = 0;

// GLOBAL: XW 0x5BECDE
uint16_t g_flightVpMaxY = 0;

// GLOBAL: XW 0x5BECE0
uint16_t g_flightVpWidth = 0;

// GLOBAL: XW 0x5BECE8
unsigned int g_flightVpBaseOffset = 0;

// GLOBAL: XW 0x5BECEC
uint16_t g_flightVpCenterX = 0;

// GLOBAL: XW 0x62ABE0
XwFlightCamera g_flightCamera = { 0 };

// GLOBAL: XW 0x62AD78
int g_projScaleHalfInt = 0;

// GLOBAL: XW 0x62BB14
uint8_t g_projPerspectiveShift = 0;

// GLOBAL: XW 0x62C926
uint16_t g_projAspectY = 0;

// GLOBAL: XW 0x6377A0
int g_projScaleInt = 0;

// GLOBAL: XW 0x6377C4
int g_projOffsetY = 0;

// FUNCTION: XW 0x482690
HRESULT FlightView_CompositeMaskedSoftwareSurface(void) {
	DDSURFACEDESC surfaceDesc;
	HRESULT result;
	int32_t sourcePitch;
	int32_t destPitch;
	uint8_t* sourcePixels;
	uint8_t* destPixels;
	uint8_t* centeredDestPixels;
	const uint8_t* mask;
	size_t maskIndex;
	unsigned int rowIndex;
	int viewportRow;
	ptrdiff_t sourceOffset;
	ptrdiff_t destOffset;
	memset(&surfaceDesc, 0, sizeof(surfaceDesc));
	surfaceDesc.dwSize = sizeof(surfaceDesc);
	do {
		result = g_flightBackBuffer->lpVtbl->Lock(g_flightBackBuffer, NULL, &surfaceDesc, 0, NULL);
		if (result != DX_DD_OK && result != DX_DDERR_WASSTILLDRAWING) {
#ifdef XW_MODERN
			XwRenderCapture_CancelView();
#endif
			return result;
		}
#ifdef XW_MODERN
		if (result != DX_DD_OK) {
			XwRenderCapture_CancelView();
			return result;
		}
#endif
	} while (result != DX_DD_OK);
	destPitch = surfaceDesc.lPitch;
	destPixels = surfaceDesc.lpSurface;
	memset(&surfaceDesc, 0, sizeof(surfaceDesc));
	surfaceDesc.dwSize = sizeof(surfaceDesc);
	do {
		result =
			g_flightOffscreenSurface->lpVtbl->Lock(g_flightOffscreenSurface, NULL, &surfaceDesc, 0, NULL);
		if (result != DX_DD_OK && result != DX_DDERR_WASSTILLDRAWING) {
#ifdef XW_MODERN
			XwRenderCapture_CancelView();
#endif
			return result;
		}
#ifdef XW_MODERN
		if (result != DX_DD_OK) {
			XwRenderCapture_CancelView();
			return result;
		}
#endif
	} while (result != DX_DD_OK);
	sourcePitch = surfaceDesc.lPitch;
	sourcePixels = surfaceDesc.lpSurface;
	centeredDestPixels =
		destPixels +
		destPitch * (((unsigned int)g_flightDisplayHeight - (unsigned int)g_surfaceHeight) >> 1) +
		g_flightBytesPerPixel * (((unsigned int)g_flightDisplayWidth - (unsigned int)g_surfaceWidth) >> 1);
	sourceOffset = 0;
	destOffset = 0;
	for (rowIndex = 0; rowIndex < g_flightVpY;
		 ++rowIndex, sourceOffset += sourcePitch, destOffset += destPitch) {
		uint8_t* sourceRow = sourcePixels + sourceOffset;
		uint8_t* destRow = centeredDestPixels + destOffset;
		if ((g_surfaceWidth & 1) != 0) {
			memcpy(destRow, sourceRow, sizeof(uint16_t));
			if (g_surfaceWidth != 0) {
				memcpy(destRow + sizeof(uint16_t), sourceRow + sizeof(uint16_t),
					   (g_surfaceWidth - 1) * g_flightBytesPerPixel);
			}
		} else {
			memcpy(destRow, sourceRow, g_surfaceWidth * g_flightBytesPerPixel);
		}
	}
	mask = (const uint8_t*)g_flightAuxBuffer;
	maskIndex = (uint16_t)g_viewportSpanMaskOffset;
	for (viewportRow = 0; viewportRow < g_flightVpHeight;
		 ++viewportRow, sourceOffset += sourcePitch, destOffset += destPitch) {
		uint8_t* sourceRow = sourcePixels + sourceOffset;
		uint8_t* destRow = centeredDestPixels + destOffset;
		int8_t copySpanFlag;
		int spanPixelOffset;
		unsigned int rightStart;
		unsigned int byteOffset;
		if (g_flightVpX != 0) {
			if ((g_flightVpX & 1) != 0) {
				memcpy(destRow, sourceRow, sizeof(uint16_t));
				if (g_flightVpX != 0) {
					memcpy(destRow + sizeof(uint16_t), sourceRow + sizeof(uint16_t),
						   (g_flightVpX - 1) * g_flightBytesPerPixel);
				}
			} else {
				memcpy(destRow, sourceRow, g_flightVpX * g_flightBytesPerPixel);
			}
		}
		byteOffset = g_flightVpX * g_flightBytesPerPixel;
		copySpanFlag = (int8_t)mask[maskIndex++];
		for (spanPixelOffset = 0; spanPixelOffset < g_flightVpWidth;) {
			int spanLength = mask[maskIndex++];
			if (spanLength == 0) {
				spanLength = mask[maskIndex++];
				if (spanLength == 0) {
					spanLength = mask[maskIndex++] + FLIGHT_VIEW_MASK_LONG_BASE;
				}
				spanLength += FLIGHT_VIEW_MASK_EXTENDED_BASE;
			}
			if (copySpanFlag < 0) {
				uint8_t* sourceSpan = sourceRow + byteOffset;
				uint8_t* destSpan = destRow + byteOffset;
				if ((spanLength & 1) != 0) {
					memcpy(destSpan, sourceSpan, sizeof(uint16_t));
					memcpy(destSpan + sizeof(uint16_t), sourceSpan + sizeof(uint16_t),
						   (spanLength - 1) * g_flightBytesPerPixel);
				} else {
					memcpy(destSpan, sourceSpan, spanLength * g_flightBytesPerPixel);
				}
			}
			byteOffset += spanLength * g_flightBytesPerPixel;
			spanPixelOffset += spanLength;
			copySpanFlag = -copySpanFlag;
		}
		rightStart = g_flightVpX + spanPixelOffset;
		if ((unsigned int)g_surfaceWidth > rightStart) {
			unsigned int remaining = (unsigned int)g_surfaceWidth - rightStart;
			uint8_t* sourceSpan = sourceRow + byteOffset;
			uint8_t* destSpan = destRow + byteOffset;
			if ((remaining & 1) != 0) {
				memcpy(destSpan, sourceSpan, sizeof(uint16_t));
				memcpy(destSpan + sizeof(uint16_t), sourceSpan + sizeof(uint16_t),
					   (remaining - 1) * g_flightBytesPerPixel);
			} else {
				memcpy(destSpan, sourceSpan, remaining * g_flightBytesPerPixel);
			}
		}
	}
	for (rowIndex = g_flightVpY + g_flightVpHeight; rowIndex < (unsigned int)g_surfaceHeight;
		 ++rowIndex, sourceOffset += sourcePitch, destOffset += destPitch) {
		uint8_t* sourceRow = sourcePixels + sourceOffset;
		uint8_t* destRow = centeredDestPixels + destOffset;
		if ((g_surfaceWidth & 1) != 0) {
			memcpy(destRow, sourceRow, sizeof(uint16_t));
			if (g_surfaceWidth != 0) {
				memcpy(destRow + sizeof(uint16_t), sourceRow + sizeof(uint16_t),
					   (g_surfaceWidth - 1) * g_flightBytesPerPixel);
			}
		} else {
			memcpy(destRow, sourceRow, g_surfaceWidth * g_flightBytesPerPixel);
		}
	}
#ifdef XW_MODERN
	{
		HRESULT backResult;
		result = g_flightOffscreenSurface->lpVtbl->Unlock(g_flightOffscreenSurface, sourcePixels);
		backResult = g_flightBackBuffer->lpVtbl->Unlock(g_flightBackBuffer, destPixels);
		if (result == DX_DD_OK)
			result = backResult;
		XwRenderCapture_CompleteHudComposite(result == DX_DD_OK);
		return result;
	}
#else
	g_flightOffscreenSurface->lpVtbl->Unlock(g_flightOffscreenSurface, sourcePixels);
	return g_flightBackBuffer->lpVtbl->Unlock(g_flightBackBuffer, destPixels);
#endif
}

// FUNCTION: XW 0x484BB0
void j_std3D_FlushTextureCache(void) { std3D_FlushTextureCache(); }

// FUNCTION: XW 0x49DF10
void SetFlightViewport(uint16_t width, uint16_t height, int unused, unsigned int byteOffset) {
	(void)unused;
	g_flightVpWidth = width;
	g_flightVpMaxX = width - 1;
	g_flightVpCenterX = width >> 1;
	g_flightVpHeight = height;
	g_flightVpMaxY = height - 1;
	g_flightVpCenterY = height >> 1;
	g_flightVpBaseOffset = byteOffset;
	g_flightVpY = byteOffset / g_surfacePitch;
	g_flightVpX = byteOffset % g_surfacePitch / g_flightBytesPerPixel;
}
