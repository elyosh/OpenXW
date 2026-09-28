#ifndef XW_RUNTIME_INPUT_CONTROLLER_MAPPING_H
#define XW_RUNTIME_INPUT_CONTROLLER_MAPPING_H
#include "xw_runtime/input/controller_options.h"
void XwControllerMapping_Init(const XwControllerOptions* options);
void XwControllerMapping_SetOptions(const XwControllerOptions* options);
void XwControllerMapping_ApplyPending(void);
void XwControllerMapping_Update(const AeronInputSnapshot* input);
void XwControllerMapping_Suspend(void);
void XwControllerMapping_Shutdown(void);
const XwControllerOptions* XwControllerMapping_Options(void);
int XwControllerMapping_Axis(XwInputAxis axis);
uint16_t XwControllerMapping_Modifiers(void);
uint16_t XwControllerMapping_ReadKey(void);
void XwControllerMapping_ReleaseCommands(void);
const AeronControllerSnapshot* XwControllerMapping_Resolve(const XwControllerModel* model,
														   const AeronInputSnapshot* input,
														   uint32_t preferred);
uint32_t XwControllerMapping_AnalogInstance(const char* guid);
uint16_t XwControllerMapping_ThrottlePosition(int16_t raw, AeronControllerKind kind, int source, bool invert);
bool XwControllerMapping_ThrottleSample(uint16_t* position, uint32_t* generation);
int XwControllerMapping_Present(void);
int XwControllerMapping_MenuAxis(XwInputAxis axis);
uint8_t XwControllerMapping_MenuButtons(void);
uint8_t XwControllerMapping_MenuHat(void);
#endif
