#ifndef XW_DOS94_AUDIO_GAMESND_H
#define XW_DOS94_AUDIO_GAMESND_H
#include <stdint.h>

/* 0x107F0 */
int16_t Dos94_gamesnd_Open_Pre_iMuse(void);

/* 0x10872 */
void Dos94_gamesnd_Close_Pre_iMuse(void);

/* 0x108C0 */
void* Dos94_gamesnd_GetSoundAddr(intptr_t soundId);

/* 0x10DE0 */
void Dos94_gamesnd_game_Set_Front_Sound(void);

/* 0x10E1A */
void Dos94_gamesnd_game_Set_Flight_Sound(void);
#endif
