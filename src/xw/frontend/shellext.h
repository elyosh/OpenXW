#ifndef XW_FRONTEND_SHELLEXT_H
#define XW_FRONTEND_SHELLEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/landru_config.h"

#include <landru/btnpush.h>
#include <landru/dialog.h>
#include <landru/input.h>
#include <landru/rect.h>
#include <stddef.h>
#include <stdint.h>
#include <xw/frontend/shell.h>
#include <xw/input/joystick.h>

enum {
	SHELLEXT_OPTIONS_LABEL_CAPACITY = 80,
	SHELLEXT_OPTIONS_INITIAL_FIELD = 7,
	SHELLEXT_PALETTE_COLORS = 256,
	SHELLEXT_OPTIONS_PALETTE_DELAY = 2,
	SHELLEXT_ALIGN_START = 0,
	SHELLEXT_ALIGN_CENTER = 1,
	SHELLEXT_ALIGN_END = 2,
	SHELLEXT_OPTIONS_TEXT_CAPACITY = 256,
	SHELLEXT_OPTIONS_FRAME_COLOR = 16,
	SHELLEXT_OPTIONS_BAR_COLOR = 62,
	SHELLEXT_OPTIONS_TEXT_COLOR = 15,
	SHELLEXT_OPTIONS_ACTION_COLOR = 14,
	SHELLEXT_OPTIONS_HEADER_HEIGHT = 16,
	SHELLEXT_OPTIONS_PANEL_LEFT = 4,
	SHELLEXT_OPTIONS_PANEL_RIGHT = 136,
	SHELLEXT_OPTIONS_AUDIO_TOP = 14,
	SHELLEXT_OPTIONS_AUDIO_BOTTOM = 50,
	SHELLEXT_OPTIONS_AUDIO_PANELS = 2,
	SHELLEXT_OPTIONS_AUDIO_SPACING = 38,
	SHELLEXT_OPTIONS_VOLUME_LABEL_X = 8,
	SHELLEXT_OPTIONS_VOLUME_LABEL_Y = 22,
	SHELLEXT_OPTIONS_BRIGHTNESS_TOP = 53,
	SHELLEXT_OPTIONS_BRIGHTNESS_BOTTOM = 89,
	SHELLEXT_SLIDER_IGNORED_EVENT = 3,
	SHELLEXT_VOLUME_SLIDER_STEP = 5,
	SHELLEXT_BRIGHTNESS_SLIDER_STEP = 10,
	SHELLEXT_BRIGHTNESS_MAX = 8,
	SHELLEXT_ACTION_ROWS = 9,
	SHELLEXT_TEXTURE_QUALITY_MAX = 2,
	SHELLEXT_ACTION_ROW_HEIGHT = 14,
	SHELLEXT_ACTION_LIST_LEFT = 4,
	SHELLEXT_ACTION_LIST_TOP = 49,
	SHELLEXT_ACTION_LIST_RIGHT = 316,
	SHELLEXT_ACTION_LIST_BOTTOM = 176,
	SHELLEXT_ACTION_LIST_INSET = 4
};

extern int g_optionsJoystickSlot;
extern unsigned int g_optionsActionIndex;

