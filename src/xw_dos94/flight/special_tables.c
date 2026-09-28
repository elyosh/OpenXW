#include "xw_dos94/flight/special_world.h"
#include <string.h>

/* DOS94 0x4FEDE. */
static const struct {
	uint8_t count;
	XwSurfaceObjectPlacement placements[14];
} surfacePlacement0 = { 14,
						{ { 43, 77 },
						  { 190, 74 },
						  { 141, 79 },
						  { 168, 77 },
						  { 232, 75 },
						  { 53, 76 },
						  { 212, 76 },
						  { 83, 78 },
						  { 198, 80 },
						  { 132, 81 },
						  { 51, 82 },
						  { 219, 84 },
						  { 104, 86 },
						  { 61, 87 } } };

/* DOS94 0x4FF3A. */
static const struct {
	uint8_t count;
	XwSurfaceObjectPlacement placements[14];
} surfacePlacement1 = { 14,
						{ { 63, 75 },
						  { 254, 78 },
						  { 141, 76 },
						  { 42, 76 },
						  { 247, 78 },
						  { 118, 79 },
						  { 34, 74 },
						  { 97, 77 },
						  { 194, 80 },
						  { 70, 82 },
						  { 68, 82 },
						  { 189, 83 },
						  { 91, 85 },
						  { 199, 86 } } };

/* DOS94 0x4FEB0. */
static const struct {
	uint8_t count;
	XwSurfaceObjectPlacement placements[14];
} surfacePlacement2 = { 14,
						{ { 92, 76 },
						  { 142, 74 },
						  { 234, 75 },
						  { 200, 76 },
						  { 38, 77 },
						  { 117, 79 },
						  { 114, 78 },
						  { 163, 74 },
						  { 205, 80 },
						  { 51, 80 },
						  { 61, 83 },
						  { 169, 85 },
						  { 212, 86 },
						  { 89, 84 } } };

/* DOS94 0x4FF0C. */
static const struct {
	uint8_t count;
	XwSurfaceObjectPlacement placements[14];
} surfacePlacement3 = { 14,
						{ { 238, 74 },
						  { 157, 76 },
						  { 44, 77 },
						  { 136, 79 },
						  { 216, 75 },
						  { 53, 76 },
						  { 211, 76 },
						  { 130, 78 },
						  { 51, 81 },
						  { 61, 82 },
						  { 93, 82 },
						  { 205, 83 },
						  { 180, 84 },
						  { 74, 87 } } };

/* DOS94 0x4FF68. */
static const struct {
	uint8_t count;
	XwSurfaceObjectPlacement placements[8];
} surfacePlacement4 = {
	8, { { 24, 76 }, { 56, 79 }, { 88, 76 }, { 120, 79 }, { 152, 76 }, { 184, 79 }, { 216, 76 }, { 248, 79 } }
};

/* DOS94 0x4FF98. */
static const struct {
	uint8_t count;
	XwSurfaceObjectPlacement placements[17];
} trenchPlacement0 = { 17,
					   { { 197, 101 },
						 { 204, 100 },
						 { 229, 100 },
						 { 58, 102 },
						 { 17, 92 },
						 { 145, 98 },
						 { 25, 92 },
						 { 153, 98 },
						 { 30, 99 },
						 { 224, 94 },
						 { 48, 96 },
						 { 244, 97 },
						 { 57, 91 },
						 { 34, 107 },
						 { 62, 103 },
						 { 10, 104 },
						 { 170, 105 } } };

/* DOS94 0x4FFCC. */
static const struct {
	uint8_t count;
	XwSurfaceObjectPlacement placements[17];
} trenchPlacement1 = { 17,
					   { { 196, 101 },
						 { 205, 101 },
						 { 34, 102 },
						 { 185, 100 },
						 { 5, 92 },
						 { 17, 90 },
						 { 252, 90 },
						 { 213, 94 },
						 { 216, 92 },
						 { 232, 92 },
						 { 169, 95 },
						 { 45, 95 },
						 { 10, 107 },
						 { 38, 103 },
						 { 246, 105 },
						 { 24, 93 },
						 { 41, 93 } } };

