#ifndef XW_FLIGHT_DEATH_STAR_H
#define XW_FLIGHT_DEATH_STAR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/assets/opt_model.h"

#include <stddef.h>
#include <stdint.h>

enum {
	DEATH_STAR_SURFACE_TILE_HALF_WIDTH = 1048576,
	DEATH_STAR_TRENCH_OPENING_HALF_WIDTH = 3072,
	DEATH_STAR_SURFACE_DETAIL_ALTITUDE = 16384,
	DEATH_STAR_SURFACE_STEP_SHIFT = 5,
	DEATH_STAR_SURFACE_HALF_STEP_SHIFT = 6,
	DEATH_STAR_DETAIL_PRESET_COUNT = 4,
	DEATH_STAR_SURFACE_CELL_CULL_MARGIN = 73728,
	DEATH_STAR_SURFACE_BASE_MODEL = 182,
	DEATH_STAR_SURFACE_BASE_REFERENCE_OFFSET = 22528,
	DEATH_STAR_SURFACE_BASE_STEP_INDEX = 3,
	DEATH_STAR_SURFACE_LARGE_TYPE_FIRST = 167,
	DEATH_STAR_SURFACE_LARGE_TYPE_LAST = 172,
	DEATH_STAR_PLACEMENT_NIBBLE_BITS = 4,
	DEATH_STAR_PLACEMENT_VIEW_SCALE = 4
};

extern const float g_surfaceSpecialDetailMinProjectedSize;
extern const float g_surfaceDetailMinProjectedSize;
extern float g_deathStarDetailScreenSizeScale[DEATH_STAR_DETAIL_PRESET_COUNT];

enum {
	DEATH_STAR_TRENCH_FLOOR_STEP_INDEX = 5,
	DEATH_STAR_TRENCH_MOUNT_STEP_INDEX = 2,
	DEATH_STAR_TRENCH_DAMAGE_READ_COUNT = 19,
	DEATH_STAR_TRENCH_END_DRAW_DISTANCE = 98304,
	DEATH_STAR_TRENCH_DETAIL_DISTANCE_BASE = 3,
	DEATH_STAR_TRENCH_DETAIL_DISTANCE_SHIFT = 14,
	DEATH_STAR_TRENCH_END_DETAIL = 50,
	DEATH_STAR_CELL_WORLD_MASK = -65536
};

enum {
	DEATH_STAR_TRENCH_MODEL = 143,
	DEATH_STAR_TRENCH_TILE_ALTITUDE = 65536,
	DEATH_STAR_TRENCH_TILE_SIZE = 0x200000,
	DEATH_STAR_TRENCH_TILE_DEPTH_MARGIN = 0x400000,
	DEATH_STAR_TRENCH_DETAIL_RENDER_REF = 0x1000,
	DEATH_STAR_TRENCH_STEP_SCALE = 64,
	DEATH_STAR_TRENCH_INITIAL_STEP_SHIFT = 3,
	DEATH_STAR_TRENCH_INITIAL_ROW_MASK = 0xFFF8,
	DEATH_STAR_TRENCH_DETAIL_HALF_WIDTH = 32768,
	DEATH_STAR_TRENCH_DETAIL_MAX_HEIGHT = 131072,
	DEATH_STAR_TRENCH_TRAVERSAL_LEVELS = 4,
	DEATH_STAR_TRENCH_ROWS_PER_LEVEL = 4
};

enum { DEATH_STAR_DETAIL_RENDER_REF = 0x2000, DEATH_STAR_DETAIL_LOD = 1 };

enum { DEATH_STAR_SURFACE_VERTEX_COUNT = 4, DEATH_STAR_SURFACE_FACE_COUNT = 1 };

extern OptVector g_deathStarSurfaceVertices[DEATH_STAR_SURFACE_VERTEX_COUNT];
extern OptTexCoord g_deathStarSurfaceTexCoords[DEATH_STAR_SURFACE_VERTEX_COUNT];
extern OptVector g_deathStarSurfaceVertexNormals[DEATH_STAR_SURFACE_FACE_COUNT];

