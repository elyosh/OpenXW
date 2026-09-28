#include "xw/render/rotscale.h"

#include "xw/assets/bitmap.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/xw.h"
#include "xw/math/trig2.h"
#include "xw/render/flight_view.h"
#include "xw/render/sw3d.h"

#include <limits.h>
#include <stdlib.h>

// GLOBAL: XW 0x4CEF80
const uint8_t g_rotSpriteRunLengthMaskByFormat[ROTSCALE_RLE_FORMAT_COUNT] = {
	0, 1, 3, 7, 15, 31, 63, 127, 255
};

// GLOBAL: XW 0x4CEF90
const uint8_t g_rotSpriteColorShiftByFormat[ROTSCALE_RLE_FORMAT_COUNT] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };

// GLOBAL: XW 0x4DBF30
const uint16_t g_rotSpriteTangent91[ROTSCALE_TANGENT_91_COUNT] = {
	0,     366,   731,   1097,  1463,  1828,  2194,  2561,  2927,  3293,  3660,  4027,  4395,  4762,
	5131,  5499,  5868,  6237,  6607,  6977,  7348,  7720,  8092,  8464,  8838,  9212,  9586,  9962,
	10338, 10715, 11093, 11471, 11851, 12231, 12613, 12995, 13379, 13763, 14149, 14536, 14924, 15313,
	15703, 16095, 16488, 16882, 17277, 17674, 18073, 18473, 18874, 19277, 19682, 20088, 20496, 20906,
	21317, 21731, 22146, 22563, 22982, 23403, 23826, 24251, 24678, 25107, 25539, 25973, 26409, 26848,
	27289, 27732, 28178, 28627, 29078, 29532, 29989, 30449, 30911, 31377, 31845, 32317, 32791, 33269,
	33751, 34235, 34723, 35215, 35710, 36209, 36711, 37217, 37727, 38242, 38760, 39282, 39809, 40340,
	40875, 41415, 41960, 42509, 43063, 43622, 44186, 44755, 45330, 45910, 46495, 47086, 47683, 48286,
	48895, 49509, 50131, 50758, 51392, 52033, 52681, 53336, 53999, 54668, 55345, 56030, 56723, 57424,
	58134, 58852, 59578, 60314, 61059, 61813, 62577, 63351, 64135, 64929, 65535,
};

// GLOBAL: XW 0x4DC048
const uint16_t g_rotSpriteTangent100[ROTSCALE_TANGENT_100_COUNT] = {
	0,     402,   804,   1206,  1608,  2011,  2414,  2817,  2927,  3293,  3660,  4027,  4395,  4762,
	5131,  5499,  5868,  6237,  6607,  6977,  7348,  7720,  8092,  8464,  8838,  9212,  9586,  9962,
	10338, 10715, 11093, 11471, 11851, 12231, 12613, 12995, 13379, 13763, 14149, 14536, 14924, 15313,
	15703, 16095, 16488, 16882, 17277, 17674, 18073, 18473, 18874, 19277, 19682, 20088, 20496, 20906,
	21317, 21731, 22146, 22563, 22982, 23403, 23826, 24251, 24678, 25107, 25539, 25973, 26409, 26848,
	27289, 27732, 28178, 28627, 29078, 29532, 29989, 30449, 30911, 31377, 31845, 32317, 32791, 33269,
	33751, 34235, 34723, 35215, 35710, 36209, 36711, 37217, 37727, 38242, 38760, 39282, 39809, 40340,
	40875, 41415, 41960, 42509, 43063, 43622, 44186, 44755, 45330, 45910, 46495, 47086, 47683, 48286,
	48895, 49509, 50131, 50758, 51392, 52033, 52681, 53336, 53999, 54668, 55345, 56030, 56723, 57424,
	58134, 58852, 59578, 60314, 61059, 61813, 62577, 63351, 64135, 64929, 65535,
};

// GLOBAL: XW 0x4DC160
const uint16_t g_rotSpriteTangent110[ROTSCALE_TANGENT_110_COUNT] = {
	0,     442,   885,   1327,  1770,  2212,  2655,  3098,  3542,  3985,  4429,  4873,  5318,  5763,
	6208,  6654,  7100,  7547,  7995,  8443,  8891,  9341,  9791,  10242, 10693, 11146, 11599, 12054,
	12509, 12965, 13422, 13880, 14340, 14800, 15261, 15724, 16188, 16654, 17120, 17588, 18058, 18528,
	19001, 19474, 19950, 20427, 20906, 21386, 21868, 22352, 22838, 23326, 23815, 24307, 24800, 25296,
	25794, 26294, 26796, 27301, 27808, 28317, 28829, 29344, 29860, 30380, 30902, 31427, 31955, 32486,
	33019, 33556, 34096, 34639, 35185, 35734, 36287, 36843, 37403, 37966, 38533, 39103, 39678, 40256,
	40838, 41425, 42015, 42610, 43209, 43812, 44420, 45033, 45650, 46272, 46899, 47532, 48169, 48811,
	49459, 50112, 50771, 51436, 52106, 52783, 53465, 54154, 54849, 55551, 56259, 56974, 57697, 58426,
	59162, 59906, 60658, 61417, 62185, 62961, 63744, 64537, 65338, 65535,
};

// GLOBAL: XW 0x4DC254
int g_flightSwRotSpriteSpanRunsEnabled = 1;

// GLOBAL: XW 0x561D18
int g_flightSwRotSpriteCoeffsValid = 0;

// GLOBAL: XW 0x561D1C
int g_flightSwRotSpriteSquarePixelMode = 0;

// GLOBAL: XW 0x5B8B60
XwRotSpriteSpanRun g_flightSwRotSpriteSpanRuns[ROTSCALE_SPAN_RUN_CAPACITY] = { 0 };

// GLOBAL: XW 0x5BA360
int16_t g_flightSwRotSpriteClipMinRunIdx03 = 0;

// GLOBAL: XW 0x5BA362
int16_t g_flightSwRotSpritePointOutputY = 0;

// GLOBAL: XW 0x5BA364
int16_t g_flightSwRotSpritePointOutputX = 0;

// GLOBAL: XW 0x5BA366
int16_t g_flightSwRotSpriteSecondaryEdgeY = 0;

// GLOBAL: XW 0x5BA368
int16_t g_flightSwRotSpriteSecondaryEdgeX = 0;

// GLOBAL: XW 0x5BA36C
uint8_t* g_flightSwRotSpriteDestLinePtr = NULL;

// GLOBAL: XW 0x5BA380
uint8_t g_flightSwRotSpriteTintTable[ROTSCALE_TINT_TABLE_ENTRIES] = { 0 };

// GLOBAL: XW 0x5BA480
int16_t g_flightSwRotSpriteEdgeCursorX = 0;

// GLOBAL: XW 0x5BA482
int16_t g_flightSwRotSpriteEdgeCursorY = 0;

// GLOBAL: XW 0x5BA484
int16_t g_flightSwRotSpritePointInputX = 0;

// GLOBAL: XW 0x5BA486
int16_t g_flightSwRotSpritePointInputY = 0;

// GLOBAL: XW 0x5BA488
int g_flightSwRotSpriteDestPitchBytes = 0;

// GLOBAL: XW 0x5BA48C
int16_t g_flightSwRotSpriteSkipSecondaryScaleStep = 0;

// GLOBAL: XW 0x5BA4A0
FlightSwRotSpriteCoeffState g_flightSwRotSpriteCoeffStorage = { 0 };

// GLOBAL: XW 0x5BE35C
int16_t g_flightSwRotSpriteClipMaxX = 0;

// GLOBAL: XW 0x5BE360
uint8_t g_flightSwRotSpriteTintHiTable[ROTSCALE_TINT_TABLE_ENTRIES] = { 0 };

// GLOBAL: XW 0x5BE460
uint16_t g_flightSwRotSpriteSecondaryScaleAccum = 0;

// GLOBAL: XW 0x5BE462
int16_t g_flightSwRotSpriteClipMaxRunIdx03 = 0;

// GLOBAL: XW 0x5BE480
uint8_t g_flightSwRotSpriteTintLoTable[ROTSCALE_TINT_TABLE_ENTRIES] = { 0 };

// GLOBAL: XW 0x5BE580
int16_t g_flightSwRotSpriteViewportMaxX = 0;

// GLOBAL: XW 0x5BE582
int16_t g_flightSwRotSpriteClipMaxRunIdx47 = 0;

// GLOBAL: XW 0x5BE584
int g_flightSwRotSpriteSpanRunCountdown = 0;

// GLOBAL: XW 0x5BE588
int16_t g_flightSwRotSpriteClipMinRunIdx47 = 0;

// GLOBAL: XW 0x5BE58A
int16_t g_flightSwRotSpriteViewportWidth = 0;

// GLOBAL: XW 0x5BE58C
int16_t g_flightSwRotSpriteDestYMode = 0;

// GLOBAL: XW 0x5BE58E
int16_t g_flightSwRotSpriteSavedClipMinX = 0;

// GLOBAL: XW 0x5BE590
int16_t g_flightSwRotSpriteSavedClipMaxX = 0;

// GLOBAL: XW 0x5BE592
int16_t g_flightSwRotSpritePrimaryEdgeY = 0;

// GLOBAL: XW 0x5BE594
int16_t g_flightSwRotSpritePrimaryEdgeX = 0;

// GLOBAL: XW 0x5BE596
int16_t g_flightSwRotSpriteClipMinX = 0;

// GLOBAL: XW 0x5BE598
int16_t g_flightSwRotSpriteSpanBaseX = 0;

// GLOBAL: XW 0x5BE5A0
XwRotSpriteScaleState g_rotSpriteScaleState = { 0 };

// GLOBAL: XW 0x5BE9AC
int16_t g_flightSwRotSpriteViewportMaxY = 0;

// GLOBAL: XW 0x5BE9B0
FlightSwRotSpriteCoeffState* g_flightSwRotSpriteCoeffs = NULL;

// GLOBAL: XW 0x5BE9B4
int16_t g_flightSwRotSpriteSavedPrimaryEdgeY = 0;

// GLOBAL: XW 0x5BE9B6
int16_t g_flightSwRotSpriteSavedPrimaryEdgeX = 0;

// GLOBAL: XW 0x5BE9B8
int16_t g_flightSwRotSpriteViewportHeight = 0;

// GLOBAL: XW 0x5BE9BA
uint16_t g_flightSwRotSpriteAxisSwapThreshold = 0;

// GLOBAL: XW 0x5BECE4
uint8_t* g_flightSwRotSpriteDestBuffer = NULL;

// FUNCTION: XW 0x491FD0
void rotscale_BlitPreparedRotatedSpriteSpans(uint8_t* pixels, int rowSkipBytes, int startX, int startY,
											 int endX, int endY) {
	float reciprocalDepth;
	ptrdiff_t byteIndex = 0;
	int y;
	if (g_flightSurfaceAlreadyLocked == 0)
		FlightDisplay_LockSurface();
	reciprocalDepth = (float)(unsigned int)g_projScaleInt / (float)g_objectViewZ;
	if (g_flightBytesPerPixel == sizeof(uint16_t)) {
		for (y = startY; y < endY; ++y) {
			int x;
			for (x = startX; x < endX;) {
				int runStartX;
				const uint8_t* runPixels;
				for (; x < endX; byteIndex += sizeof(uint16_t), ++x) {
					if (pixels[byteIndex + 1] == ROTSCALE_INDEXED_PIXEL_TAG)
						break;
				}
				if (x == endX)
					break;
				runStartX = x;
				runPixels = &pixels[byteIndex];
				for (; x < endX; byteIndex += sizeof(uint16_t), ++x) {
					int tintIndex;
					if (pixels[byteIndex + 1] != ROTSCALE_INDEXED_PIXEL_TAG)
						break;
					tintIndex = pixels[byteIndex];
					pixels[byteIndex + 1] = g_flightSwRotSpriteTintHiTable[tintIndex];
					pixels[byteIndex] = g_flightSwRotSpriteTintLoTable[tintIndex];
				}
				sw3d_BlitOccludedSpan(runPixels, runStartX, x, y, reciprocalDepth);
			}
			byteIndex += rowSkipBytes;
		}
	} else {
		for (y = startY; y < endY; ++y) {
			int x;
			for (x = startX; x < endX;) {
				int runStartX;
				const uint8_t* runPixels;
				for (; x < endX; ++byteIndex, ++x) {
					if (pixels[byteIndex] < ROTSCALE_INDEXED_TINT_COUNT)
						break;
				}
				if (x == endX)
					break;
				runStartX = x;
				runPixels = &pixels[byteIndex];
				for (; x < endX; ++byteIndex, ++x) {
					int tintIndex;
					if (pixels[byteIndex] >= ROTSCALE_INDEXED_TINT_COUNT)
						break;
					tintIndex = pixels[byteIndex];
					pixels[byteIndex] = g_flightSwRotSpriteTintTable[tintIndex];
				}
				sw3d_BlitOccludedSpan(runPixels, runStartX, x, y, reciprocalDepth);
			}
			byteIndex += rowSkipBytes;
		}
	}
	if (g_flightSurfaceAlreadyLocked == 0)
		FlightDisplay_UnlockSurface();
}

// FUNCTION: XW 0x49DF00
void rotscale_SetRotatedSpriteDestBuffer(uint8_t* bufferAddress) {
	g_flightSwRotSpriteDestBuffer = bufferAddress;
}

