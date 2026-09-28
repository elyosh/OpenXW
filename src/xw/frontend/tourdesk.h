#ifndef XW_FRONTEND_TOURDESK_H
#define XW_FRONTEND_TOURDESK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/frontend/register.h"

#include "xw/landru_config.h"

#include <landru/actor.h>
#include <landru/btnpush.h>
#include <landru/film.h>
#include <landru/input.h>
#include <landru/rect.h>
#include <landru/sound.h>
#include <stddef.h>
#include <stdint.h>
#include <xw/frontend/shell.h>

struct XwShellContext;

enum {
	XW_TOURDESK_INITIAL_FOCUS = 2,
	XW_TOURDESK_INITIAL_MOUSE_X = 208,
	XW_TOURDESK_INITIAL_MOUSE_Y = 128,
	XW_TOURDESK_EXTRA_TOUR_FIRST = 3,
	XW_TOURDESK_EXTRA_TOUR_SECOND = 4,
	XW_TOURDESK_EXTRA_TOUR_THIRD = 5,
	XW_TOURDESK_WIDTH = 640,
	XW_TOURDESK_HEIGHT = 480,
	XW_TOURDESK_EXIT_RIGHT = 60,
	XW_TOURDESK_ENTER_LEFT = 400,
	XW_TOURDESK_ENTER_INPUT = 1,
	XW_TOURDESK_HINT_LEFT = 44,
	XW_TOURDESK_HINT_TOP = 328,
	XW_TOURDESK_HINT_RIGHT = 391,
	XW_TOURDESK_HINT_BOTTOM = 424,
	XW_TOURDESK_PREVIOUS_LEFT = 125,
	XW_TOURDESK_PREVIOUS_RIGHT = 163,
	XW_TOURDESK_NEXT_LEFT = 297,
	XW_TOURDESK_NEXT_RIGHT = 332,
	XW_TOURDESK_BUTTON_TOP = 444,
	XW_TOURDESK_BUTTON_BOTTOM = 479,
	XW_TOURDESK_TITLE_LEFT = 168,
	XW_TOURDESK_TITLE_TOP = 452,
	XW_TOURDESK_TITLE_RIGHT = 282,
	XW_TOURDESK_TITLE_BOTTOM = 465,
	XW_TOURDESK_DESCRIPTION_LEFT = 60,
	XW_TOURDESK_DESCRIPTION_TOP = 340,
	XW_TOURDESK_DESCRIPTION_RIGHT = 370,
	XW_TOURDESK_DESCRIPTION_BOTTOM = 410,
	XW_TOURDESK_DESCRIPTION_INPUT = 1
};

extern Film* g_tourDeskFilm;
extern Actor* g_tourDeskRobotActor;
extern Input* g_tourDeskEnterInput;

enum {
	XW_TOURDESK_MUSIC_CLOSE_PRIORITY = 126,
	XW_TOURDESK_MUSIC_FADE_DURATION = 180,
	XW_TOURDESK_MUSIC_OPEN_VOLUME = 95,
	XW_TOURDESK_MUSIC_OPEN_DURATION = 300
};

enum { XW_TOURDESK_PILOT_FILENAME_CAPACITY = 32, XW_TOURDESK_PILOT_PATH_CAPACITY = 256 };

enum XwTourDeskDoorSoundAction { XW_TOURDESK_DOOR_OPEN = 1, XW_TOURDESK_DOOR_CLOSE = 2 };

enum { XW_TOURDESK_DOOR_CLOSED_FRAME = 0, XW_TOURDESK_DOOR_CLOSE_SOUND_FRAME = 1 };

enum { XW_TOURDESK_PREVIOUS_BUTTON = 0, XW_TOURDESK_NEXT_BUTTON = 1 };

enum { XW_TOURDESK_SPEECH_COUNT = 2 };

enum { XW_TOURDESK_FOCUS_ROWS = 1, XW_TOURDESK_FOCUS_COLUMNS = 4, XW_TOURDESK_WELCOME_TIME = 2 };

enum { XW_TOURDESK_SPEECH_WELCOME = 0, XW_TOURDESK_SPEECH_JOIN = 1 };

enum {
	XW_TOURDESK_HINT_CAPACITY = 32,
	XW_TOURDESK_HINT_BACKGROUND = 131,
	XW_TOURDESK_HINT_FONT = 3,
	XW_TOURDESK_HINT_SHADOW = 16,
	XW_TOURDESK_HINT_COLOR = 17,
	XW_TOURDESK_HINT_SHADOW_OFFSET = 1
};

