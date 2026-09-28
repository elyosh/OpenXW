#ifndef XW_FLIGHT_FLIGHT_DISPLAY_H
#define XW_FLIGHT_FLIGHT_DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/render/shade.h"

#include <aeron/compat/ddraw.h>
#include <aeron/compat/win_types.h>
#include <stddef.h>
#include <stdint.h>

enum {
	FLIGHT_DISPLAY_MODE_13H = 0x13,
	FLIGHT_DISPLAY_MODE_101H = 0x101,
	FLIGHT_DISPLAY_MODE_111H = 0x111,
	FLIGHT_DISPLAY_MODE_1FFH = 0x1FF
};

enum {
	FLIGHT_DISPLAY_LOW_RESOLUTION_HEIGHT = 200,
	FLIGHT_DISPLAY_DIRECT_BITS_PER_PIXEL = 16,
	FLIGHT_DISPLAY_PIXEL_FORMAT_RGB565 = 565,
	FLIGHT_DISPLAY_GREEN_SIXTH_BIT = 0x400,
	FLIGHT_DISPLAY_PALETTE_INITIALIZE = 8,
	FLIGHT_DISPLAY_ERROR_PRIMARY = 4,
	FLIGHT_DISPLAY_ERROR_BACKBUFFER = 5,
	FLIGHT_DISPLAY_ERROR_OFFSCREEN = 6,
	FLIGHT_DISPLAY_EXIT_COOPERATIVE = 100,
	FLIGHT_DISPLAY_EXIT_PRIMARY = 101,
	FLIGHT_DISPLAY_EXIT_DEFAULT_DRIVER = 102,
	FLIGHT_DISPLAY_EXIT_FALLBACK_DRIVER = 103,
	FLIGHT_DISPLAY_EXIT_DEFAULT_COOPERATIVE = 104,
	FLIGHT_DISPLAY_EXIT_SELECTED_COOPERATIVE = 105
};

enum {
	FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH = 320,
	FLIGHT_DISPLAY_WIDTH = 640,
	FLIGHT_DISPLAY_HEIGHT = 480,
	FLIGHT_DISPLAY_GRAYSCALE_ENTRIES = 256,
	FLIGHT_DISPLAY_PALETTE_ENTRIES = 256,
	FLIGHT_DISPLAY_RGB6_TO_RGB8_SHIFT = 2,
	FLIGHT_DISPLAY_DEBUG_MESSAGE_CAPACITY = 256,
	FLIGHT_DISPLAY_PIXEL_FORMAT_INDEXED = 8,
	FLIGHT_DISPLAY_PIXEL_FORMAT_RGB555 = 555,
	FLIGHT_DISPLAY_INDEXED_BYTES_PER_PIXEL = 1,
	FLIGHT_DISPLAY_DIRECT_BYTES_PER_PIXEL = 2,
	FLIGHT_DISPLAY_INITIAL_BACKGROUND_PALETTE_INDEX = 251,
	FLIGHT_DISPLAY_INITIAL_PITCH_BYTES = 640,
	FLIGHT_DISPLAY_INITIAL_CLEAR_CHUNK_SIZE = 0xF000,
	FLIGHT_DISPLAY_INITIAL_RESOLUTION_MODE = FLIGHT_DISPLAY_MODE_111H,
#ifdef XW_MODERN
	FLIGHT_DISPLAY_INITIAL_FRAMEBUFFER_ADDRESS = 0,
#else
	FLIGHT_DISPLAY_INITIAL_FRAMEBUFFER_ADDRESS = 0xA0000,
#endif
	FLIGHT_DISPLAY_INITIAL_VIEWPORT_BYTE_SPAN = 0x10000
};

enum {
	FLIGHT_DISPLAY_VBLANK_SAMPLES = 100,
	FLIGHT_DISPLAY_VBLANK_PHASE_SCALE = 100000,
	FLIGHT_DISPLAY_VBLANK_SAMPLE_SCALE = 10000000,
	FLIGHT_DISPLAY_VBLANK_PHASE_WINDOW = 20000
};

enum {
	FLIGHT_DISPLAY_LOW_CLEAR_CHUNK_SIZE = 0x10000,
	FLIGHT_DISPLAY_INITIAL_LOW_RESOLUTION_STATE = 15,
	FLIGHT_DISPLAY_HIGH_RESOLUTION_PITCH = 1024,
	FLIGHT_DISPLAY_LOW_HUD_CLEAR_HEIGHT = 189,
	FLIGHT_DISPLAY_HIGH_HUD_CLEAR_HEIGHT = 455,
	FLIGHT_DISPLAY_LOW_PROJECTION_SHIFT = 8,
	FLIGHT_DISPLAY_HIGH_PROJECTION_SHIFT = 9,
	FLIGHT_DISPLAY_LOW_ASPECT_Q16 = 59578
};

