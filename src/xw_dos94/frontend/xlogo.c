#include "xw/frontend/scenes/xlogo.h"
#include "xw_dos94/frontend/intro.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include "xw/util/shared.h"
#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/xlogo_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/view.h>
#include <landru/viewadd.h>

/* DOS94 0x343252. */
XwShellSceneResult Dos94_XLogo_XLogo(struct XwShellContext* shellContext) {
	ResFile* resourceFile = xres_Open_Resource("xlogo.lfd");
	Rect frame;
	Film* film;
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	if (Shared_ReturnZero() != 0)
		film = xfilm_Res_Callback_Film(resourceFile, "xlogo_l", &frame, 0, 0, 0, XLogo_film_Callback);
	else
		film = xfilm_Res_Callback_Film(resourceFile, "xlogo_f", &frame, 0, 0, 0, XLogo_film_Callback);
	g_xlogoFilm = film;
	g_xlogoUpdateActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, XLOGO_UPDATE_Z);
	xactor_Set_Actor_User_Function(g_xlogoUpdateActor, XLogo_user_BeginDirtyFrame);
	xactor_Set_Actor_Draw_Function(g_xlogoUpdateActor, Cutscene_DrawEraseViewClip);
	g_xlogoCloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, XLOGO_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_xlogoCloseActor, XLogo_user_Close);
	xactor_Set_Actor_Draw_Function(g_xlogoCloseActor, XwCutscene_DrawCloseOnRefresh);
	xfilm_Set_Film_Def_Palette(g_xlogoFilm, shellContext->standardPalette);
	XLogo_OpenMusic(resourceFile, g_xlogoFilm);
	xview_Set_View_Update_Function(Dos94_XLogo_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	XwXLogo_RunView(resourceFile);
}

/* DOS94 0x34344a. */
void Dos94_XLogo_end_View(int time) {
	int16_t exitScene;
	(void)time;
	if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_INTRO_CREDITS, XW_SCENE_REGISTER_INITIAL,
								  g_xlogoFilm->cur_cel == g_xlogoFilm->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}
