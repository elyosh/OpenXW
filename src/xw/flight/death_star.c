#include "xw/flight/death_star.h"

#ifdef XW_MODERN
#include "xw_runtime/timing/flight_integration.h"
#include "xw_runtime/timing/reference_motion.h"
#endif

#ifdef XW_MODERN
#include "xw_runtime/snapshot/render_objects.h"
#endif

#ifdef XW_MODERN
#include "xw_dos94/flight/special_world.h"
#include "xw_runtime/runtime/flight_types.h"
#endif

#include "xw/assets/model_mesh.h"
#include "xw/assets/opt_model.h"
#include "xw/flight/fediskio.h"
#include "xw/flight/feinput.h"
#include "xw/flight/flight_display.h"
#include "xw/render/backdrp2.h"
#include "xw/render/render_scene.h"
#ifdef XW_MODERN
#include "xw_runtime/storage/opt_texture.h"
#endif
#include "xw/audio/fsfx.h"
#include "xw/flight/fview.h"
#include "xw/flight/hud/msg.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/collide.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/laser.h"
#include "xw/flight/object/object.h"
#include "xw/flight/object/starship.h"
#include "xw/flight/player/user.h"
#include "xw/flight/xw.h"
#include "xw/math/math2.h"
#include "xw/math/transfm2.h"
#include "xw/math/trig2.h"
#include "xw/util/memory.h"

#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4C323C
const float g_surfaceSpecialDetailMinProjectedSize = 10.0f;

// GLOBAL: XW 0x4C3240
const float g_surfaceDetailMinProjectedSize = 7.0f;

// GLOBAL: XW 0x4CE920
float g_deathStarDetailScreenSizeScale[DEATH_STAR_DETAIL_PRESET_COUNT] = { 5.0f, 0.5f, 0.75f, 1.0f };

// GLOBAL: XW 0x4CE940
OptVector g_deathStarSurfaceVertices[DEATH_STAR_SURFACE_VERTEX_COUNT] = { { 1049088.0f, 1049088.0f, 0.0f },
																		  { 1049088.0f, -1049088.0f, 0.0f },
																		  { -1049088.0f, -1049088.0f, 0.0f },
																		  { -1049088.0f, 1049088.0f, 0.0f } };

// GLOBAL: XW 0x4CE970
OptNode g_deathStarSurfaceVertexNode = {
	NULL, OPT_MESHVERTS, 0, NULL, DEATH_STAR_SURFACE_VERTEX_COUNT, g_deathStarSurfaceVertices
};

// GLOBAL: XW 0x4CE988
float g_deathStarSurfaceUvPerWorldUnit = 0.0000152587890625f;

// GLOBAL: XW 0x4CE990
OptTexCoord g_deathStarSurfaceTexCoords[DEATH_STAR_SURFACE_VERTEX_COUNT] = { { 32.0078125f, -0.0078125f },
																			 { 32.0078125f, 32.0078125f },
																			 { -0.0078125f, 32.0078125f },
																			 { -0.0078125f, -0.0078125f } };

// GLOBAL: XW 0x4CE9B0
OptNode g_deathStarSurfaceTexcoordNode = {
	NULL, OPT_TEXCOORDS, 0, NULL, DEATH_STAR_SURFACE_VERTEX_COUNT, g_deathStarSurfaceTexCoords
};

// GLOBAL: XW 0x4CE9C8
OptVector g_deathStarSurfaceVertexNormals[DEATH_STAR_SURFACE_FACE_COUNT] = { { 0.0f, 0.0f, 0.75f } };

// GLOBAL: XW 0x4CE9D8
OptNode g_deathStarSurfaceNormalNode = {
	NULL, OPT_VERTNORMALS, 0, NULL, DEATH_STAR_SURFACE_FACE_COUNT, g_deathStarSurfaceVertexNormals
};

// GLOBAL: XW 0x4CE9F0
XwSurfaceFaceData g_deathStarSurfaceFaceData = {
	4,
	{ { { 0, 1, 2, 3 }, { 0, 1, 2, 3 }, { 0, 1, 2, 3 }, { 0, 0, 0, 0 } } },
	{ 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f }
};
// GLOBAL: XW 0x4CEA58
OptNode g_deathStarSurfaceFaceNode = {
	NULL, OPT_FACEDATA, 0, NULL, DEATH_STAR_SURFACE_FACE_COUNT, &g_deathStarSurfaceFaceData
};
// GLOBAL: XW 0x4CEA70
OptNode g_deathStarSurfaceTextureNode = { NULL, OPT_TEXTURE, 0, NULL, 1, NULL };
// GLOBAL: XW 0x4CEA88
OptNode* g_deathStarSurfaceChildren[5] = { &g_deathStarSurfaceVertexNode, &g_deathStarSurfaceTexcoordNode,
										   &g_deathStarSurfaceNormalNode, &g_deathStarSurfaceTextureNode,
										   &g_deathStarSurfaceFaceNode };
// GLOBAL: XW 0x4CEAA0
OptNode g_deathStarSurfaceRootNode = {
	NULL, 0, 5, g_deathStarSurfaceChildren, 5, g_deathStarSurfaceChildren
};
// GLOBAL: XW 0x4CEAB8
OptNode* g_deathStarSurfaceRoots[1] = { &g_deathStarSurfaceRootNode };
// GLOBAL: XW 0x4CEAC0
OptimizedPolyObject g_deathStarSurfaceModelHeader = { &g_deathStarSurfaceModelHeader, 0, 1,
													  g_deathStarSurfaceRoots };

// GLOBAL: XW 0x4CEB20
OptVector g_deathStarTrenchVertices[DEATH_STAR_TRENCH_VERTEX_COUNT] = {
	{ 3072.0f, 1049088.0f, 0.0f },       { 3072.0f, -1049088.0f, 0.0f },
	{ -3072.0f, -1049088.0f, 0.0f },     { -3072.0f, 1049088.0f, 0.0f },
	{ 3072.0f, 1049088.0f, -6144.0f },   { 3072.0f, -1049088.0f, -6144.0f },
	{ -3072.0f, -1049088.0f, -6144.0f }, { -3072.0f, 1049088.0f, -6144.0f },
	{ 3072.0f, 1049088.0f, -6144.0f },   { 3072.0f, -1049088.0f, -6144.0f },
	{ -3072.0f, -1049088.0f, -6144.0f }, { -3072.0f, 1049088.0f, -6144.0f }
};

// GLOBAL: XW 0x4CEBB0
OptNode g_deathStarTrenchVertexNode = {
	NULL, OPT_MESHVERTS, 0, NULL, DEATH_STAR_TRENCH_VERTEX_COUNT, g_deathStarTrenchVertices
};

// GLOBAL: XW 0x4CEBC8
OptTexCoord g_deathStarTrenchTexCoords[DEATH_STAR_TRENCH_TEXCOORD_COUNT] = {
	{ 1.0f, -0.0078125f },
	{ 1.0f, 32.0078125f },
	{ 0.0f, 32.0078125f },
	{ 0.0f, -0.0078125f },
	{ 0.666670024394989f, -0.0078125f },
	{ 0.666670024394989f, 32.0078125f },
	{ 0.3333300054073334f, 32.0078125f },
	{ 0.3333300054073334f, -0.0078125f }
};

// GLOBAL: XW 0x4CEC08
OptNode g_deathStarTrenchTexcoordNode = {
	NULL, OPT_TEXCOORDS, 0, NULL, DEATH_STAR_TRENCH_TEXCOORD_COUNT, g_deathStarTrenchTexCoords
};

// GLOBAL: XW 0x4CEC20
OptVector g_deathStarTrenchVertexNormals[DEATH_STAR_TRENCH_FACE_COUNT] = { { 0.0f, 0.0f, 1.0f },
																		   { 1.0f, 0.0f, 0.0f },
																		   { -1.0f, 0.0f, 0.0f } };

// GLOBAL: XW 0x4CEC48
OptNode g_deathStarTrenchNormalNode = {
	NULL, OPT_VERTNORMALS, 0, NULL, DEATH_STAR_TRENCH_FACE_COUNT, g_deathStarTrenchVertexNormals
};

// GLOBAL: XW 0x4CEC60
XwTrenchFaceData g_deathStarTrenchFaceData = {
	12,
	{ { { 0, 1, 5, 4 }, { 0, 1, 2, 3 }, { 0, 1, 5, 4 }, { 2, 2, 2, 2 } },
	  { { 8, 9, 10, 11 }, { 4, 5, 6, 7 }, { 4, 5, 6, 7 }, { 0, 0, 0, 0 } },
	  { { 7, 6, 2, 3 }, { 8, 9, 10, 11 }, { 7, 6, 2, 3 }, { 1, 1, 1, 1 } } },
	{ 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
	  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f }
};

// GLOBAL: XW 0x4CED90
OptNode g_deathStarTrenchFaceNode = {
	NULL, OPT_FACEDATA, 0, NULL, DEATH_STAR_TRENCH_FACE_COUNT, &g_deathStarTrenchFaceData
};
// GLOBAL: XW 0x4CEDA8
OptNode g_deathStarTrenchTextureNode = { NULL, OPT_TEXTURE, 0, NULL, 1, NULL };
// GLOBAL: XW 0x4CEDC0
OptNode* g_deathStarTrenchChildren[5] = { &g_deathStarTrenchVertexNode, &g_deathStarTrenchTexcoordNode,
										  &g_deathStarTrenchNormalNode, &g_deathStarTrenchTextureNode,
										  &g_deathStarTrenchFaceNode };
// GLOBAL: XW 0x4CEDD8
OptNode g_deathStarTrenchRootNode = { NULL, 0, 5, g_deathStarTrenchChildren, 5, g_deathStarTrenchChildren };
// GLOBAL: XW 0x4CEDF0
OptNode* g_deathStarTrenchRoots[1] = { &g_deathStarTrenchRootNode };
// GLOBAL: XW 0x4CEDF8
OptimizedPolyObject g_deathStarTrenchModelHeader = { &g_deathStarTrenchModelHeader, 0, 1,
													 g_deathStarTrenchRoots };

// GLOBAL: XW 0x4CF208
const XwSurfaceDestructionPattern2 g_surfaceDestructionPattern0 = {
	2, { { 0, 0, 2, 4, 0 }, { 0, 0, 12, 4, 0 } }
};

// GLOBAL: XW 0x4CF220
const XwSurfaceDestructionPattern2 g_surfaceDestructionPattern1 = {
	2, { { 0, 0, 1, 4, 0 }, { 0, 0, 6, 4, 0 } }
};

// GLOBAL: XW 0x4CF238
const XwSurfaceDestructionPattern4 g_surfaceDestructionPattern2 = {
	4, { { 9, 4, 1, 4, 0 }, { 9, 252, 1, 4, 0 }, { 251, 4, 2, 4, 0 }, { 251, 252, 2, 4, 0 } }
};

// GLOBAL: XW 0x4CF260
const XwSurfaceDestructionPattern5 g_surfaceDestructionPattern3 = { 5,
																	{ { 0, 0, 2, 4, 0 },
																	  { 6, 11, -32763, 4, 0 },
																	  { 250, 11, -32763, 4, 0 },
																	  { 6, 245, -32763, 4, 0 },
																	  { 250, 245, -32763, 4, 0 } } };

// GLOBAL: XW 0x4CF290
const XwSurfaceDestructionPattern1 g_surfaceDestructionPattern4 = { 1, { { 0, 0, 1, 8, 0 } } };

// GLOBAL: XW 0x4CF2A0
const XwSurfaceDestructionPattern4 g_surfaceDestructionPattern5 = {
	4, { { 250, 4, 2, 4, 0 }, { 250, 252, 2, 4, 0 }, { 8, 7, -32763, 4, 0 }, { 8, 249, -32763, 4, 0 } }
};

// GLOBAL: XW 0x4CF2C8
const XwSurfaceDestructionPattern4 g_surfaceDestructionPattern6 = {
	4,
	{ { 247, 8, -32766, 4, 0 }, { 247, 248, -32766, 4, 0 }, { 9, 8, -32766, 4, 0 }, { 9, 248, -32766, 4, 0 } }
};

// GLOBAL: XW 0x4CF2F0
const XwSurfaceDestructionPattern3 g_surfaceDestructionPattern7 = {
	3, { { 240, 4, 16385, 4, 0 }, { 16, 4, 16385, 4, 0 }, { 0, 4, 16385, 4, 0 } }
};

// GLOBAL: XW 0x4CF310
const XwSurfaceDestructionPattern4 g_surfaceDestructionPattern8 = {
	4, { { 250, 14, 3, 4, 0 }, { 6, 14, 3, 4, 0 }, { 250, 242, 3, 4, 0 }, { 6, 242, 3, 4, 0 } }
};

// GLOBAL: XW 0x4CF338
const XwSurfaceDestructionPattern1 g_surfaceDestructionPattern9 = { 1, { { 0, 0, 0, 4, 0 } } };

// GLOBAL: XW 0x4CF348
const XwSurfaceDestructionPattern* g_surfaceDestructionPatterns[DEATH_STAR_DESTRUCTION_PATTERN_COUNT] = {
	(const XwSurfaceDestructionPattern*)&g_surfaceDestructionPattern0,
	(const XwSurfaceDestructionPattern*)&g_surfaceDestructionPattern1,
	(const XwSurfaceDestructionPattern*)&g_surfaceDestructionPattern2,
	(const XwSurfaceDestructionPattern*)&g_surfaceDestructionPattern3,
	(const XwSurfaceDestructionPattern*)&g_surfaceDestructionPattern4,
	(const XwSurfaceDestructionPattern*)&g_surfaceDestructionPattern5,
	(const XwSurfaceDestructionPattern*)&g_surfaceDestructionPattern6,
	(const XwSurfaceDestructionPattern*)&g_surfaceDestructionPattern7,
	(const XwSurfaceDestructionPattern*)&g_surfaceDestructionPattern8,
	(const XwSurfaceDestructionPattern*)&g_surfaceDestructionPattern9
};

// GLOBAL: XW 0x4CF370
const XwSurfacePlacementList14 g_surfacePlacements2 = { 14,
														{ { 92, 169 },
														  { 142, 167 },
														  { 234, 170 },
														  { 200, 169 },
														  { 38, 170 },
														  { 117, 167 },
														  { 114, 167 },
														  { 163, 167 },
														  { 205, 173 },
														  { 51, 173 },
														  { 61, 176 },
														  { 169, 178 },
														  { 212, 179 },
														  { 89, 177 } } };

// GLOBAL: XW 0x4CF390
const XwSurfaceHealthList14 g_surfaceHealth2 = {
	14, { 20, 20, 5, 20, 40, 40, 10, 20, 100, 100, 125, 125, 150, 150 }
};

// GLOBAL: XW 0x4CF3A0
const uint8_t g_surfacePatternIndices2[DEATH_STAR_SURFACE_OBJECT_COUNT] = { 1, 1, 1, 1, 0, 0, 0,
																			1, 2, 2, 5, 7, 6, 6 };

// GLOBAL: XW 0x4CF3B0
const XwSurfacePlacementList14 g_surfacePlacements0 = { 14,
														{ { 43, 170 },
														  { 190, 167 },
														  { 141, 172 },
														  { 168, 170 },
														  { 232, 167 },
														  { 53, 169 },
														  { 212, 169 },
														  { 83, 170 },
														  { 198, 173 },
														  { 132, 174 },
														  { 51, 175 },
														  { 219, 177 },
														  { 104, 179 },
														  { 61, 180 } } };

// GLOBAL: XW 0x4CF3D0
const XwSurfaceHealthList14 g_surfaceHealth0 = {
	14, { 40, 20, 40, 40, 5, 20, 20, 10, 100, 125, 50, 150, 150, 125 }
};

// GLOBAL: XW 0x4CF3E0
const uint8_t g_surfacePatternIndices0[DEATH_STAR_SURFACE_OBJECT_COUNT] = { 0, 1, 0, 0, 1, 1, 1,
																			0, 2, 3, 4, 6, 6, 8 };

// GLOBAL: XW 0x4CF3F0
const XwSurfacePlacementList14 g_surfacePlacements3 = { 14,
														{ { 238, 167 },
														  { 157, 169 },
														  { 44, 170 },
														  { 136, 167 },
														  { 216, 168 },
														  { 53, 169 },
														  { 211, 169 },
														  { 130, 170 },
														  { 51, 174 },
														  { 61, 175 },
														  { 93, 175 },
														  { 205, 176 },
														  { 180, 177 },
														  { 74, 180 } } };

// GLOBAL: XW 0x4CF410
const XwSurfaceHealthList14 g_surfaceHealth3 = {
	14, { 20, 20, 40, 40, 5, 20, 20, 10, 125, 50, 50, 125, 150, 125 }
};

// GLOBAL: XW 0x4CF420
const uint8_t g_surfacePatternIndices3[DEATH_STAR_SURFACE_OBJECT_COUNT] = { 1, 1, 0, 0, 1, 1, 1,
																			0, 3, 4, 4, 5, 6, 8 };

// GLOBAL: XW 0x4CF430
const XwSurfacePlacementList14 g_surfacePlacements1 = { 14,
														{ { 63, 167 },
														  { 254, 170 },
														  { 141, 169 },
														  { 42, 169 },
														  { 247, 167 },
														  { 118, 170 },
														  { 34, 167 },
														  { 97, 170 },
														  { 194, 173 },
														  { 70, 175 },
														  { 68, 175 },
														  { 189, 176 },
														  { 91, 178 },
														  { 199, 179 } } };

// GLOBAL: XW 0x4CF450
const XwSurfaceHealthList14 g_surfaceHealth1 = {
	14, { 5, 10, 20, 20, 10, 40, 20, 40, 100, 50, 50, 125, 125, 150 }
};

// GLOBAL: XW 0x4CF460
const uint8_t g_surfacePatternIndices1[DEATH_STAR_SURFACE_OBJECT_COUNT] = { 1, 0, 1, 1, 0, 0, 1,
																			0, 2, 4, 4, 5, 7, 6 };

// GLOBAL: XW 0x4CF470
const XwSurfacePlacementList8 g_surfacePlacements4 = { 8,
													   { { 24, 169 },
														 { 56, 167 },
														 { 88, 169 },
														 { 120, 167 },
														 { 152, 169 },
														 { 184, 167 },
														 { 216, 169 },
														 { 248, 167 } } };

// GLOBAL: XW 0x4CF488
const XwSurfaceHealthList8 g_surfaceHealth4 = { 8, { 60, 60, 60, 60, 60, 60, 60, 60 } };

// GLOBAL: XW 0x4CF498
const uint8_t g_surfacePatternIndices4[DEATH_STAR_SHORT_LAYOUT_OBJECT_COUNT] = { 1, 0, 1, 0, 1, 0, 1, 0 };

// GLOBAL: XW 0x4CF4A0
const XwSurfacePlacementList* g_surfacePlacementLists[DEATH_STAR_SURFACE_LAYOUT_COUNT] = {
	(const XwSurfacePlacementList*)&g_surfacePlacements0,
	(const XwSurfacePlacementList*)&g_surfacePlacements1,
	(const XwSurfacePlacementList*)&g_surfacePlacements2,
	(const XwSurfacePlacementList*)&g_surfacePlacements3, (const XwSurfacePlacementList*)&g_surfacePlacements4
};

// GLOBAL: XW 0x4CF4B8
const XwSurfaceHealthList* g_surfaceHealthLists[DEATH_STAR_SURFACE_LAYOUT_COUNT] = {
	(const XwSurfaceHealthList*)&g_surfaceHealth0, (const XwSurfaceHealthList*)&g_surfaceHealth1,
	(const XwSurfaceHealthList*)&g_surfaceHealth2, (const XwSurfaceHealthList*)&g_surfaceHealth3,
	(const XwSurfaceHealthList*)&g_surfaceHealth4
};

