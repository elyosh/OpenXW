#ifndef XW_RUNTIME_RUNTIME_FRONTEND_TASK_H
#define XW_RUNTIME_RUNTIME_FRONTEND_TASK_H
#include "xw/frontend/shell.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Begin a shell visit. Its parent yields until this task completes, then reads
 * the result before beginning another shell visit. Memory is consumed in Begin. */
void XwFrontend_Begin(XwShellSceneId scene, struct XwLegacyMemoryConfig* memory);
XwShellSceneId XwFrontend_Result(void);
int XwFrontend_IsActive(void);
#ifdef __cplusplus
}
#endif
#endif
