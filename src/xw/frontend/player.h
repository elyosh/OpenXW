#ifndef XW_FRONTEND_PLAYER_H
#define XW_FRONTEND_PLAYER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/flight/mission/mission.h"
#include "xw/flight/player/user.h"
#include "xw/landru_config.h"

#include "xw/assets/file.h"
#include <landru/actor.h>
#include <landru/memhdl.h>
#include <landru/rect.h>
#include <stddef.h>
#include <stdint.h>

enum {
	PLAYER_PARAGRAPH_BUFFER_SIZE = 128,
	PLAYER_PARAGRAPH_FONT = 3,
	PLAYER_PARAGRAPH_LINE_HEIGHT = 20,
	PLAYER_PARAGRAPH_TEXT_COLOR = 47,
	PLAYER_PARAGRAPH_CENTER_COLOR = 59,
	PLAYER_PARAGRAPH_BOLD_COLOR = 2,
	PLAYER_PARAGRAPH_NORMAL_CONTROL = 1,
	PLAYER_PARAGRAPH_BOLD_CONTROL = 2,
	PLAYER_PARAGRAPH_INSET = 2,
	PLAYER_PARAGRAPH_NO_LINE = -1
};

enum {
	PLAYER_TEXT_VIEWPORT_COUNT = 4,
	PLAYER_TEXT_VIEWPORT_SCALE = 2,
	PLAYER_TEXT_VIEWPORT_BOTTOM = 398,
	PLAYER_TEXT_BACKGROUND_COLOR = 0,
	PLAYER_TEXT_VIEWPORT_COLOR = 1,
	PLAYER_TEXT_INSET_X = 2,
	PLAYER_TEXT_INSET_Y = 1
};

enum {
	PLAYER_GRID_DARK_MAJOR_COLOR = 40,
	PLAYER_GRID_DARK_MINOR_COLOR = 28,
	PLAYER_GRID_LIGHT_MAJOR_COLOR = 62,
	PLAYER_GRID_LIGHT_MINOR_COLOR = 63,
	PLAYER_GRID_HALF_DETAIL_SCALE = 16,
	PLAYER_GRID_FULL_DETAIL_SCALE = 32,
	PLAYER_GRID_MAJOR_INTERVAL = 4,
	PLAYER_GRID_CELL_FRACTION_BITS = 8,
	PLAYER_GRID_CELL_FRACTION_MASK = (1 << PLAYER_GRID_CELL_FRACTION_BITS) - 1
};

enum { PLAYER_CARGO_HUD_SHIP_COUNT = 5 };

enum { PLAYER_MAP_CRAFT_LIMIT = 28, PLAYER_MAP_RECORD_CAPACITY = 40 };

enum {
	PLAYER_MAP_ICON_ACTOR_COUNT = 3,
	PLAYER_MAP_ICON_BASE_COUNT = 25,
	PLAYER_MAP_ICON_LOW_ZOOM_THRESHOLD = 32,
	PLAYER_MAP_ICON_IFF0_LOW_ZOOM_OFFSET = 108,
	PLAYER_MAP_SELECTION_HALF_SIZE = 8,
	PLAYER_MAP_SELECTION_EXPANSION = 2,
	PLAYER_MAP_SELECTION_BACKGROUND = 52,
	PLAYER_MAP_SELECTION_FRAME_COLOR = 54,
	PLAYER_MAP_PLAYER_ICON_COLOR = 14
};

enum { PLAYER_MAP_WORLD_COORDINATE_SCALE = 256 };

enum { PLAYER_MAP_LABEL_BUFFER_SIZE = 64, PLAYER_MAP_INITIAL_RIGHT = 421, PLAYER_MAP_INITIAL_BOTTOM = 327 };

enum { PLAYER_MAP_INITIAL_PICK_DISTANCE = 999, PLAYER_MAP_PICK_RADIUS = 12 };

enum { PLAYER_MAP_BASE_ZOOM_LEVEL = 25, PLAYER_MAP_SHIFT_COUNT_MASK = 31 };

enum { PLAYER_MAP_BACKGROUND_COLOR = 24, PLAYER_STARS_BACKGROUND_COLOR = 16 };

enum {
	PLAYER_COMMAND_COUNT = 42,
	PLAYER_COMMAND_PARAMETER_CAPACITY = 4,
	PLAYER_COMMAND_CLEAR_TEXT = 10,
	PLAYER_COMMAND_FIRST_TEXT_SLOT = 11,
	PLAYER_COMMAND_LAST_TEXT_SLOT = 14,
	PLAYER_COMMAND_END = 41,
	PLAYER_COMMAND_HEADER_WORD_COUNT = 2,
	PLAYER_SCRIPT_SENTINEL_TIME = 9999,
	PLAYER_SCRIPT_DEFAULT_END_TIME = 200
};

enum { PLAYER_MISSION_PATH_CAPACITY = 256 };

