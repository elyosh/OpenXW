#include "xw/render/power_vr.h"

#include "xw_runtime/compat/win_strings.h"
#include "xw_runtime/platform/legacy_hardware.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4C33F8
const DxGuid g_iidDirect3DTexture = {
	0x2CDCD9E0, 0x25A0, 0x11CF, { 0xA3, 0x1A, 0x00, 0xAA, 0x00, 0xB9, 0x33, 0x56 }
};

// GLOBAL: XW 0x4DA378
const char g_powerVrHalDllFilename[] = "PVRHAL32.DLL";

// GLOBAL: XW 0x55CAF4
int g_powerVrProbeDeviceCount = 0;

// GLOBAL: XW 0x5BEDA0
PowerVrProbeDevice g_powerVrProbeDevices[POWERVR_PROBE_DEVICE_CAPACITY] = { 0 };

// FUNCTION: XW 0x485550
int32_t PowerVr_ProbeSupport(struct PowerVrProbeInfoPrefix* info) {
	if (PowerVr_FindPciDevice(POWERVR_VENDOR_NEC, POWERVR_DEVICE_61)) {
		if (info != NULL) {
			info->stage = POWERVR_STAGE_PCI_DEVICE;
			info->vendorId = POWERVR_VENDOR_NEC;
			info->deviceId = POWERVR_DEVICE_61;
		}
	} else if (PowerVr_FindPciDevice(POWERVR_VENDOR_NEC, POWERVR_DEVICE_46)) {
		if (info != NULL) {
			info->stage = POWERVR_STAGE_PCI_DEVICE;
			info->vendorId = POWERVR_VENDOR_NEC;
			info->deviceId = POWERVR_DEVICE_46;
		}
	} else if (PowerVr_FindPciDevice(POWERVR_VENDOR_NEC, POWERVR_DEVICE_2A)) {
		if (info != NULL) {
			info->stage = POWERVR_STAGE_PCI_DEVICE;
			info->vendorId = POWERVR_VENDOR_NEC;
			info->deviceId = POWERVR_DEVICE_2A;
		}
	} else if (PowerVr_FindPciDevice(POWERVR_VENDOR_NEC, POWERVR_DEVICE_1F)) {
		if (info != NULL) {
			info->stage = POWERVR_STAGE_PCI_DEVICE;
			info->vendorId = POWERVR_VENDOR_NEC;
			info->deviceId = POWERVR_DEVICE_1F;
		}
	} else {
		return 0;
	}
	if (info != NULL) {
		if (info->deviceId == POWERVR_DEVICE_46 && PowerVr_DetectMatroxM3D())
			strcpy(info->boardLabel, "Matrox");
		else
			strcpy(info->boardLabel, "Generic");
	}
	if (!PowerVr_GetHalDllVersion(info))
		return 0;
	if (info != NULL)
		info->stage = POWERVR_STAGE_HAL_RESOURCE;
	if (!PowerVr_ProbeTextureInterface(info))
		return 0;
	if (info != NULL)
		info->stage = POWERVR_STAGE_COMPLETE;
	return 1;
}

// FUNCTION: XW 0x485690
int32_t PowerVr_FindPciDevice(unsigned int vendorId, unsigned int deviceId) {
	/* Legacy PCI BIOS probing is explicitly disabled in both builds. */
	(void)vendorId;
	(void)deviceId;
	return 0;
}

// FUNCTION: XW 0x4856F0
int32_t PowerVr_IsPc98(void) {
#ifdef XW_MODERN
	return 0;
#else
	int32_t detected = 0;
	if (GetKeyboardType(POWERVR_KEYBOARD_TYPE) == POWERVR_KEYBOARD_JAPANESE) {
		int subtype = GetKeyboardType(POWERVR_KEYBOARD_SUBTYPE);
		if (subtype > POWERVR_PC98_SUBTYPE_LOWER_BOUND && subtype < POWERVR_PC98_SUBTYPE_UPPER_BOUND) {
			detected = 1;
			OutputDebugStringA("PC98 detected!\n");
		} else {
			detected = 0;
		}
	}
	return detected;
#endif
}

