#include "xw/frontend/mainmenu.h"

#include "xw/landru_config.h"

#include "xw/audio/frontend_audio.h"
#include "xw/audio/soundext.h"
#include "xw/util/landru_display.h"
#include "xw_runtime/integration/mainmenu_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/mainmenu_task.h"
#endif
#include "xw/frontend/register.h"
#include "xw/frontend/shell.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#include <landru/actanim.h>
#include <landru/actdelt.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/file.h>
#include <landru/font.h>
#include <landru/fourcc.h>
#include <landru/inpattr.h>
#include <landru/io.h>
#include <landru/paint.h>
#include <landru/view.h>
#include <landru/viewadd.h>

#ifndef XW_MODERN
#include <mbstring.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <time.h>

// GLOBAL: XW 0x4D5C70
char g_MainMenuTrainingTitle[MAINMENU_TITLE_STRING_CAPACITY] = "Pilot Proving Ground";

// GLOBAL: XW 0x4D5C88
char g_MainMenuCombatTitle[MAINMENU_TITLE_STRING_CAPACITY] = "Historical Combat";

// GLOBAL: XW 0x4D5CA0
char g_MainMenuContinueTourTitle[MAINMENU_TITLE_STRING_CAPACITY] = "Continue Tour";

// GLOBAL: XW 0x4D5CB8
char g_MainMenuTechRoomTitle[MAINMENU_TITLE_STRING_CAPACITY] = "Tech Room";

// GLOBAL: XW 0x4D5CD0
char g_MainMenuFilmRoomTitle[MAINMENU_TITLE_STRING_CAPACITY] = "Film Room";

// GLOBAL: XW 0x4D5CE8
char g_MainMenuRegisterTitle[MAINMENU_TITLE_STRING_CAPACITY] = "Register";

// GLOBAL: XW 0x4D5D00
char g_MainMenuNewTourTitle[MAINMENU_TITLE_STRING_CAPACITY] = "New Tour";

// GLOBAL: XW 0x4D5D48
const char g_MainMenuTourSuffixes[MAINMENU_TOUR_SUFFIX_COUNT][MAINMENU_TOUR_SUFFIX_CAPACITY] = {
	" I", " II", " III", " IV", " V", " VI", " VII", " VIII"
};

// GLOBAL: XW 0x4D5E38
char g_MainMenuChangeToursTitle[MAINMENU_TITLE_STRING_CAPACITY] = "Change Tours";

// GLOBAL: XW 0x4D5E50
char g_MainMenuCutscenesTitle[MAINMENU_TITLE_STRING_CAPACITY] = "View TOD Cutscenes";

// GLOBAL: XW 0x4D5E68
int16_t g_MainMenuMouseX[MAINMENU_FOCUS_COUNT] = { 34, 145, 245, 262, 71, 71, 222, 298 };

// GLOBAL: XW 0x4D5E78
int16_t g_MainMenuMouseY[MAINMENU_FOCUS_COUNT] = { 64, 64, 76, 64, 122, 122, 122, 170 };

// GLOBAL: XW 0x4D5EE8
const char g_mainMenuChristmasDate[MAINMENU_CHRISTMAS_DATE_CAPACITY] = "12/25";

// GLOBAL: XW 0x4F80D0
Actor* g_MainMenuBackground = NULL;

// GLOBAL: XW 0x4F80D4
Input* g_MainMenuProvingGroundInput = NULL;

// GLOBAL: XW 0x4F80D8
Input* g_MainMenuRootInput = NULL;

// GLOBAL: XW 0x4F80E0
REGISTER_PilotFileRecord g_MainMenuPilotData = { 0 };

// GLOBAL: XW 0x4F8790
Film* g_MainMenuFilm = NULL;

// GLOBAL: XW 0x4F8794
Input* g_MainMenuTechInput = NULL;

// GLOBAL: XW 0x4F8798
Input* g_MainMenuHistoricalCombatInput = NULL;

// GLOBAL: XW 0x4F87A0
Actor* g_MainMenuWindActor = NULL;

// GLOBAL: XW 0x4F87A4
Actor* g_MainMenuShakeActor = NULL;

// GLOBAL: XW 0x4F87A8
Actor* g_MainMenuWaveActor = NULL;

// GLOBAL: XW 0x4F87AC
Input* g_MainMenuRegisterInput = NULL;

// GLOBAL: XW 0x4F87B0
Actor* g_MainMenuStars = NULL;

// GLOBAL: XW 0x4F87B8
Actor* g_MainMenuDoors[MAINMENU_DOOR_COUNT] = { NULL, NULL, NULL, NULL, NULL, NULL };

// GLOBAL: XW 0x4F87D4
Input* g_MainMenuFilmInput = NULL;

// GLOBAL: XW 0x4F87D8
Input* g_MainMenuTourInput = NULL;

// GLOBAL: XW 0x4F87DC
Actor* g_MainMenuTitle = NULL;

// GLOBAL: XW 0x4F87E0
int16_t g_MainMenuFocus = 0;

// GLOBAL: XW 0x4F8820
Sound* g_MainMenuMarchMusic = NULL;

// GLOBAL: XW 0x4F8824
Sound* g_MainMenuSecurityMusic = NULL;

// GLOBAL: XW 0x4F8828
int g_MainMenuLastMusicDoorId = 0;

// GLOBAL: XW 0x4F8830
Sound* g_MainMenuAmbientSpeech[MAINMENU_AMBIENT_SPEECH_COUNT] = { NULL, NULL, NULL };

