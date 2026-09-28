#include "xw/flight/mission/fscript.h"
#ifdef XW_MODERN
#include "xw_dos94/audio/fscript.h"
#endif

#ifdef XW_MODERN
#include "xw_runtime/audio/flight_music.h"
#endif

#include "xw/audio/lolevel.h"
#include "xw/util/folded.h"
#include "xw/util/shared.h"

#include <stdlib.h>

// GLOBAL: XW 0x4C56A8
XwMusicEventMapping g_musicStateEventMappings[FSCRIPT_STATE_COUNT] = { { 0, 0 }, { 1, 1 }, { 2, 2 },
																	   { 3, 3 }, { 4, 4 }, { 5, 5 },
																	   { 6, 6 }, { 7, 7 }, { 8, 8 } };

// GLOBAL: XW 0x4C56F0
XwMusicEventMapping g_musicSequenceEventMappings[FSCRIPT_SEQUENCE_COUNT] = {
	{ 15, 0 }, { 16, 1 }, { 11, 2 }, { 12, 3 }, { 13, 4 }, { 14, 5 }, { 9, 6 }, { 10, 7 }, { 17, 8 }
};

// GLOBAL: XW 0x4C7498
XwMusicSdp g_musicSequenceData[FSCRIPT_SEQUENCE_RECORD_COUNT] = {
	{ "EM01", "EM01", 0, 0, { "", "", "", "" } },
	{ "EM11", "EM11", 0, 0, { "", "", "", "" } },
	{ "EM21", "EM21", 0, 0, { "", "", "", "" } },
	{ "EM31", "EM31", 0, 0, { "", "", "", "" } },
	{ "RB01", "RB01", 0, 0, { "", "", "", "" } },
	{ "RB11", "RB11", 0, 0, { "", "", "", "" } },
	{ "RB21", "RB21", 0, 0, { "", "", "", "" } },
	{ "RB22", "RB22", 0, 0, { "", "", "", "" } },
	{ "RB23", "RB23", 0, 0, { "", "", "", "" } },
	{ "RB24", "RB24", 0, 0, { "", "", "", "" } },
	{ "RB31", "RB31", 0, 0, { "", "", "", "" } },
	{ "TR41", "TR41", 0, 0, { "", "", "", "" } },
	{ "", "", 1, 0, { "EM01", "", "", "" } },
	{ "", "", 1, 0, { "EM11", "", "", "" } },
	{ "", "", 1, 0, { "EM21", "", "", "" } },
	{ "", "", 1, 0, { "EM31", "", "", "" } },
	{ "", "", 1, 0, { "RB01", "", "", "" } },
	{ "", "", 1, 0, { "RB11", "", "", "" } },
	{ "", "", 4, 0, { "RB21", "RB22", "RB23", "RB24" } },
	{ "", "", 1, 0, { "RB31", "", "", "" } },
	{ "", "", 1, 0, { "TR41", "", "", "" } }
};

// GLOBAL: XW 0x4C7738
const int g_musicSequencePriorities[FSCRIPT_SEQUENCE_COUNT] = { 3, 5, 1, 7, 4, 6, 2, 8, 1 };

// GLOBAL: XW 0x4F48D4
XwMusicScriptServices* g_musicScriptServices = NULL;

// GLOBAL: XW 0x5BECB4
int g_musicEventDispatchGate = 0;

// GLOBAL: XW 0x63BA40
XwMusicSdp* g_musicCurrentSdp = NULL;

// GLOBAL: XW 0x63BA44
int g_musicCurrentState = 0;

// GLOBAL: XW 0x63BA48
int g_musicScriptChoiceError = 0;

// GLOBAL: XW 0x63BA4C
uint32_t g_musicScriptRandomStateA = 0;

// GLOBAL: XW 0x63BA50
uint32_t g_musicScriptRandomStateB = 0;

// GLOBAL: XW 0x63BA54
XwMusicSoundId g_musicPreviousSoundId = 0;

// GLOBAL: XW 0x63BA58
XwMusicSoundId g_musicNextSoundId = 0;

// GLOBAL: XW 0x63BA5C
int g_musicSequencePriority = 0;

