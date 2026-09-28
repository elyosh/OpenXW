#ifndef XW_RUNTIME_INTEGRATION_LANDRU_SOUND_H
#define XW_RUNTIME_INTEGRATION_LANDRU_SOUND_H

#include "xw/landru_config.h"

#include <landru/host.h>
#include <landru/sound.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Configure the audio callbacks before installing the complete LandruHost. */
void XwLandru_ConfigureSoundHost(LandruHost* host);
void XwLandru_ServiceAudio(void);

/* Adapt the rescue scene's getter thunk to the Landru speech callback ABI. */
void XwLandru_RescueSpeechCallback(Sound* sound, int time);

#ifdef __cplusplus
}
#endif

#endif
