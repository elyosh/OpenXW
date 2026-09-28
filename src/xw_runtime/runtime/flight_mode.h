#ifndef XW_RUNTIME_FLIGHT_MODE_H
#define XW_RUNTIME_FLIGHT_MODE_H

#include "xw_runtime/runtime/profile.h"

bool XwFlightMode_PrepareMission(char* error, size_t capacity);
bool XwFlightMode_Activate(char* error, size_t capacity);
void XwFlightMode_Deactivate(void);
/* The shell retains DOS resources while the simulator yields to a modal visit. */
bool XwFlightMode_IsSuspended(void);
void XwFlightMode_Suspend(void);
void XwFlightMode_DiscardSuspended(void);
void XwFlightMode_ReleaseMission(void);
void XwFlightMode_ReportError(const char* error);

#endif