// FUNCTION: XW 0x4A2CB0
void rotscale_DrawRotSpriteSpanRuns8(const struct XwRotSpriteSpanRun* runs, uint8_t* destination,
									 const int* spanOffsets) {
	int runIndex;
	for (runIndex = 0;; ++runIndex) {
		int start = runs[runIndex].start + g_flightSwRotSpriteSpanBaseX;
		unsigned int color = runs[runIndex].colorIndex;
		int length = runs[runIndex].length;
		int pixelIndex;
		for (pixelIndex = 0; pixelIndex < length; ++pixelIndex) {
			int destinationOffset = spanOffsets[start + pixelIndex];
			destination[destinationOffset] = (uint8_t)color;
		}
		if (--g_flightSwRotSpriteSpanRunCountdown == 0) {
			break;
		}
	}
}

// FUNCTION: XW 0x4A2D00
void rotscale_DrawClippedRotSpriteSpanRuns8(const struct XwRotSpriteSpanRun* runs, uint8_t* destination,
											const int* spanOffsets) {
	int runIndex;
	for (runIndex = 0;; ++runIndex) {
		int spanStart = runs[runIndex].start + g_flightSwRotSpriteSpanBaseX;
		int spanEnd = spanStart + runs[runIndex].length;
		int clipMin = g_flightSwRotSpriteClipMinX;
		int clipMax = g_flightSwRotSpriteClipMaxX;
		if (spanStart < clipMin) {
			spanStart = clipMin;
		}
		if (spanStart <= clipMax && spanEnd >= clipMin) {
			int pixelsRemaining;
			if (spanEnd > clipMax) {
				spanEnd = clipMax;
			}
			pixelsRemaining = spanEnd - spanStart;
			if (pixelsRemaining != 0) {
				unsigned int color = runs[runIndex].colorIndex;
				int pixelIndex;
				for (pixelIndex = spanStart; pixelsRemaining != 0; ++pixelIndex, --pixelsRemaining) {
					destination[spanOffsets[pixelIndex]] = (uint8_t)color;
				}
			}
		}
		if (--g_flightSwRotSpriteSpanRunCountdown == 0) {
			break;
		}
	}
}

// FUNCTION: XW 0x4A2D70
void rotscale_DrawRotSpriteSpanRuns16(const struct XwRotSpriteSpanRun* runs, uint8_t* destination,
									  const int* spanOffsets) {
	int runIndex;
	for (runIndex = 0;; ++runIndex) {
		uint16_t color =
			(uint8_t)runs[runIndex].colorIndex | ((uint16_t)ROTSCALE_INDEXED_PIXEL_TAG << CHAR_BIT);
		int length = runs[runIndex].length;
		int start = runs[runIndex].start + g_flightSwRotSpriteSpanBaseX;
		int pixelIndex;
		for (pixelIndex = 0; pixelIndex < length; ++pixelIndex) {
			int destinationOffset = spanOffsets[start + pixelIndex];
			destination[destinationOffset] = (uint8_t)color;
			destination[destinationOffset + 1] = (uint8_t)(color >> CHAR_BIT);
		}
		if (--g_flightSwRotSpriteSpanRunCountdown == 0) {
			break;
		}
	}
}

// FUNCTION: XW 0x4A2DC0
void rotscale_DrawClippedRotSpriteSpanRuns16(const struct XwRotSpriteSpanRun* runs, uint8_t* destination,
											 const int* spanOffsets) {
	int runIndex;
	for (runIndex = 0;; ++runIndex) {
		int spanStart = runs[runIndex].start + g_flightSwRotSpriteSpanBaseX;
		int spanEnd = spanStart + runs[runIndex].length;
		int clipMin = g_flightSwRotSpriteClipMinX;
		int clipMax = g_flightSwRotSpriteClipMaxX;
		if (spanStart < clipMin) {
			spanStart = clipMin;
		}
		if (spanStart <= clipMax && spanEnd >= clipMin) {
			int pixelsRemaining;
			if (spanEnd > clipMax) {
				spanEnd = clipMax;
			}
			pixelsRemaining = spanEnd - spanStart;
			if (pixelsRemaining != 0) {
				unsigned int color = runs[runIndex].colorIndex;
				int pixelIndex;
				for (pixelIndex = spanStart; pixelsRemaining != 0; ++pixelIndex, --pixelsRemaining) {
					int destinationOffset = spanOffsets[pixelIndex];
					destination[destinationOffset] = (uint8_t)color;
					destination[destinationOffset + 1] = ROTSCALE_INDEXED_PIXEL_TAG;
				}
			}
		}
		if (--g_flightSwRotSpriteSpanRunCountdown == 0) {
			break;
		}
	}
}

// FUNCTION: XW 0x4A2E30
void rotscale_rotatescaleimage(int16_t screenX, int16_t screenY, uint16_t screenScale,
							   const struct XwBitmapFramePrefix* frame) {
	const XwBitmapSpriteHeader* spriteHeader =
		(const XwBitmapSpriteHeader*)((const uint8_t*)frame + frame->spriteOffset);
	int16_t originX;
	int16_t originY;
	int16_t rightX;
	int quadXY[ROTSCALE_QUAD_COORDINATE_COUNT];
	if (g_flightSwRotSpriteSpanRunsEnabled == 1)
		originX = spriteHeader->originX;
	else
		originX = -spriteHeader->flippedOriginX;
	g_flightSwRotSpritePointInputX = originX;
	originY = -spriteHeader->originY;
	g_flightSwRotSpritePointInputY = originY;
	rotscale_scalesetup(screenScale, g_flightSwRotSpriteCoeffs, &g_rotSpriteScaleState);
	rotscale_adjustoffsets(g_flightSwRotSpriteCoeffs, &g_rotSpriteScaleState);
	g_flightSwRotSpriteEdgeCursorX = g_flightSwRotSpritePointOutputX + screenX;
	quadXY[0] = (int16_t)(g_flightSwRotSpritePointOutputX + screenX);
	g_flightSwRotSpriteEdgeCursorY = g_flightSwRotSpritePointOutputY + screenY;
	quadXY[1] = (int16_t)(g_flightSwRotSpritePointOutputY + screenY);
	rotscale_rotatescale((uint8_t*)spriteHeader + sizeof(*spriteHeader), frame->rleFormatIndex);
	rightX = (int16_t)frame->width + originX;
	g_flightSwRotSpritePointInputY = originY;
	g_flightSwRotSpritePointInputX = rightX;
	rotscale_adjustoffsets(g_flightSwRotSpriteCoeffs, &g_rotSpriteScaleState);
	quadXY[2] = screenX + g_flightSwRotSpritePointOutputX;
	quadXY[3] = screenY + g_flightSwRotSpritePointOutputY;
	g_flightSwRotSpritePointInputX = (int16_t)frame->width + originX;
	g_flightSwRotSpritePointInputY = originY - (int16_t)frame->height;
	rotscale_adjustoffsets(g_flightSwRotSpriteCoeffs, &g_rotSpriteScaleState);
	g_flightSwRotSpritePointInputX = originX;
	quadXY[4] = screenX + g_flightSwRotSpritePointOutputX;
	quadXY[5] = screenY + g_flightSwRotSpritePointOutputY;
	g_flightSwRotSpritePointInputY = originY - (int16_t)frame->height;
	rotscale_adjustoffsets(g_flightSwRotSpriteCoeffs, &g_rotSpriteScaleState);
	quadXY[6] = screenX + g_flightSwRotSpritePointOutputX;
	quadXY[7] = screenY + g_flightSwRotSpritePointOutputY;
	rotscale_ClipAndBlitPreparedRotatedSprite(quadXY);
}

// FUNCTION: XW 0x4A2FD0
void rotscale_ClipAndBlitPreparedRotatedSprite(int* quadXY) {
	int minX, maxX, minY, maxY;
	quadXY[1] = g_flightVpMaxY - quadXY[1];
	quadXY[3] = g_flightVpMaxY - quadXY[3];
	quadXY[5] = g_flightVpMaxY - quadXY[5];
	quadXY[7] = g_flightVpMaxY - quadXY[7];
	minX = maxX = quadXY[0];
	minY = maxY = quadXY[1];
	if (quadXY[2] < minX)
		minX = quadXY[2];
	if (quadXY[4] < minX)
		minX = quadXY[4];
	if (quadXY[6] < minX)
		minX = quadXY[6];
	if (quadXY[2] > maxX)
		maxX = quadXY[2];
	if (quadXY[4] > maxX)
		maxX = quadXY[4];
	if (quadXY[6] > maxX)
		maxX = quadXY[6];
	if (quadXY[3] < minY)
		minY = quadXY[3];
	if (quadXY[5] < minY)
		minY = quadXY[5];
	if (quadXY[7] < minY)
		minY = quadXY[7];
	if (quadXY[3] > maxY)
		maxY = quadXY[3];
	if (quadXY[5] > maxY)
		maxY = quadXY[5];
	if (quadXY[7] > maxY)
		maxY = quadXY[7];
	minX -= ROTSCALE_CLIP_MARGIN;
	minY -= ROTSCALE_CLIP_MARGIN;
	maxX += ROTSCALE_CLIP_MARGIN;
	maxY += ROTSCALE_CLIP_MARGIN;
	if (maxY >= 0 && minY < g_flightSwRotSpriteViewportHeight) {
		if (maxY >= g_flightSwRotSpriteViewportHeight)
			maxY = g_flightSwRotSpriteViewportMaxY;
		if (minY < 0)
			minY = 0;
		if (maxX >= 0 && minX < g_flightSwRotSpriteViewportWidth) {
			if (maxX >= g_flightSwRotSpriteViewportWidth)
				maxX = g_flightSwRotSpriteViewportMaxX;
			if (minX < 0)
				minX = 0;
			rotscale_BlitPreparedRotatedSpriteSpans(
				g_flightSwRotSpriteDestBuffer + g_flightBytesPerPixel * minX +
					g_flightSwRotSpriteDestPitchBytes * minY,
				g_flightSwRotSpriteDestPitchBytes - g_flightBytesPerPixel * (maxX - minX), minX, minY, maxX,
				maxY);
		}
	}
}

// FUNCTION: XW 0x4A3110
void rotscale_preparefastdraw(uint16_t rotationAngle) {
	g_flightSwRotSpriteViewportHeight = g_flightVpHeight;
	g_flightSwRotSpriteViewportWidth = g_flightVpWidth;
	g_flightSwRotSpriteDestLinePtr = g_flightSwRotSpriteDestBuffer;
	g_flightSwRotSpriteDestPitchBytes = g_flightBytesPerPixel * g_flightVpWidth;
	g_flightSwRotSpriteViewportMaxX = g_flightVpMaxX;
	g_flightSwRotSpriteSquarePixelMode = g_projAspectY == 0;
	g_flightSwRotSpriteViewportMaxY = g_flightVpMaxY;
	g_flightSwRotSpriteDestYMode = -1;
	g_flightSwRotSpriteAxisSwapThreshold = g_flightSwRotSpriteSquarePixelMode == ROTSCALE_SQUARE_PIXEL_MODE
											   ? ROTSCALE_SQUARE_AXIS_SWAP_ANGLE
											   : ROTSCALE_RECTANGULAR_AXIS_SWAP_ANGLE;
	g_flightSwRotSpriteCoeffs = &g_flightSwRotSpriteCoeffStorage;
	if (g_flightSwRotSpriteCoeffStorage.rotationAngle != rotationAngle ||
		g_flightSwRotSpriteCoeffsValid == 0) {
		rotscale_buildlinedata(rotationAngle, &g_flightSwRotSpriteCoeffStorage);
		g_flightSwRotSpriteCoeffsValid = 1;
	}
}

// FUNCTION: XW 0x4A31D0
void rotscale_preparecolor(const struct XwBitmapFramePrefix* frame) {
	const uint8_t* palette = (const uint8_t*)frame + frame->paletteOffset;
	int count = frame->paletteEntryCount;
	if (g_flightBytesPerPixel == sizeof(uint16_t)) {
		int index;
		int paletteIndex = 0;
		for (index = 0; index < count; ++index) {
			uint8_t lowByte = palette[paletteIndex++];
			uint8_t highByte;
			g_flightSwRotSpriteTintLoTable[index] = lowByte;
			highByte = palette[paletteIndex++];
			g_flightSwRotSpriteTintHiTable[index] = highByte;
		}
	} else {
		int index;
		for (index = 0; index < count; ++index) {
			g_flightSwRotSpriteTintTable[index] = palette[index];
		}
	}
}

// FUNCTION: XW 0x4A3220
uint16_t rotscale_GetUpdateIncrement(uint16_t foldedAngle, int16_t aspectSelector) {
	uint16_t index = foldedAngle >> ROTSCALE_TANGENT_ANGLE_SHIFT;
	if (aspectSelector == ROTSCALE_ASPECT_91)
		return g_rotSpriteTangent91[index];
	if (aspectSelector == ROTSCALE_ASPECT_110)
		return g_rotSpriteTangent110[index];
	return g_rotSpriteTangent100[index];
}

