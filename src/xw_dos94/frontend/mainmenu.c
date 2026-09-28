#include "xw/frontend/mainmenu.h"
#include "xw/frontend/register.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"
#include "xw_dos94/frontend/scenes.h"
#include "xw_runtime/runtime/mainmenu_task.h"
#include <landru/actanim.h>
#include <landru/actdelt.h>
#include <landru/cursor.h>
#include <landru/font.h>
#include <landru/fourcc.h>
#include <landru/inpattr.h>
#include <landru/io.h>
#include <landru/view.h>
#include <landru/viewadd.h>
#include <stdlib.h>
#include <string.h>

static Actor* door_overlay;
static Actor* tour_desk;

/* DOS94 0x5015ca. */
static void update_door(Actor* actor, int32_t time) {
	(void)time;
	mainmenu_UpdateDoorMusic(actor);
	if (actor->var1) {
		if (!actor->state)
			mainmenu_PlayDoorSoundCue(1);
		if (actor->state < actor->arraySize - 1) {
			xactor_Set_Actor_State(actor, actor->state + 1, 0);
			xactor_Refresh_Actor(door_overlay);
		}
		actor->var1 = 0;
	} else {
		if (actor->state == 1)
			mainmenu_PlayDoorSoundCue(2);
		if (actor->state > 0) {
			xactor_Set_Actor_State(actor, actor->state - 1, 0);
			xactor_Refresh_Actor(door_overlay);
		}
	}
}

/* DOS94 0x5011f8. */
static void update_tour_desk(Actor* actor, int32_t time) {
	mainmenu_UpdateDoorMusic(actor);
	if (!time)
		actor->var2 = 0;
	if (!(time & 3))
		actor->var1 = rand() & 3;
}

/* DOS94 0x501236. */
static int16_t draw_tour_desk(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	if (!refresh)
		return 0;
	xactor_Set_Actor_State(actor, actor->var1, 0);
	xactanim_Draw_Anim_Actor(actor, frame, clip, x, y, refresh);
	xactor_Set_Actor_State(actor, 5 - actor->var2, 0);
	int16_t result = xactanim_Draw_Anim_Actor(actor, frame, clip, x, y, refresh);
	actor->var2 = 0;
	return result;
}

/* DOS94 0x5012d4. */
static void update_body_parts(Actor* actor, int32_t time) {
	(void)time;
	actor->var1 = rand() % actor->arraySize;
	actor->var2 = rand() % actor->arraySize;
}

/* DOS94 0x501306. */
static int16_t draw_body_parts(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	if (!refresh)
		return 0;
	xactor_Set_Actor_State(actor, 0, 0);
	xactanim_Draw_Anim_Actor(actor, frame, clip, x, y, refresh);
	xactor_Set_Actor_State(actor, actor->var1, 0);
	xactanim_Draw_Anim_Actor(actor, frame, clip, x, y, refresh);
	xactor_Set_Actor_State(actor, actor->var2, 0);
	return xactanim_Draw_Anim_Actor(actor, frame, clip, x, y, refresh);
}

/* DOS94 0x5010b4 adds the tour-desk hover pulse to the shared navigation. */
static int16_t update_input(Input* input, Rect* frame, Rect* clip, int16_t key, int left, int right,
							int16_t x, int16_t y) {
	if (!key && input->id == MAINMENU_INPUT_TOUR_DESK)
		tour_desk->var2 = 1;
	return mainmenu_iupdate_MainMenu(input, frame, clip, key, left, right, x, y);
}

/* DOS94 0x501030; the keyboard grid also survives unchanged in Windows. */
static void end_view(int32_t time) {
	if (!time && !xcursor_Is_Cursor_Visible())
		xcursor_Show_Cursor();
	int16_t key = xio_Get_Free_Key();
	if (key && shellext_MoveGridFocus(&g_MainMenuFocus, g_MainMenuMouseX, g_MainMenuMouseY, 2, 4, key)) {
		xio_Set_Mouse_Position(g_MainMenuMouseX[g_MainMenuFocus], g_MainMenuMouseY[g_MainMenuFocus]);
		xio_Get_Free_Key();
	}
}

