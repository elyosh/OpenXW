#ifndef XW_RUNTIME_RUNTIME_REGISTER_TASK_H
#define XW_RUNTIME_RUNTIME_REGISTER_TASK_H

#include "xw/landru_config.h"

#include <landru/dialog.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { XW_REGISTER_VIEW_RESUME_PROTECTION = -1 };

/* Resume the frame-zero view callback after the protection dialog. */
void XwRegister_CompleteProtection(int16_t result, void* context);

/* Wait for acceptance speech, then select/create the pilot and exit the scene. */
void XwRegister_ScheduleAcceptPilot(Input* input, int context);

/* Resume Delete/Modify actions after the modal result. */
void XwRegister_CompletePilotDeletion(int16_t result, void* context);

/* Push a child wait for registration speech slot 1. The caller must yield
 * after register_PlaySpeech and retain its speech resources until resumption. */
void XwRegister_WaitForSpeech(void);

/* Own the dialog and restore key-button mode on dismissal or cancellation.
 * On dismissal, complete receives the original delete/cancel/revive button ID. */
void XwRegister_ScheduleDeleteDialog(Input* dialog, int16_t hadKeyButtons, DialogSubResultHandler complete,
									 void* context);

/* Own the copy-protection dialog and restore key-button mode on cleanup.
 * On dismissal, complete receives zero for Exit and one otherwise. */
void XwRegister_ScheduleProtectDialog(Input* dialog, int16_t hadKeyButtons, DialogSubResultHandler complete,
									  void* context);

#ifdef __cplusplus
}
#endif

#endif
