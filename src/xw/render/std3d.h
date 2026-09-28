#ifndef XW_RENDER_STD3D_H
#define XW_RENDER_STD3D_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw_runtime/compat/com_unknown.h"

#include <aeron/compat/d3d.h>
#include <aeron/compat/ddraw.h>
#include <aeron/compat/win_types.h>
#include <stddef.h>
#include <stdint.h>
#include <xw/compiler.h>

typedef struct ColorInfo ColorInfo;
typedef struct Std3DDevice Std3DDevice;
typedef struct Std3DDeviceCaps Std3DDeviceCaps;
typedef struct Std3DDeviceDesc Std3DDeviceDesc;
typedef struct Std3DErrorStringEntry Std3DErrorStringEntry;
typedef struct Std3DRasterInfo Std3DRasterInfo;
typedef struct Std3DRenderState Std3DRenderState;
typedef struct Std3DRenderTri Std3DRenderTri;
typedef struct Std3DSurfaceBlock Std3DSurfaceBlock;
typedef struct Std3DTexCacheNode Std3DTexCacheNode;
typedef struct Std3DTexFmt Std3DTexFmt;
typedef struct Std3DVBuffer Std3DVBuffer;
typedef struct Std3DViewportRect Std3DViewportRect;

enum { STD3D_TEXTURE_BINDING_UNSET = 1 };

enum {
	STD3D_INITIAL_BUFFER_SIZE = 4096,
	STD3D_DEFAULT_EXEC_BUFFER_SIZE = 65536,
	STD3D_MAX_EXEC_VERTICES = 512,
	STD3D_FALLBACK_TEXTURE_SIZE = 32,
	STD3D_INITIAL_STATE_COUNT = 25,
	STD3D_INITIAL_BLEND_STATE_COUNT = 3,
	STD3D_EXEC_BUFFER_SIZE_PRESENT = 1,
	STD3D_SHADE_GOURAUD = 2,
	STD3D_FILL_SOLID = 3,
	STD3D_CULL_NONE = 1
};

enum { STD3D_TEXTURE_FORMAT_CAPACITY = 8, STD3D_PALETTE_BITS = 8, STD3D_PIXEL_FORMAT_PALETTE4 = 0x8 };

enum {
	STD3D_FORMAT_MATCH_NONE = 0,
	STD3D_FORMAT_MATCH_MODE = 1,
	STD3D_FORMAT_MATCH_BPP = 2,
	STD3D_FORMAT_MATCH_ALPHA = 3
};

/* Original IDB size: 72 bytes. */
struct Std3DDeviceCaps {
	/* IDB +0x0: Hardware descriptor selected when its dcmColorModel is nonzero; otherwise software
	 * descriptor. */
	int bHardware;
	/* IDB +0x4: Triangle texture capability bit 1 (perspective correction). */
	int bTexturePerspective;
	/* IDB +0x8: Selected descriptor has nonzero Z-buffer bit-depth mask. */
	int bHasZBuffer;
	/* IDB +0xC: Triangle texture caps bit 8 (color-key support). */
	int bColorKeyTexture;
	/* IDB +0x10: Triangle texture caps bit 4 (alpha texture support). */
	int bAlphaTexture;
	/* IDB +0x14: Shade caps: bit 0x2000 set and bit 0x1000 clear. */
	int bStippledShade;
	/* IDB +0x18: True when (textureBlendCaps&8 and shadeCaps&0x4000), or stippled shading is selected. */
	int bAlphaBlend;
	/* IDB +0x1C: Triangle texture caps bit 0x20 (square-only textures). */
	int bSquareOnlyTexture;
	/* IDB +0x20: Not assigned by enumeration callback; semantic role not established in this pass. */
	int field_20;
	/* IDB +0x24: Low two selected color-model bits (MONO=1, RGB=2). */
	unsigned int colorModelFlags;
	/* IDB +0x28: Compact engine render-depth mask from PackRenderBitDepths. */
	unsigned int renderBitDepthMask;
	/* IDB +0x2C: Compact engine Z-comparison mask from PackZCmpCaps. */
	unsigned int zCmpCapsMask;
	/* IDB +0x30: Hardcoded 1 by enumeration, not descriptor minimum. */
	unsigned int minTextureWidth;
	/* IDB +0x34: Hardcoded 1 by enumeration. */
	unsigned int minTextureHeight;
	/* IDB +0x38: Hardcoded 256 by enumeration, not descriptor maximum. */
	unsigned int maxTextureWidth;
	/* IDB +0x3C: Hardcoded 256 by enumeration. */
	unsigned int maxTextureHeight;
	/* IDB +0x40: Selected descriptor dwMaxBufferSize. */
	unsigned int maxBufferSize;
	/* IDB +0x44: Selected descriptor dwMaxVertexCount. */
	unsigned int maxVertexCount;
};

