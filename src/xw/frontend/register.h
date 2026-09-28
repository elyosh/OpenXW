#ifndef XW_FRONTEND_REGISTER_H
#define XW_FRONTEND_REGISTER_H

#ifdef XW_MODERN
#include "xw_runtime/storage/pilot_file.h"
#define register_ReadPilotRecord XwPilot_Read
#else
#define register_ReadPilotRecord(file, record) xfile_Read_Data_From_File(file, record, sizeof(*(record)))
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/frontend/scenes/cutscene.h"
#include "xw/landru_config.h"

#include <landru/actor.h>
#include <landru/btnpush.h>
#include <landru/dialog.h>
#include <landru/filedir.h>
#include <landru/film.h>
#include <landru/input.h>
#include <landru/rect.h>
#include <landru/res.h>
#include <landru/sound.h>
#include <stddef.h>
#include <stdint.h>
#include <xw/frontend/shell.h>
#include <xw/frontend/uniform.h>

struct XwShellContext;

enum {
	REGISTER_INITIAL_FOCUS = 54,
	REGISTER_WIDTH = 640,
	REGISTER_HEIGHT = 480,
	REGISTER_RESOURCE_FILE = 0,
	REGISTER_RESOURCE_FILM = 1,
	REGISTER_RESOURCE_BACKGROUND = 2,
	REGISTER_RESOURCE_DOOR = 3,
	REGISTER_RESOURCE_LEFT_BUTTON = 4,
	REGISTER_RESOURCE_SOLDIER = 5,
	REGISTER_RESOURCE_RIGHT_BUTTON = 6,
	REGISTER_RESOURCE_ROBOT = 7,
	REGISTER_RESOURCE_RIGHT_DESK = 8,
	REGISTER_RESOURCE_LEFT_DESK = 9,
	REGISTER_RESOURCE_TOURS = 16,
	REGISTER_TEXT_LOG = 21,
	REGISTER_TEXT_MERITS = 22,
	REGISTER_RESOURCE_MERITS_BUTTON = 31,
	REGISTER_RESOURCE_LOG_BUTTON = 32,
	REGISTER_RESOURCE_DELETE_BUTTON = 33,
	REGISTER_SOLDIER_ID = 1,
	REGISTER_PAGE_NUMBER_INPUT = 5,
	REGISTER_NAME_FILENAME_MODE = 1,
	REGISTER_PREVIOUS_PAGE_BUTTON = 0,
	REGISTER_NEXT_PAGE_BUTTON = 1,
	REGISTER_LOG_BUTTON = 2,
	REGISTER_MERITS_BUTTON = 3,
	REGISTER_DELETE_BUTTON = 4,
	REGISTER_DOOR_LEFT = 0,
	REGISTER_DOOR_TOP = 127,
	REGISTER_DOOR_RIGHT = 170,
	REGISTER_DOOR_BOTTOM = 400,
	REGISTER_LIST_LEFT = 425,
	REGISTER_LIST_TOP = 180,
	REGISTER_LIST_RIGHT = 540,
	REGISTER_LIST_BOTTOM = 370,
	REGISTER_PREVIOUS_LEFT = 522,
	REGISTER_PREVIOUS_TOP = 389,
	REGISTER_PREVIOUS_RIGHT = 557,
	REGISTER_PREVIOUS_BOTTOM = 421,
	REGISTER_NEXT_LEFT = 431,
	REGISTER_NEXT_TOP = 388,
	REGISTER_NEXT_RIGHT = 464,
	REGISTER_NEXT_BOTTOM = 421,
	REGISTER_PAGE_NUMBER_LEFT = 464,
	REGISTER_PAGE_NUMBER_TOP = 388,
	REGISTER_PAGE_NUMBER_RIGHT = 522,
	REGISTER_PAGE_NUMBER_BOTTOM = 421,
	REGISTER_NAME_LEFT = 436,
	REGISTER_NAME_TOP = 430,
	REGISTER_NAME_RIGHT = 543,
	REGISTER_NAME_BOTTOM = 453,
	REGISTER_INFO_LEFT = 282,
	REGISTER_INFO_TOP = 186,
	REGISTER_INFO_RIGHT = 381,
	REGISTER_INFO_BOTTOM = 373,
	REGISTER_LOG_LEFT = 346,
	REGISTER_LOG_TOP = 389,
	REGISTER_LOG_RIGHT = 385,
	REGISTER_LOG_BOTTOM = 421,
	REGISTER_MERITS_LEFT = 258,
	REGISTER_MERITS_TOP = 387,
	REGISTER_MERITS_RIGHT = 344,
	REGISTER_MERITS_BOTTOM = 419,
	REGISTER_DELETE_LEFT = 260,
	REGISTER_DELETE_TOP = 420,
	REGISTER_DELETE_RIGHT = 384,
	REGISTER_DELETE_BOTTOM = 455
};

