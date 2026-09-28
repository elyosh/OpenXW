#include "xw/frontend/combat.h"

#ifdef XW_MODERN
#include "xw_runtime/integration/landru_adapter.h"
#endif

#include "xw/audio/frontend_audio.h"
#include "xw/audio/soundext.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"
#include "xw/frontend/textext.h"
#include "xw/util/landru_display.h"
#include "xw/util/shared.h"
#include "xw_runtime/integration/combat_callbacks.h"

#ifdef XW_MODERN
#include "xw_runtime/runtime/combat_task.h"
#endif

#include <landru/actanim.h>
#include <landru/actcust.h>
#include <landru/actdelt.h>
#include <landru/actrect.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/file.h>
#include <landru/font.h>
#include <landru/fourcc.h>
#include <landru/inpattr.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/paint.h>
#include <landru/paragrp.h>
#include <landru/style.h>
#include <landru/view.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4D1630
char g_combatWelcomeLines[COMBAT_WELCOME_LINE_COUNT][COMBAT_WELCOME_LINE_CAPACITY] = {
	"Welcome to the Rebel Alliance",  "Combat Training School.  Here", "you will re-enact historical",
	"encounters with Imperial",       "Forces.  These exercises are",  "designed to teach you the",
	"skills you will need to engage", "the enemy in actual combat.",   ""
};

// GLOBAL: XW 0x4D1798
int16_t g_combatEnemySpecLineBounds[COMBAT_ENEMY_PREVIEW_COUNT + 1] = { 4, 7, 10, 15 };

// GLOBAL: XW 0x4D17A0
int16_t g_combatEnemySpecX[COMBAT_ENEMY_PREVIEW_COUNT] = { 20, 220, 8 };

// GLOBAL: XW 0x4D17A8
int16_t g_combatEnemySpecY[COMBAT_ENEMY_PREVIEW_COUNT] = { 96, 96, 96 };

// GLOBAL: XW 0x4D17B0
char g_combatEnemyText[COMBAT_ENEMY_TEXT_COUNT][COMBAT_ENEMY_TEXT_CAPACITY] = {
	"Imperial",   "TIE Fighter", "TIE Interceptor", "TIE Bomber", "100 MGLT",
	"15 RU Hull", "Twin Lasers", "110 MGLT",        "20 RU Hull", "Quad Lasers",
	"80 MGLT",    "50 RU Hull",  "Twin Lasers",     "Missiles",   "Proton Torpedoes"
};

// GLOBAL: XW 0x4D19E8
char g_combatEnemyResourceNames[COMBAT_ENEMY_PREVIEW_COUNT][COMBAT_ENEMY_RESOURCE_NAME_CAPACITY] = {
	"cbr-fght", "cbr-intr", "cbr-bomb"
};

// GLOBAL: XW 0x4D1B08
int16_t g_combatKeyboardFocusX[COMBAT_FOCUS_CAPACITY] = { 8,   160, 160, 160, 312, 8,   110, 160,
														  212, 312, 8,   110, 160, 212, 312, 0 };

// GLOBAL: XW 0x4D1B28
int16_t g_combatKeyboardFocusY[COMBAT_FOCUS_CAPACITY] = { 116, 90,  90,  90,  116, 116, 162, 162,
														  162, 116, 116, 186, 186, 186, 116, 0 };

// GLOBAL: XW 0x4D1C24
char g_combatMissionFormatA[] = "%da";

// GLOBAL: XW 0x4D1C28
char g_combatMissionFormatB[] = "%db";

// GLOBAL: XW 0x4F6250
XwCombatMusicState g_combatMusicState = { NULL, NULL, NULL, COMBAT_MUSIC_WAIT_FILM, 0 };

// GLOBAL: XW 0x4F6264
int16_t g_combatTypingSoundActive = 0;

// GLOBAL: XW 0x4F6268
int16_t g_combatTypingRequested = 0;

// GLOBAL: XW 0x4F627C
Actor* g_combatShipInfoActor = NULL;

// GLOBAL: XW 0x4F6280
Actor* g_combatWelcomeTextActor = NULL;

// GLOBAL: XW 0x4F6284
Actor* g_combatNextShipButtonActor = NULL;

// GLOBAL: XW 0x4F6288
LandruHandle g_combatShipListText = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4F628C
Actor* g_combatScreenActor = NULL;

// GLOBAL: XW 0x4F6290
Actor* g_combatFilmTag3Actor = NULL;

// GLOBAL: XW 0x4F6294
Actor* g_combatMonitorBackgroundActor = NULL;

// GLOBAL: XW 0x4F629C
Actor* g_combatLeftDoorActor = NULL;

// GLOBAL: XW 0x4F62A0
Input* g_combatRightDoorInput = NULL;

// GLOBAL: XW 0x4F62A8
Actor* g_combatStarsActor = NULL;

// GLOBAL: XW 0x4F62AC
Actor* g_combatStarsWrapActor = NULL;

// GLOBAL: XW 0x4F62B0
int16_t g_combatSavedDecorStates[COMBAT_DECOR_STATE_COUNT] = { 0 };

// GLOBAL: XW 0x4F62B4
Actor* g_combatNextMissionButtonActor = NULL;

// GLOBAL: XW 0x4F62B8
LandruHandle g_combatScoreHandle = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4F62BC
Actor* g_combatFallbackIconActor = NULL;

// GLOBAL: XW 0x4F62C0
Actor* g_combatEnemyPreviewActors[COMBAT_ENEMY_PREVIEW_COUNT] = { NULL };

// GLOBAL: XW 0x4F62CC
Actor* g_combatPreviousShipButtonActor = NULL;

// GLOBAL: XW 0x4F62D0
Actor* g_combatScoreActor = NULL;

// GLOBAL: XW 0x4F62D8
REGISTER_PilotFileRecord g_combatPilotData = { 0 };

// GLOBAL: XW 0x4F6984
Actor* g_combatNavigationRevealActor = NULL;

// GLOBAL: XW 0x4F6988
Input* g_combatRootInput = NULL;

// GLOBAL: XW 0x4F698C
Input* g_combatHoverLabelInput = NULL;

// GLOBAL: XW 0x4F6990
Actor* g_combatRightDoorActor = NULL;

// GLOBAL: XW 0x4F6998
int16_t g_combatAvailableTourCount = 0;

// GLOBAL: XW 0x4F699C
int16_t g_combatScoreSelection = 0;

// GLOBAL: XW 0x4F69A0
LandruHandle g_combatMissionParagraphs[COMBAT_MISSION_PARAGRAPH_COUNT] = { 0 };

// GLOBAL: XW 0x4F69BC
int16_t g_combatAvailableShipCount = 0;

// GLOBAL: XW 0x4F69C0
int16_t g_combatMissionCounts[SHIPEXT_COMBAT_SELECTION_COUNT] = { 0 };

// GLOBAL: XW 0x4F69E0
Actor* g_combatShipIconsActor = NULL;

// GLOBAL: XW 0x4F69E4
Actor* g_combatShipIconFrameActor = NULL;

// GLOBAL: XW 0x4F69E8
Input* g_combatNavigationInput = NULL;

// GLOBAL: XW 0x4F69EC
Input* g_combatLeftDoorInput = NULL;

// GLOBAL: XW 0x4F69F4
Film* g_combatFilm = NULL;

// GLOBAL: XW 0x4F69F8
Actor* g_combatMissionTextActor = NULL;

// GLOBAL: XW 0x4F69FC
ResFile* g_combatResourceFile = NULL;

// GLOBAL: XW 0x4F6A00
Actor* g_combatFilmTag1Actor = NULL;

// GLOBAL: XW 0x4F6A08
int16_t g_combatSelectionSourceIndices[SHIPEXT_COMBAT_SELECTION_COUNT] = { 0 };

// GLOBAL: XW 0x4F6A24
Actor* g_combatCurrentEnemyPreview = NULL;

// GLOBAL: XW 0x4F6A2C
Actor* g_combatPreviousMissionButtonActor = NULL;

// GLOBAL: XW 0x4F6A30
int16_t g_combatScoreMissionCount = 0;

// GLOBAL: XW 0x4F6A34
int g_combatMonitorTime = 0;

// GLOBAL: XW 0x4F6A38
int16_t g_combatKeyboardFocusIndex = 0;

// FUNCTION: XW 0x43E4B0
void combat_OpenMusic(ResFile* unusedResourceFile, Film* film, int* monitorTime) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_combatMusicState.sound = xsound_Find_Gmid("mission");
		g_combatMusicState.film = film;
		g_combatMusicState.monitorTime = monitorTime;
		if (g_combatMusicState.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource(":X-Wing Data\\RESOURCE\\cbmusic.lfd");
			if (musicResource == NULL)
				musicResource = xres_Open_Resource("cbmusic.lfd");
			g_combatMusicState.sound = xsound_Res_Music(musicResource, "mission");
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_combatMusicState.sound);
			if (g_combatMusicState.completedVisits <= COMBAT_MUSIC_INITIAL_VISIT_LIMIT)
				soundext_ScanMidi(g_combatMusicState.sound, 0, COMBAT_MUSIC_START_BEAT, 0);
		}
		if (g_combatMusicState.completedVisits > COMBAT_MUSIC_INITIAL_VISIT_LIMIT) {
			soundext_SetHook(g_combatMusicState.sound, XW_SOUND_CONTROL_DIRECT, COMBAT_MUSIC_CONTROL_4, 0);
			soundext_SetPartEnabled(g_combatMusicState.sound, COMBAT_MUSIC_CHANNEL_INTERCEPTOR, 0);
			soundext_SetPartEnabled(g_combatMusicState.sound, COMBAT_MUSIC_CHANNEL_FIGHTER, 0);
			soundext_SetPartEnabled(g_combatMusicState.sound, COMBAT_MUSIC_CHANNEL_BOMBER, 0);
		}
		soundext_FadeVolume(g_combatMusicState.sound, COMBAT_MUSIC_OPEN_VOLUME, COMBAT_MUSIC_OPEN_DURATION);
		xsound_Set_Sound_Keep(g_combatMusicState.sound);
		xsound_Set_Sound_User_Function(g_combatMusicState.sound, combat_user_Music);
	}
}

// FUNCTION: XW 0x43E5F0
void combat_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0) {
		if (xerror_Get_Landru_Exit() == XW_SCENE_COMBAT_RETURN_SHUTTLE) {
			int tick = soundext_GetMusicParam(g_combatMusicState.sound, XW_SOUND_QUERY_TICK, 0);
			soundext_JumpMidi(g_combatMusicState.sound, COMBAT_MUSIC_RETURN_GROUP, COMBAT_MUSIC_RETURN_BEAT,
							  tick);
			soundext_FadeVolume(g_combatMusicState.sound, COMBAT_MUSIC_RETURN_VOLUME,
								COMBAT_MUSIC_RETURN_DURATION);
		} else if (g_combatMusicState.completedVisits < COMBAT_MUSIC_REPEAT_VISITS) {
			int position = soundext_GetMusicParam(g_combatMusicState.sound, XW_SOUND_QUERY_BEAT, 0);
			if (position <= COMBAT_MUSIC_SECTION_END) {
				if (position > COMBAT_MUSIC_SECTION_4_START)
					soundext_SetHook(g_combatMusicState.sound, XW_SOUND_CONTROL_DIRECT,
									 COMBAT_MUSIC_CONTROL_4, 0);
				else if (position > COMBAT_MUSIC_SECTION_3_START)
					soundext_SetHook(g_combatMusicState.sound, XW_SOUND_CONTROL_DIRECT,
									 COMBAT_MUSIC_CONTROL_3, 0);
				else if (position > COMBAT_MUSIC_SECTION_2_START)
					soundext_SetHook(g_combatMusicState.sound, XW_SOUND_CONTROL_DIRECT,
									 COMBAT_MUSIC_CONTROL_2, 0);
				else
					soundext_SetHook(g_combatMusicState.sound, XW_SOUND_CONTROL_DIRECT,
									 COMBAT_MUSIC_CONTROL_1, 0);
			}
			soundext_SetHook(g_combatMusicState.sound, XW_SOUND_CONTROL_DIRECT, COMBAT_MUSIC_CONTROL_3, 0);
		} else {
			soundext_SetHook(g_combatMusicState.sound, XW_SOUND_CONTROL_DIRECT, COMBAT_MUSIC_CONTROL_5, 0);
		}
		g_combatMusicState.phase = COMBAT_MUSIC_WAIT_FILM;
		++g_combatMusicState.completedVisits;
	}
}

