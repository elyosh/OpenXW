#include "xw/flight/fediskio.h"

#ifdef XW_MODERN
#include "xw_runtime/snapshot/render_cockpit_assets.h"
#endif
#ifdef XW_MODERN
#include "xw_runtime/snapshot/render_assets.h"
#endif

#ifdef XW_MODERN
#include "xw_runtime/runtime/flight_types.h"
#include "xw_runtime/storage/storage.h"
#endif

#include "xw/assets/bitmap.h"
#include "xw/assets/model_mesh.h"
#include "xw/audio/fsfx.h"
#include "xw/flight/death_star.h"
#include "xw/flight/hud/panel.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/object.h"
#include "xw/flight/player/user.h"
#include "xw/flight/shell_flight.h"
#include "xw/frontend/register.h"
#include "xw/frontend/shell.h"
#include "xw/frontend/shipext.h"
#include "xw/render/image_quantizer.h"
#include "xw/render/render_scene.h"
#include "xw/util/memory.h"
#include "xw/util/shared.h"

#include "xw/flight/feinput.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/hud/festring.h"
#include "xw/flight/replay/replay.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/render/renderer.h"
#include "xw/render/rotscale.h"
#include "xw/render/rtsrgb.h"
#include "xw/render/rtsvga2.h"

#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4C5378
char g_flightResourcePrefix[] = ";RESOURCE\\";

// GLOBAL: XW 0x4C53E0
const char* g_diskFatalErrorMessages[FEDISKIO_ERROR_COUNT] = {
	"Error! Not Enough Memory!\n", "Error! The following file is missing or inaccessible: "
};

// GLOBAL: XW 0x4C53E8
const int g_pilotPromotionScoreThresholdByRank[FEDISKIO_PILOT_RANK_COUNT] = { 20000,  50000,  100000,
																			  250000, 500000, -1 };

// GLOBAL: XW 0x4C5400
const uint16_t g_kalidorAwardMissionScoreThreshold[FEDISKIO_KALIDOR_LEVEL_COUNT] = { 10000, 12000, 14000,
																					 16000, 18000, 20000 };

// GLOBAL: XW 0x4C5410
char g_speciesListNames[FEDISKIO_SPECIES_LIST_COUNT][FEDISKIO_SPECIES_LIST_NAME_CAPACITY] = { "SPEC", "SPEC2",
																							  "SPEC3",
																							  "DSTAR" };

// GLOBAL: XW 0x4CEFB8
char g_currentMissionFile[FEDISKIO_FILENAME_CAPACITY] = "XTEST1.XWI";

// GLOBAL: XW 0x4E4130
uint16_t g_replayBufferHandle = 0;

// GLOBAL: XW 0x4E4134
uint16_t g_renderObjectListHandle = 0;

// GLOBAL: XW 0x4E4138
uint16_t g_flightOffscreenBufferHandle = 0;

// GLOBAL: XW 0x4E4140
uint8_t g_rgb565ToPaletteIndexStorage[RTSVGA2_RGB565_LUT_SIZE] = { 0 };

// GLOBAL: XW 0x4F4140
uint16_t g_flightTinyFontHandle = 0;

// GLOBAL: XW 0x4F4144
uint16_t g_hudPanelSpriteDataHandle = 0;

// GLOBAL: XW 0x4F4148
uint16_t g_flightMicroFontHandle = 0;

// GLOBAL: XW 0x4F414C
int g_paletteGenerationEnabled = 0;

// GLOBAL: XW 0x4F4A44
int g_generateMissionPalette = 0;

// GLOBAL: XW 0x5668A4
int g_loadingModel = 0;

// GLOBAL: XW 0x5BED9C
uint8_t* g_rgb565ToPaletteIndexLut = NULL;

// GLOBAL: XW 0x5E7514
FlightEntryMode g_flightEntryMode = FLIGHT_ENTRY_NEW_MISSION;

// GLOBAL: XW 0x62AFF0
int16_t g_fileBlockError = 0;

// GLOBAL: XW 0x62B00C
uint8_t* gTinyFntBuf = NULL;

// GLOBAL: XW 0x62B4F0
struct RgbTriplet* g_flightAuxBuffer = NULL;

// GLOBAL: XW 0x62B514
uint8_t* g_flightOffscreenBuffer = NULL;

// GLOBAL: XW 0x62BB20
void* g_FileLoadBuffer = NULL;

// GLOBAL: XW 0x637330
uint8_t* gMicroFntBuf = NULL;

// GLOBAL: XW 0x637820
XwCraftModelBounds g_craftModelBounds[XW_CRAFT_TYPE_COUNT] = { 0 };

// GLOBAL: XW 0x63BAA8
int g_fediskioUnknownState = 0;

// GLOBAL: XW 0x63BAB0
XwFile* g_stream = NULL;

// GLOBAL: XW 0x63BAC0
char g_fileName[FEDISKIO_FILENAME_CAPACITY] = { 0 };

// GLOBAL: XW 0x63BBC0
int g_flightLoadingReplayFilm = 0;

// GLOBAL: XW 0x63BBC4
uint16_t g_flightAuxBufferHandle = 0;