/* Original IDB size: 204 bytes. */
struct Std3DDeviceDesc {
	/* IDB +0x0 */
	uint32_t dwSize;
	/* IDB +0x4 */
	uint32_t dwFlags;
	/* IDB +0x8 */
	uint32_t dcmColorModel;
	/* IDB +0xC */
	uint32_t dwDevCaps;
	/* IDB +0x10 */
	D3DTRANSFORMCAPS dtcTransformCaps;
	/* IDB +0x18 */
	int32_t bClipping;
	/* IDB +0x1C */
	D3DLIGHTINGCAPS dlcLightingCaps;
	/* IDB +0x2C */
	D3DPRIMCAPS dpcLineCaps;
	/* IDB +0x64 */
	D3DPRIMCAPS dpcTriCaps;
	/* IDB +0x9C */
	uint32_t dwDeviceRenderBitDepth;
	/* IDB +0xA0 */
	uint32_t dwDeviceZBufferBitDepth;
	/* IDB +0xA4 */
	uint32_t dwMaxBufferSize;
	/* IDB +0xA8 */
	uint32_t dwMaxVertexCount;
	/* IDB +0xAC */
	uint32_t dwMinTextureWidth;
	/* IDB +0xB0 */
	uint32_t dwMinTextureHeight;
	/* IDB +0xB4 */
	uint32_t dwMaxTextureWidth;
	/* IDB +0xB8 */
	uint32_t dwMaxTextureHeight;
	/* IDB +0xBC */
	uint32_t dwMinStippleWidth;
	/* IDB +0xC0 */
	uint32_t dwMaxStippleWidth;
	/* IDB +0xC4 */
	uint32_t dwMinStippleHeight;
	/* IDB +0xC8 */
	uint32_t dwMaxStippleHeight;
};

enum { STD3D_ERROR_STRING_COUNT = 121 };

enum { STD3D_DEVICE_CAPACITY = 4 };

enum {
	STD3D_COLOR_MODEL_MONO = 1,
	STD3D_COLOR_MODEL_RGB = 2,
	STD3D_TEXTURE_CAP_PERSPECTIVE = 1,
	STD3D_TEXTURE_CAP_ALPHA = 4,
	STD3D_TEXTURE_CAP_COLOR_KEY = 8,
	STD3D_TEXTURE_CAP_SQUARE_ONLY = 0x20,
	STD3D_SHADE_ALPHA_FLAT_BLEND = 0x1000,
	STD3D_SHADE_ALPHA_FLAT_STIPPLE = 0x2000,
	STD3D_SHADE_ALPHA_GOURAUD_BLEND = 0x4000,
	STD3D_TEXTURE_BLEND_MODULATE_ALPHA = 8,
	STD3D_DEVICE_MIN_TEXTURE_SIZE = 1,
	STD3D_DEVICE_MAX_TEXTURE_SIZE = 256,
	STD3D_ENUM_STOP = 0,
	STD3D_ENUM_CONTINUE = 1
};

enum {
	STD3D_DEVICE_MATCH_NONE = 0,
	STD3D_DEVICE_MATCH_PERSPECTIVE = 1,
	STD3D_DEVICE_MATCH_Z_BUFFER = 2,
	STD3D_DEVICE_MATCH_COLOR_MODEL = 3
};

enum Std3DZCmpCaps {
	STD3D_ZCMP_NEVER = 0x01,
	STD3D_ZCMP_LESS = 0x02,
	STD3D_ZCMP_EQUAL = 0x04,
	STD3D_ZCMP_LESS_EQUAL = 0x08,
	STD3D_ZCMP_GREATER = 0x10,
	STD3D_ZCMP_NOT_EQUAL = 0x20,
	STD3D_ZCMP_GREATER_EQUAL = 0x40,
	STD3D_ZCMP_ALWAYS = 0x80
};

enum Std3DComparisonFunction {
	STD3D_CMP_NEVER = 1,
	STD3D_CMP_LESS = 2,
	STD3D_CMP_EQUAL = 3,
	STD3D_CMP_LESS_EQUAL = 4,
	STD3D_CMP_GREATER = 5,
	STD3D_CMP_NOT_EQUAL = 6,
	STD3D_CMP_GREATER_EQUAL = 7,
	STD3D_CMP_ALWAYS = 8
};

