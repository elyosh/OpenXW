#include "xw/render/backdrp2.h"

#include "xw/assets/bitmap.h"
#include "xw/assets/model_mesh.h"
#include "xw/flight/object/create.h"
#include "xw/flight/player/user.h"
#include "xw/flight/xw.h"
#include "xw/math/transfm2.h"
#include "xw/math/trig2.h"
#include "xw/render/flight_starfield.h"
#include "xw/render/flight_view.h"
#include "xw/render/render_quad.h"
#include "xw/render/renderer.h"
#include "xw/render/rotscale.h"
#include "xw/util/memory.h"
#include "xw/util/shared.h"

// GLOBAL: XW 0x638060
int g_cameraWorldXToViewYSteps[BACKDRP2_CAMERA_STEP_COUNT] = { 0 };

// GLOBAL: XW 0x638840
int g_cameraWorldXToViewZSteps[BACKDRP2_CAMERA_STEP_COUNT] = { 0 };

// GLOBAL: XW 0x638880
int g_cameraWorldXToViewXSteps[BACKDRP2_CAMERA_STEP_COUNT] = { 0 };

// GLOBAL: XW 0x6388C0
int g_cameraWorldYToViewYSteps[BACKDRP2_CAMERA_STEP_COUNT] = { 0 };

// GLOBAL: XW 0x638900
int g_cameraWorldYToViewZSteps[BACKDRP2_CAMERA_STEP_COUNT] = { 0 };

// GLOBAL: XW 0x638960
int g_cameraWorldYToViewXSteps[BACKDRP2_CAMERA_STEP_COUNT] = { 0 };

// GLOBAL: XW 0x63A1A0
int g_cameraWorldZToViewYSteps[BACKDRP2_CAMERA_STEP_COUNT] = { 0 };

// GLOBAL: XW 0x63A1E0
int g_cameraWorldZToViewZSteps[BACKDRP2_CAMERA_STEP_COUNT] = { 0 };

// GLOBAL: XW 0x63A220
int g_cameraWorldZToViewXSteps[BACKDRP2_CAMERA_STEP_COUNT] = { 0 };