// FUNCTION: XW 0x4098B0
int16_t fediskio_updatepilotrecord(uint16_t objectRef, uint8_t lostStatus, int16_t ejected) {
	CraftData* craft;
	uint16_t pilotSlot;
	REGISTER_PilotFileRecord* pilot;
#ifdef XW_MODERN
	REGISTER_PilotFileRecord pilotRecord;
#endif
	char pilotFilename[FEDISKIO_PILOT_FILENAME_CAPACITY];
	if (g_replayviewmode != 0 || (uint8_t)g_flightDisplaySurfaceMode == 0)
		return 0;
	craft = g_objectTable[objectRef].instanceData;
	for (pilotSlot = 0; pilotSlot < XW_FLIGHT_PILOT_SLOT_COUNT; ++pilotSlot) {
		if (g_PilotObjectRefs[pilotSlot] == objectRef)
			break;
	}
	if (pilotSlot >= XW_FLIGHT_PILOT_SLOT_COUNT)
		return 0;
	strcpy(pilotFilename, g_PilotSlotNames[pilotSlot]);
	strcat(pilotFilename, ".PLT");
	if (fediskio_tryopenfile(pilotFilename, "rb", 1) == 0)
		return 0;
#ifdef XW_MODERN
	pilot = &pilotRecord;
	if (!XwPilot_Read(g_stream, pilot)) {
		fediskio_tryclosefile(0);
		return 0;
	}
#else
	pilot = g_FileLoadBuffer;
	fediskio_ReadFileBlockBuffered(pilot, sizeof(*pilot), 1, g_stream);
#endif
	fediskio_tryclosefile(0);
	if (g_missionRuntimeState.provingGroundsActive != 0) {
		uint8_t completedLevel = g_missionRuntimeState.provingGroundsLevel - 1;
		unsigned int* trainingHighScore =
			&pilot->trainingBestScores[g_missionRuntimeState.provingGroundsSelectedCraft - 1];
		uint8_t previousLevel;
		if ((unsigned int)g_missionRuntimeState.provingGroundsScore > *trainingHighScore)
			*trainingHighScore = g_missionRuntimeState.provingGroundsScore;
		previousLevel = pilot->trainingLevelProgress[g_missionRuntimeState.provingGroundsSelectedCraft - 1];
		if (completedLevel > previousLevel) {
			if (previousLevel < FEDISKIO_PROMOTION_TRAINING_LEVEL &&
				completedLevel >= FEDISKIO_PROMOTION_TRAINING_LEVEL && pilot->rank < FEDISKIO_RANK_OFFICER) {
				pilot->rank = FEDISKIO_RANK_OFFICER;
				g_missionRuntimeState.newRank = FEDISKIO_RANK_OFFICER;
			}
			pilot->trainingLevelProgress[g_missionRuntimeState.provingGroundsSelectedCraft - 1] =
				completedLevel;
		}
	} else {
		uint16_t opposingSide = g_playerFlightState.object->iff == 0;
		uint16_t typeIndex;
		int missionScore =
			FEDISKIO_LASER_HIT_SCORE *
				(craft->weaponStats.laserSurfaceHits + craft->weaponStats.laserSpacecraftHits) +
			FEDISKIO_SPACE_OBJECT_SCORE *
				(craft->killStats.spaceObjects +
				 FEDISKIO_WARHEAD_HIT_WEIGHT *
					 (craft->weaponStats.warheadSpacecraftHits + craft->weaponStats.warheadSurfaceHits) -
				 craft->weaponStats.warheadsFired) +
			FEDISKIO_SURFACE_KILL_SCORE * craft->killStats.deathStarBuildings -
			craft->weaponStats.laserShotsFired;
		for (typeIndex = 0; typeIndex < FEDISKIO_KILL_CATEGORY_COUNT; ++typeIndex) {
			uint16_t killValue;
			int scoreWithKills;
			int weightedCaptures;
#ifdef XW_MODERN
			killValue = XwFlightTypes_ScoreWeight(typeIndex);
#else
			if (typeIndex == FEDISKIO_SPECIAL_FIGHTER_CATEGORY)
				killValue = g_craftTypeDefs[FEDISKIO_TIE_FIGHTER_DEFINITION].killValue;
			else if (typeIndex == FEDISKIO_SPECIAL_DESTROYER_CATEGORY)
				killValue = g_craftTypeDefs[FEDISKIO_STAR_DESTROYER_DEFINITION].killValue;
			else
				killValue = g_craftTypeDefs[g_craftTypeToObjectType[typeIndex]].killValue;
#endif
			scoreWithKills = missionScore + FEDISKIO_CRAFT_KILL_SCORE *
												(killValue * craft->killStats.spacecraftByType[typeIndex]);
			weightedCaptures =
				killValue * g_missionRuntimeState.captureCountsByIffAndType[opposingSide][typeIndex];
			missionScore = scoreWithKills + FEDISKIO_CAPTURE_SCORE * weightedCaptures;
		}
		if (ejected != 0)
			missionScore -= FEDISKIO_EJECTION_PENALTY;
		if (g_missionRuntimeState.objectivesCompleted == 1) {
			if (g_deathStarSurfaceModeActive != 0)
				missionScore += FEDISKIO_DEATH_STAR_OBJECTIVE_BONUS;
			else
				missionScore += FEDISKIO_OBJECTIVE_BONUS;
		}
		if (missionScore < 0)
			missionScore = 0;
		if (g_missionRuntimeState.mode == FEDISKIO_MISSION_MODE_TOUR) {
			unsigned int careerScore;
			unsigned int quarterScore;
			pilot->lost_status = lostStatus;
			pilot->laser_shots_fired += craft->weaponStats.laserShotsFired;
			pilot->laser_spacecraft_hits += craft->weaponStats.laserSpacecraftHits;
			pilot->laser_surface_hits += craft->weaponStats.laserSurfaceHits;
			pilot->warheads_fired += craft->weaponStats.warheadsFired;
			pilot->warhead_spacecraft_hits += craft->weaponStats.warheadSpacecraftHits;
			pilot->warhead_surface_hits += craft->weaponStats.warheadSurfaceHits;
			pilot->total_kills += craft->killStats.spaceObjects;
			pilot->surfaceVictories += craft->killStats.deathStarBuildings;
			for (typeIndex = 0; typeIndex < FEDISKIO_KILL_CATEGORY_COUNT; ++typeIndex) {
				pilot->kills_by_type[typeIndex] += craft->killStats.spacecraftByType[typeIndex];
				pilot->total_kills += craft->killStats.spacecraftByType[typeIndex];
				pilot->captures_by_type[typeIndex] +=
					g_missionRuntimeState.captureCountsByIffAndType[opposingSide][typeIndex];
				pilot->total_captures +=
					g_missionRuntimeState.captureCountsByIffAndType[opposingSide][typeIndex];
			}
			if (ejected != 0)
				++pilot->ejections;
			careerScore = pilot->score + missionScore;
			pilot->score = careerScore;
			quarterScore = careerScore >> FEDISKIO_CAREER_SKILL_SHIFT;
			if (quarterScore > UINT16_MAX)
				quarterScore = UINT16_MAX;
			if ((uint16_t)quarterScore > pilot->skillValue)
				pilot->skillValue = (uint16_t)quarterScore;
			if (g_missionRuntimeState.objectivesCompleted == 1) {
				if (careerScore > (unsigned int)g_pilotPromotionScoreThresholdByRank[pilot->rank]) {
					uint8_t newRank = pilot->rank + 1;
					pilot->rank = newRank;
					g_missionRuntimeState.newRank = newRank;
				}
				if (pilot->kalidorAwardLevel < FEDISKIO_KALIDOR_LEVEL_COUNT &&
					missionScore > g_kalidorAwardMissionScoreThreshold[pilot->kalidorAwardLevel]) {
					g_missionRuntimeState.newMedal = pilot->kalidorAwardLevel + 1;
					++pilot->kalidorAwardLevel;
				}
			}
		}
		if (objectRef == g_playerFlightState.objectIndex) {
			g_missionRuntimeState.provingGroundsScore = missionScore;
			if (g_missionRuntimeState.mode == FEDISKIO_MISSION_MODE_HISTORICAL) {
				unsigned int* historicalHighScore;
				uint8_t* historicalCompletion;
				uint8_t* shipCompletion;
				if (pilot->combatShip == FEDISKIO_BONUS_HISTORICAL_SHIP) {
					historicalHighScore = &pilot->bonusHistoricBestScores[pilot->combatMission];

				} else {
					historicalHighScore = &pilot->historicBestScores[pilot->combatShip][pilot->combatMission];
				}
				if ((unsigned int)missionScore > *historicalHighScore)
					*historicalHighScore = missionScore;
				if (pilot->combatShip == FEDISKIO_BONUS_HISTORICAL_SHIP)
					shipCompletion = pilot->bonusHistoricCompleted;
				else
					shipCompletion = pilot->combatAwards[pilot->combatShip].missionPatch;
				historicalCompletion = &shipCompletion[pilot->combatMission];
				if (*historicalCompletion == 0) {
					*historicalCompletion = g_missionRuntimeState.objectivesCompleted;
					if (g_missionRuntimeState.objectivesCompleted != 0) {
						uint8_t shipCoursesComplete = 1;
						uint8_t firstThreeShipsComplete = 1;
						unsigned int courseIndex;
						if (g_missionCheatOptionsUsed == 0) {
							if (pilot->combatShip == FEDISKIO_BONUS_HISTORICAL_SHIP)
								g_missionRuntimeState.newBattlePatch = 0;
							else
								g_missionRuntimeState.newBattlePatch =
									pilot->combatMission +
									FEDISKIO_HISTORICAL_COURSE_COUNT * pilot->combatShip + 1;
						}
						if (pilot->skillValue < FEDISKIO_HISTORICAL_SKILL_LIMIT)
							pilot->skillValue += FEDISKIO_HISTORICAL_SKILL_BONUS;
						for (courseIndex = 0; courseIndex < FEDISKIO_HISTORICAL_COURSE_COUNT; ++courseIndex) {
							shipCoursesComplete &= shipCompletion[courseIndex];
							firstThreeShipsComplete &=
								pilot->combatAwards[SHIPEXT_SHIP_YWING].missionPatch[courseIndex] &
								pilot->combatAwards[SHIPEXT_SHIP_XWING].missionPatch[courseIndex] &
								pilot->combatAwards[SHIPEXT_SHIP_AWING].missionPatch[courseIndex];
						}
						if (firstThreeShipsComplete != 0) {
							if (pilot->rank < FEDISKIO_RANK_LIEUTENANT) {
								pilot->rank = FEDISKIO_RANK_LIEUTENANT;
								g_missionRuntimeState.newRank = FEDISKIO_RANK_LIEUTENANT;
							}
						} else if (shipCoursesComplete != 0 && pilot->rank < FEDISKIO_RANK_OFFICER) {
							pilot->rank = FEDISKIO_RANK_OFFICER;
							g_missionRuntimeState.newRank = FEDISKIO_RANK_OFFICER;
						}
					}
				}
			} else if (g_missionRuntimeState.mode == FEDISKIO_MISSION_MODE_TOUR) {
				uint8_t currentTour = pilot->current_tour;
				uint16_t completedMission = pilot->selectedTourMission;
				XwTourOperation* tourSteps = g_tourOperationTables[currentTour];
				if ((unsigned int)missionScore > pilot->tourMissionScores[currentTour][completedMission])
					pilot->tourMissionScores[currentTour][completedMission] = missionScore;
				++pilot->field_281;
				if (g_missionRuntimeState.flightExitReason < SHIPEXT_EXIT_RESCUED) {
					unsigned int tourIndex;
					for (tourIndex = 0; tourIndex < SHIPEXT_TOUR_COUNT; ++tourIndex) {
						if (pilot->tour_status[tourIndex] == SHIPEXT_TOUR_STATUS_ACTIVE)
							pilot->tour_status[tourIndex] = SHIPEXT_TOUR_STATUS_LOST;
					}
				} else if (g_missionRuntimeState.objectivesCompleted == 1) {
					uint16_t branchMissionA, branchMissionB;
					g_missionRuntimeState.tourCutsceneIndex =
						tourSteps[pilot->currentTourOperation].cutsceneIndex;
					branchMissionA = tourSteps[pilot->currentTourOperation].missionChoiceA;
					branchMissionB = tourSteps[pilot->currentTourOperation].missionChoiceB;
					pilot->tourReplayUnlockMission[currentTour] = branchMissionA;
					if (branchMissionB != SHIPEXT_TOUR_NO_ENTRY)
						pilot->tourReplayUnlockMission[currentTour] = branchMissionB;
					if (completedMission == branchMissionA && branchMissionB != SHIPEXT_TOUR_NO_ENTRY)
						pilot->tourMissionScores[currentTour][branchMissionB] = 0;
					if (completedMission == branchMissionB && branchMissionA != SHIPEXT_TOUR_NO_ENTRY)
						pilot->tourMissionScores[currentTour][branchMissionA] = 0;
					++pilot->currentTourOperation;
					pilot->tourOperationProgress[currentTour] = pilot->currentTourOperation;
					if (pilot->currentTourOperation >= g_tourOperationCounts[currentTour]) {
						pilot->tour_status[currentTour] = SHIPEXT_TOUR_STATUS_UNSELECTABLE;
						if (currentTour < FEDISKIO_BASE_TOUR_MEDAL_COUNT)
							pilot->uniformMedals[currentTour] = 1;
						else
							pilot->expansionMedals[currentTour - FEDISKIO_BASE_TOUR_MEDAL_COUNT] = 1;
						g_missionRuntimeState.newMedal = currentTour + FEDISKIO_TOUR_MEDAL_BASE;
					} else {
						pilot->briefingMissionChoices[0] =
							tourSteps[pilot->currentTourOperation].missionChoiceA;
						pilot->briefingMissionChoices[1] =
							tourSteps[pilot->currentTourOperation].missionChoiceB;
					}
				}
			}
		}
	}
	if (g_missionCheatOptionsUsed == 0 || objectRef != g_playerFlightState.objectIndex) {
		if (fediskio_tryopenfile(pilotFilename, "wb", 1) == 0)
			return 0;
		fediskio_WriteFileBlockBuffered(pilot, sizeof(*pilot), 1, g_stream);
		fediskio_tryclosefile(0);
	}
	g_PilotObjectRefs[pilotSlot] = UINT8_MAX;
	return 1;
}

