#ifndef XW_DOS94_AUDIO_FSCRIPT_H
#define XW_DOS94_AUDIO_FSCRIPT_H
#include "xw/flight/mission/fscript.h"
/* 0x696FCC */
void Dos94_fscript_MsStartScript(void);
/* 0x697158 */
void Dos94_fscript_MsRefreshScript(void);
/* 0x697288 */
void Dos94_fscript_DispatchMusicEvent(int eventId);
/* 0x6972F0 */
void Dos94_fscript_SetAttributeValue(int attribute, int value);
/* 0x69741A */
void Dos94_fscript_ChangeState(int state);
/* 0x697664 */
void Dos94_fscript_PlaySequence(int sequence);
/* 0x697766 */
struct XwMusicSdp* Dos94_fscript_SelectSdp(struct XwMusicSdp* sdp);
/* 0x697814 */
const char* Dos94_fscript_SelectSequence(int sequenceIndex);
/* 0x6978FC */
int Dos94_fscript_ChooseDest(struct XwMusicSdp* sdp);
/* 0x697A34 */
int Dos94_fscript_GetRandom(int16_t minimum, int16_t maximum);
#endif
