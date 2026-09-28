#ifndef XW_RUNTIME_AUDIO_FLIGHT_MUSIC_H
#define XW_RUNTIME_AUDIO_FLIGHT_MUSIC_H
#include <stdbool.h>
#include <stddef.h>

typedef struct XwFlightMusicResource {
	char name[9];
	void* data;
	size_t size;
} XwFlightMusicResource;

extern XwFlightMusicResource g_flightMusicResources[125];
bool XwFlightMusic_Start(void);
void XwFlightMusic_Stop(void);
#endif
