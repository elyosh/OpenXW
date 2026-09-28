#include "xw_app/settings/game_page.h"
#include "xw_app/settings/installation_page.h"
#include "xw_app/settings/settings.h"
#include "xw_runtime/runtime/profile.h"
#include <string.h>

static XwGameVersion VersionSelector(AeronUiContext* ui, const char* label, char* value, bool flight,
									 const char* inheritVersion) {
	static const char* const names[] = { "X-Wing 1993", "X-Wing 1994", "X-Wing 1998" };
	static const char* const keys[] = { "xw93", "xw94", "xw98" };
	static const XwGameVersion versions[] = { XW_GAME_VERSION_93, XW_GAME_VERSION_94, XW_GAME_VERSION_98 };
	const char* availableNames[4];
	int availableVersions[4];
	int count = 0, choice = 0, selected = 2;
	if (inheritVersion) {
		availableNames[count] = "Same as Cutscenes and Menus";
		availableVersions[count++] = -1;
		if (!strcmp(value, "frontend"))
			selected = -1;
	}
	/* Game folders become available after startup validation and binding. */
	for (int i = flight ? 0 : 1; i < 3; ++i) {
		bool current = !strcmp(value, keys[i]);
		if (current)
			selected = i;
		if (!XwStorage_HasInstallation(versions[i]))
			continue;
		if (flight) {
			const XwFlightProfile* profile = XwProfile_Flight(versions[i]);
			if (!profile || !XwStorage_HasInstallation(profile->mission_version))
				continue;
		}
		if (current)
			choice = count;
		availableNames[count] = names[i];
		availableVersions[count++] = i;
	}
	if (AeronUi_SelectorEnabled(ui, label, &choice, availableNames, count, count > 1)) {
		selected = availableVersions[choice];
		strcpy(value, selected < 0 ? "frontend" : keys[selected]);
	}
	if (selected < 0)
		return !strcmp(inheritVersion, "xw94") ? XW_GAME_VERSION_94 : XW_GAME_VERSION_98;
	return versions[selected];
}

void XwGamePage_Draw(AeronUiContext* ui, const AeronInputSnapshot* input) {
	XwSettings* draft = XwSettingsMenu_Draft();
	XwShellPreferences* p = &draft->game.preferences;
	if (!AeronUi_BeginScroll(ui, "Game settings", XwSettingsMenu_ScrollHeight()))
		return;
	AeronUi_Header(ui, "Version Selection");
	XwGameVersion frontend = VersionSelector(ui, "Cutscenes and Menus", draft->frontend_version, false, NULL);
	if (frontend != XwProfile_Frontend()->version)
		AeronUi_Help(ui, "Restart OpenXW to use the selected cutscenes and menus.");
	const char* activeFrontend = XwProfile_Frontend()->version == XW_GAME_VERSION_94 ? "xw94" : "xw98";
	XwGameVersion inflight =
		VersionSelector(ui, "In-flight Menus", draft->inflight_frontend_version, false, activeFrontend);
	if (inflight != XwProfile_InflightFrontend()->version)
		AeronUi_Help(ui, "In-flight menu changes apply when you next open mission selection.");
	XwGameVersion requested = VersionSelector(ui, "Flight Engine", draft->flight_version, true, NULL);
	static const char* const flightRates[] = { "Native", "Unlocked" };
	int updateRate = draft->flight_update_rate;
	if (AeronUi_Selector(ui, "Flight Update Rate", &updateRate, flightRates, 2))
		draft->flight_update_rate = (XwFlightUpdateRate)updateRate;
	if (XwProfile_MissionFlight()->version != requested ||
		XwProfile_MissionFlightRate() != draft->flight_update_rate)
		AeronUi_Help(ui, "Flight engine and rate changes apply when you next open mission selection.");
	AeronUi_Header(ui, "Startup");
	AeronUi_Toggle(ui, "Skip intro on launch", &draft->skip_intro);
	AeronUi_Header(ui, "Scenes and missions");
	int value = p->spokenTextEnabled;
	if (AeronUi_Toggle(ui, "Show spoken text", &value))
		p->spokenTextEnabled = (int16_t)value;
	value = p->transitionsEnabled;
	if (AeronUi_Toggle(ui, "Show transitions", &value))
		p->transitionsEnabled = (int16_t)value;
	if (requested != XW_GAME_VERSION_93) {
		value = p->classicMissions;
		if (AeronUi_Toggle(ui, "Classic missions", &value))
			p->classicMissions = (uint8_t)value;
	}
	AeronUi_Header(ui, "Flight Gameplay");
	static const char* const off_on[] = { "Off", "On" };
	static const char* const vulnerability[] = { "Vulnerable", "Invulnerable" };
	static const char* const ammunition[] = { "Limited", "Unlimited" };
	AeronUi_Selector(ui, "Starfighter Collision Damage", &draft->starfighter_collision_damage, off_on, 2);
	AeronUi_Selector(ui, "Player Spacecraft", &draft->player_invulnerable, vulnerability, 2);
	AeronUi_Selector(ui, "Ammunition", &draft->unlimited_ammunition, ammunition, 2);
	XwInstallationPage_Draw(ui, input);
	AeronUi_EndScroll(ui);
}
