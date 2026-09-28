#include "xw_runtime/runtime/flight_camera.h"
#include "xw/flight/fview.h"
#include "xw/flight/object/anim.h"
#include "xw/flight/object/create.h"
#include "xw/flight/player/user.h"
#include "xw/flight/replay/replay.h"
#include "xw/flight/xw.h"
#include "xw/math/transfm2.h"
#include "xw/math/trig2.h"
#include "xw/render/flight_view.h"
#include "xw_dos94/render/dos93_math.h"
#include "xw_runtime/runtime/flight_types.h"
#include "xw_runtime/runtime/profile.h"
#include "xw_runtime/timing/flight_timing.h"

/* TIE's timestamped ring, using X-Wing's completed movement clock. */
typedef XwChaseTimingState ChaseTiming;
static ChaseTiming chase;

void XwFlightCamera_SaveTiming(XwChaseTimingState* out) { *out = chase; }

void XwFlightCamera_RestoreTiming(const XwChaseTimingState* state) { chase = *state; }

void XwFlightCamera_ResetChase(void) { chase = (ChaseTiming) { 0 }; }

void XwFlightCamera_ResetObject(unsigned slot) {
	if (slot == g_flightCamera.focusObjectRef || (chase.valid && slot == chase.focus))
		XwFlightCamera_ResetChase();
}

static void UpdateChase(void) {
	uint64_t now = XwFlightTiming_CompletedMovementTicks();
	uint64_t serial = XwFlightTiming_AdvanceSerial();
	const ObjectRecord* object = &g_objectTable[g_flightCamera.focusObjectRef];
	if (!chase.valid || chase.focus != g_flightCamera.focusObjectRef || now < chase.last_ticks) {
		for (unsigned i = 0; i < FLIGHT_VIEW_ANGLE_HISTORY_COUNT; ++i) {
			g_flightCamera.angleHistory.roll[i] = object->roll;
			g_flightCamera.angleHistory.pitch[i] = object->pitch;
			g_flightCamera.angleHistory.yaw[i] = object->yaw;
			chase.ticks[i] = now;
		}
		g_flightCamera.angleHistory.writeIndex = 0;
		chase.focus = g_flightCamera.focusObjectRef;
		chase.serial = serial;
		chase.last_ticks = now;
		chase.valid = true;
	}
	uint64_t delay = XW_EXTERNAL_VIEW_LAG * XwFlightTiming_ReferenceTicks();
	uint64_t target = now > delay ? now - delay : 0;
	unsigned write = g_flightCamera.angleHistory.writeIndex;
	unsigned selected = write;
	for (unsigned i = 0; i < FLIGHT_VIEW_ANGLE_HISTORY_COUNT; ++i) {
		selected = (write + FLIGHT_VIEW_ANGLE_HISTORY_COUNT - 1 - i) % FLIGHT_VIEW_ANGLE_HISTORY_COUNT;
		if (chase.ticks[selected] <= target)
			break;
	}
	g_flightCamera.viewRoll = g_flightCamera.angleHistory.roll[selected];
	g_flightCamera.viewPitch = g_flightCamera.angleHistory.pitch[selected];
	g_flightCamera.viewYaw = g_flightCamera.angleHistory.yaw[selected];
	if (chase.serial != serial) {
		g_flightCamera.angleHistory.roll[write] = object->roll;
		g_flightCamera.angleHistory.pitch[write] = object->pitch;
		g_flightCamera.angleHistory.yaw[write] = object->yaw;
		chase.ticks[write] = now;
		g_flightCamera.angleHistory.writeIndex = (write + 1) % FLIGHT_VIEW_ANGLE_HISTORY_COUNT;
		chase.serial = serial;
		chase.last_ticks = now;
	}
}

void XwFlightCamera_CartesianToPolar(int32_t x, int32_t y, int32_t z) {
	if (XwProfile_ActiveFlight()->version != XW_GAME_VERSION_93) {
		trig2_ctop(x, y, z);
		return;
	}
	Dos93PolarCoordinates result = Dos93_trig2_ctop(x, y, z);
	g_trig2Yaw = result.yaw;
	g_trig2Pitch = result.pitch;
	g_trig2PolarDistance = (int32_t)result.distance;
}