/* DOS94 0x50000. */
static const struct {
	uint8_t count;
	XwSurfaceObjectPlacement placements[19];
} trenchPlacement2 = { 19,
					   { { 8, 101 },
						 { 16, 100 },
						 { 120, 101 },
						 { 249, 101 },
						 { 0, 91 },
						 { 1, 91 },
						 { 153, 99 },
						 { 220, 92 },
						 { 221, 92 },
						 { 244, 90 },
						 { 188, 96 },
						 { 61, 94 },
						 { 14, 104 },
						 { 30, 107 },
						 { 38, 106 },
						 { 42, 106 },
						 { 37, 93 },
						 { 40, 93 },
						 { 122, 105 } } };

/* DOS94 0x50038. */
static const struct {
	uint8_t count;
	XwSurfaceObjectPlacement placements[19];
} trenchPlacement3 = { 19,
					   { { 18, 102 },
						 { 49, 101 },
						 { 157, 100 },
						 { 156, 101 },
						 { 196, 92 },
						 { 216, 98 },
						 { 24, 98 },
						 { 33, 92 },
						 { 32, 92 },
						 { 168, 99 },
						 { 41, 90 },
						 { 233, 97 },
						 { 245, 97 },
						 { 54, 103 },
						 { 38, 104 },
						 { 6, 107 },
						 { 190, 105 },
						 { 13, 93 },
						 { 60, 93 } } };

/* DOS94 0x50070. */
static const struct {
	uint8_t count;
	XwSurfaceObjectPlacement placements[18];
} trenchPlacement4 = { 18,
					   { { 152, 101 },
						 { 153, 101 },
						 { 168, 100 },
						 { 169, 100 },
						 { 10, 95 },
						 { 201, 97 },
						 { 140, 94 },
						 { 52, 95 },
						 { 121, 98 },
						 { 252, 97 },
						 { 60, 91 },
						 { 189, 96 },
						 { 6, 107 },
						 { 42, 107 },
						 { 26, 103 },
						 { 34, 104 },
						 { 150, 105 },
						 { 234, 105 } } };

/* DOS94 0x500A6. */
static const struct {
	uint8_t count;
	XwSurfaceObjectPlacement placements[19];
} trenchPlacement5 = { 19,
					   { { 18, 102 },
						 { 224, 101 },
						 { 225, 101 },
						 { 58, 102 },
						 { 133, 99 },
						 { 204, 92 },
						 { 21, 92 },
						 { 88, 96 },
						 { 228, 97 },
						 { 40, 90 },
						 { 49, 94 },
						 { 180, 98 },
						 { 245, 91 },
						 { 38, 107 },
						 { 14, 103 },
						 { 62, 106 },
						 { 138, 105 },
						 { 25, 93 },
						 { 60, 93 } } };

/* DOS94 0x500DE. */
static const struct {
	uint8_t count;
	XwSurfaceObjectPlacement placements[10];
} trenchPlacement6 = { 10,
					   { { 136, 100 },
						 { 137, 100 },
						 { 184, 101 },
						 { 185, 101 },
						 { 22, 92 },
						 { 21, 91 },
						 { 212, 92 },
						 { 152, 90 },
						 { 34, 98 },
						 { 165, 94 } } };

/* DOS94 0x50118. */
static const struct {
	uint8_t count;
	XwSurfaceObjectPlacement placements[18];
} trenchPlacement7 = { 18,
					   { { 204, 101 },
						 { 76, 100 },
						 { 42, 102 },
						 { 185, 100 },
						 { 201, 92 },
						 { 208, 97 },
						 { 26, 97 },
						 { 61, 92 },
						 { 38, 99 },
						 { 229, 90 },
						 { 56, 91 },
						 { 10, 103 },
						 { 54, 107 },
						 { 22, 104 },
						 { 42, 106 },
						 { 60, 93 },
						 { 13, 93 },
						 { 28, 93 } } };

/* DOS94 0x5014E. */
static const struct {
	uint8_t count;
	XwSurfaceObjectPlacement placements[13];
} trenchPlacement8 = { 13,
					   { { 62, 108 },
						 { 132, 101 },
						 { 133, 101 },
						 { 208, 101 },
						 { 209, 101 },
						 { 92, 101 },
						 { 93, 101 },
						 { 168, 101 },
						 { 169, 101 },
						 { 14, 102 },
						 { 26, 102 },
						 { 38, 102 },
						 { 50, 102 } } };

