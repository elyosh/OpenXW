#include "xw/frontend/filmview.h"
#ifdef XW_MODERN
#include "xw_dos94/audio/soundext.h"
#endif

#ifdef XW_MODERN
#include "xw_runtime/audio/imuse_session.h"
#endif

#ifdef XW_MODERN
#include "xw_runtime/runtime/port.h"
#endif

#include "xw/audio/frontend_audio.h"
#include "xw/audio/soundext.h"
#include "xw/flight/replay/replay.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"
#include "xw_runtime/runtime/filmview_task.h"

#include <landru/dialog.h>
#include <landru/dlgjoy.h>
#include <landru/error.h>
#include <landru/font.h>
#include <landru/inpattr.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/paint.h>
#include <landru/style.h>
#include <landru/view.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4D339A
const char g_filmviewRoomFilmName[FILMVIEW_FILM_NAME_CAPACITY] = "filmview";

// GLOBAL: XW 0x4D33AC
const char g_filmviewAlternateFilmName[FILMVIEW_FILM_NAME_CAPACITY] = "filmview";

// GLOBAL: XW 0x4D33BE
char g_filmviewDeleteLabel[FILMVIEW_DELETE_LABEL_CAPACITY] = "Delete Film";

// GLOBAL: XW 0x4D3440
const int16_t g_filmviewFocusX[FILMVIEW_FOCUS_COUNT] = {
	30, 130, 30, 130, 30, 130, 30, 130, 30, 130, 30, 130, 30, 130, 30, 130, 4, 164, 44, 124, 44, 124
};

// GLOBAL: XW 0x4D3470
const int16_t g_filmviewFocusY[FILMVIEW_FOCUS_COUNT] = { 4,  4,  12, 12, 20, 20, 28, 28, 36, 36,  44,
														 44, 52, 52, 60, 60, 76, 76, 96, 96, 112, 112 };

// GLOBAL: XW 0x4D349C
int16_t g_FilmViewDeleteMouseX[FILMVIEW_DELETE_TARGET_COUNT] = { 368, 272 };

// GLOBAL: XW 0x4D34A0
int16_t g_FilmViewDeleteMouseY[FILMVIEW_DELETE_TARGET_COUNT] = { 250, 250 };

// GLOBAL: XW 0x4D37D8
const char* g_filmviewMusicName = "waiting";

// GLOBAL: XW 0x4D37DC
const char* g_filmviewMusicFilename = "bpmusic.lfd";

// GLOBAL: XW 0x4D37E0
const char* g_filmviewTransitionMusicName = "halmarch";

// GLOBAL: XW 0x4F7480
Input* g_filmviewDeleteInput = NULL;

// GLOBAL: XW 0x4F7484
Film* g_filmviewSceneFilm = NULL;

// GLOBAL: XW 0x4F7488
int16_t g_filmviewPageCount = 0;

// GLOBAL: XW 0x4F748C
Input* g_filmviewLoadInput = NULL;

// GLOBAL: XW 0x4F7490
int16_t g_filmviewCurrentPage = 0;
// GLOBAL: XW 0x4F7494
int16_t g_filmviewReturnToConcourse = 0;

// GLOBAL: XW 0x4F7498
int16_t g_filmviewFocusIndex = 0;

// GLOBAL: XW 0x4F74A0
char g_filmviewSelectedName[FILMVIEW_SELECTED_NAME_CAPACITY] = { 0 };

// GLOBAL: XW 0x4F74B0
int16_t g_FilmViewDeleteFocus = 0;

// GLOBAL: XW 0x4F7518
XwCutsceneMusicTransitionState g_filmviewMusicState = { NULL, NULL, NULL };

