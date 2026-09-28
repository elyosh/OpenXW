#ifndef XW_RENDER_IMAGE_QUANTIZER_H
#define XW_RENDER_IMAGE_QUANTIZER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <xw/compiler.h>

typedef struct ImageQuantizerImage ImageQuantizerImage;
typedef struct ImageQuantizerNode ImageQuantizerNode;
typedef struct ImageQuantizerNodeBlock ImageQuantizerNodeBlock;
typedef struct ImageQuantizerPaletteEntry ImageQuantizerPaletteEntry;
typedef struct ImageQuantizerPixelRun ImageQuantizerPixelRun;

enum {
	IMAGE_QUANTIZER_TEXT_CAPACITY = 2048,
	IMAGE_QUANTIZER_DEFAULT_CLASS = 1,
	IMAGE_QUANTIZER_DEFAULT_COMPRESSION = 2,
	IMAGE_QUANTIZER_DEFAULT_FIELD1038 = 8,
	IMAGE_QUANTIZER_DEFAULT_FIELD1060 = 2,
	IMAGE_QUANTIZER_DEFAULT_FIELD1062 = 72,
	IMAGE_QUANTIZER_DEFAULT_FIELD1066 = 72,
	IMAGE_QUANTIZER_CHILD_COUNT = 8,
	IMAGE_QUANTIZER_NODE_LIMIT = 0x41241,
	IMAGE_QUANTIZER_OCTANT_BLUE = 1,
	IMAGE_QUANTIZER_OCTANT_GREEN = 2,
	IMAGE_QUANTIZER_OCTANT_RED = 4,
	IMAGE_QUANTIZER_BLOCK_NODE_COUNT = 2048,
	IMAGE_QUANTIZER_MIN_DEPTH = 2,
	IMAGE_QUANTIZER_MAX_DEPTH = 8,
	IMAGE_QUANTIZER_ROOT_MIDPOINT = 128,
	IMAGE_QUANTIZER_MAX_CHANNEL = 255,
	IMAGE_QUANTIZER_RGB_CHANNEL_COUNT = 3,
	IMAGE_QUANTIZER_ROOT_ERROR_PER_PIXEL = IMAGE_QUANTIZER_RGB_CHANNEL_COUNT *
										   (IMAGE_QUANTIZER_MAX_CHANNEL + 1) *
										   (IMAGE_QUANTIZER_MAX_CHANNEL + 1),
	IMAGE_QUANTIZER_RED_CHANNEL = 0,
	IMAGE_QUANTIZER_GREEN_CHANNEL = 1,
	IMAGE_QUANTIZER_BLUE_CHANNEL = 2,
	IMAGE_QUANTIZER_SIX_BIT_SHIFT = 2,
	IMAGE_QUANTIZER_FIVE_BIT_SHIFT = 3,
	IMAGE_QUANTIZER_RGB565_RED_SHIFT = 11,
	IMAGE_QUANTIZER_RGB565_GREEN_SHIFT = 5,
	IMAGE_QUANTIZER_SQUARED_DIFF_COUNT = IMAGE_QUANTIZER_MAX_CHANNEL * 2 + 1
};

enum {
	IMAGE_QUANTIZER_MAX_REPEAT_MINUS_ONE = 255,
	IMAGE_QUANTIZER_COMPRESSION_MODE_1 = 1,
	IMAGE_QUANTIZER_DEFAULT_CLASS_RUN_NUMERATOR = 3,
	IMAGE_QUANTIZER_DEFAULT_CLASS_RUN_SHIFT = 2,
	IMAGE_QUANTIZER_OTHER_CLASS_RUN_SHIFT = 1
};

