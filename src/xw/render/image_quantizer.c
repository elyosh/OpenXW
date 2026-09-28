#include "xw/render/image_quantizer.h"

#include "xw/flight/fediskio.h"
#include "xw/util/shared.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

// GLOBAL: XW 0x4DA928
const char g_imageQuantizerDefaultFormat[5] = "MIFF";

// GLOBAL: XW 0x5619F0
struct ImageQuantizerNode* g_imageQuantizerRoot = NULL;

// GLOBAL: XW 0x5619F4
unsigned int g_imageQuantizerMaxTreeDepth = 0;

// GLOBAL: XW 0x5619F8
unsigned int g_imageQuantizerColorCount = 0;

// GLOBAL: XW 0x561A05
ImageQuantizerPaletteEntry* g_imageQuantizerPaletteEntries = NULL;

// GLOBAL: XW 0x561A11
double g_imageQuantizerPruneThreshold = 0.0;

// GLOBAL: XW 0x561A19
double g_imageQuantizerNextPruneThreshold = 0.0;

// GLOBAL: XW 0x561A21
unsigned int* g_imageQuantizerSquaredDiffTable = NULL;

// GLOBAL: XW 0x561A25
unsigned int g_imageQuantizerNodeCount = 0;

// GLOBAL: XW 0x561A29
unsigned int g_imageQuantizerBlockNodesRemaining = 0;

// GLOBAL: XW 0x561A31
struct ImageQuantizerNode* g_imageQuantizerNextFreeNode = NULL;

// GLOBAL: XW 0x561A35
struct ImageQuantizerNodeBlock* g_imageQuantizerNodeBlocks = NULL;

// FUNCTION: XW 0x49FDD0
void XW_NORETURN ImageQuantizer_FatalAllocationError(void) { fediskio_fatalerror(FEDISKIO_ERROR_ALLOCATION); }

// FUNCTION: XW 0x49FDE0
struct ImageQuantizerImage* ImageQuantizer_AllocateImage(void) {
	ImageQuantizerImage* image;

	image = malloc(sizeof(*image));
	if (image == NULL) {
		ImageQuantizer_FatalAllocationError();
		return NULL;
	}

	image->field0000 = 0;
	image->field0004 = 0;
	image->field0008 = 0;
	image->text000C[0] = '\0';
	image->field080C = 0;
	image->field0810 = 0;
	memcpy(image->formatName, g_imageQuantizerDefaultFormat, sizeof(g_imageQuantizerDefaultFormat));
	image->ownedData1014 = NULL;
	image->ownedData1018 = NULL;
	image->field101C = 0;
	image->field1020 = 0;
	image->imageClass = IMAGE_QUANTIZER_DEFAULT_CLASS;
	image->compareAuxiliary = 0;
	image->compressionMode = IMAGE_QUANTIZER_DEFAULT_COMPRESSION;
	image->width = 0;
	image->field1060 = IMAGE_QUANTIZER_DEFAULT_FIELD1060;
	image->height = 0;
	image->field1038 = IMAGE_QUANTIZER_DEFAULT_FIELD1038;
	image->field106E = 0;
	image->field103C = 0;
	image->field1040 = 0;
	image->field1062 = IMAGE_QUANTIZER_DEFAULT_FIELD1062;
	image->field1066 = IMAGE_QUANTIZER_DEFAULT_FIELD1066;
	image->field1058 = 0;
	image->field1076 = 0;
	image->ownedData1044 = NULL;
	image->ownedData1048 = NULL;
	image->ownedData104C = NULL;
	image->field1050 = 0;
	image->field1054 = 0;
	image->field105C = 0;
	image->field107A = 0;
	image->field1072 = 0;
	image->field106A = 0;
	image->field107E = 0;
	image->ownedData1082 = NULL;
	image->runs = NULL;
	image->field108A = 0;
	image->runCount = 0;
	image->field1096 = 0;
	image->ownedData109A = NULL;
	image->text10A2[0] = '\0';
	image->field18A2 = 0;
	image->field18A6 = 0;
	image->creationTime = (unsigned int)time(NULL);
	image->field18AA = 0;
	image->ownedData18AE = NULL;
	image->ownedData18B2 = NULL;
	image->field18B6 = 0;
	image->field18BA = 0;
	image->field18BE = 0;
	image->field18C2 = 0;
	return image;
}

