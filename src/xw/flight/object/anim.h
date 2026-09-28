#ifndef XW_FLIGHT_OBJECT_ANIM_H
#define XW_FLIGHT_OBJECT_ANIM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/flight/object/object.h"
#include "xw/render/render_scene.h"

#include <stddef.h>
#include <stdint.h>

enum { ANIM_BITMAP_QUEUE_CAPACITY = 32 };

enum {
	ANIM_BITMAP_FIRST_FRAME = 0x8000,
	ANIM_BITMAP_SCALE_SHIFT = 6,
	ANIM_BITMAP_UNIT_SCALE = 256,
	ANIM_BITMAP_SELECTION_MASK = 0x7FFF,
	ANIM_BITMAP_FRAME_MASK = 0x7F,
	ANIM_BITMAP_MODEL_SHIFT = 7,
	ANIM_MISSION_POSITION_SHIFT = 8
};

enum {
	ANIM_HYPERSPACE_IDLE = 0,
	ANIM_HYPERSPACE_ALIGN = 1,
	ANIM_HYPERSPACE_SETUP = 2,
	ANIM_HYPERSPACE_DEPART = 3,
	ANIM_HYPERSPACE_EXTERNAL = 4,
	ANIM_HYPERSPACE_RETURN = 5,
	ANIM_HYPERSPACE_ARRIVE = 6,
	ANIM_HYPERSPACE_ALIGN_RATE = 20,
	ANIM_QUARTER_TURN = 0x4000,
	ANIM_HALF_TURN = 0x8000,
	ANIM_THREE_QUARTER_TURN = 0xC000,
	ANIM_HYPERSPACE_PATH_CHECK_TICK = 708,
	ANIM_HYPERSPACE_START_TICK = 1180,
	ANIM_HYPERSPACE_DEPART_MOVE_TICK = 1416,
	ANIM_HYPERSPACE_EXTERNAL_TICK = 1652,
	ANIM_HYPERSPACE_RETURN_TICK = 1888,
	ANIM_HYPERSPACE_RETURN_FADE_TICK = 2124,
	ANIM_HYPERSPACE_ARRIVE_TICK = 2360,
	ANIM_HYPERSPACE_PATH_LENGTH = 0x40000,
	ANIM_HYPERSPACE_SURFACE_CLEARANCE = 0x2000,
	ANIM_HYPERSPACE_ABORT_SPEED = 10,
	ANIM_HYPERSPACE_JUMP_SPEED = 100,
	ANIM_HYPERSPACE_ARRIVE_SPEED = 30,
	ANIM_HYPERSPACE_LAUNCH_STEP = 1792,
	ANIM_HYPERSPACE_STEP = 224,
	ANIM_HYPERSPACE_STREAK_INITIAL = 32256,
	ANIM_HYPERSPACE_STREAK_MINIMUM = 33280,
	ANIM_HYPERSPACE_RANDOM_DEPTH_SHIFT = 4,
	ANIM_HYPERSPACE_RANDOM_DEPTH_MASK = 31,
	ANIM_HYPERSPACE_RANDOM_DEPTH_BASE = 256,
	ANIM_HYPERSPACE_CAMERA_SIDE = 128,
	ANIM_HYPERSPACE_CAMERA_FORWARD = 896,
	ANIM_HYPERSPACE_CAMERA_PITCH = -2048,
	ANIM_HYPERSPACE_CAMERA_RETURN_YAW = 30720,
	ANIM_HYPERSPACE_EXTERNAL_VIEW = 18,
	ANIM_HYPERSPACE_RETURN_OFFSET = 52864,
	ANIM_HYPERSPACE_SLOWDOWN_POSITION = -768,
	ANIM_HYPERSPACE_SLOWDOWN_SHIFT = 6,
	ANIM_HYPERSPACE_FLIGHT_EXIT = 2,
	ANIM_HYPERSPACE_LOST_PILOT = 2
};

enum {
	ANIM_UPDATE_INTERVAL = 29,
	ANIM_LARGE_CRAFT_EXTENT = 2800,
	ANIM_LARGE_CRAFT_MESH_SKIP = 3,
	ANIM_COMPONENT_DETACH_THRESHOLD = 0x100,
	ANIM_CRAFT_EMBER_THRESHOLD = 0x1800,
	ANIM_COMPONENT_EMBER_THRESHOLD = 0x800,
	ANIM_BWING_CLOSED_ROTATION = 64,
	ANIM_BWING_ROTATION_STEP = 4,
	ANIM_XWING_UPPER_CLOSED_ROTATION = 12,
	ANIM_XWING_LOWER_CLOSED_ROTATION = 8
};

extern int16_t g_sceneBillboardQueueCount;
extern uint32_t g_hyperspaceStreakLength;
extern uint16_t g_playerHyperspaceElapsedTicks;
extern uint8_t g_hyperspaceflag;
extern uint16_t g_hyperspaceSavedDebrisEnabled;
extern uint16_t g_hyperspaceSavedBackdropsEnabled;
extern uint8_t g_hyperspaceAbortAndCollisionsAllowed;
extern SceneBillboardQueueEntry g_sceneBillboardQueue[ANIM_BITMAP_QUEUE_CAPACITY];
extern XwObjectGenus g_currentModelObjectGenus;
extern uint16_t g_animationFrameIndex;
extern const uint16_t* g_animationFrames;

enum {
	ANIM_FRAME_JUMP_BASE = 0xFF00,
	ANIM_FRAME_RESTART = 0xFFFD,
	ANIM_FRAME_ADVANCE = 0xFFFE,
	ANIM_FRAME_END_OBJECT = 0xFFFF,
	ANIM_FIRST_DISPLAY_FRAME = 2,
	ANIM_COMPONENT_DAMAGE_LOOP_FRAME = 12
};

struct SceneBillboardQueueEntry;
/* Declarations follow ascending original IDB address. */

/* 0x401070 */
void anim_drawverysimpleobject(uint16_t objectIndex);

/* 0x401270 */
void anim_add_bitmap_draw(uint16_t objectRef, int16_t frameCode, int16_t screenScale, int16_t screenX,
						  int16_t screenY, int depth, int16_t rotationAngle);

/* 0x4012E0 */
void anim_sort_and_draw_bitmaps(int drawTargetMarkersUnused);

/* 0x4013E0 */
void anim_draw_bitmap(const struct SceneBillboardQueueEntry* quadRecord);

/* 0x4015A0 */
void anim_updateanimation(void);

/* 0x401AC0 */
void anim_updateanimstate(uint16_t objectRef);

/* 0x401B50 */
void anim_AdvanceSurfaceObjectStateCounters(void);

/* 0x401B90 */
void anim_dohyperspace(void);

#ifdef __cplusplus
}
#endif

#endif