enum Std3DErrorCode {
	STD3D_D3D_OK = 0,
	STD3D_D3DERR_BADMAJORVERSION = -2005531972,
	STD3D_D3DERR_BADMINORVERSION = -2005531971,
	STD3D_D3DERR_EXECUTE_DESTROY_FAILED = -2005531961,
	STD3D_D3DERR_EXECUTE_LOCK_FAILED = -2005531960,
	STD3D_D3DERR_EXECUTE_UNLOCK_FAILED = -2005531959,
	STD3D_D3DERR_EXECUTE_LOCKED = -2005531958,
	STD3D_D3DERR_EXECUTE_NOT_LOCKED = -2005531957,
	STD3D_D3DERR_EXECUTE_CLIPPED_FAILED = -2005531955,
	STD3D_D3DERR_TEXTURE_CREATE_FAILED = -2005531951,
	STD3D_D3DERR_TEXTURE_DESTROY_FAILED = -2005531950,
	STD3D_D3DERR_TEXTURE_LOCK_FAILED = -2005531949,
	STD3D_D3DERR_TEXTURE_UNLOCK_FAILED = -2005531948,
	STD3D_D3DERR_TEXTURE_LOAD_FAILED = -2005531947,
	STD3D_D3DERR_TEXTURE_SWAP_FAILED = -2005531946,
	STD3D_D3DERR_TEXTURE_LOCKED = -2005531945,
	STD3D_D3DERR_TEXTURE_NOT_LOCKED = -2005531944,
	STD3D_D3DERR_TEXTURE_GETSURF_FAILED = -2005531943,
	STD3D_D3DERR_MATRIX_DESTROY_FAILED = -2005531941,
	STD3D_D3DERR_MATRIX_SETDATA_FAILED = -2005531940,
	STD3D_D3DERR_MATRIX_GETDATA_FAILED = -2005531939,
	STD3D_D3DERR_SETVIEWPORTDATA_FAILED = -2005531938,
	STD3D_D3DERR_MATERIAL_DESTROY_FAILED = -2005531931,
	STD3D_D3DERR_MATERIAL_SETDATA_FAILED = -2005531930,
	STD3D_D3DERR_MATERIAL_GETDATA_FAILED = -2005531929,
	STD3D_D3DERR_SCENE_NOT_IN_SCENE = -2005531911,
	STD3D_D3DERR_SCENE_BEGIN_FAILED = -2005531910,
	STD3D_D3DERR_SCENE_END_FAILED = -2005531909,
	STD3D_DD_OK = 0,
	STD3D_DDERR_ALREADYINITIALIZED = -2005532667,
	STD3D_DDERR_CANNOTATTACHSURFACE = -2005532662,
	STD3D_DDERR_CANNOTDETACHSURFACE = -2005532652,
	STD3D_DDERR_CURRENTLYNOTAVAIL = -2005532632,
	STD3D_DDERR_EXCEPTION = -2005532617,
	STD3D_DDERR_GENERIC = -2147467259,
	STD3D_DDERR_HEIGHTALIGN = -2005532582,
	STD3D_DDERR_INCOMPATIBLEPRIMARY = -2005532577,
	STD3D_DDERR_INVALIDCAPS = -2005532572,
	STD3D_DDERR_INVALIDCLIPLIST = -2005532562,
	STD3D_DDERR_INVALIDMODE = -2005532552,
	STD3D_DDERR_INVALIDOBJECT = -2005532542,
	STD3D_DDERR_INVALIDPARAMS = -2147024809,
	STD3D_DDERR_INVALIDPIXELFORMAT = -2005532527,
	STD3D_DDERR_INVALIDRECT = -2005532522,
	STD3D_DDERR_LOCKEDSURFACES = -2005532512,
	STD3D_DDERR_NO3D = -2005532502,
	STD3D_DDERR_NOALPHAHW = -2005532492,
	STD3D_DDERR_NOCLIPLIST = -2005532467,
	STD3D_DDERR_NOCOLORCONVHW = -2005532462,
	STD3D_DDERR_NOCOOPERATIVELEVELSET = -2005532460,
	STD3D_DDERR_NOCOLORKEY = -2005532457,
	STD3D_DDERR_NOCOLORKEYHW = -2005532452,
	STD3D_DDERR_NODIRECTDRAWSUPPORT = -2005532450,
	STD3D_DDERR_NOEXCLUSIVEMODE = -2005532447,
	STD3D_DDERR_NOFLIPHW = -2005532442,
	STD3D_DDERR_NOGDI = -2005532432,
	STD3D_DDERR_NOMIRRORHW = -2005532422,
	STD3D_DDERR_NOTFOUND = -2005532417,
	STD3D_DDERR_NOOVERLAYHW = -2005532412,
	STD3D_DDERR_NORASTEROPHW = -2005532392,
	STD3D_DDERR_NOROTATIONHW = -2005532382,
	STD3D_DDERR_NOSTRETCHHW = -2005532362,
	STD3D_DDERR_NOT4BITCOLOR = -2005532356,
	STD3D_DDERR_NOT4BITCOLORINDEX = -2005532355,
	STD3D_DDERR_NOT8BITCOLOR = -2005532352,
	STD3D_DDERR_NOTEXTUREHW = -2005532342,
	STD3D_DDERR_NOVSYNCHW = -2005532337,
	STD3D_DDERR_NOZBUFFERHW = -2005532332,
	STD3D_DDERR_NOZOVERLAYHW = -2005532322,
	STD3D_DDERR_OUTOFCAPS = -2005532312,
	STD3D_DDERR_OUTOFMEMORY = -2147024882,
	STD3D_DDERR_OUTOFVIDEOMEMORY = -2005532292,
	STD3D_DDERR_OVERLAYCANTCLIP = -2005532290,
	STD3D_DDERR_OVERLAYCOLORKEYONLYONEACTIVE = -2005532288,
	STD3D_DDERR_PALETTEBUSY = -2005532285,
	STD3D_DDERR_COLORKEYNOTSET = -2005532272,
	STD3D_DDERR_SURFACEALREADYATTACHED = -2005532262,
	STD3D_DDERR_SURFACEALREADYDEPENDENT = -2005532252,
	STD3D_DDERR_SURFACEBUSY = -2005532242,
	STD3D_DDERR_SURFACEISOBSCURED = -2005532232,
	STD3D_DDERR_SURFACELOST = -2005532222,
	STD3D_DDERR_SURFACENOTATTACHED = -2005532212,
	STD3D_DDERR_TOOBIGHEIGHT = -2005532202,
	STD3D_DDERR_TOOBIGSIZE = -2005532192,
	STD3D_DDERR_TOOBIGWIDTH = -2005532182,
	STD3D_DDERR_UNSUPPORTED = -2147467263,
	STD3D_DDERR_UNSUPPORTEDFORMAT = -2005532162,
	STD3D_DDERR_UNSUPPORTEDMASK = -2005532152,
	STD3D_DDERR_VERTICALBLANKINPROGRESS = -2005532135,
	STD3D_DDERR_WASSTILLDRAWING = -2005532132,
	STD3D_DDERR_XALIGN = -2005532112,
	STD3D_DDERR_INVALIDDIRECTDRAWGUID = -2005532111,
	STD3D_DDERR_DIRECTDRAWALREADYCREATED = -2005532110,
	STD3D_DDERR_NODIRECTDRAWHW = -2005532109,
	STD3D_DDERR_PRIMARYSURFACEALREADYEXISTS = -2005532108,
	STD3D_DDERR_NOEMULATION = -2005532107,
	STD3D_DDERR_REGIONTOOSMALL = -2005532106,
	STD3D_DDERR_CLIPPERISUSINGHWND = -2005532105,
	STD3D_DDERR_NOCLIPPERATTACHED = -2005532104,
	STD3D_DDERR_NOHWND = -2005532103,
	STD3D_DDERR_HWNDSUBCLASSED = -2005532102,
	STD3D_DDERR_HWNDALREADYSET = -2005532101,
	STD3D_DDERR_NOPALETTEATTACHED = -2005532100,
	STD3D_DDERR_NOPALETTEHW = -2005532099,
	STD3D_DDERR_BLTFASTCANTCLIP = -2005532098,
	STD3D_DDERR_NOBLTHW = -2005532097,
	STD3D_DDERR_NODDROPSHW = -2005532096,
	STD3D_DDERR_OVERLAYNOTVISIBLE = -2005532095,
	STD3D_DDERR_NOOVERLAYDEST = -2005532094,
	STD3D_DDERR_INVALIDPOSITION = -2005532093,
	STD3D_DDERR_NOTAOVERLAYSURFACE = -2005532092,
	STD3D_DDERR_EXCLUSIVEMODEALREADYSET = -2005532091,
	STD3D_DDERR_NOTFLIPPABLE = -2005532090,
	STD3D_DDERR_CANTDUPLICATE = -2005532089,
	STD3D_DDERR_NOTLOCKED = -2005532088,
	STD3D_DDERR_CANTCREATEDC = -2005532087,
	STD3D_DDERR_NODC = -2005532086,
	STD3D_DDERR_WRONGMODE = -2005532085,
	STD3D_DDERR_IMPLICITLYCREATED = -2005532084,
	STD3D_DDERR_NOTPALETTIZED = -2005532083,
	STD3D_DDERR_UNSUPPORTEDMODE = -2005532082,
};