enum {
	REGISTER_MUSIC_START_CEL = 1,
	REGISTER_MUSIC_VOLUME = 95,
	REGISTER_MUSIC_FADE_DURATION = 300,
	REGISTER_MUSIC_CLOSE_VOLUME = 127,
	REGISTER_MUSIC_CLOSE_DURATION = 180,
	REGISTER_MUSIC_CLOSE_CONTROL = 1,
	REGISTER_MUSIC_CLOSE_MARKER = 1
};

enum {
	REGISTER_SPEECH_FIRST = 0,
	REGISTER_SPEECH_WAIT = 1,
	REGISTER_SPEECH_REGISTER = 2,
	REGISTER_SPEECH_COUNT = 3
};

extern Sound* g_RegisterSpeech[REGISTER_SPEECH_COUNT];

enum {
	REGISTER_NAVIGATION_ROWS = 11,
	REGISTER_NAVIGATION_COLUMNS = 5,
	REGISTER_NAVIGATION_COUNT = REGISTER_NAVIGATION_ROWS * REGISTER_NAVIGATION_COLUMNS,
	REGISTER_CURSOR_NORMAL = 0,
	REGISTER_CURSOR_BUSY = 1,
	REGISTER_INITIAL_SPEECH_FRAME = 2,
	REGISTER_PROTECT_RETURN_MOUSE_X = 40,
	REGISTER_PROTECT_RETURN_MOUSE_Y = 160,
	REGISTER_DELETE_LABEL_UNKNOWN = -1,
	REGISTER_DELETE_LABEL_DELETE = 0,
	REGISTER_DELETE_LABEL_MODIFY = 1
};

enum { REGISTER_DELETE_CONFIRM = 1, REGISTER_DELETE_CANCEL = 2, REGISTER_DELETE_REVIVE = 3 };

enum { REGISTER_DELETE_INITIAL_FOCUS = 1 };

enum {
	REGISTER_DELETE_DIALOG_WIDTH = 180,
	REGISTER_DELETE_DIALOG_HEIGHT = 46,
	REGISTER_DELETE_BUTTON_INSET = 4,
	REGISTER_DELETE_BUTTON_WIDTH = 50,
	REGISTER_DELETE_BUTTON_BOTTOM = 20
};

enum { REGISTER_DIALOG_ALIGN_NEAR = 0, REGISTER_DIALOG_ALIGN_CENTER = 1, REGISTER_DIALOG_ALIGN_FAR = 2 };

enum { REGISTER_NAME_INPUT_LIMIT = 16, REGISTER_PILOT_SLOT_NONE = -1 };

enum {
	REGISTER_NAME_FONT = 2,
	REGISTER_NAME_TEXT_COLOR = 18,
	REGISTER_NAME_TEXT_X = 2,
	REGISTER_NAME_TEXT_Y = 1,
	REGISTER_NAME_CARET_X = 4,
	REGISTER_NAME_CARET_LIT_Y = 12,
	REGISTER_NAME_CARET_ERASE_Y = 7,
	REGISTER_NAME_CARET_WIDTH = 5
};

enum { REGISTER_PILOT_PATH_CAPACITY = 32, REGISTER_PILOT_BUTTON_PATH_CAPACITY = 48 };

enum { REGISTER_DIRECTORY_NAME_CAPACITY = 256, REGISTER_FAST_PILOT_NAME_COPY_BYTES = 32 };

enum { REGISTER_MOUSE_RELEASE = 3, REGISTER_PILOT_ROW_HEIGHT = 10, REGISTER_PILOT_VISIBLE_ROWS = 19 };

enum {
	REGISTER_PILOT_LIST_TEXT_CAPACITY = 32,
	REGISTER_PILOT_LIST_CLIP_INSET = 3,
	REGISTER_PILOT_LIST_TEXT_X = 1,
	REGISTER_PILOT_LIST_TEXT_Y = 1,
	REGISTER_PILOT_LIST_FONT = 0,
	REGISTER_PILOT_LIST_NORMAL_COLOR = 15,
	REGISTER_PILOT_LIST_LOST_COLOR = 4,
	REGISTER_PILOT_LIST_ACTIVE_COLOR = 14
};

enum {
	REGISTER_DOOR_IDLE = 0,
	REGISTER_DOOR_ACCEPT = 1,
	REGISTER_DOOR_HOVER = 2,
	REGISTER_DOOR_EMPTY_NAME = 3,
	REGISTER_DOOR_LOST_PILOT = 4,
	REGISTER_DOOR_LOST_PILOT_ALTERNATE = 5,
	REGISTER_PILOT_AVAILABLE = 0,
	REGISTER_PILOT_LOST_ALTERNATE = 2
};

enum { REGISTER_BACKGROUND_COLOR = 0 };

