#include "xw/render/flight_starfield.h"
#ifdef XW_MODERN
#include "xw_runtime/snapshot/render_sky.h"
#endif

#include "xw/flight/fediskio.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/hud/festring.h"
#include "xw/flight/player/user.h"
#include "xw/math/math2.h"
#include "xw/math/transfm2.h"
#include "xw/render/flight_view.h"
#include "xw/render/rtsrgb.h"
#include "xw/render/rtsvga2.h"
#include "xw/render/sw3d.h"
#include "xw/util/memory.h"
#include "xw/util/shared.h"

// GLOBAL: XW 0x4C9C60
int g_starfieldGridDimension = FLIGHT_STARFIELD_GRID_SIDE;

// GLOBAL: XW 0x4F49B4
int g_starfieldColors8Initialized = 0;

// GLOBAL: XW 0x4F49B8
int g_starfieldColors16Initialized = 0;

// GLOBAL: XW 0x4F49BC
int g_starfieldRandomVectorIndicesInitialized = 0;

// GLOBAL: XW 0x4F49C0
uint16_t g_starfieldColors8Handle = 0;

// GLOBAL: XW 0x4F49C4
uint16_t g_starfieldColors16Handle = 0;

// GLOBAL: XW 0x4F49C8
uint16_t g_starfieldRandomVectorIndicesHandle = 0;

// GLOBAL: XW 0x62B510
int g_starfieldColorCacheReusable = 0;

// GLOBAL: XW 0x637A60
int g_starfieldJitterY[FLIGHT_STARFIELD_JITTER_COUNT] = { 0 };

// GLOBAL: XW 0x637C60
int g_starfieldJitterZ[FLIGHT_STARFIELD_JITTER_COUNT] = { 0 };

// GLOBAL: XW 0x637E60
int g_starfieldJitterX[FLIGHT_STARFIELD_JITTER_COUNT] = { 0 };