// FUNCTION: XW 0x485730
int32_t PowerVr_DetectMatroxM3D(void) {
#ifdef XW_MODERN
	return 0;
#else
	int32_t isMatrox = 0;
	int boardIndex = 0;
	PowerVrPhysicalBoardInfo boardInfo;
	char debugText[POWERVR_DEBUG_TEXT_CAPACITY];
	void* vxdHandle = CreateFileA("\\\\.\\VSGL.VXD", 0, 0, NULL, 0, POWERVR_VXD_DELETE_ON_CLOSE, NULL);
	if (XwLegacyHardware_IsInvalidHandle(vxdHandle)) {
		OutputDebugStringA("Failed to open VxD\n");
		CloseHandle(vxdHandle);
		return 0;
	} else if (DeviceIoControl(vxdHandle, POWERVR_GET_PHYSICAL_BOARD_INFO, &boardIndex, sizeof(boardIndex),
							   &boardInfo, sizeof(boardInfo), NULL, NULL)) {
		uint8_t* gpPort = boardInfo.registerBase + POWERVR_GP_PORT_OFFSET;
		XwLegacyHardware_WriteRegister32(gpPort, 0);
		Sleep(POWERVR_GP_PORT_SETTLE_MS);
		if ((XwLegacyHardware_ReadRegister32(gpPort) & UINT16_MAX) == POWERVR_MATROX_SIGNATURE) {
			isMatrox = 1;
			OutputDebugStringA("Matrox M3D detected\n");
		}
		sprintf(debugText, "GP PORT address=0x%X\nGP PORT value=0x%X\n",
				XwLegacyHardware_RegisterAddress(gpPort), XwLegacyHardware_ReadRegister32(gpPort));
		OutputDebugStringA(debugText);
	} else {
		OutputDebugStringA("Unable to get physical board info\n");
	}
	CloseHandle(vxdHandle);
	return isMatrox;
#endif
}

// FUNCTION: XW 0x485820
int32_t PowerVr_GetHalDllVersion(struct PowerVrProbeInfoPrefix* info) {
#ifdef XW_MODERN
	(void)info;
	return 0;
#else
	uint32_t versionHandle;
	XwLegacyFixedFileInfo* fixedVersion;
	unsigned int* textLength = calloc(1, sizeof(*textLength));
	void* versionText;
	unsigned int fixedVersionBytes;
	char dllFilename[POWERVR_VERSION_FILENAME_CAPACITY];
	uint32_t versionBytes;
	memcpy(dllFilename, g_powerVrHalDllFilename, sizeof(g_powerVrHalDllFilename));
	versionBytes = GetFileVersionInfoSizeA(dllFilename, &versionHandle);
	if (versionBytes == 0) {
		OutputDebugStringA("GetFileVersionInfoSize() failed in GetSGLDLLVersion()\n");
		return 0;
	}
	if (info != NULL) {
		void* fixedBlockHandle = GlobalAlloc(POWERVR_GLOBAL_MOVEABLE, versionBytes);
		void* fixedBlock;
		void* textBlockHandle;
		void* textBlock;
		if (fixedBlockHandle == NULL) {
			OutputDebugStringA("Memory could not be allocated\n");
			return 0;
		}
		fixedBlock = GlobalLock(fixedBlockHandle);
		if (fixedBlock == NULL) {
			OutputDebugStringA("GlobalLock() failed in GetHALDLLVersion()\n");
			GlobalFree(fixedBlockHandle);
			return 0;
		}
		if (!GetFileVersionInfoA(dllFilename, versionHandle, versionBytes, fixedBlock)) {
			OutputDebugStringA("GetFileVersionInfo() failed in GetHALDLLVersion()\n");
			GlobalUnlock(fixedBlockHandle);
			GlobalFree(fixedBlockHandle);
			return 0;
		}
		if (!VerQueryValueA(fixedBlock, "\\", (void**)&fixedVersion, &fixedVersionBytes)) {
			OutputDebugStringA("VerQueryValue() failed in GetHALDLLVersion()\n");
			GlobalUnlock(fixedBlockHandle);
			GlobalFree(fixedBlockHandle);
			return 0;
		}
		info->halVersionMajor = fixedVersion->dwFileVersionMS >> POWERVR_VERSION_COMPONENT_SHIFT;
		info->halVersionMinor = (uint16_t)fixedVersion->dwFileVersionMS;
		info->halVersionBuild = fixedVersion->dwFileVersionLS >> POWERVR_VERSION_COMPONENT_SHIFT;
		info->halVersionRevision = (uint16_t)fixedVersion->dwFileVersionLS;
		GlobalUnlock(fixedBlockHandle);
		GlobalFree(fixedBlockHandle);
		textBlockHandle = GlobalAlloc(POWERVR_GLOBAL_MOVEABLE, versionBytes);
		if (textBlockHandle == NULL) {
			OutputDebugStringA("Memory could not be allocated\n");
			return 0;
		}
		textBlock = GlobalLock(textBlockHandle);
		if (textBlock == NULL) {
			OutputDebugStringA("GlobalLock() failed in GetHALDLLVersion()\n");
			GlobalFree(textBlockHandle);
			return 0;
		}
		if (!GetFileVersionInfoA(dllFilename, versionHandle, versionBytes, textBlock)) {
			OutputDebugStringA("GetFileVersionInfo() failed in GetHALDLLVersion()\n");
			GlobalUnlock(textBlockHandle);
			GlobalFree(textBlockHandle);
			return 0;
		}
		if (!VerQueryValueA(textBlock, "\\StringFileInfo\\040904E4\\FileVersion", &versionText, textLength)) {
			OutputDebugStringA("VerQueryValue() failed in GetHALDLLVersion()\n");
			GlobalUnlock(textBlockHandle);
			GlobalFree(textBlockHandle);
			return 0;
		}
		/* The optional caller buffer includes a string tail after the verified prefix. */
		strcpy((char*)(info + 1), versionText);
		GlobalUnlock(textBlockHandle);
		GlobalFree(textBlockHandle);
	}
	return 1;
#endif
}