enum {
	SHELLEXT_KEY_EXTENDED = 0x100,
	SHELLEXT_KEY_SCAN_SHIFT = 8,
	SHELLEXT_KEY_F1 = 0x3B00,
	SHELLEXT_KEY_F10 = 0x4400,
	SHELLEXT_KEY_SHIFT_F1 = 0x5400,
	SHELLEXT_KEY_SHIFT_F10 = 0x5D00,
	SHELLEXT_KEY_ALT_Q = 0x1000,
	SHELLEXT_KEY_ALT_W = 0x1100,
	SHELLEXT_KEY_ALT_E = 0x1200,
	SHELLEXT_KEY_ALT_R = 0x1300,
	SHELLEXT_KEY_ALT_T = 0x1400,
	SHELLEXT_KEY_ALT_Y = 0x1500,
	SHELLEXT_KEY_ALT_U = 0x1600,
	SHELLEXT_KEY_ALT_I = 0x1700,
	SHELLEXT_KEY_ALT_O = 0x1800,
	SHELLEXT_KEY_ALT_P = 0x1900,
	SHELLEXT_KEY_ALT_A = 0x1E00,
	SHELLEXT_KEY_ALT_S = 0x1F00,
	SHELLEXT_KEY_ALT_D = 0x2000,
	SHELLEXT_KEY_ALT_F = 0x2100,
	SHELLEXT_KEY_ALT_G = 0x2200,
	SHELLEXT_KEY_ALT_H = 0x2300,
	SHELLEXT_KEY_ALT_J = 0x2400,
	SHELLEXT_KEY_ALT_K = 0x2500,
	SHELLEXT_KEY_ALT_L = 0x2600,
	SHELLEXT_KEY_ALT_Z = 0x2C00,
	SHELLEXT_KEY_ALT_X = 0x2D00,
	SHELLEXT_KEY_ALT_C = 0x2E00,
	SHELLEXT_KEY_ALT_V = 0x2F00,
	SHELLEXT_KEY_ALT_B = 0x3000,
	SHELLEXT_KEY_ALT_N = 0x3100,
	SHELLEXT_KEY_ALT_M = 0x3200,
	SHELLEXT_KEY_HOME = 0x4700,
	SHELLEXT_KEY_UP = 0x4800,
	SHELLEXT_KEY_PAGE_UP = 0x4900,
	SHELLEXT_KEY_LEFT = 0x4B00,
	SHELLEXT_KEY_RIGHT = 0x4D00,
	SHELLEXT_KEY_END = 0x4F00,
	SHELLEXT_KEY_DOWN = 0x5000,
	SHELLEXT_KEY_PAGE_DOWN = 0x5100,
	SHELLEXT_KEY_INSERT = 0x5200,
	SHELLEXT_KEY_DELETE = 0x5300,
	SHELLEXT_KEY_ALT_1 = 0x7800,
	SHELLEXT_KEY_ALT_2 = 0x7900,
	SHELLEXT_KEY_ALT_3 = 0x7A00,
	SHELLEXT_KEY_ALT_4 = 0x7B00,
	SHELLEXT_KEY_ALT_5 = 0x7C00,
	SHELLEXT_KEY_ALT_6 = 0x7D00,
	SHELLEXT_KEY_ALT_7 = 0x7E00,
	SHELLEXT_KEY_ALT_8 = 0x7F00,
	SHELLEXT_KEY_ALT_9 = 0x8000,
	SHELLEXT_KEY_ALT_0 = 0x8100
};

typedef struct XwJoystickSlotLabelRecord XwJoystickSlotLabelRecord;
typedef struct XwLegacyMemoryConfig XwLegacyMemoryConfig;
typedef struct XwSceneTransitionRedirect XwSceneTransitionRedirect;

enum {
	SHELLEXT_JOYSTICK_ENTRY_COUNT = 128,
	SHELLEXT_JOYSTICK_PARSE_CAPACITY = 256,
	SHELLEXT_JOYSTICK_READ_CAPACITY = 128
};

enum { XW_CONTENT_MARKER_PATH_CAPACITY = 256 };

enum { SHELLEXT_RESOURCE_PATH_CAPACITY = 256, SHELLEXT_CD_DRIVE_UNKNOWN = -1 };

enum { XW_SCENE_TRANSITION_REDIRECT_COUNT = 43, XW_SCENE_TRANSITION_TABLE_END = 0 };

typedef char XwJoystickSlotLabel[80];

/* Original IDB size: 88 bytes. */
struct XwLegacyMemoryConfig {
	/* IDB +0x0: Four zeroed entries; no identified consumers in this build. */
	unsigned int field_00[4];
	/* IDB +0x10: Four zeroed entries; no identified consumers in this build. */
	unsigned int field_10[4];
	/* IDB +0x20: Four legacy block pointers. Block 0 assigned the 4 MiB allocation; blocks 2/3 cleared for
	 * scenes 360..362. Forwarded configuration is ignored by handle startup. */
	void* blockBases[4];
	/* IDB +0x30: Four byte counts paired with blockBases. Block 0 starts at 4 MiB, restored to the 3 MiB
	 * usable-size scalar on scene/flight entry. */
	unsigned int blockSizes[4];
	/* IDB +0x40: Four zeroed entries; no identified consumers in this build. */
	uint16_t field_40[4];
	/* IDB +0x48: Four zeroed entries; no identified consumers in this build. */
	unsigned int field_48[4];
};

