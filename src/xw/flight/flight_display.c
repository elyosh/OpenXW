#include "xw/flight/flight_display.h"
#ifdef XW_MODERN
#include "xw_runtime/platform/classic_surfaces.h"
#include "xw_runtime/runtime/presentation.h"
#endif

#include "xw/assets/file.h"
#include "xw/assets/model_texture.h"
#include "xw/flight/fediskio.h"
#include "xw/flight/flight.h"
#include "xw/flight/replay/replay.h"
#include "xw/input/win_mouse.h"
#include "xw/landru_config.h"
#include "xw/render/flight_palette.h"
#include "xw/render/renderer.h"
#include "xw/render/rtsvga2.h"
#include "xw/render/std3d.h"
#include "xw_runtime/platform/startup_dialog.h"
#include "xw_runtime/timing/host_clock.h"

#include <landru/cursor.h>
#ifdef XW_MODERN
#include <aeron/compat/host.h>
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4C9BFC
uint8_t* g_flightSwFramebufferBase = (uint8_t*)FLIGHT_DISPLAY_INITIAL_FRAMEBUFFER_ADDRESS;

// GLOBAL: XW 0x4CEF44
uint8_t* g_surfacePixels = (uint8_t*)FLIGHT_DISPLAY_INITIAL_FRAMEBUFFER_ADDRESS;

// GLOBAL: XW 0x4CEF58
uint8_t g_flightBackgroundPaletteIndex = FLIGHT_DISPLAY_INITIAL_BACKGROUND_PALETTE_INDEX;

// GLOBAL: XW 0x4CEF9C
int g_swFramebufferClearChunkSize = FLIGHT_DISPLAY_INITIAL_CLEAR_CHUNK_SIZE;

// GLOBAL: XW 0x4CEFA0
int g_flightLowResolutionState = FLIGHT_DISPLAY_INITIAL_LOW_RESOLUTION_STATE;

// GLOBAL: XW 0x4CEFA4
int g_flightScreenWidth = FLIGHT_DISPLAY_WIDTH;

// GLOBAL: XW 0x4CEFA8
unsigned int g_flightScreenHeight = FLIGHT_DISPLAY_HEIGHT;

// GLOBAL: XW 0x4CEFAC
int g_surfacePitch = FLIGHT_DISPLAY_INITIAL_PITCH_BYTES;

// GLOBAL: XW 0x4CEFB0
int g_flightBytesPerPixel = FLIGHT_DISPLAY_INDEXED_BYTES_PER_PIXEL;

// GLOBAL: XW 0x4CF0B8
unsigned int g_flightResolutionMode = FLIGHT_DISPLAY_INITIAL_RESOLUTION_MODE;

// GLOBAL: XW 0x4CF0BC
int g_flightDefaultResolutionMode = FLIGHT_DISPLAY_MODE_101H;

/* The original default is replaced with the locked surface address. */
// GLOBAL: XW 0x4DECE0
uint8_t* g_swFramebufferBase = (uint8_t*)FLIGHT_DISPLAY_INITIAL_FRAMEBUFFER_ADDRESS;

// GLOBAL: XW 0x4DECE4
int g_flightSurfaceViewport480ByteSpan = FLIGHT_DISPLAY_INITIAL_VIEWPORT_BYTE_SPAN;

// GLOBAL: XW 0x4DECEC
int g_frontendDisplayWndProcMode = -1;

// GLOBAL: XW 0x4DECF4
int g_flightFullscreen = 1;

// GLOBAL: XW 0x4DECF8
int g_flightPageFlip = 1;

// GLOBAL: XW 0x4DECFC
int g_pixelFormatCode = FLIGHT_DISPLAY_PIXEL_FORMAT_INDEXED;

// GLOBAL: XW 0x4DED00
int g_appliedDisplayMode = FLIGHT_DISPLAY_MODE_101H;

// GLOBAL: XW 0x4DED04
int g_surfaceWidth = FLIGHT_DISPLAY_WIDTH;

// GLOBAL: XW 0x4DED08
int g_surfaceHeight = FLIGHT_DISPLAY_HEIGHT;

// GLOBAL: XW 0x4DED0C
int g_flightDisplayWidth = FLIGHT_DISPLAY_WIDTH;

// GLOBAL: XW 0x4DED10
int g_flightDisplayHeight = FLIGHT_DISPLAY_HEIGHT;

// GLOBAL: XW 0x4DED1C
int g_flightLockOffscreenSurface = 1;

// GLOBAL: XW 0x4DED7C
const int g_flightVblankPhaseWindow = FLIGHT_DISPLAY_VBLANK_PHASE_WINDOW;

// GLOBAL: XW 0x4F4A50
int g_flightSurfaceAlreadyLocked = 0;

// GLOBAL: XW 0x566854
uint32_t g_flightVblankFrequencyEstimate = 0;

// GLOBAL: XW 0x566860
DxGuid g_configuredDirectDrawDriverGuid = { 0 };

// GLOBAL: XW 0x56688C
int g_flightConfFlicker = 0;

// GLOBAL: XW 0x5668EC
int g_flightGrayscaleInitialized = 0;

// GLOBAL: XW 0x566914
int g_surfaceLockCount = 0;

// GLOBAL: XW 0x566918
uint32_t g_flightVblankEpochMs = 0;

// GLOBAL: XW 0x5678F4
IDirectDrawSurface* g_flightOffscreenSurface = NULL;

// GLOBAL: XW 0x5678FC
IDirectDraw* g_flightDirectDraw = NULL;

// GLOBAL: XW 0x567900
IDirectDrawPalette* g_ddPalette = NULL;

// GLOBAL: XW 0x567920
XwGrayscaleEntry g_flightGrayscaleEntries[FLIGHT_DISPLAY_GRAYSCALE_ENTRIES] = { 0 };

// GLOBAL: XW 0x567D20
IDirectDrawSurface* g_flightRenderSurface = NULL;

// GLOBAL: XW 0x568160
char g_flightDisplayDebugMessage[FLIGHT_DISPLAY_DEBUG_MESSAGE_CAPACITY] = { 0 };

// GLOBAL: XW 0x568280
XwDirectDrawPaletteEntry g_directDrawPaletteEntries[FLIGHT_DISPLAY_PALETTE_ENTRIES] = { 0 };

// GLOBAL: XW 0x568680
IDirectDrawSurface* g_flightPrimarySurface = NULL;

// GLOBAL: XW 0x568688
IDirectDrawSurface* g_flightAuxiliarySurface = NULL;

// GLOBAL: XW 0x568690
int g_flightSurfacePitchBytes = 0;

// GLOBAL: XW 0x568694
int g_flightColorKeyCapabilityMode = 0;

// GLOBAL: XW 0x568698
IDirectDrawSurface* g_flightBackBuffer = NULL;

// GLOBAL: XW 0x62C938
int16_t g_flightDisplaySurfaceMode = 0;

// FUNCTION: XW 0x4AB960
void FlightDisplay_SetViewport480ByteSpan(int byteSpan) { g_flightSurfaceViewport480ByteSpan = byteSpan; }

// FUNCTION: XW 0x4AB970
void FlightDisplay_SetSoftwareFramebufferBase(uint8_t* framebufferBase) {
	g_swFramebufferBase = framebufferBase;
}

// FUNCTION: XW 0x4AB980
uint8_t* FlightDisplay_GetSoftwareFramebufferBase(void) { return g_swFramebufferBase; }

// FUNCTION: XW 0x4ACC30
int32_t FlightDisplay_PostPrimarySurfaceCreateOrRestoreStub(void) { return 1; }

// FUNCTION: XW 0x4ACC40
const DxGuid* FlightDisplay_LoadDriverGuid(void) {
#ifdef XW_MODERN
	return NULL;
#else
	XwFile* stream = File_RawOpen("video.cfg", "rb");
	if (stream == NULL) {
		return NULL;
	}
	if (File_RawRead(&g_configuredDirectDrawDriverGuid, sizeof(uint8_t),
					 sizeof(g_configuredDirectDrawDriverGuid),
					 stream) != sizeof(g_configuredDirectDrawDriverGuid)) {
		File_RawClose(stream);
		return NULL;
	}
	File_RawClose(stream);
	return &g_configuredDirectDrawDriverGuid;
#endif
}

