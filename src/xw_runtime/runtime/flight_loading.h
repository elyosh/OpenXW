#ifndef XW_RUNTIME_FLIGHT_LOADING_H
#define XW_RUNTIME_FLIGHT_LOADING_H
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

/* Host-thread handoff: resident resources, first completed view, then reveal. */
void XwFlightLoading_Begin(void);
void XwFlightLoading_End(void);
bool XwFlightLoading_Active(void);
bool XwFlightLoading_ResourcesReady(void);
void XwFlightLoading_SetResourcesReady(bool ready);
void XwFlightLoading_ViewCompleted(void);
bool XwFlightLoading_Waiting(void);
/* Cover flight initialization until the requested renderer has a complete view. */
void XwFlightLoading_Submit(void);
#ifdef __cplusplus
}
#endif
#endif
