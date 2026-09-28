#ifndef XW_FLIGHT_MISSION_FSCRIPT_H
#define XW_FLIGHT_MISSION_FSCRIPT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

enum { FSCRIPT_DESTINATION_MASK_COUNT = 8 };

enum {
	FSCRIPT_STATE_COUNT = 9,
	FSCRIPT_INACTIVE_STATE = 9,
	FSCRIPT_SEQUENCE_COUNT = 9,
	FSCRIPT_NO_SEQUENCE = 9
};

enum { FSCRIPT_SEQUENCE_RECORD_COUNT = 21 };

enum {
	FSCRIPT_MUSIC_PRIORITY = 128,
	FSCRIPT_INTENSITY_HOOK_BIAS = 0x80,
	FSCRIPT_INTENSITY_HOOK_MASK = 0x1100,
	FSCRIPT_TRANSITION_HOOK = 1,
	FSCRIPT_TRANSITION_MARKER = 1,
	FSCRIPT_TRANSITION_CALLBACK = 0
};

enum {
	FSCRIPT_RANDOM_STATE_A_STEPS = 23,
	FSCRIPT_RANDOM_STATE_B_STEPS = 37,
	FSCRIPT_RANDOM_FEEDBACK_MASK = 0x20000000,
	FSCRIPT_RANDOM_FEEDBACK_SHIFT = 29,
	FSCRIPT_RANDOM_RANGE_SHIFT = 16
};

typedef struct XwMusicEventMapping XwMusicEventMapping;
typedef struct XwMusicSdp XwMusicSdp;

/* Original IDB size: 8 bytes. */
struct XwMusicEventMapping {
	/* IDB +0x0: Legacy event ID compared with the dispatcher input. */
	int eventId;
	/* IDB +0x4: Index 0..8 passed to ChangeState or PlaySequence according to the containing table. */
	int scriptIndex;
};

/* Original IDB size: 32 bytes. */
struct XwMusicSdp {
	/* IDB +0x0: Four-character record key plus NUL. Empty key terminates the named-record list; unnamed
	 * transition/sequence records follow it. */
	char name[5];
	/* IDB +0x5: Four-character music sound/resource name plus NUL. Returned to the script sound loader. */
	char soundName[5];
	/* IDB +0xA: Number of valid entries in destinations (four slots in this 32-byte format). */
	uint8_t destinationCount;
	/* IDB +0xB: Mutable mask of remaining destinations. ChooseDest clears the selected bit and refills
	 * excluding the last choice when exhausted. */
	uint8_t availableMask;
	/* IDB +0xC: Destination record keys, each in a five-byte NUL-terminated slot. */
	char destinations[4][5];
};

typedef unsigned int XwMusicSoundId;

typedef struct XwMusicScriptServices {
	void* field_00[5];
	XwMusicSoundId (*loadSound)(const char* soundName);
} XwMusicScriptServices;

extern XwMusicEventMapping g_musicStateEventMappings[FSCRIPT_STATE_COUNT];
extern XwMusicEventMapping g_musicSequenceEventMappings[FSCRIPT_SEQUENCE_COUNT];
extern int g_musicEventDispatchGate;
extern const int g_musicSequencePriorities[FSCRIPT_SEQUENCE_COUNT];
extern int g_musicSequencePriority;
extern int g_musicCurrentSequence;
extern XwMusicSoundId g_musicPreviousSoundId;
extern XwMusicScriptServices* g_musicScriptServices;
extern XwMusicSdp* g_musicCurrentSdp;
extern XwMusicSoundId g_musicNextSoundId;
extern XwMusicSoundId g_musicSequenceSoundId;
extern XwMusicSoundId g_musicCurrentSoundId;

extern XwMusicSdp g_musicSequenceData[FSCRIPT_SEQUENCE_RECORD_COUNT];
extern int g_musicCurrentState;
extern int g_musicScriptChoiceError;
extern uint32_t g_musicScriptRandomStateA;
extern uint32_t g_musicScriptRandomStateB;
extern XwMusicSdp* g_musicStateSdpBanks[FSCRIPT_STATE_COUNT];
extern int g_musicScriptIntensity;

/* Declarations follow ascending original IDB address. */

/* 0x40BDE0 */
void fscript_DispatchMusicEvent(int eventId);

/* 0x40BE50 */
void fscript_ChangeState(int newState);

/* 0x40C070 */
void fscript_PlaySequence(int sequenceIndex);

/* 0x40C160 */
struct XwMusicSdp* fscript_SelectSdp(struct XwMusicSdp* sdp);

/* 0x40C1D0 */
const char* fscript_SelectSequence(int sequenceIndex);

/* 0x40C270 */
int fscript_ChooseDest(struct XwMusicSdp* sdp);

/* 0x40C340 */
int fscript_GetRandom(int16_t minimum, int16_t maximum);

#ifdef __cplusplus
}
#endif

#endif
