#ifndef XW_DOS94_FRONTEND_DISPATCH_H
#define XW_DOS94_FRONTEND_DISPATCH_H
#include "xw/frontend/shell.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Returns nonzero when a DOS scene continuation was started. */
int16_t Dos94_ConvertTransition(int16_t scene, int16_t sudden);
int Dos94_OpenScene(XwShellSceneId scene, XwShellContext* shell);
#ifdef __cplusplus
}
#endif
#endif
