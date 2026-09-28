#ifndef XW_DOS94_FLIGHT_MUSIC_H
#define XW_DOS94_FLIGHT_MUSIC_H
#include <stdint.h>
extern int16_t g_dos94DynamicMusicState;
extern int16_t g_dos94DynamicMusicIntensity;
extern int16_t g_dos94MusicChangeCooldownTicks;
extern uint8_t g_dos94DynamicMusicCombatEntered;
extern uint8_t g_dos94DynamicMusicLastState;
/* 0x681804 */
void Dos94_Xw_UpdateDynamicMusicState(void);
#endif
