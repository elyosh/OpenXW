#ifndef XW_RUNTIME_TIMING_FLIGHT_INTEGRATION_H
#define XW_RUNTIME_TIMING_FLIGHT_INTEGRATION_H

#include "xw/flight/object/create.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	XW_INTEGRATE_IMPULSE_ROLL,
	XW_INTEGRATE_PUSH_X,
	XW_INTEGRATE_PUSH_Y,
	XW_INTEGRATE_PUSH_Z,
	XW_INTEGRATE_HOME_YAW,
	XW_INTEGRATE_HOME_PITCH,
	XW_INTEGRATE_ROLL,
	XW_INTEGRATE_PITCH,
	XW_INTEGRATE_YAW,
	XW_INTEGRATE_BANK,
	XW_INTEGRATE_VELOCITY,
	XW_INTEGRATE_COUNT
};

typedef struct XwIntegrationCarry {
	int64_t remainder;
	int8_t direction;
} XwIntegrationCarry;

typedef struct XwMobileIntegration {
	int64_t position[3];
	XwIntegrationCarry channels[XW_INTEGRATE_ROLL];
	uint16_t target;
	uint8_t tier;
} XwMobileIntegration;

typedef struct XwFlightIntegrationState {
	XwMobileIntegration mobile[XW_OBJECT_COUNT];
	XwIntegrationCarry craft[XW_CRAFT_OBJECT_COUNT][XW_INTEGRATE_COUNT - XW_INTEGRATE_ROLL];
} XwFlightIntegrationState;

void XwFlightIntegration_Save(XwFlightIntegrationState* out);
/* Install only state checked by the snapshot codec. */
void XwFlightIntegration_Restore(const XwFlightIntegrationState* state);

/* Flight-owner thread only. Craft carry belongs to the mobile slot, never instanceData. */
void XwFlightIntegration_ClearAll(void);
void XwFlightIntegration_Reset(unsigned slot);
void XwFlightIntegration_Reposition(unsigned slot);
void XwFlightIntegration_ResetManeuver(unsigned slot);
void XwFlightIntegration_ClearPush(unsigned slot);
void XwFlightIntegration_Clear(unsigned slot, unsigned channel);
void XwFlightIntegration_Observe(unsigned slot);
void XwFlightIntegration_ObserveCraft(unsigned slot);
int XwFlightIntegration_Rate(unsigned slot, unsigned channel, int rate, unsigned elapsed, int divisor);
unsigned XwFlightIntegration_Steer(unsigned slot, unsigned channel, uint16_t rate, uint16_t accel,
								   uint16_t factor, int direction);
unsigned XwFlightIntegration_Bank(unsigned slot, uint16_t step, uint16_t factor, int direction);
void XwFlightIntegration_ChangeVelocity(unsigned slot, uint16_t rate, int direction);
void XwFlightIntegration_Move(unsigned slot);
void XwFlightIntegration_ClampPosition(unsigned slot);
void XwFlightIntegration_Push(unsigned slot, unsigned axis, int* remaining, int cap, int* output);
int16_t XwFlightIntegration_Home(unsigned slot, unsigned channel, int16_t current, int16_t target,
								 uint16_t rate);

#ifdef __cplusplus
}
#endif
#endif
