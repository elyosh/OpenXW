#ifndef XW_FLIGHT_FEDISKIO_H
#define XW_FLIGHT_FEDISKIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/assets/file.h"
#include "xw/flight/xw.h"
#include <stddef.h>
#include <stdint.h>
#include <xw/compiler.h>

enum {
	FEDISKIO_ERROR_ALLOCATION = 0,
	FEDISKIO_ERROR_FILE_ACCESS = 1,
	FEDISKIO_ERROR_COUNT = 2,
	FEDISKIO_FATAL_MESSAGE_CAPACITY = 128,
	FEDISKIO_FILENAME_CAPACITY = 256,
	FEDISKIO_BLOCK_BUFFER_CAPACITY = 64,
	FEDISKIO_FILE_READ_BUFFER_CAPACITY = 512
};

enum { FEDISKIO_CRAFT_DEFINITION_NONE = 255, FEDISKIO_SCALED_BOUND_LIMIT = 640 };

enum {
	FEDISKIO_SPECIES_LIST_COUNT = 4,
	FEDISKIO_SPECIES_LIST_NAME_CAPACITY = 9,
	FEDISKIO_SPECIES_LIST_PATH_CAPACITY = 60
};

enum { FEDISKIO_EXTENSION_LENGTH = 3, FEDISKIO_BACKGROUND_BLUE = 2 };

enum {
	FEDISKIO_PILOT_FILENAME_CAPACITY = 32,
	FEDISKIO_PROMOTION_TRAINING_LEVEL = 5,
	FEDISKIO_RANK_CADET = 0,
	FEDISKIO_RANK_OFFICER = 1,
	FEDISKIO_RANK_LIEUTENANT = 2,
	FEDISKIO_KILL_CATEGORY_COUNT = 24,
	FEDISKIO_SPECIAL_FIGHTER_CATEGORY = 17,
	FEDISKIO_SPECIAL_DESTROYER_CATEGORY = 18,
	FEDISKIO_TIE_FIGHTER_DEFINITION = 4,
	FEDISKIO_STAR_DESTROYER_DEFINITION = 51,
	FEDISKIO_LASER_HIT_SCORE = 3,
	FEDISKIO_SPACE_OBJECT_SCORE = 50,
	FEDISKIO_WARHEAD_HIT_WEIGHT = 2,
	FEDISKIO_SURFACE_KILL_SCORE = 20,
	FEDISKIO_CRAFT_KILL_SCORE = 40,
	FEDISKIO_CAPTURE_SCORE = 200,
	FEDISKIO_EJECTION_PENALTY = 5000,
	FEDISKIO_OBJECTIVE_BONUS = 1500,
	FEDISKIO_DEATH_STAR_OBJECTIVE_BONUS = 7500,
	FEDISKIO_MISSION_MODE_HISTORICAL = 1,
	FEDISKIO_MISSION_MODE_TOUR = 3,
	FEDISKIO_MISSION_MODE_COMBAT_TOUR = 5,
	FEDISKIO_CAREER_SKILL_SHIFT = 2,
	FEDISKIO_KALIDOR_LEVEL_COUNT = 6,
	FEDISKIO_PILOT_RANK_COUNT = 6,
	FEDISKIO_BONUS_HISTORICAL_SHIP = 4,
	FEDISKIO_HISTORICAL_COURSE_COUNT = 6,
	FEDISKIO_HISTORICAL_SKILL_LIMIT = 36000,
	FEDISKIO_HISTORICAL_SKILL_BONUS = 2000,
	FEDISKIO_BASE_TOUR_MEDAL_COUNT = 3,
	FEDISKIO_TOUR_MEDAL_BASE = 7
};

extern const int g_pilotPromotionScoreThresholdByRank[FEDISKIO_PILOT_RANK_COUNT];
extern const uint16_t g_kalidorAwardMissionScoreThreshold[FEDISKIO_KALIDOR_LEVEL_COUNT];
extern char g_speciesListNames[FEDISKIO_SPECIES_LIST_COUNT][FEDISKIO_SPECIES_LIST_NAME_CAPACITY];

