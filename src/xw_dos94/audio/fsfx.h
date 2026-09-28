#ifndef XW_DOS94_AUDIO_FSFX_H
#define XW_DOS94_AUDIO_FSFX_H
#include <stdint.h>

/* 0x6965E0 */
uint16_t Dos94_fsfx_loadsfx(const char* path);

/* 0x69674E */
int16_t Dos94_fsfx_triggersfx(uint16_t soundId, uint16_t objectIndex);

/* 0x696C06 */
void Dos94_fsfx_triggergunsightsfx(uint16_t toneState);

/* 0x696D0E */
int16_t Dos94_fsfx_triggervoicesfx(uint16_t sfxSlot);

/* 0x696E04 */
void Dos94_fsfx_checkblastqueue(void);
#endif
