#ifndef XW_RUNTIME_AUDIO_FRONTEND_MUSIC_H
#define XW_RUNTIME_AUDIO_FRONTEND_MUSIC_H
#include "xw/landru_config.h"
#include <landru/sound.h>
Sound* XwFrontendMusic_Load(ResFile* resource, const char* name);
void XwFrontendMusic_Release(Sound* sound);
#endif
