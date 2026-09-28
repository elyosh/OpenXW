#ifndef XW_RUNTIME_RUNTIME_TOURDESK_TASK_H
#define XW_RUNTIME_RUNTIME_TOURDESK_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Push a child wait for the tour desk join speech. The caller must yield
 * after tourdesk_PlaySpeech and retain its speech resources until resumption. */
void XwTourDesk_WaitForSpeech(void);

#ifdef __cplusplus
}
#endif

#endif