// FUNCTION: XW 0x43E700
void combat_user_Music(Sound* sound, int time) {
	(void)sound;
	(void)time;
	switch (g_combatMusicState.phase) {
		case COMBAT_MUSIC_WAIT_FILM:
			if (g_combatMusicState.film->cur_cel >= 1 &&
				soundext_Count_Resource_Instances(g_combatMusicState.sound) == 1) {
				soundext_SetHook(g_combatMusicState.sound, XW_SOUND_CONTROL_DIRECT, COMBAT_MUSIC_CONTROL_1,
								 0);
				g_combatMusicState.phase = COMBAT_MUSIC_WAIT_SCORE_END;
			}
			break;
		case COMBAT_MUSIC_WAIT_SCORE_END:
			if (*g_combatMusicState.monitorTime >= COMBAT_SCORE_END) {
				soundext_SetHook(g_combatMusicState.sound, XW_SOUND_CONTROL_DIRECT, COMBAT_MUSIC_CONTROL_2,
								 0);
				g_combatMusicState.phase = COMBAT_MUSIC_WAIT_FIGHTER;
			}
			break;
		case COMBAT_MUSIC_WAIT_FIGHTER:
			if (*g_combatMusicState.monitorTime >= COMBAT_TIE_FIGHTER_START &&
				*g_combatMusicState.monitorTime < COMBAT_MONITOR_INTERVAL_3_END) {
				soundext_SetHook(g_combatMusicState.sound, XW_SOUND_CONTROL_PACKED, 1,
								 COMBAT_MUSIC_CHANNEL_FIGHTER);
				g_combatMusicState.phase = COMBAT_MUSIC_WAIT_INTERCEPTOR;
			}
			break;
		case COMBAT_MUSIC_WAIT_INTERCEPTOR:
			if (*g_combatMusicState.monitorTime >= COMBAT_TIE_INTERCEPTOR_START) {
				soundext_SetHook(g_combatMusicState.sound, XW_SOUND_CONTROL_PACKED, 1,
								 COMBAT_MUSIC_CHANNEL_INTERCEPTOR);
				g_combatMusicState.phase = COMBAT_MUSIC_WAIT_BOMBER;
			}
			break;
		case COMBAT_MUSIC_WAIT_BOMBER:
			if (*g_combatMusicState.monitorTime >= COMBAT_TIE_BOMBER_START) {
				soundext_SetHook(g_combatMusicState.sound, XW_SOUND_CONTROL_PACKED, 1,
								 COMBAT_MUSIC_CHANNEL_BOMBER);
				soundext_SetHook(g_combatMusicState.sound, XW_SOUND_CONTROL_PACKED, 1,
								 COMBAT_MUSIC_CHANNEL_BOMBER_SECONDARY);
				g_combatMusicState.phase = COMBAT_MUSIC_WAIT_BOMBER_RESUME;
			}
			break;
		case COMBAT_MUSIC_WAIT_BOMBER_RESUME:
			if (*g_combatMusicState.monitorTime >= COMBAT_TIE_BOMBER_RESUME) {
				soundext_SetHook(g_combatMusicState.sound, XW_SOUND_CONTROL_DIRECT, COMBAT_MUSIC_CONTROL_3,
								 0);
				g_combatMusicState.phase = COMBAT_MUSIC_WAIT_REPEAT;
			}
			break;
		case COMBAT_MUSIC_WAIT_REPEAT:
			if (*g_combatMusicState.monitorTime < COMBAT_TIE_BOMBER_START) {
				soundext_SetHook(g_combatMusicState.sound, XW_SOUND_CONTROL_PACKED, 0,
								 COMBAT_MUSIC_CHANNEL_FIGHTER);
				soundext_SetHook(g_combatMusicState.sound, XW_SOUND_CONTROL_PACKED, 0,
								 COMBAT_MUSIC_CHANNEL_INTERCEPTOR);
				soundext_SetHook(g_combatMusicState.sound, XW_SOUND_CONTROL_PACKED, 0,
								 COMBAT_MUSIC_CHANNEL_BOMBER);
				soundext_SetHook(g_combatMusicState.sound, XW_SOUND_CONTROL_PACKED, 0,
								 COMBAT_MUSIC_CHANNEL_BOMBER_SECONDARY);
				g_combatMusicState.phase = COMBAT_MUSIC_WAIT_FIGHTER;
			}
			break;
	}
}

