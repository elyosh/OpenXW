#ifndef XW_INPUT_JOYSTICK_H
#define XW_INPUT_JOYSTICK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

enum {
	JOYSTICK_ACTION_NONE = 0,
	JOYSTICK_ACTION_F1 = 0xC3,
	JOYSTICK_ACTION_SHIFT_F1 = 0xCF,
	JOYSTICK_ACTION_KEY_ALT_Q = 0x90,
	JOYSTICK_ACTION_KEY_ALT_W = 0x96,
	JOYSTICK_ACTION_KEY_ALT_E = 0x84,
	JOYSTICK_ACTION_KEY_ALT_R = 0x91,
	JOYSTICK_ACTION_KEY_ALT_T = 0x93,
	JOYSTICK_ACTION_KEY_ALT_Y = 0x98,
	JOYSTICK_ACTION_KEY_ALT_U = 0x94,
	JOYSTICK_ACTION_KEY_ALT_I = 0x88,
	JOYSTICK_ACTION_KEY_ALT_O = 0x8E,
	JOYSTICK_ACTION_KEY_ALT_P = 0x8F,
	JOYSTICK_ACTION_KEY_ALT_A = 0x80,
	JOYSTICK_ACTION_KEY_ALT_S = 0x92,
	JOYSTICK_ACTION_KEY_ALT_D = 0x83,
	JOYSTICK_ACTION_KEY_ALT_F = 0x85,
	JOYSTICK_ACTION_KEY_ALT_G = 0x86,
	JOYSTICK_ACTION_KEY_ALT_H = 0x87,
	JOYSTICK_ACTION_KEY_ALT_J = 0x89,
	JOYSTICK_ACTION_KEY_ALT_K = 0x8A,
	JOYSTICK_ACTION_KEY_ALT_L = 0x8B,
	JOYSTICK_ACTION_KEY_ALT_Z = 0x99,
	JOYSTICK_ACTION_KEY_ALT_X = 0x97,
	JOYSTICK_ACTION_KEY_ALT_C = 0x82,
	JOYSTICK_ACTION_KEY_ALT_V = 0x95,
	JOYSTICK_ACTION_KEY_ALT_B = 0x81,
	JOYSTICK_ACTION_KEY_ALT_N = 0x8D,
	JOYSTICK_ACTION_KEY_ALT_M = 0x8C,
	JOYSTICK_ACTION_KEY_HOME = 0xAA,
	JOYSTICK_ACTION_KEY_UP = 0xA6,
	JOYSTICK_ACTION_KEY_PAGE_UP = 0xAC,
	JOYSTICK_ACTION_KEY_LEFT = 0xA4,
	JOYSTICK_ACTION_KEY_RIGHT = 0xA5,
	JOYSTICK_ACTION_KEY_END = 0xAB,
	JOYSTICK_ACTION_KEY_DOWN = 0xA7,
	JOYSTICK_ACTION_KEY_PAGE_DOWN = 0xAD,
	JOYSTICK_ACTION_KEY_INSERT = 0xA8,
	JOYSTICK_ACTION_KEY_DELETE = 0xA9,
	JOYSTICK_ACTION_KEY_ALT_1 = 0x9B,
	JOYSTICK_ACTION_KEY_ALT_2 = 0x9C,
	JOYSTICK_ACTION_KEY_ALT_3 = 0x9D,
	JOYSTICK_ACTION_KEY_ALT_4 = 0x9E,
	JOYSTICK_ACTION_KEY_ALT_5 = 0x9F,
	JOYSTICK_ACTION_KEY_ALT_6 = 0xA0,
	JOYSTICK_ACTION_KEY_ALT_7 = 0xA1,
	JOYSTICK_ACTION_KEY_ALT_8 = 0xA2,
	JOYSTICK_ACTION_KEY_ALT_9 = 0xA3,
	JOYSTICK_ACTION_KEY_ALT_0 = 0x9A
};

enum {
	JOYSTICK_DEFAULT_BUTTON_COUNT = 2,
	JOYSTICK_ACTION_CAPACITY = 32,
	JOYSTICK_DEFAULT_ASSIGNMENT_LIMIT = 28
};

/* Action codes from the original JOYSTICK.TXT. */
enum {
	JOYSTICK_ACTION_FIRE = 156,
	JOYSTICK_ACTION_ROLL_TARGET = 157,
	JOYSTICK_ACTION_NEAREST_FIGHTER = 'r',
	JOYSTICK_ACTION_TOGGLE_COCKPIT = '.',
	JOYSTICK_ACTION_NEAREST_ATTACKER = 'e',
	JOYSTICK_ACTION_IDENTIFY = 'i',
	JOYSTICK_ACTION_THIRD_THROTTLE = '[',
	JOYSTICK_ACTION_FULL_THROTTLE = '\b',
	JOYSTICK_ACTION_MATCH_SPEED = '\r',
	JOYSTICK_ACTION_TWO_THIRDS_THROTTLE = ']',
	JOYSTICK_ACTION_RELEASE_REPEAT = 0xB2,
	JOYSTICK_ACTION_RELEASE_NEUTRAL = 0xBA,
	JOYSTICK_ACTION_RELEASE_RANGE_LAST = 0xBB
};