// FUNCTION: XW 0x402420
void backdrp2_backdrop(void) {
	int worldXViewZMultiple = g_camMatR2_X;
	int worldZViewXMultiple = g_camMatR0_Z;
	int worldZViewYMultiple = g_camMatR1_Z;
	int worldYViewZMultiple = g_camMatR2_Y;
	int worldXViewXMultiple = g_camMatR0_X;
	int worldYViewYMultiple = g_camMatR1_Y;
	int worldXViewYMultiple = g_camMatR1_X;
	int worldYViewXMultiple = g_camMatR0_Y;
	int worldZViewZMultiple = g_camMatR2_Z;
	int stepIndex, plane, row, column;
	int starDirectionIndex;
	int planeX, planeY, planeZ;
	for (stepIndex = 0; stepIndex < BACKDRP2_CAMERA_STEP_COUNT; ++stepIndex) {
		g_cameraWorldXToViewXSteps[stepIndex] = worldXViewXMultiple >> BACKDRP2_CAMERA_STEP_SHIFT;
		g_cameraWorldXToViewYSteps[stepIndex] = worldXViewYMultiple >> BACKDRP2_CAMERA_STEP_SHIFT;
		g_cameraWorldXToViewZSteps[stepIndex] = worldXViewZMultiple >> BACKDRP2_CAMERA_STEP_SHIFT;
		g_cameraWorldYToViewXSteps[stepIndex] = worldYViewXMultiple >> BACKDRP2_CAMERA_STEP_SHIFT;
		g_cameraWorldYToViewYSteps[stepIndex] = worldYViewYMultiple >> BACKDRP2_CAMERA_STEP_SHIFT;
		g_cameraWorldZToViewYSteps[stepIndex] = worldZViewYMultiple >> BACKDRP2_CAMERA_STEP_SHIFT;
		g_cameraWorldYToViewZSteps[stepIndex] = worldYViewZMultiple >> BACKDRP2_CAMERA_STEP_SHIFT;
		g_cameraWorldZToViewZSteps[stepIndex] = worldZViewZMultiple >> BACKDRP2_CAMERA_STEP_SHIFT;
		g_cameraWorldZToViewXSteps[stepIndex] = worldZViewXMultiple >> BACKDRP2_CAMERA_STEP_SHIFT;
		worldXViewXMultiple += (uint32_t)g_camMatR0_X;
		worldXViewYMultiple += (uint32_t)g_camMatR1_X;
		worldXViewZMultiple += (uint32_t)g_camMatR2_X;
		worldYViewXMultiple += (uint32_t)g_camMatR0_Y;
		worldYViewYMultiple += (uint32_t)g_camMatR1_Y;
		worldYViewZMultiple += (uint32_t)g_camMatR2_Y;
		worldZViewXMultiple += (uint32_t)g_camMatR0_Z;
		worldZViewYMultiple += (uint32_t)g_camMatR1_Z;
		worldZViewZMultiple += (uint32_t)g_camMatR2_Z;
	}
	starDirectionIndex = 0;
	planeZ = (int32_t)(0u - BACKDRP2_STAR_DIRECTION_RADIUS * (uint32_t)g_camMatR2_X);
	planeY = (int32_t)(0u - BACKDRP2_STAR_DIRECTION_RADIUS * (uint32_t)g_camMatR1_X);
	planeX = (int32_t)(0u - BACKDRP2_STAR_DIRECTION_RADIUS * (uint32_t)g_camMatR0_X);
	for (plane = 0; plane < BACKDRP2_STAR_DIRECTION_SIDE; ++plane) {
		int rowZ = (int32_t)((uint32_t)planeZ - BACKDRP2_STAR_DIRECTION_RADIUS * (uint32_t)g_camMatR2_Y);
		int rowY = (int32_t)((uint32_t)planeY - BACKDRP2_STAR_DIRECTION_RADIUS * (uint32_t)g_camMatR1_Y);
		int rowX = (int32_t)((uint32_t)planeX - BACKDRP2_STAR_DIRECTION_RADIUS * (uint32_t)g_camMatR0_Y);
		for (row = 0; row < BACKDRP2_STAR_DIRECTION_SIDE; ++row) {
			int z = (int32_t)((uint32_t)rowZ - BACKDRP2_STAR_DIRECTION_RADIUS * (uint32_t)g_camMatR2_Z);
			int y = (int32_t)((uint32_t)rowY - BACKDRP2_STAR_DIRECTION_RADIUS * (uint32_t)g_camMatR1_Z);
			int x = (int32_t)((uint32_t)rowX - BACKDRP2_STAR_DIRECTION_RADIUS * (uint32_t)g_camMatR0_Z);
			for (column = 0; column < BACKDRP2_STAR_DIRECTION_SIDE; ++column) {
				g_starfieldJitterX[starDirectionIndex] = x >> BACKDRP2_STAR_DIRECTION_SHIFT;
				g_starfieldJitterY[starDirectionIndex] = y >> BACKDRP2_STAR_DIRECTION_SHIFT;
				g_starfieldJitterZ[starDirectionIndex] = z >> BACKDRP2_STAR_DIRECTION_SHIFT;
				x += (uint32_t)g_camMatR0_Z;
				y += (uint32_t)g_camMatR1_Z;
				++starDirectionIndex;
				z += (uint32_t)g_camMatR2_Z;
			}
			rowX += (uint32_t)g_camMatR0_Y;
			rowY += (uint32_t)g_camMatR1_Y;
			rowZ += (uint32_t)g_camMatR2_Y;
		}
		planeX += (uint32_t)g_camMatR0_X;
		planeY += (uint32_t)g_camMatR1_X;
		planeZ += (uint32_t)g_camMatR2_X;
	}
	if (g_backdropsEnabled) {
		int cursor = 0;
		int rotation;
		rotation = (int32_t)(0u - (uint32_t)trig2_arctan(g_camMatR1_X, g_camMatR0_X));
		if (g_camMatR2_Y >= 0) {
			int entries;
			for (entries = g_backdropPositiveYCount; entries != 0; --entries) {
				uint8_t direction = g_backdropPackedDirections[cursor++];
				int magnitude = direction & BACKDRP2_DIRECTION_MAGNITUDE_MASK;
				int viewX = g_cameraWorldXToViewXSteps[magnitude];
				int viewY = g_cameraWorldXToViewYSteps[magnitude];
				int viewZ = g_cameraWorldXToViewZSteps[magnitude];
				if (direction & BACKDRP2_DIRECTION_LOW_NEGATIVE) {
					viewX = (int32_t)(0u - (uint32_t)viewX);
					viewY = (int32_t)(0u - (uint32_t)viewY);
					viewZ = (int32_t)(0u - (uint32_t)viewZ);
				}
				magnitude = (direction >> BACKDRP2_DIRECTION_HIGH_SHIFT) & BACKDRP2_DIRECTION_MAGNITUDE_MASK;
				if (direction & BACKDRP2_DIRECTION_HIGH_NEGATIVE) {
					viewX -= (uint32_t)g_cameraWorldZToViewXSteps[magnitude];
					viewY -= (uint32_t)g_cameraWorldZToViewYSteps[magnitude];
					viewZ -= (uint32_t)g_cameraWorldZToViewZSteps[magnitude];
				} else {
					viewX += (uint32_t)g_cameraWorldZToViewXSteps[magnitude];
					viewY += (uint32_t)g_cameraWorldZToViewYSteps[magnitude];
					viewZ += (uint32_t)g_cameraWorldZToViewZSteps[magnitude];
				}
				viewX += (uint32_t)(g_camMatR0_Y >> BACKDRP2_AXIS_DEPTH_SHIFT);
				viewY += (uint32_t)(g_camMatR1_Y >> BACKDRP2_AXIS_DEPTH_SHIFT);
				viewZ += (uint32_t)(g_camMatR2_Y >> BACKDRP2_AXIS_DEPTH_SHIFT);
				if (viewZ >= 0)
					backdrp2_backdrawbitmap(viewX, viewY, viewZ, rotation, cursor);
			}
			cursor += g_backdropNegativeYCount;
		} else {
			int entries;
			cursor += g_backdropPositiveYCount;
			for (entries = g_backdropNegativeYCount; entries != 0; --entries) {
				uint8_t direction = g_backdropPackedDirections[cursor++];
				int magnitude = direction & BACKDRP2_DIRECTION_MAGNITUDE_MASK;
				int viewX = g_cameraWorldXToViewXSteps[magnitude];
				int viewY = g_cameraWorldXToViewYSteps[magnitude];
				int viewZ = g_cameraWorldXToViewZSteps[magnitude];
				if (direction & BACKDRP2_DIRECTION_LOW_NEGATIVE) {
					viewX = (int32_t)(0u - (uint32_t)viewX);
					viewY = (int32_t)(0u - (uint32_t)viewY);
					viewZ = (int32_t)(0u - (uint32_t)viewZ);
				}
				magnitude = (direction >> BACKDRP2_DIRECTION_HIGH_SHIFT) & BACKDRP2_DIRECTION_MAGNITUDE_MASK;
				if (direction & BACKDRP2_DIRECTION_HIGH_NEGATIVE) {
					viewX -= (uint32_t)g_cameraWorldZToViewXSteps[magnitude];
					viewY -= (uint32_t)g_cameraWorldZToViewYSteps[magnitude];
					viewZ -= (uint32_t)g_cameraWorldZToViewZSteps[magnitude];
				} else {
					viewX += (uint32_t)g_cameraWorldZToViewXSteps[magnitude];
					viewY += (uint32_t)g_cameraWorldZToViewYSteps[magnitude];
					viewZ += (uint32_t)g_cameraWorldZToViewZSteps[magnitude];
				}
				viewX -= (uint32_t)(g_camMatR0_Y >> BACKDRP2_AXIS_DEPTH_SHIFT);
				viewY -= (uint32_t)(g_camMatR1_Y >> BACKDRP2_AXIS_DEPTH_SHIFT);
				viewZ -= (uint32_t)(g_camMatR2_Y >> BACKDRP2_AXIS_DEPTH_SHIFT);
				if (viewZ >= 0)
					backdrp2_backdrawbitmap(viewX, viewY, viewZ, rotation, cursor);
			}
		}
		rotation = (int32_t)(0u - (uint32_t)trig2_arctan(g_camMatR1_Y, g_camMatR0_Y));
		if (g_camMatR2_X >= 0) {
			int entries;
			for (entries = g_backdropPositiveXCount; entries != 0; --entries) {
				uint8_t direction = g_backdropPackedDirections[cursor++];
				int magnitude = direction & BACKDRP2_DIRECTION_MAGNITUDE_MASK;
				int viewX = g_cameraWorldYToViewXSteps[magnitude];
				int viewY = g_cameraWorldYToViewYSteps[magnitude];
				int viewZ = g_cameraWorldYToViewZSteps[magnitude];
				if (direction & BACKDRP2_DIRECTION_LOW_NEGATIVE) {
					viewX = (int32_t)(0u - (uint32_t)viewX);
					viewY = (int32_t)(0u - (uint32_t)viewY);
					viewZ = (int32_t)(0u - (uint32_t)viewZ);
				}
				magnitude = (direction >> BACKDRP2_DIRECTION_HIGH_SHIFT) & BACKDRP2_DIRECTION_MAGNITUDE_MASK;
				if (direction & BACKDRP2_DIRECTION_HIGH_NEGATIVE) {
					viewX -= (uint32_t)g_cameraWorldZToViewXSteps[magnitude];
					viewY -= (uint32_t)g_cameraWorldZToViewYSteps[magnitude];
					viewZ -= (uint32_t)g_cameraWorldZToViewZSteps[magnitude];
				} else {
					viewX += (uint32_t)g_cameraWorldZToViewXSteps[magnitude];
					viewY += (uint32_t)g_cameraWorldZToViewYSteps[magnitude];
					viewZ += (uint32_t)g_cameraWorldZToViewZSteps[magnitude];
				}
				viewX += (uint32_t)(g_camMatR0_X >> BACKDRP2_AXIS_DEPTH_SHIFT);
				viewY += (uint32_t)(g_camMatR1_X >> BACKDRP2_AXIS_DEPTH_SHIFT);
				viewZ += (uint32_t)(g_camMatR2_X >> BACKDRP2_AXIS_DEPTH_SHIFT);
				if (viewZ >= 0)
					backdrp2_backdrawbitmap(viewX, viewY, viewZ, rotation, cursor);
			}
			cursor += g_backdropNegativeXCount;
		} else {
			int entries;
			cursor += g_backdropPositiveXCount;
			for (entries = g_backdropNegativeXCount; entries != 0; --entries) {
				uint8_t direction = g_backdropPackedDirections[cursor++];
				int magnitude = direction & BACKDRP2_DIRECTION_MAGNITUDE_MASK;
				int viewX = g_cameraWorldYToViewXSteps[magnitude];
				int viewY = g_cameraWorldYToViewYSteps[magnitude];
				int viewZ = g_cameraWorldYToViewZSteps[magnitude];
				if (direction & BACKDRP2_DIRECTION_LOW_NEGATIVE) {
					viewX = (int32_t)(0u - (uint32_t)viewX);
					viewY = (int32_t)(0u - (uint32_t)viewY);
					viewZ = (int32_t)(0u - (uint32_t)viewZ);
				}
				magnitude = (direction >> BACKDRP2_DIRECTION_HIGH_SHIFT) & BACKDRP2_DIRECTION_MAGNITUDE_MASK;
				if (direction & BACKDRP2_DIRECTION_HIGH_NEGATIVE) {
					viewX -= (uint32_t)g_cameraWorldZToViewXSteps[magnitude];
					viewY -= (uint32_t)g_cameraWorldZToViewYSteps[magnitude];
					viewZ -= (uint32_t)g_cameraWorldZToViewZSteps[magnitude];
				} else {
					viewX += (uint32_t)g_cameraWorldZToViewXSteps[magnitude];
					viewY += (uint32_t)g_cameraWorldZToViewYSteps[magnitude];
					viewZ += (uint32_t)g_cameraWorldZToViewZSteps[magnitude];
				}
				viewX -= (uint32_t)(g_camMatR0_X >> BACKDRP2_AXIS_DEPTH_SHIFT);
				viewY -= (uint32_t)(g_camMatR1_X >> BACKDRP2_AXIS_DEPTH_SHIFT);
				viewZ -= (uint32_t)(g_camMatR2_X >> BACKDRP2_AXIS_DEPTH_SHIFT);
				if (viewZ >= 0)
					backdrp2_backdrawbitmap(viewX, viewY, viewZ, rotation, cursor);
			}
		}
		rotation = (int32_t)(0u - (uint32_t)trig2_arctan(g_camMatR1_X, g_camMatR0_X));
		if (g_camMatR2_Z >= 0) {
			int entries;
			for (entries = g_backdropPositiveZCount; entries != 0; --entries) {
				uint8_t direction = g_backdropPackedDirections[cursor++];
				int magnitude = direction & BACKDRP2_DIRECTION_MAGNITUDE_MASK;
				int viewX = g_cameraWorldYToViewXSteps[magnitude];
				int viewY = g_cameraWorldYToViewYSteps[magnitude];
				int viewZ = g_cameraWorldYToViewZSteps[magnitude];
				if (direction & BACKDRP2_DIRECTION_LOW_NEGATIVE) {
					viewX = (int32_t)(0u - (uint32_t)viewX);
					viewY = (int32_t)(0u - (uint32_t)viewY);
					viewZ = (int32_t)(0u - (uint32_t)viewZ);
				}
				magnitude = (direction >> BACKDRP2_DIRECTION_HIGH_SHIFT) & BACKDRP2_DIRECTION_MAGNITUDE_MASK;
				if (direction & BACKDRP2_DIRECTION_HIGH_NEGATIVE) {
					viewX -= (uint32_t)g_cameraWorldXToViewXSteps[magnitude];
					viewY -= (uint32_t)g_cameraWorldXToViewYSteps[magnitude];
					viewZ -= (uint32_t)g_cameraWorldXToViewZSteps[magnitude];
				} else {
					viewX += (uint32_t)g_cameraWorldXToViewXSteps[magnitude];
					viewY += (uint32_t)g_cameraWorldXToViewYSteps[magnitude];
					viewZ += (uint32_t)g_cameraWorldXToViewZSteps[magnitude];
				}
				viewX += (uint32_t)(g_camMatR0_Z >> BACKDRP2_AXIS_DEPTH_SHIFT);
				viewY += (uint32_t)(g_camMatR1_Z >> BACKDRP2_AXIS_DEPTH_SHIFT);
				viewZ += (uint32_t)(g_camMatR2_Z >> BACKDRP2_AXIS_DEPTH_SHIFT);
				if (viewZ >= 0)
					backdrp2_backdrawbitmap(viewX, viewY, viewZ, rotation, cursor);
			}
		} else {
			int entries;
			cursor += g_backdropPositiveZCount;
			for (entries = g_backdropNegativeZCount; entries != 0; --entries) {
				uint8_t direction = g_backdropPackedDirections[cursor++];
				int magnitude = direction & BACKDRP2_DIRECTION_MAGNITUDE_MASK;
				int viewX = g_cameraWorldYToViewXSteps[magnitude];
				int viewY = g_cameraWorldYToViewYSteps[magnitude];
				int viewZ = g_cameraWorldYToViewZSteps[magnitude];
				if (direction & BACKDRP2_DIRECTION_LOW_NEGATIVE) {
					viewX = (int32_t)(0u - (uint32_t)viewX);
					viewY = (int32_t)(0u - (uint32_t)viewY);
					viewZ = (int32_t)(0u - (uint32_t)viewZ);
				}
				magnitude = (direction >> BACKDRP2_DIRECTION_HIGH_SHIFT) & BACKDRP2_DIRECTION_MAGNITUDE_MASK;
				if (direction & BACKDRP2_DIRECTION_HIGH_NEGATIVE) {
					viewX -= (uint32_t)g_cameraWorldXToViewXSteps[magnitude];
					viewY -= (uint32_t)g_cameraWorldXToViewYSteps[magnitude];
					viewZ -= (uint32_t)g_cameraWorldXToViewZSteps[magnitude];
				} else {
					viewX += (uint32_t)g_cameraWorldXToViewXSteps[magnitude];
					viewY += (uint32_t)g_cameraWorldXToViewYSteps[magnitude];
					viewZ += (uint32_t)g_cameraWorldXToViewZSteps[magnitude];
				}
				viewX -= (uint32_t)(g_camMatR0_Z >> BACKDRP2_AXIS_DEPTH_SHIFT);
				viewY -= (uint32_t)(g_camMatR1_Z >> BACKDRP2_AXIS_DEPTH_SHIFT);
				viewZ -= (uint32_t)(g_camMatR2_Z >> BACKDRP2_AXIS_DEPTH_SHIFT);
				if (viewZ >= 0)
					backdrp2_backdrawbitmap(viewX, viewY, viewZ, rotation, cursor);
			}
		}
	}
}