// FUNCTION: XW 0x40A020
int16_t fediskio_loadbufferdata(const char* fileName, uint16_t firstSpriteIndex, int16_t spriteCount,
								uint16_t recordsToSkip) {
	int16_t recordIndex;
	int16_t remainingSprites;
	uint8_t* dataStart;
	size_t byteIndex;
	fediskio_tryopenfile(fileName, "rb", 1);
#ifdef XW_MODERN
	XwCockpitAssets_Panel(XwRenderAssets_RegisterStream(XW_SOURCE_PANEL, 0, g_stream, XwStorage_LastPath()),
						  firstSpriteIndex, spriteCount, recordsToSkip);
#endif
	remainingSprites = spriteCount;
	dataStart = g_hudPanelSpriteDataWriteCursor;
	byteIndex = 0;
	for (recordIndex = 0; remainingSprites != 0; ++recordIndex) {
		int16_t byteValue;
		g_hudPanelSpriteDataByIndex[firstSpriteIndex] = g_hudPanelSpriteDataWriteCursor;
		if (recordIndex >= (int)recordsToSkip)
			++firstSpriteIndex;
		for (byteValue = File_RawGetChar(g_stream); !File_RawAtEnd(g_stream);
			 byteValue = File_RawGetChar(g_stream)) {
			if (byteValue == PANEL_SPRITE_RECORD_END)
				break;
			if (recordIndex >= (int)recordsToSkip) {
				dataStart[byteIndex++] = byteValue;
				g_hudPanelSpriteDataWriteCursor = &dataStart[byteIndex];
			}
		}
		if (recordIndex >= (int)recordsToSkip) {
			dataStart[byteIndex++] = PANEL_SPRITE_RECORD_END;
			--remainingSprites;
			g_hudPanelSpriteDataWriteCursor = &dataStart[byteIndex];
		}
	}
	return fediskio_tryclosefile(0);
}