// FUNCTION: XW 0x4A3270
void rotscale_scalesetup(uint16_t scaleQ8, const struct FlightSwRotSpriteCoeffState* coeffs,
						 struct XwRotSpriteScaleState* state) {
	unsigned int primaryStep;
	unsigned int unadjustedPrimaryStep;
	unsigned int secondaryStep;
	state->scaleQ8 = scaleQ8;
	if (g_flightSwRotSpriteSquarePixelMode == ROTSCALE_SQUARE_PIXEL_MODE) {
		state->aspectYQ8 = ROTSCALE_SQUARE_ASPECT_Q8;
		state->aspectXQ8 = ROTSCALE_SQUARE_ASPECT_Q8;
		g_flightSwRotSpriteAxisSwapThreshold = ROTSCALE_SQUARE_AXIS_SWAP_ANGLE;
	} else {
		state->aspectYQ8 = ROTSCALE_RECTANGULAR_ASPECT_Y_Q8;
		state->aspectXQ8 = ROTSCALE_RECTANGULAR_ASPECT_X_Q8;
		g_flightSwRotSpriteAxisSwapThreshold = ROTSCALE_RECTANGULAR_AXIS_SWAP_ANGLE;
	}
	primaryStep = ((unsigned int)scaleQ8 * coeffs->primaryCosQ15) >> ROTSCALE_PRODUCT_FRACTION_BITS;
	state->primaryStepQ8 = primaryStep;
	unadjustedPrimaryStep = primaryStep;
	if (coeffs->primaryAxisSwap != 0) {
		primaryStep = (state->aspectYQ8 * primaryStep) >> ROTSCALE_FRACTION_BITS;
		state->primaryStepQ8 = primaryStep;
	}
	secondaryStep = ((unadjustedPrimaryStep * coeffs->secondaryStepByte) >> ROTSCALE_FRACTION_BITS) +
					unadjustedPrimaryStep;
	if (coeffs->secondaryAxisSwap == 0) {
		secondaryStep = (state->aspectXQ8 * secondaryStep) >> ROTSCALE_FRACTION_BITS;
	}
	state->secondaryStepQ8 = secondaryStep;
	if ((uint16_t)primaryStep != state->cachedPrimaryStepQ8) {
		unsigned int increment = (unsigned int)(uint16_t)primaryStep << ROTSCALE_FRACTION_BITS;
		unsigned int accumulator = increment;
		unsigned int index;
		state->cachedPrimaryStepQ8 = primaryStep;
		for (index = 0; index < ROTSCALE_SCALE_TABLE_COUNT; ++index) {
			state->stepFraction[index] = accumulator;
			state->stepInteger[index] = accumulator >> ROTSCALE_PRODUCT_FRACTION_BITS;
			accumulator += increment;
		}
	}
}

// FUNCTION: XW 0x4A3380
void rotscale_adjustoffsets(const struct FlightSwRotSpriteCoeffState* coeffs,
							const struct XwRotSpriteScaleState* scaleState) {
	int16_t inputX = g_flightSwRotSpritePointInputX;
	int16_t inputY;
	int16_t originalX = inputX;
	int16_t originalY;
	uint16_t scaledX;
	uint16_t scaledY;
	int aspectScaledY;
	uint32_t xTerm;
	uint32_t yTerm;
	int16_t rotatedY;
	int16_t outputY;
	int finalY;
	if (inputX < 0) {
		inputX = -inputX;
		g_flightSwRotSpritePointInputX = inputX;
	}
	scaledX = (inputX * scaleState->scaleQ8 + ROTSCALE_SCALE_ROUNDING) >> ROTSCALE_FRACTION_BITS;
	inputY = g_flightSwRotSpritePointInputY;
	originalY = inputY;
	if (inputY < 0) {
		inputY = -inputY;
		g_flightSwRotSpritePointInputY = inputY;
	}
	aspectScaledY = (inputY * scaleState->aspectXQ8 + ROTSCALE_SCALE_ROUNDING) >> ROTSCALE_FRACTION_BITS;
	scaledY = (int32_t)((uint32_t)aspectScaledY * scaleState->scaleQ8 + ROTSCALE_SCALE_ROUNDING) >>
			  ROTSCALE_FRACTION_BITS;
	xTerm = (uint32_t)coeffs->cosQ15 * scaledX;
	if (((coeffs->cosSignMask ^ (uint16_t)originalX) & ROTSCALE_COORDINATE_SIGN) != 0)
		xTerm = 0u - xTerm;
	yTerm = (uint32_t)coeffs->sinQ15 * scaledY;
	if (((coeffs->sinSignMask ^ (uint16_t)originalY) & ROTSCALE_COORDINATE_SIGN) != 0)
		yTerm = 0u - yTerm;
	g_flightSwRotSpritePointOutputX =
		(int32_t)(xTerm + yTerm + ROTSCALE_PRODUCT_ROUNDING) >> ROTSCALE_PRODUCT_FRACTION_BITS;
	xTerm = (uint32_t)scaledX * coeffs->sinQ15;
	if (((coeffs->sinSignMask ^ (uint16_t)originalX) & ROTSCALE_COORDINATE_SIGN) != 0)
		xTerm = 0u - xTerm;
	yTerm = (uint32_t)coeffs->cosQ15 * scaledY;
	if (((coeffs->cosSignMask ^ (uint16_t)originalY) & ROTSCALE_COORDINATE_SIGN) == 0)
		yTerm = 0u - yTerm;
	outputY = (int32_t)(xTerm + yTerm + ROTSCALE_PRODUCT_ROUNDING) >> ROTSCALE_PRODUCT_FRACTION_BITS;
	g_flightSwRotSpritePointOutputY = outputY;
	rotatedY = outputY;
	if (outputY < 0) {
		outputY = -outputY;
		g_flightSwRotSpritePointOutputY = outputY;
	}
	finalY = (outputY * scaleState->aspectYQ8 + ROTSCALE_SCALE_ROUNDING) >> ROTSCALE_FRACTION_BITS;
	if (rotatedY < 0)
		finalY = -finalY;
	g_flightSwRotSpritePointOutputY = finalY;
}

// FUNCTION: XW 0x4A34D0
void rotscale_buildlinedata(uint16_t rotationAngle, struct FlightSwRotSpriteCoeffState* coeffs) {
	uint16_t xDirection, yDirection;
	unsigned int foldedAngle, primaryAngle;
	uint16_t tangent;
	uint16_t primarySwap, secondarySwap;
	uint16_t index, runIndex, remaining;
	unsigned int reciprocal;
	coeffs->sinSignMask = rotationAngle & TRIG2_ANGLE_SIGN_BIT;
	coeffs->cosSignMask = (rotationAngle + TRIG2_QUARTER_TURN) & TRIG2_ANGLE_SIGN_BIT;
	coeffs->rotationAngle = rotationAngle;
	xDirection = 0;
	yDirection = 0;
	if (rotationAngle >= TRIG2_ANGLE_SIGN_BIT) {
		rotationAngle &= TRIG2_ANGLE_SIGN_BIT - 1;
		yDirection = ROTSCALE_REVERSE_Y;
		if (rotationAngle < TRIG2_QUARTER_TURN)
			xDirection = ROTSCALE_REVERSE_X;
	} else if (rotationAngle >= TRIG2_QUARTER_TURN) {
		xDirection = ROTSCALE_REVERSE_X;
	}
	coeffs->yDirectionFlag = yDirection;
	coeffs->xDirectionFlag = xDirection;
	foldedAngle = rotationAngle;
	if (foldedAngle >= TRIG2_QUARTER_TURN)
		foldedAngle = TRIG2_ANGLE_SIGN_BIT - foldedAngle;
	coeffs->sinQ15 = rotscale_LookupSpriteSineQ15(foldedAngle);
	coeffs->cosQ15 = rotscale_LookupSpriteSineQ15(foldedAngle + TRIG2_QUARTER_TURN);
	primaryAngle = foldedAngle;
	if (primaryAngle < g_flightSwRotSpriteAxisSwapThreshold) {
		primarySwap = 0;
		coeffs->primaryAxisSwap = primarySwap;
		if (g_flightSwRotSpriteSquarePixelMode == ROTSCALE_SQUARE_PIXEL_MODE)
			tangent = rotscale_GetUpdateIncrement(primaryAngle, ROTSCALE_ASPECT_100);
		else
			tangent = rotscale_GetUpdateIncrement(primaryAngle, ROTSCALE_ASPECT_91);
	} else {
		primarySwap = ROTSCALE_SWAP_AXES;
		primaryAngle = TRIG2_QUARTER_TURN - primaryAngle;
		coeffs->primaryAxisSwap = primarySwap;
		if (g_flightSwRotSpriteSquarePixelMode == ROTSCALE_SQUARE_PIXEL_MODE)
			tangent = rotscale_GetUpdateIncrement(primaryAngle, ROTSCALE_ASPECT_100);
		else
			tangent = rotscale_GetUpdateIncrement(primaryAngle, ROTSCALE_ASPECT_110);
	}
	coeffs->primaryCosQ15 = rotscale_LookupSpriteSineQ15(primaryAngle + TRIG2_QUARTER_TURN);
	reciprocal = (1u << 31) / coeffs->primaryCosQ15;
	if (primarySwap != 0 && g_flightSwRotSpriteSquarePixelMode == 0)
		reciprocal = (g_projAspectY * (unsigned int)(uint16_t)reciprocal + ROTSCALE_PRODUCT_ROUNDING) >>
					 ROTSCALE_PRODUCT_FRACTION_BITS;
	coeffs->primaryStepReciprocal = reciprocal;
	coeffs->edgePoints[0].x = 0;
	coeffs->edgePoints[0].y = 0;
	if (primarySwap == 0) {
		uint16_t fraction = ROTSCALE_PRODUCT_ROUNDING;
		int16_t x = 0, y = 0;
		uint16_t steps;
		coeffs->scanCount = g_flightSwRotSpriteViewportWidth;
		steps = (uint16_t)(coeffs->scanCount - 1);
		for (index = 1; steps != 0; ++index, --steps) {
			uint16_t previous = fraction;
			fraction += tangent;
			++x;
			if (fraction < previous)
				++y;
			coeffs->edgePoints[index].x = x;
			coeffs->edgePoints[index].y = y;
		}
	} else {
		uint16_t fraction = ROTSCALE_PRODUCT_ROUNDING;
		int16_t x = 0, y = 0;
		uint16_t steps;
		coeffs->scanCount = g_flightSwRotSpriteViewportHeight;
		steps = (uint16_t)(coeffs->scanCount - 1);
		for (index = 1; steps != 0; ++index, --steps) {
			uint16_t previous = fraction;
			fraction += tangent;
			++y;
			if (fraction < previous)
				++x;
			coeffs->edgePoints[index].x = x;
			coeffs->edgePoints[index].y = y;
		}
	}
	remaining = coeffs->scanCount;
	runIndex = 0;
	for (index = 0; remaining > 0; ++index) {
		uint16_t length = 1;
		int16_t coordinate =
			coeffs->primaryAxisSwap == 0 ? coeffs->edgePoints[index].y : coeffs->edgePoints[index].x;
		--remaining;
		for (; remaining > 0; ++index) {
			int16_t next = coeffs->primaryAxisSwap == 0 ? coeffs->edgePoints[index + 1].y
														: coeffs->edgePoints[index + 1].x;
			if (coordinate != next)
				break;
			--remaining;
			++length;
		}
		coeffs->runLengths[runIndex++] = length;
	}
	coeffs->runLengthCount = runIndex;
	foldedAngle = (rotationAngle - TRIG2_QUARTER_TURN) & (TRIG2_ANGLE_SIGN_BIT - 1);
	if (foldedAngle >= TRIG2_QUARTER_TURN)
		foldedAngle = TRIG2_ANGLE_SIGN_BIT - foldedAngle;
	if (foldedAngle < g_flightSwRotSpriteAxisSwapThreshold) {
		secondarySwap = 0;
		coeffs->secondaryAxisSwap = secondarySwap;
		if (g_flightSwRotSpriteSquarePixelMode == ROTSCALE_SQUARE_PIXEL_MODE)
			coeffs->secondaryScaleLow = rotscale_GetUpdateIncrement(foldedAngle, ROTSCALE_ASPECT_100);
		else
			coeffs->secondaryScaleLow = rotscale_GetUpdateIncrement(foldedAngle, ROTSCALE_ASPECT_91);
	} else {
		foldedAngle = TRIG2_QUARTER_TURN - foldedAngle;
		secondarySwap = ROTSCALE_SWAP_AXES;
		coeffs->secondaryAxisSwap = secondarySwap;
		if (g_flightSwRotSpriteSquarePixelMode == ROTSCALE_SQUARE_PIXEL_MODE)
			coeffs->secondaryScaleLow = rotscale_GetUpdateIncrement(foldedAngle, ROTSCALE_ASPECT_100);
		else
			coeffs->secondaryScaleLow = rotscale_GetUpdateIncrement(foldedAngle, ROTSCALE_ASPECT_110);
	}
	coeffs->secondaryScaleHigh = 0;
	if ((primarySwap ^ secondarySwap) == 0 && g_flightSwRotSpriteSquarePixelMode == 0) {
		uint16_t angle = coeffs->rotationAngle;
		if ((angle < TRIG2_ANGLE_SIGN_BIT + TRIG2_QUARTER_TURN + ROTSCALE_SQUARE_AXIS_SWAP_ANGLE &&
			 angle >= TRIG2_ANGLE_SIGN_BIT + ROTSCALE_SQUARE_AXIS_SWAP_ANGLE) ||
			(angle < TRIG2_QUARTER_TURN + ROTSCALE_SQUARE_AXIS_SWAP_ANGLE &&
			 angle >= ROTSCALE_SQUARE_AXIS_SWAP_ANGLE)) {
			unsigned int scale = (uint16_t)(ROTSCALE_SECONDARY_RECIPROCAL / coeffs->secondaryScaleLow);
			scale <<= ROTSCALE_FRACTION_BITS;
			coeffs->secondaryScaleLow = scale;
			coeffs->secondaryScaleHigh = scale >> ROTSCALE_PRODUCT_FRACTION_BITS;
			coeffs->secondaryStepByte = tangent >> ROTSCALE_FRACTION_BITS;
		} else {
			coeffs->secondaryScaleLow = ROTSCALE_SQUARE_ASPECT_Q8;
			coeffs->secondaryScaleHigh = ROTSCALE_SQUARE_ASPECT_Q8;
			coeffs->secondaryStepByte =
				(ROTSCALE_RECTANGULAR_STEP_SCALE * (unsigned int)tangent) >> ROTSCALE_SECONDARY_PRODUCT_BITS;
		}
	} else {
		coeffs->secondaryStepByte =
			(coeffs->secondaryScaleLow * (unsigned int)tangent + ROTSCALE_SECONDARY_ROUNDING) >>
			ROTSCALE_SECONDARY_PRODUCT_BITS;
	}
	coeffs->axisDirectionSum = coeffs->yDirectionFlag + (coeffs->xDirectionFlag >> 1);
	coeffs->octant = coeffs->yDirectionFlag | coeffs->xDirectionFlag | coeffs->primaryAxisSwap;
	coeffs->firstEdgeX = coeffs->edgePoints[0].x;
	coeffs->firstEdgeY = coeffs->edgePoints[0].y;
	coeffs->firstEdgeScreenY = g_flightSwRotSpriteViewportMaxY - coeffs->firstEdgeY;
	coeffs->lastEdgeX = coeffs->edgePoints[coeffs->scanCount - 1].x;
	coeffs->lastEdgeY = coeffs->edgePoints[coeffs->scanCount - 1].y;
	coeffs->lastEdgeScreenY = g_flightSwRotSpriteViewportMaxY - coeffs->lastEdgeY;
	coeffs->absEdgeDeltaX = coeffs->lastEdgeX - coeffs->firstEdgeX;
	if ((int16_t)coeffs->absEdgeDeltaX < 0)
		coeffs->absEdgeDeltaX = -coeffs->absEdgeDeltaX;
	coeffs->absEdgeDeltaY = coeffs->lastEdgeScreenY - coeffs->firstEdgeScreenY;
	if ((int16_t)coeffs->absEdgeDeltaY < 0)
		coeffs->absEdgeDeltaY = -coeffs->absEdgeDeltaY;
	if (g_flightBytesPerPixel == sizeof(uint16_t)) {
		for (index = 0; index < coeffs->scanCount; ++index) {
			int16_t x = coeffs->edgePoints[index].x;
			int16_t y;
			int offset;
			if (coeffs->xDirectionFlag != 0)
				x = -x;
			y = coeffs->edgePoints[index].y;
			if (coeffs->yDirectionFlag != 0)
				y = -y;
			if (g_flightSwRotSpriteDestYMode > 0) {
#ifdef XW_MODERN
				offset =
					(int)((unsigned int)g_flightSwRotSpriteDestPitchBytes * y + (int)sizeof(uint16_t) * x);
#else
				offset = g_flightSwRotSpriteDestPitchBytes * y + (int)sizeof(uint16_t) * x;
#endif
			} else {
#ifdef XW_MODERN
				offset =
					(int)((int)sizeof(uint16_t) * x - (unsigned int)g_flightSwRotSpriteDestPitchBytes * y);
#else
				offset = (int)sizeof(uint16_t) * x - g_flightSwRotSpriteDestPitchBytes * y;
#endif
			}
			coeffs->spanOffsets[index] = offset;
		}
	} else {
		for (index = 0; index < coeffs->scanCount; ++index) {
			int16_t x = coeffs->edgePoints[index].x;
			int16_t y;
			int offset;
			if (coeffs->xDirectionFlag != 0)
				x = -x;
			y = coeffs->edgePoints[index].y;
			if (coeffs->yDirectionFlag != 0)
				y = -y;
			if (g_flightSwRotSpriteDestYMode > 0) {
#ifdef XW_MODERN
				offset = (int)((unsigned int)g_flightSwRotSpriteDestPitchBytes * y + x);
#else
				offset = g_flightSwRotSpriteDestPitchBytes * y + x;
#endif
			} else {
#ifdef XW_MODERN
				offset = (int)(x - (unsigned int)g_flightSwRotSpriteDestPitchBytes * y);
#else
				offset = x - g_flightSwRotSpriteDestPitchBytes * y;
#endif
			}
			coeffs->spanOffsets[index] = offset;
		}
	}
}

