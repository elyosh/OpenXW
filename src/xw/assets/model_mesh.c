#include "xw/assets/model_mesh.h"

#include "xw/flight/object/anim.h"
#include "xw/util/folded.h"
#include "xw/util/memory.h"
#include "xw/util/shared.h"

#include <stdlib.h>

// GLOBAL: XW 0x4C5030
const uint16_t g_modelType133AnimationFrames[] = {
	ANIM_FRAME_ADVANCE,
	ANIM_FRAME_JUMP_BASE,
	0xc280,
	0xc281,
	0xc282,
	0xc283,
	0xc284,
	0xc285,
	0xc286,
	0xc286,
	0xc287,
	0xc288,
	ANIM_FRAME_END_OBJECT,
};

// GLOBAL: XW 0x4C5050
const uint16_t g_modelType137AnimationFrames[] = {
	ANIM_FRAME_ADVANCE, ANIM_FRAME_JUMP_BASE, 0xc480, 0xc481, 0xc482, 0xc483, ANIM_FRAME_END_OBJECT,
};

// GLOBAL: XW 0x4C5060
uint16_t g_fragmentSecondaryAnimationFrames[] = {
	ANIM_FRAME_ADVANCE,
	ANIM_FRAME_JUMP_BASE,
	0xc500,
	0xc501,
	0xc502,
	0xc503,
	0xc504,
	ANIM_FRAME_END_OBJECT,
	0xc580,
	ANIM_FRAME_JUMP_BASE,
	0xc600,
	ANIM_FRAME_JUMP_BASE,
	ANIM_FRAME_ADVANCE,
	ANIM_FRAME_JUMP_BASE,
	0xc680,
	0xc681,
	0xc682,
	0xc683,
	0xc684,
	0xc685,
	0xc686,
	0xc687,
	0xc688,
	0xc689,
	0xc68a,
	ANIM_FRAME_RESTART,
	0,
	0,
};

// GLOBAL: XW 0x4C5098
const uint16_t g_componentDamageAnimationFrames[] = {
	ANIM_FRAME_ADVANCE,
	ANIM_FRAME_JUMP_BASE,
	0xc680,
	0xc681,
	0xc682,
	0xc683,
	0xc684,
	0xc685,
	0xc686,
	0xc687,
	0xc688,
	0xc689,
	0xc700,
	0xc700,
	0xc701,
	0xc701,
	0xc702,
	0xc702,
	0xc703,
	0xc703,
	0xc704,
	0xc704,
	0xc705,
	0xc705,
	ANIM_FRAME_JUMP_BASE + ANIM_COMPONENT_DAMAGE_LOOP_FRAME,
};

// GLOBAL: XW 0x4C50D0
const uint16_t g_modelType110AnimationFrames[] = {
	ANIM_FRAME_ADVANCE,
	ANIM_FRAME_JUMP_BASE,
	0xb700,
	0xb701,
	0xb702,
	0xb703,
	0xb704,
	0xb705,
	ANIM_FRAME_JUMP_BASE + ANIM_FIRST_DISPLAY_FRAME,
};

// GLOBAL: XW 0x4C50E8
const uint16_t g_modelType111AnimationFrames[] = {
	ANIM_FRAME_ADVANCE,
	ANIM_FRAME_JUMP_BASE,
	0xb780,
	0xb781,
	0xb782,
	0xb783,
	0xb784,
	0xb785,
	0xb786,
	0xb787,
	ANIM_FRAME_JUMP_BASE + ANIM_FIRST_DISPLAY_FRAME,
};

// GLOBAL: XW 0x4C5100
const uint16_t g_modelType112AnimationFrames[] = {
	ANIM_FRAME_ADVANCE,
	ANIM_FRAME_JUMP_BASE,
	0xb800,
	0xb801,
	0xb802,
	0xb803,
	0xb804,
	0xb805,
	0xb806,
	0xb807,
	ANIM_FRAME_JUMP_BASE + ANIM_FIRST_DISPLAY_FRAME,
};

// GLOBAL: XW 0x4C5118
const uint16_t g_modelType113AnimationFrames[] = {
	ANIM_FRAME_ADVANCE,
	ANIM_FRAME_JUMP_BASE,
	0xb880,
	0xb881,
	0xb882,
	0xb883,
	0xb884,
	0xb885,
	ANIM_FRAME_JUMP_BASE + ANIM_FIRST_DISPLAY_FRAME,
};

// GLOBAL: XW 0x4C5130
const uint16_t g_modelType134AnimationFrames[] = {
	ANIM_FRAME_ADVANCE,
	ANIM_FRAME_JUMP_BASE,
	0xc300,
	0xc301,
	0xc302,
	0xc303,
	0xc304,
	0xc305,
	0xc306,
	0xc307,
	0xc308,
	0xc309,
	ANIM_FRAME_END_OBJECT,
};

// GLOBAL: XW 0x4C5150
const uint16_t g_modelType135AnimationFrames[] = {
	ANIM_FRAME_ADVANCE,
	ANIM_FRAME_JUMP_BASE,
	0xc380,
	0xc381,
	0xc382,
	0xc383,
	0xc384,
	0xc385,
	0xc386,
	0xc387,
	0xc388,
	0xc389,
	0xc38a,
	0xc38b,
	0xc38c,
	ANIM_FRAME_END_OBJECT,
};

// GLOBAL: XW 0x4C5170
const uint16_t g_modelType136AnimationFrames[] = {
	ANIM_FRAME_ADVANCE,
	ANIM_FRAME_JUMP_BASE,
	0xc400,
	0xc401,
	0xc402,
	0xc403,
	0xc404,
	0xc405,
	0xc406,
	0xc407,
	0xc408,
	0xc409,
	0xc40a,
	0xc40b,
	ANIM_FRAME_END_OBJECT,
};

