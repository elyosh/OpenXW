#include "xw/frontend/scenes/intro1.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include "xw/util/landru_display.h"

#ifdef XW_MODERN
#include "xw_runtime/runtime/intro1_task.h"
#endif

#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/view.h>
#include <stdlib.h>

// GLOBAL: XW 0x4D49E0
const char* g_intro1MusicFilename = "inmusic.lfd";

// GLOBAL: XW 0x4D49E4
const char* g_intro1MusicName = "inattack";

// GLOBAL: XW 0x4D50C0
const char* g_intro1ResourceFilename = "intro1.lfd";

// GLOBAL: XW 0x4D50C8
const char* g_intro1FilmName = "opening";

// GLOBAL: XW 0x4F76E4
XwSceneMusicHandles g_intro1MusicState = { NULL, NULL };

// GLOBAL: XW 0x4F76EC
Film* g_intro1SoundFilm = NULL;

// GLOBAL: XW 0x4F78EC
Film* g_intro1Film = NULL;

// FUNCTION: XW 0x44FD00
void Intro1_OpenMusic(ResFile* unusedResourceFile, Film* film) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled()) {
		g_intro1MusicState.film = film;
		g_intro1MusicState.sound = xsound_Find_Gmid(g_intro1MusicName);
		if (g_intro1MusicState.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource(g_intro1MusicFilename);
			g_intro1MusicState.sound = xsound_Res_Music(musicResource, g_intro1MusicName);
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_intro1MusicState.sound);
		}
		xsound_Set_Sound_User_Function(g_intro1MusicState.sound, Intro1_user_Music);
		xsound_Set_Sound_Keep(g_intro1MusicState.sound);
	}
}

// FUNCTION: XW 0x44FDA0
void Intro1_user_Music(Sound* sound, int time) {
	(void)sound;
	(void)time;
	switch (g_intro1MusicState.film->cur_cel) {
		case INTRO1_MUSIC_QUIET_CEL:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_FadeVolume(g_intro1MusicState.sound, INTRO1_MUSIC_QUIET_VOLUME,
									INTRO1_MUSIC_QUIET_FADE_DURATION);
			}
			break;
		case INTRO1_MUSIC_LOUD_CEL:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_FadeVolume(g_intro1MusicState.sound, INTRO1_MUSIC_LOUD_VOLUME,
									INTRO1_MUSIC_LOUD_FADE_DURATION);
			}
			break;
	}
}

// FUNCTION: XW 0x44FE00
void Intro1_LoadSoundEffects(ResFile* unusedResourceFile, Film* film) {
	(void)unusedResourceFile;
	g_intro1SoundFilm = film;
	if (ShellPreferences_GetSfxEnabled()) {
		if (ShellPreferences_GetSfxEnabled()) {
			soundext_LoadSfx(XW_SHELL_SFX_TIE_APPROACH_1, 0, NULL, 0, 1);
		} else {
			soundext_LoadSfx(XW_SHELL_SFX_TIE_APPROACH_4, 0, NULL, 0, 0);
		}
	}
	ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x44FE40
void Intro1_CloseSoundEffects(void) {
	ShellPreferences_GetSfxEnabled();
	ShellPreferences_GetSfxEnabled();
	if (shellext_Is_Sudden_Scene_End()) {
		soundext_ResetSfxCache(1);
	} else {
		soundext_ResetSfxCache(0);
	}
}

// FUNCTION: XW 0x44FE70
void Intro1_HandleSoundAction(int16_t action) {
	if (ShellPreferences_GetSfxEnabled()) {
		switch (action) {
			case INTRO1_ACTION_TIE_APPROACH_4:
				if (!ShellPreferences_GetSfxEnabled()) {
					soundext_Play_SFX(XW_SHELL_SFX_TIE_APPROACH_4);
				}
				break;
			case INTRO1_ACTION_TIE_APPROACH_1:
				if (ShellPreferences_GetSfxEnabled()) {
					soundext_Play_SFX(XW_SHELL_SFX_TIE_APPROACH_1);
				}
				break;
		}
	}
}

// FUNCTION: XW 0x454470
XwShellSceneResult Intro1_Opening(struct XwShellContext* shell) {
	ResFile* resourceFile = xres_Open_Resource(g_intro1ResourceFilename);
	Rect canvasBounds;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	xfade_AddTimedText("Star Destroyer patrol near the planet Turkana", INTRO1_CAPTION_START,
					   INTRO1_CAPTION_END, INTRO1_CAPTION_FONT, INTRO1_CAPTION_X, INTRO1_CAPTION_Y,
					   INTRO1_CAPTION_COLOR);
	g_intro1Film =
		xfilm_Res_Callback_Film(resourceFile, g_intro1FilmName, &canvasBounds, 0, 0, 0, Intro1_film_Callback);
	xfilm_Set_Film_Def_Palette(g_intro1Film, shell->standardPalette);
	xview_Set_View_Update_Function(Intro1_end_View);
	Intro1_OpenMusic(resourceFile, g_intro1Film);
	Intro1_LoadSoundEffects(resourceFile, g_intro1Film);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
#ifdef XW_MODERN
	XwIntro1_RunView(resourceFile);
#else
	j_xviewadd_Handle_View();
	j_ShellPreferences_GetMusicEnabled();
	Intro1_CloseSoundEffects();
	xview_Clear_View_Update_Function();
	xres_Close_Resource(resourceFile);
	LandruDisplay_ForwardLegacyNoOp(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x454560
void Intro1_end_View(int time) {
	int16_t exitScene;
	(void)time;
	if (g_savedShellPreferences.introPlaybackMode == 0) {
		if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_INTRO_IMPERIAL_BRIDGE, XW_SCENE_REGISTER_INITIAL,
									  g_intro1Film->cur_cel == g_intro1Film->cels) != 0) {
			xerror_Set_Landru_Exit(exitScene);
		}
	} else if (g_intro1Film->cur_cel == g_intro1Film->cels) {
		xerror_Set_Landru_Exit(XW_SCENE_INTRO_IMPERIAL_BRIDGE);
	}
}

// FUNCTION: XW 0x4545C0
int16_t Intro1_film_Callback(Film* film, FilmObject* filmObject) {
	if (filmObject->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, filmObject, filmObject + 1);
		actor = filmObject->object;
		if (actor->var1 == CUTSCENE_ACTOR_ROLE_SOUND)
			xactor_Set_Actor_User_Function(actor, Intro1_user_SoundAction);
	}
	return 0;
}

// FUNCTION: XW 0x454600
void Intro1_user_SoundAction(Actor* actor, int time) {
	int16_t action = actor->var2;
	(void)time;
	if (action) {
		Intro1_HandleSoundAction(action);
	}
}
