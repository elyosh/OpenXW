#ifndef XW_RENDER_POWER_VR_H
#define XW_RENDER_POWER_VR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <aeron/compat/win_types.h>
#include <stddef.h>
#include <stdint.h>
#include <xw/compiler.h>
#include <xw/render/std3d.h>

struct IUnknown;

extern const DxGuid g_iidDirect3DTexture;

enum {
	POWERVR_STAGE_PCI_DEVICE = 1,
	POWERVR_STAGE_HAL_RESOURCE = 2,
	POWERVR_STAGE_COMPLETE = 5,
	POWERVR_VENDOR_NEC = 0x1033,
	POWERVR_DEVICE_61 = 0x61,
	POWERVR_DEVICE_46 = 0x46,
	POWERVR_DEVICE_2A = 0x2A,
	POWERVR_DEVICE_1F = 0x1F,
	POWERVR_STAGE_DIRECTDRAW2 = 3,
	POWERVR_STAGE_HARDWARE_DEVICE = 4,
	POWERVR_PROBE_SURFACE_SIZE = 2,
	POWERVR_PIXEL_FORMAT_FOURCC = 4,
	POWERVR_PVRC_FOURCC = 0x43525650,
	POWERVR_TEXTURE_RESERVED_WORD = 1
};

enum {
	POWERVR_PROBE_DEVICE_CAPACITY = 10,
	POWERVR_ENUM_CONTINUE = 1,
	POWERVR_KEYBOARD_TYPE = 0,
	POWERVR_KEYBOARD_SUBTYPE = 1,
	POWERVR_KEYBOARD_JAPANESE = 7,
	POWERVR_PC98_SUBTYPE_LOWER_BOUND = 0xD00,
	POWERVR_PC98_SUBTYPE_UPPER_BOUND = 0xD08,
	POWERVR_VXD_DELETE_ON_CLOSE = 0x04000000,
	POWERVR_GET_PHYSICAL_BOARD_INFO = 0x1B,
	POWERVR_GP_PORT_OFFSET = 0x58,
	POWERVR_GP_PORT_SETTLE_MS = 10,
	POWERVR_MATROX_SIGNATURE = 0xF6F6,
	POWERVR_DEBUG_TEXT_CAPACITY = 300,
	POWERVR_VERSION_FILENAME_CAPACITY = 72,
	POWERVR_GLOBAL_MOVEABLE = 2,
	POWERVR_VERSION_COMPONENT_SHIFT = 16
};

extern const char g_powerVrHalDllFilename[];

typedef struct PowerVrD3DInterface PowerVrD3DInterface;
typedef struct PowerVrD3DVtblPrefix PowerVrD3DVtblPrefix;
typedef struct PowerVrPhysicalBoardInfo PowerVrPhysicalBoardInfo;
typedef struct PowerVrProbeDevice PowerVrProbeDevice;
typedef struct PowerVrProbeInfoPrefix PowerVrProbeInfoPrefix;
typedef struct PowerVrTextureInterface PowerVrTextureInterface;
typedef struct PowerVrTextureVtblPrefix PowerVrTextureVtblPrefix;

/* Original IDB size: 4 bytes. */
struct PowerVrD3DInterface {
	/* IDB +0x0 */
	struct PowerVrD3DVtblPrefix* lpVtbl;
};

/* Original IDB size: 164 bytes. */
struct PowerVrPhysicalBoardInfo {
	/* IDB +0x0 */
	uint8_t gap00[84];
	/* IDB +0x54: Pointer returned by VSGL.VXD IOCTL 0x1B at offset 84. Probe accesses the dword register at
	 * base+0x58. */
	uint8_t* registerBase;
	/* IDB +0x58 */
	uint8_t gap58[76];
};

/* Original IDB size: 529 bytes. */
struct PowerVrProbeDevice {
	/* IDB +0x0 */
	int isHardware;
	/* IDB +0x4: Same 204-byte Direct3D hardware/software descriptor layout used by Std3DDevice.d3dDesc;
	 * selected by dcmColorModel. */
	struct Std3DDeviceDesc caps;
	/* IDB +0xD0 */
	char name[50];
	/* IDB +0x102 */
	char description[255];
	/* IDB +0x201 */
	DxGuid guid;
};