// FUNCTION: XW 0x4A39E0
void rotscale_rotatescale(uint8_t* spriteData, int formatIndex) {
	uint8_t rowFraction;
	unsigned int paletteBase;
	size_t sourceIndex;
	g_flightSwRotSpriteSpanRunsEnabled = 1;
	if (rotscale_setstartvars() == 0)
		return;
	g_flightSwRotSpriteSavedClipMinX = g_flightSwRotSpriteClipMinX;
	g_flightSwRotSpriteSavedPrimaryEdgeX = g_flightSwRotSpritePrimaryEdgeX;
	g_flightSwRotSpriteSavedClipMaxX = g_flightSwRotSpriteClipMaxX;
	g_flightSwRotSpriteSavedPrimaryEdgeY = g_flightSwRotSpritePrimaryEdgeY;
	if (g_flightSwRotSpriteDestYMode > 0) {
		g_flightSwRotSpriteCoeffs->destLinePtr =
			g_flightSwRotSpriteDestLinePtr +
			(ptrdiff_t)g_flightBytesPerPixel * g_flightSwRotSpritePrimaryEdgeX +
			(ptrdiff_t)g_flightSwRotSpriteDestPitchBytes * g_flightSwRotSpritePrimaryEdgeY;
		g_flightSwRotSpriteCoeffs->destPitchDelta = -g_flightSwRotSpriteDestPitchBytes;
	} else {
		g_flightSwRotSpriteCoeffs->destLinePtr =
			g_flightSwRotSpriteDestLinePtr +
			(ptrdiff_t)g_flightSwRotSpriteDestPitchBytes *
				(g_flightSwRotSpriteViewportMaxY - g_flightSwRotSpritePrimaryEdgeY) +
			(ptrdiff_t)g_flightBytesPerPixel * g_flightSwRotSpritePrimaryEdgeX;
		g_flightSwRotSpriteCoeffs->destPitchDelta = g_flightSwRotSpriteDestPitchBytes;
	}
	rowFraction = 0;
	paletteBase = 0;
	g_flightSwRotSpriteDestLinePtr = g_flightSwRotSpriteCoeffs->destLinePtr;
	g_flightSwRotSpriteSkipSecondaryScaleStep = 1;
	g_flightSwRotSpriteSecondaryScaleAccum = 0;
	for (sourceIndex = 0; spriteData[sourceIndex] != ROTSCALE_RLE_END_SPRITE;) {
		/* The integral row position cancels when taking the difference; retain its fractional carry. */
		uint16_t rowRepeats = (rowFraction + g_rotSpriteScaleState.secondaryStepQ8) >> ROTSCALE_FRACTION_BITS;
		int runCount = 0;
		uint16_t spanFraction = 0;
		uint32_t spanEnd = 0;
		uint16_t rowIndex;
		if (g_flightSwRotSpriteSpanRunsEnabled == 1) {
			for (; spriteData[sourceIndex] != ROTSCALE_RLE_END_ROW;) {
				uint8_t command = spriteData[sourceIndex];
				if (command == ROTSCALE_RLE_PALETTE_BASE) {
					paletteBase =
						spriteData[sourceIndex + 1] + (spriteData[sourceIndex + 2] << ROTSCALE_FRACTION_BITS);
					sourceIndex += 1 + sizeof(uint16_t);
				} else if (command == ROTSCALE_RLE_SKIP) {
					uint8_t length = spriteData[sourceIndex + 1];
					uint16_t previousFraction = spanFraction;
					sourceIndex += 1 + sizeof(length);
					spanFraction += g_rotSpriteScaleState.stepFraction[length];
					spanEnd += g_rotSpriteScaleState.stepInteger[length];
					if (spanFraction < previousFraction)
						++spanEnd;
				} else {
					uint8_t length;
					uint8_t color;
					uint16_t previousFraction;
					uint32_t spanStart;
					if (command == ROTSCALE_RLE_EXPLICIT_RUN) {
						length = spriteData[sourceIndex + 1];
						color = spriteData[sourceIndex + 2];
						sourceIndex += 1 + sizeof(length) + sizeof(color);
					} else {
						color =
							(command >> g_rotSpriteColorShiftByFormat[formatIndex]) + (uint8_t)paletteBase;
						length = command & g_rotSpriteRunLengthMaskByFormat[formatIndex];
						++sourceIndex;
					}
					spanStart = spanEnd;
					previousFraction = spanFraction;
					spanFraction += g_rotSpriteScaleState.stepFraction[length];
					spanEnd += g_rotSpriteScaleState.stepInteger[length];
					if (spanFraction < previousFraction)
						++spanEnd;
					g_flightSwRotSpriteSpanRuns[runCount].start = spanStart;
					g_flightSwRotSpriteSpanRuns[runCount].colorIndex = color;
					g_flightSwRotSpriteSpanRuns[runCount].length = spanEnd - spanStart + 1;
					++runCount;
				}
			}
			++sourceIndex;
		}
		/* A subpixel row is drawn once without advancing the edges. */
		for (rowIndex = rowRepeats != 0 ? rowRepeats : 1; rowIndex != 0; --rowIndex) {
			if (runCount != 0 && g_flightSwRotSpriteClipMaxX >= 0) {
				int16_t spanLimit = spanEnd + g_flightSwRotSpriteSpanBaseX;
				if (spanLimit >= 0 && spanLimit < g_flightSwRotSpriteClipMaxX &&
					g_flightSwRotSpriteSpanBaseX >= 0 &&
					g_flightSwRotSpriteSpanBaseX >= g_flightSwRotSpriteClipMinX) {
					g_flightSwRotSpriteSpanRunCountdown = runCount;
					if (g_flightBytesPerPixel == sizeof(uint16_t))
						rotscale_DrawRotSpriteSpanRuns16(g_flightSwRotSpriteSpanRuns,
														 g_flightSwRotSpriteDestLinePtr,
														 g_flightSwRotSpriteCoeffs->spanOffsets);
					else
						rotscale_DrawRotSpriteSpanRuns8(g_flightSwRotSpriteSpanRuns,
														g_flightSwRotSpriteDestLinePtr,
														g_flightSwRotSpriteCoeffs->spanOffsets);
				} else {
					g_flightSwRotSpriteSpanRunCountdown = runCount;
					if (g_flightBytesPerPixel == sizeof(uint16_t))
						rotscale_DrawClippedRotSpriteSpanRuns16(g_flightSwRotSpriteSpanRuns,
																g_flightSwRotSpriteDestLinePtr,
																g_flightSwRotSpriteCoeffs->spanOffsets);
					else
						rotscale_DrawClippedRotSpriteSpanRuns8(g_flightSwRotSpriteSpanRuns,
															   g_flightSwRotSpriteDestLinePtr,
															   g_flightSwRotSpriteCoeffs->spanOffsets);
				}
			}
			if (rowRepeats != 0) {
				if (rotscale_updatecases() == 0)
					return;
				rotscale_updateperp();
			}
		}
		rowFraction += (uint8_t)g_rotSpriteScaleState.secondaryStepQ8;
	}
}

// FUNCTION: XW 0x4A3E10
void rotscale_updateperp(void) {
	int16_t stepCount = 1;
	uint16_t previousAccum;
	int16_t edgeIndex;
	FlightSwRotSpriteCoeffState* coeffs;
	if (g_flightSwRotSpriteSkipSecondaryScaleStep != 0) {
		g_flightSwRotSpriteSkipSecondaryScaleStep = 0;
		return;
	}
	previousAccum = g_flightSwRotSpriteSecondaryScaleAccum;
	coeffs = g_flightSwRotSpriteCoeffs;
	g_flightSwRotSpriteSecondaryScaleAccum += coeffs->secondaryScaleLow;
	if (coeffs->secondaryScaleHigh != 0) {
		if (g_flightSwRotSpriteSecondaryScaleAccum < previousAccum)
			stepCount = 2;
	} else if (g_flightSwRotSpriteSecondaryScaleAccum >= previousAccum) {
		return;
	}
	edgeIndex = g_flightSwRotSpriteSpanBaseX;
	while (edgeIndex < 0)
		edgeIndex += coeffs->scanCount;
	while (edgeIndex >= (int)coeffs->scanCount)
		edgeIndex -= coeffs->scanCount;
	if (coeffs->primaryAxisSwap == 0) {
		int16_t y = coeffs->edgePoints[edgeIndex].y;
		if (coeffs->axisDirectionSum == ROTSCALE_OPPOSED_AXIS_DIRECTIONS) {
			int16_t nextCoordinate = edgeIndex + 1 == ROTSCALE_EDGE_POINT_COUNT
										 ? (int16_t)coeffs->runLengthCount
										 : coeffs->edgePoints[edgeIndex + 1].y;
			g_flightSwRotSpriteSpanBaseX += stepCount;
			if (y != nextCoordinate)
				g_flightSwRotSpriteSkipSecondaryScaleStep = 1;
		} else {
			int16_t previousY =
				edgeIndex == 0 ? (int16_t)coeffs->absEdgeDeltaY : coeffs->edgePoints[edgeIndex - 1].y;
			g_flightSwRotSpriteSpanBaseX -= stepCount;
			if (y != previousY)
				g_flightSwRotSpriteSkipSecondaryScaleStep = 1;
		}
	} else {
		int16_t x = coeffs->edgePoints[edgeIndex].x;
		if (coeffs->axisDirectionSum != ROTSCALE_OPPOSED_AXIS_DIRECTIONS) {
			int16_t nextCoordinate = edgeIndex + 1 == ROTSCALE_EDGE_POINT_COUNT
										 ? (int16_t)coeffs->secondaryStepByte
										 : coeffs->edgePoints[edgeIndex + 1].x;
			g_flightSwRotSpriteSpanBaseX += stepCount;
			if (x != nextCoordinate)
				g_flightSwRotSpriteSkipSecondaryScaleStep = 1;
		} else {
			int16_t previousX =
				edgeIndex == 0 ? (int16_t)coeffs->absEdgeDeltaX : coeffs->edgePoints[edgeIndex - 1].x;
			g_flightSwRotSpriteSpanBaseX -= stepCount;
			if (x != previousX)
				g_flightSwRotSpriteSkipSecondaryScaleStep = 1;
		}
	}
}

