#include "xw/input/joystick.h"

#include "xw/flight/feinput.h"

#include <aeron/compat/mmsystem.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4DED6C
int g_joyAxisRangeX = 1;

// GLOBAL: XW 0x4DED70
int g_joyAxisRangeY = 1;

// GLOBAL: XW 0x4DED74
int g_joyAxisRangeZ = 1;

// GLOBAL: XW 0x4DED78
unsigned int g_joystickButtonCount = JOYSTICK_DEFAULT_BUTTON_COUNT;

// GLOBAL: XW 0x566840
int g_joyAxisCenterZ = 0;

// GLOBAL: XW 0x566848
unsigned int g_joyDeviceId[JOYSTICK_DEVICE_SLOT_COUNT] = { 0 };

// GLOBAL: XW 0x566858
int g_joyAxisCenterX = 0;

// GLOBAL: XW 0x566870
int g_joyAxisCenterY = 0;

// GLOBAL: XW 0x5668F0
int g_joystickCalibrationInitialized[JOYSTICK_DEVICE_SLOT_COUNT] = { 0 };

// GLOBAL: XW 0x5668F8
JoystickAxisCalibration g_joystickAxisCalibration = { 0 };

// GLOBAL: XW 0x5678EC
unsigned int g_joystickCachedButtons = 0;

// GLOBAL: XW 0x5B8AA0
uint8_t g_flightJoystickActions[JOYSTICK_ACTION_CAPACITY] = { 0 };

// FUNCTION: XW 0x4AC810
unsigned int Joystick_GetButtonCount(void) { return g_joystickButtonCount; }

// FUNCTION: XW 0x4AC820
int Joystick_HasPov(void) { return g_joystickAxisCalibration.hasPov; }

// FUNCTION: XW 0x4AC830
unsigned int Joystick_GetDeviceId(int connectedDeviceIndex) {
	JOYCAPSA capabilities;
	JOYINFOEX position;
	int deviceSlotCount = joyGetNumDevs();
	int deviceId = 0;
	int connectedOrdinal;
	if (deviceSlotCount == 0)
		return 0;
	connectedOrdinal = 0;
	for (; deviceId < deviceSlotCount; ++deviceId) {
		if (joyGetDevCapsA(deviceId, &capabilities, sizeof(capabilities)) == MMSYSERR_NOERROR) {
			position.dwSize = sizeof(position);
			position.dwFlags = JOY_RETURNX | JOY_RETURNY | JOY_RETURNBUTTONS | JOY_RETURNCENTERED;
			if (joyGetPosEx(deviceId, &position) == MMSYSERR_NOERROR) {
				if (connectedOrdinal == connectedDeviceIndex)
					return deviceId;
				++connectedOrdinal;
			}
		}
	}
	return deviceId;
}