// GLOBAL: XW 0x4C9DD8
XwModelTypeRecord g_modelTypeTable[MODEL_TYPE_RECORD_COUNT] = {
	{ 0, 0, 0, XW_GENUS_STARFIGHTER, 0, 0, 0, NULL, { 0, 0, 0, 0 }, 0, 255, 3, 0 },
	{ 3, 1, 0, XW_GENUS_STARFIGHTER, 508, 125, 0, NULL, { 0, 0, 0, 0 }, 67, 0, 0, 0 },
	{ 3, 1, 0, XW_GENUS_STARFIGHTER, 651, 125, 0, NULL, { 0, 0, 0, 0 }, 67, 1, 0, 1 },
	{ 3, 1, 0, XW_GENUS_STARFIGHTER, 391, 125, 0, NULL, { 0, 0, 0, 0 }, 67, 2, 0, 2 },
	{ 3, 1, 0, XW_GENUS_STARFIGHTER, 680, 150, 0, NULL, { 0, 0, 0, 0 }, 67, 3, 0, 3 },
	{ 3, 1, 0, XW_GENUS_STARFIGHTER, 325, 150, 0, NULL, { 0, 0, 0, 0 }, 67, 4, 0, 4 },
	{ 3, 1, 0, XW_GENUS_STARFIGHTER, 391, 150, 0, NULL, { 0, 0, 0, 0 }, 67, 5, 0, 5 },
	{ 3, 1, 0, XW_GENUS_STARFIGHTER, 356, 150, 0, NULL, { 0, 0, 0, 0 }, 67, 6, 0, 6 },
	{ 3, 1, 0, XW_GENUS_STARFIGHTER, 450, 125, 0, NULL, { 0, 0, 0, 0 }, 67, 7, 0, 7 },
	{ 3, 1, 0, XW_GENUS_STARFIGHTER, 400, 125, 0, NULL, { 0, 0, 0, 0 }, 67, 8, 0, 8 },
	{ 0, 1, 0, XW_GENUS_STARFIGHTER, 356, 175, 0, NULL, { 0, 0, 0, 0 }, 67, 9, 3, 0 },
	{ 0, 1, 0, XW_GENUS_STARFIGHTER, 356, 175, 0, NULL, { 0, 0, 0, 0 }, 67, 10, 3, 0 },
	{ 3, 1, 0, XW_GENUS_STARFIGHTER, 356, 175, 0, NULL, { 0, 0, 0, 0 }, 67, 11, 1, 15 },
	{ 3, 1, 0, XW_GENUS_STARFIGHTER, 356, 175, 0, NULL, { 0, 0, 0, 0 }, 67, 12, 1, 9 },
	{ 3, 1, 0, XW_GENUS_STARFIGHTER, 500, 175, 0, NULL, { 0, 0, 0, 0 }, 67, 13, 0, 9 },
	{ 3, 1, 0, XW_GENUS_STARFIGHTER, 356, 175, 0, NULL, { 0, 0, 0, 0 }, 67, 14, 1, 10 },
	{ 3, 33, 0, XW_GENUS_STARFIGHTER, 620, 125, 0, NULL, { 0, 0, 0, 0 }, 67, 15, 0, 10 },
	{ 3, 33, 0, XW_GENUS_TRANSPORT, 930, 600, 0, NULL, { 0, 0, 0, 0 }, 67, 16, 0, 11 },
	{ 3, 33, 0, XW_GENUS_TRANSPORT, 930, 700, 0, NULL, { 0, 0, 0, 0 }, 67, 17, 0, 12 },
	{ 3, 33, 0, XW_GENUS_TRANSPORT, 2000, 700, 0, NULL, { 0, 0, 0, 0 }, 67, 18, 0, 13 },
	{ 3, 33, 0, XW_GENUS_TRANSPORT, 2000, 700, 0, NULL, { 0, 0, 0, 0 }, 67, 19, 1, 13 },
	{ 3, 33, 0, XW_GENUS_TRANSPORT, 600, 175, 0, NULL, { 0, 0, 0, 0 }, 67, 20, 0, 14 },
	{ 3, 33, 0, XW_GENUS_TRANSPORT, 800, 700, 0, NULL, { 0, 0, 0, 0 }, 67, 21, 0, 15 },
	{ 3, 33, 0, XW_GENUS_TRANSPORT, 930, 700, 0, NULL, { 0, 0, 0, 0 }, 67, 22, 1, 7 },
	{ 3, 33, 0, XW_GENUS_UTILITY, 210, 90, 0, NULL, { 0, 0, 0, 0 }, 67, 23, 0, 16 },
	{ 3, 33, 0, XW_GENUS_UTILITY, 186, 90, 0, NULL, { 0, 0, 0, 0 }, 67, 24, 1, 14 },
	{ 3, 1, 0, XW_GENUS_FREIGHTER, 3100, 800, 0, NULL, { 0, 0, 0, 0 }, 67, 25, 0, 17 },
	{ 3, 1, 0, XW_GENUS_FREIGHTER, 3100, 900, 0, NULL, { 0, 0, 0, 0 }, 67, 26, 0, 18 },
	{ 3, 1, 0, XW_GENUS_FREIGHTER, 3100, 900, 0, NULL, { 0, 0, 0, 0 }, 67, 27, 0, 19 },
	{ 3, 1, 0, XW_GENUS_FREIGHTER, 3100, 900, 0, NULL, { 0, 0, 0, 0 }, 67, 28, 0, 20 },
	{ 3, 1, 0, XW_GENUS_FREIGHTER, 3100, 900, 0, NULL, { 0, 0, 0, 0 }, 67, 29, 0, 21 },
	{ 0, 1, 0, XW_GENUS_FREIGHTER, 3100, 900, 0, NULL, { 0, 0, 0, 0 }, 67, 255, 3, 0 },
	{ 3, 33, 0, XW_GENUS_FREIGHTER, 4960, 1200, 0, NULL, { 0, 0, 0, 0 }, 67, 31, 0, 22 },
	{ 3, 33, 0, XW_GENUS_FREIGHTER, 5000, 900, 0, NULL, { 0, 0, 0, 0 }, 67, 32, 0, 23 },
	{ 3, 33, 0, XW_GENUS_FREIGHTER, 7500, 900, 0, NULL, { 0, 0, 0, 0 }, 67, 33, 0, 24 },
	{ 3, 33, 0, XW_GENUS_FREIGHTER, 3100, 900, 0, NULL, { 0, 0, 0, 0 }, 67, 34, 0, 74 },
	{ 0, 1, 0, XW_GENUS_FREIGHTER, 3100, 900, 0, NULL, { 0, 0, 0, 0 }, 67, 255, 3, 0 },
	{ 3, 33, 0, XW_GENUS_FREIGHTER, 3100, 900, 0, NULL, { 0, 0, 0, 0 }, 67, 36, 1, 11 },
	{ 3, 33, 0, XW_GENUS_FREIGHTER, 1100, 900, 0, NULL, { 0, 0, 0, 0 }, 67, 37, 0, 25 },
	{ 0, 1, 0, XW_GENUS_FREIGHTER, 1100, 900, 0, NULL, { 0, 0, 0, 0 }, 67, 255, 3, 0 },
	{ 3, 33, 0, XW_GENUS_FREIGHTER, 6200, 3100, 0, NULL, { 0, 0, 0, 0 }, 67, 39, 0, 26 },
	{ 3, 33, 0, XW_GENUS_FREIGHTER, 4175, 900, 0, NULL, { 0, 0, 0, 0 }, 67, 40, 0, 27 },
	{ 3, 33, 0, XW_GENUS_STARSHIP, 15000, 7500, 0, NULL, { 0, 0, 0, 0 }, 67, 41, 0, 28 },
	{ 3, 33, 0, XW_GENUS_STARSHIP, 15000, 7500, 0, NULL, { 0, 0, 0, 0 }, 67, 42, 0, 73 },
	{ 3, 33, 0, XW_GENUS_STARSHIP, 3100, 900, 0, NULL, { 0, 0, 0, 0 }, 67, 43, 1, 12 },
	{ 3, 33, 0, XW_GENUS_STARSHIP, 3100, 900, 0, NULL, { 0, 0, 0, 0 }, 67, 44, 1, 4 },
	{ 3, 33, 0, XW_GENUS_STARSHIP, 3100, 900, 0, NULL, { 0, 0, 0, 0 }, 67, 45, 1, 5 },
	{ 3, 33, 0, XW_GENUS_STARSHIP, 10000, 900, 0, NULL, { 0, 0, 0, 0 }, 67, 46, 0, 29 },
	{ 3, 33, 0, XW_GENUS_STARSHIP, 3100, 900, 0, NULL, { 0, 0, 0, 0 }, 67, 47, 1, 6 },
	{ 3, 33, 0, XW_GENUS_STARSHIP, 58000, 20000, 0, NULL, { 0, 0, 0, 0 }, 67, 48, 0, 71 },
	{ 3, 33, 0, XW_GENUS_STARSHIP, 58000, 20000, 0, NULL, { 0, 0, 0, 0 }, 67, 49, 0, 72 },
	{ 3, 33, 0, XW_GENUS_STARSHIP, 11000, 32000, 0, NULL, { 0, 0, 0, 0 }, 67, 50, 0, 30 },
	{ 3, 33, 0, XW_GENUS_STARSHIP, 64000, 32000, 0, NULL, { 0, 0, 0, 0 }, 67, 51, 0, 70 },
	{ 3, 33, 0, XW_GENUS_STARSHIP, 64000, 32000, 0, NULL, { 0, 0, 0, 0 }, 67, 52, 0, 69 },
	{ 0, 33, 0, XW_GENUS_STARSHIP, 64000, 32000, 0, NULL, { 0, 0, 0, 0 }, 67, 53, 3, 0 },
	{ 3, 1, 0, XW_GENUS_FREIGHTER, 3100, 900, 0, NULL, { 0, 0, 0, 0 }, 67, 54, 0, 75 },
	{ 3, 1, 0, XW_GENUS_FREIGHTER, 3100, 900, 0, NULL, { 0, 0, 0, 0 }, 67, 55, 1, 0 },
	{ 3, 1, 0, XW_GENUS_FREIGHTER, 3100, 900, 0, NULL, { 0, 0, 0, 0 }, 67, 56, 1, 1 },
	{ 3, 1, 0, XW_GENUS_FREIGHTER, 3100, 900, 0, NULL, { 0, 0, 0, 0 }, 67, 57, 1, 2 },
	{ 3, 1, 0, XW_GENUS_FREIGHTER, 3100, 900, 0, NULL, { 0, 0, 0, 0 }, 67, 58, 1, 3 },
	{ 3, 33, 0, XW_GENUS_STARSHIP, 9000, 32000, 0, NULL, { 0, 0, 0, 0 }, 67, 59, 0, 77 },
	{ 3, 33, 0, XW_GENUS_STARSHIP, 9000, 32000, 0, NULL, { 0, 0, 0, 0 }, 67, 60, 0, 31 },
	{ 3, 33, 0, XW_GENUS_STARSHIP, 9000, 32000, 0, NULL, { 0, 0, 0, 0 }, 67, 61, 0, 78 },
	{ 3, 33, 0, XW_GENUS_STARSHIP, 9000, 32000, 0, NULL, { 0, 0, 0, 0 }, 67, 62, 0, 81 },
	{ 3, 33, 0, XW_GENUS_STARSHIP, 9000, 32000, 0, NULL, { 0, 0, 0, 0 }, 67, 63, 0, 82 },
	{ 3, 33, 0, XW_GENUS_STARSHIP, 9000, 32000, 0, NULL, { 0, 0, 0, 0 }, 67, 64, 0, 83 },
	{ 3, 33, 0, XW_GENUS_STARSHIP, 9000, 32000, 0, NULL, { 0, 0, 0, 0 }, 67, 65, 2, 0 },
	{ 3, 33, 0, XW_GENUS_STARSHIP, 9000, 32000, 0, NULL, { 0, 0, 0, 0 }, 67, 66, 2, 1 },
	{ 3, 33, 0, XW_GENUS_STARSHIP, 9000, 32000, 0, NULL, { 0, 0, 0, 0 }, 67, 67, 2, 2 },
	{ 3, 33, 0, XW_GENUS_STARSHIP, 9000, 32000, 0, NULL, { 0, 0, 0, 0 }, 67, 68, 1, 8 },
	{ 3, 33, 2, XW_GENUS_BUOY_SATELLITE_PROBE, 300, 150, 0, NULL, { 0, 0, 0, 0 }, 131, 255, 0, 32 },
	{ 3, 33, 2, XW_GENUS_BUOY_SATELLITE_PROBE, 300, 150, 0, NULL, { 0, 0, 0, 0 }, 131, 255, 0, 32 },
	{ 0, 33, 2, XW_GENUS_BUOY_SATELLITE_PROBE, 300, 150, 0, NULL, { 0, 0, 0, 0 }, 131, 255, 3, 0 },
	{ 0, 33, 2, XW_GENUS_BUOY_SATELLITE_PROBE, 300, 150, 0, NULL, { 0, 0, 0, 0 }, 131, 255, 3, 0 },
	{ 0, 33, 2, XW_GENUS_BUOY_SATELLITE_PROBE, 300, 150, 0, NULL, { 0, 0, 0, 0 }, 131, 255, 3, 0 },
	{ 3, 33, 1, XW_GENUS_MINE, 500, 250, 0, NULL, { 0, 0, 0, 0 }, 131, 255, 0, 33 },
	{ 3, 33, 1, XW_GENUS_MINE, 500, 250, 0, NULL, { 0, 0, 0, 0 }, 131, 255, 0, 34 },
	{ 3, 33, 1, XW_GENUS_MINE, 500, 250, 0, NULL, { 0, 0, 0, 0 }, 131, 255, 0, 76 },
	{ 0, 0, 1, XW_GENUS_MINE, 500, 250, 0, NULL, { 0, 0, 0, 0 }, 131, 255, 3, 0 },
	{ 0, 0, 1, XW_GENUS_MINE, 500, 250, 0, NULL, { 0, 0, 0, 0 }, 131, 255, 3, 0 },
	{ 3, 33, 2, XW_GENUS_BUOY_SATELLITE_PROBE, 200, 100, 0, NULL, { 0, 0, 0, 0 }, 131, 255, 0, 35 },
	{ 3, 33, 2, XW_GENUS_BUOY_SATELLITE_PROBE, 400, 100, 0, NULL, { 0, 0, 0, 0 }, 131, 255, 0, 36 },
	{ 0, 33, 2, XW_GENUS_BUOY_SATELLITE_PROBE, 200, 100, 0, NULL, { 0, 0, 0, 0 }, 131, 255, 3, 0 },
	{ 3, 1, 2, XW_GENUS_BUOY_SATELLITE_PROBE, 250, 125, 0, NULL, { 0, 0, 0, 0 }, 131, 255, 0, 36 },
	{ 3, 1, 2, XW_GENUS_BUOY_SATELLITE_PROBE, 250, 125, 0, NULL, { 0, 0, 0, 0 }, 131, 255, 0, 36 },
	{ 0, 0, 3, XW_GENUS_DEBRIS, 480, 240, 0, NULL, { 0, 0, 0, 0 }, 131, 255, 3, 0 },
	{ 1, 0, 3, XW_GENUS_ASTEROID, 480, 240, 0, NULL, { 0, 0, 0, 0 }, 128, 255, 3, 0 },
	{ 1, 0, 3, XW_GENUS_PLANET_BACKDROP, 480, 240, 0, NULL, { 0, 0, 0, 0 }, 32, 255, 3, 47 },
	{ 1, 0, 3, XW_GENUS_SCENERY, 480, 240, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 0 },
	{ 0, 0, 3, XW_GENUS_DEBRIS, 480, 240, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 0 },
	{ 0, 0, 3, XW_GENUS_DEBRIS, 480, 240, 0, NULL, { 0, 0, 0, 0 }, 0, 255, 3, 0 },
	{ 0, 0, 3, XW_GENUS_DEBRIS, 480, 240, 0, NULL, { 0, 0, 0, 0 }, 0, 255, 3, 0 },
	{ 0, 0, 3, XW_GENUS_DEBRIS, 480, 240, 0, NULL, { 0, 0, 0, 0 }, 0, 255, 3, 0 },
	{ 0, 0, 3, XW_GENUS_DEBRIS, 480, 240, 0, NULL, { 0, 0, 0, 0 }, 0, 255, 3, 0 },
	{ 0, 0, 3, XW_GENUS_DEBRIS, 480, 240, 0, NULL, { 0, 0, 0, 0 }, 0, 255, 3, 0 },
	{ 0, 0, 3, XW_GENUS_DEBRIS, 480, 240, 0, NULL, { 0, 0, 0, 0 }, 0, 255, 3, 0 },
	{ 0, 0, 3, XW_GENUS_DEBRIS, 480, 240, 0, NULL, { 0, 0, 0, 0 }, 0, 255, 3, 0 },
	{ 0, 0, 3, XW_GENUS_DEBRIS, 480, 240, 0, NULL, { 0, 0, 0, 0 }, 0, 255, 3, 0 },
	{ 3, 73, 6, XW_GENUS_SCENERY, 480, 240, 0, NULL, { 0, 0, 0, 0 }, 67, 255, 0, 79 },
	{ 3, 73, 6, XW_GENUS_SCENERY, 480, 240, 0, NULL, { 0, 0, 0, 0 }, 67, 255, 0, 80 },
	{ 3, 33, 3, XW_GENUS_ASTEROID, 6000, 3000, 0, NULL, { 0, 0, 0, 0 }, 128, 255, 0, 37 },
	{ 3, 33, 3, XW_GENUS_ASTEROID, 6000, 3000, 0, NULL, { 0, 0, 0, 0 }, 128, 255, 0, 38 },
	{ 3, 33, 3, XW_GENUS_ASTEROID, 6000, 3000, 0, NULL, { 0, 0, 0, 0 }, 128, 255, 0, 39 },
	{ 3, 33, 3, XW_GENUS_ASTEROID, 6000, 3000, 0, NULL, { 0, 0, 0, 0 }, 128, 255, 0, 40 },
	{ 3, 33, 3, XW_GENUS_ASTEROID, 6000, 3000, 0, NULL, { 0, 0, 0, 0 }, 128, 255, 0, 41 },
	{ 3, 33, 3, XW_GENUS_ASTEROID, 6000, 3000, 0, NULL, { 0, 0, 0, 0 }, 128, 255, 0, 42 },
	{ 0, 0, 3, XW_GENUS_ASTEROID, 480, 240, 0, NULL, { 0, 0, 0, 0 }, 128, 255, 3, 0 },
	{ 0, 0, 3, XW_GENUS_ASTEROID, 480, 240, 0, NULL, { 0, 0, 0, 0 }, 128, 255, 3, 0 },
	{ 0, 0, 3, XW_GENUS_ASTEROID, 480, 240, 0, NULL, { 0, 0, 0, 0 }, 128, 255, 3, 0 },
	{ 0, 0, 3, XW_GENUS_ASTEROID, 480, 240, 0, NULL, { 0, 0, 0, 0 }, 128, 255, 3, 0 },
	{ 3,
	  10,
	  3,
	  XW_GENUS_DEBRIS,
	  512,
	  256,
	  0,
	  g_modelType110AnimationFrames,
	  { 112, 157, 76, 0 },
	  128,
	  255,
	  0,
	  43 },
	{ 3,
	  10,
	  3,
	  XW_GENUS_DEBRIS,
	  512,
	  256,
	  0,
	  g_modelType111AnimationFrames,
	  { 128, 157, 76, 0 },
	  128,
	  255,
	  0,
	  44 },
	{ 3,
	  10,
	  3,
	  XW_GENUS_DEBRIS,
	  512,
	  256,
	  0,
	  g_modelType112AnimationFrames,
	  { 144, 157, 76, 0 },
	  128,
	  255,
	  0,
	  45 },
	{ 3,
	  10,
	  3,
	  XW_GENUS_DEBRIS,
	  512,
	  256,
	  0,
	  g_modelType113AnimationFrames,
	  { 160, 157, 76, 0 },
	  128,
	  255,
	  0,
	  46 },
	{ 3, 10, 4, XW_GENUS_PLANET_BACKDROP, 480, 240, 0, NULL, { 192, 156, 76, 0 }, 32, 255, 0, 47 },
	{ 3, 10, 4, XW_GENUS_PLANET_BACKDROP, 480, 240, 0, NULL, { 208, 156, 76, 0 }, 32, 255, 0, 48 },
	{ 3, 10, 4, XW_GENUS_PLANET_BACKDROP, 480, 240, 0, NULL, { 224, 156, 76, 0 }, 32, 255, 0, 49 },
	{ 3, 10, 4, XW_GENUS_PLANET_BACKDROP, 480, 240, 0, NULL, { 240, 156, 76, 0 }, 32, 255, 0, 50 },
	{ 3, 10, 4, XW_GENUS_PLANET_BACKDROP, 480, 240, 0, NULL, { 0, 157, 76, 0 }, 32, 255, 0, 51 },
	{ 3, 10, 4, XW_GENUS_PLANET_BACKDROP, 480, 240, 0, NULL, { 16, 157, 76, 0 }, 32, 255, 0, 52 },
	{ 3, 10, 4, XW_GENUS_PLANET_BACKDROP, 480, 240, 0, NULL, { 32, 157, 76, 0 }, 32, 255, 0, 53 },
	{ 3, 10, 4, XW_GENUS_PLANET_BACKDROP, 480, 240, 0, NULL, { 48, 157, 76, 0 }, 32, 255, 0, 54 },
	{ 3, 10, 4, XW_GENUS_PLANET_BACKDROP, 480, 240, 0, NULL, { 96, 157, 76, 0 }, 32, 255, 3, 0 },
	{ 3, 10, 4, XW_GENUS_STARFIELD_BACKDROP, 480, 240, 0, NULL, { 176, 156, 76, 0 }, 32, 255, 0, 55 },
	{ 3, 10, 4, XW_GENUS_STARFIELD_BACKDROP, 480, 240, 0, NULL, { 128, 156, 76, 0 }, 32, 255, 0, 56 },
	{ 3, 10, 4, XW_GENUS_STARFIELD_BACKDROP, 480, 240, 0, NULL, { 144, 156, 76, 0 }, 32, 255, 0, 57 },
	{ 3, 10, 4, XW_GENUS_STARFIELD_BACKDROP, 480, 240, 0, NULL, { 160, 156, 76, 0 }, 32, 255, 0, 58 },
	{ 3, 10, 4, XW_GENUS_STARFIELD_BACKDROP, 480, 240, 0, NULL, { 64, 157, 76, 0 }, 32, 255, 0, 55 },
	{ 3, 10, 4, XW_GENUS_STARFIELD_BACKDROP, 480, 240, 0, NULL, { 80, 157, 76, 0 }, 32, 255, 0, 56 },
	{ 3, 10, 4, XW_GENUS_STARFIELD_BACKDROP, 480, 240, 0, NULL, { 80, 157, 76, 0 }, 32, 255, 0, 57 },
	{ 3, 10, 4, XW_GENUS_STARFIELD_BACKDROP, 480, 240, 0, NULL, { 80, 157, 76, 0 }, 32, 255, 0, 58 },
	{ 3, 10, 4, XW_GENUS_STARFIELD_BACKDROP, 480, 240, 0, NULL, { 80, 157, 76, 0 }, 32, 255, 0, 59 },
	{ 3, 10, 4, XW_GENUS_STARFIELD_BACKDROP, 480, 240, 0, NULL, { 80, 157, 76, 0 }, 32, 255, 0, 60 },
	{ 3,
	  10,
	  5,
	  XW_GENUS_EXPLOSION_EFFECT,
	  3328,
	  240,
	  0,
	  g_modelType133AnimationFrames,
	  { 112, 156, 76, 0 },
	  64,
	  255,
	  0,
	  62 },
	{ 3,
	  10,
	  5,
	  XW_GENUS_EXPLOSION_EFFECT,
	  3328,
	  240,
	  0,
	  g_modelType134AnimationFrames,
	  { 112, 156, 76, 0 },
	  64,
	  255,
	  0,
	  63 },
	{ 3,
	  10,
	  5,
	  XW_GENUS_EXPLOSION_EFFECT,
	  3328,
	  240,
	  0,
	  g_modelType135AnimationFrames,
	  { 112, 156, 76, 0 },
	  64,
	  255,
	  0,
	  61 },
	{ 3,
	  10,
	  5,
	  XW_GENUS_EXPLOSION_EFFECT,
	  3328,
	  240,
	  0,
	  g_modelType136AnimationFrames,
	  { 112, 156, 76, 0 },
	  64,
	  255,
	  0,
	  85 },
	{ 3,
	  10,
	  5,
	  XW_GENUS_EXPLOSION_EFFECT,
	  1664,
	  240,
	  0,
	  g_modelType137AnimationFrames,
	  { 112, 156, 76, 0 },
	  64,
	  255,
	  0,
	  64 },
	{ 3,
	  10,
	  5,
	  XW_GENUS_EXPLOSION_EFFECT,
	  1664,
	  240,
	  0,
	  g_fragmentSecondaryAnimationFrames,
	  { 176, 157, 76, 0 },
	  64,
	  255,
	  0,
	  65 },
	{ 3,
	  10,
	  5,
	  XW_GENUS_EXPLOSION_EFFECT,
	  1280,
	  240,
	  0,
	  &g_fragmentSecondaryAnimationFrames[MODEL_TYPE139_ANIMATION_OFFSET],
	  { 176, 157, 76, 0 },
	  64,
	  255,
	  0,
	  86 },
	{ 3,
	  10,
	  5,
	  XW_GENUS_EXPLOSION_EFFECT,
	  1280,
	  240,
	  0,
	  &g_fragmentSecondaryAnimationFrames[MODEL_TYPE140_ANIMATION_OFFSET],
	  { 192, 157, 76, 0 },
	  64,
	  255,
	  0,
	  66 },
	{ 3, 10, 5, XW_GENUS_EXPLOSION_EFFECT, 1664, 240, 0, NULL, { 112, 156, 76, 0 }, 64, 255, 0, 67 },
	{ 3,
	  10,
	  5,
	  XW_GENUS_EXPLOSION_EFFECT,
	  1664,
	  240,
	  0,
	  g_componentDamageAnimationFrames,
	  { 176, 157, 76, 0 },
	  64,
	  255,
	  0,
	  68 },
	{ 3, 9, 1, XW_GENUS_OTHER_PROJECTILE, 2048, 1024, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 2, 3 },
	{ 3, 9, 1, XW_GENUS_OTHER_PROJECTILE, 2048, 1024, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 2, 6 },
	{ 3, 9, 1, XW_GENUS_OTHER_PROJECTILE, 2048, 1024, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 2, 4 },
	{ 3, 9, 1, XW_GENUS_OTHER_PROJECTILE, 2048, 1024, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 2, 7 },
	{ 3, 9, 1, XW_GENUS_OTHER_PROJECTILE, 2048, 1024, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 2, 5 },
	{ 3, 9, 1, XW_GENUS_OTHER_PROJECTILE, 2048, 1024, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 2, 8 },
	{ 3, 9, 1, XW_GENUS_OTHER_PROJECTILE, 2048, 256, 0, NULL, { 0, 0, 0, 0 }, 67, 255, 2, 10 },
	{ 3, 9, 1, XW_GENUS_OTHER_PROJECTILE, 2048, 256, 0, NULL, { 0, 0, 0, 0 }, 67, 255, 2, 9 },
	{ 3, 9, 1, XW_GENUS_OTHER_PROJECTILE, 2048, 1024, 0, NULL, { 0, 0, 0, 0 }, 0, 255, 2, 6 },
	{ 3, 9, 1, XW_GENUS_OTHER_PROJECTILE, 2048, 1024, 0, NULL, { 0, 0, 0, 0 }, 0, 255, 2, 7 },
	{ 3, 9, 1, XW_GENUS_OTHER_PROJECTILE, 2048, 1024, 0, NULL, { 0, 0, 0, 0 }, 0, 255, 2, 8 },
	{ 3, 9, 1, XW_GENUS_OTHER_PROJECTILE, 2048, 256, 0, NULL, { 0, 0, 0, 0 }, 67, 255, 2, 10 },
	{ 3, 9, 1, XW_GENUS_OTHER_PROJECTILE, 2048, 256, 0, NULL, { 0, 0, 0, 0 }, 67, 255, 2, 9 },
	{ 3, 41, 1, XW_GENUS_OTHER_PROJECTILE, 2048, 256, 0, NULL, { 0, 0, 0, 0 }, 67, 255, 0, 84 },
	{ 3, 9, 1, XW_GENUS_OTHER_PROJECTILE, 2048, 256, 0, NULL, { 0, 0, 0, 0 }, 67, 255, 2, 11 },
	{ 3, 9, 1, XW_GENUS_OTHER_PROJECTILE, 2048, 256, 0, NULL, { 0, 0, 0, 0 }, 67, 255, 2, 12 },
	{ 3, 9, 1, XW_GENUS_OTHER_PROJECTILE, 2048, 256, 0, NULL, { 0, 0, 0, 0 }, 67, 255, 2, 12 },
	{ 3, 9, 1, XW_GENUS_OTHER_PROJECTILE, 2048, 256, 0, NULL, { 0, 0, 0, 0 }, 67, 255, 2, 12 },
	{ 1, 0, 1, XW_GENUS_OTHER_PROJECTILE, 2048, 256, 0, NULL, { 0, 0, 0, 0 }, 0, 255, 3, 0 },
	{ 1, 0, 1, XW_GENUS_OTHER_PROJECTILE, 2048, 256, 0, NULL, { 0, 0, 0, 0 }, 0, 255, 3, 0 },
	{ 1, 0, 1, XW_GENUS_OTHER_PROJECTILE, 2048, 256, 0, NULL, { 0, 0, 0, 0 }, 0, 255, 3, 0 },
	{ 1, 0, 1, XW_GENUS_OTHER_PROJECTILE, 2048, 256, 0, NULL, { 0, 0, 0, 0 }, 0, 255, 3, 0 },
	{ 1, 0, 1, XW_GENUS_OTHER_PROJECTILE, 2048, 256, 0, NULL, { 0, 0, 0, 0 }, 0, 255, 3, 0 },
	{ 1, 0, 1, XW_GENUS_OTHER_PROJECTILE, 2048, 256, 0, NULL, { 0, 0, 0, 0 }, 0, 255, 3, 0 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 1 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 2 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 3 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 4 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 5 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 6 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 7 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 8 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 9 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 10 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 11 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 12 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 13 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 14 },
	{ 0, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 14 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 15 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 16 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 17 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 18 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 19 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 20 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 21 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 22 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 23 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 24 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 25 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 26 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 27 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 28 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 29 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 30 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 31 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 32 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 33 },
	{ 3, 129, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 64, 255, 3, 34 },
	{ 3, 65, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 128, 255, 3, 35 },
	{ 3, 65, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 128, 255, 3, 36 },
	{ 3, 65, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 128, 255, 3, 37 },
	{ 3, 65, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 128, 255, 3, 38 },
	{ 3, 65, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 128, 255, 3, 39 },
	{ 3, 65, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 128, 255, 3, 40 },
	{ 3, 65, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 128, 255, 3, 41 },
	{ 3, 65, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 128, 255, 3, 42 },
	{ 3, 65, 6, XW_GENUS_SCENERY, 2000, 1000, 0, NULL, { 0, 0, 0, 0 }, 128, 255, 3, 43 }
};

