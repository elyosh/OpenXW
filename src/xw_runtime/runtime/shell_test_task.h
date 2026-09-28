#ifndef XW_RUNTIME_RUNTIME_SHELL_TEST_TASK_H
#define XW_RUNTIME_RUNTIME_SHELL_TEST_TASK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Push the diagnostic view and clear its callback after it exits. The shell
 * owner must yield, then read the Landru exit code after resumption. */
void XwShellTest_RunView(void);

/* Schedule the choices dialog, then resume the scene callback with its result.
 * Returns zero while the owning view is waiting for the dialog. */
int XwShellTest_ReadSceneChoice(const char* labels, int16_t* choice);
int XwShellTest_ReadOutcomeChoice(const char* labels, int16_t* choice);

#ifdef __cplusplus
}
#endif

#endif
