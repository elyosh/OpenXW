#include "xw/render/std3d.h"

#include "xw/flight/flight_display.h"
#include "xw/render/power_vr.h"
#include "xw/render/renderer.h"
#include "xw/util/folded.h"
#include "xw/util/shared.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4C3308
const DxGuid g_directDraw2InterfaceGuid = {
	0xB3A6F3E0, 0x2B43, 0x11CF, { 0xA2, 0xDE, 0x00, 0xAA, 0x00, 0xB9, 0x33, 0x56 }
};

// GLOBAL: XW 0x4C3378
const DxGuid IID_IDirect3D = {
	0x3BBA0080, 0x2421, 0x11CF, { 0xA3, 0x1A, 0x00, 0xAA, 0x00, 0xB9, 0x33, 0x56 }
};

// GLOBAL: XW 0x4C3528
const double g_textureTileCountRoundingBias = 0.5;

// GLOBAL: XW 0x4DFAD8
unsigned int g_std3DCapFlags = 0;

// GLOBAL: XW 0x4DFADC
Std3DRenderStateFlags g_d3dStateFlags = 0;

// GLOBAL: XW 0x4DFAE0
unsigned int g_std3DNumDevices = 0;

// GLOBAL: XW 0x4DFAE4
Std3DDevice* g_pStd3DCurDevice = NULL;

// GLOBAL: XW 0x4DFAE8
unsigned int g_std3DNumTextureFormats = 0;

// GLOBAL: XW 0x4DFAEC
Std3DTexFmt* g_pFmtRGB565 = NULL;

// GLOBAL: XW 0x4DFAF0
Std3DTexFmt* g_pFmtRGBA1555 = NULL;

// GLOBAL: XW 0x4DFAF4
Std3DTexFmt* g_pFmtRGBA4444 = NULL;

// GLOBAL: XW 0x4DFAF8
int g_std3DMinTextureWidth = STD3D_DEVICE_MIN_TEXTURE_SIZE;

// GLOBAL: XW 0x4DFAFC
int g_std3DMinTextureHeight = STD3D_DEVICE_MIN_TEXTURE_SIZE;

// GLOBAL: XW 0x4DFB08
unsigned int g_std3DExecBufMaxVerts = 0;

// GLOBAL: XW 0x4DFB0C
unsigned int g_std3DTextureFrameTag = 1;

// GLOBAL: XW 0x4DFB10
int g_texCacheCount = 0;

// GLOBAL: XW 0x4DFB14
Std3DTexCacheNode* g_pTexCacheHead = NULL;

// GLOBAL: XW 0x4DFB18
Std3DTexCacheNode* g_pTexCacheTail = NULL;

// GLOBAL: XW 0x4DFB1C
Std3DVBuffer* g_pStd3DVBuffer = NULL;

// GLOBAL: XW 0x4DFB20
IDirect3DExecuteBuffer* g_d3dExecuteBuffer = NULL;

// GLOBAL: XW 0x4DFB24
unsigned int g_std3DExecBufSize = 0;

// GLOBAL: XW 0x4DFB28
unsigned int g_d3dBufVertCount = 0;

// GLOBAL: XW 0x4DFB2C
unsigned int g_std3DExecBufTriCount = 0;

// GLOBAL: XW 0x4DFB30
Std3DTexCacheNode* g_d3dCurTexture = (Std3DTexCacheNode*)STD3D_TEXTURE_BINDING_UNSET;

// GLOBAL: XW 0x4DFB34
Std3DSurfaceBlock* g_pStd3DZBufferState = NULL;

// GLOBAL: XW 0x4DFB38
int g_std3DZBufferEnabled = 1;

// GLOBAL: XW 0x4DFB3C
IDirectDraw* g_std3DDirectDraw = NULL;

// GLOBAL: XW 0x4DFB40
int g_std3DStartupDone = 0;

// GLOBAL: XW 0x4DFB44
int g_std3DDeviceOpen = 0;

// GLOBAL: XW 0x4DFB48
const Std3DErrorStringEntry g_std3DErrorStringTable[STD3D_ERROR_STRING_COUNT] = {
	{ STD3D_D3D_OK, "D3D_OK" },
	{ STD3D_D3DERR_BADMAJORVERSION, "D3DERR_BADMAJORVERSION" },
	{ STD3D_D3DERR_BADMINORVERSION, "D3DERR_BADMINORVERSION" },
	{ STD3D_D3DERR_EXECUTE_DESTROY_FAILED, "D3DERR_EXECUTE_DESTROY_FAILED" },
	{ STD3D_D3DERR_EXECUTE_LOCK_FAILED, "D3DERR_EXECUTE_LOCK_FAILED" },
	{ STD3D_D3DERR_EXECUTE_UNLOCK_FAILED, "D3DERR_EXECUTE_UNLOCK_FAILED" },
	{ STD3D_D3DERR_EXECUTE_LOCKED, "D3DERR_EXECUTE_LOCKED" },
	{ STD3D_D3DERR_EXECUTE_NOT_LOCKED, "D3DERR_EXECUTE_NOT_LOCKED" },
	{ STD3D_D3DERR_EXECUTE_CLIPPED_FAILED, "D3DERR_EXECUTE_CLIPPED_FAILED" },
	{ STD3D_D3DERR_TEXTURE_CREATE_FAILED, "D3DERR_TEXTURE_CREATE_FAILED" },
	{ STD3D_D3DERR_TEXTURE_DESTROY_FAILED, "D3DERR_TEXTURE_DESTROY_FAILED" },
	{ STD3D_D3DERR_TEXTURE_LOCK_FAILED, "D3DERR_TEXTURE_LOCK_FAILED" },
	{ STD3D_D3DERR_TEXTURE_UNLOCK_FAILED, "D3DERR_TEXTURE_UNLOCK_FAILED" },
	{ STD3D_D3DERR_TEXTURE_LOAD_FAILED, "D3DERR_TEXTURE_LOAD_FAILED" },
	{ STD3D_D3DERR_TEXTURE_SWAP_FAILED, "D3DERR_TEXTURE_SWAP_FAILED" },
	{ STD3D_D3DERR_TEXTURE_LOCKED, "D3DERR_TEXTURE_LOCKED" },
	{ STD3D_D3DERR_TEXTURE_NOT_LOCKED, "D3DERR_TEXTURE_NOT_LOCKED" },
	{ STD3D_D3DERR_TEXTURE_GETSURF_FAILED, "D3DERR_TEXTURE_GETSURF_FAILED" },
	{ STD3D_D3DERR_MATRIX_DESTROY_FAILED, "D3DERR_MATRIX_DESTROY_FAILED" },
	{ STD3D_D3DERR_MATRIX_SETDATA_FAILED, "D3DERR_MATRIX_SETDATA_FAILED" },
	{ STD3D_D3DERR_MATRIX_GETDATA_FAILED, "D3DERR_MATRIX_GETDATA_FAILED" },
	{ STD3D_D3DERR_SETVIEWPORTDATA_FAILED, "D3DERR_SETVIEWPORTDATA_FAILED" },
	{ STD3D_D3DERR_MATERIAL_DESTROY_FAILED, "D3DERR_MATERIAL_DESTROY_FAILED" },
	{ STD3D_D3DERR_MATERIAL_SETDATA_FAILED, "D3DERR_MATERIAL_SETDATA_FAILED" },
	{ STD3D_D3DERR_MATERIAL_GETDATA_FAILED, "D3DERR_MATERIAL_GETDATA_FAILED" },
	{ STD3D_D3DERR_SCENE_NOT_IN_SCENE, "D3DERR_SCENE_NOT_IN_SCENE" },
	{ STD3D_D3DERR_SCENE_BEGIN_FAILED, "D3DERR_SCENE_BEGIN_FAILED" },
	{ STD3D_D3DERR_SCENE_END_FAILED, "D3DERR_SCENE_END_FAILED" },
	{ STD3D_DD_OK, "DD_OK" },
	{ STD3D_DDERR_ALREADYINITIALIZED, "DDERR_ALREADYINITIALIZED" },
	{ STD3D_DDERR_CANNOTATTACHSURFACE, "DDERR_CANNOTATTACHSURFACE" },
	{ STD3D_DDERR_CANNOTDETACHSURFACE, "DDERR_CANNOTDETACHSURFACE" },
	{ STD3D_DDERR_CURRENTLYNOTAVAIL, "DDERR_CURRENTLYNOTAVAIL" },
	{ STD3D_DDERR_EXCEPTION, "DDERR_EXCEPTION" },
	{ STD3D_DDERR_GENERIC, "DDERR_GENERIC" },
	{ STD3D_DDERR_HEIGHTALIGN, "DDERR_HEIGHTALIGN" },
	{ STD3D_DDERR_INCOMPATIBLEPRIMARY, "DDERR_INCOMPATIBLEPRIMARY" },
	{ STD3D_DDERR_INVALIDCAPS, "DDERR_INVALIDCAPS" },
	{ STD3D_DDERR_INVALIDCLIPLIST, "DDERR_INVALIDCLIPLIST" },
	{ STD3D_DDERR_INVALIDMODE, "DDERR_INVALIDMODE" },
	{ STD3D_DDERR_INVALIDOBJECT, "DDERR_INVALIDOBJECT" },
	{ STD3D_DDERR_INVALIDPARAMS, "DDERR_INVALIDPARAMS" },
	{ STD3D_DDERR_INVALIDPIXELFORMAT, "DDERR_INVALIDPIXELFORMAT" },
	{ STD3D_DDERR_INVALIDRECT, "DDERR_INVALIDRECT" },
	{ STD3D_DDERR_LOCKEDSURFACES, "DDERR_LOCKEDSURFACES" },
	{ STD3D_DDERR_NO3D, "DDERR_NO3D" },
	{ STD3D_DDERR_NOALPHAHW, "DDERR_NOALPHAHW" },
	{ STD3D_DDERR_NOCLIPLIST, "DDERR_NOCLIPLIST" },
	{ STD3D_DDERR_NOCOLORCONVHW, "DDERR_NOCOLORCONVHW" },
	{ STD3D_DDERR_NOCOOPERATIVELEVELSET, "DDERR_NOCOOPERATIVELEVELSET" },
	{ STD3D_DDERR_NOCOLORKEY, "DDERR_NOCOLORKEY" },
	{ STD3D_DDERR_NOCOLORKEYHW, "DDERR_NOCOLORKEYHW" },
	{ STD3D_DDERR_NODIRECTDRAWSUPPORT, "DDERR_NODIRECTDRAWSUPPORT" },
	{ STD3D_DDERR_NOEXCLUSIVEMODE, "DDERR_NOEXCLUSIVEMODE" },
	{ STD3D_DDERR_NOFLIPHW, "DDERR_NOFLIPHW" },
	{ STD3D_DDERR_NOGDI, "DDERR_NOGDI" },
	{ STD3D_DDERR_NOMIRRORHW, "DDERR_NOMIRRORHW" },
	{ STD3D_DDERR_NOTFOUND, "DDERR_NOTFOUND" },
	{ STD3D_DDERR_NOOVERLAYHW, "DDERR_NOOVERLAYHW" },
	{ STD3D_DDERR_NORASTEROPHW, "DDERR_NORASTEROPHW" },
	{ STD3D_DDERR_NOROTATIONHW, "DDERR_NOROTATIONHW" },
	{ STD3D_DDERR_NOSTRETCHHW, "DDERR_NOSTRETCHHW" },
	{ STD3D_DDERR_NOT4BITCOLOR, "DDERR_NOT4BITCOLOR" },
	{ STD3D_DDERR_NOT4BITCOLORINDEX, "DDERR_NOT4BITCOLORINDEX" },
	{ STD3D_DDERR_NOT8BITCOLOR, "DDERR_NOT8BITCOLOR" },
	{ STD3D_DDERR_NOTEXTUREHW, "DDERR_NOTEXTUREHW" },
	{ STD3D_DDERR_NOVSYNCHW, "DDERR_NOVSYNCHW" },
	{ STD3D_DDERR_NOZBUFFERHW, "DDERR_NOZBUFFERHW" },
	{ STD3D_DDERR_NOZOVERLAYHW, "DDERR_NOZOVERLAYHW" },
	{ STD3D_DDERR_OUTOFCAPS, "DDERR_OUTOFCAPS" },
	{ STD3D_DDERR_OUTOFMEMORY, "DDERR_OUTOFMEMORY" },
	{ STD3D_DDERR_OUTOFVIDEOMEMORY, "DDERR_OUTOFVIDEOMEMORY" },
	{ STD3D_DDERR_OVERLAYCANTCLIP, "DDERR_OVERLAYCANTCLIP" },
	{ STD3D_DDERR_OVERLAYCOLORKEYONLYONEACTIVE, "DDERR_OVERLAYCOLORKEYONLYONEACTIVE" },
	{ STD3D_DDERR_PALETTEBUSY, "DDERR_PALETTEBUSY" },
	{ STD3D_DDERR_COLORKEYNOTSET, "DDERR_COLORKEYNOTSET" },
	{ STD3D_DDERR_SURFACEALREADYATTACHED, "DDERR_SURFACEALREADYATTACHED" },
	{ STD3D_DDERR_SURFACEALREADYDEPENDENT, "DDERR_SURFACEALREADYDEPENDENT" },
	{ STD3D_DDERR_SURFACEBUSY, "DDERR_SURFACEBUSY" },
	{ STD3D_DDERR_SURFACEISOBSCURED, "DDERR_SURFACEISOBSCURED" },
	{ STD3D_DDERR_SURFACELOST, "DDERR_SURFACELOST" },
	{ STD3D_DDERR_SURFACENOTATTACHED, "DDERR_SURFACENOTATTACHED" },
	{ STD3D_DDERR_TOOBIGHEIGHT, "DDERR_TOOBIGHEIGHT" },
	{ STD3D_DDERR_TOOBIGSIZE, "DDERR_TOOBIGSIZE" },
	{ STD3D_DDERR_TOOBIGWIDTH, "DDERR_TOOBIGWIDTH" },
	{ STD3D_DDERR_UNSUPPORTED, "DDERR_UNSUPPORTED" },
	{ STD3D_DDERR_UNSUPPORTEDFORMAT, "DDERR_UNSUPPORTEDFORMAT" },
	{ STD3D_DDERR_UNSUPPORTEDMASK, "DDERR_UNSUPPORTEDMASK" },
	{ STD3D_DDERR_VERTICALBLANKINPROGRESS, "DDERR_VERTICALBLANKINPROGRESS" },
	{ STD3D_DDERR_WASSTILLDRAWING, "DDERR_WASSTILLDRAWING" },
	{ STD3D_DDERR_XALIGN, "DDERR_XALIGN" },
	{ STD3D_DDERR_INVALIDDIRECTDRAWGUID, "DDERR_INVALIDDIRECTDRAWGUID" },
	{ STD3D_DDERR_DIRECTDRAWALREADYCREATED, "DDERR_DIRECTDRAWALREADYCREATED" },
	{ STD3D_DDERR_NODIRECTDRAWHW, "DDERR_NODIRECTDRAWHW" },
	{ STD3D_DDERR_PRIMARYSURFACEALREADYEXISTS, "DDERR_PRIMARYSURFACEALREADYEXISTS" },
	{ STD3D_DDERR_NOEMULATION, "DDERR_NOEMULATION" },
	{ STD3D_DDERR_REGIONTOOSMALL, "DDERR_REGIONTOOSMALL" },
	{ STD3D_DDERR_CLIPPERISUSINGHWND, "DDERR_CLIPPERISUSINGHWND" },
	{ STD3D_DDERR_NOCLIPPERATTACHED, "DDERR_NOCLIPPERATTACHED" },
	{ STD3D_DDERR_NOHWND, "DDERR_NOHWND" },
	{ STD3D_DDERR_HWNDSUBCLASSED, "DDERR_HWNDSUBCLASSED" },
	{ STD3D_DDERR_HWNDALREADYSET, "DDERR_HWNDALREADYSET" },
	{ STD3D_DDERR_NOPALETTEATTACHED, "DDERR_NOPALETTEATTACHED" },
	{ STD3D_DDERR_NOPALETTEHW, "DDERR_NOPALETTEHW" },
	{ STD3D_DDERR_BLTFASTCANTCLIP, "DDERR_BLTFASTCANTCLIP" },
	{ STD3D_DDERR_NOBLTHW, "DDERR_NOBLTHW" },
	{ STD3D_DDERR_NODDROPSHW, "DDERR_NODDROPSHW" },
	{ STD3D_DDERR_OVERLAYNOTVISIBLE, "DDERR_OVERLAYNOTVISIBLE" },
	{ STD3D_DDERR_NOOVERLAYDEST, "DDERR_NOOVERLAYDEST" },
	{ STD3D_DDERR_INVALIDPOSITION, "DDERR_INVALIDPOSITION" },
	{ STD3D_DDERR_NOTAOVERLAYSURFACE, "DDERR_NOTAOVERLAYSURFACE" },
	{ STD3D_DDERR_EXCLUSIVEMODEALREADYSET, "DDERR_EXCLUSIVEMODEALREADYSET" },
	{ STD3D_DDERR_NOTFLIPPABLE, "DDERR_NOTFLIPPABLE" },
	{ STD3D_DDERR_CANTDUPLICATE, "DDERR_CANTDUPLICATE" },
	{ STD3D_DDERR_NOTLOCKED, "DDERR_NOTLOCKED" },
	{ STD3D_DDERR_CANTCREATEDC, "DDERR_CANTCREATEDC" },
	{ STD3D_DDERR_NODC, "DDERR_NODC" },
	{ STD3D_DDERR_WRONGMODE, "DDERR_WRONGMODE" },
	{ STD3D_DDERR_IMPLICITLYCREATED, "DDERR_IMPLICITLYCREATED" },
	{ STD3D_DDERR_NOTPALETTIZED, "DDERR_NOTPALETTIZED" },
	{ STD3D_DDERR_UNSUPPORTEDMODE, "DDERR_UNSUPPORTEDMODE" },
};