typedef struct XwSurfaceFaceData {
	int edgeCount;
	OptPackedFaceRecord records[DEATH_STAR_SURFACE_FACE_COUNT];
	float vectors[DEATH_STAR_SURFACE_FACE_COUNT * (OPT_VECTOR_COMPONENT_COUNT + OPT_TANGENT_COMPONENT_COUNT)];
} XwSurfaceFaceData;

extern XwSurfaceFaceData g_deathStarSurfaceFaceData;
extern OptNode g_deathStarSurfaceVertexNode;
extern float g_deathStarSurfaceUvPerWorldUnit;
extern OptNode g_deathStarSurfaceTexcoordNode;
extern OptNode g_deathStarSurfaceNormalNode;
extern OptNode g_deathStarSurfaceFaceNode;
extern OptNode g_deathStarSurfaceTextureNode;
extern OptNode* g_deathStarSurfaceChildren[5];
extern OptNode g_deathStarSurfaceRootNode;
extern OptNode* g_deathStarSurfaceRoots[1];
extern OptimizedPolyObject g_deathStarSurfaceModelHeader;
extern int g_deathStarSurfaceTextureInitialized;

enum {
	DEATH_STAR_TRENCH_VERTEX_COUNT = 12,
	DEATH_STAR_TRENCH_TEXCOORD_COUNT = 8,
	DEATH_STAR_TRENCH_FACE_COUNT = 3
};

extern OptVector g_deathStarTrenchVertices[DEATH_STAR_TRENCH_VERTEX_COUNT];
extern OptTexCoord g_deathStarTrenchTexCoords[DEATH_STAR_TRENCH_TEXCOORD_COUNT];
extern OptVector g_deathStarTrenchVertexNormals[DEATH_STAR_TRENCH_FACE_COUNT];

typedef struct XwTrenchFaceData {
	int edgeCount;
	OptPackedFaceRecord records[DEATH_STAR_TRENCH_FACE_COUNT];
	float vectors[DEATH_STAR_TRENCH_FACE_COUNT * (OPT_VECTOR_COMPONENT_COUNT + OPT_TANGENT_COMPONENT_COUNT)];
} XwTrenchFaceData;

extern XwTrenchFaceData g_deathStarTrenchFaceData;
extern OptNode g_deathStarTrenchVertexNode;
extern OptNode g_deathStarTrenchTexcoordNode;
extern OptNode g_deathStarTrenchNormalNode;
extern OptNode g_deathStarTrenchFaceNode;
extern OptNode g_deathStarTrenchTextureNode;
extern OptNode* g_deathStarTrenchChildren[5];
extern OptNode g_deathStarTrenchRootNode;
extern OptNode* g_deathStarTrenchRoots[1];
extern OptimizedPolyObject g_deathStarTrenchModelHeader;

extern int g_deathStarTrenchTextureInitialized;

enum {
	DEATH_STAR_DESTRUCTION_PATTERN_COUNT = 10,
	DEATH_STAR_SURFACE_LAYOUT_COUNT = 5,
	DEATH_STAR_TRENCH_LAYOUT_COUNT = 9,
	DEATH_STAR_SHORT_LAYOUT_OBJECT_COUNT = 8,
	DEATH_STAR_EXPLOSION_XY_MASK = 0x1FF,
	DEATH_STAR_EXPLOSION_Z_MASK = 0x7F,
	DEATH_STAR_EXPLOSION_TYPE_MASK = 1,
	DEATH_STAR_EXPLOSION_SCALE = 12,
	DEATH_STAR_FRAGMENT_FIRST_TYPE = 110,
	DEATH_STAR_FRAGMENT_TYPE_MASK = 3,
	DEATH_STAR_FRAGMENT_SOURCE_REF = 0x2000,
	DEATH_STAR_FRAGMENT_FAMILY = 3,
	DEATH_STAR_FRAGMENT_SCALE = 64,
	DEATH_STAR_FRAGMENT_DAMAGE = 250,
	DEATH_STAR_FRAGMENT_SPEED_MASK = 63,
	DEATH_STAR_FRAGMENT_MIN_SPEED = 63,
	DEATH_STAR_FRAGMENT_PITCH_SHIFT = 2,
	DEATH_STAR_QUARTER_TURN_SHIFT = 14,
	DEATH_STAR_SURFACE_CELL_COUNT = 64,
	DEATH_STAR_SURFACE_GOAL_COUNT = 4,
	DEATH_STAR_SURFACE_OBJECT_COUNT = 14,
	DEATH_STAR_DESTRUCTION_STATE_FIRST = 250
};