// GLOBAL: XW 0x4CF4D0
const uint8_t* g_surfaceDestructionPatternIndices[DEATH_STAR_SURFACE_LAYOUT_COUNT] = {
	g_surfacePatternIndices0, g_surfacePatternIndices1, g_surfacePatternIndices2, g_surfacePatternIndices3,
	g_surfacePatternIndices4
};

// GLOBAL: XW 0x4CF4E8
const XwSurfacePlacementList17 g_trenchPlacements0 = { 17,
													   { { 197, 194 },
														 { 204, 193 },
														 { 229, 193 },
														 { 58, 195 },
														 { 170, 198 },
														 { 10, 197 },
														 { 48, 186 },
														 { 17, 185 },
														 { 224, 187 },
														 { 25, 185 },
														 { 34, 200 },
														 { 244, 185 },
														 { 62, 196 },
														 { 57, 184 },
														 { 30, 192 },
														 { 145, 191 },
														 { 153, 191 } } };

// GLOBAL: XW 0x4CF510
const XwSurfaceHealthList17 g_trenchHealth0 = {
	17, { 10, 10, 10, 10, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 }
};

// GLOBAL: XW 0x4CF528
const uint8_t g_trenchPatternIndices0[DEATH_STAR_SURFACE_OBJECT_COUNT] = { 9, 9, 9, 9, 9, 9, 9,
																		   9, 9, 9, 9, 9, 9, 9 };

// GLOBAL: XW 0x4CF538
const XwSurfacePlacementList17 g_trenchPlacements1 = { 17,
													   { { 196, 194 },
														 { 205, 194 },
														 { 34, 195 },
														 { 185, 193 },
														 { 17, 183 },
														 { 252, 183 },
														 { 246, 198 },
														 { 38, 196 },
														 { 169, 188 },
														 { 41, 186 },
														 { 213, 187 },
														 { 232, 185 },
														 { 10, 200 },
														 { 5, 185 },
														 { 24, 186 },
														 { 45, 188 },
														 { 216, 185 } } };

// GLOBAL: XW 0x4CF560
const XwSurfaceHealthList17 g_trenchHealth1 = {
	17, { 10, 10, 10, 10, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 }
};

// GLOBAL: XW 0x4CF578
const XwSurfacePlacementList19 g_trenchPlacements2 = { 19,
													   { { 136, 194 },
														 { 16, 193 },
														 { 120, 194 },
														 { 249, 194 },
														 { 122, 198 },
														 { 14, 197 },
														 { 38, 199 },
														 { 42, 199 },
														 { 244, 183 },
														 { 188, 186 },
														 { 61, 187 },
														 { 220, 185 },
														 { 37, 186 },
														 { 221, 185 },
														 { 30, 200 },
														 { 40, 186 },
														 { 0, 184 },
														 { 1, 184 },
														 { 153, 192 } } };

// GLOBAL: XW 0x4CF5A0
const XwSurfaceHealthList19 g_trenchHealth2 = {
	19, { 10, 10, 10, 10, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 }
};

// GLOBAL: XW 0x4CF5B8
const XwSurfacePlacementList19 g_trenchPlacements3 = { 19,
													   { { 18, 195 },
														 { 177, 194 },
														 { 157, 193 },
														 { 156, 194 },
														 { 190, 198 },
														 { 38, 197 },
														 { 54, 196 },
														 { 60, 186 },
														 { 41, 183 },
														 { 233, 185 },
														 { 32, 185 },
														 { 245, 185 },
														 { 196, 185 },
														 { 13, 186 },
														 { 6, 200 },
														 { 33, 185 },
														 { 168, 192 },
														 { 216, 191 },
														 { 24, 191 } } };

// GLOBAL: XW 0x4CF5E0
const XwSurfaceHealthList19 g_trenchHealth3 = {
	19, { 10, 10, 10, 10, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 }
};

// GLOBAL: XW 0x4CF5F8
const XwSurfacePlacementList18 g_trenchPlacements4 = { 18,
													   { { 152, 194 },
														 { 153, 194 },
														 { 168, 193 },
														 { 169, 193 },
														 { 34, 197 },
														 { 150, 198 },
														 { 201, 185 },
														 { 26, 196 },
														 { 234, 198 },
														 { 140, 187 },
														 { 189, 186 },
														 { 252, 185 },
														 { 60, 184 },
														 { 121, 191 },
														 { 8, 188 },
														 { 52, 188 },
														 { 6, 200 },
														 { 42, 200 } } };

// GLOBAL: XW 0x4CF620
const XwSurfaceHealthList18 g_trenchHealth4 = {
	18, { 10, 10, 10, 10, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 }
};

// GLOBAL: XW 0x4CF638
const XwSurfacePlacementList19 g_trenchPlacements5 = { 19,
													   { { 18, 195 },
														 { 224, 194 },
														 { 225, 194 },
														 { 58, 195 },
														 { 40, 183 },
														 { 138, 198 },
														 { 62, 199 },
														 { 14, 196 },
														 { 204, 185 },
														 { 88, 186 },
														 { 228, 185 },
														 { 21, 185 },
														 { 49, 187 },
														 { 245, 184 },
														 { 38, 200 },
														 { 25, 186 },
														 { 133, 192 },
														 { 180, 191 },
														 { 60, 186 } } };

// GLOBAL: XW 0x4CF660
const XwSurfaceHealthList19 g_trenchHealth5 = {
	19, { 10, 10, 10, 10, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 }
};

// GLOBAL: XW 0x4CF678
const XwSurfacePlacementList19 g_trenchPlacements6 = { 19,
													   { { 136, 193 },
														 { 137, 193 },
														 { 184, 194 },
														 { 185, 194 },
														 { 54, 197 },
														 { 6, 197 },
														 { 242, 198 },
														 { 22, 185 },
														 { 162, 198 },
														 { 152, 183 },
														 { 32, 186 },
														 { 40, 185 },
														 { 165, 187 },
														 { 212, 185 },
														 { 44, 184 },
														 { 50, 191 },
														 { 58, 196 },
														 { 34, 191 },
														 { 21, 184 } } };

// GLOBAL: XW 0x4CF6A0
const XwSurfaceHealthList19 g_trenchHealth6 = {
	19, { 10, 10, 10, 10, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 }
};

// GLOBAL: XW 0x4CF6B8
const XwSurfacePlacementList17 g_trenchPlacements7 = { 17,
													   { { 204, 194 },
														 { 76, 193 },
														 { 42, 195 },
														 { 185, 193 },
														 { 42, 199 },
														 { 26, 185 },
														 { 22, 197 },
														 { 229, 183 },
														 { 208, 185 },
														 { 10, 196 },
														 { 60, 186 },
														 { 201, 185 },
														 { 54, 200 },
														 { 61, 185 },
														 { 56, 184 },
														 { 13, 186 },
														 { 28, 186 } } };

// GLOBAL: XW 0x4CF6E0
const XwSurfaceHealthList17 g_trenchHealth7 = {
	17, { 10, 10, 10, 10, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 }
};

// GLOBAL: XW 0x4CF6F8
const XwSurfacePlacementList13 g_trenchPlacements8 = { 13,
													   { { 62, 201 },
														 { 132, 194 },
														 { 133, 194 },
														 { 208, 194 },
														 { 209, 194 },
														 { 92, 194 },
														 { 93, 194 },
														 { 168, 194 },
														 { 169, 194 },
														 { 14, 195 },
														 { 26, 195 },
														 { 38, 195 },
														 { 50, 195 } } };

// GLOBAL: XW 0x4CF718
const XwSurfaceHealthList13 g_trenchHealth8 = { 13, { 240, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10 } };

// GLOBAL: XW 0x4CF728
const XwSurfacePlacementList* g_trenchPlacementLists[DEATH_STAR_TRENCH_LAYOUT_COUNT] = {
	(const XwSurfacePlacementList*)&g_trenchPlacements0, (const XwSurfacePlacementList*)&g_trenchPlacements1,
	(const XwSurfacePlacementList*)&g_trenchPlacements2, (const XwSurfacePlacementList*)&g_trenchPlacements3,
	(const XwSurfacePlacementList*)&g_trenchPlacements4, (const XwSurfacePlacementList*)&g_trenchPlacements5,
	(const XwSurfacePlacementList*)&g_trenchPlacements6, (const XwSurfacePlacementList*)&g_trenchPlacements7,
	(const XwSurfacePlacementList*)&g_trenchPlacements8
};

// GLOBAL: XW 0x4CF750
const XwSurfaceHealthList* g_trenchHealthLists[DEATH_STAR_TRENCH_LAYOUT_COUNT] = {
	(const XwSurfaceHealthList*)&g_trenchHealth0, (const XwSurfaceHealthList*)&g_trenchHealth1,
	(const XwSurfaceHealthList*)&g_trenchHealth2, (const XwSurfaceHealthList*)&g_trenchHealth3,
	(const XwSurfaceHealthList*)&g_trenchHealth4, (const XwSurfaceHealthList*)&g_trenchHealth5,
	(const XwSurfaceHealthList*)&g_trenchHealth6, (const XwSurfaceHealthList*)&g_trenchHealth7,
	(const XwSurfaceHealthList*)&g_trenchHealth8
};

// GLOBAL: XW 0x4CF778
const uint8_t* g_trenchDestructionPatternIndices[DEATH_STAR_TRENCH_LAYOUT_COUNT] = {
	g_trenchPatternIndices0, g_trenchPatternIndices0, g_trenchPatternIndices0,
	g_trenchPatternIndices0, g_trenchPatternIndices0, g_trenchPatternIndices0,
	g_trenchPatternIndices0, g_trenchPatternIndices0, g_trenchPatternIndices0
};

// GLOBAL: XW 0x4F4A24
int g_deathStarSurfaceTextureInitialized = 0;

// GLOBAL: XW 0x4F4A28
int g_deathStarTrenchTextureInitialized = 0;

// GLOBAL: XW 0x62B520
XwSurfaceGunCell g_surfaceGunCells[DEATH_STAR_SURFACE_GUN_CELL_CAPACITY] = { 0 };

// GLOBAL: XW 0x62B6B3
uint8_t g_surfaceSpecialTargetHit = 0;

// GLOBAL: XW 0x62B6D8
uint16_t g_surfaceGoalCellHashSlots[DEATH_STAR_SURFACE_GOAL_COUNT] = { 0 };

// GLOBAL: XW 0x62B94C
uint8_t g_surfaceSpecialTargetCollisionMode = 0;

// GLOBAL: XW 0x62BB24
uint8_t g_surfaceVictoryExitSecond = 0;

// GLOBAL: XW 0x62BC98
uint16_t g_surfaceGoalCellKeys[DEATH_STAR_SURFACE_GOAL_COUNT] = { 0 };

// GLOBAL: XW 0x62BDA0
int16_t g_surfaceGunCellRefreshTicks = 0;

// GLOBAL: XW 0x62D0F0
int16_t g_deathStarSurfaceCellX = 0;

// GLOBAL: XW 0x62D0F8
int16_t g_deathStarSurfaceCellY = 0;

// GLOBAL: XW 0x62D361
uint8_t g_trenchSpecialCellVoicePlayed = 0;

// GLOBAL: XW 0x63734B
uint8_t g_deathStarSurfaceModeActive = 0;

// GLOBAL: XW 0x637360
uint16_t g_surfaceGunCellCount = 0;

// GLOBAL: XW 0x6373A0
XwSurfaceCellDamageState g_surfaceCellDamageStates[DEATH_STAR_SURFACE_CELL_COUNT] = { 0 };

// GLOBAL: XW 0x6377EC
XwDeathStarCellViewSteps g_deathStarCellViewSteps = { 0 };

// GLOBAL: XW 0x63BD00
struct ObjectRecord g_deathStarRenderObject = { 0 };

// GLOBAL: XW 0x63BD80
struct CraftData g_deathStarRenderCraft = { 0 };

// FUNCTION: XW 0x426850
void DeathStar_InitSurfaceTexture(void) {
	SceneMesh conversionState;
	OptTextureData* texture;
	int pixelDataBytes;
	uint8_t* sourceShadeTable;
	uint8_t* destShadeTable;
	size_t allocationSize;
	if (g_deathStarSurfaceTextureInitialized != 0) {
		return;
	}
	OptModel_SetSourceVertices(g_deathStarSurfaceVertices);
	OptModel_SetSourceTexCoords(g_deathStarSurfaceTexCoords);
	conversionState.vertexNormals = g_deathStarSurfaceVertexNormals;
	OptModel_BuildFaceNormalTangentData(g_deathStarSurfaceFaceData.vectors,
										(OptPackedFaceData*)&g_deathStarSurfaceFaceData,
										DEATH_STAR_SURFACE_FACE_COUNT, &conversionState);
	g_loadingModel = 1;
	g_deathStarSurfaceTextureInitialized = 1;
	allocationSize = OptModel_GetExternalTextureSerializedSize("DeathStarTexture.rgb");
#ifdef XW_MODERN
	allocationSize += sizeof(OptTextureData) - sizeof(OptTextureFileHeader);
#endif
	texture = (OptTextureData*)malloc(allocationSize);
	g_deathStarSurfaceTextureNode.param2 = texture;
#ifdef XW_MODERN
	XwPort_LoadNativeOptTexture(texture, "DeathStarTexture.rgb");
#else
	OptModel_LoadRgbOrTexFile((uint8_t*)texture, "DeathStarTexture.rgb");
#endif
	pixelDataBytes = texture->height * texture->width;
	sourceShadeTable = (uint8_t*)(texture + 1);
	if (pixelDataBytes == texture->textureSize) {
		pixelDataBytes = texture->dataSize;
	}
	sourceShadeTable += pixelDataBytes;
	destShadeTable = sourceShadeTable;
	if (g_flightBytesPerPixel == FLIGHT_DISPLAY_DIRECT_BYTES_PER_PIXEL) {
		sourceShadeTable += OPT_TEXTURE_PALETTE_COLOR_COUNT * texture->paletteType;
	}
	if (g_flightBytesPerPixel == FLIGHT_DISPLAY_INDEXED_BYTES_PER_PIXEL) {
		const uint16_t* rgb565Colors = (const uint16_t*)(sourceShadeTable + OPT_TEXTURE_SHADE_PALETTE_COUNT);
		int shadeIndex;
		for (shadeIndex = 0; shadeIndex < OPT_TEXTURE_SHADE_PALETTE_COUNT; ++shadeIndex) {
			destShadeTable[shadeIndex] = g_rgb565ToPaletteIndexLut[rgb565Colors[shadeIndex]];
		}
	} else {
		texture->paletteType = 0;
		texture->palette = (uint16_t*)destShadeTable;
#ifdef XW_MODERN
		/* The 8192-byte RGB565 table moves down by 4096 bytes, overlapping its source. */
		memmove(destShadeTable, sourceShadeTable, g_flightBytesPerPixel * OPT_TEXTURE_SHADE_PALETTE_COUNT);
#else
		memcpy(destShadeTable, sourceShadeTable, g_flightBytesPerPixel * OPT_TEXTURE_SHADE_PALETTE_COUNT);
#endif
		texture->palette = (uint16_t*)((uint8_t*)texture->palette - OPT_TEXTURE_SHADE_PALETTE_COUNT);
	}
	if (g_flightBytesPerPixel == FLIGHT_DISPLAY_DIRECT_BYTES_PER_PIXEL) {
		OptModel_PrepareTexturePalette((uint16_t*)destShadeTable, OPT_TEXTURE_SHADE_PALETTE_COUNT);
	}
	g_loadingModel = 0;
}

