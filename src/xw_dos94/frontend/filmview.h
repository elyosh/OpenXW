#ifndef XW_DOS94_FRONTEND_FILMVIEW_H
#define XW_DOS94_FRONTEND_FILMVIEW_H
#include "xw/frontend/filmview.h"
#ifdef __cplusplus
extern "C" {
#endif
XwShellSceneResult Dos94_filmview_FilmView(struct XwShellContext* shell);
void Dos94_filmview_Do_FV_File_Dialog(DialogSubResultHandler complete, void* context);
int16_t Dos94_filmview_Build_FV_File_Dialog(Input** outRoot, struct FILMVIEW_FileDialog* dialog,
											const char* title);
void Dos94_filmview_idraw_FilmView_Page(Input* unusedInput, Rect* frame, Rect* unusedClip, int16_t refresh);
void Dos94_filmview_idraw_FV_File(Input* input, Rect* frame, Rect* unusedClip, int16_t refresh);
int16_t Dos94_filmview_iupdate_FV_File(Input* input, Rect* frame, Rect* unusedClip, int16_t key,
									   int unusedLeftEvent, int unusedRightEvent, int16_t x, int16_t y);
void Dos94_filmview_iuser_FV_File(Input* input, int unusedContext);
void Dos94_filmview_Select_Active_FV_File(Input* input, Rect* frame, int16_t x, int16_t y);
void Dos94_filmview_Do_Delete_Dialog(DialogSubResultHandler complete, void* context);
Input* Dos94_filmview_Build_Delete_Dialog(void);
int16_t Dos94_filmview_iupdate_Delete_Input(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
											int rightEvent, int16_t x, int16_t y);
void Dos94_filmview_end_View(int time);
#ifdef __cplusplus
}
#endif
#endif
