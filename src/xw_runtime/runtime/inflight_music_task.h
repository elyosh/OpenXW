#ifndef XW_RUNTIME_RUNTIME_INFLIGHT_MUSIC_TASK_H
#define XW_RUNTIME_RUNTIME_INFLIGHT_MUSIC_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Return nonzero when startup is suspended. The UI owner must yield and retain
 * music resources until the child completes the remainder of OpenMusic. */
int XwInflightMusic_WaitForTick(int unusedSceneContext);

#ifdef __cplusplus
}
#endif

#endif