/* DOS94 0x4FEFC. */
static const struct {
	uint8_t count;
	uint8_t health[14];
} surfaceHealth0 = { 14, { 40, 20, 40, 40, 5, 20, 20, 10, 100, 125, 50, 150, 150, 125 } };

/* DOS94 0x4FF58. */
static const struct {
	uint8_t count;
	uint8_t health[14];
} surfaceHealth1 = { 14, { 5, 10, 20, 20, 10, 40, 20, 40, 100, 50, 50, 125, 125, 150 } };

/* DOS94 0x4FECE. */
static const struct {
	uint8_t count;
	uint8_t health[14];
} surfaceHealth2 = { 14, { 20, 20, 5, 20, 40, 40, 10, 20, 100, 100, 125, 125, 150, 150 } };

/* DOS94 0x4FF2A. */
static const struct {
	uint8_t count;
	uint8_t health[14];
} surfaceHealth3 = { 14, { 20, 20, 40, 40, 5, 20, 20, 10, 125, 50, 50, 125, 150, 125 } };

/* DOS94 0x4FF7A. */
static const struct {
	uint8_t count;
	uint8_t health[8];
} surfaceHealth4 = { 8, { 60, 60, 60, 60, 60, 60, 60, 60 } };

/* DOS94 0x4FFBC. */
static const struct {
	uint8_t count;
	uint8_t health[14];
} trenchHealth0 = { 14, { 10, 10, 10, 10, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 } };

/* DOS94 0x4FFF0. */
static const struct {
	uint8_t count;
	uint8_t health[14];
} trenchHealth1 = { 14, { 10, 10, 10, 10, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 } };

/* DOS94 0x50028. */
static const struct {
	uint8_t count;
	uint8_t health[14];
} trenchHealth2 = { 14, { 10, 10, 10, 10, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 } };

/* DOS94 0x50060. */
static const struct {
	uint8_t count;
	uint8_t health[14];
} trenchHealth3 = { 14, { 10, 10, 10, 10, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 } };

/* DOS94 0x50096. */
static const struct {
	uint8_t count;
	uint8_t health[14];
} trenchHealth4 = { 14, { 10, 10, 10, 10, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 } };

/* DOS94 0x500CE. */
static const struct {
	uint8_t count;
	uint8_t health[14];
} trenchHealth5 = { 14, { 10, 10, 10, 10, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 } };

/* DOS94 0x50108. */
static const struct {
	uint8_t count;
	uint8_t health[14];
} trenchHealth6 = { 14, { 10, 10, 10, 10, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 } };

/* DOS94 0x5013E. */
static const struct {
	uint8_t count;
	uint8_t health[14];
} trenchHealth7 = { 14, { 10, 10, 10, 10, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 } };

/* DOS94 0x5016A. */
static const struct {
	uint8_t count;
	uint8_t health[13];
} trenchHealth8 = { 13, { 240, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10 } };

static const XwSurfaceDestructionPattern2 pattern0 = { 2, { { 0, 0, 512, 4, 0 }, { 0, 0, 3072, 4, 0 } } };
static const XwSurfaceDestructionPattern2 pattern1 = { 2, { { 0, 0, 256, 4, 0 }, { 0, 0, 1536, 4, 0 } } };
static const XwSurfaceDestructionPattern4 pattern2 = { 4,
													   { { 2304, 1024, 256, 4, 0 },
														 { 2304, -1024, 256, 4, 0 },
														 { -1280, 1024, 512, 4, 0 },
														 { -1280, -1024, 512, 4, 0 } } };
static const XwSurfaceDestructionPattern5 pattern3 = { 5,
													   { { 0, 0, 512, 4, 0 },
														 { 1536, 2816, 1408, 4, 0 },
														 { -1536, 2816, 1408, 4, 0 },
														 { 1536, -2816, 1408, 4, 0 },
														 { -1536, -2816, 1408, 4, 0 } } };