// FUNCTION: XW 0x402CC0
void backdrp2_backdrawbitmap(int viewX, int viewY, int viewDepth, int rotationAngle,
							 int backdropIndexPlusOne) {
	int screenOffsetX, screenOffsetY;
	if (viewX < 0) {
		uint32_t magnitude = 0u - (uint32_t)viewX;
		uint64_t numerator;
		uint32_t quotient;
		if ((int32_t)magnitude > viewDepth)
			return;
		numerator = ((uint64_t)magnitude << (g_projPerspectiveShift & BACKDRP2_SHIFT_MASK)) +
					(uint32_t)g_projScaleHalfInt;
		if ((uint32_t)(numerator >> 32) < (uint32_t)viewDepth)
			quotient = (uint32_t)(numerator / (uint32_t)viewDepth);
		else
			quotient = BACKDRP2_PROJECTION_OVERFLOW;
		screenOffsetX = (int32_t)(0u - quotient);
	} else {
		uint64_t numerator;
		if (viewX > viewDepth)
			return;
		numerator = ((uint64_t)(uint32_t)viewX << (g_projPerspectiveShift & BACKDRP2_SHIFT_MASK)) +
					(uint32_t)g_projScaleHalfInt;
		if ((uint32_t)(numerator >> 32) < (uint32_t)viewDepth)
			screenOffsetX = (int32_t)(numerator / (uint32_t)viewDepth);
		else
			screenOffsetX = BACKDRP2_PROJECTION_OVERFLOW;
	}
	if (viewY < 0) {
		uint32_t magnitude = 0u - (uint32_t)viewY;
		uint64_t numerator;
		uint32_t quotient;
		if ((int32_t)magnitude > viewDepth)
			return;
		numerator = ((uint64_t)magnitude << (g_projPerspectiveShift & BACKDRP2_SHIFT_MASK)) +
					(uint32_t)g_projScaleHalfInt;
		if ((uint32_t)(numerator >> 32) < (uint32_t)viewDepth)
			quotient = (uint32_t)(numerator / (uint32_t)viewDepth);
		else
			quotient = BACKDRP2_PROJECTION_OVERFLOW;
		screenOffsetY = (int32_t)(0u - quotient);
	} else {
		uint64_t numerator;
		if (viewY > viewDepth)
			return;
		numerator = ((uint64_t)(uint32_t)viewY << (g_projPerspectiveShift & BACKDRP2_SHIFT_MASK)) +
					(uint32_t)g_projScaleHalfInt;
		if ((uint32_t)(numerator >> 32) < (uint32_t)viewDepth)
			screenOffsetY = (int32_t)(numerator / (uint32_t)viewDepth);
		else
			screenOffsetY = BACKDRP2_PROJECTION_OVERFLOW;
	}
	backdrp2_DrawModelTexQuadAtScreen(
		g_backdropModelTypes[backdropIndexPlusOne - 1],
		(uint16_t)(g_flightVpCenterX + (uint32_t)screenOffsetX),
		(uint16_t)(g_flightVpHeight - (uint32_t)g_projOffsetY - (uint32_t)screenOffsetY - g_flightVpCenterY),
		rotationAngle);
}

