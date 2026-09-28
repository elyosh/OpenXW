#ifndef XW_FRONTEND_FILMVIEW_H
#define XW_FRONTEND_FILMVIEW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/frontend/scenes/cutscene.h"
#include "xw/landru_config.h"

#include <landru/btnpush.h>
#include <landru/dialog.h>
#include <landru/filedir.h>
#include <landru/film.h>
#include <landru/input.h>
#include <landru/rect.h>
#include <landru/res.h>
#include <stddef.h>
#include <stdint.h>
#include <xw/frontend/shell.h>

enum {
	FILMVIEW_SCENE_WIDTH = 640,
	FILMVIEW_SCENE_HEIGHT = 480,
	FILMVIEW_FILM_NAME_CAPACITY = 9,
	FILMVIEW_FILES_PER_PAGE = 16,
	FILMVIEW_FILES_PER_COLUMN = 8,
	FILMVIEW_SELECTED_NAME_CAPACITY = 16,
	FILMVIEW_TITLE_ID = 0,
	FILMVIEW_LIST_ID = 1,
	FILMVIEW_FIRST_BUTTON_ID = 2,
	FILMVIEW_LAST_BUTTON_ID = 5,
	FILMVIEW_TITLE_HEIGHT = 26,
	FILMVIEW_TITLE_COLOR = 26,
	FILMVIEW_ROW_HEIGHT = 16,
	FILMVIEW_SELECTION_HEIGHT = 14,
	FILMVIEW_SELECTION_COLOR = 49,
	FILMVIEW_SELECTED_TEXT_COLOR = 16,
	FILMVIEW_TEXT_COLOR = 15,
	FILMVIEW_NAME_INSET = 4,
	FILMVIEW_SIZE_INSET = 8,
	FILMVIEW_SIZE_LABEL_CAPACITY = 16,
	FILMVIEW_PAGE_LABEL_CAPACITY = 32,
	FILMVIEW_PAGE_FONT = 0
};

enum { FILMVIEW_FOCUS_COUNT = 22, FILMVIEW_FOCUS_COLUMNS = 2, FILMVIEW_ACCEPT_FILE = 1 };

enum {
	FILMVIEW_LOAD_BUTTON = 2,
	FILMVIEW_EXIT_BUTTON = 3,
	FILMVIEW_DELETE_BUTTON = 4,
	FILMVIEW_LAST_FILM_BUTTON = 5,
	FILMVIEW_CANCEL_FILE = 2,
	FILMVIEW_CLIP_PATH_CAPACITY = 256
};

enum { FILMVIEW_DELETE_CONFIRM = 1, FILMVIEW_DELETE_CANCEL = 2 };

enum {
	FILMVIEW_DELETE_DIALOG_ID = 0,
	FILMVIEW_DELETE_DIALOG_WIDTH = 180,
	FILMVIEW_DELETE_DIALOG_HEIGHT = 46,
	FILMVIEW_DELETE_TITLE_CAPACITY = 32,
	FILMVIEW_DELETE_LABEL_CAPACITY = 12,
	FILMVIEW_DELETE_TITLE_HEIGHT = 14,
	FILMVIEW_DELETE_TITLE_FONT = 0,
	FILMVIEW_DELETE_FRAME_COLOR = 16,
	FILMVIEW_DELETE_FRAME_INSET = 1,
	FILMVIEW_DELETE_BUTTON_INSET = 4,
	FILMVIEW_DELETE_BUTTON_RIGHT = 76,
	FILMVIEW_DELETE_BUTTON_BOTTOM = 20,
	FILMVIEW_INPUT_ALIGN_START = 0,
	FILMVIEW_INPUT_ALIGN_CENTER = 1,
	FILMVIEW_INPUT_ALIGN_END = 2
};

enum { FILMVIEW_PREVIOUS_PAGE_BUTTON = 0, FILMVIEW_NEXT_PAGE_BUTTON = 1 };

enum {
	FILMVIEW_DIALOG_TOP = 28,
	FILMVIEW_DIALOG_RIGHT = 360,
	FILMVIEW_DIALOG_BOTTOM = 296,
	FILMVIEW_CONTROL_LEFT = 8,
	FILMVIEW_LIST_TOP = 24,
	FILMVIEW_LIST_RIGHT = 352,
	FILMVIEW_LIST_BOTTOM = 154,
	FILMVIEW_PAGE_BUTTON_TOP = 72,
	FILMVIEW_PAGE_BUTTON_RIGHT = 36,
	FILMVIEW_PAGE_BUTTON_BOTTOM = 104,
	FILMVIEW_PAGE_COUNTER_TOP = 74,
	FILMVIEW_PAGE_COUNTER_RIGHT = 280,
	FILMVIEW_PAGE_COUNTER_BOTTOM = 102,
	FILMVIEW_BUTTON_TOP = 8,
	FILMVIEW_BUTTON_RIGHT = 176,
	FILMVIEW_BUTTON_BOTTOM = 36,
	FILMVIEW_SECOND_BUTTON_TOP = 40,
	FILMVIEW_SECOND_BUTTON_BOTTOM = 68,
	FILMVIEW_LIST_MOUSE_USAGE = 1
};

