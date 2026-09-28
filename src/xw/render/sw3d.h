#ifndef XW_RENDER_SW3D_H
#define XW_RENDER_SW3D_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

enum {
	SW3D_MAX_SHADE_Q8 = 0xEFF,
	SW3D_TEXCOORD_BIAS_COUNT = 15,
	SW3D_TEXTURE_SHIFT_8 = 3,
	SW3D_TEXTURE_SHIFT_16 = 4,
	SW3D_TEXTURE_SHIFT_32 = 5,
	SW3D_TEXTURE_SHIFT_64 = 6,
	SW3D_TEXTURE_SHIFT_128 = 7,
	SW3D_TEXTURE_SHIFT_256 = 8
};

enum { SW3D_MIP_AREA_SHIFT = 8 };

enum {
	SW3D_MIP_AREA_LIMIT = 1 << SW3D_MIP_AREA_SHIFT,
	SW3D_MIP_MIN_DIMENSION = 8,
	SW3D_MIP_AREA_REDUCTION_SHIFT = 2,
	SW3D_TEXTURE_LOG2_INDEX_SHIFT = 4,
	SW3D_TEXTURE_LOG2_TABLE_COUNT = 65
};

enum { SW3D_INVALID_EDGE = -1, SW3D_REJECTED_EDGE = -2 };

struct ProjVertex;
struct SceneEdge;
struct SceneFace;
struct SceneMesh;
typedef struct SoftwareSpanShadeCounter SoftwareSpanShadeCounter;

enum {
	SW3D_PACKED_U_SHIFT = 16,
	SW3D_SHADE_LEVEL_SHIFT = 16,
	SW3D_PIXEL16_BITS = 16,
	SW3D_SPAN_COUNT_SHIFT = 24,
	SW3D_SPAN_COUNT_MASK = (int)0xFF000000U,
	SW3D_SPAN_COUNT_STEP_MASK = (int)0xFE000000U,
	SW3D_TEXTURE_FRACTION_BITS = 8,
	SW3D_SHADE_FRACTION_BITS = 8,
	SW3D_SHADE_LEVEL_MASK = 15,
	SW3D_SHADE_TABLE_STRIDE = 256,
	SW3D_PACKED_SHADE_TABLE_OFFSET = (SW3D_SHADE_LEVEL_MASK + 1) * SW3D_SHADE_TABLE_STRIDE,
	SW3D_SHIFT_COUNT_MASK = 31,
	SW3D_TEXTURE_COLUMN_MASK = 0xFF,
	SW3D_PACKED_V_SHIFT_128 = 1,
	SW3D_PACKED_V_SHIFT_8 = 5,
	SW3D_PACKED_V_SHIFT_16 = 4,
	SW3D_PACKED_V_SHIFT_32 = 3,
	SW3D_PACKED_V_SHIFT_64 = 2,
	SW3D_PACKED_UV_MASK_8 = 0x07FEFFFF,
	SW3D_PACKED_UV_MASK_16 = 0x0FFEFFFF,
	SW3D_PACKED_UV_MASK_32 = 0x1FFEFFFF,
	SW3D_PACKED_UV_MASK_64 = 0x3FFEFFFF,
	SW3D_PACKED_UV_MASK_128 = 0x7FFEFFFF,
	SW3D_PACKED_UV_MASK_256 = ~(1 << SW3D_PACKED_U_SHIFT)
};

enum {
	SW3D_SPAN_RECIPROCAL_COUNT = 70,
	SW3D_LIGHT_SAMPLE_BLOCK_SHIFT = 4,
	SW3D_LIGHT_SAMPLE_BLOCK_SIZE = 1 << SW3D_LIGHT_SAMPLE_BLOCK_SHIFT
};