// GLOBAL: XW 0x55FBF0
OptVector g_modelBoundsMin[MODEL_TYPE_RECORD_COUNT];

// GLOBAL: XW 0x5605D8
int g_modelBoundsCached[MODEL_TYPE_RECORD_COUNT];

// GLOBAL: XW 0x560928
OptVector g_modelBoundsMax[MODEL_TYPE_RECORD_COUNT];

// GLOBAL: XW 0x62B340
uint16_t g_loadedModels[MODEL_LOADED_HANDLE_COUNT] = { 0 };

// GLOBAL: XW 0x62D380
ModelMeshObjectTypeCache g_objectTypeMeshCache[MODEL_MESH_CACHE_COUNT] = { 0 };

// GLOBAL: XW 0x63BC20
uint8_t g_objectTypeHudShipIds[MODEL_TYPE_RECORD_COUNT] = { 0 };

// FUNCTION: XW 0x490870
int ModelMesh_GetObjectTypeMeshCount(int objectType) {
	OptimizedPolyObject* model;
	int meshCount;
	if (g_loadedModels[objectType] == 0) {
		return 0;
	}
	if ((g_modelTypeTable[objectType].flags & MODEL_TYPE_FLAG_OPT_MESH) == 0) {
		return 0;
	}
	model = Memory_LockHandle(g_loadedModels[objectType]);
	if (model->selfMarker != model) {
		OptModel_AdjustOptimizedPolyObjectPointers(model);
	}
	meshCount = model->rootNodeCount;
	if (model->rootNodes[0]->nodeType == OPT_TEXTURE) {
		--meshCount;
	}
	Memory_UnlockHandle(g_loadedModels[objectType]);
	if (meshCount > MODEL_MESH_MAX_COUNT) {
		meshCount = MODEL_MESH_MAX_COUNT;
	}
	return meshCount;
}