// FUNCTION: XW 0x4232F0
void FlightStarfield_Render(void) {
	uint16_t backgroundColor16;
	void* starColors;
	uint8_t* randomVectorIndices;
	float stepX[FLIGHT_STARFIELD_PLANE_COUNT];
	float stepY[FLIGHT_STARFIELD_PLANE_COUNT];
	float stepZ[FLIGHT_STARFIELD_PLANE_COUNT];
	float initialViewX, initialViewY, initialViewZ;
	float baseViewX, baseViewY, baseViewZ;
	int planeIndex, rowIndex, columnIndex, starIndex;
	int columnAxis, rowAxis;
	RgbTriplet targetRgb;
	g_starfieldGridDimension = FLIGHT_STARFIELD_GRID_SIDE / g_starDensity;
	if (g_flightBytesPerPixel == sizeof(uint16_t)) {
		backgroundColor16 = g_flightTextPalette[g_flightColorEscapeBypassChar];
		if (g_starfieldColors16Initialized == 0 || g_starfieldColorCacheReusable == 0) {
			uint16_t* colors;
			int index, remaining;
			g_starfieldColors16Handle = Memory_AllocHandle(FLIGHT_STARFIELD_STAR_COUNT * sizeof(*colors), 0);
			if (g_starfieldColors16Handle == 0)
				fediskio_fatalerror(0);
			colors = Memory_LockHandle(g_starfieldColors16Handle);
			if (FlightDisplay_IsPixelFormat555()) {
				for (index = 0, remaining = FLIGHT_STARFIELD_STAR_COUNT; remaining != 0; ++index, --remaining)
					colors[index] = FLIGHT_STARFIELD_RGB555_GRAY *
									((math2_getrandom() & FLIGHT_STARFIELD_INTENSITY_MASK) +
									 FLIGHT_STARFIELD_MIN_INTENSITY);
			} else {
				for (index = 0, remaining = FLIGHT_STARFIELD_STAR_COUNT; remaining != 0; ++index, --remaining)
					colors[index] = FLIGHT_STARFIELD_RGB565_GRAY *
									((math2_getrandom() & FLIGHT_STARFIELD_INTENSITY_MASK) +
									 FLIGHT_STARFIELD_MIN_INTENSITY);
			}
			nullsub_SharedNoOp();
			g_starfieldColors16Initialized = 1;
		}
		starColors = Memory_LockHandle(g_starfieldColors16Handle);
	} else {
		if (g_starfieldColors8Initialized == 0 || g_starfieldColorCacheReusable == 0) {
			uint8_t* colors;
			int index;
			g_starfieldColors8Handle = Memory_AllocHandle(FLIGHT_STARFIELD_STAR_COUNT * sizeof(*colors), 0);
			if (g_starfieldColors8Handle == 0)
				fediskio_fatalerror(0);
			colors = Memory_LockHandle(g_starfieldColors8Handle);
			for (index = 0; index < FLIGHT_STARFIELD_STAR_COUNT; ++index) {
				targetRgb.r =
					(math2_getrandom() & FLIGHT_STARFIELD_INTENSITY_MASK) + FLIGHT_STARFIELD_MIN_INTENSITY;
				targetRgb.g = targetRgb.r;
				targetRgb.b = targetRgb.r;
				colors[index] = rtsvga2_FindNearestRgbTripletIndex(
					&targetRgb, g_swPalette, FLIGHT_STARFIELD_PALETTE_FIRST, RTSVGA2_PALETTE_COLOR_COUNT);
			}
			nullsub_SharedNoOp();
			g_starfieldColors8Initialized = 1;
		}
		starColors = Memory_LockHandle(g_starfieldColors8Handle);
	}
	if (g_starfieldRandomVectorIndicesInitialized == 0) {
		uint8_t* indices;
		int index;
		unsigned int vectorIndex;
		g_starfieldRandomVectorIndicesHandle =
			Memory_AllocHandle(FLIGHT_STARFIELD_RANDOM_ALLOCATION_BYTES, 0);
		if (g_starfieldRandomVectorIndicesHandle == 0)
			fediskio_fatalerror(0);
		indices = Memory_LockHandle(g_starfieldRandomVectorIndicesHandle);
		for (index = 0; index < FLIGHT_STARFIELD_STAR_COUNT; ++index) {
			do {
				vectorIndex = math2_getrandom() & FLIGHT_STARFIELD_JITTER_RANDOM_MASK;
			} while (vectorIndex >= FLIGHT_STARFIELD_JITTER_COUNT);
			indices[index] = (uint8_t)vectorIndex;
		}
		nullsub_SharedNoOp();
		g_starfieldRandomVectorIndicesInitialized = 1;
	}
	randomVectorIndices = Memory_LockHandle(g_starfieldRandomVectorIndicesHandle);
#ifdef XW_MODERN
	XwRenderSky_Stars(randomVectorIndices, starColors);
#endif
	starIndex = 0;
	initialViewX = -(g_camMatR0_X + g_camMatR0_Z + g_camMatR0_Y) >> 2;
	initialViewY = -(g_camMatR1_X + g_camMatR1_Z + g_camMatR1_Y) >> 2;
	initialViewZ = -(g_camMatR2_X + g_camMatR2_Y + g_camMatR2_Z) >> 2;
	baseViewX = initialViewX;
	baseViewY = initialViewY;
	baseViewZ = initialViewZ;
	stepX[0] = (double)(g_camMatR0_X >> 1) * g_sw3dSpanLengthReciprocal[g_starfieldGridDimension];
	stepY[0] = (double)(g_camMatR1_X >> 1) * g_sw3dSpanLengthReciprocal[g_starfieldGridDimension];
	stepZ[0] = (double)(g_camMatR2_X >> 1) * g_sw3dSpanLengthReciprocal[g_starfieldGridDimension];
	stepX[1] = (double)(g_camMatR0_Y >> 1) * g_sw3dSpanLengthReciprocal[g_starfieldGridDimension];
	stepY[1] = (double)(g_camMatR1_Y >> 1) * g_sw3dSpanLengthReciprocal[g_starfieldGridDimension];
	stepZ[1] = (double)(g_camMatR2_Y >> 1) * g_sw3dSpanLengthReciprocal[g_starfieldGridDimension];
	stepX[2] = (double)(g_camMatR0_Z >> 1) * g_sw3dSpanLengthReciprocal[g_starfieldGridDimension];
	stepY[2] = (double)(g_camMatR1_Z >> 1) * g_sw3dSpanLengthReciprocal[g_starfieldGridDimension];
	stepZ[2] = (double)(g_camMatR2_Z >> 1) * g_sw3dSpanLengthReciprocal[g_starfieldGridDimension];
	columnAxis = 0;
	rowAxis = 1;
	for (planeIndex = 0; planeIndex < FLIGHT_STARFIELD_PLANE_COUNT; ++planeIndex) {
		for (rowIndex = 0; rowIndex < g_starfieldGridDimension; ++rowIndex) {
			float currentViewX = baseViewX;
			float currentViewY = baseViewY;
			float currentViewZ = baseViewZ;
			for (columnIndex = 0; columnIndex < g_starfieldGridDimension; ++columnIndex) {
				int jitterIndex = randomVectorIndices[starIndex];
				double viewX = (double)g_starfieldJitterX[jitterIndex] + currentViewX;
				double viewY = (double)g_starfieldJitterY[jitterIndex] + currentViewY;
				double viewZ = (double)g_starfieldJitterZ[jitterIndex] + currentViewZ;
				double absoluteX, absoluteY;
				if (viewZ < 0.0) {
					viewX = -viewX;
					viewY = -viewY;
					viewZ = -viewZ;
				}
				absoluteX = viewX;
				if (absoluteX < 0.0)
					absoluteX = -absoluteX;
				if (absoluteX < viewZ) {
					absoluteY = viewY;
					if (absoluteY < 0.0)
						absoluteY = -absoluteY;
					if (absoluteY < viewZ) {
						double scale = (double)(uint32_t)g_projScaleInt / viewZ;
						int screenX = g_flightVpCenterX + (int)(scale * viewX);
						int screenY = g_flightVpCenterY + g_projOffsetY + (int)(scale * viewY);
						if (screenX >= 0 && screenX < g_flightVpWidth && screenY >= 0 &&
							screenY < g_flightVpHeight) {
							if (g_flightBytesPerPixel == sizeof(uint16_t)) {
								uint16_t* pixel =
									(uint16_t*)&g_flightSwFramebufferBase[sizeof(uint16_t) *
																			  (g_flightVpX + screenX) +
																		  g_surfacePitch *
																			  (g_flightVpY + screenY)];
								if (*pixel == backgroundColor16)
									*pixel = ((uint16_t*)starColors)[starIndex];
							} else {
								uint8_t* pixel =
									&g_flightSwFramebufferBase[g_flightVpX + screenX +
															   g_surfacePitch * (g_flightVpY + screenY)];
								if (*pixel == g_flightColorEscapeBypassChar)
									*pixel = ((uint8_t*)starColors)[starIndex];
							}
						}
					}
				}
				currentViewX += stepX[columnAxis];
				currentViewY += stepY[columnAxis];
				currentViewZ += stepZ[columnAxis];
				++starIndex;
			}
			baseViewX = stepX[rowAxis] + baseViewX;
			baseViewY = stepY[rowAxis] + baseViewY;
			baseViewZ += stepZ[rowAxis];
		}
		baseViewX = initialViewX;
		baseViewY = initialViewY;
		baseViewZ = initialViewZ;
		if (planeIndex == 0)
			++rowAxis;
		if (planeIndex == 1)
			++columnAxis;
	}
	nullsub_SharedNoOp();
	nullsub_SharedNoOp();
}
