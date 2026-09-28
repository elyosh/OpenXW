#ifndef XW_RENDER_ROTSCALE_H
#define XW_RENDER_ROTSCALE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

enum {
	ROTSCALE_FRACTION_BITS = 8,
	ROTSCALE_MAX_PROJECTED_SIZE = 512,
	ROTSCALE_CLIP_MARGIN = 2,
	ROTSCALE_QUAD_COORDINATE_COUNT = 8
};

enum {
	ROTSCALE_SCALE_ROUNDING = 0x80,
	ROTSCALE_PRODUCT_ROUNDING = 0x8000,
	ROTSCALE_COORDINATE_SIGN = 0x8000
};

enum {
	ROTSCALE_SCALE_TABLE_COUNT = 256,
	ROTSCALE_SQUARE_PIXEL_MODE = 1,
	ROTSCALE_SQUARE_ASPECT_Q8 = 256,
	ROTSCALE_RECTANGULAR_ASPECT_Y_Q8 = 233,
	ROTSCALE_RECTANGULAR_ASPECT_X_Q8 = 282,
	ROTSCALE_SQUARE_AXIS_SWAP_ANGLE = 0x2000,
	ROTSCALE_RECTANGULAR_AXIS_SWAP_ANGLE = 0x2200,
	ROTSCALE_PRODUCT_FRACTION_BITS = 16
};

enum {
	ROTSCALE_REVERSE_Y = 1,
	ROTSCALE_REVERSE_X = 2,
	ROTSCALE_SWAP_AXES = 4,
	ROTSCALE_SECONDARY_PRODUCT_BITS = 24,
	ROTSCALE_SECONDARY_ROUNDING = 0x800000,
	ROTSCALE_SECONDARY_RECIPROCAL = 0x1000000,
	ROTSCALE_RECTANGULAR_STEP_SCALE = 55296
};

enum { ROTSCALE_OPPOSED_AXIS_DIRECTIONS = 1, ROTSCALE_EDGE_POINT_COUNT = 1600 };

enum { ROTSCALE_INDEXED_PIXEL_TAG = 0x80, ROTSCALE_INDEXED_TINT_COUNT = 64 };

enum {
	ROTSCALE_RLE_FORMAT_COUNT = 9,
	ROTSCALE_SPAN_RUN_CAPACITY = 512,
	ROTSCALE_RLE_PALETTE_BASE = 0xFB,
	ROTSCALE_RLE_SKIP = 0xFC,
	ROTSCALE_RLE_EXPLICIT_RUN = 0xFD,
	ROTSCALE_RLE_END_ROW = 0xFE,
	ROTSCALE_RLE_END_SPRITE = 0xFF
};

enum { ROTSCALE_TINT_TABLE_ENTRIES = 256 };

enum {
	ROTSCALE_BEFORE_FIRST_RUN = -1,
	ROTSCALE_EMPTY_CLIP_MAX = -1,
	ROTSCALE_BEFORE_FIRST_ROW = -1,
	ROTSCALE_NO_CLIP_POINT = -1
};

enum {
	ROTSCALE_TANGENT_ANGLE_SHIFT = 6,
	ROTSCALE_ASPECT_91 = 91,
	ROTSCALE_ASPECT_100 = 100,
	ROTSCALE_ASPECT_110 = 110,
	ROTSCALE_TANGENT_91_COUNT = 137,
	ROTSCALE_TANGENT_100_COUNT = 137,
	ROTSCALE_TANGENT_110_COUNT = 122
};

enum XwRotSpriteOctant {
	ROTSCALE_OCTANT_0 = 0,
	ROTSCALE_OCTANT_1,
	ROTSCALE_OCTANT_2,
	ROTSCALE_OCTANT_3,
	ROTSCALE_OCTANT_4,
	ROTSCALE_OCTANT_5,
	ROTSCALE_OCTANT_6,
	ROTSCALE_OCTANT_7
};

struct XwBitmapFramePrefix;
typedef struct FlightSwRotSpriteCoeffState FlightSwRotSpriteCoeffState;
typedef struct FlightSwRotSpriteEdgePoint FlightSwRotSpriteEdgePoint;
typedef struct XwRotSpriteScaleState XwRotSpriteScaleState;
typedef struct XwRotSpriteSpanRun XwRotSpriteSpanRun;