/* Original IDB size: 8 bytes. */
struct Std3DErrorStringEntry {
	/* IDB +0x0 */
	HRESULT code;
	/* IDB +0x4 */
	const char* message;
};

typedef int32_t Std3DLegacyRenderStateType;

enum Std3DLegacyRenderStateTypeValues {
	STD3D_STATE_TEXTUREHANDLE = 0x1,
	STD3D_STATE_ANTIALIAS = 0x2,
	STD3D_STATE_TEXTUREADDRESS = 0x3,
	STD3D_STATE_TEXTUREPERSPECTIVE = 0x4,
	STD3D_STATE_WRAPU = 0x5,
	STD3D_STATE_WRAPV = 0x6,
	STD3D_STATE_ZENABLE = 0x7,
	STD3D_STATE_FILLMODE = 0x8,
	STD3D_STATE_SHADEMODE = 0x9,
	STD3D_STATE_MONOENABLE = 0xB,
	STD3D_STATE_ZWRITEENABLE = 0xE,
	STD3D_STATE_ALPHATESTENABLE = 0xF,
	STD3D_STATE_TEXTUREMAG = 0x11,
	STD3D_STATE_TEXTUREMIN = 0x12,
	STD3D_STATE_SRCBLEND = 0x13,
	STD3D_STATE_DESTBLEND = 0x14,
	STD3D_STATE_TEXTUREMAPBLEND = 0x15,
	STD3D_STATE_CULLMODE = 0x16,
	STD3D_STATE_ZFUNC = 0x17,
	STD3D_STATE_ALPHAFUNC = 0x19,
	STD3D_STATE_DITHERENABLE = 0x1A,
	STD3D_STATE_BLENDENABLE = 0x1B,
	STD3D_STATE_FOGENABLE = 0x1C,
	STD3D_STATE_SPECULARENABLE = 0x1D,
	STD3D_STATE_SUBPIXEL = 0x1F,
	STD3D_STATE_SUBPIXELX = 0x20,
	STD3D_STATE_STIPPLEDALPHA = 0x21,
	STD3D_STATE_FOGCOLOR = 0x22,
	STD3D_STATE_FOGTABLEMODE = 0x23,
	STD3D_STATE_FOGTABLESTART = 0x24,
	STD3D_STATE_FOGTABLEEND = 0x25
};