extern const float g_softwareDistantDepthSubtract;
extern const float g_softwareTriangleVertexCountFloat;
extern const float g_softwareQuadVertexCountFloat;
extern const float g_softwareDistantProjectionScale;
extern float g_sw3dSpanLengthReciprocal[SW3D_SPAN_RECIPROCAL_COUNT];
extern int g_sw3dShadeDitherInitialByScanlineParity[2];
extern const int g_textureLog2BySizeDiv16[SW3D_TEXTURE_LOG2_TABLE_COUNT];
extern const float g_sw3dSpanOneFloat;
extern const float g_sw3dLightIntensityToShadeScale;
extern const float g_sw3dFloatToIntRoundBias;
extern const float g_sw3dTexCoordBiasByShift[SW3D_TEXCOORD_BIAS_COUNT];
extern int g_sw3dSkipOddScanlines;
extern struct ProjVertex* g_sw3dGeneratedClipVertex;
extern struct ProjVertex* g_sw3dClipTop;
extern struct ProjVertex* g_sw3dClipBottom;
extern struct SceneFace* g_sw3dCurrentFace;
extern int g_sw3dLightSampleBlockMask;
extern unsigned int g_sw3dSpanShadeDitherAccum;
extern int g_sw3dLightSampleBlockSize;
extern uint8_t* g_sw3dSpanCachedTexels;
extern int g_sw3dSpanFramebufferRowOffset;
extern float g_sw3dLightSampleSubrowLerpT;
extern SoftwareSpanShadeCounter g_sw3dSpanShadeCounterScratch;
extern int g_sw3dSpanVQ8;
extern float g_sw3dLightSampleRowsToNextBlockFloat;
extern float g_sw3dLightSampleSubrowFloat;
extern float g_sw3dLightSampleInvBlockSize;
extern int g_sw3dSpanUQ8;
extern int g_sw3dSpanShadeStepQ8;
extern int g_sw3dSpanStepUQ8;
extern int g_sw3dCurrentScanlineY;
extern int g_sw3dSpanLength;
extern int g_sw3dSpanStartX;
extern int g_sw3dCurrentLightSampleCacheStamp;
extern unsigned int g_sw3dSpanPackedUVScratch;
extern float g_sw3dLightSampleBlockSizeFloat;
extern struct SceneMesh* g_sw3dSpanSceneMesh;
extern float g_sw3dSpanTextureWidthFloat;
extern float g_sw3dSpanTextureHeightFloat;
extern int g_sw3dSpanTextureWidthShift;
extern int g_sw3dSpanTextureHeightShift;
extern uint8_t* g_sw3dSpanShadeTable;
extern uint8_t* g_sw3dSpanTexels;
extern unsigned int g_sw3dSpanTexelMask;
extern uint8_t* g_sw3dSpanCachedShadeTable;
extern int g_sw3dLightSampleBlockShift;
extern int g_sw3dSpanShadeQ8;
extern int g_sw3dSpanStepVQ8;

/* Original IDB size: 4 bytes. */
struct SoftwareSpanShadeCounter {
	/* IDB +0x0: Fractional shade error; ADD CL,CH accumulates shadeFraction and carry rounds shadeLevel for
	 * each pixel. */
	uint8_t ditherRemainder;
	/* IDB +0x1: Low 8 bits of Q8 shade; copied into bits 8..15 of packed ECX. */
	uint8_t shadeFraction;
	/* IDB +0x2: Integer shade byte, read at scratch+2 and incremented by fractional-add carry for lookup.
	 * Caller clamps shade to 0..0xEFF. */
	uint8_t shadeLevel;
	/* IDB +0x3: Count byte initialized to low-byte spanLength minus one; packed shade-step addition also
	 * decrements this counter. Signed ECX terminates. */
	int8_t remainingMinusOne;
};

/* Declarations follow ascending original IDB address. */

/* 0x486490 */
void sw3d_ProjectMeshVertices(struct SceneMesh* mesh);

/* 0x486C00 */
void sw3d_ProjectMeshVerticesDistant(struct SceneMesh* mesh);

/* 0x4872C0 */
void sw3d_RasterizeMeshFaces(struct SceneMesh* mesh);