// FUNCTION: XW 0x4ACC90
int FlightDisplay_Init(void) {
	DDSCAPS backBufferCaps;
	DDSURFACEDESC surfaceDesc;
	XwDirectDrawCapsDx5 driverCaps;
	HRESULT modeResult;
	int paletteIndex;
#ifdef XW_MODERN
	if (DirectDrawCreate_Compat(NULL, &g_flightDirectDraw, NULL) != 0)
#else
	if (DirectDrawCreate(NULL, &g_flightDirectDraw, NULL) != 0)
#endif
		return FlightDisplay_CleanupAndReportError(FLIGHT_DISPLAY_ERROR_CREATE);
	if (g_flightFullscreen == 0) {
		if (g_flightDirectDraw->lpVtbl->SetCooperativeLevel(g_flightDirectDraw, g_flightMainWindowHandle,
															DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE |
																DDSCL_ALLOWMODEX) != 0)
			return FlightDisplay_CleanupAndReportError(FLIGHT_DISPLAY_ERROR_COOPERATIVE);
		if (g_flightDirectDraw->lpVtbl->SetDisplayMode(
				g_flightDirectDraw, g_flightDisplayWidth, g_flightDisplayHeight,
				FLIGHT_DISPLAY_PIXEL_FORMAT_INDEXED * g_flightBytesPerPixel) != 0)
			return FlightDisplay_CleanupAndReportError(FLIGHT_DISPLAY_ERROR_COOPERATIVE);
		if (g_flightDirectDraw->lpVtbl->SetCooperativeLevel(g_flightDirectDraw, g_flightMainWindowHandle,
															DDSCL_NORMAL) != 0)
			return FlightDisplay_CleanupAndReportError(FLIGHT_DISPLAY_ERROR_COOPERATIVE);
	} else if (g_flightDirectDraw->lpVtbl->SetCooperativeLevel(g_flightDirectDraw, g_flightMainWindowHandle,
															   DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE |
																   DDSCL_ALLOWMODEX) != 0) {
		return FlightDisplay_CleanupAndReportError(FLIGHT_DISPLAY_ERROR_COOPERATIVE);
	}
	if (g_useHardware3D != 0)
		g_flightBytesPerPixel = FLIGHT_DISPLAY_DIRECT_BYTES_PER_PIXEL;
	if (g_flightFullscreen != 0) {
		modeResult = g_flightDirectDraw->lpVtbl->SetDisplayMode(
			g_flightDirectDraw, g_flightDisplayWidth, g_flightDisplayHeight,
			FLIGHT_DISPLAY_PIXEL_FORMAT_INDEXED * g_flightBytesPerPixel);
		if (modeResult != 0) {
			if (g_flightDisplayWidth == FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH) {
				g_flightDisplayWidth = FLIGHT_DISPLAY_FALLBACK_WIDTH;
				g_flightDisplayHeight = FLIGHT_DISPLAY_FALLBACK_HEIGHT;
				modeResult = g_flightDirectDraw->lpVtbl->SetDisplayMode(
					g_flightDirectDraw, g_flightDisplayWidth, g_flightDisplayHeight,
					FLIGHT_DISPLAY_PIXEL_FORMAT_INDEXED * g_flightBytesPerPixel);
				if (modeResult != 0) {
					g_flightDisplayWidth = FLIGHT_DISPLAY_WIDTH;
					g_flightDisplayHeight = FLIGHT_DISPLAY_HEIGHT;
					modeResult = g_flightDirectDraw->lpVtbl->SetDisplayMode(
						g_flightDirectDraw, g_flightDisplayWidth, g_flightDisplayHeight,
						FLIGHT_DISPLAY_PIXEL_FORMAT_INDEXED * g_flightBytesPerPixel);
					if (modeResult != 0) {
						g_flightDisplayWidth = FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH;
						g_flightDisplayHeight = FLIGHT_DISPLAY_LOW_MODE_HEIGHT;
					}
				}
			} else if (g_flightDisplayWidth == FLIGHT_DISPLAY_FALLBACK_WIDTH) {
				g_flightDisplayWidth = FLIGHT_DISPLAY_WIDTH;
				g_flightDisplayHeight = FLIGHT_DISPLAY_HEIGHT;
				modeResult = g_flightDirectDraw->lpVtbl->SetDisplayMode(
					g_flightDirectDraw, g_flightDisplayWidth, g_flightDisplayHeight,
					FLIGHT_DISPLAY_PIXEL_FORMAT_INDEXED * g_flightBytesPerPixel);
				if (modeResult != 0) {
					g_flightDisplayWidth = FLIGHT_DISPLAY_FALLBACK_WIDTH;
					g_flightDisplayHeight = FLIGHT_DISPLAY_FALLBACK_HEIGHT;
				}
			}
			if (modeResult != 0 && g_flightBytesPerPixel == FLIGHT_DISPLAY_DIRECT_BYTES_PER_PIXEL) {
				g_flightBytesPerPixel = FLIGHT_DISPLAY_INDEXED_BYTES_PER_PIXEL;
				modeResult = g_flightDirectDraw->lpVtbl->SetDisplayMode(
					g_flightDirectDraw, g_flightDisplayWidth, g_flightDisplayHeight,
					FLIGHT_DISPLAY_PIXEL_FORMAT_INDEXED);
				if (modeResult != 0) {
					if (g_flightDisplayWidth == FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH) {
						g_flightDisplayWidth = FLIGHT_DISPLAY_FALLBACK_WIDTH;
						g_flightDisplayHeight = FLIGHT_DISPLAY_FALLBACK_HEIGHT;
						modeResult = g_flightDirectDraw->lpVtbl->SetDisplayMode(
							g_flightDirectDraw, g_flightDisplayWidth, g_flightDisplayHeight,
							FLIGHT_DISPLAY_PIXEL_FORMAT_INDEXED * g_flightBytesPerPixel);
						if (modeResult != 0) {
							g_flightDisplayWidth = FLIGHT_DISPLAY_WIDTH;
							g_flightDisplayHeight = FLIGHT_DISPLAY_HEIGHT;
							modeResult = g_flightDirectDraw->lpVtbl->SetDisplayMode(
								g_flightDirectDraw, g_flightDisplayWidth, g_flightDisplayHeight,
								FLIGHT_DISPLAY_PIXEL_FORMAT_INDEXED * g_flightBytesPerPixel);
							if (modeResult != 0) {
								g_flightDisplayWidth = FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH;
								g_flightDisplayHeight = FLIGHT_DISPLAY_LOW_MODE_HEIGHT;
							}
						}
					} else if (g_flightDisplayWidth == FLIGHT_DISPLAY_FALLBACK_WIDTH) {
						g_flightDisplayWidth = FLIGHT_DISPLAY_WIDTH;
						g_flightDisplayHeight = FLIGHT_DISPLAY_HEIGHT;
						modeResult = g_flightDirectDraw->lpVtbl->SetDisplayMode(
							g_flightDirectDraw, g_flightDisplayWidth, g_flightDisplayHeight,
							FLIGHT_DISPLAY_PIXEL_FORMAT_INDEXED * g_flightBytesPerPixel);
						if (modeResult != 0) {
							g_flightDisplayWidth = FLIGHT_DISPLAY_FALLBACK_WIDTH;
							g_flightDisplayHeight = FLIGHT_DISPLAY_FALLBACK_HEIGHT;
						}
					}
				}
			} else if (modeResult != 0 && g_flightBytesPerPixel == FLIGHT_DISPLAY_INDEXED_BYTES_PER_PIXEL) {
				g_flightBytesPerPixel = FLIGHT_DISPLAY_DIRECT_BYTES_PER_PIXEL;
				modeResult = g_flightDirectDraw->lpVtbl->SetDisplayMode(
					g_flightDirectDraw, g_flightDisplayWidth, g_flightDisplayHeight,
					FLIGHT_DISPLAY_DIRECT_BITS_PER_PIXEL);
				if (modeResult != 0) {
					if (g_flightDisplayWidth == FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH) {
						g_flightDisplayWidth = FLIGHT_DISPLAY_FALLBACK_WIDTH;
						g_flightDisplayHeight = FLIGHT_DISPLAY_FALLBACK_HEIGHT;
						modeResult = g_flightDirectDraw->lpVtbl->SetDisplayMode(
							g_flightDirectDraw, g_flightDisplayWidth, g_flightDisplayHeight,
							FLIGHT_DISPLAY_PIXEL_FORMAT_INDEXED * g_flightBytesPerPixel);
						if (modeResult != 0) {
							g_flightDisplayWidth = FLIGHT_DISPLAY_WIDTH;
							g_flightDisplayHeight = FLIGHT_DISPLAY_HEIGHT;
							modeResult = g_flightDirectDraw->lpVtbl->SetDisplayMode(
								g_flightDirectDraw, g_flightDisplayWidth, g_flightDisplayHeight,
								FLIGHT_DISPLAY_PIXEL_FORMAT_INDEXED * g_flightBytesPerPixel);
							if (modeResult != 0) {
								g_flightDisplayWidth = FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH;
								g_flightDisplayHeight = FLIGHT_DISPLAY_LOW_MODE_HEIGHT;
							}
						}
					} else if (g_flightDisplayWidth == FLIGHT_DISPLAY_FALLBACK_WIDTH) {
						g_flightDisplayWidth = FLIGHT_DISPLAY_WIDTH;
						g_flightDisplayHeight = FLIGHT_DISPLAY_HEIGHT;
						modeResult = g_flightDirectDraw->lpVtbl->SetDisplayMode(
							g_flightDirectDraw, g_flightDisplayWidth, g_flightDisplayHeight,
							FLIGHT_DISPLAY_PIXEL_FORMAT_INDEXED * g_flightBytesPerPixel);
						if (modeResult != 0) {
							g_flightDisplayWidth = FLIGHT_DISPLAY_FALLBACK_WIDTH;
							g_flightDisplayHeight = FLIGHT_DISPLAY_FALLBACK_HEIGHT;
						}
					}
				}
			}
			if (modeResult != 0)
				return FlightDisplay_CleanupAndReportError(FLIGHT_DISPLAY_ERROR_MODE);
		}
	}
	if (g_flightBytesPerPixel != FLIGHT_DISPLAY_DIRECT_BYTES_PER_PIXEL)
		g_useHardware3D = 0;
	memset(&driverCaps, 0, sizeof(driverCaps));
	g_flightColorKeyCapabilityMode = FLIGHT_DISPLAY_COLORKEY_DEFAULT;
	driverCaps.dwSize = sizeof(driverCaps);
	if (g_flightDirectDraw->lpVtbl->GetCaps(g_flightDirectDraw, &driverCaps, NULL) == 0 &&
		(driverCaps.dwCaps & FLIGHT_DISPLAY_CAPS_COLORKEY) != 0 &&
		(driverCaps.dwCKeyCaps & FLIGHT_DISPLAY_CKEY_DESTBLT) != 0 &&
		(driverCaps.dwCKeyCaps & FLIGHT_DISPLAY_CKEY_SRCBLT) == 0) {
		g_flightColorKeyCapabilityMode = FLIGHT_DISPLAY_COLORKEY_DEST_ONLY;
	}
	if (g_flightFullscreen != 0) {
		memset(&surfaceDesc, 0, sizeof(surfaceDesc));
		surfaceDesc.dwSize = sizeof(surfaceDesc);
		if (g_flightPageFlip != 0) {
			surfaceDesc.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
			surfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
			surfaceDesc.dwBackBufferCount = 1;
			if (g_useHardware3D != 0)
				surfaceDesc.ddsCaps.dwCaps =
					DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_3DDEVICE;
		} else {
			surfaceDesc.dwFlags = DDSD_CAPS;
			surfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
			if (g_useHardware3D != 0)
				surfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE;
		}
		if (g_flightDirectDraw->lpVtbl->CreateSurface(g_flightDirectDraw, &surfaceDesc,
													  &g_flightPrimarySurface, NULL) != 0)
			return FlightDisplay_CleanupAndReportError(FLIGHT_DISPLAY_ERROR_PRIMARY);
		g_pixelFormatCode = g_flightBytesPerPixel != FLIGHT_DISPLAY_DIRECT_BYTES_PER_PIXEL
								? FLIGHT_DISPLAY_PIXEL_FORMAT_INDEXED
								: FLIGHT_DISPLAY_PIXEL_FORMAT_RGB565;
		memset(&surfaceDesc, 0, sizeof(surfaceDesc));
		surfaceDesc.dwSize = sizeof(surfaceDesc);
		g_flightPrimarySurface->lpVtbl->GetSurfaceDesc(g_flightPrimarySurface, &surfaceDesc);
		g_flightSurfacePitchBytes = surfaceDesc.lPitch;
		g_surfacePitch = surfaceDesc.lPitch;
		if ((surfaceDesc.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8) != 0) {
			g_flightBytesPerPixel = FLIGHT_DISPLAY_INDEXED_BYTES_PER_PIXEL;
			g_pixelFormatCode = FLIGHT_DISPLAY_PIXEL_FORMAT_INDEXED;
		} else if ((surfaceDesc.ddpfPixelFormat.dwFlags & DDPF_RGB) != 0) {
			g_flightBytesPerPixel = FLIGHT_DISPLAY_DIRECT_BYTES_PER_PIXEL;
			g_pixelFormatCode = (surfaceDesc.ddpfPixelFormat.dwGBitMask & FLIGHT_DISPLAY_GREEN_SIXTH_BIT) != 0
									? FLIGHT_DISPLAY_PIXEL_FORMAT_RGB565
									: FLIGHT_DISPLAY_PIXEL_FORMAT_RGB555;
		}
		FlightDisplay_ClearSurface(g_flightPrimarySurface);
		if (g_flightPageFlip != 0) {
			backBufferCaps.dwCaps = DDSCAPS_BACKBUFFER;
			if (g_flightPrimarySurface->lpVtbl->GetAttachedSurface(g_flightPrimarySurface, &backBufferCaps,
																   &g_flightBackBuffer) != 0)
				return FlightDisplay_CleanupAndReportError(FLIGHT_DISPLAY_ERROR_BACKBUFFER);
			surfaceDesc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
			surfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
			if (g_useHardware3D != 0)
				surfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
			surfaceDesc.dwWidth = g_surfaceWidth;
			surfaceDesc.dwHeight = g_surfaceHeight;
			if (g_flightDirectDraw->lpVtbl->CreateSurface(g_flightDirectDraw, &surfaceDesc,
														  &g_flightOffscreenSurface, NULL) != 0)
				return FlightDisplay_CleanupAndReportError(FLIGHT_DISPLAY_ERROR_OFFSCREEN);
			surfaceDesc.dwWidth = g_surfaceWidth;
			surfaceDesc.dwHeight = g_surfaceHeight;
			surfaceDesc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
			surfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
			if (g_flightDirectDraw->lpVtbl->CreateSurface(g_flightDirectDraw, &surfaceDesc,
														  &g_flightAuxiliarySurface, NULL) != 0)
				return FlightDisplay_CleanupAndReportError(FLIGHT_DISPLAY_ERROR_OFFSCREEN);
			FlightDisplay_ClearSurface(g_flightBackBuffer);
			g_flightRenderSurface = g_flightBackBuffer;
			FlightDisplay_ClearSurface(g_flightOffscreenSurface);
			FlightDisplay_ClearSurface(g_flightAuxiliarySurface);
		} else {
			g_flightRenderSurface = g_flightPrimarySurface;
			g_flightBackBuffer = g_flightPrimarySurface;
		}
		if (!FlightDisplay_PostPrimarySurfaceCreateOrRestoreStub())
			return FlightDisplay_CleanupAndReportError(FLIGHT_DISPLAY_ERROR_POST_CREATE);
	} else {
		memset(&surfaceDesc, 0, sizeof(surfaceDesc));
		surfaceDesc.dwSize = sizeof(surfaceDesc);
		surfaceDesc.dwFlags = DDSD_CAPS;
		surfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		if (g_flightDirectDraw->lpVtbl->CreateSurface(g_flightDirectDraw, &surfaceDesc,
													  &g_flightPrimarySurface, NULL) != 0)
			return FlightDisplay_CleanupAndReportError(FLIGHT_DISPLAY_ERROR_PRIMARY);
		if (!FlightDisplay_PostPrimarySurfaceCreateOrRestoreStub())
			return FlightDisplay_CleanupAndReportError(FLIGHT_DISPLAY_ERROR_POST_CREATE);
		memset(&surfaceDesc, 0, sizeof(surfaceDesc));
		surfaceDesc.dwSize = sizeof(surfaceDesc);
		g_flightPrimarySurface->lpVtbl->GetSurfaceDesc(g_flightPrimarySurface, &surfaceDesc);
		g_flightSurfacePitchBytes = surfaceDesc.lPitch;
		if ((surfaceDesc.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8) != 0 ||
			(surfaceDesc.ddpfPixelFormat.dwFlags & DDPF_RGB) == 0) {
			g_flightBytesPerPixel = FLIGHT_DISPLAY_INDEXED_BYTES_PER_PIXEL;
			g_pixelFormatCode = FLIGHT_DISPLAY_PIXEL_FORMAT_INDEXED;
		} else {
			g_flightBytesPerPixel = FLIGHT_DISPLAY_DIRECT_BYTES_PER_PIXEL;
			g_pixelFormatCode = (surfaceDesc.ddpfPixelFormat.dwGBitMask & FLIGHT_DISPLAY_GREEN_SIXTH_BIT) != 0
									? FLIGHT_DISPLAY_PIXEL_FORMAT_RGB565
									: FLIGHT_DISPLAY_PIXEL_FORMAT_RGB555;
		}
		surfaceDesc.dwWidth = surfaceDesc.lPitch;
		surfaceDesc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		surfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
		surfaceDesc.dwHeight = g_flightDisplayHeight;
		if (g_flightDirectDraw->lpVtbl->CreateSurface(g_flightDirectDraw, &surfaceDesc, &g_flightBackBuffer,
													  NULL) != 0)
			return FlightDisplay_CleanupAndReportError(FLIGHT_DISPLAY_ERROR_OFFSCREEN);
		surfaceDesc.dwWidth = g_surfaceWidth;
		surfaceDesc.dwHeight = g_surfaceHeight;
		surfaceDesc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		surfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
		if (g_flightDirectDraw->lpVtbl->CreateSurface(g_flightDirectDraw, &surfaceDesc,
													  &g_flightOffscreenSurface, NULL) != 0)
			return FlightDisplay_CleanupAndReportError(FLIGHT_DISPLAY_ERROR_OFFSCREEN);
		surfaceDesc.dwWidth = g_surfaceWidth;
		surfaceDesc.dwHeight = g_surfaceHeight;
		surfaceDesc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		surfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
		if (g_flightDirectDraw->lpVtbl->CreateSurface(g_flightDirectDraw, &surfaceDesc,
													  &g_flightAuxiliarySurface, NULL) != 0)
			return FlightDisplay_CleanupAndReportError(FLIGHT_DISPLAY_ERROR_OFFSCREEN);
	}
	if (g_flightBytesPerPixel == FLIGHT_DISPLAY_INDEXED_BYTES_PER_PIXEL) {
		for (paletteIndex = 0; paletteIndex < FLIGHT_DISPLAY_PALETTE_ENTRIES; ++paletteIndex) {
			g_directDrawPaletteEntries[paletteIndex].red = 0;
			g_directDrawPaletteEntries[paletteIndex].green = 0;
			g_directDrawPaletteEntries[paletteIndex].blue = 0;
		}
		g_flightDirectDraw->lpVtbl->CreatePalette(
			g_flightDirectDraw, DDPCAPS_8BIT | FLIGHT_DISPLAY_PALETTE_INITIALIZE | DDPCAPS_ALLOW256,
			g_directDrawPaletteEntries, &g_ddPalette, NULL);
		if (g_ddPalette != NULL)
			g_flightPrimarySurface->lpVtbl->SetPalette(g_flightPrimarySurface, g_ddPalette);
	} else {
		g_ddPalette = NULL;
	}
	if (g_useHardware3D != 0)
		Renderer_InitD3DDevice();
	FlightDisplay_InitGrayscaleTable();
#ifdef XW_MODERN
	XwDisplay_EndSurfaceChange(0);
#endif
	return 1;
}