// FUNCTION: XW 0x49FF70
void ImageQuantizer_CompressPixelRuns(struct ImageQuantizerImage* image) {
	if (image != NULL) {
		ImageQuantizerPixelRun* runs = image->runs;
		unsigned int sourceIndex = 0;
		unsigned int destinationIndex = 0;
		unsigned int pixelIndex;
		unsigned int repeatMinusOne = runs[0].repeatMinusOne;
		image->runCount = 0;
		image->inputRunRemaining = repeatMinusOne + 1;
		runs[0].repeatMinusOne = IMAGE_QUANTIZER_MAX_REPEAT_MINUS_ONE;
		if (image->compareAuxiliary != 0) {
			for (pixelIndex = 0; pixelIndex < image->width * image->height; ++pixelIndex) {
				if (image->inputRunRemaining != 0) {
					--image->inputRunRemaining;
				} else {
					++sourceIndex;
					image->inputRunRemaining = runs[sourceIndex].repeatMinusOne;
				}
				if (runs[sourceIndex].red == runs[destinationIndex].red &&
					runs[sourceIndex].green == runs[destinationIndex].green &&
					runs[sourceIndex].blue == runs[destinationIndex].blue &&
					runs[sourceIndex].auxiliary == runs[destinationIndex].auxiliary &&
					runs[destinationIndex].repeatMinusOne < IMAGE_QUANTIZER_MAX_REPEAT_MINUS_ONE) {
					++runs[destinationIndex].repeatMinusOne;
				} else {
					if (image->runCount != 0)
						++destinationIndex;
					++image->runCount;
					runs[destinationIndex] = runs[sourceIndex];
					runs[destinationIndex].repeatMinusOne = 0;
				}
			}
		} else {
			for (pixelIndex = 0; pixelIndex < image->width * image->height; ++pixelIndex) {
				if (image->inputRunRemaining != 0) {
					--image->inputRunRemaining;
				} else {
					++sourceIndex;
					image->inputRunRemaining = runs[sourceIndex].repeatMinusOne;
				}
				if (runs[sourceIndex].red == runs[destinationIndex].red &&
					runs[sourceIndex].green == runs[destinationIndex].green &&
					runs[sourceIndex].blue == runs[destinationIndex].blue &&
					runs[destinationIndex].repeatMinusOne < IMAGE_QUANTIZER_MAX_REPEAT_MINUS_ONE) {
					++runs[destinationIndex].repeatMinusOne;
				} else {
					if (image->runCount != 0)
						++destinationIndex;
					++image->runCount;
					runs[destinationIndex] = runs[sourceIndex];
					runs[destinationIndex].repeatMinusOne = 0;
				}
			}
		}
		image->runs = realloc(image->runs, image->runCount * sizeof(*image->runs));
		if (image->imageClass == IMAGE_QUANTIZER_DEFAULT_CLASS) {
			if (image->runCount >=
				(IMAGE_QUANTIZER_DEFAULT_CLASS_RUN_NUMERATOR * image->width * image->height) >>
				IMAGE_QUANTIZER_DEFAULT_CLASS_RUN_SHIFT)
				image->compressionMode = IMAGE_QUANTIZER_COMPRESSION_MODE_1;
		} else if (image->runCount >= (image->width * image->height) >>
				   IMAGE_QUANTIZER_OTHER_CLASS_RUN_SHIFT) {
			image->compressionMode = IMAGE_QUANTIZER_COMPRESSION_MODE_1;
		}
	}
}