enum {
	PLAYER_REPAIR_COLUMN_COUNT = 3,
	PLAYER_REPAIR_TITLE_CAPACITY = 24,
	PLAYER_REPAIR_VALUE_CAPACITY = 16,
	PLAYER_REPAIR_BORDER_X = 4,
	PLAYER_REPAIR_BORDER_Y = 3,
	PLAYER_REPAIR_CONTENT_INSET_X = 1,
	PLAYER_REPAIR_HEADER_HEIGHT = 14,
	PLAYER_REPAIR_HEADER_TOP = 6,
	PLAYER_REPAIR_HEADER_FONT = 2,
	PLAYER_REPAIR_ROW_FONT = 3,
	PLAYER_REPAIR_INITIAL_ROW_OFFSET = 16,
	PLAYER_REPAIR_HIGHLIGHT_TOP = 8,
	PLAYER_REPAIR_TEXT_TOP = 12,
	PLAYER_REPAIR_TIMER_INSET = 10,
	PLAYER_REPAIR_BACKGROUND = 0,
	PLAYER_REPAIR_BEVEL_TOP = 50,
	PLAYER_REPAIR_BEVEL_BOTTOM = 48,
	PLAYER_REPAIR_BEVEL_FILL = 49,
	PLAYER_REPAIR_SEPARATOR_COLOR = 16,
	PLAYER_REPAIR_DRAG_COLOR = 34,
	PLAYER_REPAIR_SELECTION_COLOR = 28,
	PLAYER_REPAIR_HEALTHY_COLOR = 15,
	PLAYER_REPAIR_FULL_HEALTH = 100,
	PLAYER_REPAIR_SECONDS_PER_MINUTE = 60,
	PLAYER_REPAIR_TWO_DIGIT_SECONDS = 10
};

extern char g_inflightSubsystemNames[XW_PLAYER_SUBSYSTEM_COUNT][PLAYER_REPAIR_TITLE_CAPACITY];
extern char g_inflightRepairColumnTitles[PLAYER_REPAIR_COLUMN_COUNT][PLAYER_REPAIR_TITLE_CAPACITY];
extern int16_t g_inflightRepairColumnX[PLAYER_REPAIR_COLUMN_COUNT];
extern uint16_t g_inflightPanelBoldColors[PLAYER_REPAIR_COLUMN_COUNT];
extern uint16_t g_inflightPanelTextColors[PLAYER_REPAIR_COLUMN_COUNT];
extern const char g_repairCompleteLabel[5];

extern XwMissionHeader g_inflightMissionHeader;

extern const int16_t g_inflightMapCommandParameterCounts[PLAYER_COMMAND_COUNT];
extern uint8_t g_inflightCargoHudShipIds[PLAYER_CARGO_HUD_SHIP_COUNT];
extern Actor* g_inflightMapStarsActor;
extern int16_t g_inflightMapShipCount;
extern LandruHandle g_inflightMapFlightGroupHandles[PLAYER_MAP_RECORD_CAPACITY];
extern Actor* g_inflightMapIconActors[PLAYER_MAP_ICON_ACTOR_COUNT];
extern LandruHandle g_inflightMapSecondaryRecordHandles[PLAYER_MAP_RECORD_CAPACITY];

/* Declarations follow ascending original IDB address. */

/* 0x451DC0 */
void player_Rewind_Page(int16_t scriptIndex);

/* 0x451E10 */
void player_Step_Page(int16_t scriptIndex);

/* 0x451FA0 */
int16_t player_Reseek_Page(int16_t scriptIndex);

/* 0x451FE0 */
int16_t player_Seek_Page(int16_t scriptIndex, int16_t targetTime);

/* 0x452050 */
int16_t player_DrawInflightMap(Rect* bounds, Rect* clip, int16_t refresh);

/* 0x452080 */
int16_t player_Draw_Display_Grid(Rect* bounds, Rect* unusedClip);

/* 0x452340 */
int16_t player_Draw_Display_Ship(Rect* frame, Rect* clip);

/* 0x4525B0 */
int16_t player_DrawBriefingText(Rect* frame, Rect* clip, int16_t refresh);

/* 0x452720 */
void player_Draw_Map_Paragraph(Rect* bounds, LandruHandle textHandle, LandruHandle attributeHandle,
							   int16_t literalMode);

/* 0x4529E0 */
int16_t player_DrawDamageControl(Rect* frame, Rect* clip, int16_t refresh);

/* 0x452CE0 */
void player_Stars_To_Back(void);

/* 0x452D50 */
void player_Init_Display_Map(void);

/* 0x452FC0 */
void player_Free_Display_Map(void);

/* 0x453080 */
void player_Clear_Page_Commands(int16_t scriptIndex);

/* 0x453120 */
void player_ApplyBriefingLayout(int16_t layoutIndex);

/* 0x4531D0 */
int player_Find_Ship_On_Screen(Rect* bounds, int16_t screenX, int16_t screenY, int16_t* selectedIndex);

/* 0x4532F0 */
int16_t player_VisibleIndexToObject(int16_t visibleShipIndex);

/* 0x453330 */
int16_t player_HasCargoReadout(int16_t objectIndex);

/* 0x453370 */
void player_SelectPlayerShip(void);

/* 0x453400 */
void player_OrderPendingRepairsFirst(void);

/* 0x453450 */
int16_t player_IsObjectVisible(int16_t objectIndex);

/* 0x453490 */
void player_Map_To_Screen_Pos(Rect* bounds, int16_t mapX, int16_t mapY, int16_t* screenX, int16_t* screenY);

/* 0x453520 */
void player_Screen_To_Map_Pos(Rect* bounds, int16_t screenX, int16_t screenY, int16_t* mapX, int16_t* mapY);

/* 0x4535B0 */
void player_LoadMissionRecords(const char* missionName);

/* 0x453870 */
int player_Load_Display_Map(const char* missionName);

/* 0x453B00 */
void player_ReadScripts(XwFile* stream);

/* 0x453C20 */
void player_ReadIconData(XwFile* stream);

/* 0x453EB0 */
void player_ReadLayout(XwFile* stream, int16_t layoutIndex);

/* 0x453F30 */
void player_ReadExtendedIconData(XwFile* stream);

/* 0x4542E0 */
void player_ReadTextBuffers(XwFile* stream);

#ifdef __cplusplus
}
#endif

#endif
