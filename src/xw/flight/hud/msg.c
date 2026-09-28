#include "xw/flight/hud/msg.h"

#ifdef XW_MODERN
#include "xw_runtime/snapshot/render_hud.h"
#endif
#include "xw/audio/fsfx.h"
#include "xw/flight/ai/pai.h"
#include "xw/flight/feinput.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/hud/festring.h"
#include "xw/flight/hud/panel.h"
#include "xw/flight/hud/panelrts.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/object.h"
#include "xw/flight/player/user.h"
#include "xw/flight/xw.h"
#include "xw/math/math2.h"
#include "xw/math/trig2.h"
#include "xw/util/shared.h"

#include <stdlib.h>

// GLOBAL: XW 0x4C8BA0
const char* const g_flightMessageTemplates[XW_MSG_TEMPLATE_COUNT] = {
	"Xwing95 v1.0 5/27/98",
	"Mission paused.  Press any key to continue",
	"Mission resumed",
	"\005Laser cannons armed",
	"\005Ion cannons armed",
	"\005* launchers armed",
	"\005* launchers set to \007single fire",
	"\005* launchers set to \007dual fire",
	"Proton torpedo",
	"Concussion missile",
	"\005Cannons set to \007single fire",
	"\005Cannons set to \007dual fire",
	"\005Cannons \007fire linked",
	"\005Deflector shields set *",
	"\007full strength forward",
	"\007even strength forward and aft",
	"\007full strength aft",
	"\005Laser* \005rate",
	"\005Shield* \005rate",
	" power redirected to engines at \007maximum",
	" power redirected to engines at \007minimum",
	"s recharging at \007normal",
	"s recharging at \007increased",
	"s recharging at \007maximum",
	"\007* \001system is *",
	"damaged and inoperative",
	"repaired",
	"Engine",
	"Flight control",
	"Laser",
	"Ion Cannon",
	"Proton Torpedo Launcher",
	"Concussion Missile Launcher",
	"Targeting computer",
	"Auto-ejection system",
	"Deflector shield",
	"Hyperdrive",
	"Graphics detail set to \001* \011level",
	"lowest",
	"low",
	"high",
	"highest",
	"\005Preparing for jump to light speed..",
	"\005Hyperspace jump aborted",
	"\005Hyperspace jump completed",
	"\001Sensors detect \007&\001 \001new \007* \001at \007&\002 \001KM",
	"\001Sensors detect \007&\001 \001new \007*s \001at \007&\002 \001KM",
	"                     ",
	"Congratulations! Level Completed.  Time Left:            Bonus:",
	"Bonus points awarded: &\005",
	"\013An enemy craft is attempting missile lock, \007should it be targeted?",
	"\013Warning! A missile has been launched at us, \007should it be targeted?",
	"\001Missile was targeted",
	"\014Object detected in jump path, hyperspace jump aborted",
	"Replay camera \001on",
	"Replay camera \001off",
	"Error! Failed to start camera",
	"Camera footage is being saved to disk",
	"\007Camera footage saved",
	"\007Camera film used up",
	"\007Loading camera footage from disk",
	"No film recorded",
	"At end of film",
	"Film started",
	"Film stopped",
	"Film advance on",
	"Film advance off",
	"Film rewound to start",
	"Camera now in \001FOLLOW \011mode",
	"Camera now in \001FREE \011mode",
	"\001Mission ends in\007 2 \001minutes",
	"\001Mission ends in\007 1 \001minute",
	"\001Mission time expired, hyperspace sequence begun..",
	"\001Hangar tractor beam activated. \007Do you want to end your mission?",
	"\001Mission objectives not accomplished, return to base",
	"\007Enter file name:",
	"\014File Exists! Replace it?",
	"\007Replay clip saved",
	"\014Replay clip not saved",
	"\014File Error!",
	"\005Engine throttle set to\007 *",
	"no power",
	"1/3 power",
	"2/3 power",
	"full power",
	"\005Combat multi-view display set to \007* \005mode",
	"targeting",
	"identification",
	"\005S-Foils opening",
	"\005S-Foils closing",
	"\005S-Foils have reached \007* \005position",
	"open",
	"closed",
	"\005Transferring partial power from shields to \007laser system",
	"\005Transferring partial power from lasers to \007shields",
	"\001\007 * * &\001 \001*",
	"\001\007 * * \001*",
	"has entered hyperspace",
	"has been \007destroyed",
	"has been \007disabled",
	"has been \007repaired",
	"has been \007captured",
	"has \007docked",
	"has been targeted",
	"\007 * &\001 \001from flight group\007 *\001 *",
	"\007 * * \001*",
	"\005* launcher empty",
	"\005* fired",
	"\005*s fired",
	"has entered hangar",
	"\003Message acknowledged:\007 * * &\001 \003*",
	"\003Message acknowledged:\007 * * \003*",
	"heading home",
	"coming to your aid",
	"making evasive maneuver",
	"waiting for further orders",
	"proceeding with mission",
	"using designated target",
	"ignoring target",
	"reports docking operation complete",
	"\003\007 * * &\001 \003reporting in: *",
	"\003\007 * * \003reporting in: *",
	"holding steady",
	"leading formation flight",
	"following in formation",
	"patrolling for enemy",
	"patrolling for escorters",
	"patrolling for transports",
	"patrolling for freighters",
	"patrolling for starships",
	"patrolling for satellites",
	"setting up for attack run",
	"attacking target",
	"doing evasive maneuver",
	"following flight leader",
	"looking for enemy to disable",
	"looking for transports to disable",
	"looking for freighters to disable",
	"looking for starships to disable",
	"flying close escort",
	"flying loose escort",
	"following close escort",
	"following loose escort",
	"looking for craft to board",
	"looking for craft to capture",
	"doing docking operation",
	"flying to rendezvous point",
	"awaiting boarding craft",
	"disabled, awaiting rescue",
	"flying home",
	"entering hangar",
	"exiting hangar",
	"entering hyperspace",
	"exiting hyperspace",
	"holding station",
	"patrolling area",
	"waiting for craft to return",
	"waiting for craft to launch",
	"waiting for docking craft",
	"awaiting further orders",
	"\001Mission complete",
	"\001*",
	"\001You have ejected safely",
	"&\0010000 points awarded for previous levels",
	"\014Cannot enter simulation at this point!",
	"Launcher",
	"Cannon",
	"\013You have died",
	"\013Direct Hit on the Exhaust Port!!",
	"Entering hyperspace",
	"????",
	"has been identified",
	"\003Matching speed with target",
	"\003Trying to match speeds with target, throttle set to maximum",
	"\003Saving film to disk.",
};

