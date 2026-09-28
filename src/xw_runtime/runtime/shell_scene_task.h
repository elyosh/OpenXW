#ifndef XW_RUNTIME_RUNTIME_SHELL_SCENE_TASK_H
#define XW_RUNTIME_RUNTIME_SHELL_SCENE_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Start the sudden fade, then save the scene after it completes. The shell
 * owner must yield until both child tasks have completed. */
void XwShell_FadeAndSaveScene(void);

#ifdef __cplusplus
}
#endif

#endif
