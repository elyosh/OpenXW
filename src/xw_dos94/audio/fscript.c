#include "xw_dos94/audio/fscript.h"
#include "xw_dos94/audio/fmusic.h"
#include "xw_dos94/audio/fscript_data.h"
#include "xw_dos94/audio/hilevel.h"
#include "xw_runtime/audio/imuse_host.h"
#include "xw_runtime/audio/imuse_session.h"
#include <stdbool.h>

// FUNCTION: DOS94 0x696FCC
void Dos94_fscript_MsStartScript(void) {
	for (intptr_t id = 500; id < 625; ++id)
		imuse_forget_sound(g_dos94Imuse, id);
	XwMusicSdp* banks[] = { g_dos94MusicStateSdpBank0, g_dos94MusicStateSdpBank1, g_dos94MusicStateSdpBank2,
							g_dos94MusicStateSdpBank3, g_dos94MusicStateSdpBank4, g_dos94MusicStateSdpBank5,
							g_dos94MusicStateSdpBank6, g_dos94MusicStateSdpBank7, g_dos94MusicStateSdpBank8 };
	const size_t counts[] = { 32, 43, 28, 20, 16, 21, 14, 34, 27 };
	for (int i = 0; i < 9; ++i) {
		g_musicStateSdpBanks[i] = banks[i];
		for (size_t j = 0; j < counts[i]; ++j)
			banks[i][j].availableMask = 0;
	}
	for (int i = 0; i < FSCRIPT_SEQUENCE_RECORD_COUNT; ++i)
		g_musicSequenceData[i].availableMask = 0;
	/* Stable music-only seeds replace the original address-derived seeds. */
	g_musicScriptRandomStateA = 0x12345678;
	g_musicScriptRandomStateB = 0xedcba987;
	g_musicCurrentSoundId = g_musicNextSoundId = g_musicSequenceSoundId = g_musicPreviousSoundId = 0;
	g_musicCurrentState = 9;
	g_musicCurrentSequence = 9;
	g_musicSequencePriority = 0;
	g_musicScriptChoiceError = 0;
	g_musicScriptIntensity = 0;
}

// FUNCTION: DOS94 0x697158
void Dos94_fscript_MsRefreshScript(void) {
	if (g_musicCurrentState >= FSCRIPT_INACTIVE_STATE)
		return;
	imuse_t* im = g_dos94Imuse;
	bool music_playing = false;
	for (intptr_t id = imuse_next_sound(im, 0); id != 0; id = imuse_next_sound(im, id)) {
		if (imuse_get_param(im, id, IMUSE_PARAM_SOUND_GROUP) == IMUSE_GROUP_MUSIC) {
			music_playing = true;
			break;
		}
	}
	if (!music_playing) {
		for (intptr_t id = 500; id < 625; ++id)
			imuse_clear_trigger(g_dos94Imuse, id, -1, -1);
		g_musicCurrentSoundId = g_musicNextSoundId = g_musicSequenceSoundId = g_musicPreviousSoundId = 0;
		g_musicCurrentState = FSCRIPT_INACTIVE_STATE;
		g_musicCurrentSequence = FSCRIPT_NO_SEQUENCE;
		g_musicSequencePriority = 0;
		g_musicScriptChoiceError = 0;
		g_musicScriptIntensity = 0;
		/* Keep the catalog active; the next mood update starts the script again. */
		return;
	}
	g_musicPreviousSoundId = 0;
	if (!g_musicNextSoundId) {
		g_musicCurrentSdp = Dos94_fscript_SelectSdp(g_musicCurrentSdp);
		g_musicNextSoundId = Dos94_fmusic_fmLoadSound(g_musicCurrentSdp->soundName);
		if (!g_musicNextSoundId) {
			for (intptr_t id = 500; id < 625; ++id)
				imuse_forget_sound(g_dos94Imuse, id);
			g_musicCurrentState = FSCRIPT_INACTIVE_STATE;
		}
	}
}

// FUNCTION: DOS94 0x697288
void Dos94_fscript_DispatchMusicEvent(int eventId) {
	int index;

	if (!g_musicEventDispatchGate)
		return;
	for (index = 0; index < FSCRIPT_STATE_COUNT; ++index) {
		if (eventId == g_musicStateEventMappings[index].eventId) {
			int newState;
			newState = g_musicStateEventMappings[index].scriptIndex;
			if (newState != g_musicCurrentState)
				Dos94_fscript_ChangeState(newState);
			return;
		}
	}
	for (index = 0; index < FSCRIPT_SEQUENCE_COUNT; ++index) {
		if (eventId == g_musicSequenceEventMappings[index].eventId) {
			Dos94_fscript_PlaySequence(g_musicSequenceEventMappings[index].scriptIndex);
			return;
		}
	}
}