/* Original IDB size: 4 bytes. */
struct FlightSwRotSpriteEdgePoint {
	/* IDB +0x0 */
	int16_t x;
	/* IDB +0x2 */
	int16_t y;
};

/* Original IDB size: 1036 bytes. */
struct XwRotSpriteScaleState {
	/* IDB +0x0 */
	uint16_t scaleQ8;
	/* IDB +0x2 */
	uint16_t cachedPrimaryStepQ8;
	/* IDB +0x4 */
	uint16_t primaryStepQ8;
	/* IDB +0x6 */
	uint16_t secondaryStepQ8;
	/* IDB +0x8 */
	uint16_t stepFraction[ROTSCALE_SCALE_TABLE_COUNT];
	/* IDB +0x208 */
	uint16_t stepInteger[ROTSCALE_SCALE_TABLE_COUNT];
	/* IDB +0x408 */
	uint16_t aspectYQ8;
	/* IDB +0x40A */
	uint16_t aspectXQ8;
};

/* Original IDB size: 12 bytes. */
struct XwRotSpriteSpanRun {
	/* IDB +0x0 */
	int start;
	/* IDB +0x4 */
	unsigned int colorIndex;
	/* IDB +0x8 */
	int length;
};

/* Original IDB size: 16060 bytes. */
struct FlightSwRotSpriteCoeffState {
	/* IDB +0x0 */
	uint16_t rotationAngle;
	/* IDB +0x2: Unsigned magnitude from FlightSw_LookupSpriteSineQ15; sign is tracked separately in
	 * sinSignMask. */
	uint16_t sinQ15;
	/* IDB +0x4 */
	uint16_t sinSignMask;
	/* IDB +0x6: Unsigned magnitude from FlightSw_LookupSpriteSineQ15; sign is tracked separately in
	 * cosSignMask. */
	uint16_t cosQ15;
	/* IDB +0x8 */
	uint16_t cosSignMask;
	/* IDB +0xA */
	uint16_t primaryCosQ15;
	/* IDB +0xC */
	uint16_t primaryStepReciprocal;
	/* IDB +0xE */
	uint16_t scanCount;
	/* IDB +0x10 */
	uint16_t yDirectionFlag;
	/* IDB +0x12 */
	uint16_t xDirectionFlag;
	/* IDB +0x14: yDirectionFlag + (xDirectionFlag >> 1), 0..2; 1 selects opposed-axis edge traversal. */
	uint16_t axisDirectionSum;
	/* IDB +0x16 */
	uint16_t octant;
	/* IDB +0x18 */
	uint16_t primaryAxisSwap;
	/* IDB +0x1A */
	uint16_t secondaryAxisSwap;
	/* IDB +0x1C */
	uint16_t secondaryScaleLow;
	/* IDB +0x1E */
	uint16_t secondaryScaleHigh;
	/* IDB +0x20 */
	int16_t firstEdgeX;
	/* IDB +0x22 */
	int16_t lastEdgeX;
	/* IDB +0x24 */
	int16_t firstEdgeScreenY;
	/* IDB +0x26 */
	int16_t lastEdgeScreenY;
	/* IDB +0x28 */
	int16_t firstEdgeY;
	/* IDB +0x2A */
	int16_t lastEdgeY;
	/* IDB +0x2C */
	uint16_t absEdgeDeltaX;
	/* IDB +0x2E */
	uint16_t absEdgeDeltaY;
	/* IDB +0x30: 1600 packed 4-byte X/Y pairs, active count scanCount. */
	struct FlightSwRotSpriteEdgePoint edgePoints[ROTSCALE_EDGE_POINT_COUNT];
	/* IDB +0x1930 */
	uint16_t secondaryStepByte;
	/* IDB +0x1932 */
	uint16_t runLengthCount;
	/* IDB +0x1934: 1600 16-bit slots; runLengthCount entries group repeated secondary coordinates. */
	uint16_t runLengths[1600];
	/* IDB +0x25B4 */
	uint8_t* destLinePtr;
	/* IDB +0x25B8: 1600 signed byte displacements from the destination line base. */
	int spanOffsets[1600];
	/* IDB +0x3EB8 */
	int destPitchDelta;
};

