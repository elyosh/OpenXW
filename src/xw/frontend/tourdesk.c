#include "xw/frontend/tourdesk.h"

#ifdef XW_MODERN
#include "xw_runtime/integration/landru_adapter.h"
#endif

#include "xw/audio/frontend_audio.h"
#include "xw/audio/soundext.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"
#include "xw/util/landru_display.h"
#include "xw_runtime/integration/awards_callbacks.h"

#ifdef XW_MODERN
#include "xw_runtime/runtime/tourdesk_task.h"
#include "xw_runtime/runtime/tourdesk_view_task.h"
#endif

#include <landru/actanim.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/file.h>
#include <landru/font.h>
#include <landru/fourcc.h>
#include <landru/inpattr.h>
#include <landru/io.h>
#include <landru/paint.h>
#include <landru/paragrp.h>
#include <landru/view.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4D7D38
int16_t g_tourReplayScenes[XW_TOURDESK_REPLAY_CAPACITY] = { XW_SCENE_EMPIRE_ASSAULT_1,
															XW_SCENE_DESTROY_STAR_DESTROYER_1,
															XW_SCENE_SEND_PLANS_1,
															XW_SCENE_RECOVER_PLANS_1,
															XW_SCENE_DEATH_STAR_COMPLETED,
															XW_SCENE_DEATH_STAR_FIRE_1,
															XW_SCENE_DEATH_STAR_DESTRUCTION,
															XW_SCENE_YAVIN_DEPARTURE_1,
															XW_SCENE_GHORIN_1,
															XW_SCENE_IMPERIAL_DRYDOCK_1,
															XW_SCENE_RAMMING_ATTACK_1,
															XW_SCENE_BWING_ARRIVAL_1,
															XW_SCENE_IMPERIAL_PROBES_1,
															XW_SCENE_HOTH_1,
															XW_SCENE_TOUR_DESK,
															XW_SCENE_TOUR_DESK };

// GLOBAL: XW 0x4D7EF0
int16_t g_tourDeskFocusX[XW_TOURDESK_FOCUS_COLUMNS] = { 10, 42, 208, 310 };

// GLOBAL: XW 0x4D7EF8
int16_t g_tourDeskFocusY[XW_TOURDESK_FOCUS_COLUMNS] = { 128, 128, 128, 128 };

// GLOBAL: XW 0x4FA1E4
Sound* g_tourDeskMusic = NULL;

// GLOBAL: XW 0x4FA1E8
Sound* g_tourDeskPreviousMusic = NULL;

// GLOBAL: XW 0x4FA1EC
Film* g_tourDeskMusicFilm = NULL;

// GLOBAL: XW 0x4FA1F0
Sound* g_tourDeskSpeech[XW_TOURDESK_SPEECH_COUNT] = { NULL, NULL };

// GLOBAL: XW 0x4FADC8
Input* g_tourDeskHintInput = NULL;

// GLOBAL: XW 0x4FADCC
Actor* g_tourDeskBackgroundActor = NULL;

// GLOBAL: XW 0x4FADD0
Film* g_tourDeskFilm = NULL;

// GLOBAL: XW 0x4FADD4
Actor* g_tourDeskDoorActor = NULL;

// GLOBAL: XW 0x4FADDC
Input* g_tourDeskExitInput = NULL;

// GLOBAL: XW 0x4FADE0
Actor* g_tourDeskStarsActor = NULL;

// GLOBAL: XW 0x4FADE4
Actor* g_tourDeskNextButtonActor = NULL;

// GLOBAL: XW 0x4FADE8
LandruHandle g_tourDeskText = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4FADEC
Input* g_tourDeskEmptyHintInput = NULL;

// GLOBAL: XW 0x4FADF8
int16_t g_tourDeskReplayCutsceneIndices[XW_TOURDESK_REPLAY_CAPACITY] = { 0 };

// GLOBAL: XW 0x4FAE18
Actor* g_tourDeskPreviousButtonActor = NULL;

// GLOBAL: XW 0x4FAE1C
Input* g_tourDeskRootInput = NULL;

// GLOBAL: XW 0x4FAE20
int16_t g_tourDeskReplayCount = 0;

// GLOBAL: XW 0x4FAE28
REGISTER_PilotFileRecord g_tourDeskPilotRecord = { 0 };

// GLOBAL: XW 0x4FB4D4
Actor* g_tourDeskRobotActor = NULL;

// GLOBAL: XW 0x4FB4D8
Actor* g_tourDeskDeskActor = NULL;

// GLOBAL: XW 0x4FB4DC
Input* g_tourDeskEnterInput = NULL;

// GLOBAL: XW 0x4FB4E0
Actor* g_tourDeskOverlayActor = NULL;

