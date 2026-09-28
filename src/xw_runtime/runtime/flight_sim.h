#ifndef XW_RUNTIME_RUNTIME_FLIGHT_SIM_H
#define XW_RUNTIME_RUNTIME_FLIGHT_SIM_H
#include "xw/flight/xw.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Push the lifecycle task; its caller resumes after the task ends. */
void XwFlightSim_Begin(FlightEntryMode entryMode);
int XwFlightSim_IsActive(void);
int XwFlightSim_IsPlayerControl(void);
int XwFlightSim_IsPaused(void);
#ifdef __cplusplus
}
#endif
#endif