// FUNCTION: XW 0x40A100
uint16_t fediskio_readfiletofarmemory(const char* logicalPath, uint8_t* destination) {
	uint8_t readBuffer[FEDISKIO_FILE_READ_BUFFER_CAPACITY];
	unsigned int totalBytes;
	fediskio_tryopenfile(logicalPath, "rb", 1);
#ifdef XW_MODERN
	if (destination == gTinyFntBuf || destination == gMicroFntBuf)
		XwRenderAssets_RegisterStream(
			XW_SOURCE_FONT, destination == gTinyFntBuf ? g_flightTinyFontHandle : g_flightMicroFontHandle,
			g_stream, XwStorage_LastPath());
	else if (strstr(logicalPath, "vga.pac"))
		XwRenderAssets_RegisterStream(XW_SOURCE_PALETTE, 0, g_stream, XwStorage_LastPath());
#endif
	totalBytes = 0;
	if (g_stream != NULL) {
		uint16_t bytesRead;
		for (bytesRead = sizeof(readBuffer); bytesRead == sizeof(readBuffer); totalBytes += bytesRead) {
			bytesRead = File_RawRead(readBuffer, sizeof(readBuffer[0]), bytesRead, g_stream);
			if (bytesRead > 0)
				memcpy(&destination[totalBytes], readBuffer, bytesRead);
		}
	}
	fediskio_tryclosefile(0);
	return totalBytes;
}

// FUNCTION: XW 0x40A1B0
void fediskio_Init_Buffers_and_Fonts(void) {
	int16_t allocationFailed;
	int paletteIndex;
	char archivePath[FEDISKIO_SOUND_PATH_CAPACITY];
	allocationFailed = 0;
	g_flightTinyFontHandle = Memory_AllocHandle(FEDISKIO_TINY_FONT_BUFFER_SIZE, 0);
	if (g_flightTinyFontHandle == 0)
		allocationFailed = 1;
	g_flightMicroFontHandle = Memory_AllocHandle(FEDISKIO_MICRO_FONT_BUFFER_SIZE, 0);
	if (g_flightMicroFontHandle == 0)
		allocationFailed = 1;
	g_flightOffscreenBufferHandle = Memory_AllocHandle(g_flightScreenHeight * g_surfacePitch, 0);
	if (g_flightOffscreenBufferHandle == 0)
		allocationFailed = 1;
	g_flightAuxBufferHandle = Memory_AllocHandle(g_flightScreenHeight * g_surfacePitch, 0);
	if (g_flightAuxBufferHandle == 0)
		allocationFailed = 1;
	g_hudPanelSpriteDataHandle = Memory_AllocHandle(FEDISKIO_PANEL_BUFFER_SIZE, 0);
	if (g_hudPanelSpriteDataHandle == 0)
		allocationFailed = 1;
#ifdef XW_MODERN
	g_replayBufferHandle = Memory_AllocHandle(REPLAY_BUFFER_CAPACITY, 0);
#else
	g_replayBufferHandle = Memory_AllocHandle(FEDISKIO_REPLAY_BUFFER_SIZE, 0);
#endif
	if (g_replayBufferHandle == 0)
		allocationFailed = 1;
	g_renderObjectListHandle =
		Memory_AllocHandle(XW_RENDER_OBJECT_LIST_CAPACITY * sizeof(*g_renderObjectListEntries), 0);
	if (g_renderObjectListHandle == 0)
		allocationFailed = 1;
	g_fediskioUnknownState = 0;
	if (allocationFailed)
		fediskio_fatalerror(FEDISKIO_ERROR_ALLOCATION);
	g_renderObjectListEntries = (RenderObjectListEntry*)Memory_LockHandle(g_renderObjectListHandle);
	gTinyFntBuf = (uint8_t*)Memory_LockHandle(g_flightTinyFontHandle);
	gMicroFntBuf = (uint8_t*)Memory_LockHandle(g_flightMicroFontHandle);
	g_flightOffscreenBuffer = (uint8_t*)Memory_LockHandle(g_flightOffscreenBufferHandle);
	rotscale_SetRotatedSpriteDestBuffer(g_flightOffscreenBuffer);
	memset(g_flightOffscreenBuffer, FEDISKIO_INITIAL_BUFFER_BYTE,
		   g_flightScreenWidth * g_flightScreenHeight * g_flightBytesPerPixel);
	g_flightAuxBuffer = (RgbTriplet*)Memory_LockHandle(g_flightAuxBufferHandle);
	g_FileLoadBuffer = g_flightAuxBuffer;
	g_ReplayBufferStart = (uint8_t*)Memory_LockHandle(g_replayBufferHandle);
	g_hudPanelSpriteDataBuffer = (uint8_t*)Memory_LockHandle(g_hudPanelSpriteDataHandle);
	for (paletteIndex = 0; paletteIndex <= RTSVGA2_MODEL_PALETTE_FIRST_COLOR - 1; ++paletteIndex) {
		g_swPalette[paletteIndex].r = 0;
		g_swPalette[paletteIndex].g = 0;
		g_swPalette[paletteIndex].b = 0;
	}
	g_flightTextPalette[0] = 0;
	if (g_flightBytesPerPixel == FLIGHT_DISPLAY_INDEXED_BYTES_PER_PIXEL)
		festring_setbackcolor(FEDISKIO_TEXT_BACKGROUND_COLOR);
	else
		festring_setbackcolor(0);
	fediskio_readfiletofarmemory(";vga.pac", (uint8_t*)g_flightAuxBuffer);
	g_flightSetPaletteRangeFn(g_flightAuxBuffer, RTSVGA2_MODEL_PALETTE_FIRST_COLOR,
							  RTSVGA2_PALETTE_COLOR_COUNT - RTSVGA2_MODEL_PALETTE_FIRST_COLOR);
	rtsvga2_unblankVGA();
	switch ((int)g_flightResolutionMode) {
		case RTSVGA2_MODE_13H:
			fediskio_readfiletofarmemory(";TINY.FNT", gTinyFntBuf);
			fediskio_readfiletofarmemory(";MICRO.FNT", gMicroFntBuf);
			break;
		case FLIGHT_DISPLAY_MODE_101H:
		case FLIGHT_DISPLAY_MODE_111H:
		case FLIGHT_DISPLAY_MODE_1FFH:
			fediskio_readfiletofarmemory(";TINY64.FNT", gTinyFntBuf);
			fediskio_readfiletofarmemory(";MICRO64.FNT", gMicroFntBuf);
			break;
		default:
			fediskio_readfiletofarmemory(";TINY.FNT", gTinyFntBuf);
			fediskio_readfiletofarmemory(";MICRO.FNT", gMicroFntBuf);
			break;
	}
	festring_setfontsize(FLIGHT_FONT_TINY);
	festring_setbound(0, 0, g_flightScreenWidth, g_flightScreenHeight);
	g_flightTextColorIndex = FEDISKIO_LOADING_TEXT_COLOR;
	g_flightTextShadowColor = 0;
	g_flightTextShadowEnabled = 0;
	festring_setcursor(0,
					   (g_flightScreenHeight >> 1) - FEDISKIO_LOADING_TITLE_LINES * g_flightFontLineHeight);
	if (g_flightLoadingReplayFilm != 0)
		festring_outstringcenter("Loading Film...");
	else if (g_flightEntryMode == FLIGHT_ENTRY_RESUME_SAVED)
		festring_outstringcenter("Re-Entering Combat");
	else
		festring_outstringcenter("Initializing Combat Sequence...");
	if (g_flightLoadingReplayFilm == 0 && g_flightEntryMode != FLIGHT_ENTRY_RESUME_SAVED &&
		g_shellPreferences.resolutionIndex == XW_SHELL_RESOLUTION_HARDWARE && g_useHardware3D == 0) {
		festring_setcursor(0, (g_flightScreenHeight >> 1) + (g_flightFontLineHeight >> 1) -
								  FEDISKIO_LOADING_STATUS_LINES * g_flightFontLineHeight);
		festring_outstringcenter("Cannot find 3D hardware, using Software mode");
		switch (g_flightResolutionMode) {
			case RTSVGA2_MODE_13H:
				g_shellPreferences.resolutionIndex = XW_SHELL_RESOLUTION_320_200;
				break;
			case FLIGHT_DISPLAY_MODE_101H:
				g_shellPreferences.resolutionIndex = XW_SHELL_RESOLUTION_640_480_INDEXED;
				break;
			case FLIGHT_DISPLAY_MODE_111H:
				g_shellPreferences.resolutionIndex = XW_SHELL_RESOLUTION_640_480_RGB;
				break;
		}
		g_savedShellPreferences.preferences = g_shellPreferences;
		ShellPreferences_Save();
	}
	strcpy(archivePath, g_flightResourcePrefix);
	strcat(archivePath, "BLAST.LFD");
	fsfx_loadsfx(archivePath, FSFX_BLAST_RESOURCE_TAG);
	festring_setcursor(0,
					   (g_flightScreenHeight >> 1) - FEDISKIO_LOADING_STATUS_LINES * g_flightFontLineHeight);
}