// GLOBAL: XW 0x4E0238
const char g_std3DUnknownErrorMessage[] = "Unknown Error";

// GLOBAL: XW 0x5669A0
uint16_t g_texConvBuf4444[STD3D_PALETTE_COLOR_COUNT] = { 0 };

// GLOBAL: XW 0x566BA0
unsigned int g_std3DFogTableEndBits = 0;

// GLOBAL: XW 0x566BA4
unsigned int g_std3DFogColorBlue8 = 0;

// GLOBAL: XW 0x566BA8
Std3DRenderTargetDesc g_std3DRenderTargetDesc = { 0 };

// GLOBAL: XW 0x566BF4
unsigned int g_std3DFogTableStartBits = 0;

// GLOBAL: XW 0x566BF8
IDirect3DDevice* g_d3dDevice = NULL;

// GLOBAL: XW 0x566C00
uint16_t g_texConvBuf1555[STD3D_PALETTE_COLOR_COUNT] = { 0 };

// GLOBAL: XW 0x566E00
unsigned int g_std3DFogColorRed8 = 0;

// GLOBAL: XW 0x566E04
uint8_t* g_d3dExecBufBase = NULL;

// GLOBAL: XW 0x566E08
D3DEXECUTEBUFFERDESC g_d3dExecBufDesc = { 0 };

// GLOBAL: XW 0x566E20
uint16_t g_std3DPaletteScratch16[STD3D_PALETTE_COLOR_COUNT] = { 0 };

// GLOBAL: XW 0x567020
IDirect3D* g_lpD3D = NULL;

// GLOBAL: XW 0x567024
IDirect3DViewport* g_d3dViewport = NULL;

// GLOBAL: XW 0x567028
unsigned int g_std3DFogColorGreen8 = 0;

// GLOBAL: XW 0x56702C
IUnknown* g_d3dViewportMaterial = NULL;

// GLOBAL: XW 0x567330
uint8_t* g_d3dWritePtr = NULL;

// GLOBAL: XW 0x567334
uint8_t* g_d3dInstrStart = NULL;

// GLOBAL: XW 0x5C024C
IDirectDrawSurface* g_std3DZBufferSurface = NULL;

// GLOBAL: XW 0x63BF58
int g_fmtIdxRGBA4444 = 0;

// GLOBAL: XW 0x63BF5C
unsigned int g_std3DCurDeviceIdx = 0;

// GLOBAL: XW 0x63BF60
float g_std3DColorOverlayRed = 0.0f;

// GLOBAL: XW 0x63BF64
float g_std3DColorOverlayGreen = 0.0f;

// GLOBAL: XW 0x63BF68
float g_std3DColorOverlayBlue = 0.0f;

// GLOBAL: XW 0x63BF6C
int32_t g_std3DColorOverlayEnabled = 0;

// GLOBAL: XW 0x63BF70
unsigned int g_std3DZCmpMask = 0;

// GLOBAL: XW 0x63BF80
Std3DRenderTri g_std3DViewportQuadTriangles[2];

// GLOBAL: XW 0x63BFA8
int g_fmtIdxRGB565 = 0;

// GLOBAL: XW 0x63BFC0
D3DTLVERTEX g_std3DQuadVerts[4];

// GLOBAL: XW 0x63C040
Std3DRenderTargetDesc* g_pStd3DRenderTarget = NULL;

// GLOBAL: XW 0x63C050
Std3DViewportRect g_std3DQuadRect;

// GLOBAL: XW 0x63C060
Std3DTexFmt g_std3DTextureFormats[STD3D_TEXTURE_FORMAT_CAPACITY] = { 0 };

// GLOBAL: XW 0x63C580
Std3DDevice g_std3DDevices[STD3D_DEVICE_CAPACITY] = { 0 };

// GLOBAL: XW 0x63CE40
Std3DVBuffer g_std3DZBufferVBuffer = { 0 };

// GLOBAL: XW 0x63CF84
IDirectDrawSurface* g_std3DRenderSurface = NULL;

// GLOBAL: XW 0x63CFF8
int g_fmtIdxRGBA1555 = 0;

// FUNCTION: XW 0x482A80
void std3D_DetachAndReleaseZBufferSurface(void) {
	IDirectDrawSurface* zBuffer = g_std3DZBufferSurface;
	if (zBuffer != NULL) {
		IDirectDrawSurface* backBuffer = g_flightBackBuffer;
		backBuffer->lpVtbl->DeleteAttachedSurface(backBuffer, 0, zBuffer);
		zBuffer = g_std3DZBufferSurface;
		zBuffer->lpVtbl->Release(zBuffer);
		g_std3DZBufferSurface = NULL;
	}
}

// FUNCTION: XW 0x4B1AF0
void std3D_CopyPaletteToScratch16(const uint16_t* palette, int colorCount) {
	memcpy(g_std3DPaletteScratch16, palette, colorCount * sizeof(g_std3DPaletteScratch16[0]));
}

// FUNCTION: XW 0x4B1B20
void std3D_ConvertTexTo1555(const uint16_t* sourcePixels, int pixelCount) {
	Std3DTexFmt* sourceFormat;
	Std3DTexFmt* targetFormat;
	int index;
	if (g_pFmtRGBA1555 == g_pFmtRGB565) {
		memcpy(g_texConvBuf1555, sourcePixels, pixelCount * sizeof(*sourcePixels));
		return;
	}
	sourceFormat = g_pFmtRGB565;
	targetFormat = g_pFmtRGBA1555;
	for (index = 0; index < pixelCount; ++index) {
		uint8_t channel;
		uint16_t converted;
		channel = sourcePixels[index] >> (uint8_t)sourceFormat->colorInfo.redPosShift;
		channel <<= (uint8_t)sourceFormat->colorInfo.redPosShiftRight;
		channel >>= (uint8_t)targetFormat->colorInfo.redPosShiftRight;
		converted = channel << (uint8_t)targetFormat->colorInfo.redPosShift;
		g_texConvBuf1555[index] = converted;
		channel = sourcePixels[index] >> (uint8_t)sourceFormat->colorInfo.greenPosShift;
		channel <<= (uint8_t)sourceFormat->colorInfo.greenPosShiftRight;
		channel >>= (uint8_t)targetFormat->colorInfo.greenPosShiftRight;
		converted |= channel << (uint8_t)targetFormat->colorInfo.greenPosShift;
		g_texConvBuf1555[index] = converted;
		channel = sourcePixels[index] >> (uint8_t)sourceFormat->colorInfo.bluePosShift;
		channel <<= (uint8_t)sourceFormat->colorInfo.bluePosShiftRight;
		channel >>= (uint8_t)targetFormat->colorInfo.bluePosShiftRight;
		converted |= channel << (uint8_t)targetFormat->colorInfo.bluePosShift;
		g_texConvBuf1555[index] = converted;
		if (index != 0) {
			channel = UINT8_MAX;
			channel >>= (uint8_t)targetFormat->colorInfo.alphaPosShiftRight;
			converted |= channel << (uint8_t)targetFormat->colorInfo.alphaPosShift;
			g_texConvBuf1555[index] = converted;
		}
	}
}

// FUNCTION: XW 0x4B1C30
int std3D_Startup(void) {
	HRESULT status;
	g_std3DDirectDraw = Renderer_GetDirectDraw();
	if (g_std3DDirectDraw == NULL) {
		DebugPrintf("DDraw device not created yet!\n", 0, 0, 0, 0);
		return 0;
	}
	g_std3DCapFlags = STD3D_RS_TEXTURE_PERSPECTIVE | STD3D_RS_DITHER | STD3D_RS_SUBPIXEL |
					  STD3D_RS_SUBPIXEL_X | STD3D_RS_MAG_LINEAR | STD3D_RS_MIN_LINEAR | STD3D_RS_Z_TEST |
					  STD3D_RS_Z_WRITE;
	g_std3DZBufferEnabled = 1;
	DebugPrintf("Creating D3D interface object.\n", 0, 0, 0, 0);
	{
		IDirectDraw* directDraw = g_std3DDirectDraw;
		status = directDraw->lpVtbl->QueryInterface(directDraw, &IID_IDirect3D, (void**)&g_lpD3D);
	}
	if (status != STD3D_D3D_OK) {
		DebugPrintf("Error %s creating Direct3D interface object.\n",
					std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT), 0, 0,
					0);
		return 0;
	}
	DebugPrintf("Enumerating D3D devices.\n", 0, 0, 0, 0);
	g_std3DNumDevices = 0;
	{
		IDirect3D* direct3D = g_lpD3D;
		status = direct3D->lpVtbl->EnumDevices(direct3D, std3D_EnumDevicesCallback, NULL);
	}
	if (status != STD3D_D3D_OK) {
		DebugPrintf("Error %s when enumerating D3D devices.\n",
					std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT), 0, 0,
					0);
		return 0;
	}
	if (g_std3DNumDevices == 0)
		return 0;
	DebugPrintf("%d D3D devices found.\n", g_std3DNumDevices,
				std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT), 0, 0);
	g_std3DStartupDone = 1;
	DebugPrintf("Startup Succeeded.\n", 0, 0, 0, 0);
	return 1;
}

// FUNCTION: XW 0x4B1D70
void std3D_InitRenderTargetDesc(unsigned int width, unsigned int height, int pitch) {
	memset(&g_std3DRenderTargetDesc, 0, sizeof(g_std3DRenderTargetDesc));
	g_pStd3DRenderTarget = &g_std3DRenderTargetDesc;
	g_pStd3DRenderTarget->width = width;
	g_pStd3DRenderTarget->height = height;
	g_pStd3DRenderTarget->pitch = pitch;
	g_pStd3DRenderTarget->widthPixels = pitch / (int)sizeof(uint16_t);
	g_pStd3DRenderTarget->sizeBytes = g_pStd3DRenderTarget->pitch * g_pStd3DRenderTarget->height;
	g_pStd3DRenderTarget->colorInfo.colorMode = STDCOLOR_RGB;
	g_pStd3DRenderTarget->colorInfo.bpp = CHAR_BIT * sizeof(uint16_t);
	g_pStd3DRenderTarget->colorInfo.redBPP = STDCOLOR_RGB565_RED_BITS;
	g_pStd3DRenderTarget->colorInfo.greenBPP = STDCOLOR_RGB565_GREEN_BITS;
	g_pStd3DRenderTarget->colorInfo.blueBPP = STDCOLOR_RGB565_BLUE_BITS;
	g_pStd3DRenderTarget->colorInfo.redPosShift = STDCOLOR_RGB565_GREEN_BITS + STDCOLOR_RGB565_BLUE_BITS;
	g_pStd3DRenderTarget->colorInfo.greenPosShift = STDCOLOR_RGB565_BLUE_BITS;
	g_pStd3DRenderTarget->colorInfo.bluePosShift = 0;
	g_pStd3DRenderTarget->colorInfo.redPosShiftRight = CHAR_BIT - STDCOLOR_RGB565_RED_BITS;
	g_pStd3DRenderTarget->colorInfo.greenPosShiftRight = CHAR_BIT - STDCOLOR_RGB565_GREEN_BITS;
	g_pStd3DRenderTarget->colorInfo.bluePosShiftRight = CHAR_BIT - STDCOLOR_RGB565_BLUE_BITS;
	g_pStd3DRenderTarget->colorInfo.alphaBPP = 0;
	g_pStd3DRenderTarget->colorInfo.alphaPosShift = 0;
	g_pStd3DRenderTarget->colorInfo.alphaPosShiftRight = 0;
}