// FUNCTION: XW 0x4AD5B0
int FlightDisplay_InitGrayscaleTable(void) {
	int level = g_flightGrayscaleInitialized;
	if (level == 0) {
		for (level = 0; level < FLIGHT_DISPLAY_GRAYSCALE_ENTRIES / 2; ++level) {
			int highIndex = FLIGHT_DISPLAY_GRAYSCALE_ENTRIES - 1 - level;
			uint8_t highLevel;
			g_flightGrayscaleEntries[level].channel2 = (uint8_t)level;
			g_flightGrayscaleEntries[level].channel1 = (uint8_t)level;
			g_flightGrayscaleEntries[level].channel0 = (uint8_t)level;
			highLevel = (uint8_t)(UINT8_MAX - level);
			g_flightGrayscaleEntries[highIndex].channel2 = highLevel;
			g_flightGrayscaleEntries[highIndex].channel1 = highLevel;
			g_flightGrayscaleEntries[highIndex].channel0 = highLevel;
		}
		g_flightGrayscaleInitialized = 1;
	}
	return level;
}

// FUNCTION: XW 0x4AD600
void FlightDisplay_SetPaletteEntries(const uint8_t* rgb6Palette, int startIndex, unsigned int count) {
#ifdef XW_MODERN
	XwDisplay_SetPalette(rgb6Palette, startIndex, count);
#else
	XwDirectDrawPaletteEntry entries[FLIGHT_DISPLAY_PALETTE_ENTRIES];
	int lockCount;
	int index;
	const RgbTriplet* colors = (const RgbTriplet*)rgb6Palette;
	while (g_windowActive == 0) {
		if (g_quitRequested != 0)
			break;
		Flight_PumpWindowMessages();
	}
	if (g_SoftwareCursor != 0)
		xcursor_Select_Contrast_Colors(rgb6Palette);
	lockCount = FlightDisplay_GetSurfaceLockCount();
	if (lockCount > 0) {
		int remainingLocks;
		for (remainingLocks = lockCount; remainingLocks != 0; --remainingLocks)
			FlightDisplay_UnlockSurface();
	}
	for (index = startIndex; index < (int)(startIndex + count); ++index) {
		entries[index].red = colors[index].r << FLIGHT_DISPLAY_RGB6_TO_RGB8_SHIFT;
		entries[index].green = colors[index].g << FLIGHT_DISPLAY_RGB6_TO_RGB8_SHIFT;
		entries[index].blue = colors[index].b << FLIGHT_DISPLAY_RGB6_TO_RGB8_SHIFT;
	}
	if (g_ddPalette != NULL)
		g_ddPalette->lpVtbl->SetEntries(g_ddPalette, 0, startIndex, count, entries);
	if (lockCount > 0) {
		int remainingLocks;
		for (remainingLocks = lockCount; remainingLocks != 0; --remainingLocks)
			FlightDisplay_LockSurface();
	}
#endif
}

