#ifndef XW_RENDER_FLIGHT_VIEW_H
#define XW_RENDER_FLIGHT_VIEW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <aeron/compat/win_types.h>
#include <stddef.h>
#include <stdint.h>

typedef struct XwCameraAngleHistory XwCameraAngleHistory;
typedef struct XwCameraPosition XwCameraPosition;
typedef struct XwFlightCamera XwFlightCamera;

enum { FLIGHT_VIEW_MASK_EXTENDED_BASE = 255, FLIGHT_VIEW_MASK_LONG_BASE = 256 };

enum { FLIGHT_VIEW_ANGLE_HISTORY_COUNT = 60 };

/* Original IDB size: 362 bytes. */
struct XwCameraAngleHistory {
	/* IDB +0x0 */
	int16_t roll[FLIGHT_VIEW_ANGLE_HISTORY_COUNT];
	/* IDB +0x78 */
	int16_t pitch[FLIGHT_VIEW_ANGLE_HISTORY_COUNT];
	/* IDB +0xF0 */
	int16_t yaw[FLIGHT_VIEW_ANGLE_HISTORY_COUNT];
	/* IDB +0x168 */
	uint16_t writeIndex;
};

/* Original IDB size: 12 bytes. */
struct XwCameraPosition {
	/* IDB +0x0: Camera world X, signed fixed-point coordinate with 8 fractional bits. Dynamic object
	 * coordinates use same units; mission-object coordinates are multiplied by 256. */
	int x;
	/* IDB +0x4: Camera world Y, signed fixed-point coordinate with 8 fractional bits. Dynamic object
	 * coordinates use same units; mission-object coordinates are multiplied by 256. */
	int y;
	/* IDB +0x8: Camera world Z, signed fixed-point coordinate with 8 fractional bits. Dynamic object
	 * coordinates use same units; mission-object coordinates are multiplied by 256. */
	int z;
};

/* Original IDB size: 405 bytes. */
struct XwFlightCamera {
	/* IDB +0x0: Signed 24.8 world XYZ. Live camera is updated by FlightView_Render; replay position is copied
	 * into it by REPLAY_calcreplayview. Camera-relative render/audio consumers subtract these coordinates. */
	struct XwCameraPosition worldPosition;
	/* IDB +0xC: Live camera focus object; 0xFFFF selects the fixed-position/post-destruction view. Replay
	 * instance uses this as its chase object reference (dynamic object or 0x3800-based mission object). */
	uint16_t focusObjectRef;
	/* IDB +0xE: 16-bit pitch angle passed to FVIEW_BuildCameraOrient; taken from focused object, chase
	 * history or trig2 look-at output. */
	int16_t viewPitch;
	/* IDB +0x10: 16-bit yaw angle passed to FVIEW_BuildCameraOrient; taken from focused object, chase history
	 * or trig2 look-at output. */
	int16_t viewYaw;
	/* IDB +0x12: 16-bit roll angle. Replay free/tracking modes clear roll; chase mode takes the target roll.
	 */
	int16_t viewRoll;
	/* IDB +0x14: Additional angle passed as FVIEW_BuildCameraOrient.viewAngleD by the live internal-object
	 * view. No direct writer found; included in the 405-byte snapshot. More precise independent role
	 * unresolved. */
	int16_t viewAngleD;
	/* IDB +0x16: View aim offset passed as hudAimX; modified by view keys/manual camera input. Replay
	 * instance stores the chase-view offset. */
	int16_t hudAimX;
	/* IDB +0x18: View aim offset passed as hudAimY; modified by view keys/manual camera input. Replay
	 * instance stores the chase-view offset. */
	int16_t hudAimY;
	/* IDB +0x1A: Live cockpit/HUD selector. Hud_SetHudViewState writes it; render/display users consume it.
	 * No direct replay-instance access found. */
	uint8_t hudStateLive;
	/* IDB +0x1B: Written alongside hudStateLive by Hud_SetHudViewState. No direct reader found; do not infer
	 * a restore role from the duplicate write. */
	uint8_t hudStateMirror;
	/* IDB +0x1C: Live view key toggles bit 3 (XOR 8), adds this value to the HUD selector and uses value<<10
	 * for hudAimX. Reset for mission/replay transitions. */
	uint8_t rearViewHudOffset;
	/* IDB +0x1D: Saved live player cockpit selector when switching to external/other-object view; restored by
	 * Player_UpdateHudViewForCameraFocus. */
	uint8_t savedHudState;
	/* IDB +0x1E: Unidentified byte included in the serialized live camera block. No direct instruction
	 * reference found in either instance; not asserted to be padding. */
	uint8_t field_1E;
	/* IDB +0x1F: Live player hudAimX saved before a view switch and restored when returning to the player
	 * internal view. */
	int16_t savedHudAimX;
	/* IDB +0x21: Live player hudAimY saved before a view switch and restored when returning to the player
	 * internal view. */
	int16_t savedHudAimY;
	/* IDB +0x23: Live: nonzero redirects controls to camera aim/zoom and suppresses craft control/fire.
	 * Replay: nonzero selects independently moved free camera; zero follows the chase target. Both are
	 * toggled/tested as booleans. */
	uint16_t manualControlActive;
	/* IDB +0x25: Unsigned step magnitude. Live camera zoom: reset 32, increment 32, cap 1024. Replay
	 * movement/zoom: reset 64, increment 128, cap 24576; elapsed-time scaled before use. */
	uint16_t movementStep;
	/* IDB +0x27: Live: selects external chase camera and HUD 18. Replay instance is initialized to 1 by
	 * REPLAY_doreplayscreen; no replay-instance reader found, so no additional replay behavior inferred. */
	uint16_t externalViewActive;
	/* IDB +0x29: Signed distance along the backward camera basis. Initial 1280; live manual zoom clamps
	 * 768..5120, replay zoom clamps 0..5120 and adds target-size clearance. */
	int16_t externalDistance;
	/* IDB +0x2B: Live 60-sample roll/pitch/yaw ring plus write index; read five samples behind, write current
	 * focus angles, wrap at 60. Replay instance has no direct accesses to this tail; shared layout inferred
	 * from matching prefix/storage and TIE camera/replaycam lineage. */
	struct XwCameraAngleHistory angleHistory;
};

/* Declarations follow ascending original IDB address. */

/* 0x482690 */
HRESULT FlightView_CompositeMaskedSoftwareSurface(void);

/* 0x484BB0 */
void j_std3D_FlushTextureCache(void);

/* 0x49DF10 */
void SetFlightViewport(uint16_t width, uint16_t height, int unused, unsigned int byteOffset);

extern uint16_t g_flightVpHeight;
extern uint16_t g_flightVpCenterY;
extern unsigned int g_flightVpY;
extern unsigned int g_flightVpX;
extern uint16_t g_flightVpMaxX;
extern uint16_t g_flightVpMaxY;
extern uint16_t g_flightVpWidth;
extern unsigned int g_flightVpBaseOffset;
extern uint16_t g_flightVpCenterX;
extern XwFlightCamera g_flightCamera;
extern int g_projScaleHalfInt;
extern uint8_t g_projPerspectiveShift;
extern uint16_t g_projAspectY;
extern int g_projScaleInt;
extern int g_projOffsetY;

#ifdef __cplusplus
}
#endif

#endif