enum {
	DEATH_STAR_SURFACE_GUN_CELL_CAPACITY = 32,
	DEATH_STAR_TRENCH_GUN_CELL_X = -32768,
	DEATH_STAR_TRENCH_GUN_INITIAL_TICKS = 236,
	DEATH_STAR_SURFACE_GUN_INITIAL_TICKS = 708,
	DEATH_STAR_SURFACE_GUN_REFRESH_TICKS = 944,
	DEATH_STAR_SURFACE_GUN_MAX_HEIGHT = 0x10000,
	DEATH_STAR_CELL_COORDINATE_SHIFT = 16,
	DEATH_STAR_CELL_MIDPOINT = 0x8000
};

enum {
	DEATH_STAR_CELL_HASH_AXIS_MASK = 7,
	DEATH_STAR_CELL_HASH_AXIS_BITS = 3,
	DEATH_STAR_CELL_KEY_X_MASK = 0xF8,
	DEATH_STAR_CELL_KEY_Y_MASK = 0xFFF8,
	DEATH_STAR_CELL_KEY_Y_SHIFT = 5,
	DEATH_STAR_TRENCH_KEY_FLAG = 4,
	DEATH_STAR_TRENCH_END_LAYOUT = 8,
	DEATH_STAR_SURFACE_EDGE_LAYOUT = 4,
	DEATH_STAR_SURFACE_LOW_LAYOUT = 1,
	DEATH_STAR_TRENCH_GUN_COUNT = 4,
	DEATH_STAR_TRENCH_END_GUN_COUNT = 13,
	DEATH_STAR_SURFACE_GUN_COUNT = 8,
	DEATH_STAR_PLACEMENT_NIBBLE_MASK = 15,
	DEATH_STAR_PLACEMENT_STEP_SHIFT = 12,
	DEATH_STAR_TRENCH_SIDE_MASK = 3,
	DEATH_STAR_TRENCH_SPECIAL_MESH_TYPE = 185,
	DEATH_STAR_MIDPOINT_TYPE_FIRST = 198,
	DEATH_STAR_MIDPOINT_TYPE_LAST = 199,
	DEATH_STAR_DAMAGE_SHIFT = 6,
	DEATH_STAR_SPECIAL_CELL_VOICE = 68,
	DEATH_STAR_DIRECT_HIT_VOICE = 61,
	DEATH_STAR_VICTORY_DELAY_SECONDS = 4,
	DEATH_STAR_VICTORY_GOAL_TICKS = 708,
	DEATH_STAR_TRENCH_LONGITUDINAL_SHIFT = 2,
	DEATH_STAR_TRENCH_HEIGHT_SHIFT = 6,
	DEATH_STAR_TRENCH_HEIGHT_STEP = 1024,
	DEATH_STAR_TRENCH_FLOOR = -6144,
	DEATH_STAR_TRENCH_MOUNT_X = 3072,
	DEATH_STAR_SURFACE_BASE_HEIGHT = 4096,
	DEATH_STAR_SURFACE_WALL_MARGIN = 1768,
	DEATH_STAR_GUN_SMALL_HEIGHT = 2048,
	DEATH_STAR_GUN_LARGE_HEIGHT = 4096,
	DEATH_STAR_GUN_201_HEIGHT = 4736,
	DEATH_STAR_GUN_201_MUZZLE_X = 1424,
	DEATH_STAR_GUN_195_HEIGHT = 1500,
	DEATH_STAR_GUN_LOWER_HEIGHT = 1000,
	DEATH_STAR_GUN_LOWER_MUZZLE_X = 1750,
	DEATH_STAR_GUN_UPPER_OFFSET = 500,
	DEATH_STAR_GUN_YAW_LIMIT = 0x2000,
	DEATH_STAR_GUN_YAW_RIGHT = 0x4000,
	DEATH_STAR_GUN_YAW_LEFT = -16384,
	DEATH_STAR_GUN_YAW_BACK = -32768,
	DEATH_STAR_GUN_FULL_ACCURACY = 0xFFFF,
	DEATH_STAR_GUN_PITCH_MIN = 0x1000,
	DEATH_STAR_GUN_PITCH_MAX = 0x5000,
	DEATH_STAR_GUN_SURFACE_LEAD_SHIFT = 14,
	DEATH_STAR_GUN_TRENCH_LEAD_SHIFT = 15,
	DEATH_STAR_GUN_LEAD_RANDOM_MASK = 3,
	DEATH_STAR_GUN_SPEED_LOW = 1000,
	DEATH_STAR_GUN_SPEED_HIGH = 1128,
	DEATH_STAR_GUN_SPEED_ACCURACY_BASE = 0xE7FF,
	DEATH_STAR_GUN_SPEED_ACCURACY_SHIFT = 8,
	DEATH_STAR_GUN_SCATTER_BIAS = 256,
	DEATH_STAR_GUN_SCATTER_MASK = 1023,
	DEATH_STAR_GUN_PROJECTILE_FAMILY = 1,
	DEATH_STAR_GUN_PROJECTILE_IFF = 1,
	DEATH_STAR_GUN_SOURCE_REF = 0x2000,
	DEATH_STAR_GUN_EDGE_DAMAGE_SCALE = 4,
	DEATH_STAR_GUN_167 = 167,
	DEATH_STAR_GUN_168 = 168,
	DEATH_STAR_GUN_169 = 169,
	DEATH_STAR_GUN_170 = 170,
	DEATH_STAR_GUN_171 = 171,
	DEATH_STAR_GUN_172 = 172,
	DEATH_STAR_GUN_193 = 193,
	DEATH_STAR_GUN_194 = 194,
	DEATH_STAR_GUN_195 = 195,
	DEATH_STAR_GUN_201 = 201
};