// FUNCTION: XW 0x4908F0
struct OptNode* ModelMesh_FindFirstMeshVertsNode(struct OptNode* node) {
	int childIndex;
	OptNode* result;

	if (node == NULL) {
		return NULL;
	}
	if (node->nodeType == OPT_MESHVERTS) {
		return node;
	}
	for (childIndex = 0; childIndex < node->childCount; ++childIndex) {
		if (node->pChildren[childIndex] != NULL) {
			result = ModelMesh_FindFirstMeshVertsNode(node->pChildren[childIndex]);
			if (result != NULL) {
				return result;
			}
		}
	}
	return NULL;
}

// FUNCTION: XW 0x490940
void ModelMesh_EnsureModelBoundsCached(int objectType) {
	OptVector minBounds;
	OptVector maxBounds;
	OptimizedPolyObject* model;
	int rootIndex;

	minBounds.x = minBounds.y = minBounds.z = MODEL_BOUNDS_INITIAL_LIMIT;
	maxBounds.x = maxBounds.y = maxBounds.z = -MODEL_BOUNDS_INITIAL_LIMIT;
	if (objectType == 0 || (g_modelTypeTable[objectType].flags & MODEL_TYPE_FLAG_OPT_MESH)) {
		model = Memory_LockHandle(g_loadedModels[objectType]);
		if (model->selfMarker != model)
			OptModel_AdjustOptimizedPolyObjectPointers(model);
		for (rootIndex = 0; rootIndex < model->rootNodeCount; ++rootIndex) {
			OptNode* rootNode = model->rootNodes[rootIndex];
			if (rootNode != NULL && rootNode->nodeType != OPT_TEXTURE) {
				OptNode* verticesNode = ModelMesh_FindFirstMeshVertsNode(rootNode);
				if (verticesNode != NULL) {
					const OptVector* vertices = verticesNode->param2;
					int vertexCount = verticesNode->param1;
					if (vertexCount >= MODEL_BOUNDS_VECTOR_COUNT) {
						const OptVector* minimum = &vertices[vertexCount - MODEL_BOUNDS_VECTOR_COUNT];
						const OptVector* maximum;

						if (minimum->x < minBounds.x)
							minBounds.x = minimum->x;
						if (minimum->y < minBounds.y)
							minBounds.y = minimum->y;
						if (minimum->z < minBounds.z)
							minBounds.z = minimum->z;
						maximum = &minimum[1];
						if (maximum->x > maxBounds.x)
							maxBounds.x = maximum->x;
						if (maximum->y > maxBounds.y)
							maxBounds.y = maximum->y;
						if (maximum->z > maxBounds.z)
							maxBounds.z = maximum->z;
					}
				}
			}
		}
		g_modelBoundsMin[objectType].x = minBounds.x;
		g_modelBoundsMin[objectType].y = minBounds.y;
		g_modelBoundsMin[objectType].z = minBounds.z;
		g_modelBoundsMax[objectType].x = maxBounds.x;
		g_modelBoundsMax[objectType].y = maxBounds.y;
		g_modelBoundsMax[objectType].z = maxBounds.z;
		if (objectType != 0)
			g_modelBoundsCached[objectType] = 1;
		Memory_UnlockHandle(g_loadedModels[objectType]);
	}
}