// GLOBAL: XW 0x637310
uint16_t g_msgArgTable[XW_MSG_ARGUMENT_COUNT] = { 0 };

// GLOBAL: XW 0x63ADCE
uint8_t g_messageDynamicStringCursor = 0;

// GLOBAL: XW 0x63ADE0
XwHudInFlightMessageRecord g_readyMessagePaneQueue[XW_MSG_READY_QUEUE_CAPACITY] = { { 0 } };

// GLOBAL: XW 0x63B1BE
uint8_t g_readyMessageQueueCount = 0;

// GLOBAL: XW 0x63B1C0
const char* g_messageDynamicStrings[XW_MSG_DYNAMIC_STRING_COUNT] = { NULL };

// FUNCTION: XW 0x411CF0
void msg_messageinit(void) {
#ifdef XW_MODERN
	int hud_pane = XwHud_Push(XW_SNAP_PANE_READY_MESSAGE);
#endif

	int stripTop;
	festring_setlinewrap(0);
	festring_setautofill(0);
	festring_setfontsize(FLIGHT_FONT_TINY);
	FlightDisplay_LockSurface();
	stripTop = g_flightScreenWidth == FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH ? XW_MSG_STRIP_LOW_RESOLUTION_TOP
																		  : XW_MSG_STRIP_HIGH_RESOLUTION_TOP;
	festring_setbound(0, stripTop - 1, g_flightScreenWidth, stripTop);
	festring_setbackcolor(XW_MSG_BORDER_COLOR);
	g_flightFillClipRectFn();
	festring_setbackcolor(XW_MSG_BACKGROUND_COLOR);
	festring_setbound(0,
					  g_flightScreenWidth == FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH
						  ? XW_MSG_STRIP_LOW_RESOLUTION_TOP
						  : XW_MSG_STRIP_HIGH_RESOLUTION_TOP,
					  g_flightScreenWidth, g_flightScreenHeight);
	g_flightFillClipRectFn();
	nullsub_SharedNoOp();
	festring_settextcolor(XW_MSG_TEXT_COLOR);
	g_readyMessagePaneQueue[0].stateOrMessageId = XW_MSG_INACTIVE;
	FlightDisplay_UnlockSurface();

#ifdef XW_MODERN
	XwHud_Pop(hud_pane);
#endif
}