typedef struct XwBounds16 XwBounds16;
typedef struct XwDeathStarCellViewSteps XwDeathStarCellViewSteps;
extern XwDeathStarCellViewSteps g_deathStarCellViewSteps;
typedef struct XwSurfaceCellDamageState XwSurfaceCellDamageState;
typedef struct XwSurfaceDestructionEffect XwSurfaceDestructionEffect;
typedef struct XwSurfaceDestructionPattern XwSurfaceDestructionPattern;
typedef struct XwSurfaceGunCell XwSurfaceGunCell;
struct ObjectRecord;
struct CraftData;
typedef struct XwSurfaceHealthList XwSurfaceHealthList;
typedef struct XwSurfaceObjectPlacement XwSurfaceObjectPlacement;
typedef struct XwSurfacePlacementList XwSurfacePlacementList;

/* Original IDB size: 12 bytes. */
struct XwBounds16 {
	/* IDB +0x0 */
	int16_t minX;
	/* IDB +0x2 */
	int16_t minY;
	/* IDB +0x4 */
	int16_t minZ;
	/* IDB +0x6 */
	int16_t maxX;
	/* IDB +0x8 */
	int16_t maxY;
	/* IDB +0xA */
	int16_t maxZ;
};

/* Original IDB size: 48 bytes. */
struct XwDeathStarCellViewSteps {
	/* IDB +0x0: Projected 32768-unit half-cell increment in surface rendering. */
	int halfXViewY;
	/* IDB +0x4: Projected 32768-unit half-cell increment in surface rendering. */
	int halfXViewX;
	/* IDB +0x8: Projected 32768-unit half-cell increment in surface rendering. */
	int halfXViewZ;
	/* IDB +0xC: Projected 65536-unit cell increment in surface rendering. Trench traversal reuses this field
	 * for progressively halved Y steps. */
	int stepYViewZ;
	/* IDB +0x10: Projected 65536-unit cell increment in surface rendering. Trench traversal reuses this field
	 * for progressively halved Y steps. */
	int stepYViewX;
	/* IDB +0x14: Projected 65536-unit cell increment in surface rendering. Trench traversal reuses this field
	 * for progressively halved Y steps. */
	int stepYViewY;
	/* IDB +0x18: Projected 65536-unit cell increment in surface rendering. */
	int stepXViewY;
	/* IDB +0x1C: Projected 65536-unit cell increment in surface rendering. */
	int stepXViewX;
	/* IDB +0x20: Projected 65536-unit cell increment in surface rendering. */
	int stepXViewZ;
	/* IDB +0x24: Projected 32768-unit half-cell increment in surface rendering. */
	int halfYViewZ;
	/* IDB +0x28: Projected 32768-unit half-cell increment in surface rendering. */
	int halfYViewY;
	/* IDB +0x2C: Projected 32768-unit half-cell increment in surface rendering. */
	int halfYViewX;
};