enum {
	STD3D_BLEND_ZERO = 1,
	STD3D_BLEND_ONE = 2,
	STD3D_BLEND_SRC_ALPHA = 5,
	STD3D_BLEND_INV_SRC_ALPHA = 6,
	STD3D_MAP_BLEND_MODULATE = 2,
	STD3D_MAP_BLEND_MODULATE_ALPHA = 4,
	STD3D_COMPARE_ALWAYS = 8,
	STD3D_FILTER_NEAREST = 1,
	STD3D_FILTER_LINEAR = 2,
	STD3D_FOG_LINEAR = 3,
	STD3D_FOG_GREEN_SHIFT = 8,
	STD3D_FOG_RED_SHIFT = 16,
	STD3D_BLEND_STATE_COUNT = 4,
	STD3D_DEPTH_STATE_COUNT = 2,
	STD3D_FILTER_STATE_COUNT = 2,
	STD3D_FOG_STATE_COUNT = 5
};

typedef int32_t Std3DRenderStateFlags;

enum Std3DRenderStateFlagsValues {
	STD3D_RS_TEXTURE_PERSPECTIVE = 0x1,
	STD3D_RS_DITHER = 0x2,
	STD3D_RS_SPECULAR = 0x4,
	STD3D_RS_ANTIALIAS = 0x8,
	STD3D_RS_SUBPIXEL = 0x10,
	STD3D_RS_SUBPIXEL_X = 0x20,
	STD3D_RS_FOG = 0x40,
	STD3D_RS_MAG_LINEAR = 0x80,
	STD3D_RS_MIN_LINEAR = 0x100,
	STD3D_RS_ALPHA_BLEND = 0x200,
	STD3D_RS_MODULATE_ALPHA = 0x400,
	STD3D_RS_Z_TEST = 0x800,
	STD3D_RS_Z_WRITE = 0x1000,
	STD3D_RS_DISABLE_MONO = 0x8000
};

typedef struct Std3DRasterInfo Std3DRenderTargetDesc;

/* Original IDB size: 20 bytes. */
struct Std3DRenderTri {
	/* IDB +0x0 */
	int v0;
	/* IDB +0x4 */
	int v1;
	/* IDB +0x8 */
	int v2;
	/* IDB +0xC: Engine render-state flags passed to std3D_SetRenderState; adjacent triangles batch only when
	 * flags and texture match. */
	unsigned int flags;
	/* IDB +0x10: Optional texture cache entry; submission reads texHandle at +116. NULL selects untextured
	 * state. */
	struct Std3DTexCacheNode* texture;
};

/* Original IDB size: 116 bytes. */
struct Std3DSurfaceBlock {
	/* IDB +0x0: DirectDraw surface; Release used by FreeVBuffer or owner cleanup. */
	IDirectDrawSurface* surface;
	/* IDB +0x4: Unknown/reserved word; no semantic evidence yet. */
	unsigned int field_4;
	/* IDB +0x8: Legacy 108-byte DirectDraw descriptor at +8; Z creation uses dwZBufferBitDepth union at
	 * descriptor+24. */
	DDSURFACEDESC desc;
};

/* Original IDB size: 152 bytes. */
struct Std3DTexCacheNode {
	/* IDB +0x0: Direct3D texture interface created by QueryInterface and loaded in CreateMipSurface. */
	IDirect3DTexture* pCachedTexture;
	/* IDB +0x4: Cached upload destination surface; released before texture interface during eviction/flush.
	 */
	IDirectDrawSurface* pCachedSurface;
	/* IDB +0x8: 108-byte description copied from source surface during CreateMipSurface. */
	DDSURFACEDESC ddsd;
	/* IDB +0x74: Handle obtained from texture GetHandle for the current Direct3D device. */
	unsigned int texHandle;
	/* IDB +0x78: Nonzero when populated/resident. Lookup uses zero as free; initialization clears only this
	 * field. Flush/eviction clear it. */
	int bCached;
	/* IDB +0x7C: Nonzero when CreateMipSurface selected an alpha texture format. */
	int usesAlphaFormat;
	/* IDB +0x80: Actual texture width after size constraints/tiling. */
	unsigned int width;
	/* IDB +0x84: Actual texture height after size constraints/tiling. */
	unsigned int height;
	/* IDB +0x88: Stored as width*height (not multiplied by bytes per pixel). Append/Remove debit/credit
	 * device field +332 by this value; eviction accumulates it toward requested pixel area. */
	unsigned int pixelCount;
	/* IDB +0x8C: Current global texture-frame tag at upload/use; current-tag entries stop eviction scan. */
	unsigned int cacheFrameTag;
	/* IDB +0x90: Previous node in cache list; Remove does not clear detached links. */
	struct Std3DTexCacheNode* pPrev;
	/* IDB +0x94: Next node; head is eviction candidate, tail is most recently uploaded/used. */
	struct Std3DTexCacheNode* pNext;
};

/* Original IDB size: 16 bytes. */
struct Std3DViewportRect {
	/* IDB +0x0 */
	int x;
	/* IDB +0x4 */
	int y;
	/* IDB +0x8 */
	int width;
	/* IDB +0xC */
	int height;
};