/* 0x4876B0 */
void sw3d_ScanConvertFace(struct SceneFace* face);

/* 0x487CE0 */
int sw3d_SetupClippedEdge(struct SceneMesh* mesh, struct SceneEdge* edge, const struct ProjVertex* vTop,
						  const struct ProjVertex* vBot);

/* 0x488040 */
int sw3d_SetupEdge(struct SceneEdge* edge, const struct ProjVertex* vTop, const struct ProjVertex* vBot);

/* 0x492140 */
void sw3d_DrawVisibleFacesToSurface(void);

/* 0x4924E0 */
void sw3d_InsertSpan(float xLeft, float xRight, int scanY, struct SceneFace* face);

/* 0x4934D0 */
void sw3d_DrawTexturedSpan(int startX, int endX, float depth);

/* 0x4947F0 */
void sw3d_DrawTexturedShadeSpan8_8x8(void);

/* 0x494960 */
void sw3d_DrawTexturedShadeSpan8_8x16(void);

/* 0x494AD0 */
void sw3d_DrawTexturedShadeSpan8_8x32(void);

/* 0x494C40 */
void sw3d_DrawTexturedShadeSpan8_8x64(void);

/* 0x494DB0 */
void sw3d_DrawTexturedShadeSpan8_8x128(void);

/* 0x494F20 */
void sw3d_DrawTexturedShadeSpan8_8x256(void);

/* 0x495090 */
void sw3d_DrawTexturedShadeSpan8_16x8(void);

/* 0x495200 */
void sw3d_DrawTexturedShadeSpan8_16x16(void);

/* 0x495370 */
void sw3d_DrawTexturedShadeSpan8_16x32(void);

/* 0x4954E0 */
void sw3d_DrawTexturedShadeSpan8_16x64(void);

/* 0x495650 */
void sw3d_DrawTexturedShadeSpan8_16x128(void);

/* 0x4957C0 */
void sw3d_DrawTexturedShadeSpan8_16x256(void);

/* 0x495930 */
void sw3d_DrawTexturedShadeSpan8_32x8(void);

/* 0x495AA0 */
void sw3d_DrawTexturedShadeSpan8_32x16(void);

/* 0x495C10 */
void sw3d_DrawTexturedShadeSpan8_32x32(void);

/* 0x495D80 */
void sw3d_DrawTexturedShadeSpan8_32x64(void);

/* 0x495EF0 */
void sw3d_DrawTexturedShadeSpan8_32x128(void);

/* 0x496060 */
void sw3d_DrawTexturedShadeSpan8_32x256(void);

/* 0x4961D0 */
void sw3d_DrawTexturedShadeSpan8_64x8(void);

/* 0x496340 */
void sw3d_DrawTexturedShadeSpan8_64x16(void);

/* 0x4964B0 */
void sw3d_DrawTexturedShadeSpan8_64x32(void);

/* 0x496620 */
void sw3d_DrawTexturedShadeSpan8_64x64(void);

/* 0x496790 */
void sw3d_DrawTexturedShadeSpan8_64x128(void);

/* 0x496900 */
void sw3d_DrawTexturedShadeSpan8_64x256(void);

/* 0x496A70 */
void sw3d_DrawTexturedShadeSpan8_128x8(void);

/* 0x496BE0 */
void sw3d_DrawTexturedShadeSpan8_128x16(void);

/* 0x496D50 */
void sw3d_DrawTexturedShadeSpan8_128x32(void);

/* 0x496EC0 */
void sw3d_DrawTexturedShadeSpan8_128x64(void);

/* 0x497030 */
void sw3d_DrawTexturedShadeSpan8_128x128(void);

/* 0x4971A0 */
void sw3d_DrawTexturedShadeSpan8_128x256(void);

/* 0x497310 */
void sw3d_DrawTexturedShadeSpan8_256x8(void);

/* 0x497480 */
void sw3d_DrawTexturedShadeSpan8_256x16(void);

/* 0x4975F0 */
void sw3d_DrawTexturedShadeSpan8_256x32(void);

