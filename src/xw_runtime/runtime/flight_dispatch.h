#ifndef XW_RUNTIME_FLIGHT_DISPATCH_H
#define XW_RUNTIME_FLIGHT_DISPATCH_H
#include <stdint.h>
/* Coarse boundaries used by the native task owners and shared simulation. */
int16_t XwFlightMode_OpenMission(const char* path);
void XwFlightMode_ResourceError(const char* path, const char* reason);

void XwFlightMode_InitBuffers(void);
void XwFlightMode_FreeResources(void);
void XwFlightMode_LoadSounds(void);
void XwFlightMode_UnloadSounds(void);
void XwFlightMode_LoadResources(void);
void XwFlightMode_UpdateScreen(void);
void XwFlightMode_LoadPanel(void);
void XwFlightMode_SetPanelView(uint16_t hudViewState);
void XwFlightMode_ApplyPanelView(uint16_t hudViewState);
void XwFlightMode_UpdateCourseBonus(void);
void XwFlightMode_UpdateAnimation(void);

#endif