// FUNCTION: XW 0x411DB0
void msg_messageprintf(XwFlightMessageId messageId) {
	const char* messageTemplate;
	unsigned int formatIndex;
	char formatCharacter;
	int argumentIndex = 0;
	int textLength = 0;
#ifdef XW_MODERN
	XwHudInFlightMessageRecord message = { 0 };
#else
	XwHudInFlightMessageRecord message;
#endif
	message.stateOrMessageId = messageId;
	message.clockHours = g_missionElapsedClock.hours;
	messageTemplate = g_flightMessageTemplates[messageId];
	message.clockSubsecondTicks = g_missionElapsedClock.subsecondTicks;
	message.clockSeconds = g_missionElapsedClock.seconds;
	message.clockMinutes = g_missionElapsedClock.minutes;
	message.field_08 = 0;
	message.field_09 = 1;
	for (formatIndex = 0; (formatCharacter = messageTemplate[formatIndex]) != 0 &&
						  (uint16_t)textLength < sizeof(message.text);) {
		if (formatCharacter == '*') {
			uint16_t token = g_msgArgTable[(uint16_t)argumentIndex++];
			const char* substitution;
			unsigned int substitutionIndex;
			char substitutionCharacter;
			++formatIndex;
			if (token >= XW_MSG_DYNAMIC_STRING_FLAG)
				substitution = g_messageDynamicStrings[token & (XW_MSG_DYNAMIC_STRING_FLAG - 1)];
			else
				substitution = g_flightMessageTemplates[token];
			for (substitutionIndex = 0; (substitutionCharacter = substitution[substitutionIndex]) != 0 &&
										(uint16_t)textLength < sizeof(message.text);
				 ++substitutionIndex)
				message.text[(uint16_t)textLength++] = substitutionCharacter;
		} else if (formatCharacter == '&') {
			uint16_t digitsRemaining = (uint8_t)messageTemplate[formatIndex + 1];
			uint16_t remainingValue = g_msgArgTable[(uint16_t)argumentIndex++];
			int16_t hasSignificantDigit = 0;
			formatIndex += 2;
			for (; digitsRemaining > 0; --digitsRemaining) {
				uint16_t divisor = g_flightTextDecimalDivisors[digitsRemaining];
				int digit = remainingValue / divisor;
				int character;
				remainingValue -= digit * divisor;
				if (hasSignificantDigit != 0 || digitsRemaining <= 1 || (uint16_t)digit != 0) {
					hasSignificantDigit = 1;
					if ((uint16_t)digit > PANELRTS_DECIMAL_MAX_DIGIT)
						digit = PANELRTS_DECIMAL_MAX_DIGIT;
					character = digit + '0';
				} else {
					character = ' ';
				}
#ifdef XW_MODERN
				if ((uint16_t)textLength < sizeof(message.text))
					message.text[(uint16_t)textLength] = (char)character;
				++textLength;
#else
				message.text[(uint16_t)textLength++] = (char)character;
#endif
			}
		} else {
			message.text[(uint16_t)textLength++] = formatCharacter;
			++formatIndex;
		}
	}
#ifdef XW_MODERN
	if ((uint16_t)textLength < sizeof(message.text))
#endif
		message.text[(uint16_t)textLength] = 0;
	if (messageTemplate[0] == '\r')
		message.paneType = XW_MSG_PANE_LONG;
	else
		message.paneType = messageTemplate[0] == XW_MSG_PANE_MEDIUM;
	if (g_readyMessagePaneQueue[0].stateOrMessageId != XW_MSG_INACTIVE) {
		switch (g_readyMessagePaneQueue[0].paneType) {
			case XW_MSG_PANE_DEFAULT:
				break;
			case XW_MSG_PANE_MEDIUM:
				if (message.paneType == XW_MSG_PANE_MEDIUM) {
					uint8_t pendingCount = g_readyMessageQueueCount;
					unsigned int queueIndex = pendingCount;
					++pendingCount;
					g_readyMessageQueueCount = pendingCount;
					g_readyMessagePaneQueue[queueIndex + 1] = message;
					if (pendingCount >= XW_MSG_MAX_PENDING_MESSAGES + 1)
						g_readyMessageQueueCount = pendingCount - 1;
					g_messageDynamicStringCursor = 0;
					return;
				}
				msg_movecurrentmessageinqueue();
				break;
			case XW_MSG_PANE_LONG:
				if (message.paneType == XW_MSG_PANE_DEFAULT)
					return;
				if (message.paneType == XW_MSG_PANE_MEDIUM) {
					uint8_t pendingCount = g_readyMessageQueueCount;
					unsigned int queueIndex = pendingCount;
					++pendingCount;
					g_readyMessageQueueCount = pendingCount;
					g_readyMessagePaneQueue[queueIndex + 1] = message;
					if (pendingCount >= XW_MSG_MAX_PENDING_MESSAGES + 1)
						g_readyMessageQueueCount = pendingCount - 1;
					g_messageDynamicStringCursor = 0;
					return;
				}
				break;
			default:
				g_messageDynamicStringCursor = 0;
				return;
		}
	}
	g_readyMessagePaneQueue[0] = message;
	msg_messagedisplay();
	g_messageDynamicStringCursor = 0;
}

