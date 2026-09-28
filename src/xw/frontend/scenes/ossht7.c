#include "xw/frontend/scenes/ossht7.h"

#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/scenes/sun_shot.h"
#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/ossht7_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/memhdl.h>
#include <landru/view.h>
#include <landru/viewadd.h>

// GLOBAL: XW 0x4CFBF0
const char* g_ossht7ResourceFilename = "shot7640.lfd";

// GLOBAL: XW 0x4CFBF4
const char* g_ossht7FilmName = "ossht7";

// GLOBAL: XW 0x4F4AE8
Film* g_ossht7Film = NULL;

// GLOBAL: XW 0x4F4AEC
Actor* g_ossht7CloseActor = NULL;

// GLOBAL: XW 0x4F4AF0
LandruHandle g_ossht7BufferHandle = LANDRU_NULL_HANDLE;

// FUNCTION: XW 0x432860
XwShellSceneResult Ossht7_Play(struct XwShellContext* context) {
	ResFile* resourceFile = xres_Open_Resource(g_ossht7ResourceFilename);
	Rect frame;
	xrect_Set_Rect(&frame, 0, 0, OSSHT7_SCENE_WIDTH, OSSHT7_SCENE_HEIGHT);
	g_ossht7BufferHandle = xmemhdl_Alloc_Clear_Handle(OSSHT7_BUFFER_BYTES, LANDRU_MEMORY_RESOURCE);
	xviewadd_Clear_View();
	xcanvas_Invalid_Screen_Diff();
	g_ossht7Film = xfilm_Res_Film(resourceFile, g_ossht7FilmName, &frame, 0, 0, 0);
	g_ossht7CloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, OSSHT7_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_ossht7CloseActor, Ossht7_user_Close);
	xactor_Set_Actor_Draw_Function(g_ossht7CloseActor, XwCutscene_DrawClose);
	xfilm_Set_Film_Def_Palette(g_ossht7Film, context->standardPalette);
	xview_Set_View_Update_Function(Ossht7_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	Cutscene_EnsureTroFightMusic(resourceFile, g_ossht7Film);
	SunShot_LoadSounds();
#ifdef XW_MODERN
	XwOssht7_RunView(resourceFile);
#else
	j_xviewadd_Handle_View();
	j_ShellPreferences_GetMusicEnabled();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	{
		LandruHandle buffer = g_ossht7BufferHandle;
		xmemhdl_Free_Handle(buffer);
	}
	xres_Close_Resource(resourceFile);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x4329D0
void Ossht7_end_View(int time) {
	int16_t exitScene;

	(void)time;
	if (g_savedShellPreferences.introPlaybackMode == 0) {
		if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_INTRO_SUNSHOT, XW_SCENE_REGISTER_INITIAL,
									  g_ossht7Film->cur_cel == g_ossht7Film->cels) != 0) {
			xerror_Set_Landru_Exit(exitScene);
		}
	} else if (g_ossht7Film->cur_cel == g_ossht7Film->cels) {
		xerror_Set_Landru_Exit(XW_SCENE_INTRO_SUNSHOT);
	}
}

// FUNCTION: XW 0x432A30
void Ossht7_user_Close(Actor* actor, int time) {
	(void)time;
	if (g_ossht7Film->cur_cel == g_ossht7Film->cels) {
		actor->var1 = 1;
	} else {
		actor->var1 = 0;
	}
}