enum {
	JOYSTICK_DEVICE_SLOT_COUNT = 2,
	JOYSTICK_CAPS_POV4DIR = 0x10,
	JOYSTICK_DEADZONE_DIVISOR = 20,
	JOYSTICK_AXIS_SCALE = 255,
	JOYSTICK_AXIS_UNAVAILABLE = 32000,
	JOYSTICK_BUTTON_MASK = 0x0FFFFFFF,
	JOYSTICK_POV_FIRST_BUTTON = 0x10000000,
	JOYSTICK_POV_QUARTER_TURN = 9000
};

extern int g_joyAxisRangeX;
extern int g_joyAxisRangeY;
extern int g_joyAxisRangeZ;
extern unsigned int g_joystickButtonCount;
extern int g_joyAxisCenterZ;
extern unsigned int g_joyDeviceId[JOYSTICK_DEVICE_SLOT_COUNT];
extern int g_joyAxisCenterX;
extern int g_joyAxisCenterY;
extern int g_joystickCalibrationInitialized[JOYSTICK_DEVICE_SLOT_COUNT];

typedef struct JoystickAxisCalibration JoystickAxisCalibration;
typedef struct JoystickEntry JoystickEntry;
typedef struct XwJoystickCalibrationRecord XwJoystickCalibrationRecord;

/* Original IDB size: 28 bytes. */
struct JoystickAxisCalibration {
	/* IDB +0x0: Half range minus maximum; added to raw X before multiplying by 255/range. */
	int axisNormalizeOffsetX;
	/* IDB +0x4: Half range minus maximum; added to raw Y before multiplying by 255/range. */
	int axisNormalizeOffsetY;
	/* IDB +0x8: Half range minus maximum; added to raw Z before multiplying by 255/range. */
	int axisNormalizeOffsetZ;
	/* IDB +0xC: X range / 20; compared with distance from independently stored X center. */
	int axisDeadzoneX;
	/* IDB +0x10: Y range / 20; compared with distance from independently stored Y center. */
	int axisDeadzoneY;
	/* IDB +0x14: Z range / 20; compared with distance from independently stored Z center. */
	int axisDeadzoneZ;
	/* IDB +0x18: Nonzero when WINMM JOYCAPS_POV4DIR bit 0x10 is set. */
	int hasPov;
};

/* Original IDB size: 149 bytes. */
struct JoystickEntry {
	/* IDB +0x0: Decimal action code from joystick.txt; compared with the configured joystick button mapping.
	 */
	uint8_t actionCode;
	/* IDB +0x1: Space-delimited short action name, displayed in the options UI. */
	char name[20];
	/* IDB +0x15: Remainder of the joystick.txt line, displayed beside the short name. */
	char description[128];
};

/* Original IDB size: 30 bytes. */
struct XwJoystickCalibrationRecord {
	/* IDB +0x0: Sampled X center. */
	int centerX;
	/* IDB +0x4: Sampled Y center. */
	int centerY;
	/* IDB +0x8: max(centerX - sampledX, 2). */
	int leftRange;
	/* IDB +0xC: max(centerY - sampledY, 2). */
	int upRange;
	/* IDB +0x10: max(sampledX - centerX, 2). */
	int rightRange;
	/* IDB +0x14: max(sampledY - centerY, 2). */
	int downRange;
	/* IDB +0x18: No observed X-Wing reader; initialized to 4/6. */
	unsigned int field_18;
	/* IDB +0x1C: No observed X-Wing reader; initialized to 1/4. */
	uint8_t field_1C;
	/* IDB +0x1D: No observed X-Wing reader; initialized to 2/8. */
	uint8_t field_1D;
};

/* Declarations follow ascending original IDB address. */

extern JoystickAxisCalibration g_joystickAxisCalibration;
extern unsigned int g_joystickCachedButtons;
extern uint8_t g_flightJoystickActions[JOYSTICK_ACTION_CAPACITY];

/* 0x4AC810 */
unsigned int Joystick_GetButtonCount(void);

/* 0x4AC820 */
int Joystick_HasPov(void);

/* 0x4AC830 */
unsigned int Joystick_GetDeviceId(int connectedDeviceIndex);

/* 0x4AC8D0 */
void Joystick_PollRawAxes(int deviceIndex, int* pAxisX, int* pAxisY, int* pAxisZ, unsigned int* pButtons);

/* 0x4AF2A0 */
void Joystick_PollRawAxesIfEnabled(int deviceIndex, int* outX, int* outY, int* outZ);

#ifdef __cplusplus
}
#endif

#endif