// FUNCTION: XW 0x4A3F40
int rotscale_setstartvars(void) {
	int octant = g_flightSwRotSpriteCoeffs->octant;
	switch (octant) {
		case ROTSCALE_OCTANT_0:
			return rotscale_setstartcase0();
		case ROTSCALE_OCTANT_1:
			return rotscale_setstartcase1();
		case ROTSCALE_OCTANT_2:
			return rotscale_setstartcase2();
		case ROTSCALE_OCTANT_3:
			return rotscale_setstartcase3();
		case ROTSCALE_OCTANT_4:
			return rotscale_setstartcase4();
		case ROTSCALE_OCTANT_5:
			return rotscale_setstartcase5();
		case ROTSCALE_OCTANT_6:
			return rotscale_setstartcase6();
		case ROTSCALE_OCTANT_7:
			return rotscale_setstartcase7();
	}
	return octant;
}

// FUNCTION: XW 0x4A3FB0
int rotscale_updatecases(void) {
	int octant = g_flightSwRotSpriteCoeffs->octant;
	switch (octant) {
		case ROTSCALE_OCTANT_0:
			return rotscale_updatecase0();
		case ROTSCALE_OCTANT_1:
			return rotscale_updatecase1();
		case ROTSCALE_OCTANT_2:
			return rotscale_updatecase2();
		case ROTSCALE_OCTANT_3:
			return rotscale_updatecase3();
		case ROTSCALE_OCTANT_4:
			return rotscale_updatecase4();
		case ROTSCALE_OCTANT_5:
			return rotscale_updatecase5();
		case ROTSCALE_OCTANT_6:
			return rotscale_updatecase6();
		case ROTSCALE_OCTANT_7:
			return rotscale_updatecase7();
	}
	return octant;
}

// FUNCTION: XW 0x4A4020
int rotscale_setstartcase0(void) {
	int16_t edgeX = g_flightSwRotSpriteEdgeCursorX;
	FlightSwRotSpriteCoeffState* coeffs = g_flightSwRotSpriteCoeffs;
	int16_t edgeY = g_flightSwRotSpriteEdgeCursorY;
	int16_t wrapOffset = 0;
	int16_t primaryY;
	int16_t secondaryY;
	int16_t clipMin = 0;
	int16_t viewportHeight;
	while (edgeX < 0) {
		wrapOffset -= g_flightSwRotSpriteViewportWidth;
		edgeX += coeffs->absEdgeDeltaX + 1;
		g_flightSwRotSpriteEdgeCursorX = edgeX;
		edgeY += coeffs->absEdgeDeltaY + 1;
		g_flightSwRotSpriteEdgeCursorY = edgeY;
	}
	while (edgeX >= g_flightSwRotSpriteViewportWidth) {
		wrapOffset += g_flightSwRotSpriteViewportWidth;
		edgeX += -1 - coeffs->absEdgeDeltaX;
		g_flightSwRotSpriteEdgeCursorX = edgeX;
		edgeY += -1 - coeffs->absEdgeDeltaY;
		g_flightSwRotSpriteEdgeCursorY = edgeY;
	}
	viewportHeight = g_flightSwRotSpriteViewportHeight;
	primaryY = edgeY - coeffs->edgePoints[edgeX].y;
	g_flightSwRotSpriteEdgeCursorY = primaryY;
	g_flightSwRotSpritePrimaryEdgeY = primaryY;
	if (primaryY >= viewportHeight) {
		g_flightSwRotSpriteSecondaryEdgeY = primaryY + coeffs->absEdgeDeltaY;
		return 0;
	}
	g_flightSwRotSpritePrimaryEdgeX = 0;
	if (primaryY < 0) {
		int16_t clippedY = -primaryY;
		g_flightSwRotSpriteEdgeCursorY = clippedY;
		g_flightSwRotSpriteClipMinRunIdx03 = clippedY;
		clipMin = clippedY;
		if (clippedY > (int16_t)coeffs->absEdgeDeltaY) {
			clipMin = ROTSCALE_NO_CLIP_POINT;
		} else {
			uint16_t i;
			for (i = 0; i <= (int16_t)coeffs->absEdgeDeltaX; ++i) {
				if (clippedY == (uint16_t)coeffs->edgePoints[i].y) {
					clipMin = coeffs->edgePoints[i].x;
					break;
				}
			}
		}
	}
	g_flightSwRotSpriteClipMinX = clipMin;
	g_flightSwRotSpriteSecondaryEdgeX = coeffs->absEdgeDeltaX;
	g_flightSwRotSpriteClipMaxRunIdx03 = viewportHeight;
	secondaryY = g_flightSwRotSpritePrimaryEdgeY + coeffs->absEdgeDeltaY;
	g_flightSwRotSpriteSecondaryEdgeY = secondaryY;
	if (secondaryY < 0) {
		g_flightSwRotSpriteClipMaxX = ROTSCALE_NO_CLIP_POINT;
	} else if (secondaryY < viewportHeight) {
		g_flightSwRotSpriteClipMaxX = coeffs->scanCount - 1;
	} else {
		int16_t clippedY = viewportHeight - g_flightSwRotSpritePrimaryEdgeY;
		uint16_t i;
		g_flightSwRotSpriteClipMaxRunIdx03 = clippedY;
		for (i = 0; i <= (int16_t)coeffs->absEdgeDeltaX; ++i) {
			if (clippedY == (uint16_t)coeffs->edgePoints[i].y)
				break;
		}
		g_flightSwRotSpriteClipMaxX = coeffs->edgePoints[i].x - 1;
	}
	g_flightSwRotSpriteSpanBaseX = wrapOffset + edgeX;
	return 1;
}

// FUNCTION: XW 0x4A4220
int rotscale_updatecase0(void) {
	g_flightSwRotSpriteDestLinePtr -= g_flightSwRotSpriteCoeffs->destPitchDelta;
	++g_flightSwRotSpritePrimaryEdgeY;
	if (g_flightSwRotSpritePrimaryEdgeY == 0) {
		g_flightSwRotSpriteClipMinX = 0;
	} else {
		if (g_flightSwRotSpritePrimaryEdgeY >= g_flightSwRotSpriteViewportHeight) {
			return 0;
		}
		if (g_flightSwRotSpritePrimaryEdgeY < 0
#ifdef XW_MODERN
			/* The entry transition below initializes the cursor once the edge is visible. */
			&& g_flightSwRotSpriteSecondaryEdgeY >= 0
#endif
		) {
			--g_flightSwRotSpriteClipMinRunIdx03;
			g_flightSwRotSpriteClipMinX -=
				g_flightSwRotSpriteCoeffs->runLengths[g_flightSwRotSpriteClipMinRunIdx03];
		}
	}
	++g_flightSwRotSpriteSecondaryEdgeY;
	if (g_flightSwRotSpriteSecondaryEdgeY == 0) {
		g_flightSwRotSpriteClipMaxX = g_flightSwRotSpriteCoeffs->scanCount - 1;
		g_flightSwRotSpriteClipMinRunIdx03 = g_flightSwRotSpriteCoeffs->runLengthCount - 1;
		g_flightSwRotSpriteClipMinX =
			g_flightSwRotSpriteClipMaxX -
			g_flightSwRotSpriteCoeffs->runLengths[g_flightSwRotSpriteClipMinRunIdx03] + 1;
		if (g_flightSwRotSpriteClipMinX < 0) {
			g_flightSwRotSpriteClipMinX = 0;
		}
	} else if (g_flightSwRotSpriteSecondaryEdgeY >= g_flightSwRotSpriteViewportHeight) {
		if (g_flightSwRotSpriteSecondaryEdgeY == g_flightSwRotSpriteViewportHeight) {
			g_flightSwRotSpriteClipMaxRunIdx03 = g_flightSwRotSpriteCoeffs->runLengthCount;
		}
		--g_flightSwRotSpriteClipMaxRunIdx03;
		g_flightSwRotSpriteClipMaxX -=
			g_flightSwRotSpriteCoeffs->runLengths[g_flightSwRotSpriteClipMaxRunIdx03];
	}
	return 1;
}

// FUNCTION: XW 0x4A4320
int rotscale_setstartcase1(void) {
	int16_t edgeX = g_flightSwRotSpriteEdgeCursorX;
	FlightSwRotSpriteCoeffState* coeffs = g_flightSwRotSpriteCoeffs;
	int16_t edgeY = g_flightSwRotSpriteEdgeCursorY;
	int16_t wrapOffset = 0;
	int16_t primaryY;
	int16_t secondaryY;
	int16_t clipMin;
	int16_t viewportHeight;
	while (edgeX < 0) {
		wrapOffset -= g_flightSwRotSpriteViewportWidth;
		edgeX += coeffs->absEdgeDeltaX + 1;
		g_flightSwRotSpriteEdgeCursorX = edgeX;
		edgeY += -1 - coeffs->absEdgeDeltaY;
		g_flightSwRotSpriteEdgeCursorY = edgeY;
	}
	while (edgeX >= g_flightSwRotSpriteViewportWidth) {
		wrapOffset += g_flightSwRotSpriteViewportWidth;
		edgeX += -1 - coeffs->absEdgeDeltaX;
		g_flightSwRotSpriteEdgeCursorX = edgeX;
		edgeY += coeffs->absEdgeDeltaY + 1;
		g_flightSwRotSpriteEdgeCursorY = edgeY;
	}
	viewportHeight = g_flightSwRotSpriteViewportHeight;
	primaryY = edgeY + coeffs->edgePoints[edgeX].y;
	g_flightSwRotSpritePrimaryEdgeX = 0;
	g_flightSwRotSpriteEdgeCursorY = primaryY;
	g_flightSwRotSpritePrimaryEdgeY = primaryY;
	if (primaryY < 0) {
		clipMin = ROTSCALE_NO_CLIP_POINT;
	} else if (primaryY < viewportHeight) {
		clipMin = 0;
	} else {
		int16_t clippedDistance = primaryY - viewportHeight;
		int clippedY = clippedDistance + 1;
		g_flightSwRotSpriteClipMinRunIdx03 = clippedDistance;
		if (clippedY > (int16_t)coeffs->absEdgeDeltaY) {
			clipMin = ROTSCALE_NO_CLIP_POINT;
		} else {
			uint16_t i;
			clipMin = wrapOffset;
			for (i = 0; i <= (int16_t)coeffs->absEdgeDeltaX; ++i) {
				if (clippedY == (uint16_t)coeffs->edgePoints[i].y) {
					clipMin = coeffs->edgePoints[i].x;
					break;
				}
			}
		}
	}
	g_flightSwRotSpriteClipMinX = clipMin;
	g_flightSwRotSpriteSecondaryEdgeX = coeffs->absEdgeDeltaX;
	g_flightSwRotSpriteClipMaxRunIdx03 = viewportHeight;
	secondaryY = primaryY - coeffs->absEdgeDeltaY;
	g_flightSwRotSpriteSecondaryEdgeY = secondaryY;
	if (secondaryY >= viewportHeight)
		return 0;
	if (secondaryY >= 0) {
		g_flightSwRotSpriteClipMaxX = coeffs->scanCount - 1;
	} else if (primaryY < 0) {
		g_flightSwRotSpriteClipMaxX = ROTSCALE_NO_CLIP_POINT;
	} else {
		int16_t maxClipY = primaryY + coeffs->firstEdgeY + 1;
		uint16_t i;
		g_flightSwRotSpriteClipMaxRunIdx03 = primaryY;
		for (i = 0; i <= (int16_t)coeffs->absEdgeDeltaX; ++i) {
			if (maxClipY == coeffs->edgePoints[i].y)
				break;
		}
		g_flightSwRotSpriteClipMaxX = coeffs->edgePoints[i].x - 1;
	}
	g_flightSwRotSpriteSpanBaseX = wrapOffset + edgeX;
	return 1;
}

// FUNCTION: XW 0x4A4510
int rotscale_updatecase1(void) {
	g_flightSwRotSpriteDestLinePtr -= g_flightSwRotSpriteCoeffs->destPitchDelta;
	++g_flightSwRotSpritePrimaryEdgeY;
	if (g_flightSwRotSpritePrimaryEdgeY == 0) {
		g_flightSwRotSpriteClipMinX = 0;
		g_flightSwRotSpriteClipMaxX = g_flightSwRotSpriteCoeffs->runLengths[0] - 1;
		g_flightSwRotSpriteClipMaxRunIdx03 = 0;
	} else if (g_flightSwRotSpritePrimaryEdgeY >= g_flightSwRotSpriteViewportHeight) {
		if (g_flightSwRotSpritePrimaryEdgeY == g_flightSwRotSpriteViewportHeight) {
			g_flightSwRotSpriteClipMinRunIdx03 = ROTSCALE_BEFORE_FIRST_RUN;
		}
		++g_flightSwRotSpriteClipMinRunIdx03;
		g_flightSwRotSpriteClipMinX +=
			g_flightSwRotSpriteCoeffs->runLengths[g_flightSwRotSpriteClipMinRunIdx03];
	}
	++g_flightSwRotSpriteSecondaryEdgeY;
	if (g_flightSwRotSpriteSecondaryEdgeY == 0) {
		g_flightSwRotSpriteClipMaxX = g_flightSwRotSpriteCoeffs->scanCount - 1;
		return 1;
	}
	if (g_flightSwRotSpriteSecondaryEdgeY >= g_flightSwRotSpriteViewportHeight) {
		return 0;
	}
	if (g_flightSwRotSpriteSecondaryEdgeY < 0 && g_flightSwRotSpritePrimaryEdgeY > 0) {
		++g_flightSwRotSpriteClipMaxRunIdx03;
		g_flightSwRotSpriteClipMaxX +=
			g_flightSwRotSpriteCoeffs->runLengths[g_flightSwRotSpriteClipMaxRunIdx03];
	}
	return 1;
}