// FUNCTION: XW 0x40A610
void fediskio_FreeFlightHandles(void) {
	int cockpitIndex;
	int modelIndex;
	j_Sound_UnloadAllEffects();
	Memory_FreeHandle(g_renderObjectListHandle);
	Memory_FreeHandle(g_flightTinyFontHandle);
	Memory_FreeHandle(g_flightMicroFontHandle);
	Memory_FreeHandle(g_flightOffscreenBufferHandle);
	Memory_FreeHandle(g_flightAuxBufferHandle);
	Memory_FreeHandle(g_hudPanelSpriteDataHandle);
	Memory_FreeHandle(g_replayBufferHandle);
	for (cockpitIndex = 0; cockpitIndex < PANEL_COCKPIT_DESCRIPTOR_COUNT; ++cockpitIndex) {
		if (g_hudCockpitResources[cockpitIndex].memoryHandle != 0) {
			Memory_FreeHandle(g_hudCockpitResources[cockpitIndex].memoryHandle);
			g_hudCockpitResources[cockpitIndex].memoryHandle = 0;
		}
	}
	for (modelIndex = 0; modelIndex < MODEL_TYPE_RECORD_COUNT; ++modelIndex) {
		uint16_t modelHandle = g_modelTypeTable[modelIndex].memoryHandle;
		if (modelHandle != 0) {
			int previousIndex;
			for (previousIndex = 0; previousIndex < modelIndex; ++previousIndex) {
				if (g_modelTypeTable[previousIndex].memoryHandle == modelHandle)
					break;
			}
			if (previousIndex >= modelIndex)
				Memory_FreeHandle(modelHandle);
		}
	}
	memset(g_loadedModels, 0, sizeof(g_loadedModels));
	for (modelIndex = 0; modelIndex < MODEL_TYPE_RECORD_COUNT; ++modelIndex)
		g_modelTypeTable[modelIndex].memoryHandle = 0;
	RenderScene_FreeBuffers();
}