// FUNCTION: XW 0x408F20
void backdrp2_DrawModelTexQuadAtScreen(uint16_t objectType, uint16_t screenX, uint16_t screenY,
									   unsigned int rotationAngle) {
	uint8_t* modelBytes;
	XwBitmapModelPrefix* model;
	const uint32_t* frameOffsets;
	XwBitmapFramePrefix* frame;

	g_flightSwRotSpriteSpanRunsEnabled = 1;
	g_camRelWorldZ = BACKDRP2_CAMERA_DEPTH;
	g_objectViewZ = BACKDRP2_VIEW_DEPTH;
	modelBytes = (uint8_t*)Memory_LockHandle(g_modelTypeTable[objectType].memoryHandle);
	nullsub_SharedNoOp();
	model = (XwBitmapModelPrefix*)modelBytes;
	frameOffsets = (const uint32_t*)(modelBytes + model->frameOffsetsOffset);
	frame = (XwBitmapFramePrefix*)(modelBytes + frameOffsets[0]);
	if (g_useHardware3D) {
		RenderQuad_DrawRotatedSprite(rotationAngle, screenX, screenY, 1 << ROTSCALE_FRACTION_BITS, frame);
	} else {
		rotscale_preparefastdraw((uint16_t)rotationAngle);
		rotscale_preparecolor(frame);
		rotscale_rotatescaleimage(screenX, screenY, 1 << ROTSCALE_FRACTION_BITS, frame);
	}
}