// FUNCTION: XW 0x43E900
void combat_LoadSoundEffects(void) {
	if (ShellPreferences_GetSfxEnabled()) {
		soundext_LoadSfx(XW_SHELL_SFX_DOOR_6, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_DOOR_CLOSE_2, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_HYDROL_1, 0, NULL, 0, 0);
		soundext_LoadSfx(XW_SHELL_SFX_TARGET_5, 0, NULL, 0, 0);
		soundext_LoadSfx(XW_SHELL_SFX_TEXT_5, 0, NULL, 0, 0);
		g_combatTypingSoundActive = 0;
	}
	ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x43E980
void combat_HandleSoundAction(int16_t action) {
	if (ShellPreferences_GetSfxEnabled()) {
		switch (action) {
			case COMBAT_SOUND_DOOR_OPEN:
				soundext_Play_SFX(XW_SHELL_SFX_DOOR_6);
				break;
			case COMBAT_SOUND_DOOR_CLOSE:
				soundext_Play_SFX(XW_SHELL_SFX_DOOR_CLOSE_2);
				break;
			case COMBAT_SOUND_HYDRAULICS_START:
				soundext_Play_SFX(XW_SHELL_SFX_HYDROL_1);
				break;
			case COMBAT_SOUND_HYDRAULICS_STOP:
				soundext_Stop_SFX(XW_SHELL_SFX_HYDROL_1);
				break;
			case COMBAT_SOUND_TARGET:
				soundext_Play_SFX(XW_SHELL_SFX_TARGET_5);
				break;
			case COMBAT_SOUND_TYPING_START:
				if (!g_combatTypingSoundActive) {
					soundext_Play_SFX(XW_SHELL_SFX_TEXT_5);
					g_combatTypingSoundActive = 1;
				}
				g_combatTypingRequested = 1;
				break;
			case COMBAT_SOUND_TYPING_STOP:
				if (g_combatTypingSoundActive) {
					soundext_Stop_SFX(XW_SHELL_SFX_TEXT_5);
					g_combatTypingSoundActive = 0;
				}
				g_combatTypingRequested = 1;
				break;
			case COMBAT_SOUND_TYPING_UPDATE:
				if (!g_combatTypingRequested) {
					soundext_Stop_SFX(XW_SHELL_SFX_TEXT_5);
					g_combatTypingSoundActive = 0;
				}
				g_combatTypingRequested = 0;
				break;
		}
	}
}

// FUNCTION: XW 0x43EB40
XwShellSceneResult combat_Combat(struct XwShellContext* shell) {
	ResFile* missionResource;
	Rect rect;
	char paragraphName[COMBAT_PARAGRAPH_NAME_CAPACITY];
	int16_t unavailableCount;
	int unavailableTours;
	int unavailableShips;
	int16_t sourceIndex;
	int16_t missionListIndex;
	int16_t enemyIndex;
	PushButton* button;
	Input* monitorInput;
	Input* shipLabelInput;
	Input* missionLabelInput;
#ifndef XW_MODERN
	int16_t selectedMission;
	int16_t releaseIndex;
#endif
	g_combatMonitorTime = 0;
	g_combatKeyboardFocusIndex = COMBAT_INITIAL_FOCUS;
	xio_Set_Mouse_Position(COMBAT_INITIAL_MOUSE_X, COMBAT_INITIAL_MOUSE_Y);
	g_combatScoreSelection = -1;
	combat_Load_Combat_High_Scores();
	combat_ReadPilot();
#ifdef XW_MODERN
	missionResource = XwLandru_OpenMissionResource(":X-Wing Data\\Resource\\missions.lfd");
#else
	missionResource = xres_Open_Resource(":X-Wing Data\\Resource\\missions.lfd");
#endif
	if (missionResource == NULL)
#ifdef XW_MODERN
		missionResource = XwLandru_OpenMissionResource("missions.lfd");
#else
		missionResource = xres_Open_Resource("missions.lfd");
#endif
	g_combatShipListText = xparagrp_Res_Paragraph(missionResource, "comships");
	g_combatAvailableShipCount =
		xparagrp_Count_Paragraph_Strings(g_combatShipListText, COMBAT_SHIP_NAMES_PARAGRAPH);
	g_combatAvailableTourCount =
		xparagrp_Count_Paragraph_Strings(g_combatShipListText, COMBAT_TOUR_NAMES_PARAGRAPH);
	unavailableCount = 0;
	unavailableShips = 0;
	unavailableTours = 0;
	for (sourceIndex = 0; sourceIndex < g_combatAvailableShipCount + g_combatAvailableTourCount;
		 ++sourceIndex) {
		if (sourceIndex < g_combatAvailableShipCount) {
			if (shipext_IsShipAvailable(sourceIndex)) {
				int selectionIndex;
				xparagrp_Get_Paragraph_String(g_combatShipListText, paragraphName,
											  COMBAT_RESOURCE_NAMES_PARAGRAPH, sourceIndex);
				selectionIndex = sourceIndex - unavailableCount;
				g_combatMissionParagraphs[selectionIndex] =
					xparagrp_Res_Paragraph(missionResource, paragraphName);
				g_combatSelectionSourceIndices[selectionIndex] = sourceIndex;
			} else {
				++unavailableShips;
				++unavailableCount;
			}
		} else {
#ifdef XW_MODERN
			/* A pilot may have completed this tour in another installation. */
			if (!shipext_IsTourAvailable(sourceIndex - g_combatAvailableShipCount)) {
				++unavailableTours;
				++unavailableCount;
				continue;
			}
#endif
			if (g_combatPilotData.tour_status[sourceIndex - g_combatAvailableShipCount] != 0 &&
				g_combatPilotData.tourReplayUnlockMission[sourceIndex - g_combatAvailableShipCount] !=
					SHIPEXT_PILOT_UNSET_BYTE) {
				int selectionIndex;
				xparagrp_Get_Paragraph_String(g_combatShipListText, paragraphName,
											  COMBAT_RESOURCE_NAMES_PARAGRAPH, sourceIndex);
				selectionIndex = sourceIndex - unavailableCount;
				g_combatMissionParagraphs[selectionIndex] =
					xparagrp_Res_Paragraph(missionResource, paragraphName);
				g_combatSelectionSourceIndices[selectionIndex] = sourceIndex;
			} else {
				++unavailableTours;
				++unavailableCount;
			}
		}
	}
	g_combatAvailableTourCount -= unavailableTours;
	g_combatAvailableShipCount -= unavailableShips;
	for (missionListIndex = 0; missionListIndex < g_combatAvailableShipCount + g_combatAvailableTourCount;
		 ++missionListIndex) {
		LandruHandle missionParagraph = g_combatMissionParagraphs[missionListIndex];
		if (missionParagraph != LANDRU_NULL_HANDLE) {
			int16_t missionCount = xparagrp_Count_Paragraph_Strings(missionParagraph, 0);
			g_combatMissionCounts[missionListIndex] = missionCount;
			if (missionListIndex >= g_combatAvailableShipCount) {
				int16_t tourIndex = g_combatSelectionSourceIndices[missionListIndex] - SHIPEXT_SHIP_COUNT;
				int16_t operationCount = g_combatPilotData.tourOperationProgress[tourIndex];
				int16_t availableVariantCount = 0;
				int16_t operationIndex;
				for (operationIndex = 0; operationIndex < operationCount; ++operationIndex) {
					if (g_tourOperationTables[tourIndex][operationIndex].missionChoiceB !=
						SHIPEXT_TOUR_NO_ENTRY)
						++availableVariantCount;
					++availableVariantCount;
				}
				if (missionCount > (int16_t)availableVariantCount)
					g_combatMissionCounts[missionListIndex] = availableVariantCount;
			}
		}
	}
	xres_Close_Resource(missionResource);
	g_combatResourceFile = xres_Open_Resource(":X-Wing Data\\RESOURCE\\combat64.lfd");
	if (g_combatResourceFile == NULL)
		g_combatResourceFile = xres_Open_Resource("combat64.lfd");
	xview_Disable_All_View_Erase();
	xrect_Set_Rect(&rect, COMBAT_CANVAS_LEFT, COMBAT_CANVAS_TOP, COMBAT_CANVAS_RIGHT, COMBAT_CANVAS_BOTTOM);
	g_combatFilm =
		xfilm_Res_Callback_Film(g_combatResourceFile, "combat", &rect, 0, 0, 0, combat_film_Combat_Callback);
	xfilm_Set_Film_Def_Palette(g_combatFilm, shell->standardPalette);
	xrect_Set_Rect(&rect, COMBAT_PREVIEW_FRAME_LEFT, COMBAT_PREVIEW_FRAME_TOP, COMBAT_PREVIEW_FRAME_RIGHT,
				   COMBAT_PREVIEW_FRAME_BOTTOM);
	g_combatStarsActor = xactdelt_Res_Delta_Actor(g_combatResourceFile, "stars", &rect, 0, 0, COMBAT_STARS_Z);
	g_combatStarsWrapActor = xactdelt_Res_Delta_Actor(g_combatResourceFile, "stars", &rect,
													  COMBAT_CANVAS_WIDTH, 0, COMBAT_STARS_Z);
	xactor_Set_Actor_User_Function(g_combatStarsActor, combat_user_Stars);
	xactor_Set_Actor_User_Function(g_combatStarsWrapActor, combat_user_Stars);
	g_combatMonitorBackgroundActor = xactrect_Alloc_Blank_Actor(0, &rect, 0, 0, COMBAT_BACKGROUND_Z);
	xactor_Set_Actor_Size(g_combatMonitorBackgroundActor,
						  COMBAT_PREVIEW_FRAME_RIGHT - COMBAT_PREVIEW_FRAME_LEFT,
						  COMBAT_PREVIEW_FRAME_BOTTOM - COMBAT_PREVIEW_FRAME_TOP);
	xactor_Set_Actor_Color(g_combatMonitorBackgroundActor, 0, 0);
	g_combatShipInfoActor = xactcust_Alloc_Custom_Actor(0, &rect, 0, 0, COMBAT_MONITOR_TEXT_Z);
	xactor_Set_Actor_Update_Function(g_combatShipInfoActor, combat_update_ShipInfo);
	xactor_Set_Actor_User_Function(g_combatShipInfoActor, combat_user_ShipInfo);
	xactor_Set_Actor_Draw_Function(g_combatShipInfoActor, combat_Draw_Combat_Screen_Flyby);
	g_combatShipInfoActor->var1 = 0;
	g_combatScoreActor = xactcust_Alloc_Custom_Actor(0, &rect, 0, 0, COMBAT_MONITOR_TEXT_Z);
	xactor_Set_Actor_User_Function(g_combatScoreActor, combat_user_Score);
	xactor_Set_Actor_Draw_Function(g_combatScoreActor, combat_Draw_Combat_Screen_Score);
	g_combatScoreActor->var1 = 0;
	g_combatWelcomeTextActor = xactcust_Alloc_Custom_Actor(0, &rect, 0, 0, COMBAT_MONITOR_TEXT_Z);
	xactor_Set_Actor_User_Function(g_combatWelcomeTextActor, combat_user_Welcome);
	xactor_Set_Actor_Draw_Function(g_combatWelcomeTextActor, combat_draw_Welcome);
	g_combatWelcomeTextActor->var1 = 0;
	g_combatMissionTextActor = xactcust_Alloc_Custom_Actor(0, &rect, 0, 0, COMBAT_MONITOR_TEXT_Z);
	xactor_Set_Actor_User_Function(g_combatMissionTextActor, combat_user_MissionText);
	xactor_Set_Actor_Draw_Function(g_combatMissionTextActor, combat_Draw_Combat_Screen_Mission);
	g_combatMissionTextActor->var1 = 0;
	xrect_Set_Rect(&rect, COMBAT_CANVAS_LEFT, COMBAT_CANVAS_TOP, COMBAT_CANVAS_RIGHT, COMBAT_CANVAS_BOTTOM);
	g_combatRootInput = xinput_Alloc_Input(NULL, &rect, 0, 0);
	xrect_Set_Rect(&rect, COMBAT_PREVIEW_FRAME_LEFT, COMBAT_PREVIEW_FRAME_TOP, COMBAT_PREVIEW_FRAME_RIGHT,
				   COMBAT_PREVIEW_FRAME_BOTTOM);
	monitorInput = xinput_Alloc_Input(g_combatRootInput, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(monitorInput, XwCombat_UpdateMonitor);
	xinpattr_Set_Input_User_Function(monitorInput, combat_iuser_Combat_Screen);
	xrect_Set_Rect(&rect, COMBAT_LEFT_DOOR_LEFT, COMBAT_LEFT_DOOR_TOP, COMBAT_LEFT_DOOR_RIGHT,
				   COMBAT_LEFT_DOOR_BOTTOM);
	g_combatLeftDoorInput = xinput_Alloc_Input(g_combatRootInput, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(g_combatLeftDoorInput, combat_iupdate_Door);
	xinpattr_Set_Input_User_Function(g_combatLeftDoorInput, combat_iuser_Door);
	g_combatLeftDoorInput->mouseUsage = allInput;
	g_combatLeftDoorInput->id = 0;
	xrect_Set_Rect(&rect, COMBAT_RIGHT_DOOR_LEFT, COMBAT_RIGHT_DOOR_TOP, COMBAT_RIGHT_DOOR_RIGHT,
				   COMBAT_RIGHT_DOOR_BOTTOM);
	g_combatRightDoorInput = xinput_Alloc_Input(g_combatRootInput, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(g_combatRightDoorInput, combat_iupdate_Door);
	xinpattr_Set_Input_User_Function(g_combatRightDoorInput, combat_iuser_Door);
	g_combatRightDoorInput->mouseUsage = allInput;
	g_combatRightDoorInput->id = 1;
	xrect_Set_Rect(&rect, COMBAT_SHIP_LABEL_LEFT, COMBAT_SHIP_LABEL_TOP, COMBAT_SHIP_LABEL_RIGHT,
				   COMBAT_SHIP_LABEL_BOTTOM);
	g_combatHoverLabelInput = xinput_Alloc_Input(g_combatRootInput, &rect, 0, 0);
	xinpattr_Set_Input_Draw_Function(g_combatHoverLabelInput, combat_idraw_DoorLabel);
	xinpattr_Hide_Input(g_combatHoverLabelInput);
	xrect_Set_Rect(&rect, COMBAT_NAVIGATION_LEFT, COMBAT_NAVIGATION_TOP, COMBAT_NAVIGATION_RIGHT,
				   COMBAT_NAVIGATION_BOTTOM);
	g_combatNavigationInput = xinput_Alloc_Input(g_combatRootInput, &rect, 0, 0);
	xrect_Set_Rect(&rect, COMBAT_PREVIOUS_SHIP_LEFT, COMBAT_PREVIOUS_SHIP_TOP, COMBAT_PREVIOUS_SHIP_RIGHT,
				   COMBAT_PREVIOUS_SHIP_BOTTOM);
	xrect_Offset_Rect(&rect, -COMBAT_NAVIGATION_LEFT, -COMBAT_NAVIGATION_TOP);
	button = xbtnpush_Alloc_Button(g_combatNavigationInput, &rect, 0, combat_iuser_Combat, NULL,
								   COMBAT_BUTTON_PREVIOUS_SHIP);
	xinpattr_Set_Input_Draw_Function(&button->header, XwCombat_DrawArrowButton);
	g_combatPreviousShipButtonActor = xactor_Find_Actor(FOURCC_ANIM, "tlbutt");
	xrect_Set_Rect(&rect, COMBAT_NEXT_SHIP_LEFT, COMBAT_NEXT_SHIP_TOP, COMBAT_NEXT_SHIP_RIGHT,
				   COMBAT_NEXT_SHIP_BOTTOM);
	xrect_Offset_Rect(&rect, -COMBAT_NAVIGATION_LEFT, -COMBAT_NAVIGATION_TOP);
	button = xbtnpush_Alloc_Button(g_combatNavigationInput, &rect, 0, combat_iuser_Combat, NULL,
								   COMBAT_BUTTON_NEXT_SHIP);
	xinpattr_Set_Input_Draw_Function(&button->header, XwCombat_DrawArrowButton);
	g_combatNextShipButtonActor = xactor_Find_Actor(FOURCC_ANIM, "trbutt");
	xrect_Set_Rect(&rect, COMBAT_SHIP_LABEL_LEFT, COMBAT_SHIP_LABEL_TOP, COMBAT_SHIP_LABEL_RIGHT,
				   COMBAT_SHIP_LABEL_BOTTOM);
	xrect_Offset_Rect(&rect, -COMBAT_NAVIGATION_LEFT, -COMBAT_NAVIGATION_TOP);
	shipLabelInput = xinput_Alloc_Input(g_combatNavigationInput, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(shipLabelInput, combat_iupdate_SelectionLabel);
	xinpattr_Set_Input_Draw_Function(shipLabelInput, combat_idraw_Combat);
	shipLabelInput->id = 0;
	xrect_Set_Rect(&rect, COMBAT_PREVIOUS_MISSION_LEFT, COMBAT_PREVIOUS_MISSION_TOP,
				   COMBAT_PREVIOUS_MISSION_RIGHT, COMBAT_PREVIOUS_MISSION_BOTTOM);
	xrect_Offset_Rect(&rect, -COMBAT_NAVIGATION_LEFT, -COMBAT_NAVIGATION_TOP);
	button = xbtnpush_Alloc_Button(g_combatNavigationInput, &rect, 0, combat_iuser_Combat, NULL,
								   COMBAT_BUTTON_PREVIOUS_MISSION);
	xinpattr_Set_Input_Draw_Function(&button->header, XwCombat_DrawArrowButton);
	g_combatPreviousMissionButtonActor = xactor_Find_Actor(FOURCC_ANIM, "blbutt");
	xrect_Set_Rect(&rect, COMBAT_NEXT_MISSION_LEFT, COMBAT_NEXT_MISSION_TOP, COMBAT_NEXT_MISSION_RIGHT,
				   COMBAT_NEXT_MISSION_BOTTOM);
	xrect_Offset_Rect(&rect, -COMBAT_NAVIGATION_LEFT, -COMBAT_NAVIGATION_TOP);
	button = xbtnpush_Alloc_Button(g_combatNavigationInput, &rect, 0, combat_iuser_Combat, NULL,
								   COMBAT_BUTTON_NEXT_MISSION);
	xinpattr_Set_Input_Draw_Function(&button->header, XwCombat_DrawArrowButton);
	g_combatNextMissionButtonActor = xactor_Find_Actor(FOURCC_ANIM, "brbutt");
	xrect_Set_Rect(&rect, COMBAT_MISSION_LABEL_LEFT, COMBAT_MISSION_LABEL_TOP, COMBAT_MISSION_LABEL_RIGHT,
				   COMBAT_MISSION_LABEL_BOTTOM);
	xrect_Offset_Rect(&rect, -COMBAT_NAVIGATION_LEFT, -COMBAT_NAVIGATION_TOP);
	missionLabelInput = xinput_Alloc_Input(g_combatNavigationInput, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(missionLabelInput, combat_iupdate_SelectionLabel);
	xinpattr_Set_Input_Draw_Function(missionLabelInput, combat_idraw_Combat);
	missionLabelInput->id = COMBAT_MISSION_LABEL;
	if (Shared_ReturnZero() != 0) {
		g_combatEnemyPreviewActors[0] = NULL;
		g_combatEnemyPreviewActors[1] = NULL;
		g_combatEnemyPreviewActors[2] = NULL;
	} else {
		xrect_Set_Rect(&rect, COMBAT_PREVIEW_FRAME_LEFT, COMBAT_PREVIEW_FRAME_TOP, COMBAT_PREVIEW_FRAME_RIGHT,
					   COMBAT_PREVIEW_FRAME_BOTTOM);
		g_combatEnemyPreviewActors[0] =
			xactanim_Res_Anim_Actor(g_combatResourceFile, g_combatEnemyResourceNames[0], &rect,
									COMBAT_PREVIEW_FRAME_LEFT, COMBAT_PREVIEW_FRAME_TOP, COMBAT_PREVIEW_Z);
		xactor_Set_Actor_User_Function(g_combatEnemyPreviewActors[0], combat_user_TieFighter);
		g_combatEnemyPreviewActors[1] =
			xactanim_Res_Anim_Actor(g_combatResourceFile, g_combatEnemyResourceNames[1], &rect,
									COMBAT_PREVIEW_FRAME_LEFT, COMBAT_PREVIEW_FRAME_TOP, COMBAT_PREVIEW_Z);
		xactor_Set_Actor_User_Function(g_combatEnemyPreviewActors[1], combat_user_TieInterceptor);
		g_combatEnemyPreviewActors[2] =
			xactanim_Res_Anim_Actor(g_combatResourceFile, g_combatEnemyResourceNames[2], &rect,
									COMBAT_PREVIEW_FRAME_LEFT, COMBAT_PREVIEW_FRAME_TOP, COMBAT_PREVIEW_Z);
		xactor_Set_Actor_User_Function(g_combatEnemyPreviewActors[2], combat_user_TieBomber);
		for (enemyIndex = 0; enemyIndex < COMBAT_ENEMY_PREVIEW_COUNT; ++enemyIndex) {
			g_combatEnemyPreviewActors[enemyIndex]->id = enemyIndex;
			g_combatEnemyPreviewActors[enemyIndex]->var1 = COMBAT_PREVIEW_ROLE;
		}
	}
	xio_Set_Key_Buttons();
	combat_OpenMusic(g_combatResourceFile, g_combatFilm, &g_combatMonitorTime);
	combat_LoadSoundEffects();
	FrontendAudio_PlayFile("XwingCD\\music\\combat.wav", 1u);
	xview_Set_View_Update_Function(combat_end_Combat_View);
#ifdef XW_MODERN
	XwCombat_RunView();
#else
	j_xviewadd_Handle_View();
	xview_Clear_View_Update_Function();
	selectedMission = shipext_Get_Combat_Mission();
	xparagrp_Get_Paragraph_String(g_combatMissionParagraphs[shipext_Get_Combat_Ship()], g_shellMissionName, 0,
								  selectedMission);
	xparagrp_Free_Paragraph(g_combatShipListText);
	for (releaseIndex = 0; releaseIndex < g_combatAvailableShipCount + g_combatAvailableTourCount;
		 ++releaseIndex)
		xparagrp_Free_Paragraph(g_combatMissionParagraphs[releaseIndex]);
	xview_Enable_All_View_Erase();
	combat_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xio_Clear_Key_Buttons();
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	LandruDisplay_ForwardLegacyNoOp(0);
	xres_Close_Resource(g_combatResourceFile);
	xmemhdl_Free_Handle(g_combatScoreHandle);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x43F6E0
void combat_end_Combat_View(int time) {
	int16_t key;
	if (time == 0 && !xcursor_Is_Cursor_Visible())
		xcursor_Show_Cursor();
	if (time == COMBAT_HYDRAULICS_START_FRAME)
		combat_HandleSoundAction(COMBAT_SOUND_HYDRAULICS_START);
	if (time == COMBAT_HYDRAULICS_STOP_FRAME)
		combat_HandleSoundAction(COMBAT_SOUND_HYDRAULICS_STOP);
	combat_HandleSoundAction(COMBAT_SOUND_TYPING_UPDATE);
	key = xio_Get_Free_Key();
	if (key && shellext_MoveGridFocus(&g_combatKeyboardFocusIndex, g_combatKeyboardFocusX,
									  g_combatKeyboardFocusY, COMBAT_FOCUS_ROWS, COMBAT_FOCUS_COLUMNS, key)) {
		xio_Set_Mouse_Position(g_combatKeyboardFocusX[g_combatKeyboardFocusIndex],
							   g_combatKeyboardFocusY[g_combatKeyboardFocusIndex]);
		xio_Get_Key();
	}
	if (time == COMBAT_SCREEN_RELEASE_FRAME) {
		g_combatScreenActor = xactor_Find_Actor(FOURCC_ANIM, "screen");
		xactor_Non_Refreshable_Actor(g_combatScreenActor);
		g_combatScreenActor = NULL;
	}
	if (time < g_combatFilm->cels) {
		xactor_Refresh_Actor(g_combatScreenActor);
		xactor_Refresh_Actor(g_combatFilmTag3Actor);
		xactor_Refresh_Actor(g_combatNavigationRevealActor);
		xactor_Refresh_Actor(g_combatShipIconFrameActor);
		if (xinpattr_Is_Input_Visible(g_combatNavigationInput))
			xinpattr_Refresh_Input(g_combatNavigationInput);
		if (xinpattr_Is_Input_Visible(g_combatHoverLabelInput))
			xinpattr_Refresh_Input(g_combatHoverLabelInput);
	}
	if (time >= g_combatFilm->cels - COMBAT_MONITOR_INTRO_OVERLAP)
		g_combatMonitorTime = (g_combatMonitorTime + 1) % COMBAT_MONITOR_PERIOD;
	combat_LoadPreviewOnDemand(g_combatMonitorTime);
}

// FUNCTION: XW 0x43F870
int16_t combat_film_Combat_Callback(Film* film, FilmObject* filmObject) {
	if (filmObject->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, filmObject, filmObject + 1);
		actor = filmObject->object;
		switch (actor->var1) {
			case COMBAT_ROLE_TAG_1:
				g_combatFilmTag1Actor = actor;
				xactor_Non_Refreshable_Actor(actor);
				break;
			case COMBAT_ROLE_SCREEN:
				g_combatScreenActor = actor;
				xactor_Set_Actor_User_Function(actor, combat_user_Door);
				xactor_Non_Refreshable_Actor(g_combatScreenActor);
				g_combatScreenActor->id = COMBAT_DOOR_SCREEN;
				break;
			case COMBAT_ROLE_TAG_3:
				g_combatFilmTag3Actor = actor;
				xactor_Set_Actor_User_Function(actor, combat_user_Door);
				xactor_Non_Refreshable_Actor(g_combatFilmTag3Actor);
				g_combatFilmTag3Actor->id = COMBAT_DOOR_SCREEN;
				break;
			case COMBAT_ROLE_LEFT_DOOR:
				g_combatLeftDoorActor = actor;
				xactor_Set_Actor_User_Function(actor, combat_user_Door);
				xactor_Non_Refreshable_Actor(g_combatLeftDoorActor);
				g_combatLeftDoorActor->id = COMBAT_DOOR_LEFT;
				break;
			case COMBAT_ROLE_RIGHT_DOOR:
				g_combatRightDoorActor = actor;
				xactor_Set_Actor_User_Function(actor, combat_user_Door);
				xactor_Non_Refreshable_Actor(g_combatRightDoorActor);
				g_combatRightDoorActor->id = COMBAT_DOOR_RIGHT;
				break;
			case COMBAT_ROLE_NAVIGATION_REVEAL:
				g_combatNavigationRevealActor = actor;
				xactor_Set_Actor_User_Function(actor, combat_user_NavigationReveal);
				xactor_Non_Refreshable_Actor(g_combatNavigationRevealActor);
				break;
			case COMBAT_ROLE_SHIP_ICON_FRAME:
				g_combatShipIconFrameActor = actor;
				xactor_Set_Actor_User_Function(actor, combat_user_ShipIconFrame);
				xactor_Set_Actor_Draw_Function(g_combatShipIconFrameActor, combat_draw_ShipIconFrame);
				xactor_Non_Refreshable_Actor(g_combatShipIconFrameActor);
				break;
			case COMBAT_ROLE_FIRST_DECOR:
			case COMBAT_ROLE_SECOND_DECOR:
				xactor_Set_Actor_User_Function(actor, combat_user_DecorState);
				break;
			case COMBAT_ROLE_RANDOM_ANIMATION:
				if (rand() & 1)
					xactor_Set_Actor_Time(actor, 0, 0);
				break;
		}
	}
	return 0;
}

// FUNCTION: XW 0x43FA50
int16_t combat_iupdate_Combat_Screen(Input* input, Rect* frame, Rect* clip, int16_t key, uint8_t leftEvent,
									 uint8_t rightEvent, int16_t x, int16_t y) {
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (key == 0 && (leftEvent == COMBAT_MOUSE_PRESS || rightEvent == COMBAT_MOUSE_PRESS)) {
		input->var1 = 1;
	}
	return 0;
}

// FUNCTION: XW 0x43FA80
void combat_iuser_Combat_Screen(Input* input, int context) {
	(void)context;
	if (input->var1) {
		if ((uint16_t)xactor_Is_Actor_Visible(g_combatShipInfoActor) != 0) {
			xactor_Hide_Actor(g_combatShipInfoActor);
			g_combatShipInfoActor->var1 = 0;
			g_combatShipInfoActor->var2 = 0;
		}
		if (g_combatMonitorTime < COMBAT_MISSION_TEXT_END) {
			g_combatMonitorTime = COMBAT_MISSION_TEXT_END - 1;
		} else if (g_combatMonitorTime < COMBAT_SCORE_END) {
			g_combatMonitorTime = COMBAT_SCORE_END - 1;
		} else if (g_combatMonitorTime < COMBAT_MONITOR_INTERVAL_3_END) {
			g_combatMonitorTime = COMBAT_MONITOR_INTERVAL_3_END - 1;
		} else if (g_combatMonitorTime < COMBAT_MONITOR_INTERVAL_4_END) {
			g_combatMonitorTime = COMBAT_MONITOR_INTERVAL_4_END - 1;
		} else if (g_combatMonitorTime < COMBAT_MONITOR_INTERVAL_5_END) {
			g_combatMonitorTime = COMBAT_MONITOR_INTERVAL_5_END - 1;
		} else {
			g_combatMonitorTime = COMBAT_WELCOME_END - 1;
		}
		input->var1 = 0;
	}
}

// FUNCTION: XW 0x43FB60
int16_t combat_iupdate_Door(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent, int rightEvent,
							int16_t x, int16_t y) {
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (key != 0) {
		return 0;
	}
	if (leftEvent == COMBAT_MOUSE_SELECT || rightEvent == COMBAT_MOUSE_SELECT) {
		input->var1 = COMBAT_DOOR_EXIT_REQUEST;
		if (input->id != COMBAT_DOOR_LEFT) {
			input->var2 = XW_SCENE_BRIEFING_COMBAT;
		} else {
			input->var2 = XW_SCENE_COMBAT_RETURN_SHUTTLE;
		}
	} else {
		input->var1 = COMBAT_DOOR_HOVER_REQUEST;
	}
	if (input->id != COMBAT_DOOR_LEFT) {
		g_combatRightDoorActor->var1 = 1;
	} else {
		g_combatLeftDoorActor->var1 = 1;
	}
	return 1;
}

// FUNCTION: XW 0x43FBD0
void combat_iuser_Door(Input* input, int context) {
	(void)context;
	switch (input->var1) {
		case COMBAT_DOOR_IDLE:
			if ((uint16_t)xinpattr_Is_Input_Visible(g_combatHoverLabelInput) != 0 &&
				g_combatHoverLabelInput->var1 == input->id) {
				xinpattr_Show_Input(g_combatNavigationInput);
				xinpattr_Hide_Input(g_combatHoverLabelInput);
				xactor_Refresh_Actor(g_combatNavigationRevealActor);
				xactor_Refresh_Actor(g_combatShipIconFrameActor);
				xinpattr_Refresh_Input(g_combatNavigationInput);
			}
			break;
		case COMBAT_DOOR_EXIT_REQUEST:
			xerror_Set_Landru_Exit(input->var2);
			break;
		case COMBAT_DOOR_HOVER_REQUEST:
			if ((uint16_t)xinpattr_Is_Input_Visible(g_combatNavigationInput) != 0) {
				xinpattr_Hide_Input(g_combatNavigationInput);
				xinpattr_Show_Input(g_combatHoverLabelInput);
				g_combatHoverLabelInput->var1 = input->id;
				xactor_Refresh_Actor(g_combatNavigationRevealActor);
				xactor_Refresh_Actor(g_combatShipIconFrameActor);
				xinpattr_Refresh_Input(g_combatHoverLabelInput);
			}
			input->var1 = COMBAT_DOOR_IDLE;
			break;
	}
}

// FUNCTION: XW 0x43FCE0
void combat_idraw_DoorLabel(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	(void)clip;
	if (refresh != 0) {
		xpaint_Paint_Clipped_Rect(frame, COMBAT_DOOR_LABEL_BACKGROUND_COLOR);
		if (input->var1 != 0) {
			xrect_Offset_Rect(frame, COMBAT_DOOR_LABEL_SHADOW_OFFSET, COMBAT_DOOR_LABEL_SHADOW_OFFSET);
			xfont_Print_Centered_Text("Enter Combat Briefing", frame, COMBAT_DOOR_LABEL_FONT,
									  COMBAT_DOOR_LABEL_SHADOW_COLOR);
			xrect_Offset_Rect(frame, -COMBAT_DOOR_LABEL_SHADOW_OFFSET, -COMBAT_DOOR_LABEL_SHADOW_OFFSET);
			xfont_Print_Centered_Text("Enter Combat Briefing", frame, COMBAT_DOOR_LABEL_FONT,
									  COMBAT_DOOR_LABEL_TEXT_COLOR);
		} else {
			xrect_Offset_Rect(frame, COMBAT_DOOR_LABEL_SHADOW_OFFSET, COMBAT_DOOR_LABEL_SHADOW_OFFSET);
			xfont_Print_Centered_Text("Return To Spaceport", frame, COMBAT_DOOR_LABEL_FONT,
									  COMBAT_DOOR_LABEL_SHADOW_COLOR);
			xrect_Offset_Rect(frame, -COMBAT_DOOR_LABEL_SHADOW_OFFSET, -COMBAT_DOOR_LABEL_SHADOW_OFFSET);
			xfont_Print_Centered_Text("Return To Spaceport", frame, COMBAT_DOOR_LABEL_FONT,
									  COMBAT_DOOR_LABEL_TEXT_COLOR);
		}
	}
}

// FUNCTION: XW 0x43FD90
void combat_iuser_Combat(Input* input, int time) {
	(void)time;
	if (xinpattr_Get_Input_Selected(input) != 0) {
		int16_t selectedShip = shipext_Get_Combat_Ship();
		int16_t selectedMission = shipext_Get_Combat_Mission();
		int16_t selectionChanged = 0;
		switch (input->id) {
			case COMBAT_BUTTON_PREVIOUS_SHIP:
				do {
					if (selectedShip != 0)
						--selectedShip;
					else
						selectedShip = g_combatAvailableShipCount + g_combatAvailableTourCount - 1;
					selectionChanged = selectedShip >= g_combatAvailableShipCount ||
									   shipext_IsShipAvailable(selectedShip) != 0;
				} while (selectionChanged == 0);
				shipext_Set_Combat_Ship(selectedShip, selectedShip >= g_combatAvailableShipCount);
				shipext_Set_Combat_Source_Index(g_combatSelectionSourceIndices[selectedShip]);
				break;
			case COMBAT_BUTTON_NEXT_SHIP:
				do {
					if (selectedShip < g_combatAvailableShipCount + g_combatAvailableTourCount - 1)
						++selectedShip;
					else
						selectedShip = 0;
					selectionChanged = selectedShip >= g_combatAvailableShipCount ||
									   shipext_IsShipAvailable(selectedShip) != 0;
				} while (selectionChanged == 0);
				shipext_Set_Combat_Ship(selectedShip, selectedShip >= g_combatAvailableShipCount);
				shipext_Set_Combat_Source_Index(g_combatSelectionSourceIndices[selectedShip]);
				break;
			case COMBAT_BUTTON_PREVIOUS_MISSION:
				if (selectedMission != 0)
					shipext_Set_Combat_Mission(selectedMission - 1);
				else
					shipext_Set_Combat_Mission(g_combatMissionCounts[selectedShip] - 1);
				selectionChanged = 1;
				break;
			case COMBAT_BUTTON_NEXT_MISSION:
				if (selectedMission < g_combatMissionCounts[selectedShip] - 1)
					shipext_Set_Combat_Mission(selectedMission + 1);
				else
					shipext_Set_Combat_Mission(0);
				selectionChanged = 1;
				break;
		}
		if (selectionChanged != 0) {
			xactor_Refresh_Actor(g_combatNavigationRevealActor);
			xactor_Refresh_Actor(g_combatShipIconFrameActor);
			xinpattr_Refresh_Input(g_combatNavigationInput);
			combat_ShowMissionText();
		}
	}
}

// FUNCTION: XW 0x43FF00
void combat_idraw_ArrowButton(PushButton* button, Rect* frame, Rect* clip, int16_t refresh) {
	if (refresh != 0) {
		Actor* buttonActor;
		switch (button->header.id) {
			case COMBAT_BUTTON_PREVIOUS_SHIP:
				buttonActor = g_combatPreviousShipButtonActor;
				break;
			case COMBAT_BUTTON_NEXT_SHIP:
				buttonActor = g_combatNextShipButtonActor;
				break;
			case COMBAT_BUTTON_PREVIOUS_MISSION:
				buttonActor = g_combatPreviousMissionButtonActor;
				break;
			case COMBAT_BUTTON_NEXT_MISSION:
				buttonActor = g_combatNextMissionButtonActor;
				break;
			default:
				buttonActor = NULL;
				break;
		}
		if (buttonActor != NULL) {
			xactor_Set_Actor_State(buttonActor, button->pressed, 0);
			xactanim_Draw_Anim_Actor(buttonActor, frame, clip, 0, 0, refresh);
		}
	}
}

// FUNCTION: XW 0x43FF80
int16_t combat_iupdate_SelectionLabel(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
									  int rightEvent, int16_t x, int16_t y) {
	(void)input;
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (key == 0 && (leftEvent == COMBAT_MOUSE_PRESS || rightEvent == COMBAT_MOUSE_PRESS)) {
		if (xactor_Is_Actor_Visible(g_combatMissionTextActor) != 0) {
			g_combatMonitorTime = COMBAT_MISSION_TEXT_START;
			g_combatMissionTextActor->var2 = COMBAT_MISSION_TEXT_REVEAL_ALL;
		} else {
			combat_ShowMissionText();
		}
	}
	return 0;
}

// FUNCTION: XW 0x43FFE0
void combat_idraw_Combat(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	int16_t selectedShip = shipext_Get_Combat_Ship();
	int16_t selectedMission = shipext_Get_Combat_Mission();
	char missionNumber[COMBAT_MISSION_NUMBER_CAPACITY];
	char labelText[COMBAT_SELECTION_LABEL_CAPACITY];
	(void)clip;
	if (refresh != 0) {
		xpaint_Paint_Clipped_Rect(frame, COMBAT_SELECTION_LABEL_BACKGROUND_COLOR);
		if (input->id == COMBAT_SELECTION_SHIP_LABEL) {
			if (selectedShip >= g_combatAvailableShipCount)
				xparagrp_Get_Paragraph_String(g_combatShipListText, labelText, COMBAT_TOUR_NAMES_PARAGRAPH,
											  g_combatSelectionSourceIndices[selectedShip] -
												  SHIPEXT_SHIP_COUNT);
			else
				xparagrp_Get_Paragraph_String(g_combatShipListText, labelText, COMBAT_SHIP_NAMES_PARAGRAPH,
											  selectedShip);
		} else if (selectedShip >= g_combatAvailableShipCount) {
			int16_t selectedBaseMission = 0;
			int16_t baseMissionCount = 0;
			int16_t secondVariant = 0;
			int16_t selectedSecondVariant = 0;
			int16_t variantIndex;
			for (variantIndex = 0; variantIndex < g_combatMissionCounts[selectedShip]; ++variantIndex) {
				if (variantIndex == selectedMission) {
					selectedBaseMission = baseMissionCount;
					selectedSecondVariant = secondVariant;
				}
				if (g_tourOperationTables[g_combatSelectionSourceIndices[selectedShip] - SHIPEXT_SHIP_COUNT]
										 [baseMissionCount]
											 .missionChoiceB != SHIPEXT_TOUR_NO_ENTRY) {
					if (secondVariant != 0) {
						secondVariant = 0;
						++baseMissionCount;
					} else {
						secondVariant = 1;
					}
				} else {
					++baseMissionCount;
				}
			}
			strcpy(labelText, "Mission ");
			if (g_tourOperationTables[g_combatSelectionSourceIndices[selectedShip] - SHIPEXT_SHIP_COUNT]
									 [selectedBaseMission]
										 .missionChoiceB != SHIPEXT_TOUR_NO_ENTRY) {
				if (selectedSecondVariant != 0)
					sprintf(missionNumber, "%db/%d", selectedBaseMission + 1, baseMissionCount);
				else
					sprintf(missionNumber, "%da/%d", selectedBaseMission + 1, baseMissionCount);
			} else {
				sprintf(missionNumber, "%d/%d", selectedBaseMission + 1, baseMissionCount);
			}
			strcat(labelText, missionNumber);
		} else {
			sprintf(labelText, "Mission %d/%d", selectedMission + 1, g_combatMissionCounts[selectedShip]);
		}
		xfont_Print_Centered_Text(labelText, frame, COMBAT_SELECTION_LABEL_FONT,
								  COMBAT_SELECTION_LABEL_TEXT_COLOR);
	}
}

// FUNCTION: XW 0x4401D0
void combat_user_DecorState(Actor* actor, int time) {
	if (time == 0) {
		int16_t stateCount = actor->arraySize - COMBAT_DECOR_SKIPPED_STATE;
		int16_t selectedState = rand() % stateCount;
		if (selectedState != 0) {
			++selectedState;
		}
		if (shellext_Check_Last_Scene(XW_SCENE_BRIEFING_COMBAT) != 0) {
			selectedState = g_combatSavedDecorStates[actor->var1 - COMBAT_DECOR_ROLE_BASE];
		}
		xactor_Set_Actor_State(actor, selectedState, 0);
		g_combatSavedDecorStates[actor->var1 - COMBAT_DECOR_ROLE_BASE] = selectedState;
	}
}

// FUNCTION: XW 0x440230
void combat_user_Stars(Actor* actor, int time) {
	(void)time;
	if (!xactor_Is_Actor_Visible(actor)) {
		xactor_Show_Actor(actor);
	}
	actor->x += COMBAT_STARFIELD_STEP;
	if (actor->x >= COMBAT_STARFIELD_WIDTH) {
		actor->x -= COMBAT_STARFIELD_WRAP;
	}
}

// FUNCTION: XW 0x440270
void combat_user_Door(Actor* actor, int time) {
	int16_t needsRefresh = 1;
	(void)time;
	if (!actor->var1) {
		if (actor->state == 1) {
			combat_HandleSoundAction(COMBAT_SOUND_DOOR_CLOSE);
		}
		if (actor->state) {
			xactor_Set_Actor_State(actor, actor->state - 1, 0);
		} else {
			needsRefresh = 0;
		}
	} else {
		if (actor->state == 0) {
			combat_HandleSoundAction(COMBAT_SOUND_DOOR_OPEN);
		}
		if (actor->state != actor->arraySize - 1) {
			xactor_Set_Actor_State(actor, actor->state + 1, 0);
		} else {
			needsRefresh = 0;
		}
		actor->var1 = 0;
	}
	if (needsRefresh) {
		xactor_Refresh_Actor(actor);
	}
}

// FUNCTION: XW 0x440300
void combat_user_NavigationReveal(Actor* actor, int time) {
	(void)time;
	if (actor->state == actor->arraySize - 1 && xinpattr_Is_Input_Visible(g_combatNavigationInput) == 0 &&
		xinpattr_Is_Input_Visible(g_combatHoverLabelInput) == 0) {
		xinpattr_Show_Input(g_combatNavigationInput);
	}
}

// FUNCTION: XW 0x440350
void combat_user_ShipIconFrame(Actor* actor, int time) {
	if (time < g_combatFilm->cels - 1) {
		xactor_Refresh_Actor(actor);
	}
}

// FUNCTION: XW 0x440380
int16_t combat_draw_ShipIconFrame(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
								  int16_t refresh) {
	int16_t selectedShip = shipext_Get_Combat_Ship();
	Rect iconFrame;
	if (refresh) {
		xactdelt_Draw_Delta_Actor(actor, frame, clip, x, y, refresh);
		if (selectedShip < g_combatAvailableShipCount && selectedShip != SHIPEXT_DEFAULT_COMBAT_SELECTION) {
			xrect_Set_Rect(&iconFrame, x, y, x + actor->w, y + actor->h);
			xstyle_Style_Draw_Centered_Actor(g_combatShipIconsActor, &iconFrame, clip, selectedShip);
		} else {
			xrect_Set_Rect(&iconFrame, x, y, x + actor->w, y + actor->h);
			xstyle_Style_Draw_Centered_Actor(g_combatFallbackIconActor, &iconFrame, clip, 0);
		}
	}
	return 0;
}

// FUNCTION: XW 0x440450
void combat_user_TieFighter(Actor* actor, int time) {
	(void)time;
	if (g_combatMonitorTime >= COMBAT_TIE_FIGHTER_START &&
		g_combatMonitorTime < COMBAT_MONITOR_INTERVAL_3_END) {
		switch (g_combatMonitorTime) {
			case COMBAT_TIE_FIGHTER_START:
				xactor_Show_Actor(actor);
				xactor_Set_Actor_State_Speed(actor, 1, 0);
				actor->state = 0;
				break;
			case COMBAT_TIE_FIGHTER_INFO:
				xactor_Show_Actor(g_combatShipInfoActor);
				xactor_Set_Actor_State_Speed(actor, 0, 0);
				g_combatShipInfoActor->var1 = COMBAT_SHIP_INFO_PHASE_1;
				g_combatShipInfoActor->var2 = 0;
				g_combatCurrentEnemyPreview = actor;
				break;
			case COMBAT_TIE_FIGHTER_RESUME:
				xactor_Set_Actor_State_Speed(actor, 1, 0);
				xactor_Hide_Actor(g_combatShipInfoActor);
				g_combatShipInfoActor->var1 = COMBAT_SHIP_INFO_IDLE;
				break;
		}
	} else if ((uint16_t)xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Set_Actor_Pos(actor, COMBAT_ENEMY_PREVIEW_X, COMBAT_ENEMY_PREVIEW_Y, 0, 0);
		xactor_Set_Actor_State(actor, 0, 0);
		xactor_Set_Actor_State_Speed(actor, 0, 0);
		xactor_Hide_Actor(actor);
	}
}

// FUNCTION: XW 0x440560
void combat_user_TieInterceptor(Actor* actor, int time) {
	(void)time;
	if (g_combatMonitorTime >= COMBAT_TIE_INTERCEPTOR_START &&
		g_combatMonitorTime < COMBAT_MONITOR_INTERVAL_4_END) {
		switch (g_combatMonitorTime) {
			case COMBAT_TIE_INTERCEPTOR_START:
				xactor_Show_Actor(actor);
				xactor_Set_Actor_State_Speed(actor, 1, 0);
				actor->state = 0;
				break;
			case COMBAT_TIE_INTERCEPTOR_INFO:
				xactor_Show_Actor(g_combatShipInfoActor);
				xactor_Set_Actor_State_Speed(actor, 0, 0);
				g_combatShipInfoActor->var1 = COMBAT_SHIP_INFO_PHASE_1;
				g_combatShipInfoActor->var2 = 0;
				g_combatCurrentEnemyPreview = actor;
				break;
			case COMBAT_TIE_INTERCEPTOR_RESUME:
				xactor_Set_Actor_State_Speed(actor, 1, 0);
				xactor_Hide_Actor(g_combatShipInfoActor);
				g_combatShipInfoActor->var1 = COMBAT_SHIP_INFO_IDLE;
				break;
		}
	} else if ((uint16_t)xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Set_Actor_Pos(actor, COMBAT_ENEMY_PREVIEW_X, COMBAT_ENEMY_PREVIEW_Y, 0, 0);
		xactor_Set_Actor_State(actor, 0, 0);
		xactor_Set_Actor_State_Speed(actor, 0, 0);
		xactor_Hide_Actor(actor);
	}
}

// FUNCTION: XW 0x440670
void combat_user_TieBomber(Actor* actor, int time) {
	(void)time;
	if (g_combatMonitorTime >= COMBAT_TIE_BOMBER_START &&
		g_combatMonitorTime < COMBAT_MONITOR_INTERVAL_5_END) {
		switch (g_combatMonitorTime) {
			case COMBAT_TIE_BOMBER_START:
				xactor_Show_Actor(actor);
				xactor_Set_Actor_State_Speed(actor, 1, 0);
				actor->state = 0;
				break;
			case COMBAT_TIE_BOMBER_INFO:
				xactor_Show_Actor(g_combatShipInfoActor);
				xactor_Set_Actor_State_Speed(actor, 0, 0);
				g_combatShipInfoActor->var1 = COMBAT_SHIP_INFO_PHASE_1;
				g_combatShipInfoActor->var2 = 0;
				g_combatCurrentEnemyPreview = actor;
				break;
			case COMBAT_TIE_BOMBER_RESUME:
				xactor_Set_Actor_State_Speed(actor, 1, 0);
				xactor_Hide_Actor(g_combatShipInfoActor);
				g_combatShipInfoActor->var1 = COMBAT_SHIP_INFO_IDLE;
				break;
		}
	} else if ((uint16_t)xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Set_Actor_Pos(actor, COMBAT_TIE_BOMBER_X, COMBAT_TIE_BOMBER_Y, 0, 0);
		xactor_Set_Actor_State(actor, 0, 0);
		xactor_Set_Actor_State_Speed(actor, 0, 0);
		xactor_Hide_Actor(actor);
	}
}

// FUNCTION: XW 0x440780
void combat_update_ShipInfo(Actor* actor) {
	int16_t phase = actor->var1;
	switch (phase) {
		case COMBAT_SHIP_INFO_PHASE_1:
			if (++actor->var2 == COMBAT_SHIP_INFO_PHASE_1_TICKS) {
				actor->var2 = 0;
				actor->var1 = phase + 1;
			}
			break;
		case COMBAT_SHIP_INFO_PHASE_2:
			++actor->var2;
			break;
	}
}

// FUNCTION: XW 0x4407B0
void combat_user_ShipInfo(Actor* actor, int time) {
	(void)time;
	if (g_combatMonitorTime == 0 && xactor_Is_Actor_Visible(actor)) {
		xactor_Hide_Actor(actor);
		actor->var1 = COMBAT_SHIP_INFO_IDLE;
	}
}

// FUNCTION: XW 0x4407E0
int16_t combat_Draw_Combat_Screen_Flyby(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
										int16_t refresh) {
	Rect enemyRect;
	Rect titleRect;
	(void)clip;
	(void)x;
	(void)y;
	(void)refresh;
	if (actor->var1 == COMBAT_SHIP_INFO_PHASE_1 && actor->var2 == 1)
		combat_HandleSoundAction(COMBAT_SOUND_TARGET);
	switch (actor->var1) {
		case COMBAT_SHIP_INFO_PHASE_1: {
			int16_t tick;
			int16_t borderCount;
			int16_t borderColor;
			uint32_t borderIndex;
			xactor_Get_Actor_Rect(g_combatCurrentEnemyPreview, &enemyRect);
			tick = actor->var2 - 1;
			if (tick < COMBAT_ENEMY_BORDER_GROW_END) {
				xrect_Inset_Rect(&enemyRect, -COMBAT_ENEMY_BORDER_INITIAL_EXPANSION,
								 -COMBAT_ENEMY_BORDER_INITIAL_EXPANSION);
				borderColor = COMBAT_ENEMY_COLOR_MAX - COMBAT_ENEMY_COLOR_STEP * tick;
				borderCount = tick + 1;
			} else if (tick < COMBAT_ENEMY_BORDER_SHRINK_START) {
				int16_t inset = COMBAT_ENEMY_BORDER_STEP * (tick - COMBAT_ENEMY_BORDER_END);
				xrect_Inset_Rect(&enemyRect, inset, inset);
				borderColor = COMBAT_ENEMY_COLOR_MIN;
				borderCount = COMBAT_ENEMY_BORDER_MAX_COUNT;
			} else {
				int16_t inset;
				borderCount = COMBAT_ENEMY_BORDER_END - tick;
				inset = COMBAT_ENEMY_BORDER_STEP * borderCount;
				xrect_Inset_Rect(&enemyRect, -inset, -inset);
				borderColor = COMBAT_ENEMY_COLOR_MIN;
			}
			if (borderCount != 0) {
				borderIndex = borderCount;
				do {
					xpaint_Frame_Clipped_Rect(&enemyRect, borderColor);
					xrect_Inset_Rect(&enemyRect, COMBAT_ENEMY_BORDER_STEP, COMBAT_ENEMY_BORDER_STEP);
					borderColor += COMBAT_ENEMY_COLOR_STEP;
				} while (--borderIndex != 0);
			}
			break;
		}
		case COMBAT_SHIP_INFO_PHASE_2: {
			int16_t titleColor =
				COMBAT_ENEMY_TITLE_COLOR_STEP * (actor->var2 + COMBAT_ENEMY_TITLE_COLOR_START);
			int16_t enemyIndex;
			int16_t specX;
			int16_t specY;
			int16_t firstLine;
			int16_t endLine;
			int16_t revealChars;
			int16_t lineDelay;
			int16_t lineIndex;
			if (titleColor > COMBAT_ENEMY_COLOR_MAX)
				titleColor = COMBAT_ENEMY_COLOR_MAX;
			xrect_Copy_Rect(&titleRect, frame);
			titleRect.bottom = titleRect.top + COMBAT_ENEMY_TITLE_HEIGHT;
			xrect_Offset_Rect(&titleRect, 0, COMBAT_ENEMY_TITLE_TOP);
			enemyIndex = g_combatCurrentEnemyPreview->id;
			xfont_Print_Centered_Text(g_combatEnemyText[0], &titleRect, COMBAT_ENEMY_TITLE_FONT, titleColor);
			xrect_Offset_Rect(&titleRect, 0, COMBAT_ENEMY_TITLE_SPACING);
			xfont_Print_Centered_Text(g_combatEnemyText[enemyIndex + 1], &titleRect, COMBAT_ENEMY_TITLE_FONT,
									  titleColor);
			specX = g_combatEnemySpecX[enemyIndex];
			firstLine = g_combatEnemySpecLineBounds[enemyIndex];
			endLine = g_combatEnemySpecLineBounds[enemyIndex + 1];
			specY = g_combatEnemySpecY[enemyIndex];
			revealChars = actor->var2;
			lineDelay = revealChars;
			if (firstLine < endLine) {
				for (lineIndex = firstLine; lineIndex < endLine; ++lineIndex) {
					int16_t textY = frame->top;
					int16_t textX = specX;
					textX += frame->left;
					textY += specY;
					textext_Draw_Typewriter_Line(g_combatEnemyText[lineIndex], COMBAT_ENEMY_SPEC_FONT, textX,
												 textY, revealChars);
					revealChars = actor->var2;
					/* The original updates this delay but passes var2 to every line. */
					if (lineDelay == revealChars)
						lineDelay -= COMBAT_ENEMY_LINE_DELAY;
					specY += COMBAT_ENEMY_SPEC_SPACING;
				}
			}
			xactor_Get_Actor_Rect(g_combatCurrentEnemyPreview, &enemyRect);
			xrect_Inset_Rect(&enemyRect, -COMBAT_ENEMY_BORDER_STEP, -COMBAT_ENEMY_BORDER_STEP);
			xpaint_Paint_Clipped_Rect(&enemyRect, COMBAT_ENEMY_BACKGROUND_COLOR);
			xpaint_Frame_Clipped_Rect(&enemyRect, COMBAT_ENEMY_FRAME_COLOR);
			return 1;
		}
		default:
			break;
	}
	return 1;
}

// FUNCTION: XW 0x440A40
void combat_user_Score(Actor* actor, int time) {
	(void)time;
	if (g_combatMonitorTime != 0 && g_combatMonitorTime >= COMBAT_SCORE_START &&
		g_combatMonitorTime < COMBAT_SCORE_END) {
		if (actor->var1 != 0) {
			actor->var2 += COMBAT_SCORE_REVEAL_STEP;
		} else {
			xactor_Show_Actor(actor);
			actor->var1 = 1;
			actor->var2 = 0;
		}
	} else if (xactor_Is_Actor_Visible(actor)) {
		xactor_Hide_Actor(actor);
		actor->var1 = 0;
	}
}

// FUNCTION: XW 0x440AB0
int16_t combat_Draw_Combat_Screen_Score(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
										int16_t refresh) {
	char pilotName[COMBAT_SCORE_PILOT_TEXT_CAPACITY];
	char scoreText[COMBAT_SCORE_TEXT_CAPACITY];
	(void)clip;
	(void)x;
	(void)y;
	if (refresh != 0) {
		XwMissionHighScores* missionScores;
		int16_t missionIndex;
		int16_t mission;
		combat_Load_Combat_High_Scores();
		mission = shipext_Get_Combat_Mission();
		xparagrp_Get_Paragraph_String(g_combatMissionParagraphs[shipext_Get_Combat_Ship()],
									  g_shellMissionName, 0, mission);
		missionIndex = 0;
		missionScores = xmemhdl_Lock_Handle(g_combatScoreHandle);
		for (; strcmp(missionScores[missionIndex].missionName, g_shellMissionName) != 0; ++missionIndex) {
			if (missionScores[missionIndex].missionName[0] == 0 || missionIndex >= g_combatScoreMissionCount)
				break;
		}
		if (missionIndex < g_combatScoreMissionCount && missionScores[missionIndex].missionName[0] != 0) {
			int16_t revealPosition = actor->var2;
			int16_t textX = frame->left + COMBAT_SCORE_MARGIN;
			int16_t textY, detailY, entryIndex;
			if (revealPosition > COMBAT_SCORE_SCROLL_LIMIT)
				textY = frame->top + COMBAT_SCORE_MARGIN;
			else
				textY = frame->top - revealPosition + COMBAT_SCORE_SCROLL_LIMIT + COMBAT_SCORE_MARGIN;
			detailY = textY + COMBAT_SCORE_DETAIL_Y_OFFSET;
			for (entryIndex = 0; entryIndex < COMBAT_SCORE_ENTRY_COUNT; ++entryIndex) {
				int16_t color;
				if (revealPosition < 0)
					break;
				color = revealPosition + COMBAT_SCORE_COLOR_BASE;
				if (revealPosition > COMBAT_SCORE_FADE_LIMIT)
					color = COMBAT_SCORE_COLOR_MAX;
				strcpy(pilotName, missionScores[missionIndex].entries[entryIndex].pilotName);
				if (pilotName[0] != 0) {
					int score;
					int16_t kills, nameWidth;
					xfont_Print_Clipped_Text(pilotName, textX, textY, COMBAT_SCORE_PILOT_FONT, color);
					score = missionScores[missionIndex].entries[entryIndex].score;
					kills = missionScores[missionIndex].entries[entryIndex].kills;
					nameWidth = xfont_Get_String_Width_0(COMBAT_SCORE_PILOT_FONT, "abcdefghijklmnopqr ");
					sprintf(scoreText, "Score %6ld    Kills %u", (long)score, (unsigned int)(int)kills);
					textext_Draw_Typewriter_Line(scoreText, COMBAT_SCORE_DETAIL_FONT, textX + nameWidth,
												 detailY, revealPosition);
				}
				revealPosition -= COMBAT_SCORE_ENTRY_REVEAL_STEP;
				textY += COMBAT_SCORE_LINE_HEIGHT;
				detailY += COMBAT_SCORE_LINE_HEIGHT;
			}
		} else
			xfont_Print_Centered_Text("No High Scores", frame, COMBAT_SCORE_EMPTY_FONT,
									  COMBAT_SCORE_COLOR_MAX);
		nullsub_SharedNoOp();
	}
	return 1;
}

// FUNCTION: XW 0x440D50
void combat_user_Welcome(Actor* actor, int time) {
	(void)time;
	if (g_combatMonitorTime != 0 && g_combatMonitorTime >= COMBAT_WELCOME_START &&
		g_combatMonitorTime < COMBAT_WELCOME_END) {
		if (actor->var1 != 0) {
			actor->var2 += COMBAT_WELCOME_REVEAL_STEP;
		} else {
			xactor_Show_Actor(actor);
			actor->var1 = 1;
			actor->var2 = 0;
		}
	} else if (xactor_Is_Actor_Visible(actor)) {
		xactor_Hide_Actor(actor);
		actor->var1 = 0;
	}
}

// FUNCTION: XW 0x440DC0
int16_t combat_draw_Welcome(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	(void)clip;
	(void)x;
	(void)y;
	if (refresh != 0) {
		int16_t revealBudget = actor->var2;
		int16_t drawX = frame->left + COMBAT_WELCOME_LEFT_INSET;
		int16_t drawY;
		int16_t lineIndex;
		int16_t lineLength;
		if (revealBudget > COMBAT_WELCOME_SCROLL_END)
			drawY = frame->top + COMBAT_WELCOME_TOP_INSET;
		else
			drawY = frame->top - revealBudget + COMBAT_WELCOME_SCROLL_ORIGIN;
		for (lineIndex = 0;
			 (lineLength = (int16_t)strlen(g_combatWelcomeLines[lineIndex])) != 0 && revealBudget >= 0;
			 ++lineIndex, revealBudget -= lineLength >> COMBAT_WELCOME_LENGTH_SHIFT) {
			combat_DrawSoundTypewriterLine(g_combatWelcomeLines[lineIndex], COMBAT_WELCOME_FONT, drawX, drawY,
										   revealBudget);
			drawY += COMBAT_WELCOME_LINE_SPACING;
		}
	}
	return 1;
}

// FUNCTION: XW 0x440E90
void combat_user_MissionText(Actor* actor, int time) {
	(void)time;
	if (g_combatMonitorTime != 0 && g_combatMonitorTime >= COMBAT_MISSION_TEXT_START &&
		g_combatMonitorTime < COMBAT_MISSION_TEXT_END) {
		if (actor->var1 != 0) {
			++actor->var2;
		} else {
			xactor_Show_Actor(actor);
			actor->var1 = 1;
			actor->var2 = 0;
		}
	} else if (xactor_Is_Actor_Visible(actor)) {
		xactor_Hide_Actor(actor);
		actor->var1 = 0;
	}
}

// FUNCTION: XW 0x440EF0
int16_t combat_Draw_Combat_Screen_Mission(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
										  int16_t refresh) {
	int16_t selectedShip = shipext_Get_Combat_Ship();
	int16_t shipIndex = selectedShip;
	int16_t selectedMission = shipext_Get_Combat_Mission();
	Rect titleRect;
	int16_t paragraphWidth;
	int16_t paragraphHeight;
	char text[COMBAT_MISSION_SCREEN_TEXT_CAPACITY];
	char missionNumber[COMBAT_MISSION_SCREEN_TEXT_CAPACITY];
	(void)clip;
	(void)x;
	(void)y;
	if (refresh != 0) {
		if (selectedShip >= g_combatAvailableShipCount)
			xparagrp_Get_Paragraph_String(g_combatShipListText, text, COMBAT_TOUR_NAMES_PARAGRAPH,
										  g_combatSelectionSourceIndices[selectedShip] - SHIPEXT_SHIP_COUNT);
		else
			xparagrp_Get_Paragraph_String(g_combatShipListText, text, COMBAT_SHIP_NAMES_PARAGRAPH,
										  selectedShip);
		strcat(text, " Mission ");
		if (selectedShip >= g_combatAvailableShipCount) {
			int16_t secondVariant = 0;
			int16_t operationIndex = 0;
			int16_t variantsRemaining;
			if (selectedMission > 0)
				for (variantsRemaining = selectedMission; variantsRemaining != 0; --variantsRemaining) {
					if (g_tourOperationTables[g_combatSelectionSourceIndices[selectedShip] -
											  SHIPEXT_SHIP_COUNT][operationIndex]
							.missionChoiceB != SHIPEXT_TOUR_NO_ENTRY) {
						if (secondVariant != 0) {
							secondVariant = 0;
							++operationIndex;
						} else {
							secondVariant = 1;
						}
					} else {
						++operationIndex;
					}
				}
			if (g_tourOperationTables[g_combatSelectionSourceIndices[selectedShip] - SHIPEXT_SHIP_COUNT]
									 [operationIndex]
										 .missionChoiceB != SHIPEXT_TOUR_NO_ENTRY) {
				if (secondVariant != 0)
					sprintf(missionNumber, g_combatMissionFormatB, operationIndex + 1);
				else
					sprintf(missionNumber, g_combatMissionFormatA, operationIndex + 1);
			} else {
				sprintf(missionNumber, "%d", operationIndex + 1);
			}
		} else {
			sprintf(missionNumber, "%d", selectedMission + 1);
		}
		strcat(text, missionNumber);
		xrect_Copy_Rect(&titleRect, frame);
		titleRect.top += COMBAT_MISSION_SCREEN_TOP_INSET;
		titleRect.bottom = titleRect.top + COMBAT_MISSION_SCREEN_TITLE_HEIGHT;
		xfont_Print_Centered_Text(text, &titleRect, COMBAT_MISSION_SCREEN_FONT, COMBAT_MISSION_SCREEN_COLOR);
		xrect_Offset_Rect(&titleRect, 0, COMBAT_MISSION_SCREEN_LINE_SPACING);
		xparagrp_Get_Paragraph_String(g_combatMissionParagraphs[shipIndex], text,
									  COMBAT_MISSION_TITLES_PARAGRAPH, selectedMission);
		xfont_Print_Centered_Text(text, &titleRect, COMBAT_MISSION_SCREEN_FONT, COMBAT_MISSION_SCREEN_COLOR);
		{
			int16_t revealBudget = actor->var2 - COMBAT_MISSION_SCREEN_REVEAL_DELAY;
			if (revealBudget > 0) {
				int16_t paragraph = selectedMission + COMBAT_MISSION_DESCRIPTION_PARAGRAPH_BASE;
				int16_t drawX;
				int16_t drawY;
				int16_t lineCount;
				int16_t lineIndex;
				xparagrp_Get_Paragraph_Size(g_combatMissionParagraphs[shipIndex], COMBAT_MISSION_SCREEN_FONT,
											paragraph, &paragraphWidth, &paragraphHeight);
				drawX = frame->left + ((frame->right - paragraphWidth - frame->left) >> 1);
				drawY = frame->top + COMBAT_MISSION_SCREEN_DESCRIPTION_TOP;
				lineCount = xparagrp_Count_Paragraph_Strings(g_combatMissionParagraphs[shipIndex], paragraph);
				for (lineIndex = 0; revealBudget > 0;) {
					if (lineIndex >= lineCount)
						break;
					xparagrp_Get_Paragraph_String(g_combatMissionParagraphs[shipIndex], text, paragraph,
												  lineIndex);
					combat_DrawSoundTypewriterLine(text, COMBAT_MISSION_SCREEN_FONT, drawX, drawY,
												   revealBudget);
					++lineIndex;
					revealBudget -= strlen(text) >> COMBAT_MISSION_SCREEN_LENGTH_SHIFT;
					drawY += COMBAT_MISSION_SCREEN_LINE_SPACING;
				}
			}
		}
	}
	return 1;
}

// FUNCTION: XW 0x4411E0
void combat_DrawSoundTypewriterLine(const char* text, uint16_t fontId, int16_t x, int16_t y,
									int16_t revealSteps) {
	if (revealSteps >= 0) {
		if (strlen(text) > (size_t)(TEXTEXT_CHARACTERS_PER_STEP * revealSteps)) {
			if (text[TEXTEXT_CHARACTERS_PER_STEP * revealSteps] == ' ' ||
				text[TEXTEXT_CHARACTERS_PER_STEP * revealSteps + 1] == ' ')
				combat_HandleSoundAction(COMBAT_SOUND_TYPING_STOP);
			else
				combat_HandleSoundAction(COMBAT_SOUND_TYPING_START);
		}
		textext_Draw_Typewriter_Line(text, fontId, x, y, TEXTEXT_CHARACTERS_PER_STEP * revealSteps);
	}
}

// FUNCTION: XW 0x4413C0
void combat_ShowMissionText(void) {
	g_combatMonitorTime = COMBAT_MISSION_TEXT_START;
	if (xactor_Is_Actor_Visible(g_combatMissionTextActor) != 0) {
		g_combatMissionTextActor->var1 = 1;
		g_combatMissionTextActor->var2 = COMBAT_MISSION_TEXT_REVEAL_ALL;
	}
	if (xactor_Is_Actor_Visible(g_combatShipInfoActor) != 0) {
		xactor_Hide_Actor(g_combatShipInfoActor);
		g_combatShipInfoActor->var1 = 0;
		g_combatShipInfoActor->var2 = 0;
	}
}

// FUNCTION: XW 0x441430
void combat_LoadPreviewOnDemand(int monitorTime) {
	if (Shared_ReturnZero() != 0) {
		int16_t loadRequested = 0;
		int16_t enemyIndex;
		if (monitorTime == COMBAT_TIE_FIGHTER_START - 1) {
			enemyIndex = COMBAT_ENEMY_FIGHTER;
			loadRequested = 1;
		} else {
			enemyIndex = monitorTime;
		}
		if (monitorTime == COMBAT_TIE_INTERCEPTOR_START - 1) {
			enemyIndex = COMBAT_ENEMY_INTERCEPTOR;
			loadRequested = 1;
		}
		if (monitorTime == COMBAT_TIE_BOMBER_START - 1) {
			enemyIndex = COMBAT_ENEMY_BOMBER;
			loadRequested = 1;
		}
		if (loadRequested != 0) {
			int16_t actorsRemaining;
			Rect monitorFrame;
			for (actorsRemaining = COMBAT_ENEMY_PREVIEW_COUNT; actorsRemaining != 0; --actorsRemaining) {
				if (g_combatEnemyPreviewActors[COMBAT_ENEMY_PREVIEW_COUNT - actorsRemaining] != NULL) {
					xactor_Free_Actor_From_System(
						g_combatEnemyPreviewActors[COMBAT_ENEMY_PREVIEW_COUNT - actorsRemaining]);
					xactor_Free_Actor(
						g_combatEnemyPreviewActors[COMBAT_ENEMY_PREVIEW_COUNT - actorsRemaining]);
					g_combatEnemyPreviewActors[COMBAT_ENEMY_PREVIEW_COUNT - actorsRemaining] = NULL;
				}
			}
			xrect_Set_Rect(&monitorFrame, COMBAT_PREVIEW_FRAME_LEFT, COMBAT_PREVIEW_FRAME_TOP,
						   COMBAT_PREVIEW_FRAME_RIGHT, COMBAT_PREVIEW_FRAME_BOTTOM);
			g_combatEnemyPreviewActors[enemyIndex] = xactanim_Res_Anim_Actor(
				g_combatResourceFile, g_combatEnemyResourceNames[enemyIndex], &monitorFrame,
				COMBAT_PREVIEW_FRAME_LEFT, COMBAT_PREVIEW_FRAME_TOP, COMBAT_PREVIEW_Z);
			xactor_Start_Actor(g_combatEnemyPreviewActors[enemyIndex]);
			g_combatEnemyPreviewActors[enemyIndex]->id = enemyIndex;
			g_combatEnemyPreviewActors[enemyIndex]->var1 = COMBAT_PREVIEW_ROLE;
			switch (enemyIndex) {
				case COMBAT_ENEMY_FIGHTER:
					xactor_Set_Actor_User_Function(g_combatEnemyPreviewActors[enemyIndex],
												   combat_user_TieFighter);
					break;
				case COMBAT_ENEMY_INTERCEPTOR:
					xactor_Set_Actor_User_Function(g_combatEnemyPreviewActors[enemyIndex],
												   combat_user_TieInterceptor);
					break;
				case COMBAT_ENEMY_BOMBER:
					xactor_Set_Actor_User_Function(g_combatEnemyPreviewActors[enemyIndex],
												   combat_user_TieBomber);
					break;
			}
		}
	}
}

// FUNCTION: XW 0x441590
void combat_ReadPilot(void) {
	char pilotFilename[COMBAT_PILOT_FILENAME_CAPACITY];
	LandruFile* pilotFile;
	strcpy(pilotFilename, g_RegisterShellPilot.name);
	strcat(pilotFilename, ".PLT");
	pilotFile = xfile_Open_File(LANDRU_FILE_ROOT_USER, pilotFilename, "rb");
	if (pilotFile != NULL) {
		register_ReadPilotRecord(pilotFile, &g_combatPilotData);
		xfile_Close_File(pilotFile);
	}
}

// FUNCTION: XW 0x441630
void combat_Load_Combat_High_Scores(void) {
	char filename[COMBAT_SCORE_FILENAME_CAPACITY];
	uint16_t fileMissionCount;
	int16_t missionCapacity;
	LandruFile* scoreFile;
	int16_t selectedGroup = shipext_Get_Combat_Source_Index();
	if (selectedGroup == g_combatScoreSelection)
		return;
	if (selectedGroup >= SHIPEXT_SHIP_COUNT) {
		sprintf(filename, "X-Wing Data\\tour%ld.hgh", (long)selectedGroup + 1);
		missionCapacity = COMBAT_TOUR_SCORE_MISSION_CAPACITY;
	} else {
		sprintf(filename, "X-Wing Data\\combat%ld.hgh", (long)selectedGroup + 1);
		missionCapacity = COMBAT_SCORE_MISSION_CAPACITY;
	}
	g_combatScoreHandle = xmemhdl_Alloc_Clear_Handle((int16_t)(missionCapacity * sizeof(XwMissionHighScores)),
													 LANDRU_MEMORY_RESOURCE);
	scoreFile = xfile_Open_File(LANDRU_FILE_ROOT_USER, filename, "rb");
	if (scoreFile != NULL) {
		xfile_Read_Word_From_File(scoreFile, &fileMissionCount);
		g_combatScoreMissionCount = (int16_t)fileMissionCount;
		if (g_combatScoreMissionCount > missionCapacity) {
			g_combatScoreMissionCount = 0;
		}
		if (g_combatScoreMissionCount != 0) {
			XwMissionHighScores* missionScores = xmemhdl_Lock_Handle(g_combatScoreHandle);
			int scoreBytes = g_combatScoreMissionCount * (int)sizeof(*missionScores);
			xres_Resource_Data_To_Buffer(scoreFile, missionScores, scoreBytes);
			xmemhdl_Unlock_Handle(g_combatScoreHandle);
		}
		xfile_Close_File(scoreFile);
	} else {
		g_combatScoreMissionCount = 0;
	}
	g_combatScoreSelection = selectedGroup;
}