/* 0x497760 */
void sw3d_DrawTexturedShadeSpan8_256x64(void);

/* 0x4978D0 */
void sw3d_DrawTexturedShadeSpan8_256x128(void);

/* 0x497A40 */
void sw3d_DrawTexturedShadeSpan8_256x256(void);

/* 0x497BB0 */
void sw3d_DrawTexturedShadeSpan16_8x8(void);

/* 0x497D50 */
void sw3d_DrawTexturedShadeSpan16_8x16(void);

/* 0x497EF0 */
void sw3d_DrawTexturedShadeSpan16_8x32(void);

/* 0x498090 */
void sw3d_DrawTexturedShadeSpan16_8x64(void);

/* 0x498230 */
void sw3d_DrawTexturedShadeSpan16_8x128(void);

/* 0x4983D0 */
void sw3d_DrawTexturedShadeSpan16_8x256(void);

/* 0x498570 */
void sw3d_DrawTexturedShadeSpan16_16x8(void);

/* 0x498710 */
void sw3d_DrawTexturedShadeSpan16_16x16(void);

/* 0x4988B0 */
void sw3d_DrawTexturedShadeSpan16_16x32(void);

/* 0x498A50 */
void sw3d_DrawTexturedShadeSpan16_16x64(void);

/* 0x498BF0 */
void sw3d_DrawTexturedShadeSpan16_16x128(void);

/* 0x498D90 */
void sw3d_DrawTexturedShadeSpan16_16x256(void);

/* 0x498F30 */
void sw3d_DrawTexturedShadeSpan16_32x8(void);

/* 0x4990D0 */
void sw3d_DrawTexturedShadeSpan16_32x16(void);

/* 0x499270 */
void sw3d_DrawTexturedShadeSpan16_32x32(void);

/* 0x499410 */
void sw3d_DrawTexturedShadeSpan16_32x64(void);

/* 0x4995B0 */
void sw3d_DrawTexturedShadeSpan16_32x128(void);

/* 0x499750 */
void sw3d_DrawTexturedShadeSpan16_32x256(void);

/* 0x4998F0 */
void sw3d_DrawTexturedShadeSpan16_64x8(void);

/* 0x499A90 */
void sw3d_DrawTexturedShadeSpan16_64x16(void);

/* 0x499C30 */
void sw3d_DrawTexturedShadeSpan16_64x32(void);

/* 0x499DD0 */
void sw3d_DrawTexturedShadeSpan16_64x64(void);

/* 0x499F70 */
void sw3d_DrawTexturedShadeSpan16_64x128(void);

/* 0x49A110 */
void sw3d_DrawTexturedShadeSpan16_64x256(void);

/* 0x49A2B0 */
void sw3d_DrawTexturedShadeSpan16_128x8(void);

/* 0x49A440 */
void sw3d_DrawTexturedShadeSpan16_128x16(void);

/* 0x49A5D0 */
void sw3d_DrawTexturedShadeSpan16_128x32(void);

/* 0x49A760 */
void sw3d_DrawTexturedShadeSpan16_128x64(void);

/* 0x49A8F0 */
void sw3d_DrawTexturedShadeSpan16_128x128(void);

/* 0x49AA80 */
void sw3d_DrawTexturedShadeSpan16_128x256(void);

/* 0x49AC10 */
void sw3d_DrawTexturedShadeSpan16_256x8(void);

/* 0x49ADA0 */
void sw3d_DrawTexturedShadeSpan16_256x16(void);

/* 0x49AF30 */
void sw3d_DrawTexturedShadeSpan16_256x32(void);

/* 0x49B0C0 */
void sw3d_DrawTexturedShadeSpan16_256x64(void);

/* 0x49B250 */
void sw3d_DrawTexturedShadeSpan16_256x128(void);

/* 0x49B3E0 */
void sw3d_DrawTexturedShadeSpan16_256x256(void);

/* 0x49B570 */
void sw3d_DrawUnshadedSpan8_8x8(void);