// FUNCTION: XW 0x4AD6E0
void FlightDisplay_RestorePaletteAfterActivation(void) {
#ifdef XW_MODERN
	XwDisplay_RestorePalette();
#else
	int lockCount;
	int lockIndex;
	while (g_windowActive == 0) {
		if (g_quitRequested != 0)
			break;
		Flight_PumpWindowMessages();
	}
	lockCount = FlightDisplay_GetSurfaceLockCount();
	if (lockCount > 0) {
		for (lockIndex = lockCount; lockIndex != 0; --lockIndex)
			FlightDisplay_UnlockSurface();
	}
	if (g_ddPalette != NULL)
		g_ddPalette->lpVtbl->SetEntries(g_ddPalette, 0, 0, FLIGHT_DISPLAY_PALETTE_ENTRIES,
										g_directDrawPaletteEntries);
	if (lockCount > 0) {
		for (lockIndex = lockCount; lockIndex != 0; --lockIndex)
			FlightDisplay_LockSurface();
	}
#endif
}

// FUNCTION: XW 0x4AD750
void FlightDisplay_UpdateCachedPaletteEntries(const uint8_t* rgb6Palette, int firstEntry, int entryCount) {
#ifdef XW_MODERN
	if (entryCount > 0)
		XwDisplay_SetPalette(rgb6Palette, firstEntry, (unsigned)entryCount);
#else
	int lockCount;
	int index;
	const RgbTriplet* colors = (const RgbTriplet*)rgb6Palette;
	while (g_windowActive == 0) {
		if (g_quitRequested != 0)
			break;
		Flight_PumpWindowMessages();
	}
	if (g_SoftwareCursor != 0)
		xcursor_Select_Contrast_Colors(rgb6Palette);
	lockCount = FlightDisplay_GetSurfaceLockCount();
	if (lockCount > 0) {
		int remainingLocks;
		for (remainingLocks = lockCount; remainingLocks != 0; --remainingLocks)
			FlightDisplay_UnlockSurface();
	}
	for (index = firstEntry; index < firstEntry + entryCount; ++index) {
		uint8_t green = colors[index].g;
		uint8_t blue = colors[index].b;
		uint8_t red = colors[index].r;
		g_directDrawPaletteEntries[index].red = red << FLIGHT_DISPLAY_RGB6_TO_RGB8_SHIFT;
		g_directDrawPaletteEntries[index].green = green << FLIGHT_DISPLAY_RGB6_TO_RGB8_SHIFT;
		g_directDrawPaletteEntries[index].blue = blue << FLIGHT_DISPLAY_RGB6_TO_RGB8_SHIFT;
	}
	if (g_ddPalette != NULL)
		g_ddPalette->lpVtbl->SetEntries(g_ddPalette, 0, firstEntry, entryCount, g_directDrawPaletteEntries);
	if (lockCount > 0) {
		int remainingLocks;
		for (remainingLocks = lockCount; remainingLocks != 0; --remainingLocks)
			FlightDisplay_LockSurface();
	}
#endif
}

// FUNCTION: XW 0x4AD820
int FlightDisplay_CleanupAndReportError(int errorCode) {
#ifdef XW_MODERN
	XwDisplay_BeginSurfaceChange();
	snprintf(g_flightDisplayDebugMessage, sizeof(g_flightDisplayDebugMessage),
			 "___CleanupAndExit  err = %d\n", errorCode);
#else
	wsprintfA(g_flightDisplayDebugMessage, "___CleanupAndExit  err = %d\n", errorCode);
#endif
	XwPort_WriteStartupDiagnostic(g_flightDisplayDebugMessage);
	if (g_flightPrimarySurface != NULL) {
		g_flightPrimarySurface->lpVtbl->Release(g_flightPrimarySurface);
		g_flightPrimarySurface = NULL;
	}
	if (g_ddPalette != NULL) {
		g_ddPalette->lpVtbl->Release(g_ddPalette);
		g_ddPalette = NULL;
	}
	if (g_flightOffscreenSurface != NULL) {
		g_flightOffscreenSurface->lpVtbl->Release(g_flightOffscreenSurface);
		g_flightOffscreenSurface = NULL;
	}
	if (g_flightAuxiliarySurface != NULL) {
		g_flightAuxiliarySurface->lpVtbl->Release(g_flightAuxiliarySurface);
		g_flightAuxiliarySurface = NULL;
	}
	XwPort_ShowStartupDialog(NULL, "Game could not start", "ERROR", XW_STARTUP_DIALOG_OK);
	return 0;
}

// FUNCTION: XW 0x4AD8B0
int FlightDisplay_GetPrimarySurfacePitch(void) { return g_flightSurfacePitchBytes; }

// FUNCTION: XW 0x4AD8C0
int FlightDisplay_GetSurfaceLockCount(void) { return g_surfaceLockCount; }