// FUNCTION: XW 0x426990
void DeathStar_DrawSurfaceDetailModel(uint8_t objectType, int worldX, int worldY, int worldZ) {
	int savedForcedLodLevel;

	g_deathStarRenderObject.objectType = objectType;
	g_deathStarRenderObject.worldX = worldX;
	g_deathStarRenderObject.worldY = worldY;
	g_deathStarRenderObject.worldZ = worldZ;
	g_billboardObjectOrTypeIndex = DEATH_STAR_DETAIL_RENDER_REF;
	g_deathStarRenderObject.genusId = XW_GENUS_SCENERY;
	g_deathStarRenderObject.roll = 0;
	g_deathStarRenderObject.yaw = 0;
	g_deathStarRenderObject.pitch = TRIG2_QUARTER_TURN;
	g_camRelWorldX = worldX - g_flightCamera.worldPosition.x;
	g_camRelWorldY = worldY - g_flightCamera.worldPosition.y;
	g_camRelWorldZ = worldZ - g_flightCamera.worldPosition.z;
	g_objectViewZ = transfm2_geteyez(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
	g_objectViewX = transfm2_geteyex(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
	g_objectViewY = transfm2_geteyey(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
	g_deathStarRenderObject.orientMatrixDirty = 1;
	fview_newcalcrotate(g_deathStarRenderObject.roll, g_deathStarRenderObject.pitch,
						g_deathStarRenderObject.yaw, 0, &g_deathStarRenderObject);
	savedForcedLodLevel = g_forcedLodLevel;
	g_forcedLodLevel = DEATH_STAR_DETAIL_LOD;
	RenderScene_DrawObjectModel(&g_deathStarRenderObject);
	g_forcedLodLevel = savedForcedLodLevel;
}

// FUNCTION: XW 0x426AB0
void DeathStar_DrawSurfaceAndTrench(void) {
	float savedMipScale = g_textureMipScale;
	OptimizedPolyObject* modelHeader;
	OptimizedPolyObject savedHeader;
	int altitude, tileRadius, row, column, initialTileWorldX;
	g_textureMipScale = 0.0f;
	DeathStar_InitSurfaceTexture();
	modelHeader = Memory_LockHandle(g_loadedModels[DEATH_STAR_TRENCH_MODEL]);
	savedHeader = *modelHeader;
	g_deathStarSurfaceModelHeader.selfMarker = modelHeader;
	*modelHeader = g_deathStarSurfaceModelHeader;
	g_deathStarRenderObject.worldX = g_flightCamera.worldPosition.x & DEATH_STAR_CELL_WORLD_MASK;
	g_deathStarRenderObject.worldY = g_flightCamera.worldPosition.y & DEATH_STAR_CELL_WORLD_MASK;
	g_billboardObjectOrTypeIndex = DEATH_STAR_DETAIL_RENDER_REF;
	g_deathStarRenderObject.worldZ = 0;
	g_deathStarRenderObject.objectType = DEATH_STAR_TRENCH_MODEL;
	g_deathStarRenderObject.genusId = XW_GENUS_SCENERY;
	g_deathStarRenderObject.roll = 0;
	g_deathStarRenderObject.yaw = 0;
	g_deathStarRenderObject.pitch = TRIG2_QUARTER_TURN;
	g_camRelWorldX =
		(int32_t)((uint32_t)g_deathStarRenderObject.worldX - (uint32_t)g_flightCamera.worldPosition.x);
	g_camRelWorldY =
		(int32_t)((uint32_t)g_deathStarRenderObject.worldY - (uint32_t)g_flightCamera.worldPosition.y);
	g_camRelWorldZ = (int32_t)(0u - (uint32_t)g_flightCamera.worldPosition.z);
	g_objectViewZ = transfm2_geteyez(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
	g_objectViewX = transfm2_geteyex(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
	g_objectViewY = transfm2_geteyey(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
	g_deathStarRenderObject.orientMatrixDirty = 1;
	fview_newcalcrotate(g_deathStarRenderObject.roll, g_deathStarRenderObject.pitch,
						g_deathStarRenderObject.yaw, 0, &g_deathStarRenderObject);
	altitude = g_flightCamera.worldPosition.z;
	for (tileRadius = 1; altitude > DEATH_STAR_TRENCH_TILE_ALTITUDE; ++tileRadius)
		altitude >>= 1;
	g_deathStarRenderObject.worldY = (int32_t)((uint32_t)g_deathStarRenderObject.worldY -
											   (uint32_t)tileRadius * DEATH_STAR_TRENCH_TILE_SIZE);
	initialTileWorldX = (int32_t)((uint32_t)g_deathStarRenderObject.worldX -
								  (uint32_t)(tileRadius + 1) * DEATH_STAR_TRENCH_TILE_SIZE);
	g_deathStarRenderObject.worldX = initialTileWorldX;
	for (row = -tileRadius; row <= tileRadius; ++row) {
		for (column = -tileRadius; column <= tileRadius; ++column) {
			int crossesTrenchBand, tileViewZ, tileViewY;
			g_deathStarRenderObject.worldX =
				(int32_t)((uint32_t)g_deathStarRenderObject.worldX + DEATH_STAR_TRENCH_TILE_SIZE);
			crossesTrenchBand = g_deathStarRenderObject.worldX >= -DEATH_STAR_SURFACE_TILE_HALF_WIDTH &&
								g_deathStarRenderObject.worldX <= DEATH_STAR_SURFACE_TILE_HALF_WIDTH;
			g_camRelWorldX = (int32_t)((uint32_t)g_deathStarRenderObject.worldX -
									   (uint32_t)g_flightCamera.worldPosition.x);
			g_camRelWorldY = (int32_t)((uint32_t)g_deathStarRenderObject.worldY -
									   (uint32_t)g_flightCamera.worldPosition.y);
			g_camRelWorldZ = (int32_t)((uint32_t)g_deathStarRenderObject.worldZ -
									   (uint32_t)g_flightCamera.worldPosition.z);
			g_objectViewZ = transfm2_geteyez(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
			tileViewZ = g_objectViewZ;
			g_objectViewX = transfm2_geteyex(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
			tileViewY = transfm2_geteyey(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
			tileViewZ = (int32_t)((uint32_t)tileViewZ + DEATH_STAR_TRENCH_TILE_DEPTH_MARGIN);
			g_objectViewY = tileViewY;
			if (tileViewZ < 0 || g_objectViewX > tileViewZ ||
				(int32_t)(0u - (uint32_t)g_objectViewX) > tileViewZ || tileViewY > tileViewZ ||
				(int32_t)(0u - (uint32_t)tileViewY) > tileViewZ)
				continue;
			if (crossesTrenchBand) {
				float leftLocalX = (int32_t)(0u - DEATH_STAR_TRENCH_OPENING_HALF_WIDTH -
											 (uint32_t)g_deathStarRenderObject.worldX);
				float rightLocalX;
				if (leftLocalX > g_deathStarSurfaceVertices[2].x) {
					float savedX = g_deathStarSurfaceVertices[0].x;
					float savedU = g_deathStarSurfaceTexCoords[0].u;
					float clippedU;
					g_deathStarSurfaceVertices[1].x = (float)leftLocalX;
					clippedU = g_deathStarSurfaceTexCoords[0].u -
							   (g_deathStarSurfaceVertices[0].x - (double)leftLocalX) *
								   g_deathStarSurfaceUvPerWorldUnit;
					g_deathStarSurfaceVertices[0].x = (float)leftLocalX;
					g_deathStarSurfaceTexCoords[1].u = (float)clippedU;
					g_deathStarSurfaceTexCoords[0].u = (float)clippedU;
					RenderScene_DrawObjectModel(&g_deathStarRenderObject);
					g_deathStarSurfaceVertices[1].x = savedX;
					g_deathStarSurfaceTexCoords[1].u = savedU;
					g_deathStarSurfaceVertices[0].x = savedX;
					g_deathStarSurfaceTexCoords[0].u = savedU;
				}
				rightLocalX = (int32_t)(DEATH_STAR_TRENCH_OPENING_HALF_WIDTH -
										(uint32_t)g_deathStarRenderObject.worldX);
				if (rightLocalX < g_deathStarSurfaceVertices[0].x) {
					float savedX = g_deathStarSurfaceVertices[2].x;
					float savedU = g_deathStarSurfaceTexCoords[2].u;
					g_deathStarSurfaceVertices[3].x = (float)rightLocalX;
					g_deathStarSurfaceTexCoords[3].u =
						(float)(((double)rightLocalX - g_deathStarSurfaceVertices[2].x) *
									g_deathStarSurfaceUvPerWorldUnit +
								g_deathStarSurfaceTexCoords[2].u);
					g_deathStarSurfaceVertices[2].x = (float)rightLocalX;
					g_deathStarSurfaceTexCoords[2].u = g_deathStarSurfaceTexCoords[3].u;
					RenderScene_DrawObjectModel(&g_deathStarRenderObject);
					g_deathStarSurfaceVertices[3].x = savedX;
					g_deathStarSurfaceVertices[2].x = savedX;
					g_deathStarSurfaceTexCoords[3].u = savedU;
					g_deathStarSurfaceTexCoords[2].u = savedU;
				}
			} else {
				RenderScene_DrawObjectModel(&g_deathStarRenderObject);
			}
		}
		g_deathStarRenderObject.worldX = initialTileWorldX;
		g_deathStarRenderObject.worldY =
			(int32_t)((uint32_t)g_deathStarRenderObject.worldY + DEATH_STAR_TRENCH_TILE_SIZE);
	}
	*modelHeader = savedHeader;
	g_objViewMat_R1_X = -g_camMatR0_Y;
	g_objViewMat_R1_Y = -g_camMatR1_Y;
	g_textureMipScale = savedMipScale;
	g_objViewMat_R1_Z = -g_camMatR2_Y;
	g_objViewMat_R2_X = g_camMatR0_Z;
	g_objViewMat_R2_Y = g_camMatR1_Z;
	g_objViewMat_R2_Z = g_camMatR2_Z;
	g_objectLightDirectionX = g_modelLightDirectionX;
	g_objViewMat_R0_X = g_camMatR0_X;
	g_objectLightDirectionY = g_modelLightDirectionY;
	g_objectLightDirectionZ = g_modelLightDirectionZ;
	g_objViewMat_R0_Y = g_camMatR1_X;
	g_objViewMat_R0_Z = g_camMatR2_X;
	g_fviewSideX_Q15 = FVIEW_MATRIX_ONE;
	g_fviewSideY_Q15 = 0;
	g_fviewSideZ_Q15 = 0;
	g_fviewForwardX_Q15 = 0;
	g_fviewForwardY_Q15 = FVIEW_MATRIX_ONE;
	g_fviewForwardZ_Q15 = 0;
	g_fviewUpX_Q15 = 0;
	g_fviewUpY_Q15 = 0;
	g_fviewUpZ_Q15 = FVIEW_MATRIX_ONE;
	g_billboardObjectOrTypeIndex = DEATH_STAR_DETAIL_RENDER_REF;
	if (g_flightCamera.worldPosition.z >= 0) {
		int relativeX, relativeY, relativeZ;
		int cellViewX, cellViewY, cellViewZ;
		int minimumRadius, altitudeLevels, detailRadius, rowWidth;
		int scaledXViewX = (int16_t)g_camMatR0_X * DEATH_STAR_TRENCH_STEP_SCALE;
		int scaledXViewY = (int16_t)g_camMatR1_X * DEATH_STAR_TRENCH_STEP_SCALE;
		int scaledXViewZ = (int16_t)g_camMatR2_X * DEATH_STAR_TRENCH_STEP_SCALE;
		int scaledYViewX = (int16_t)g_camMatR0_Y * DEATH_STAR_TRENCH_STEP_SCALE;
		int scaledYViewY = (int16_t)g_camMatR1_Y * DEATH_STAR_TRENCH_STEP_SCALE;
		int scaledYViewZ = (int16_t)g_camMatR2_Y * DEATH_STAR_TRENCH_STEP_SCALE;
		g_deathStarCellViewSteps.stepXViewX = scaledXViewX >> DEATH_STAR_SURFACE_STEP_SHIFT;
		g_deathStarCellViewSteps.halfXViewX = scaledXViewX >> DEATH_STAR_SURFACE_HALF_STEP_SHIFT;
		g_deathStarCellViewSteps.stepXViewY = scaledXViewY >> DEATH_STAR_SURFACE_STEP_SHIFT;
		g_deathStarCellViewSteps.halfXViewY = scaledXViewY >> DEATH_STAR_SURFACE_HALF_STEP_SHIFT;
		g_deathStarCellViewSteps.stepXViewZ = scaledXViewZ >> DEATH_STAR_SURFACE_STEP_SHIFT;
		g_deathStarCellViewSteps.halfXViewZ = scaledXViewZ >> DEATH_STAR_SURFACE_HALF_STEP_SHIFT;
		g_deathStarCellViewSteps.stepYViewX = scaledYViewX >> DEATH_STAR_SURFACE_STEP_SHIFT;
		g_deathStarCellViewSteps.halfYViewX = scaledYViewX >> DEATH_STAR_SURFACE_HALF_STEP_SHIFT;
		g_deathStarCellViewSteps.stepYViewY = scaledYViewY >> DEATH_STAR_SURFACE_STEP_SHIFT;
		g_deathStarCellViewSteps.halfYViewY = scaledYViewY >> DEATH_STAR_SURFACE_HALF_STEP_SHIFT;
		g_deathStarCellViewSteps.stepYViewZ = scaledYViewZ >> DEATH_STAR_SURFACE_STEP_SHIFT;
		g_deathStarCellViewSteps.halfYViewZ = scaledYViewZ >> DEATH_STAR_SURFACE_HALF_STEP_SHIFT;
		g_deathStarSurfaceCellX = g_flightCamera.worldPosition.x >> DEATH_STAR_CELL_COORDINATE_SHIFT;
		g_deathStarSurfaceCellY = g_flightCamera.worldPosition.y >> DEATH_STAR_CELL_COORDINATE_SHIFT;
		relativeZ = (int32_t)(0u - (uint32_t)g_flightCamera.worldPosition.z);
		relativeY = (int32_t)((uint32_t)(g_flightCamera.worldPosition.y & DEATH_STAR_CELL_WORLD_MASK) -
							  (uint32_t)g_flightCamera.worldPosition.y);
		relativeX = (int32_t)((uint32_t)(g_flightCamera.worldPosition.x & DEATH_STAR_CELL_WORLD_MASK) -
							  (uint32_t)g_flightCamera.worldPosition.x);
		cellViewX = transfm2_geteyex(relativeX, relativeY, relativeZ);
		cellViewY = transfm2_geteyey(relativeX, relativeY, relativeZ);
		cellViewZ = transfm2_geteyez(relativeX, relativeY, relativeZ);
		minimumRadius = g_flightGraphicsDetailPreset <= 1 ? 1 : g_flightGraphicsDetailPreset;
		altitude = g_flightCamera.worldPosition.z;
		for (altitudeLevels = 0; altitude > DEATH_STAR_SURFACE_DETAIL_ALTITUDE; ++altitudeLevels)
			altitude >>= 1;
		detailRadius = altitudeLevels * (g_flightGraphicsDetailPreset + 1) / 2 + minimumRadius;
		cellViewX = (int32_t)((uint32_t)g_deathStarCellViewSteps.halfXViewX +
							  (uint32_t)g_deathStarCellViewSteps.halfYViewX -
							  (uint32_t)detailRadius * ((uint32_t)g_deathStarCellViewSteps.stepXViewX +
														(uint32_t)g_deathStarCellViewSteps.stepYViewX) +
							  (uint32_t)cellViewX);
		cellViewY = (int32_t)((uint32_t)g_deathStarCellViewSteps.halfXViewY +
							  (uint32_t)g_deathStarCellViewSteps.halfYViewY -
							  (uint32_t)detailRadius * ((uint32_t)g_deathStarCellViewSteps.stepXViewY +
														(uint32_t)g_deathStarCellViewSteps.stepYViewY) +
							  (uint32_t)cellViewY);
		cellViewZ = (int32_t)((uint32_t)g_deathStarCellViewSteps.halfXViewZ +
							  (uint32_t)g_deathStarCellViewSteps.halfYViewZ -
							  (uint32_t)detailRadius * ((uint32_t)g_deathStarCellViewSteps.stepXViewZ +
														(uint32_t)g_deathStarCellViewSteps.stepYViewZ) +
							  (uint32_t)cellViewZ);
		g_deathStarSurfaceCellX -= detailRadius;
		g_deathStarSurfaceCellY -= detailRadius;
		rowWidth = 2 * detailRadius + 1;
		for (row = -detailRadius; row <= detailRadius; ++row) {
			for (column = -detailRadius; column <= detailRadius; ++column) {
				DeathStar_DrawSurfaceCellDetails(cellViewX, cellViewY, cellViewZ);
				cellViewX = (int32_t)((uint32_t)cellViewX + (uint32_t)g_deathStarCellViewSteps.stepXViewX);
				cellViewZ = (int32_t)((uint32_t)cellViewZ + (uint32_t)g_deathStarCellViewSteps.stepXViewZ);
				cellViewY = (int32_t)((uint32_t)cellViewY + (uint32_t)g_deathStarCellViewSteps.stepXViewY);
				++g_deathStarSurfaceCellX;
			}
			g_deathStarSurfaceCellX -= rowWidth;
			cellViewX = (int32_t)((uint32_t)cellViewX + (uint32_t)g_deathStarCellViewSteps.stepYViewX -
								  (uint32_t)rowWidth * (uint32_t)g_deathStarCellViewSteps.stepXViewX);
			cellViewY = (int32_t)((uint32_t)cellViewY + (uint32_t)g_deathStarCellViewSteps.stepYViewY -
								  (uint32_t)rowWidth * (uint32_t)g_deathStarCellViewSteps.stepXViewY);
			cellViewZ = (int32_t)((uint32_t)cellViewZ + (uint32_t)g_deathStarCellViewSteps.stepYViewZ -
								  (uint32_t)rowWidth * (uint32_t)g_deathStarCellViewSteps.stepXViewZ);
			++g_deathStarSurfaceCellY;
		}
	}
	DeathStar_DrawTrench();
}

// FUNCTION: XW 0x427320
void DeathStar_DrawSurfaceCellDetails(int cellViewX, int cellViewY, int cellViewZ) {
	int absoluteViewX = cellViewX;
	int absoluteViewY = cellViewY;
	int expandedDepth = (int32_t)((uint32_t)cellViewZ + DEATH_STAR_SURFACE_CELL_CULL_MARGIN);
	uint32_t cellWorldX, cellWorldY;
	uint16_t hashStart, hashIndex, cellKey, placementCount, limitedCount;
	uint8_t variant;
	int16_t hasDamageState;
	int originalViewZ;
	const XwSurfacePlacementList* placementList;
	int placementIndex;
	int placementsRemaining;
	if (expandedDepth < 0)
		return;
	if (absoluteViewX < 0)
		absoluteViewX = (int32_t)(0u - (uint32_t)absoluteViewX);
	if (absoluteViewX > expandedDepth)
		return;
	if (absoluteViewY < 0)
		absoluteViewY = (int32_t)(0u - (uint32_t)absoluteViewY);
	if (absoluteViewY > expandedDepth)
		return;
	cellWorldX = ((uint32_t)(uint16_t)g_deathStarSurfaceCellX << DEATH_STAR_CELL_COORDINATE_SHIFT) |
				 (uint16_t)g_camRelWorldX;
	cellWorldY = ((uint32_t)(uint16_t)g_deathStarSurfaceCellY << DEATH_STAR_CELL_COORDINATE_SHIFT) |
				 (uint16_t)g_camRelWorldY;
	hashStart =
		(g_deathStarSurfaceCellX & DEATH_STAR_CELL_HASH_AXIS_MASK) |
		((g_deathStarSurfaceCellY & DEATH_STAR_CELL_HASH_AXIS_MASK) << DEATH_STAR_CELL_HASH_AXIS_BITS);
	++g_billboardObjectOrTypeIndex;
	g_camRelWorldX = (int32_t)cellWorldX;
	cellKey =
		hashStart | (((g_deathStarSurfaceCellX & DEATH_STAR_CELL_KEY_X_MASK) |
					  ((g_deathStarSurfaceCellY & DEATH_STAR_CELL_KEY_Y_MASK) << DEATH_STAR_CELL_KEY_Y_SHIFT))
					 << DEATH_STAR_CELL_HASH_AXIS_BITS);
	variant = (g_deathStarSurfaceCellX & 1) | ((g_deathStarSurfaceCellY & 1) << 1);
	g_camRelWorldY = (int32_t)cellWorldY;
	if (g_deathStarSurfaceCellX == 0 || g_deathStarSurfaceCellX == -1)
		variant = DEATH_STAR_SURFACE_EDGE_LAYOUT;
	originalViewZ = cellViewZ;
	if (variant != DEATH_STAR_SURFACE_LOW_LAYOUT && variant != DEATH_STAR_SURFACE_EDGE_LAYOUT) {
		uint32_t baseWorldX = (cellWorldX & DEATH_STAR_CELL_WORLD_MASK) | DEATH_STAR_CELL_MIDPOINT;
		uint32_t baseWorldY = (cellWorldY & DEATH_STAR_CELL_WORLD_MASK) | DEATH_STAR_CELL_MIDPOINT;
		g_billboardObjectOrTypeIndex += DEATH_STAR_SURFACE_BASE_REFERENCE_OFFSET;
		g_camRelWorldZ = (int32_t)(0u - (uint32_t)g_flightCamera.worldPosition.z);
		g_camRelWorldY = (int32_t)(baseWorldY - (uint32_t)g_flightCamera.worldPosition.y);
		g_camRelWorldX = (int32_t)(baseWorldX - (uint32_t)g_flightCamera.worldPosition.x);
		DeathStar_DrawSurfaceDetailModel(
			DEATH_STAR_SURFACE_BASE_MODEL,
			(int32_t)((uint32_t)g_camRelWorldX + (uint32_t)g_flightCamera.worldPosition.x),
			(int32_t)((uint32_t)g_camRelWorldY + (uint32_t)g_flightCamera.worldPosition.y),
			(int32_t)((uint32_t)g_camRelWorldZ + (uint32_t)g_flightCamera.worldPosition.z));
		cellViewX = (int32_t)((uint32_t)g_cameraWorldZToViewXSteps[DEATH_STAR_SURFACE_BASE_STEP_INDEX] -
							  (uint32_t)g_deathStarCellViewSteps.halfXViewX -
							  (uint32_t)g_deathStarCellViewSteps.halfYViewX + (uint32_t)cellViewX);
		g_billboardObjectOrTypeIndex -= DEATH_STAR_SURFACE_BASE_REFERENCE_OFFSET;
		cellViewY = (int32_t)((uint32_t)g_cameraWorldZToViewYSteps[DEATH_STAR_SURFACE_BASE_STEP_INDEX] -
							  (uint32_t)g_deathStarCellViewSteps.halfXViewY -
							  (uint32_t)g_deathStarCellViewSteps.halfYViewY + (uint32_t)cellViewY);
		cellViewZ = (int32_t)((uint32_t)g_cameraWorldZToViewZSteps[DEATH_STAR_SURFACE_BASE_STEP_INDEX] -
							  (uint32_t)g_deathStarCellViewSteps.halfXViewZ -
							  (uint32_t)g_deathStarCellViewSteps.halfYViewZ + (uint32_t)cellViewZ);
	} else {
		cellViewX = (int32_t)((uint32_t)cellViewX - ((uint32_t)g_deathStarCellViewSteps.halfXViewX +
													 (uint32_t)g_deathStarCellViewSteps.halfYViewX));
		cellViewY = (int32_t)((uint32_t)cellViewY - ((uint32_t)g_deathStarCellViewSteps.halfXViewY +
													 (uint32_t)g_deathStarCellViewSteps.halfYViewY));
		cellViewZ = (int32_t)((uint32_t)cellViewZ - ((uint32_t)g_deathStarCellViewSteps.halfXViewZ +
													 (uint32_t)g_deathStarCellViewSteps.halfYViewZ));
	}
	if (originalViewZ > (g_flightGraphicsDetailPreset + 1) << DEATH_STAR_CELL_COORDINATE_SHIFT)
		return;
	hasDamageState = 1;
	hashIndex = hashStart;
	if (g_surfaceCellDamageStates[hashIndex].cellKey == 0) {
		hasDamageState = 0;
	} else {
		while (g_surfaceCellDamageStates[hashIndex].cellKey != cellKey) {
			++hashIndex;
			if (hashIndex == DEATH_STAR_SURFACE_CELL_COUNT)
				hashIndex = 0;
			if (hashIndex == hashStart || g_surfaceCellDamageStates[hashIndex].cellKey == 0) {
				hasDamageState = 0;
				break;
			}
		}
	}
	++g_billboardObjectOrTypeIndex;
	placementList = g_surfacePlacementLists[variant];
	placementCount = placementList->count;
	if (placementCount < g_surfaceObjectDetailLimit)
		limitedCount = placementCount;
	else
		limitedCount = g_surfaceObjectDetailLimit;
	placementCount = DEATH_STAR_SHORT_LAYOUT_OBJECT_COUNT;
	if (limitedCount >= DEATH_STAR_SHORT_LAYOUT_OBJECT_COUNT)
		placementCount = limitedCount;
	for (placementIndex = 0, placementsRemaining = placementCount; placementsRemaining != 0;
		 ++placementIndex, --placementsRemaining) {
		uint16_t placementType;
		int objectType;
		uint16_t packedPosition, xNibble, yNibble;
		int viewOffsetX, viewOffsetY, viewOffsetZ, viewX, viewY, viewZ, extent;
		int relativeX, relativeY;
		float projectedNumerator;
		float projectedSize;
		if (hasDamageState &&
			g_surfaceCellDamageStates[hashIndex].objectHealthOrEffectState[placementIndex] == 0)
			continue;
		packedPosition = placementList->placements[placementIndex].packedPosition;
		placementType = placementList->placements[placementIndex].objectType;
		xNibble = packedPosition & DEATH_STAR_PLACEMENT_NIBBLE_MASK;
		yNibble = (packedPosition >> DEATH_STAR_PLACEMENT_NIBBLE_BITS) & DEATH_STAR_PLACEMENT_NIBBLE_MASK;
		relativeX =
			(int32_t)((((uint32_t)(uint16_t)g_deathStarSurfaceCellX << DEATH_STAR_CELL_COORDINATE_SHIFT) |
					   (uint16_t)(xNibble << DEATH_STAR_PLACEMENT_STEP_SHIFT)) -
					  (uint32_t)g_flightCamera.worldPosition.x);
		g_camRelWorldX = relativeX;
		relativeY =
			(int32_t)((((uint32_t)(uint16_t)g_deathStarSurfaceCellY << DEATH_STAR_CELL_COORDINATE_SHIFT) |
					   (uint16_t)(yNibble << DEATH_STAR_PLACEMENT_STEP_SHIFT)) -
					  (uint32_t)g_flightCamera.worldPosition.y);
		g_camRelWorldY = relativeY;
		if (variant == DEATH_STAR_SURFACE_LOW_LAYOUT || variant == DEATH_STAR_SURFACE_EDGE_LAYOUT)
			g_camRelWorldZ = (int32_t)(0u - (uint32_t)g_flightCamera.worldPosition.z);
		else
			g_camRelWorldZ = (int32_t)(DEATH_STAR_SURFACE_BASE_HEIGHT - (uint32_t)g_flightCamera.worldPosition.z);
		if (xNibble == 0) {
			viewOffsetZ = 0;
			viewOffsetY = 0;
			viewOffsetX = 0;
		} else {
			viewOffsetX = g_cameraWorldXToViewXSteps[xNibble - 1];
			viewOffsetY = g_cameraWorldXToViewYSteps[xNibble - 1];
			viewOffsetZ = g_cameraWorldXToViewZSteps[xNibble - 1];
		}
		if (yNibble != 0) {
			viewOffsetX =
				(int32_t)((uint32_t)viewOffsetX + (uint32_t)g_cameraWorldYToViewXSteps[yNibble - 1]);
			viewOffsetY =
				(int32_t)((uint32_t)viewOffsetY + (uint32_t)g_cameraWorldYToViewYSteps[yNibble - 1]);
			viewOffsetZ =
				(int32_t)((uint32_t)viewOffsetZ + (uint32_t)g_cameraWorldYToViewZSteps[yNibble - 1]);
		}
		viewX = (int32_t)((uint32_t)cellViewX + DEATH_STAR_PLACEMENT_VIEW_SCALE * (uint32_t)viewOffsetX);
		viewY = (int32_t)((uint32_t)cellViewY + DEATH_STAR_PLACEMENT_VIEW_SCALE * (uint32_t)viewOffsetY);
		viewZ = (int32_t)((uint32_t)cellViewZ + DEATH_STAR_PLACEMENT_VIEW_SCALE * (uint32_t)viewOffsetZ);
		objectType = placementType;
		extent = g_modelTypeTable[objectType].maxBoundsExtent;
		projectedNumerator = (float)((uint32_t)g_projScaleInt * (uint32_t)extent);
		projectedSize = projectedNumerator /
						(float)(uint32_t)collide_roughdistance3d(relativeX, relativeY, g_camRelWorldZ) *
						g_deathStarDetailScreenSizeScale[g_flightGraphicsDetailPreset];
		if (objectType >= DEATH_STAR_SURFACE_LARGE_TYPE_FIRST &&
			objectType <= DEATH_STAR_SURFACE_LARGE_TYPE_LAST) {
			if (projectedSize < g_surfaceSpecialDetailMinProjectedSize)
				continue;
		} else if (projectedSize < g_surfaceDetailMinProjectedSize) {
			continue;
		}
		expandedDepth = (int32_t)((uint32_t)viewZ + (uint32_t)extent);
		if (expandedDepth < 0)
			continue;
		if (viewX < 0) {
			if ((int32_t)(0u - (uint32_t)viewX) > expandedDepth)
				continue;
		} else if (viewX > expandedDepth) {
			continue;
		}
		if (viewY < 0) {
			if ((int32_t)(0u - (uint32_t)viewY) > expandedDepth)
				continue;
		} else if (viewY > expandedDepth) {
			continue;
		}
		DeathStar_DrawSurfaceDetailModel(
			objectType, (int32_t)((uint32_t)g_camRelWorldX + (uint32_t)g_flightCamera.worldPosition.x),
			(int32_t)((uint32_t)g_camRelWorldY + (uint32_t)g_flightCamera.worldPosition.y),
			(int32_t)((uint32_t)g_camRelWorldZ + (uint32_t)g_flightCamera.worldPosition.z));
		++g_billboardObjectOrTypeIndex;
	}
}

// FUNCTION: XW 0x4278D0
void DeathStar_UpdateSurfaceGuns(void) {
	int cellIndex;
	uint16_t fireIntervalTicks, earliestCountdown;
	int16_t firedCellCount;
	XwSurfaceGunCell* earliestCell;
#ifdef XW_MODERN
	if (!XwFlightTypes_Dos() && create_findslot(XW_GENUS_OTHER_PROJECTILE) == XW_OBJECT_SLOT_UNAVAILABLE)
#else
	if (create_findslot(XW_GENUS_OTHER_PROJECTILE) == XW_OBJECT_SLOT_UNAVAILABLE)
#endif
		return;
	if (g_surfaceGunCellRefreshTicks <= (int)g_elapsedTicks) {
		g_surfaceGunCellRefreshTicks = g_playerFlightState.object->worldZ < 0
										   ? DEATH_STAR_TRENCH_GUN_INITIAL_TICKS
										   : DEATH_STAR_SURFACE_GUN_REFRESH_TICKS;
		DeathStar_RebuildSurfaceGunCells();
	} else {
		g_surfaceGunCellRefreshTicks -= g_elapsedTicks;
	}
	firedCellCount = 0;
	earliestCell = g_surfaceGunCells;
	fireIntervalTicks = g_playerFlightState.object->worldZ < 0 ? DEATH_STAR_TRENCH_GUN_INITIAL_TICKS
															   : DEATH_STAR_SURFACE_GUN_INITIAL_TICKS;
	earliestCountdown = fireIntervalTicks;
	for (cellIndex = 0; (uint16_t)cellIndex < g_surfaceGunCellCount; ++cellIndex) {
		if (g_surfaceGunCells[cellIndex].fireCountdownTicks <= g_elapsedTicks) {
			g_surfaceGunCells[cellIndex].fireCountdownTicks = fireIntervalTicks;
			DeathStar_FireSurfaceGunCell(&g_surfaceGunCells[cellIndex]);
			++firedCellCount;
		} else {
			uint16_t remainingTicks = g_surfaceGunCells[cellIndex].fireCountdownTicks - g_elapsedTicks;
			g_surfaceGunCells[cellIndex].fireCountdownTicks = remainingTicks;
			if (remainingTicks < earliestCountdown) {
				earliestCountdown = remainingTicks;
				earliestCell = &g_surfaceGunCells[cellIndex];
			}
		}
	}
	if (firedCellCount == 0 && earliestCountdown < (uint16_t)(fireIntervalTicks >> 1)) {
		earliestCell->fireCountdownTicks = fireIntervalTicks;
		DeathStar_FireSurfaceGunCell(earliestCell);
	}
}

// FUNCTION: XW 0x427A00
void DeathStar_RebuildSurfaceGunCells(void) {
	uint16_t objectIndex;
	g_surfaceGunCellCount = 0;
	for (objectIndex = 0; objectIndex < XW_CRAFT_OBJECT_COUNT; ++objectIndex) {
		if (g_objectTable[objectIndex].objectType != XW_OBJ_NONE && g_objectTable[objectIndex].iff == 0 &&
			g_objectTable[objectIndex].worldZ < DEATH_STAR_SURFACE_GUN_MAX_HEIGHT) {
			uint32_t worldY = (uint32_t)g_objectTable[objectIndex].worldY;
			uint32_t worldX = (uint32_t)g_objectTable[objectIndex].worldX;
			uint32_t cellY = worldY >> DEATH_STAR_CELL_COORDINATE_SHIFT;
			uint32_t cellX = worldX >> DEATH_STAR_CELL_COORDINATE_SHIFT;
			int16_t neighborX;
			DeathStar_AddSurfaceGunCell((int16_t)cellX, (int16_t)cellY);
			if ((uint16_t)worldX >= DEATH_STAR_CELL_MIDPOINT) {
				neighborX = (int16_t)(cellX + 1);
			} else {
				neighborX = (int16_t)(cellX - 1);
			}
			DeathStar_AddSurfaceGunCell(neighborX, (int16_t)cellY);
			if ((uint16_t)worldY >= DEATH_STAR_CELL_MIDPOINT) {
				++cellY;
			} else {
				--cellY;
			}
			DeathStar_AddSurfaceGunCell(neighborX, (int16_t)cellY);
			DeathStar_AddSurfaceGunCell((int16_t)cellX, (int16_t)cellY);
		}
	}
}

// FUNCTION: XW 0x427AB0
void DeathStar_AddSurfaceGunCell(int16_t cellX, int16_t cellY) {
	uint16_t scanIndex;
	if (g_surfaceGunCellCount == DEATH_STAR_SURFACE_GUN_CELL_CAPACITY) {
		return;
	}
	for (scanIndex = 0; scanIndex < g_surfaceGunCellCount; ++scanIndex) {
		if (g_surfaceGunCells[scanIndex].cellX == cellX && g_surfaceGunCells[scanIndex].cellY == cellY) {
			return;
		}
	}
	if (cellX == 0) {
		g_surfaceGunCells[g_surfaceGunCellCount].cellX = DEATH_STAR_TRENCH_GUN_CELL_X;
		g_surfaceGunCells[g_surfaceGunCellCount].cellY = cellY;
		g_surfaceGunCells[g_surfaceGunCellCount].fireCountdownTicks = DEATH_STAR_TRENCH_GUN_INITIAL_TICKS;
		++g_surfaceGunCellCount;
		if (g_surfaceGunCellCount == DEATH_STAR_SURFACE_GUN_CELL_CAPACITY) {
			return;
		}
	}
	g_surfaceGunCells[g_surfaceGunCellCount].cellX = cellX;
	g_surfaceGunCells[g_surfaceGunCellCount].cellY = cellY;
	g_surfaceGunCells[g_surfaceGunCellCount].fireCountdownTicks = DEATH_STAR_SURFACE_GUN_INITIAL_TICKS;
	++g_surfaceGunCellCount;
}

// FUNCTION: XW 0x427B70
void DeathStar_FireSurfaceGunCell(const struct XwSurfaceGunCell* cell) {
	int16_t cellX = cell->cellX;
	uint16_t cellY = cell->cellY;
	uint16_t hashStart, hashIndex, cellKey;
	int16_t isTrench, hasDamageState;
	uint8_t layoutIndex;
	const XwSurfacePlacementList* placementList;
	int gunCount, gunIndex;
	int16_t mountYaw = 0;
	uint16_t yawLimit = 0;
	if (cellX == DEATH_STAR_TRENCH_GUN_CELL_X) {
		hashStart = cellY & (DEATH_STAR_SURFACE_CELL_COUNT - 1);
		layoutIndex = cellY & DEATH_STAR_CELL_HASH_AXIS_MASK;
		isTrench = 1;
#ifdef XW_MODERN
		if (XwFlightTypes_Dos())
			cellKey = Dos94World_TrenchKey(cellY);
		else
#endif
			cellKey = (cellY & DEATH_STAR_CELL_HASH_AXIS_MASK) |
					  (((cellY & DEATH_STAR_CELL_KEY_Y_MASK) | DEATH_STAR_TRENCH_KEY_FLAG) << 8);
		if (cellY == 0)
			layoutIndex = DEATH_STAR_TRENCH_END_LAYOUT;
	} else {
		hashStart = ((cellY & DEATH_STAR_CELL_HASH_AXIS_MASK) << DEATH_STAR_CELL_HASH_AXIS_BITS) |
					(cellX & DEATH_STAR_CELL_HASH_AXIS_MASK);
		isTrench = 0;
		layoutIndex = (cellX & 1) | ((cellY & 1) << 1);
		cellKey = hashStart | (((cellX & DEATH_STAR_CELL_KEY_X_MASK) |
								((cellY & DEATH_STAR_CELL_KEY_Y_MASK) << DEATH_STAR_CELL_KEY_Y_SHIFT))
							   << DEATH_STAR_CELL_HASH_AXIS_BITS);
		if (cellX == 0 || cellX == -1)
			layoutIndex = DEATH_STAR_SURFACE_EDGE_LAYOUT;
	}
	hasDamageState = 1;
	for (hashIndex = hashStart;;) {
		if (g_surfaceCellDamageStates[hashIndex].cellKey == 0) {
			hasDamageState = 0;
			break;
		}
		if (g_surfaceCellDamageStates[hashIndex].cellKey == cellKey)
			break;
		++hashIndex;
		if (hashIndex == DEATH_STAR_SURFACE_CELL_COUNT)
			hashIndex = 0;
		if (hashIndex == hashStart) {
			hasDamageState = 0;
			break;
		}
	}
	if (isTrench) {
		placementList = g_trenchPlacementLists[layoutIndex];
		gunCount = DEATH_STAR_TRENCH_GUN_COUNT;
		if (layoutIndex == DEATH_STAR_TRENCH_END_LAYOUT)
			gunCount = DEATH_STAR_TRENCH_END_GUN_COUNT;
	} else {
		gunCount = DEATH_STAR_SURFACE_GUN_COUNT;
		placementList = g_surfacePlacementLists[layoutIndex];
	}
	for (gunIndex = 0; gunIndex < gunCount; ++gunIndex) {
		uint16_t packedPosition, objectType, mountMeshIndex;
		int shooterX, shooterY, shooterZ;
		uint16_t targetIndex, candidateIndex;
		unsigned int bestDistance, rangeLimit;
		int targetSlot;
		uint16_t leadSteps, yawDifference;
		int16_t aimYaw, aimPitch;
		uint16_t projectileType, clampedDistance, rangePenalty, speed, rangeAccuracy, speedAccuracy;
		uint16_t hitProbability, projectileIndex;
		if (hasDamageState && g_surfaceCellDamageStates[hashIndex].objectHealthOrEffectState[gunIndex] == 0)
			continue;
		packedPosition = placementList->placements[gunIndex].packedPosition;
		objectType = placementList->placements[gunIndex].objectType;
		if (isTrench) {
			shooterX = DEATH_STAR_TRENCH_MOUNT_X;
			mountMeshIndex = 1;
			if ((packedPosition & DEATH_STAR_TRENCH_SIDE_MASK) != 0) {
				if ((packedPosition & DEATH_STAR_TRENCH_SIDE_MASK) == 1) {
					mountMeshIndex = 0;
					shooterX = -DEATH_STAR_TRENCH_MOUNT_X;
				} else {
					shooterX = 0;
				}
			}
			shooterY = (int32_t)(((uint32_t)cellY << DEATH_STAR_CELL_COORDINATE_SHIFT) |
								 (((packedPosition >> DEATH_STAR_TRENCH_LONGITUDINAL_SHIFT) &
								   DEATH_STAR_PLACEMENT_NIBBLE_MASK)
								  << DEATH_STAR_PLACEMENT_STEP_SHIFT));
			shooterZ = packedPosition >> DEATH_STAR_TRENCH_HEIGHT_SHIFT;
			if (shooterZ != 0)
				mountMeshIndex += 2;
			shooterZ = (int16_t)(shooterZ * DEATH_STAR_TRENCH_HEIGHT_STEP + DEATH_STAR_TRENCH_FLOOR);
#ifdef XW_MODERN
			if (!XwFlightTypes_Dos())
#endif
				if (mountMeshIndex >= ModelMesh_GetCachedObjectTypeMeshCount(objectType))
					mountMeshIndex =
						mountMeshIndex == 3 && ModelMesh_GetCachedObjectTypeMeshCount(objectType) > 1;
		} else {
			shooterX = (int32_t)(((uint32_t)(uint16_t)cellX << DEATH_STAR_CELL_COORDINATE_SHIFT) |
								 (uint16_t)(packedPosition << DEATH_STAR_PLACEMENT_STEP_SHIFT));
			shooterY = (int32_t)(((uint32_t)cellY << DEATH_STAR_CELL_COORDINATE_SHIFT) |
								 ((packedPosition & 0xF0) << 8));
			mountMeshIndex = 0;
			shooterZ = DEATH_STAR_SURFACE_BASE_HEIGHT;
			if (layoutIndex == DEATH_STAR_SURFACE_LOW_LAYOUT || layoutIndex == DEATH_STAR_SURFACE_EDGE_LAYOUT)
				shooterZ = 0;
		}
#ifdef XW_MODERN
		objectType = XwFlightTypes_CanonicalType(objectType);
#endif
		if (objectType == DEATH_STAR_GUN_201) {
			shooterZ += DEATH_STAR_GUN_201_HEIGHT;
			if ((uint16_t)math2_getrandom() >= TRIG2_ANGLE_SIGN_BIT)
				shooterX -= DEATH_STAR_GUN_201_MUZZLE_X;
			else
				shooterX += DEATH_STAR_GUN_201_MUZZLE_X;
		} else if (objectType == DEATH_STAR_GUN_193 || objectType == DEATH_STAR_GUN_194) {
			if (mountMeshIndex < 2) {
				shooterZ += DEATH_STAR_GUN_LOWER_HEIGHT;
				if (mountMeshIndex & 1)
					shooterX -= DEATH_STAR_GUN_LOWER_MUZZLE_X;
				else
					shooterX += DEATH_STAR_GUN_LOWER_MUZZLE_X;
			} else {
				shooterZ += DEATH_STAR_GUN_UPPER_OFFSET;
				if (mountMeshIndex & 1)
					shooterX -= DEATH_STAR_GUN_UPPER_OFFSET;
				else
					shooterX += DEATH_STAR_GUN_UPPER_OFFSET;
			}
		} else if (objectType == DEATH_STAR_GUN_195) {
			shooterZ += DEATH_STAR_GUN_195_HEIGHT;
		} else if (objectType >= DEATH_STAR_GUN_170) {
			shooterZ += DEATH_STAR_GUN_LARGE_HEIGHT;
		} else {
			shooterZ += DEATH_STAR_GUN_SMALL_HEIGHT;
		}
		switch (objectType) {
			case DEATH_STAR_GUN_167:
			case DEATH_STAR_GUN_193:
				mountYaw = 0;
				yawLimit = DEATH_STAR_GUN_YAW_LIMIT;
				break;
			case DEATH_STAR_GUN_168:
				mountYaw = DEATH_STAR_GUN_YAW_RIGHT;
				yawLimit = DEATH_STAR_GUN_YAW_LIMIT;
				break;
			case DEATH_STAR_GUN_170:
				mountYaw = DEATH_STAR_GUN_YAW_LEFT;
				yawLimit = DEATH_STAR_GUN_YAW_LIMIT;
				break;
			case DEATH_STAR_GUN_171:
			case DEATH_STAR_GUN_194:
			case DEATH_STAR_GUN_201:
				mountYaw = DEATH_STAR_GUN_YAW_BACK;
				yawLimit = DEATH_STAR_GUN_YAW_LIMIT;
				break;
			case DEATH_STAR_GUN_169:
			case DEATH_STAR_GUN_172:
			case DEATH_STAR_GUN_195:
				mountYaw = 0;
				yawLimit = DEATH_STAR_GUN_FULL_ACCURACY;
				break;
			default:
				break;
		}
		targetIndex = 0;
		bestDistance = UINT32_MAX;
		for (candidateIndex = 0; candidateIndex < XW_CRAFT_OBJECT_COUNT; ++candidateIndex) {
			int targetZ = g_objectTable[candidateIndex].worldZ;
			if (g_objectTable[candidateIndex].objectType != XW_OBJ_NONE &&
				g_objectTable[candidateIndex].iff == 0 && targetZ < DEATH_STAR_SURFACE_GUN_MAX_HEIGHT) {
				unsigned int candidateDistance;
				if (isTrench ? targetZ > 0 : targetZ < 0)
					continue;
				candidateDistance = collide_roughdistance3d(
					(int32_t)((uint32_t)g_objectTable[candidateIndex].worldX - (uint32_t)shooterX),
					(int32_t)((uint32_t)g_objectTable[candidateIndex].worldY - (uint32_t)shooterY),
					(int32_t)((uint32_t)targetZ - (uint32_t)shooterZ));
				if (candidateDistance < bestDistance) {
					targetIndex = candidateIndex;
					bestDistance = candidateDistance;
				}
			}
		}
		rangeLimit = DEATH_STAR_CELL_MIDPOINT;
		if (isTrench || layoutIndex == DEATH_STAR_SURFACE_EDGE_LAYOUT)
			rangeLimit = DEATH_STAR_SURFACE_GUN_MAX_HEIGHT;
		if (bestDistance >= rangeLimit)
			continue;
		targetSlot = targetIndex;
		trig2_ctop((int32_t)((uint32_t)g_objectTable[targetSlot].worldX - (uint32_t)shooterX),
				   (int32_t)((uint32_t)g_objectTable[targetSlot].worldY - (uint32_t)shooterY),
				   (int32_t)((uint32_t)g_objectTable[targetSlot].worldZ - (uint32_t)shooterZ));
		g_trig2PolarDistance = (int32_t)((uint32_t)g_simStepScale * (uint32_t)g_trig2PolarDistance);
		if (isTrench)
			g_trig2PolarDistance >>= DEATH_STAR_GUN_TRENCH_LEAD_SHIFT;
		else
			g_trig2PolarDistance >>= DEATH_STAR_GUN_SURFACE_LEAD_SHIFT;
		leadSteps = g_trig2PolarDistance;
		if (!isTrench)
			leadSteps = (math2_getrandom() & DEATH_STAR_GUN_LEAD_RANDOM_MASK) + leadSteps - 1;
#ifdef XW_MODERN
		trig2_ctop((int32_t)((uint32_t)g_objectTable[targetSlot].worldX +
							 (uint32_t)leadSteps * (uint32_t)XwReferenceMotion_Axis(targetSlot, 0) -
							 (uint32_t)shooterX),
				   (int32_t)((uint32_t)g_objectTable[targetSlot].worldY +
							 (uint32_t)leadSteps * (uint32_t)XwReferenceMotion_Axis(targetSlot, 1) -
							 (uint32_t)shooterY),
				   (int32_t)((uint32_t)g_objectTable[targetSlot].worldZ +
							 (uint32_t)leadSteps * (uint32_t)XwReferenceMotion_Axis(targetSlot, 2) -
							 (uint32_t)shooterZ));
#else
		trig2_ctop((int32_t)((uint32_t)g_objectTable[targetSlot].worldX +
							 (uint32_t)leadSteps * ((uint32_t)g_objectTable[targetSlot].worldX -
													(uint32_t)g_objectTable[targetSlot].prevWorldX) -
							 (uint32_t)shooterX),
				   (int32_t)((uint32_t)g_objectTable[targetSlot].worldY +
							 (uint32_t)leadSteps * ((uint32_t)g_objectTable[targetSlot].worldY -
													(uint32_t)g_objectTable[targetSlot].prevWorldY) -
							 (uint32_t)shooterY),
				   (int32_t)((uint32_t)g_objectTable[targetSlot].worldZ +
							 (uint32_t)leadSteps * ((uint32_t)g_objectTable[targetSlot].worldZ -
													(uint32_t)g_objectTable[targetSlot].prevWorldZ) -
							 (uint32_t)shooterZ));
#endif
		aimPitch = g_trig2Pitch;
		aimYaw = g_trig2Yaw;
		yawDifference = aimYaw - mountYaw;
		if (yawDifference >= TRIG2_ANGLE_SIGN_BIT)
			yawDifference = -yawDifference;
		if (yawDifference > yawLimit || (uint16_t)aimPitch >= DEATH_STAR_GUN_PITCH_MAX ||
			(uint16_t)aimPitch <= DEATH_STAR_GUN_PITCH_MIN)
			continue;
		projectileType = XW_OBJ_LASER_145;
		if (objectType >= DEATH_STAR_GUN_170)
			projectileType = XW_OBJ_LASER_146;
		clampedDistance = DEATH_STAR_GUN_FULL_ACCURACY;
		if (g_trig2PolarDistance < DEATH_STAR_SURFACE_GUN_MAX_HEIGHT)
			clampedDistance = g_trig2PolarDistance;
		if (isTrench) {
			projectileType = XW_OBJ_LASER_145;
			rangePenalty = clampedDistance >> 1;
		} else if (layoutIndex == DEATH_STAR_SURFACE_EDGE_LAYOUT) {
			rangePenalty = 0;
			projectileType = XW_OBJ_LASER_146;
		} else {
			rangePenalty = clampedDistance >> 1;
		}
		speed = g_objectTable[targetIndex].speed;
		rangeAccuracy = ~rangePenalty;
		if (speed <= DEATH_STAR_GUN_SPEED_LOW)
			speedAccuracy = DEATH_STAR_GUN_FULL_ACCURACY;
		else if (speed >= DEATH_STAR_GUN_SPEED_HIGH)
			speedAccuracy = TRIG2_ANGLE_SIGN_BIT;
		else
			speedAccuracy =
				DEATH_STAR_GUN_SPEED_ACCURACY_BASE - (speed << DEATH_STAR_GUN_SPEED_ACCURACY_SHIFT);
		hitProbability = math2_fraction(rangeAccuracy, speedAccuracy);
		if ((uint16_t)math2_getrandom() > hitProbability) {
			int yawScatter =
				((uint16_t)math2_getrandom() - DEATH_STAR_GUN_SCATTER_BIAS) & DEATH_STAR_GUN_SCATTER_MASK;
			int pitchScatter;
			if ((uint16_t)math2_getrandom() >= TRIG2_ANGLE_SIGN_BIT)
				yawScatter = -yawScatter;
			aimYaw += yawScatter;
			pitchScatter =
				((uint16_t)math2_getrandom() - DEATH_STAR_GUN_SCATTER_BIAS) & DEATH_STAR_GUN_SCATTER_MASK;
			if ((uint16_t)math2_getrandom() >= TRIG2_ANGLE_SIGN_BIT)
				aimPitch -= pitchScatter;
			else
				aimPitch += pitchScatter;
		}
		projectileIndex = create_findslot(XW_GENUS_OTHER_PROJECTILE);
		if (projectileIndex != XW_OBJECT_SLOT_UNAVAILABLE) {
			int projectileSlot = projectileIndex;
			int projectileDataIndex = projectileType - XW_OBJ_LASER_143;
			int launchOffsetX, launchOffsetY, launchOffsetZ;
			int guidanceSlot;
			g_objectTable[projectileSlot].familyId = DEATH_STAR_GUN_PROJECTILE_FAMILY;
			g_objectTable[projectileSlot].genusId = XW_GENUS_OTHER_PROJECTILE;
#ifdef XW_MODERN
			g_objectTable[projectileSlot].objectType = XwFlightTypes_ObjectType(projectileType);
#else
			g_objectTable[projectileSlot].objectType = projectileType;
#endif
			g_objectTable[projectileSlot].ageSeconds = 1;
			g_objectTable[projectileSlot].sourceObjectRef = DEATH_STAR_GUN_SOURCE_REF;
			g_objectTable[projectileSlot].sourceObjectType = XW_OBJ_NONE;
			g_objectTable[projectileSlot].iff = DEATH_STAR_GUN_PROJECTILE_IFF;
			g_objectTable[projectileSlot].pitch = aimPitch;
			g_objectTable[projectileSlot].roll = 0;
			g_objectTable[projectileSlot].yaw = aimYaw;
			g_objectTable[projectileSlot].orientMatrixDirty = 1;
			g_objectTable[projectileSlot].moveVectorDirty = 1;
			g_objectTable[projectileSlot].speed = g_projectileSpeedByType[projectileDataIndex];
			g_objectTable[projectileSlot].damageAmount = g_projectileBaseDamageByType[projectileDataIndex];
			if (!isTrench && layoutIndex == DEATH_STAR_SURFACE_EDGE_LAYOUT)
				g_objectTable[projectileSlot].damageAmount *= DEATH_STAR_GUN_EDGE_DAMAGE_SCALE;
			g_objectTable[projectileSlot].lifetimeTicks =
				XW_SIMULATION_TICKS_PER_SECOND * g_projectileLifetimeSecondsByType[projectileDataIndex];
			fview_calcrotatemove(aimPitch, aimYaw, &g_objectTable[projectileSlot]);
			g_objectTable[projectileSlot].prevWorldX = shooterX;
			g_objectTable[projectileSlot].prevWorldY = shooterY;
			g_objectTable[projectileSlot].prevWorldZ = shooterZ;
			launchOffsetX = (int32_t)((uint64_t)(g_projectileLaunchOffsetByType[projectileDataIndex] *
												 (int64_t)g_craftMoveX) >>
									  FVIEW_MATRIX_FRACTION_BITS);
			launchOffsetY = (int32_t)((uint64_t)(g_projectileLaunchOffsetByType[projectileDataIndex] *
												 (int64_t)g_craftMoveZ) >>
									  FVIEW_MATRIX_FRACTION_BITS);
			launchOffsetZ = (int32_t)((uint64_t)(g_projectileLaunchOffsetByType[projectileDataIndex] *
												 (int64_t)g_craftMoveY) >>
									  FVIEW_MATRIX_FRACTION_BITS);
			g_objectTable[projectileSlot].worldX = (int32_t)((uint32_t)shooterX + (uint32_t)launchOffsetX);
			g_objectTable[projectileSlot].worldY = (int32_t)((uint32_t)shooterY + (uint32_t)launchOffsetY);
			g_objectTable[projectileSlot].worldZ = (int32_t)((uint32_t)shooterZ + (uint32_t)launchOffsetZ);
			fsfx_triggerlasersfx(projectileIndex);
			guidanceSlot = (uint16_t)(projectileIndex - XW_CRAFT_OBJECT_COUNT);
			g_objectTable[projectileSlot].instanceData = &g_warheadGuidanceTable[guidanceSlot];
			g_warheadGuidanceTable[guidanceSlot].homingTier = 0;
			g_warheadGuidanceTable[guidanceSlot].targetObjIdx = 0;
		}
	}
}

// FUNCTION: XW 0x428420
int DeathStar_TestSurfaceCollision(uint16_t objectIndex) {
	int16_t probeCellX = g_collisionProbeWorldX >> DEATH_STAR_CELL_COORDINATE_SHIFT;
	int16_t probeCellY = g_collisionProbeWorldY >> DEATH_STAR_CELL_COORDINATE_SHIFT;
	int16_t startCellX = g_collisionSegmentStartWorldX >> DEATH_STAR_CELL_COORDINATE_SHIFT;
	int16_t startCellY = g_collisionSegmentStartWorldY >> DEATH_STAR_CELL_COORDINATE_SHIFT;
	uint8_t probeLayoutKind = ((probeCellY & 1) << 1) | (probeCellX & 1);
	uint8_t startLayoutKind = ((startCellY & 1) << 1) | (startCellX & 1);
	int16_t terrainHit = 0;
	if (probeCellX == 0 || probeCellX == -1)
		probeLayoutKind = DEATH_STAR_SURFACE_EDGE_LAYOUT;
	if (startCellX == 0 || startCellX == -1)
		startLayoutKind = DEATH_STAR_SURFACE_EDGE_LAYOUT;
	if (g_objectTable[objectIndex].iff == 0) {
		if (DeathStar_TestCellObjectCollision(objectIndex, probeCellX, probeCellY, 0) != 0)
			return 1;
		if (startCellX == probeCellX) {
			if (startCellY != probeCellY &&
				DeathStar_TestCellObjectCollision(objectIndex, probeCellX, startCellY, 0) != 0)
				return 1;
		} else {
			if (DeathStar_TestCellObjectCollision(objectIndex, startCellX, probeCellY, 0) != 0)
				return 1;
			if (startCellY != probeCellY) {
				if (DeathStar_TestCellObjectCollision(objectIndex, startCellX, startCellY, 0) != 0)
					return 1;
				if (DeathStar_TestCellObjectCollision(objectIndex, probeCellX, startCellY, 0) != 0)
					return 1;
			}
		}
	}
	if (probeLayoutKind != DEATH_STAR_SURFACE_LOW_LAYOUT &&
		probeLayoutKind != DEATH_STAR_SURFACE_EDGE_LAYOUT &&
		g_collisionProbeWorldZ < DEATH_STAR_SURFACE_BASE_HEIGHT &&
		(int16_t)(g_collisionProbeWorldX >> DEATH_STAR_CELL_COORDINATE_SHIFT) != 0 &&
		(int16_t)(g_collisionProbeWorldX >> DEATH_STAR_CELL_COORDINATE_SHIFT) != -1) {
		int16_t xWallHeight = g_collisionProbeWorldX;
		if (((uint32_t)g_collisionProbeWorldX & DEATH_STAR_CELL_MIDPOINT) != 0)
			xWallHeight = -xWallHeight;
		xWallHeight -= DEATH_STAR_SURFACE_WALL_MARGIN;
		if (xWallHeight >= 0 && (uint16_t)xWallHeight >= (uint16_t)g_collisionProbeWorldZ) {
			int16_t yWallHeight = g_collisionProbeWorldY;
			if (((uint32_t)g_collisionProbeWorldY & DEATH_STAR_CELL_MIDPOINT) != 0)
				yWallHeight = -yWallHeight;
			yWallHeight -= DEATH_STAR_SURFACE_WALL_MARGIN;
			if (yWallHeight >= 0 && (uint16_t)yWallHeight >= (uint16_t)g_collisionProbeWorldZ)
				terrainHit = 1;
		}
	}
	if (!terrainHit) {
		if (startLayoutKind != DEATH_STAR_SURFACE_LOW_LAYOUT &&
			startLayoutKind != DEATH_STAR_SURFACE_EDGE_LAYOUT &&
			g_collisionSegmentStartWorldZ < DEATH_STAR_SURFACE_BASE_HEIGHT &&
			(int16_t)(g_collisionSegmentStartWorldX >> DEATH_STAR_CELL_COORDINATE_SHIFT) != 0 &&
			(int16_t)(g_collisionSegmentStartWorldX >> DEATH_STAR_CELL_COORDINATE_SHIFT) != -1) {
			int16_t xWallHeight = g_collisionSegmentStartWorldX;
			if (((uint32_t)g_collisionSegmentStartWorldX & DEATH_STAR_CELL_MIDPOINT) != 0)
				xWallHeight = -xWallHeight;
			xWallHeight -= DEATH_STAR_SURFACE_WALL_MARGIN;
			if (xWallHeight >= 0 && (uint16_t)xWallHeight >= (uint16_t)g_collisionSegmentStartWorldZ) {
				int16_t yWallHeight = g_collisionSegmentStartWorldY;
				if (((uint32_t)g_collisionSegmentStartWorldY & DEATH_STAR_CELL_MIDPOINT) != 0)
					yWallHeight = -yWallHeight;
				yWallHeight -= DEATH_STAR_SURFACE_WALL_MARGIN;
				if (yWallHeight >= 0 && (uint16_t)yWallHeight >= (uint16_t)g_collisionSegmentStartWorldZ)
					terrainHit = 1;
			}
		}
		if (!terrainHit &&
			(g_collisionProbeWorldZ >= 0 || (g_collisionProbeWorldX >= -DEATH_STAR_TRENCH_MOUNT_X &&
											 g_collisionProbeWorldX <= DEATH_STAR_TRENCH_MOUNT_X &&
											 g_collisionProbeWorldZ >= DEATH_STAR_TRENCH_FLOOR)))
			return 0;
	}
	if (objectIndex == g_playerFlightState.objectIndex)
		return 1;
	g_objectTable[objectIndex].worldX = g_objectTable[objectIndex].prevWorldX;
	g_objectTable[objectIndex].worldY = g_objectTable[objectIndex].prevWorldY;
	g_objectTable[objectIndex].worldZ = g_objectTable[objectIndex].prevWorldZ;
#ifdef XW_MODERN
	if (XwFlightTypes_Dos())
		g_objectTable[objectIndex].instanceData = NULL;
#endif
	g_objectTable[objectIndex].ageSeconds = 0;
	g_objectTable[objectIndex].lifetimeTicks = 0;
	g_objectTable[objectIndex].billboardScaleCode = 0;
	g_objectTable[objectIndex].speed = 0;
	g_objectTable[objectIndex].pitch = 0;
	g_objectTable[objectIndex].yaw = 0;
	g_objectTable[objectIndex].roll = 0;
	g_objectTable[objectIndex].orientMatrixDirty = 1;
	g_objectTable[objectIndex].moveVectorDirty = 1;
	if (g_objectTable[objectIndex].genusId == XW_GENUS_DEBRIS ||

#ifdef XW_MODERN
		g_objectTable[objectIndex].objectType == XwFlightTypes_ObjectType(XW_OBJ_WARHEAD_149)
#else
		g_objectTable[objectIndex].objectType == XW_OBJ_WARHEAD_149
#endif
		||

#ifdef XW_MODERN
		g_objectTable[objectIndex].objectType == XwFlightTypes_ObjectType(XW_OBJ_TRACKED_WARHEAD)
#else
		g_objectTable[objectIndex].objectType == XW_OBJ_TRACKED_WARHEAD
#endif
	) {
#ifdef XW_MODERN
		g_objectTable[objectIndex].objectType =
			XwFlightTypes_ObjectType((math2_getrandom() & 1) + XW_OBJ_EXPLOSION_133);
#else
		g_objectTable[objectIndex].objectType = (math2_getrandom() & 1) + XW_OBJ_EXPLOSION_133;
#endif
		fsfx_triggersfx(FSFX_OBJECT_EXPLOSION_SLOT, objectIndex);
	} else {
#ifdef XW_MODERN
		g_objectTable[objectIndex].objectType = XwFlightTypes_ObjectType(XW_OBJ_ASTEROID_IMPACT);
#else
		g_objectTable[objectIndex].objectType = XW_OBJ_ASTEROID_IMPACT;
#endif
		fsfx_triggersfx(FSFX_ASTEROID_IMPACT_SLOT, objectIndex);
	}
#ifdef XW_MODERN
	XwFlightIntegration_Reset(objectIndex);
	XwRenderObjects_ReplaceMobile(objectIndex);
#endif
	g_objectTable[objectIndex].genusId = XW_GENUS_EXPLOSION_EFFECT;
	g_objectTable[objectIndex].familyId = XW_OBJECT_FAMILY_5;
	g_objectTable[objectIndex].animationState = XW_OBJECT_ANIMATION_BREAKUP;
	return 1;
}

// FUNCTION: XW 0x428950
int DeathStar_TestCellObjectCollision(uint16_t objectIndex, int16_t cellX, int16_t cellY,
									  int16_t trenchPass) {
	int16_t isTrench, hasDamageState;
	uint8_t layoutIndex;
	uint16_t hashStart, hashIndex, cellKey;
	const XwSurfacePlacementList* placementList;
	uint16_t placementCount;
	unsigned int placementIndex;
	if ((cellX == 0 || cellX == -1) && trenchPass == 1) {
		if (g_collisionProbeWorldZ > 0 && g_collisionSegmentStartWorldZ > 0)
			return 0;
		isTrench = 1;
		layoutIndex = cellY & DEATH_STAR_CELL_HASH_AXIS_MASK;
		hashStart = cellY & (DEATH_STAR_SURFACE_CELL_COUNT - 1);
#ifdef XW_MODERN
		if (XwFlightTypes_Dos())
			cellKey = Dos94World_TrenchKey(cellY);
		else
#endif
			cellKey = layoutIndex |
					  ((uint16_t)((cellY & ~DEATH_STAR_TRENCH_SIDE_MASK) | DEATH_STAR_TRENCH_KEY_FLAG) << 8);
		if (cellY == 0)
			layoutIndex = DEATH_STAR_TRENCH_END_LAYOUT;
	} else {
		if (cellX == 0 || cellX == -1) {
			if (DeathStar_TestCellObjectCollision(objectIndex, cellX, cellY, 1) != 0)
				return 1;
			if (g_collisionProbeWorldZ < 0 && g_collisionSegmentStartWorldZ < 0)
				return 0;
		}
		isTrench = 0;
		hashStart = (cellX & DEATH_STAR_CELL_HASH_AXIS_MASK) |
					((cellY & DEATH_STAR_CELL_HASH_AXIS_MASK) << DEATH_STAR_CELL_HASH_AXIS_BITS);
		cellKey = hashStart | (((cellX & DEATH_STAR_CELL_KEY_X_MASK) |
								((cellY & DEATH_STAR_CELL_KEY_Y_MASK) << DEATH_STAR_CELL_KEY_Y_SHIFT))
							   << DEATH_STAR_CELL_HASH_AXIS_BITS);
		layoutIndex = (cellX & 1) | ((cellY & 1) << 1);
		if (cellX == 0 || cellX == -1)
			layoutIndex = DEATH_STAR_SURFACE_EDGE_LAYOUT;
		if (layoutIndex != DEATH_STAR_SURFACE_LOW_LAYOUT && layoutIndex != DEATH_STAR_SURFACE_EDGE_LAYOUT &&
			g_collisionProbeWorldZ < DEATH_STAR_SURFACE_BASE_HEIGHT &&
			g_collisionSegmentStartWorldZ < DEATH_STAR_SURFACE_BASE_HEIGHT)
			return 0;
	}
	hashIndex = hashStart;
	hasDamageState = 1;
	for (;;) {
		if (g_surfaceCellDamageStates[hashIndex].cellKey == 0) {
			hasDamageState = 0;
			break;
		}
		if (g_surfaceCellDamageStates[hashIndex].cellKey == cellKey)
			break;
		if (++hashIndex == DEATH_STAR_SURFACE_CELL_COUNT)
			hashIndex = 0;
		if (hashIndex == hashStart) {
			hasDamageState = 0;
			break;
		}
	}
	if (isTrench)
		placementList = g_trenchPlacementLists[layoutIndex];
	else
		placementList = g_surfacePlacementLists[layoutIndex];
	placementCount = placementList->count;
	if (placementCount >= g_surfaceObjectDetailLimit)
		placementCount = g_surfaceObjectDetailLimit;
	if (placementCount <= DEATH_STAR_SHORT_LAYOUT_OBJECT_COUNT)
		placementCount = DEATH_STAR_SHORT_LAYOUT_OBJECT_COUNT;
	if (isTrench && layoutIndex == DEATH_STAR_TRENCH_END_LAYOUT) {
		g_surfaceSpecialTargetCollisionMode = 1;
		if (g_trenchSpecialCellVoicePlayed == 0 && fsfx_speakeravailable() != 0 &&
			g_playerFlightState.object->worldZ < 0) {
			fsfx_triggervoicesfx(DEATH_STAR_SPECIAL_CELL_VOICE);
			g_trenchSpecialCellVoicePlayed = 1;
		}
		placementCount = DEATH_STAR_TRENCH_END_GUN_COUNT;
	}
	for (placementIndex = 0; placementCount-- != 0; ++placementIndex) {
		uint16_t packedPosition, objectType, meshIndex;
		int worldX, worldY, worldZ;
		int minX, minY, minZ, maxX, maxY, maxZ;
		int16_t hitResult;
		uint16_t remainingHealth = 1;
#ifdef XW_MODERN
		const Dos94MeshView* dosMesh = NULL;
#endif
		if (placementCount != DEATH_STAR_TRENCH_END_GUN_COUNT - 1)
			g_surfaceSpecialTargetCollisionMode = 0;
		if (hasDamageState && placementIndex < DEATH_STAR_SURFACE_OBJECT_COUNT) {
			uint8_t health = g_surfaceCellDamageStates[hashIndex].objectHealthOrEffectState[placementIndex];
			if (health == 0 || health >= DEATH_STAR_DESTRUCTION_STATE_FIRST)
				continue;
		}
		packedPosition = placementList->placements[placementIndex].packedPosition;
		objectType = placementList->placements[placementIndex].objectType;
		if (isTrench) {
			uint16_t side = packedPosition & DEATH_STAR_TRENCH_SIDE_MASK;
			uint16_t height =
				(packedPosition >> DEATH_STAR_TRENCH_HEIGHT_SHIFT) * DEATH_STAR_TRENCH_HEIGHT_STEP;
			meshIndex = 1;
			worldX = DEATH_STAR_TRENCH_MOUNT_X;
			if (side != 0) {
				if (side == 1) {
					meshIndex = 0;
					worldX = -DEATH_STAR_TRENCH_MOUNT_X;
				} else {
#ifdef XW_MODERN
					meshIndex = XwFlightTypes_Dos() ? 0 : 3;
#else
					meshIndex = 3;
#endif
					worldX = 0;
				}
			}
#ifdef XW_MODERN
			if (!XwFlightTypes_Dos() && objectType == DEATH_STAR_TRENCH_SPECIAL_MESH_TYPE && side == 2)
#else
			if (objectType == DEATH_STAR_TRENCH_SPECIAL_MESH_TYPE && side == 2)
#endif
				meshIndex = 2;
			worldY = (int32_t)(((uint32_t)(uint16_t)cellY << DEATH_STAR_CELL_COORDINATE_SHIFT) |
							   (((packedPosition >> DEATH_STAR_TRENCH_LONGITUDINAL_SHIFT) &
								 DEATH_STAR_PLACEMENT_NIBBLE_MASK)
								<< DEATH_STAR_PLACEMENT_STEP_SHIFT));
			if (height != 0)
				meshIndex += 2;
			worldZ = (int16_t)(height + DEATH_STAR_TRENCH_FLOOR);
#ifdef XW_MODERN
			if (!XwFlightTypes_Dos())
#endif
				if (meshIndex >= ModelMesh_GetCachedObjectTypeMeshCount(objectType))
					meshIndex = meshIndex == 3 && ModelMesh_GetCachedObjectTypeMeshCount(objectType) > 1;
		} else {
			meshIndex = 0;
			worldX = (int32_t)(((uint32_t)(uint16_t)cellX << DEATH_STAR_CELL_COORDINATE_SHIFT) |
							   (uint16_t)(packedPosition << DEATH_STAR_PLACEMENT_STEP_SHIFT));
			worldY = (int32_t)(((uint32_t)(uint16_t)cellY << DEATH_STAR_CELL_COORDINATE_SHIFT) |
							   ((packedPosition >> 4) << DEATH_STAR_PLACEMENT_STEP_SHIFT));
		}
#ifdef XW_MODERN
		if (XwFlightTypes_Dos()) {
			XwBounds16 bounds;
			dosMesh = Dos94World_Bounds(objectType, meshIndex, &bounds);
			if (!dosMesh)
				continue;
			minX = bounds.minX;
			minY = bounds.minY;
			minZ = bounds.minZ;
			maxX = bounds.maxX;
			maxY = bounds.maxY;
			maxZ = bounds.maxZ;
		} else
#endif
		{
			minX = ModelMesh_GetBoundsMinX(objectType, meshIndex);
			minY = ModelMesh_GetBoundsMinY(objectType, meshIndex);
			minZ = ModelMesh_GetBoundsMinZ(objectType, meshIndex);
			maxX = ModelMesh_GetBoundsMaxX(objectType, meshIndex);
			maxY = ModelMesh_GetBoundsMaxY(objectType, meshIndex);
			maxZ = ModelMesh_GetBoundsMaxZ(objectType, meshIndex);
		}
		if (isTrench) {
			int bound;
			bound = worldZ + minZ;
			if (g_collisionProbeWorldZ < bound && g_collisionSegmentStartWorldZ < bound)
				continue;
			bound = worldZ + maxZ;
			if (g_collisionProbeWorldZ > bound && g_collisionSegmentStartWorldZ > bound)
				continue;
			bound = worldX + maxX;
			if (g_collisionProbeWorldX > bound && g_collisionSegmentStartWorldX > bound)
				continue;
			bound = worldX + minX;
			if (g_collisionProbeWorldX < bound && g_collisionSegmentStartWorldX < bound)
				continue;
			bound = (int32_t)((uint32_t)worldY + (uint32_t)maxY);
			if (g_collisionProbeWorldY > bound && g_collisionSegmentStartWorldY > bound)
				continue;
			bound = (int32_t)((uint32_t)worldY + (uint32_t)minY);
			if (g_collisionProbeWorldY < bound && g_collisionSegmentStartWorldY < bound)
				continue;
		} else {
			uint16_t extentX = maxX - minX;
			uint16_t extentY = maxY - minY;
			uint16_t extentZ = maxZ - minZ;
			int32_t probeDelta, startDelta;
			int16_t probeCell, startCell;
			uint16_t probeDistance, startDistance;
#ifdef XW_MODERN
			if (XwFlightTypes_Dos()) {
				extentX = (uint16_t)(dosMesh->bounds[3] - dosMesh->bounds[0]) >> 1;
				extentY = (uint16_t)(dosMesh->bounds[4] - dosMesh->bounds[1]) >> 1;
				extentZ = (uint16_t)(dosMesh->bounds[5] - dosMesh->bounds[2]) >> 1;
			}
#endif
			if (layoutIndex != DEATH_STAR_SURFACE_LOW_LAYOUT && layoutIndex != DEATH_STAR_SURFACE_EDGE_LAYOUT)
				extentZ += DEATH_STAR_SURFACE_BASE_HEIGHT;
			if (g_collisionProbeWorldZ > extentZ && g_collisionSegmentStartWorldZ > extentZ)
				continue;
			probeDelta = (int32_t)((uint32_t)g_collisionProbeWorldX - (uint32_t)worldX);
			startDelta = (int32_t)((uint32_t)g_collisionSegmentStartWorldX - (uint32_t)worldX);
			probeCell = (uint32_t)probeDelta >> DEATH_STAR_CELL_COORDINATE_SHIFT;
			startCell = (uint32_t)startDelta >> DEATH_STAR_CELL_COORDINATE_SHIFT;
			probeDistance = probeDelta;
			startDistance = startDelta;
			if (((probeCell ^ startCell) & DEATH_STAR_CELL_MIDPOINT) == 0) {
				if (probeCell == -1)
					probeDistance = -probeDistance;
				if ((probeCell != 0 && probeCell != -1) || probeDistance > extentX) {
					if (startCell == -1)
						startDistance = -startDistance;
					if ((startCell != 0 && startCell != -1) || startDistance > extentX)
						continue;
				}
			}
			probeDelta = (int32_t)((uint32_t)g_collisionProbeWorldY - (uint32_t)worldY);
			startDelta = (int32_t)((uint32_t)g_collisionSegmentStartWorldY - (uint32_t)worldY);
			probeCell = (uint32_t)probeDelta >> DEATH_STAR_CELL_COORDINATE_SHIFT;
			startCell = (uint32_t)startDelta >> DEATH_STAR_CELL_COORDINATE_SHIFT;
			probeDistance = probeDelta;
			startDistance = startDelta;
			if (((probeCell ^ startCell) & DEATH_STAR_CELL_MIDPOINT) == 0) {
				if (probeCell == -1)
					probeDistance = -probeDistance;
				if ((probeCell != 0 && probeCell != -1) || probeDistance > extentY) {
					if (startCell == -1)
						startDistance = -startDistance;
					if ((startCell != 0 && startCell != -1) || startDistance > extentY)
						continue;
				}
			}
			worldZ = DEATH_STAR_SURFACE_BASE_HEIGHT;
			if (layoutIndex == DEATH_STAR_SURFACE_LOW_LAYOUT || layoutIndex == DEATH_STAR_SURFACE_EDGE_LAYOUT)
				worldZ = 0;
		}
#ifdef XW_MODERN
		if (XwFlightTypes_Dos())
			hitResult =
				Dos94World_Hit(dosMesh, worldX, worldY, worldZ, objectType == 105 || objectType == 106);
		else
#endif
		{
			if (objectType == DEATH_STAR_MIDPOINT_TYPE_FIRST || objectType == DEATH_STAR_MIDPOINT_TYPE_LAST) {
				hitResult = (int16_t)DEATH_STAR_CELL_MIDPOINT;
				g_collisionHitOffsetX =
					(int16_t)((uint32_t)g_collisionProbeWorldX - (uint32_t)g_collisionSegmentStartWorldX) >>
					1;
				g_collisionHitOffsetY =
					(int16_t)((uint32_t)g_collisionProbeWorldY - (uint32_t)g_collisionSegmentStartWorldY) >>
					1;
				g_collisionHitOffsetZ =
					(int16_t)((uint32_t)g_collisionProbeWorldZ - (uint32_t)g_collisionSegmentStartWorldZ) >>
					1;
			} else {
				hitResult = starship_CheckSweptMeshCollision(
					objectType, meshIndex, (int32_t)((uint32_t)g_collisionProbeWorldX - (uint32_t)worldX),
					(int32_t)((uint32_t)worldY - (uint32_t)g_collisionProbeWorldY),
					(int32_t)((uint32_t)g_collisionProbeWorldZ - (uint32_t)worldZ),
					(int32_t)((uint32_t)g_collisionSegmentStartWorldX - (uint32_t)worldX),
					(int32_t)((uint32_t)worldY - (uint32_t)g_collisionSegmentStartWorldY),
					(int32_t)((uint32_t)g_collisionSegmentStartWorldZ - (uint32_t)worldZ));
			}
		}
		if (hitResult == 0)
			continue;
		if (g_surfaceSpecialTargetHit != 0) {
			if (
#ifdef XW_MODERN
				g_objectTable[objectIndex].objectType == XwFlightTypes_ObjectType(XW_OBJ_WARHEAD_149)
#else
				g_objectTable[objectIndex].objectType == XW_OBJ_WARHEAD_149
#endif
				&& (g_missionHeader.missionRuleFlags & MISSION_RULE_SURFACE_SPECIAL_TARGET) != 0) {
				g_flightGlobalCountdownTimers.ticks[XW_TIMER_MISSION_GOAL] = DEATH_STAR_VICTORY_GOAL_TICKS;
				msg_messageprintf(XW_MSG_EXHAUST_PORT_DIRECT_HIT);
				if (fsfx_speakeravailable() != 0)
					fsfx_triggervoicesfx(DEATH_STAR_DIRECT_HIT_VOICE);
				else
					fsfx_triggersfx(FSFX_FRIENDLY_DEPARTURE_SLOT, FSFX_UNPOSITIONED_OBJECT);
				g_surfaceVictoryExitSecond = g_missionElapsedClock.seconds + DEATH_STAR_VICTORY_DELAY_SECONDS;
				if (g_surfaceVictoryExitSecond >= XW_SECONDS_PER_MINUTE)
					g_surfaceVictoryExitSecond = g_missionElapsedClock.seconds -
												 (XW_SECONDS_PER_MINUTE - DEATH_STAR_VICTORY_DELAY_SECONDS);
			} else {
				g_surfaceSpecialTargetHit = 0;
			}
		}
		collide_updatehits(objectIndex, 0);
		if (placementIndex < DEATH_STAR_SURFACE_OBJECT_COUNT) {
			uint16_t damage = g_objectTable[objectIndex].iff == DEATH_STAR_GUN_PROJECTILE_IFF
								  ? 0
								  : g_objectTable[objectIndex].damageAmount;
			uint8_t* health = g_surfaceCellDamageStates[hashIndex].objectHealthOrEffectState;
			damage >>= DEATH_STAR_DAMAGE_SHIFT;
			if (!hasDamageState) {
				const XwSurfaceHealthList* initialHealth;
				unsigned int healthIndex;
				unsigned int healthCount;
				g_surfaceCellDamageStates[hashIndex].cellKey = cellKey;
				if (isTrench)
					initialHealth = g_trenchHealthLists[layoutIndex];
				else
					initialHealth = g_surfaceHealthLists[layoutIndex];
				healthCount = initialHealth->count;
#ifdef XW_MODERN
				if (healthCount > sizeof(g_surfaceCellDamageStates[hashIndex].objectHealthOrEffectState))
					healthCount = sizeof(g_surfaceCellDamageStates[hashIndex].objectHealthOrEffectState);
#endif
				for (healthIndex = 0; healthIndex < healthCount; ++healthIndex)
					health[healthIndex] = initialHealth->health[healthIndex];
			}
			if (g_surfaceSpecialTargetCollisionMode == 0) {
				if (damage <= health[placementIndex])
					health[placementIndex] -= damage;
				else
					health[placementIndex] = 0;
			}
			remainingHealth = health[placementIndex];
			if (remainingHealth == 0)
				health[placementIndex] = DEATH_STAR_DESTRUCTION_STATE_FIRST;
		}
		if (objectIndex != g_playerFlightState.objectIndex) {
			g_objectTable[objectIndex].worldX =
				(int32_t)((uint32_t)g_collisionSegmentStartWorldX + (uint32_t)g_collisionHitOffsetX);
			g_objectTable[objectIndex].worldY =
				(int32_t)((uint32_t)g_collisionSegmentStartWorldY + (uint32_t)g_collisionHitOffsetY);
			g_objectTable[objectIndex].worldZ =
				(int32_t)((uint32_t)g_collisionSegmentStartWorldZ + (uint32_t)g_collisionHitOffsetZ);
			if (g_surfaceSpecialTargetHit != 0) {
				g_objectTable[objectIndex].objectType = XW_OBJ_NONE;
			} else {
				if (
#ifdef XW_MODERN
					g_objectTable[objectIndex].objectType == XwFlightTypes_ObjectType(XW_OBJ_WARHEAD_149)
#else
					g_objectTable[objectIndex].objectType == XW_OBJ_WARHEAD_149
#endif
					||

#ifdef XW_MODERN
					g_objectTable[objectIndex].objectType == XwFlightTypes_ObjectType(XW_OBJ_TRACKED_WARHEAD)
#else
					g_objectTable[objectIndex].objectType == XW_OBJ_TRACKED_WARHEAD
#endif
				) {
#ifdef XW_MODERN
					g_objectTable[objectIndex].objectType =
						XwFlightTypes_ObjectType((math2_getrandom() & 1) + XW_OBJ_EXPLOSION_133);
#else
					g_objectTable[objectIndex].objectType = (math2_getrandom() & 1) + XW_OBJ_EXPLOSION_133;
#endif
					fsfx_triggersfx(FSFX_OBJECT_EXPLOSION_SLOT, objectIndex);
				} else {
#ifdef XW_MODERN
					g_objectTable[objectIndex].objectType = XwFlightTypes_ObjectType(XW_OBJ_ASTEROID_IMPACT);
#else
					g_objectTable[objectIndex].objectType = XW_OBJ_ASTEROID_IMPACT;
#endif
					fsfx_triggersfx(FSFX_ASTEROID_IMPACT_SLOT, objectIndex);
				}
			}
#ifdef XW_MODERN
			XwFlightIntegration_Reset(objectIndex);
			XwRenderObjects_ReplaceMobile(objectIndex);
#endif
			g_objectTable[objectIndex].genusId = XW_GENUS_EXPLOSION_EFFECT;
			g_objectTable[objectIndex].familyId = XW_OBJECT_FAMILY_5;
			g_objectTable[objectIndex].animationState = XW_OBJECT_ANIMATION_BREAKUP;
#ifdef XW_MODERN
			if (XwFlightTypes_Dos())
				g_objectTable[objectIndex].instanceData = NULL;
#endif
			g_objectTable[objectIndex].ageSeconds = 0;
			g_objectTable[objectIndex].lifetimeTicks = 0;
			g_objectTable[objectIndex].billboardScaleCode = 0;
			g_objectTable[objectIndex].speed = 0;
			g_objectTable[objectIndex].pitch = 0;
			g_objectTable[objectIndex].yaw = 0;
			g_objectTable[objectIndex].roll = 0;
			g_objectTable[objectIndex].orientMatrixDirty = 1;
			g_objectTable[objectIndex].moveVectorDirty = 1;
		}
		if (remainingHealth == 0) {
			XwBounds16 bounds;
#ifdef XW_MODERN
			if (XwFlightTypes_Dos()) {
				minX = dosMesh->bounds[0];
				minY = dosMesh->bounds[1];
				minZ = dosMesh->bounds[2];
				maxX = dosMesh->bounds[3];
				maxY = dosMesh->bounds[4];
				maxZ = dosMesh->bounds[5];
			}
#endif
			if (objectIndex != g_playerFlightState.objectIndex) {
#ifdef XW_MODERN
				g_objectTable[objectIndex].objectType =
					XwFlightTypes_ObjectType((math2_getrandom() & 1) + XW_OBJ_EXPLOSION_133);
#else
				g_objectTable[objectIndex].objectType = (math2_getrandom() & 1) + XW_OBJ_EXPLOSION_133;
#endif
				fsfx_triggersfx(COLLIDE_CRAFT_EXPLOSION_SOUND, objectIndex);
			}
			collide_updatekills(g_objectTable[objectIndex].sourceObjectRef, XW_OBJECT_SLOT_UNAVAILABLE, 0);
			if (isTrench) {
				bounds.minX = minX;
				bounds.minY = minY;
				bounds.minZ = minZ;
				bounds.maxX = maxX;
				bounds.maxY = maxY;
				bounds.maxZ = maxZ;
				DeathStar_SpawnSurfaceDestructionEffects(worldX, worldY, worldZ, placementIndex, layoutIndex,
														 1, &bounds);
				return 1;
			}
			bounds.minX = minX;
			bounds.minY = minY;
			bounds.minZ = minZ;
			bounds.maxX = maxX;
			bounds.maxY = maxY;
			bounds.maxZ = maxZ;
			DeathStar_SpawnSurfaceDestructionEffects(worldX, worldY, worldZ, placementIndex, layoutIndex, 0,
													 &bounds);
		}
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x429490
void DeathStar_SpawnSurfaceDestructionEffects(int worldX, int worldY, int worldZ, uint16_t placementIndex,
											  uint16_t layoutIndex, int16_t isTrench,
											  const struct XwBounds16* bounds) {
	const uint8_t* patternIndices;
	const XwSurfaceDestructionPattern* pattern;
	int16_t centerX;
	int16_t centerY;
	int16_t centerZ;
	int effectIndex;
	uint16_t effectCount;
	if (isTrench != 0) {
		patternIndices = g_trenchDestructionPatternIndices[layoutIndex];
		centerX = bounds->minX + ((int16_t)(bounds->maxX - bounds->minX) >> 1);
		centerY = bounds->minY + ((int16_t)(bounds->maxY - bounds->minY) >> 1);
		centerZ = bounds->minZ + ((int16_t)(bounds->maxZ - bounds->minZ) >> 1);
#ifdef XW_MODERN
		if (XwFlightTypes_Dos()) {
			centerX >>= 1;
			centerY >>= 1;
			centerZ >>= 1;
		}
#endif
	} else {
		patternIndices = g_surfaceDestructionPatternIndices[layoutIndex];
	}
	pattern = g_surfaceDestructionPatterns[patternIndices[placementIndex]];
	effectCount = pattern->count;
	for (effectIndex = 0; effectCount-- != 0; ++effectIndex) {
		const XwSurfaceDestructionEffect* effect = &pattern->effects[effectIndex];
		uint16_t objectIndex;
		uint16_t fragmentIndex;
		objectIndex = create_findslot(XW_GENUS_EXPLOSION_EFFECT);
		if (objectIndex == XW_OBJECT_SLOT_UNAVAILABLE) {
			break;
		}
		g_objectTable[objectIndex].worldX = worldX;
		g_objectTable[objectIndex].worldY = worldY;
		g_objectTable[objectIndex].worldZ = worldZ;

#ifdef XW_MODERN
		g_objectTable[objectIndex].worldX =
			(int32_t)((uint32_t)g_objectTable[objectIndex].worldX + (uint32_t)(effect->offsetX));
#else
		g_objectTable[objectIndex].worldX += effect->offsetX;
#endif

#ifdef XW_MODERN
		g_objectTable[objectIndex].worldY =
			(int32_t)((uint32_t)g_objectTable[objectIndex].worldY + (uint32_t)(effect->offsetY));
#else
		g_objectTable[objectIndex].worldY += effect->offsetY;
#endif

#ifdef XW_MODERN
		g_objectTable[objectIndex].worldZ =
			(int32_t)((uint32_t)g_objectTable[objectIndex].worldZ + (uint32_t)(effect->offsetZ));
#else
		g_objectTable[objectIndex].worldZ += effect->offsetZ;
#endif
		if (isTrench != 0) {

#ifdef XW_MODERN
			g_objectTable[objectIndex].worldX =
				(int32_t)((uint32_t)g_objectTable[objectIndex].worldX + (uint32_t)(centerX));
#else
			g_objectTable[objectIndex].worldX += centerX;
#endif

#ifdef XW_MODERN
			g_objectTable[objectIndex].worldY =
				(int32_t)((uint32_t)g_objectTable[objectIndex].worldY + (uint32_t)(centerY));
#else
			g_objectTable[objectIndex].worldY += centerY;
#endif

#ifdef XW_MODERN
			g_objectTable[objectIndex].worldZ =
				(int32_t)((uint32_t)g_objectTable[objectIndex].worldZ + (uint32_t)(centerZ));
#else
			g_objectTable[objectIndex].worldZ += centerZ;
#endif
		} else {

#ifdef XW_MODERN
			g_objectTable[objectIndex].worldX =
				(int32_t)((uint32_t)g_objectTable[objectIndex].worldX +
						  (uint32_t)(math2_getrandom() & DEATH_STAR_EXPLOSION_XY_MASK));
#else
			g_objectTable[objectIndex].worldX += math2_getrandom() & DEATH_STAR_EXPLOSION_XY_MASK;
#endif

#ifdef XW_MODERN
			g_objectTable[objectIndex].worldY =
				(int32_t)((uint32_t)g_objectTable[objectIndex].worldY +
						  (uint32_t)(math2_getrandom() & DEATH_STAR_EXPLOSION_XY_MASK));
#else
			g_objectTable[objectIndex].worldY += math2_getrandom() & DEATH_STAR_EXPLOSION_XY_MASK;
#endif

#ifdef XW_MODERN
			g_objectTable[objectIndex].worldZ =
				(int32_t)((uint32_t)g_objectTable[objectIndex].worldZ +
						  (uint32_t)(math2_getrandom() & DEATH_STAR_EXPLOSION_Z_MASK));
#else
			g_objectTable[objectIndex].worldZ += math2_getrandom() & DEATH_STAR_EXPLOSION_Z_MASK;
#endif
		}
#ifdef XW_MODERN
		g_objectTable[objectIndex].objectType = XwFlightTypes_ObjectType(
			(math2_getrandom() & DEATH_STAR_EXPLOSION_TYPE_MASK) + XW_OBJ_EXPLOSION_133);
#else
		g_objectTable[objectIndex].objectType =
			(math2_getrandom() & DEATH_STAR_EXPLOSION_TYPE_MASK) + XW_OBJ_EXPLOSION_133;
#endif
		g_objectTable[objectIndex].genusId = XW_GENUS_EXPLOSION_EFFECT;
		g_objectTable[objectIndex].familyId = XW_OBJECT_FAMILY_5;
		g_objectTable[objectIndex].animationState = XW_OBJECT_ANIMATION_BREAKUP;
#ifdef XW_MODERN
		if (XwFlightTypes_Dos())
			g_objectTable[objectIndex].instanceData = NULL;
#endif
		g_objectTable[objectIndex].ageSeconds = 0;
		g_objectTable[objectIndex].lifetimeTicks = 0;
		g_objectTable[objectIndex].billboardScaleCode = DEATH_STAR_EXPLOSION_SCALE;
		g_objectTable[objectIndex].speed = 0;
		g_objectTable[objectIndex].pitch = 0;
		g_objectTable[objectIndex].yaw = 0;
		g_objectTable[objectIndex].roll = 0;
		g_objectTable[objectIndex].orientMatrixDirty = 1;
		g_objectTable[objectIndex].moveVectorDirty = 1;
		for (fragmentIndex = effect->fragmentCount; fragmentIndex-- != 0;) {
			int16_t pitch;
			int16_t randomPitch;
			objectIndex = create_findslot(XW_GENUS_DEBRIS);
			if (objectIndex == XW_OBJECT_SLOT_UNAVAILABLE) {
				break;
			}
			g_objectTable[objectIndex].worldX = worldX;

#ifdef XW_MODERN
			g_objectTable[objectIndex].worldX =
				(int32_t)((uint32_t)g_objectTable[objectIndex].worldX + (uint32_t)(effect->offsetX));
#else
			g_objectTable[objectIndex].worldX += effect->offsetX;
#endif
			g_objectTable[objectIndex].worldY = worldY;

#ifdef XW_MODERN
			g_objectTable[objectIndex].worldY =
				(int32_t)((uint32_t)g_objectTable[objectIndex].worldY + (uint32_t)(effect->offsetY));
#else
			g_objectTable[objectIndex].worldY += effect->offsetY;
#endif
			g_objectTable[objectIndex].worldZ = worldZ;

#ifdef XW_MODERN
			g_objectTable[objectIndex].worldZ =
				(int32_t)((uint32_t)g_objectTable[objectIndex].worldZ + (uint32_t)(effect->offsetZ));
#else
			g_objectTable[objectIndex].worldZ += effect->offsetZ;
#endif
			if (isTrench != 0) {

#ifdef XW_MODERN
				g_objectTable[objectIndex].worldX =
					(int32_t)((uint32_t)g_objectTable[objectIndex].worldX + (uint32_t)(centerX));
#else
				g_objectTable[objectIndex].worldX += centerX;
#endif

#ifdef XW_MODERN
				g_objectTable[objectIndex].worldY =
					(int32_t)((uint32_t)g_objectTable[objectIndex].worldY + (uint32_t)(centerY));
#else
				g_objectTable[objectIndex].worldY += centerY;
#endif

#ifdef XW_MODERN
				g_objectTable[objectIndex].worldZ =
					(int32_t)((uint32_t)g_objectTable[objectIndex].worldZ + (uint32_t)(centerZ));
#else
				g_objectTable[objectIndex].worldZ += centerZ;
#endif
			}
			g_objectTable[objectIndex].prevWorldX = g_objectTable[objectIndex].worldX;
			g_objectTable[objectIndex].prevWorldY = g_objectTable[objectIndex].worldY;
			g_objectTable[objectIndex].prevWorldZ = g_objectTable[objectIndex].worldZ;
#ifdef XW_MODERN
			g_objectTable[objectIndex].objectType = XwFlightTypes_ObjectType(
				(math2_getrandom() & DEATH_STAR_FRAGMENT_TYPE_MASK) + DEATH_STAR_FRAGMENT_FIRST_TYPE);
#else
			g_objectTable[objectIndex].objectType =
				(math2_getrandom() & DEATH_STAR_FRAGMENT_TYPE_MASK) + DEATH_STAR_FRAGMENT_FIRST_TYPE;
#endif
			g_objectTable[objectIndex].sourceObjectRef = DEATH_STAR_FRAGMENT_SOURCE_REF;
			g_objectTable[objectIndex].genusId = XW_GENUS_DEBRIS;
			g_objectTable[objectIndex].familyId = DEATH_STAR_FRAGMENT_FAMILY;
			g_objectTable[objectIndex].billboardScaleCode = DEATH_STAR_FRAGMENT_SCALE;
			g_objectTable[objectIndex].animationState = XW_OBJECT_ANIMATION_BREAKUP;
			g_objectTable[objectIndex].damageAmount = DEATH_STAR_FRAGMENT_DAMAGE;
#ifdef XW_MODERN
			if (XwFlightTypes_Dos())
				g_objectTable[objectIndex].instanceData = NULL;
#endif
			g_objectTable[objectIndex].ageSeconds = 0;
			g_objectTable[objectIndex].lifetimeTicks = 0;
			g_objectTable[objectIndex].speed =
				(math2_getrandom() & DEATH_STAR_FRAGMENT_SPEED_MASK) + DEATH_STAR_FRAGMENT_MIN_SPEED;
			randomPitch = math2_getrandom();
			pitch = (effect->pitchQuarterTurns << DEATH_STAR_QUARTER_TURN_SHIFT) +
					(randomPitch >> DEATH_STAR_FRAGMENT_PITCH_SHIFT);
			if (pitch < 0) {
				pitch = -pitch;
			}
			/* The original signed-word comparison with positive 32768 never clamps this result. */
			g_objectTable[objectIndex].pitch = pitch;
			g_objectTable[objectIndex].yaw = math2_getrandom();
			g_objectTable[objectIndex].roll = 0;
			g_objectTable[objectIndex].orientMatrixDirty = 1;
			g_objectTable[objectIndex].moveVectorDirty = 1;
		}
	}
}

// FUNCTION: XW 0x429AB0
void DeathStar_InitTrenchTexture(void) {
	SceneMesh conversionState;
	OptTextureData* texture;
	int pixelDataBytes;
	uint8_t* sourceShadeTable;
	uint8_t* destShadeTable;
	size_t allocationSize;
	if (g_deathStarTrenchTextureInitialized != 0) {
		return;
	}
	OptModel_SetSourceVertices(g_deathStarTrenchVertices);
	OptModel_SetSourceTexCoords(g_deathStarTrenchTexCoords);
	conversionState.vertexNormals = g_deathStarTrenchVertexNormals;
	OptModel_BuildFaceNormalTangentData(g_deathStarTrenchFaceData.vectors,
										(const OptPackedFaceData*)&g_deathStarTrenchFaceData,
										DEATH_STAR_TRENCH_FACE_COUNT, &conversionState);
	g_loadingModel = 1;
	g_deathStarTrenchTextureInitialized = 1;
	allocationSize = OptModel_GetExternalTextureSerializedSize("TrenchTexture.rgb");
#ifdef XW_MODERN
	allocationSize += sizeof(OptTextureData) - sizeof(OptTextureFileHeader);
#endif
	texture = (OptTextureData*)malloc(allocationSize);
	g_deathStarTrenchTextureNode.param2 = texture;
#ifdef XW_MODERN
	XwPort_LoadNativeOptTexture(texture, "TrenchTexture.rgb");
#else
	OptModel_LoadRgbOrTexFile((uint8_t*)texture, "TrenchTexture.rgb");
#endif
	pixelDataBytes = texture->height * texture->width;
	sourceShadeTable = (uint8_t*)(texture + 1);
	if (pixelDataBytes == texture->textureSize) {
		pixelDataBytes = texture->dataSize;
	}
	sourceShadeTable += pixelDataBytes;
	destShadeTable = sourceShadeTable;
	if (g_flightBytesPerPixel == FLIGHT_DISPLAY_DIRECT_BYTES_PER_PIXEL) {
		sourceShadeTable += OPT_TEXTURE_PALETTE_COLOR_COUNT * texture->paletteType;
	}
	if (g_flightBytesPerPixel == FLIGHT_DISPLAY_INDEXED_BYTES_PER_PIXEL) {
		const uint16_t* rgb565Colors = (const uint16_t*)(sourceShadeTable + OPT_TEXTURE_SHADE_PALETTE_COUNT);
		int shadeIndex;
		for (shadeIndex = 0; shadeIndex < OPT_TEXTURE_SHADE_PALETTE_COUNT; ++shadeIndex) {
			destShadeTable[shadeIndex] = g_rgb565ToPaletteIndexLut[rgb565Colors[shadeIndex]];
		}
	} else {
		texture->paletteType = 0;
		texture->palette = (uint16_t*)destShadeTable;
#ifdef XW_MODERN
		/* The 8192-byte RGB565 table moves down by 4096 bytes, overlapping its source. */
		memmove(destShadeTable, sourceShadeTable, g_flightBytesPerPixel * OPT_TEXTURE_SHADE_PALETTE_COUNT);
#else
		memcpy(destShadeTable, sourceShadeTable, g_flightBytesPerPixel * OPT_TEXTURE_SHADE_PALETTE_COUNT);
#endif
		texture->palette = (uint16_t*)((uint8_t*)texture->palette - OPT_TEXTURE_SHADE_PALETTE_COUNT);
	}
	if (g_flightBytesPerPixel == FLIGHT_DISPLAY_DIRECT_BYTES_PER_PIXEL) {
		OptModel_PrepareTexturePalette((uint16_t*)destShadeTable, OPT_TEXTURE_SHADE_PALETTE_COUNT);
	}
	g_loadingModel = 0;
}

// FUNCTION: XW 0x429BF0
void DeathStar_DrawTrenchDetailMesh(uint8_t objectType, int rootMeshIndex, int worldX, int worldY,
									int worldZ) {
	int savedForcedLodLevel;

	g_deathStarRenderObject.objectType = objectType;
	g_deathStarRenderObject.worldX = worldX;
	g_deathStarRenderObject.worldY = worldY;
	g_deathStarRenderObject.worldZ = worldZ;
	g_billboardObjectOrTypeIndex = DEATH_STAR_DETAIL_RENDER_REF;
	g_deathStarRenderObject.genusId = XW_GENUS_SCENERY;
	g_deathStarRenderObject.roll = 0;
	g_deathStarRenderObject.yaw = 0;
	g_deathStarRenderObject.pitch = TRIG2_QUARTER_TURN;
	g_camRelWorldX = worldX - g_flightCamera.worldPosition.x;
	g_camRelWorldY = worldY - g_flightCamera.worldPosition.y;
	g_camRelWorldZ = worldZ - g_flightCamera.worldPosition.z;
	g_objectViewZ = transfm2_geteyez(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
	g_objectViewX = transfm2_geteyex(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
	g_objectViewY = transfm2_geteyey(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
	g_deathStarRenderObject.orientMatrixDirty = 1;
	fview_newcalcrotate(g_deathStarRenderObject.roll, g_deathStarRenderObject.pitch,
						g_deathStarRenderObject.yaw, 0, &g_deathStarRenderObject);
	savedForcedLodLevel = g_forcedLodLevel;
	g_forcedLodLevel = DEATH_STAR_DETAIL_LOD;
	RenderScene_DrawNoAssetSourceModel(&g_deathStarRenderObject, rootMeshIndex);
	g_forcedLodLevel = savedForcedLodLevel;
}

// FUNCTION: XW 0x429D20
void DeathStar_DrawTrench(void) {
	float savedMipScale = g_textureMipScale;
	OptimizedPolyObject* modelHeader;
	OptimizedPolyObject savedHeader;
	int altitude, tileRadius, tileIndex;
	int cameraRelativeX, cameraRelativeZ;
	uint16_t rowMask;
	int level;
	g_textureMipScale = 0.0f;
	DeathStar_InitTrenchTexture();
	modelHeader = Memory_LockHandle(g_loadedModels[DEATH_STAR_TRENCH_MODEL]);
	memcpy(&savedHeader, modelHeader, sizeof(savedHeader));
	g_deathStarTrenchModelHeader.selfMarker = modelHeader;
	memcpy(modelHeader, &g_deathStarTrenchModelHeader, sizeof(*modelHeader));
	g_billboardObjectOrTypeIndex = DEATH_STAR_DETAIL_RENDER_REF;
	g_deathStarRenderObject.worldY = g_flightCamera.worldPosition.y & DEATH_STAR_CELL_WORLD_MASK;
	g_deathStarRenderObject.worldX = 0;
	g_deathStarRenderObject.worldZ = 0;
	g_deathStarRenderObject.objectType = DEATH_STAR_TRENCH_MODEL;
	g_deathStarRenderObject.genusId = XW_GENUS_SCENERY;
	g_deathStarRenderObject.roll = 0;
	g_deathStarRenderObject.yaw = 0;
	g_deathStarRenderObject.pitch = TRIG2_QUARTER_TURN;
	g_camRelWorldX = (int32_t)(0u - (uint32_t)g_flightCamera.worldPosition.x);
	g_camRelWorldY =
		(int32_t)((uint32_t)g_deathStarRenderObject.worldY - (uint32_t)g_flightCamera.worldPosition.y);
	g_camRelWorldZ = (int32_t)(0u - (uint32_t)g_flightCamera.worldPosition.z);
	g_objectViewZ = transfm2_geteyez(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
	g_objectViewX = transfm2_geteyex(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
	g_objectViewY = transfm2_geteyey(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
	g_deathStarRenderObject.orientMatrixDirty = 1;
	fview_newcalcrotate(g_deathStarRenderObject.roll, g_deathStarRenderObject.pitch,
						g_deathStarRenderObject.yaw, 0, &g_deathStarRenderObject);
	altitude = g_flightCamera.worldPosition.z;
	for (tileRadius = 1; altitude > DEATH_STAR_TRENCH_TILE_ALTITUDE; ++tileRadius)
		altitude >>= 1;
	g_deathStarRenderObject.worldY = (int32_t)((uint32_t)g_deathStarRenderObject.worldY +
											   (uint32_t)(-1 - tileRadius) * DEATH_STAR_TRENCH_TILE_SIZE);
	for (tileIndex = -tileRadius; tileIndex <= tileRadius; ++tileIndex) {
		int tileViewZ, tileViewY;
		g_camRelWorldX =
			(int32_t)((uint32_t)g_deathStarRenderObject.worldX - (uint32_t)g_flightCamera.worldPosition.x);
		g_camRelWorldY =
			(int32_t)((uint32_t)g_deathStarRenderObject.worldY - (uint32_t)g_flightCamera.worldPosition.y);
		g_camRelWorldZ =
			(int32_t)((uint32_t)g_deathStarRenderObject.worldZ - (uint32_t)g_flightCamera.worldPosition.z);
		g_deathStarRenderObject.worldY =
			(int32_t)((uint32_t)g_deathStarRenderObject.worldY + DEATH_STAR_TRENCH_TILE_SIZE);
		g_objectViewZ = transfm2_geteyez(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
		tileViewZ = g_objectViewZ;
		g_objectViewX = transfm2_geteyex(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
		tileViewY = transfm2_geteyey(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
		tileViewZ = (int32_t)((uint32_t)tileViewZ + DEATH_STAR_TRENCH_TILE_DEPTH_MARGIN);
		g_objectViewY = tileViewY;
		if (tileViewZ >= 0 && g_objectViewX <= tileViewZ &&
			(int32_t)(0u - (uint32_t)g_objectViewX) <= tileViewZ && tileViewY <= tileViewZ &&
			(int32_t)(0u - (uint32_t)tileViewY) <= tileViewZ)
			RenderScene_DrawObjectModel(&g_deathStarRenderObject);
	}
	memcpy(modelHeader, &savedHeader, sizeof(*modelHeader));
	g_textureMipScale = savedMipScale;
	cameraRelativeZ = (int32_t)(0u - (uint32_t)g_flightCamera.worldPosition.z);
	cameraRelativeX = (int32_t)(0u - (uint32_t)g_flightCamera.worldPosition.x);
	g_billboardObjectOrTypeIndex = DEATH_STAR_TRENCH_DETAIL_RENDER_REF;
	g_deathStarCellViewSteps.stepYViewX = (int16_t)g_camMatR0_Y * DEATH_STAR_TRENCH_STEP_SCALE;
	g_deathStarCellViewSteps.stepYViewY = (int16_t)g_camMatR1_Y * DEATH_STAR_TRENCH_STEP_SCALE;
	g_deathStarCellViewSteps.stepYViewZ = (int16_t)g_camMatR2_Y * DEATH_STAR_TRENCH_STEP_SCALE;
	transfm2_geteyex(cameraRelativeX, 0, cameraRelativeZ);
	transfm2_geteyey(cameraRelativeX, 0, cameraRelativeZ);
	transfm2_geteyez(cameraRelativeX, 0, cameraRelativeZ);
	g_deathStarCellViewSteps.stepYViewZ >>= DEATH_STAR_TRENCH_INITIAL_STEP_SHIFT;
	g_deathStarCellViewSteps.stepYViewX >>= DEATH_STAR_TRENCH_INITIAL_STEP_SHIFT;
	g_deathStarCellViewSteps.stepYViewY >>= DEATH_STAR_TRENCH_INITIAL_STEP_SHIFT;
	rowMask = DEATH_STAR_TRENCH_INITIAL_ROW_MASK;
	if (g_flightCamera.worldPosition.x >= -DEATH_STAR_TRENCH_DETAIL_HALF_WIDTH &&
		g_flightCamera.worldPosition.x <= DEATH_STAR_TRENCH_DETAIL_HALF_WIDTH &&
		g_flightCamera.worldPosition.z <= DEATH_STAR_TRENCH_DETAIL_MAX_HEIGHT) {
		for (level = DEATH_STAR_TRENCH_TRAVERSAL_LEVELS; level > 0; --level) {
			uint16_t stepCells = rowMask;
			uint32_t rowStepWorldY, cellWorldY;
			int row;
			if ((int16_t)g_camMatR2_Y >= 0)
				stepCells = -rowMask;
			rowStepWorldY = (uint32_t)stepCells << DEATH_STAR_CELL_COORDINATE_SHIFT;
			cellWorldY =
				(uint16_t)(g_flightCamera.worldPosition.y >> DEATH_STAR_CELL_COORDINATE_SHIFT) & rowMask;
			cellWorldY <<= DEATH_STAR_CELL_COORDINATE_SHIFT;
			rowMask = (int16_t)rowMask >> 1;
			for (row = DEATH_STAR_TRENCH_ROWS_PER_LEVEL; row > 0; --row) {
				int cameraRelativeY = (int32_t)(cellWorldY - (uint32_t)g_flightCamera.worldPosition.y);
				int cellViewX = transfm2_geteyex(cameraRelativeX, cameraRelativeY, cameraRelativeZ);
				int cellViewY = transfm2_geteyey(cameraRelativeX, cameraRelativeY, cameraRelativeZ);
				int cellViewZ = transfm2_geteyez(cameraRelativeX, cameraRelativeY, cameraRelativeZ);
				if (level == 1)
					DeathStar_DrawTrenchCellDetails(cellViewX, cellViewY, cellViewZ, cellWorldY);
				cellWorldY += rowStepWorldY;
			}
			g_deathStarCellViewSteps.stepYViewX >>= 1;
			g_deathStarCellViewSteps.stepYViewY >>= 1;
			g_deathStarCellViewSteps.stepYViewZ >>= 1;
		}
	}
}

// FUNCTION: XW 0x42A1B0
void DeathStar_DrawTrenchCellDetails(int cellViewX, int cellViewY, int cellViewZ, unsigned int cellWorldY) {
	uint16_t layoutIndex, cellKey, hashStart, hashIndex, placementCount;
	int16_t hasDamageState;
	int16_t specialEntryPending;
	const XwSurfacePlacementList* placementList;
	uint16_t placementIndex;
#ifdef XW_MODERN
	specialEntryPending = 0;
#endif
	if (g_flightCamera.worldPosition.z > DEATH_STAR_CELL_MIDPOINT)
		return;
	cellViewX -= g_cameraWorldZToViewXSteps[DEATH_STAR_TRENCH_FLOOR_STEP_INDEX];
	cellViewY -= g_cameraWorldZToViewYSteps[DEATH_STAR_TRENCH_FLOOR_STEP_INDEX];
	cellViewZ -= g_cameraWorldZToViewZSteps[DEATH_STAR_TRENCH_FLOOR_STEP_INDEX];
	layoutIndex = (cellWorldY >> DEATH_STAR_CELL_COORDINATE_SHIFT) & DEATH_STAR_CELL_HASH_AXIS_MASK;
	cellKey =
		layoutIndex | ((((cellWorldY >> DEATH_STAR_CELL_COORDINATE_SHIFT) & DEATH_STAR_CELL_KEY_Y_MASK) |
						DEATH_STAR_TRENCH_KEY_FLAG)
					   << 8);
	if (cellWorldY == 0)
		layoutIndex = DEATH_STAR_TRENCH_END_LAYOUT;
	hashStart = (cellWorldY >> DEATH_STAR_CELL_COORDINATE_SHIFT) & (DEATH_STAR_SURFACE_CELL_COUNT - 1);
	hasDamageState = 1;
	for (hashIndex = hashStart;;) {
		if (g_surfaceCellDamageStates[hashIndex].cellKey == 0) {
			hasDamageState = 0;
			break;
		}
		if (g_surfaceCellDamageStates[hashIndex].cellKey == cellKey)
			break;
		++hashIndex;
		if (hashIndex == DEATH_STAR_SURFACE_CELL_COUNT)
			hashIndex = 0;
		if (hashIndex == hashStart) {
			hasDamageState = 0;
			break;
		}
	}
	++g_billboardObjectOrTypeIndex;
	placementList = g_trenchPlacementLists[layoutIndex];
	placementCount = placementList->count;
	if (placementCount >= g_trenchObjectDetailLimit)
		placementCount = g_trenchObjectDetailLimit;
	if (placementCount <= DEATH_STAR_SHORT_LAYOUT_OBJECT_COUNT)
		placementCount = DEATH_STAR_SHORT_LAYOUT_OBJECT_COUNT;
	if (layoutIndex == DEATH_STAR_TRENCH_END_LAYOUT) {
		placementCount = DEATH_STAR_TRENCH_END_GUN_COUNT;
		specialEntryPending = 1;
	}
	for (placementIndex = 0; placementCount-- != 0; ++placementIndex) {
		uint8_t packedPosition, objectType, side, yNibble, heightBand;
		int viewX, viewY, viewZ, expandedDepth;
		uint16_t rootMeshIndex, savedDetailValue;
#ifdef XW_MODERN
		if (placementIndex < sizeof(g_surfaceCellDamageStates[hashIndex].objectHealthOrEffectState) &&
#else
		if (placementIndex < DEATH_STAR_TRENCH_DAMAGE_READ_COUNT &&
#endif
			hasDamageState != 0 &&
			g_surfaceCellDamageStates[hashIndex].objectHealthOrEffectState[placementIndex] == 0)
			continue;
		packedPosition = placementList->placements[placementIndex].packedPosition;
		objectType = placementList->placements[placementIndex].objectType;
		side = packedPosition & DEATH_STAR_TRENCH_SIDE_MASK;
		yNibble = (packedPosition >> DEATH_STAR_TRENCH_LONGITUDINAL_SHIFT) & DEATH_STAR_PLACEMENT_NIBBLE_MASK;
		heightBand = (packedPosition >> DEATH_STAR_TRENCH_HEIGHT_SHIFT) * 2;
		if (side == 0)
			g_camRelWorldX = DEATH_STAR_TRENCH_MOUNT_X - g_flightCamera.worldPosition.x;
		else if (side == 1)
			g_camRelWorldX = -DEATH_STAR_TRENCH_MOUNT_X - g_flightCamera.worldPosition.x;
		else
			g_camRelWorldX = -g_flightCamera.worldPosition.x;
		g_camRelWorldY = (int32_t)((cellWorldY & DEATH_STAR_CELL_WORLD_MASK) |
								   ((unsigned int)yNibble << DEATH_STAR_PLACEMENT_STEP_SHIFT)) -
						 g_flightCamera.worldPosition.y;
		g_camRelWorldZ = DEATH_STAR_TRENCH_FLOOR + heightBand * (DEATH_STAR_TRENCH_HEIGHT_STEP / 2) -
						 g_flightCamera.worldPosition.z;
		if (yNibble == 0) {
			viewX = 0;
			viewY = 0;
			viewZ = 0;
		} else {
			viewX = g_cameraWorldYToViewXSteps[yNibble - 1] * 4;
			viewY = g_cameraWorldYToViewYSteps[yNibble - 1] * 4;
			viewZ = g_cameraWorldYToViewZSteps[yNibble - 1] * 4;
		}
		if (heightBand != 0) {
			viewX += g_cameraWorldZToViewXSteps[heightBand - 1] >> 1;
			viewY += g_cameraWorldZToViewYSteps[heightBand - 1] >> 1;
			viewZ += g_cameraWorldZToViewZSteps[heightBand - 1] >> 1;
		}
		viewX += cellViewX;
		viewY += cellViewY;
		viewZ += cellViewZ;
		if (side == 0) {
			viewX += g_cameraWorldXToViewXSteps[DEATH_STAR_TRENCH_MOUNT_STEP_INDEX];
			viewY += g_cameraWorldXToViewYSteps[DEATH_STAR_TRENCH_MOUNT_STEP_INDEX];
			viewZ += g_cameraWorldXToViewZSteps[DEATH_STAR_TRENCH_MOUNT_STEP_INDEX];
		} else if (side == 1) {
			viewX -= g_cameraWorldXToViewXSteps[DEATH_STAR_TRENCH_MOUNT_STEP_INDEX];
			viewY -= g_cameraWorldXToViewYSteps[DEATH_STAR_TRENCH_MOUNT_STEP_INDEX];
			viewZ -= g_cameraWorldXToViewZSteps[DEATH_STAR_TRENCH_MOUNT_STEP_INDEX];
		}
		if (specialEntryPending != 0) {
			if (viewZ > DEATH_STAR_TRENCH_END_DRAW_DISTANCE)
				continue;
		} else if (viewZ > (g_flightGraphicsDetailPreset + DEATH_STAR_TRENCH_DETAIL_DISTANCE_BASE)
							   << DEATH_STAR_TRENCH_DETAIL_DISTANCE_SHIFT) {
			continue;
		}
		expandedDepth = viewZ + g_modelTypeTable[objectType].maxBoundsExtent;
		if (expandedDepth < 0)
			continue;
		if (viewX < 0) {
			if (-viewX > expandedDepth)
				continue;
		} else if (viewX > expandedDepth) {
			continue;
		}
		if (viewY < 0) {
			if (-viewY > expandedDepth)
				continue;
		} else if (viewY > expandedDepth) {
			continue;
		}
		rootMeshIndex = side == 0;
		if (heightBand != 0)
			rootMeshIndex += 2;
		if (objectType == DEATH_STAR_TRENCH_SPECIAL_MESH_TYPE) {
			if (side == 2)
				rootMeshIndex = 2;
			else if (side == 1)
				rootMeshIndex = 0;
		}
		if (rootMeshIndex >= ModelMesh_GetCachedObjectTypeMeshCount(objectType)) {
			if (rootMeshIndex == 3) {
				rootMeshIndex = 1;
				if (ModelMesh_GetCachedObjectTypeMeshCount(objectType) <= 1)
					rootMeshIndex = 0;
			} else {
				rootMeshIndex = 0;
			}
		}
		if (specialEntryPending != 0) {
			savedDetailValue = g_shipDetailPolyCount;
			g_shipDetailPolyCount = DEATH_STAR_TRENCH_END_DETAIL;
		}
		DeathStar_DrawTrenchDetailMesh(
			objectType, rootMeshIndex, g_flightCamera.worldPosition.x + g_camRelWorldX,
			g_flightCamera.worldPosition.y + g_camRelWorldY, g_flightCamera.worldPosition.z + g_camRelWorldZ);
		++g_billboardObjectOrTypeIndex;
		if (specialEntryPending != 0) {
			specialEntryPending = 0;
			g_shipDetailPolyCount = savedDetailValue;
		}
	}
}