typedef int32_t StdColorMode;

enum StdColorModeValues { STDCOLOR_PAL = 0x0, STDCOLOR_RGB = 0x1, STDCOLOR_RGBA = 0x2 };

enum {
	STDCOLOR_RGB555_CHANNEL_BITS = 5,
	STDCOLOR_RGBA1555_ALPHA_BITS = 1,
	STDCOLOR_RGBA4444_CHANNEL_BITS = 4,
	STDCOLOR_RGB565_RED_BITS = 5,
	STDCOLOR_RGB565_GREEN_BITS = 6,
	STDCOLOR_RGB565_BLUE_BITS = 5
};

typedef D3DEnumDevicesCb XwD3DEnumDevicesCallback;

typedef int(AERON_DXAPI* XwD3DEnumTextureFormatsCallback)(DDSURFACEDESC*, void*);

/* Original IDB size: 56 bytes. */
struct ColorInfo {
	/* IDB +0x0: Palette, RGB or RGBA mode as selected by texture enumeration. */
	StdColorMode colorMode;
	/* IDB +0x4: Total bits per pixel. */
	int bpp;
	/* IDB +0x8 */
	int redBPP;
	/* IDB +0xC */
	int greenBPP;
	/* IDB +0x10 */
	int blueBPP;
	/* IDB +0x14 */
	int redPosShift;
	/* IDB +0x18 */
	int greenPosShift;
	/* IDB +0x1C */
	int bluePosShift;
	/* IDB +0x20 */
	int redPosShiftRight;
	/* IDB +0x24 */
	int greenPosShiftRight;
	/* IDB +0x28 */
	int bluePosShiftRight;
	/* IDB +0x2C */
	int alphaBPP;
	/* IDB +0x30 */
	int alphaPosShift;
	/* IDB +0x34 */
	int alphaPosShiftRight;
};

/* Original IDB size: 556 bytes. */
struct Std3DDevice {
	/* IDB +0x0 */
	struct Std3DDeviceCaps caps;
	/* IDB +0x48: 128-byte strncpy copy of callback deviceName; no explicit terminator if source is >=128
	 * bytes. */
	char deviceName[128];
	/* IDB +0xC8: 128-byte strncpy copy of callback description; no explicit terminator. */
	char deviceDescription[128];
	/* IDB +0x148: Total texture memory from DirectDraw2 GetAvailableVidMem, in bytes; queried for selected
	 * device. */
	uint32_t totalMemory;
	/* IDB +0x14C: Initially free bytes from GetAvailableVidMem. Cache Append/Remove subsequently debit/credit
	 * pixelCount (width*height, no 2-byte factor); Flush resets to totalMemory. */
	uint32_t availableMemory;
	/* IDB +0x150: 204-byte descriptor copied from hardware or software callback argument; distinct from later
	 * 252-byte SDK layout. */
	struct Std3DDeviceDesc d3dDesc;
	/* IDB +0x21C: 16-byte enumerated device GUID, used for render-surface QueryInterface in CreateDevice. */
	DxGuid guid;
};

/* Original IDB size: 8 bytes. */
struct Std3DRenderState {
	/* IDB +0x0: Legacy Direct3D render-state token in an opcode-8 STATERENDER payload. */
	Std3DLegacyRenderStateType state;
	/* IDB +0x4: Raw 32-bit value: boolean, enum, handle, RGB color or float bits according to token. */
	unsigned int value;
};

/* Original IDB size: 76 bytes. */
struct Std3DRasterInfo {
	/* IDB +0x0 */
	unsigned int width;
	/* IDB +0x4 */
	unsigned int height;
	/* IDB +0x8: Copied metadata; AllocVBuffer recalculates pitch only, leaving this value unchanged. */
	unsigned int sizeBytes;
	/* IDB +0xC: Bytes per row. InitRenderTargetDesc accepts it; AllocVBuffer sets width*(unsigned bpp>>3);
	 * surface lock replaces it with lPitch. */
	int pitch;
	/* IDB +0x10: Pitch in 16-bit pixels in InitRenderTargetDesc (signed pitch/2); otherwise copied metadata.
	 * Not refreshed by AllocVBuffer or LockVBuffer. */
	unsigned int widthPixels;
	/* IDB +0x14 */
	struct ColorInfo colorInfo;
};

/* Original IDB size: 164 bytes. */
struct Std3DTexFmt {
	/* IDB +0x0: Channel masks summarized as widths, bit positions and conversion shifts. */
	struct ColorInfo colorInfo;
	/* IDB +0x38: Original 108-byte descriptor copied from the Direct3D enumeration callback. */
	DDSURFACEDESC ddsd;
};

enum { STD3D_STORAGE_SOFTWARE = 0, STD3D_STORAGE_SURFACE = 1 };

enum {
	STD3D_DESC_ZBUFFER_BIT_DEPTH = 0x40,
	STD3D_DEPTH_MASK_32 = 0x100,
	STD3D_DEPTH_MASK_16 = 0x400,
	STD3D_DEPTH_MASK_8 = 0x800
};