extern int g_flightPageFlip;

enum {
	FLIGHT_DISPLAY_FALLBACK_WIDTH = 512,
	FLIGHT_DISPLAY_FALLBACK_HEIGHT = 384,
	FLIGHT_DISPLAY_LOW_MODE_HEIGHT = 240,
	FLIGHT_DISPLAY_ERROR_CREATE = 1,
	FLIGHT_DISPLAY_ERROR_COOPERATIVE = 2,
	FLIGHT_DISPLAY_ERROR_MODE = 3,
	FLIGHT_DISPLAY_ERROR_POST_CREATE = 12,
	FLIGHT_DISPLAY_CAPS_COLORKEY = 0x400000,
	FLIGHT_DISPLAY_CKEY_DESTBLT = 0x1,
	FLIGHT_DISPLAY_CKEY_SRCBLT = 0x200,
	FLIGHT_DISPLAY_COLORKEY_DEFAULT = 1,
	FLIGHT_DISPLAY_COLORKEY_DEST_ONLY = 2,
	FLIGHT_DISPLAY_CAPS_ROP_COUNT = 8
};

/* DirectX 5 capability record used by the original GetCaps call. */
typedef struct XwDirectDrawCapsDx5 {
	uint32_t dwSize;
	uint32_t dwCaps;
	uint32_t dwCaps2;
	uint32_t dwCKeyCaps;
	uint32_t dwFXCaps;
	uint32_t dwFXAlphaCaps;
	uint32_t dwPalCaps;
	uint32_t dwSVCaps;
	uint32_t dwAlphaBltConstBitDepths;
	uint32_t dwAlphaBltPixelBitDepths;
	uint32_t dwAlphaBltSurfaceBitDepths;
	uint32_t dwAlphaOverlayConstBitDepths;
	uint32_t dwAlphaOverlayPixelBitDepths;
	uint32_t dwAlphaOverlaySurfaceBitDepths;
	uint32_t dwZBufferBitDepths;
	uint32_t dwVidMemTotal;
	uint32_t dwVidMemFree;
	uint32_t dwMaxVisibleOverlays;
	uint32_t dwCurrVisibleOverlays;
	uint32_t dwNumFourCCCodes;
	uint32_t dwAlignBoundarySrc;
	uint32_t dwAlignSizeSrc;
	uint32_t dwAlignBoundaryDest;
	uint32_t dwAlignSizeDest;
	uint32_t dwAlignStrideAlign;
	uint32_t dwRops[FLIGHT_DISPLAY_CAPS_ROP_COUNT];
	DDSCAPS ddsCaps;
	uint32_t dwMinOverlayStretch;
	uint32_t dwMaxOverlayStretch;
	uint32_t dwMinLiveVideoStretch;
	uint32_t dwMaxLiveVideoStretch;
	uint32_t dwMinHwCodecStretch;
	uint32_t dwMaxHwCodecStretch;
	uint32_t dwReserved1;
	uint32_t dwReserved2;
	uint32_t dwReserved3;
	uint32_t dwSVBCaps;
	uint32_t dwSVBCKeyCaps;
	uint32_t dwSVBFXCaps;
	uint32_t dwSVBRops[FLIGHT_DISPLAY_CAPS_ROP_COUNT];
	uint32_t dwVSBCaps;
	uint32_t dwVSBCKeyCaps;
	uint32_t dwVSBFXCaps;
	uint32_t dwVSBRops[FLIGHT_DISPLAY_CAPS_ROP_COUNT];
	uint32_t dwSSBCaps;
	uint32_t dwSSBCKeyCaps;
	uint32_t dwSSBFXCaps;
	uint32_t dwSSBRops[FLIGHT_DISPLAY_CAPS_ROP_COUNT];
	uint32_t dwMaxVideoPorts;
	uint32_t dwCurrVideoPorts;
	uint32_t dwSVBCaps2;
	uint32_t dwNLVBCaps;
	uint32_t dwNLVBCaps2;
	uint32_t dwNLVBCKeyCaps;
	uint32_t dwNLVBFXCaps;
	uint32_t dwNLVBRops[FLIGHT_DISPLAY_CAPS_ROP_COUNT];
} XwDirectDrawCapsDx5;

extern int g_flightFullscreen;
extern int g_appliedDisplayMode;
extern IDirectDrawSurface* g_flightRenderSurface;
extern int g_flightConfFlicker;
extern uint32_t g_flightVblankEpochMs;
extern uint32_t g_flightVblankFrequencyEstimate;
extern const int g_flightVblankPhaseWindow;

typedef struct XwDirectDrawPaletteEntry {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t flags;
} XwDirectDrawPaletteEntry;

typedef struct XwDirectDrawRect {
	int32_t left;
	int32_t top;
	int32_t right;
	int32_t bottom;
} XwDirectDrawRect;