// FUNCTION: XW 0x40A720
void fediskio_loadspecies(void) {
	int modelIndex;
	int16_t listIndex;
	uint16_t modelHandle;
	char logicalPath[FEDISKIO_SPECIES_LIST_PATH_CAPACITY];
	char modelPath[FEDISKIO_FILENAME_CAPACITY];

	memset(g_loadedModels, 0, sizeof(g_loadedModels));
	for (modelIndex = 0; modelIndex != MODEL_TYPE_RECORD_COUNT; ++modelIndex)
		g_modelTypeTable[modelIndex].memoryHandle = 0;
	g_sceneEdgeFlagsCapacity = 0;
	g_vertexRemapCapacity = 0;
	for (listIndex = 0; (uint16_t)listIndex < FEDISKIO_SPECIES_LIST_COUNT; ++listIndex) {
		XwFile* listStream;
		uint16_t lineIndex;
		const char* resolutionSuffix;
		strcpy(logicalPath, "ivfiles\\");
		strcat(logicalPath, g_speciesListNames[listIndex]);
		resolutionSuffix = "640";
		if (g_flightResolutionMode == RTSVGA2_MODE_13H)
			resolutionSuffix = "320";
		strcat(logicalPath, resolutionSuffix);
		strcat(logicalPath, ".LST");
		fediskio_tryopenfile(logicalPath, "rb", 1);
		listStream = g_stream;
		lineIndex = 0;
		while (File_RawReadLine(modelPath, sizeof(modelPath), listStream) != NULL) {
			int characterIndex;
			uint8_t selected = 0;
			uint8_t resourceFlags;
			uint16_t typeIndex;
			for (characterIndex = 0; modelPath[characterIndex] != '\n' && modelPath[characterIndex] != '\r';
				 ++characterIndex) {
#ifdef XW_MODERN
				if (modelPath[characterIndex] == '\0')
					break;
#endif
			}
			modelPath[characterIndex] = '\0';
			if (modelPath[0] == '\0')
				continue;
			++lineIndex;
			for (modelIndex = 0; modelIndex != MODEL_TYPE_RECORD_COUNT; ++modelIndex) {
				uint8_t flags;
				if ((g_modelTypeTable[modelIndex].resourceFlags & MODEL_TYPE_LOAD_FROM_LIST) == 0 ||
					g_modelTypeTable[modelIndex].resourceListIndex != listIndex ||
					g_modelTypeTable[modelIndex].resourceEntryIndex != lineIndex - 1)
					continue;
				flags = g_modelTypeTable[modelIndex].flags;
				if (g_deathStarSurfaceModeActive != 0) {
					if ((flags & MODEL_TYPE_FLAG_LOAD_DEATH_STAR) == 0)
						continue;
				} else if (g_missionRuntimeState.provingGroundsActive != 0) {
					if ((flags & MODEL_TYPE_FLAG_LOAD_PROVING_GROUNDS) == 0)
						continue;
				} else if ((flags & MODEL_TYPE_FLAG_LOAD_COMMON) == 0) {
					continue;
				}
				if ((flags & MODEL_TYPE_FLAG_CRAFT_RECORD) == 0 ||
					g_missionRuntimeState.provingGroundsActive != 0) {
					selected = 1;
					resourceFlags = flags;
				}
			}
			if (selected == 0)
				continue;
			if ((resourceFlags & MODEL_TYPE_FLAG_OPT_MESH) != 0) {
				modelHandle = OptModel_LoadHandle(modelPath);
				Memory_LockHandle(modelHandle);
			} else if ((resourceFlags & MODEL_TYPE_FLAG_BITMAP) != 0) {
				XwFile* modelStream;
				int32_t fileSize;
				int32_t paletteEntryCount;
				XwBitmapModelPrefix* bitmap;
				fediskio_tryopenfile(modelPath, "rb", 1);
				modelStream = g_stream;
				fediskio_readfileblock(&fileSize, sizeof(fileSize), 1, modelStream);
				fediskio_readfileblock(&paletteEntryCount, sizeof(paletteEntryCount), 1, modelStream);
				modelHandle = Memory_AllocHandle(fileSize + g_flightBytesPerPixel * paletteEntryCount, 0);
				if (modelHandle == 0)
					fediskio_fatalerror(FEDISKIO_ERROR_ALLOCATION);
#ifdef XW_MODERN
				XwRenderAssets_RegisterStream(XW_SOURCE_BITMAP, modelHandle, modelStream,
											  XwStorage_LastPath());
#endif
				bitmap = Memory_LockHandle(modelHandle);
				bitmap->serializedSize = fileSize;
				bitmap->runtimePaletteEntryCount = paletteEntryCount;
				fediskio_readfileblock(bitmap->gap08,
									   fileSize - sizeof(bitmap->serializedSize) -
										   sizeof(bitmap->runtimePaletteEntryCount),
									   1, modelStream);
				g_stream = modelStream;
				fediskio_tryclosefile(0);
				if (g_flightBytesPerPixel == FLIGHT_DISPLAY_DIRECT_BYTES_PER_PIXEL)
					rtsvga2_Convert24BppPalettesTo16Bpp(bitmap);
				else
					rtsvga2_Convert24BppPalettesTo8Bpp(bitmap);
			}
			for (typeIndex = 0; typeIndex < MODEL_TYPE_RECORD_COUNT; ++typeIndex) {
				uint8_t flags;
				if ((g_modelTypeTable[typeIndex].resourceFlags & MODEL_TYPE_LOAD_FROM_LIST) == 0 ||
					g_modelTypeTable[typeIndex].resourceListIndex != listIndex ||
					g_modelTypeTable[typeIndex].resourceEntryIndex != lineIndex - 1)
					continue;
				flags = g_modelTypeTable[typeIndex].flags;
				if (g_deathStarSurfaceModeActive != 0) {
					if ((flags & MODEL_TYPE_FLAG_LOAD_DEATH_STAR) == 0)
						continue;
				} else if (g_missionRuntimeState.provingGroundsActive != 0) {
					if ((flags & MODEL_TYPE_FLAG_LOAD_PROVING_GROUNDS) == 0)
						continue;
				} else if ((flags & MODEL_TYPE_FLAG_LOAD_COMMON) == 0) {
					continue;
				}
				if ((flags & MODEL_TYPE_FLAG_CRAFT_RECORD) == 0 ||
					g_missionRuntimeState.provingGroundsActive != 0) {
					uint8_t craftDefinitionIndex;
					g_modelTypeTable[typeIndex].memoryHandle = modelHandle;
					g_loadedModels[typeIndex] = modelHandle;
					craftDefinitionIndex = g_modelTypeTable[typeIndex].craftDefinitionIndex;
					if ((resourceFlags & MODEL_TYPE_FLAG_OPT_MESH) != 0)
						fediskio_fillinspec(craftDefinitionIndex, typeIndex);
				}
			}
			/* The folded unlock backend ignores its original handle argument. */
			nullsub_SharedNoOp();
		}
		g_stream = listStream;
		fediskio_tryclosefile(0);
	}
	RenderScene_AllocateBuffers();
}