// FUNCTION: XW 0x490B00
int ModelMesh_GetModelMaxExtent(int modelType) {
	OptVector size;

	if (!g_modelBoundsCached[modelType])
		ModelMesh_EnsureModelBoundsCached(modelType);
	size.y = g_modelBoundsMax[modelType].y;
	size.x = g_modelBoundsMax[modelType].x;
	size.y -= g_modelBoundsMin[modelType].y;
	size.x -= g_modelBoundsMin[modelType].x;
	size.z = g_modelBoundsMax[modelType].z - g_modelBoundsMin[modelType].z;
	if (size.y >= size.x && size.y >= size.z)
		return (int)size.y;
	if (size.z >= size.x && size.z >= size.y)
		size.x = size.z;
	return (int)size.x;
}

// FUNCTION: XW 0x490BB0
int64_t ModelMesh_GetModelSizeX(int objectType) {
	if (!g_modelBoundsCached[objectType]) {
		ModelMesh_EnsureModelBoundsCached(objectType);
	}
	return (int64_t)(g_modelBoundsMax[objectType].x - g_modelBoundsMin[objectType].x);
}

// FUNCTION: XW 0x490BF0
int64_t ModelMesh_GetModelSizeY(int objectType) {
	if (!g_modelBoundsCached[objectType]) {
		ModelMesh_EnsureModelBoundsCached(objectType);
	}
	return (int64_t)(g_modelBoundsMax[objectType].y - g_modelBoundsMin[objectType].y);
}