// GLOBAL: XW 0x63BA60
int g_musicCurrentSequence = 0;

// GLOBAL: XW 0x63BA64
XwMusicSoundId g_musicSequenceSoundId = 0;

// GLOBAL: XW 0x63BA68
XwMusicSoundId g_musicCurrentSoundId = 0;

// GLOBAL: XW 0x63BA80
XwMusicSdp* g_musicStateSdpBanks[FSCRIPT_STATE_COUNT] = { NULL };

// GLOBAL: XW 0x63BAA4
int g_musicScriptIntensity;

// FUNCTION: XW 0x40BDE0
void fscript_DispatchMusicEvent(int eventId) {
#ifdef XW_MODERN
	Dos94_fscript_DispatchMusicEvent(eventId);
#else
	int index;
	if (!g_musicEventDispatchGate)
		return;
	for (index = 0; index < FSCRIPT_STATE_COUNT; ++index) {
		if (eventId == g_musicStateEventMappings[index].eventId) {
			int newState;
			newState = g_musicStateEventMappings[index].scriptIndex;
			if (newState != g_musicCurrentState)
				fscript_ChangeState(newState);
			return;
		}
	}
	for (index = 0; index < FSCRIPT_SEQUENCE_COUNT; ++index) {
		if (eventId == g_musicSequenceEventMappings[index].eventId) {
			fscript_PlaySequence(g_musicSequenceEventMappings[index].scriptIndex);
			return;
		}
	}
#endif
}

// FUNCTION: XW 0x40BE50
void fscript_ChangeState(int newState) {
#ifdef XW_MODERN
	Dos94_fscript_ChangeState(newState);
#else
	if (g_musicCurrentState >= FSCRIPT_INACTIVE_STATE) {
		g_musicCurrentState = newState;
		g_musicCurrentSdp = g_musicStateSdpBanks[newState];
		lolevel_ImPause();
		g_musicCurrentSoundId = g_musicScriptServices->loadSound(g_musicCurrentSdp->soundName);
		lolevel_ImResume();
		if (!g_musicCurrentSoundId) {
			g_musicCurrentState = FSCRIPT_INACTIVE_STATE;
			gamesnd_Report_Sound_Load_Failure(g_musicCurrentSdp->soundName);
			return;
		}
		g_musicCurrentSdp = fscript_SelectSdp(g_musicCurrentSdp);
		lolevel_ImPause();
		g_musicNextSoundId = g_musicScriptServices->loadSound(g_musicCurrentSdp->soundName);
		lolevel_ImResume();
		if (!g_musicNextSoundId) {
			g_musicCurrentState = FSCRIPT_INACTIVE_STATE;
			gamesnd_Report_Sound_Load_Failure(g_musicCurrentSdp->soundName);
			return;
		}
		lolevel_ImPause();
		hilevel_ImStartMusic(g_musicCurrentSoundId, FSCRIPT_MUSIC_PRIORITY);
		lolevel_ImSetHook(g_musicCurrentSoundId,
						  ((uint32_t)g_musicScriptIntensity + FSCRIPT_INTENSITY_HOOK_BIAS) |
							  FSCRIPT_INTENSITY_HOOK_MASK);
		lolevel_ImClearTrigger((uint32_t)-1, -1, -1);
		lolevel_ImSetTrigger(g_musicCurrentSoundId, FSCRIPT_TRANSITION_MARKER, FSCRIPT_TRANSITION_CALLBACK);
		lolevel_ImResume();
	} else {
		int previousState;
		int recordIndex;
		XwMusicSdp* bank;
		XwMusicSdp* nextSdp;
		XwMusicSoundId nextSound;
		previousState = g_musicCurrentState;
		g_musicCurrentState = newState;
		bank = g_musicStateSdpBanks[newState];
		for (recordIndex = 0;; ++recordIndex) {
			nextSdp = &bank[recordIndex];
			if (!nextSdp->name[0])
				break;
		}
		if (g_musicSequenceSoundId)
			nextSdp = &nextSdp[newState];
		else
			nextSdp = &nextSdp[previousState];
		nextSdp = fscript_SelectSdp(nextSdp);
		lolevel_ImPause();
		nextSound = g_musicScriptServices->loadSound(nextSdp->soundName);
		lolevel_ImResume();
		if (nextSound == g_musicCurrentSoundId) {
			nextSdp = fscript_SelectSdp(nextSdp);
			lolevel_ImPause();
			nextSound = g_musicScriptServices->loadSound(nextSdp->soundName);
			lolevel_ImResume();
		}
		if (!nextSound) {
			lolevel_ImStopAllSounds();
			g_musicCurrentState = FSCRIPT_INACTIVE_STATE;
			gamesnd_Report_Sound_Load_Failure(nextSdp->soundName);
		} else if (nextSound != g_musicNextSoundId) {
			lolevel_ImPause();
			g_musicNextSoundId = nextSound;
			g_musicCurrentSdp = nextSdp;
			lolevel_ImSetHook(g_musicCurrentSoundId, FSCRIPT_TRANSITION_HOOK);
			lolevel_ImResume();
		}
	}
#endif
}

