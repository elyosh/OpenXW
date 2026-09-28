#include "xw_dos94/frontend/filmview.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/port.h"
#endif
#include "xw/frontend/filmview.h"

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

static const int16_t delete_x[2] = { 112, 214 };
static const int16_t delete_y[2] = { 110, 110 };

/* DOS94 0x4c0000. */
XwShellSceneResult Dos94_filmview_FilmView(struct XwShellContext* shell) {
	ResFile* resourceFile;
	Rect screenRect;
	g_filmviewPageCount = 1;
	g_filmviewCurrentPage = 0;
	resourceFile = xres_Open_Resource("filmview.lfd");
	xrect_Set_Rect(&screenRect, 0, 0, 320, 200);
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
			g_filmviewSceneFilm = xfilm_Res_Film(resourceFile, "filmview", &screenRect, 0, 0, 0);
			break;
		case SHIPEXT_MISSION_OUTCOME_LEGACY_15:
		case SHIPEXT_MISSION_OUTCOME_LEGACY_17:
			g_filmviewSceneFilm = xfilm_Res_Film(resourceFile, "filmload", &screenRect, 0, 0, 0);
			break;
	}
	xfilm_Set_Film_Def_Palette(g_filmviewSceneFilm, shell->standardPalette);
	xview_Set_View_Update_Function(Dos94_filmview_end_View);
	filmview_OpenMusic(resourceFile, g_filmviewSceneFilm);
	soundext_RecheckSfxPreference();
	XwFilmView_RunView(resourceFile);
}

/* DOS94 0x4c01d0. */
void Dos94_filmview_Do_FV_File_Dialog(DialogSubResultHandler complete, void* context) {
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
	built = Dos94_filmview_Build_FV_File_Dialog(&root, dialog, "Load Mission Film");
	dialog->root = root;
	if (dialog->directory.count != 0) {
		filmview_Set_Active_FV_File(dialog, 0, 1);
		dialog->hit_count = 0;
	}
	XwFilmView_ScheduleFileDialog(built);
}

/* DOS94 0x4c02d2. */
int16_t Dos94_filmview_Build_FV_File_Dialog(Input** outRoot, struct FILMVIEW_FileDialog* dialog,
											const char* title) {
	Rect rect;
	Input* root;
	Input* input;
	PushButton* button;
	xrect_Set_Rect(&rect, 0, 14, 180, 148);
	root = xinput_Alloc_Dialog_Input(NULL, &rect, 0, 0);
	xinpattr_Set_Input_Draw_Function(root, Dos94_filmview_idraw_FV_File);
	xinpattr_Set_Input_Allign(root, FILMVIEW_INPUT_ALIGN_CENTER, FILMVIEW_INPUT_ALIGN_START);
	root->varptr = (void*)title;
	root->id = FILMVIEW_TITLE_ID;
	xrect_Set_Rect(&rect, 4, 12, 176, 77);
	input = xinput_Alloc_Dialog_Input(root, &rect, 0, 0);
	xinpattr_Set_Input_Draw_Function(input, Dos94_filmview_idraw_FV_File);
	xinpattr_Set_Input_Update_Function(input, Dos94_filmview_iupdate_FV_File);
	xinpattr_Set_Input_User_Function(input, Dos94_filmview_iuser_FV_File);
	input->mouseUsage = FILMVIEW_LIST_MOUSE_USAGE;
	input->varptr = dialog;
	input->id = FILMVIEW_LIST_ID;
	xrect_Set_Rect(&rect, 4, 36, 18, 52);
	button = xbtnpush_Alloc_Button(root, &rect, 0, filmview_iuser_FilmView_Button, NULL,
								   FILMVIEW_PREVIOUS_PAGE_BUTTON);
	xinpattr_Set_Input_Draw_Function(&button->header, filmview_idraw_FilmView_Button);
	xinpattr_Set_Input_Allign(&button->header, FILMVIEW_INPUT_ALIGN_START, FILMVIEW_INPUT_ALIGN_END);
	button->header.varptr = dialog;
	xrect_Set_Rect(&rect, 4, 36, 18, 52);
	button = xbtnpush_Alloc_Button(root, &rect, 0, filmview_iuser_FilmView_Button, NULL,
								   FILMVIEW_NEXT_PAGE_BUTTON);
	xinpattr_Set_Input_Draw_Function(&button->header, filmview_idraw_FilmView_Button);
	xinpattr_Set_Input_Allign(&button->header, FILMVIEW_INPUT_ALIGN_END, FILMVIEW_INPUT_ALIGN_END);
	button->header.varptr = dialog;
	xrect_Set_Rect(&rect, 0, 37, 140, 51);
	input = xinput_Alloc_Input(root, &rect, 0, 0);
	xinpattr_Set_Input_Draw_Function(input, Dos94_filmview_idraw_FilmView_Page);
	xinpattr_Set_Input_Allign(input, FILMVIEW_INPUT_ALIGN_CENTER, FILMVIEW_INPUT_ALIGN_END);
	input->id = FILMVIEW_LIST_ID;
	if (dialog->directory.count != 0) {
		xrect_Set_Rect(&rect, 4, 4, 88, 18);
		button = xbtnpush_Alloc_Button(root, &rect, 0, Dos94_filmview_iuser_FV_File, "Load Film",
									   FILMVIEW_LOAD_BUTTON);
		xinpattr_Set_Input_Draw_Function(&button->header, Dos94_filmview_idraw_FV_File);
		xinpattr_Set_Input_Allign(&button->header, FILMVIEW_INPUT_ALIGN_START, FILMVIEW_INPUT_ALIGN_END);
		g_filmviewLoadInput = &button->header;
	} else {
		g_filmviewLoadInput = NULL;
	}
	xrect_Set_Rect(&rect, 4, 4, 88, 18);
	button = xbtnpush_Alloc_Button(root, &rect, 0, Dos94_filmview_iuser_FV_File, "Exit Film Room",
								   FILMVIEW_EXIT_BUTTON);
	xinpattr_Set_Input_Draw_Function(&button->header, Dos94_filmview_idraw_FV_File);
	xinpattr_Set_Input_Allign(&button->header, FILMVIEW_INPUT_ALIGN_END, FILMVIEW_INPUT_ALIGN_END);
	if (dialog->directory.count != 0) {
		xrect_Set_Rect(&rect, 4, 20, 88, 34);
		button = xbtnpush_Alloc_Button(root, &rect, 0, Dos94_filmview_iuser_FV_File, "Delete Film",
									   FILMVIEW_DELETE_BUTTON);
		xinpattr_Set_Input_Draw_Function(&button->header, Dos94_filmview_idraw_FV_File);
		xinpattr_Set_Input_Allign(&button->header, FILMVIEW_INPUT_ALIGN_START, FILMVIEW_INPUT_ALIGN_END);
		button->header.varptr = dialog;
		g_filmviewDeleteInput = &button->header;
	} else {
		g_filmviewDeleteInput = NULL;
	}
	if (g_ReplayClipName[0] != '\0') {
		xrect_Set_Rect(&rect, 4, 20, 88, 34);
		button = xbtnpush_Alloc_Button(root, &rect, 0, Dos94_filmview_iuser_FV_File, "View Last Film",
									   FILMVIEW_LAST_FILM_BUTTON);
		xinpattr_Set_Input_Draw_Function(&button->header, Dos94_filmview_idraw_FV_File);
		xinpattr_Set_Input_Allign(&button->header, FILMVIEW_INPUT_ALIGN_END, FILMVIEW_INPUT_ALIGN_END);
	}
	*outRoot = root;
	return 1;
}

