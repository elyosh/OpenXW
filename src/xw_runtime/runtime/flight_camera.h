#ifndef XW_RUNTIME_FLIGHT_CAMERA_H
#define XW_RUNTIME_FLIGHT_CAMERA_H
#include "xw/render/flight_view.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct XwChaseTimingState {
	uint64_t ticks[FLIGHT_VIEW_ANGLE_HISTORY_COUNT];
	uint64_t serial, last_ticks;
	uint16_t focus;
	bool valid;
} XwChaseTimingState;

void XwFlightCamera_SaveTiming(XwChaseTimingState* out);
void XwFlightCamera_RestoreTiming(const XwChaseTimingState* state);
/* Graphics-only polar conversion; shared flight-control/AI math keeps its own entry. */
void XwFlightCamera_CartesianToPolar(int32_t x, int32_t y, int32_t z);
void XwFlightCamera_Update(void);
/* World/control lifecycle resets; presentation-only rebuilds preserve chase history. */
void XwFlightCamera_ResetChase(void);
void XwFlightCamera_ResetObject(unsigned slot);
#endif