// FUNCTION: XW 0x40C070
void fscript_PlaySequence(int sequenceIndex) {
#ifdef XW_MODERN
	Dos94_fscript_PlaySequence(sequenceIndex);
#else
	const char* soundName;
	XwMusicSoundId sequenceSound;
	lolevel_ImPause();
	if (g_musicSequenceSoundId) {
		if (g_musicSequencePriorities[sequenceIndex] <= g_musicSequencePriority) {
			lolevel_ImResume();
			return;
		}
		g_musicSequenceSoundId = 0;
		g_musicCurrentSequence = FSCRIPT_NO_SEQUENCE;
	}
	lolevel_ImResume();
	soundName = fscript_SelectSequence(sequenceIndex);
	lolevel_ImPause();
	sequenceSound = g_musicScriptServices->loadSound(soundName);
	lolevel_ImResume();
	if (!sequenceSound) {
		lolevel_ImStopAllSounds();
		g_musicCurrentState = FSCRIPT_INACTIVE_STATE;
		gamesnd_Report_Sound_Load_Failure(soundName);
		return;
	}
	if (sequenceSound != g_musicCurrentSoundId && sequenceSound != g_musicPreviousSoundId) {
		g_musicSequenceSoundId = sequenceSound;
		g_musicCurrentSequence = sequenceIndex;
		g_musicSequencePriority = g_musicSequencePriorities[sequenceIndex];
		lolevel_ImPause();
		lolevel_ImSetHook(g_musicCurrentSoundId, FSCRIPT_TRANSITION_HOOK);
		lolevel_ImResume();
		fscript_ChangeState(g_musicCurrentState);
	}
#endif
}

// FUNCTION: XW 0x40C160
struct XwMusicSdp* fscript_SelectSdp(struct XwMusicSdp* sdp) {
#ifdef XW_MODERN
	return Dos94_fscript_SelectSdp(sdp);
#else
	if (sdp->destinationCount == 0) {
		return sdp;
	} else {
		int destinationIndex = fscript_ChooseDest(sdp);
		const char* destinationName = sdp->destinations[destinationIndex];
		XwMusicSdp* bank = g_musicStateSdpBanks[g_musicCurrentState];
		int recordIndex;
		for (recordIndex = 0;; ++recordIndex) {
			XwMusicSdp* candidate = &bank[recordIndex];
			int nameIndex;
			if (candidate->name[0] == 0) {
				return candidate;
			}
			for (nameIndex = 0; candidate->name[nameIndex] != 0; ++nameIndex) {
				if (candidate->name[nameIndex] != destinationName[nameIndex]) {
					break;
				}
			}
			if (candidate->name[nameIndex] == 0 && destinationName[nameIndex] == 0) {
				return candidate;
			}
		}
	}
#endif
}