/* Original IDB size: 216 bytes. */
struct Std3DVBuffer {
	/* IDB +0x0: 1 selects DirectDraw surface storage; all other values take software/free(pixels) path.
	 * AllocVBuffer initializes 0. */
	int storageType;
	/* IDB +0x4: Nested lock count. First surface lock publishes pixels/pitch; last successful unlock
	 * decrements to zero. Failures leave count unchanged. */
	int lockCount;
	/* IDB +0x8: Z-buffer creator sets this when actual surface caps include DDSCAPS_VIDEOMEMORY. */
	int bVideoMemory;
	/* IDB +0xC */
	struct Std3DRasterInfo raster;
	/* IDB +0x58: Unknown word; zeroed by Z-buffer creation and whole-record allocation. */
	unsigned int field_58;
	/* IDB +0x5C: Owned malloc pixels for software buffers; borrowed Lock lpSurface for DirectDraw storage.
	 * Unlock does not clear it. */
	uint8_t* pixels;
	/* IDB +0x60: Packed RGB source color key used when the device lacks alpha textures. */
	unsigned int transparentColor;
	/* IDB +0x64: Embedded surface block begins at +100; full video-buffer allocation/clear size is 216 bytes.
	 */
	struct Std3DSurfaceBlock surfaceBlock;
};

extern const DxGuid g_directDraw2InterfaceGuid;
extern const DxGuid IID_IDirect3D;
extern const double g_textureTileCountRoundingBias;
extern unsigned int g_std3DCapFlags;
extern Std3DRenderStateFlags g_d3dStateFlags;
extern unsigned int g_std3DNumDevices;
extern Std3DDevice* g_pStd3DCurDevice;
extern unsigned int g_std3DNumTextureFormats;
extern Std3DTexFmt* g_pFmtRGB565;
extern Std3DTexFmt* g_pFmtRGBA1555;
extern Std3DTexFmt* g_pFmtRGBA4444;
extern int g_std3DMinTextureWidth;
extern int g_std3DMinTextureHeight;
extern unsigned int g_std3DExecBufMaxVerts;
extern unsigned int g_std3DTextureFrameTag;
extern int g_texCacheCount;
extern Std3DTexCacheNode* g_pTexCacheHead;
extern Std3DTexCacheNode* g_pTexCacheTail;
extern Std3DVBuffer* g_pStd3DVBuffer;
extern IDirect3DExecuteBuffer* g_d3dExecuteBuffer;
extern unsigned int g_std3DExecBufSize;
extern unsigned int g_d3dBufVertCount;
extern unsigned int g_std3DExecBufTriCount;
extern Std3DTexCacheNode* g_d3dCurTexture;
extern Std3DSurfaceBlock* g_pStd3DZBufferState;
extern int g_std3DZBufferEnabled;
extern IDirectDraw* g_std3DDirectDraw;
extern int g_std3DStartupDone;
extern int g_std3DDeviceOpen;
extern const Std3DErrorStringEntry g_std3DErrorStringTable[STD3D_ERROR_STRING_COUNT];
extern const char g_std3DUnknownErrorMessage[];

enum { STD3D_PALETTE_COLOR_COUNT = 256 };

extern uint16_t g_texConvBuf4444[STD3D_PALETTE_COLOR_COUNT];
extern unsigned int g_std3DFogTableEndBits;
extern unsigned int g_std3DFogColorBlue8;
extern Std3DRenderTargetDesc g_std3DRenderTargetDesc;
extern unsigned int g_std3DFogTableStartBits;
extern IDirect3DDevice* g_d3dDevice;
extern uint16_t g_texConvBuf1555[STD3D_PALETTE_COLOR_COUNT];
extern IDirect3DViewport* g_d3dViewport;
extern unsigned int g_std3DFogColorGreen8;
extern IUnknown* g_d3dViewportMaterial;
extern unsigned int g_std3DFogColorRed8;
extern uint8_t* g_d3dExecBufBase;
extern D3DEXECUTEBUFFERDESC g_d3dExecBufDesc;
extern uint16_t g_std3DPaletteScratch16[STD3D_PALETTE_COLOR_COUNT];
extern IDirect3D* g_lpD3D;
extern uint8_t* g_d3dWritePtr;
extern uint8_t* g_d3dInstrStart;
extern IDirectDrawSurface* g_std3DZBufferSurface;
extern int g_fmtIdxRGBA4444;
extern unsigned int g_std3DCurDeviceIdx;
extern float g_std3DColorOverlayRed;
extern float g_std3DColorOverlayGreen;
extern float g_std3DColorOverlayBlue;
extern int32_t g_std3DColorOverlayEnabled;
extern unsigned int g_std3DZCmpMask;
extern Std3DRenderTri g_std3DViewportQuadTriangles[2];
extern int g_fmtIdxRGB565;
extern D3DTLVERTEX g_std3DQuadVerts[4];
extern Std3DRenderTargetDesc* g_pStd3DRenderTarget;
extern Std3DViewportRect g_std3DQuadRect;
extern Std3DTexFmt g_std3DTextureFormats[STD3D_TEXTURE_FORMAT_CAPACITY];
extern Std3DDevice g_std3DDevices[STD3D_DEVICE_CAPACITY];
extern Std3DVBuffer g_std3DZBufferVBuffer;
extern IDirectDrawSurface* g_std3DRenderSurface;