enum {
	REGISTER_RESOURCE_TEXT_COUNT = 34,
	REGISTER_RESOURCE_TEXT_CAPACITY = 20,
	REGISTER_TEXT_YOU_MUST = 10,
	REGISTER_TEXT_REGISTER = 11,
	REGISTER_TEXT_ENTER = 12,
	REGISTER_TEXT_SPACEPORT = 13,
	REGISTER_TEXT_PROTECT_OK = 18,
	REGISTER_TEXT_PROTECT_EXIT = 19,
	REGISTER_TEXT_PROTECT_CONTINUE = 20,
	REGISTER_TEXT_DELETE_PILOT = 23,
	REGISTER_TEXT_DELETE = 24,
	REGISTER_TEXT_CANCEL = 25,
	REGISTER_TEXT_THIS_PILOT_IS = 26,
	REGISTER_TEXT_CAPTURED = 27,
	REGISTER_TEXT_DEAD = 28,
	REGISTER_TEXT_MODIFY_PILOT = 29,
	REGISTER_TEXT_REVIVE = 30
};

enum {
	REGISTER_MESSAGE_ENTER = 0,
	REGISTER_MESSAGE_REGISTER = 1,
	REGISTER_MESSAGE_CAPTURED = 2,
	REGISTER_MESSAGE_DEAD = 3,
	REGISTER_MESSAGE_FONT = 0,
	REGISTER_MESSAGE_NORMAL_COLOR = 15,
	REGISTER_MESSAGE_WARNING_COLOR = 4,
	REGISTER_MESSAGE_SHADOW_COLOR = 16,
	REGISTER_MESSAGE_SHADOW_OFFSET = 1,
	REGISTER_MESSAGE_FIRST_LINE_Y = -5,
	REGISTER_MESSAGE_LINE_SPACING = 10
};

enum {
	REGISTER_PILOT_BUTTON_LEFT = 0,
	REGISTER_PILOT_BUTTON_RIGHT = 1,
	REGISTER_PILOT_BUTTON_LOG = 2,
	REGISTER_PILOT_BUTTON_MERITS = 3,
	REGISTER_PILOT_BUTTON_DELETE = 4,
	REGISTER_PILOT_BUTTON_FONT = 0,
	REGISTER_PILOT_BUTTON_TEXT_COLOR = 15
};

enum {
	REGISTER_PROTECT_TARGET_COUNT = 2,
	REGISTER_DELETE_TARGET_COUNT = 3,
	REGISTER_KEY_LEFT = 0x4B00,
	REGISTER_KEY_UP = 0x4800,
	REGISTER_KEY_RIGHT = 0x4D00,
	REGISTER_KEY_DOWN = 0x5000
};

enum { REGISTER_KEY_DELETE = 0x5300, REGISTER_KEY_ASCII_DELETE = 0x7F, REGISTER_STRING_WORK_CAPACITY = 32 };

enum {
	REGISTER_PROTECT_ACCEPT_INPUT = 1,
	REGISTER_PROTECT_CANCEL_INPUT = 2,
	REGISTER_PROTECT_SYMBOL_INPUT = 3,
	REGISTER_PROTECT_FOCUS_INPUT = 4,
	REGISTER_PROTECT_CHOOSE_INPUT = 5,
	REGISTER_PROTECT_QUESTION_COUNT = 32,
	REGISTER_PROTECT_ANSWER_CAPACITY = 14,
	REGISTER_PROTECT_ATTEMPT_LIMIT = 3,
	REGISTER_PROTECT_PAGE_INPUT = 6,
	REGISTER_PROTECT_SYMBOL_COUNT = 3,
	REGISTER_PROTECT_SYMBOL_WIDTH = 32,
	REGISTER_PROTECT_SYMBOL_SPACING = 36,
	REGISTER_PROTECT_SYMBOL_INSET = 2,
	REGISTER_PROTECT_SYMBOL_X_OFFSET = 4,
	REGISTER_PROTECT_SYMBOL_Y_OFFSET = 1,
	REGISTER_PROTECT_QUESTIONS_PER_PAGE_SHIFT = 1,
	REGISTER_PROTECT_FIRST_MANUAL_PAGE = 3
};

enum {
	REGISTER_PROTECT_DIALOG_WIDTH = 112,
	REGISTER_PROTECT_DIALOG_HEIGHT = 86,
	REGISTER_PROTECT_INSET = 4,
	REGISTER_PROTECT_CONTENT_RIGHT = 108,
	REGISTER_PROTECT_SYMBOL_TOP = 14,
	REGISTER_PROTECT_SYMBOL_BOTTOM = 37,
	REGISTER_PROTECT_PAGE_TOP = 39,
	REGISTER_PROTECT_PAGE_BOTTOM = 49,
	REGISTER_PROTECT_NAME_TOP = 51,
	REGISTER_PROTECT_NAME_BOTTOM = 65,
	REGISTER_PROTECT_BUTTON_BOTTOM = 20,
	REGISTER_PROTECT_OK_RIGHT = 32,
	REGISTER_PROTECT_EXIT_RIGHT = 76
};

