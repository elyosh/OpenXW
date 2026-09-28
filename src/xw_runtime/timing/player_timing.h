#ifndef XW_RUNTIME_TIMING_PLAYER_TIMING_H
#define XW_RUNTIME_TIMING_PLAYER_TIMING_H
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	XW_PLAYER_SLEW_ROLL,
	XW_PLAYER_SLEW_PITCH,
	XW_PLAYER_ROLL,
	XW_PLAYER_PITCH,
	XW_PLAYER_CAMERA_YAW,
	XW_PLAYER_CAMERA_PITCH,
	XW_PLAYER_ZOOM_ACCELERATION,
	XW_PLAYER_ZOOM_DISTANCE,
	XW_PLAYER_BASE_TIMING_CHANNELS,
	XW_PLAYER_SLEW_ANALOG_ROLL = XW_PLAYER_BASE_TIMING_CHANNELS,
	XW_PLAYER_ANALOG_ROLL,
	XW_PLAYER_TIMING_CHANNELS
};

typedef struct XwPlayerTimingState {
	int64_t remainder[XW_PLAYER_TIMING_CHANNELS];
	int8_t direction[XW_PLAYER_TIMING_CHANNELS];
	uint16_t slot, craft_type, focus;
	uint32_t mode;
	bool valid;
	int16_t analog_roll;
} XwPlayerTimingState;

typedef struct XwRecoveryTimingState {
	int32_t position[3];
	uint64_t serial;
	uint16_t slot;
	bool valid;
} XwRecoveryTimingState;

void XwPlayerTiming_Save(XwPlayerTimingState* out, XwRecoveryTimingState* pose);
void XwPlayerTiming_Restore(const XwPlayerTimingState* state, const XwRecoveryTimingState* pose);
void XwPlayerTiming_RestoreRoll(const XwPlayerTimingState* state);
void XwPlayerTiming_ResetControls(void);
/* Preserve simulation carry while physical input is reacquired after a restore or reentry. */
void XwPlayerTiming_ResumeControls(void);

/* Flight-owner thread; physical throttle filtering remains in the input adapter. */
void XwPlayerTiming_Reset(void);
void XwPlayerTiming_ResetObject(unsigned slot);
void XwPlayerTiming_RepositionObject(unsigned slot);
/* Course history is world state: ordinary control resets must preserve it. */
void XwPlayerTiming_ResetRecovery(void);
bool XwPlayerTiming_RecordRecovery(int32_t position[3]);
void XwPlayerTiming_BeginControls(void);
int16_t XwPlayerTiming_Slew(unsigned channel, int16_t current, int16_t target);
int XwPlayerTiming_Scale(unsigned channel, int value);
int16_t XwPlayerTiming_RollStep(int16_t input);
void XwPlayerTiming_ManualCamera(void);

#ifdef __cplusplus
}
#endif
#endif
