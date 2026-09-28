#ifndef XW_RUNTIME_AUDIO_MIDI_BACKEND_H
#define XW_RUNTIME_AUDIO_MIDI_BACKEND_H
#include <imuse.h>
#include <stdbool.h>
struct XwMusicSettings;
ImuseMidiBackend* XwMidiBackend_Create(char* error, size_t capacity);
void XwMidiBackend_ReleaseResources(void);
bool XwMidiBackend_SettingsPending(const struct XwMusicSettings* requested);
bool XwMidiBackend_UsesImuse(void);
uint32_t XwMidiBackend_ResourceType(void);
#endif