enum {
	REGISTER_DIALOG_TITLE_CAPACITY = 64,
	REGISTER_DIALOG_TITLE_HEIGHT = 14,
	REGISTER_DIALOG_FRAME_COLOR = 16,
	REGISTER_DIALOG_FRAME_INSET = 1,
	REGISTER_DIALOG_PANEL_TOP_COLOR = 242,
	REGISTER_DIALOG_PANEL_BOTTOM_COLOR = 243,
	REGISTER_DIALOG_PANEL_FILL_COLOR = 244,
	REGISTER_DIALOG_PANEL_PRESSED = 6
};

enum {
	REGISTER_DIALOG_OUTER_TOP_COLOR = 70,
	REGISTER_DIALOG_INNER_TOP_COLOR = 71,
	REGISTER_DIALOG_OUTER_BOTTOM_COLOR = 72,
	REGISTER_DIALOG_INNER_BOTTOM_COLOR = 73,
	REGISTER_DIALOG_FILL_COLOR = 240,
	REGISTER_DIALOG_BEVEL_PRESSED = 245,
	REGISTER_DIALOG_FONT = 0,
	REGISTER_DIALOG_TEXT_COLOR = 15
};

enum {
	REGISTER_INFO_NONE = 0,
	REGISTER_INFO_LOAD = 1,
	REGISTER_INFO_ACCESS = 2,
	REGISTER_INFO_CACHED = 3,
	REGISTER_INFO_START = 1,
	REGISTER_INFO_RUNNING = 2,
	REGISTER_INFO_DELAY = 16,
	REGISTER_INFO_FLASH_BIT = 4,
	REGISTER_INFO_SHORT_TEXT_SIZE = 16,
	REGISTER_RANK_COUNT = 6,
	REGISTER_STATUS_LABEL_COUNT = 3,
	REGISTER_STATUS_LABEL_SIZE = 12,
	REGISTER_INFO_LABEL_FONT = 0,
	REGISTER_INFO_VALUE_FONT = 2,
	REGISTER_INFO_TEXT_X = 4,
	REGISTER_INFO_RULE_X = 3,
	REGISTER_INFO_RULE_MARGIN = 5,
	REGISTER_INFO_NAME_Y = 4,
	REGISTER_INFO_RANK_Y = 34,
	REGISTER_INFO_SCORE_Y = 64,
	REGISTER_INFO_TOUR_Y = 94,
	REGISTER_INFO_NAME_RULE_Y = 14,
	REGISTER_INFO_RANK_RULE_Y = 44,
	REGISTER_INFO_SCORE_RULE_Y = 74,
	REGISTER_INFO_TOUR_RULE_Y = 104,
	REGISTER_INFO_NAME_VALUE_Y = 16,
	REGISTER_INFO_RANK_VALUE_Y = 46,
	REGISTER_INFO_SCORE_VALUE_Y = 76,
	REGISTER_INFO_TOUR_VALUE_Y = 106,
	REGISTER_INFO_TOUR_BOTTOM = 118,
	REGISTER_INFO_STATUS_X = 6,
	REGISTER_INFO_NAME_COLOR = 17,
	REGISTER_INFO_RANK_COLOR = 24,
	REGISTER_INFO_SCORE_COLOR = 21,
	REGISTER_INFO_TOUR_COLOR = 19,
	REGISTER_INFO_NAME_VALUE_COLOR = 18,
	REGISTER_INFO_RANK_VALUE_COLOR = 23,
	REGISTER_INFO_SCORE_VALUE_COLOR = 22,
	REGISTER_INFO_TOUR_VALUE_COLOR = 20,
	REGISTER_INFO_STATUS_COLOR = 14,
	REGISTER_INFO_RULE_COLOR = 1,
	REGISTER_INFO_ACCESS_TEXT = 14,
	REGISTER_INFO_ACCESS_Y = -8,
	REGISTER_INFO_ACCESS_SHADOW_COLOR = 16,
	REGISTER_INFO_ACCESS_COLOR = 2,
	REGISTER_INFO_ACCESS_NAME_Y = 10
};

extern char g_RegisterRankNames[REGISTER_RANK_COUNT][REGISTER_INFO_SHORT_TEXT_SIZE];
extern char g_RegisterLostStatusLabels[REGISTER_STATUS_LABEL_COUNT][REGISTER_STATUS_LABEL_SIZE];
extern LandruHandle g_RegisterTourParagraph;
extern Actor* g_RegisterLeftDeskActor;

enum { REGISTER_TROOP_SOUND_FRAME = 7 };

enum {
	REGISTER_STRING_FONT = 0,
	REGISTER_STRING_TEXT_OFFSET = 3,
	REGISTER_STRING_CARET_X = 4,
	REGISTER_STRING_CARET_Y = 11,
	REGISTER_STRING_CARET_WIDTH = 6
};