extern int g_fmtIdxRGBA1555;

/* Declarations follow ascending original IDB address. */

/* 0x482A80 */
void std3D_DetachAndReleaseZBufferSurface(void);

/* 0x4B1AF0 */
void std3D_CopyPaletteToScratch16(const uint16_t* palette, int colorCount);

/* 0x4B1B20 */
void std3D_ConvertTexTo1555(const uint16_t* sourcePixels, int pixelCount);

/* 0x4B1C30 */
int std3D_Startup(void);

/* 0x4B1D70 */
void std3D_InitRenderTargetDesc(unsigned int width, unsigned int height, int pitch);

/* 0x4B1E70 */
void std3D_Shutdown(void);

/* 0x4B1EB0 */
int std3D_CreateDevice(unsigned int deviceIdx, int bUseZBuffer);

/* 0x4B2420 */
void std3D_SetRenderSurface(IDirectDrawSurface* surface);

/* 0x4B2430 */
void std3D_SetColorOverlayParams(float red, float green, float blue, int32_t enabled);

/* 0x4B2460 */
int std3D_Log2Floor(int value);

/* 0x4B2480 */
const char* std3D_LookupErrorString(HRESULT errorCode, const struct Std3DErrorStringEntry* entries,
									int entryCount);

/* 0x4B24C0 */
struct Std3DVBuffer* std3D_AllocVBuffer(const struct Std3DRasterInfo* raster, int unused1, int unused2,
										int unused3);

/* 0x4B2520 */
void std3D_FreeVBuffer(struct Std3DVBuffer* vbuffer);

/* 0x4B2560 */
void std3D_LockVBuffer(struct Std3DVBuffer* vbuffer);

/* 0x4B25E0 */
void std3D_UnlockVBuffer(struct Std3DVBuffer* vbuffer);

/* 0x4B2640 */
void std3D_Close(void);

/* 0x4B2730 */
void std3D_BlitVBuffer(struct Std3DVBuffer* destination, struct Std3DVBuffer* source, int destinationX,
					   int destinationY);

/* 0x4B27D0 */
void std3D_StartScene(void);

/* 0x4B2810 */
void std3D_EndScene(void);

/* 0x4B2850 */
int std3D_LockExecuteBuffer(void);

/* 0x4B28D0 */
int std3D_AddVertices(const D3DTLVERTEX* vertices, unsigned int count);

/* 0x4B2930 */
int std3D_BeginInstructions(void);

/* 0x4B29A0 */
int std3D_AddTriangles(const struct Std3DRenderTri* triangles, unsigned int count);

/* 0x4B2BE0 */
int std3D_ExecuteBuffer(void);

/* 0x4B2CF0 */
void std3D_SetRenderState(Std3DRenderStateFlags flags);

/* 0x4B3100 */
int std3D_CreateMipSurface(struct Std3DVBuffer* sourceBuffer, struct Std3DTexCacheNode* cacheEntry,
						   int useTransparency, int useFourBitAlpha);

/* 0x4B3AF0 */
void std3D_FlushTextureCache(void);

/* 0x4B3B70 */
void std3D_CacheListAppend(struct Std3DTexCacheNode* node);

/* 0x4B3BE0 */
void std3D_CacheListRemove(struct Std3DTexCacheNode* node);

/* 0x4B3C90 */
int std3D_QueryTextureVidMem(uint32_t* totalBytes, uint32_t* freeBytes);

/* 0x4B3D00 */
void std3D_CacheTextureSurface(struct Std3DTexCacheNode* node);

/* 0x4B3D30 */
int std3D_ClearZBuffer(void);

/* 0x4B3E00 */
int std3D_SelectBestDevice(const struct Std3DDeviceCaps* requested);

/* 0x4B3ED0 */
int std3D_FindClosestFormat(const struct ColorInfo* requested, const struct Std3DTexFmt* formats,
							unsigned int formatCount);

/* 0x4B4010 */
void std3D_BuildViewportQuad(const struct Std3DViewportRect* rect);

/* 0x4B4100 */
int std3D_SetInitialRenderState(void);

/* 0x4B45B0 */
int std3D_CreateViewport(unsigned int width, unsigned int height);

/* 0x4B4740 */
int std3D_CreateZBuffer(unsigned int width, unsigned int height);

/* 0x4B4990 */
HRESULT AERON_DXAPI std3D_EnumDevicesCallback(DxGuid* deviceGuid, char* description, char* deviceName,
											  D3DDEVICEDESC* hardwareDesc, D3DDEVICEDESC* softwareDesc,
											  void* context);

/* 0x4B4C10 */
int AERON_DXAPI std3D_EnumTextureFormats(DDSURFACEDESC* surfaceDesc, void* context);

/* 0x4B4F60 */
unsigned int std3D_PackRenderBitDepths(unsigned int ddbdFlags);

/* 0x4B4FB0 */
unsigned int std3D_PackZCmpCaps(unsigned int d3dpcmpcaps);

/* 0x4B5000 */
unsigned int std3D_MapZCmpFunc(unsigned int capsMask);

#ifdef __cplusplus
}
#endif

#endif