// FUNCTION: XW 0x457E50
XwShellSceneResult mainmenu_Main_Menu(struct XwShellContext* shell) {
	ResFile* resource;
	int16_t doorIndex;
	int16_t tourIndex;
	int16_t continueTourAvailable;
	int16_t newTourAvailable;
	char christmasDate[MAINMENU_CHRISTMAS_DATE_CAPACITY - 1];
	char currentDate[MAINMENU_CURRENT_DATE_CAPACITY];
	Rect frame;
	memcpy(christmasDate, g_mainMenuChristmasDate, sizeof(christmasDate));
	g_MainMenuFocus = MAINMENU_INITIAL_FOCUS;
	xio_Set_Mouse_Position(MAINMENU_INITIAL_MOUSE_X, MAINMENU_INITIAL_MOUSE_Y);
	mainmenu_LoadPilotRecord();
	resource = xres_Open_Resource(":X-Wing Data\\RESOURCE\\mm640.lfd");
	if (resource == NULL)
		resource = xres_Open_Resource("mm640.lfd");
	xrect_Set_Rect(&frame, 0, 0, MAINMENU_WIDTH, MAINMENU_HEIGHT);
	g_MainMenuFilm = xfilm_Res_Film(resource, "mm-640", &frame, 0, 0, 0);
	xfilm_Set_Film_Def_Palette(g_MainMenuFilm, shell->standardPalette);
	LandruDisplay_ForwardLegacyNoOp(0);
	g_MainMenuBackground = xactor_Find_Actor(FOURCC_DELT, "mainmen1");
	xactor_Non_Refreshable_Actor(g_MainMenuBackground);
	g_MainMenuDoors[MAINMENU_INPUT_TRAINING] = xactor_Find_Actor(FOURCC_ANIM, "mm-tpldr");
	g_MainMenuDoors[MAINMENU_INPUT_COMBAT] = xactor_Find_Actor(FOURCC_ANIM, "mm-tpmdr");
	g_MainMenuDoors[MAINMENU_INPUT_TOUR] = xactor_Find_Actor(FOURCC_ANIM, "mm-tprdr");
	g_MainMenuDoors[MAINMENU_INPUT_TECH_ROOM] = xactor_Find_Actor(FOURCC_ANIM, "mm-btldr");
	g_MainMenuDoors[MAINMENU_INPUT_FILM_ROOM] = xactor_Find_Actor(FOURCC_ANIM, "mm-btrdr");
	g_MainMenuDoors[MAINMENU_INPUT_REGISTER] = xactor_Find_Actor(FOURCC_ANIM, "mm-regdr");
	for (doorIndex = 0; doorIndex < MAINMENU_DOOR_COUNT; ++doorIndex) {
		xactor_Set_Actor_User_Function(g_MainMenuDoors[doorIndex], mainmenu_user_Door);
		g_MainMenuDoors[doorIndex]->id = doorIndex;
	}
	g_MainMenuStars = xactor_Find_Actor(FOURCC_DELT, "mm-stars");
	g_MainMenuTitle = xactdelt_Res_Delta_Actor(resource, "mm-ttlol", &frame, 0, 0, 0);
	xactor_Set_Actor_User_Function(g_MainMenuTitle, mainmenu_user_Title);
	xactor_Set_Actor_Draw_Function(g_MainMenuTitle, XwMainMenu_DrawTitle);
	g_MainMenuRootInput = xinput_Alloc_Input(NULL, &frame, 0, 0);
	xrect_Set_Rect(&frame, MAINMENU_TRAINING_LEFT, MAINMENU_TRAINING_TOP, MAINMENU_TRAINING_RIGHT,
				   MAINMENU_TRAINING_BOTTOM);
	g_MainMenuProvingGroundInput = xinput_Alloc_Input(g_MainMenuRootInput, &frame, 0, 0);
	xinpattr_Set_Input_Update_Function(g_MainMenuProvingGroundInput, mainmenu_iupdate_MainMenu);
	xinpattr_Set_Input_User_Function(g_MainMenuProvingGroundInput, mainmenu_iuser_MainMenu);
	g_MainMenuProvingGroundInput->mouseUsage = allInput;
	g_MainMenuProvingGroundInput->id = MAINMENU_INPUT_TRAINING;
	xrect_Set_Rect(&frame, MAINMENU_COMBAT_LEFT, MAINMENU_COMBAT_TOP, MAINMENU_COMBAT_RIGHT,
				   MAINMENU_COMBAT_BOTTOM);
	g_MainMenuHistoricalCombatInput = xinput_Alloc_Input(g_MainMenuRootInput, &frame, 0, 0);
	xinpattr_Set_Input_Update_Function(g_MainMenuHistoricalCombatInput, mainmenu_iupdate_MainMenu);
	xinpattr_Set_Input_User_Function(g_MainMenuHistoricalCombatInput, mainmenu_iuser_MainMenu);
	g_MainMenuHistoricalCombatInput->mouseUsage = allInput;
	g_MainMenuHistoricalCombatInput->id = MAINMENU_INPUT_COMBAT;
	newTourAvailable = 1;
	continueTourAvailable =
		g_MainMenuPilotData.tour_status[g_MainMenuPilotData.current_tour] == SHIPEXT_TOUR_STATUS_ACTIVE;
#ifdef XW_MODERN
	continueTourAvailable =
		continueTourAvailable && shipext_IsTourAvailable(g_MainMenuPilotData.current_tour);
#endif
	for (tourIndex = 0; tourIndex < SHIPEXT_TOUR_COUNT; ++tourIndex) {
		if (shipext_IsTourAvailable(tourIndex) != 0 &&
			g_MainMenuPilotData.tour_status[tourIndex] <= SHIPEXT_TOUR_STATUS_ACTIVE)
			newTourAvailable = 1;
	}
	if (continueTourAvailable && newTourAvailable) {
		xrect_Set_Rect(&frame, MAINMENU_TOUR_LEFT, MAINMENU_TOUR_TOP, MAINMENU_TOUR_RIGHT,
					   MAINMENU_TOUR_BOTTOM);
		g_MainMenuTourInput = xinput_Alloc_Input(g_MainMenuRootInput, &frame, 0, 0);
		xinpattr_Set_Input_Update_Function(g_MainMenuTourInput, mainmenu_iupdate_MainMenu);
		xinpattr_Set_Input_User_Function(g_MainMenuTourInput, mainmenu_iuser_MainMenu);
		g_MainMenuTourInput->mouseUsage = allInput;
		g_MainMenuTourInput->id = MAINMENU_INPUT_TOUR;
		xrect_Set_Rect(&frame, MAINMENU_TOUR_DESK_LEFT, MAINMENU_TOUR_DESK_TOP, MAINMENU_TOUR_DESK_RIGHT,
					   MAINMENU_TOUR_DESK_BOTTOM);
		g_MainMenuTourInput = xinput_Alloc_Input(g_MainMenuRootInput, &frame, 0, 0);
		xinpattr_Set_Input_Update_Function(g_MainMenuTourInput, mainmenu_iupdate_MainMenu);
		xinpattr_Set_Input_User_Function(g_MainMenuTourInput, mainmenu_iuser_MainMenu);
		g_MainMenuTourInput->mouseUsage = allInput;
		g_MainMenuTourInput->id = MAINMENU_INPUT_TOUR_DESK;
	} else if (continueTourAvailable || newTourAvailable) {
		xrect_Set_Rect(&frame, MAINMENU_TOUR_DESK_LEFT, MAINMENU_TOUR_DESK_TOP, MAINMENU_TOUR_DESK_RIGHT,
					   MAINMENU_TOUR_DESK_BOTTOM);
		g_MainMenuTourInput = xinput_Alloc_Input(g_MainMenuRootInput, &frame, 0, 0);
		xinpattr_Set_Input_Update_Function(g_MainMenuTourInput, mainmenu_iupdate_MainMenu);
		xinpattr_Set_Input_User_Function(g_MainMenuTourInput, mainmenu_iuser_MainMenu);
		g_MainMenuTourInput->mouseUsage = allInput;
		if (continueTourAvailable)
			g_MainMenuTourInput->id = MAINMENU_INPUT_TOUR;
		else
			g_MainMenuTourInput->id = MAINMENU_INPUT_TOUR_DESK;
	}
	xrect_Set_Rect(&frame, MAINMENU_TECH_LEFT, MAINMENU_TECH_TOP, MAINMENU_TECH_RIGHT, MAINMENU_TECH_BOTTOM);
	g_MainMenuTechInput = xinput_Alloc_Input(g_MainMenuRootInput, &frame, 0, 0);
	xinpattr_Set_Input_Update_Function(g_MainMenuTechInput, mainmenu_iupdate_MainMenu);
	xinpattr_Set_Input_User_Function(g_MainMenuTechInput, mainmenu_iuser_MainMenu);
	g_MainMenuTechInput->mouseUsage = allInput;
	g_MainMenuTechInput->id = MAINMENU_INPUT_TECH_ROOM;
	xrect_Set_Rect(&frame, MAINMENU_FILM_LEFT, MAINMENU_FILM_TOP, MAINMENU_FILM_RIGHT, MAINMENU_FILM_BOTTOM);
	g_MainMenuFilmInput = xinput_Alloc_Input(g_MainMenuRootInput, &frame, 0, 0);
	xinpattr_Set_Input_Update_Function(g_MainMenuFilmInput, mainmenu_iupdate_MainMenu);
	xinpattr_Set_Input_User_Function(g_MainMenuFilmInput, mainmenu_iuser_MainMenu);
	g_MainMenuFilmInput->mouseUsage = allInput;
	g_MainMenuFilmInput->id = MAINMENU_INPUT_FILM_ROOM;
	xrect_Set_Rect(&frame, MAINMENU_REGISTER_LEFT, MAINMENU_REGISTER_TOP, MAINMENU_REGISTER_RIGHT,
				   MAINMENU_REGISTER_BOTTOM);
	g_MainMenuRegisterInput = xinput_Alloc_Input(g_MainMenuRootInput, &frame, 0, 0);
	xinpattr_Set_Input_Update_Function(g_MainMenuRegisterInput, mainmenu_iupdate_MainMenu);
	xinpattr_Set_Input_User_Function(g_MainMenuRegisterInput, mainmenu_iuser_MainMenu);
	g_MainMenuRegisterInput->mouseUsage = allInput;
	g_MainMenuRegisterInput->id = MAINMENU_INPUT_REGISTER;
	xrect_Set_Rect(&frame, 0, 0, MAINMENU_WIDTH, MAINMENU_HEIGHT);
#ifdef XW_MODERN
	XwMainMenu_GetDate(currentDate, sizeof(currentDate));
	if (strncmp(currentDate, christmasDate, sizeof(christmasDate)) == 0) {
#else
	_strdate(currentDate);
	if (_mbsnbicmp((const unsigned char*)currentDate, (const unsigned char*)christmasDate,
				   sizeof(christmasDate)) == 0) {
#endif
		g_MainMenuWaveActor = xactanim_Res_Anim_Actor(resource, "wave", &frame, 0, 0, MAINMENU_AMBIENT_Z);
		xactor_Set_Actor_Draw_Function(g_MainMenuWaveActor, mainmenu_draw_AmbientAnimation);
		xactor_Show_Actor(g_MainMenuWaveActor);
	}
	if ((int16_t)(rand() % MAINMENU_AMBIENT_RANDOM_RANGE) == MAINMENU_WIND_ROLL) {
		g_MainMenuWindActor = xactanim_Res_Anim_Actor(resource, "wind", &frame, 0, 0, MAINMENU_AMBIENT_Z);
		xactor_Set_Actor_Draw_Function(g_MainMenuWindActor, mainmenu_draw_AmbientAnimation);
		xactor_Show_Actor(g_MainMenuWindActor);
	}
	if ((int16_t)(rand() % MAINMENU_AMBIENT_RANDOM_RANGE) == MAINMENU_SHAKE_ROLL) {
		g_MainMenuShakeActor = xactanim_Res_Anim_Actor(resource, "shake", &frame, 0, 0, MAINMENU_AMBIENT_Z);
		xactor_Show_Actor(g_MainMenuShakeActor);
		xactor_Set_Actor_Draw_Function(g_MainMenuShakeActor, mainmenu_draw_AmbientAnimation);
	}
	xio_Set_Key_Buttons();
	mainmenu_OpenMusic(resource, g_MainMenuFilm);
	mainmenu_LoadSoundEffects(resource, g_MainMenuFilm);
	FrontendAudio_PlayFile("XwingCD\\music\\mainmenu.wav", 1);
	xview_Set_View_Update_Function(mainmenu_end_View);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
#ifdef XW_MODERN
	XwMainMenu_RunView(resource);
#else
	j_xviewadd_Handle_View();
	xview_Enable_All_View_Erase();
	xview_Clear_View_Update_Function();
	mainmenu_CloseMusic();
	soundext_ResetEnabledSfxCache();
	LandruDisplay_ForwardLegacyNoOp(0);
	xio_Clear_Key_Buttons();
	if ((uint16_t)xcursor_Is_Cursor_Visible())
		xcursor_Hide_Cursor();
	xres_Close_Resource(resource);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x458630
void mainmenu_end_View(int time) {
	int16_t key;
	if (time == 0) {
		if ((uint16_t)xcursor_Is_Cursor_Visible() == 0)
			xcursor_Show_Cursor();
		xactor_Set_Actor_Frame(g_MainMenuStars, &g_MainMenuDoors[MAINMENU_INPUT_COMBAT]->bounds);
	}
	key = xio_Get_Free_Key();
	if (key != 0 && shellext_MoveGridFocus(&g_MainMenuFocus, g_MainMenuMouseX, g_MainMenuMouseY,
										   MAINMENU_FOCUS_ROWS, MAINMENU_FOCUS_COLUMNS, key) != 0) {
		xio_Set_Mouse_Position(g_MainMenuMouseX[g_MainMenuFocus], g_MainMenuMouseY[g_MainMenuFocus]);
		xio_Get_Free_Key();
	}
}

// FUNCTION: XW 0x4586C0
int16_t mainmenu_iupdate_MainMenu(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
								  int rightEvent, int16_t x, int16_t y) {
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (key != 0) {
		return 0;
	}
	if (input->id >= 0 && input->id < MAINMENU_DOOR_COUNT) {
		g_MainMenuDoors[input->id]->var1 = MAINMENU_DOOR_OPEN_PULSE;
	}
	g_MainMenuTitle->var1 = MAINMENU_TITLE_SHOW_PULSE;
	g_MainMenuTitle->var2 = input->id;
	if (leftEvent == MAINMENU_ACTIVATE_EVENT || rightEvent == MAINMENU_ACTIVATE_EVENT) {
		switch (input->id) {
			case MAINMENU_INPUT_TRAINING:
				input->var1 = 1;
				input->var2 = XW_SCENE_TRAINING_DEPART_INDEPENDENCE;
				break;
			case MAINMENU_INPUT_COMBAT:
				input->var1 = 1;
				input->var2 = XW_SCENE_COMBAT_DEPART_INDEPENDENCE;
				break;
			case MAINMENU_INPUT_TOUR:
				input->var1 = 1;
				input->var2 = XW_SCENE_TOUR_DEPART_INDEPENDENCE;
				break;
			case MAINMENU_INPUT_TECH_ROOM:
				input->var1 = 1;
				input->var2 = XW_SCENE_TECH_ROOM;
				break;
			case MAINMENU_INPUT_FILM_ROOM:
				input->var1 = 1;
				input->var2 = XW_SCENE_FILM_ROOM;
				shipext_Set_Mission_Outcome(SHIPEXT_MISSION_OUTCOME_FILM_ROOM);
				break;
			case MAINMENU_INPUT_REGISTER:
				input->var1 = 1;
				input->var2 = XW_SCENE_REGISTER_RETURN;
				break;
			case MAINMENU_INPUT_TOUR_DESK:
				input->var1 = 1;
				input->var2 = XW_SCENE_TOUR_DESK;
				break;
		}
	}
	return 1;
}

// FUNCTION: XW 0x4587C0
void mainmenu_iuser_MainMenu(Input* input, int time) {
	(void)time;
	if (input->var1 != 0) {
		xerror_Set_Landru_Exit(input->var2);
	}
}

// FUNCTION: XW 0x4587E0
int16_t mainmenu_draw_AmbientAnimation(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
									   int16_t refresh) {
	int16_t state;
	if (refresh == 0) {
		return 0;
	}
	state = actor->state;
	if (state < actor->arraySize - 1) {
		xactor_Set_Actor_State(actor, state + 1, 0);
	} else {
		xactor_Set_Actor_State(actor, 0, 0);
	}
	return xactanim_Draw_Anim_Actor(actor, frame, clip, x, y, refresh);
}

// FUNCTION: XW 0x458840
void mainmenu_user_Title(Actor* actor, int time) {
	(void)time;
	if (actor->var1 == MAINMENU_TITLE_SHOW_PULSE) {
		if (!xactor_Is_Actor_Visible(actor))
			xactor_Show_Actor(actor);
		actor->var1 = 0;
	} else if (xactor_Is_Actor_Visible(actor)) {
		xactor_Hide_Actor(actor);
	}
}

// FUNCTION: XW 0x458890
void mainmenu_draw_Title(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	Rect labelRect;
	char text[MAINMENU_TITLE_CAPACITY];
	int16_t offsetX;
	int16_t offsetY;
	if (refresh == 0)
		return;
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
			if (
#ifdef XW_MODERN
				shipext_IsTourAvailable(g_MainMenuPilotData.current_tour) &&
#endif
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
	xfont_Print_Centered_Text(text, &labelRect, MAINMENU_TITLE_FONT, MAINMENU_TITLE_SHADOW_COLOR);
	xrect_Offset_Rect(&labelRect, -MAINMENU_TITLE_SHADOW_OFFSET, -MAINMENU_TITLE_SHADOW_OFFSET);
	xfont_Print_Centered_Text(text, &labelRect, MAINMENU_TITLE_FONT, MAINMENU_TITLE_TEXT_COLOR);
}

// FUNCTION: XW 0x458AA0
void mainmenu_user_Door(Actor* actor, int time) {
	(void)time;
	mainmenu_UpdateDoorMusic(actor);
	if (actor->var1 != 0) {
		int16_t state;
		if (actor->state == MAINMENU_DOOR_CLOSED)
			mainmenu_PlayDoorSoundCue(XW_MAINMENU_CUE_DOOR_OPEN);
		state = actor->state;
		if (state < actor->arraySize - 1) {
			xactor_Set_Actor_State(actor, state + 1, 0);
			if (actor->id == MAINMENU_INPUT_COMBAT) {
				xpaint_Paint_Clipped_Rect(&actor->bounds, 0);
				xactor_Refresh_Actor(g_MainMenuStars);
			}
		}
		actor->var1 = 0;
	} else {
		int16_t state;
		if (actor->state == MAINMENU_DOOR_OPEN)
			mainmenu_PlayDoorSoundCue(XW_MAINMENU_CUE_DOOR_CLOSE);
		state = actor->state;
		if (state > MAINMENU_DOOR_CLOSED) {
			xactor_Set_Actor_State(actor, state - 1, 0);
			if (actor->id == MAINMENU_INPUT_COMBAT) {
				xpaint_Paint_Clipped_Rect(&actor->bounds, 0);
				xactor_Refresh_Actor(g_MainMenuStars);
			}
		}
	}
}

// FUNCTION: XW 0x458B60
int16_t mainmenu_LoadPilotRecord(void) {
	char pilotPath[MAINMENU_PILOT_FILENAME_CAPACITY];
	LandruFile* stream;
	strcpy(pilotPath, g_RegisterShellPilot.name);
	strcat(pilotPath, ".PLT");
	stream = xfile_Open_File(LANDRU_FILE_ROOT_USER, pilotPath, "rb");
	if (stream != NULL) {
		register_ReadPilotRecord(stream, &g_MainMenuPilotData);
		xfile_Close_File(stream);
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x4593C0
void mainmenu_OpenMusic(ResFile* unusedResourceFile, Film* unusedFilm) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_MainMenuSecurityMusic = xsound_Find_Gmid("security");
		xsound_Set_Sound_Keep(g_MainMenuSecurityMusic);
		g_MainMenuMarchMusic = xsound_Find_Gmid("halmarch");
		if (g_MainMenuMarchMusic != NULL) {
			if (soundext_Count_Resource_Instances(g_MainMenuMarchMusic) == 1) {
				int group = soundext_GetMusicParam(g_MainMenuMarchMusic, XW_SOUND_QUERY_CHUNK, 0);
				if (group != 0) {
					soundext_FadeVolume(g_MainMenuMarchMusic, 0, MAINMENU_EXIT_MUSIC_FADE_DURATION);
					g_MainMenuMarchMusic = NULL;
				} else {
					soundext_FadeVolume(g_MainMenuMarchMusic, MAINMENU_TARGET_MUSIC_VOLUME,
										MAINMENU_TARGET_MUSIC_FADE_DURATION);
				}
			}
		}
		if (g_MainMenuMarchMusic == NULL) {
			ResFile* musicResource = xres_Open_Resource(":X-Wing Data\\RESOURCE\\mmmusic.lfd");
			if (musicResource == NULL)
				musicResource = xres_Open_Resource("mmmusic.lfd");
			g_MainMenuMarchMusic = xsound_Res_Music(musicResource, "halmarch");
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_MainMenuMarchMusic);
			soundext_FadeVolume(g_MainMenuMarchMusic, MAINMENU_TARGET_MUSIC_VOLUME,
								MAINMENU_TARGET_MUSIC_FADE_DURATION);
		}
		xsound_Set_Sound_Keep(g_MainMenuMarchMusic);
		xsound_Set_Sound_User_Function(g_MainMenuMarchMusic, mainmenu_user_Music);
	}
}

// FUNCTION: XW 0x4594F0
void mainmenu_UpdateDoorMusic(Actor* door) {
	int controlValue;
	int doorId;
	int16_t state;
	if (ShellPreferences_GetMusicEnabled() == 0)
		return;
	state = door->state;
	controlValue = state == MAINMENU_DOOR_CLOSED;
	if (!((door->var1 != 0 && state == MAINMENU_DOOR_CLOSED) ||
		  (door->var1 == 0 && state == MAINMENU_DOOR_OPEN))) {
		int16_t tourControl;
		if (door->id != MAINMENU_INPUT_TOUR_DESK && door->id != MAINMENU_INPUT_TOUR)
			return;
		tourControl = door->var2;
		if (!((tourControl != 0 && state == MAINMENU_DOOR_TOUR_OPEN) ||
			  (tourControl == 0 && state == MAINMENU_DOOR_TOUR_CLOSED)))
			return;
		controlValue = tourControl;
	}
	doorId = door->id;
	g_MainMenuLastMusicDoorId = doorId;
	switch (doorId) {
		case MAINMENU_INPUT_TRAINING:
			if (controlValue != 0) {
				soundext_SetPartEnabled(g_MainMenuMarchMusic, MAINMENU_MUSIC_TRAINING_CHANNEL, controlValue);
				soundext_SetHook(g_MainMenuMarchMusic, XW_SOUND_CONTROL_PACKED, 0,
								 MAINMENU_MUSIC_TRAINING_CHANNEL);
				soundext_FadeVolume(g_MainMenuMarchMusic, MAINMENU_MUSIC_DOOR_ACTIVE_VOLUME,
									MAINMENU_MUSIC_DOOR_ACTIVE_DURATION);
			} else {
				soundext_SetHook(g_MainMenuMarchMusic, XW_SOUND_CONTROL_PACKED, 1,
								 MAINMENU_MUSIC_TRAINING_CHANNEL);
				soundext_FadeVolume(g_MainMenuMarchMusic, MAINMENU_MUSIC_DOOR_INACTIVE_VOLUME,
									MAINMENU_MUSIC_DOOR_INACTIVE_DURATION);
			}
			break;
		case MAINMENU_INPUT_COMBAT:
			if (controlValue != 0) {
				soundext_SetPartEnabled(g_MainMenuMarchMusic, MAINMENU_MUSIC_COMBAT_CHANNEL, controlValue);
				soundext_SetHook(g_MainMenuMarchMusic, XW_SOUND_CONTROL_PACKED, 0,
								 MAINMENU_MUSIC_COMBAT_CHANNEL);
				soundext_FadeVolume(g_MainMenuMarchMusic, MAINMENU_MUSIC_DOOR_ACTIVE_VOLUME,
									MAINMENU_MUSIC_DOOR_ACTIVE_DURATION);
			} else {
				soundext_SetHook(g_MainMenuMarchMusic, XW_SOUND_CONTROL_PACKED, 1,
								 MAINMENU_MUSIC_COMBAT_CHANNEL);
				soundext_FadeVolume(g_MainMenuMarchMusic, MAINMENU_MUSIC_DOOR_INACTIVE_VOLUME,
									MAINMENU_MUSIC_DOOR_INACTIVE_DURATION);
			}
			break;
		case MAINMENU_INPUT_TOUR:
		case MAINMENU_INPUT_TOUR_DESK:
			if (controlValue != 0) {
				soundext_SetPartEnabled(g_MainMenuMarchMusic, MAINMENU_MUSIC_TOUR_CHANNEL, controlValue);
				soundext_SetHook(g_MainMenuMarchMusic, XW_SOUND_CONTROL_PACKED, 0,
								 MAINMENU_MUSIC_TOUR_CHANNEL);
				soundext_FadeVolume(g_MainMenuMarchMusic, MAINMENU_MUSIC_TOUR_ACTIVE_VOLUME,
									MAINMENU_MUSIC_DOOR_ACTIVE_DURATION);
			} else {
				soundext_SetHook(g_MainMenuMarchMusic, XW_SOUND_CONTROL_PACKED, 1,
								 MAINMENU_MUSIC_TOUR_CHANNEL);
				soundext_FadeVolume(g_MainMenuMarchMusic, MAINMENU_MUSIC_DOOR_INACTIVE_VOLUME,
									MAINMENU_MUSIC_DOOR_INACTIVE_DURATION);
			}
			break;
		default:
			return;
	}
}

// FUNCTION: XW 0x4596E0
void mainmenu_CloseMusic(void) {
	int exitScene = xerror_Get_Landru_Exit();
	if (ShellPreferences_GetMusicEnabled() != 0) {
		soundext_FadeVolume(g_MainMenuMarchMusic, MAINMENU_MARCH_VOLUME, MAINMENU_CLOSE_MUSIC_FADE_DURATION);
		switch (exitScene) {
			case XW_SCENE_TRAINING_DEPART_INDEPENDENCE:
				mainmenu_TransitionToTrainingMusic();
				break;
			case XW_SCENE_COMBAT_DEPART_INDEPENDENCE:
			case XW_SCENE_TOUR_DEPART_INDEPENDENCE:
				mainmenu_TransitionToCombatMusic();
				break;
			case XW_SCENE_TOUR_DESK:
				mainmenu_TransitionToTourDeskMusic();
				break;
			case XW_SCENE_TECH_ROOM:
				mainmenu_TransitionToBlueprintMusic();
				break;
			case XW_SCENE_FILM_ROOM:
				mainmenu_TransitionToFilmMusic();
				break;
			case XW_SCENE_REGISTER_RETURN:
				mainmenu_StopMusicForExit();
				break;
			default:
				return;
		}
	}
}

// FUNCTION: XW 0x4597B0
void mainmenu_user_Music(Sound* unusedSound, int unusedTime) {
	(void)unusedSound;
	(void)unusedTime;
	if (g_MainMenuSecurityMusic != NULL && soundext_Count_Resource_Instances(g_MainMenuSecurityMusic) != 1) {
		xsound_Clear_Sound_Keep(g_MainMenuSecurityMusic);
		xsound_Free_Sound(g_MainMenuSecurityMusic);
		g_MainMenuSecurityMusic = NULL;
	}
}

// FUNCTION: XW 0x4597F0
void mainmenu_TransitionToTrainingMusic(void) {
	Sound* loadedMusic;
	mainmenu_PrepareMusicTransition("trmusic.lfd", "rebels", MAINMENU_TRAINING_MUSIC_CONTROL, &loadedMusic);
}

// FUNCTION: XW 0x459810
void mainmenu_TransitionToCombatMusic(void) {
	Sound* loadedMusic;
	mainmenu_PrepareMusicTransition("cbmusic.lfd", "mission", MAINMENU_COMBAT_MUSIC_CONTROL, &loadedMusic);
}

// FUNCTION: XW 0x459830
void mainmenu_TransitionToTourDeskMusic(void) {
	ResFile* musicResource = xres_Open_Resource(":X-Wing Data\\RESOURCE\\tdmusic.lfd");
	Sound* tourDeskMusic;
	Sound* marchMusic;
	if (musicResource == NULL)
		musicResource = xres_Open_Resource("tdmusic.lfd");
	tourDeskMusic = xsound_Res_Music(musicResource, "tourdesk");
	xres_Close_Resource(musicResource);
	soundext_ClearTriggers();
	if (soundext_Count_Resource_Instances(g_MainMenuMarchMusic) != 1) {
		soundext_FadeVolume(g_MainMenuSecurityMusic, 0, MAINMENU_SECURITY_FADE_DURATION);
		/* Resource pointers are outside the numeric flight-sound ID range. */
		soundext_SetPriority(0, 0);
		marchMusic = g_MainMenuMarchMusic;
		soundext_Start_Resource_Sound(marchMusic);
	}
	marchMusic = g_MainMenuMarchMusic;
	soundext_FadeVolume(marchMusic, MAINMENU_MARCH_VOLUME, MAINMENU_EXIT_MUSIC_FADE_DURATION);
	soundext_SetTriggerContext(g_MainMenuMarchMusic, MAINMENU_TOUR_DESK_JUMP_MARKER);
	soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_JUMP, g_MainMenuMarchMusic->id,
								 MAINMENU_TOUR_DESK_JUMP_GROUP, 0, 0, 0, 0);
	soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_END, 0, 0, 0, 0, 0, 0);
	soundext_SetHook(g_MainMenuMarchMusic, XW_SOUND_CONTROL_DIRECT, MAINMENU_TOUR_DESK_MUSIC_CONTROL, 0);
	soundext_SetTriggerContext(g_MainMenuMarchMusic, MAINMENU_MUSIC_TRANSITION_MARKER);
	soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_START, tourDeskMusic->id, 0, 0, 0, 0, 0);
	soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_SHARE_PARTS, g_MainMenuMarchMusic->id, tourDeskMusic->id, 0,
								 0, 0, 0);
	soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_FADE_VOLUME, tourDeskMusic->id,
								 MAINMENU_TARGET_MUSIC_VOLUME, MAINMENU_TARGET_MUSIC_FADE_DURATION, 0, 0, 0);
	soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_END, 0, 0, 0, 0, 0, 0);
}

