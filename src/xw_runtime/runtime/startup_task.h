#ifndef XW_RUNTIME_RUNTIME_STARTUP_TASK_H
#define XW_RUNTIME_RUNTIME_STARTUP_TASK_H
#ifdef __cplusplus
extern "C" {
#endif
/* Begin returns startup acceptance; completion is read after the child task ends. */
int XwStartup_Begin(char* commandLine);
int XwStartup_Result(void);
#ifdef __cplusplus
}
#endif
#endif
