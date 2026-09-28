#ifndef XW_RUNTIME_RUNTIME_FLIGHT_INPUT_H
#define XW_RUNTIME_RUNTIME_FLIGHT_INPUT_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum XwFlightInputPhase {
	XW_INPUT_BEGIN,
	XW_INPUT_PAUSE,
	XW_INPUT_AFTER_REPLAY,
	XW_INPUT_RECORD,
	XW_INPUT_ACTION
} XwFlightInputPhase;

enum { XW_INPUT_THROTTLE_PRESENT = 1 };

typedef struct XwFlightInput {
	XwFlightInputPhase phase;
	int16_t savedMusicVolume;
	uint8_t flags;
	uint16_t throttle;
} XwFlightInput;

/* Service any child tasks before resuming. Returns 0 while suspended. */
int XwFlightInput_Tick(XwFlightInput* state);
void XwFlightInput_Cancel(XwFlightInput* state);
#ifdef __cplusplus
}
#endif
#endif