// FUNCTION: XW 0x4AC8D0
void Joystick_PollRawAxes(int deviceIndex, int* pAxisX, int* pAxisY, int* pAxisZ, unsigned int* pButtons) {
	JOYINFOEX position;
	JOYCAPSA capabilities;
	deviceIndex = deviceIndex != 0;
	if (!g_joystickCalibrationInitialized[deviceIndex]) {
		MMRESULT result;
		memset(&capabilities, 0, sizeof(capabilities));
		g_joystickCalibrationInitialized[deviceIndex] = 1;
		g_joyDeviceId[deviceIndex] = Joystick_GetDeviceId(0);
		result = joyGetDevCapsA(g_joyDeviceId[deviceIndex], &capabilities, sizeof(capabilities));
		if (result != MMSYSERR_NOERROR) {
			g_joyDeviceId[deviceIndex] = Joystick_GetDeviceId(1);
			result = joyGetDevCapsA(g_joyDeviceId[deviceIndex], &capabilities, sizeof(capabilities));
		}
		if (result == MMSYSERR_NOERROR) {
			int rangeX;
			int rangeY;
			int rangeZ;
			g_joystickAxisCalibration.hasPov = (capabilities.wCaps & JOYSTICK_CAPS_POV4DIR) != 0;
			g_joystickButtonCount = capabilities.wNumButtons;
			rangeX = capabilities.wXmax - capabilities.wXmin;
			rangeY = capabilities.wYmax - capabilities.wYmin;
			rangeZ = capabilities.wZmax - capabilities.wZmin;
			g_joyAxisRangeX = rangeX;
			g_joystickAxisCalibration.axisNormalizeOffsetX = (rangeX >> 1) - capabilities.wXmax;
			g_joyAxisRangeY = rangeY;
			g_joystickAxisCalibration.axisNormalizeOffsetY = (rangeY >> 1) - capabilities.wYmax;
			g_joyAxisRangeZ = rangeZ;
			g_joystickAxisCalibration.axisNormalizeOffsetZ = (rangeZ >> 1) - capabilities.wZmax;
			g_joystickAxisCalibration.axisDeadzoneX = rangeX / JOYSTICK_DEADZONE_DIVISOR;
			g_joystickAxisCalibration.axisDeadzoneY = rangeY / JOYSTICK_DEADZONE_DIVISOR;
			g_joystickAxisCalibration.axisDeadzoneZ = rangeZ / JOYSTICK_DEADZONE_DIVISOR;
			g_joyAxisCenterX = (rangeX >> 1) + capabilities.wXmin;
			g_joyAxisCenterY = (rangeY >> 1) + capabilities.wYmin;
			g_joyAxisCenterZ = (rangeZ >> 1) + capabilities.wZmin;
		} else {
			g_joystickAxisCalibration.hasPov = 0;
			g_joyAxisRangeX = 1;
			g_joyAxisRangeY = 1;
			g_joyAxisRangeZ = 1;
			g_joystickAxisCalibration.axisNormalizeOffsetX = 0;
			g_joystickAxisCalibration.axisNormalizeOffsetY = 0;
			g_joystickAxisCalibration.axisNormalizeOffsetZ = 0;
			g_joystickAxisCalibration.axisDeadzoneX = 0;
			g_joystickAxisCalibration.axisDeadzoneY = 0;
			g_joystickAxisCalibration.axisDeadzoneZ = 0;
			g_joystickButtonCount = 0;
		}
	}
	memset(&position, 0, sizeof(position));
	*pButtons = 0;
	position.dwSize = sizeof(position);
	position.dwFlags =
		JOY_RETURNX | JOY_RETURNY | JOY_RETURNZ | JOY_RETURNPOV | JOY_RETURNBUTTONS | JOY_RETURNCENTERED;
	if (joyGetPosEx(g_joyDeviceId[deviceIndex], &position) == MMSYSERR_NOERROR) {
		int distance;
		distance = position.dwXpos - g_joyAxisCenterX;
		if (distance < 0)
			distance = -distance;
		if (distance > g_joystickAxisCalibration.axisDeadzoneX)
			*pAxisX = JOYSTICK_AXIS_SCALE *
					  (g_joystickAxisCalibration.axisNormalizeOffsetX + position.dwXpos) / g_joyAxisRangeX;
		else
			*pAxisX = 0;
		distance = position.dwYpos - g_joyAxisCenterY;
		if (distance < 0)
			distance = -distance;
		if (distance > g_joystickAxisCalibration.axisDeadzoneY)
			*pAxisY = JOYSTICK_AXIS_SCALE *
					  (g_joystickAxisCalibration.axisNormalizeOffsetY + position.dwYpos) / g_joyAxisRangeY;
		else
			*pAxisY = 0;
		distance = position.dwZpos - g_joyAxisCenterZ;
		if (distance < 0)
			distance = -distance;
		if (distance > g_joystickAxisCalibration.axisDeadzoneZ && g_joyAxisRangeZ > 0)
			*pAxisZ = JOYSTICK_AXIS_SCALE *
					  (g_joystickAxisCalibration.axisNormalizeOffsetZ + position.dwZpos) / g_joyAxisRangeZ;
		else
			*pAxisZ = 0;
		*pButtons = position.dwButtons & JOYSTICK_BUTTON_MASK;
		if (g_joystickAxisCalibration.hasPov && position.dwPOV != JOY_POVCENTERED)
			*pButtons |= (unsigned int)JOYSTICK_POV_FIRST_BUTTON
						 << (position.dwPOV / JOYSTICK_POV_QUARTER_TURN);
	} else {
		*pAxisX = JOYSTICK_AXIS_UNAVAILABLE;
		*pAxisY = JOYSTICK_AXIS_UNAVAILABLE;
		*pAxisZ = JOYSTICK_AXIS_UNAVAILABLE;
	}
}

// FUNCTION: XW 0x4AF2A0
void Joystick_PollRawAxesIfEnabled(int deviceIndex, int* outX, int* outY, int* outZ) {
	if (g_joystickPollingSuppressed == 0)
		Joystick_PollRawAxes(deviceIndex, outX, outY, outZ, &g_joystickCachedButtons);
}