// FUNCTION: XW 0x4A4600
int rotscale_setstartcase2(void) {
	int16_t edgeX = g_flightSwRotSpriteEdgeCursorX;
	FlightSwRotSpriteCoeffState* coeffs = g_flightSwRotSpriteCoeffs;
	int16_t edgeY = g_flightSwRotSpriteEdgeCursorY;
	int16_t wrapOffset = 0;
	int16_t mirroredX;
	int16_t primaryY;
	int16_t secondaryY;
	int16_t clipMin;
	int16_t clipMax;
	int minSearchY;
#ifdef XW_MODERN
	minSearchY = ROTSCALE_NO_CLIP_POINT;
#endif
	while (edgeX < 0) {
		wrapOffset += g_flightSwRotSpriteViewportWidth;
		edgeX += coeffs->absEdgeDeltaX + 1;
		g_flightSwRotSpriteEdgeCursorX = edgeX;
		edgeY += -1 - coeffs->absEdgeDeltaY;
		g_flightSwRotSpriteEdgeCursorY = edgeY;
	}
	while (edgeX >= g_flightSwRotSpriteViewportWidth) {
		wrapOffset -= g_flightSwRotSpriteViewportWidth;
		edgeX += -1 - coeffs->absEdgeDeltaX;
		g_flightSwRotSpriteEdgeCursorX = edgeX;
		edgeY += coeffs->absEdgeDeltaY + 1;
		g_flightSwRotSpriteEdgeCursorY = edgeY;
	}
	mirroredX = g_flightSwRotSpriteViewportMaxX - edgeX;
	primaryY = edgeY - coeffs->edgePoints[mirroredX].y;
	g_flightSwRotSpritePrimaryEdgeX = g_flightSwRotSpriteViewportMaxX;
	g_flightSwRotSpriteEdgeCursorY = primaryY;
	g_flightSwRotSpritePrimaryEdgeY = primaryY;
	g_flightSwRotSpriteSecondaryEdgeX = g_flightSwRotSpriteViewportMaxX - coeffs->absEdgeDeltaX;
	secondaryY = primaryY + coeffs->absEdgeDeltaY;
	g_flightSwRotSpriteClipMinX = ROTSCALE_NO_CLIP_POINT;
	g_flightSwRotSpriteSecondaryEdgeY = secondaryY;
	g_flightSwRotSpriteClipMaxX = ROTSCALE_NO_CLIP_POINT;
	g_flightSwRotSpriteClipMinRunIdx03 = ROTSCALE_BEFORE_FIRST_RUN;
	g_flightSwRotSpriteClipMaxRunIdx03 = g_flightSwRotSpriteViewportHeight;
	if (primaryY >= 0 && primaryY >= g_flightSwRotSpriteViewportHeight) {
		g_flightSwRotSpriteClipMaxX = ROTSCALE_NO_CLIP_POINT;
		g_flightSwRotSpriteSpanBaseX = wrapOffset + mirroredX;
		return 1;
	}
	if (primaryY < 0) {
		int16_t clippedY = -primaryY;
		g_flightSwRotSpriteClipMinRunIdx03 = clippedY - 1;
		clipMin = ROTSCALE_NO_CLIP_POINT;
		if (clippedY <= (int16_t)coeffs->absEdgeDeltaY) {
			int16_t i;
			for (i = 0;; ++i) {
				if (i > (int16_t)coeffs->absEdgeDeltaX) {
					clipMin = minSearchY;
					break;
				}
				minSearchY = clippedY;
				if (minSearchY == (uint16_t)coeffs->edgePoints[i].y) {
					clipMin = coeffs->edgePoints[i].x;
					break;
				}
			}
		}
	} else {
		clipMin = 0;
	}
	g_flightSwRotSpriteClipMinX = clipMin;
	if (secondaryY < 0)
		return 0;
	if (secondaryY < g_flightSwRotSpriteViewportHeight) {
		clipMax = coeffs->scanCount - 1;
	} else {
		int16_t clippedY = g_flightSwRotSpriteViewportHeight - primaryY - 1;
		int16_t i;
		g_flightSwRotSpriteClipMaxRunIdx03 = clippedY;
		for (i = coeffs->absEdgeDeltaX;; --i) {
			if (i < 0) {
				clipMax = minSearchY;
				break;
			}
			if (clippedY == (uint16_t)coeffs->edgePoints[i].y) {
				clipMax = coeffs->edgePoints[i].x;
				break;
			}
		}
	}
	g_flightSwRotSpriteClipMaxX = clipMax;
	g_flightSwRotSpriteSpanBaseX = wrapOffset + mirroredX;
	return 1;
}

// FUNCTION: XW 0x4A4800
int rotscale_updatecase2(void) {
	g_flightSwRotSpriteDestLinePtr += g_flightSwRotSpriteCoeffs->destPitchDelta;
	--g_flightSwRotSpritePrimaryEdgeY;
	if (g_flightSwRotSpritePrimaryEdgeY == g_flightSwRotSpriteViewportMaxY) {
		g_flightSwRotSpriteClipMinX = 0;
		g_flightSwRotSpriteClipMaxX = ROTSCALE_EMPTY_CLIP_MAX;
		g_flightSwRotSpriteClipMaxRunIdx03 = ROTSCALE_BEFORE_FIRST_RUN;
	} else if (g_flightSwRotSpritePrimaryEdgeY < 0) {
		if (g_flightSwRotSpritePrimaryEdgeY == ROTSCALE_BEFORE_FIRST_ROW) {
			g_flightSwRotSpriteClipMinRunIdx03 = ROTSCALE_BEFORE_FIRST_RUN;
		}
		++g_flightSwRotSpriteClipMinRunIdx03;
		g_flightSwRotSpriteClipMinX +=
			g_flightSwRotSpriteCoeffs->runLengths[g_flightSwRotSpriteClipMinRunIdx03];
	}
	--g_flightSwRotSpriteSecondaryEdgeY;
	if (g_flightSwRotSpriteSecondaryEdgeY == g_flightSwRotSpriteViewportMaxY) {
		g_flightSwRotSpriteClipMaxX = g_flightSwRotSpriteCoeffs->scanCount - 1;
		return 1;
	}
	if (g_flightSwRotSpriteSecondaryEdgeY < 0) {
		return 0;
	}
	if (g_flightSwRotSpriteSecondaryEdgeY >= g_flightSwRotSpriteViewportHeight &&
		g_flightSwRotSpritePrimaryEdgeY < g_flightSwRotSpriteViewportHeight) {
		++g_flightSwRotSpriteClipMaxRunIdx03;
		g_flightSwRotSpriteClipMaxX +=
			g_flightSwRotSpriteCoeffs->runLengths[g_flightSwRotSpriteClipMaxRunIdx03];
	}
	return 1;
}

// FUNCTION: XW 0x4A48F0
int rotscale_setstartcase3(void) {
	int16_t edgeX = g_flightSwRotSpriteEdgeCursorX;
	FlightSwRotSpriteCoeffState* coeffs = g_flightSwRotSpriteCoeffs;
	int16_t edgeY = g_flightSwRotSpriteEdgeCursorY;
	int16_t wrapOffset = 0;
	int16_t mirroredX;
	int16_t primaryY;
	int16_t secondaryY;
	int16_t clipMin;
	int16_t clipMax;
	while (edgeX < 0) {
		wrapOffset += g_flightSwRotSpriteViewportWidth;
		edgeX += coeffs->absEdgeDeltaX + 1;
		g_flightSwRotSpriteEdgeCursorX = edgeX;
		edgeY += coeffs->absEdgeDeltaY + 1;
		g_flightSwRotSpriteEdgeCursorY = edgeY;
	}
	while (edgeX >= g_flightSwRotSpriteViewportWidth) {
		wrapOffset -= g_flightSwRotSpriteViewportWidth;
		edgeX += -1 - coeffs->absEdgeDeltaX;
		g_flightSwRotSpriteEdgeCursorX = edgeX;
		edgeY += -1 - coeffs->absEdgeDeltaY;
		g_flightSwRotSpriteEdgeCursorY = edgeY;
	}
	mirroredX = g_flightSwRotSpriteViewportMaxX - edgeX;
	primaryY = edgeY + coeffs->edgePoints[mirroredX].y;
	g_flightSwRotSpritePrimaryEdgeX = g_flightSwRotSpriteViewportMaxX;
	clipMin = ROTSCALE_NO_CLIP_POINT;
	g_flightSwRotSpriteEdgeCursorY = primaryY;
	g_flightSwRotSpritePrimaryEdgeY = primaryY;
	g_flightSwRotSpriteSecondaryEdgeX = g_flightSwRotSpriteViewportMaxX - coeffs->absEdgeDeltaX;
	secondaryY = primaryY - coeffs->absEdgeDeltaY;
	g_flightSwRotSpriteClipMinX = ROTSCALE_NO_CLIP_POINT;
	g_flightSwRotSpriteClipMaxX = ROTSCALE_EMPTY_CLIP_MAX;
	g_flightSwRotSpriteClipMinRunIdx03 = ROTSCALE_BEFORE_FIRST_RUN;
	g_flightSwRotSpriteSecondaryEdgeY = secondaryY;
	g_flightSwRotSpriteClipMaxRunIdx03 = coeffs->runLengthCount;
	if (primaryY < 0)
		return 0;
	if (primaryY < g_flightSwRotSpriteViewportHeight) {
		clipMin = 0;
	} else {
		int16_t clippedY = primaryY - g_flightSwRotSpriteViewportMaxY;
		g_flightSwRotSpriteClipMinRunIdx03 = clippedY;
		if (clippedY <= (int16_t)coeffs->absEdgeDeltaY) {
			int16_t lastPoint = coeffs->absEdgeDeltaX;
			int16_t i;
			for (i = 0; i <= lastPoint; ++i) {
				if (clippedY == (uint16_t)coeffs->edgePoints[i].y)
					break;
			}
			if (i <= lastPoint)
				clipMin = coeffs->edgePoints[i].x;
			else
				clipMin = mirroredX;
		}
	}
	g_flightSwRotSpriteClipMinX = clipMin;
	if (secondaryY >= g_flightSwRotSpriteViewportHeight) {
		clipMax = ROTSCALE_NO_CLIP_POINT;
	} else if (secondaryY >= 0) {
		clipMax = coeffs->scanCount - 1;
	} else {
		int16_t clippedY = secondaryY + coeffs->absEdgeDeltaY;
		int16_t i;
		g_flightSwRotSpriteClipMaxRunIdx03 = clippedY + 1;
		for (i = coeffs->absEdgeDeltaX;; --i) {
			if (i < 0) {
				clipMax = clippedY;
				break;
			}
			if (clippedY == (uint16_t)coeffs->edgePoints[i].y) {
				clipMax = coeffs->edgePoints[i].x;
				break;
			}
		}
	}
	g_flightSwRotSpriteClipMaxX = clipMax;
	g_flightSwRotSpriteSpanBaseX = wrapOffset + mirroredX;
	return 1;
}

// FUNCTION: XW 0x4A4AE0
int rotscale_updatecase3(void) {
	g_flightSwRotSpriteDestLinePtr += g_flightSwRotSpriteCoeffs->destPitchDelta;
	--g_flightSwRotSpritePrimaryEdgeY;
	--g_flightSwRotSpriteSecondaryEdgeY;
	if (g_flightSwRotSpritePrimaryEdgeY == g_flightSwRotSpriteViewportMaxY) {
		g_flightSwRotSpriteClipMinX = 0;
		g_flightSwRotSpriteClipMinRunIdx03 = ROTSCALE_BEFORE_FIRST_RUN;
	} else {
		if (g_flightSwRotSpritePrimaryEdgeY < 0)
			return 0;
		if (g_flightSwRotSpritePrimaryEdgeY >= g_flightSwRotSpriteViewportHeight &&
			g_flightSwRotSpriteSecondaryEdgeY < g_flightSwRotSpriteViewportHeight
#ifdef XW_MODERN
			/* On the entry row, reset the cursor below before reading any runs. */
			&& g_flightSwRotSpriteSecondaryEdgeY != g_flightSwRotSpriteViewportMaxY
#endif
		) {
			--g_flightSwRotSpriteClipMinRunIdx03;
			g_flightSwRotSpriteClipMinX -=
				g_flightSwRotSpriteCoeffs->runLengths[g_flightSwRotSpriteClipMinRunIdx03];
		}
	}
	if (g_flightSwRotSpriteSecondaryEdgeY == g_flightSwRotSpriteViewportMaxY) {
		int16_t runCount;
		g_flightSwRotSpriteClipMaxX = g_flightSwRotSpriteCoeffs->scanCount - 1;
		runCount = g_flightSwRotSpriteCoeffs->runLengthCount;
		g_flightSwRotSpriteClipMaxRunIdx03 = runCount;
		g_flightSwRotSpriteClipMinRunIdx03 = runCount - 1;
		g_flightSwRotSpriteClipMinX =
			g_flightSwRotSpriteClipMaxX -
			g_flightSwRotSpriteCoeffs->runLengths[g_flightSwRotSpriteClipMinRunIdx03] + 1;
		if (g_flightSwRotSpriteClipMinX < 0)
			g_flightSwRotSpriteClipMinX = 0;
	} else if (g_flightSwRotSpriteSecondaryEdgeY < 0) {
		--g_flightSwRotSpriteClipMaxRunIdx03;
		g_flightSwRotSpriteClipMaxX -=
			g_flightSwRotSpriteCoeffs->runLengths[g_flightSwRotSpriteClipMaxRunIdx03];
	}
	return 1;
}

