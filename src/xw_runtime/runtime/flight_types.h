#ifndef XW_RUNTIME_FLIGHT_TYPES_H
#define XW_RUNTIME_FLIGHT_TYPES_H

#include "xw/assets/model_mesh.h"
#include "xw_runtime/storage/storage.h"

/* Capture before frontend model loading. Select only after the prior owner is torn down. */
void XwFlightTypes_Init(void);
void XwFlightTypes_Select(XwGameVersion version);
bool XwFlightTypes_Dos(void);
/* Translate individual identities. Canonical IDs are temporary classification/media keys,
 * never values to store in selected-version simulation state. Unmapped IDs return UINT16_MAX. */
uint16_t XwFlightTypes_ObjectType(uint16_t windowsType);
uint16_t XwFlightTypes_CanonicalType(uint16_t objectType);
uint8_t XwFlightTypes_MissionType(uint16_t missionType);
uint16_t XwFlightTypes_Definition(uint16_t objectType);
uint16_t XwFlightTypes_StatisticsCategory(uint16_t objectType);
uint16_t XwFlightTypes_ScoreWeight(uint16_t category);
int XwFlightTypes_ProjectileIndex(uint16_t objectType);
int XwFlightTypes_ProjectileSound(uint16_t objectType);
bool XwFlightTypes_IsWarhead(uint16_t objectType);
bool XwFlightTypes_IsMine(uint16_t objectType);
bool XwFlightTypes_IsFlybyCraft(uint16_t objectType);
bool XwFlightTypes_Targetable(uint16_t objectType);
uint8_t XwFlightTypes_ModelFlags(uint16_t objectType);
void XwFlightTypes_SetModelFlags(uint16_t objectType, uint8_t flags);
void XwFlightTypes_ClearRequiredModels(void);
void XwFlightTypes_RequireModel(uint16_t objectType);
void XwFlightTypes_RefreshModel(uint16_t objectType);
uint16_t XwFlightTypes_ComponentCount(uint16_t objectType);

#endif