/* Declarations follow ascending original IDB address. */

extern const uint8_t g_rotSpriteRunLengthMaskByFormat[ROTSCALE_RLE_FORMAT_COUNT];
extern const uint8_t g_rotSpriteColorShiftByFormat[ROTSCALE_RLE_FORMAT_COUNT];
extern const uint16_t g_rotSpriteTangent91[ROTSCALE_TANGENT_91_COUNT];
extern const uint16_t g_rotSpriteTangent100[ROTSCALE_TANGENT_100_COUNT];
extern const uint16_t g_rotSpriteTangent110[ROTSCALE_TANGENT_110_COUNT];
extern int g_flightSwRotSpriteSpanRunsEnabled;
extern int g_flightSwRotSpriteCoeffsValid;
extern int g_flightSwRotSpriteSquarePixelMode;
extern XwRotSpriteSpanRun g_flightSwRotSpriteSpanRuns[ROTSCALE_SPAN_RUN_CAPACITY];
extern int16_t g_flightSwRotSpriteClipMinRunIdx03;
extern int16_t g_flightSwRotSpritePointOutputY;
extern int16_t g_flightSwRotSpritePointOutputX;
extern int16_t g_flightSwRotSpriteSecondaryEdgeY;
extern int16_t g_flightSwRotSpriteSecondaryEdgeX;
extern uint8_t* g_flightSwRotSpriteDestLinePtr;
extern uint8_t g_flightSwRotSpriteTintTable[ROTSCALE_TINT_TABLE_ENTRIES];
extern int16_t g_flightSwRotSpriteEdgeCursorX;
extern int16_t g_flightSwRotSpriteEdgeCursorY;
extern int16_t g_flightSwRotSpritePointInputX;
extern int16_t g_flightSwRotSpritePointInputY;
extern int g_flightSwRotSpriteDestPitchBytes;
extern int16_t g_flightSwRotSpriteSkipSecondaryScaleStep;
extern FlightSwRotSpriteCoeffState g_flightSwRotSpriteCoeffStorage;
extern int16_t g_flightSwRotSpriteClipMaxX;
extern uint8_t g_flightSwRotSpriteTintHiTable[ROTSCALE_TINT_TABLE_ENTRIES];
extern uint16_t g_flightSwRotSpriteSecondaryScaleAccum;
extern int16_t g_flightSwRotSpriteClipMaxRunIdx03;
extern uint8_t g_flightSwRotSpriteTintLoTable[ROTSCALE_TINT_TABLE_ENTRIES];
extern int16_t g_flightSwRotSpriteViewportMaxX;
extern int16_t g_flightSwRotSpriteClipMaxRunIdx47;
extern int g_flightSwRotSpriteSpanRunCountdown;
extern int16_t g_flightSwRotSpriteClipMinRunIdx47;
extern int16_t g_flightSwRotSpriteViewportWidth;
extern int16_t g_flightSwRotSpriteDestYMode;
extern int16_t g_flightSwRotSpriteSavedClipMinX;
extern int16_t g_flightSwRotSpriteSavedClipMaxX;
extern int16_t g_flightSwRotSpritePrimaryEdgeY;
extern int16_t g_flightSwRotSpritePrimaryEdgeX;
extern int16_t g_flightSwRotSpriteClipMinX;
extern int16_t g_flightSwRotSpriteSpanBaseX;
extern XwRotSpriteScaleState g_rotSpriteScaleState;
extern int16_t g_flightSwRotSpriteViewportMaxY;
extern FlightSwRotSpriteCoeffState* g_flightSwRotSpriteCoeffs;
extern int16_t g_flightSwRotSpriteSavedPrimaryEdgeY;
extern int16_t g_flightSwRotSpriteSavedPrimaryEdgeX;
extern int16_t g_flightSwRotSpriteViewportHeight;
extern uint16_t g_flightSwRotSpriteAxisSwapThreshold;
extern uint8_t* g_flightSwRotSpriteDestBuffer;

/* 0x491FD0 */
void rotscale_BlitPreparedRotatedSpriteSpans(uint8_t* pixels, int rowSkipBytes, int startX, int startY,
											 int endX, int endY);

