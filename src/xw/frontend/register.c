#include "xw/frontend/register.h"

#ifdef XW_MODERN
#include "xw_runtime/runtime/port.h"
#endif

#include "xw/audio/frontend_audio.h"
#include "xw/audio/soundext.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"
#include "xw/util/landru_display.h"
#include "xw/util/shared.h"
#include "xw_runtime/integration/input_callbacks.h"
#include "xw_runtime/integration/register_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/register_view_task.h"
#endif

#include "xw_runtime/runtime/register_task.h"

#include <ctype.h>
#include <landru/actanim.h>
#include <landru/actdelt.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/dialog.h>
#include <landru/dlgjoy.h>
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

// GLOBAL: XW 0x4D6B08
int16_t g_RegisterActivePilot = REGISTER_PILOT_SLOT_NONE;

// GLOBAL: XW 0x4D6B10
const int16_t g_RegisterNavigationX[REGISTER_NAVIGATION_COUNT] = {
	12,  118, 153, 153, 153, 12,  118, 153, 153, 153, 12,  118, 153, 153, 153, 12,  118, 153, 153,
	153, 12,  118, 153, 153, 153, 12,  118, 153, 153, 153, 12,  118, 153, 153, 153, 12,  118, 153,
	153, 153, 12,  118, 153, 153, 153, 12,  118, 153, 219, 256, 12,  115, 175, 243, 536
};

// GLOBAL: XW 0x4D6B80
const int16_t g_RegisterNavigationY[REGISTER_NAVIGATION_COUNT] = {
	160, 98,  98,  98,  98,  160, 105, 105, 105, 105, 160, 112, 112, 112, 112, 160, 119, 119, 119,
	119, 160, 126, 126, 126, 126, 160, 133, 133, 133, 133, 160, 140, 140, 140, 140, 160, 147, 147,
	147, 147, 160, 154, 154, 154, 154, 160, 161, 161, 179, 179, 160, 175, 175, 191, 394
};

// GLOBAL: XW 0x4D6BF0
int16_t g_RegisterDeleteMouseX[REGISTER_DELETE_TARGET_COUNT] = { 274, 376, 478 };

// GLOBAL: XW 0x4D6BF8
int16_t g_RegisterDeleteMouseY[REGISTER_DELETE_TARGET_COUNT] = { 254, 254, 254 };

// GLOBAL: XW 0x4D6C00
int16_t g_RegisterProtectMouseX[REGISTER_PROTECT_TARGET_COUNT] = { 122, 180 };

// GLOBAL: XW 0x4D6C04
int16_t g_RegisterProtectMouseY[REGISTER_PROTECT_TARGET_COUNT] = { 130, 130 };

// GLOBAL: XW 0x4D6C08
char g_RegisterRankNames[REGISTER_RANK_COUNT][REGISTER_INFO_SHORT_TEXT_SIZE] = { "Flt. Cadet", "Flt. Officer",
																				 "Lieutenant", "Captain",
																				 "Commander",  "General" };

// GLOBAL: XW 0x4D6C8C
char g_RegisterLostStatusLabels[REGISTER_STATUS_LABEL_COUNT][REGISTER_STATUS_LABEL_SIZE] = { "Active Tour",
																							 "(Captured)",
																							 "(Killed)" };

// GLOBAL: XW 0x4D6CB0
const char g_RegisterResourceAndLabelText[REGISTER_RESOURCE_TEXT_COUNT][REGISTER_RESOURCE_TEXT_CAPACITY] = {
	"regis640.lfd", "regis4",      "regis",
	"door2mm",      "lh_btn",      "solder28",
	"rh_btn",       "robot42",     "rh-desk",
	"lh-desk",      "You MUST",    "Register!",
	"Enter",        "Spaceport",   "accessing",
	"namebck2",     "tours",       "symbols",
	"OK",           "Exit to DOS", "Press to Continue",
	"Log",          "Merits",      "Delete Pilot",
	"Delete",       "Cancel",      "This Pilot is",
	"Captured!",    "Dead!",       "Modify Pilot",
	"Revive",       "button1",     "button2",
	"button3"
};

// GLOBAL: XW 0x4D6F58
int16_t g_RegisterProtectQuestionSymbols[REGISTER_PROTECT_QUESTION_COUNT][REGISTER_PROTECT_SYMBOL_COUNT] = {
	{ 11, 9, 0 },  { 3, 5, 2 },  { 1, 0, 6 },  { 4, 10, 3 }, { 10, 9, 3 },  { 8, 11, 6 }, { 9, 10, 4 },
	{ 6, 1, 11 },  { 1, 5, 7 },  { 2, 0, 10 }, { 2, 9, 3 },  { 10, 0, 11 }, { 3, 9, 4 },  { 6, 0, 1 },
	{ 7, 11, 1 },  { 1, 10, 2 }, { 10, 1, 0 }, { 2, 10, 0 }, { 3, 8, 9 },   { 10, 5, 8 }, { 6, 3, 5 },
	{ 7, 8, 10 },  { 4, 6, 11 }, { 11, 7, 6 }, { 5, 0, 9 },  { 9, 2, 7 },   { 6, 7, 8 },  { 0, 1, 2 },
	{ 9, 10, 11 }, { 3, 4, 5 },  { 0, 9, 1 },  { 10, 4, 2 },
};

// GLOBAL: XW 0x4D7018
char g_RegisterProtectAnswers[REGISTER_PROTECT_QUESTION_COUNT][REGISTER_PROTECT_ANSWER_CAPACITY] = {
	"mantooine", "corellian", "alderaan",  "ghorman", "calamari", "gamorr",  "ithor",      "ottega",
	"sullust",   "ryloth",    "hoth",      "dagobah", "geedon",   "delalt",  "bimmisaari", "dantooine",
	"briggia",   "massassi",  "chandrila", "agamar",  "lahara",   "yavin",   "hadar",      "condorra",
	"endaba",    "tullianne", "ehapah",    "altar",   "antike",   "dargoan", "kashyyyk",   "atrivis",
};

// GLOBAL: XW 0x4F92A8
LandruHandle g_RegisterFastPilotHandle = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4F92B0
Actor* g_RegisterDeleteButtonActor = NULL;

// GLOBAL: XW 0x4F92B4
Input* g_RegisterDoorInput = NULL;

// GLOBAL: XW 0x4F92B8
Actor* g_RegisterSoldierActor = NULL;

// GLOBAL: XW 0x4F92BC
Input* g_RegisterPilotInfoInput = NULL;

// GLOBAL: XW 0x4F92C0
int16_t g_RegisterProtectChosen = 0;

// GLOBAL: XW 0x4F92C4
Actor* g_RegisterLeftButtonActor = NULL;

// GLOBAL: XW 0x4F92C8
Input* g_RegisterDeleteButton = NULL;

// GLOBAL: XW 0x4F92D0
REGISTER_PilotFileRecord g_RegisterPilotReadBuffer = { 0 };

// GLOBAL: XW 0x4F9980
Directory g_RegisterDirectory = { 0 };

// GLOBAL: XW 0x4F99B0
Input* g_RegisterPilotListInput = NULL;

// GLOBAL: XW 0x4F99B4
Actor* g_RegisterLogButtonActor = NULL;

// GLOBAL: XW 0x4F99BC
int16_t g_RegisterPageCount = 0;

// GLOBAL: XW 0x4F99C0
int16_t g_RegisterProtectSymbols[REGISTER_PROTECT_SYMBOL_COUNT] = { 0, 0, 0 };

// GLOBAL: XW 0x4F99C8
int16_t g_RegisterProtectQuestionIndex = 0;

// GLOBAL: XW 0x4F99CC
Input* g_RegisterProtectDialog = NULL;

// GLOBAL: XW 0x4F99D0
Input* g_RegisterLogButton = NULL;

// GLOBAL: XW 0x4F99D4
Actor* g_RegisterRightButtonActor = NULL;

// GLOBAL: XW 0x4F99D8
Actor* g_RegisterMeritsButtonActor = NULL;

// GLOBAL: XW 0x4F99DC
REGISTER_RegStringButton* g_RegisterPilotNameInput = NULL;

// GLOBAL: XW 0x4F99E0
LandruHandle g_RegisterTourParagraph = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4F99E4
Actor* g_RegisterLeftDeskActor = NULL;

// GLOBAL: XW 0x4F99E8
int16_t g_RegisterCurrentPage = 0;

// GLOBAL: XW 0x4F99EC
int16_t g_RegisterProtectAttempts = 0;

// GLOBAL: XW 0x4F99F0
REGISTER_PilotFileRecord g_RegisterPilotData = { 0 };

// GLOBAL: XW 0x4FA09C
Actor* g_RegisterRightDeskActor = NULL;

// GLOBAL: XW 0x4FA0A0
REGISTER_RegStringButton* g_RegisterProtectNameInput = NULL;

// GLOBAL: XW 0x4FA0A4
Input* g_RegisterProtectOkInput = NULL;

// GLOBAL: XW 0x4FA0A8
Input* g_RegisterProtectExitInput = NULL;

// GLOBAL: XW 0x4FA0B0
int16_t g_RegisterDeleteLabelState = 0;

// GLOBAL: XW 0x4FA0B4
Actor* g_RegisterRobotActor = NULL;

// GLOBAL: XW 0x4FA0B8
Input* g_RegisterListOverlayInput = NULL;

// GLOBAL: XW 0x4FA0BC
Film* g_RegisterFilm = NULL;

// GLOBAL: XW 0x4FA0C4
Actor* g_RegisterDoorActor = NULL;

// GLOBAL: XW 0x4FA0C8
Input* g_RegisterMeritsButton = NULL;

// GLOBAL: XW 0x4FA0CC
Actor* g_RegisterProtectSymbolActor = NULL;

// GLOBAL: XW 0x4FA0D0
Actor* g_RegisterBackgroundActor = NULL;

// GLOBAL: XW 0x4FA0D4
Input* g_RegisterPageInput = NULL;

// GLOBAL: XW 0x4FA0D8
int16_t g_RegisterNavigationIndex = 0;

// GLOBAL: XW 0x4FA0DC
int16_t g_RegisterPilotOffset = 0;

// GLOBAL: XW 0x4FA0E0
int16_t g_RegisterPilotCount = 0;

// GLOBAL: XW 0x4FA0E4
int16_t g_RegisterLoadedPilotCount = 0;

// GLOBAL: XW 0x4FA0EC
int16_t g_RegisterRobotState = 0;

// GLOBAL: XW 0x4FA0F8
int16_t g_RegisterDeleteFocus = 0;

// GLOBAL: XW 0x4FA0FC
int16_t g_RegisterProtectFocus = 0;

// GLOBAL: XW 0x4FA130
XwSceneMusicHandles g_RegisterMusicState = { NULL, NULL };

// GLOBAL: XW 0x4FA138
Sound* g_RegisterSpeech[REGISTER_SPEECH_COUNT] = { NULL, NULL, NULL };

// GLOBAL: XW 0x5B8AC0
REGISTER_FastPilotRecord g_RegisterShellPilot = { 0 };

// GLOBAL: XW 0x5BECA8
int g_RegisterProtectionEnabled = 0;

