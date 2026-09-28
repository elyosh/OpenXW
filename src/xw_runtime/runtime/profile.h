#ifndef XW_RUNTIME_PROFILE_H
#define XW_RUNTIME_PROFILE_H

#include "xw_runtime/storage/storage.h"

typedef enum XwFlightUpdateRate {
	XW_FLIGHT_UPDATE_RATE_NATIVE = 0,
	XW_FLIGHT_UPDATE_RATE_UNLOCKED = 1
} XwFlightUpdateRate;

typedef struct XwFrontendProfile {
	XwGameVersion version;
	int width, height, font_count;
} XwFrontendProfile;

typedef struct XwFlightProfile {
	XwGameVersion version;
	XwGameVersion mission_version;
	/* Host delivery interval; game-time arithmetic still uses 236 ticks/second. */
	uint16_t tick_period_us;
	uint16_t minimum_frame_ticks;
	uint16_t model_count, craft_definition_count, component_count;
	uint16_t object_count, static_object_count;
} XwFlightProfile;

/* Owner thread only. A pin survives flight cleanup through debrief and retry. */
/* Immutable revision data; NULL for an unsupported version. */
const XwFlightProfile* XwProfile_Flight(XwGameVersion version);
const XwFlightProfile* XwProfile_RequestedFlight(void);
const XwFlightProfile* XwProfile_MissionFlight(void);
const XwFlightProfile* XwProfile_ActiveFlight(void);
XwFlightUpdateRate XwProfile_RequestedFlightRate(void);
XwFlightUpdateRate XwProfile_MissionFlightRate(void);
XwFlightUpdateRate XwProfile_ActiveFlightRate(void);
bool XwProfile_HasMission(void);
bool XwProfile_HasActiveFlight(void);
bool XwProfile_MissionClassic(void);
/* Recorded content selection may be restored before activation; an active mission is immutable. */
bool XwProfile_RestoreMissionContent(bool classic);
/* Only validated recording metadata may replace the pinned timing policy. */
bool XwProfile_RestoreMissionTiming(XwFlightUpdateRate rate);
bool XwProfile_PinMission(XwGameVersion version, char* error, size_t capacity);
bool XwProfile_PinRequestedMission(char* error, size_t capacity);
bool XwProfile_ActivateFlight(char* error, size_t capacity);
void XwProfile_DeactivateFlight(void);
bool XwProfile_ReleaseMission(void);
bool XwProfile_DosFlight(void);

/* Ordinary frontend selection is a launch setting, as in OpenTIE. */
bool XwProfile_Init(char* error, size_t capacity);
const XwFrontendProfile* XwProfile_Frontend(void);
/* Pinned with the mission; otherwise resolves the current requested choice. */
const XwFrontendProfile* XwProfile_InflightFrontend(void);
/* Select before opening Landru; restore the ordinary frontend after closing it. */
void XwProfile_SelectFrontend(bool inflight);
const XwFrontendProfile* XwProfile_ActiveFrontend(void);
bool XwProfile_DosFrontend(void);

#endif