/* Original IDB size: 16 bytes. */
struct XwSurfaceCellDamageState {
	/* IDB +0x0: Packed surface cell key; zero marks an unused record. Lookups begin at a coordinate-derived
	 * slot and linearly probe with wrap at 64. */
	uint16_t cellKey;
	/* IDB +0x2: Per-object health/state. Damage subtracts projectile damageAmount >> 6 with saturation;
	 * destruction starts at 250, animation increments 250..255 to zero. Zero is absent/destroyed. Cell
	 * templates supply initial health. */
	uint8_t objectHealthOrEffectState[DEATH_STAR_SURFACE_OBJECT_COUNT];
};

/* Original IDB size: 8 bytes. */
struct XwSurfaceDestructionEffect {
	/* IDB +0x0: Signed X offset from the placement origin (plus bounds center in trench mode). */
	int16_t offsetX;
	/* IDB +0x2: Signed Y offset from the placement origin (plus bounds center in trench mode). */
	int16_t offsetY;
	/* IDB +0x4: Signed Z offset from the placement origin (plus bounds center in trench mode). */
	int16_t offsetZ;
	/* IDB +0x6: Number of genus-10 fragments requested at this effect offset. */
	uint8_t fragmentCount;
	/* IDB +0x7: Added as value<<14 to the signed random pitch, then reflected as a signed word.
	 * The original clamp compares a sign-extended word with positive 32768 and cannot trigger. */
	uint8_t pitchQuarterTurns;
};

/* Original IDB size: 6 bytes. */
struct XwSurfaceGunCell {
	/* IDB +0x0: World X high word for surface cells; 0x8000 identifies the additional trench-cell entry
	 * generated when cellX is zero. */
	int16_t cellX;
	/* IDB +0x2: World Y high word identifying the cell. */
	int16_t cellY;
	/* IDB +0x4: Countdown decremented by g_elapsedTicks. Initial values: 708 surface, 236 trench. */
	uint16_t fireCountdownTicks;
};

/* Original IDB size: 1 bytes. */
struct XwSurfaceHealthList {
	/* IDB +0x0: Number of initial health bytes copied verbatim when creating a cell-damage hash record. */
	uint8_t count;
	/* IDB +0x1: Initial health by placement index; damage is scaled by >>6 before subtraction. */
	uint8_t health[];
};

typedef struct XwSurfaceHealthList8 {
	uint8_t count;
	uint8_t health[8];
} XwSurfaceHealthList8;

typedef struct XwSurfaceHealthList13 {
	uint8_t count;
	uint8_t health[13];
} XwSurfaceHealthList13;

typedef struct XwSurfaceHealthList14 {
	uint8_t count;
	uint8_t health[14];
} XwSurfaceHealthList14;

typedef struct XwSurfaceHealthList17 {
	uint8_t count;
	uint8_t health[17];
} XwSurfaceHealthList17;

typedef struct XwSurfaceHealthList18 {
	uint8_t count;
	uint8_t health[18];
} XwSurfaceHealthList18;