// FUNCTION: XW 0x412090
void msg_movecurrentmessageinqueue(void) {
	uint8_t pendingCount = g_readyMessageQueueCount;
	uint16_t recordsToShift = pendingCount + 1;
	uint16_t index;
	if (recordsToShift > 0) {
		for (index = recordsToShift; index > 0; --index) {
			g_readyMessagePaneQueue[index] = g_readyMessagePaneQueue[index - 1];
		}
	}
	++pendingCount;
	g_readyMessageQueueCount = pendingCount;
	if (pendingCount >= XW_MSG_MAX_PENDING_MESSAGES + 1) {
		g_readyMessageQueueCount = pendingCount - 1;
	}
}

// FUNCTION: XW 0x4120F0
void msg_getmessagefromqueue(void) {
	uint16_t count = g_readyMessageQueueCount;
	if (count > 0) {
		uint16_t index;
		for (index = 0; index < count; ++index) {
			g_readyMessagePaneQueue[index] = g_readyMessagePaneQueue[index + 1];
		}
	}
	--g_readyMessageQueueCount;
}

// FUNCTION: XW 0x412130
void msg_messagedisplay(void) {
#ifdef XW_MODERN
	int hud_pane = XwHud_Push(XW_SNAP_PANE_READY_MESSAGE);
#endif

	uint16_t index;
#ifdef XW_MODERN
	char lastCharacter = ' ';
#else
	char lastCharacter;
#endif
	msg_readymessage();
	FlightDisplay_LockSurface();
#ifdef XW_MODERN
	for (index = 0;
		 index < sizeof(g_readyMessagePaneQueue[0].text) && g_readyMessagePaneQueue[0].text[index] != '\0';
		 ++index) {
#else
	for (index = 0;
		 g_readyMessagePaneQueue[0].text[index] != '\0' && index < sizeof(g_readyMessagePaneQueue[0].text);
		 ++index) {
#endif
		uint16_t character = (uint8_t)g_readyMessagePaneQueue[0].text[index];
		if ((uint8_t)character < FLIGHT_TEXT_FIRST_DRAWABLE_BYTE) {
			festring_settextcolor(character + FLIGHT_TEXT_ENCODED_COLOR_BASE);
		} else {
			g_flightDrawCharFn(character);
			lastCharacter = g_readyMessagePaneQueue[0].text[index];
		}
	}
	msg_completemessage(g_readyMessagePaneQueue[0].paneType, lastCharacter);
	FlightDisplay_UnlockSurface();

#ifdef XW_MODERN
	XwHud_Pop(hud_pane);
#endif
}

// FUNCTION: XW 0x4121B0
void msg_readymessage(void) {
	festring_setfontsize(FLIGHT_FONT_TINY);
	festring_setbackcolor(XW_MSG_BACKGROUND_COLOR);
	g_flightTextShadowEnabled = 1;
	festring_setdropcolor(XW_MSG_SHADOW_COLOR);
	festring_setbound(0,
					  g_flightScreenWidth == FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH
						  ? XW_MSG_STRIP_LOW_RESOLUTION_TOP
						  : XW_MSG_STRIP_HIGH_RESOLUTION_TOP,
					  (int16_t)g_flightScreenWidth, (uint16_t)g_flightScreenHeight);
	festring_setcursor(XW_MSG_STRIP_LEFT_MARGIN,
					   g_flightScreenWidth == FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH
						   ? XW_MSG_STRIP_LOW_RESOLUTION_TOP + XW_MSG_STRIP_TOP_MARGIN
						   : XW_MSG_STRIP_HIGH_RESOLUTION_TOP + XW_MSG_STRIP_TOP_MARGIN);
	festring_settextcolor(XW_MSG_TEXT_COLOR);
}

// FUNCTION: XW 0x412240
void msg_completemessage(int16_t paneType, char lastCharacter) {
	if (lastCharacter != '?' && lastCharacter != '!' && lastCharacter != ':' && lastCharacter != ' ') {
		g_flightDrawCharFn('.');
	}
	festring_setautofill(1);
	g_flightDrawCharFn('\n');
	festring_setautofill(0);
	if (paneType == XW_MSG_PANE_LONG) {
		g_flightGlobalCountdownTimers.ticks[XW_TIMER_READY_MESSAGE] = XW_MSG_LONG_DURATION_TICKS;
	} else if (paneType == XW_MSG_PANE_MEDIUM) {
		g_flightGlobalCountdownTimers.ticks[XW_TIMER_READY_MESSAGE] = XW_MSG_MEDIUM_DURATION_TICKS;
	} else {
		g_flightGlobalCountdownTimers.ticks[XW_TIMER_READY_MESSAGE] =
			g_readyMessageQueueCount != 0 ? XW_MSG_QUEUED_DURATION_TICKS : XW_MSG_DEFAULT_DURATION_TICKS;
	}
	nullsub_SharedNoOp();
	festring_setfontsize(FLIGHT_FONT_MICRO);
}

// FUNCTION: XW 0x4122F0
void msg_messageupdate(void) {
#ifdef XW_MODERN
	int hud_pane = XwHud_Push(XW_SNAP_PANE_READY_MESSAGE);
#endif

	if (g_flightGlobalCountdownTimers.ticks[XW_TIMER_READY_MESSAGE] == 0 &&
		g_readyMessagePaneQueue[0].stateOrMessageId != XW_MSG_INACTIVE) {
		if (g_readyMessageQueueCount != 0) {
			msg_getmessagefromqueue();
			msg_messagedisplay();
		} else {
			festring_setbackcolor(XW_MSG_BACKGROUND_COLOR);
			festring_setbound(0,
							  g_flightScreenWidth == FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH
								  ? XW_MSG_STRIP_LOW_RESOLUTION_TOP
								  : XW_MSG_STRIP_HIGH_RESOLUTION_TOP,
							  (int16_t)g_flightScreenWidth, (uint16_t)g_flightScreenHeight);
			g_flightFillClipRectFn();
			g_readyMessagePaneQueue[0].stateOrMessageId = XW_MSG_INACTIVE;
		}
	}
	if (g_flightGlobalCountdownTimers.ticks[XW_TIMER_INCOMING_WARHEAD] == 0) {
		g_playerFlightState.incomingWarheadAlertState = 0;
	}
	if (g_playerFlightState.incomingWarheadAlertState == USER_WARHEAD_ALERT_TRACKING &&
		g_objectTable[g_playerFlightState.incomingWarheadObjectIndex].objectType != XW_OBJ_TRACKED_WARHEAD) {
		g_playerFlightState.incomingWarheadAlertState = 0;
	}

#ifdef XW_MODERN
	XwHud_Pop(hud_pane);
#endif
}

// FUNCTION: XW 0x4123A0
void msg_clearmessagequeue(void) {
	g_readyMessagePaneQueue[0].stateOrMessageId = XW_MSG_INACTIVE;
	g_readyMessageQueueCount = 0;
	g_messageDynamicStringCursor = 0;
}

// FUNCTION: XW 0x4123C0
void msg_reportfgcreation(uint16_t flightGroupIndex, uint16_t craftTypeIndex) {
	int rangeKm;
	uint16_t numberOfCraft;
	if (g_missionFlightGroups[flightGroupIndex].arrivalMethod != 0) {
		create_getworldposition(PAI_TARGET_WAYPOINT_BASE, flightGroupIndex);
	} else {
		uint16_t leaderObjectIndex;
		for (leaderObjectIndex = 0; leaderObjectIndex < XW_CRAFT_OBJECT_COUNT; ++leaderObjectIndex) {
			if (g_objectTable[leaderObjectIndex].objectType != XW_OBJ_NONE) {
				const CraftData* leaderCraft = g_objectTable[leaderObjectIndex].instanceData;
				if (leaderCraft->flightGroupIndex == flightGroupIndex &&
					leaderCraft->aiLeaderObjectIndex == XW_CRAFT_NO_AI_LEADER) {
					create_getworldposition(leaderObjectIndex, flightGroupIndex);
					break;
				}
			}
		}
	}
#ifdef XW_MODERN
	trig2_ctop((int32_t)((uint32_t)g_resolvedWorldX - (uint32_t)g_playerFlightState.object->worldX),
			   (int32_t)((uint32_t)g_resolvedWorldY - (uint32_t)g_playerFlightState.object->worldY),
			   (int32_t)((uint32_t)g_resolvedWorldZ - (uint32_t)g_playerFlightState.object->worldZ));
	g_trig2PolarDistance = (int32_t)((uint32_t)g_trig2PolarDistance * PANEL_DISTANCE_SCALE);
#else
	trig2_ctop(g_resolvedWorldX - g_playerFlightState.object->worldX,
			   g_resolvedWorldY - g_playerFlightState.object->worldY,
			   g_resolvedWorldZ - g_playerFlightState.object->worldZ);
	g_trig2PolarDistance *= PANEL_DISTANCE_SCALE;
#endif
	rangeKm = ((uint16_t)(g_trig2PolarDistance >> PANEL_DISTANCE_SCALE_SHIFT) +
			   PANEL_DISTANCE_HUNDREDTHS_PER_UNIT / 2) /
			  PANEL_DISTANCE_HUNDREDTHS_PER_UNIT;
	if ((uint16_t)rangeKm == 0)
		rangeKm = 1;
	numberOfCraft = g_missionFlightGroups[flightGroupIndex].numberOfCraft;
	g_msgArgTable[2] = rangeKm;
	g_msgArgTable[0] = numberOfCraft;
	msg_addmessageptr(1, g_craftTypeDefs[craftTypeIndex].name);
	if (numberOfCraft == 1)
		msg_messageprintf(XW_MSG_SENSORS_NEW_CRAFT_SINGULAR);
	else
		msg_messageprintf(XW_MSG_SENSORS_NEW_CRAFT_PLURAL);
}

// FUNCTION: XW 0x412500
void msg_addmessageptr(uint16_t slot, const char* value) {
	uint8_t cursor = g_messageDynamicStringCursor;
	g_messageDynamicStrings[cursor] = value;
	g_msgArgTable[slot] = cursor + XW_MSG_DYNAMIC_STRING_FLAG;
	g_messageDynamicStringCursor = cursor + 1;
	if (g_messageDynamicStringCursor >= XW_MSG_DYNAMIC_STRING_COUNT)
		--g_messageDynamicStringCursor;
}

// FUNCTION: XW 0x412550
void msg_craftmessage(uint16_t objectIndex, const struct CraftData* craft, XwFlightMessageId messageTextId) {
	msg_addmessageptr(0, g_craftTypeDefs[craft->craftTypeIndex].name);
	if (user_CanRevealCraftIdentity(objectIndex, craft) != 0)
		msg_addmessageptr(1, g_missionFlightGroups[craft->flightGroupIndex].name);
	else
		g_msgArgTable[1] = XW_MSG_UNIDENTIFIED_CRAFT_NAME;
	if (g_missionFlightGroups[craft->flightGroupIndex].numberOfCraft > 1) {
		g_msgArgTable[2] = craft->craftIndexInFlightGroup + 1;
		g_msgArgTable[3] = messageTextId;
		msg_messageprintf(XW_MSG_CRAFT_EVENT_NUMBERED);
	} else {
		g_msgArgTable[2] = messageTextId;
		msg_messageprintf(XW_MSG_CRAFT_EVENT_UNNUMBERED);
	}
}

// FUNCTION: XW 0x412610
void msg_radiomessage(const struct CraftData* craft, XwFlightMessageId commandTextId) {
	msg_addmessageptr(0, g_craftTypeDefs[craft->craftTypeIndex].name);
	msg_addmessageptr(1, g_missionFlightGroups[craft->flightGroupIndex].name);
	if (g_missionFlightGroups[craft->flightGroupIndex].numberOfCraft > 1) {
		g_msgArgTable[2] = craft->craftIndexInFlightGroup + 1;
		g_msgArgTable[3] = commandTextId;
		msg_messageprintf(XW_MSG_RADIO_ACKNOWLEDGED_NUMBERED);
	} else {
		g_msgArgTable[2] = commandTextId;
		msg_messageprintf(XW_MSG_RADIO_ACKNOWLEDGED_UNNUMBERED);
	}
	switch (commandTextId) {
		case XW_MSG_RADIO_HEADING_HOME:
			fsfx_triggervoicesfx(FSFX_ACKNOWLEDGE_VOICE_SLOT);
			break;
		case XW_MSG_RADIO_COMING_TO_YOUR_AID:
			if (((uint16_t)math2_getrandom() & XW_MSG_RADIO_RESPONSE_RANDOM_BIT) != 0) {
				fsfx_triggervoicesfx(FSFX_ACKNOWLEDGE_VOICE_SLOT);
				fsfx_triggervoicesfx(FSFX_AID_VOICE_SLOT);
			} else {
				fsfx_triggervoicesfx(FSFX_ACKNOWLEDGE_VOICE_SLOT);
				fsfx_triggervoicesfx(FSFX_AID_ALTERNATE_VOICE_SLOT);
			}
			break;
		case XW_MSG_RADIO_WAITING_FOR_ORDERS:
			fsfx_triggervoicesfx(FSFX_ACKNOWLEDGE_VOICE_SLOT);
			fsfx_triggervoicesfx(FSFX_WAITING_ORDERS_VOICE_SLOT);
			break;
		case XW_MSG_RADIO_PROCEEDING_WITH_MISSION:
			fsfx_triggervoicesfx(FSFX_ACKNOWLEDGE_VOICE_SLOT);
			fsfx_triggervoicesfx(FSFX_PROCEEDING_VOICE_SLOT);
			break;
		case XW_MSG_RADIO_USING_DESIGNATED_TARGET:
			fsfx_triggervoicesfx(FSFX_ACKNOWLEDGE_VOICE_SLOT);
			fsfx_triggervoicesfx(FSFX_DESIGNATED_TARGET_VOICE_SLOT);
			break;
		case XW_MSG_RADIO_IGNORING_TARGET:
			fsfx_triggervoicesfx(FSFX_ACKNOWLEDGE_VOICE_SLOT);
			fsfx_triggervoicesfx(FSFX_IGNORE_TARGET_VOICE_SLOT);
			break;
	}
}

// FUNCTION: XW 0x412770
void msg_reportmessage(const struct CraftData* craft, XwFlightMessageId reportTextId) {
	msg_addmessageptr(0, g_craftTypeDefs[craft->craftTypeIndex].name);
	msg_addmessageptr(1, g_missionFlightGroups[craft->flightGroupIndex].name);
	if (g_missionFlightGroups[craft->flightGroupIndex].numberOfCraft > 1) {
		g_msgArgTable[2] = craft->craftIndexInFlightGroup + 1;
		g_msgArgTable[3] = reportTextId;
		msg_messageprintf(XW_MSG_CRAFT_REPORT_NUMBERED);
	} else {
		g_msgArgTable[2] = reportTextId;
		msg_messageprintf(XW_MSG_CRAFT_REPORT_UNNUMBERED);
	}
}