enum {
	REGISTER_PAGE_TEXT_CAPACITY = 32,
	REGISTER_PAGE_BACKGROUND_COLOR = 16,
	REGISTER_PAGE_TEXT_COLOR = 15,
	REGISTER_PAGE_FONT = 0
};

enum {
	REGISTER_SOUND_GUARD = 1,
	REGISTER_SOUND_DOOR_OPEN = 2,
	REGISTER_SOUND_DOOR_CLOSE = 3,
	REGISTER_SOUND_PILOT_INFO = 4
};

typedef struct REGISTER_FastPilotRecord REGISTER_FastPilotRecord;
typedef struct REGISTER_PilotFileRecord REGISTER_PilotFileRecord;
typedef struct REGISTER_RegStringButton REGISTER_RegStringButton;

extern Input* g_RegisterDoorInput;
extern Actor* g_RegisterRobotActor;
extern Film* g_RegisterFilm;
extern Actor* g_RegisterBackgroundActor;
extern int16_t g_RegisterActivePilot;
extern const int16_t g_RegisterNavigationX[REGISTER_NAVIGATION_COUNT];
extern const int16_t g_RegisterNavigationY[REGISTER_NAVIGATION_COUNT];
extern Input* g_RegisterDeleteButton;
extern Input* g_RegisterLogButton;
extern int16_t g_RegisterDeleteLabelState;
extern Input* g_RegisterMeritsButton;
extern int16_t g_RegisterNavigationIndex;
extern int g_RegisterProtectionEnabled;
extern int16_t g_RegisterDeleteMouseX[REGISTER_DELETE_TARGET_COUNT];
extern int16_t g_RegisterDeleteMouseY[REGISTER_DELETE_TARGET_COUNT];
extern int16_t g_RegisterDeleteFocus;
extern int16_t g_RegisterProtectMouseX[REGISTER_PROTECT_TARGET_COUNT];
extern int16_t g_RegisterProtectMouseY[REGISTER_PROTECT_TARGET_COUNT];
extern const char g_RegisterResourceAndLabelText[REGISTER_RESOURCE_TEXT_COUNT]
												[REGISTER_RESOURCE_TEXT_CAPACITY];
extern int16_t g_RegisterProtectQuestionSymbols[REGISTER_PROTECT_QUESTION_COUNT]
											   [REGISTER_PROTECT_SYMBOL_COUNT];
extern char g_RegisterProtectAnswers[REGISTER_PROTECT_QUESTION_COUNT][REGISTER_PROTECT_ANSWER_CAPACITY];
extern LandruHandle g_RegisterFastPilotHandle;
extern Actor* g_RegisterDeleteButtonActor;
extern Actor* g_RegisterSoldierActor;
extern Input* g_RegisterPilotInfoInput;
extern int16_t g_RegisterProtectChosen;
extern Actor* g_RegisterLeftButtonActor;
extern REGISTER_PilotFileRecord g_RegisterPilotReadBuffer;
extern Directory g_RegisterDirectory;
extern Input* g_RegisterPilotListInput;
extern Actor* g_RegisterLogButtonActor;
extern int16_t g_RegisterPageCount;
extern int16_t g_RegisterProtectSymbols[REGISTER_PROTECT_SYMBOL_COUNT];
extern int16_t g_RegisterProtectQuestionIndex;
extern Input* g_RegisterProtectDialog;
extern Actor* g_RegisterRightButtonActor;
extern Actor* g_RegisterMeritsButtonActor;
extern REGISTER_RegStringButton* g_RegisterPilotNameInput;
extern int16_t g_RegisterCurrentPage;
extern int16_t g_RegisterProtectAttempts;
extern REGISTER_PilotFileRecord g_RegisterPilotData;
extern Actor* g_RegisterRightDeskActor;
extern REGISTER_RegStringButton* g_RegisterProtectNameInput;
extern Input* g_RegisterProtectOkInput;
extern Input* g_RegisterProtectExitInput;
extern Input* g_RegisterListOverlayInput;
extern Actor* g_RegisterDoorActor;
extern Actor* g_RegisterProtectSymbolActor;
extern Input* g_RegisterPageInput;
extern int16_t g_RegisterPilotOffset;
extern int16_t g_RegisterPilotCount;
extern int16_t g_RegisterLoadedPilotCount;
extern int16_t g_RegisterRobotState;
extern int16_t g_RegisterProtectFocus;
extern XwSceneMusicHandles g_RegisterMusicState;
extern REGISTER_FastPilotRecord g_RegisterShellPilot;

