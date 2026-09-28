#ifndef XW_DOS94_AUDIO_SOUNDEXT_H
#define XW_DOS94_AUDIO_SOUNDEXT_H
#include "xw/audio/soundext.h"
/* Frontend opaque DOS sound identities are native Sound pointers. */
/* 0x1228C */
int16_t Dos94_soundext_Start_Resource_Sound(const Sound* sound);
/* 0x122B0 */
int16_t Dos94_soundext_Start_Resource_SFX(const Sound* sound);
/* 0x122C2 */
int16_t Dos94_soundext_Start_Resource_Voice(const Sound* sound);
/* 0x122D4 */
int16_t Dos94_soundext_Stop_Resource_Sound(const Sound* sound);
/* 0x122EC */
int16_t Dos94_soundext_Count_Resource_Instances(const Sound* sound);
/* 0x1232E */
int16_t Dos94_soundext_GetMusicParam(const Sound* sound, int selector);
/* 0x123E2 */
int16_t Dos94_soundext_SetPriority(const Sound* sound, uint16_t value);
/* 0x123FC */
int16_t Dos94_soundext_SetVolume(const Sound* sound, uint16_t value);
/* 0x12430 */
int16_t Dos94_soundext_SetTranspose(const Sound* sound, int16_t skipReset, int16_t value);
/* 0x1247A */
int16_t Dos94_soundext_SetGroup(const Sound* sound, uint16_t value);
/* 0x12494 */
int16_t Dos94_soundext_JumpMidi(const Sound* sound, int chunk, unsigned int beat, int tick);
/* 0x124C8 */
int16_t Dos94_soundext_ScanMidi(const Sound* sound, int chunk, unsigned int beat, int tick);
/* 0x124F8 */
int16_t Dos94_soundext_SetPartEnabled(const Sound* sound, int selector, int16_t enabled);
/* 0x12518 */
int16_t Dos94_soundext_SetHook(const Sound* sound, int mode, int value, int channel);
/* 0x12558 */
int16_t Dos94_soundext_FadeVolume(const Sound* sound, int volume, int duration);
/* 0x12574 */
int16_t Dos94_soundext_SetTriggerContext(Sound* sound, uint16_t marker);
/* 0x12594 */
int16_t Dos94_soundext_QueueTriggerCommand(uint16_t command, intptr_t soundId, intptr_t arg1, intptr_t arg2,
										   intptr_t arg3);
/* 0x12714 */
int16_t Dos94_soundext_ClearTriggers(void);
/* 0x12726 */
int16_t Dos94_soundext_CheckTriggers(void);
/* 0x12750 */
int16_t Dos94_soundext_ShareParts(const Sound* first, const Sound* second);
/* 0x101F4E */
void Dos94_soundext_Action_iMuse(XwSoundAction action, Sound* sound, int16_t value, int16_t duration);
#endif