// FUNCTION: XW 0x485A90
int32_t PowerVr_ProbeTextureInterface(struct PowerVrProbeInfoPrefix* info) {
#ifdef XW_MODERN
	(void)info;
	return 0;
#else
	int selectedDeviceIndex = -1;
	int deviceIndex;
	HRESULT result;
	uint32_t* savedTextureWords;
	IDirectDraw* directDraw2;
	PowerVrD3DInterface* direct3D;
	IDirectDrawSurface* deviceSurface;
	IUnknown* device;
	IDirectDrawSurface* textureSurface;
	IDirectDraw* directDraw;
	PowerVrTextureInterface* texture;
	unsigned int textureHandle;
	DDSURFACEDESC surfaceDesc;
	result = DirectDrawCreate(NULL, &directDraw, NULL);
	if (result) {
		OutputDebugStringA("Error in DirectDrawCreate\n");
		if (directDraw != NULL) {
			directDraw->lpVtbl->Release(directDraw);
		}
		return 0;
	}
	result =
		directDraw->lpVtbl->QueryInterface(directDraw, &g_directDraw2InterfaceGuid, (void**)&directDraw2);
	if (result) {
		OutputDebugStringA("Failed to find a interface to DirectDraw2\n");
		if (directDraw2 != NULL) {
			directDraw2->lpVtbl->Release(directDraw2);
			directDraw2 = NULL;
		}
		if (directDraw != NULL) {
			directDraw->lpVtbl->Release(directDraw);
		}
		return 0;
	}
	if (info != NULL)
		info->stage = POWERVR_STAGE_DIRECTDRAW2;
	result = directDraw2->lpVtbl->QueryInterface(directDraw2, &IID_IDirect3D, (void**)&direct3D);
	if (result) {
		OutputDebugStringA("Failed to detect Direct3D version 2.0\n");
		if (direct3D != NULL) {
			direct3D->lpVtbl->Release(direct3D);
			direct3D = NULL;
		}
		if (directDraw2 != NULL) {
			directDraw2->lpVtbl->Release(directDraw2);
			directDraw2 = NULL;
		}
		if (directDraw != NULL) {
			directDraw->lpVtbl->Release(directDraw);
		}
		return 0;
	}
	result = directDraw2->lpVtbl->SetCooperativeLevel(directDraw2, GetActiveWindow(), DDSCL_NORMAL);
	if (result) {
		OutputDebugStringA("Error in SetCooperativeLevel\n");
		if (direct3D != NULL) {
			direct3D->lpVtbl->Release(direct3D);
			direct3D = NULL;
		}
		if (directDraw2 != NULL) {
			directDraw2->lpVtbl->Release(directDraw2);
			directDraw2 = NULL;
		}
		if (directDraw != NULL) {
			directDraw->lpVtbl->Release(directDraw);
		}
		return 0;
	}
	memset(&surfaceDesc, 0, sizeof(surfaceDesc));
	surfaceDesc.dwSize = sizeof(surfaceDesc);
	surfaceDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	surfaceDesc.dwWidth = POWERVR_PROBE_SURFACE_SIZE;
	surfaceDesc.dwHeight = POWERVR_PROBE_SURFACE_SIZE;
	surfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
	result = directDraw2->lpVtbl->CreateSurface(directDraw2, &surfaceDesc, &deviceSurface, NULL);
	if (result) {
		OutputDebugStringA("Failed to create surface\n");
		if (deviceSurface != NULL) {
			deviceSurface->lpVtbl->Release(deviceSurface);
			deviceSurface = NULL;
		}
		if (direct3D != NULL) {
			direct3D->lpVtbl->Release(direct3D);
			direct3D = NULL;
		}
		if (directDraw2 != NULL) {
			directDraw2->lpVtbl->Release(directDraw2);
			directDraw2 = NULL;
		}
		if (directDraw != NULL) {
			directDraw->lpVtbl->Release(directDraw);
		}
		return 0;
	}
	result = direct3D->lpVtbl->EnumDevices(direct3D, PowerVr_EnumDevicesCallback, NULL);
	if (result) {
		OutputDebugStringA("Failed in EnumDevices\n");
		if (deviceSurface != NULL) {
			deviceSurface->lpVtbl->Release(deviceSurface);
			deviceSurface = NULL;
		}
		if (direct3D != NULL) {
			direct3D->lpVtbl->Release(direct3D);
			direct3D = NULL;
		}
		if (directDraw2 != NULL) {
			directDraw2->lpVtbl->Release(directDraw2);
			directDraw2 = NULL;
		}
		if (directDraw != NULL) {
			directDraw->lpVtbl->Release(directDraw);
		}
		return 0;
	}
	for (deviceIndex = 0; deviceIndex < g_powerVrProbeDeviceCount; ++deviceIndex) {
		if (g_powerVrProbeDevices[deviceIndex].isHardware != 0 &&
			(g_powerVrProbeDevices[deviceIndex].caps.dcmColorModel & STD3D_COLOR_MODEL_RGB) != 0) {
			selectedDeviceIndex = deviceIndex;
			break;
		}
	}
	if (selectedDeviceIndex == -1)
		return 0;
	if (info != NULL)
		info->stage = POWERVR_STAGE_HARDWARE_DEVICE;
	result = deviceSurface->lpVtbl->QueryInterface(
		deviceSurface, &g_powerVrProbeDevices[selectedDeviceIndex].guid, (void**)&device);
	if (result) {
		OutputDebugStringA("Failed in getting a device\n");
		if (deviceSurface != NULL) {
			deviceSurface->lpVtbl->Release(deviceSurface);
			deviceSurface = NULL;
		}
		if (direct3D != NULL) {
			direct3D->lpVtbl->Release(direct3D);
			direct3D = NULL;
		}
		if (directDraw2 != NULL) {
			directDraw2->lpVtbl->Release(directDraw2);
			directDraw2 = NULL;
		}
		if (directDraw != NULL) {
			directDraw->lpVtbl->Release(directDraw);
		}
		return 0;
	}
	surfaceDesc.dwSize = sizeof(surfaceDesc);
	surfaceDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	surfaceDesc.dwHeight = POWERVR_PROBE_SURFACE_SIZE;
	surfaceDesc.dwWidth = POWERVR_PROBE_SURFACE_SIZE;
	surfaceDesc.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
	surfaceDesc.ddpfPixelFormat.dwSize = sizeof(surfaceDesc.ddpfPixelFormat);
	surfaceDesc.ddpfPixelFormat.dwFlags = POWERVR_PIXEL_FORMAT_FOURCC;
	surfaceDesc.ddpfPixelFormat.dwFourCC = POWERVR_PVRC_FOURCC;
	result = directDraw2->lpVtbl->CreateSurface(directDraw2, &surfaceDesc, &textureSurface, NULL);
	if (result) {
		OutputDebugStringA("Failed to create surface\n");
		if (device != NULL) {
			device->lpVtbl->Release(device);
			device = NULL;
		}
		if (deviceSurface != NULL) {
			deviceSurface->lpVtbl->Release(deviceSurface);
			deviceSurface = NULL;
		}
		if (direct3D != NULL) {
			direct3D->lpVtbl->Release(direct3D);
			direct3D = NULL;
		}
		if (directDraw2 != NULL) {
			directDraw2->lpVtbl->Release(directDraw2);
			directDraw2 = NULL;
		}
		if (directDraw != NULL) {
			directDraw->lpVtbl->Release(directDraw);
		}
		return 0;
	}
	result = textureSurface->lpVtbl->Lock(textureSurface, NULL, &surfaceDesc, DDLOCK_WAIT, NULL);
	if (result) {
		OutputDebugStringA("Failed to lock surface\n");
		if (textureSurface != NULL) {
			textureSurface->lpVtbl->Release(textureSurface);
			textureSurface = NULL;
		}
		if (device != NULL) {
			device->lpVtbl->Release(device);
			device = NULL;
		}
		if (deviceSurface != NULL) {
			deviceSurface->lpVtbl->Release(deviceSurface);
			deviceSurface = NULL;
		}
		if (direct3D != NULL) {
			direct3D->lpVtbl->Release(direct3D);
			direct3D = NULL;
		}
		if (directDraw2 != NULL) {
			directDraw2->lpVtbl->Release(directDraw2);
			directDraw2 = NULL;
		}
		if (directDraw != NULL) {
			directDraw->lpVtbl->Release(directDraw);
		}
		return 0;
	}
	savedTextureWords = surfaceDesc.lpSurface;
	result = textureSurface->lpVtbl->Unlock(textureSurface, &surfaceDesc);
	if (result) {
		OutputDebugStringA("Failed to Unlock surface\n");
		if (textureSurface != NULL) {
			textureSurface->lpVtbl->Release(textureSurface);
			textureSurface = NULL;
		}
		if (device != NULL) {
			device->lpVtbl->Release(device);
			device = NULL;
		}
		if (deviceSurface != NULL) {
			deviceSurface->lpVtbl->Release(deviceSurface);
			deviceSurface = NULL;
		}
		if (direct3D != NULL) {
			direct3D->lpVtbl->Release(direct3D);
			direct3D = NULL;
		}
		if (directDraw2 != NULL) {
			directDraw2->lpVtbl->Release(directDraw2);
			directDraw2 = NULL;
		}
		if (directDraw != NULL) {
			directDraw->lpVtbl->Release(directDraw);
		}
		return 0;
	}
	result = textureSurface->lpVtbl->QueryInterface(textureSurface, &g_iidDirect3DTexture, (void**)&texture);
	if (result) {
		OutputDebugStringA("Failed to get texturing interface\r\n");
		if (textureSurface != NULL) {
			textureSurface->lpVtbl->Release(textureSurface);
			textureSurface = NULL;
		}
		if (device != NULL) {
			device->lpVtbl->Release(device);
			device = NULL;
		}
		if (deviceSurface != NULL) {
			deviceSurface->lpVtbl->Release(deviceSurface);
			deviceSurface = NULL;
		}
		if (direct3D != NULL) {
			direct3D->lpVtbl->Release(direct3D);
			direct3D = NULL;
		}
		if (directDraw2 != NULL) {
			directDraw2->lpVtbl->Release(directDraw2);
			directDraw2 = NULL;
		}
		if (directDraw != NULL) {
			directDraw->lpVtbl->Release(directDraw);
		}
		return 0;
	}
	result = texture->lpVtbl->GetHandle(texture, device, &textureHandle);
	if (result) {
		OutputDebugStringA("Failed to get texture handle\n");
		if (texture != NULL) {
			texture->lpVtbl->Release(texture);
			texture = NULL;
		}
		if (textureSurface != NULL) {
			textureSurface->lpVtbl->Release(textureSurface);
			textureSurface = NULL;
		}
		if (device != NULL) {
			device->lpVtbl->Release(device);
			device = NULL;
		}
		if (deviceSurface != NULL) {
			deviceSurface->lpVtbl->Release(deviceSurface);
			deviceSurface = NULL;
		}
		if (direct3D != NULL) {
			direct3D->lpVtbl->Release(direct3D);
			direct3D = NULL;
		}
		if (directDraw2 != NULL) {
			directDraw2->lpVtbl->Release(directDraw2);
			directDraw2 = NULL;
		}
		if (directDraw != NULL) {
			directDraw->lpVtbl->Release(directDraw);
		}
		return 0;
	}
	if (savedTextureWords[POWERVR_TEXTURE_RESERVED_WORD] == 0) {
		OutputDebugStringA("Reserved word is NULL\n");
		if (texture != NULL) {
			texture->lpVtbl->Release(texture);
			texture = NULL;
		}
		if (textureSurface != NULL) {
			textureSurface->lpVtbl->Release(textureSurface);
			textureSurface = NULL;
		}
		if (device != NULL) {
			device->lpVtbl->Release(device);
			device = NULL;
		}
		if (deviceSurface != NULL) {
			deviceSurface->lpVtbl->Release(deviceSurface);
			deviceSurface = NULL;
		}
		if (direct3D != NULL) {
			direct3D->lpVtbl->Release(direct3D);
			direct3D = NULL;
		}
		if (directDraw2 != NULL) {
			directDraw2->lpVtbl->Release(directDraw2);
			directDraw2 = NULL;
		}
		if (directDraw != NULL) {
			directDraw->lpVtbl->Release(directDraw);
		}
		return 0;
	}
	if (texture != NULL) {
		texture->lpVtbl->Release(texture);
		texture = NULL;
	}
	if (textureSurface != NULL) {
		textureSurface->lpVtbl->Release(textureSurface);
		textureSurface = NULL;
	}
	if (device != NULL) {
		device->lpVtbl->Release(device);
		device = NULL;
	}
	if (deviceSurface != NULL) {
		deviceSurface->lpVtbl->Release(deviceSurface);
		deviceSurface = NULL;
	}
	if (direct3D != NULL) {
		direct3D->lpVtbl->Release(direct3D);
		direct3D = NULL;
	}
	if (directDraw2 != NULL) {
		directDraw2->lpVtbl->Release(directDraw2);
		directDraw2 = NULL;
	}
	if (directDraw != NULL) {
		directDraw->lpVtbl->Release(directDraw);
	}
	return 1;
#endif
}

// FUNCTION: XW 0x4861E0
HRESULT AERON_DXAPI PowerVr_EnumDevicesCallback(DxGuid* deviceGuid, char* description, char* name,
												struct Std3DDeviceDesc* hardwareDesc,
												struct Std3DDeviceDesc* softwareDesc, void* context) {
	(void)context;
	g_powerVrProbeDevices[g_powerVrProbeDeviceCount].guid = *deviceGuid;
	XwWinStringCopy(g_powerVrProbeDevices[g_powerVrProbeDeviceCount].description, description);
	XwWinStringCopy(g_powerVrProbeDevices[g_powerVrProbeDeviceCount].name, name);
	if (hardwareDesc->dcmColorModel != 0) {
		g_powerVrProbeDevices[g_powerVrProbeDeviceCount].isHardware = 1;
		g_powerVrProbeDevices[g_powerVrProbeDeviceCount].caps = *hardwareDesc;
	} else {
		g_powerVrProbeDevices[g_powerVrProbeDeviceCount].isHardware = 0;
		g_powerVrProbeDevices[g_powerVrProbeDeviceCount].caps = *softwareDesc;
	}
	++g_powerVrProbeDeviceCount;
	return POWERVR_ENUM_CONTINUE;
}
