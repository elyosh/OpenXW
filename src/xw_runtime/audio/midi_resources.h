#ifndef XW_RUNTIME_AUDIO_MIDI_RESOURCES_H
#define XW_RUNTIME_AUDIO_MIDI_RESOURCES_H

#include <stdbool.h>
#include <stddef.h>

bool XwMidiResources_Sc55RomDirectoryValidate(const char* path, char* error, size_t capacity);
bool XwMidiResources_Mt32Available(void);
bool XwMidiResources_Mt32RomValidate(const char* path, bool control, char* error, size_t capacity);
bool XwMidiResources_Mt32PairValidate(const char* control, const char* pcm, char* error, size_t capacity);
const char* XwMidiResources_ParentDirectory(const char* path, char* out, size_t capacity);

#endif