/* DOS94 0x501424. */
static int16_t draw_title(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	Rect labelRect;
	char text[MAINMENU_TITLE_CAPACITY];
	int16_t offsetX;
	int16_t offsetY;
	if (refresh == 0)
		return 0;
	xactdelt_Draw_Delta_Actor(actor, frame, clip, x, y, refresh);
	xactdelt_Get_Delta_Actor_Offset(actor, &offsetX, &offsetY);
	xrect_Set_Rect(&labelRect, offsetX, offsetY, offsetX + actor->w, offsetY + actor->h);
	switch (actor->var2) {
		case MAINMENU_INPUT_TRAINING:
			strcpy(text, g_MainMenuTrainingTitle);
			break;
		case MAINMENU_INPUT_COMBAT:
			strcpy(text, g_MainMenuCombatTitle);
			break;
		case MAINMENU_INPUT_TOUR:
			strcpy(text, g_MainMenuContinueTourTitle);
			strcat(text, g_MainMenuTourSuffixes[g_MainMenuPilotData.current_tour]);
			break;
		case MAINMENU_INPUT_TECH_ROOM:
			strcpy(text, g_MainMenuTechRoomTitle);
			break;
		case MAINMENU_INPUT_FILM_ROOM:
			strcpy(text, g_MainMenuFilmRoomTitle);
			break;
		case MAINMENU_INPUT_REGISTER:
			strcpy(text, g_MainMenuRegisterTitle);
			break;
		case MAINMENU_INPUT_TOUR_DESK:
			if (shipext_IsTourAvailable(g_MainMenuPilotData.current_tour) &&
				g_MainMenuPilotData.tour_status[g_MainMenuPilotData.current_tour] ==
					SHIPEXT_TOUR_STATUS_ACTIVE) {
				strcpy(text, g_MainMenuChangeToursTitle);
			} else {
				int16_t newTourAvailable = 0;
				int16_t tourIndex;
				for (tourIndex = 0; tourIndex < SHIPEXT_TOUR_COUNT; ++tourIndex) {
					if (shipext_IsTourAvailable(tourIndex) != 0 &&
						g_MainMenuPilotData.tour_status[tourIndex] <= SHIPEXT_TOUR_STATUS_ACTIVE)
						newTourAvailable = 1;
				}
				if (newTourAvailable != 0)
					strcpy(text, g_MainMenuNewTourTitle);
				else
					strcpy(text, g_MainMenuCutscenesTitle);
			}
			break;
	}
	xrect_Offset_Rect(&labelRect, MAINMENU_TITLE_SHADOW_OFFSET, MAINMENU_TITLE_SHADOW_OFFSET);
	xfont_Print_Centered_Text(text, &labelRect, 0, MAINMENU_TITLE_SHADOW_COLOR);
	xrect_Offset_Rect(&labelRect, -MAINMENU_TITLE_SHADOW_OFFSET, -MAINMENU_TITLE_SHADOW_OFFSET);
	xfont_Print_Centered_Text(text, &labelRect, 0, MAINMENU_TITLE_TEXT_COLOR);
	return 1;
}