// FUNCTION: XW 0x4B1E70
void std3D_Shutdown(void) {
	if (g_lpD3D != NULL) {
		(void)g_lpD3D->lpVtbl->Release(g_lpD3D);
	}
	nullsub_SharedNoOp();
	g_std3DStartupDone = 0;
}

// FUNCTION: XW 0x4B1EB0
int std3D_CreateDevice(unsigned int deviceIdx, int bUseZBuffer) {
	HRESULT status;
	ColorInfo requestedColor;
	Std3DRasterInfo raster;
	unsigned int maxBufferSize;
	unsigned int maxVertexCount;
	if (g_std3DDeviceOpen != 0) {
		nullsub_SharedNoOp();
		return 0;
	}
	if (g_std3DNumDevices <= deviceIdx)
		return 0;
	g_std3DCurDeviceIdx = deviceIdx;
	g_pStd3DCurDevice = &g_std3DDevices[deviceIdx];
	g_std3DZBufferEnabled = bUseZBuffer != 0 && g_pStd3DCurDevice->caps.bHasZBuffer != 0 &&
							(g_std3DCapFlags & (STD3D_RS_Z_TEST | STD3D_RS_Z_WRITE)) != 0;
	if (g_std3DZBufferEnabled != 0) {
		if (std3D_CreateZBuffer(g_pStd3DRenderTarget->width, g_pStd3DRenderTarget->height) == 0) {
			nullsub_SharedNoOp();
			return 0;
		}
		if ((g_pStd3DCurDevice->caps.zCmpCapsMask & STD3D_ZCMP_GREATER) != 0)
			g_std3DZCmpMask = STD3D_ZCMP_GREATER;
		else
			g_std3DZCmpMask = STD3D_ZCMP_LESS;
		nullsub_SharedNoOp();
	}
	nullsub_SharedNoOp();
	{
		IDirectDrawSurface* surface = g_std3DRenderSurface;
		status = surface->lpVtbl->QueryInterface(surface, &g_pStd3DCurDevice->guid, (void**)&g_d3dDevice);
	}
	if (status != STD3D_D3D_OK) {
		std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
		nullsub_SharedNoOp();
		return 0;
	}
	g_std3DNumTextureFormats = 0;
	status = g_d3dDevice->lpVtbl->EnumTextureFormats(g_d3dDevice, (void*)std3D_EnumTextureFormats, NULL);
	if (status != STD3D_D3D_OK) {
		std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
		nullsub_SharedNoOp();
		return 0;
	}
	if (g_std3DNumTextureFormats == 0) {
		nullsub_SharedNoOp();
		return 0;
	}
	std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
	nullsub_SharedNoOp();
	if (std3D_CreateViewport(g_pStd3DRenderTarget->width, g_pStd3DRenderTarget->height) == 0) {
		nullsub_SharedNoOp();
		return 0;
	}
	if (std3D_SetInitialRenderState() == 0) {
		nullsub_SharedNoOp();
		return 0;
	}
	nullsub_SharedNoOp();
	maxBufferSize = g_pStd3DCurDevice->caps.maxBufferSize;
	g_std3DExecBufSize = STD3D_DEFAULT_EXEC_BUFFER_SIZE;
	if (maxBufferSize != 0)
		g_std3DExecBufSize = maxBufferSize;
	memset(&g_d3dExecBufDesc, 0, sizeof(g_d3dExecBufDesc));
	g_d3dExecBufDesc.dwSize = sizeof(g_d3dExecBufDesc);
	g_d3dExecBufDesc.dwFlags = STD3D_EXEC_BUFFER_SIZE_PRESENT;
	g_d3dExecBufDesc.dwBufferSize = g_std3DExecBufSize;
	maxVertexCount = g_pStd3DCurDevice->caps.maxVertexCount;
	if (maxVertexCount == 0)
		g_std3DExecBufMaxVerts = STD3D_MAX_EXEC_VERTICES;
	else {
		if (maxVertexCount >= STD3D_MAX_EXEC_VERTICES)
			maxVertexCount = STD3D_MAX_EXEC_VERTICES;
		g_std3DExecBufMaxVerts = maxVertexCount;
	}
	nullsub_SharedNoOp();
	nullsub_SharedNoOp();
	{
		IDirect3DDevice* device = g_d3dDevice;
		status = device->lpVtbl->CreateExecuteBuffer(device, &g_d3dExecBufDesc, &g_d3dExecuteBuffer, NULL);
	}
	if (status != STD3D_D3D_OK) {
		std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
		nullsub_SharedNoOp();
	}
	g_texCacheCount = 0;
	g_pTexCacheHead = NULL;
	g_pTexCacheTail = NULL;
	g_std3DTextureFrameTag = 1;
	g_fmtIdxRGB565 = std3D_FindClosestFormat(&g_pStd3DRenderTarget->colorInfo, g_std3DTextureFormats,
											 g_std3DNumTextureFormats);
	g_pFmtRGB565 = &g_std3DTextureFormats[g_fmtIdxRGB565];
	if (g_pStd3DCurDevice->caps.bAlphaTexture != 0) {
		requestedColor.colorMode = STDCOLOR_RGBA;
		requestedColor.redBPP = STDCOLOR_RGB555_CHANNEL_BITS;
		requestedColor.greenBPP = STDCOLOR_RGB555_CHANNEL_BITS;
		requestedColor.blueBPP = STDCOLOR_RGB555_CHANNEL_BITS;
		requestedColor.bpp = CHAR_BIT * sizeof(uint16_t);
		requestedColor.alphaBPP = STDCOLOR_RGBA1555_ALPHA_BITS;
		g_fmtIdxRGBA1555 =
			std3D_FindClosestFormat(&requestedColor, g_std3DTextureFormats, g_std3DNumTextureFormats);
		g_pFmtRGBA1555 = &g_std3DTextureFormats[g_fmtIdxRGBA1555];
		if (g_pStd3DCurDevice->caps.bAlphaBlend == 0) {
			requestedColor.colorMode = STDCOLOR_RGBA;
			requestedColor.redBPP = STDCOLOR_RGBA4444_CHANNEL_BITS;
			requestedColor.greenBPP = STDCOLOR_RGBA4444_CHANNEL_BITS;
			requestedColor.blueBPP = STDCOLOR_RGBA4444_CHANNEL_BITS;
			requestedColor.alphaBPP = STDCOLOR_RGBA4444_CHANNEL_BITS;
			requestedColor.bpp = CHAR_BIT * sizeof(uint16_t);
			g_fmtIdxRGBA4444 =
				std3D_FindClosestFormat(&requestedColor, g_std3DTextureFormats, g_std3DNumTextureFormats);
			g_pFmtRGBA4444 = &g_std3DTextureFormats[g_fmtIdxRGBA4444];
#ifdef XW_MODERN
			raster.sizeBytes = 0;
			raster.pitch = 0;
			raster.widthPixels = 0;
#endif
			raster.width = STD3D_FALLBACK_TEXTURE_SIZE;
			raster.height = STD3D_FALLBACK_TEXTURE_SIZE;
			raster.colorInfo = g_pFmtRGBA4444->colorInfo;
			g_pStd3DVBuffer = std3D_AllocVBuffer(&raster, 1, 0, 0);
		}
	}
	std3D_QueryTextureVidMem(&g_pStd3DCurDevice->totalMemory, &g_pStd3DCurDevice->availableMemory);
	nullsub_SharedNoOp();
	nullsub_SharedNoOp();
	nullsub_SharedNoOp();
	g_std3DDeviceOpen = 1;
	return 1;
}

// FUNCTION: XW 0x4B2420
void std3D_SetRenderSurface(IDirectDrawSurface* surface) { g_std3DRenderSurface = surface; }

// FUNCTION: XW 0x4B2430
void std3D_SetColorOverlayParams(float red, float green, float blue, int32_t enabled) {
	g_std3DColorOverlayRed = red;
	g_std3DColorOverlayGreen = green;
	g_std3DColorOverlayBlue = blue;
	g_std3DColorOverlayEnabled = enabled;
}

// FUNCTION: XW 0x4B2460
int std3D_Log2Floor(int value) {
	int result = 0;
	while (value > 1) {
		value >>= 1;
		++result;
	}
	return result;
}

// FUNCTION: XW 0x4B2480
const char* std3D_LookupErrorString(HRESULT errorCode, const struct Std3DErrorStringEntry* entries,
									int entryCount) {
	const char* message = g_std3DUnknownErrorMessage;
	int entryIndex;
	for (entryIndex = 0; entryIndex < entryCount; ++entryIndex) {
		if (entries[entryIndex].code == errorCode) {
			message = entries[entryIndex].message;
			break;
		}
	}
	return message;
}

// FUNCTION: XW 0x4B24C0
struct Std3DVBuffer* std3D_AllocVBuffer(const struct Std3DRasterInfo* raster, int unused1, int unused2,
										int unused3) {
	Std3DVBuffer* vbuffer = (Std3DVBuffer*)malloc(sizeof(*vbuffer));
	(void)unused1;
	(void)unused2;
	(void)unused3;
	memset(vbuffer, 0, sizeof(*vbuffer));
	vbuffer->storageType = STD3D_STORAGE_SOFTWARE;
	memcpy(&vbuffer->raster, raster, sizeof(vbuffer->raster));
	vbuffer->pixels =
		(uint8_t*)malloc(((unsigned int)raster->colorInfo.bpp / CHAR_BIT) * raster->height * raster->width);
	vbuffer->raster.pitch = raster->width * ((unsigned int)raster->colorInfo.bpp / CHAR_BIT);
	return vbuffer;
}

// FUNCTION: XW 0x4B2520
void std3D_FreeVBuffer(struct Std3DVBuffer* vbuffer) {
	if (vbuffer->storageType == STD3D_STORAGE_SURFACE) {
		IDirectDrawSurface* surface = vbuffer->surfaceBlock.surface;
		if (surface != NULL)
			surface->lpVtbl->Release(surface);
	} else {
		free(vbuffer->pixels);
	}
	memset(vbuffer, 0, sizeof(*vbuffer));
	free(vbuffer);
}

// FUNCTION: XW 0x4B2560
void std3D_LockVBuffer(struct Std3DVBuffer* vbuffer) {
	if (vbuffer->storageType == STD3D_STORAGE_SURFACE && vbuffer->lockCount == 0) {
		DDSURFACEDESC lockedDesc;
		IDirectDrawSurface* surface;
		memset(&lockedDesc, 0, sizeof(lockedDesc));
		lockedDesc.dwSize = sizeof(lockedDesc);
		surface = vbuffer->surfaceBlock.surface;
		if (surface->lpVtbl->Lock(surface, NULL, &lockedDesc, DDLOCK_WAIT, NULL) != STD3D_D3D_OK) {
			nullsub_SharedNoOp();
			return;
		}
		vbuffer->pixels = lockedDesc.lpSurface;
		vbuffer->raster.pitch = lockedDesc.lPitch;
	}
	vbuffer->lockCount = (int)((unsigned int)vbuffer->lockCount + 1u);
}

// FUNCTION: XW 0x4B25E0
void std3D_UnlockVBuffer(struct Std3DVBuffer* vbuffer) {
	unsigned int lockCount = (unsigned int)vbuffer->lockCount;
	if (lockCount < 1u) {
		nullsub_SharedNoOp();
		return;
	}
	if (lockCount == 1u && vbuffer->storageType == STD3D_STORAGE_SURFACE) {
		IDirectDrawSurface* surface = vbuffer->surfaceBlock.surface;
		if (surface->lpVtbl->Unlock(surface, vbuffer->pixels) != STD3D_D3D_OK) {
			nullsub_SharedNoOp();
			return;
		}
	}
	vbuffer->lockCount = (int)((unsigned int)vbuffer->lockCount - 1u);
}

// FUNCTION: XW 0x4B2640
void std3D_Close(void) {
	if (g_std3DDeviceOpen == 0) {
		nullsub_SharedNoOp();
		return;
	}
	if (g_pStd3DVBuffer != NULL) {
		std3D_FreeVBuffer(g_pStd3DVBuffer);
		g_pStd3DVBuffer = NULL;
	}
	g_d3dExecuteBuffer->lpVtbl->Release(g_d3dExecuteBuffer);
	std3D_FlushTextureCache();
	if (g_d3dViewport != NULL) {
		g_d3dViewport->lpVtbl->Release(g_d3dViewport);
		g_d3dViewport = NULL;
	}
	if (g_d3dViewportMaterial != NULL) {
		g_d3dViewportMaterial->lpVtbl->Release(g_d3dViewportMaterial);
		g_d3dViewportMaterial = NULL;
	}
	if (g_std3DZBufferVBuffer.surfaceBlock.surface != NULL) {
		g_std3DZBufferVBuffer.surfaceBlock.surface->lpVtbl->Release(
			g_std3DZBufferVBuffer.surfaceBlock.surface);
		g_std3DZBufferVBuffer.surfaceBlock.surface = NULL;
	}
	if (g_d3dDevice != NULL) {
		g_d3dDevice->lpVtbl->Release(g_d3dDevice);
		g_d3dDevice = NULL;
	}
	nullsub_SharedNoOp();
	g_std3DDeviceOpen = 0;
}

// FUNCTION: XW 0x4B2730
void std3D_BlitVBuffer(struct Std3DVBuffer* destination, struct Std3DVBuffer* source, int destinationX,
					   int destinationY) {
	uint8_t* destinationPixels;
	const uint8_t* sourcePixels;
	ptrdiff_t destinationOffset;
	ptrdiff_t sourceOffset;
	unsigned int rowBytes;
	unsigned int row;
	std3D_LockVBuffer(destination);
	std3D_LockVBuffer(source);
	sourcePixels = source->pixels;
	destinationOffset =
		(ptrdiff_t)destinationY * destination->raster.pitch +
		(ptrdiff_t)destinationX * ((unsigned int)destination->raster.colorInfo.bpp / CHAR_BIT);
	destinationPixels = destination->pixels + destinationOffset;
	destinationOffset = 0;
	sourceOffset = 0;
	rowBytes = source->raster.width * ((unsigned int)source->raster.colorInfo.bpp / CHAR_BIT);
	for (row = 0; row < source->raster.height; ++row) {
		memcpy(&destinationPixels[destinationOffset], &sourcePixels[sourceOffset], rowBytes);
		destinationOffset += destination->raster.pitch;
		sourceOffset += source->raster.pitch;
	}
	std3D_UnlockVBuffer(destination);
	std3D_UnlockVBuffer(source);
}