extern uint8_t* g_flightSwFramebufferBase;
extern uint8_t* g_surfacePixels;
extern uint8_t g_flightBackgroundPaletteIndex;
extern int g_swFramebufferClearChunkSize;
extern int g_flightLowResolutionState;
extern int g_flightScreenWidth;
extern unsigned int g_flightScreenHeight;
extern int g_surfacePitch;
extern int g_flightBytesPerPixel;
extern int g_flightSurfaceAlreadyLocked;
extern unsigned int g_flightResolutionMode;
extern int g_flightDefaultResolutionMode;
extern uint8_t* g_swFramebufferBase;
extern int g_flightSurfaceViewport480ByteSpan;
extern int g_frontendDisplayWndProcMode;
extern int g_pixelFormatCode;
extern int g_flightGrayscaleInitialized;
extern XwGrayscaleEntry g_flightGrayscaleEntries[FLIGHT_DISPLAY_GRAYSCALE_ENTRIES];
extern int g_surfaceLockCount;
extern IDirectDraw* g_flightDirectDraw;
extern IDirectDrawSurface* g_flightPrimarySurface;
extern int g_flightSurfacePitchBytes;
extern int g_flightColorKeyCapabilityMode;

extern int g_flightLockOffscreenSurface;
extern DxGuid g_configuredDirectDrawDriverGuid;
extern IDirectDrawSurface* g_flightOffscreenSurface;
extern IDirectDrawPalette* g_ddPalette;
extern char g_flightDisplayDebugMessage[FLIGHT_DISPLAY_DEBUG_MESSAGE_CAPACITY];
extern XwDirectDrawPaletteEntry g_directDrawPaletteEntries[FLIGHT_DISPLAY_PALETTE_ENTRIES];
extern IDirectDrawSurface* g_flightAuxiliarySurface;
extern IDirectDrawSurface* g_flightBackBuffer;
extern int16_t g_flightDisplaySurfaceMode;

extern int g_surfaceWidth;
extern int g_surfaceHeight;
extern int g_flightDisplayWidth;
extern int g_flightDisplayHeight;

/* Declarations follow ascending original IDB address. */

/* 0x4AB960 */
void FlightDisplay_SetViewport480ByteSpan(int byteSpan);

/* 0x4AB970 */
void FlightDisplay_SetSoftwareFramebufferBase(uint8_t* framebufferBase);

/* 0x4AB980 */
uint8_t* FlightDisplay_GetSoftwareFramebufferBase(void);

/* 0x4ACC30 */
int32_t FlightDisplay_PostPrimarySurfaceCreateOrRestoreStub(void);

/* 0x4ACC40 */
const DxGuid* FlightDisplay_LoadDriverGuid(void);

/* 0x4ACC90 */
int FlightDisplay_Init(void);

/* 0x4AD5B0 */
int FlightDisplay_InitGrayscaleTable(void);

/* 0x4AD600 */
void FlightDisplay_SetPaletteEntries(const uint8_t* rgb6Palette, int startIndex, unsigned int count);

/* 0x4AD6E0 */
void FlightDisplay_RestorePaletteAfterActivation(void);

/* 0x4AD750 */
void FlightDisplay_UpdateCachedPaletteEntries(const uint8_t* rgb6Palette, int firstEntry, int entryCount);

/* 0x4AD820 */
int FlightDisplay_CleanupAndReportError(int errorCode);

/* 0x4AD8B0 */
int FlightDisplay_GetPrimarySurfacePitch(void);

/* 0x4AD8C0 */
int FlightDisplay_GetSurfaceLockCount(void);

/* 0x4AD8D0 */
void FlightDisplay_LockSurface(void);

/* 0x4ADC40 */
void FlightDisplay_UnlockSurface(void);

/* 0x4ADCF0 */
int FlightDisplay_Flip(void);

/* 0x4ADFE0 */
void FlightDisplay_PresentBackBuffer(void);

/* 0x4AE210 */
void FlightDisplay_BlitRenderSurface(void);

/* 0x4AE320 */
void FlightDisplay_RebuildForMode(int requestedMode);

/* 0x4AEC40 */
void FlightDisplay_ClearSurface(IDirectDrawSurface* surface);

/* 0x4AECC0 */
int32_t FlightDisplay_RestorePrimarySurface(void);

/* 0x4AED00 */
int32_t FlightDisplay_IsPixelFormat555(void);

/* 0x4AEDD0 */
void FlightDisplay_ClearBackAndAuxiliarySurfaces(void);

/* 0x4AEDF0 */
void FlightDisplay_ClearOffscreenSurface(void);

/* 0x4AEE00 */
int32_t FlightDisplay_ShowStartupMessageBox(const char* text, const char* caption, int32_t allowCancel);

#ifdef __cplusplus
}
#endif

#endif