/* DOS94 0x500724. Shared pilot storage uses the port's canonical record. */
void Dos94_MainMenu(XwShellContext* shell) {
	static const char* const doors[] = { "lhdoor", "middoor", "toddoor", "lwr-lhdr", "lwr-rhdr", "pilotdr" };
	g_MainMenuFocus = 1;
	xio_Set_Mouse_Position(147, 100);
	mainmenu_LoadPilotRecord();
	ResFile* resource = xres_Open_Resource("mainmenu.lfd");
	Rect frame;
	xrect_Set_Rect(&frame, 0, 0, 320, 200);
	g_MainMenuFilm = xfilm_Res_Film(resource, "mainmenu", &frame, 0, 0, 0);
	xfilm_Set_Film_Def_Palette(g_MainMenuFilm, shell->standardPalette);
	g_MainMenuBackground = xactor_Find_Actor(FOURCC_DELT, "mainmenu");
	xactor_Non_Refreshable_Actor(g_MainMenuBackground);
	door_overlay = xactor_Find_Actor(FOURCC_DELT, "maindoor");
	xactor_Non_Refreshable_Actor(door_overlay);
	for (int i = 0; i < 6; ++i)
		g_MainMenuDoors[i] = xactor_Find_Actor(FOURCC_ANIM, doors[i]);
	for (int i = 0; i < 6; ++i) {
		xactor_Set_Actor_User_Function(g_MainMenuDoors[i], update_door);
		g_MainMenuDoors[i]->id = i;
	}
	tour_desk = xactor_Find_Actor(FOURCC_ANIM, "todmmrb");
	xactor_Set_Actor_User_Function(tour_desk, update_tour_desk);
	xactor_Set_Actor_Draw_Function(tour_desk, draw_tour_desk);
	tour_desk->id = 6;
	Actor* body_parts = xactor_Find_Actor(FOURCC_ANIM, "bodyprts");
	xactor_Set_Actor_User_Function(body_parts, update_body_parts);
	xactor_Set_Actor_Draw_Function(body_parts, draw_body_parts);
	g_MainMenuTitle = xactdelt_Res_Delta_Actor(resource, "title", &frame, 0, 0, 0);
	xactor_Set_Actor_User_Function(g_MainMenuTitle, mainmenu_user_Title);
	xactor_Set_Actor_Draw_Function(g_MainMenuTitle, draw_title);
	g_MainMenuRootInput = xinput_Alloc_Input(NULL, &frame, 0, 0);

	/* DOS94 hotspot bounds; the Register rectangle is set at 0x500f0e. */
	const bool active = shipext_IsTourAvailable(g_MainMenuPilotData.current_tour) &&
						g_MainMenuPilotData.tour_status[g_MainMenuPilotData.current_tour] == 1;
	const int ids[] = { 0, 1, active ? 2 : 6, 6, 3, 4, 5 };
	const int bounds[][4] = {
		{ 10, 22, 74, 80 },    { 102, 36, 190, 78 }, { active ? 256 : 226, 24, 294, 86 },
		{ 226, 24, 256, 86 },  { 32, 98, 110, 144 }, { 188, 98, 266, 144 },
		{ 270, 138, 320, 200 }
	};
	for (int i = 0; i < 7; ++i) {
		if (i == 3 && !active)
			continue;
		xrect_Set_Rect(&frame, bounds[i][0], bounds[i][1], bounds[i][2], bounds[i][3]);
		Input* input = xinput_Alloc_Input(g_MainMenuRootInput, &frame, 0, 0);
		xinpattr_Set_Input_Update_Function(input, update_input);
		xinpattr_Set_Input_User_Function(input, mainmenu_iuser_MainMenu);
		input->mouseUsage = allInput;
		input->id = ids[i];
		switch (input->id) {
			case 0:
				g_MainMenuProvingGroundInput = input;
				break;
			case 1:
				g_MainMenuHistoricalCombatInput = input;
				break;
			case 2:
			case 6:
				g_MainMenuTourInput = input;
				break;
			case 3:
				g_MainMenuTechInput = input;
				break;
			case 4:
				g_MainMenuFilmInput = input;
				break;
			case 5:
				g_MainMenuRegisterInput = input;
				break;
		}
	}
	xio_Set_Key_Buttons();
	mainmenu_OpenMusic(resource, g_MainMenuFilm);
	mainmenu_LoadSoundEffects(resource, g_MainMenuFilm);
	xview_Set_View_Update_Function(end_view);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	XwMainMenu_RunView(resource);
}