// FUNCTION: XW 0x4599B0
void mainmenu_TransitionToFilmMusic(void) {
	mainmenu_PrepareFadedMusicTransition("bpmusic.lfd", "waiting", MAINMENU_FILM_MUSIC_CONTROL);
}

// FUNCTION: XW 0x4599D0
void mainmenu_TransitionToBlueprintMusic(void) {
	mainmenu_PrepareFadedMusicTransition("bpmusic.lfd", "waiting", MAINMENU_BLUEPRINT_MUSIC_CONTROL);
}

// FUNCTION: XW 0x4599F0
void mainmenu_StopMusicForExit(void) {
	Sound* securityMusic = xsound_Find_Gmid("security");
	if (securityMusic != NULL)
		soundext_Stop_Resource_Sound(securityMusic);
	soundext_FadeVolume(g_MainMenuMarchMusic, 0, MAINMENU_EXIT_MUSIC_FADE_DURATION);
}

// FUNCTION: XW 0x459A20
void mainmenu_PrepareFadedMusicTransition(const char* filename, const char* name, uint8_t controlValue) {
	ResFile* musicResource = xres_Open_Resource(filename);
	Sound* targetMusic = xsound_Res_Music(musicResource, name);
	Sound* marchMusic;
	xres_Close_Resource(musicResource);
	soundext_ClearTriggers();
	if (soundext_Count_Resource_Instances(g_MainMenuMarchMusic) != 1) {
		soundext_FadeVolume(g_MainMenuSecurityMusic, 0, MAINMENU_SECURITY_FADE_DURATION);
		/* Resource pointers are outside the numeric flight-sound ID range. */
		soundext_SetPriority(0, 0);
		marchMusic = g_MainMenuMarchMusic;
		soundext_Start_Resource_Sound(marchMusic);
	}
	marchMusic = g_MainMenuMarchMusic;
	soundext_FadeVolume(marchMusic, MAINMENU_MARCH_VOLUME, MAINMENU_EXIT_MUSIC_FADE_DURATION);
	/* Resource pointers are outside the numeric flight-sound ID range. */
	soundext_SetPriority(0, 0);
	marchMusic = g_MainMenuMarchMusic;
	soundext_SetHook(marchMusic, XW_SOUND_CONTROL_DIRECT, controlValue, 0);
	marchMusic = g_MainMenuMarchMusic;
	soundext_SetTriggerContext(marchMusic, MAINMENU_MUSIC_TRANSITION_MARKER);
	soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_START, targetMusic->id, 0, 0, 0, 0, 0);
	soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_FADE_VOLUME, targetMusic->id, MAINMENU_TARGET_MUSIC_VOLUME,
								 MAINMENU_TARGET_MUSIC_FADE_DURATION, 0, 0, 0);
	soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_END, 0, 0, 0, 0, 0, 0);
	xsound_Set_Sound_Keep(targetMusic);
}