typedef HRESULT(AERON_DXAPI* PowerVrProbeEnumCallback)(DxGuid* deviceGuid, char* description, char* name,
													   struct Std3DDeviceDesc* hardwareDesc,
													   struct Std3DDeviceDesc* softwareDesc, void* context);

/* Original IDB size: 92 bytes. */
struct PowerVrProbeInfoPrefix {
	/* IDB +0x0: Progress stage: 1 PCI device, 2 HAL resource, 3 DirectDraw2, 4 hardware 3D candidate, 5
	 * completed probe. Failure leaves the last written stage. */
	unsigned int stage;
	/* IDB +0x4 */
	unsigned int deviceId;
	/* IDB +0x8 */
	unsigned int vendorId;
	/* IDB +0xC: Eight bytes model the observed Generic/Matrox strings only; the full label capacity is not
	 * established. */
	char boardLabel[8];
	/* IDB +0x14: Unknown bytes before fixed version fields. FileVersion text starts immediately AFTER this
	 * 92-byte prefix; total output size is unproven. */
	uint8_t gap14[56];
	/* IDB +0x4C */
	unsigned int halVersionMajor;
	/* IDB +0x50 */
	unsigned int halVersionMinor;
	/* IDB +0x54 */
	unsigned int halVersionBuild;
	/* IDB +0x58 */
	unsigned int halVersionRevision;
};

/* Original IDB size: 4 bytes. */
struct PowerVrTextureInterface {
	/* IDB +0x0 */
	struct PowerVrTextureVtblPrefix* lpVtbl;
};

/* Original IDB size: 20 bytes. */
struct PowerVrTextureVtblPrefix {
	/* IDB +0x0 */
	HRESULT(AERON_DXAPI* QueryInterface)(struct PowerVrTextureInterface*, const DxGuid*, void**);
	/* IDB +0x4 */
	uint32_t(AERON_DXAPI* AddRef)(struct PowerVrTextureInterface*);
	/* IDB +0x8 */
	uint32_t(AERON_DXAPI* Release)(struct PowerVrTextureInterface*);
	/* IDB +0xC: Unmodeled method slot. This type describes only the vtable prefix through GetHandle. */
	void* gap0C;
	/* IDB +0x10 */
	HRESULT(AERON_DXAPI* GetHandle)(struct PowerVrTextureInterface*, struct IUnknown*, unsigned int*);
};

/* Original IDB size: 20 bytes. */
struct PowerVrD3DVtblPrefix {
	/* IDB +0x0 */
	HRESULT(AERON_DXAPI* QueryInterface)(struct PowerVrD3DInterface*, const DxGuid*, void**);
	/* IDB +0x4 */
	uint32_t(AERON_DXAPI* AddRef)(struct PowerVrD3DInterface*);
	/* IDB +0x8 */
	uint32_t(AERON_DXAPI* Release)(struct PowerVrD3DInterface*);
	/* IDB +0xC: Unmodeled method slot. This type describes only the vtable prefix through EnumDevices. */
	void* gap0C;
	/* IDB +0x10 */
	HRESULT(AERON_DXAPI* EnumDevices)(struct PowerVrD3DInterface*, PowerVrProbeEnumCallback, void*);
};

extern int g_powerVrProbeDeviceCount;
extern PowerVrProbeDevice g_powerVrProbeDevices[POWERVR_PROBE_DEVICE_CAPACITY];

/* Declarations follow ascending original IDB address. */

/* 0x485550 */
int32_t PowerVr_ProbeSupport(struct PowerVrProbeInfoPrefix* info);

/* 0x485690 */
int32_t PowerVr_FindPciDevice(unsigned int vendorId, unsigned int deviceId);

/* 0x4856F0 */
int32_t PowerVr_IsPc98(void);

/* 0x485730 */
int32_t PowerVr_DetectMatroxM3D(void);

/* 0x485820 */
int32_t PowerVr_GetHalDllVersion(struct PowerVrProbeInfoPrefix* info);

/* 0x485A90 */
int32_t PowerVr_ProbeTextureInterface(struct PowerVrProbeInfoPrefix* info);

/* 0x4861E0 */
HRESULT AERON_DXAPI PowerVr_EnumDevicesCallback(DxGuid* deviceGuid, char* description, char* name,
												struct Std3DDeviceDesc* hardwareDesc,
												struct Std3DDeviceDesc* softwareDesc, void* context);

#ifdef __cplusplus
}
#endif

#endif