// FUNCTION: XW 0x4A0160
void ImageQuantizer_DestroyImage(struct ImageQuantizerImage* image) {
	if (image != NULL) {
		if (image->ownedData1014 != NULL) {
			free(image->ownedData1014);
		}
		if (image->ownedData1018 != NULL) {
			free(image->ownedData1018);
		}
		if (image->ownedData1044 != NULL) {
			free(image->ownedData1044);
		}
		if (image->ownedData1048 != NULL) {
			free(image->ownedData1048);
		}
		if (image->ownedData104C != NULL) {
			free(image->ownedData104C);
		}
		if (image->ownedData1082 != NULL) {
			free(image->ownedData1082);
		}
		if (image->runs != NULL) {
			free(image->runs);
		}
		if (image->ownedData109A != NULL) {
			free(image->ownedData109A);
		}
		if (image->ownedData18AE != NULL) {
			free(image->ownedData18AE);
		}
		if (image->ownedData18B2 != NULL) {
			free(image->ownedData18B2);
		}
		free(image);
	}
}

// FUNCTION: XW 0x4A0240
void ImageQuantizer_ClassifyImageColors(const struct ImageQuantizerImage* image) {
	unsigned int runIndex;
	const ImageQuantizerPixelRun* runs;
	g_imageQuantizerRoot->quantizationError +=
		(double)image->width * (double)image->height * IMAGE_QUANTIZER_ROOT_ERROR_PER_PIXEL;
	runs = image->runs;
	for (runIndex = 0; runIndex < image->runCount; ++runIndex) {
		const ImageQuantizerPixelRun* run = &runs[runIndex];
		ImageQuantizerNode* node;
		int runPixelCount;
		unsigned int level;
		int channelBit;
		if (g_imageQuantizerNodeCount > IMAGE_QUANTIZER_NODE_LIMIT) {
			ImageQuantizer_CollapseDeepestLevelRecursive(g_imageQuantizerRoot);
			--g_imageQuantizerMaxTreeDepth;
		}
		node = g_imageQuantizerRoot;
		runPixelCount = run->repeatMinusOne + 1;
		for (level = 1, channelBit = IMAGE_QUANTIZER_MAX_DEPTH - 1; level <= g_imageQuantizerMaxTreeDepth;
			 ++level, --channelBit) {
			int octant = ((((unsigned int)run->red >> (uint8_t)channelBit) & 1) << 2) |
						 ((((unsigned int)run->green >> (uint8_t)channelBit) & 1) << 1) |
						 (((unsigned int)run->blue >> (uint8_t)channelBit) & 1);
			if (node->children[octant] == NULL) {
				unsigned int midpointStep;
				int blueStep, greenStep, redStep;
				node->childrenMask |= 1U << octant;
				midpointStep = (1U << channelBit) >> 1;
				blueStep =
					(octant & IMAGE_QUANTIZER_OCTANT_BLUE) != 0 ? (int)midpointStep : -(int)midpointStep;
				greenStep =
					(octant & IMAGE_QUANTIZER_OCTANT_GREEN) != 0 ? (int)midpointStep : -(int)midpointStep;
				redStep = (octant & IMAGE_QUANTIZER_OCTANT_RED) != 0 ? (int)midpointStep : -(int)midpointStep;
				node->children[octant] = ImageQuantizer_AllocateNode(
					octant, level, node, node->midpointRed + redStep, node->midpointGreen + greenStep,
					node->midpointBlue + blueStep);
				if (node->children[octant] == NULL) {
					ImageQuantizer_FatalAllocationError();
					exit(EXIT_FAILURE);
				}
				if (level == g_imageQuantizerMaxTreeDepth)
					++g_imageQuantizerColorCount;
			}
			node = node->children[octant];
			node->quantizationError +=
				((double)g_imageQuantizerSquaredDiffTable[run->blue - node->midpointBlue] +
				 ((double)g_imageQuantizerSquaredDiffTable[run->red - node->midpointRed] +
				  (double)g_imageQuantizerSquaredDiffTable[run->green - node->midpointGreen])) *
				(double)(unsigned int)runPixelCount;
		}
		node->pixelCount += runPixelCount;
		node->redSum += (double)((unsigned int)runPixelCount * run->red);
		node->greenSum += (double)((unsigned int)runPixelCount * run->green);
		node->blueSum += (double)((unsigned int)runPixelCount * run->blue);
		if (runIndex + 1 == image->runCount || runIndex % image->height == 0)
			nullsub_SharedNoOp();
	}
}

