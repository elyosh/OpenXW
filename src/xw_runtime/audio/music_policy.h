#ifndef XW_RUNTIME_AUDIO_MUSIC_POLICY_H
#define XW_RUNTIME_AUDIO_MUSIC_POLICY_H
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Soundtrack policy is independent of the frontend and flight versions. */
bool XwMusicPolicy_UsesImuse(void);
void XwMusicPolicy_Init(bool cd_available);
bool XwMusicPolicy_CdAvailable(void);
void XwMusicPolicy_DisableCd(const char* reason);
/* Close a failed CD device, clear its track count and disable it for this session. */
int XwMusicPolicy_FailCd(const char* reason);
#ifdef __cplusplus
}
#endif
#endif
