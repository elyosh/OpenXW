#ifndef XW_DOS94_RESIDENT_MODELS_H
#define XW_DOS94_RESIDENT_MODELS_H
#include <stddef.h>
#include <stdint.h>

typedef struct Dos94ResidentModel {
	const uint8_t* bytes;
	size_t size;
} Dos94ResidentModel;

extern const uint8_t Dos94_rebelLaserData[183];
extern const uint8_t Dos94_turboRebelLaserData[183];
extern const uint8_t Dos94_empireLaserData[183];
extern const uint8_t Dos94_ionCannonData[183];
extern const uint8_t Dos94_torpedoData[186];
extern const uint8_t Dos94_concussionData[186];
extern const uint8_t Dos94_hyperstarData[21];
extern const Dos94ResidentModel Dos94_projectileModels[6];
#endif