// GLOBAL: XW 0x4FB4E4
int16_t g_tourDeskSelection = 0;

// GLOBAL: XW 0x4FB4E8
int16_t g_tourDeskTourCount = 0;

// GLOBAL: XW 0x4FB4EC
int16_t g_tourDeskFocusIndex = 0;

// FUNCTION: XW 0x461770
void tourdesk_OpenMusic(void* unusedResource, Film* film) {
	(void)unusedResource;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_tourDeskMusicFilm = film;
		g_tourDeskMusic = xsound_Find_Gmid("halmarch");
		if (g_tourDeskMusic != NULL)
			soundext_FadeVolume(g_tourDeskMusic, XW_TOURDESK_MUSIC_OPEN_VOLUME,
								XW_TOURDESK_MUSIC_OPEN_DURATION);
		g_tourDeskMusic = xsound_Find_Gmid("tourdesk");
		if (g_tourDeskMusic == NULL) {
			ResFile* musicResource = xres_Open_Resource(":X-Wing Data\\RESOURCE\\tdmusic.lfd");
			if (musicResource == NULL)
				musicResource = xres_Open_Resource("tdmusic.lfd");
			g_tourDeskMusic = xsound_Res_Music(musicResource, "tourdesk");
			soundext_Start_Resource_Sound(g_tourDeskMusic);
			xres_Close_Resource(musicResource);
		}
		soundext_FadeVolume(g_tourDeskMusic, XW_TOURDESK_MUSIC_OPEN_VOLUME, XW_TOURDESK_MUSIC_OPEN_DURATION);
		xsound_Set_Sound_Keep(g_tourDeskMusic);
		g_tourDeskPreviousMusic = xsound_Find_Gmid("halmarch");
		if (g_tourDeskPreviousMusic != NULL &&
			soundext_Count_Resource_Instances(g_tourDeskPreviousMusic) == 1)
			xsound_Set_Sound_Keep(g_tourDeskPreviousMusic);
	}
}

// FUNCTION: XW 0x461870
void tourdesk_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0) {
		soundext_ClearTriggers();
		if (g_tourDeskPreviousMusic == NULL || soundext_Count_Resource_Instances(g_tourDeskMusic) == 1) {
			/* Resource pointers are outside the numeric flight-sound ID range. */
			soundext_SetPriority(0, XW_TOURDESK_MUSIC_CLOSE_PRIORITY);
			soundext_FadeVolume(g_tourDeskMusic, 0, XW_TOURDESK_MUSIC_FADE_DURATION);
			if (g_tourDeskPreviousMusic != NULL) {
				Sound* previousMusic;
				xsound_Clear_Sound_Keep(g_tourDeskPreviousMusic);
				previousMusic = g_tourDeskPreviousMusic;
				xsound_Free_Sound(previousMusic);
			}
		} else {
			soundext_ClearTriggers();
		}
	}
}

// FUNCTION: XW 0x4618F0
void tourdesk_LoadSounds(void) {
	if (ShellPreferences_GetSfxEnabled()) {
		soundext_LoadSfx(XW_SHELL_SFX_DOOR_1A, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_TOUR_DOOR_CLOSE_1, 0, NULL, 0, 1);
	}
	if (ShellPreferences_GetSfxEnabled()) {
		g_tourDeskSpeech[0] = soundext_LoadSpeech(XW_SHELL_SPEECH_DUTY_REGISTRATION, 0, NULL, 0);
		g_tourDeskSpeech[1] = soundext_LoadSpeech(XW_SHELL_SPEECH_GOOD_LUCK, 1, NULL, 1);
	}
}

// FUNCTION: XW 0x461960
void tourdesk_PlaySpeech(int16_t speechIndex) {
	if (ShellPreferences_GetSfxEnabled() != 0 &&
		soundext_Count_Resource_Instances(g_tourDeskSpeech[speechIndex]) == 0) {
		if (speechIndex == XW_TOURDESK_SPEECH_JOIN &&
			soundext_Count_Resource_Instances(g_tourDeskSpeech[XW_TOURDESK_SPEECH_WELCOME]) == 1) {
			soundext_Stop_Resource_Sound(g_tourDeskSpeech[XW_TOURDESK_SPEECH_WELCOME]);
		}
		soundext_Start_Resource_SFX(g_tourDeskSpeech[speechIndex]);
		if (speechIndex == XW_TOURDESK_SPEECH_JOIN) {
			if ((uint16_t)xcursor_Is_Cursor_Visible() != 0) {
				xcursor_Hide_Cursor();
			}
#ifdef XW_MODERN
			XwTourDesk_WaitForSpeech();
#else
			while (soundext_Count_Resource_Instances(g_tourDeskSpeech[XW_TOURDESK_SPEECH_JOIN]) == 1) {
			}
#endif
		}
	}
}