enum {
	FILMVIEW_DELETE_TARGET_COUNT = 2,
	FILMVIEW_KEY_LEFT = 0x4B00,
	FILMVIEW_KEY_UP = 0x4800,
	FILMVIEW_KEY_RIGHT = 0x4D00,
	FILMVIEW_KEY_DOWN = 0x5000
};

extern const char g_filmviewRoomFilmName[FILMVIEW_FILM_NAME_CAPACITY];
extern const char g_filmviewAlternateFilmName[FILMVIEW_FILM_NAME_CAPACITY];
extern Film* g_filmviewSceneFilm;

extern char g_filmviewDeleteLabel[FILMVIEW_DELETE_LABEL_CAPACITY];
extern const int16_t g_filmviewFocusX[FILMVIEW_FOCUS_COUNT];
extern const int16_t g_filmviewFocusY[FILMVIEW_FOCUS_COUNT];
extern int16_t g_FilmViewDeleteMouseX[FILMVIEW_DELETE_TARGET_COUNT];
extern int16_t g_FilmViewDeleteMouseY[FILMVIEW_DELETE_TARGET_COUNT];

enum { FILMVIEW_MUSIC_FADE_DURATION = 300, FILMVIEW_TRANSITION_MUSIC_CONTROL = 3 };

extern const char* g_filmviewMusicName;
extern const char* g_filmviewMusicFilename;
extern const char* g_filmviewTransitionMusicName;
extern Input* g_filmviewDeleteInput;
extern int16_t g_filmviewPageCount;
extern Input* g_filmviewLoadInput;
extern int16_t g_filmviewCurrentPage;
extern int16_t g_filmviewReturnToConcourse;
extern int16_t g_filmviewFocusIndex;
extern char g_filmviewSelectedName[FILMVIEW_SELECTED_NAME_CAPACITY];
extern int16_t g_FilmViewDeleteFocus;
extern XwCutsceneMusicTransitionState g_filmviewMusicState;

struct XwShellContext;
typedef struct FILMVIEW_FileDialog FILMVIEW_FileDialog;

/* Original IDB size: 66 bytes. */
struct FILMVIEW_FileDialog {
	/* IDB +0x0 */
	Directory directory;
	/* IDB +0x2E: Root input retained by Do_FV_File_Dialog; refreshed when file selection changes. */
	Input* root;
	/* IDB +0x32 */
	uint8_t gap_32[8];
	/* IDB +0x3A: First entry on current 16-file page. */
	int16_t page_start;
	/* IDB +0x3C: Selected directory entry; clamped to valid range by Set_Active_FV_File. */
	int16_t selected_file;
	/* IDB +0x3E: Repeated hits on the selected entry; >1 accepts it. */
	int16_t hit_count;
	/* IDB +0x40: Initialized to 1 when the dialog opens; precise purpose unproven. */
	int16_t field_40;
};

/* Declarations follow ascending original IDB address. */

/* 0x448CC0 */
XwShellSceneResult filmview_FilmView(struct XwShellContext* shell);

/* 0x448DF0 */
void filmview_end_View(int unusedTime);

/* 0x448E60 */
/* Schedules the modal and reports the original boolean result on resume. */
void filmview_Do_FV_File_Dialog(DialogSubResultHandler complete, void* context);

/* 0x448F60 */
int16_t filmview_Build_FV_File_Dialog(Input** outRoot, struct FILMVIEW_FileDialog* dialog, const char* title);

/* 0x4492B0 */
void filmview_iuser_FilmView_Button(Input* input, int unusedContext);

/* 0x449350 */
void filmview_idraw_FilmView_Button(Input* input, Rect* frame, Rect* clip, int16_t refresh);

/* 0x4493A0 */
void filmview_idraw_FilmView_Page(Input* unusedInput, Rect* frame, Rect* unusedClip, int16_t refresh);

/* 0x449400 */
void filmview_idraw_FV_File(Input* input, Rect* frame, Rect* unusedClip, int16_t refresh);

/* 0x449670 */
int16_t filmview_iupdate_FV_File(Input* input, Rect* frame, Rect* unusedClip, int16_t key,
								 int unusedLeftEvent, int unusedRightEvent, int16_t x, int16_t y);

/* 0x449780 */
void filmview_iuser_FV_File(Input* input, int unusedContext);

/* 0x4499D0 */
void filmview_Select_Active_FV_File(Input* input, Rect* frame, int16_t x, int16_t y);

/* 0x449A20 */
void filmview_Set_Active_FV_File(struct FILMVIEW_FileDialog* dialog, int16_t fileIndex, int16_t hit);

/* 0x449B70 */
/* Schedules the modal. The completion callback receives the original boolean result on resume. */
void filmview_Do_Delete_Dialog(DialogSubResultHandler complete, void* context);

/* 0x449BE0 */
Input* filmview_Build_Delete_Dialog(void);

/* 0x449CD0 */
int16_t filmview_iupdate_Delete_Input(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
									  int rightEvent, int16_t x, int16_t y);

/* 0x449D60 */
void filmview_iuser_Delete_Input(Input* input, int context);

/* 0x449D90 */
void filmview_idraw_Delete_Input(Input* input, Rect* frame, Rect* clip, int16_t refresh);

/* 0x44B170 */
void filmview_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x44B230 */
void filmview_CloseMusic(void);

#ifdef __cplusplus
}
#endif

#endif