// FUNCTION: DOS94 0x6972F0
void Dos94_fscript_SetAttributeValue(int attribute, int value) {
	if (!g_dos94Imuse || attribute != 0 || value >= 8)
		return;
	g_musicScriptIntensity = value;
	imuse_pause(g_dos94Imuse);
	if (imuse_get_param(g_dos94Imuse, g_musicCurrentSoundId, IMUSE_PARAM_SOUND_PLAY_COUNT))
		imuse_xwing_set_hook(g_dos94Imuse, g_musicCurrentSoundId, 2, 128 + value, 16);
	if (imuse_get_param(g_dos94Imuse, g_musicNextSoundId, IMUSE_PARAM_SOUND_PLAY_COUNT))
		imuse_xwing_set_hook(g_dos94Imuse, g_musicNextSoundId, 2, 128 + value, 16);
	imuse_resume(g_dos94Imuse);
}

// FUNCTION: DOS94 0x69741A
void Dos94_fscript_ChangeState(int state) {
	if (!g_musicEventDispatchGate || state < 0 || state >= 9)
		return;
	imuse_t* im = g_dos94Imuse;
	if (g_musicCurrentState >= 9) {
		g_musicCurrentState = state;
		g_musicCurrentSdp = g_musicStateSdpBanks[state];
		g_musicCurrentSoundId = Dos94_fmusic_fmLoadSound(g_musicCurrentSdp->soundName);
		g_musicCurrentSdp = Dos94_fscript_SelectSdp(g_musicCurrentSdp);
		g_musicNextSoundId = Dos94_fmusic_fmLoadSound(g_musicCurrentSdp->soundName);
		if (Dos94_hilevel_ImStartMusic(g_musicCurrentSoundId) != 0) {
			g_musicCurrentState = 9;
			return;
		}
		imuse_xwing_set_hook(im, g_musicCurrentSoundId, 2, 128 + g_musicScriptIntensity, 16);
		for (intptr_t id = 500; id < 625; ++id)
			imuse_clear_trigger(g_dos94Imuse, id, -1, -1);
		ImuseCmd command = { .opcode = (intptr_t)XwImuseHost_MusicMarker };
		imuse_set_trigger(g_dos94Imuse, g_musicCurrentSoundId, 1, &command);
		return;
	}
	int old = g_musicCurrentState;
	g_musicCurrentState = state;
	XwMusicSdp* s = g_musicStateSdpBanks[state];
	while (s->name[0])
		++s;
	s = Dos94_fscript_SelectSdp(s + (g_musicSequenceSoundId ? state : old));
	XwMusicSoundId id = Dos94_fmusic_fmLoadSound(s->soundName);
	if (id == g_musicCurrentSoundId) {
		s = Dos94_fscript_SelectSdp(s);
		id = Dos94_fmusic_fmLoadSound(s->soundName);
	}
	if (!id) {
		g_musicCurrentState = 9;
		return;
	}
	if (id != g_musicNextSoundId) {
		g_musicNextSoundId = id;
		g_musicCurrentSdp = s;
		imuse_set_hook(im, g_musicCurrentSoundId, 1);
	}
}

// FUNCTION: DOS94 0x697664
void Dos94_fscript_PlaySequence(int sequence) {
	if (!g_musicEventDispatchGate || sequence < 0 || sequence >= 9)
		return;
	if (g_musicSequenceSoundId) {
		if (g_musicSequencePriorities[sequence] <= g_musicSequencePriority)
			return;
		/* DOS cancels the old request even if its replacement is already playing. */
		g_musicSequenceSoundId = 0;
		g_musicCurrentSequence = FSCRIPT_NO_SEQUENCE;
	}
	XwMusicSoundId id = Dos94_fmusic_fmLoadSound(Dos94_fscript_SelectSequence(sequence));
	if (!id) {
		for (intptr_t id = 500; id < 625; ++id)
			imuse_forget_sound(g_dos94Imuse, id);
		g_musicCurrentState = FSCRIPT_INACTIVE_STATE;
		return;
	}
	if (id == g_musicCurrentSoundId || id == g_musicPreviousSoundId)
		return;
	g_musicSequenceSoundId = id;
	g_musicCurrentSequence = sequence;
	g_musicSequencePriority = g_musicSequencePriorities[sequence];
	imuse_set_hook(g_dos94Imuse, g_musicCurrentSoundId, 1);
	Dos94_fscript_ChangeState(g_musicCurrentState);
}

// FUNCTION: DOS94 0x697766
struct XwMusicSdp* Dos94_fscript_SelectSdp(struct XwMusicSdp* sdp) {
	if (sdp->destinationCount == 0) {
		return sdp;
	} else {
		int destinationIndex = Dos94_fscript_ChooseDest(sdp);
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
}

// FUNCTION: DOS94 0x697814
const char* Dos94_fscript_SelectSequence(int sequenceIndex) {
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
		int destinationIndex = Dos94_fscript_ChooseDest(selection);
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
}

// FUNCTION: DOS94 0x6978FC
int Dos94_fscript_ChooseDest(struct XwMusicSdp* sdp) {
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
	remainingOrdinal = (int16_t)Dos94_fscript_GetRandom(0, availableCount - 1);
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
}

// FUNCTION: DOS94 0x697A34
int Dos94_fscript_GetRandom(int16_t minimum, int16_t maximum) {
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
}