// FUNCTION: XW 0x4AD8D0
void FlightDisplay_LockSurface(void) {
#ifdef XW_MODERN
	XwDisplay_LockSurface();
#else
	DDSURFACEDESC surfaceDesc;
	while (!g_windowActive) {
		if (g_quitRequested)
			break;
		Flight_PumpWindowMessages();
	}
	if (g_surfaceLockCount) {
		++g_surfaceLockCount;
		return;
	}
	g_surfaceLockCount = 1;
	if (!g_flightDisplaySurfaceMode && !g_replayviewmode && !g_ReplayReenterSimulation) {
		unsigned int horizontalOffset;
		unsigned int verticalOffset;
		memset(&surfaceDesc, 0, sizeof(surfaceDesc));
		surfaceDesc.dwSize = sizeof(surfaceDesc);
		do {
			if (g_flightAuxiliarySurface->lpVtbl->Lock(g_flightAuxiliarySurface, NULL, &surfaceDesc,
													   DDLOCK_WAIT | DDLOCK_NOSYSLOCK,
													   NULL) == DX_DDERR_SURFACELOST)
				g_flightAuxiliarySurface->lpVtbl->Restore(g_flightAuxiliarySurface);
			else
				break;
		} while (1);
		g_flightSurfacePitchBytes = surfaceDesc.lPitch;
		FlightDisplay_SetSoftwareFramebufferBase((uint8_t*)surfaceDesc.lpSurface);
		g_flightSwFramebufferBase = (uint8_t*)surfaceDesc.lpSurface;
		g_surfacePixels = (uint8_t*)surfaceDesc.lpSurface;
		memset(&surfaceDesc, 0, sizeof(surfaceDesc));
		surfaceDesc.dwSize = sizeof(surfaceDesc);
		g_flightAuxiliarySurface->lpVtbl->GetSurfaceDesc(g_flightAuxiliarySurface, &surfaceDesc);
		g_flightSurfacePitchBytes = surfaceDesc.lPitch;
		verticalOffset = (uint32_t)surfaceDesc.lPitch *
						 (((uint32_t)g_flightDisplayHeight - (uint32_t)g_surfaceHeight) >> 1);
		horizontalOffset = (uint32_t)g_flightBytesPerPixel *
						   (((uint32_t)g_flightDisplayWidth - (uint32_t)g_surfaceWidth) >> 1);
		g_flightSwFramebufferBase += verticalOffset + horizontalOffset;
		g_surfacePixels += verticalOffset + horizontalOffset;
		FlightDisplay_SetViewport480ByteSpan(FLIGHT_DISPLAY_HEIGHT * FlightDisplay_GetPrimarySurfacePitch());
		if (g_surfacePitch == g_flightSurfacePitchBytes)
			return;
	} else if (g_flightLockOffscreenSurface) {
		memset(&surfaceDesc, 0, sizeof(surfaceDesc));
		surfaceDesc.dwSize = sizeof(surfaceDesc);
		do {
			if (g_flightOffscreenSurface->lpVtbl->Lock(g_flightOffscreenSurface, NULL, &surfaceDesc,
													   DDLOCK_WAIT | DDLOCK_NOSYSLOCK,
													   NULL) == DX_DDERR_SURFACELOST)
				g_flightOffscreenSurface->lpVtbl->Restore(g_flightOffscreenSurface);
			else
				break;
		} while (1);
		g_flightSurfacePitchBytes = surfaceDesc.lPitch;
		FlightDisplay_SetSoftwareFramebufferBase((uint8_t*)surfaceDesc.lpSurface);
		g_flightSwFramebufferBase = (uint8_t*)surfaceDesc.lpSurface;
		g_surfacePixels = (uint8_t*)surfaceDesc.lpSurface;
		memset(&surfaceDesc, 0, sizeof(surfaceDesc));
		surfaceDesc.dwSize = sizeof(surfaceDesc);
		g_flightOffscreenSurface->lpVtbl->GetSurfaceDesc(g_flightOffscreenSurface, &surfaceDesc);
		g_flightSurfacePitchBytes = surfaceDesc.lPitch;
		FlightDisplay_SetViewport480ByteSpan(FLIGHT_DISPLAY_HEIGHT * FlightDisplay_GetPrimarySurfacePitch());
		if (g_surfacePitch == g_flightSurfacePitchBytes)
			return;
	} else {
		unsigned int horizontalOffset;
		unsigned int verticalOffset;
		memset(&surfaceDesc, 0, sizeof(surfaceDesc));
		surfaceDesc.dwSize = sizeof(surfaceDesc);
		do {
			if (g_flightBackBuffer->lpVtbl->Lock(g_flightBackBuffer, NULL, &surfaceDesc,
												 DDLOCK_WAIT | DDLOCK_NOSYSLOCK,
												 NULL) == DX_DDERR_SURFACELOST)
				g_flightBackBuffer->lpVtbl->Restore(g_flightBackBuffer);
			else
				break;
		} while (1);
		g_flightSurfacePitchBytes = surfaceDesc.lPitch;
		FlightDisplay_SetSoftwareFramebufferBase((uint8_t*)surfaceDesc.lpSurface);
		g_flightSwFramebufferBase = (uint8_t*)surfaceDesc.lpSurface;
		g_surfacePixels = (uint8_t*)surfaceDesc.lpSurface;
		memset(&surfaceDesc, 0, sizeof(surfaceDesc));
		surfaceDesc.dwSize = sizeof(surfaceDesc);
		g_flightBackBuffer->lpVtbl->GetSurfaceDesc(g_flightBackBuffer, &surfaceDesc);
		g_flightSurfacePitchBytes = surfaceDesc.lPitch;
		verticalOffset = (uint32_t)surfaceDesc.lPitch *
						 (((uint32_t)g_flightDisplayHeight - (uint32_t)g_surfaceHeight) >> 1);
		horizontalOffset = (uint32_t)g_flightBytesPerPixel *
						   (((uint32_t)g_flightDisplayWidth - (uint32_t)g_surfaceWidth) >> 1);
		g_flightSwFramebufferBase += verticalOffset + horizontalOffset;
		g_surfacePixels += verticalOffset + horizontalOffset;
		FlightDisplay_SetViewport480ByteSpan(FLIGHT_DISPLAY_HEIGHT * FlightDisplay_GetPrimarySurfacePitch());
		if (g_surfacePitch == g_flightSurfacePitchBytes)
			return;
	}
	g_surfacePitch = g_flightSurfacePitchBytes;
	rtsvga2_setvgapointers(g_flightSwFramebufferBase, g_flightSurfacePitchBytes, FLIGHT_DISPLAY_HEIGHT);
#endif
}

// FUNCTION: XW 0x4ADC40
void FlightDisplay_UnlockSurface(void) {
#ifdef XW_MODERN
	XwDisplay_UnlockSurface();
#else
	if (g_surfaceLockCount > 1)
		--g_surfaceLockCount;
	else if (g_surfaceLockCount < 1)
		g_surfaceLockCount = 0;
	else {
		while (!g_windowActive) {
			if (g_quitRequested)
				break;
			Flight_PumpWindowMessages();
		}
		--g_surfaceLockCount;
		if (!g_flightDisplaySurfaceMode && !g_replayviewmode && !g_ReplayReenterSimulation)
			g_flightAuxiliarySurface->lpVtbl->Unlock(g_flightAuxiliarySurface, g_surfacePixels);
		else {
			IDirectDrawSurface* surface = g_flightOffscreenSurface;
			if (!g_flightLockOffscreenSurface)
				surface = g_flightBackBuffer;
			surface->lpVtbl->Unlock(surface, g_surfacePixels);
		}
	}
#endif
}

// FUNCTION: XW 0x4ADCF0
int FlightDisplay_Flip(void) {
#ifdef XW_MODERN
	return XwDisplay_Flip();
#else
	int result;

	int sampleIndex;
	int phase;
	int32_t inVblank;
	DDSURFACEDESC surface;

	do {
		Flight_PumpWindowMessages();
	} while (g_windowActive == 0 && g_quitRequested == 0);
	result = g_quitRequested;
	if (result == 0) {
		if (g_SoftwareCursor != 0 && xcursor_Get_Display_Count() >= 0) {
			memset(&surface, 0, sizeof(surface));
			surface.dwSize = sizeof(surface);
			if (g_flightBackBuffer->lpVtbl->Lock(g_flightBackBuffer, NULL, &surface,
												 DDLOCK_WAIT | DDLOCK_NOSYSLOCK,
												 NULL) == DX_DDERR_SURFACELOST)
				return FlightDisplay_RestorePrimarySurface();
			xcursor_Draw_Software_Cursor_To_Surface(surface.lpSurface, surface.lPitch, FLIGHT_DISPLAY_HEIGHT);
			g_flightBackBuffer->lpVtbl->Unlock(g_flightBackBuffer, surface.lpSurface);
		}
		if (g_flightPageFlip != 0) {

			if (g_flightConfFlicker != 0) {
				if (g_flightVblankEpochMs == 0) {
					if (g_flightDirectDraw->lpVtbl->GetVerticalBlankStatus(g_flightDirectDraw, &inVblank) ==
						0) {
						if (inVblank != 0) {
							do {
							} while (g_flightDirectDraw->lpVtbl->GetVerticalBlankStatus(g_flightDirectDraw,
																						&inVblank) == 0 &&
									 inVblank != 0);
						}
						if (inVblank == 0) {
							do {
							} while (g_flightDirectDraw->lpVtbl->GetVerticalBlankStatus(g_flightDirectDraw,
																						&inVblank) == 0 &&
									 inVblank == 0);
						}
					}
					g_flightVblankEpochMs = timeGetTime();
					if (g_flightDirectDraw->lpVtbl->GetMonitorFrequency(
							g_flightDirectDraw, &g_flightVblankFrequencyEstimate) != 0) {
						for (sampleIndex = FLIGHT_DISPLAY_VBLANK_SAMPLES; sampleIndex != 0; --sampleIndex) {
							if (inVblank != 0) {
								do {
								} while (g_flightDirectDraw->lpVtbl->GetVerticalBlankStatus(
											 g_flightDirectDraw, &inVblank) == 0 &&
										 inVblank != 0);
							}
							if (inVblank == 0) {
								do {
								} while (g_flightDirectDraw->lpVtbl->GetVerticalBlankStatus(
											 g_flightDirectDraw, &inVblank) == 0 &&
										 inVblank == 0);
							}
						}
						g_flightVblankFrequencyEstimate = FLIGHT_DISPLAY_VBLANK_SAMPLE_SCALE /
														  (int32_t)(timeGetTime() - g_flightVblankEpochMs);
					}
				} else {
					phase =
						(int32_t)(g_flightVblankFrequencyEstimate * (timeGetTime() - g_flightVblankEpochMs)) %
						FLIGHT_DISPLAY_VBLANK_PHASE_SCALE;
					if ((phase < g_flightVblankPhaseWindow ||
						 phase > FLIGHT_DISPLAY_VBLANK_PHASE_SCALE - g_flightVblankPhaseWindow) &&
						g_flightDirectDraw->lpVtbl->GetVerticalBlankStatus(g_flightDirectDraw, &inVblank) ==
							0 &&
						inVblank == 0) {
						do {
						} while (g_flightDirectDraw->lpVtbl->GetVerticalBlankStatus(g_flightDirectDraw,
																					&inVblank) == 0 &&
								 inVblank == 0);
						g_flightVblankEpochMs = timeGetTime();
					}
				}
			}

			result = g_flightPrimarySurface->lpVtbl->Flip(g_flightPrimarySurface, NULL, DDFLIP_WAIT);
			if (result == DX_DDERR_NOEXCLUSIVEMODE) {
				g_flightDirectDraw->lpVtbl->SetCooperativeLevel(g_flightDirectDraw, g_flightMainWindowHandle,
																DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE |
																	DDSCL_ALLOWMODEX);
				if (g_flightPrimarySurface->lpVtbl->Flip(g_flightPrimarySurface, NULL, DDFLIP_WAIT) ==
					DX_DDERR_SURFACELOST) {
					g_flightPrimarySurface->lpVtbl->Restore(g_flightPrimarySurface);
					g_flightBackBuffer->lpVtbl->Restore(g_flightBackBuffer);
					g_std3DZBufferSurface->lpVtbl->Restore(g_std3DZBufferSurface);
					g_flightPrimarySurface->lpVtbl->Flip(g_flightPrimarySurface, NULL, DDFLIP_WAIT);
				}
				result = g_flightDirectDraw->lpVtbl->SetCooperativeLevel(
					g_flightDirectDraw, g_flightMainWindowHandle, DDSCL_NORMAL);
			}
			if (result == DX_DDERR_SURFACELOST) {
				result = FlightDisplay_RestorePrimarySurface();
				if (result != 0)
					result = FlightDisplay_PostPrimarySurfaceCreateOrRestoreStub();
			}
		} else {
			result = g_flightPrimarySurface->lpVtbl->Blt(g_flightPrimarySurface, NULL, g_flightBackBuffer,
														 NULL, DDBLT_WAIT, NULL);
		}
	}
	return result;
#endif
}