// FUNCTION: XW 0x40AB10
unsigned int fediskio_InitResources(void) {
	char logicalPath[FEDISKIO_FILENAME_CAPACITY];

	g_generateMissionPalette &= g_paletteGenerationEnabled;
	if (g_flightBytesPerPixel == FLIGHT_DISPLAY_INDEXED_BYTES_PER_PIXEL) {
		size_t extensionIndex;
		char originalExtension[FEDISKIO_EXTENSION_LENGTH];
		g_rgb565ToPaletteIndexLut = g_rgb565ToPaletteIndexStorage;
		extensionIndex = strlen(g_currentMissionFile) - sizeof(originalExtension);
		originalExtension[0] = g_currentMissionFile[extensionIndex];
		originalExtension[1] = g_currentMissionFile[extensionIndex + 1];
		originalExtension[2] = g_currentMissionFile[extensionIndex + 2];
		g_currentMissionFile[extensionIndex] = 'i';
		g_currentMissionFile[extensionIndex + 1] = 'n';
		g_currentMissionFile[extensionIndex + 2] = 'v';
		if (fediskio_tryopenfile(g_currentMissionFile, "rb", 0) == 0) {
			strcpy(logicalPath, "newpal.inv");
			if (fediskio_tryopenfile(logicalPath, "rb", 0) == 0) {
				rtsvga2_BuildRgb565ToPaletteIndexLut(g_rgb565ToPaletteIndexLut,
													 RTSVGA2_MODEL_PALETTE_FIRST_COLOR,
													 RTSVGA2_PALETTE_COLOR_COUNT);
				if (fediskio_tryopenfile(g_currentMissionFile, "wb", 0) != 0) {
					File_RawWrite(g_rgb565ToPaletteIndexLut, RTSVGA2_RGB565_LUT_ROW_SIZE,
								  RTSVGA2_RGB565_LUT_SIZE / RTSVGA2_RGB565_LUT_ROW_SIZE, g_stream);
					fediskio_tryclosefile(1);
				}
			} else {
				fediskio_tryclosefile(0);
				fediskio_readfiletofarmemory(logicalPath, g_rgb565ToPaletteIndexLut);
			}
		} else {
			fediskio_tryclosefile(0);
			fediskio_readfiletofarmemory(g_currentMissionFile, g_rgb565ToPaletteIndexLut);
		}
		g_currentMissionFile[extensionIndex] = originalExtension[0];
		g_currentMissionFile[extensionIndex + 1] = originalExtension[1];
		g_currentMissionFile[extensionIndex + 2] = originalExtension[2];
	}
	if (g_generateMissionPalette != 0 && g_flightBytesPerPixel == FLIGHT_DISPLAY_INDEXED_BYTES_PER_PIXEL)
		ImageQuantizer_w_InitializeColorTree(RTSVGA2_MODEL_PALETTE_COLOR_COUNT, IMAGE_QUANTIZER_MAX_DEPTH);
	g_loadingModel = 1;
	fediskio_loadspecies();
	g_loadingModel = 0;
	ModelMesh_BuildObjectTypeMeshCache();
	if (g_generateMissionPalette != 0 && g_flightBytesPerPixel == FLIGHT_DISPLAY_INDEXED_BYTES_PER_PIXEL) {
		size_t extensionIndex;
		char originalExtension[FEDISKIO_EXTENSION_LENGTH];
		int colorIndex;
		ImageQuantizer_ExportPalette6BitAndDestroy(RTSVGA2_MODEL_PALETTE_COLOR_COUNT,
												   IMAGE_QUANTIZER_MAX_DEPTH,
												   &g_swPalette[RTSVGA2_MODEL_PALETTE_FIRST_COLOR].r);
		extensionIndex = strlen(g_currentMissionFile) - sizeof(originalExtension);
		originalExtension[0] = g_currentMissionFile[extensionIndex];
		originalExtension[1] = g_currentMissionFile[extensionIndex + 1];
		originalExtension[2] = g_currentMissionFile[extensionIndex + 2];
		g_currentMissionFile[extensionIndex] = 'p';
		g_currentMissionFile[extensionIndex + 1] = 'a';
		g_currentMissionFile[extensionIndex + 2] = 'l';
		if (fediskio_tryopenfile(g_currentMissionFile, "wb", 0) != 0) {
			File_RawWrite(&g_swPalette[RTSVGA2_MODEL_PALETTE_FIRST_COLOR],
						  sizeof(g_swPalette[0]) * RTSVGA2_MODEL_PALETTE_COLOR_COUNT, 1, g_stream);
			fediskio_tryclosefile(1);
		}
		g_rgb565ToPaletteIndexLut = g_rgb565ToPaletteIndexStorage;
		rtsvga2_BuildRgb565ToPaletteIndexLut(g_rgb565ToPaletteIndexStorage, RTSVGA2_MODEL_PALETTE_FIRST_COLOR,
											 RTSVGA2_PALETTE_COLOR_COUNT);
		g_currentMissionFile[extensionIndex] = 'i';
		g_currentMissionFile[extensionIndex + 1] = 'n';
		g_currentMissionFile[extensionIndex + 2] = 'v';
		if (fediskio_tryopenfile(g_currentMissionFile, "wb", 0) != 0) {
			File_RawWrite(g_rgb565ToPaletteIndexLut, RTSVGA2_RGB565_LUT_ROW_SIZE,
						  RTSVGA2_RGB565_LUT_SIZE / RTSVGA2_RGB565_LUT_ROW_SIZE, g_stream);
			fediskio_tryclosefile(1);
		}
		g_currentMissionFile[extensionIndex] = originalExtension[0];
		g_currentMissionFile[extensionIndex + 1] = originalExtension[1];
		g_currentMissionFile[extensionIndex + 2] = originalExtension[2];
		fediskio_readfiletofarmemory("newpal.act", &g_flightAuxBuffer->r);
		for (colorIndex = 0; colorIndex < RTSVGA2_PALETTE_COLOR_COUNT / 2; ++colorIndex) {
			uint8_t channel = g_flightAuxBuffer[colorIndex].r >> RTSVGA2_RGB8_TO_RGB6_SHIFT;
			g_flightAuxBuffer[colorIndex].r =
				g_flightAuxBuffer[RTSVGA2_PALETTE_COLOR_COUNT - 1 - colorIndex].r >>
				RTSVGA2_RGB8_TO_RGB6_SHIFT;
			g_flightAuxBuffer[RTSVGA2_PALETTE_COLOR_COUNT - 1 - colorIndex].r = channel;
			channel = g_flightAuxBuffer[colorIndex].g >> RTSVGA2_RGB8_TO_RGB6_SHIFT;
			g_flightAuxBuffer[colorIndex].g =
				g_flightAuxBuffer[RTSVGA2_PALETTE_COLOR_COUNT - 1 - colorIndex].g >>
				RTSVGA2_RGB8_TO_RGB6_SHIFT;
			g_flightAuxBuffer[RTSVGA2_PALETTE_COLOR_COUNT - 1 - colorIndex].g = channel;
			channel = g_flightAuxBuffer[colorIndex].b >> RTSVGA2_RGB8_TO_RGB6_SHIFT;
			g_flightAuxBuffer[colorIndex].b =
				g_flightAuxBuffer[RTSVGA2_PALETTE_COLOR_COUNT - 1 - colorIndex].b >>
				RTSVGA2_RGB8_TO_RGB6_SHIFT;
			g_flightAuxBuffer[RTSVGA2_PALETTE_COLOR_COUNT - 1 - colorIndex].b = channel;
		}
		g_flightSetPaletteRangeFn(g_flightAuxBuffer, 0, RTSVGA2_PALETTE_COLOR_COUNT);
	}
	{
		RgbTriplet targetRgb;
		unsigned int backgroundIndex;
		targetRgb.r = 0;
		targetRgb.g = 0;
		targetRgb.b = FEDISKIO_BACKGROUND_BLUE;
		backgroundIndex =
			rtsvga2_FindNearestRgbTripletIndex(&targetRgb, g_swPalette, 0, RTSVGA2_PALETTE_COLOR_COUNT);
		g_flightBackgroundPaletteIndex = backgroundIndex;
		g_flightColorEscapeBypassChar = backgroundIndex;
		return backgroundIndex;
	}
}

// FUNCTION: XW 0x40AF10
void fediskio_fillinspec(uint8_t craftDefinitionIndex, uint8_t modelType) {
	uint16_t maxExtent = ModelMesh_GetModelMaxExtent(modelType);

	g_modelTypeTable[modelType].maxBoundsExtent = maxExtent;
	g_modelTypeTable[modelType].halfMaxBoundsExtent = maxExtent >> 1;
	if (craftDefinitionIndex != FEDISKIO_CRAFT_DEFINITION_NONE) {
		unsigned int sizeX = ModelMesh_GetModelSizeX(modelType);
		unsigned int sizeY = ModelMesh_GetModelSizeY(modelType);
		unsigned int sizeZ = ModelMesh_GetModelSizeZ(modelType);
		uint16_t boundsShift = 0;

		while (sizeX > FEDISKIO_SCALED_BOUND_LIMIT || sizeY > FEDISKIO_SCALED_BOUND_LIMIT ||
			   sizeZ > FEDISKIO_SCALED_BOUND_LIMIT) {
			sizeX >>= 1;
			sizeY >>= 1;
			sizeZ >>= 1;
			++boundsShift;
		}
		g_craftModelBounds[craftDefinitionIndex].boundSizeShift = boundsShift;
		g_craftModelBounds[craftDefinitionIndex].boundSizeX = sizeX;
		g_craftModelBounds[craftDefinitionIndex].boundSizeY = sizeY;
		g_craftModelBounds[craftDefinitionIndex].boundSizeZ = sizeZ;
	}
}