// FUNCTION: XW 0x4A0510
void ImageQuantizer_BuildPaletteEntriesRecursive(struct ImageQuantizerNode* node) {
	if (node->childrenMask != 0) {
		unsigned int childIndex;
		for (childIndex = 0; childIndex < IMAGE_QUANTIZER_CHILD_COUNT; ++childIndex) {
			if ((node->childrenMask & (1U << childIndex)) != 0) {
				ImageQuantizer_BuildPaletteEntriesRecursive(node->children[childIndex]);
			}
		}
	}
	if (node->pixelCount != 0) {
		double roundingBias = (double)(node->pixelCount / 2);
		double pixelDivisor = (double)node->pixelCount;
		g_imageQuantizerPaletteEntries[g_imageQuantizerColorCount].red =
			(uint8_t)((node->redSum + roundingBias) / pixelDivisor);
		g_imageQuantizerPaletteEntries[g_imageQuantizerColorCount].green =
			(uint8_t)((node->greenSum + roundingBias) / pixelDivisor);
		g_imageQuantizerPaletteEntries[g_imageQuantizerColorCount].blue =
			(uint8_t)((node->blueSum + roundingBias) / pixelDivisor);
		node->paletteIndex = g_imageQuantizerColorCount++;
	}
}

// FUNCTION: XW 0x4A0600
void ImageQuantizer_InitializeColorTree(int requestedDepth) {
	int channelDifference;
	g_imageQuantizerNodeBlocks = NULL;
	g_imageQuantizerNodeCount = 0;
	g_imageQuantizerBlockNodesRemaining = 0;
	if (requestedDepth > IMAGE_QUANTIZER_MAX_DEPTH)
		requestedDepth = IMAGE_QUANTIZER_MAX_DEPTH;
	if (requestedDepth < IMAGE_QUANTIZER_MIN_DEPTH)
		requestedDepth = IMAGE_QUANTIZER_MIN_DEPTH;
	g_imageQuantizerMaxTreeDepth = requestedDepth;
	g_imageQuantizerRoot =
		ImageQuantizer_AllocateNode(0, 0, NULL, IMAGE_QUANTIZER_ROOT_MIDPOINT, IMAGE_QUANTIZER_ROOT_MIDPOINT,
									IMAGE_QUANTIZER_ROOT_MIDPOINT);
	g_imageQuantizerSquaredDiffTable =
		malloc(IMAGE_QUANTIZER_SQUARED_DIFF_COUNT * sizeof(*g_imageQuantizerSquaredDiffTable));
	if (g_imageQuantizerRoot == NULL || g_imageQuantizerSquaredDiffTable == NULL) {
		ImageQuantizer_FatalAllocationError();
		exit(EXIT_FAILURE);
	}
	g_imageQuantizerRoot->parent = g_imageQuantizerRoot;
	g_imageQuantizerRoot->quantizationError = 0.0;
	g_imageQuantizerColorCount = 0;
	g_imageQuantizerSquaredDiffTable += IMAGE_QUANTIZER_MAX_CHANNEL;
	for (channelDifference = -IMAGE_QUANTIZER_MAX_CHANNEL; channelDifference <= IMAGE_QUANTIZER_MAX_CHANNEL;
		 ++channelDifference)
		g_imageQuantizerSquaredDiffTable[channelDifference] = channelDifference * channelDifference;
}

