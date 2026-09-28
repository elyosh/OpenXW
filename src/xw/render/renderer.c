#include "xw/render/renderer.h"

#include "xw/flight/flight_display.h"

#include "xw/assets/file.h"
#include "xw/math/math.h"
#include "xw/render/power_vr.h"
#include "xw/render/render_texture.h"
#include "xw/render/std3d.h"
#include "xw/util/shared.h"

#include <string.h>

// GLOBAL: XW 0x55CADC
int g_isPowerVr = 0;

// GLOBAL: XW 0x566878
int g_useHardware3D = 0;

// FUNCTION: XW 0x47E740
IDirectDraw* Renderer_GetDirectDraw(void) { return g_flightDirectDraw; }

// FUNCTION: XW 0x47EEF0
void Renderer_InitD3DDevice(void) {
	unsigned int deviceIndex;
	XwFile* file;
	DDSURFACEDESC attachmentDesc;
	Std3DDeviceCaps deviceCaps;
	HRESULT attachmentResult;
	g_isPowerVr = 0;
	file = File_RawOpenText("powervr.txt", "r");
	if (file != NULL) {
		File_RawClose(file);
		g_isPowerVr = 1;
	}
	file = File_RawOpenText("nopowervr.txt", "r");
	if (file != NULL) {
		File_RawClose(file);
	} else if (!g_isPowerVr && PowerVr_ProbeSupport(NULL)) {
		g_isPowerVr = 1;
	}
	g_renderTextureCacheCursor = -1;
	std3D_InitRenderTargetDesc(g_flightDisplayWidth, g_flightDisplayHeight, g_surfacePitch);
	std3D_SetRenderSurface(g_flightBackBuffer);
	std3D_SetColorOverlayParams(0.0f, 0.0f, 0.0f, 0);
	std3D_Startup();
	memset(&deviceCaps, 0, sizeof(deviceCaps));
	deviceCaps.bHardware = 1;
	deviceCaps.bTexturePerspective = 1;
	deviceCaps.bHasZBuffer = 1;
	deviceCaps.colorModelFlags = STD3D_COLOR_MODEL_RGB;
	deviceIndex = std3D_SelectBestDevice(&deviceCaps);
	memcpy(&deviceCaps, &g_std3DDevices[deviceIndex].caps, sizeof(deviceCaps));
	if (!deviceCaps.bHasZBuffer || !deviceCaps.bTexturePerspective || !deviceCaps.bHardware) {
		nullsub_SharedNoOp();
		std3D_Shutdown();
		g_useHardware3D = 0;
		g_flightResolutionMode = FLIGHT_DISPLAY_MODE_111H;
		Math_SetFpuSinglePrecisionMode();
		return;
	}
	{
		std3D_CreateDevice(deviceIndex, 1);
		memset(&attachmentDesc, 0, sizeof(attachmentDesc));
		attachmentDesc.dwSize = sizeof(attachmentDesc);
		attachmentDesc.dwFlags = DDSD_CAPS;
		attachmentDesc.ddsCaps.dwCaps = DDSCAPS_ZBUFFER;
		attachmentResult = g_flightBackBuffer->lpVtbl->GetAttachedSurface(
			g_flightBackBuffer, &attachmentDesc.ddsCaps, &g_std3DZBufferSurface);
		if (attachmentResult != 0) {
			nullsub_SharedNoOp();
			std3D_Close();
			std3D_Shutdown();
			g_useHardware3D = 0;
			g_flightResolutionMode = FLIGHT_DISPLAY_MODE_111H;
			Math_SetFpuSinglePrecisionMode();
		} else {
			Math_SetFpuSinglePrecisionMode();
		}
	}
}