/* Original IDB size: 36 bytes. */
struct REGISTER_FastPilotRecord {
	/* IDB +0x0: 24-byte name storage; builder copies 32 bytes then replaces metadata. Registration editing is
	 * capped at 16 characters. */
	char name[24];
	/* IDB +0x18: Copied from pilot-file byte 0; precise meaning not established. */
	uint8_t field_18;
	/* IDB +0x19: 0 means visible/usable slot; -1 marks failed load or soft deletion. */
	int8_t deleted;
	/* IDB +0x1A: 0 means available; nonzero enables Modify/Revive. Copied from pilot-file byte 2 and cleared
	 * by revival. */
	uint8_t lost_status;
	/* IDB +0x1B: Copied from pilot-file byte 3; used to select displayed rank text. */
	uint8_t rank;
	/* IDB +0x1C: Copied from pilot-file byte 0x280. */
	uint8_t current_tour;
	/* IDB +0x1D: Cleared when creating a shell summary; no precise meaning established. */
	uint8_t field_1D;
	/* IDB +0x1E: Copied into the launch pilot roster. Craft initialization uses max(pilot skill value,
	 * mission AI skill value). */
	uint16_t skillValue;
	/* IDB +0x20: Copied from pilot-file +4; displayed as unsigned TOD Score. */
	unsigned int score;
};

/* Preserve the serialized format in native builds; matching uses /Zp1. */
#ifdef XW_MODERN
#pragma pack(push, 1)
#endif
/* Original IDB size: 1705 bytes. */
struct REGISTER_PilotFileRecord {
	/* IDB +0x0 */
	uint8_t field_0;
	/* IDB +0x1: Copied into summary.deleted; zero participates in visible-pilot searches. */
	int8_t deleted;
	/* IDB +0x2: Displayed pilot availability state; revival clears it. */
	uint8_t lost_status;
	/* IDB +0x3: Index into 16-byte rank-name table; revive mode 0 clamps values above 1. */
	uint8_t rank;
	/* IDB +0x4: Unsigned TOD score; cleared by revive mode 0. */
	unsigned int score;
	/* IDB +0x8: Pilot AI skill, raised by career score/historical missions and copied to the launch roster.
	 */
	uint16_t skillValue;
	/* IDB +0xA: Uniform actor roles 12..14: nonzero awards for Corellian Cross, Mantooine Medallion and Star
	 * of Alderaan. Read by uniform screen; award writers not rechecked here. */
	uint8_t uniformMedals[3];
	/* IDB +0xD: Award-box roles6..8: nonzero flags for Shield of Yavin, Talons of Hoth and unused medal.
	 * Read/label evidence; award writers not rechecked here. */
	uint8_t expansionMedals[3];
	/* IDB +0x10 */
	uint8_t gap_10;
	/* IDB +0x11: Uniform display: 0 absent; 1 Kalidor Crescent; higher values unlock successive
	 * embellishments (hover slots 30..34). */
	uint8_t kalidorAwardLevel;
	/* IDB +0x12 */
	uint8_t gap_12[20];
	/* IDB +0x26: Four ship scores read by pilot log at offset38; only four entries established here. */
	unsigned int trainingBestScores[4];
	/* IDB +0x36 */
	uint8_t gap_36[80];
	/* IDB +0x86: Per-ship highest completed proving-ground level (zero-based). FEDISKIO_updatepilotrecord
	 * raises it using one-based flight ship code minus 1; the training room offers value+1 levels, capped by
	 * the mission-text level count. */
	uint8_t trainingLevelProgress[6];
	/* IDB +0x8C */
	uint8_t gap_8C[18];
	/* IDB +0x9E: Saved SHIPEXT_Get_Combat_Ship selection; stamped before launch. */
	uint8_t combatShip;
	/* IDB +0x9F: Saved SHIPEXT_Get_Combat_Mission selection; stamped before launch. */
	uint8_t combatMission;
	/* IDB +0xA0: Four ship rows,16 dword slots each; pilot log displays missions0..5 of each row. */
	unsigned int historicBestScores[4][16];
	/* IDB +0x1A0: Six bonus mission scores displayed by pilot log. */
	unsigned int bonusHistoricBestScores[6];
	/* IDB +0x1B8 */
	uint8_t gap_1B8[104];
	/* IDB +0x220: 16-byte stride: X/Y/A/B-Wing records. First6 bytes enable individual mission battle
	 * patches. Remaining10 bytes per record unidentified; fourth record confirmed by award-box B-Wing
	 * display. */
	struct XwUniformCombatAwards combatAwards[4];
	/* IDB +0x260: Six nonzero completion flags displayed as consecutive mission ranges. */
	uint8_t bonusHistoricCompleted[6];
	/* IDB +0x266 */
	uint8_t gap_266[26];
	/* IDB +0x280: Index used for active-tour name/status display. */
	uint8_t current_tour;
	/* IDB +0x281 */
	uint8_t field_281;
	/* IDB +0x282: Mission string index selected from briefingMissionChoices in the current tour paragraph. */
	uint8_t selectedTourMission;
	/* IDB +0x283: Operation index within the selected tour. Restored from tourOperationProgress when joining;
	 * advanced by FEDISKIO_updatepilotrecord after success. */
	uint8_t currentTourOperation;
	/* IDB +0x284 */
	uint8_t gap_284[3];
	/* IDB +0x287: Two mission choices used by the briefing A/B buttons; 0xFF means unavailable. */
	uint8_t briefingMissionChoices[2];
	/* IDB +0x289 */
	uint8_t gap_289[86];
	/* IDB +0x2DF: Eight tour states; UI displays current tour when state=1, revive maps state 2 to 1. */
	uint8_t tour_status[8];
	/* IDB +0x2E7: Per-tour Combat Chamber unlock; 0xFF means none. Completed operation stores choice B if
	 * present, otherwise A, regardless of which choice was flown. */
	uint8_t tourReplayUnlockMission[8];
	/* IDB +0x2EF: Eight per-tour operation indices/completion counts. Used to restore a tour and enumerate
	 * unlocked cutscenes from completed operations. */
	uint8_t tourOperationProgress[8];
	/* IDB +0x2F7: Eight tours with25 dword mission slots each. Operation display takes max of
	 * missionChoiceA/B, with255 meaning no B choice. */
	unsigned int tourMissionScores[8][25];
	/* IDB +0x617 */
	uint8_t gap_617[24];
	/* IDB +0x62F */
	uint16_t total_kills;
	/* IDB +0x631 */
	uint16_t total_captures;
	/* IDB +0x633: Pilot log displays this unsigned word as TOD Surface Victories. */
	uint16_t surfaceVictories;
	/* IDB +0x635 */
	uint16_t kills_by_type[24];
	/* IDB +0x665 */
	uint16_t captures_by_type[24];
	/* IDB +0x695: Career total accumulated from the corresponding CraftData.weaponStats counter. */
	unsigned int laser_shots_fired;
	/* IDB +0x699: Career total accumulated from the corresponding CraftData.weaponStats counter. */
	unsigned int laser_spacecraft_hits;
	/* IDB +0x69D: Career total accumulated from the corresponding CraftData.weaponStats counter. */
	unsigned int laser_surface_hits;
	/* IDB +0x6A1: Career total accumulated from the corresponding CraftData.weaponStats counter. */
	uint16_t warheads_fired;
	/* IDB +0x6A3: Career total accumulated from the corresponding CraftData.weaponStats counter. */
	uint16_t warhead_spacecraft_hits;
	/* IDB +0x6A5: Career total accumulated from the corresponding CraftData.weaponStats counter. */
	uint16_t warhead_surface_hits;
	/* IDB +0x6A7: Existing ejection counter; pilot log labels the same word TOD Craft Lost. */
	uint16_t ejections;
};
#ifdef XW_MODERN
#pragma pack(pop)
#endif
typedef char xw_size_REGISTER_PilotFileRecord[(sizeof(REGISTER_PilotFileRecord) == 1705) ? 1 : -1];
typedef char xw_offset_REGISTER_PilotFileRecord_trainingBestScores
	[(offsetof(REGISTER_PilotFileRecord, trainingBestScores) == 38) ? 1 : -1];