// FUNCTION: XW 0x4A4C00
int rotscale_setstartcase4(void) {
	int16_t edgeY = g_flightSwRotSpriteEdgeCursorY;
	FlightSwRotSpriteCoeffState* coeffs = g_flightSwRotSpriteCoeffs;
	int16_t edgeX = g_flightSwRotSpriteEdgeCursorX;
	int16_t wrapOffset = 0;
	int16_t primaryX;
	int16_t clipMax;
	int16_t secondaryX;
	int16_t viewportWidth;

	while (edgeY < 0) {
		wrapOffset -= g_flightSwRotSpriteViewportHeight;
		edgeX += coeffs->absEdgeDeltaX + 1;
		g_flightSwRotSpriteEdgeCursorX = edgeX;
		edgeY += coeffs->absEdgeDeltaY + 1;
		g_flightSwRotSpriteEdgeCursorY = edgeY;
	}
	while (edgeY >= g_flightSwRotSpriteViewportHeight) {
		wrapOffset += g_flightSwRotSpriteViewportHeight;
		edgeX += -1 - coeffs->absEdgeDeltaX;
		g_flightSwRotSpriteEdgeCursorX = edgeX;
		edgeY += -1 - coeffs->absEdgeDeltaY;
		g_flightSwRotSpriteEdgeCursorY = edgeY;
	}
	viewportWidth = g_flightSwRotSpriteViewportWidth;
	primaryX = edgeX - coeffs->edgePoints[edgeY].x;
	g_flightSwRotSpritePrimaryEdgeY = 0;
	g_flightSwRotSpriteClipMinX = 0;
	g_flightSwRotSpriteClipMinRunIdx47 = ROTSCALE_BEFORE_FIRST_RUN;
	g_flightSwRotSpriteEdgeCursorX = primaryX;
	g_flightSwRotSpritePrimaryEdgeX = primaryX;
	g_flightSwRotSpriteClipMaxRunIdx47 = viewportWidth;
	if (primaryX < 0) {
		int16_t leftDistance = -primaryX;
		int16_t clipMin;
		int16_t i;
		g_flightSwRotSpriteClipMinRunIdx47 = leftDistance - 1;
		if (leftDistance > (int16_t)coeffs->absEdgeDeltaX) {
			clipMin = ROTSCALE_NO_CLIP_POINT;
		} else {
			for (i = 0;; ++i) {
				if (i > (int16_t)coeffs->absEdgeDeltaY) {
					clipMin = wrapOffset;
					break;
				}
				if (leftDistance == (uint16_t)coeffs->edgePoints[i].x) {
					clipMin = coeffs->edgePoints[i].y;
					break;
				}
			}
		}
		g_flightSwRotSpriteClipMinX = clipMin;
	}
	secondaryX = primaryX + coeffs->absEdgeDeltaX;
	g_flightSwRotSpriteSecondaryEdgeX = secondaryX;
	g_flightSwRotSpriteSecondaryEdgeY = coeffs->absEdgeDeltaY;
	if (secondaryX < 0)
		return 0;
	if (secondaryX < viewportWidth) {
		clipMax = coeffs->scanCount - 1;
	} else if (primaryX < viewportWidth) {
		int16_t rightDistance = viewportWidth - primaryX;
		int16_t i;
		g_flightSwRotSpriteClipMaxRunIdx47 = rightDistance - 1;
		for (i = 0;; ++i) {
			if (i > (int16_t)coeffs->absEdgeDeltaY) {
				clipMax = wrapOffset;
				break;
			}
			if (rightDistance == (uint16_t)coeffs->edgePoints[i].x) {
				clipMax = coeffs->edgePoints[i].y - 1;
				break;
			}
		}
	} else {
		clipMax = ROTSCALE_NO_CLIP_POINT;
	}
	g_flightSwRotSpriteClipMaxX = clipMax;
	g_flightSwRotSpriteSpanBaseX = wrapOffset + edgeY;
	return 1;
}

// FUNCTION: XW 0x4A4DD0
int rotscale_updatecase4(void) {
	g_flightSwRotSpriteDestLinePtr -= g_flightBytesPerPixel;
	--g_flightSwRotSpritePrimaryEdgeX;
	if (g_flightSwRotSpritePrimaryEdgeX == g_flightSwRotSpriteViewportMaxX) {
		g_flightSwRotSpriteClipMinX = 0;
		g_flightSwRotSpriteClipMaxX = ROTSCALE_EMPTY_CLIP_MAX;
		g_flightSwRotSpriteClipMaxRunIdx47 = ROTSCALE_BEFORE_FIRST_RUN;
	} else if (g_flightSwRotSpritePrimaryEdgeX < 0) {
		++g_flightSwRotSpriteClipMinRunIdx47;
		g_flightSwRotSpriteClipMinX +=
			g_flightSwRotSpriteCoeffs->runLengths[g_flightSwRotSpriteClipMinRunIdx47];
	}
	--g_flightSwRotSpriteSecondaryEdgeX;
	if (g_flightSwRotSpriteSecondaryEdgeX < 0)
		return 0;
	if (g_flightSwRotSpriteSecondaryEdgeX == g_flightSwRotSpriteViewportMaxX) {
		g_flightSwRotSpriteClipMaxX = g_flightSwRotSpriteCoeffs->scanCount - 1;
	} else if (g_flightSwRotSpriteSecondaryEdgeX >= g_flightSwRotSpriteViewportWidth &&
			   g_flightSwRotSpritePrimaryEdgeX < g_flightSwRotSpriteViewportWidth) {
		++g_flightSwRotSpriteClipMaxRunIdx47;
		g_flightSwRotSpriteClipMaxX +=
			g_flightSwRotSpriteCoeffs->runLengths[g_flightSwRotSpriteClipMaxRunIdx47];
	}
	return 1;
}

// FUNCTION: XW 0x4A4EB0
int rotscale_setstartcase5(void) {
	int16_t edgeY = g_flightSwRotSpriteEdgeCursorY;
	FlightSwRotSpriteCoeffState* coeffs = g_flightSwRotSpriteCoeffs;
	int16_t edgeX = g_flightSwRotSpriteEdgeCursorX;
	int16_t wrapOffset = 0;
	int16_t mirroredY;
	int16_t primaryX;
	int16_t secondaryX;
	int16_t clipMin;
	int16_t clipMax;
	int minSearchX;
#ifdef XW_MODERN
	minSearchX = ROTSCALE_NO_CLIP_POINT;
#endif
	while (edgeY < 0) {
		wrapOffset += g_flightSwRotSpriteViewportHeight;
		edgeX += -1 - coeffs->absEdgeDeltaX;
		g_flightSwRotSpriteEdgeCursorX = edgeX;
		edgeY += coeffs->absEdgeDeltaY + 1;
		g_flightSwRotSpriteEdgeCursorY = edgeY;
	}
	while (edgeY >= g_flightSwRotSpriteViewportHeight) {
		wrapOffset -= g_flightSwRotSpriteViewportHeight;
		edgeX += coeffs->absEdgeDeltaX + 1;
		g_flightSwRotSpriteEdgeCursorX = edgeX;
		edgeY += -1 - coeffs->absEdgeDeltaY;
		g_flightSwRotSpriteEdgeCursorY = edgeY;
	}
	mirroredY = g_flightSwRotSpriteViewportMaxY - edgeY;
	primaryX = edgeX - coeffs->edgePoints[mirroredY].x;
	g_flightSwRotSpritePrimaryEdgeY = g_flightSwRotSpriteViewportMaxY;
	clipMin = ROTSCALE_NO_CLIP_POINT;
	g_flightSwRotSpriteEdgeCursorX = primaryX;
	g_flightSwRotSpritePrimaryEdgeX = primaryX;
	g_flightSwRotSpriteSecondaryEdgeY = g_flightSwRotSpriteViewportMaxY - coeffs->absEdgeDeltaY;
	secondaryX = primaryX + coeffs->absEdgeDeltaX;
	g_flightSwRotSpriteClipMinX = 0;
	g_flightSwRotSpriteSecondaryEdgeX = secondaryX;
	g_flightSwRotSpriteClipMinRunIdx47 = ROTSCALE_BEFORE_FIRST_RUN;
	g_flightSwRotSpriteClipMaxRunIdx47 = g_flightSwRotSpriteViewportWidth;
	if (primaryX >= g_flightSwRotSpriteViewportWidth)
		return 0;
	if (primaryX < 0 && secondaryX < 0) {
		g_flightSwRotSpriteClipMaxX = ROTSCALE_NO_CLIP_POINT;
	} else {
		if (primaryX < 0) {
			int16_t clippedX = -primaryX;
			g_flightSwRotSpriteClipMinRunIdx47 = clippedX;
			if (clippedX <= (int16_t)coeffs->absEdgeDeltaX) {
				int16_t i;
				for (i = 0;; ++i) {
					if (i > (int16_t)coeffs->absEdgeDeltaY) {
						clipMin = minSearchX;
						break;
					}
					minSearchX = clippedX;
					if (minSearchX == (uint16_t)coeffs->edgePoints[i].x) {
						clipMin = coeffs->edgePoints[i].y;
						break;
					}
				}
			}
			g_flightSwRotSpriteClipMinX = clipMin;
		}
		if (secondaryX < g_flightSwRotSpriteViewportWidth && secondaryX >= 0) {
			clipMax = coeffs->scanCount - 1;
		} else if (secondaryX < g_flightSwRotSpriteViewportWidth) {
			clipMax = ROTSCALE_NO_CLIP_POINT;
		} else {
			int16_t clippedX = g_flightSwRotSpriteViewportWidth - primaryX;
			int16_t lastPoint;
			int16_t i;
			g_flightSwRotSpriteClipMaxRunIdx47 = clippedX;
			lastPoint = coeffs->absEdgeDeltaY;
			for (i = 0; i <= lastPoint; ++i) {
				if (clippedX == (uint16_t)coeffs->edgePoints[i].x)
					break;
			}
			if (i <= lastPoint)
				clipMax = coeffs->edgePoints[i].y - 1;
			else
				clipMax = minSearchX;
		}
		g_flightSwRotSpriteClipMaxX = clipMax;
	}
	g_flightSwRotSpriteSpanBaseX = wrapOffset + mirroredY;
	return 1;
}

// FUNCTION: XW 0x4A50B0
int rotscale_updatecase5(void) {
	g_flightSwRotSpriteDestLinePtr += g_flightBytesPerPixel;
	++g_flightSwRotSpritePrimaryEdgeX;
	if (g_flightSwRotSpritePrimaryEdgeX == 0) {
		g_flightSwRotSpriteClipMinX = 0;
	} else if (g_flightSwRotSpritePrimaryEdgeX < 0
#ifdef XW_MODERN
			   /* The entry transition below initializes the cursor once the edge is visible. */
			   && g_flightSwRotSpriteSecondaryEdgeX >= 0
#endif
	) {
		--g_flightSwRotSpriteClipMinRunIdx47;
		g_flightSwRotSpriteClipMinX -=
			g_flightSwRotSpriteCoeffs->runLengths[g_flightSwRotSpriteClipMinRunIdx47];
		if (g_flightSwRotSpriteClipMinX < 0)
			g_flightSwRotSpriteClipMinX = 0;
	} else if (g_flightSwRotSpritePrimaryEdgeX >= g_flightSwRotSpriteViewportWidth) {
		return 0;
	}
	++g_flightSwRotSpriteSecondaryEdgeX;
	if (g_flightSwRotSpriteSecondaryEdgeX == 0) {
		g_flightSwRotSpriteClipMaxX = g_flightSwRotSpriteCoeffs->scanCount - 1;
		g_flightSwRotSpriteClipMinRunIdx47 = g_flightSwRotSpriteCoeffs->runLengthCount - 1;
		g_flightSwRotSpriteClipMinX =
			g_flightSwRotSpriteViewportHeight -
			g_flightSwRotSpriteCoeffs->runLengths[g_flightSwRotSpriteClipMinRunIdx47];
		if (g_flightSwRotSpriteClipMinX < 0)
			g_flightSwRotSpriteClipMinX = 0;
	} else if (g_flightSwRotSpriteSecondaryEdgeX >= g_flightSwRotSpriteViewportWidth) {
		if (g_flightSwRotSpriteSecondaryEdgeX == g_flightSwRotSpriteViewportWidth) {
			g_flightSwRotSpriteClipMaxX = g_flightSwRotSpriteViewportMaxY;
			g_flightSwRotSpriteClipMaxRunIdx47 = g_flightSwRotSpriteCoeffs->runLengthCount;
		}
		--g_flightSwRotSpriteClipMaxRunIdx47;
		g_flightSwRotSpriteClipMaxX -=
			g_flightSwRotSpriteCoeffs->runLengths[g_flightSwRotSpriteClipMaxRunIdx47];
	}
	return 1;
}