extern const char g_imageQuantizerDefaultFormat[5];
extern struct ImageQuantizerNode* g_imageQuantizerRoot;
extern unsigned int g_imageQuantizerMaxTreeDepth;
extern unsigned int g_imageQuantizerColorCount;
extern ImageQuantizerPaletteEntry* g_imageQuantizerPaletteEntries;
extern double g_imageQuantizerPruneThreshold;
extern double g_imageQuantizerNextPruneThreshold;
extern unsigned int* g_imageQuantizerSquaredDiffTable;
extern unsigned int g_imageQuantizerNodeCount;
extern unsigned int g_imageQuantizerBlockNodesRemaining;
extern struct ImageQuantizerNode* g_imageQuantizerNextFreeNode;
extern struct ImageQuantizerNodeBlock* g_imageQuantizerNodeBlocks;

/* Original IDB size: 6342 bytes. */
struct ImageQuantizerImage {
	/* IDB +0x0 */
	unsigned int field0000;
	unsigned int field0004;
	unsigned int field0008;
	char text000C[IMAGE_QUANTIZER_TEXT_CAPACITY];
	unsigned int field080C;
	unsigned int field0810;
	char formatName[IMAGE_QUANTIZER_TEXT_CAPACITY];
	/* IDB +0x1014 */
	void* ownedData1014;
	/* IDB +0x1018 */
	void* ownedData1018;
	/* IDB +0x101C */
	unsigned int field101C;
	unsigned int field1020;
	/* IDB +0x1024 */
	unsigned int imageClass;
	/* IDB +0x1028 */
	unsigned int compareAuxiliary;
	/* IDB +0x102C */
	unsigned int compressionMode;
	/* IDB +0x1030 */
	unsigned int width;
	/* IDB +0x1034 */
	unsigned int height;
	/* IDB +0x1038 */
	unsigned int field1038;
	/* IDB +0x103C */
	unsigned int field103C;
	unsigned int field1040;
	/* IDB +0x1044 */
	void* ownedData1044;
	/* IDB +0x1048 */
	void* ownedData1048;
	/* IDB +0x104C */
	void* ownedData104C;
	/* IDB +0x1050 */
	unsigned int field1050;
	unsigned int field1054;
	unsigned int field1058;
	unsigned int field105C;
	uint16_t field1060;
	float field1062;
	float field1066;
	unsigned int field106A;
	unsigned int field106E;
	unsigned int field1072;
	unsigned int field1076;
	unsigned int field107A;
	unsigned int field107E;
	/* IDB +0x1082 */
	void* ownedData1082;
	/* IDB +0x1086 */
	struct ImageQuantizerPixelRun* runs;
	/* IDB +0x108A */
	unsigned int field108A;
	/* IDB +0x108E */
	unsigned int runCount;
	/* IDB +0x1092 */
	unsigned int inputRunRemaining;
	/* IDB +0x1096 */
	unsigned int field1096;
	/* IDB +0x109A */
	void* ownedData109A;
	/* IDB +0x109E */
	unsigned int creationTime;
	/* IDB +0x10A2 */
	char text10A2[IMAGE_QUANTIZER_TEXT_CAPACITY];
	unsigned int field18A2;
	unsigned int field18A6;
	unsigned int field18AA;
	/* IDB +0x18AE */
	void* ownedData18AE;
	/* IDB +0x18B2 */
	void* ownedData18B2;
	/* IDB +0x18B6 */
	unsigned int field18B6;
	unsigned int field18BA;
	unsigned int field18BE;
	unsigned int field18C2;
};

/* Original IDB size: 82 bytes. */
struct ImageQuantizerNode {
	/* IDB +0x0 */
	uint8_t childIndex;
	/* IDB +0x1 */
	uint8_t level;
	/* IDB +0x2 */
	uint8_t childrenMask;
	/* IDB +0x3 */
	uint8_t midpointRed;
	/* IDB +0x4 */
	uint8_t midpointGreen;
	/* IDB +0x5 */
	uint8_t midpointBlue;
	/* IDB +0x6 */
	unsigned int paletteIndex;
	/* IDB +0xA */
	unsigned int pixelCount;
	/* IDB +0xE */
	double quantizationError;
	/* IDB +0x16 */
	double redSum;
	/* IDB +0x1E */
	double greenSum;
	/* IDB +0x26 */
	double blueSum;
	/* IDB +0x2E */
	struct ImageQuantizerNode* parent;
	/* IDB +0x32 */
	struct ImageQuantizerNode* children[IMAGE_QUANTIZER_CHILD_COUNT];
};