/* DOS94 0x4c0892. */
void Dos94_filmview_idraw_FilmView_Page(Input* unusedInput, Rect* frame, Rect* unusedClip, int16_t refresh) {
	char pageLabel[FILMVIEW_PAGE_LABEL_CAPACITY];
	(void)unusedInput;
	(void)unusedClip;
	if (refresh != 0) {
		xpaint_Paint_Clipped_Bevel(frame, 38, 26, 0, 1);
		sprintf(pageLabel, "Page %d/%d", g_filmviewCurrentPage + 1, g_filmviewPageCount);
		xfont_Print_Centered_Text(pageLabel, frame, 1, FILMVIEW_TEXT_COLOR);
	}
}

/* DOS94 0x4c08e8. */
void Dos94_filmview_idraw_FV_File(Input* input, Rect* frame, Rect* unusedClip, int16_t refresh) {
	Rect content;
	(void)unusedClip;
	if (!refresh)
		return;
	xrect_Copy_Rect(&content, frame);
	switch (input->id) {
		case FILMVIEW_TITLE_ID:
			xstyle_Style_Paint_Base(frame, 0);
			xstyle_Style_Trim_Button(&content);

			xfont_Print_Clipped_Text(input->varptr, content.left + 35, content.top + 1, 0,
									 FILMVIEW_TITLE_COLOR);
			break;
		case FILMVIEW_LIST_ID: {
			FILMVIEW_FileDialog* dialog;
			const DirEntry* entries;
			int16_t index;
			int16_t rowTop, columnLeft, columnWidth;
			Rect selection;
			char sizeLabel[FILMVIEW_SIZE_LABEL_CAPACITY];
			xpaint_Paint_Clipped_Bevel(frame, 38, 26, 0, 1);
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
						xrect_Set_Rect(&selection, columnLeft, rowTop, columnLeft + columnWidth, rowTop + 7);
						xpaint_Paint_Clipped_Rect(&selection, FILMVIEW_SELECTION_COLOR);
						textColor = FILMVIEW_SELECTED_TEXT_COLOR;
					} else {
						textColor = FILMVIEW_TEXT_COLOR;
					}
					xfont_Print_Clipped_Text(entries[index].name, columnLeft + FILMVIEW_NAME_INSET,
											 rowTop + 1, 1, textColor);
					sprintf(sizeLabel, "%uK", (unsigned int)(uint16_t)entries[index].size_kb);
					width = xfont_Get_String_Width_0(1, sizeLabel) + FILMVIEW_SIZE_INSET;
					xfont_Print_Clipped_Text(sizeLabel, columnLeft + columnWidth - width, rowTop + 1, 1,
											 textColor);
					if (index == dialog->page_start + FILMVIEW_FILES_PER_COLUMN - 1) {
						rowTop = content.top;
						columnLeft = columnWidth + content.left + 1;
					} else {
						rowTop += 8;
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

/* DOS94 0x4c0b6e. */
int16_t Dos94_filmview_iupdate_FV_File(Input* input, Rect* frame, Rect* unusedClip, int16_t key,
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
				return 0;
		}
		xio_Set_Mouse_Position(frame->left + g_filmviewFocusX[g_filmviewFocusIndex],
							   frame->top + g_filmviewFocusY[g_filmviewFocusIndex]);
		return resultCode;
	}
	switch (input->id) {
		case FILMVIEW_LIST_ID:
			Dos94_filmview_Select_Active_FV_File(input, frame, x, y);
			break;
	}
	return 1;
}

/* DOS94 0x4c0c70. */
void Dos94_filmview_iuser_FV_File(Input* input, int unusedContext) {
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
				Dos94_filmview_Do_Delete_Dialog(XwFilmView_CompleteFileDeletion, input);
			break;
		case FILMVIEW_LAST_FILM_BUTTON:
			if (xinpattr_Is_Input_Selected(input)) {
				xdialog_Set_Dialog_Exit(FILMVIEW_ACCEPT_FILE);
				strcpy(g_filmviewSelectedName, g_ReplayClipName);
			}
			break;
	}
}