// FUNCTION: XW 0x4A06E0
struct ImageQuantizerNode* ImageQuantizer_AllocateNode(uint8_t childIndex, uint8_t level,
													   struct ImageQuantizerNode* parent, uint8_t midpointRed,
													   uint8_t midpointGreen, uint8_t midpointBlue) {
	unsigned int remaining = g_imageQuantizerBlockNodesRemaining;
	ImageQuantizerNode* node;
	if (remaining == 0) {
		ImageQuantizerNodeBlock* block = malloc(sizeof(*block));
		if (block == NULL) {
			return NULL;
		}
		block->next = g_imageQuantizerNodeBlocks;
		g_imageQuantizerNodeBlocks = block;
		remaining = IMAGE_QUANTIZER_BLOCK_NODE_COUNT;
		node = &block->nodes[0];
	} else {
		node = g_imageQuantizerNextFreeNode;
	}
	++g_imageQuantizerNodeCount;
	g_imageQuantizerNextFreeNode = node + 1;
	g_imageQuantizerBlockNodesRemaining = remaining - 1;
	node->parent = parent;
	memset(node->children, 0, sizeof(node->children));
	node->childIndex = childIndex;
	node->level = level;
	node->childrenMask = 0;
	node->midpointRed = midpointRed;
	node->midpointBlue = midpointBlue;
	node->midpointGreen = midpointGreen;
	node->pixelCount = 0;
	node->quantizationError = 0.0;
	node->redSum = 0.0;
	node->greenSum = 0.0;
	node->blueSum = 0.0;
	return node;
}

// FUNCTION: XW 0x4A07A0
void ImageQuantizer_CollapseDeepestLevelRecursive(struct ImageQuantizerNode* node) {
	int childIndex;
	if (node->childrenMask != 0) {
		for (childIndex = 0; childIndex < IMAGE_QUANTIZER_CHILD_COUNT; ++childIndex) {
			if ((node->childrenMask & (1u << childIndex)) != 0) {
				ImageQuantizer_CollapseDeepestLevelRecursive(node->children[childIndex]);
			}
		}
	}
	if (node->level == g_imageQuantizerMaxTreeDepth) {
		ImageQuantizer_MergeNodeIntoParent(node);
	}
}

// FUNCTION: XW 0x4A0800
void ImageQuantizer_MergeNodeIntoParent(struct ImageQuantizerNode* node) {
	ImageQuantizerNode* parent = node->parent;
	parent->childrenMask &= ~(1u << node->childIndex);
	parent->pixelCount += node->pixelCount;
	parent->redSum = node->redSum + parent->redSum;
	parent->greenSum = node->greenSum + parent->greenSum;
	parent->blueSum = node->blueSum + parent->blueSum;
	--g_imageQuantizerNodeCount;
}

// FUNCTION: XW 0x4A0850
void ImageQuantizer_ReduceColorTree(unsigned int targetColorCount) {
	g_imageQuantizerNextPruneThreshold = 1.0;
	while (g_imageQuantizerColorCount > targetColorCount) {
		g_imageQuantizerPruneThreshold = g_imageQuantizerNextPruneThreshold;
		g_imageQuantizerNextPruneThreshold = g_imageQuantizerRoot->quantizationError - 1.0;
		g_imageQuantizerColorCount = 0;
		ImageQuantizer_ReduceColorTreePassRecursive(g_imageQuantizerRoot);
		nullsub_SharedNoOp();
	}
}

// FUNCTION: XW 0x4A08E0
void ImageQuantizer_ReduceColorTreePassRecursive(struct ImageQuantizerNode* node) {
	unsigned int childIndex;
	if (node->childrenMask != 0) {
		for (childIndex = 0; childIndex < IMAGE_QUANTIZER_CHILD_COUNT; ++childIndex) {
			if ((node->childrenMask & (1u << childIndex)) != 0) {
				ImageQuantizer_ReduceColorTreePassRecursive(node->children[childIndex]);
			}
		}
	}
	if (!(node->quantizationError > g_imageQuantizerPruneThreshold)) {
		ImageQuantizer_MergeNodeIntoParent(node);
	} else {
		if (node->pixelCount > 0) {
			++g_imageQuantizerColorCount;
		}
		if (node->quantizationError < g_imageQuantizerNextPruneThreshold) {
			g_imageQuantizerNextPruneThreshold = node->quantizationError;
		}
	}
}

// FUNCTION: XW 0x4A0970
void ImageQuantizer_w_InitializeColorTree(int unusedColorLimit, int requestedDepth) {
	(void)unusedColorLimit;
	ImageQuantizer_InitializeColorTree(requestedDepth);
}