// FUNCTION: XW 0x461A10
void tourdesk_PlayDoorSound(int16_t action) {
	if (ShellPreferences_GetSfxEnabled()) {
		switch (action) {
			case XW_TOURDESK_DOOR_OPEN:
				soundext_Play_SFX(XW_SHELL_SFX_DOOR_1A);
				break;
			case XW_TOURDESK_DOOR_CLOSE:
				soundext_Play_SFX(XW_SHELL_SFX_TOUR_DOOR_CLOSE_1);
				break;
		}
	}
}

// FUNCTION: XW 0x464160
XwShellSceneResult tourdesk_TourDesk(struct XwShellContext* shellContext) {
	ResFile* extraTextResource;
	ResFile* deskResource;
	PushButton* selectionButton;
	Input* textInput;
	Rect rect;
	g_tourDeskFocusIndex = XW_TOURDESK_INITIAL_FOCUS;
	xio_Set_Mouse_Position(XW_TOURDESK_INITIAL_MOUSE_X, XW_TOURDESK_INITIAL_MOUSE_Y);
	tourdesk_LoadPilotRecord();
	tourdesk_BuildReplayList();
	if (shipext_IsTourAvailable(XW_TOURDESK_EXTRA_TOUR_FIRST) == 0 &&
		shipext_IsTourAvailable(XW_TOURDESK_EXTRA_TOUR_SECOND) == 0 &&
		shipext_IsTourAvailable(XW_TOURDESK_EXTRA_TOUR_THIRD) == 0) {
		g_tourDeskText = xparagrp_Res_Paragraph(shellContext->resourceFile, "tours");
		extraTextResource = NULL;
	} else {
#ifdef XW_MODERN
		extraTextResource = XwLandru_OpenMissionResource(":X-Wing Data\\Resource\\missions.lfd");
#else
		extraTextResource = xres_Open_Resource(":X-Wing Data\\Resource\\missions.lfd");
#endif
		if (extraTextResource == NULL)
#ifdef XW_MODERN
			extraTextResource = XwLandru_OpenMissionResource("missions.lfd");
#else
			extraTextResource = xres_Open_Resource("missions.lfd");
#endif
		g_tourDeskText = xparagrp_Res_Paragraph(extraTextResource, "tours");
	}
	g_tourDeskTourCount = xparagrp_Count_Paragraph_Strings(g_tourDeskText, XW_TOURDESK_TOUR_NAMES_PARAGRAPH);
	g_tourDeskSelection = 0;
	while (g_tourDeskSelection < g_tourDeskTourCount) {
		if (shipext_IsTourAvailable(g_tourDeskSelection) != 0 &&
			g_tourDeskPilotRecord.tour_status[g_tourDeskSelection] != SHIPEXT_TOUR_STATUS_UNSELECTABLE)
			break;
		if (g_tourDeskSelection >= g_tourDeskTourCount + g_tourDeskReplayCount - 1) {
			g_tourDeskSelection = 0;
			break;
		}
		++g_tourDeskSelection;
	}
	deskResource = xres_Open_Resource(":X-Wing Data\\RESOURCE\\toddesk.lfd");
	if (deskResource == NULL)
		deskResource = xres_Open_Resource("toddesk.lfd");
	xrect_Set_Rect(&rect, 0, 0, XW_TOURDESK_WIDTH, XW_TOURDESK_HEIGHT);
	g_tourDeskFilm = xfilm_Res_Film(deskResource, "toddesk", &rect, 0, 0, 0);
	xfilm_Set_Film_Def_Palette(g_tourDeskFilm, shellContext->standardPalette);
	g_tourDeskRobotActor = xactor_Find_Actor(FOURCC_ANIM, "rbot17");
	xactor_Set_Actor_User_Function(g_tourDeskRobotActor, tourdesk_user_Robot);
	g_tourDeskDeskActor = xactor_Find_Actor(FOURCC_DELT, "desk_01");
	xactor_Non_Refreshable_Actor(g_tourDeskDeskActor);
	g_tourDeskBackgroundActor = xactor_Find_Actor(FOURCC_DELT, "toddskbk");
	xactor_Non_Refreshable_Actor(g_tourDeskBackgroundActor);
	g_tourDeskOverlayActor = xactor_Find_Actor(FOURCC_DELT, "overlay");
	xactor_Non_Refreshable_Actor(g_tourDeskOverlayActor);
	g_tourDeskStarsActor = xactor_Find_Actor(FOURCC_DELT, "starstod");
	g_tourDeskStarsActor->var1 = 0;
	xactor_Non_Refreshable_Actor(g_tourDeskStarsActor);
	g_tourDeskDoorActor = xactor_Find_Actor(FOURCC_ANIM, "door-01");
	xactor_Set_Actor_User_Function(g_tourDeskDoorActor, tourdesk_user_Door);
	xactor_Non_Refreshable_Actor(g_tourDeskDoorActor);
	g_tourDeskRootInput = xinput_Alloc_Input(NULL, &rect, 0, 0);
	xinpattr_Refresh_Input(g_tourDeskRootInput);
	xrect_Set_Rect(&rect, 0, 0, XW_TOURDESK_EXIT_RIGHT, XW_TOURDESK_HEIGHT);
	g_tourDeskExitInput = xinput_Alloc_Input(g_tourDeskRootInput, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(g_tourDeskExitInput, tourdesk_iupdate_TourDesk);
	xinpattr_Set_Input_User_Function(g_tourDeskExitInput, tourdesk_iuser_TourDesk);
	g_tourDeskExitInput->mouseUsage = allInput;
	g_tourDeskExitInput->id = XW_TOURDESK_LEAVE_INPUT;
	xrect_Set_Rect(&rect, XW_TOURDESK_ENTER_LEFT, 0, XW_TOURDESK_WIDTH, XW_TOURDESK_HEIGHT);
	g_tourDeskEnterInput = xinput_Alloc_Input(g_tourDeskRootInput, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(g_tourDeskEnterInput, tourdesk_iupdate_TourDesk);
	g_tourDeskEnterInput->mouseUsage = allInput;
	g_tourDeskEnterInput->id = XW_TOURDESK_ENTER_INPUT;
	xrect_Set_Rect(&rect, XW_TOURDESK_HINT_LEFT, XW_TOURDESK_HINT_TOP, XW_TOURDESK_HINT_RIGHT,
				   XW_TOURDESK_HINT_BOTTOM);
	g_tourDeskHintInput = xinput_Alloc_Input(g_tourDeskRootInput, &rect, 0, 0);
	xinpattr_Set_Input_Draw_Function(g_tourDeskHintInput, tourdesk_idraw_ActionHint);
	xrect_Set_Rect(&rect, XW_TOURDESK_HINT_LEFT, XW_TOURDESK_HINT_TOP, XW_TOURDESK_HINT_RIGHT,
				   XW_TOURDESK_HINT_BOTTOM);
	g_tourDeskEmptyHintInput = xinput_Alloc_Input(g_tourDeskRootInput, &rect, 0, 0);
	xrect_Set_Rect(&rect, XW_TOURDESK_PREVIOUS_LEFT, XW_TOURDESK_BUTTON_TOP, XW_TOURDESK_PREVIOUS_RIGHT,
				   XW_TOURDESK_BUTTON_BOTTOM);
	selectionButton = xbtnpush_Alloc_Button(g_tourDeskRootInput, &rect, 0, tourdesk_iuser_SelectionButton,
											NULL, XW_TOURDESK_PREVIOUS_BUTTON);
	xinpattr_Set_Input_Draw_Function(&selectionButton->header, XwTourDesk_DrawSelectionButton);
	g_tourDeskPreviousButtonActor = xactor_Find_Actor(FOURCC_ANIM, "td_but_l");
	xrect_Set_Rect(&rect, XW_TOURDESK_NEXT_LEFT, XW_TOURDESK_BUTTON_TOP, XW_TOURDESK_NEXT_RIGHT,
				   XW_TOURDESK_BUTTON_BOTTOM);
	selectionButton = xbtnpush_Alloc_Button(g_tourDeskRootInput, &rect, 0, tourdesk_iuser_SelectionButton,
											NULL, XW_TOURDESK_NEXT_BUTTON);
	xinpattr_Set_Input_Draw_Function(&selectionButton->header, XwTourDesk_DrawSelectionButton);
	g_tourDeskNextButtonActor = xactor_Find_Actor(FOURCC_ANIM, "td_but_r");
	xrect_Set_Rect(&rect, XW_TOURDESK_TITLE_LEFT, XW_TOURDESK_TITLE_TOP, XW_TOURDESK_TITLE_RIGHT,
				   XW_TOURDESK_TITLE_BOTTOM);
	textInput = xinput_Alloc_Input(g_tourDeskRootInput, &rect, 0, 0);
	xinpattr_Set_Input_Draw_Function(textInput, tourdesk_idraw_TourText);
	textInput->id = XW_TOURDESK_TITLE_INPUT;
	xrect_Set_Rect(&rect, XW_TOURDESK_DESCRIPTION_LEFT, XW_TOURDESK_DESCRIPTION_TOP,
				   XW_TOURDESK_DESCRIPTION_RIGHT, XW_TOURDESK_DESCRIPTION_BOTTOM);
	textInput = xinput_Alloc_Input(g_tourDeskRootInput, &rect, 0, 0);
	xinpattr_Set_Input_Draw_Function(textInput, tourdesk_idraw_TourText);
	textInput->id = XW_TOURDESK_DESCRIPTION_INPUT;
	xio_Set_Key_Buttons();
	tourdesk_OpenMusic(deskResource, g_tourDeskFilm);
	tourdesk_LoadSounds();
	FrontendAudio_PlayFile("XwingCD\\music\\mainmenu.wav", 1);
	xview_Set_View_Update_Function(tourdesk_end_View);
	xview_Disable_All_View_Erase();
#ifdef XW_MODERN
	XwTourDesk_RunView(deskResource, extraTextResource);
#else
	j_xviewadd_Handle_View();
	tourdesk_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xview_Enable_All_View_Erase();
	xio_Clear_Key_Buttons();
	xview_Clear_View_Update_Function();
	xparagrp_Free_Paragraph(g_tourDeskText);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	xres_Close_Resource(deskResource);
	if (extraTextResource != NULL)
		xres_Close_Resource(extraTextResource);
	LandruDisplay_ForwardLegacyNoOp(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x464740
void tourdesk_end_View(int time) {
	int16_t key;
	if (time == 0 && (uint16_t)xcursor_Is_Cursor_Visible() == 0)
		xcursor_Show_Cursor();
	if (time == XW_TOURDESK_WELCOME_TIME)
		tourdesk_PlaySpeech(XW_TOURDESK_SPEECH_WELCOME);
	key = xio_Get_Free_Key();
	if (key != 0 && shellext_MoveGridFocus(&g_tourDeskFocusIndex, g_tourDeskFocusX, g_tourDeskFocusY,
										   XW_TOURDESK_FOCUS_ROWS, XW_TOURDESK_FOCUS_COLUMNS, key) != 0) {
		xio_Set_Mouse_Position(g_tourDeskFocusX[g_tourDeskFocusIndex],
							   g_tourDeskFocusY[g_tourDeskFocusIndex]);
		xio_Get_Key();
	}
}

// FUNCTION: XW 0x4647C0
void tourdesk_user_Door(Actor* actor, int time) {
	(void)time;
	if (actor->var1 != 0) {
		if (actor->state == XW_TOURDESK_DOOR_CLOSED_FRAME)
			tourdesk_PlayDoorSound(XW_TOURDESK_DOOR_OPEN);
		if (actor->state < actor->arraySize - 1)
			xactor_Set_Actor_State(actor, actor->state + 1, 0);
		g_tourDeskStarsActor->x += g_tourDeskStarsActor->xv;
		++g_tourDeskStarsActor->var1;
		xactor_Refresh_Actor(actor);
		xactor_Refresh_Actor(g_tourDeskStarsActor);
		xactor_Refresh_Actor(g_tourDeskBackgroundActor);
		xactor_Refresh_Actor(g_tourDeskDeskActor);
		xinpattr_Refresh_Input(g_tourDeskHintInput);
		actor->var1 = 0;
	} else {
		if (actor->state == XW_TOURDESK_DOOR_CLOSE_SOUND_FRAME)
			tourdesk_PlayDoorSound(XW_TOURDESK_DOOR_CLOSE);
		if (actor->state > XW_TOURDESK_DOOR_CLOSED_FRAME) {
			xactor_Set_Actor_State(actor, actor->state - 1, 0);
			g_tourDeskStarsActor->x += g_tourDeskStarsActor->xv;
			++g_tourDeskStarsActor->var1;
			xactor_Refresh_Actor(actor);
			xactor_Refresh_Actor(g_tourDeskStarsActor);
			xactor_Refresh_Actor(g_tourDeskBackgroundActor);
			xactor_Refresh_Actor(g_tourDeskDeskActor);
			xinpattr_Refresh_Input(g_tourDeskRootInput);
		} else {
			g_tourDeskStarsActor->x -= g_tourDeskStarsActor->var1 * g_tourDeskStarsActor->xv;
			g_tourDeskStarsActor->var1 = 0;
		}
	}
}

// FUNCTION: XW 0x464910
int16_t tourdesk_iupdate_TourDesk(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
								  int rightEvent, int16_t x, int16_t y) {
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (key != 0) {
		return 0;
	}
	if (leftEvent != XW_TOURDESK_MOUSE_SELECT && rightEvent != XW_TOURDESK_MOUSE_SELECT) {
		g_tourDeskExitInput->var1 = input->id + XW_TOURDESK_HOVER_REQUEST_BASE;
	} else {
		g_tourDeskExitInput->var1 = XW_TOURDESK_EXIT_REQUEST;
		if (input->id != XW_TOURDESK_LEAVE_INPUT) {
			int16_t selection = g_tourDeskSelection;
			if (selection < g_tourDeskTourCount) {
				g_tourDeskExitInput->var2 = XW_SCENE_TOUR_TITLE_CRAWL;
			} else {
				g_tourDeskExitInput->var2 =
					g_tourReplayScenes[g_tourDeskReplayCutsceneIndices[selection - g_tourDeskTourCount]];
				shipext_Set_Pending_Medal(XW_SCENE_TOUR_DESK, 0);
				shipext_Set_Post_Award_Scene(XW_SCENE_TOUR_DESK);
			}
		} else {
			g_tourDeskExitInput->var2 = XW_SCENE_CONCOURSE;
		}
	}
	if (input->id != XW_TOURDESK_LEAVE_INPUT) {
		g_tourDeskDoorActor->var1 = 1;
	}
	return 1;
}

// FUNCTION: XW 0x4649E0
void tourdesk_iuser_TourDesk(Input* input, int context) {
	(void)context;
	switch (input->var1) {
		case XW_TOURDESK_IDLE:
			if ((uint16_t)xinpattr_Is_Input_Visible(g_tourDeskHintInput) != 0) {
				xinpattr_Show_Input(g_tourDeskEmptyHintInput);
				xinpattr_Hide_Input(g_tourDeskHintInput);
				xinpattr_Refresh_Input(g_tourDeskEmptyHintInput);
				xinpattr_Refresh_Input(g_tourDeskRootInput);
				xactor_Refresh_Actor(g_tourDeskOverlayActor);
			}
			break;
		case XW_TOURDESK_EXIT_REQUEST:
			xerror_Set_Landru_Exit(input->var2);
			if (input->var2 == XW_SCENE_TOUR_TITLE_CRAWL) {
				tourdesk_CommitTourSelection();
				tourdesk_WritePilotRecord();
				tourdesk_PlaySpeech(XW_TOURDESK_SPEECH_JOIN);
			}
			break;
		case XW_TOURDESK_HOVER_REQUEST_BASE:
		case XW_TOURDESK_HOVER_ENTER:
			if ((uint16_t)xinpattr_Is_Input_Visible(g_tourDeskEmptyHintInput) != 0) {
				xinpattr_Hide_Input(g_tourDeskEmptyHintInput);
				xinpattr_Show_Input(g_tourDeskHintInput);
				xinpattr_Refresh_Input(g_tourDeskHintInput);
				xactor_Refresh_Actor(g_tourDeskOverlayActor);
			}
			g_tourDeskHintInput->var1 = input->var1 - XW_TOURDESK_HOVER_REQUEST_BASE;
			input->var1 = XW_TOURDESK_IDLE;
			break;
	}
}

// FUNCTION: XW 0x464B00
void tourdesk_idraw_ActionHint(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	char text[XW_TOURDESK_HINT_CAPACITY];
	(void)clip;
	if (refresh != 0) {
		const char* hintText;
		xpaint_Paint_Clipped_Rect(frame, XW_TOURDESK_HINT_BACKGROUND);
		if (input->var1 != 0) {
			if (g_tourDeskSelection < g_tourDeskTourCount) {
				hintText = "Enter Tour of Duty";
			} else {
				hintText = "View Cutscene";
			}
		} else {
			hintText = "Exit Tour Desk";
		}
		strcpy(text, hintText);
		xrect_Offset_Rect(frame, XW_TOURDESK_HINT_SHADOW_OFFSET, XW_TOURDESK_HINT_SHADOW_OFFSET);
		xfont_Print_Centered_Text(text, frame, XW_TOURDESK_HINT_FONT, XW_TOURDESK_HINT_SHADOW);
		xrect_Offset_Rect(frame, -XW_TOURDESK_HINT_SHADOW_OFFSET, -XW_TOURDESK_HINT_SHADOW_OFFSET);
		xfont_Print_Centered_Text(text, frame, XW_TOURDESK_HINT_FONT, XW_TOURDESK_HINT_COLOR);
	}
}

// FUNCTION: XW 0x464BC0
void tourdesk_iuser_SelectionButton(Input* input, int context) {
	(void)context;
	if (xinpattr_Get_Input_Selected(input) != 0) {
		switch (input->id) {
			case XW_TOURDESK_PREVIOUS_BUTTON:
				if (g_tourDeskSelection > 0)
					--g_tourDeskSelection;
				else
					g_tourDeskSelection = g_tourDeskTourCount + g_tourDeskReplayCount - 1;
				while (g_tourDeskSelection < g_tourDeskTourCount) {
					if (shipext_IsTourAvailable(g_tourDeskSelection) != 0 &&
						g_tourDeskPilotRecord.tour_status[g_tourDeskSelection] !=
							SHIPEXT_TOUR_STATUS_UNSELECTABLE)
						break;
					if (g_tourDeskSelection > 0)
						--g_tourDeskSelection;
					else
						g_tourDeskSelection = g_tourDeskTourCount + g_tourDeskReplayCount - 1;
				}
				break;
			case XW_TOURDESK_NEXT_BUTTON:
				if (g_tourDeskSelection == g_tourDeskTourCount + g_tourDeskReplayCount - 1)
					g_tourDeskSelection = 0;
				else
					++g_tourDeskSelection;
				while (g_tourDeskSelection < g_tourDeskTourCount) {
					if (shipext_IsTourAvailable(g_tourDeskSelection) != 0 &&
						g_tourDeskPilotRecord.tour_status[g_tourDeskSelection] !=
							SHIPEXT_TOUR_STATUS_UNSELECTABLE)
						break;
					if (g_tourDeskSelection == g_tourDeskTourCount + g_tourDeskReplayCount - 1)
						g_tourDeskSelection = 0;
					else
						++g_tourDeskSelection;
				}
				break;
		}
		xinpattr_Refresh_Input(g_tourDeskRootInput);
	}
}

// FUNCTION: XW 0x464D10
void tourdesk_idraw_SelectionButton(PushButton* button, Rect* frame, Rect* clip, int16_t refresh) {
	if (refresh != 0) {
		Actor* buttonActor;
		switch (button->header.id) {
			case XW_TOURDESK_PREVIOUS_BUTTON:
				buttonActor = g_tourDeskPreviousButtonActor;
				break;
			case XW_TOURDESK_NEXT_BUTTON:
				buttonActor = g_tourDeskNextButtonActor;
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

// FUNCTION: XW 0x464D70
void tourdesk_idraw_TourText(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	Rect lineRect;
	char operationNumber[XW_TOURDESK_OPERATION_NUMBER_CAPACITY];
	char text[XW_TOURDESK_TEXT_CAPACITY];
	(void)clip;
	if (refresh != 0) {
		int16_t titleColor;
		int16_t descriptionColor;
		if (input->id == XW_TOURDESK_TITLE_INPUT)
			xpaint_Paint_Clipped_Rect(frame, XW_TOURDESK_TITLE_BACKGROUND);
		else
			xpaint_Paint_Clipped_Rect(frame, XW_TOURDESK_DESCRIPTION_BACKGROUND);
		if (g_tourDeskSelection < g_tourDeskTourCount) {
			titleColor = XW_TOURDESK_TOUR_TITLE_COLOR;
			descriptionColor = XW_TOURDESK_TOUR_DESCRIPTION_COLOR;
		} else {
			titleColor = XW_TOURDESK_REPLAY_TITLE_COLOR;
			descriptionColor = XW_TOURDESK_REPLAY_DESCRIPTION_COLOR;
		}
		if (input->id == XW_TOURDESK_TITLE_INPUT) {
			if (g_tourDeskSelection < g_tourDeskTourCount) {
				xparagrp_Get_Paragraph_String(g_tourDeskText, text, XW_TOURDESK_TOUR_NAMES_PARAGRAPH,
											  g_tourDeskSelection);
				if (g_tourDeskPilotRecord.tourOperationProgress[g_tourDeskSelection] != 0) {
					strcat(text, " Operation ");
					sprintf(operationNumber, "%d",
							g_tourDeskPilotRecord.tourOperationProgress[g_tourDeskSelection] + 1);
					strcat(text, operationNumber);
				}
			} else {
				xparagrp_Get_Paragraph_String(
					g_tourDeskText, text, XW_TOURDESK_REPLAY_NAMES_PARAGRAPH,
					g_tourDeskReplayCutsceneIndices[g_tourDeskSelection - g_tourDeskTourCount]);
			}
			xfont_Print_Centered_Text(text, frame, XW_TOURDESK_TITLE_FONT, titleColor);
		} else {
			int16_t lineCount = xparagrp_Count_Paragraph_Strings(
				g_tourDeskText, g_tourDeskSelection + XW_TOURDESK_DESCRIPTION_PARAGRAPH_BASE);
			int16_t paragraphIndex;
			int16_t lineIndex;
			xrect_Copy_Rect(&lineRect, frame);
			lineRect.top += XW_TOURDESK_DESCRIPTION_TOP_OFFSET;
			paragraphIndex = g_tourDeskSelection;
			lineRect.bottom = lineRect.top + XW_TOURDESK_DESCRIPTION_LINE_HEIGHT;
			if (g_tourDeskSelection >= g_tourDeskTourCount)
				paragraphIndex = g_tourDeskTourCount +
								 g_tourDeskReplayCutsceneIndices[g_tourDeskSelection - g_tourDeskTourCount];
			paragraphIndex += XW_TOURDESK_DESCRIPTION_PARAGRAPH_BASE;
			for (lineIndex = 0; lineIndex < lineCount; ++lineIndex) {
				xparagrp_Get_Paragraph_String(g_tourDeskText, text, paragraphIndex, lineIndex);
				xfont_Print_Centered_Text(text, &lineRect, XW_TOURDESK_DESCRIPTION_FONT, descriptionColor);
				xrect_Offset_Rect(&lineRect, XW_TOURDESK_DESCRIPTION_LINE_INDENT,
								  XW_TOURDESK_DESCRIPTION_LINE_HEIGHT);
			}
		}
	}
}

// FUNCTION: XW 0x464F80
void tourdesk_user_Robot(Actor* actor, int time) {
	int16_t state = actor->state;
	(void)time;
	if (state < actor->arraySize - 1) {
		xactor_Set_Actor_State(actor, state + 1, 0);
	} else {
		xactor_Set_Actor_State(actor, 0, 0);
	}
	xactor_Refresh_Actor(actor);
}

// FUNCTION: XW 0x464FD0
int16_t tourdesk_LoadPilotRecord(void) {
	char pilotPath[XW_TOURDESK_PILOT_FILENAME_CAPACITY];
	LandruFile* stream;
	strcpy(pilotPath, g_RegisterShellPilot.name);
	strcat(pilotPath, ".PLT");
	stream = xfile_Open_File(LANDRU_FILE_ROOT_USER, pilotPath, "rb");
	if (stream != NULL) {
		register_ReadPilotRecord(stream, &g_tourDeskPilotRecord);
		xfile_Close_File(stream);
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x465080
void tourdesk_WritePilotRecord(void) {
	char pilotPath[XW_TOURDESK_PILOT_PATH_CAPACITY];
	LandruFile* stream;
	strcpy(pilotPath, g_RegisterShellPilot.name);
	strcat(pilotPath, ".PLT");
	stream = xfile_Open_File(LANDRU_FILE_ROOT_USER, pilotPath, "wb");
	if (stream != NULL) {
		xfile_Write_Data_To_File(stream, &g_tourDeskPilotRecord, sizeof(g_tourDeskPilotRecord));
		xfile_Close_File(stream);
	}
	g_RegisterShellPilot.current_tour = g_tourDeskPilotRecord.current_tour;
}

// FUNCTION: XW 0x465130
void tourdesk_BuildReplayList(void) {
	int16_t tourIndex;
	g_tourDeskReplayCount = 0;
	for (tourIndex = 0; tourIndex < SHIPEXT_TOUR_COUNT; ++tourIndex) {
		if (shipext_IsTourAvailable(tourIndex) != 0) {
			int16_t operationCount = g_tourDeskPilotRecord.tourOperationProgress[tourIndex];
			if (operationCount != 0) {
				XwTourOperation* operations = g_tourOperationTables[tourIndex];
				int16_t operationIndex;
				for (operationIndex = 0; operationIndex < operationCount; ++operationIndex) {
					if (operations[operationIndex].cutsceneIndex != SHIPEXT_TOUR_NO_ENTRY) {
						g_tourDeskReplayCutsceneIndices[g_tourDeskReplayCount++] =
							operations[operationIndex].cutsceneIndex;
					}
				}
			}
		}
	}
}

// FUNCTION: XW 0x4651C0
void tourdesk_CommitTourSelection(void) {
	XwTourOperation* operations;
	g_tourDeskPilotRecord.field_281 = 0;
	g_tourDeskPilotRecord.current_tour = (uint8_t)g_tourDeskSelection;
	operations = g_tourOperationTables[g_tourDeskSelection];
	g_tourDeskPilotRecord.currentTourOperation =
		g_tourDeskPilotRecord.tourOperationProgress[g_tourDeskSelection];
	g_tourDeskPilotRecord.briefingMissionChoices[0] =
		operations[g_tourDeskPilotRecord.currentTourOperation].missionChoiceA;
	g_tourDeskPilotRecord.briefingMissionChoices[1] =
		operations[g_tourDeskPilotRecord.currentTourOperation].missionChoiceB;
	g_tourDeskPilotRecord.tour_status[g_tourDeskSelection] = SHIPEXT_TOUR_STATUS_ACTIVE;
}