// FUNCTION: XW 0x40AFC0
int16_t fediskio_tryopenfile(const char* logicalPath, const char* mode, int fatalOnFailure) {
#ifdef XW_MODERN
	int length;
	/* Keep recovered logical prefixes here; storage receives their expanded paths. */
	if (logicalPath[0] == ':')
		length = snprintf(g_fileName, sizeof(g_fileName), "%c:\\XwingCD\\%s", g_installDriveLetter,
						  logicalPath + 1);
	else if (logicalPath[0] == ';' || logicalPath[0] == '+')
		length = snprintf(g_fileName, sizeof(g_fileName), "X-Wing Data\\%s", logicalPath + 1);
	else
		length = snprintf(g_fileName, sizeof(g_fileName), "%s", logicalPath);
	g_stream = length >= 0 && (size_t)length < sizeof(g_fileName) ? XwStorage_Open(g_fileName, mode) : NULL;
	if (g_stream)
		return 1;
	if (fatalOnFailure)
		fediskio_fatalerror(FEDISKIO_ERROR_FILE_ACCESS);
	return 0;
#else
	int tryFallback;
	if (logicalPath[0] == ':') {
		sprintf(g_fileName, "%c:\\XwingCD\\%s", g_installDriveLetter, logicalPath + 1);
		tryFallback = 0;
	} else if (logicalPath[0] == ';' || logicalPath[0] == '+') {
		strcpy(g_fileName, "X-Wing Data\\");
		strcat(g_fileName, logicalPath + 1);
		tryFallback = 1;
	} else {
		strcpy(g_fileName, logicalPath);
		tryFallback = fatalOnFailure;
	}
	g_stream = File_RawOpen(g_fileName, mode);
	if (g_stream != NULL)
		return 1;
	if (tryFallback != 0) {
		if (logicalPath[0] == ';')
			sprintf(g_fileName, "%c:\\XwingCD\\%s", g_installDriveLetter, logicalPath + 1);
		else
			strcpy(g_fileName, logicalPath + 1);
		g_stream = File_RawOpen(g_fileName, mode);
		if (g_stream != NULL)
			return 1;
	}
	if ((int16_t)fatalOnFailure != 0)
		fediskio_fatalerror(FEDISKIO_ERROR_FILE_ACCESS);
	else
		return 0;
#endif
}

// FUNCTION: XW 0x40B130
int16_t fediskio_tryclosefile(int16_t deleteOnError) {
#ifdef XW_MODERN
	int16_t closeFailed = (int16_t)XwStorage_CloseGlobalStream(g_stream, deleteOnError);
	g_stream = NULL;
	return closeFailed;
#else
	int16_t closeFailed = 0;
	if (File_RawHasError(g_stream) || File_RawClose(g_stream) == EOF) {
		closeFailed = 1;
	}
	if (deleteOnError != 0 && closeFailed != 0) {
		File_RawRemove(g_fileName);
	}
	g_stream = NULL;
	return closeFailed;
#endif
}

// FUNCTION: XW 0x40B180
size_t fediskio_readfileblock(void* buffer, size_t elementSize, size_t elementCount, XwFile* stream) {
	size_t result = File_RawRead(buffer, elementSize, elementCount, stream);
	if (result != elementCount) {
		g_fileBlockError = 1;
		return 0;
	}
	g_fileBlockError = 0;
	return result;
}

// FUNCTION: XW 0x40B1C0
size_t fediskio_writefileblock(const void* buffer, size_t elementSize, size_t elementCount, XwFile* stream) {
	size_t result = File_RawWrite(buffer, elementSize, elementCount, stream);
	if (result != elementCount) {
		g_fileBlockError = 1;
		return 0;
	}
	g_fileBlockError = 0;
	return result;
}

// FUNCTION: XW 0x40B200
int fediskio_ReadFileBlockBuffered(void* buffer, int elementSize, int elementCount, XwFile* stream) {
#ifdef XW_MODERN
	uint8_t chunkBuffer[FEDISKIO_BLOCK_BUFFER_CAPACITY] = { 0 };
#else
	uint8_t chunkBuffer[FEDISKIO_BLOCK_BUFFER_CAPACITY];
#endif
	unsigned int remainingBytes = (unsigned int)elementSize * (unsigned int)elementCount;
	int16_t hadReadError = 0;
	uint16_t destinationOffset = 0;
	while ((uint16_t)remainingBytes != 0) {
		unsigned int chunkSize = remainingBytes;
		uint16_t index;
		if ((uint16_t)chunkSize > sizeof(chunkBuffer)) {
			chunkSize = sizeof(chunkBuffer);
		}
		hadReadError |= (uint16_t)File_RawRead(chunkBuffer, sizeof(chunkBuffer[0]), (uint16_t)chunkSize,
											   stream) != (uint16_t)chunkSize;
		for (index = 0; index < (uint16_t)chunkSize; ++index, ++destinationOffset) {
			((uint8_t*)buffer)[destinationOffset] = chunkBuffer[index];
		}
		remainingBytes -= chunkSize;
	}
	g_fileBlockError = hadReadError;
	return hadReadError == 0;
}

// FUNCTION: XW 0x40B2B0
int fediskio_WriteFileBlockBuffered(const void* buffer, unsigned int elementSize, unsigned int elementCount,
									XwFile* stream) {
	uint8_t chunkBuffer[FEDISKIO_BLOCK_BUFFER_CAPACITY];
	uint16_t remainingBytes = elementSize * elementCount;
	int16_t hadWriteError = 0;
	uint16_t sourceOffset = 0;
	while (remainingBytes != 0) {
		uint16_t chunkSize = remainingBytes;
		uint16_t index;
		if (chunkSize > sizeof(chunkBuffer)) {
			chunkSize = sizeof(chunkBuffer);
		}
		for (index = 0; index < chunkSize; ++index, ++sourceOffset) {
			chunkBuffer[index] = ((const uint8_t*)buffer)[sourceOffset];
		}
		hadWriteError |=
			(uint16_t)File_RawWrite(chunkBuffer, sizeof(chunkBuffer[0]), chunkSize, stream) != chunkSize;
		remainingBytes -= chunkSize;
	}
	g_fileBlockError = hadWriteError;
	return hadWriteError == 0;
}

// FUNCTION: XW 0x40B360
void XW_NORETURN fediskio_fatalerror(uint16_t errorIndex) {
#ifdef XW_MODERN
	XwStorage_Fatal(g_diskFatalErrorMessages[errorIndex], XW_SHELL_FATAL_EXIT_CODE);
#else
	char message[FEDISKIO_FATAL_MESSAGE_CAPACITY];
	uint16_t messageLength;
	const char* errorText = g_diskFatalErrorMessages[errorIndex];
	for (messageLength = 0; messageLength < sizeof(message); ++messageLength) {
		char character = errorText[messageLength];
		message[messageLength] = character;
		if (character == '\0') {
			break;
		}
	}
	if (errorIndex == FEDISKIO_ERROR_FILE_ACCESS) {
		uint16_t filenameIndex;
		for (filenameIndex = 0; messageLength < sizeof(message); ++messageLength) {
			char character = g_fileName[filenameIndex++];
			message[messageLength] = character;
			if (character == '\0') {
				break;
			}
		}
		message[messageLength++] = '\n';
		message[messageLength] = '\0';
	}
	ShellFlight_FatalExit(message);
#endif
}