// FUNCTION: XW 0x40C1D0
const char* fscript_SelectSequence(int sequenceIndex) {
#ifdef XW_MODERN
	return Dos94_fscript_SelectSequence(sequenceIndex);
#else
	int recordIndex;
	XwMusicSdp* selection;
	for (recordIndex = 0;; ++recordIndex) {
		selection = &g_musicSequenceData[recordIndex];
		if (selection->name[0] == 0) {
			break;
		}
	}
	selection = &selection[sequenceIndex];
	if (selection->destinationCount == 0) {
		return NULL;
	} else {
		int destinationIndex = fscript_ChooseDest(selection);
		const char* destinationName = selection->destinations[destinationIndex];
		for (recordIndex = 0;; ++recordIndex) {
			XwMusicSdp* candidate = &g_musicSequenceData[recordIndex];
			int nameIndex;
			if (candidate->name[0] == 0) {
				return NULL;
			}
			for (nameIndex = 0; candidate->name[nameIndex] != 0; ++nameIndex) {
				if (candidate->name[nameIndex] != destinationName[nameIndex]) {
					break;
				}
			}
			if (candidate->name[nameIndex] == 0 && destinationName[nameIndex] == 0) {
				return candidate->soundName;
			}
		}
	}
#endif
}

// FUNCTION: XW 0x40C270
int fscript_ChooseDest(struct XwMusicSdp* sdp) {
#ifdef XW_MODERN
	return Dos94_fscript_ChooseDest(sdp);
#else
	const uint8_t fullMasks[FSCRIPT_DESTINATION_MASK_COUNT] = { 0, 1, 3, 7, 15, 31, 63, 127 };
	int availableCount;
	int destinationIndex;
	int remainingOrdinal;
	uint8_t bit;
	if (sdp->availableMask == 0) {
		sdp->availableMask = fullMasks[sdp->destinationCount];
	}
	availableCount = 0;
	bit = 1;
	for (destinationIndex = sdp->destinationCount; destinationIndex > 0; --destinationIndex) {
		if ((bit & sdp->availableMask) != 0) {
			++availableCount;
		}
		bit <<= 1;
	}
	bit = 1;
	remainingOrdinal = (int16_t)fscript_GetRandom(0, availableCount - 1);
	for (destinationIndex = 0; destinationIndex < sdp->destinationCount; ++destinationIndex) {
		if ((bit & sdp->availableMask) != 0) {
			if (remainingOrdinal == 0) {
				bit = ~bit;
				sdp->availableMask &= bit;
				if (sdp->availableMask == 0) {
					sdp->availableMask = bit & fullMasks[sdp->destinationCount];
				}
				break;
			}
			--remainingOrdinal;
		}
		bit <<= 1;
	}
	if (remainingOrdinal != 0) {
		if (g_musicScriptChoiceError == 0) {
			g_musicScriptChoiceError = 1;
		}
		return 0;
	}
	return destinationIndex;
#endif
}

/* Remaining matching differences are register allocation for the feedback masks, counters, and steps. */
// FUNCTION: XW 0x40C340
int fscript_GetRandom(int16_t minimum, int16_t maximum) {
#ifdef XW_MODERN
	return Dos94_fscript_GetRandom(minimum, maximum);
#else
	int step;
	for (step = FSCRIPT_RANDOM_STATE_A_STEPS; step != 0; --step) {
		g_musicScriptRandomStateA = ((((~g_musicScriptRandomStateA >> 1) & FSCRIPT_RANDOM_FEEDBACK_MASK) ^
									  (g_musicScriptRandomStateB & FSCRIPT_RANDOM_FEEDBACK_MASK)) >>
									 FSCRIPT_RANDOM_FEEDBACK_SHIFT) +
									2 * g_musicScriptRandomStateA;
	}
	for (step = FSCRIPT_RANDOM_STATE_B_STEPS; step != 0; --step) {
		g_musicScriptRandomStateB = ((((~g_musicScriptRandomStateB >> 1) & FSCRIPT_RANDOM_FEEDBACK_MASK) ^
									  (g_musicScriptRandomStateA & FSCRIPT_RANDOM_FEEDBACK_MASK)) >>
									 FSCRIPT_RANDOM_FEEDBACK_SHIFT) +
									2 * g_musicScriptRandomStateB;
	}
	return minimum + (((uint32_t)(uint16_t)(g_musicScriptRandomStateA + g_musicScriptRandomStateB) *
					   (maximum - minimum + 1)) >>
					  FSCRIPT_RANDOM_RANGE_SHIFT);
#endif
}