// FUNCTION: XW 0x4ADFE0
void FlightDisplay_PresentBackBuffer(void) {
#ifdef XW_MODERN
	if (XwPresentation_BaseSource() == XW_PRESENT_DOS_FLIGHT)
		return;
#endif
#ifdef XW_MODERN
	if (!g_quitRequested)
		XwDisplay_Blit(g_flightPrimarySurface, NULL, g_flightBackBuffer, NULL, DDBLT_WAIT, NULL);
#else
	while (g_windowActive == 0) {
		if (g_quitRequested != 0)
			return;
		Flight_PumpWindowMessages();
	}
	if (g_quitRequested == 0)
		g_flightPrimarySurface->lpVtbl->Blt(g_flightPrimarySurface, NULL, g_flightBackBuffer, NULL,
											DDBLT_WAIT, NULL);
#endif
}

// FUNCTION: XW 0x4AE210
void FlightDisplay_BlitRenderSurface(void) {
#ifdef XW_MODERN
	if (XwPresentation_BaseSource() == XW_PRESENT_DOS_FLIGHT)
		return;
	if (g_quitRequested)
		return;
	{
		int offsetX = (g_flightDisplayWidth - g_surfaceWidth) / 2;
		int offsetY = (g_flightDisplayHeight - g_surfaceHeight) / 2;
		XwDirectDrawRect destination = { offsetX, offsetY, offsetX + g_surfaceWidth,
										 offsetY + g_surfaceHeight };
		XwDirectDrawRect source = { 0, 0, g_surfaceWidth, g_surfaceHeight };
		DDBLTFX effects = { 0 };
		effects.dwSize = sizeof effects;
		effects.dwROP = DDROP_SRCCOPY;
		XwDisplay_Blit(g_flightBackBuffer, &destination, g_flightOffscreenSurface, &source, DDBLT_ROP,
					   &effects);
	}
#else
	XwDirectDrawRect destinationRect;
	XwDirectDrawRect sourceRect;
	DDBLTFX blitEffects;
	HRESULT result;
	while (g_windowActive == 0) {
		if (g_quitRequested != 0)
			return;
		Flight_PumpWindowMessages();
	}
	if (g_quitRequested == 0) {
		memset(&blitEffects, 0, sizeof(blitEffects));
		blitEffects.dwSize = sizeof(blitEffects);
		blitEffects.dwROP = DDROP_SRCCOPY;
		result = DX_DDERR_WASSTILLDRAWING;
		while (result == DX_DDERR_WASSTILLDRAWING) {
			uint32_t offsetX = ((uint32_t)g_flightDisplayWidth - (uint32_t)g_surfaceWidth) / 2;
			uint32_t offsetY = ((uint32_t)g_flightDisplayHeight - (uint32_t)g_surfaceHeight) / 2;
			destinationRect.left = offsetX;
			destinationRect.right = (uint32_t)g_surfaceWidth + offsetX;
			destinationRect.top = offsetY;
			destinationRect.bottom = (uint32_t)g_surfaceHeight + offsetY;
			sourceRect.left = 0;
			sourceRect.top = 0;
			sourceRect.right = g_surfaceWidth;
			sourceRect.bottom = g_surfaceHeight;
			result = g_flightBackBuffer->lpVtbl->Blt(g_flightBackBuffer, &destinationRect,
													 g_flightOffscreenSurface, &sourceRect, DDBLT_ROP,
													 &blitEffects);
			if (result == DX_DD_OK)
				break;
			if (result == DX_DDERR_SURFACELOST) {
				if (!FlightDisplay_RestorePrimarySurface())
					return;
				FlightDisplay_PostPrimarySurfaceCreateOrRestoreStub();
			}
		}
	}
#endif
}

