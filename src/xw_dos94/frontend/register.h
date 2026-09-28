#ifndef XW_DOS94_FRONTEND_REGISTER_H
#define XW_DOS94_FRONTEND_REGISTER_H
#include "xw/frontend/register.h"
#ifdef __cplusplus
extern "C" {
#endif
void Dos94_register_end_View(int time);
int16_t Dos94_register_iupdate_Register(Input* input, Rect* frame, Rect* clip, int16_t phase, int leftEvent,
										int rightEvent, int16_t x, int16_t y);
void Dos94_register_iuser_Pilot_Button(Input* input, int unusedContext);
void Dos94_register_iuser_Pilot_Name(Input* input, int unusedContext);
void Dos94_register_xuser_Pilot_Name(const char* searchName);
void Dos94_register_Delete_Pilot_Record(void);
struct REGISTER_RegStringButton* Dos94_register_Alloc_Input_Reg_String_Button(
	Input* parent, Rect* frame, int16_t zinput, InputUserFunc user, const char* initialName,
	int16_t isFilenameMode, int16_t id);
int16_t Dos94_register_iupdate_Reg_String_Button(Input* input, Rect* frame, Rect* clip, int16_t key,
												 int leftEvent, int rightEvent, int16_t x, int16_t y);
int16_t Dos94_register_Add_Key_To_Reg_String(struct REGISTER_RegStringButton* input, char* text,
											 char character);
void Dos94_register_Do_Delete_Dialog(DialogSubResultHandler complete, void* context);
Input* Dos94_register_Build_Delete_Dialog(void);
int16_t Dos94_register_iupdate_Delete_Input(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
											int rightEvent, int16_t x, int16_t y);
void Dos94_register_idraw_Delete_Input(Input* input, Rect* frame, Rect* clip, int16_t refresh);
void Dos94_register_idraw_Pilot_Info(Input* input, Rect* frame, Rect* clip, int16_t refresh);
void Dos94_register_idraw_ListMessage(Input* input, Rect* frame, Rect* unusedClip, int16_t refresh);
void Dos94_register_user_Music(Sound* unusedSound, int unusedTime);
void Dos94_Register_CompleteDeletion(int16_t result, void* context);
void Dos94_Register(XwShellContext* shell);
#ifdef __cplusplus
}
#endif
#endif