// FUNCTION: XW 0x45BF50
XwShellSceneResult register_Register(struct XwShellContext* shell) {
	ResFile* resource;
	PushButton* previousButton;
	PushButton* nextButton;
	Input* pageNumber;
	Rect rect;
	g_RegisterNavigationIndex = REGISTER_INITIAL_FOCUS;
	xio_Set_Mouse_Position(g_RegisterNavigationX[REGISTER_INITIAL_FOCUS],
						   g_RegisterNavigationY[REGISTER_INITIAL_FOCUS]);
	g_RegisterTourParagraph =
		xparagrp_Res_Paragraph(shell->resourceFile, g_RegisterResourceAndLabelText[REGISTER_RESOURCE_TOURS]);
	resource = xres_Open_Resource(":X-Wing Data\\RESOURCE\\regis640.lfd");
	if (resource == NULL)
		resource = xres_Open_Resource(g_RegisterResourceAndLabelText[REGISTER_RESOURCE_FILE]);
	g_RegisterActivePilot = REGISTER_PILOT_SLOT_NONE;
	g_RegisterPilotOffset = 0;
	g_RegisterPilotCount = 0;
	g_RegisterCurrentPage = 0;
	g_RegisterPageCount = 1;
	g_RegisterFastPilotHandle = LANDRU_NULL_HANDLE;
	xrect_Set_Rect(&rect, 0, 0, REGISTER_WIDTH, REGISTER_HEIGHT);
	g_RegisterFilm =
		xfilm_Res_Film(resource, g_RegisterResourceAndLabelText[REGISTER_RESOURCE_FILM], &rect, 0, 0, 0);
	xfilm_Set_Film_Def_Palette(g_RegisterFilm, shell->standardPalette);
	g_RegisterBackgroundActor =
		xactor_Find_Actor(FOURCC_DELT, g_RegisterResourceAndLabelText[REGISTER_RESOURCE_BACKGROUND]);
	xactor_Set_Actor_Draw_Function(g_RegisterBackgroundActor, register_draw_Register_Back);
	g_RegisterDoorActor =
		xactor_Find_Actor(FOURCC_ANIM, g_RegisterResourceAndLabelText[REGISTER_RESOURCE_DOOR]);
	xactor_Set_Actor_User_Function(g_RegisterDoorActor, register_user_Door);
	g_RegisterRobotActor =
		xactor_Find_Actor(FOURCC_ANIM, g_RegisterResourceAndLabelText[REGISTER_RESOURCE_ROBOT]);
	xactor_Set_Actor_User_Function(g_RegisterRobotActor, register_user_Robot);
	xactor_Set_Actor_Draw_Function(g_RegisterRobotActor, register_draw_Robot);
	g_RegisterSoldierActor =
		xactor_Find_Actor(FOURCC_ANIM, g_RegisterResourceAndLabelText[REGISTER_RESOURCE_SOLDIER]);
	xactor_Set_Actor_User_Function(g_RegisterSoldierActor, register_user_Troop);
	xactor_Set_Actor_Draw_Function(g_RegisterSoldierActor, register_draw_Troop);
	g_RegisterSoldierActor->id = REGISTER_SOLDIER_ID;
	g_RegisterRightDeskActor = xactdelt_Res_Delta_Actor(
		resource, g_RegisterResourceAndLabelText[REGISTER_RESOURCE_RIGHT_DESK], &rect, 0, 0, 0);
	xactor_Set_Actor_Time(g_RegisterRightDeskActor, 0, 0);
	g_RegisterLeftDeskActor = xactdelt_Res_Delta_Actor(
		resource, g_RegisterResourceAndLabelText[REGISTER_RESOURCE_LEFT_DESK], &rect, 0, 0, 0);
	xactor_Set_Actor_Time(g_RegisterLeftDeskActor, 0, 0);
	g_RegisterPageInput = xinput_Alloc_Input(NULL, &rect, 0, 0);
	xinpattr_Refresh_Input(g_RegisterPageInput);
	xrect_Set_Rect(&rect, REGISTER_DOOR_LEFT, REGISTER_DOOR_TOP, REGISTER_DOOR_RIGHT, REGISTER_DOOR_BOTTOM);
	g_RegisterDoorInput = xinput_Alloc_Input(g_RegisterPageInput, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(g_RegisterDoorInput, register_iupdate_Register);
	xinpattr_Set_Input_User_Function(g_RegisterDoorInput, register_iuser_Register);
	g_RegisterDoorInput->mouseUsage = allInput;
	xrect_Set_Rect(&rect, REGISTER_LIST_LEFT, REGISTER_LIST_TOP, REGISTER_LIST_RIGHT, REGISTER_LIST_BOTTOM);
	g_RegisterPilotListInput = xinput_Alloc_Input(g_RegisterPageInput, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(g_RegisterPilotListInput, register_iupdate_Pilot_List);
	xinpattr_Set_Input_User_Function(g_RegisterPilotListInput, XwInput_UserNoOp);
	xinpattr_Set_Input_Draw_Function(g_RegisterPilotListInput, register_idraw_Pilot_List);
	g_RegisterPilotListInput->id = 0;
	g_RegisterListOverlayInput = xinput_Alloc_Input(g_RegisterPageInput, &rect, 0, 0);
	xinpattr_Set_Input_Draw_Function(g_RegisterListOverlayInput, register_idraw_ListMessage);
	xinpattr_Hide_Input(g_RegisterListOverlayInput);
	xrect_Set_Rect(&rect, REGISTER_PREVIOUS_LEFT, REGISTER_PREVIOUS_TOP, REGISTER_PREVIOUS_RIGHT,
				   REGISTER_PREVIOUS_BOTTOM);
	previousButton = xbtnpush_Alloc_Button(g_RegisterPageInput, &rect, 0, register_iuser_Pilot_Button, NULL,
										   REGISTER_PREVIOUS_PAGE_BUTTON);
	xinpattr_Set_Input_Draw_Function(&previousButton->header, XwRegister_DrawPilotButton);
	g_RegisterLeftButtonActor =
		xactor_Find_Actor(FOURCC_ANIM, g_RegisterResourceAndLabelText[REGISTER_RESOURCE_LEFT_BUTTON]);
	xrect_Set_Rect(&rect, REGISTER_NEXT_LEFT, REGISTER_NEXT_TOP, REGISTER_NEXT_RIGHT, REGISTER_NEXT_BOTTOM);
	nextButton = xbtnpush_Alloc_Button(g_RegisterPageInput, &rect, 0, register_iuser_Pilot_Button, NULL,
									   REGISTER_NEXT_PAGE_BUTTON);
	xinpattr_Set_Input_Draw_Function(&nextButton->header, XwRegister_DrawPilotButton);
	g_RegisterRightButtonActor =
		xactor_Find_Actor(FOURCC_ANIM, g_RegisterResourceAndLabelText[REGISTER_RESOURCE_RIGHT_BUTTON]);
	xrect_Set_Rect(&rect, REGISTER_PAGE_NUMBER_LEFT, REGISTER_PAGE_NUMBER_TOP, REGISTER_PAGE_NUMBER_RIGHT,
				   REGISTER_PAGE_NUMBER_BOTTOM);
	pageNumber = xinput_Alloc_Input(g_RegisterPageInput, &rect, 0, 0);
	xinpattr_Set_Input_Draw_Function(pageNumber, register_idraw_PageNumber);
	pageNumber->id = REGISTER_PAGE_NUMBER_INPUT;
	xrect_Set_Rect(&rect, REGISTER_NAME_LEFT, REGISTER_NAME_TOP, REGISTER_NAME_RIGHT, REGISTER_NAME_BOTTOM);
	g_RegisterPilotNameInput =
		register_Alloc_Input_Reg_String_Button(g_RegisterPageInput, &rect, 0, register_iuser_Pilot_Name,
											   g_sharedEmptyString, REGISTER_NAME_FILENAME_MODE, 0);
	xinpattr_Set_Input_Draw_Function(&g_RegisterPilotNameInput->input, XwRegister_DrawPilotName);
	xrect_Set_Rect(&rect, REGISTER_INFO_LEFT, REGISTER_INFO_TOP, REGISTER_INFO_RIGHT, REGISTER_INFO_BOTTOM);
	g_RegisterPilotInfoInput = xinput_Alloc_Input(g_RegisterPageInput, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(g_RegisterPilotInfoInput, XwInput_UpdateNoOp);
	xinpattr_Set_Input_User_Function(g_RegisterPilotInfoInput, register_iuser_Pilot_Info);
	xinpattr_Set_Input_Draw_Function(g_RegisterPilotInfoInput, register_idraw_Pilot_Info);
	g_RegisterPilotInfoInput->id = 0;
	xrect_Set_Rect(&rect, REGISTER_LOG_LEFT, REGISTER_LOG_TOP, REGISTER_LOG_RIGHT, REGISTER_LOG_BOTTOM);
	g_RegisterLogButton =
		&xbtnpush_Alloc_Button(g_RegisterPageInput, &rect, 0, register_iuser_Pilot_Button,
							   g_RegisterResourceAndLabelText[REGISTER_TEXT_LOG], REGISTER_LOG_BUTTON)
			 ->header;
	xinpattr_Set_Input_Draw_Function(g_RegisterLogButton, XwRegister_DrawPilotButton);
	xinpattr_Hide_Input(g_RegisterLogButton);
	g_RegisterLogButtonActor =
		xactor_Find_Actor(FOURCC_ANIM, g_RegisterResourceAndLabelText[REGISTER_RESOURCE_LOG_BUTTON]);
	xrect_Set_Rect(&rect, REGISTER_MERITS_LEFT, REGISTER_MERITS_TOP, REGISTER_MERITS_RIGHT,
				   REGISTER_MERITS_BOTTOM);
	g_RegisterMeritsButton =
		&xbtnpush_Alloc_Button(g_RegisterPageInput, &rect, 0, register_iuser_Pilot_Button,
							   g_RegisterResourceAndLabelText[REGISTER_TEXT_MERITS], REGISTER_MERITS_BUTTON)
			 ->header;
	xinpattr_Set_Input_Draw_Function(g_RegisterMeritsButton, XwRegister_DrawPilotButton);
	xinpattr_Hide_Input(g_RegisterMeritsButton);
	g_RegisterMeritsButtonActor =
		xactor_Find_Actor(FOURCC_ANIM, g_RegisterResourceAndLabelText[REGISTER_RESOURCE_MERITS_BUTTON]);
	xrect_Set_Rect(&rect, REGISTER_DELETE_LEFT, REGISTER_DELETE_TOP, REGISTER_DELETE_RIGHT,
				   REGISTER_DELETE_BOTTOM);
	g_RegisterDeleteButton =
		&xbtnpush_Alloc_Button(g_RegisterPageInput, &rect, 0, register_iuser_Pilot_Button,
							   g_RegisterResourceAndLabelText[REGISTER_TEXT_DELETE_PILOT],
							   REGISTER_DELETE_BUTTON)
			 ->header;
	xinpattr_Set_Input_Draw_Function(g_RegisterDeleteButton, XwRegister_DrawPilotButton);
	xinpattr_Hide_Input(g_RegisterDeleteButton);
	g_RegisterDeleteButtonActor =
		xactor_Find_Actor(FOURCC_ANIM, g_RegisterResourceAndLabelText[REGISTER_RESOURCE_DELETE_BUTTON]);
	xfiledir_Init_Directory(&g_RegisterDirectory, ".PLT", 0);
	register_OpenMusic(resource, g_RegisterFilm);
	register_LoadSoundEffects(resource, g_RegisterFilm);
	FrontendAudio_PlayFile("XwingCD\\music\\regbrief.wav", 1);
	xview_Set_View_Update_Function(register_end_View);
	xview_Disable_All_View_Erase();
	xio_Set_Key_Buttons();
#ifdef XW_MODERN
	XwRegister_RunView(resource);
#else
	j_xviewadd_Handle_View();
	xio_Clear_Key_Buttons();
	xview_Enable_All_View_Erase();
	register_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xparagrp_Free_Paragraph(g_RegisterTourParagraph);
	xview_Clear_View_Update_Function();
	xfiledir_Free_Directory(&g_RegisterDirectory);
	if ((uint16_t)xcursor_Is_Cursor_Visible())
		xcursor_Hide_Cursor();
	if (g_RegisterFastPilotHandle != LANDRU_NULL_HANDLE)
		xmemhdl_Free_Handle(g_RegisterFastPilotHandle);
	LandruDisplay_ForwardLegacyNoOp(0);
	xres_Close_Resource(resource);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x45C6A0
void register_end_View(int time) {
	char typedName[REGISTER_STRING_WORK_CAPACITY];
	int16_t key;
	if (time == 0) {
		if ((uint16_t)xcursor_Is_Cursor_Visible() == 0)
			xcursor_Show_Cursor();
		if (g_RegisterProtectionEnabled != 0 && shellext_Get_Cur_Scene() == XW_SCENE_REGISTER_INITIAL) {
			register_Do_Protect_Dialog(XwRegister_CompleteProtection, NULL);
			return;
		}
	}
	if (time == 0 || time == XW_REGISTER_VIEW_RESUME_PROTECTION) {
		xfiledir_Read_Directory(&g_RegisterDirectory);
		g_RegisterLoadedPilotCount = g_RegisterDirectory.count;
		xcursor_Set_Cursor(REGISTER_CURSOR_BUSY);
		register_Build_Fast_Pilot_Record();
		xcursor_Set_Cursor(REGISTER_CURSOR_NORMAL);
		g_RegisterPageCount =
			(g_RegisterPilotCount + REGISTER_PILOT_VISIBLE_ROWS - 1) / REGISTER_PILOT_VISIBLE_ROWS;
		if (g_RegisterPageCount == 0)
			g_RegisterPageCount = 1;
		g_RegisterCurrentPage = 0;
		register_Set_Your_Reg_Pilot();
	}
	register_Get_Reg_String_Button_Name(g_RegisterPilotNameInput, typedName);
	register_xuser_Pilot_Name(typedName);
	if (typedName[0] != 0) {
		if ((uint16_t)xinpattr_Is_Input_Visible(g_RegisterLogButton) == 0) {
			xinpattr_Show_Input(g_RegisterLogButton);
			xinpattr_Show_Input(g_RegisterMeritsButton);
			xview_Refresh_View();
		}
		if ((uint16_t)xinpattr_Is_Input_Visible(g_RegisterDeleteButton) != 0) {
			if (g_RegisterActivePilot == REGISTER_PILOT_SLOT_NONE) {
				xinpattr_Hide_Input(g_RegisterDeleteButton);
				xview_Refresh_View();
			}
		} else if (g_RegisterActivePilot != REGISTER_PILOT_SLOT_NONE) {
			xinpattr_Show_Input(g_RegisterDeleteButton);
			g_RegisterDeleteLabelState = REGISTER_DELETE_LABEL_UNKNOWN;
			xview_Refresh_View();
		}
		if ((uint16_t)xinpattr_Is_Input_Visible(g_RegisterDeleteButton) != 0) {
			register_Index_To_Pilot_Record(g_RegisterActivePilot, &g_RegisterShellPilot);
			if (g_RegisterShellPilot.lost_status != 0) {
				if (g_RegisterDeleteLabelState != REGISTER_DELETE_LABEL_MODIFY) {
					xbtnpush_Set_Button_Name((PushButton*)g_RegisterDeleteButton,
											 g_RegisterResourceAndLabelText[REGISTER_TEXT_MODIFY_PILOT]);
					g_RegisterDeleteLabelState = REGISTER_DELETE_LABEL_MODIFY;
				}
			} else if (g_RegisterDeleteLabelState != REGISTER_DELETE_LABEL_DELETE) {
				xbtnpush_Set_Button_Name((PushButton*)g_RegisterDeleteButton,
										 g_RegisterResourceAndLabelText[REGISTER_TEXT_DELETE_PILOT]);
				g_RegisterDeleteLabelState = REGISTER_DELETE_LABEL_DELETE;
			}
			xview_Refresh_View();
		}
	} else if ((uint16_t)xinpattr_Is_Input_Visible(g_RegisterLogButton) != 0) {
		xinpattr_Hide_Input(g_RegisterLogButton);
		xinpattr_Hide_Input(g_RegisterMeritsButton);
		xinpattr_Hide_Input(g_RegisterDeleteButton);
		xview_Refresh_View();
	}
	xinpattr_Refresh_Input(g_RegisterPageInput);
	if (time == REGISTER_INITIAL_SPEECH_FRAME && shellext_Get_Cur_Scene() == XW_SCENE_REGISTER_INITIAL)
		register_PlaySpeech(REGISTER_SPEECH_FIRST, time);
	key = xio_Get_Free_Key();
	if (key != 0 &&
		shellext_MoveGridFocus(&g_RegisterNavigationIndex, g_RegisterNavigationX, g_RegisterNavigationY,
							   REGISTER_NAVIGATION_ROWS, REGISTER_NAVIGATION_COLUMNS, key) != 0)
		xio_Set_Mouse_Position(g_RegisterNavigationX[g_RegisterNavigationIndex],
							   g_RegisterNavigationY[g_RegisterNavigationIndex]);
}

// FUNCTION: XW 0x45C970
int16_t register_draw_Register_Back(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
									int16_t refresh) {
	if (refresh != 0) {
		Rect viewRect;
		xcanvas_Get_Screen_Bounds(&viewRect);
		xpaint_Paint_Clipped_Rect(&viewRect, REGISTER_BACKGROUND_COLOR);
	}
	return xactdelt_Draw_Delta_Actor(actor, frame, clip, x, y, refresh);
}

// FUNCTION: XW 0x45C9C0
void register_user_Robot(Actor* actor, int time) {
	if (time == 0 || actor->var1 <= 0) {
		if (g_RegisterRobotState < actor->arraySize - 1)
			++g_RegisterRobotState;
		else
			g_RegisterRobotState = 0;
	}
	--actor->var1;
}

// FUNCTION: XW 0x45CA00
int16_t register_draw_Robot(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	if (refresh) {
		xactor_Set_Actor_State(actor, g_RegisterRobotState, 0);
		return xactanim_Draw_Anim_Actor(actor, frame, clip, x, y, refresh);
	}
	return 0;
}

// FUNCTION: XW 0x45CA50
void register_user_Door(Actor* door, int time) {
	int16_t needsRefresh = 1;
	(void)time;
	if (door->var1 != 0) {
		if (door->state == 0) {
			register_PlaySoundCue(REGISTER_SOUND_DOOR_OPEN);
		}
		if (door->state < door->arraySize - 1) {
			xactor_Set_Actor_State(door, door->state + 1, 0);
		} else {
			needsRefresh = 0;
		}
		door->var1 = 0;
	} else {
		if (door->state == 1) {
			register_PlaySoundCue(REGISTER_SOUND_DOOR_CLOSE);
		}
		if (door->state != 0) {
			xactor_Set_Actor_State(door, door->state - 1, 0);
		} else {
			needsRefresh = 0;
		}
	}
	if (needsRefresh != 0) {
		xactor_Refresh_Actor(door);
	}
}

// FUNCTION: XW 0x45CAE0
void register_user_Troop(Actor* troop, int time) {
	if (troop->var1 != 0) {
		if (troop->var2 == REGISTER_TROOP_SOUND_FRAME) {
			register_PlaySoundCue(REGISTER_SOUND_GUARD);
		}
		if (troop->var2 < troop->arraySize - 1 && (time & 1) != 0) {
			++troop->var2;
		}
		troop->var1 = 0;
	} else if (troop->var2 > 0) {
		if ((time & 1) != 0) {
			--troop->var2;
		}
	} else {
		troop->var2 = 0;
	}
}

// FUNCTION: XW 0x45CB40
int16_t register_draw_Troop(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	int16_t result;
	result = 0;
	if (refresh) {
		xactor_Set_Actor_State(actor, actor->var2, 0);
		result = xactanim_Draw_Anim_Actor(actor, frame, clip, x, y, refresh);
	}
	return result;
}

// FUNCTION: XW 0x45CB80
int16_t register_iupdate_Register(Input* input, Rect* frame, Rect* clip, int16_t phase, int leftEvent,
								  int rightEvent, int16_t x, int16_t y) {
	char pilotName[REGISTER_PILOT_PATH_CAPACITY];
	REGISTER_FastPilotRecord pilotRecord;
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (phase != 0) {
		return 0;
	}
	register_Get_Reg_String_Button_Name(g_RegisterPilotNameInput, pilotName);
	if (strlen(pilotName) == 0) {
		input->var1 = REGISTER_DOOR_EMPTY_NAME;
		g_RegisterSoldierActor->var1 = 1;
	} else {
		register_Index_To_Pilot_Record(g_RegisterActivePilot, &pilotRecord);
		if (g_RegisterActivePilot != REGISTER_PILOT_SLOT_NONE &&
			pilotRecord.lost_status != REGISTER_PILOT_AVAILABLE) {
			if (pilotRecord.lost_status == REGISTER_PILOT_LOST_ALTERNATE) {
				input->var1 = REGISTER_DOOR_LOST_PILOT_ALTERNATE;
			} else {
				input->var1 = REGISTER_DOOR_LOST_PILOT;
			}
			g_RegisterSoldierActor->var1 = 1;
		} else {
			if (leftEvent == REGISTER_MOUSE_RELEASE || rightEvent == REGISTER_MOUSE_RELEASE) {
				input->var1 = REGISTER_DOOR_ACCEPT;
				input->var2 = XW_SCENE_CONCOURSE;
			} else {
				input->var1 = REGISTER_DOOR_HOVER;
			}
			g_RegisterDoorActor->var1 = 1;
		}
	}
	return 1;
}

// FUNCTION: XW 0x45CC90
void register_iuser_Register(Input* input, int context) {
#ifndef XW_MODERN
	char pilotFileName[REGISTER_PILOT_PATH_CAPACITY];
#endif
	switch (input->var1) {
		case REGISTER_DOOR_IDLE:
			if ((int16_t)xinpattr_Is_Input_Visible(g_RegisterListOverlayInput) != 0) {
				xinpattr_Show_Input(g_RegisterPilotListInput);
				xinpattr_Hide_Input(g_RegisterListOverlayInput);
				xinpattr_Refresh_Input(g_RegisterPilotListInput);
			}
			break;
		case REGISTER_DOOR_ACCEPT:
#ifdef XW_MODERN
			XwRegister_ScheduleAcceptPilot(input, context);
#else
			register_PlaySpeech(REGISTER_SPEECH_WAIT, context);
			xerror_Set_Landru_Exit(input->var2);
			if (g_RegisterActivePilot == REGISTER_PILOT_SLOT_NONE) {
				register_Get_Reg_String_Button_Name(g_RegisterPilotNameInput, pilotFileName);
				strcpy(g_RegisterShellPilot.name, pilotFileName);
				g_RegisterShellPilot.field_18 = 0;
				g_RegisterShellPilot.deleted = 0;
				g_RegisterShellPilot.lost_status = 0;
				g_RegisterShellPilot.rank = 0;
				g_RegisterShellPilot.current_tour = 0;
				g_RegisterShellPilot.field_1D = 0;
				g_RegisterShellPilot.score = 0;
				strcat(pilotFileName, ".PLT");
				shipext_Revive_Pilot(pilotFileName, SHIPEXT_CREATE_PILOT);
			} else {
				register_Index_To_Pilot_Record(g_RegisterActivePilot, &g_RegisterShellPilot);
			}
			shipext_ResetMissionSelections();
#endif
			break;
		case REGISTER_DOOR_HOVER:
			if ((int16_t)xinpattr_Is_Input_Visible(g_RegisterPilotListInput) != 0 ||
				g_RegisterListOverlayInput->var1 != REGISTER_MESSAGE_ENTER) {
				xinpattr_Hide_Input(g_RegisterPilotListInput);
				xinpattr_Show_Input(g_RegisterListOverlayInput);
				xinpattr_Refresh_Input(g_RegisterListOverlayInput);
				g_RegisterListOverlayInput->var1 = REGISTER_MESSAGE_ENTER;
			}
			input->var1 = REGISTER_DOOR_IDLE;
			break;
		case REGISTER_DOOR_EMPTY_NAME:
		case REGISTER_DOOR_LOST_PILOT:
		case REGISTER_DOOR_LOST_PILOT_ALTERNATE:
			if ((int16_t)xinpattr_Is_Input_Visible(g_RegisterPilotListInput) != 0 ||
				g_RegisterListOverlayInput->var1 == REGISTER_MESSAGE_ENTER) {
				register_PlaySpeech(REGISTER_SPEECH_REGISTER, context);
				xinpattr_Hide_Input(g_RegisterPilotListInput);
				xinpattr_Show_Input(g_RegisterListOverlayInput);
				xinpattr_Refresh_Input(g_RegisterListOverlayInput);
				g_RegisterListOverlayInput->var1 = input->var1 - REGISTER_DOOR_HOVER;
			}
			input->var1 = REGISTER_DOOR_IDLE;
			break;
	}
}

// FUNCTION: XW 0x45CEF0
int16_t register_iupdate_Pilot_List(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
									int rightEvent, int16_t x, int16_t y) {
	char pilotName[REGISTER_PILOT_PATH_CAPACITY];
	int mouseEvent;
	int16_t rowIndex;
	int16_t pilotSlot;
	(void)frame;
	(void)clip;
	(void)x;
	if (key != 0)
		return 0;
	mouseEvent = leftEvent;
	if (mouseEvent == 0)
		mouseEvent = rightEvent;
	if (mouseEvent == REGISTER_MOUSE_RELEASE) {
		rowIndex = y / REGISTER_PILOT_ROW_HEIGHT;
		if (rowIndex < 0)
			rowIndex = 0;
		if (rowIndex >= REGISTER_PILOT_VISIBLE_ROWS)
			rowIndex = REGISTER_PILOT_VISIBLE_ROWS - 1;
		if (rowIndex >= 0 && rowIndex < REGISTER_PILOT_VISIBLE_ROWS &&
			register_Index_To_Pilot(rowIndex + g_RegisterPilotOffset, &pilotSlot) != 0 &&
			register_Find_Reg_Dir_Name(&g_RegisterDirectory, pilotName, pilotSlot) != 0) {
			register_Set_Reg_String_Button_Name(g_RegisterPilotNameInput, pilotName);
			xinpattr_Refresh_Input(input);
			g_RegisterActivePilot = g_RegisterPilotOffset + rowIndex;
			g_RegisterPilotInfoInput->var1 = 1;
		}
	}
	return 1;
}

// FUNCTION: XW 0x45CFD0
void register_idraw_Pilot_List(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	Rect rowRect;
	char pilotName[REGISTER_PILOT_LIST_TEXT_CAPACITY];
	REGISTER_FastPilotRecord pilotRecord;
	(void)input;
	if (refresh != 0) {
		int16_t rowIndex;
		int16_t pilotSlot;
		xactdelt_Draw_Delta_Actor(g_RegisterRightDeskActor, frame, clip, 0, 0, refresh);
		xrect_Copy_Rect(&rowRect, frame);
		rowRect.right -= REGISTER_PILOT_LIST_CLIP_INSET;
		xcanvas_Set_Drawing_Canvas_Clip(&rowRect);
		rowRect.right += REGISTER_PILOT_LIST_CLIP_INSET;
		rowRect.top += REGISTER_PILOT_LIST_TEXT_Y;
		rowRect.bottom = rowRect.top + REGISTER_PILOT_ROW_HEIGHT;
		xfont_Enable_FontID_Shadow(REGISTER_PILOT_LIST_FONT);
		for (rowIndex = 0; rowIndex < REGISTER_PILOT_VISIBLE_ROWS; ++rowIndex) {
			if (register_Index_To_Pilot(rowIndex + g_RegisterPilotOffset, &pilotSlot) != 0) {
				if (register_Find_Reg_Dir_Name(&g_RegisterDirectory, pilotName, pilotSlot) != 0) {
					int16_t textColor;
					register_Index_To_Pilot_Record(rowIndex + g_RegisterPilotOffset, &pilotRecord);
					textColor = REGISTER_PILOT_LIST_NORMAL_COLOR;
					if (pilotRecord.lost_status != 0)
						textColor = REGISTER_PILOT_LIST_LOST_COLOR;
					if (g_RegisterActivePilot == rowIndex + g_RegisterPilotOffset)
						textColor = REGISTER_PILOT_LIST_ACTIVE_COLOR;
					xfont_Print_Clipped_Text(pilotName, rowRect.left + REGISTER_PILOT_LIST_TEXT_X,
											 rowRect.top, REGISTER_PILOT_LIST_FONT, textColor);
				}
				xrect_Offset_Rect(&rowRect, 0, REGISTER_PILOT_ROW_HEIGHT);
			}
		}
		xfont_Disable_FontID_Shadow(REGISTER_PILOT_LIST_FONT);
	}
}

// FUNCTION: XW 0x45D110
void register_idraw_ListMessage(Input* input, Rect* frame, Rect* unusedClip, int16_t refresh) {
	Rect textRect;
	(void)unusedClip;
	if (refresh != 0) {
		int16_t firstLineIndex;
		int16_t secondLineIndex;
		int16_t textColor;
		const char* line;
		xrect_Copy_Rect(&textRect, frame);
		switch (input->var1) {
			case REGISTER_MESSAGE_ENTER:
				firstLineIndex = REGISTER_TEXT_ENTER;
				secondLineIndex = REGISTER_TEXT_SPACEPORT;
				textColor = REGISTER_MESSAGE_NORMAL_COLOR;
				break;
			case REGISTER_MESSAGE_REGISTER:
				firstLineIndex = REGISTER_TEXT_YOU_MUST;
				secondLineIndex = REGISTER_TEXT_REGISTER;
				textColor = REGISTER_MESSAGE_WARNING_COLOR;
				break;
			case REGISTER_MESSAGE_CAPTURED:
				firstLineIndex = REGISTER_TEXT_THIS_PILOT_IS;
				secondLineIndex = REGISTER_TEXT_CAPTURED;
				textColor = REGISTER_MESSAGE_WARNING_COLOR;
				break;
			case REGISTER_MESSAGE_DEAD:
				firstLineIndex = REGISTER_TEXT_THIS_PILOT_IS;
				secondLineIndex = REGISTER_TEXT_DEAD;
				textColor = REGISTER_MESSAGE_WARNING_COLOR;
				break;
			default:
				textColor = refresh;
				secondLineIndex = refresh;
				firstLineIndex = refresh;
				break;
		}
		xrect_Offset_Rect(&textRect, REGISTER_MESSAGE_SHADOW_OFFSET,
						  REGISTER_MESSAGE_FIRST_LINE_Y + REGISTER_MESSAGE_SHADOW_OFFSET);
		line = g_RegisterResourceAndLabelText[firstLineIndex];
		xfont_Print_Centered_Text(line, &textRect, REGISTER_MESSAGE_FONT, REGISTER_MESSAGE_SHADOW_COLOR);
		xrect_Offset_Rect(&textRect, -REGISTER_MESSAGE_SHADOW_OFFSET, -REGISTER_MESSAGE_SHADOW_OFFSET);
		xfont_Print_Centered_Text(line, &textRect, REGISTER_MESSAGE_FONT, textColor);
		xrect_Offset_Rect(&textRect, REGISTER_MESSAGE_SHADOW_OFFSET,
						  REGISTER_MESSAGE_LINE_SPACING + REGISTER_MESSAGE_SHADOW_OFFSET);
		line = g_RegisterResourceAndLabelText[secondLineIndex];
		xfont_Print_Centered_Text(line, &textRect, REGISTER_MESSAGE_FONT, REGISTER_MESSAGE_SHADOW_COLOR);
		xrect_Offset_Rect(&textRect, -REGISTER_MESSAGE_SHADOW_OFFSET, -REGISTER_MESSAGE_SHADOW_OFFSET);
		xfont_Print_Centered_Text(line, &textRect, REGISTER_MESSAGE_FONT, textColor);
	}
}

// FUNCTION: XW 0x45D260
void register_iuser_Pilot_Button(Input* input, int unusedContext) {
	char pilotFilename[REGISTER_PILOT_BUTTON_PATH_CAPACITY];
	int16_t id;
	(void)unusedContext;
	if (xinpattr_Get_Input_Selected(input) == 0)
		return;
	id = input->id;
	if (id < REGISTER_PILOT_BUTTON_LOG) {
		if (id == REGISTER_PILOT_BUTTON_LEFT) {
			if (g_RegisterCurrentPage > 0) {
				--g_RegisterCurrentPage;
				xinput_Refresh_System_Inputs();
			} else {
				g_RegisterCurrentPage = g_RegisterPageCount - 1;
			}
		} else if (g_RegisterCurrentPage < g_RegisterPageCount - 1) {
			++g_RegisterCurrentPage;
		} else {
			g_RegisterCurrentPage = 0;
		}
		if (g_RegisterPageCount > 1) {
			g_RegisterPilotOffset = REGISTER_PILOT_VISIBLE_ROWS * g_RegisterCurrentPage;
			xinpattr_Refresh_Input(g_RegisterPageInput);
		}
	} else if (id == REGISTER_PILOT_BUTTON_DELETE) {
		register_Do_Delete_Dialog(XwRegister_CompletePilotDeletion, NULL);
	} else {
		if (id == REGISTER_PILOT_BUTTON_LOG)
			xerror_Set_Landru_Exit(XW_SCENE_PILOT_LOG_FROM_REGISTER);
		else
			xerror_Set_Landru_Exit(XW_SCENE_UNIFORM_FROM_REGISTER);
		if (g_RegisterActivePilot == REGISTER_PILOT_SLOT_NONE) {
			register_Get_Reg_String_Button_Name(g_RegisterPilotNameInput, pilotFilename);
			strcpy(g_RegisterShellPilot.name, pilotFilename);
			g_RegisterShellPilot.field_18 = 0;
			g_RegisterShellPilot.deleted = 0;
			g_RegisterShellPilot.lost_status = 0;
			g_RegisterShellPilot.rank = 0;
			g_RegisterShellPilot.current_tour = 0;
			g_RegisterShellPilot.field_1D = 0;
			g_RegisterShellPilot.score = 0;
			strcat(pilotFilename, ".PLT");
			shipext_Revive_Pilot(pilotFilename, SHIPEXT_CREATE_PILOT);
		} else {
			register_Index_To_Pilot_Record(g_RegisterActivePilot, &g_RegisterShellPilot);
		}
	}
}

// FUNCTION: XW 0x45D4A0
void register_idraw_Pilot_Button(PushButton* button, Rect* frame, Rect* clip, int16_t refresh) {
	if (refresh != 0) {
		Actor* buttonActor;
		switch (button->header.id) {
			case REGISTER_PILOT_BUTTON_LEFT:
				buttonActor = g_RegisterLeftButtonActor;
				break;
			case REGISTER_PILOT_BUTTON_RIGHT:
				buttonActor = g_RegisterRightButtonActor;
				break;
			case REGISTER_PILOT_BUTTON_LOG:
				buttonActor = g_RegisterLogButtonActor;
				break;
			case REGISTER_PILOT_BUTTON_MERITS:
				buttonActor = g_RegisterMeritsButtonActor;
				break;
			case REGISTER_PILOT_BUTTON_DELETE:
				buttonActor = g_RegisterDeleteButtonActor;
				break;
			default:
#ifdef XW_MODERN
				return;
#else
				/* The original caller supplies one of the five button IDs. */
				break;
#endif
		}
		xactor_Set_Actor_State(buttonActor, button->pressed, 0);
		xactanim_Draw_Anim_Actor(buttonActor, frame, clip, 0, 0, refresh);
		if (button->header.id >= REGISTER_PILOT_BUTTON_LOG)
			xfont_Print_Centered_Text(button->labels, frame, REGISTER_PILOT_BUTTON_FONT,
									  REGISTER_PILOT_BUTTON_TEXT_COLOR);
	}
}

// FUNCTION: XW 0x45D550
void register_idraw_PageNumber(Input* unusedInput, Rect* frame, Rect* unusedClip, int16_t refresh) {
	char pageText[REGISTER_PAGE_TEXT_CAPACITY];
	(void)unusedInput;
	(void)unusedClip;
	if (refresh != 0) {
		xpaint_Paint_Clipped_Rect(frame, REGISTER_PAGE_BACKGROUND_COLOR);
		sprintf(pageText, "Page %d/%d", g_RegisterCurrentPage + 1, g_RegisterPageCount);
		xfont_Print_Centered_Text(pageText, frame, REGISTER_PAGE_FONT, REGISTER_PAGE_TEXT_COLOR);
	}
}

// FUNCTION: XW 0x45D5B0
void register_iuser_Pilot_Name(Input* input, int unusedContext) {
	REGISTER_RegStringButton* button = (REGISTER_RegStringButton*)input;
	(void)unusedContext;
	if (xinpattr_Get_Input_Selected(input) != 0) {
		register_xuser_Pilot_Name(button->name);
		if (strlen(button->name) != 0) {
			g_RegisterPilotInfoInput->var1 = 1;
		} else {
			g_RegisterPilotInfoInput->var1 = 0;
			g_RegisterPilotInfoInput->var2 = 0;
			xinpattr_Refresh_Input(g_RegisterPilotInfoInput);
		}
	}
}

// FUNCTION: XW 0x45D620
void register_xuser_Pilot_Name(const char* searchName) {
	int16_t logicalIndex;
	int16_t matchedIndex = REGISTER_PILOT_SLOT_NONE;
	int16_t pilotSlot;
	char pilotName[REGISTER_PILOT_PATH_CAPACITY];
	for (logicalIndex = 0; logicalIndex < g_RegisterPilotCount && matchedIndex == REGISTER_PILOT_SLOT_NONE;
		 ++logicalIndex) {
		if (register_Index_To_Pilot(logicalIndex, &pilotSlot) != 0 &&
			register_Find_Reg_Dir_Name(&g_RegisterDirectory, pilotName, pilotSlot) != 0 &&
			strcmp(pilotName, searchName) == 0)
			matchedIndex = logicalIndex;
	}
	if (matchedIndex != REGISTER_PILOT_SLOT_NONE) {
		if (g_RegisterActivePilot != matchedIndex) {
			int matchingPage;
			xinpattr_Refresh_Input(g_RegisterPilotListInput);
			g_RegisterActivePilot = matchedIndex;
			matchingPage = matchedIndex / REGISTER_PILOT_VISIBLE_ROWS;
			if (matchingPage != g_RegisterCurrentPage) {
				g_RegisterCurrentPage = matchedIndex / REGISTER_PILOT_VISIBLE_ROWS;
				g_RegisterPilotOffset = REGISTER_PILOT_VISIBLE_ROWS * g_RegisterCurrentPage;
				xinpattr_Refresh_Input(g_RegisterPageInput);
			}
		}
	} else if (g_RegisterActivePilot != REGISTER_PILOT_SLOT_NONE) {
		xinpattr_Refresh_Input(g_RegisterPilotListInput);
		g_RegisterActivePilot = REGISTER_PILOT_SLOT_NONE;
	}
}

// FUNCTION: XW 0x45D750
void register_idraw_Pilot_Name(struct REGISTER_RegStringButton* button, Rect* frame, Rect* clip,
							   int16_t refresh) {
	char* name = button->name;
	int16_t textWidth = xfont_Get_String_Width_0(REGISTER_NAME_FONT, name);
	(void)clip;
	if (refresh != 0) {
		xpaint_Paint_Clipped_Rect(frame, REGISTER_BACKGROUND_COLOR);
		xfont_Print_Clipped_Text(name, frame->left + REGISTER_NAME_TEXT_X, frame->top + REGISTER_NAME_TEXT_Y,
								 REGISTER_NAME_FONT, REGISTER_NAME_TEXT_COLOR);
	}
	if ((uint16_t)xinpattr_Is_Input_Active(&button->input) != 0 && strlen(name) < REGISTER_NAME_INPUT_LIMIT) {
		if ((uint16_t)xio_Blink() != 0) {
			xpaint_Horiz_Clipped_Line(textWidth + frame->left + REGISTER_NAME_CARET_X,
									  frame->top + REGISTER_NAME_CARET_LIT_Y, REGISTER_NAME_CARET_WIDTH,
									  REGISTER_NAME_TEXT_COLOR);
		} else {
			xpaint_Horiz_Clipped_Line(textWidth + frame->left + REGISTER_NAME_CARET_X,
									  frame->top + REGISTER_NAME_CARET_ERASE_Y, REGISTER_NAME_CARET_WIDTH,
									  REGISTER_BACKGROUND_COLOR);
		}
	} else {
		xpaint_Horiz_Clipped_Line(textWidth + frame->left + REGISTER_NAME_CARET_X,
								  frame->top + REGISTER_NAME_CARET_ERASE_Y, REGISTER_NAME_CARET_WIDTH,
								  REGISTER_BACKGROUND_COLOR);
	}
}

// FUNCTION: XW 0x45D830
void register_iuser_Pilot_Info(Input* input, int unusedContext) {
	(void)unusedContext;
	if (input->var1 != 0) {
		input->var2++;
	}
}

// FUNCTION: XW 0x45D840
void register_idraw_Pilot_Info(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	unsigned int drawMode = REGISTER_INFO_NONE;
	int16_t outSlot[2];
	char tourName[REGISTER_INFO_SHORT_TEXT_SIZE];
	Rect rect;
	char rankName[REGISTER_INFO_SHORT_TEXT_SIZE];
	char scoreText[REGISTER_INFO_SHORT_TEXT_SIZE];
	char pilotName[REGISTER_DIRECTORY_NAME_CAPACITY];
	char pilotFileName[REGISTER_DIRECTORY_NAME_CAPACITY];
	if (refresh != 0) {
		xactdelt_Draw_Delta_Actor(g_RegisterLeftDeskActor, frame, clip, 0, 0, refresh);
		drawMode = REGISTER_INFO_CACHED;
	}
	if (input->var1 != 0) {
		if (input->var1 == REGISTER_INFO_START) {
			input->var1 = REGISTER_INFO_RUNNING;
			input->var2 = 0;
			drawMode = REGISTER_INFO_NONE;
		} else {
			xactdelt_Draw_Delta_Actor(g_RegisterLeftDeskActor, frame, clip, 0, 0, 1);
			if (input->var2 < REGISTER_INFO_DELAY) {
				if (input->var2 == REGISTER_INFO_FLASH_BIT)
					register_PlaySoundCue(REGISTER_SOUND_PILOT_INFO);
				drawMode = ((unsigned int)input->var2 & REGISTER_INFO_FLASH_BIT) >> 1;
			} else {
				drawMode = REGISTER_INFO_LOAD;
				input->var1 = 0;
				input->var2 = 0;
			}
		}
	}
	if ((uint16_t)drawMode == REGISTER_INFO_LOAD || (uint16_t)drawMode == REGISTER_INFO_CACHED) {
		register_Get_Reg_String_Button_Name(g_RegisterPilotNameInput, pilotName);
		if ((uint16_t)drawMode == REGISTER_INFO_LOAD)
			memset(&g_RegisterPilotData, 0, sizeof(g_RegisterPilotData));
		if (g_RegisterActivePilot != REGISTER_PILOT_SLOT_NONE && (uint16_t)drawMode == REGISTER_INFO_LOAD &&
			register_Index_To_Pilot(g_RegisterActivePilot, outSlot) != 0 &&
			register_Find_Reg_Dir_Name(&g_RegisterDirectory, pilotFileName, outSlot[0]) != 0) {
			strcat(pilotFileName, ".PLT");
			if (shipext_Load_Pilot(pilotFileName, 0) == 0) {
				pilotName[0] = 0;
				g_RegisterActivePilot = REGISTER_PILOT_SLOT_NONE;
				register_Set_Reg_String_Button_Name(g_RegisterPilotNameInput, pilotName);
				xinpattr_Refresh_Input(&g_RegisterPilotNameInput->input);
			}
		}
		if (strlen(pilotName) != 0) {
			strcpy(rankName, g_RegisterRankNames[g_RegisterPilotData.rank]);
			sprintf(scoreText, "%lu", (unsigned long)g_RegisterPilotData.score);
			if (g_RegisterPilotData.tour_status[g_RegisterPilotData.current_tour] == 1)
				xparagrp_Get_Paragraph_String(g_RegisterTourParagraph, tourName, 0,
											  g_RegisterPilotData.current_tour);
			else
				tourName[0] = 0;
			xfont_Enable_FontID_Shadow(REGISTER_INFO_VALUE_FONT);
			xfont_Enable_FontID_Shadow(REGISTER_INFO_LABEL_FONT);
			xfont_Print_Clipped_Text("Name", frame->left + REGISTER_INFO_TEXT_X,
									 frame->top + REGISTER_INFO_NAME_Y, REGISTER_INFO_LABEL_FONT,
									 REGISTER_INFO_NAME_COLOR);
			xpaint_Horiz_Clipped_Line(
				frame->left + REGISTER_INFO_RULE_X, frame->top + REGISTER_INFO_NAME_RULE_Y,
				frame->right - frame->left - REGISTER_INFO_RULE_MARGIN, REGISTER_INFO_RULE_COLOR);
			if (g_RegisterPilotData.lost_status != 0) {
				int16_t labelWidth = xfont_Get_String_Width_0(REGISTER_INFO_LABEL_FONT, "Name");
				xfont_Print_Clipped_Text(g_RegisterLostStatusLabels[g_RegisterPilotData.lost_status],
										 frame->left + labelWidth + REGISTER_INFO_STATUS_X,
										 frame->top + REGISTER_INFO_NAME_Y, REGISTER_INFO_LABEL_FONT,
										 REGISTER_INFO_STATUS_COLOR);
			}
			xfont_Print_Clipped_Text("Rank", frame->left + REGISTER_INFO_TEXT_X,
									 frame->top + REGISTER_INFO_RANK_Y, REGISTER_INFO_LABEL_FONT,
									 REGISTER_INFO_RANK_COLOR);
			xpaint_Horiz_Clipped_Line(
				frame->left + REGISTER_INFO_RULE_X, frame->top + REGISTER_INFO_RANK_RULE_Y,
				frame->right - frame->left - REGISTER_INFO_RULE_MARGIN, REGISTER_INFO_RULE_COLOR);
			xfont_Print_Clipped_Text("TOD Score", frame->left + REGISTER_INFO_TEXT_X,
									 frame->top + REGISTER_INFO_SCORE_Y, REGISTER_INFO_LABEL_FONT,
									 REGISTER_INFO_SCORE_COLOR);
			xpaint_Horiz_Clipped_Line(
				frame->left + REGISTER_INFO_RULE_X, frame->top + REGISTER_INFO_SCORE_RULE_Y,
				frame->right - frame->left - REGISTER_INFO_RULE_MARGIN, REGISTER_INFO_RULE_COLOR);
			if (tourName[0] != 0) {
				xfont_Print_Clipped_Text(g_RegisterLostStatusLabels[0], frame->left + REGISTER_INFO_TEXT_X,
										 frame->top + REGISTER_INFO_TOUR_Y, REGISTER_INFO_LABEL_FONT,
										 REGISTER_INFO_TOUR_COLOR);
				xpaint_Horiz_Clipped_Line(
					frame->left + REGISTER_INFO_RULE_X, frame->top + REGISTER_INFO_TOUR_RULE_Y,
					frame->right - frame->left - REGISTER_INFO_RULE_MARGIN, REGISTER_INFO_RULE_COLOR);
			}
			xfont_Print_Clipped_Text(pilotName, frame->left + REGISTER_INFO_TEXT_X,
									 frame->top + REGISTER_INFO_NAME_VALUE_Y, REGISTER_INFO_VALUE_FONT,
									 REGISTER_INFO_NAME_VALUE_COLOR);
			xfont_Print_Clipped_Text(rankName, frame->left + REGISTER_INFO_TEXT_X,
									 frame->top + REGISTER_INFO_RANK_VALUE_Y, REGISTER_INFO_VALUE_FONT,
									 REGISTER_INFO_RANK_VALUE_COLOR);
			xfont_Print_Clipped_Text(scoreText, frame->left + REGISTER_INFO_TEXT_X,
									 frame->top + REGISTER_INFO_SCORE_VALUE_Y, REGISTER_INFO_VALUE_FONT,
									 REGISTER_INFO_SCORE_VALUE_COLOR);
			if (tourName[0] != 0) {
				xrect_Set_Rect(&rect, frame->left, frame->top + REGISTER_INFO_TOUR_VALUE_Y, frame->right,
							   frame->top + REGISTER_INFO_TOUR_BOTTOM);
				xfont_Print_Centered_Text(tourName, &rect, REGISTER_INFO_VALUE_FONT,
										  REGISTER_INFO_TOUR_VALUE_COLOR);
			}
			xfont_Disable_FontID_Shadow(REGISTER_INFO_VALUE_FONT);
			xfont_Disable_FontID_Shadow(REGISTER_INFO_LABEL_FONT);
		}
	}
	if ((uint16_t)drawMode == REGISTER_INFO_ACCESS) {
		register_Get_Reg_String_Button_Name(g_RegisterPilotNameInput, pilotName);
		xrect_Offset_Rect(frame, 1, REGISTER_INFO_ACCESS_Y);
		xfont_Print_Centered_Text(g_RegisterResourceAndLabelText[REGISTER_INFO_ACCESS_TEXT], frame,
								  REGISTER_INFO_LABEL_FONT, REGISTER_INFO_ACCESS_SHADOW_COLOR);
		xrect_Offset_Rect(frame, -1, -1);
		xfont_Print_Centered_Text(g_RegisterResourceAndLabelText[REGISTER_INFO_ACCESS_TEXT], frame,
								  REGISTER_INFO_LABEL_FONT, REGISTER_INFO_ACCESS_COLOR);
		xrect_Offset_Rect(frame, 1, REGISTER_INFO_ACCESS_NAME_Y);
		xfont_Print_Centered_Text(pilotName, frame, REGISTER_INFO_LABEL_FONT,
								  REGISTER_INFO_ACCESS_SHADOW_COLOR);
		xrect_Offset_Rect(frame, -1, -1);
		xfont_Print_Centered_Text(pilotName, frame, REGISTER_INFO_LABEL_FONT, REGISTER_INFO_ACCESS_COLOR);
	}
}

// FUNCTION: XW 0x45DE70
int16_t register_Index_To_Pilot(int16_t logicalIndex, int16_t* outSlot) {
	if (g_RegisterFastPilotHandle == LANDRU_NULL_HANDLE) {
		return 0;
	} else {
		const REGISTER_FastPilotRecord* records = xmemhdl_Lock_Handle(g_RegisterFastPilotHandle);
		int slotIndex;
		int16_t visibleIndex = 0;
		*outSlot = REGISTER_PILOT_SLOT_NONE;
		for (slotIndex = 0;
			 (int16_t)slotIndex < g_RegisterLoadedPilotCount && *outSlot == REGISTER_PILOT_SLOT_NONE;
			 ++slotIndex) {
			if (records[slotIndex].deleted == 0) {
				if (visibleIndex == logicalIndex) {
					*outSlot = slotIndex;
				} else {
					++visibleIndex;
				}
			}
		}
		xmemhdl_Unlock_Handle(g_RegisterFastPilotHandle);
		return *outSlot != REGISTER_PILOT_SLOT_NONE;
	}
}

// FUNCTION: XW 0x45DEF0
int16_t register_Index_To_Pilot_Record(int16_t logicalIndex, struct REGISTER_FastPilotRecord* outRecord) {
	if (g_RegisterFastPilotHandle == LANDRU_NULL_HANDLE) {
		return 0;
	} else {
		const REGISTER_FastPilotRecord* records = xmemhdl_Lock_Handle(g_RegisterFastPilotHandle);
		int slotIndex;
		int16_t matchedSlot = REGISTER_PILOT_SLOT_NONE;
		int16_t visibleIndex = 0;
		for (slotIndex = 0;
			 (int16_t)slotIndex < g_RegisterLoadedPilotCount && matchedSlot == REGISTER_PILOT_SLOT_NONE;
			 ++slotIndex) {
			if (records[slotIndex].deleted == 0) {
				if (visibleIndex == logicalIndex) {
					matchedSlot = slotIndex;
					*outRecord = records[slotIndex];
				} else {
					++visibleIndex;
				}
			}
		}
		xmemhdl_Unlock_Handle(g_RegisterFastPilotHandle);
		return matchedSlot != REGISTER_PILOT_SLOT_NONE;
	}
}

// FUNCTION: XW 0x45DF80
void register_Build_Fast_Pilot_Record(void) {
	REGISTER_FastPilotRecord* records;
	int slotIndex;
	char pilotName[REGISTER_DIRECTORY_NAME_CAPACITY];
	char pilotPath[REGISTER_DIRECTORY_NAME_CAPACITY];
	g_RegisterFastPilotHandle = xmemhdl_Alloc_Handle(
		sizeof(REGISTER_FastPilotRecord) * g_RegisterLoadedPilotCount, LANDRU_MEMORY_RESOURCE);
	records = xmemhdl_Lock_Handle(g_RegisterFastPilotHandle);
	g_RegisterPilotCount = 0;
	for (slotIndex = 0; (int16_t)slotIndex < g_RegisterLoadedPilotCount; ++slotIndex) {
		REGISTER_FastPilotRecord* record = &records[slotIndex];
		record->name[0] = '\0';
		record->deleted = REGISTER_PILOT_SLOT_NONE;
		if (register_Find_Reg_Dir_Name(&g_RegisterDirectory, pilotName, slotIndex) != 0) {
			LandruFile* pilotFile;
			strcpy(pilotPath, pilotName);
			strcat(pilotPath, ".PLT");
			pilotFile = xfile_Open_File(LANDRU_FILE_ROOT_USER, pilotPath, "rb");
			if (pilotFile != NULL) {
				if ((uint16_t)register_ReadPilotRecord(pilotFile, &g_RegisterPilotReadBuffer) != 0) {
					unsigned int byteIndex;
					/* The original name copy includes metadata and the skill bytes. */
					for (byteIndex = 0; byteIndex < REGISTER_FAST_PILOT_NAME_COPY_BYTES; ++byteIndex) {
						((unsigned char*)record)[byteIndex] = (unsigned char)pilotName[byteIndex];
					}
					record->field_18 = g_RegisterPilotReadBuffer.field_0;
					record->deleted = g_RegisterPilotReadBuffer.deleted;
					record->lost_status = g_RegisterPilotReadBuffer.lost_status;
					record->rank = g_RegisterPilotReadBuffer.rank;
					record->current_tour = g_RegisterPilotReadBuffer.current_tour;
					record->score = g_RegisterPilotReadBuffer.score;
					++g_RegisterPilotCount;
				}
				xfile_Close_File(pilotFile);
			}
		}
	}
	xmemhdl_Unlock_Handle(g_RegisterFastPilotHandle);
}

// FUNCTION: XW 0x45E120
void register_Delete_Pilot_Record(void) {
	if (g_RegisterFastPilotHandle != LANDRU_NULL_HANDLE) {
		REGISTER_FastPilotRecord* records = xmemhdl_Lock_Handle(g_RegisterFastPilotHandle);
		int16_t slotIndex;
		int16_t visibleIndex = 0;
		int16_t deleted = 0;
		for (slotIndex = 0; slotIndex < g_RegisterLoadedPilotCount; ++slotIndex) {
			if (records[slotIndex].deleted == 0) {
				if (visibleIndex == g_RegisterActivePilot) {
					records[slotIndex].name[0] = 0;
					records[slotIndex].deleted = -1;
					deleted = 1;
				}
				++visibleIndex;
			}
		}
		xmemhdl_Unlock_Handle(g_RegisterFastPilotHandle);
		if (deleted != 0) {
			g_RegisterPageCount =
				(--g_RegisterPilotCount + REGISTER_PILOT_VISIBLE_ROWS - 1) / REGISTER_PILOT_VISIBLE_ROWS;
			if (g_RegisterPageCount == 0) {
				g_RegisterPageCount = 1;
			}
			if (g_RegisterCurrentPage >= g_RegisterPageCount) {
				g_RegisterCurrentPage = g_RegisterPageCount - 1;
			}
		}
		g_RegisterActivePilot = REGISTER_PILOT_SLOT_NONE;
		memset(&g_RegisterPilotData, 0, sizeof(g_RegisterPilotData));
	}
}

// FUNCTION: XW 0x45E200
void register_Revive_Pilot_Record(void) {
	char pilotPath[REGISTER_PILOT_PATH_CAPACITY];
	register_Get_Reg_String_Button_Name(g_RegisterPilotNameInput, pilotPath);
	strcat(pilotPath, ".PLT");
	shipext_Load_Pilot(pilotPath, 0);
	shipext_Revive_Pilot(pilotPath, SHIPEXT_REVIVE_WITH_PENALTY);
	if (g_RegisterFastPilotHandle != LANDRU_NULL_HANDLE) {
		REGISTER_FastPilotRecord* records = xmemhdl_Lock_Handle(g_RegisterFastPilotHandle);
		int16_t visibleIndex = 0;
		int slotIndex;
		for (slotIndex = 0; (int16_t)slotIndex < g_RegisterLoadedPilotCount; ++slotIndex) {
			if (records[slotIndex].deleted == 0) {
				if (visibleIndex == g_RegisterActivePilot) {
					records[slotIndex].lost_status = 0;
				}
				++visibleIndex;
			}
		}
		xmemhdl_Unlock_Handle(g_RegisterFastPilotHandle);
	}
}

// FUNCTION: XW 0x45E2D0
void register_Set_Your_Reg_Pilot(void) {
	if (g_RegisterFastPilotHandle == LANDRU_NULL_HANDLE) {
		g_RegisterShellPilot.name[0] = 0;
	} else {
		const REGISTER_FastPilotRecord* records = xmemhdl_Lock_Handle(g_RegisterFastPilotHandle);
		int slotIndex;
		for (slotIndex = 0; (int16_t)slotIndex < g_RegisterLoadedPilotCount &&
							g_RegisterActivePilot == REGISTER_PILOT_SLOT_NONE;
			 ++slotIndex) {
			if (records[slotIndex].deleted == 0 &&
				strcmp(records[slotIndex].name, g_RegisterShellPilot.name) == 0) {
				g_RegisterActivePilot = slotIndex;
			}
		}
		if (g_RegisterActivePilot != REGISTER_PILOT_SLOT_NONE) {
			register_Set_Reg_String_Button_Name(g_RegisterPilotNameInput, g_RegisterShellPilot.name);
			g_RegisterPilotInfoInput->var1 = 1;
		}
		xmemhdl_Unlock_Handle(g_RegisterFastPilotHandle);
	}
}

// FUNCTION: XW 0x45E390
struct REGISTER_RegStringButton* register_Alloc_Input_Reg_String_Button(Input* parent, Rect* frame,
																		int16_t zinput, InputUserFunc user,
																		const char* initialName,
																		int16_t isFilenameMode, int16_t id) {
	REGISTER_RegStringButton* button = (REGISTER_RegStringButton*)xinput_Alloc_Dialog_Input(
		parent, frame, zinput, sizeof(REGISTER_RegStringButton) - sizeof(Input));
	int16_t index;
	xinpattr_Set_Input_Draw_Function(&button->input, register_idraw_Reg_String_Button);
	xinpattr_Set_Input_Update_Function(&button->input, register_iupdate_Reg_String_Button);
	xinpattr_Set_Input_User_Function(&button->input, user);
	button->input.id = id;
	for (index = 0; initialName[index]; index++)
		button->name[index] = initialName[index];
	button->name[index] = 0;
	button->is_filename_mode = isFilenameMode;
	return button;
}

// FUNCTION: XW 0x45E420
void register_idraw_Reg_String_Button(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	REGISTER_RegStringButton* button = (REGISTER_RegStringButton*)input;
	int16_t textWidth = xfont_Get_String_Width_0(REGISTER_STRING_FONT, button->name);
	(void)clip;
	if (refresh != 0) {
		xstyle_Style_Paint_TextField(frame);
		xfont_Print_Clipped_Text(button->name, frame->left + REGISTER_STRING_TEXT_OFFSET,
								 frame->top + REGISTER_STRING_TEXT_OFFSET, REGISTER_STRING_FONT,
								 xstyle_Get_Style_Down_Color());
	}
	if ((uint16_t)xinpattr_Is_Input_Active(input) != 0) {
		if ((uint16_t)xio_Blink() != 0) {
			xpaint_Horiz_Clipped_Line(textWidth + frame->left + REGISTER_STRING_CARET_X,
									  frame->top + REGISTER_STRING_CARET_Y, REGISTER_STRING_CARET_WIDTH,
									  xstyle_Get_Style_Down_Color());
		} else {
			xpaint_Horiz_Clipped_Line(textWidth + frame->left + REGISTER_STRING_CARET_X,
									  frame->top + REGISTER_STRING_CARET_Y, REGISTER_STRING_CARET_WIDTH, 0);
		}
	}
}

// FUNCTION: XW 0x45E4E0
int16_t register_iupdate_Reg_String_Button(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
										   int rightEvent, int16_t x, int16_t y) {
	REGISTER_RegStringButton* button = (REGISTER_RegStringButton*)input;
	char workingText[REGISTER_STRING_WORK_CAPACITY];
	int16_t index;
	int16_t length;
	int16_t accepted;
	(void)frame;
	(void)clip;
	(void)leftEvent;
	(void)rightEvent;
	(void)x;
	(void)y;
	for (index = 0; button->name[index] != '\0'; ++index)
		workingText[index] = button->name[index];
	workingText[index] = '\0';
	length = (int16_t)strlen(workingText);
	accepted = 1;
	if (key != 0) {
		if (key == REGISTER_KEY_DELETE || key == '\b') {
			if (length != 0)
				workingText[length - 1] = '\0';
		} else if (key > ' ' && key < REGISTER_KEY_ASCII_DELETE) {
			if ((isalpha)(key)) {
				accepted = register_Add_Key_To_Reg_String(button, workingText, (char)toupper(key));
			} else if ((isdigit)(key) && button->is_filename_mode != 0) {
				accepted = register_Add_Key_To_Reg_String(button, workingText, (char)key);
			} else if ((key == '_' || key == '-') && button->is_filename_mode != 0) {
				accepted = register_Add_Key_To_Reg_String(button, workingText, (char)key);
			} else {
				accepted = 0;
			}
		} else {
			accepted = 0;
		}
	}
	for (index = 0; workingText[index] != '\0'; ++index)
		button->name[index] = workingText[index];
	button->name[index] = '\0';
	if (accepted != 0) {
		xinpattr_Refresh_Input(input);
		xinpattr_Selected_Input(input);
	}
	return accepted;
}

// FUNCTION: XW 0x45E620
int16_t register_Add_Key_To_Reg_String(struct REGISTER_RegStringButton* input, char* text, char character) {
	int16_t length = (int16_t)strlen(text);
	(void)input;
	if (length < REGISTER_NAME_INPUT_LIMIT) {
		text[length] = character;
		text[length + 1] = '\0';
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x45E660
void register_Set_Reg_String_Button_Name(struct REGISTER_RegStringButton* input, const char* name) {
	int16_t index;
	for (index = 0; name[index] != '\0'; ++index)
		input->name[index] = name[index];
	input->name[index] = '\0';
	xinpattr_Refresh_Input(&input->input);
}

// FUNCTION: XW 0x45E6A0
int register_Get_Reg_String_Button_Name(const struct REGISTER_RegStringButton* input, char* destination) {
	int16_t index;
	for (index = 0; input->name[index] != '\0'; ++index)
		destination[index] = input->name[index];
	destination[index] = '\0';
	return index;
}

// FUNCTION: XW 0x45E6D0
void register_Do_Delete_Dialog(DialogSubResultHandler complete, void* context) {
	Input* dialog;
	int16_t hadKeyButtons = xio_Is_Key_Buttons();
	if (hadKeyButtons == 0)
		xio_Set_Key_Buttons();
	g_RegisterDeleteFocus = REGISTER_DELETE_INITIAL_FOCUS;
	xio_Set_Mouse_Position(g_RegisterDeleteMouseX[REGISTER_DELETE_INITIAL_FOCUS],
						   g_RegisterDeleteMouseY[REGISTER_DELETE_INITIAL_FOCUS]);
	dialog = register_Build_Delete_Dialog();
	XwRegister_ScheduleDeleteDialog(dialog, hadKeyButtons, complete, context);
}

// FUNCTION: XW 0x45E740
Input* register_Build_Delete_Dialog(void) {
	Rect rect;
	Input* dialog;
	PushButton* button;
	xrect_Set_Rect(&rect, 0, 0, REGISTER_DELETE_DIALOG_WIDTH, REGISTER_DELETE_DIALOG_HEIGHT);
	dialog = xinput_Alloc_Dialog_Input(NULL, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(dialog, register_iupdate_Delete_Input);
	xinpattr_Set_Input_Draw_Function(dialog, register_idraw_Delete_Input);
	xinpattr_Set_Input_Allign(dialog, REGISTER_DIALOG_ALIGN_CENTER, REGISTER_DIALOG_ALIGN_CENTER);
	xinpattr_Show_Input(dialog);
	dialog->id = 0;
	xrect_Set_Rect(&rect, REGISTER_DELETE_BUTTON_INSET, REGISTER_DELETE_BUTTON_INSET,
				   REGISTER_DELETE_BUTTON_INSET + REGISTER_DELETE_BUTTON_WIDTH,
				   REGISTER_DELETE_BUTTON_BOTTOM);
	button =
		xbtnpush_Alloc_Button(dialog, &rect, 0, register_iuser_Delete_Input,
							  g_RegisterResourceAndLabelText[REGISTER_TEXT_DELETE], REGISTER_DELETE_CONFIRM);
	xinpattr_Set_Input_Draw_Function(&button->header, (InputDrawFunc)register_idraw_DialogButton);
	xinpattr_Set_Input_Allign(&button->header, REGISTER_DIALOG_ALIGN_NEAR, REGISTER_DIALOG_ALIGN_FAR);
	register_Index_To_Pilot_Record(g_RegisterActivePilot, &g_RegisterShellPilot);
	if (g_RegisterShellPilot.lost_status != REGISTER_PILOT_AVAILABLE) {
		xrect_Set_Rect(&rect, 0, REGISTER_DELETE_BUTTON_INSET, REGISTER_DELETE_BUTTON_WIDTH,
					   REGISTER_DELETE_BUTTON_BOTTOM);
		button = xbtnpush_Alloc_Button(dialog, &rect, 0, register_iuser_Delete_Input,
									   g_RegisterResourceAndLabelText[REGISTER_TEXT_REVIVE],
									   REGISTER_DELETE_REVIVE);
		xinpattr_Set_Input_Draw_Function(&button->header, (InputDrawFunc)register_idraw_DialogButton);
		xinpattr_Set_Input_Allign(&button->header, REGISTER_DIALOG_ALIGN_CENTER, REGISTER_DIALOG_ALIGN_FAR);
	}
	xrect_Set_Rect(&rect, REGISTER_DELETE_BUTTON_INSET, REGISTER_DELETE_BUTTON_INSET,
				   REGISTER_DELETE_BUTTON_INSET + REGISTER_DELETE_BUTTON_WIDTH,
				   REGISTER_DELETE_BUTTON_BOTTOM);
	button =
		xbtnpush_Alloc_Button(dialog, &rect, 0, register_iuser_Delete_Input,
							  g_RegisterResourceAndLabelText[REGISTER_TEXT_CANCEL], REGISTER_DELETE_CANCEL);
	xinpattr_Set_Input_Draw_Function(&button->header, (InputDrawFunc)register_idraw_DialogButton);
	xinpattr_Set_Input_Allign(&button->header, REGISTER_DIALOG_ALIGN_FAR, REGISTER_DIALOG_ALIGN_FAR);
	return dialog;
}

// FUNCTION: XW 0x45E8C0
int16_t register_iupdate_Delete_Input(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
									  int rightEvent, int16_t x, int16_t y) {
	(void)input;
	(void)frame;
	(void)clip;
	(void)leftEvent;
	(void)rightEvent;
	(void)x;
	(void)y;
	if (key != 0) {
		if (xio_Is_Joystick_Input() && xio_Is_Joystick_Callibrate() && key == LANDRU_JOYSTICK_CALIBRATE_KEY) {
#ifdef XW_MODERN
			XwPort_RequestSettingsPage(XW_SETTINGS_CONTROLLER);
#else
			/* The shared dialog task clears its exit state when calibration returns. */
			xdlgjoy_Schedule_Joystick_Callibrate(NULL, NULL);
#endif
			return 1;
		}
		if (key == REGISTER_KEY_LEFT || key == REGISTER_KEY_UP) {
			g_RegisterDeleteFocus =
				(g_RegisterDeleteFocus + REGISTER_DELETE_TARGET_COUNT - 1) % REGISTER_DELETE_TARGET_COUNT;
			xio_Set_Mouse_Position(g_RegisterDeleteMouseX[g_RegisterDeleteFocus],
								   g_RegisterDeleteMouseY[g_RegisterDeleteFocus]);
			return 1;
		} else if (key == REGISTER_KEY_RIGHT || key == REGISTER_KEY_DOWN) {
			g_RegisterDeleteFocus = (g_RegisterDeleteFocus + 1) % REGISTER_DELETE_TARGET_COUNT;
			xio_Set_Mouse_Position(g_RegisterDeleteMouseX[g_RegisterDeleteFocus],
								   g_RegisterDeleteMouseY[g_RegisterDeleteFocus]);
			return 1;
		}
	}
	return 0;
}

// FUNCTION: XW 0x45E9A0
void register_iuser_Delete_Input(Input* input, int context) {
	(void)context;
	if (xinpattr_Get_Input_Selected(input)) {
		int16_t id = input->id;
		switch (id) {
			case REGISTER_DELETE_CONFIRM:
			case REGISTER_DELETE_CANCEL:
			case REGISTER_DELETE_REVIVE:
				xdialog_Set_Dialog_Exit(id);
				break;
		}
	}
}

// FUNCTION: XW 0x45E9D0
void register_idraw_Delete_Input(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	Rect titleRect;
	char pilotName[REGISTER_STRING_WORK_CAPACITY];
	char title[REGISTER_DIALOG_TITLE_CAPACITY];
	(void)clip;
	if (refresh != 0) {
		register_Get_Reg_String_Button_Name(g_RegisterPilotNameInput, pilotName);
		register_Index_To_Pilot_Record(g_RegisterActivePilot, &g_RegisterShellPilot);
		if (g_RegisterShellPilot.lost_status != REGISTER_PILOT_AVAILABLE)
			strcpy(title, g_RegisterResourceAndLabelText[REGISTER_TEXT_MODIFY_PILOT]);
		else
			strcpy(title, g_RegisterResourceAndLabelText[REGISTER_TEXT_DELETE_PILOT]);
		strcat(title, " ");
		strcat(title, pilotName);
		strcat(title, "?");
		if (input->id == 0) {
			xrect_Copy_Rect(&titleRect, frame);
			xpaint_Frame_Clipped_Rect(&titleRect, REGISTER_DIALOG_FRAME_COLOR);
			xrect_Inset_Rect(&titleRect, REGISTER_DIALOG_FRAME_INSET, REGISTER_DIALOG_FRAME_INSET);
			xpaint_Paint_Clipped_Bevel(frame, REGISTER_DIALOG_PANEL_TOP_COLOR,
									   REGISTER_DIALOG_PANEL_BOTTOM_COLOR, REGISTER_DIALOG_PANEL_FILL_COLOR,
									   REGISTER_DIALOG_PANEL_PRESSED);
			titleRect.bottom = titleRect.top + REGISTER_DIALOG_TITLE_HEIGHT;
			xfont_Enable_FontID_Shadow(REGISTER_DIALOG_FONT);
			xfont_Print_Centered_Text(title, &titleRect, REGISTER_DIALOG_FONT, REGISTER_DIALOG_TEXT_COLOR);
			xfont_Disable_FontID_Shadow(REGISTER_DIALOG_FONT);
		}
	}
}

// FUNCTION: XW 0x45EB60
void register_idraw_DialogButton(PushButton* button, Rect* frame, Rect* unusedClip, int16_t refresh) {
	(void)unusedClip;
	if (refresh != 0) {
		const char* labels = button->labels;
		int16_t labelsToSkip = button->labelIndex;
		size_t labelOffset;
		char character;
		const char* labelText;
		for (labelOffset = 0, character = labels[0]; character != 0 && labelsToSkip != 0;
			 --labelsToSkip, character = labels[labelOffset]) {
			for (++labelOffset; character != 0; ++labelOffset) {
				character = labels[labelOffset];
			}
		}
		labelText = &labels[labelOffset];
		xpaint_Paint_Clipped_DBevel(frame, REGISTER_DIALOG_OUTER_TOP_COLOR, REGISTER_DIALOG_INNER_TOP_COLOR,
									REGISTER_DIALOG_OUTER_BOTTOM_COLOR, REGISTER_DIALOG_INNER_BOTTOM_COLOR,
									REGISTER_DIALOG_FILL_COLOR, REGISTER_DIALOG_BEVEL_PRESSED);
		xfont_Print_Centered_Text(labelText, frame, REGISTER_DIALOG_FONT,
								  REGISTER_DIALOG_TEXT_COLOR + (button->pressed != 0));
	}
}

// FUNCTION: XW 0x45EBD0
void register_Do_Protect_Dialog(DialogSubResultHandler complete, void* context) {
	Input* dialog;
	int16_t hadKeyButtons = xio_Is_Key_Buttons();
	if (hadKeyButtons == 0)
		xio_Set_Key_Buttons();
	xio_Set_Mouse_Position(g_RegisterProtectMouseX[g_RegisterProtectFocus],
						   g_RegisterProtectMouseY[g_RegisterProtectFocus]);
	g_RegisterProtectChosen = 0;
	g_RegisterProtectAttempts = 0;
	g_RegisterProtectQuestionIndex = 0;
	memcpy(g_RegisterProtectSymbols, g_RegisterProtectQuestionSymbols[0], sizeof(g_RegisterProtectSymbols));
	dialog = register_Build_Protect_Dialog();
	XwRegister_ScheduleProtectDialog(dialog, hadKeyButtons, complete, context);
}

// FUNCTION: XW 0x45EC70
Input* register_Build_Protect_Dialog(void) {
	Rect rect;
	Input* dialog;
	Input* panel;
	REGISTER_RegStringButton* nameInput;
	PushButton* button;
	xrect_Set_Rect(&rect, 0, 0, REGISTER_PROTECT_DIALOG_WIDTH, REGISTER_PROTECT_DIALOG_HEIGHT);
	dialog = xinput_Alloc_Dialog_Input(NULL, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(dialog, register_iupdate_Protect_Input);
	xinpattr_Set_Input_Draw_Function(dialog, register_idraw_Protect_Input);
	xinpattr_Set_Input_Allign(dialog, REGISTER_DIALOG_ALIGN_CENTER, REGISTER_DIALOG_ALIGN_CENTER);
	xinpattr_Show_Input(dialog);
	dialog->id = 0;
	g_RegisterProtectDialog = dialog;
	xrect_Set_Rect(&rect, REGISTER_PROTECT_INSET, REGISTER_PROTECT_SYMBOL_TOP, REGISTER_PROTECT_CONTENT_RIGHT,
				   REGISTER_PROTECT_SYMBOL_BOTTOM);
	panel = xinput_Alloc_Dialog_Input(dialog, &rect, 0, 0);
	xinpattr_Set_Input_User_Function(panel, register_iuser_Protect_Input);
	xinpattr_Set_Input_Draw_Function(panel, register_idraw_Protect_Input);
	xinpattr_Show_Input(panel);
	panel->id = REGISTER_PROTECT_SYMBOL_INPUT;
	xrect_Set_Rect(&rect, REGISTER_PROTECT_INSET, REGISTER_PROTECT_PAGE_TOP, REGISTER_PROTECT_CONTENT_RIGHT,
				   REGISTER_PROTECT_PAGE_BOTTOM);
	panel = xinput_Alloc_Dialog_Input(dialog, &rect, 0, 0);
	xinpattr_Set_Input_Draw_Function(panel, register_idraw_Protect_Input);
	xinpattr_Show_Input(panel);
	panel->id = REGISTER_PROTECT_PAGE_INPUT;
	xrect_Set_Rect(&rect, REGISTER_PROTECT_INSET, REGISTER_PROTECT_NAME_TOP, REGISTER_PROTECT_CONTENT_RIGHT,
				   REGISTER_PROTECT_NAME_BOTTOM);
	nameInput = register_Alloc_Input_Reg_String_Button(dialog, &rect, 0, register_iuser_Protect_Input,
													   g_sharedEmptyString, 0, REGISTER_PROTECT_FOCUS_INPUT);
	xinpattr_Hide_Input(&nameInput->input);
	g_RegisterProtectNameInput = nameInput;
	xrect_Set_Rect(&rect, REGISTER_PROTECT_INSET, REGISTER_PROTECT_INSET, REGISTER_PROTECT_OK_RIGHT,
				   REGISTER_PROTECT_BUTTON_BOTTOM);
	button = xbtnpush_Alloc_Button(dialog, &rect, 0, register_iuser_Protect_Input,
								   g_RegisterResourceAndLabelText[REGISTER_TEXT_PROTECT_OK],
								   REGISTER_PROTECT_ACCEPT_INPUT);
	xinpattr_Set_Input_Allign(&button->header, REGISTER_DIALOG_ALIGN_NEAR, REGISTER_DIALOG_ALIGN_FAR);
	xinpattr_Hide_Input(&button->header);
	g_RegisterProtectOkInput = &button->header;
	xrect_Set_Rect(&rect, REGISTER_PROTECT_INSET, REGISTER_PROTECT_INSET, REGISTER_PROTECT_EXIT_RIGHT,
				   REGISTER_PROTECT_BUTTON_BOTTOM);
	button = xbtnpush_Alloc_Button(dialog, &rect, 0, register_iuser_Protect_Input,
								   g_RegisterResourceAndLabelText[REGISTER_TEXT_PROTECT_EXIT],
								   REGISTER_PROTECT_CANCEL_INPUT);
	xinpattr_Set_Input_Allign(&button->header, REGISTER_DIALOG_ALIGN_FAR, REGISTER_DIALOG_ALIGN_FAR);
	xinpattr_Hide_Input(&button->header);
	g_RegisterProtectExitInput = &button->header;
	xrect_Set_Rect(&rect, REGISTER_PROTECT_INSET, REGISTER_PROTECT_INSET, REGISTER_PROTECT_CONTENT_RIGHT,
				   REGISTER_PROTECT_BUTTON_BOTTOM);
	button = xbtnpush_Alloc_Button(dialog, &rect, 0, register_iuser_Protect_Input,
								   g_RegisterResourceAndLabelText[REGISTER_TEXT_PROTECT_CONTINUE],
								   REGISTER_PROTECT_CHOOSE_INPUT);
	xinpattr_Set_Input_Allign(&button->header, REGISTER_DIALOG_ALIGN_NEAR, REGISTER_DIALOG_ALIGN_FAR);
	return dialog;
}

// FUNCTION: XW 0x45EEA0
int16_t register_iupdate_Protect_Input(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
									   int rightEvent, int16_t x, int16_t y) {
	(void)input;
	(void)frame;
	(void)clip;
	(void)leftEvent;
	(void)rightEvent;
	(void)x;
	(void)y;
	if (key != 0) {
		if (xio_Is_Joystick_Input() != 0 && xio_Is_Joystick_Callibrate() != 0 &&
			key == LANDRU_JOYSTICK_CALIBRATE_KEY) {
#ifdef XW_MODERN
			XwPort_RequestSettingsPage(XW_SETTINGS_CONTROLLER);
#else
			/* The shared dialog task clears its exit state when calibration returns. */
			xdlgjoy_Schedule_Joystick_Callibrate(NULL, NULL);
#endif
			return 1;
		}
		if (key == REGISTER_KEY_LEFT || key == REGISTER_KEY_UP || key == REGISTER_KEY_RIGHT ||
			key == REGISTER_KEY_DOWN) {
			g_RegisterProtectFocus ^= 1;
			xio_Set_Mouse_Position(g_RegisterProtectMouseX[g_RegisterProtectFocus],
								   g_RegisterProtectMouseY[g_RegisterProtectFocus]);
			return 1;
		}
	}
	return 0;
}

// FUNCTION: XW 0x45EF30
void register_iuser_Protect_Input(Input* input, int context) {
	char answer[REGISTER_STRING_WORK_CAPACITY];
	if (input->id == REGISTER_PROTECT_SYMBOL_INPUT && g_RegisterProtectChosen == 0) {
		g_RegisterProtectQuestionIndex = abs(rand() % REGISTER_PROTECT_QUESTION_COUNT);
		{
			int symbolIndex;
			for (symbolIndex = 0; symbolIndex < REGISTER_PROTECT_SYMBOL_COUNT; ++symbolIndex)
				g_RegisterProtectSymbols[symbolIndex] =
					g_RegisterProtectQuestionSymbols[g_RegisterProtectQuestionIndex][symbolIndex];
		}
		xinpattr_Refresh_Input(input);
	}
	if (xinpattr_Get_Input_Selected(input) != 0) {
		switch (input->id) {
			case REGISTER_PROTECT_ACCEPT_INPUT: {
				int16_t characterIndex;
				register_Get_Reg_String_Button_Name(g_RegisterProtectNameInput, answer);
				for (characterIndex = 0; answer[characterIndex] != '\0'; ++characterIndex) {
#ifdef XW_MODERN
					answer[characterIndex] = tolower((unsigned char)answer[characterIndex]);
#else
					answer[characterIndex] = tolower(answer[characterIndex]);
#endif
				}
				if (strcmp(answer, g_RegisterProtectAnswers[g_RegisterProtectQuestionIndex]) == 0) {
					xdialog_Set_Dialog_Exit(input->id);
				} else if (++g_RegisterProtectAttempts < REGISTER_PROTECT_ATTEMPT_LIMIT) {
					register_Set_Reg_String_Button_Name(g_RegisterProtectNameInput, g_sharedEmptyString);
					xinpattr_Refresh_Input(&g_RegisterProtectNameInput->input);
					register_PlaySpeech(REGISTER_SPEECH_REGISTER, context);
				} else {
					xdialog_Set_Dialog_Exit(REGISTER_PROTECT_CANCEL_INPUT);
				}
				break;
			}
			case REGISTER_PROTECT_CANCEL_INPUT:
				xdialog_Set_Dialog_Exit(input->id);
				break;
			case REGISTER_PROTECT_FOCUS_INPUT:
				g_RegisterProtectFocus = 0;
				xio_Set_Mouse_Position(g_RegisterProtectMouseX[0], g_RegisterProtectMouseY[0]);
				break;
			case REGISTER_PROTECT_CHOOSE_INPUT:
				g_RegisterProtectChosen = 1;
				xinpattr_Hide_Input(input);
				xinpattr_Show_Input(&g_RegisterProtectNameInput->input);
				xinpattr_Show_Input(g_RegisterProtectOkInput);
				xinpattr_Show_Input(g_RegisterProtectExitInput);
				xinpattr_Refresh_Input(g_RegisterProtectDialog);
				break;
		}
	}
}

// FUNCTION: XW 0x45F160
void register_idraw_Protect_Input(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	Rect cellRect;
	char pageText[REGISTER_PAGE_TEXT_CAPACITY];
	if (refresh != 0) {
		switch (input->id) {
			case 0:
				xrect_Copy_Rect(&cellRect, frame);
				xpaint_Frame_Clipped_Rect(&cellRect, REGISTER_DIALOG_FRAME_COLOR);
				xrect_Inset_Rect(&cellRect, REGISTER_DIALOG_FRAME_INSET, REGISTER_DIALOG_FRAME_INSET);
				xstyle_Style_Paint_Border(&cellRect, 0);
				cellRect.bottom = cellRect.top + REGISTER_DIALOG_TITLE_HEIGHT;
				xfont_Enable_FontID_Shadow(REGISTER_DIALOG_FONT);
				xfont_Print_Centered_Text("Copy Protection", &cellRect, REGISTER_DIALOG_FONT,
										  REGISTER_DIALOG_TEXT_COLOR);
				xfont_Disable_FontID_Shadow(REGISTER_DIALOG_FONT);
				break;
			case REGISTER_PROTECT_SYMBOL_INPUT: {
				int16_t xOffset = 0;
				int symbolIndex;
				for (symbolIndex = 0;
					 xOffset < REGISTER_PROTECT_SYMBOL_COUNT * REGISTER_PROTECT_SYMBOL_SPACING;
					 ++symbolIndex) {
					xrect_Copy_Rect(&cellRect, frame);
					cellRect.left += xOffset;
					cellRect.right = cellRect.left + REGISTER_PROTECT_SYMBOL_WIDTH;
					xstyle_Style_Paint_TextField(&cellRect);
					xrect_Inset_Rect(&cellRect, REGISTER_PROTECT_SYMBOL_INSET, REGISTER_PROTECT_SYMBOL_INSET);
					xactor_Set_Actor_State(g_RegisterProtectSymbolActor,
										   g_RegisterProtectSymbols[symbolIndex], 0);
					xactanim_Draw_Anim_Actor(g_RegisterProtectSymbolActor, frame, clip,
											 cellRect.left + REGISTER_PROTECT_SYMBOL_X_OFFSET,
											 cellRect.top + REGISTER_PROTECT_SYMBOL_Y_OFFSET, 1);
					xOffset += REGISTER_PROTECT_SYMBOL_SPACING;
				}
				break;
			}
			case REGISTER_PROTECT_PAGE_INPUT:
				if (g_RegisterProtectChosen != 0) {
					sprintf(pageText, "See Manual Page %d",
							(g_RegisterProtectQuestionIndex >> REGISTER_PROTECT_QUESTIONS_PER_PAGE_SHIFT) +
								REGISTER_PROTECT_FIRST_MANUAL_PAGE);
					xfont_Enable_FontID_Shadow(REGISTER_DIALOG_FONT);
					xfont_Print_Centered_Text(pageText, frame, REGISTER_DIALOG_FONT,
											  REGISTER_DIALOG_TEXT_COLOR);
					xfont_Disable_FontID_Shadow(REGISTER_DIALOG_FONT);
				}
				break;
		}
	}
}

// FUNCTION: XW 0x45F2F0
int16_t register_Find_Reg_Dir_Name(Directory* directory, char* destination, int16_t index) {
	if (index < (int)directory->entries) {
		const DirEntry* entries = xmemhdl_Lock_Handle(directory->entries);
		strcpy(destination, entries[index].name);
		xmemhdl_Unlock_Handle(directory->entries);
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x45F9B0
void register_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		ResFile* musicResource;
		g_RegisterMusicState.film = sceneFilm;
		soundext_ClearTriggers();
		musicResource = xres_Open_Resource(":X-Wing Data\\RESOURCE\\rgmusic.lfd");
		if (musicResource == NULL) {
			musicResource = xres_Open_Resource("rgmusic.lfd");
		}
		g_RegisterMusicState.sound = xsound_Res_Music(musicResource, "security");
		xres_Close_Resource(musicResource);
		xsound_Set_Sound_User_Function(g_RegisterMusicState.sound, register_user_Music);
	}
}

// FUNCTION: XW 0x45FA20
void register_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0) {
		if (xerror_Get_Landru_Exit() == XW_SCENE_CONCOURSE) {
			ResFile* musicResource;
			Sound* marchMusic;
			soundext_FadeVolume(g_RegisterMusicState.sound, REGISTER_MUSIC_CLOSE_VOLUME,
								REGISTER_MUSIC_CLOSE_DURATION);
			musicResource = xres_Open_Resource(":X-Wing Data\\RESOURCE\\mmmusic.lfd");
			if (musicResource == NULL)
				musicResource = xres_Open_Resource("mmmusic.lfd");
			marchMusic = xsound_Res_Music(musicResource, "halmarch");
			xres_Close_Resource(musicResource);
			soundext_SetHook(g_RegisterMusicState.sound, XW_SOUND_CONTROL_DIRECT,
							 REGISTER_MUSIC_CLOSE_CONTROL, 0);
			soundext_SetTriggerContext(g_RegisterMusicState.sound, REGISTER_MUSIC_CLOSE_MARKER);
			soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_START, marchMusic->id, 0, 0, 0, 0, 0);
			soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_SHARE_PARTS, g_RegisterMusicState.sound->id,
										 marchMusic->id, 0, 0, 0, 0);
			soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_FADE_VOLUME, marchMusic->id, REGISTER_MUSIC_VOLUME,
										 REGISTER_MUSIC_FADE_DURATION, 0, 0, 0);
			soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_END, 0, 0, 0, 0, 0, 0);
			xsound_Set_Sound_Keep(marchMusic);
		} else {
			soundext_FadeVolume(g_RegisterMusicState.sound, 0, REGISTER_MUSIC_CLOSE_DURATION);
		}
	}
}

// FUNCTION: XW 0x45FB50
void register_user_Music(Sound* unusedSound, int unusedTime) {
	(void)unusedSound;
	(void)unusedTime;
	if (g_RegisterMusicState.film->cur_cel == REGISTER_MUSIC_START_CEL) {
		soundext_Start_Resource_Sound(g_RegisterMusicState.sound);
		xsound_Set_Sound_Keep(g_RegisterMusicState.sound);
		soundext_FadeVolume(g_RegisterMusicState.sound, REGISTER_MUSIC_VOLUME, REGISTER_MUSIC_FADE_DURATION);
		++g_RegisterMusicState.film->cur_cel;
	}
}

// FUNCTION: XW 0x45FBA0
void register_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	if (ShellPreferences_GetSfxEnabled() != 0) {
		soundext_LoadSfx(XW_SHELL_SFX_GUARD, 0, NULL, 0, 0);
		soundext_LoadSfx(XW_SHELL_SFX_DOOR_1, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_MENU_DOOR_CLOSE_1, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_LOGON_C, 0, NULL, 0, 1);
	}
	if (ShellPreferences_GetSfxEnabled() != 0) {
		g_RegisterSpeech[REGISTER_SPEECH_FIRST] =
			soundext_LoadSpeech(XW_SHELL_SPEECH_WELCOME, REGISTER_SPEECH_FIRST, NULL, 0);
		g_RegisterSpeech[REGISTER_SPEECH_WAIT] =
			soundext_LoadSpeech(XW_SHELL_SPEECH_YOU_MAY, REGISTER_SPEECH_WAIT, NULL, 1);
		g_RegisterSpeech[REGISTER_SPEECH_REGISTER] =
			soundext_LoadSpeech(XW_SHELL_SPEECH_REGISTER, REGISTER_SPEECH_REGISTER, NULL, 0);
	}
}

// FUNCTION: XW 0x45FC40
void register_PlaySpeech(int16_t speechIndex, int unusedContext) {
	(void)unusedContext;
	if (ShellPreferences_GetSfxEnabled() &&
		soundext_Count_Resource_Instances(g_RegisterSpeech[speechIndex]) == 0) {
		if (speechIndex == REGISTER_SPEECH_WAIT &&
			soundext_Count_Resource_Instances(g_RegisterSpeech[REGISTER_SPEECH_FIRST]) == 1) {
			soundext_Stop_Resource_Sound(g_RegisterSpeech[REGISTER_SPEECH_FIRST]);
		}
		soundext_Start_Resource_SFX(g_RegisterSpeech[speechIndex]);
		if (speechIndex == REGISTER_SPEECH_WAIT) {
#ifdef XW_MODERN
			XwRegister_WaitForSpeech();
#else
			while (soundext_Count_Resource_Instances(g_RegisterSpeech[REGISTER_SPEECH_WAIT]) == 1) {
			}
#endif
		}
	}
}

// FUNCTION: XW 0x45FCE0
void register_PlaySoundCue(int16_t cue) {
	if (ShellPreferences_GetSfxEnabled() != 0) {
		switch (cue) {
			case REGISTER_SOUND_GUARD:
				soundext_Play_SFX(XW_SHELL_SFX_GUARD);
				break;
			case REGISTER_SOUND_DOOR_OPEN:
				soundext_Play_SFX(XW_SHELL_SFX_DOOR_1);
				break;
			case REGISTER_SOUND_DOOR_CLOSE:
				soundext_Play_SFX(XW_SHELL_SFX_MENU_DOOR_CLOSE_1);
				break;
			case REGISTER_SOUND_PILOT_INFO:
				soundext_Play_SFX(XW_SHELL_SFX_LOGON_C);
				break;
		}
	}
}