// FUNCTION: XW 0x4B27D0
void std3D_StartScene(void) {
	HRESULT result = g_d3dDevice->lpVtbl->BeginScene(g_d3dDevice);
	if (result != STD3D_D3D_OK) {
		(void)std3D_LookupErrorString(result, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
		nullsub_SharedNoOp();
	}
}

// FUNCTION: XW 0x4B2810
void std3D_EndScene(void) {
	HRESULT result = g_d3dDevice->lpVtbl->EndScene(g_d3dDevice);
	if (result != STD3D_D3D_OK) {
		std3D_LookupErrorString(result, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
		nullsub_SharedNoOp();
	}
}

// FUNCTION: XW 0x4B2850
int std3D_LockExecuteBuffer(void) {
	HRESULT result;
	++g_std3DTextureFrameTag;
	g_d3dBufVertCount = 0;
	g_std3DExecBufTriCount = 0;
	g_d3dCurTexture = (Std3DTexCacheNode*)STD3D_TEXTURE_BINDING_UNSET;
	result = g_d3dExecuteBuffer->lpVtbl->Lock(g_d3dExecuteBuffer, &g_d3dExecBufDesc);
	if (result != STD3D_D3D_OK) {
		std3D_LookupErrorString(result, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
		nullsub_SharedNoOp();
		return 0;
	}
	g_d3dExecBufBase = g_d3dExecBufDesc.lpData;
	g_d3dWritePtr = g_d3dExecBufDesc.lpData;
	return 1;
}

// FUNCTION: XW 0x4B28D0
int std3D_AddVertices(const D3DTLVERTEX* vertices, unsigned int count) {
	unsigned int previousVertexCount = g_d3dBufVertCount;
	uint8_t* writeStart;

	if (previousVertexCount + count > g_std3DExecBufMaxVerts) {
		return 0;
	}
	writeStart = g_d3dWritePtr;
	if (writeStart != (const uint8_t*)vertices) {
		memcpy(writeStart, vertices, count * sizeof(*vertices));
	}
	g_d3dBufVertCount = previousVertexCount + count;
	g_d3dWritePtr = writeStart + count * sizeof(*vertices);
	return 1;
}

// FUNCTION: XW 0x4B2930
int std3D_BeginInstructions(void) {
	g_d3dInstrStart = g_d3dWritePtr;
	((D3DINSTRUCTION*)g_d3dWritePtr)->bOpcode = D3DOP_PROCESSVERTICES;
	((D3DINSTRUCTION*)g_d3dWritePtr)->bSize = sizeof(D3DPROCESSVERTICES);
	((D3DINSTRUCTION*)g_d3dWritePtr)->wCount = 1;
	g_d3dWritePtr += sizeof(D3DINSTRUCTION);
	((D3DPROCESSVERTICES*)g_d3dWritePtr)->dwFlags = D3DPROCESSVERTICES_COPY;
	((D3DPROCESSVERTICES*)g_d3dWritePtr)->wStart = 0;
	((D3DPROCESSVERTICES*)g_d3dWritePtr)->wDest = 0;
	((D3DPROCESSVERTICES*)g_d3dWritePtr)->dwCount = g_d3dBufVertCount;
	((D3DPROCESSVERTICES*)g_d3dWritePtr)->dwReserved = 0;
	g_d3dWritePtr += sizeof(D3DPROCESSVERTICES);
	return 1;
}

// FUNCTION: XW 0x4B29A0
int std3D_AddTriangles(const struct Std3DRenderTri* triangles, unsigned int count) {
	unsigned int batchStart;
	unsigned int batchCount;
	for (batchStart = 0; batchStart < count; batchStart += batchCount) {
		Std3DTexCacheNode* batchTexture = triangles[batchStart].texture;
		unsigned int scanIndex;
		unsigned int triangleIndex;
		batchCount = 0;
		if (batchTexture == NULL) {
			unsigned int batchFlags = triangles[batchStart].flags;
			for (scanIndex = batchStart; scanIndex < count; ++scanIndex) {
				if (triangles[scanIndex].texture != NULL || triangles[scanIndex].flags != batchFlags)
					break;
				++batchCount;
			}
			std3D_SetRenderState(batchFlags);
			if (g_d3dCurTexture != NULL) {
				((D3DINSTRUCTION*)g_d3dWritePtr)->bOpcode = D3DOP_STATERENDER;
				((D3DINSTRUCTION*)g_d3dWritePtr)->bSize = sizeof(Std3DRenderState);
				((D3DINSTRUCTION*)g_d3dWritePtr)->wCount = 1;
				g_d3dWritePtr += sizeof(D3DINSTRUCTION);
				((Std3DRenderState*)g_d3dWritePtr)->state = STD3D_STATE_TEXTUREHANDLE;
				((Std3DRenderState*)g_d3dWritePtr)->value = 0;
				g_d3dCurTexture = NULL;
				g_d3dWritePtr += sizeof(Std3DRenderState);
			}
			((D3DINSTRUCTION*)g_d3dWritePtr)->bOpcode = D3DOP_TRIANGLE;
			((D3DINSTRUCTION*)g_d3dWritePtr)->bSize = sizeof(D3DTRIANGLE);
			((D3DINSTRUCTION*)g_d3dWritePtr)->wCount = batchCount;
			g_d3dWritePtr += sizeof(D3DINSTRUCTION);
			for (triangleIndex = 0; triangleIndex < batchCount; ++triangleIndex) {
				((D3DTRIANGLE*)g_d3dWritePtr)->v1 = triangles[batchStart + triangleIndex].v0;
				((D3DTRIANGLE*)g_d3dWritePtr)->v2 = triangles[batchStart + triangleIndex].v1;
				((D3DTRIANGLE*)g_d3dWritePtr)->v3 = triangles[batchStart + triangleIndex].v2;
				((D3DTRIANGLE*)g_d3dWritePtr)->wFlags =
					D3DTRIFLAG_EDGEENABLE1 | D3DTRIFLAG_EDGEENABLE2 | D3DTRIFLAG_EDGEENABLE3;
				g_d3dWritePtr += sizeof(D3DTRIANGLE);
			}
		} else {
			unsigned int batchFlags = triangles[batchStart].flags;
			for (scanIndex = batchStart; scanIndex < count; ++scanIndex) {
				if (triangles[scanIndex].texture != batchTexture || triangles[scanIndex].flags != batchFlags)
					break;
				++batchCount;
			}
			std3D_SetRenderState(batchFlags);
			if (triangles[batchStart].texture != g_d3dCurTexture) {
				((D3DINSTRUCTION*)g_d3dWritePtr)->bOpcode = D3DOP_STATERENDER;
				((D3DINSTRUCTION*)g_d3dWritePtr)->bSize = sizeof(Std3DRenderState);
				((D3DINSTRUCTION*)g_d3dWritePtr)->wCount = 1;
				g_d3dWritePtr += sizeof(D3DINSTRUCTION);
				((Std3DRenderState*)g_d3dWritePtr)->state = STD3D_STATE_TEXTUREHANDLE;
				((Std3DRenderState*)g_d3dWritePtr)->value = batchTexture->texHandle;
				g_d3dWritePtr += sizeof(Std3DRenderState);
				g_d3dCurTexture = batchTexture;
			}
			((D3DINSTRUCTION*)g_d3dWritePtr)->bOpcode = D3DOP_TRIANGLE;
			((D3DINSTRUCTION*)g_d3dWritePtr)->bSize = sizeof(D3DTRIANGLE);
			((D3DINSTRUCTION*)g_d3dWritePtr)->wCount = batchCount;
			g_d3dWritePtr += sizeof(D3DINSTRUCTION);
			for (triangleIndex = 0; triangleIndex < batchCount; ++triangleIndex) {
				((D3DTRIANGLE*)g_d3dWritePtr)->v1 = triangles[batchStart + triangleIndex].v0;
				((D3DTRIANGLE*)g_d3dWritePtr)->v2 = triangles[batchStart + triangleIndex].v1;
				((D3DTRIANGLE*)g_d3dWritePtr)->v3 = triangles[batchStart + triangleIndex].v2;
				((D3DTRIANGLE*)g_d3dWritePtr)->wFlags =
					D3DTRIFLAG_EDGEENABLE1 | D3DTRIFLAG_EDGEENABLE2 | D3DTRIFLAG_EDGEENABLE3;
				g_d3dWritePtr += sizeof(D3DTRIANGLE);
			}
		}
	}
	g_std3DExecBufTriCount += count;
	return 1;
}

// FUNCTION: XW 0x4B2BE0
int std3D_ExecuteBuffer(void) {
	D3DEXECUTEDATA executeData;
	HRESULT status;
	((D3DINSTRUCTION*)g_d3dWritePtr)->bOpcode = D3DOP_EXIT;
	((D3DINSTRUCTION*)g_d3dWritePtr)->bSize = 0;
	((D3DINSTRUCTION*)g_d3dWritePtr)->wCount = 0;
	g_d3dWritePtr += sizeof(D3DINSTRUCTION);
	status = g_d3dExecuteBuffer->lpVtbl->Unlock(g_d3dExecuteBuffer);
	if (status != 0) {
		std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
		nullsub_SharedNoOp();
		return 0;
	}
	memset(&executeData, 0, sizeof(executeData));
	executeData.dwSize = sizeof(executeData);
	executeData.dwVertexCount = g_d3dBufVertCount;
	executeData.dwInstructionOffset = g_d3dInstrStart - g_d3dExecBufBase;
	executeData.dwInstructionLength = g_d3dWritePtr - g_d3dInstrStart;
	g_d3dExecuteBuffer->lpVtbl->SetExecuteData(g_d3dExecuteBuffer, &executeData);
	status =
		g_d3dDevice->lpVtbl->Execute(g_d3dDevice, g_d3dExecuteBuffer, g_d3dViewport, D3DEXECUTE_UNCLIPPED);
	if (status != 0) {
		std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
		nullsub_SharedNoOp();
		return 0;
	}
	return 1;
}

// FUNCTION: XW 0x4B2CF0
void std3D_SetRenderState(Std3DRenderStateFlags flags) {
	if (g_d3dStateFlags != flags) {
		if (((flags ^ g_d3dStateFlags) & STD3D_RS_DISABLE_MONO) != 0) {
			((D3DINSTRUCTION*)g_d3dWritePtr)->bOpcode = D3DOP_STATERENDER;
			((D3DINSTRUCTION*)g_d3dWritePtr)->bSize = sizeof(Std3DRenderState);
			((D3DINSTRUCTION*)g_d3dWritePtr)->wCount = 1;
			g_d3dWritePtr += sizeof(D3DINSTRUCTION);
			((Std3DRenderState*)g_d3dWritePtr)->state = STD3D_STATE_MONOENABLE;
			if ((flags & STD3D_RS_DISABLE_MONO) != 0)
				((Std3DRenderState*)g_d3dWritePtr)->value = 0;
			else
				((Std3DRenderState*)g_d3dWritePtr)->value = 1;
			g_d3dWritePtr += sizeof(Std3DRenderState);
		}
		if (((flags ^ g_d3dStateFlags) & (STD3D_RS_ALPHA_BLEND | STD3D_RS_MODULATE_ALPHA)) != 0) {
			((D3DINSTRUCTION*)g_d3dWritePtr)->bOpcode = D3DOP_STATERENDER;
			((D3DINSTRUCTION*)g_d3dWritePtr)->bSize = sizeof(Std3DRenderState);
			((D3DINSTRUCTION*)g_d3dWritePtr)->wCount = STD3D_BLEND_STATE_COUNT;
			if ((flags & (STD3D_RS_ALPHA_BLEND | STD3D_RS_MODULATE_ALPHA)) != 0) {
				g_d3dWritePtr += sizeof(D3DINSTRUCTION);
				((Std3DRenderState*)g_d3dWritePtr)->state = STD3D_STATE_SRCBLEND;
				((Std3DRenderState*)g_d3dWritePtr)->value = STD3D_BLEND_SRC_ALPHA;
				g_d3dWritePtr += sizeof(Std3DRenderState);
				((Std3DRenderState*)g_d3dWritePtr)->state = STD3D_STATE_DESTBLEND;
				((Std3DRenderState*)g_d3dWritePtr)->value = STD3D_BLEND_INV_SRC_ALPHA;
				g_d3dWritePtr += sizeof(Std3DRenderState);
				((Std3DRenderState*)g_d3dWritePtr)->state = STD3D_STATE_TEXTUREMAPBLEND;
				if ((flags & STD3D_RS_MODULATE_ALPHA) != 0)
					((Std3DRenderState*)g_d3dWritePtr)->value = STD3D_MAP_BLEND_MODULATE_ALPHA;
				else
					((Std3DRenderState*)g_d3dWritePtr)->value = STD3D_MAP_BLEND_MODULATE;
				g_d3dWritePtr += sizeof(Std3DRenderState);
				((Std3DRenderState*)g_d3dWritePtr)->state = STD3D_STATE_BLENDENABLE;
				((Std3DRenderState*)g_d3dWritePtr)->value = 1;
				g_d3dWritePtr += sizeof(Std3DRenderState);
			} else {
				g_d3dWritePtr += sizeof(D3DINSTRUCTION);
				((Std3DRenderState*)g_d3dWritePtr)->state = STD3D_STATE_SRCBLEND;
				((Std3DRenderState*)g_d3dWritePtr)->value = STD3D_BLEND_ONE;
				g_d3dWritePtr += sizeof(Std3DRenderState);
				((Std3DRenderState*)g_d3dWritePtr)->state = STD3D_STATE_DESTBLEND;
				((Std3DRenderState*)g_d3dWritePtr)->value = STD3D_BLEND_ZERO;
				g_d3dWritePtr += sizeof(Std3DRenderState);
				((Std3DRenderState*)g_d3dWritePtr)->state = STD3D_STATE_TEXTUREMAPBLEND;
				if ((flags & STD3D_RS_MODULATE_ALPHA) != 0)
					((Std3DRenderState*)g_d3dWritePtr)->value = STD3D_MAP_BLEND_MODULATE_ALPHA;
				else
					((Std3DRenderState*)g_d3dWritePtr)->value = STD3D_MAP_BLEND_MODULATE;
				g_d3dWritePtr += sizeof(Std3DRenderState);
				((Std3DRenderState*)g_d3dWritePtr)->state = STD3D_STATE_BLENDENABLE;
				((Std3DRenderState*)g_d3dWritePtr)->value = 0;
				g_d3dWritePtr += sizeof(Std3DRenderState);
			}
		}
		if (((flags ^ g_d3dStateFlags) & (STD3D_RS_Z_TEST | STD3D_RS_Z_WRITE)) != 0) {
			((D3DINSTRUCTION*)g_d3dWritePtr)->bOpcode = D3DOP_STATERENDER;
			((D3DINSTRUCTION*)g_d3dWritePtr)->bSize = sizeof(Std3DRenderState);
			((D3DINSTRUCTION*)g_d3dWritePtr)->wCount = STD3D_DEPTH_STATE_COUNT;
			g_d3dWritePtr += sizeof(D3DINSTRUCTION);
			((Std3DRenderState*)g_d3dWritePtr)->state = STD3D_STATE_ZFUNC;
			((Std3DRenderState*)g_d3dWritePtr)->value =
				(flags & STD3D_RS_Z_TEST) != 0 ? std3D_MapZCmpFunc(g_std3DZCmpMask) : STD3D_COMPARE_ALWAYS;
			g_d3dWritePtr += sizeof(Std3DRenderState);
			((Std3DRenderState*)g_d3dWritePtr)->state = STD3D_STATE_ZWRITEENABLE;
			if ((flags & STD3D_RS_Z_WRITE) != 0)
				((Std3DRenderState*)g_d3dWritePtr)->value = 1;
			else
				((Std3DRenderState*)g_d3dWritePtr)->value = 0;
			g_d3dWritePtr += sizeof(Std3DRenderState);
		}
		if (((flags ^ g_d3dStateFlags) & (STD3D_RS_MAG_LINEAR | STD3D_RS_MIN_LINEAR)) != 0) {
			((D3DINSTRUCTION*)g_d3dWritePtr)->bOpcode = D3DOP_STATERENDER;
			((D3DINSTRUCTION*)g_d3dWritePtr)->bSize = sizeof(Std3DRenderState);
			((D3DINSTRUCTION*)g_d3dWritePtr)->wCount = STD3D_FILTER_STATE_COUNT;
			g_d3dWritePtr += sizeof(D3DINSTRUCTION);
			((Std3DRenderState*)g_d3dWritePtr)->state = STD3D_STATE_TEXTUREMAG;
			((Std3DRenderState*)g_d3dWritePtr)->value =
				(flags & STD3D_RS_MAG_LINEAR) != 0 ? STD3D_FILTER_LINEAR : STD3D_FILTER_NEAREST;
			g_d3dWritePtr += sizeof(Std3DRenderState);
			((Std3DRenderState*)g_d3dWritePtr)->state = STD3D_STATE_TEXTUREMIN;
			((Std3DRenderState*)g_d3dWritePtr)->value =
				(flags & STD3D_RS_MIN_LINEAR) != 0 ? STD3D_FILTER_LINEAR : STD3D_FILTER_NEAREST;
			g_d3dWritePtr += sizeof(Std3DRenderState);
		}
		if (((flags ^ g_d3dStateFlags) & STD3D_RS_FOG) != 0) {
			((D3DINSTRUCTION*)g_d3dWritePtr)->bOpcode = D3DOP_STATERENDER;
			((D3DINSTRUCTION*)g_d3dWritePtr)->bSize = sizeof(Std3DRenderState);
			if ((flags & STD3D_RS_FOG) != 0) {
				((D3DINSTRUCTION*)g_d3dWritePtr)->wCount = STD3D_FOG_STATE_COUNT;
				g_d3dWritePtr += sizeof(D3DINSTRUCTION);
				((Std3DRenderState*)g_d3dWritePtr)->state = STD3D_STATE_FOGENABLE;
				((Std3DRenderState*)g_d3dWritePtr)->value = 1;
				g_d3dWritePtr += sizeof(Std3DRenderState);
				((Std3DRenderState*)g_d3dWritePtr)->state = STD3D_STATE_FOGCOLOR;
				((Std3DRenderState*)g_d3dWritePtr)->value = (g_std3DFogColorRed8 << STD3D_FOG_RED_SHIFT) |
															(g_std3DFogColorGreen8 << STD3D_FOG_GREEN_SHIFT) |
															g_std3DFogColorBlue8;
				g_d3dWritePtr += sizeof(Std3DRenderState);
				((Std3DRenderState*)g_d3dWritePtr)->state = STD3D_STATE_FOGTABLEMODE;
				((Std3DRenderState*)g_d3dWritePtr)->value = STD3D_FOG_LINEAR;
				g_d3dWritePtr += sizeof(Std3DRenderState);
				((Std3DRenderState*)g_d3dWritePtr)->state = STD3D_STATE_FOGTABLESTART;
				((Std3DRenderState*)g_d3dWritePtr)->value = g_std3DFogTableStartBits;
				g_d3dWritePtr += sizeof(Std3DRenderState);
				((Std3DRenderState*)g_d3dWritePtr)->state = STD3D_STATE_FOGTABLEEND;
				((Std3DRenderState*)g_d3dWritePtr)->value = g_std3DFogTableEndBits;
				g_d3dWritePtr += sizeof(Std3DRenderState);
			} else {
				((D3DINSTRUCTION*)g_d3dWritePtr)->wCount = 1;
				g_d3dWritePtr += sizeof(D3DINSTRUCTION);
				((Std3DRenderState*)g_d3dWritePtr)->state = STD3D_STATE_FOGENABLE;
				((Std3DRenderState*)g_d3dWritePtr)->value = 0;
				g_d3dWritePtr += sizeof(Std3DRenderState);
			}
		}
		g_d3dStateFlags = flags;
	}
}

// FUNCTION: XW 0x4B3100
int std3D_CreateMipSurface(struct Std3DVBuffer* sourceBuffer, struct Std3DTexCacheNode* cacheEntry,
						   int useTransparency, int useFourBitAlpha) {
	IDirectDrawSurface* systemSurface = NULL;
	IDirectDrawSurface* videoSurface = NULL;
	IDirect3DTexture* systemTexture = NULL;
	IDirect3DTexture* videoTexture = NULL;
	Std3DVBuffer* uploadBuffer = sourceBuffer;
	Std3DVBuffer* tiledBuffer = NULL;
	unsigned int width = sourceBuffer->raster.width;
	unsigned int height;
	unsigned int pixelCount;
	D3DTEXTUREHANDLE textureHandle;
	DDSURFACEDESC descriptor;
	DDSURFACEDESC lockedDesc;
	DDCOLORKEY colorKey;
	HRESULT status;
	if (width >= STD3D_DEVICE_MIN_TEXTURE_SIZE) {
		if (width >= STD3D_DEVICE_MAX_TEXTURE_SIZE)
			width = STD3D_DEVICE_MAX_TEXTURE_SIZE;
	} else {
		width = STD3D_DEVICE_MIN_TEXTURE_SIZE;
	}
	{
		unsigned int sourceHeight = sourceBuffer->raster.height;
		if (sourceHeight >= STD3D_DEVICE_MIN_TEXTURE_SIZE) {
			if (sourceHeight >= STD3D_DEVICE_MAX_TEXTURE_SIZE)
				sourceHeight = STD3D_DEVICE_MAX_TEXTURE_SIZE;
			height = sourceHeight;
		} else {
			height = STD3D_DEVICE_MIN_TEXTURE_SIZE;
		}
	}
	if (width < (unsigned int)g_std3DMinTextureWidth || (unsigned int)g_std3DMinTextureHeight > height ||
		(g_pStd3DCurDevice->caps.bSquareOnlyTexture != 0 && width != height)) {
		Std3DRasterInfo tiledRaster = sourceBuffer->raster;
		unsigned int requiredWidth;
		unsigned int requiredHeight;
		int horizontalTiles;
		int verticalTiles;
		int row;
		if (g_pStd3DCurDevice->caps.bSquareOnlyTexture == 0 || width == height) {
			requiredWidth = width > (unsigned int)g_std3DMinTextureWidth ? width : g_std3DMinTextureWidth;
			requiredHeight =
				(unsigned int)g_std3DMinTextureHeight > height ? g_std3DMinTextureHeight : height;
		} else {
			requiredWidth = width > height ? width : height;
			requiredHeight = requiredWidth;
		}
		horizontalTiles = (int)((double)requiredWidth / (double)width + g_textureTileCountRoundingBias);
		verticalTiles = (int)((double)requiredHeight / (double)height + g_textureTileCountRoundingBias);
		tiledRaster.width *= horizontalTiles;
		tiledRaster.height *= verticalTiles;
		tiledBuffer = std3D_AllocVBuffer(&tiledRaster, 0, 0, 0);
		for (row = 0; row != verticalTiles; ++row) {
			int column;
			for (column = 0; column != horizontalTiles; ++column)
				std3D_BlitVBuffer(tiledBuffer, sourceBuffer, column * width, row * height);
		}
		width = tiledRaster.width;
		uploadBuffer = tiledBuffer;
		height = tiledRaster.height;
	}
	pixelCount = height * width;
	if (useTransparency != 0 && g_pStd3DCurDevice->caps.bAlphaTexture != 0) {
		cacheEntry->usesAlphaFormat = 1;
		if (useFourBitAlpha != 0)
			descriptor = g_pFmtRGBA4444->ddsd;
		else
			descriptor = g_pFmtRGBA1555->ddsd;
	} else if (useFourBitAlpha != 0) {
		cacheEntry->usesAlphaFormat = 1;
		descriptor = g_pFmtRGBA4444->ddsd;
	} else {
		cacheEntry->usesAlphaFormat = 0;
		descriptor = g_pFmtRGB565->ddsd;
	}
	nullsub_SharedNoOp();
	descriptor.dwWidth = width;
	descriptor.dwHeight = height;
	descriptor.dwSize = sizeof(descriptor);
	descriptor.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	descriptor.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;
	do {
		status =
			g_std3DDirectDraw->lpVtbl->CreateSurface(g_std3DDirectDraw, &descriptor, &systemSurface, NULL);
		if (status != STD3D_D3D_OK) {
			std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
			nullsub_SharedNoOp();
			systemSurface = NULL;
			break;
		}
		memset(&lockedDesc, 0, sizeof(lockedDesc));
		lockedDesc.dwSize = sizeof(lockedDesc);
		status = systemSurface->lpVtbl->Lock(systemSurface, NULL, &lockedDesc, DDLOCK_WAIT, NULL);
		if (status != STD3D_D3D_OK) {
			std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
			nullsub_SharedNoOp();
			break;
		}
		switch (uploadBuffer->raster.colorInfo.colorMode) {
			case STDCOLOR_PAL: {
				unsigned int row;
				std3D_LockVBuffer(uploadBuffer);
				if (useTransparency != 0 && g_pStd3DCurDevice->caps.bAlphaTexture != 0) {
					if (useFourBitAlpha != 0) {
						for (row = 0; row < height; ++row) {
							const uint8_t* sourceRow =
								uploadBuffer->pixels + row * uploadBuffer->raster.pitch;
							uint16_t* destinationRow =
								(uint16_t*)((uint8_t*)lockedDesc.lpSurface + row * lockedDesc.lPitch);
							unsigned int column;
							for (column = 0; column != width; ++column)
								destinationRow[column] = g_texConvBuf4444[sourceRow[column]];
						}
					} else {
						for (row = 0; row < height; ++row) {
							const uint8_t* sourceRow =
								uploadBuffer->pixels + row * uploadBuffer->raster.pitch;
							uint16_t* destinationRow =
								(uint16_t*)((uint8_t*)lockedDesc.lpSurface + row * lockedDesc.lPitch);
							unsigned int column;
							for (column = 0; column != width; ++column)
								destinationRow[column] = g_texConvBuf1555[sourceRow[column]];
						}
					}
				} else if (useFourBitAlpha != 0) {
					for (row = 0; row < height; ++row) {
						const uint8_t* sourceRow = uploadBuffer->pixels + row * uploadBuffer->raster.pitch;
						uint16_t* destinationRow =
							(uint16_t*)((uint8_t*)lockedDesc.lpSurface + row * lockedDesc.lPitch);
						unsigned int column;
						for (column = 0; column != width; ++column)
							destinationRow[column] = g_texConvBuf4444[sourceRow[column]];
					}
				} else {
					for (row = 0; row < height; ++row) {
						const uint8_t* sourceRow = uploadBuffer->pixels + row * uploadBuffer->raster.pitch;
						uint16_t* destinationRow =
							(uint16_t*)((uint8_t*)lockedDesc.lpSurface + row * lockedDesc.lPitch);
						unsigned int column;
						for (column = 0; column != width; ++column)
							destinationRow[column] = g_std3DPaletteScratch16[sourceRow[column]];
					}
				}
				std3D_UnlockVBuffer(uploadBuffer);
				break;
			}
			case STDCOLOR_RGB:
			case STDCOLOR_RGBA: {
				unsigned int row;
				std3D_LockVBuffer(uploadBuffer);
				for (row = 0; row < height; ++row)
					memcpy((uint8_t*)lockedDesc.lpSurface + row * lockedDesc.lPitch,
						   uploadBuffer->pixels + row * uploadBuffer->raster.pitch, sizeof(uint16_t) * width);
				std3D_UnlockVBuffer(uploadBuffer);
			} break;
			default:
				break;
		}
		status = systemSurface->lpVtbl->Unlock(systemSurface, NULL);
		if (status != STD3D_D3D_OK) {
			std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
			nullsub_SharedNoOp();
			break;
		}
		if (useTransparency != 0 && g_pStd3DCurDevice->caps.bAlphaTexture == 0) {
#ifdef XW_MODERN
			memset(&colorKey, 0, sizeof(colorKey));
#endif
			switch (uploadBuffer->raster.colorInfo.colorMode) {
				case STDCOLOR_PAL:
					colorKey.dwColorSpaceLowValue = g_std3DPaletteScratch16[0];
					colorKey.dwColorSpaceHighValue = g_std3DPaletteScratch16[0];
					break;
				case STDCOLOR_RGB:
					colorKey.dwColorSpaceLowValue = uploadBuffer->transparentColor;
					colorKey.dwColorSpaceHighValue = uploadBuffer->transparentColor;
					break;
				default:
					break;
			}
			systemSurface->lpVtbl->SetColorKey(systemSurface, DDCKEY_SRCBLT, &colorKey);
		}
		status = systemSurface->lpVtbl->QueryInterface(systemSurface, &g_iidDirect3DTexture,
													   (void**)&systemTexture);
		if (status != STD3D_D3D_OK) {
			std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
			nullsub_SharedNoOp();
			systemTexture = NULL;
			break;
		}
		status = systemSurface->lpVtbl->GetSurfaceDesc(systemSurface, &descriptor);
		if (status != STD3D_D3D_OK) {
			std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
			nullsub_SharedNoOp();
			systemTexture = NULL;
			break;
		}
		cacheEntry->ddsd = descriptor;
		descriptor.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
		descriptor.ddsCaps.dwCaps = DDSCAPS_ALLOCONLOAD | DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY;
		status =
			g_std3DDirectDraw->lpVtbl->CreateSurface(g_std3DDirectDraw, &descriptor, &videoSurface, NULL);
		if (status != STD3D_D3D_OK) {
			Std3DTexCacheNode* evictionEntry;
			int surfaceCreated = 0;
			if (status != STD3D_DDERR_OUTOFVIDEOMEMORY) {
				std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
				nullsub_SharedNoOp();
				break;
			}
			std3D_LookupErrorString(STD3D_DDERR_OUTOFVIDEOMEMORY, g_std3DErrorStringTable,
									STD3D_ERROR_STRING_COUNT);
			nullsub_SharedNoOp();
			nullsub_SharedNoOp();
			evictionEntry = g_pTexCacheHead;
			while (surfaceCreated == 0) {
				unsigned int freedPixelCount = 0;
				while (freedPixelCount < pixelCount) {
					if (evictionEntry == NULL)
						break;
					if (g_std3DTextureFrameTag == evictionEntry->cacheFrameTag)
						break;
					evictionEntry->pCachedSurface->lpVtbl->Release(evictionEntry->pCachedSurface);
					evictionEntry->pCachedTexture->lpVtbl->Release(evictionEntry->pCachedTexture);
					evictionEntry->bCached = 0;
					freedPixelCount += evictionEntry->pixelCount;
					std3D_CacheListRemove(evictionEntry);
					evictionEntry = evictionEntry->pNext;
				}
				if (freedPixelCount < pixelCount) {
					nullsub_SharedNoOp();
					videoSurface = NULL;
					break;
				}
				status = g_std3DDirectDraw->lpVtbl->CreateSurface(g_std3DDirectDraw, &descriptor,
																  &videoSurface, NULL);
				if (status == STD3D_D3D_OK) {
					surfaceCreated = 1;
					nullsub_SharedNoOp();
				} else if (status != STD3D_DDERR_OUTOFVIDEOMEMORY) {
					std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
					nullsub_SharedNoOp();
					break;
				}
			}
			if (status != STD3D_D3D_OK)
				break;
		}
		status =
			videoSurface->lpVtbl->QueryInterface(videoSurface, &g_iidDirect3DTexture, (void**)&videoTexture);
		if (status != STD3D_D3D_OK) {
			std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
			nullsub_SharedNoOp();
			videoTexture = NULL;
			break;
		}
		status = videoTexture->lpVtbl->Load(videoTexture, systemTexture);
		if (status != STD3D_D3D_OK) {
			std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
			nullsub_SharedNoOp();
			break;
		}
		status = videoTexture->lpVtbl->GetHandle(videoTexture, g_d3dDevice, &textureHandle);
		if (status != STD3D_D3D_OK) {
			std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
			nullsub_SharedNoOp();
			textureHandle = 0;
		}
		systemTexture->lpVtbl->Release(systemTexture);
		systemTexture = NULL;
		systemSurface->lpVtbl->Release(systemSurface);
		systemSurface = NULL;
		if (tiledBuffer != NULL)
			std3D_FreeVBuffer(tiledBuffer);
		cacheEntry->pCachedTexture = videoTexture;
		cacheEntry->pCachedSurface = videoSurface;
		cacheEntry->texHandle = textureHandle;
		cacheEntry->width = width;
		cacheEntry->height = height;
		cacheEntry->bCached = 1;
		cacheEntry->pixelCount = pixelCount;
		cacheEntry->cacheFrameTag = g_std3DTextureFrameTag;
		std3D_CacheListAppend(cacheEntry);
		return 1;
	} while (0);
	if (systemSurface != NULL)
		systemSurface->lpVtbl->Release(systemSurface);
	if (systemTexture != NULL)
		systemTexture->lpVtbl->Release(systemTexture);
	if (tiledBuffer != NULL)
		std3D_FreeVBuffer(tiledBuffer);
	if (videoSurface != NULL)
		videoSurface->lpVtbl->Release(videoSurface);
	if (videoTexture != NULL)
		videoTexture->lpVtbl->Release(videoTexture);
	cacheEntry->pCachedTexture = NULL;
	cacheEntry->pCachedSurface = NULL;
	cacheEntry->texHandle = 0;
	cacheEntry->bCached = 0;
	cacheEntry->cacheFrameTag = 0;
	nullsub_SharedNoOp();
	return 0;
}

// FUNCTION: XW 0x4B3AF0
void std3D_FlushTextureCache(void) {
	Std3DTexCacheNode* node = g_pTexCacheHead;
	while (node != NULL) {
		Std3DTexCacheNode* releasedNode;
		if (node->pCachedSurface != NULL) {
			node->pCachedSurface->lpVtbl->Release(node->pCachedSurface);
			node->pCachedSurface = NULL;
		}
		if (node->pCachedTexture != NULL) {
			node->pCachedTexture->lpVtbl->Release(node->pCachedTexture);
			node->pCachedTexture = NULL;
		}
		releasedNode = node;
		node->bCached = 0;
		node->cacheFrameTag = 0;
		node = node->pNext;
		releasedNode->pNext = NULL;
		releasedNode->pPrev = NULL;
	}
	g_pTexCacheHead = NULL;
	g_pTexCacheTail = NULL;
	g_texCacheCount = 0;
	g_pStd3DCurDevice->availableMemory = g_pStd3DCurDevice->totalMemory;
	g_std3DTextureFrameTag = 1;
}

// FUNCTION: XW 0x4B3B70
void std3D_CacheListAppend(struct Std3DTexCacheNode* node) {
	if (g_pTexCacheHead == NULL) {
		g_pTexCacheTail = node;
		g_pTexCacheHead = node;
		node->pPrev = NULL;
		node->pNext = NULL;
	} else {
		g_pTexCacheTail->pNext = node;
		node->pPrev = g_pTexCacheTail;
		node->pNext = NULL;
		g_pTexCacheTail = node;
	}
	++g_texCacheCount;
	g_pStd3DCurDevice->availableMemory -= node->pixelCount;
}

// FUNCTION: XW 0x4B3BE0
void std3D_CacheListRemove(struct Std3DTexCacheNode* node) {
	if (g_pTexCacheHead == node) {
		g_pTexCacheHead = node->pNext;
		if (g_pTexCacheHead != NULL) {
			g_pTexCacheHead->pPrev = NULL;
			if (g_pTexCacheHead->pNext == NULL) {
				g_pTexCacheTail = g_pTexCacheHead;
			}
		} else {
			g_pTexCacheTail = NULL;
		}
	} else if (g_pTexCacheTail == node) {
		g_pTexCacheTail = node->pPrev;
		g_pTexCacheTail->pNext = NULL;
	} else {
		node->pPrev->pNext = node->pNext;
		node->pNext->pPrev = node->pPrev;
	}
	--g_texCacheCount;
	g_pStd3DCurDevice->availableMemory += node->pixelCount;
}

// FUNCTION: XW 0x4B3C90
int std3D_QueryTextureVidMem(uint32_t* totalBytes, uint32_t* freeBytes) {
	IDirectDraw* directDraw2 = NULL;
	DDSCAPS textureCaps;
	if (g_std3DDirectDraw->lpVtbl->QueryInterface(g_std3DDirectDraw, &g_directDraw2InterfaceGuid,
												  (void**)&directDraw2) != 0) {
		return 0;
	}
	textureCaps.dwCaps = DDSCAPS_TEXTURE;
	if (directDraw2->lpVtbl->GetAvailableVidMem(directDraw2, &textureCaps, totalBytes, freeBytes) != 0) {
		return 0;
	}
	directDraw2->lpVtbl->Release(directDraw2);
	return 1;
}

// FUNCTION: XW 0x4B3D00
void std3D_CacheTextureSurface(struct Std3DTexCacheNode* node) {
	node->cacheFrameTag = g_std3DTextureFrameTag;
	std3D_CacheListRemove(node);
	std3D_CacheListAppend(node);
}

// FUNCTION: XW 0x4B3D30
int std3D_ClearZBuffer(void) {
	D3DRECT clearRect;
	DDBLTFX blitEffects;
	HRESULT status;

	memset(&blitEffects, 0, sizeof(blitEffects));
	blitEffects.dwSize = sizeof(blitEffects);
	blitEffects.dwFillDepth = 0;
	if (g_std3DZCmpMask != STD3D_ZCMP_GREATER)
		blitEffects.dwFillDepth = UINT16_MAX;
	clearRect.x1 = g_std3DQuadRect.x;
	clearRect.y1 = g_std3DQuadRect.y;
	clearRect.x2 = g_std3DQuadRect.x + g_std3DQuadRect.width;
	clearRect.y2 = g_std3DQuadRect.y + g_std3DQuadRect.height;
	for (;;) {
		status = g_std3DZBufferVBuffer.surfaceBlock.surface->lpVtbl->Blt(
			g_std3DZBufferVBuffer.surfaceBlock.surface, &clearRect, NULL, NULL, DDBLT_WAIT | DDBLT_DEPTHFILL,
			&blitEffects);
		if (status == STD3D_D3D_OK)
			return 1;
		if (status == STD3D_DDERR_SURFACELOST)
			status = g_std3DZBufferVBuffer.surfaceBlock.surface->lpVtbl->Restore(
				g_std3DZBufferVBuffer.surfaceBlock.surface);
		if (status != STD3D_D3D_OK) {
			std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
			nullsub_SharedNoOp();
			return 0;
		}
	}
}

// FUNCTION: XW 0x4B3E00
int std3D_SelectBestDevice(const struct Std3DDeviceCaps* requested) {
	int bestScore;
	int bestDeviceIndex;
	unsigned int deviceIndex;
	if (g_std3DNumDevices == 0) {
		return 0;
	}
	bestScore = STD3D_DEVICE_MATCH_NONE;
	bestDeviceIndex = 0;
	for (deviceIndex = 0; deviceIndex < g_std3DNumDevices; ++deviceIndex) {
		Std3DDevice* device = &g_std3DDevices[deviceIndex];
		int matchScore = STD3D_DEVICE_MATCH_NONE;
		int requestedPerspective = requested->bTexturePerspective;
		if (requestedPerspective == 0 || device->caps.bTexturePerspective == requestedPerspective) {
			int requestedZBuffer;
			matchScore = STD3D_DEVICE_MATCH_PERSPECTIVE;
			requestedZBuffer = requested->bHasZBuffer;
			if (requestedZBuffer == 0 || device->caps.bHasZBuffer == requestedZBuffer) {
				matchScore = STD3D_DEVICE_MATCH_Z_BUFFER;
				if ((requested->colorModelFlags & device->caps.colorModelFlags) != 0) {
					matchScore = STD3D_DEVICE_MATCH_COLOR_MODEL;
					if (requested->bHardware == device->caps.bHardware) {
						nullsub_SharedNoOp();
						return deviceIndex;
					}
				}
			}
		}
		if (bestScore < matchScore) {
			bestScore = matchScore;
			bestDeviceIndex = deviceIndex;
		}
	}
	nullsub_SharedNoOp();
	return bestDeviceIndex;
}

// FUNCTION: XW 0x4B3ED0
int std3D_FindClosestFormat(const struct ColorInfo* requested, const struct Std3DTexFmt* formats,
							unsigned int formatCount) {
	unsigned int formatIndex;
	int bestScore;
	int bestIndex;
	if (formatCount == 0)
		return 0;
	bestScore = STD3D_FORMAT_MATCH_NONE;
	bestIndex = 0;
	for (formatIndex = 0; formatIndex < formatCount; ++formatIndex) {
		const ColorInfo* color = &formats[formatIndex].colorInfo;
		int matchScore = STD3D_FORMAT_MATCH_NONE;
		if (color->colorMode == requested->colorMode) {
			matchScore = STD3D_FORMAT_MATCH_MODE;
			if (color->bpp == requested->bpp) {
				matchScore = STD3D_FORMAT_MATCH_BPP;
				switch (requested->colorMode) {
					case STDCOLOR_RGB:
						if (color->redBPP == requested->redBPP && color->greenBPP == requested->greenBPP &&
							color->blueBPP == requested->blueBPP) {
							nullsub_SharedNoOp();
							return formatIndex;
						}
						break;
					case STDCOLOR_RGBA:
						if (color->colorMode == STDCOLOR_RGBA)
							matchScore = STD3D_FORMAT_MATCH_ALPHA;
						if (color->redBPP == requested->redBPP && color->greenBPP == requested->greenBPP &&
							color->blueBPP == requested->blueBPP && color->alphaBPP == requested->alphaBPP) {
							nullsub_SharedNoOp();
							return formatIndex;
						}
						break;
					default:
						nullsub_SharedNoOp();
						return formatIndex;
				}
			}
		}
		if (bestScore < matchScore) {
			bestScore = matchScore;
			bestIndex = formatIndex;
		}
	}
	nullsub_SharedNoOp();
	return bestIndex;
}

// FUNCTION: XW 0x4B4010
void std3D_BuildViewportQuad(const struct Std3DViewportRect* rect) {
	memcpy(&g_std3DQuadRect, rect, sizeof(g_std3DQuadRect));
	memset(g_std3DQuadVerts, 0, sizeof(g_std3DQuadVerts));
	g_std3DQuadVerts[0].sx = (float)g_std3DQuadRect.x;
	g_std3DQuadVerts[0].sy = (float)g_std3DQuadRect.y;
	g_std3DQuadVerts[1].sx = (float)(g_std3DQuadRect.x + g_std3DQuadRect.width);
	g_std3DQuadVerts[1].sy = (float)g_std3DQuadRect.y;
	g_std3DQuadVerts[2].sx = (float)(g_std3DQuadRect.x + g_std3DQuadRect.width);
	g_std3DQuadVerts[2].sy = (float)(g_std3DQuadRect.y + g_std3DQuadRect.height);
	g_std3DQuadVerts[3].sx = (float)g_std3DQuadRect.x;
	g_std3DQuadVerts[3].sy = (float)(g_std3DQuadRect.y + g_std3DQuadRect.height);
	g_std3DViewportQuadTriangles[0].v0 = 0;
	g_std3DViewportQuadTriangles[0].v1 = 1;
	g_std3DViewportQuadTriangles[0].v2 = 2;
	g_std3DViewportQuadTriangles[0].texture = NULL;
	g_std3DViewportQuadTriangles[0].flags = 0x8200;
	g_std3DViewportQuadTriangles[1].v0 = 0;
	g_std3DViewportQuadTriangles[1].v1 = 2;
	g_std3DViewportQuadTriangles[1].v2 = 3;
	g_std3DViewportQuadTriangles[1].texture = NULL;
	g_std3DViewportQuadTriangles[1].flags = 0x8200;
}

// FUNCTION: XW 0x4B4100
int std3D_SetInitialRenderState(void) {
	IDirect3DExecuteBuffer* executeBuffer = NULL;
	D3DEXECUTEBUFFERDESC bufferDesc;
	D3DEXECUTEDATA executeData;
	HRESULT status;
	D3DINSTRUCTION* instruction;
	Std3DRenderState* states;
	int stateIndex;
	int enableZBuffer;
	memset(&bufferDesc, 0, sizeof(bufferDesc));
	bufferDesc.dwSize = sizeof(bufferDesc);
	bufferDesc.dwFlags = STD3D_EXEC_BUFFER_SIZE_PRESENT;
	bufferDesc.dwBufferSize = STD3D_INITIAL_BUFFER_SIZE;
	status = g_d3dDevice->lpVtbl->CreateExecuteBuffer(g_d3dDevice, &bufferDesc, &executeBuffer, NULL);
	if (status != STD3D_D3D_OK) {
		std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
		nullsub_SharedNoOp();
	}
	status = executeBuffer->lpVtbl->Lock(executeBuffer, &bufferDesc);
	if (status != STD3D_D3D_OK) {
		std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
		nullsub_SharedNoOp();
	}
	memset(bufferDesc.lpData, 0, STD3D_INITIAL_BUFFER_SIZE);
	instruction = bufferDesc.lpData;
	instruction->bOpcode = D3DOP_STATERENDER;
	instruction->bSize = sizeof(Std3DRenderState);
	instruction->wCount = STD3D_INITIAL_STATE_COUNT;
	states = (Std3DRenderState*)(instruction + 1);
	stateIndex = 0;
	states[stateIndex].state = STD3D_STATE_TEXTUREPERSPECTIVE;
	states[stateIndex++].value = g_std3DCapFlags & STD3D_RS_TEXTURE_PERSPECTIVE;
	states[stateIndex].state = STD3D_STATE_TEXTUREMAG;
	states[stateIndex++].value =
		(g_std3DCapFlags & STD3D_RS_MAG_LINEAR) != 0 ? STD3D_FILTER_LINEAR : STD3D_FILTER_NEAREST;
	states[stateIndex].state = STD3D_STATE_TEXTUREMIN;
	states[stateIndex++].value =
		(g_std3DCapFlags & STD3D_RS_MIN_LINEAR) != 0 ? STD3D_FILTER_LINEAR : STD3D_FILTER_NEAREST;
	states[stateIndex].state = STD3D_STATE_SUBPIXEL;
	states[stateIndex++].value = (g_std3DCapFlags & STD3D_RS_SUBPIXEL) != 0;
	states[stateIndex].state = STD3D_STATE_SUBPIXELX;
	states[stateIndex++].value = (g_std3DCapFlags & STD3D_RS_SUBPIXEL_X) != 0;
	states[stateIndex].state = STD3D_STATE_WRAPU;
	states[stateIndex++].value = 0;
	states[stateIndex].state = STD3D_STATE_WRAPV;
	states[stateIndex++].value = 0;
	states[stateIndex].state = STD3D_STATE_BLENDENABLE;
	states[stateIndex++].value = (g_std3DCapFlags & (STD3D_RS_ALPHA_BLEND | STD3D_RS_MODULATE_ALPHA)) != 0;
	if ((g_std3DCapFlags & (STD3D_RS_ALPHA_BLEND | STD3D_RS_MODULATE_ALPHA)) != 0) {
		states[stateIndex].state = STD3D_STATE_TEXTUREMAPBLEND;
		if ((g_std3DCapFlags & STD3D_RS_MODULATE_ALPHA) != 0)
			states[stateIndex].value = STD3D_MAP_BLEND_MODULATE_ALPHA;
		else
			states[stateIndex].value = STD3D_MAP_BLEND_MODULATE;
		states[stateIndex + 1].state = STD3D_STATE_SRCBLEND;
		states[stateIndex + 1].value = STD3D_BLEND_SRC_ALPHA;
		states[stateIndex + 2].state = STD3D_STATE_DESTBLEND;
		states[stateIndex + 2].value = STD3D_BLEND_INV_SRC_ALPHA;
	} else {
		states[stateIndex].state = STD3D_STATE_TEXTUREMAPBLEND;
		states[stateIndex].value = STD3D_MAP_BLEND_MODULATE;
		states[stateIndex + 1].state = STD3D_STATE_SRCBLEND;
		states[stateIndex + 1].value = STD3D_BLEND_ONE;
		states[stateIndex + 2].state = STD3D_STATE_DESTBLEND;
		states[stateIndex + 2].value = STD3D_BLEND_ZERO;
	}
	stateIndex += STD3D_INITIAL_BLEND_STATE_COUNT;
	states[stateIndex].state = STD3D_STATE_ALPHATESTENABLE;
	states[stateIndex++].value = 1;
	states[stateIndex].state = STD3D_STATE_ALPHAFUNC;
	states[stateIndex++].value = STD3D_CMP_NOT_EQUAL;
	states[stateIndex].state = STD3D_STATE_STIPPLEDALPHA;
	if (g_pStd3DCurDevice->caps.bStippledShade != 0)
		states[stateIndex].value = 1;
	else
		states[stateIndex].value = 0;
	++stateIndex;
	states[stateIndex].state = STD3D_STATE_SHADEMODE;
	states[stateIndex++].value = STD3D_SHADE_GOURAUD;
	states[stateIndex].state = STD3D_STATE_MONOENABLE;
	states[stateIndex++].value = (g_std3DCapFlags & STD3D_RS_DISABLE_MONO) == 0;
	states[stateIndex].state = STD3D_STATE_SPECULARENABLE;
	states[stateIndex++].value = (g_std3DCapFlags & STD3D_RS_SPECULAR) != 0;
	states[stateIndex].state = STD3D_STATE_FOGENABLE;
	states[stateIndex++].value = (g_std3DCapFlags & STD3D_RS_FOG) != 0;
	states[stateIndex].state = STD3D_STATE_FILLMODE;
	states[stateIndex++].value = STD3D_FILL_SOLID;
	states[stateIndex].state = STD3D_STATE_DITHERENABLE;
	states[stateIndex++].value = (g_std3DCapFlags & STD3D_RS_DITHER) != 0;
	states[stateIndex].state = STD3D_STATE_ANTIALIAS;
	states[stateIndex++].value = (g_std3DCapFlags & STD3D_RS_ANTIALIAS) != 0;
	if (g_std3DZBufferEnabled != 0) {
		enableZBuffer = 1;
		nullsub_SharedNoOp();
	} else {
		enableZBuffer = 0;
		nullsub_SharedNoOp();
	}
	states[stateIndex].state = STD3D_STATE_ZENABLE;
	states[stateIndex++].value = enableZBuffer;
	states[stateIndex].state = STD3D_STATE_ZWRITEENABLE;
	states[stateIndex++].value = enableZBuffer;
	states[stateIndex].state = STD3D_STATE_ZFUNC;
	states[stateIndex++].value = std3D_MapZCmpFunc(g_std3DZCmpMask);
	states[stateIndex].state = STD3D_STATE_CULLMODE;
	states[stateIndex++].value = STD3D_CULL_NONE;
	instruction = (D3DINSTRUCTION*)&states[stateIndex];
	instruction->bOpcode = D3DOP_EXIT;
	instruction->bSize = 0;
	instruction->wCount = 0;
	status = executeBuffer->lpVtbl->Unlock(executeBuffer);
	if (status != STD3D_D3D_OK) {
		std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
		nullsub_SharedNoOp();
	}
	memset(&executeData, 0, sizeof(executeData));
	executeData.dwInstructionOffset = 0;
	executeData.dwInstructionLength = sizeof(D3DINSTRUCTION) * 2 + stateIndex * sizeof(Std3DRenderState);
	executeData.dwSize = sizeof(executeData);
	executeBuffer->lpVtbl->SetExecuteData(executeBuffer, &executeData);
	status = g_d3dDevice->lpVtbl->BeginScene(g_d3dDevice);
	if (status != STD3D_D3D_OK) {
		std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
		nullsub_SharedNoOp();
	}
	status = g_d3dDevice->lpVtbl->Execute(g_d3dDevice, executeBuffer, g_d3dViewport, D3DEXECUTE_UNCLIPPED);
	if (status != STD3D_D3D_OK) {
		std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
		nullsub_SharedNoOp();
	}
	status = g_d3dDevice->lpVtbl->EndScene(g_d3dDevice);
	if (status != STD3D_D3D_OK) {
		std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
		nullsub_SharedNoOp();
	}
	executeBuffer->lpVtbl->Release(executeBuffer);
	g_d3dStateFlags = g_std3DCapFlags;
	nullsub_SharedNoOp();
	return 1;
}

// FUNCTION: XW 0x4B45B0
int std3D_CreateViewport(unsigned int width, unsigned int height) {
	HRESULT status;
	D3DVIEWPORT viewport;
	Std3DViewportRect rect;
	status = g_lpD3D->lpVtbl->CreateViewport(g_lpD3D, &g_d3dViewport, NULL);
	if (status != STD3D_D3D_OK) {
		std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
		nullsub_SharedNoOp();
		return 0;
	}
	status = g_d3dDevice->lpVtbl->AddViewport(g_d3dDevice, g_d3dViewport);
	if (status != STD3D_D3D_OK) {
		std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
		nullsub_SharedNoOp();
		return 0;
	}
	memset(&viewport, 0, sizeof(viewport));
	viewport.dwY = 0;
	viewport.dwX = 0;
	viewport.dwWidth = width;
	viewport.dwHeight = height;
	viewport.dwSize = sizeof(viewport);
	viewport.dvScaleX = width * 0.5f;
	viewport.dvScaleY = height * 0.5f;
	viewport.dvMaxX = width / (viewport.dvScaleX * 2.0f);
	viewport.dvMaxY = height / (viewport.dvScaleY * 2.0f);
	status = g_d3dViewport->lpVtbl->SetViewport(g_d3dViewport, &viewport);
	if (status != STD3D_D3D_OK) {
		std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
		nullsub_SharedNoOp();
		return 0;
	}
	rect.x = 0;
	rect.y = 0;
	rect.width = width;
	rect.height = height;
	std3D_BuildViewportQuad(&rect);
	nullsub_SharedNoOp();
	return 1;
}

// FUNCTION: XW 0x4B4740
int std3D_CreateZBuffer(unsigned int width, unsigned int height) {
	uint32_t supportedDepthMask;
	HRESULT status;
	g_std3DZBufferVBuffer.storageType = STD3D_STORAGE_SURFACE;
	g_std3DZBufferVBuffer.bVideoMemory = 0;
	g_std3DZBufferVBuffer.raster = *g_pStd3DRenderTarget;
	g_std3DZBufferVBuffer.field_58 = 0;
	g_pStd3DZBufferState = &g_std3DZBufferVBuffer.surfaceBlock;
	g_std3DZBufferVBuffer.pixels = NULL;
	memset(&g_pStd3DZBufferState->desc, 0, sizeof(g_pStd3DZBufferState->desc));
	g_pStd3DZBufferState->desc.dwSize = sizeof(g_pStd3DZBufferState->desc);
	g_pStd3DZBufferState->desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | STD3D_DESC_ZBUFFER_BIT_DEPTH;
	g_pStd3DZBufferState->desc.ddsCaps.dwCaps = DDSCAPS_ZBUFFER;
	g_pStd3DZBufferState->desc.dwWidth = width;
	g_pStd3DZBufferState->desc.dwHeight = height;
	if (g_pStd3DCurDevice->caps.bHardware != 0) {
		g_pStd3DZBufferState->desc.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
	} else {
		g_pStd3DZBufferState->desc.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
	}
	supportedDepthMask = g_pStd3DCurDevice->d3dDesc.dwDeviceZBufferBitDepth;
	if ((supportedDepthMask & STD3D_DEPTH_MASK_32) != 0) {
		g_pStd3DZBufferState->desc.dwZBufferBitDepth = CHAR_BIT * sizeof(uint32_t);
	} else if ((supportedDepthMask & STD3D_DEPTH_MASK_16) != 0) {
		g_pStd3DZBufferState->desc.dwZBufferBitDepth = CHAR_BIT * sizeof(uint16_t);
	} else if ((supportedDepthMask & STD3D_DEPTH_MASK_8) != 0) {
		g_pStd3DZBufferState->desc.dwZBufferBitDepth = CHAR_BIT * sizeof(uint8_t);
	} else {
		nullsub_SharedNoOp();
		return 0;
	}
	nullsub_SharedNoOp();
	{
		Std3DSurfaceBlock* state = g_pStd3DZBufferState;
		DDSURFACEDESC* descriptor = &state->desc;
		status =
			g_std3DDirectDraw->lpVtbl->CreateSurface(g_std3DDirectDraw, descriptor, &state->surface, NULL);
	}
	if (status != STD3D_D3D_OK) {
		std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
		nullsub_SharedNoOp();
		return 0;
	}
	status =
		g_std3DRenderSurface->lpVtbl->AddAttachedSurface(g_std3DRenderSurface, g_pStd3DZBufferState->surface);
	if (status != STD3D_D3D_OK) {
		std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
		nullsub_SharedNoOp();
		return 0;
	}
	status = g_pStd3DZBufferState->surface->lpVtbl->GetSurfaceDesc(g_pStd3DZBufferState->surface,
																   &g_pStd3DZBufferState->desc);
	if (status != STD3D_D3D_OK) {
		std3D_LookupErrorString(status, g_std3DErrorStringTable, STD3D_ERROR_STRING_COUNT);
		nullsub_SharedNoOp();
		return 0;
	}
	if ((g_pStd3DZBufferState->desc.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) != 0) {
		g_std3DZBufferVBuffer.bVideoMemory = 1;
	}
	nullsub_SharedNoOp();
	nullsub_SharedNoOp();
	return 1;
}

// FUNCTION: XW 0x4B4990
HRESULT AERON_DXAPI std3D_EnumDevicesCallback(DxGuid* deviceGuid, char* description, char* deviceName,
											  D3DDEVICEDESC* hardwareDesc, D3DDEVICEDESC* softwareDesc,
											  void* context) {
	Std3DDevice* device;
	(void)context;
	if (g_std3DNumDevices < STD3D_DEVICE_CAPACITY) {
		device = &g_std3DDevices[g_std3DNumDevices];
		device->guid = *deviceGuid;
		strncpy(device->deviceDescription, description, sizeof(device->deviceDescription));
		strncpy(device->deviceName, deviceName, sizeof(device->deviceName));
		/* Aeron's descriptor has the same fixed-width prefix as the recovered record. */
		if (hardwareDesc->dcmColorModel != 0) {
			device->caps.bHardware = 1;
			memcpy(&device->d3dDesc, hardwareDesc, sizeof(device->d3dDesc));
		} else {
			device->caps.bHardware = 0;
			memcpy(&device->d3dDesc, softwareDesc, sizeof(device->d3dDesc));
		}
		device->caps.colorModelFlags = 0;
		if (device->d3dDesc.dcmColorModel & STD3D_COLOR_MODEL_RGB)
			device->caps.colorModelFlags = STD3D_COLOR_MODEL_RGB;
		if (device->d3dDesc.dcmColorModel & STD3D_COLOR_MODEL_MONO)
			device->caps.colorModelFlags |= STD3D_COLOR_MODEL_MONO;
		device->caps.bTexturePerspective =
			device->d3dDesc.dpcTriCaps.dwTextureCaps & STD3D_TEXTURE_CAP_PERSPECTIVE;
		device->caps.bHasZBuffer = device->d3dDesc.dwDeviceZBufferBitDepth != 0;
		device->caps.bSquareOnlyTexture =
			(device->d3dDesc.dpcTriCaps.dwTextureCaps & STD3D_TEXTURE_CAP_SQUARE_ONLY) != 0;
		device->caps.bAlphaTexture =
			(device->d3dDesc.dpcTriCaps.dwTextureCaps & STD3D_TEXTURE_CAP_ALPHA) != 0;
		if (!(device->d3dDesc.dpcTriCaps.dwShadeCaps & STD3D_SHADE_ALPHA_FLAT_BLEND) &&
			(device->d3dDesc.dpcTriCaps.dwShadeCaps & STD3D_SHADE_ALPHA_FLAT_STIPPLE))
			device->caps.bStippledShade = 1;
		else
			device->caps.bStippledShade = 0;
		device->caps.bAlphaBlend =
			((device->d3dDesc.dpcTriCaps.dwTextureBlendCaps & STD3D_TEXTURE_BLEND_MODULATE_ALPHA) &&
			 (device->d3dDesc.dpcTriCaps.dwShadeCaps & STD3D_SHADE_ALPHA_GOURAUD_BLEND)) ||
			device->caps.bStippledShade;
		device->caps.bColorKeyTexture =
			(device->d3dDesc.dpcTriCaps.dwTextureCaps & STD3D_TEXTURE_CAP_COLOR_KEY) != 0;
		device->caps.renderBitDepthMask = std3D_PackRenderBitDepths(device->d3dDesc.dwDeviceRenderBitDepth);
		device->caps.zCmpCapsMask = std3D_PackZCmpCaps(device->d3dDesc.dpcTriCaps.dwZCmpCaps);
		device->caps.minTextureWidth = device->caps.minTextureHeight = STD3D_DEVICE_MIN_TEXTURE_SIZE;
		device->caps.maxTextureWidth = device->caps.maxTextureHeight = STD3D_DEVICE_MAX_TEXTURE_SIZE;
		device->caps.maxBufferSize = device->d3dDesc.dwMaxBufferSize;
		device->caps.maxVertexCount = device->d3dDesc.dwMaxVertexCount;
		nullsub_SharedNoOp();
		nullsub_SharedNoOp();
		nullsub_SharedNoOp();
		++g_std3DNumDevices;
		return STD3D_ENUM_CONTINUE;
	}
	return STD3D_ENUM_STOP;
}

// FUNCTION: XW 0x4B4C10
int AERON_DXAPI std3D_EnumTextureFormats(DDSURFACEDESC* surfaceDesc, void* context) {
	Std3DTexFmt* format;
	(void)context;
	if (g_std3DNumTextureFormats < STD3D_TEXTURE_FORMAT_CAPACITY) {
		format = &g_std3DTextureFormats[g_std3DNumTextureFormats];
		memcpy(&format->ddsd, surfaceDesc, sizeof(format->ddsd));
		if (surfaceDesc->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8) {
			format->colorInfo.colorMode = STDCOLOR_PAL;
			format->colorInfo.bpp = STD3D_PALETTE_BITS;
			format->colorInfo.redPosShift = 0;
			format->colorInfo.redPosShiftRight = 0;
			format->colorInfo.redBPP = 0;
			format->colorInfo.greenPosShift = 0;
			format->colorInfo.greenPosShiftRight = 0;
			format->colorInfo.greenBPP = 0;
			format->colorInfo.bluePosShift = 0;
			format->colorInfo.bluePosShiftRight = 0;
			format->colorInfo.blueBPP = 0;
			format->colorInfo.alphaPosShift = 0;
			format->colorInfo.alphaPosShiftRight = 0;
			format->colorInfo.alphaBPP = 0;
			nullsub_SharedNoOp();
			++g_std3DNumTextureFormats;
			return STD3D_ENUM_CONTINUE;
		} else if (surfaceDesc->ddpfPixelFormat.dwFlags & STD3D_PIXEL_FORMAT_PALETTE4) {
			return STD3D_ENUM_CONTINUE;
		} else if (surfaceDesc->ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS) {
			int shift;
			int bits;
			uint32_t mask;
			shift = 0;
			format->colorInfo.colorMode = STDCOLOR_RGBA;
			format->colorInfo.bpp = surfaceDesc->ddpfPixelFormat.dwRGBBitCount;
			for (mask = surfaceDesc->ddpfPixelFormat.dwRBitMask; (mask & 1) == 0; ++shift)
				mask >>= 1;
			format->colorInfo.redPosShift = shift;
			format->colorInfo.redPosShiftRight =
				std3D_Log2Floor(UINT8_MAX / (surfaceDesc->ddpfPixelFormat.dwRBitMask >> shift));
			for (bits = 0; (mask & 1) != 0; ++bits)
				mask >>= 1;
			format->colorInfo.redBPP = bits;
			shift = 0;
			for (mask = surfaceDesc->ddpfPixelFormat.dwGBitMask; (mask & 1) == 0; ++shift)
				mask >>= 1;
			format->colorInfo.greenPosShift = shift;
			format->colorInfo.greenPosShiftRight =
				std3D_Log2Floor(UINT8_MAX / (surfaceDesc->ddpfPixelFormat.dwGBitMask >> shift));
			for (bits = 0; (mask & 1) != 0; ++bits)
				mask >>= 1;
			format->colorInfo.greenBPP = bits;
			shift = 0;
			for (mask = surfaceDesc->ddpfPixelFormat.dwBBitMask; (mask & 1) == 0; ++shift)
				mask >>= 1;
			format->colorInfo.bluePosShift = shift;
			format->colorInfo.bluePosShiftRight =
				std3D_Log2Floor(UINT8_MAX / (surfaceDesc->ddpfPixelFormat.dwBBitMask >> shift));
			for (bits = 0; (mask & 1) != 0; ++bits)
				mask >>= 1;
			format->colorInfo.blueBPP = bits;
			shift = 0;
			for (mask = surfaceDesc->ddpfPixelFormat.dwRGBAlphaBitMask; (mask & 1) == 0; ++shift)
				mask >>= 1;
			format->colorInfo.alphaPosShift = shift;
			format->colorInfo.alphaPosShiftRight =
				std3D_Log2Floor(UINT8_MAX / (surfaceDesc->ddpfPixelFormat.dwRGBAlphaBitMask >> shift));
			for (bits = 0; (mask & 1) != 0; ++bits)
				mask >>= 1;
			format->colorInfo.alphaBPP = bits;
			nullsub_SharedNoOp();
			++g_std3DNumTextureFormats;
			return STD3D_ENUM_CONTINUE;
		} else {
			int shift;
			int bits;
			uint32_t mask;
			shift = 0;
			format->colorInfo.colorMode = STDCOLOR_RGB;
			format->colorInfo.bpp = surfaceDesc->ddpfPixelFormat.dwRGBBitCount;
			for (mask = surfaceDesc->ddpfPixelFormat.dwRBitMask; (mask & 1) == 0; ++shift)
				mask >>= 1;
			format->colorInfo.redPosShift = shift;
			format->colorInfo.redPosShiftRight =
				std3D_Log2Floor(UINT8_MAX / (surfaceDesc->ddpfPixelFormat.dwRBitMask >> shift));
			for (bits = 0; (mask & 1) != 0; ++bits)
				mask >>= 1;
			format->colorInfo.redBPP = bits;
			shift = 0;
			for (mask = surfaceDesc->ddpfPixelFormat.dwGBitMask; (mask & 1) == 0; ++shift)
				mask >>= 1;
			format->colorInfo.greenPosShift = shift;
			format->colorInfo.greenPosShiftRight =
				std3D_Log2Floor(UINT8_MAX / (surfaceDesc->ddpfPixelFormat.dwGBitMask >> shift));
			for (bits = 0; (mask & 1) != 0; ++bits)
				mask >>= 1;
			format->colorInfo.greenBPP = bits;
			shift = 0;
			for (mask = surfaceDesc->ddpfPixelFormat.dwBBitMask; (mask & 1) == 0; ++shift)
				mask >>= 1;
			format->colorInfo.bluePosShift = shift;
			format->colorInfo.bluePosShiftRight =
				std3D_Log2Floor(UINT8_MAX / (surfaceDesc->ddpfPixelFormat.dwBBitMask >> shift));
			for (bits = 0; (mask & 1) != 0; ++bits)
				mask >>= 1;
			format->colorInfo.blueBPP = bits;
			format->colorInfo.alphaPosShift = 0;
			format->colorInfo.alphaPosShiftRight = 0;
			format->colorInfo.alphaBPP = 0;
		}
		nullsub_SharedNoOp();
		++g_std3DNumTextureFormats;
		return STD3D_ENUM_CONTINUE;
	}
	return STD3D_ENUM_STOP;
}

// FUNCTION: XW 0x4B4F60
unsigned int std3D_PackRenderBitDepths(unsigned int ddbdFlags) {
	unsigned int result = 0;

	if (ddbdFlags & 0x4000) {
		result = 1;
	}
	if (ddbdFlags & 0x2000) {
		result |= 2;
	}
	if (ddbdFlags & 0x1000) {
		result |= 4;
	}
	if (ddbdFlags & 0x800) {
		result |= 8;
	}
	if (ddbdFlags & 0x400) {
		result |= 0x10;
	}
	if (ddbdFlags & 0x200) {
		result |= 0x20;
	}
	if (ddbdFlags & 0x100) {
		result |= 0x40;
	}
	return result;
}

// FUNCTION: XW 0x4B4FB0
unsigned int std3D_PackZCmpCaps(unsigned int d3dpcmpcaps) {
	unsigned int result = 0;
	if (d3dpcmpcaps & STD3D_ZCMP_NEVER) {
		result = STD3D_ZCMP_NEVER;
	}
	if (d3dpcmpcaps & STD3D_ZCMP_EQUAL) {
		result |= STD3D_ZCMP_EQUAL;
	}
	if (d3dpcmpcaps & STD3D_ZCMP_LESS) {
		result |= STD3D_ZCMP_LESS;
	}
	if (d3dpcmpcaps & STD3D_ZCMP_LESS_EQUAL) {
		result |= STD3D_ZCMP_LESS_EQUAL;
	}
	if (d3dpcmpcaps & STD3D_ZCMP_GREATER) {
		result |= STD3D_ZCMP_GREATER;
	}
	if (d3dpcmpcaps & STD3D_ZCMP_NOT_EQUAL) {
		result |= STD3D_ZCMP_NOT_EQUAL;
	}
	if (d3dpcmpcaps & STD3D_ZCMP_GREATER_EQUAL) {
		result |= STD3D_ZCMP_GREATER_EQUAL;
	}
	if (d3dpcmpcaps & STD3D_ZCMP_ALWAYS) {
		result |= STD3D_ZCMP_ALWAYS;
	}
	return result;
}

// FUNCTION: XW 0x4B5000
unsigned int std3D_MapZCmpFunc(unsigned int capsMask) {
	unsigned int result = 0;
	if (capsMask & STD3D_ZCMP_NEVER) {
		result = STD3D_CMP_NEVER;
	}
	if (capsMask & STD3D_ZCMP_EQUAL) {
		result |= STD3D_CMP_EQUAL;
	}
	if (capsMask & STD3D_ZCMP_LESS) {
		result |= STD3D_CMP_LESS;
	}
	if (capsMask & STD3D_ZCMP_LESS_EQUAL) {
		result |= STD3D_CMP_LESS_EQUAL;
	}
	if (capsMask & STD3D_ZCMP_GREATER) {
		result |= STD3D_CMP_GREATER;
	}
	if (capsMask & STD3D_ZCMP_NOT_EQUAL) {
		result |= STD3D_CMP_NOT_EQUAL;
	}
	if (capsMask & STD3D_ZCMP_GREATER_EQUAL) {
		result |= STD3D_CMP_GREATER_EQUAL;
	}
	if (capsMask & STD3D_ZCMP_ALWAYS) {
		result |= STD3D_CMP_ALWAYS;
	}
	return result;
}
