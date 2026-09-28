#include "xw_runtime/runtime/profile.h"
#include "xw_runtime/config/config.h"
#include "xw_runtime/runtime/flight_types.h"
#include <stdio.h>
#include <string.h>

static const XwFrontendProfile profiles[] = {
	{ XW_GAME_VERSION_94, 320, 200, 2 },
	{ XW_GAME_VERSION_98, 640, 480, 4 },
};
static const XwFrontendProfile* frontend = &profiles[1];
static const XwFrontendProfile* inflightFrontend;
static const XwFrontendProfile* activeFrontend = &profiles[1];

static const XwFlightProfile flight_profiles[] = {
	{ XW_GAME_VERSION_94, XW_GAME_VERSION_94, 4000, 4, 120, 19, 16, 116, 64 },
	{ XW_GAME_VERSION_98, XW_GAME_VERSION_98, 4000, 8, 211, 69, 50, 116, 64 },
	{ XW_GAME_VERSION_93, XW_GAME_VERSION_93, 4237, 4, 120, 19, 16, 116, 64 },
};
static const XwFlightProfile* mission;
static const XwFlightProfile* active;
static bool missionClassic;
static bool missionLaserConvergence;
static XwFlightUpdateRate missionRate;

static const XwFrontendProfile* requested_inflight_frontend(void) {
	const char* version = XwConfig_Settings()->inflight_frontend_version;
	return !strcmp(version, "frontend") ? frontend : &profiles[!strcmp(version, "xw94") ? 0 : 1];
}

const XwFlightProfile* XwProfile_Flight(XwGameVersion version) {
	for (size_t i = 0; i < sizeof flight_profiles / sizeof flight_profiles[0]; ++i)
		if (flight_profiles[i].version == version)
			return &flight_profiles[i];
	return NULL;
}

bool XwProfile_Init(char* error, size_t capacity) {
	const XwSettings* settings = XwConfig_Settings();
	mission = active = NULL;
	inflightFrontend = NULL;
	missionRate = XW_FLIGHT_UPDATE_RATE_NATIVE;
	XwFlightTypes_Init();
	frontend = &profiles[!strcmp(settings->frontend_version, "xw94") ? 0 : 1];
	activeFrontend = frontend;
	if (!XwStorage_HasInstallation(frontend->version)) {
		snprintf(error, capacity, "The selected X-Wing %d frontend requires its installation folder.",
				 XwGameVersion_Year(frontend->version));
		return false;
	}
	const XwFrontendProfile* requested = requested_inflight_frontend();
	if (!XwStorage_HasInstallation(requested->version)) {
		snprintf(error, capacity, "The selected X-Wing %d in-flight menus require its installation folder.",
				 XwGameVersion_Year(requested->version));
		return false;
	}
	return true;
}

const XwFrontendProfile* XwProfile_Frontend(void) { return frontend; }

const XwFrontendProfile* XwProfile_InflightFrontend(void) {
	return mission ? inflightFrontend : requested_inflight_frontend();
}

void XwProfile_SelectFrontend(bool inflight) {
	activeFrontend = inflight ? XwProfile_InflightFrontend() : frontend;
}

const XwFrontendProfile* XwProfile_ActiveFrontend(void) { return activeFrontend; }

bool XwProfile_DosFrontend(void) { return activeFrontend->version == XW_GAME_VERSION_94; }

const XwFlightProfile* XwProfile_RequestedFlight(void) {
	const char* version = XwConfig_Settings()->flight_version;
	if (!strcmp(version, "xw93"))
		return XwProfile_Flight(XW_GAME_VERSION_93);
	if (!strcmp(version, "xw94"))
		return XwProfile_Flight(XW_GAME_VERSION_94);
	return XwProfile_Flight(XW_GAME_VERSION_98);
}

const XwFlightProfile* XwProfile_MissionFlight(void) {
	return mission ? mission : XwProfile_RequestedFlight();
}

const XwFlightProfile* XwProfile_ActiveFlight(void) {
	return active ? active : XwProfile_Flight(XW_GAME_VERSION_98);
}