static const XwSurfaceDestructionPattern1 pattern4 = { 1, { { 0, 0, 256, 8, 0 } } };
static const XwSurfaceDestructionPattern4 pattern5 = { 4,
													   { { -1536, 1024, 512, 4, 0 },
														 { -1536, -1024, 512, 4, 0 },
														 { 2048, 1792, 1408, 4, 0 },
														 { 2048, -1792, 1408, 4, 0 } } };
static const XwSurfaceDestructionPattern4 pattern6 = { 4,
													   { { -2304, 2048, 640, 4, 0 },
														 { -2304, -2048, 640, 4, 0 },
														 { 2304, 2048, 640, 4, 0 },
														 { 2304, -2048, 640, 4, 0 } } };
static const XwSurfaceDestructionPattern3 pattern7 = {
	3, { { -4096, 1024, 320, 4, 0 }, { 4096, 1024, 320, 4, 0 }, { 0, 1024, 320, 4, 0 } }
};
static const XwSurfaceDestructionPattern4 pattern8 = { 4,
													   { { -1536, 3584, 768, 4, 0 },
														 { 1536, 3584, 768, 4, 0 },
														 { -1536, -3584, 768, 4, 0 },
														 { 1536, -3584, 768, 4, 0 } } };
static const XwSurfaceDestructionPattern1 pattern9 = { 1, { { 0, 0, 0, 4, 0 } } };
static const uint8_t surfaceIndices2[14] = { 1, 1, 1, 1, 0, 0, 0, 1, 2, 2, 5, 7, 6, 6 };
static const uint8_t surfaceIndices0[14] = { 0, 1, 0, 0, 1, 1, 1, 0, 2, 3, 4, 6, 6, 8 };
static const uint8_t surfaceIndices3[14] = { 1, 1, 0, 0, 1, 1, 1, 0, 3, 4, 4, 5, 6, 8 };
static const uint8_t surfaceIndices1[14] = { 1, 0, 1, 1, 0, 0, 1, 0, 2, 4, 4, 5, 7, 6 };
static const uint8_t surfaceIndices4[8] = { 1, 0, 1, 0, 1, 0, 1, 0 };
static const uint8_t trenchIndices[14] = { 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9 };
const uint8_t Dos94_trenchSurfaceColors[18] = {
	1, 2, 3, 10, 4, 8, 11, 5, 9, 12, 6, 10, 13, 7, 11, 14, 8, 12
};
static const XwSurfacePlacementList* const dos_surfacePlacement[5] = {
	(const XwSurfacePlacementList*)&surfacePlacement0, (const XwSurfacePlacementList*)&surfacePlacement1,
	(const XwSurfacePlacementList*)&surfacePlacement2, (const XwSurfacePlacementList*)&surfacePlacement3,
	(const XwSurfacePlacementList*)&surfacePlacement4
};
static const XwSurfacePlacementList* windows_surfacePlacement[5];
static const XwSurfacePlacementList* const dos_trenchPlacement[9] = {
	(const XwSurfacePlacementList*)&trenchPlacement0, (const XwSurfacePlacementList*)&trenchPlacement1,
	(const XwSurfacePlacementList*)&trenchPlacement2, (const XwSurfacePlacementList*)&trenchPlacement3,
	(const XwSurfacePlacementList*)&trenchPlacement4, (const XwSurfacePlacementList*)&trenchPlacement5,
	(const XwSurfacePlacementList*)&trenchPlacement6, (const XwSurfacePlacementList*)&trenchPlacement7,
	(const XwSurfacePlacementList*)&trenchPlacement8
};
static const XwSurfacePlacementList* windows_trenchPlacement[9];
static const XwSurfaceHealthList* const dos_surfaceHealth[5] = {
	(const XwSurfaceHealthList*)&surfaceHealth0, (const XwSurfaceHealthList*)&surfaceHealth1,
	(const XwSurfaceHealthList*)&surfaceHealth2, (const XwSurfaceHealthList*)&surfaceHealth3,
	(const XwSurfaceHealthList*)&surfaceHealth4
};
static const XwSurfaceHealthList* windows_surfaceHealth[5];
static const XwSurfaceHealthList* const dos_trenchHealth[9] = {
	(const XwSurfaceHealthList*)&trenchHealth0, (const XwSurfaceHealthList*)&trenchHealth1,
	(const XwSurfaceHealthList*)&trenchHealth2, (const XwSurfaceHealthList*)&trenchHealth3,
	(const XwSurfaceHealthList*)&trenchHealth4, (const XwSurfaceHealthList*)&trenchHealth5,
	(const XwSurfaceHealthList*)&trenchHealth6, (const XwSurfaceHealthList*)&trenchHealth7,
	(const XwSurfaceHealthList*)&trenchHealth8
};
static const XwSurfaceHealthList* windows_trenchHealth[9];
static const XwSurfaceDestructionPattern* const dos_pattern[10] = {
	(const XwSurfaceDestructionPattern*)&pattern0, (const XwSurfaceDestructionPattern*)&pattern1,
	(const XwSurfaceDestructionPattern*)&pattern2, (const XwSurfaceDestructionPattern*)&pattern3,
	(const XwSurfaceDestructionPattern*)&pattern4, (const XwSurfaceDestructionPattern*)&pattern5,
	(const XwSurfaceDestructionPattern*)&pattern6, (const XwSurfaceDestructionPattern*)&pattern7,
	(const XwSurfaceDestructionPattern*)&pattern8, (const XwSurfaceDestructionPattern*)&pattern9
};
static const XwSurfaceDestructionPattern* windows_pattern[10];
static const uint8_t* const dos_surfaceIndices[5] = { surfaceIndices0, surfaceIndices1, surfaceIndices2,
													  surfaceIndices3, surfaceIndices4 };