typedef char xw_offset_REGISTER_PilotFileRecord_tourMissionScores
	[(offsetof(REGISTER_PilotFileRecord, tourMissionScores) == 759) ? 1 : -1];
typedef char xw_offset_REGISTER_PilotFileRecord_laser_shots_fired
	[(offsetof(REGISTER_PilotFileRecord, laser_shots_fired) == 1685) ? 1 : -1];

/* Original IDB size: 110 bytes. */
struct REGISTER_RegStringButton {
	/* IDB +0x0 */
	Input input;
	/* IDB +0x3C: 48-byte allocated text storage; keyboard insertion stops at 16 characters. Unbounded setters
	 * require a fitting source. */
	char name[48];
	/* IDB +0x6C: Nonzero allows digits, underscore and hyphen in addition to uppercase alphabetic input. */
	int16_t is_filename_mode;
};

/* Declarations follow ascending original IDB address. */

/* 0x45BF50 */
XwShellSceneResult register_Register(struct XwShellContext* shell);

/* 0x45C6A0 */
void register_end_View(int time);

/* 0x45C970 */
int16_t register_draw_Register_Back(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
									int16_t refresh);

/* 0x45C9C0 */
void register_user_Robot(Actor* actor, int time);

/* 0x45CA00 */
int16_t register_draw_Robot(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh);

/* 0x45CA50 */
void register_user_Door(Actor* door, int time);

/* 0x45CAE0 */
void register_user_Troop(Actor* troop, int time);

/* 0x45CB40 */
int16_t register_draw_Troop(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh);

/* 0x45CB80 */
int16_t register_iupdate_Register(Input* input, Rect* frame, Rect* clip, int16_t phase, int leftEvent,
								  int rightEvent, int16_t x, int16_t y);

/* 0x45CC90 */
void register_iuser_Register(Input* input, int context);

/* 0x45CEF0 */
int16_t register_iupdate_Pilot_List(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
									int rightEvent, int16_t x, int16_t y);