typedef struct XwSurfaceHealthList19 {
	uint8_t count;
	uint8_t health[19];
} XwSurfaceHealthList19;

extern const XwSurfaceHealthList14 g_surfaceHealth2;
extern const XwSurfaceHealthList14 g_surfaceHealth0;
extern const XwSurfaceHealthList14 g_surfaceHealth3;
extern const XwSurfaceHealthList14 g_surfaceHealth1;
extern const XwSurfaceHealthList8 g_surfaceHealth4;
extern const XwSurfaceHealthList17 g_trenchHealth0;
extern const XwSurfaceHealthList17 g_trenchHealth1;
extern const XwSurfaceHealthList19 g_trenchHealth2;
extern const XwSurfaceHealthList19 g_trenchHealth3;
extern const XwSurfaceHealthList18 g_trenchHealth4;
extern const XwSurfaceHealthList19 g_trenchHealth5;
extern const XwSurfaceHealthList19 g_trenchHealth6;
extern const XwSurfaceHealthList17 g_trenchHealth7;
extern const XwSurfaceHealthList13 g_trenchHealth8;
extern const XwSurfaceHealthList* g_surfaceHealthLists[DEATH_STAR_SURFACE_LAYOUT_COUNT];
extern const XwSurfaceHealthList* g_trenchHealthLists[DEATH_STAR_TRENCH_LAYOUT_COUNT];

/* Original IDB size: 2 bytes. */
struct XwSurfaceObjectPlacement {
	/* IDB +0x0: Surface: low nibble X and high nibble Y, each in 4096-unit steps within the cell. Trench:
	 * bits 0..1 choose lateral mounting, bits 2..5 give along-cell Y, bits 6..7 give height tier. */
	uint8_t packedPosition;
	/* IDB +0x1: Object type index used for model bounds, rendering, gun behavior, and collision. */
	uint8_t objectType;
};

#pragma pack(push, 1)

/* Packed header of an embedded variable-length destruction pattern. */
struct XwSurfaceDestructionPattern {
	/* IDB +0x0: Number of following eight-byte effect records. */
	uint8_t count;
	/* IDB +0x1: Packed destruction effects, stride 8. */
	struct XwSurfaceDestructionEffect effects[];
};

#pragma pack(pop)

typedef char XwSurfaceDestructionPattern_size[(sizeof(XwSurfaceDestructionPattern) == 1) ? 1 : -1];
typedef char XwSurfaceDestructionPattern_effects_offset[(offsetof(XwSurfaceDestructionPattern, effects) == 1)
															? 1
															: -1];

/* Original IDB size: 1 bytes. */
struct XwSurfacePlacementList {
	/* IDB +0x0: Number of two-byte placement records following this byte. Firing uses only the leading 8
	 * surface or 4 trench entries, except special trench list 8 uses all 13. */
	uint8_t count;
	/* IDB +0x1 */
	struct XwSurfaceObjectPlacement placements[];
};

typedef struct XwSurfacePlacementList8 {
	uint8_t count;
	XwSurfaceObjectPlacement placements[8];
} XwSurfacePlacementList8;

typedef struct XwSurfacePlacementList13 {
	uint8_t count;
	XwSurfaceObjectPlacement placements[13];
} XwSurfacePlacementList13;

typedef struct XwSurfacePlacementList14 {
	uint8_t count;
	XwSurfaceObjectPlacement placements[14];
} XwSurfacePlacementList14;

typedef struct XwSurfacePlacementList17 {
	uint8_t count;
	XwSurfaceObjectPlacement placements[17];
} XwSurfacePlacementList17;

typedef struct XwSurfacePlacementList18 {
	uint8_t count;
	XwSurfaceObjectPlacement placements[18];
} XwSurfacePlacementList18;

typedef struct XwSurfacePlacementList19 {
	uint8_t count;
	XwSurfaceObjectPlacement placements[19];
} XwSurfacePlacementList19;

