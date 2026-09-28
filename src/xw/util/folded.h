#ifndef XW_UTIL_FOLDED_H
#define XW_UTIL_FOLDED_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Original release-build interfaces with folded implementations. */
struct Sound;

enum { XW_FOLDED_FADE_VOLUME = 1 };

int32_t hilevel_ImStartMusic(uint32_t soundId, int priority);
int32_t hilevel_ImStartResourceMusic(const struct Sound* sound, int priority);
int32_t lolevel_ImSetTrigger(uint32_t soundId, int marker, int command, ...);
int16_t lolevel_ImSetHook(uint32_t soundId, uint32_t hook);
int16_t lolevel_ImClearTrigger(uint32_t soundId, int marker, int command);
int16_t lolevel_ImSetResourceHook(const struct Sound* sound, uint32_t hook);
int16_t lolevel_ImFadeResourceParam(const struct Sound* sound, int parameter, int target, int duration);
int16_t lolevel_ImShareResourceParts(const struct Sound* first, const struct Sound* second);

void vesa_SetWindow(int window, unsigned int bank);
void Memory_UnlockHandle(uint16_t handle);
void DebugPrintf(const char* format, ...);
void gamesnd_Report_Sound_Load_Failure(const char* soundName, ...);
void ModelMesh_DescriptorNoOp(int objectType, int meshIndex);

#ifdef __cplusplus
}
#endif

#endif