// FUNCTION: XW 0x4A0980
void ImageQuantizer_ExportPalette6BitAndDestroy(int targetColorCount, int unused, uint8_t* paletteRgb6) {
	int colorIndex;
	(void)unused;
	ImageQuantizer_ReduceColorTree(targetColorCount);
	g_imageQuantizerPaletteEntries =
		malloc(g_imageQuantizerColorCount * sizeof(*g_imageQuantizerPaletteEntries));
	if (g_imageQuantizerPaletteEntries == NULL) {
		ImageQuantizer_FatalAllocationError();
		exit(EXIT_FAILURE);
	}
	g_imageQuantizerColorCount = 0;
	ImageQuantizer_BuildPaletteEntriesRecursive(g_imageQuantizerRoot);
	for (colorIndex = 0; colorIndex < targetColorCount; ++colorIndex) {
		int green = g_imageQuantizerPaletteEntries[colorIndex].green;
		int blue = g_imageQuantizerPaletteEntries[colorIndex].blue;
		paletteRgb6[colorIndex * IMAGE_QUANTIZER_RGB_CHANNEL_COUNT + IMAGE_QUANTIZER_RED_CHANNEL] =
			g_imageQuantizerPaletteEntries[colorIndex].red >> IMAGE_QUANTIZER_SIX_BIT_SHIFT;
		paletteRgb6[colorIndex * IMAGE_QUANTIZER_RGB_CHANNEL_COUNT + IMAGE_QUANTIZER_GREEN_CHANNEL] =
			green >> IMAGE_QUANTIZER_SIX_BIT_SHIFT;
		paletteRgb6[colorIndex * IMAGE_QUANTIZER_RGB_CHANNEL_COUNT + IMAGE_QUANTIZER_BLUE_CHANNEL] =
			blue >> IMAGE_QUANTIZER_SIX_BIT_SHIFT;
	}
	free(g_imageQuantizerPaletteEntries);
	do {
		ImageQuantizerNodeBlock* nextBlock = g_imageQuantizerNodeBlocks->next;
		free(g_imageQuantizerNodeBlocks);
		g_imageQuantizerNodeBlocks = nextBlock;
	} while (g_imageQuantizerNodeBlocks != NULL);
	g_imageQuantizerSquaredDiffTable -= IMAGE_QUANTIZER_MAX_CHANNEL;
	free(g_imageQuantizerSquaredDiffTable);
}

// FUNCTION: XW 0x4A0A80
void ImageQuantizer_ClassifyIndexed16BppImage(uint8_t* indexedPixels, const uint16_t* paletteRgb565,
											  unsigned int width, unsigned int height) {
	ImageQuantizerImage* image = ImageQuantizer_AllocateImage();
	if (image != NULL) {
		unsigned int row, column, pixelIndex;
		ImageQuantizerPixelRun* runs;
		image->height = height;
		image->runCount = width * height;
		image->compareAuxiliary = 0;
		image->width = width;
		image->runs = malloc(image->runCount * sizeof(*image->runs));
		if (image->runs == NULL)
			fediskio_fatalerror(FEDISKIO_ERROR_ALLOCATION);
		runs = image->runs;
		pixelIndex = 0;
		for (row = 0; row < image->height; ++row) {
			for (column = 0; column < image->width; ++pixelIndex, ++column) {
				ImageQuantizerPixelRun* run = &runs[pixelIndex];
				run->red =
					(uint16_t)(paletteRgb565[indexedPixels[pixelIndex]] >> IMAGE_QUANTIZER_RGB565_RED_SHIFT) *
					(1 << IMAGE_QUANTIZER_FIVE_BIT_SHIFT);
				run->green = ((unsigned int)paletteRgb565[indexedPixels[pixelIndex]] >>
							  IMAGE_QUANTIZER_RGB565_GREEN_SHIFT) *
							 (1 << IMAGE_QUANTIZER_SIX_BIT_SHIFT);
				run->blue = paletteRgb565[indexedPixels[pixelIndex]] << IMAGE_QUANTIZER_FIVE_BIT_SHIFT;
				run->auxiliary = 0;
				run->repeatMinusOne = 0;
			}
		}
		ImageQuantizer_CompressPixelRuns(image);
		ImageQuantizer_ClassifyImageColors(image);
		ImageQuantizer_DestroyImage(image);
	}
}
