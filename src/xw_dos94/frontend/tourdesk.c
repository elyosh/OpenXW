#include "xw_dos94/frontend/tourdesk.h"
#include "xw/frontend/tourdesk.h"
#include "xw_runtime/integration/landru_adapter.h"

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

#include <landru/style.h>

static Actor* door_background;

static void draw_selection_button(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	Dos94_tourdesk_idraw_SelectionButton((PushButton*)input, frame, clip, refresh);
}

/* DOS94 0x5409f6. */
XwShellSceneResult Dos94_tourdesk_TourDesk(struct XwShellContext* shellContext) {
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
		extraTextResource = XwLandru_OpenMissionResource(":X-Wing Data\\Resource\\missions.lfd");
		if (extraTextResource == NULL)
			extraTextResource = XwLandru_OpenMissionResource("missions.lfd");
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
	deskResource = xres_Open_Resource("tourdesk.lfd");
	xrect_Set_Rect(&rect, 0, 0, 320, 200);
	g_tourDeskFilm = xfilm_Res_Film(deskResource, "tod", &rect, 0, 0, 0);
	xfilm_Set_Film_Def_Palette(g_tourDeskFilm, shellContext->standardPalette);
	g_tourDeskBackgroundActor = xactor_Find_Actor(FOURCC_DELT, "ltour");
	xactor_Non_Refreshable_Actor(g_tourDeskBackgroundActor);
	g_tourDeskDoorActor = xactor_Find_Actor(FOURCC_DELT, "rtour");
	xactor_Set_Actor_User_Function(g_tourDeskDoorActor, Dos94_tourdesk_user_Door);
	xactor_Non_Refreshable_Actor(g_tourDeskDoorActor);
	door_background = xactor_Find_Actor(FOURCC_DELT, "rtourbk");
	xactor_Non_Refreshable_Actor(door_background);
	g_tourDeskOverlayActor = xactor_Find_Actor(FOURCC_DELT, "plate");
	xactor_Non_Refreshable_Actor(g_tourDeskOverlayActor);
	g_tourDeskRootInput = xinput_Alloc_Input(NULL, &rect, 0, 0);
	xinpattr_Refresh_Input(g_tourDeskRootInput);
	xrect_Set_Rect(&rect, 0, 0, 30, 200);
	g_tourDeskExitInput = xinput_Alloc_Input(g_tourDeskRootInput, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(g_tourDeskExitInput, tourdesk_iupdate_TourDesk);
	xinpattr_Set_Input_User_Function(g_tourDeskExitInput, Dos94_tourdesk_iuser_TourDesk);
	g_tourDeskExitInput->mouseUsage = allInput;
	g_tourDeskExitInput->id = XW_TOURDESK_LEAVE_INPUT;
	xrect_Set_Rect(&rect, 250, 0, 320, 200);
	g_tourDeskEnterInput = xinput_Alloc_Input(g_tourDeskRootInput, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(g_tourDeskEnterInput, tourdesk_iupdate_TourDesk);
	g_tourDeskEnterInput->mouseUsage = allInput;
	g_tourDeskEnterInput->id = XW_TOURDESK_ENTER_INPUT;
	xrect_Set_Rect(&rect, 34, 120, 216, 177);
	g_tourDeskHintInput = xinput_Alloc_Input(g_tourDeskRootInput, &rect, 0, 0);
	xinpattr_Set_Input_Draw_Function(g_tourDeskHintInput, Dos94_tourdesk_idraw_ActionHint);
	xrect_Set_Rect(&rect, 34, 120, 216, 177);
	g_tourDeskEmptyHintInput = xinput_Alloc_Input(g_tourDeskRootInput, &rect, 0, 0);
	xrect_Set_Rect(&rect, 0, 0, 16, 16);
	selectionButton =
		xbtnpush_Alloc_Button(g_tourDeskEmptyHintInput, &rect, 0, Dos94_tourdesk_iuser_SelectionButton, NULL,
							  XW_TOURDESK_PREVIOUS_BUTTON);
	xinpattr_Set_Input_Draw_Function(&selectionButton->header, draw_selection_button);
	xrect_Set_Rect(&rect, 0, 0, 16, 16);
	selectionButton =
		xbtnpush_Alloc_Button(g_tourDeskEmptyHintInput, &rect, 0, Dos94_tourdesk_iuser_SelectionButton, NULL,
							  XW_TOURDESK_NEXT_BUTTON);
	xinpattr_Set_Input_Draw_Function(&selectionButton->header, draw_selection_button);
	xinpattr_Set_Input_Allign(&selectionButton->header, 2, 0);
	xrect_Set_Rect(&rect, 18, 1, 164, 15);
	textInput = xinput_Alloc_Input(g_tourDeskEmptyHintInput, &rect, 0, 0);
	xinpattr_Set_Input_Draw_Function(textInput, Dos94_tourdesk_idraw_TourText);
	textInput->id = XW_TOURDESK_TITLE_INPUT;
	xrect_Set_Rect(&rect, 0, 20, 184, 58);
	textInput = xinput_Alloc_Input(g_tourDeskEmptyHintInput, &rect, 0, 0);
	xinpattr_Set_Input_Draw_Function(textInput, Dos94_tourdesk_idraw_TourText);
	textInput->id = XW_TOURDESK_DESCRIPTION_INPUT;
	xio_Set_Key_Buttons();
	tourdesk_OpenMusic(deskResource, g_tourDeskFilm);
	tourdesk_LoadSounds();
	xview_Set_View_Update_Function(tourdesk_end_View);
	xview_Disable_All_View_Erase();
	XwTourDesk_RunView(deskResource, extraTextResource);
}

/* DOS94 0x541156. */
void Dos94_tourdesk_user_Door(Actor* actor, int time) {
	(void)time;
	if (actor->var1 != 0) {
		if (actor->x == 0)
			tourdesk_PlayDoorSound(1);
		if (actor->x < 80) {
			actor->x += 20;
			xactor_Refresh_Actor(actor);
			xactor_Refresh_Actor(door_background);
		}
		actor->var1 = 0;
	} else {
		if (actor->x == 20)
			tourdesk_PlayDoorSound(2);
		if (actor->x > 0) {
			actor->x -= 20;
			xactor_Refresh_Actor(actor);
			xactor_Refresh_Actor(door_background);
		}
	}
}

/* DOS94 0x5412de. */
void Dos94_tourdesk_iuser_TourDesk(Input* input, int context) {
	(void)context;
	switch (input->var1) {
		case XW_TOURDESK_IDLE:
			if ((uint16_t)xinpattr_Is_Input_Visible(g_tourDeskHintInput) != 0) {
				xinpattr_Show_Input(g_tourDeskEmptyHintInput);
				xinpattr_Hide_Input(g_tourDeskHintInput);
				xinpattr_Refresh_Input(g_tourDeskEmptyHintInput);
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

/* DOS94 0x54145a. */
void Dos94_tourdesk_idraw_ActionHint(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	char text[XW_TOURDESK_HINT_CAPACITY];
	(void)clip;
	if (refresh != 0) {
		const char* hintText;
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
		xfont_Print_Centered_Text(text, frame, 0, XW_TOURDESK_HINT_SHADOW);
		xrect_Offset_Rect(frame, -XW_TOURDESK_HINT_SHADOW_OFFSET, -XW_TOURDESK_HINT_SHADOW_OFFSET);
		xfont_Print_Centered_Text(text, frame, 0, 50);
	}
}

/* DOS94 0x5414fe. */
void Dos94_tourdesk_iuser_SelectionButton(Input* input, int context) {
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
		xinpattr_Refresh_Input(g_tourDeskEmptyHintInput);
	}
}

/* DOS94 0x541686. */
void Dos94_tourdesk_idraw_SelectionButton(PushButton* button, Rect* frame, Rect* clip, int16_t refresh) {
	if (refresh != 0) {
		xstyle_Style_Paint_Border(frame, button->pressed);
		xstyle_Style_Draw_Centered_Icon(button->header.id == 0 || button->header.id == 2 ? 1 : 3, frame, clip,
										button->pressed);
	}
}

/* DOS94 0x5416f0. */
void Dos94_tourdesk_idraw_TourText(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	Rect lineRect;
	char operationNumber[XW_TOURDESK_OPERATION_NUMBER_CAPACITY];
	char text[XW_TOURDESK_TEXT_CAPACITY];
	(void)clip;
	if (refresh != 0) {
		int16_t titleColor;
		int16_t descriptionColor;
		xstyle_Style_Paint_TextField(frame);
		if (g_tourDeskSelection < g_tourDeskTourCount) {
			titleColor = 51;
			descriptionColor = 50;
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
			lineRect.bottom = lineRect.top + 10;
			if (g_tourDeskSelection >= g_tourDeskTourCount)
				paragraphIndex = g_tourDeskTourCount +
								 g_tourDeskReplayCutsceneIndices[g_tourDeskSelection - g_tourDeskTourCount];
			paragraphIndex += XW_TOURDESK_DESCRIPTION_PARAGRAPH_BASE;
			for (lineIndex = 0; lineIndex < lineCount; ++lineIndex) {
				xparagrp_Get_Paragraph_String(g_tourDeskText, text, paragraphIndex, lineIndex);
				xfont_Print_Centered_Text(text, &lineRect, 0, descriptionColor);
				xrect_Offset_Rect(&lineRect, 0, 10);
			}
		}
	}
}

/* DOS94 0x541c26. */
void Dos94_tourdesk_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0) {
		soundext_ClearTriggers();
		if (g_tourDeskPreviousMusic == NULL || soundext_Count_Resource_Instances(g_tourDeskMusic) == 1) {
			soundext_SetPriority((intptr_t)g_tourDeskMusic, XW_TOURDESK_MUSIC_CLOSE_PRIORITY);
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