enum {
	XW_TOURDESK_TITLE_INPUT = 0,
	XW_TOURDESK_TITLE_BACKGROUND = 16,
	XW_TOURDESK_DESCRIPTION_BACKGROUND = 131,
	XW_TOURDESK_TOUR_TITLE_COLOR = 18,
	XW_TOURDESK_TOUR_DESCRIPTION_COLOR = 17,
	XW_TOURDESK_REPLAY_TITLE_COLOR = 14,
	XW_TOURDESK_REPLAY_DESCRIPTION_COLOR = 2,
	XW_TOURDESK_TITLE_FONT = 0,
	XW_TOURDESK_DESCRIPTION_FONT = 3,
	XW_TOURDESK_TOUR_NAMES_PARAGRAPH = 0,
	XW_TOURDESK_REPLAY_NAMES_PARAGRAPH = 1,
	XW_TOURDESK_DESCRIPTION_PARAGRAPH_BASE = 2,
	XW_TOURDESK_DESCRIPTION_TOP_OFFSET = 3,
	XW_TOURDESK_DESCRIPTION_LINE_HEIGHT = 20,
	XW_TOURDESK_DESCRIPTION_LINE_INDENT = 2,
	XW_TOURDESK_TEXT_CAPACITY = 64,
	XW_TOURDESK_OPERATION_NUMBER_CAPACITY = 8
};

enum { XW_TOURDESK_REPLAY_CAPACITY = 16 };

enum {
	XW_TOURDESK_MOUSE_SELECT = 3,
	XW_TOURDESK_LEAVE_INPUT = 0,
	XW_TOURDESK_IDLE = 0,
	XW_TOURDESK_EXIT_REQUEST = 1,
	XW_TOURDESK_HOVER_REQUEST_BASE = 2,
	XW_TOURDESK_HOVER_ENTER = 3
};

extern int16_t g_tourReplayScenes[XW_TOURDESK_REPLAY_CAPACITY];
extern int16_t g_tourDeskFocusX[XW_TOURDESK_FOCUS_COLUMNS];
extern int16_t g_tourDeskFocusY[XW_TOURDESK_FOCUS_COLUMNS];
extern Sound* g_tourDeskMusic;
extern Sound* g_tourDeskPreviousMusic;
extern Film* g_tourDeskMusicFilm;
extern Sound* g_tourDeskSpeech[XW_TOURDESK_SPEECH_COUNT];
extern Input* g_tourDeskHintInput;
extern Actor* g_tourDeskBackgroundActor;
extern Actor* g_tourDeskDoorActor;
extern Input* g_tourDeskExitInput;
extern Actor* g_tourDeskStarsActor;
extern Actor* g_tourDeskNextButtonActor;
extern LandruHandle g_tourDeskText;
extern Input* g_tourDeskEmptyHintInput;
extern int16_t g_tourDeskReplayCutsceneIndices[XW_TOURDESK_REPLAY_CAPACITY];
extern Actor* g_tourDeskPreviousButtonActor;
extern Input* g_tourDeskRootInput;
extern int16_t g_tourDeskReplayCount;
extern REGISTER_PilotFileRecord g_tourDeskPilotRecord;
extern Actor* g_tourDeskDeskActor;
extern Actor* g_tourDeskOverlayActor;
extern int16_t g_tourDeskSelection;
extern int16_t g_tourDeskTourCount;
extern int16_t g_tourDeskFocusIndex;

/* Declarations follow ascending original IDB address. */

/* 0x461770 */
void tourdesk_OpenMusic(void* unusedResource, Film* film);

/* 0x461870 */
void tourdesk_CloseMusic(void);

/* 0x4618F0 */
void tourdesk_LoadSounds(void);

/* 0x461960 */
void tourdesk_PlaySpeech(int16_t speechIndex);

/* 0x461A10 */
void tourdesk_PlayDoorSound(int16_t action);

/* 0x464160 */
XwShellSceneResult tourdesk_TourDesk(struct XwShellContext* shellContext);

/* 0x464740 */
void tourdesk_end_View(int time);

/* 0x4647C0 */
void tourdesk_user_Door(Actor* actor, int time);

/* 0x464910 */
int16_t tourdesk_iupdate_TourDesk(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
								  int rightEvent, int16_t x, int16_t y);

/* 0x4649E0 */
void tourdesk_iuser_TourDesk(Input* input, int context);

/* 0x464B00 */
void tourdesk_idraw_ActionHint(Input* input, Rect* frame, Rect* clip, int16_t refresh);

/* 0x464BC0 */
void tourdesk_iuser_SelectionButton(Input* input, int context);

/* 0x464D10 */
void tourdesk_idraw_SelectionButton(PushButton* button, Rect* frame, Rect* clip, int16_t refresh);

/* 0x464D70 */
void tourdesk_idraw_TourText(Input* input, Rect* frame, Rect* clip, int16_t refresh);

/* 0x464F80 */
void tourdesk_user_Robot(Actor* actor, int time);

/* 0x464FD0 */
int16_t tourdesk_LoadPilotRecord(void);

/* 0x465080 */
void tourdesk_WritePilotRecord(void);

/* 0x465130 */
void tourdesk_BuildReplayList(void);

/* 0x4651C0 */
/* Requires a valid tour selection and saved operation index for that tour. */
void tourdesk_CommitTourSelection(void);

#ifdef __cplusplus
}
#endif

#endif