// FUNCTION: XW 0x459B40
void mainmenu_PrepareMusicTransition(const char* filename, const char* name, uint8_t controlValue,
									 Sound** outSound) {
	ResFile* musicResource = xres_Open_Resource(filename);
	Sound* targetMusic = xsound_Res_Music(musicResource, name);
	Sound* marchMusic;
	xres_Close_Resource(musicResource);
	soundext_ClearTriggers();
	if (soundext_Count_Resource_Instances(g_MainMenuMarchMusic) != 1) {
		soundext_FadeVolume(g_MainMenuSecurityMusic, 0, MAINMENU_SECURITY_FADE_DURATION);
		soundext_Start_Resource_Sound(g_MainMenuMarchMusic);
	}
	soundext_FadeVolume(g_MainMenuMarchMusic, MAINMENU_MARCH_VOLUME, MAINMENU_EXIT_MUSIC_FADE_DURATION);
	/* Resource pointers are outside the numeric flight-sound ID range. */
	soundext_SetPriority(0, 0);
	marchMusic = g_MainMenuMarchMusic;
	soundext_SetHook(marchMusic, XW_SOUND_CONTROL_DIRECT, controlValue, 0);
	marchMusic = g_MainMenuMarchMusic;
	soundext_SetTriggerContext(marchMusic, MAINMENU_MUSIC_TRANSITION_MARKER);
	soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_START, targetMusic->id, 0, 0, 0, 0, 0);
	soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_END, 0, 0, 0, 0, 0, 0);
	xsound_Set_Sound_Keep(targetMusic);
	*outSound = targetMusic;
}