// FUNCTION: XW 0x4A51D0
int rotscale_setstartcase6(void) {
	int16_t edgeY = g_flightSwRotSpriteEdgeCursorY;
	FlightSwRotSpriteCoeffState* coeffs = g_flightSwRotSpriteCoeffs;
	int16_t edgeX = g_flightSwRotSpriteEdgeCursorX;
	int16_t wrapOffset = 0;
	int16_t primaryX;
	int16_t secondaryX;
	int16_t clipMin;
	int16_t clipMax;

	while (edgeY < 0) {
		wrapOffset -= g_flightSwRotSpriteViewportHeight;
		edgeX += -1 - coeffs->absEdgeDeltaX;
		g_flightSwRotSpriteEdgeCursorX = edgeX;
		edgeY += coeffs->absEdgeDeltaY + 1;
		g_flightSwRotSpriteEdgeCursorY = edgeY;
	}
	while (edgeY >= g_flightSwRotSpriteViewportHeight) {
		wrapOffset += g_flightSwRotSpriteViewportHeight;
		edgeX += coeffs->absEdgeDeltaX + 1;
		g_flightSwRotSpriteEdgeCursorX = edgeX;
		edgeY += -1 - coeffs->absEdgeDeltaY;
		g_flightSwRotSpriteEdgeCursorY = edgeY;
	}
	primaryX = edgeX + coeffs->edgePoints[edgeY].x;
	clipMin = 0;
	g_flightSwRotSpritePrimaryEdgeY = 0;
	g_flightSwRotSpriteEdgeCursorX = primaryX;
	g_flightSwRotSpritePrimaryEdgeX = primaryX;
	secondaryX = primaryX - coeffs->absEdgeDeltaX;
	g_flightSwRotSpriteSecondaryEdgeX = secondaryX;
	g_flightSwRotSpriteSecondaryEdgeY = coeffs->absEdgeDeltaY;
	g_flightSwRotSpriteClipMinX = ROTSCALE_NO_CLIP_POINT;
	g_flightSwRotSpriteClipMinRunIdx47 = ROTSCALE_BEFORE_FIRST_RUN;
	g_flightSwRotSpriteClipMaxX = ROTSCALE_EMPTY_CLIP_MAX;
	g_flightSwRotSpriteClipMaxRunIdx47 = coeffs->runLengthCount;
	if (primaryX < 0)
		return 0;
	if (primaryX >= g_flightSwRotSpriteViewportWidth) {
		int16_t clippedX = primaryX - g_flightSwRotSpriteViewportMaxX;
		int16_t i;
		g_flightSwRotSpriteClipMinRunIdx47 = clippedX;
		if (clippedX > (int16_t)coeffs->absEdgeDeltaX) {
			clipMin = ROTSCALE_NO_CLIP_POINT;
		} else {
			for (i = 0;; ++i) {
				if (i > (int16_t)coeffs->absEdgeDeltaY) {
					clipMin = wrapOffset;
					break;
				}
				if (clippedX == (uint16_t)coeffs->edgePoints[i].x) {
					clipMin = coeffs->edgePoints[i].y;
					break;
				}
			}
		}
	}
	g_flightSwRotSpriteClipMinX = clipMin;
	if (secondaryX >= g_flightSwRotSpriteViewportWidth) {
		clipMax = ROTSCALE_NO_CLIP_POINT;
	} else if (secondaryX >= 0) {
		clipMax = coeffs->scanCount - 1;
	} else {
		int16_t clippedX = secondaryX + coeffs->absEdgeDeltaX;
		int16_t i;
		g_flightSwRotSpriteClipMaxRunIdx47 = clippedX + 1;
		for (i = coeffs->absEdgeDeltaY;; --i) {
			if (i < 0) {
				clipMax = wrapOffset;
				break;
			}
			if (clippedX == (uint16_t)coeffs->edgePoints[i].x) {
				clipMax = coeffs->edgePoints[i].y;
				break;
			}
		}
	}
	g_flightSwRotSpriteClipMaxX = clipMax;
	g_flightSwRotSpriteSpanBaseX = wrapOffset + edgeY;
	return 1;
}

// FUNCTION: XW 0x4A53B0
int rotscale_updatecase6(void) {
	g_flightSwRotSpriteDestLinePtr -= g_flightBytesPerPixel;
	--g_flightSwRotSpriteSecondaryEdgeX;
	--g_flightSwRotSpritePrimaryEdgeX;
	if (g_flightSwRotSpritePrimaryEdgeX < 0)
		return 0;
	if (g_flightSwRotSpritePrimaryEdgeX == g_flightSwRotSpriteViewportMaxX) {
		g_flightSwRotSpriteClipMinX = 0;
	} else if (g_flightSwRotSpritePrimaryEdgeX >= g_flightSwRotSpriteViewportWidth &&
			   g_flightSwRotSpriteSecondaryEdgeX < g_flightSwRotSpriteViewportWidth
#ifdef XW_MODERN
			   /* On the entry row, reset the cursor below before reading any runs. */
			   && g_flightSwRotSpriteSecondaryEdgeX != g_flightSwRotSpriteViewportMaxX
#endif
	) {
		--g_flightSwRotSpriteClipMinRunIdx47;
		g_flightSwRotSpriteClipMinX -=
			g_flightSwRotSpriteCoeffs->runLengths[g_flightSwRotSpriteClipMinRunIdx47];
		if (g_flightSwRotSpriteClipMinX < 0)
			g_flightSwRotSpriteClipMinX = 0;
	}
	if (g_flightSwRotSpriteSecondaryEdgeX == g_flightSwRotSpriteViewportMaxX) {
		g_flightSwRotSpriteClipMaxX = g_flightSwRotSpriteCoeffs->scanCount - 1;
		g_flightSwRotSpriteClipMinRunIdx47 = g_flightSwRotSpriteCoeffs->runLengthCount - 1;
		g_flightSwRotSpriteClipMinX =
			g_flightSwRotSpriteClipMaxX -
			g_flightSwRotSpriteCoeffs->runLengths[g_flightSwRotSpriteClipMinRunIdx47] + 1;
		if (g_flightSwRotSpriteClipMinX < 0)
			g_flightSwRotSpriteClipMinX = 0;
	} else if (g_flightSwRotSpriteSecondaryEdgeX < 0) {
		--g_flightSwRotSpriteClipMaxRunIdx47;
		g_flightSwRotSpriteClipMaxX -=
			g_flightSwRotSpriteCoeffs->runLengths[g_flightSwRotSpriteClipMaxRunIdx47];
		if (g_flightSwRotSpriteClipMaxX < 0)
			g_flightSwRotSpriteClipMaxX = 0;
	}
	return 1;
}

// FUNCTION: XW 0x4A54C0
int rotscale_setstartcase7(void) {
	int16_t edgeY = g_flightSwRotSpriteEdgeCursorY;
	FlightSwRotSpriteCoeffState* coeffs = g_flightSwRotSpriteCoeffs;
	int16_t edgeX = g_flightSwRotSpriteEdgeCursorX;
	int16_t wrapOffset = 0;
	int16_t mirroredY;
	int16_t primaryX;
	int16_t secondaryX;
	int16_t clipMin;
	int16_t clipMax;
	while (edgeY < 0) {
		wrapOffset += g_flightSwRotSpriteViewportHeight;
		edgeX += coeffs->absEdgeDeltaX + 1;
		g_flightSwRotSpriteEdgeCursorX = edgeX;
		edgeY += coeffs->absEdgeDeltaY + 1;
		g_flightSwRotSpriteEdgeCursorY = edgeY;
	}
	while (edgeY >= g_flightSwRotSpriteViewportHeight) {
		wrapOffset -= g_flightSwRotSpriteViewportHeight;
		edgeX += -1 - coeffs->absEdgeDeltaX;
		g_flightSwRotSpriteEdgeCursorX = edgeX;
		edgeY += -1 - coeffs->absEdgeDeltaY;
		g_flightSwRotSpriteEdgeCursorY = edgeY;
	}
	mirroredY = g_flightSwRotSpriteViewportMaxY - edgeY;
	primaryX = coeffs->edgePoints[mirroredY].x + edgeX;
	g_flightSwRotSpritePrimaryEdgeY = g_flightSwRotSpriteViewportMaxY;
	g_flightSwRotSpriteEdgeCursorX = primaryX;
	g_flightSwRotSpritePrimaryEdgeX = primaryX;
	secondaryX = primaryX - coeffs->absEdgeDeltaX;
	g_flightSwRotSpriteSecondaryEdgeX = secondaryX;
	g_flightSwRotSpriteSecondaryEdgeY = g_flightSwRotSpriteViewportMaxY - coeffs->absEdgeDeltaY;
	g_flightSwRotSpriteClipMinRunIdx47 = ROTSCALE_BEFORE_FIRST_RUN;
	g_flightSwRotSpriteClipMaxRunIdx47 = g_flightSwRotSpriteViewportWidth;
	if (primaryX < 0) {
		clipMin = ROTSCALE_NO_CLIP_POINT;
	} else if (primaryX < g_flightSwRotSpriteViewportWidth) {
		clipMin = 0;
	} else {
		int16_t clippedX = primaryX - g_flightSwRotSpriteViewportMaxX;
		g_flightSwRotSpriteClipMinRunIdx47 = clippedX - 1;
		if (clippedX > (int16_t)coeffs->absEdgeDeltaX) {
			clipMin = ROTSCALE_NO_CLIP_POINT;
		} else {
			int16_t i;
			for (i = 0;; ++i) {
				if (i > (int16_t)coeffs->absEdgeDeltaY) {
					clipMin = mirroredY;
					break;
				}
				if (clippedX == (uint16_t)coeffs->edgePoints[i].x) {
					clipMin = coeffs->edgePoints[i].y;
					break;
				}
			}
		}
	}
	g_flightSwRotSpriteClipMinX = clipMin;
	if (secondaryX >= g_flightSwRotSpriteViewportWidth)
		return 0;
	if (secondaryX >= 0) {
		clipMax = coeffs->scanCount - 1;
	} else if (primaryX < 0) {
		clipMax = ROTSCALE_NO_CLIP_POINT;
	} else {
		int16_t clippedX = secondaryX + coeffs->absEdgeDeltaX;
		int16_t i;
		g_flightSwRotSpriteClipMaxRunIdx47 = clippedX;
		for (i = coeffs->absEdgeDeltaY;; --i) {
			if (i < 0) {
				clipMax = mirroredY;
				break;
			}
			if (clippedX == (uint16_t)coeffs->edgePoints[i].x) {
				clipMax = coeffs->edgePoints[i].y;
				break;
			}
		}
	}
	g_flightSwRotSpriteClipMaxX = clipMax;
	g_flightSwRotSpriteSpanBaseX = wrapOffset + mirroredY;
	return 1;
}

// FUNCTION: XW 0x4A56B0
int rotscale_updatecase7(void) {
	g_flightSwRotSpriteDestLinePtr += g_flightBytesPerPixel;
	++g_flightSwRotSpritePrimaryEdgeX;
	if (g_flightSwRotSpritePrimaryEdgeX == 0) {
		g_flightSwRotSpriteClipMinX = 0;
		g_flightSwRotSpriteClipMaxX = ROTSCALE_EMPTY_CLIP_MAX;
		g_flightSwRotSpriteClipMaxRunIdx47 = ROTSCALE_BEFORE_FIRST_RUN;
	} else if (g_flightSwRotSpritePrimaryEdgeX >= g_flightSwRotSpriteViewportWidth) {
		if (g_flightSwRotSpritePrimaryEdgeX == g_flightSwRotSpriteViewportWidth) {
			g_flightSwRotSpriteClipMinRunIdx47 = ROTSCALE_BEFORE_FIRST_RUN;
		}
		++g_flightSwRotSpriteClipMinRunIdx47;
		g_flightSwRotSpriteClipMinX +=
			g_flightSwRotSpriteCoeffs->runLengths[g_flightSwRotSpriteClipMinRunIdx47];
	}
	++g_flightSwRotSpriteSecondaryEdgeX;
	if (g_flightSwRotSpriteSecondaryEdgeX == 0) {
		g_flightSwRotSpriteClipMaxX = g_flightSwRotSpriteCoeffs->scanCount - 1;
	} else {
		if (g_flightSwRotSpriteSecondaryEdgeX >= g_flightSwRotSpriteViewportWidth) {
			return 0;
		}
		if (g_flightSwRotSpriteSecondaryEdgeX < 0 && g_flightSwRotSpritePrimaryEdgeX >= 0) {
			++g_flightSwRotSpriteClipMaxRunIdx47;
			g_flightSwRotSpriteClipMaxX +=
				g_flightSwRotSpriteCoeffs->runLengths[g_flightSwRotSpriteClipMaxRunIdx47];
		}
	}
	return 1;
}

// FUNCTION: XW 0x4A5790
int rotscale_calcscale(int depth, uint16_t modelExtent, uint16_t screenScale) {
	int projectedExtent;
	int projectedSize;
	if (depth < 0)
		depth = (int32_t)(0u - (uint32_t)depth);
	projectedExtent = depth >> ROTSCALE_FRACTION_BITS;
	if (projectedExtent != 0)
		projectedExtent = modelExtent / projectedExtent;
	projectedSize = (int32_t)((uint32_t)screenScale * projectedExtent) >> ROTSCALE_FRACTION_BITS;
	if (projectedSize > ROTSCALE_MAX_PROJECTED_SIZE)
		return ROTSCALE_MAX_PROJECTED_SIZE;
	return projectedSize;
}

// FUNCTION: XW 0x4A9C80
int rotscale_LookupSpriteSineQ15(unsigned int angle) { return trig2_calcsineofangle(angle); }