/* 0x49DF00 */
void rotscale_SetRotatedSpriteDestBuffer(uint8_t* bufferAddress);

/* 0x4A2CB0 */
/* Requires a positive run count and positive lengths, with valid source/destination offsets. */
void rotscale_DrawRotSpriteSpanRuns8(const struct XwRotSpriteSpanRun* runs, uint8_t* destination,
									 const int* spanOffsets);

/* 0x4A2D00 */
void rotscale_DrawClippedRotSpriteSpanRuns8(const struct XwRotSpriteSpanRun* runs, uint8_t* destination,
											const int* spanOffsets);

/* 0x4A2D70 */
/* Requires positive run count/lengths and valid byte offsets for both bytes of each pixel. */
void rotscale_DrawRotSpriteSpanRuns16(const struct XwRotSpriteSpanRun* runs, uint8_t* destination,
									  const int* spanOffsets);

/* 0x4A2DC0 */
void rotscale_DrawClippedRotSpriteSpanRuns16(const struct XwRotSpriteSpanRun* runs, uint8_t* destination,
											 const int* spanOffsets);

/* 0x4A2E30 */
void rotscale_rotatescaleimage(int16_t screenX, int16_t screenY, uint16_t screenScale,
							   const struct XwBitmapFramePrefix* frame);

/* 0x4A2FD0 */
void rotscale_ClipAndBlitPreparedRotatedSprite(int* quadXY);

/* 0x4A3110 */
void rotscale_preparefastdraw(uint16_t rotationAngle);

/* 0x4A31D0 */
/* Requires a valid frame-relative palette and at most ROTSCALE_TINT_TABLE_ENTRIES entries. */
void rotscale_preparecolor(const struct XwBitmapFramePrefix* frame);

/* 0x4A3220 */
/* Requires a folded angle whose index fits the selected table. Returns an unsigned Q0.16 slope. */
uint16_t rotscale_GetUpdateIncrement(uint16_t foldedAngle, int16_t aspectSelector);

/* 0x4A3270 */
void rotscale_scalesetup(uint16_t scaleQ8, const struct FlightSwRotSpriteCoeffState* coeffs,
						 struct XwRotSpriteScaleState* state);

/* 0x4A3380 */
void rotscale_adjustoffsets(const struct FlightSwRotSpriteCoeffState* coeffs,
							const struct XwRotSpriteScaleState* scaleState);

/* 0x4A34D0 */
void rotscale_buildlinedata(uint16_t rotationAngle, struct FlightSwRotSpriteCoeffState* coeffs);

/* 0x4A39E0 */
void rotscale_rotatescale(uint8_t* spriteData, int formatIndex);

/* 0x4A3E10 */
void rotscale_updateperp(void);

/* 0x4A3F40 */
int rotscale_setstartvars(void);

/* 0x4A3FB0 */
int rotscale_updatecases(void);

/* 0x4A4020 */
int rotscale_setstartcase0(void);

/* 0x4A4220 */
int rotscale_updatecase0(void);

/* 0x4A4320 */
int rotscale_setstartcase1(void);

/* 0x4A4510 */
int rotscale_updatecase1(void);

/* 0x4A4600 */
int rotscale_setstartcase2(void);

/* 0x4A4800 */
int rotscale_updatecase2(void);

/* 0x4A48F0 */
int rotscale_setstartcase3(void);

/* 0x4A4AE0 */
int rotscale_updatecase3(void);

/* 0x4A4C00 */
int rotscale_setstartcase4(void);

/* 0x4A4DD0 */
int rotscale_updatecase4(void);

/* 0x4A4EB0 */
int rotscale_setstartcase5(void);

/* 0x4A50B0 */
int rotscale_updatecase5(void);

/* 0x4A51D0 */
int rotscale_setstartcase6(void);

/* 0x4A53B0 */
int rotscale_updatecase6(void);

/* 0x4A54C0 */
int rotscale_setstartcase7(void);

/* 0x4A56B0 */
int rotscale_updatecase7(void);

/* 0x4A5790 */
int rotscale_calcscale(int depth, uint16_t modelExtent, uint16_t screenScale);

/* 0x4A9C80 */
int rotscale_LookupSpriteSineQ15(unsigned int angle);

#ifdef __cplusplus
}
#endif

#endif