/* Shared camera selection/history: DOS94 0x680826, Windows 0x42F810. */
void XwFlightCamera_Update(void) {
	if (!g_replayviewmode &&
		(!g_flightCamera.externalViewActive || g_flightCamera.focusObjectRef == XW_PLAYER_NO_TARGET))
		chase.valid = false;
	if (g_replayviewmode != 0) {
		replay_calcreplayview();
	} else if (g_flightCamera.focusObjectRef == XW_PLAYER_NO_TARGET) {
		if (g_hyperspaceflag == ANIM_HYPERSPACE_IDLE) {
			XwFlightCamera_CartesianToPolar((int32_t)((uint32_t)g_playerFlightState.object->worldX -
													  (uint32_t)g_flightCamera.worldPosition.x),
											(int32_t)((uint32_t)g_playerFlightState.object->worldY -
													  (uint32_t)g_flightCamera.worldPosition.y),
											(int32_t)((uint32_t)g_playerFlightState.object->worldZ -
													  (uint32_t)g_flightCamera.worldPosition.z));
			g_flightCamera.viewRoll = 0;
			g_flightCamera.viewPitch = g_trig2Pitch;
			g_flightCamera.viewYaw = g_trig2Yaw;
		}
		fview_newcalcview(g_flightCamera.viewRoll, g_flightCamera.viewPitch, g_flightCamera.viewYaw, 0,
						  g_flightCamera.hudAimX, g_flightCamera.hudAimY, NULL);
	} else if (g_flightCamera.externalViewActive != 0) {
		if (XwFlightTiming_IsUnlocked()) {
			UpdateChase();
		} else {
			int16_t historyIndex = g_flightCamera.angleHistory.writeIndex - XW_EXTERNAL_VIEW_LAG;
			if (historyIndex < 0)
				historyIndex += FLIGHT_VIEW_ANGLE_HISTORY_COUNT;
			g_flightCamera.viewRoll = g_flightCamera.angleHistory.roll[historyIndex];
			g_flightCamera.viewPitch = g_flightCamera.angleHistory.pitch[historyIndex];
			g_flightCamera.viewYaw = g_flightCamera.angleHistory.yaw[historyIndex];
			g_flightCamera.angleHistory.roll[g_flightCamera.angleHistory.writeIndex] =
				g_objectTable[g_flightCamera.focusObjectRef].roll;
			g_flightCamera.angleHistory.pitch[g_flightCamera.angleHistory.writeIndex] =
				g_objectTable[g_flightCamera.focusObjectRef].pitch;
			g_flightCamera.angleHistory.yaw[g_flightCamera.angleHistory.writeIndex] =
				g_objectTable[g_flightCamera.focusObjectRef].yaw;
			if (++g_flightCamera.angleHistory.writeIndex == FLIGHT_VIEW_ANGLE_HISTORY_COUNT)
				g_flightCamera.angleHistory.writeIndex = 0;
		}
		fview_newcalcview(g_flightCamera.viewRoll, g_flightCamera.viewPitch, g_flightCamera.viewYaw, 0,
						  g_flightCamera.hudAimX, g_flightCamera.hudAimY, NULL);
		if (XwFlightTypes_Dos() ? (g_hyperspaceflag != 3 && g_hyperspaceflag != 5)
								: (g_hyperspaceflag != ANIM_HYPERSPACE_EXTERNAL &&
								   g_hyperspaceflag != ANIM_HYPERSPACE_ARRIVE)) {
			g_flightCamera.worldPosition.x = g_objectTable[g_flightCamera.focusObjectRef].worldX;
			g_flightCamera.worldPosition.y = g_objectTable[g_flightCamera.focusObjectRef].worldY;
			g_flightCamera.worldPosition.z = g_objectTable[g_flightCamera.focusObjectRef].worldZ;
		} else {
			g_flightCamera.worldPosition.z = 0;
			g_flightCamera.worldPosition.y = 0;
			g_flightCamera.worldPosition.x = 0;
		}
		g_flightCamera.worldPosition.x =
			(int32_t)((uint32_t)g_flightCamera.worldPosition.x -
					  (uint32_t)(((int64_t)g_camMatR2_X * g_flightCamera.externalDistance) >>
								 XW_CAMERA_MATRIX_SHIFT));
		g_flightCamera.worldPosition.y =
			(int32_t)((uint32_t)g_flightCamera.worldPosition.y -
					  (uint32_t)(((int64_t)g_camMatR2_Y * g_flightCamera.externalDistance) >>
								 XW_CAMERA_MATRIX_SHIFT));
		g_flightCamera.worldPosition.z =
			(int32_t)((uint32_t)g_flightCamera.worldPosition.z -
					  (uint32_t)(((int64_t)g_camMatR2_Z * g_flightCamera.externalDistance) >>
								 XW_CAMERA_MATRIX_SHIFT));
	} else {
		g_flightCamera.viewRoll = g_objectTable[g_flightCamera.focusObjectRef].roll;
		g_flightCamera.viewPitch = g_objectTable[g_flightCamera.focusObjectRef].pitch;
		g_flightCamera.viewYaw = g_objectTable[g_flightCamera.focusObjectRef].yaw;
		fview_newcalcview(g_flightCamera.viewRoll, g_flightCamera.viewPitch, g_flightCamera.viewYaw,
						  g_flightCamera.viewAngleD, g_flightCamera.hudAimX, g_flightCamera.hudAimY,
						  &g_objectTable[g_flightCamera.focusObjectRef]);
		g_flightCamera.worldPosition.x = g_objectTable[g_flightCamera.focusObjectRef].worldX;
		g_flightCamera.worldPosition.y = g_objectTable[g_flightCamera.focusObjectRef].worldY;
		g_flightCamera.worldPosition.z = g_objectTable[g_flightCamera.focusObjectRef].worldZ;
		if (g_flightCamera.focusObjectRef == g_playerFlightState.objectIndex &&
			g_hyperspaceflag != ANIM_HYPERSPACE_DEPART && g_hyperspaceflag != ANIM_HYPERSPACE_RETURN) {
			g_flightCamera.worldPosition.x = (int32_t)((uint32_t)g_flightCamera.worldPosition.x +
													   (uint32_t)g_playerFlightState.rotatedCockpitOffset.x);
			g_flightCamera.worldPosition.y = (int32_t)((uint32_t)g_flightCamera.worldPosition.y +
													   (uint32_t)g_playerFlightState.rotatedCockpitOffset.y);
			g_flightCamera.worldPosition.z = (int32_t)((uint32_t)g_flightCamera.worldPosition.z +
													   (uint32_t)g_playerFlightState.rotatedCockpitOffset.z);
		}
	}
}
