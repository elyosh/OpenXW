#ifndef XW_FLIGHT_OBJECT_STARSHIP_H
#define XW_FLIGHT_OBJECT_STARSHIP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/assets/opt_model.h"

#include <stddef.h>
#include <stdint.h>

enum {
	STARSHIP_GUN_HIGH_SKILL = 0xAAAA,
	STARSHIP_GUN_MEDIUM_SKILL = 0x5555,
	STARSHIP_GUN_COOLDOWN_MASK = 0x7F,
	STARSHIP_GUN_ALTERNATE_PROJECTILE = 0x80,
	STARSHIP_GUN_COOLDOWN = 59,
	STARSHIP_GUN_LOW_SKILL_DIVISOR = 6,
	STARSHIP_GUN_HIGH_SKILL_SHIFT = 1,
	STARSHIP_GUN_MEDIUM_SKILL_SHIFT = 2,
	STARSHIP_GUN_MAX_RANGE = 0x20000,
	STARSHIP_GUN_ARC_FIELD_MASK = 3,
	STARSHIP_GUN_ARC_FIELD_BITS = 2,
	STARSHIP_GUN_ARC_CENTER_SHIFT = 14,
	STARSHIP_GUN_ARC_WIDTH_SHIFT = 13,
	STARSHIP_GUN_HALF_TURN = 0x8000,
	STARSHIP_GUN_ANGLE_BYTE_SHIFT = 8,
	STARSHIP_GUN_LEAD_SHIFT = 14,
	STARSHIP_GUN_LEAD_RANDOM_MASK = 3,
	STARSHIP_GUN_MESH_HALF_TURN = 0x80,
	STARSHIP_GUN_CORVETTE_FIRST_MESH = 0,
	STARSHIP_GUN_CORVETTE_SECOND_MESH = 2,
	STARSHIP_GUN_LIFETIME_MULTIPLIER = 3
};

enum {
	STARSHIP_COMPONENT_HP_INDESTRUCTIBLE = 0xFF,
	STARSHIP_COMPONENT_DAMAGE_SHIFT = 4,
	STARSHIP_COMPONENT_DESTROYED = 2,
	STARSHIP_SHIELD_GENERATOR_FIRST = 6,
	STARSHIP_SHIELD_GENERATOR_SECOND = 7,
	STARSHIP_MESH_ROTATION_SHIFT = 8,
	STARSHIP_ROTATION_FRACTION_BITS = 15,
	STARSHIP_ROTATION_CLAMP_LIMIT = 0x40000000,
	STARSHIP_ROTATION_CLAMP_MAX = 0x3FFFFFFF,
	STARSHIP_ROTATION_CLAMP_MIN = -0x3FFF0000,
	STARSHIP_COMPONENT_EXTENT_SHIFT = 9,
	STARSHIP_COMPONENT_SCALE_MAX = 255
};

enum {
	STARSHIP_DESTRUCTION_CLEAR_COUNT = 5,
	STARSHIP_CORVETTE_EXPLOSION_MESH_COUNT = 5,
	STARSHIP_DESTRUCTION_RANDOM_HARDPOINT = 0xFFFF,
	STARSHIP_DESTRUCTION_HARDPOINT_ROLL_1 = 1,
	STARSHIP_DESTRUCTION_HARDPOINT_ROLL_2 = 2,
	STARSHIP_DESTRUCTION_MESH_0 = 0,
	STARSHIP_DESTRUCTION_MESH_1 = 1,
	STARSHIP_DESTRUCTION_MESH_2 = 2,
	STARSHIP_DESTRUCTION_MESH_3 = 3,
	STARSHIP_DESTRUCTION_MESH_5 = 5,
	STARSHIP_DESTRUCTION_MESH_12 = 12,
	STARSHIP_DESTRUCTION_MESH_22 = 22,
	STARSHIP_CORVETTE_EXPLOSION_SCALE = 0xA00,
	STARSHIP_FRIGATE_MESH_2_EXPLOSION_SCALE = 0xC00,
	STARSHIP_FRIGATE_MESH_3_EXPLOSION_SCALE = 0x1800,
	STARSHIP_DESTROYER_EXPLOSION_SCALE = 0x3F00,
	STARSHIP_CRUISER_EXPLOSION_SCALE = 0x3200,
	STARSHIP_DEFAULT_EXPLOSION_SCALE = 0x2200,
	STARSHIP_DESTRUCTION_SOUND = 12
};