/* Original IDB size: 9 bytes. */
struct ImageQuantizerPaletteEntry {
	/* IDB +0x0 */
	uint8_t red;
	/* IDB +0x1 */
	uint8_t green;
	/* IDB +0x2 */
	uint8_t blue;
	/* IDB +0x3 */
	uint8_t gap03[6];
};

/* Original IDB size: 6 bytes. */
struct ImageQuantizerPixelRun {
	/* IDB +0x0 */
	uint8_t red;
	/* IDB +0x1 */
	uint8_t green;
	/* IDB +0x2 */
	uint8_t blue;
	/* IDB +0x3: Number of additional identical pixels; 0..255 represents runs of 1..256 pixels. */
	uint8_t repeatMinusOne;
	/* IDB +0x4 */
	uint16_t auxiliary;
};

/* Original IDB size: 167940 bytes. */
struct ImageQuantizerNodeBlock {
	/* IDB +0x0 */
	struct ImageQuantizerNode nodes[IMAGE_QUANTIZER_BLOCK_NODE_COUNT];
	/* IDB +0x29000 */
	struct ImageQuantizerNodeBlock* next;
};

/* Declarations follow ascending original IDB address. */

/* 0x49FDD0 */
void XW_NORETURN ImageQuantizer_FatalAllocationError(void);

/* 0x49FDE0 */
struct ImageQuantizerImage* ImageQuantizer_AllocateImage(void);

/* 0x49FF70 */
/* Nonnull images require an initial run, even when width * height is zero. */
void ImageQuantizer_CompressPixelRuns(struct ImageQuantizerImage* image);

/* 0x4A0160 */
void ImageQuantizer_DestroyImage(struct ImageQuantizerImage* image);

/* 0x4A0240 */
void ImageQuantizer_ClassifyImageColors(const struct ImageQuantizerImage* image);

/* 0x4A0510 */
void ImageQuantizer_BuildPaletteEntriesRecursive(struct ImageQuantizerNode* node);

/* 0x4A0600 */
void ImageQuantizer_InitializeColorTree(int requestedDepth);

/* 0x4A06E0 */
struct ImageQuantizerNode* ImageQuantizer_AllocateNode(uint8_t childIndex, uint8_t level,
													   struct ImageQuantizerNode* parent, uint8_t midpointRed,
													   uint8_t midpointGreen, uint8_t midpointBlue);

/* 0x4A07A0 */
void ImageQuantizer_CollapseDeepestLevelRecursive(struct ImageQuantizerNode* node);

/* 0x4A0800 */
/* Requires a non-root node with a valid child index (0..7). Storage remains in the node arena. */
void ImageQuantizer_MergeNodeIntoParent(struct ImageQuantizerNode* node);

/* 0x4A0850 */
void ImageQuantizer_ReduceColorTree(unsigned int targetColorCount);

/* 0x4A08E0 */
void ImageQuantizer_ReduceColorTreePassRecursive(struct ImageQuantizerNode* node);

/* 0x4A0970 */
void ImageQuantizer_w_InitializeColorTree(int unusedColorLimit, int requestedDepth);

/* 0x4A0980 */
void ImageQuantizer_ExportPalette6BitAndDestroy(int targetColorCount, int unused, uint8_t* paletteRgb6);

/* 0x4A0A80 */
void ImageQuantizer_ClassifyIndexed16BppImage(uint8_t* indexedPixels, const uint16_t* paletteRgb565,
											  unsigned int width, unsigned int height);

#ifdef __cplusplus
}
#endif

#endif