typedef int16_t XwOptionsControlId;

enum XwOptionsControlIdValues {
	XW_OPTIONS_ROOT = 0x0,
	XW_OPTIONS_OK = 0x1,
	XW_OPTIONS_EXIT = 0x2,
	XW_OPTIONS_MUSIC = 0x4,
	XW_OPTIONS_MUSIC_VOLUME = 0x5,
	XW_OPTIONS_SFX = 0x6,
	XW_OPTIONS_SFX_VOLUME = 0x7,
	XW_OPTIONS_SPOKEN_TEXT = 0x8,
	XW_OPTIONS_TRANSITIONS = 0x9,
	XW_OPTIONS_MISSION_SET = 0xA,
	XW_OPTIONS_GENERAL_PAGE = 0xB,
	XW_OPTIONS_OPEN_FLIGHT = 0xD,
	XW_OPTIONS_OPEN_JOYSTICK = 0xE,
	XW_OPTIONS_TEXTURE_QUALITY = 0x65,
	XW_OPTIONS_BACK_FROM_FLIGHT = 0x66,
	XW_OPTIONS_BRIGHTNESS = 0x69,
	XW_OPTIONS_FLIGHT_PAGE = 0x6F,
	XW_OPTIONS_ENGINE_SOUND = 0x71,
	XW_OPTIONS_RESOLUTION = 0x72,
	XW_OPTIONS_BACK_FROM_JOYSTICK = 0xCA,
	XW_OPTIONS_JOYSTICK_PAGE = 0xD3,
	XW_OPTIONS_PREVIOUS_ACTION_PAGE = 0xDC,
	XW_OPTIONS_NEXT_ACTION_PAGE = 0xDD,
	XW_OPTIONS_RESET_BINDINGS = 0xDE,
	XW_OPTIONS_PREVIOUS_JOYSTICK_SLOT = 0xE6,
	XW_OPTIONS_NEXT_JOYSTICK_SLOT = 0xE7
};

typedef int32_t XwOptionsPage;

enum XwOptionsPageValues {
	XW_OPTIONS_PAGE_GENERAL = 0x0,
	XW_OPTIONS_PAGE_FLIGHT = 0x1,
	XW_OPTIONS_PAGE_JOYSTICK = 0x2
};

/* Original IDB size: 4 bytes. */
struct XwSceneTransitionRedirect {
	/* IDB +0x0: XwShellSceneId value. Storage width preserved from the executable; scene IDs include explicit
	 * control sentinels. */
	int16_t sourceScene;
	/* IDB +0x2: XwShellSceneId value. Storage width preserved from the executable; scene IDs include explicit
	 * control sentinels. */
	int16_t destinationScene;
};

/* Original IDB size: 80 bytes. */
struct XwJoystickSlotLabelRecord {
	/* IDB +0x0: NUL-terminated joystick button/POV display label in a fixed 80-byte slot. */
	XwJoystickSlotLabel text;
};

/* Declarations follow ascending original IDB address. */

extern XwSceneTransitionRedirect g_sceneTransitionRedirects[XW_SCENE_TRANSITION_REDIRECT_COUNT];
extern char g_optionsMusicLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY];
extern char g_optionsSoundLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY];
extern char g_optionsSpokenTextLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY];
extern char g_optionsTransitionsLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY];
extern char g_optionsOkLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY];
extern char g_optionsExitLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY];
extern char g_optionsMissionSetLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY];
extern char g_optionsFlightLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY];
extern char g_optionsResolutionLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY];
extern char g_optionsEngineSoundLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY];
extern char g_optionsTextureLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY];
extern char g_optionsJoystickLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY];
extern char g_optionsBackLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY];
extern char g_optionsPreviousPageLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY];
extern char g_optionsNextPageLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY];
extern char g_optionsResetLabels[SHELLEXT_OPTIONS_LABEL_CAPACITY];
extern XwJoystickSlotLabelRecord g_optionsJoystickSlotLabels[JOYSTICK_ACTION_CAPACITY];
extern XwOptionsPage g_optionsPageIndex;
extern int g_optionsField5667BC;
extern int16_t g_optionsField5667D0;
extern Input* g_optionsDialog;
extern Input* g_optionsGeneralPage;
extern Input* g_optionsFlightPage;
extern Input* g_optionsJoystickPage;
extern JoystickEntry g_joystickEntries[SHELLEXT_JOYSTICK_ENTRY_COUNT];
extern int g_joystickEntryCount;
extern int16_t g_contentAvailabilityInitialized;