// FUNCTION: XW 0x4AE320
void FlightDisplay_RebuildForMode(int requestedMode) {
	HRESULT createResult;
	HRESULT cooperativeResult;
	int modeWidth;
	int modeHeight;
	int hardwareEnabled;
	int grayLevel;
	unsigned int bytesPerPixel;
	DDSURFACEDESC surfaceDesc;
	DDSCAPS backBufferCaps;
#ifdef XW_MODERN
	int savedLocks;
#endif
	if (g_appliedDisplayMode == requestedMode || g_flightDirectDraw == NULL)
		return;
#ifdef XW_MODERN
	savedLocks = XwDisplay_BeginSurfaceChange();
#endif
	if (g_useHardware3D != 0) {
		std3D_DetachAndReleaseZBufferSurface();
		std3D_Close();
		std3D_Shutdown();
	}
	if (g_flightOffscreenSurface != NULL) {
		g_flightOffscreenSurface->lpVtbl->Release(g_flightOffscreenSurface);
		g_flightOffscreenSurface = NULL;
	}
	if (g_flightAuxiliarySurface != NULL) {
		g_flightAuxiliarySurface->lpVtbl->Release(g_flightAuxiliarySurface);
		g_flightAuxiliarySurface = NULL;
	}
	if (g_flightBackBuffer != NULL) {
		FlightDisplay_ClearSurface(g_flightBackBuffer);
		g_flightBackBuffer->lpVtbl->Release(g_flightBackBuffer);
		g_flightBackBuffer = NULL;
	}
	if (g_flightPrimarySurface != NULL) {
		FlightDisplay_ClearSurface(g_flightPrimarySurface);
		g_flightPrimarySurface->lpVtbl->Release(g_flightPrimarySurface);
		g_flightPrimarySurface = NULL;
	}
	if (g_ddPalette != NULL) {
		g_ddPalette->lpVtbl->Release(g_ddPalette);
		g_ddPalette = NULL;
	}
	if (g_flightFullscreen == 0) {
		cooperativeResult = g_flightDirectDraw->lpVtbl->SetCooperativeLevel(
			g_flightDirectDraw, g_flightMainWindowHandle,
			(DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE | DDSCL_ALLOWMODEX));
		if (cooperativeResult != 0) {
			exit(FLIGHT_DISPLAY_EXIT_COOPERATIVE);
		}
	}
	if (FlightDisplay_LoadDriverGuid() != NULL) {
		if (g_appliedDisplayMode == FLIGHT_DISPLAY_MODE_1FFH) {
			g_flightDirectDraw->lpVtbl->Release(g_flightDirectDraw);
#ifdef XW_MODERN
			createResult = DirectDrawCreate_Compat(NULL, &g_flightDirectDraw, NULL);
#else
			createResult = DirectDrawCreate(NULL, &g_flightDirectDraw, NULL);
#endif
			if (createResult != 0)
				exit(FLIGHT_DISPLAY_EXIT_DEFAULT_DRIVER);
			cooperativeResult = g_flightDirectDraw->lpVtbl->SetCooperativeLevel(
				g_flightDirectDraw, g_flightMainWindowHandle,
				(DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE | DDSCL_ALLOWMODEX));
			if (cooperativeResult != 0)
				exit(FLIGHT_DISPLAY_EXIT_DEFAULT_COOPERATIVE);
		} else if (requestedMode == FLIGHT_DISPLAY_MODE_1FFH) {
			g_flightDirectDraw->lpVtbl->Release(g_flightDirectDraw);
#ifdef XW_MODERN
			createResult = DirectDrawCreate_Compat(FlightDisplay_LoadDriverGuid(), &g_flightDirectDraw, NULL);
#else
			createResult = DirectDrawCreate(FlightDisplay_LoadDriverGuid(), &g_flightDirectDraw, NULL);
#endif
			if (createResult != 0) {
#ifdef XW_MODERN
				createResult = DirectDrawCreate_Compat(NULL, &g_flightDirectDraw, NULL);
#else
				createResult = DirectDrawCreate(NULL, &g_flightDirectDraw, NULL);
#endif
				if (createResult != 0)
					exit(FLIGHT_DISPLAY_EXIT_FALLBACK_DRIVER);
				requestedMode = FLIGHT_DISPLAY_MODE_111H;
			}
			cooperativeResult = g_flightDirectDraw->lpVtbl->SetCooperativeLevel(
				g_flightDirectDraw, g_flightMainWindowHandle,
				(DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE | DDSCL_ALLOWMODEX));
			if (cooperativeResult != 0)
				exit(FLIGHT_DISPLAY_EXIT_SELECTED_COOPERATIVE);
		}
	}

	switch (requestedMode) {
		case FLIGHT_DISPLAY_MODE_13H:
			modeWidth = FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH;
			modeHeight = FLIGHT_DISPLAY_LOW_RESOLUTION_HEIGHT;
			if (g_flightDirectDraw->lpVtbl->SetDisplayMode(g_flightDirectDraw, modeWidth, modeHeight,
														   FLIGHT_DISPLAY_PIXEL_FORMAT_INDEXED) != 0) {
				requestedMode = g_appliedDisplayMode;
			} else {
				g_appliedDisplayMode = FLIGHT_DISPLAY_MODE_13H;
				g_flightBytesPerPixel = FLIGHT_DISPLAY_INDEXED_BYTES_PER_PIXEL;
				g_surfaceWidth = modeWidth;
				g_surfaceHeight = modeHeight;
				g_flightDisplayWidth = modeWidth;
				g_flightDisplayHeight = modeHeight;
			}
			break;
		case FLIGHT_DISPLAY_MODE_101H:
			modeWidth = FLIGHT_DISPLAY_WIDTH;
			modeHeight = FLIGHT_DISPLAY_HEIGHT;
			if (g_flightDirectDraw->lpVtbl->SetDisplayMode(g_flightDirectDraw, modeWidth, modeHeight,
														   FLIGHT_DISPLAY_PIXEL_FORMAT_INDEXED) != 0) {
				requestedMode = g_appliedDisplayMode;
			} else {
				g_appliedDisplayMode = FLIGHT_DISPLAY_MODE_101H;
				g_flightBytesPerPixel = FLIGHT_DISPLAY_INDEXED_BYTES_PER_PIXEL;
				g_surfaceWidth = modeWidth;
				g_surfaceHeight = modeHeight;
				g_flightDisplayWidth = modeWidth;
				g_flightDisplayHeight = modeHeight;
			}
			break;
		case FLIGHT_DISPLAY_MODE_111H:
			modeWidth = FLIGHT_DISPLAY_WIDTH;
			modeHeight = FLIGHT_DISPLAY_HEIGHT;
			if (g_flightDirectDraw->lpVtbl->SetDisplayMode(g_flightDirectDraw, modeWidth, modeHeight,
														   FLIGHT_DISPLAY_DIRECT_BITS_PER_PIXEL) != 0) {
				requestedMode = g_appliedDisplayMode;
			} else {
				g_appliedDisplayMode = FLIGHT_DISPLAY_MODE_111H;
				g_flightBytesPerPixel = FLIGHT_DISPLAY_DIRECT_BYTES_PER_PIXEL;
				g_surfaceWidth = modeWidth;
				g_surfaceHeight = modeHeight;
				g_flightDisplayWidth = modeWidth;
				g_flightDisplayHeight = modeHeight;
				g_useHardware3D = 0;
			}
			break;
		case FLIGHT_DISPLAY_MODE_1FFH:
			modeWidth = FLIGHT_DISPLAY_WIDTH;
			modeHeight = FLIGHT_DISPLAY_HEIGHT;
			if (g_flightDirectDraw->lpVtbl->SetDisplayMode(g_flightDirectDraw, modeWidth, modeHeight,
														   FLIGHT_DISPLAY_DIRECT_BITS_PER_PIXEL) != 0) {
				requestedMode = g_appliedDisplayMode;
			} else {
				g_appliedDisplayMode = FLIGHT_DISPLAY_MODE_1FFH;
				g_flightBytesPerPixel = FLIGHT_DISPLAY_DIRECT_BYTES_PER_PIXEL;
				g_surfaceWidth = modeWidth;
				g_surfaceHeight = modeHeight;
				g_flightDisplayWidth = modeWidth;
				g_flightDisplayHeight = modeHeight;
				g_useHardware3D = 1;
			}
			break;
	}
	g_flightResolutionMode = requestedMode;
	if (g_flightBytesPerPixel != FLIGHT_DISPLAY_DIRECT_BYTES_PER_PIXEL) {
		hardwareEnabled = 0;
		g_useHardware3D = 0;
	} else {
		hardwareEnabled = g_useHardware3D;
	}
	if (g_flightFullscreen != 0) {
		memset(&surfaceDesc, 0, sizeof(surfaceDesc));
		surfaceDesc.dwSize = sizeof(surfaceDesc);
		surfaceDesc.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		surfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		surfaceDesc.dwBackBufferCount = 1;
		if (hardwareEnabled != 0)
			surfaceDesc.ddsCaps.dwCaps =
				DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_3DDEVICE;
		if (g_flightDirectDraw->lpVtbl->CreateSurface(g_flightDirectDraw, &surfaceDesc,
													  &g_flightPrimarySurface, NULL) != 0) {
			FlightDisplay_CleanupAndReportError(FLIGHT_DISPLAY_ERROR_PRIMARY);
			return;
		}

		g_pixelFormatCode = g_flightBytesPerPixel != FLIGHT_DISPLAY_DIRECT_BYTES_PER_PIXEL
								? FLIGHT_DISPLAY_PIXEL_FORMAT_INDEXED
								: FLIGHT_DISPLAY_PIXEL_FORMAT_RGB565;
		memset(&surfaceDesc, 0, sizeof(surfaceDesc));
		surfaceDesc.dwSize = sizeof(surfaceDesc);
		g_flightPrimarySurface->lpVtbl->GetSurfaceDesc(g_flightPrimarySurface, &surfaceDesc);
		g_flightSurfacePitchBytes = surfaceDesc.lPitch;
		g_surfacePitch = surfaceDesc.lPitch;
		if ((surfaceDesc.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8) != 0) {
			g_flightBytesPerPixel = FLIGHT_DISPLAY_INDEXED_BYTES_PER_PIXEL;
			g_pixelFormatCode = FLIGHT_DISPLAY_PIXEL_FORMAT_INDEXED;
		} else if ((surfaceDesc.ddpfPixelFormat.dwFlags & DDPF_RGB) != 0) {
			g_flightBytesPerPixel = FLIGHT_DISPLAY_DIRECT_BYTES_PER_PIXEL;
			g_pixelFormatCode = (surfaceDesc.ddpfPixelFormat.dwGBitMask & FLIGHT_DISPLAY_GREEN_SIXTH_BIT) != 0
									? FLIGHT_DISPLAY_PIXEL_FORMAT_RGB565
									: FLIGHT_DISPLAY_PIXEL_FORMAT_RGB555;
		}

		FlightDisplay_ClearSurface(g_flightPrimarySurface);
		backBufferCaps.dwCaps = DDSCAPS_BACKBUFFER;
		if (g_flightPrimarySurface->lpVtbl->GetAttachedSurface(g_flightPrimarySurface, &backBufferCaps,
															   &g_flightBackBuffer) != 0) {
			FlightDisplay_CleanupAndReportError(FLIGHT_DISPLAY_ERROR_BACKBUFFER);
			return;
		}

		surfaceDesc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		surfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		if (g_useHardware3D != 0)
			surfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
		surfaceDesc.dwWidth = g_surfaceWidth;
		surfaceDesc.dwHeight = g_surfaceHeight;
		if (g_flightDirectDraw->lpVtbl->CreateSurface(g_flightDirectDraw, &surfaceDesc,
													  &g_flightOffscreenSurface, NULL) != 0) {
			FlightDisplay_CleanupAndReportError(FLIGHT_DISPLAY_ERROR_OFFSCREEN);
			return;
		}

		surfaceDesc.dwWidth = g_surfaceWidth;
		surfaceDesc.dwHeight = g_surfaceHeight;
		surfaceDesc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		surfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
		if (g_flightDirectDraw->lpVtbl->CreateSurface(g_flightDirectDraw, &surfaceDesc,
													  &g_flightAuxiliarySurface, NULL) != 0) {
			FlightDisplay_CleanupAndReportError(FLIGHT_DISPLAY_ERROR_OFFSCREEN);
			return;
		}

		FlightDisplay_ClearSurface(g_flightBackBuffer);
		g_flightRenderSurface = g_flightBackBuffer;
		FlightDisplay_ClearSurface(g_flightOffscreenSurface);
		FlightDisplay_ClearSurface(g_flightAuxiliarySurface);
	} else {
		if (hardwareEnabled != 0) {
			memset(&surfaceDesc, 0, sizeof(surfaceDesc));
			surfaceDesc.dwSize = sizeof(surfaceDesc);
			surfaceDesc.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
			surfaceDesc.dwBackBufferCount = 1;
			surfaceDesc.ddsCaps.dwCaps =
				DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_3DDEVICE;
			if (g_flightDirectDraw->lpVtbl->CreateSurface(g_flightDirectDraw, &surfaceDesc,
														  &g_flightPrimarySurface, NULL) != 0) {
				FlightDisplay_CleanupAndReportError(FLIGHT_DISPLAY_ERROR_PRIMARY);
				return;
			}

			g_pixelFormatCode = g_flightBytesPerPixel != FLIGHT_DISPLAY_DIRECT_BYTES_PER_PIXEL
									? FLIGHT_DISPLAY_PIXEL_FORMAT_INDEXED
									: FLIGHT_DISPLAY_PIXEL_FORMAT_RGB565;
			memset(&surfaceDesc, 0, sizeof(surfaceDesc));
			surfaceDesc.dwSize = sizeof(surfaceDesc);
			g_flightPrimarySurface->lpVtbl->GetSurfaceDesc(g_flightPrimarySurface, &surfaceDesc);
			g_flightSurfacePitchBytes = surfaceDesc.lPitch;
			g_surfacePitch = surfaceDesc.lPitch;
			if ((surfaceDesc.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8) != 0) {
				g_flightBytesPerPixel = FLIGHT_DISPLAY_INDEXED_BYTES_PER_PIXEL;
				g_pixelFormatCode = FLIGHT_DISPLAY_PIXEL_FORMAT_INDEXED;
			} else if ((surfaceDesc.ddpfPixelFormat.dwFlags & DDPF_RGB) != 0) {
				g_flightBytesPerPixel = FLIGHT_DISPLAY_DIRECT_BYTES_PER_PIXEL;
				g_pixelFormatCode =
					(surfaceDesc.ddpfPixelFormat.dwGBitMask & FLIGHT_DISPLAY_GREEN_SIXTH_BIT) != 0
						? FLIGHT_DISPLAY_PIXEL_FORMAT_RGB565
						: FLIGHT_DISPLAY_PIXEL_FORMAT_RGB555;
			}

			FlightDisplay_ClearSurface(g_flightPrimarySurface);
			backBufferCaps.dwCaps = DDSCAPS_BACKBUFFER;
			if (g_flightPrimarySurface->lpVtbl->GetAttachedSurface(g_flightPrimarySurface, &backBufferCaps,
																   &g_flightBackBuffer) != 0) {
				FlightDisplay_CleanupAndReportError(FLIGHT_DISPLAY_ERROR_BACKBUFFER);
				return;
			}

			g_flightRenderSurface = g_flightBackBuffer;
			cooperativeResult = g_flightDirectDraw->lpVtbl->SetCooperativeLevel(
				g_flightDirectDraw, g_flightMainWindowHandle, DDSCL_NORMAL);
			if (cooperativeResult != 0)
				exit(FLIGHT_DISPLAY_EXIT_COOPERATIVE);
		} else {
			cooperativeResult = g_flightDirectDraw->lpVtbl->SetCooperativeLevel(
				g_flightDirectDraw, g_flightMainWindowHandle, DDSCL_NORMAL);
			if (cooperativeResult != 0)
				exit(FLIGHT_DISPLAY_EXIT_COOPERATIVE);
			memset(&surfaceDesc, 0, sizeof(surfaceDesc));
			surfaceDesc.dwSize = sizeof(surfaceDesc);
			surfaceDesc.dwFlags = DDSD_CAPS;
			surfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
			if (g_flightDirectDraw->lpVtbl->CreateSurface(g_flightDirectDraw, &surfaceDesc,
														  &g_flightPrimarySurface, NULL) != 0)
				exit(FLIGHT_DISPLAY_EXIT_PRIMARY);
			memset(&surfaceDesc, 0, sizeof(surfaceDesc));
			surfaceDesc.dwSize = sizeof(surfaceDesc);
			g_flightPrimarySurface->lpVtbl->GetSurfaceDesc(g_flightPrimarySurface, &surfaceDesc);
			g_flightSurfacePitchBytes = surfaceDesc.lPitch;
			if ((surfaceDesc.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8) != 0 ||
				(surfaceDesc.ddpfPixelFormat.dwFlags & DDPF_RGB) == 0) {
				bytesPerPixel = FLIGHT_DISPLAY_INDEXED_BYTES_PER_PIXEL;
				g_pixelFormatCode = FLIGHT_DISPLAY_PIXEL_FORMAT_INDEXED;
				g_flightBytesPerPixel = FLIGHT_DISPLAY_INDEXED_BYTES_PER_PIXEL;
			} else {
				bytesPerPixel = FLIGHT_DISPLAY_DIRECT_BYTES_PER_PIXEL;
				g_flightBytesPerPixel = FLIGHT_DISPLAY_DIRECT_BYTES_PER_PIXEL;
				g_pixelFormatCode =
					(surfaceDesc.ddpfPixelFormat.dwGBitMask & FLIGHT_DISPLAY_GREEN_SIXTH_BIT) != 0
						? FLIGHT_DISPLAY_PIXEL_FORMAT_RGB565
						: FLIGHT_DISPLAY_PIXEL_FORMAT_RGB555;
			}
			surfaceDesc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
			surfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
			surfaceDesc.dwWidth = surfaceDesc.lPitch / bytesPerPixel;
			surfaceDesc.dwHeight = g_flightDisplayHeight;
			if (g_flightDirectDraw->lpVtbl->CreateSurface(g_flightDirectDraw, &surfaceDesc,
														  &g_flightBackBuffer, NULL) != 0) {
				FlightDisplay_CleanupAndReportError(FLIGHT_DISPLAY_ERROR_OFFSCREEN);
				return;
			}
		}
		surfaceDesc.dwWidth = g_surfaceWidth;
		surfaceDesc.dwHeight = g_surfaceHeight;
		surfaceDesc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		surfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
		if (g_flightDirectDraw->lpVtbl->CreateSurface(g_flightDirectDraw, &surfaceDesc,
													  &g_flightOffscreenSurface, NULL) != 0) {
			FlightDisplay_CleanupAndReportError(FLIGHT_DISPLAY_ERROR_OFFSCREEN);
			return;
		}

		surfaceDesc.dwWidth = g_surfaceWidth;
		surfaceDesc.dwHeight = g_surfaceHeight;
		surfaceDesc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		surfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
		if (g_flightDirectDraw->lpVtbl->CreateSurface(g_flightDirectDraw, &surfaceDesc,
													  &g_flightAuxiliarySurface, NULL) != 0) {
			FlightDisplay_CleanupAndReportError(FLIGHT_DISPLAY_ERROR_OFFSCREEN);
			return;
		}
	}
	if (g_flightBytesPerPixel == FLIGHT_DISPLAY_INDEXED_BYTES_PER_PIXEL) {
		for (grayLevel = 0; grayLevel < FLIGHT_DISPLAY_PALETTE_ENTRIES; ++grayLevel) {
			g_directDrawPaletteEntries[grayLevel].red = (uint8_t)grayLevel;
			g_directDrawPaletteEntries[grayLevel].green = (uint8_t)grayLevel;
			g_directDrawPaletteEntries[grayLevel].blue = (uint8_t)grayLevel;
		}
		g_flightDirectDraw->lpVtbl->CreatePalette(
			g_flightDirectDraw, DDPCAPS_8BIT | FLIGHT_DISPLAY_PALETTE_INITIALIZE | DDPCAPS_ALLOW256,
			g_directDrawPaletteEntries, &g_ddPalette, NULL);
		if (g_ddPalette != NULL)
			g_flightPrimarySurface->lpVtbl->SetPalette(g_flightPrimarySurface, g_ddPalette);
	} else {
		g_ddPalette = NULL;
	}
	if (g_useHardware3D != 0)
		Renderer_InitD3DDevice();
	FlightDisplay_InitGrayscaleTable();
#ifdef XW_MODERN
	XwDisplay_EndSurfaceChange(savedLocks);
#endif
}

