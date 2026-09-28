#ifndef XW_RUNTIME_SCENE_VIEW_TASK_H
#define XW_RUNTIME_SCENE_VIEW_TASK_H
/* Run the view, then its normal continuation; release also runs on cancellation. */
void XwScene_RunView(void (*complete)(void), void (*release)(void));
#endif