/* 0x4A65F0 */
void shellext_Open_Landru(struct XwLegacyMemoryConfig* memory);

/* 0x4A6850 */
void shellext_Close_Landru(void);

/* 0x4A68B0 */
void shellext_Open_Landru_Scene(XwShellSceneId scene);

/* 0x4A68F0 */
void shellext_Close_Landru_Scene(XwShellSceneId scene);

/* 0x4A6930 */
int16_t shellext_Get_Cur_Scene(void);

/* 0x4A6940 */
int16_t shellext_Check_Last_Scene(int16_t scene);

/* 0x4A6960 */
int16_t shellext_Get_Last_Scene(void);

/* 0x4A6970 */
int16_t shellext_Check_Scene_Exit(int16_t* exitScene, int16_t nextScene, int16_t nextSection,
								  int16_t sceneComplete);

/* 0x4A69F0 */
void shellext_Sudden_Scene_End(void);

/* 0x4A6A10 */
int16_t shellext_Is_Sudden_Scene_End(void);

/* 0x4A6A20 */
/* The modern scene owner must yield until the pushed fade task completes. */
void shellext_Sudden_Scene_Fade(void);

/* The original ABI returns a scene ID; the task API has no immediate result. */
#ifdef XW_MODERN
typedef void XwShellViewResult;
#else
typedef int16_t XwShellViewResult;
#endif

/* 0x4A6A50 */
/* Push the shared view task in the port. The owner yields, then reads the
 * completed scene ID with xerror_Get_Landru_Exit() after resumption. */
XwShellViewResult j_xviewadd_Handle_View(void);

/* 0x4A6A60 */
int16_t shellext_MoveGridFocus(int16_t* focusIndex, const int16_t* xPositions, const int16_t* yPositions,
							   int rows, int columns, int16_t key);

/* 0x4A7010 */
int16_t shellext_TryOpenOptions(void);

/* 0x4A7090 */
int16_t shellext_OpenOptionsAndSave(void);

/* 0x4A70F0 */
void shellext_Load_Preferences(void);

/* 0x4A72B0 */
int16_t shellext_GetTransitionsEnabled(void);

/* 0x4A72C0 */
int16_t shellext_GetClassicMissions(void);

/* 0x4A72D0 */
int16_t shellext_Convert_Transition(int16_t scene, int16_t sudden);

/* 0x4A7320 */
void shellext_Set_Prefs_Sound(void);

/* 0x4A73A0 */
void shellext_ShowOptionsDialog(DialogSubResultHandler complete, void* context);

/* 0x4A7530 */
Input* shellext_BuildOptionsDialog(void);

/* 0x4A7D00 */
void shellext_DrawJoystickArrow(Input* input, Rect* frame, Rect* clip, int16_t refresh);

/* 0x4A7D50 */
void shellext_OptionsButtonUser(Input* input, int context);

/* 0x4A84B0 */
int16_t shellext_UpdateOptionsInput(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
									int rightEvent, int16_t x, int16_t y);

/* 0x4A88A0 */
void shellext_DrawOptionsInput(Input* input, Rect* frame, Rect* clip, int16_t refresh);

/* 0x4A8FE0 */
void shellext_DetectInstalledContent(void);

/* 0x4A90C0 */
int shellext_LoadJoystickActionDictionary(void);

/* 0x4A9250 */
uint8_t shellext_KeyToJoystickAction(int16_t key);

#ifdef __cplusplus
}
#endif

#endif