extern const XwSurfacePlacementList14 g_surfacePlacements2;
extern const XwSurfacePlacementList14 g_surfacePlacements0;
extern const XwSurfacePlacementList14 g_surfacePlacements3;
extern const XwSurfacePlacementList14 g_surfacePlacements1;
extern const XwSurfacePlacementList8 g_surfacePlacements4;
extern const XwSurfacePlacementList17 g_trenchPlacements0;
extern const XwSurfacePlacementList17 g_trenchPlacements1;
extern const XwSurfacePlacementList19 g_trenchPlacements2;
extern const XwSurfacePlacementList19 g_trenchPlacements3;
extern const XwSurfacePlacementList18 g_trenchPlacements4;
extern const XwSurfacePlacementList19 g_trenchPlacements5;
extern const XwSurfacePlacementList19 g_trenchPlacements6;
extern const XwSurfacePlacementList17 g_trenchPlacements7;
extern const XwSurfacePlacementList13 g_trenchPlacements8;
extern const XwSurfacePlacementList* g_surfacePlacementLists[DEATH_STAR_SURFACE_LAYOUT_COUNT];
extern const XwSurfacePlacementList* g_trenchPlacementLists[DEATH_STAR_TRENCH_LAYOUT_COUNT];

#pragma pack(push, 1)

/* Packed records embedded in the original executable. */
typedef struct XwSurfaceDestructionPattern1 {
	uint8_t count;
	XwSurfaceDestructionEffect effects[1];
} XwSurfaceDestructionPattern1;

typedef struct XwSurfaceDestructionPattern2 {
	uint8_t count;
	XwSurfaceDestructionEffect effects[2];
} XwSurfaceDestructionPattern2;

typedef struct XwSurfaceDestructionPattern3 {
	uint8_t count;
	XwSurfaceDestructionEffect effects[3];
} XwSurfaceDestructionPattern3;

typedef struct XwSurfaceDestructionPattern4 {
	uint8_t count;
	XwSurfaceDestructionEffect effects[4];
} XwSurfaceDestructionPattern4;

typedef struct XwSurfaceDestructionPattern5 {
	uint8_t count;
	XwSurfaceDestructionEffect effects[5];
} XwSurfaceDestructionPattern5;

#pragma pack(pop)

typedef char XwSurfaceDestructionPattern1_size[(sizeof(XwSurfaceDestructionPattern1) == 9) ? 1 : -1];
typedef char XwSurfaceDestructionPattern2_size[(sizeof(XwSurfaceDestructionPattern2) == 17) ? 1 : -1];
typedef char XwSurfaceDestructionPattern3_size[(sizeof(XwSurfaceDestructionPattern3) == 25) ? 1 : -1];
typedef char XwSurfaceDestructionPattern4_size[(sizeof(XwSurfaceDestructionPattern4) == 33) ? 1 : -1];
typedef char XwSurfaceDestructionPattern5_size[(sizeof(XwSurfaceDestructionPattern5) == 41) ? 1 : -1];

extern const XwSurfaceDestructionPattern2 g_surfaceDestructionPattern0;
extern const XwSurfaceDestructionPattern2 g_surfaceDestructionPattern1;
extern const XwSurfaceDestructionPattern4 g_surfaceDestructionPattern2;
extern const XwSurfaceDestructionPattern5 g_surfaceDestructionPattern3;
extern const XwSurfaceDestructionPattern1 g_surfaceDestructionPattern4;
extern const XwSurfaceDestructionPattern4 g_surfaceDestructionPattern5;
extern const XwSurfaceDestructionPattern4 g_surfaceDestructionPattern6;
extern const XwSurfaceDestructionPattern3 g_surfaceDestructionPattern7;
extern const XwSurfaceDestructionPattern4 g_surfaceDestructionPattern8;
extern const XwSurfaceDestructionPattern1 g_surfaceDestructionPattern9;
extern const XwSurfaceDestructionPattern* g_surfaceDestructionPatterns[DEATH_STAR_DESTRUCTION_PATTERN_COUNT];
extern const uint8_t g_surfacePatternIndices2[DEATH_STAR_SURFACE_OBJECT_COUNT];
extern const uint8_t g_surfacePatternIndices0[DEATH_STAR_SURFACE_OBJECT_COUNT];
extern const uint8_t g_surfacePatternIndices3[DEATH_STAR_SURFACE_OBJECT_COUNT];
extern const uint8_t g_surfacePatternIndices1[DEATH_STAR_SURFACE_OBJECT_COUNT];
extern const uint8_t g_surfacePatternIndices4[DEATH_STAR_SHORT_LAYOUT_OBJECT_COUNT];
extern const uint8_t* g_surfaceDestructionPatternIndices[DEATH_STAR_SURFACE_LAYOUT_COUNT];
extern const uint8_t g_trenchPatternIndices0[DEATH_STAR_SURFACE_OBJECT_COUNT];
extern const uint8_t* g_trenchDestructionPatternIndices[DEATH_STAR_TRENCH_LAYOUT_COUNT];