/* DOS94 0x4c0ec0. */
void Dos94_filmview_Select_Active_FV_File(Input* input, Rect* frame, int16_t x, int16_t y) {
	FILMVIEW_FileDialog* dialog = input->varptr;
	int16_t fileIndex = y / 8;
	fileIndex += dialog->page_start;
	if (x > (frame->right - frame->left) >> 1)
		fileIndex += FILMVIEW_FILES_PER_COLUMN;
	filmview_Set_Active_FV_File(dialog, fileIndex, 1);
}

/* DOS94 0x4c105a. */
void Dos94_filmview_Do_Delete_Dialog(DialogSubResultHandler complete, void* context) {
	int16_t hadKeyButtons = xio_Is_Key_Buttons();
	if (hadKeyButtons == 0) {
		xio_Set_Key_Buttons();
	}
	xio_Set_Mouse_Position(delete_x[g_FilmViewDeleteFocus], delete_y[g_FilmViewDeleteFocus]);
	XwFilmView_ScheduleDeleteDialog(Dos94_filmview_Build_Delete_Dialog(), hadKeyButtons, complete, context);
}

/* DOS94 0x4c10d8. */
Input* Dos94_filmview_Build_Delete_Dialog(void) {
	Rect rect;
	Input* dialog;
	PushButton* deleteButton;
	PushButton* cancelButton;
	xrect_Set_Rect(&rect, 0, 0, FILMVIEW_DELETE_DIALOG_WIDTH, FILMVIEW_DELETE_DIALOG_HEIGHT);
	dialog = xinput_Alloc_Dialog_Input(NULL, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(dialog, Dos94_filmview_iupdate_Delete_Input);
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

/* DOS94 0x4c120e. */
int16_t Dos94_filmview_iupdate_Delete_Input(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
											int rightEvent, int16_t x, int16_t y) {
	(void)input;
	(void)frame;
	(void)clip;
	(void)leftEvent;
	(void)rightEvent;
	(void)x;
	(void)y;
	if (key != 0) {
		if (key == FILMVIEW_KEY_LEFT || key == FILMVIEW_KEY_UP || key == FILMVIEW_KEY_RIGHT ||
			key == FILMVIEW_KEY_DOWN) {
			g_FilmViewDeleteFocus ^= 1;
			xio_Set_Mouse_Position(delete_x[g_FilmViewDeleteFocus], delete_y[g_FilmViewDeleteFocus]);
			return 1;
		}
	}
	return 0;
}

/* DOS94 0x4c0146. The native dialog continuation resumes this same callback. */
void Dos94_filmview_end_View(int time) {
	(void)time;
	if (g_filmviewSceneFilm->cur_cel == g_filmviewSceneFilm->cels - 1) {
		if (shipext_Get_Mission_Outcome() == 17) {
			xerror_Set_Landru_Exit(shellext_OpenOptionsAndSave());
		} else {
			int accepted = XwFilmView_HandleFileDialog();
			if (accepted < 0)
				return;
			if (accepted && g_filmviewSelectedName[0])
				strcpy(g_ReplayClipName, g_filmviewSelectedName);
			else
				xerror_Set_Landru_Exit(g_filmviewReturnToConcourse ? 30 : 95);
		}
	}
	if (g_filmviewSceneFilm->cur_cel == g_filmviewSceneFilm->cels)
		xerror_Set_Landru_Exit(350);
}