// FUNCTION: XW 0x490C30
int64_t ModelMesh_GetModelSizeZ(int objectType) {
	if (!g_modelBoundsCached[objectType]) {
		ModelMesh_EnsureModelBoundsCached(objectType);
	}
	return (int64_t)(g_modelBoundsMax[objectType].z - g_modelBoundsMin[objectType].z);
}

// FUNCTION: XW 0x490C70
struct MeshDescriptor* ModelMesh_FindDescriptorNodeRecursive(struct OptNode* node,
															 struct OptimizedPolyObject* model) {
	int childIndex;
	MeshDescriptor* result;

	if (node == NULL) {
		return NULL;
	}
	if (node->nodeType == OPT_MESHDESCRIPTOR) {
		return node->param2;
	}
	for (childIndex = 0; childIndex < node->childCount; ++childIndex) {
		if (node->pChildren[childIndex] != NULL) {
			result = ModelMesh_FindDescriptorNodeRecursive(node->pChildren[childIndex], model);
			if (result != NULL) {
				return result;
			}
		}
	}
	return NULL;
}

// FUNCTION: XW 0x490CC0
struct MeshDescriptor* ModelMesh_GetDescriptor(int objectType, int meshIndex) {
	OptimizedPolyObject* model;
	OptNode** rootNodes;
	int rootCount;
	if (g_loadedModels[objectType] == 0) {
		return NULL;
	}
	if (meshIndex < 0) {
		return NULL;
	}
	if ((g_modelTypeTable[objectType].flags & MODEL_TYPE_FLAG_OPT_MESH) == 0) {
		return NULL;
	}
	model = Memory_LockHandle(g_loadedModels[objectType]);
	Memory_UnlockHandle(g_loadedModels[objectType]);
	if (model->selfMarker != model) {
		OptModel_AdjustOptimizedPolyObjectPointers(model);
	}
	rootNodes = model->rootNodes;
	if (rootNodes[0]->nodeType == OPT_TEXTURE) {
		++meshIndex;
	}
	rootCount = model->rootNodeCount;
	if (meshIndex >= rootCount) {
		meshIndex = rootCount - 1;
	}
	return ModelMesh_FindDescriptorNodeRecursive(rootNodes[meshIndex], model);
}

// FUNCTION: XW 0x490D50
int ModelMesh_GetObjectTypeMeshType(int objectType, int meshIndex) {
	OptimizedPolyObject* model;
	OptNode** rootNodes;
	MeshDescriptor* descriptor;
	int rootCount;
	int meshType;
	if (g_loadedModels[objectType] == 0)
		return 0;
	if (meshIndex < 0)
		return 0;
	if ((g_modelTypeTable[objectType].flags & MODEL_TYPE_FLAG_OPT_MESH) == 0)
		return 0;
	meshType = 0;
	model = Memory_LockHandle(g_loadedModels[objectType]);
	if (model->selfMarker != model)
		OptModel_AdjustOptimizedPolyObjectPointers(model);
	rootNodes = model->rootNodes;
	if (rootNodes[0]->nodeType == OPT_TEXTURE)
		++meshIndex;
	rootCount = model->rootNodeCount;
	if (meshIndex >= rootCount)
		meshIndex = rootCount - 1;
	descriptor = ModelMesh_FindDescriptorNodeRecursive(rootNodes[meshIndex], model);
	if (descriptor != NULL)
		meshType = descriptor->meshType;
	Memory_UnlockHandle(g_loadedModels[objectType]);
	return meshType;
}

// FUNCTION: XW 0x490DF0
int ModelMesh_CountHardpoints(int modelType, int meshIndex) {
	OptimizedPolyObject* model;
	OptNode** rootNodes;
	int rootCount;
	int count;
	if ((g_modelTypeTable[modelType].flags & MODEL_TYPE_FLAG_OPT_MESH) == 0) {
		return 0;
	}
	model = Memory_LockHandle(g_loadedModels[modelType]);
	if (model->selfMarker != model) {
		OptModel_AdjustOptimizedPolyObjectPointers(model);
	}
	rootNodes = model->rootNodes;
	if (rootNodes[0]->nodeType == OPT_TEXTURE) {
		++meshIndex;
	}
	rootCount = model->rootNodeCount;
	if (meshIndex >= rootCount) {
		meshIndex = rootCount - 1;
	}
	count = ModelMesh_FindFirstMeshVertsNode(rootNodes[meshIndex])->param1;
	Memory_UnlockHandle(g_loadedModels[modelType]);
	return count;
}

// FUNCTION: XW 0x490E70
int ModelMesh_GetVertexX(int objectType, int meshIndex, int vertexIndex) {
	OptimizedPolyObject* model;
	OptNode** rootNodes;
	OptNode* verticesNode;
	const OptVector* vertices;
	int rootCount;
	int vertexCount;
	int coordinate;
	if ((g_modelTypeTable[objectType].flags & MODEL_TYPE_FLAG_OPT_MESH) == 0)
		return 0;
	model = Memory_LockHandle(g_loadedModels[objectType]);
	if (model->selfMarker != model)
		OptModel_AdjustOptimizedPolyObjectPointers(model);
	rootNodes = model->rootNodes;
	if (rootNodes[0]->nodeType == OPT_TEXTURE)
		++meshIndex;
	rootCount = model->rootNodeCount;
	if (meshIndex >= rootCount)
		meshIndex = rootCount - 1;
	verticesNode = ModelMesh_FindFirstMeshVertsNode(rootNodes[meshIndex]);
	vertices = verticesNode->param2;
	vertexCount = verticesNode->param1;
	if (vertexIndex >= vertexCount)
		vertexIndex = vertexCount - 1;
	coordinate = (int)vertices[vertexIndex].x;
	Memory_UnlockHandle(g_loadedModels[objectType]);
	return coordinate;
}

// FUNCTION: XW 0x490F10
int ModelMesh_GetVertexY(int objectType, int meshIndex, int vertexIndex) {
	OptimizedPolyObject* model;
	OptNode** rootNodes;
	OptNode* verticesNode;
	const OptVector* vertices;
	int rootCount;
	int vertexCount;
	int coordinate;
	if ((g_modelTypeTable[objectType].flags & MODEL_TYPE_FLAG_OPT_MESH) == 0)
		return 0;
	model = Memory_LockHandle(g_loadedModels[objectType]);
	if (model->selfMarker != model)
		OptModel_AdjustOptimizedPolyObjectPointers(model);
	rootNodes = model->rootNodes;
	if (rootNodes[0]->nodeType == OPT_TEXTURE)
		++meshIndex;
	rootCount = model->rootNodeCount;
	if (meshIndex >= rootCount)
		meshIndex = rootCount - 1;
	verticesNode = ModelMesh_FindFirstMeshVertsNode(rootNodes[meshIndex]);
	vertices = verticesNode->param2;
	vertexCount = verticesNode->param1;
	if (vertexIndex >= vertexCount)
		vertexIndex = vertexCount - 1;
	coordinate = (int)vertices[vertexIndex].y;
	Memory_UnlockHandle(g_loadedModels[objectType]);
	return coordinate;
}