enum {
	STARSHIP_RECURRING_EXPLOSION_FIRST_THRESHOLD = 0x4000,
	STARSHIP_RECURRING_EXPLOSION_SECOND_THRESHOLD = 0x9000,
	STARSHIP_RECURRING_EXPLOSION_FIRST_MESH = 5,
	STARSHIP_RECURRING_EXPLOSION_SECOND_MESH = 0,
	STARSHIP_RECURRING_EXPLOSION_THIRD_MESH = 22,
	STARSHIP_RECURRING_EXPLOSION_HARDPOINT_MASK = 0x7FFF,
	STARSHIP_COMPONENT_EXPLOSION_STATE = 2,
	STARSHIP_COMPONENT_EXPLOSION_SCALE_SHIFT = 6,
	STARSHIP_COMPONENT_EXPLOSION_VARIANT_MASK = 1
};

enum { STARSHIP_COORD_X = 0, STARSHIP_COORD_Y = 1, STARSHIP_COORD_Z = 2, STARSHIP_COORD_COUNT = 3 };

enum { STARSHIP_FACE_VERTEX_NONE = -1 };

enum {
	STARSHIP_ROTATION_AXIS_X = 3,
	STARSHIP_ROTATION_AXIS_Y = 4,
	STARSHIP_ROTATION_AXIS_Z = 5,
	STARSHIP_AXIS_ANGLE_COUNT = 4,
	STARSHIP_MATRIX_CAPACITY = 16,
	STARSHIP_BOUNDS_VECTOR_COUNT = 2,
	STARSHIP_SPECIAL_FACE_GROUP_2 = 2,
	STARSHIP_SPECIAL_FACE_GROUP_3 = 3,
	STARSHIP_SPECIAL_FACE_GROUP_7 = 7
};

extern int g_collideSweepFaceGroupOrdinal;
extern OptVector g_collideSweepWalkerStart;
extern OptVector g_collideSweepWalkerEnd;
extern int g_collideSweepHitMeshOrdinal;
extern OptVector g_collideSweepModelStart;
extern OptVector g_collideSweepModelEnd;
extern int g_collideSweepCurrentMeshOrdinal;
extern float g_collideCurrentMeshRotationAngle;
extern float g_collideSweepHitFraction;
extern OptNode* g_collideCurrentMeshVertsNode;

extern const float g_collisionNoHitFraction;
extern const float g_collisionRadiansPerAngleByte;
extern const float g_collisionHitFractionBackoff;
extern const float g_collisionQ15ToFloat;
extern const float g_collisionPlaneSnapPositiveLimit;
extern const float g_collisionPlaneSnapNegativeLimit;

struct ObjectRecord;
struct OptNode;
struct OptimizedPolyObject;
/* Declarations follow ascending original IDB address. */

/* 0x423A50 */
int starship_checkstarshiphit(uint16_t sourceObjIdx, uint16_t targetObjIdx);

/* 0x424000 */
int starship_CheckSweptMeshCollision(int objectType, int meshIndex, int point1X, int point1Y, int point1Z,
									 int point2X, int point2Y, int point2Z);

/* 0x424240 */
int starship_TestSweepAgainstOptNode(struct OptimizedPolyObject* object, struct OptNode* node);

/* 0x424770 */
int starship_IntersectSegmentWithFacePlane(const float* faceNormal, const float* faceVertex,
										   const float* segmentStart, const float* segmentEnd, float* outT);

/* 0x424920 */
int32_t starship_PointInFacePolygon(const float* faceNormal, const float* vertexCoords,
									const int* faceVertexIndices, float* projectedPoint);

/* 0x424C00 */
int starship_damagecomponent(uint16_t victimObjIdx, int16_t hitMeshIndex, uint16_t damageAmount);

/* 0x424FD0 */
void starship_createstarshipexplo__partial(uint16_t objectIdx);

/* 0x4250E0 */
int starship_makestarshipcompexplo(struct ObjectRecord* object, uint16_t meshIndex, unsigned int screenScale,
								   int chooseRandomHardpoint);

/* 0x425280 */
void starship_createstarshipexplo(uint16_t objectIndex);

/* 0x425420 */
void starship_firelasergunner(uint16_t sourceObjIdx, uint16_t weaponSlotIdx, uint16_t targetRef);

#ifdef __cplusplus
}
#endif

#endif