// FUNCTION: XW 0x4AEC40
void FlightDisplay_ClearSurface(IDirectDrawSurface* surface) {
#ifdef XW_MODERN
	XwDisplay_ClearSurface(surface);
#else
	DDBLTFX blitEffects;
	HRESULT result;
	if (surface != NULL) {
		while (g_windowActive == 0) {
			if (g_quitRequested != 0)
				break;
			Flight_PumpWindowMessages();
		}
		blitEffects.dwSize = sizeof(blitEffects);
		blitEffects.dwFillColor = 0;
		do {
			result = surface->lpVtbl->Blt(surface, NULL, NULL, NULL, DDBLT_COLORFILL, &blitEffects);
			if (result == DX_DD_OK)
				break;
			if (result == DX_DDERR_SURFACELOST) {
				if (!FlightDisplay_RestorePrimarySurface())
					return;
				FlightDisplay_PostPrimarySurfaceCreateOrRestoreStub();
			}
		} while (result == DX_DDERR_WASSTILLDRAWING);
	}
#endif
}

// FUNCTION: XW 0x4AECC0
int32_t FlightDisplay_RestorePrimarySurface(void) {
	HRESULT result = g_flightPrimarySurface->lpVtbl->Restore(g_flightPrimarySurface);
	if (result == DX_DD_OK) {
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x4AED00
int32_t FlightDisplay_IsPixelFormat555(void) {
	if (g_loadingModel != 0 && g_useHardware3D != 0) {
		return ModelTexture_IsHardwareFormat555();
	}
	return g_pixelFormatCode == FLIGHT_DISPLAY_PIXEL_FORMAT_RGB555;
}

// FUNCTION: XW 0x4AEDD0
void FlightDisplay_ClearBackAndAuxiliarySurfaces(void) {
	FlightDisplay_ClearSurface(g_flightBackBuffer);
	FlightDisplay_ClearSurface(g_flightAuxiliarySurface);
}

// FUNCTION: XW 0x4AEDF0
void FlightDisplay_ClearOffscreenSurface(void) { FlightDisplay_ClearSurface(g_flightOffscreenSurface); }

// FUNCTION: XW 0x4AEE00
int32_t FlightDisplay_ShowStartupMessageBox(const char* text, const char* caption, int32_t allowCancel) {
	int result;
	if (g_flightDirectDraw != NULL) {
		g_flightDirectDraw->lpVtbl->FlipToGDISurface(g_flightDirectDraw);
	}
	if (allowCancel != 0) {
		result = XwPort_ShowStartupDialog(NULL, text, caption, XW_STARTUP_DIALOG_OK_CANCEL);
	} else {
		result = XwPort_ShowStartupDialog(NULL, text, caption, XW_STARTUP_DIALOG_OK);
	}
	return result == XW_STARTUP_DIALOG_RESULT_OK;
}
