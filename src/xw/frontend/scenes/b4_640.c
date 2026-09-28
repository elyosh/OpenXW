#include "xw/frontend/scenes/b4_640.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/b4_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/memhdl.h>
#include <landru/view.h>
#include <stdlib.h>

// GLOBAL: XW 0x4D0050
const char* g_b4ResourceFilename = "b4_640.lfd";

// GLOBAL: XW 0x4D0054
const char* g_b4FilmName = "hrc4_f";

// GLOBAL: XW 0x4D0300
const char* g_b4MusicFilename = "bl4music.lfd";

// GLOBAL: XW 0x4D0304
const char* g_b4MusicName = "runs2";

// GLOBAL: XW 0x4D0308
const char* g_b4PreviousMusicName = "trofight";

// GLOBAL: XW 0x4F4B3C
Film* g_b4Film = NULL;

// GLOBAL: XW 0x4F4B40
Actor* g_b4CloseActor = NULL;

// GLOBAL: XW 0x4F4B44
LandruHandle g_b4LegacyBufferHandle = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4F4BD8
XwSceneMusicHandles g_b4MusicState = { NULL, NULL };

// FUNCTION: XW 0x433C40
XwShellSceneResult B4_640_Play(struct XwShellContext* shell) {
	ResFile* resourceFile = xres_Open_Resource(g_b4ResourceFilename);
	Rect frame;
	xrect_Set_Rect(&frame, 0, 0, B4_SCENE_WIDTH, B4_SCENE_HEIGHT);
	g_b4Film = xfilm_Res_Film(resourceFile, g_b4FilmName, &frame, 0, 0, 0);
	g_b4CloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, B4_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_b4CloseActor, B4_640_user_Close);
	xactor_Set_Actor_Draw_Function(g_b4CloseActor, XwCutscene_DrawClose);
	xfilm_Set_Film_Def_Palette(g_b4Film, shell->standardPalette);
	xview_Set_View_Update_Function(B4_640_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	B4_640_OpenMusic(resourceFile, g_b4Film);
	B4_640_OpenSounds(resourceFile, g_b4Film);
#ifdef XW_MODERN
	XwB4_RunView(resourceFile);
#else
	j_xviewadd_Handle_View();
	j_ShellPreferences_GetMusicEnabled();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xmemhdl_Free_Handle(g_b4LegacyBufferHandle);
	xres_Close_Resource(resourceFile);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x433D90
void B4_640_end_View(int unusedTime) {
	int16_t exitScene;
	(void)unusedTime;
	if (g_savedShellPreferences.introPlaybackMode == 0) {
		if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_XWING_LOGO, XW_SCENE_REGISTER_INITIAL,
									  g_b4Film->cur_cel == g_b4Film->cels) != 0) {
			xerror_Set_Landru_Exit(exitScene);
		}
	} else if (g_b4Film->cur_cel == g_b4Film->cels) {
		xerror_Set_Landru_Exit(XW_SCENE_XWING_LOGO);
	}
}

// FUNCTION: XW 0x433DF0
void B4_640_user_Close(Actor* actor, int unusedTime) {
	(void)unusedTime;
	if (g_b4Film->cur_cel == g_b4Film->cels) {
		actor->var1 = 1;
	} else {
		actor->var1 = 0;
	}
}

// FUNCTION: XW 0x435240
void B4_640_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		Sound* previousMusic;
		g_b4MusicState.film = sceneFilm;
		previousMusic = xsound_Find_Gmid(g_b4PreviousMusicName);
		if (previousMusic != NULL) {
			xsound_Set_Sound_Keep(previousMusic);
		}
		g_b4MusicState.sound = xsound_Find_Gmid(g_b4MusicName);
		if (g_b4MusicState.sound == NULL) {
			ResFile* resourceFile = xres_Open_Resource(g_b4MusicFilename);
			g_b4MusicState.sound = xsound_Res_Music(resourceFile, g_b4MusicName);
			xres_Close_Resource(resourceFile);
			soundext_Start_Resource_Sound(g_b4MusicState.sound);
		} else if (soundext_Count_Resource_Instances(g_b4MusicState.sound) != 1 &&
				   soundext_Count_Resource_Instances(previousMusic) != 1) {
			soundext_ClearTriggers();
			soundext_Start_Resource_Sound(g_b4MusicState.sound);
		}
		xsound_Set_Sound_Keep(g_b4MusicState.sound);
	}
}

// FUNCTION: XW 0x435310
void B4_640_OpenSounds(ResFile* unusedResourceFile, Film* unusedFilm) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	if (ShellPreferences_GetSfxEnabled() != 0) {
		soundext_LoadSfx(XW_SHELL_SFX_TORPEDO_2, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_EXPLOSION_FAR, 0, NULL, 0, 0);
		soundext_LoadSfx(XW_SHELL_SFX_TIE_APPROACH_1, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_FLYBY_6, 0, NULL, 0, 0);
	}
	ShellPreferences_GetSfxEnabled();
}
