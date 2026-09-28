#include "xw/util/folded.h"

/* Keep these bodies separate from callers so matching retains argument evaluation. */

/* DOS94 confirms the operations; TIE95 also retains the music-start priority argument. */
// FUNCTION: XW 0x485100 FOLDED
int32_t hilevel_ImStartMusic(uint32_t soundId, int priority) {
	(void)soundId;
	(void)priority;
	return 0;
}

// FUNCTION: XW 0x485100 FOLDED
int32_t hilevel_ImStartResourceMusic(const struct Sound* sound, int priority) {
	(void)sound;
	(void)priority;
	return 0;
}

// FUNCTION: XW 0x485100 FOLDED
int32_t lolevel_ImSetTrigger(uint32_t soundId, int marker, int command, ...) {
	(void)soundId;
	(void)marker;
	(void)command;
	return 0;
}

/* The folded low-level backend returns AX=0, not a full EAX zero. */
// FUNCTION: XW 0x49E560 FOLDED
int16_t lolevel_ImSetHook(uint32_t soundId, uint32_t hook) {
	(void)soundId;
	(void)hook;
	return 0;
}

// FUNCTION: XW 0x49E560 FOLDED
int16_t lolevel_ImClearTrigger(uint32_t soundId, int marker, int command) {
	(void)soundId;
	(void)marker;
	(void)command;
	return 0;
}

/* Resource views preserve Sound* arguments without converting native pointers to numeric IDs. */
// FUNCTION: XW 0x49E560 FOLDED
int16_t lolevel_ImSetResourceHook(const struct Sound* sound, uint32_t hook) {
	(void)sound;
	(void)hook;
	return 0;
}

// FUNCTION: XW 0x49E560 FOLDED
int16_t lolevel_ImFadeResourceParam(const struct Sound* sound, int parameter, int target, int duration) {
	(void)sound;
	(void)parameter;
	(void)target;
	(void)duration;
	return 0;
}

// FUNCTION: XW 0x49E560 FOLDED
int16_t lolevel_ImShareResourceParts(const struct Sound* first, const struct Sound* second) {
	(void)first;
	(void)second;
	return 0;
}

// FUNCTION: XW 0x49E5C0 FOLDED
void Memory_UnlockHandle(uint16_t handle) { (void)handle; }

// FUNCTION: XW 0x49E5C0 FOLDED
void DebugPrintf(const char* format, ...) { (void)format; }

/* DOS94 0x11B2C forwards the failed sound name to an optional host callback. */
// FUNCTION: XW 0x49E5C0 FOLDED
void gamesnd_Report_Sound_Load_Failure(const char* soundName, ...) { (void)soundName; }

/* Two-index call after caching a descriptor; its exact original purpose is unresolved. */
// FUNCTION: XW 0x49E5C0 FOLDED
void ModelMesh_DescriptorNoOp(int objectType, int meshIndex) {
	(void)objectType;
	(void)meshIndex;
}

/* Bank switching is disabled in the Windows release. */
// FUNCTION: XW 0x49E5C0 FOLDED
void vesa_SetWindow(int window, unsigned int bank) {
	(void)window;
	(void)bank;
}