// FUNCTION: XW 0x490FB0
int ModelMesh_GetVertexZ(int objectType, int meshIndex, int vertexIndex) {
	OptimizedPolyObject* model;
	OptNode** rootNodes;
	OptNode* verticesNode;
	const OptVector* vertices;
	int rootCount;
	int vertexCount;
	int coordinate;
	if ((g_modelTypeTable[objectType].flags & MODEL_TYPE_FLAG_OPT_MESH) == 0)
		return 0;
	model = Memory_LockHandle(g_loadedModels[objectType]);
	if (model->selfMarker != model)
		OptModel_AdjustOptimizedPolyObjectPointers(model);
	rootNodes = model->rootNodes;
	if (rootNodes[0]->nodeType == OPT_TEXTURE)
		++meshIndex;
	rootCount = model->rootNodeCount;
	if (meshIndex >= rootCount)
		meshIndex = rootCount - 1;
	verticesNode = ModelMesh_FindFirstMeshVertsNode(rootNodes[meshIndex]);
	vertices = verticesNode->param2;
	vertexCount = verticesNode->param1;
	if (vertexIndex >= vertexCount)
		vertexIndex = vertexCount - 1;
	coordinate = (int)vertices[vertexIndex].z;
	Memory_UnlockHandle(g_loadedModels[objectType]);
	return coordinate;
}

// FUNCTION: XW 0x491050
int ModelMesh_GetCenterX(int objectType, int meshIndex) {
	OptimizedPolyObject* model;
	OptNode** rootNodes;
	MeshDescriptor* descriptor;
	int rootCount;
	int coordinate;
	if (meshIndex < 0)
		return 0;
	if ((g_modelTypeTable[objectType].flags & MODEL_TYPE_FLAG_OPT_MESH) == 0)
		return 0;
	model = Memory_LockHandle(g_loadedModels[objectType]);
	if (model->selfMarker != model)
		OptModel_AdjustOptimizedPolyObjectPointers(model);
	rootNodes = model->rootNodes;
	if (rootNodes[0]->nodeType == OPT_TEXTURE)
		++meshIndex;
	rootCount = model->rootNodeCount;
	if (meshIndex >= rootCount)
		meshIndex = rootCount - 1;
	descriptor = ModelMesh_FindDescriptorNodeRecursive(rootNodes[meshIndex], model);
	coordinate = 0;
	if (descriptor != NULL)
		coordinate = (int)descriptor->center.x;
	Memory_UnlockHandle(g_loadedModels[objectType]);
	return coordinate;
}

// FUNCTION: XW 0x4910F0
int ModelMesh_GetCenterY(int objectType, int meshIndex) {
	OptimizedPolyObject* model;
	OptNode** rootNodes;
	MeshDescriptor* descriptor;
	int rootCount;
	int coordinate;
	if (meshIndex < 0)
		return 0;
	if ((g_modelTypeTable[objectType].flags & MODEL_TYPE_FLAG_OPT_MESH) == 0)
		return 0;
	model = Memory_LockHandle(g_loadedModels[objectType]);
	if (model->selfMarker != model)
		OptModel_AdjustOptimizedPolyObjectPointers(model);
	rootNodes = model->rootNodes;
	if (rootNodes[0]->nodeType == OPT_TEXTURE)
		++meshIndex;
	rootCount = model->rootNodeCount;
	if (meshIndex >= rootCount)
		meshIndex = rootCount - 1;
	descriptor = ModelMesh_FindDescriptorNodeRecursive(rootNodes[meshIndex], model);
	coordinate = 0;
	if (descriptor != NULL)
		coordinate = (int)descriptor->center.y;
	Memory_UnlockHandle(g_loadedModels[objectType]);
	return coordinate;
}

// FUNCTION: XW 0x491190
int ModelMesh_GetCenterZ(int objectType, int meshIndex) {
	OptimizedPolyObject* model;
	OptNode** rootNodes;
	MeshDescriptor* descriptor;
	int rootCount;
	int coordinate;
	if (meshIndex < 0)
		return 0;
	if ((g_modelTypeTable[objectType].flags & MODEL_TYPE_FLAG_OPT_MESH) == 0)
		return 0;
	model = Memory_LockHandle(g_loadedModels[objectType]);
	if (model->selfMarker != model)
		OptModel_AdjustOptimizedPolyObjectPointers(model);
	rootNodes = model->rootNodes;
	if (rootNodes[0]->nodeType == OPT_TEXTURE)
		++meshIndex;
	rootCount = model->rootNodeCount;
	if (meshIndex >= rootCount)
		meshIndex = rootCount - 1;
	descriptor = ModelMesh_FindDescriptorNodeRecursive(rootNodes[meshIndex], model);
	coordinate = 0;
	if (descriptor != NULL)
		coordinate = (int)descriptor->center.z;
	Memory_UnlockHandle(g_loadedModels[objectType]);
	return coordinate;
}

// FUNCTION: XW 0x491230
int ModelMesh_GetBoundsMinX(int modelType, int meshIndex) {
	OptimizedPolyObject* model;
	OptNode** rootNodes;
	MeshDescriptor* descriptor;
	int rootCount;
	int coordinate;
	if (meshIndex < 0)
		return 0;
	if ((g_modelTypeTable[modelType].flags & MODEL_TYPE_FLAG_OPT_MESH) == 0)
		return 0;
	model = Memory_LockHandle(g_loadedModels[modelType]);
	if (model->selfMarker != model)
		OptModel_AdjustOptimizedPolyObjectPointers(model);
	rootNodes = model->rootNodes;
	if (rootNodes[0]->nodeType == OPT_TEXTURE)
		++meshIndex;
	rootCount = model->rootNodeCount;
	if (meshIndex >= rootCount)
		meshIndex = rootCount - 1;
	descriptor = ModelMesh_FindDescriptorNodeRecursive(rootNodes[meshIndex], model);
	coordinate = 0;
	if (descriptor != NULL)
		coordinate = (int)descriptor->boxMin.x;
	Memory_UnlockHandle(g_loadedModels[modelType]);
	return coordinate;
}

// FUNCTION: XW 0x4912D0
int ModelMesh_GetBoundsMinY(int modelType, int meshIndex) {
	OptimizedPolyObject* model;
	OptNode** rootNodes;
	MeshDescriptor* descriptor;
	int rootCount;
	int coordinate;
	if (meshIndex < 0)
		return 0;
	if ((g_modelTypeTable[modelType].flags & MODEL_TYPE_FLAG_OPT_MESH) == 0)
		return 0;
	model = Memory_LockHandle(g_loadedModels[modelType]);
	if (model->selfMarker != model)
		OptModel_AdjustOptimizedPolyObjectPointers(model);
	rootNodes = model->rootNodes;
	if (rootNodes[0]->nodeType == OPT_TEXTURE)
		++meshIndex;
	rootCount = model->rootNodeCount;
	if (meshIndex >= rootCount)
		meshIndex = rootCount - 1;
	descriptor = ModelMesh_FindDescriptorNodeRecursive(rootNodes[meshIndex], model);
	coordinate = 0;
	if (descriptor != NULL)
		coordinate = (int)descriptor->boxMin.y;
	Memory_UnlockHandle(g_loadedModels[modelType]);
	return coordinate;
}

// FUNCTION: XW 0x491370
int ModelMesh_GetBoundsMinZ(int modelType, int meshIndex) {
	OptimizedPolyObject* model;
	OptNode** rootNodes;
	MeshDescriptor* descriptor;
	int rootCount;
	int coordinate;
	if (meshIndex < 0)
		return 0;
	if ((g_modelTypeTable[modelType].flags & MODEL_TYPE_FLAG_OPT_MESH) == 0)
		return 0;
	model = Memory_LockHandle(g_loadedModels[modelType]);
	if (model->selfMarker != model)
		OptModel_AdjustOptimizedPolyObjectPointers(model);
	rootNodes = model->rootNodes;
	if (rootNodes[0]->nodeType == OPT_TEXTURE)
		++meshIndex;
	rootCount = model->rootNodeCount;
	if (meshIndex >= rootCount)
		meshIndex = rootCount - 1;
	descriptor = ModelMesh_FindDescriptorNodeRecursive(rootNodes[meshIndex], model);
	coordinate = 0;
	if (descriptor != NULL)
		coordinate = (int)descriptor->boxMin.z;
	Memory_UnlockHandle(g_loadedModels[modelType]);
	return coordinate;
}