enum {
	FEDISKIO_TINY_FONT_BUFFER_SIZE = 0x80E8,
	FEDISKIO_MICRO_FONT_BUFFER_SIZE = 0x3814,
	FEDISKIO_PANEL_BUFFER_SIZE = 0x32000,
	FEDISKIO_REPLAY_BUFFER_SIZE = 0x2000,
	FEDISKIO_INITIAL_BUFFER_BYTE = 0x40,
	FEDISKIO_TEXT_BACKGROUND_COLOR = 16,
	FEDISKIO_LOADING_TEXT_COLOR = 250,
	FEDISKIO_LOADING_TITLE_LINES = 5,
	FEDISKIO_LOADING_STATUS_LINES = 4,
	FEDISKIO_SOUND_PATH_CAPACITY = 40
};

/* Declarations follow ascending original IDB address. */

extern char g_flightResourcePrefix[];
extern const char* g_diskFatalErrorMessages[FEDISKIO_ERROR_COUNT];
extern char g_currentMissionFile[FEDISKIO_FILENAME_CAPACITY];
extern uint16_t g_replayBufferHandle;
extern uint16_t g_renderObjectListHandle;
extern uint16_t g_flightOffscreenBufferHandle;
extern uint8_t g_rgb565ToPaletteIndexStorage[];
extern uint16_t g_flightTinyFontHandle;
extern uint16_t g_hudPanelSpriteDataHandle;
extern uint16_t g_flightMicroFontHandle;
extern int g_paletteGenerationEnabled;
extern int g_generateMissionPalette;
extern int g_loadingModel;
extern uint8_t* g_rgb565ToPaletteIndexLut;
extern FlightEntryMode g_flightEntryMode;
extern int16_t g_fileBlockError;
extern uint8_t* gTinyFntBuf;
struct RgbTriplet;
extern struct RgbTriplet* g_flightAuxBuffer;
extern uint8_t* g_flightOffscreenBuffer;
extern void* g_FileLoadBuffer;
extern uint8_t* gMicroFntBuf;
extern int g_fediskioUnknownState;
extern XwFile* g_stream;
extern char g_fileName[FEDISKIO_FILENAME_CAPACITY];
extern int g_flightLoadingReplayFilm;
extern uint16_t g_flightAuxBufferHandle;

/* 0x4098B0 */
int16_t fediskio_updatepilotrecord(uint16_t objectRef, uint8_t lostStatus, int16_t ejected);

/* 0x40A020 */
int16_t fediskio_loadbufferdata(const char* fileName, uint16_t firstSpriteIndex, int16_t spriteCount,
								uint16_t recordsToSkip);

/* 0x40A100 */
uint16_t fediskio_readfiletofarmemory(const char* logicalPath, uint8_t* destination);

/* 0x40A1B0 */
void fediskio_Init_Buffers_and_Fonts(void);

/* 0x40A610 */
void fediskio_FreeFlightHandles(void);

/* 0x40A720 */
void fediskio_loadspecies(void);

/* 0x40AB10 */
unsigned int fediskio_InitResources(void);

/* 0x40AF10 */
void fediskio_fillinspec(uint8_t craftDefinitionIndex, uint8_t modelType);

/* 0x40AFC0 */
int16_t fediskio_tryopenfile(const char* logicalPath, const char* mode, int fatalOnFailure);

/* 0x40B130 */
int16_t fediskio_tryclosefile(int16_t deleteOnError);

/* 0x40B180 */
size_t fediskio_readfileblock(void* buffer, size_t elementSize, size_t elementCount, XwFile* stream);

/* 0x40B1C0 */
size_t fediskio_writefileblock(const void* buffer, size_t elementSize, size_t elementCount, XwFile* stream);

/* 0x40B200 */
int fediskio_ReadFileBlockBuffered(void* buffer, int elementSize, int elementCount, XwFile* stream);

/* 0x40B2B0 */
int fediskio_WriteFileBlockBuffered(const void* buffer, unsigned int elementSize, unsigned int elementCount,
									XwFile* stream);

/* 0x40B360 */
void XW_NORETURN fediskio_fatalerror(uint16_t errorIndex);

#ifdef __cplusplus
}
#endif

#endif