/* 0x49B650 */
void sw3d_DrawUnshadedSpan8_8x16(void);

/* 0x49B730 */
void sw3d_DrawUnshadedSpan8_8x32(void);

/* 0x49B810 */
void sw3d_DrawUnshadedSpan8_8x64(void);

/* 0x49B8F0 */
void sw3d_DrawUnshadedSpan8_8x128(void);

/* 0x49B9D0 */
void sw3d_DrawUnshadedSpan8_8x256(void);

/* 0x49BAB0 */
void sw3d_DrawUnshadedSpan8_16x8(void);

/* 0x49BB90 */
void sw3d_DrawUnshadedSpan8_16x16(void);

/* 0x49BC70 */
void sw3d_DrawUnshadedSpan8_16x32(void);

/* 0x49BD50 */
void sw3d_DrawUnshadedSpan8_16x64(void);

/* 0x49BE30 */
void sw3d_DrawUnshadedSpan8_16x128(void);

/* 0x49BF10 */
void sw3d_DrawUnshadedSpan8_16x256(void);

/* 0x49BFF0 */
void sw3d_DrawUnshadedSpan8_32x8(void);

/* 0x49C0D0 */
void sw3d_DrawUnshadedSpan8_32x16(void);

/* 0x49C1B0 */
void sw3d_DrawUnshadedSpan8_32x32(void);

/* 0x49C290 */
void sw3d_DrawUnshadedSpan8_32x64(void);

/* 0x49C370 */
void sw3d_DrawUnshadedSpan8_32x128(void);

/* 0x49C450 */
void sw3d_DrawUnshadedSpan8_32x256(void);

/* 0x49C530 */
void sw3d_DrawUnshadedSpan8_64x8(void);

/* 0x49C610 */
void sw3d_DrawUnshadedSpan8_64x16(void);

/* 0x49C6F0 */
void sw3d_DrawUnshadedSpan8_64x32(void);

/* 0x49C7D0 */
void sw3d_DrawUnshadedSpan8_64x64(void);

/* 0x49C8B0 */
void sw3d_DrawUnshadedSpan8_64x128(void);

/* 0x49C990 */
void sw3d_DrawUnshadedSpan8_64x256(void);

/* 0x49CA70 */
void sw3d_DrawUnshadedSpan8_128x8(void);

/* 0x49CB50 */
void sw3d_DrawUnshadedSpan8_128x16(void);

/* 0x49CC30 */
void sw3d_DrawUnshadedSpan8_128x32(void);

/* 0x49CD10 */
void sw3d_DrawUnshadedSpan8_128x64(void);

/* 0x49CDF0 */
void sw3d_DrawUnshadedSpan8_128x128(void);

/* 0x49CED0 */
void sw3d_DrawUnshadedSpan8_128x256(void);

/* 0x49CFB0 */
void sw3d_DrawUnshadedSpan8_256x8(void);

/* 0x49D090 */
void sw3d_DrawUnshadedSpan8_256x16(void);

/* 0x49D170 */
void sw3d_DrawUnshadedSpan8_256x32(void);

/* 0x49D250 */
void sw3d_DrawUnshadedSpan8_256x64(void);

/* 0x49D330 */
void sw3d_DrawUnshadedSpan8_256x128(void);

/* 0x49D410 */
void sw3d_DrawUnshadedSpan8_256x256(void);

/* 0x49D4F0 */
void sw3d_DrawTexturedShadeSpanGeneric8(void);

/* 0x49D5E0 */
void sw3d_DrawTexturedShadeSpanGeneric(void);

/* 0x49D6D0 */
void sw3d_BlitOccludedSpan(const uint8_t* sourcePixels, int startX, int endX, int scanY, float inverseDepth);

/* 0x49DB10 */
void sw3d_CopySpanToFramebuffer(const uint8_t* sourceRasterBase, int startX, int pixelCount);

#ifdef __cplusplus
}
#endif

#endif