static const uint8_t* windows_surfaceIndices[5];
static const uint8_t* const dos_trenchIndices[9] = { trenchIndices, trenchIndices, trenchIndices,
													 trenchIndices, trenchIndices, trenchIndices,
													 trenchIndices, trenchIndices, trenchIndices };
static const uint8_t* windows_trenchIndices[9];

void Dos94World_Select(bool dos) {
	static bool captured;
	if (!captured) {
		memcpy(windows_surfacePlacement, g_surfacePlacementLists, sizeof g_surfacePlacementLists);
		memcpy(windows_trenchPlacement, g_trenchPlacementLists, sizeof g_trenchPlacementLists);
		memcpy(windows_surfaceHealth, g_surfaceHealthLists, sizeof g_surfaceHealthLists);
		memcpy(windows_trenchHealth, g_trenchHealthLists, sizeof g_trenchHealthLists);
		memcpy(windows_pattern, g_surfaceDestructionPatterns, sizeof g_surfaceDestructionPatterns);
		memcpy(windows_surfaceIndices, g_surfaceDestructionPatternIndices,
			   sizeof g_surfaceDestructionPatternIndices);
		memcpy(windows_trenchIndices, g_trenchDestructionPatternIndices,
			   sizeof g_trenchDestructionPatternIndices);
		captured = true;
	}
	memcpy(g_surfacePlacementLists, dos ? dos_surfacePlacement : windows_surfacePlacement,
		   sizeof g_surfacePlacementLists);
	memcpy(g_trenchPlacementLists, dos ? dos_trenchPlacement : windows_trenchPlacement,
		   sizeof g_trenchPlacementLists);
	memcpy(g_surfaceHealthLists, dos ? dos_surfaceHealth : windows_surfaceHealth,
		   sizeof g_surfaceHealthLists);
	memcpy(g_trenchHealthLists, dos ? dos_trenchHealth : windows_trenchHealth, sizeof g_trenchHealthLists);
	memcpy(g_surfaceDestructionPatterns, dos ? dos_pattern : windows_pattern,
		   sizeof g_surfaceDestructionPatterns);
	memcpy(g_surfaceDestructionPatternIndices, dos ? dos_surfaceIndices : windows_surfaceIndices,
		   sizeof g_surfaceDestructionPatternIndices);
	memcpy(g_trenchDestructionPatternIndices, dos ? dos_trenchIndices : windows_trenchIndices,
		   sizeof g_trenchDestructionPatternIndices);
}