/* 0x45CFD0 */
void register_idraw_Pilot_List(Input* input, Rect* frame, Rect* clip, int16_t refresh);

/* 0x45D110 */
void register_idraw_ListMessage(Input* input, Rect* frame, Rect* unusedClip, int16_t refresh);

/* 0x45D260 */
void register_iuser_Pilot_Button(Input* input, int unusedContext);

/* 0x45D4A0 */
void register_idraw_Pilot_Button(PushButton* button, Rect* frame, Rect* clip, int16_t refresh);

/* 0x45D550 */
void register_idraw_PageNumber(Input* unusedInput, Rect* frame, Rect* unusedClip, int16_t refresh);

/* 0x45D5B0 */
void register_iuser_Pilot_Name(Input* input, int unusedContext);

/* 0x45D620 */
void register_xuser_Pilot_Name(const char* searchName);

/* 0x45D750 */
void register_idraw_Pilot_Name(struct REGISTER_RegStringButton* button, Rect* frame, Rect* clip,
							   int16_t refresh);

/* 0x45D830 */
void register_iuser_Pilot_Info(Input* input, int unusedContext);

/* 0x45D840 */
void register_idraw_Pilot_Info(Input* input, Rect* frame, Rect* clip, int16_t refresh);

/* 0x45DE70 */
int16_t register_Index_To_Pilot(int16_t logicalIndex, int16_t* outSlot);

/* 0x45DEF0 */
int16_t register_Index_To_Pilot_Record(int16_t logicalIndex, struct REGISTER_FastPilotRecord* outRecord);

/* 0x45DF80 */
void register_Build_Fast_Pilot_Record(void);

/* 0x45E120 */
void register_Delete_Pilot_Record(void);

/* 0x45E200 */
void register_Revive_Pilot_Record(void);

/* 0x45E2D0 */
void register_Set_Your_Reg_Pilot(void);

/* 0x45E390 */
struct REGISTER_RegStringButton* register_Alloc_Input_Reg_String_Button(Input* parent, Rect* frame,
																		int16_t zinput, InputUserFunc user,
																		const char* initialName,
																		int16_t isFilenameMode, int16_t id);

/* 0x45E420 */
void register_idraw_Reg_String_Button(Input* input, Rect* frame, Rect* clip, int16_t refresh);

/* 0x45E4E0 */
int16_t register_iupdate_Reg_String_Button(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
										   int rightEvent, int16_t x, int16_t y);

/* 0x45E620 */
int16_t register_Add_Key_To_Reg_String(struct REGISTER_RegStringButton* input, char* text, char character);

/* 0x45E660 */
void register_Set_Reg_String_Button_Name(struct REGISTER_RegStringButton* input, const char* name);

/* 0x45E6A0 */
int register_Get_Reg_String_Button_Name(const struct REGISTER_RegStringButton* input, char* destination);

/* 0x45E6D0 */
/* Schedules the dialog. The completion callback receives its original button ID on resume. */
void register_Do_Delete_Dialog(DialogSubResultHandler complete, void* context);

/* 0x45E740 */
Input* register_Build_Delete_Dialog(void);

/* 0x45E8C0 */
int16_t register_iupdate_Delete_Input(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
									  int rightEvent, int16_t x, int16_t y);

/* 0x45E9A0 */
void register_iuser_Delete_Input(Input* input, int context);

/* 0x45E9D0 */
void register_idraw_Delete_Input(Input* input, Rect* frame, Rect* clip, int16_t refresh);

/* 0x45EB60 */
void register_idraw_DialogButton(PushButton* button, Rect* frame, Rect* unusedClip, int16_t refresh);

/* 0x45EBD0 */
void register_Do_Protect_Dialog(DialogSubResultHandler complete, void* context);

/* 0x45EC70 */
Input* register_Build_Protect_Dialog(void);

/* 0x45EEA0 */
int16_t register_iupdate_Protect_Input(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
									   int rightEvent, int16_t x, int16_t y);

/* 0x45EF30 */
void register_iuser_Protect_Input(Input* input, int context);

/* 0x45F160 */
void register_idraw_Protect_Input(Input* input, Rect* frame, Rect* clip, int16_t refresh);

/* 0x45F2F0 */
int16_t register_Find_Reg_Dir_Name(Directory* directory, char* destination, int16_t index);

/* 0x45F9B0 */
void register_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x45FA20 */
void register_CloseMusic(void);

/* 0x45FB50 */
void register_user_Music(Sound* unusedSound, int unusedTime);

/* 0x45FBA0 */
void register_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm);

/* 0x45FC40 */
/* Modern callers must yield to a pushed speech task before continuing. */
void register_PlaySpeech(int16_t speechIndex, int unusedContext);

/* 0x45FCE0 */
void register_PlaySoundCue(int16_t cue);

#ifdef __cplusplus
}
#endif

#endif