// FUNCTION: XW 0x459C40
void mainmenu_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	if (ShellPreferences_GetSfxEnabled() != 0) {
		soundext_LoadSfx(XW_SHELL_SFX_DOOR_1, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_MENU_DOOR_CLOSE_1, 0, NULL, 0, 1);
	}
	if (ShellPreferences_GetSfxEnabled() != 0) {
		g_MainMenuAmbientSpeech[0] =
			soundext_LoadSpeech(XW_SHELL_SPEECH_BOARDING_1, 0, mainmenu_user_AmbientSpeech, 0);
		g_MainMenuAmbientSpeech[1] = soundext_LoadSpeech(XW_SHELL_SPEECH_BOARDING_2, 1, NULL, 0);
		g_MainMenuAmbientSpeech[2] = soundext_LoadSpeech(XW_SHELL_SPEECH_SHUTTLE, 2, NULL, 0);
	}
}

// FUNCTION: XW 0x459CC0
void mainmenu_user_AmbientSpeech(Sound* sound, int time) {
	if (time == 0) {
		sound->var1 = (rand() & MAINMENU_AMBIENT_INITIAL_RANDOM_MASK) + MAINMENU_AMBIENT_INITIAL_DELAY;
	} else if (sound->var1 != 0) {
		--sound->var1;
	} else {
		uint16_t randomChoice = (uint16_t)rand();
		soundext_Start_Resource_SFX(g_MainMenuAmbientSpeech[randomChoice % MAINMENU_AMBIENT_SPEECH_COUNT]);
		sound->var1 = (rand() & MAINMENU_AMBIENT_REPEAT_RANDOM_MASK) + MAINMENU_AMBIENT_REPEAT_DELAY;
	}
}

// FUNCTION: XW 0x459D30
void mainmenu_PlayDoorSoundCue(int16_t cue) {
	if (ShellPreferences_GetSfxEnabled()) {
		switch (cue) {
			case XW_MAINMENU_CUE_DOOR_OPEN:
				soundext_Play_SFX(XW_SHELL_SFX_DOOR_1);
				break;
			case XW_MAINMENU_CUE_DOOR_CLOSE:
				soundext_Play_SFX(XW_SHELL_SFX_MENU_DOOR_CLOSE_1);
				break;
		}
	}
}