// FUNCTION: XW 0x491410
int ModelMesh_GetBoundsMaxX(int modelType, int meshIndex) {
	OptimizedPolyObject* model;
	OptNode** rootNodes;
	MeshDescriptor* descriptor;
	int rootCount;
	int coordinate;
	if (meshIndex < 0)
		return 0;
	if ((g_modelTypeTable[modelType].flags & MODEL_TYPE_FLAG_OPT_MESH) == 0)
		return 0;
	model = Memory_LockHandle(g_loadedModels[modelType]);
	if (model->selfMarker != model)
		OptModel_AdjustOptimizedPolyObjectPointers(model);
	rootNodes = model->rootNodes;
	if (rootNodes[0]->nodeType == OPT_TEXTURE)
		++meshIndex;
	rootCount = model->rootNodeCount;
	if (meshIndex >= rootCount)
		meshIndex = rootCount - 1;
	descriptor = ModelMesh_FindDescriptorNodeRecursive(rootNodes[meshIndex], model);
	coordinate = 0;
	if (descriptor != NULL)
		coordinate = (int)descriptor->boxMax.x;
	Memory_UnlockHandle(g_loadedModels[modelType]);
	return coordinate;
}

// FUNCTION: XW 0x4914B0
int ModelMesh_GetBoundsMaxY(int modelType, int meshIndex) {
	OptimizedPolyObject* model;
	OptNode** rootNodes;
	MeshDescriptor* descriptor;
	int rootCount;
	int coordinate;
	if (meshIndex < 0)
		return 0;
	if ((g_modelTypeTable[modelType].flags & MODEL_TYPE_FLAG_OPT_MESH) == 0)
		return 0;
	model = Memory_LockHandle(g_loadedModels[modelType]);
	if (model->selfMarker != model)
		OptModel_AdjustOptimizedPolyObjectPointers(model);
	rootNodes = model->rootNodes;
	if (rootNodes[0]->nodeType == OPT_TEXTURE)
		++meshIndex;
	rootCount = model->rootNodeCount;
	if (meshIndex >= rootCount)
		meshIndex = rootCount - 1;
	descriptor = ModelMesh_FindDescriptorNodeRecursive(rootNodes[meshIndex], model);
	coordinate = 0;
	if (descriptor != NULL)
		coordinate = (int)descriptor->boxMax.y;
	Memory_UnlockHandle(g_loadedModels[modelType]);
	return coordinate;
}

// FUNCTION: XW 0x491550
int ModelMesh_GetBoundsMaxZ(int modelType, int meshIndex) {
	OptimizedPolyObject* model;
	OptNode** rootNodes;
	MeshDescriptor* descriptor;
	int rootCount;
	int coordinate;
	if (meshIndex < 0)
		return 0;
	if ((g_modelTypeTable[modelType].flags & MODEL_TYPE_FLAG_OPT_MESH) == 0)
		return 0;
	model = Memory_LockHandle(g_loadedModels[modelType]);
	if (model->selfMarker != model)
		OptModel_AdjustOptimizedPolyObjectPointers(model);
	rootNodes = model->rootNodes;
	if (rootNodes[0]->nodeType == OPT_TEXTURE)
		++meshIndex;
	rootCount = model->rootNodeCount;
	if (meshIndex >= rootCount)
		meshIndex = rootCount - 1;
	descriptor = ModelMesh_FindDescriptorNodeRecursive(rootNodes[meshIndex], model);
	coordinate = 0;
	if (descriptor != NULL)
		coordinate = (int)descriptor->boxMax.z;
	Memory_UnlockHandle(g_loadedModels[modelType]);
	return coordinate;
}

// FUNCTION: XW 0x4915F0
int ModelMesh_GetComponentMaxExtent(int modelType, int meshIndex) {
	OptimizedPolyObject* model;
	OptNode** rootNodes;
	int rootCount;
	MeshDescriptor* descriptor;
	int maxSpan;

	if (meshIndex < 0)
		return 0;
	if ((g_modelTypeTable[modelType].flags & MODEL_TYPE_FLAG_OPT_MESH) == 0)
		return 0;
	model = Memory_LockHandle(g_loadedModels[modelType]);
	if (model->selfMarker != model)
		OptModel_AdjustOptimizedPolyObjectPointers(model);
	rootNodes = model->rootNodes;
	if (rootNodes[0]->nodeType == OPT_TEXTURE)
		++meshIndex;
	rootCount = model->rootNodeCount;
	if (meshIndex >= rootCount)
		meshIndex = rootCount - 1;
	descriptor = ModelMesh_FindDescriptorNodeRecursive(rootNodes[meshIndex], model);
	maxSpan = 0;
	if (descriptor != NULL) {
		int spanX = (int)descriptor->span.x;
		int spanY = (int)descriptor->span.y;
		int spanZ = (int)descriptor->span.z;
		maxSpan = spanX;
		if (spanY >= maxSpan && spanY >= spanZ)
			maxSpan = spanY;
		else if (spanZ >= maxSpan && spanZ >= spanY)
			maxSpan = spanZ;
	}
	Memory_UnlockHandle(g_loadedModels[modelType]);
	return maxSpan;
}

// FUNCTION: XW 0x4916C0
int ModelMesh_FindBridgeIndex(struct OptimizedPolyObject* model) {
	int meshIndex = 0;
	int rootIndex;
	for (rootIndex = 0; rootIndex < model->rootNodeCount; ++rootIndex) {
		OptNode* rootNode = model->rootNodes[rootIndex];
		if (rootNode->nodeType != OPT_TEXTURE) {
			MeshDescriptor* descriptor = ModelMesh_FindDescriptorNodeRecursive(rootNode, model);
			if (descriptor != NULL && descriptor->meshType == MODEL_MESH_TYPE_BRIDGE) {
				break;
			}
			++meshIndex;
		}
	}
	if (rootIndex < model->rootNodeCount) {
		return meshIndex;
	}
	return MODEL_MESH_INDEX_NOT_FOUND;
}

// FUNCTION: XW 0x491720
void ModelMesh_BuildObjectTypeMeshCache(void) {
	int objectType;
	for (objectType = 0; objectType < MODEL_MESH_CACHE_COUNT; ++objectType) {
		int meshCount = ModelMesh_GetObjectTypeMeshCount(objectType);
		int meshIndex;
		g_objectTypeMeshCache[objectType].meshCount = meshCount;
		for (meshIndex = 0; meshIndex < meshCount; ++meshIndex) {
			g_objectTypeMeshCache[objectType].meshTypes[meshIndex] =
				ModelMesh_GetObjectTypeMeshType(objectType, meshIndex);
			g_objectTypeMeshCache[objectType].meshDescriptors[meshIndex] =
				ModelMesh_GetDescriptor(objectType, meshIndex);
			ModelMesh_DescriptorNoOp(objectType, meshIndex);
		}
	}
}

// FUNCTION: XW 0x4917A0
int ModelMesh_GetCachedObjectTypeMeshCount(int objectType) {
	if (objectType < MODEL_MESH_CACHE_COUNT) {
		return g_objectTypeMeshCache[objectType].meshCount;
	}
	return ModelMesh_GetObjectTypeMeshCount(objectType);
}

// FUNCTION: XW 0x4917D0
int ModelMesh_GetCachedObjectTypeMeshType(int objectType, int meshIndex) {
	int clampedMeshIndex = meshIndex;
	if (objectType < MODEL_MESH_CACHE_COUNT) {
		ModelMeshObjectTypeCache* cache;
		if (clampedMeshIndex < 0)
			return 0;
		cache = &g_objectTypeMeshCache[objectType];
		if (clampedMeshIndex >= cache->meshCount)
			clampedMeshIndex = cache->meshCount - 1;
		return cache->meshTypes[clampedMeshIndex];
	}
	return ModelMesh_GetObjectTypeMeshType(objectType, meshIndex);
}

// FUNCTION: XW 0x491820
struct MeshDescriptor* ModelMesh_GetCachedDescriptor(int objectType, int meshIndex) {
	int clampedMeshIndex = meshIndex;
	if (objectType < MODEL_MESH_CACHE_COUNT) {
		ModelMeshObjectTypeCache* cache;
		if (clampedMeshIndex < 0) {
			return NULL;
		}
		cache = &g_objectTypeMeshCache[objectType];
		if (clampedMeshIndex >= cache->meshCount) {
			clampedMeshIndex = cache->meshCount - 1;
		}
		return cache->meshDescriptors[clampedMeshIndex];
	}
	return ModelMesh_GetDescriptor(objectType, meshIndex);
}