extern XwSurfaceGunCell g_surfaceGunCells[DEATH_STAR_SURFACE_GUN_CELL_CAPACITY];
extern uint8_t g_surfaceSpecialTargetHit;
extern uint16_t g_surfaceGoalCellHashSlots[DEATH_STAR_SURFACE_GOAL_COUNT];
extern uint8_t g_surfaceSpecialTargetCollisionMode;
extern uint8_t g_surfaceVictoryExitSecond;
extern uint16_t g_surfaceGoalCellKeys[DEATH_STAR_SURFACE_GOAL_COUNT];
extern int16_t g_surfaceGunCellRefreshTicks;
extern int16_t g_deathStarSurfaceCellX;
extern int16_t g_deathStarSurfaceCellY;
extern uint8_t g_trenchSpecialCellVoicePlayed;
extern uint8_t g_deathStarSurfaceModeActive;
extern uint16_t g_surfaceGunCellCount;
extern XwSurfaceCellDamageState g_surfaceCellDamageStates[DEATH_STAR_SURFACE_CELL_COUNT];
extern struct ObjectRecord g_deathStarRenderObject;
extern struct CraftData g_deathStarRenderCraft;

/* Declarations follow ascending original IDB address. */

/* 0x426850 */
void DeathStar_InitSurfaceTexture(void);

/* 0x426990 */
void DeathStar_DrawSurfaceDetailModel(uint8_t objectType, int worldX, int worldY, int worldZ);

/* 0x426AB0 */
void DeathStar_DrawSurfaceAndTrench(void);

/* 0x427320 */
void DeathStar_DrawSurfaceCellDetails(int cellViewX, int cellViewY, int cellViewZ);

/* 0x4278D0 */
void DeathStar_UpdateSurfaceGuns(void);

/* 0x427A00 */
void DeathStar_RebuildSurfaceGunCells(void);

/* 0x427AB0 */
void DeathStar_AddSurfaceGunCell(int16_t cellX, int16_t cellY);

/* 0x427B70 */
void DeathStar_FireSurfaceGunCell(const struct XwSurfaceGunCell* cell);

/* 0x428420 */
int DeathStar_TestSurfaceCollision(uint16_t objectIndex);

/* 0x428950 */
int DeathStar_TestCellObjectCollision(uint16_t objectIndex, int16_t cellX, int16_t cellY, int16_t trenchPass);

/* 0x429490 */
void DeathStar_SpawnSurfaceDestructionEffects(int worldX, int worldY, int worldZ, uint16_t placementIndex,
											  uint16_t layoutIndex, int16_t isTrench,
											  const struct XwBounds16* bounds);

/* 0x429AB0 */
void DeathStar_InitTrenchTexture(void);

/* 0x429BF0 */
void DeathStar_DrawTrenchDetailMesh(uint8_t objectType, int rootMeshIndex, int worldX, int worldY,
									int worldZ);

/* 0x429D20 */
void DeathStar_DrawTrench(void);

/* 0x42A1B0 */
void DeathStar_DrawTrenchCellDetails(int cellViewX, int cellViewY, int cellViewZ, unsigned int cellWorldY);

#ifdef __cplusplus
}
#endif

#endif
