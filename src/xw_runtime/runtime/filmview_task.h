#ifndef XW_RUNTIME_RUNTIME_FILMVIEW_TASK_H
#define XW_RUNTIME_RUNTIME_FILMVIEW_TASK_H

#include "xw/landru_config.h"

#include <landru/dialog.h>
#include <landru/res.h>

#ifdef __cplusplus
extern "C" {
#endif

struct FILMVIEW_FileDialog;

/* The scene owner yields until the view and resource cleanup complete. */
void XwFilmView_RunView(ResFile* resourceFile);

enum { XW_FILMVIEW_DIALOG_PENDING = -1 };

/* Schedule once, then consume the result when the scene callback resumes. */
int16_t XwFilmView_HandleFileDialog(void);

/* Keep directory and selection state alive through the owned sub-dialog. */
struct FILMVIEW_FileDialog* XwFilmView_BeginFileDialog(int16_t hadKeyButtons, DialogSubResultHandler complete,
													   void* context);
void XwFilmView_ScheduleFileDialog(int16_t built);

/* Resume the file action after confirmation; context is the original button. */
void XwFilmView_CompleteFileDeletion(int16_t accepted, void* context);

/* The film browser schedules one deletion dialog and resumes through complete.
 * The adapter owns the dialog and restores key-button mode on cancellation too. */
void XwFilmView_ScheduleDeleteDialog(Input* dialog, int16_t hadKeyButtons, DialogSubResultHandler complete,
									 void* context);

#ifdef __cplusplus
}
#endif

#endif