// FUNCTION: XW 0x448CC0
XwShellSceneResult filmview_FilmView(struct XwShellContext* shell) {
	ResFile* resourceFile;
	Rect screenRect;
	g_filmviewPageCount = 1;
	g_filmviewCurrentPage = 0;
	resourceFile = xres_Open_Resource("filmview.lfd");
	xrect_Set_Rect(&screenRect, 0, 0, FILMVIEW_SCENE_WIDTH, FILMVIEW_SCENE_HEIGHT);
	if (shellext_Get_Last_Scene() == XW_SCENE_CONCOURSE) {
		g_filmviewReturnToConcourse = 1;
		g_ReplayClipName[0] = '\0';
	}
	if (shellext_Get_Last_Scene() == XW_SCENE_TECH_ROOM) {
		g_filmviewReturnToConcourse = 0;
		g_ReplayClipName[0] = '\0';
	}
	switch (shipext_Get_Mission_Outcome()) {
		case SHIPEXT_MISSION_OUTCOME_FILM_ROOM:
			g_filmviewSceneFilm = xfilm_Res_Film(resourceFile, g_filmviewRoomFilmName, &screenRect, 0, 0, 0);
			break;
		case SHIPEXT_MISSION_OUTCOME_LEGACY_15:
		case SHIPEXT_MISSION_OUTCOME_LEGACY_17:
			g_filmviewSceneFilm =
				xfilm_Res_Film(resourceFile, g_filmviewAlternateFilmName, &screenRect, 0, 0, 0);
			break;
	}
	xfilm_Set_Film_Def_Palette(g_filmviewSceneFilm, shell->standardPalette);
	xview_Set_View_Update_Function(filmview_end_View);
	filmview_OpenMusic(resourceFile, g_filmviewSceneFilm);
	soundext_RecheckSfxPreference();
	FrontendAudio_PlayFile("XwingCD\\music\\filmtech.wav", 1);
#ifdef XW_MODERN
	XwFilmView_RunView(resourceFile);
#else
	j_xviewadd_Handle_View();
	filmview_CloseMusic();
	soundext_RecheckSfxPreference();
	xview_Clear_View_Update_Function();
	xres_Close_Resource(resourceFile);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x448DF0
void filmview_end_View(int unusedTime) {
	int16_t accepted;
	(void)unusedTime;
	accepted = XwFilmView_HandleFileDialog();
	if (accepted < 0)
		return;
	if (accepted != 0 && g_filmviewSelectedName[0] != '\0') {
		strcpy(g_ReplayClipName, g_filmviewSelectedName);
		xerror_Set_Landru_Exit(XW_SCENE_FLIGHT_REPLAY);
	} else if (g_filmviewReturnToConcourse != 0) {
		xerror_Set_Landru_Exit(XW_SCENE_CONCOURSE);
	} else {
		xerror_Set_Landru_Exit(XW_SCENE_TECH_ROOM);
	}
}

// FUNCTION: XW 0x448E60
void filmview_Do_FV_File_Dialog(DialogSubResultHandler complete, void* context) {
	int16_t hadKeyButtons = xio_Is_Key_Buttons();
	int16_t built;
	Input* root = NULL;
	FILMVIEW_FileDialog* dialog;
	if (hadKeyButtons == 0)
		xio_Set_Key_Buttons();
	dialog = XwFilmView_BeginFileDialog(hadKeyButtons, complete, context);
	dialog->page_start = 0;
	dialog->selected_file = 0;
	dialog->hit_count = 0;
	dialog->field_40 = 1;
	g_filmviewSelectedName[0] = '\0';
	xfiledir_Init_Directory(&dialog->directory, ".CLP", 0);
	xfiledir_Read_Directory(&dialog->directory);
	g_filmviewPageCount = (dialog->directory.count + FILMVIEW_FILES_PER_PAGE - 1) / FILMVIEW_FILES_PER_PAGE;
	if (g_filmviewPageCount == 0)
		g_filmviewPageCount = 1;
	built = filmview_Build_FV_File_Dialog(&root, dialog, "Load Mission Film");
	dialog->root = root;
	if (dialog->directory.count != 0) {
		filmview_Set_Active_FV_File(dialog, 0, 1);
		dialog->hit_count = 0;
	}
	XwFilmView_ScheduleFileDialog(built);
}

// FUNCTION: XW 0x448F60
int16_t filmview_Build_FV_File_Dialog(Input** outRoot, struct FILMVIEW_FileDialog* dialog,
									  const char* title) {
	Rect rect;
	Input* root;
	Input* input;
	PushButton* button;
	xrect_Set_Rect(&rect, 0, FILMVIEW_DIALOG_TOP, FILMVIEW_DIALOG_RIGHT, FILMVIEW_DIALOG_BOTTOM);
	root = xinput_Alloc_Dialog_Input(NULL, &rect, 0, 0);
	xinpattr_Set_Input_Draw_Function(root, filmview_idraw_FV_File);
	xinpattr_Set_Input_Allign(root, FILMVIEW_INPUT_ALIGN_CENTER, FILMVIEW_INPUT_ALIGN_START);
	root->varptr = (void*)title;
	root->id = FILMVIEW_TITLE_ID;
	xrect_Set_Rect(&rect, FILMVIEW_CONTROL_LEFT, FILMVIEW_LIST_TOP, FILMVIEW_LIST_RIGHT,
				   FILMVIEW_LIST_BOTTOM);
	input = xinput_Alloc_Dialog_Input(root, &rect, 0, 0);
	xinpattr_Set_Input_Draw_Function(input, filmview_idraw_FV_File);
	xinpattr_Set_Input_Update_Function(input, filmview_iupdate_FV_File);
	xinpattr_Set_Input_User_Function(input, filmview_iuser_FV_File);
	input->mouseUsage = FILMVIEW_LIST_MOUSE_USAGE;
	input->varptr = dialog;
	input->id = FILMVIEW_LIST_ID;
	xrect_Set_Rect(&rect, FILMVIEW_CONTROL_LEFT, FILMVIEW_PAGE_BUTTON_TOP, FILMVIEW_PAGE_BUTTON_RIGHT,
				   FILMVIEW_PAGE_BUTTON_BOTTOM);
	button = xbtnpush_Alloc_Button(root, &rect, 0, filmview_iuser_FilmView_Button, NULL,
								   FILMVIEW_PREVIOUS_PAGE_BUTTON);
	xinpattr_Set_Input_Draw_Function(&button->header, filmview_idraw_FilmView_Button);
	xinpattr_Set_Input_Allign(&button->header, FILMVIEW_INPUT_ALIGN_START, FILMVIEW_INPUT_ALIGN_END);
	button->header.varptr = dialog;
	xrect_Set_Rect(&rect, FILMVIEW_CONTROL_LEFT, FILMVIEW_PAGE_BUTTON_TOP, FILMVIEW_PAGE_BUTTON_RIGHT,
				   FILMVIEW_PAGE_BUTTON_BOTTOM);
	button = xbtnpush_Alloc_Button(root, &rect, 0, filmview_iuser_FilmView_Button, NULL,
								   FILMVIEW_NEXT_PAGE_BUTTON);
	xinpattr_Set_Input_Draw_Function(&button->header, filmview_idraw_FilmView_Button);
	xinpattr_Set_Input_Allign(&button->header, FILMVIEW_INPUT_ALIGN_END, FILMVIEW_INPUT_ALIGN_END);
	button->header.varptr = dialog;
	xrect_Set_Rect(&rect, 0, FILMVIEW_PAGE_COUNTER_TOP, FILMVIEW_PAGE_COUNTER_RIGHT,
				   FILMVIEW_PAGE_COUNTER_BOTTOM);
	input = xinput_Alloc_Input(root, &rect, 0, 0);
	xinpattr_Set_Input_Draw_Function(input, filmview_idraw_FilmView_Page);
	xinpattr_Set_Input_Allign(input, FILMVIEW_INPUT_ALIGN_CENTER, FILMVIEW_INPUT_ALIGN_END);
	input->id = FILMVIEW_LIST_ID;
	if (dialog->directory.count != 0) {
		xrect_Set_Rect(&rect, FILMVIEW_CONTROL_LEFT, FILMVIEW_BUTTON_TOP, FILMVIEW_BUTTON_RIGHT,
					   FILMVIEW_BUTTON_BOTTOM);
		button =
			xbtnpush_Alloc_Button(root, &rect, 0, filmview_iuser_FV_File, "Load Film", FILMVIEW_LOAD_BUTTON);
		xinpattr_Set_Input_Draw_Function(&button->header, filmview_idraw_FV_File);
		xinpattr_Set_Input_Allign(&button->header, FILMVIEW_INPUT_ALIGN_START, FILMVIEW_INPUT_ALIGN_END);
		g_filmviewLoadInput = &button->header;
	} else {
		g_filmviewLoadInput = NULL;
	}
	xrect_Set_Rect(&rect, FILMVIEW_CONTROL_LEFT, FILMVIEW_BUTTON_TOP, FILMVIEW_BUTTON_RIGHT,
				   FILMVIEW_BUTTON_BOTTOM);
	button =
		xbtnpush_Alloc_Button(root, &rect, 0, filmview_iuser_FV_File, "Exit Film Room", FILMVIEW_EXIT_BUTTON);
	xinpattr_Set_Input_Draw_Function(&button->header, filmview_idraw_FV_File);
	xinpattr_Set_Input_Allign(&button->header, FILMVIEW_INPUT_ALIGN_END, FILMVIEW_INPUT_ALIGN_END);
	if (dialog->directory.count != 0) {
		xrect_Set_Rect(&rect, FILMVIEW_CONTROL_LEFT, FILMVIEW_SECOND_BUTTON_TOP, FILMVIEW_BUTTON_RIGHT,
					   FILMVIEW_SECOND_BUTTON_BOTTOM);
		button = xbtnpush_Alloc_Button(root, &rect, 0, filmview_iuser_FV_File, "Delete Film",
									   FILMVIEW_DELETE_BUTTON);
		xinpattr_Set_Input_Draw_Function(&button->header, filmview_idraw_FV_File);
		xinpattr_Set_Input_Allign(&button->header, FILMVIEW_INPUT_ALIGN_START, FILMVIEW_INPUT_ALIGN_END);
		button->header.varptr = dialog;
		g_filmviewDeleteInput = &button->header;
	} else {
		g_filmviewDeleteInput = NULL;
	}
	if (g_ReplayClipName[0] != '\0') {
		xrect_Set_Rect(&rect, FILMVIEW_CONTROL_LEFT, FILMVIEW_SECOND_BUTTON_TOP, FILMVIEW_BUTTON_RIGHT,
					   FILMVIEW_SECOND_BUTTON_BOTTOM);
		button = xbtnpush_Alloc_Button(root, &rect, 0, filmview_iuser_FV_File, "View Last Film",
									   FILMVIEW_LAST_FILM_BUTTON);
		xinpattr_Set_Input_Draw_Function(&button->header, filmview_idraw_FV_File);
		xinpattr_Set_Input_Allign(&button->header, FILMVIEW_INPUT_ALIGN_END, FILMVIEW_INPUT_ALIGN_END);
	}
	*outRoot = root;
	return 1;
}

// FUNCTION: XW 0x4492B0
void filmview_iuser_FilmView_Button(Input* input, int unusedContext) {
	FILMVIEW_FileDialog* dialog = input->varptr;
	(void)unusedContext;
	if (xinpattr_Get_Input_Selected(input) != 0) {
		if (input->id == FILMVIEW_PREVIOUS_PAGE_BUTTON) {
			if (g_filmviewCurrentPage > 0) {
				--g_filmviewCurrentPage;
				xinput_Refresh_System_Inputs();
			} else {
				g_filmviewCurrentPage = g_filmviewPageCount - 1;
			}
		} else if (g_filmviewCurrentPage < g_filmviewPageCount - 1) {
			++g_filmviewCurrentPage;
		} else {
			g_filmviewCurrentPage = 0;
		}
		if (g_filmviewPageCount > 1) {
			dialog->page_start = FILMVIEW_FILES_PER_PAGE * g_filmviewCurrentPage;
			filmview_Set_Active_FV_File(dialog, dialog->page_start, 0);
			xinpattr_Refresh_Input(dialog->root);
		}
	}
}

// FUNCTION: XW 0x449350
void filmview_idraw_FilmView_Button(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	PushButton* button = (PushButton*)input;
	if (refresh) {
		xstyle_Style_Paint_Border(frame, button->pressed);
		xstyle_Style_Draw_Centered_Icon(button->header.id ? iconRightArrow : iconLeftArrow, frame, clip,
										button->pressed);
	}
}

// FUNCTION: XW 0x4493A0
void filmview_idraw_FilmView_Page(Input* unusedInput, Rect* frame, Rect* unusedClip, int16_t refresh) {
	char pageLabel[FILMVIEW_PAGE_LABEL_CAPACITY];
	(void)unusedInput;
	(void)unusedClip;
	if (refresh != 0) {
		xstyle_Style_Paint_TextField(frame);
		sprintf(pageLabel, "Page %d/%d", g_filmviewCurrentPage + 1, g_filmviewPageCount);
		xfont_Print_Centered_Text(pageLabel, frame, FILMVIEW_PAGE_FONT, FILMVIEW_TEXT_COLOR);
	}
}

// FUNCTION: XW 0x449400
void filmview_idraw_FV_File(Input* input, Rect* frame, Rect* unusedClip, int16_t refresh) {
	Rect content;
	(void)unusedClip;
	if (!refresh)
		return;
	xrect_Copy_Rect(&content, frame);
	switch (input->id) {
		case FILMVIEW_TITLE_ID:
			xstyle_Style_Paint_Base(frame, 0);
			xstyle_Style_Trim_Button(&content);
			content.bottom = content.top + FILMVIEW_TITLE_HEIGHT;
			xfont_Print_Centered_Text(input->varptr, &content, 0, FILMVIEW_TITLE_COLOR);
			break;
		case FILMVIEW_LIST_ID: {
			FILMVIEW_FileDialog* dialog;
			const DirEntry* entries;
			int16_t index;
			int16_t rowTop, columnLeft, columnWidth;
			Rect selection;
			char sizeLabel[FILMVIEW_SIZE_LABEL_CAPACITY];
			xstyle_Style_Paint_TextField(frame);
			xstyle_Style_Trim_TextField(&content);
			dialog = input->varptr;
			entries = xmemhdl_Lock_Handle(dialog->directory.entries);
			rowTop = content.top;
			columnLeft = content.left;
			index = 0;
			columnWidth = ((content.right - content.left) >> 1) - 1;
			xpaint_Vert_Clipped_Line(columnWidth + content.left, content.top, content.bottom - content.top,
									 FILMVIEW_SELECTION_COLOR);
			for (; index < dialog->page_start + FILMVIEW_FILES_PER_PAGE && index < dialog->directory.count;
				 index++) {
				if (index >= dialog->page_start) {
					int16_t textColor, width;
					if (index == dialog->selected_file) {
						xrect_Set_Rect(&selection, columnLeft, rowTop, columnLeft + columnWidth,
									   rowTop + FILMVIEW_SELECTION_HEIGHT);
						xpaint_Paint_Clipped_Rect(&selection, FILMVIEW_SELECTION_COLOR);
						textColor = FILMVIEW_SELECTED_TEXT_COLOR;
					} else {
						textColor = FILMVIEW_TEXT_COLOR;
					}
					xfont_Print_Clipped_Text(entries[index].name, columnLeft + FILMVIEW_NAME_INSET,
											 rowTop + 1, 0, textColor);
					sprintf(sizeLabel, "%uK", entries[index].size_kb);
					width = xfont_Get_String_Width_0(0, sizeLabel) + FILMVIEW_SIZE_INSET;
					xfont_Print_Clipped_Text(sizeLabel, columnLeft + columnWidth - width, rowTop + 1, 0,
											 textColor);
					if (index == dialog->page_start + FILMVIEW_FILES_PER_COLUMN - 1) {
						rowTop = content.top;
						columnLeft = columnWidth + content.left + 1;
					} else {
						rowTop += FILMVIEW_ROW_HEIGHT;
					}
				}
			}
			xmemhdl_Unlock_Handle(dialog->directory.entries);
			break;
		}
		case FILMVIEW_FIRST_BUTTON_ID:
		case FILMVIEW_FIRST_BUTTON_ID + 1:
		case FILMVIEW_FIRST_BUTTON_ID + 2:
		case FILMVIEW_LAST_BUTTON_ID: {
			const PushButton* button = (const PushButton*)input;
			const char* labels = button->labels;
			xstyle_Style_Paint_Border(frame, button->pressed);
			xstyle_Style_Button_Text(labels, frame, button->pressed);
			break;
		}
	}
}

// FUNCTION: XW 0x449670
int16_t filmview_iupdate_FV_File(Input* input, Rect* frame, Rect* unusedClip, int16_t key,
								 int unusedLeftEvent, int unusedRightEvent, int16_t x, int16_t y) {
	(void)unusedClip;
	(void)unusedLeftEvent;
	(void)unusedRightEvent;
	if (key != 0) {
		int resultCode;
		switch (input->id) {
			case FILMVIEW_LIST_ID:
				break;
			default:
				return 0;
		}
		if (key == '\r') {
			xdialog_Set_Dialog_Exit(FILMVIEW_ACCEPT_FILE);
			return 1;
		}
		switch (key) {
			case FILMVIEW_KEY_UP:
				g_filmviewFocusIndex -= FILMVIEW_FOCUS_COLUMNS;
				if (g_filmviewFocusIndex < 0)
					g_filmviewFocusIndex += FILMVIEW_FOCUS_COUNT;
				resultCode = 1;
				break;
			case FILMVIEW_KEY_DOWN:
				g_filmviewFocusIndex += FILMVIEW_FOCUS_COLUMNS;
				if (g_filmviewFocusIndex >= FILMVIEW_FOCUS_COUNT)
					g_filmviewFocusIndex -= FILMVIEW_FOCUS_COUNT;
				resultCode = 1;
				break;
			case FILMVIEW_KEY_LEFT:
			case FILMVIEW_KEY_RIGHT:
				g_filmviewFocusIndex ^= 1;
				resultCode = 1;
				break;
			default:
				resultCode = key;
				break;
		}
		xio_Set_Mouse_Position(frame->left + g_filmviewFocusX[g_filmviewFocusIndex],
							   frame->top + g_filmviewFocusY[g_filmviewFocusIndex]);
		return resultCode;
	}
	switch (input->id) {
		case FILMVIEW_LIST_ID:
			filmview_Select_Active_FV_File(input, frame, x, y);
			break;
	}
	return 1;
}

// FUNCTION: XW 0x449780
void filmview_iuser_FV_File(Input* input, int unusedContext) {
	(void)unusedContext;
	switch (input->id) {
		case FILMVIEW_LOAD_BUTTON:
			if (xinpattr_Is_Input_Selected(input))
				xdialog_Set_Dialog_Exit(FILMVIEW_ACCEPT_FILE);
			break;
		case FILMVIEW_EXIT_BUTTON:
			if (xinpattr_Is_Input_Selected(input))
				xdialog_Set_Dialog_Exit(FILMVIEW_CANCEL_FILE);
			break;
		case FILMVIEW_DELETE_BUTTON:
			if (xinpattr_Is_Input_Selected(input) && g_filmviewSelectedName[0] != '\0')
				filmview_Do_Delete_Dialog(XwFilmView_CompleteFileDeletion, input);
			break;
		case FILMVIEW_LAST_FILM_BUTTON:
			if (xinpattr_Is_Input_Selected(input)) {
				xdialog_Set_Dialog_Exit(FILMVIEW_ACCEPT_FILE);
				strcpy(g_filmviewSelectedName, g_ReplayClipName);
			}
			break;
	}
}

// FUNCTION: XW 0x4499D0
void filmview_Select_Active_FV_File(Input* input, Rect* frame, int16_t x, int16_t y) {
	FILMVIEW_FileDialog* dialog = input->varptr;
	int16_t fileIndex = y / FILMVIEW_ROW_HEIGHT;
	fileIndex += dialog->page_start;
	if (x > (frame->right - frame->left) >> 1)
		fileIndex += FILMVIEW_FILES_PER_COLUMN;
	filmview_Set_Active_FV_File(dialog, fileIndex, 1);
}

// FUNCTION: XW 0x449A20
void filmview_Set_Active_FV_File(struct FILMVIEW_FileDialog* dialog, int16_t fileIndex, int16_t hit) {
	int16_t scrollNeeded = 0;
	int16_t selectionChanged;
	if (fileIndex >= dialog->directory.count)
		fileIndex = dialog->directory.count - 1;
	if (fileIndex < 0)
		fileIndex = 0;
	if (dialog->selected_file == fileIndex) {
		if (hit) {
			dialog->hit_count++;
			selectionChanged = 1;
		} else {
			selectionChanged = 0;
		}
	} else {
		dialog->selected_file = fileIndex;
		dialog->hit_count = hit;
		if (fileIndex < dialog->page_start)
			scrollNeeded = 1;
		if (fileIndex >= dialog->page_start + FILMVIEW_FILES_PER_PAGE)
			scrollNeeded++;
		selectionChanged = 1;
	}
	if (scrollNeeded) {
		int16_t pageStart = dialog->selected_file - dialog->selected_file % FILMVIEW_FILES_PER_PAGE;
		if (dialog->page_start != pageStart) {
			dialog->page_start = pageStart;
			g_filmviewCurrentPage = (pageStart + FILMVIEW_FILES_PER_PAGE - 1) / FILMVIEW_FILES_PER_PAGE;
			scrollNeeded++;
		}
	}
	if (selectionChanged) {
		const DirEntry* entries = xmemhdl_Lock_Handle(dialog->directory.entries);
		if (fileIndex < dialog->directory.count) {
			if (dialog->hit_count > 1)
				xdialog_Set_Dialog_Exit(1);
			strcpy(g_filmviewSelectedName, entries[fileIndex].name);
		}
		xmemhdl_Unlock_Handle(dialog->directory.entries);
	}
	if (selectionChanged || scrollNeeded)
		xinpattr_Refresh_Input(dialog->root);
}

// FUNCTION: XW 0x449B70
void filmview_Do_Delete_Dialog(DialogSubResultHandler complete, void* context) {
	int16_t hadKeyButtons = xio_Is_Key_Buttons();
	if (hadKeyButtons == 0) {
		xio_Set_Key_Buttons();
	}
	xio_Set_Mouse_Position(g_FilmViewDeleteMouseX[g_FilmViewDeleteFocus],
						   g_FilmViewDeleteMouseY[g_FilmViewDeleteFocus]);
	XwFilmView_ScheduleDeleteDialog(filmview_Build_Delete_Dialog(), hadKeyButtons, complete, context);
}

// FUNCTION: XW 0x449BE0
Input* filmview_Build_Delete_Dialog(void) {
	Rect rect;
	Input* dialog;
	PushButton* deleteButton;
	PushButton* cancelButton;
	xrect_Set_Rect(&rect, 0, 0, FILMVIEW_DELETE_DIALOG_WIDTH, FILMVIEW_DELETE_DIALOG_HEIGHT);
	dialog = xinput_Alloc_Dialog_Input(NULL, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(dialog, filmview_iupdate_Delete_Input);
	xinpattr_Set_Input_Draw_Function(dialog, filmview_idraw_Delete_Input);
	xinpattr_Set_Input_Allign(dialog, FILMVIEW_INPUT_ALIGN_CENTER, FILMVIEW_INPUT_ALIGN_CENTER);
	xinpattr_Show_Input(dialog);
	dialog->id = FILMVIEW_DELETE_DIALOG_ID;
	xrect_Set_Rect(&rect, FILMVIEW_DELETE_BUTTON_INSET, FILMVIEW_DELETE_BUTTON_INSET,
				   FILMVIEW_DELETE_BUTTON_RIGHT, FILMVIEW_DELETE_BUTTON_BOTTOM);
	deleteButton = xbtnpush_Alloc_Button(dialog, &rect, 0, filmview_iuser_Delete_Input, g_filmviewDeleteLabel,
										 FILMVIEW_DELETE_CONFIRM);
	xinpattr_Set_Input_Allign(&deleteButton->header, FILMVIEW_INPUT_ALIGN_START, FILMVIEW_INPUT_ALIGN_END);
	xrect_Set_Rect(&rect, FILMVIEW_DELETE_BUTTON_INSET, FILMVIEW_DELETE_BUTTON_INSET,
				   FILMVIEW_DELETE_BUTTON_RIGHT, FILMVIEW_DELETE_BUTTON_BOTTOM);
	cancelButton =
		xbtnpush_Alloc_Button(dialog, &rect, 0, filmview_iuser_Delete_Input, "Exit", FILMVIEW_DELETE_CANCEL);
	xinpattr_Set_Input_Allign(&cancelButton->header, FILMVIEW_INPUT_ALIGN_END, FILMVIEW_INPUT_ALIGN_END);
	return dialog;
}

// FUNCTION: XW 0x449CD0
int16_t filmview_iupdate_Delete_Input(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
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
		if (key == FILMVIEW_KEY_LEFT || key == FILMVIEW_KEY_UP || key == FILMVIEW_KEY_RIGHT ||
			key == FILMVIEW_KEY_DOWN) {
			g_FilmViewDeleteFocus ^= 1;
			xio_Set_Mouse_Position(g_FilmViewDeleteMouseX[g_FilmViewDeleteFocus],
								   g_FilmViewDeleteMouseY[g_FilmViewDeleteFocus]);
			return 1;
		}
	}
	return 0;
}

// FUNCTION: XW 0x449D60
void filmview_iuser_Delete_Input(Input* input, int context) {
	(void)context;
	if (xinpattr_Get_Input_Selected(input)) {
		int16_t id = input->id;
		switch (id) {
			case FILMVIEW_DELETE_CONFIRM:
			case FILMVIEW_DELETE_CANCEL:
				xdialog_Set_Dialog_Exit(id);
				break;
		}
	}
}

// FUNCTION: XW 0x449D90
void filmview_idraw_Delete_Input(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	Rect titleRect;
	char title[FILMVIEW_DELETE_TITLE_CAPACITY];
	(void)clip;
	if (refresh != 0) {
		strcpy(title, g_filmviewDeleteLabel);
		strcat(title, " ");
		strcat(title, g_filmviewSelectedName);
		strcat(title, "?");
		if (input->id == FILMVIEW_DELETE_DIALOG_ID) {
			xrect_Copy_Rect(&titleRect, frame);
			xpaint_Frame_Clipped_Rect(&titleRect, FILMVIEW_DELETE_FRAME_COLOR);
			xrect_Inset_Rect(&titleRect, FILMVIEW_DELETE_FRAME_INSET, FILMVIEW_DELETE_FRAME_INSET);
			xstyle_Style_Paint_Border(&titleRect, 0);
			titleRect.bottom = titleRect.top + FILMVIEW_DELETE_TITLE_HEIGHT;
			xfont_Enable_FontID_Shadow(FILMVIEW_DELETE_TITLE_FONT);
			xfont_Print_Centered_Text(title, &titleRect, FILMVIEW_DELETE_TITLE_FONT, FILMVIEW_TEXT_COLOR);
			xfont_Disable_FontID_Shadow(FILMVIEW_DELETE_TITLE_FONT);
		}
	}
}

// FUNCTION: XW 0x44B170
void filmview_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_filmviewMusicState.film = sceneFilm;
		g_filmviewMusicState.activeSound = xsound_Find_Gmid(g_filmviewMusicName);
		if (g_filmviewMusicState.activeSound == NULL) {
			ResFile* musicResource = xres_Open_Resource(g_filmviewMusicFilename);
			g_filmviewMusicState.activeSound = xsound_Res_Music(musicResource, g_filmviewMusicName);
			soundext_Start_Resource_Sound(g_filmviewMusicState.activeSound);
			xres_Close_Resource(musicResource);
		}
		xsound_Set_Sound_Keep(g_filmviewMusicState.activeSound);
		xsound_Set_Sound_User_Function(g_filmviewMusicState.activeSound, Cutscene_IgnoreSoundEvent);
		g_filmviewMusicState.transitionSound = xsound_Find_Gmid(g_filmviewTransitionMusicName);
		if (g_filmviewMusicState.transitionSound != NULL &&
			soundext_Count_Resource_Instances(g_filmviewMusicState.transitionSound) == 1) {
			xsound_Set_Sound_Keep(g_filmviewMusicState.transitionSound);
		}
	}
}

// FUNCTION: XW 0x44B230
void filmview_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0 && xerror_Get_Landru_Exit() != XW_SCENE_TECH_ROOM) {
		if (soundext_Count_Resource_Instances(g_filmviewMusicState.activeSound) != 1) {
			if (g_filmviewMusicState.transitionSound != NULL)
				soundext_SetHook(g_filmviewMusicState.transitionSound, XW_SOUND_CONTROL_DIRECT,
								 FILMVIEW_TRANSITION_MUSIC_CONTROL, 0);
		} else {
			Sound* music;
#ifdef XW_MODERN
			Dos94_soundext_SetPriority(g_filmviewMusicState.activeSound, 0);
#else
			soundext_SetPriority(0, 0);
#endif
			music = g_filmviewMusicState.activeSound;
			soundext_FadeVolume(music, 0, FILMVIEW_MUSIC_FADE_DURATION);
			if (g_filmviewMusicState.transitionSound != NULL) {
				xsound_Clear_Sound_Keep(g_filmviewMusicState.transitionSound);
				music = g_filmviewMusicState.transitionSound;
				xsound_Free_Sound(music);
			}
		}
		soundext_ClearTriggers();
	}
}