XwFlightUpdateRate XwProfile_RequestedFlightRate(void) { return XwConfig_Settings()->flight_update_rate; }

XwFlightUpdateRate XwProfile_MissionFlightRate(void) {
	return mission ? missionRate : XwProfile_RequestedFlightRate();
}

XwFlightUpdateRate XwProfile_ActiveFlightRate(void) {
	return active ? missionRate : XW_FLIGHT_UPDATE_RATE_NATIVE;
}

bool XwProfile_HasMission(void) { return mission != NULL; }

bool XwProfile_HasActiveFlight(void) { return active != NULL; }

bool XwProfile_MissionClassic(void) {
	if (XwProfile_MissionFlight()->version == XW_GAME_VERSION_93)
		return false;
	return mission ? missionClassic : XwConfig_Settings()->game.preferences.classicMissions != 0;
}

bool XwProfile_MissionLaserConvergence(void) {
	return mission ? missionLaserConvergence : XwConfig_Settings()->laser_convergence != 0;
}

bool XwProfile_RestoreMissionLaserConvergence(bool enabled) {
	if (!mission)
		return false;
	missionLaserConvergence = enabled;
	return true;
}

bool XwProfile_DosFlight(void) { return XwGameVersion_IsDos(XwProfile_ActiveFlight()->version); }

bool XwProfile_PinMission(XwGameVersion version, char* error, size_t capacity) {
	const XwFlightProfile* profile = XwProfile_Flight(version);
	if (!profile) {
		snprintf(error, capacity, "Unsupported flight version.");
		return false;
	}
	if (mission && mission->version == version)
		return true;
	if (mission || active) {
		snprintf(error, capacity, "The prepared mission owns its flight version until mission selection.");
		return false;
	}
	if (!XwStorage_HasInstallation(version)) {
		snprintf(error, capacity, "X-Wing %d flight requires its installation folder.",
				 XwGameVersion_Year(version));
		return false;
	}
	const XwFrontendProfile* menus = requested_inflight_frontend();
	if (!XwStorage_HasInstallation(menus->version)) {
		snprintf(error, capacity, "X-Wing %d in-flight menus require its installation folder.",
				 XwGameVersion_Year(menus->version));
		return false;
	}
	/* Keep presentation stable across pause/resume and retries of this mission. */
	inflightFrontend = menus;
	mission = profile;
	missionRate = XwProfile_RequestedFlightRate();
	missionLaserConvergence = XwConfig_Settings()->laser_convergence != 0;
	missionClassic =
		version != XW_GAME_VERSION_93 && XwConfig_Settings()->game.preferences.classicMissions != 0;
	return true;
}

bool XwProfile_PinRequestedMission(char* error, size_t capacity) {
	return mission || XwProfile_PinMission(XwProfile_RequestedFlight()->version, error, capacity);
}

bool XwProfile_ActivateFlight(char* error, size_t capacity) {
	if (active) {
		snprintf(error, capacity, "A flight task already owns the active profile.");
		return false;
	}
	if (!XwProfile_PinRequestedMission(error, capacity))
		return false;
	active = mission;
	return true;
}

void XwProfile_DeactivateFlight(void) { active = NULL; }

bool XwProfile_ReleaseMission(void) {
	if (active)
		return false;
	mission = NULL;
	inflightFrontend = NULL;
	missionRate = XW_FLIGHT_UPDATE_RATE_NATIVE;
	return true;
}

bool XwProfile_RestoreMissionContent(bool classic) {
	if (!mission || (mission->version == XW_GAME_VERSION_93 && classic) ||
		(active && missionClassic != classic))
		return false;
	missionClassic = classic;
	return true;
}

bool XwProfile_RestoreMissionTiming(XwFlightUpdateRate rate) {
	if (!mission || (rate != XW_FLIGHT_UPDATE_RATE_NATIVE && rate != XW_FLIGHT_UPDATE_RATE_UNLOCKED))
		return false;
	missionRate = rate;
	return true;
}
