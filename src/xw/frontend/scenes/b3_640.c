#include "xw/frontend/scenes/b3_640.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/b3_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/memhdl.h>
#include <landru/view.h>

// GLOBAL: XW 0x4CFF70
const char* g_b3ResourceFilename = "b3_640.lfd";

// GLOBAL: XW 0x4CFF74
const char* g_b3FilmName = "hrc3_f1";

// GLOBAL: XW 0x4D02D8
const char* g_b3MusicFilename = "bl1music.lfd";

// GLOBAL: XW 0x4D02DC
const char* g_b3MusicName = "trofight";

// GLOBAL: XW 0x4D02E0
const char* g_b3NextMusicFilename = "bl4music.lfd";

// GLOBAL: XW 0x4D02E4
const char* g_b3NextMusicName = "runs2";

// GLOBAL: XW 0x4F4B2C
Film* g_b3Film = NULL;

// GLOBAL: XW 0x4F4B30
Actor* g_b3CloseActor = NULL;

// GLOBAL: XW 0x4F4B34
LandruHandle g_b3LegacyBufferHandle = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4F4BBC
XwSceneMusicHandles g_b3MusicState = { NULL, NULL };

// GLOBAL: XW 0x4F4BC8
XwCutsceneSpeechState g_b3SpeechState = { { NULL, NULL, NULL }, NULL };

// FUNCTION: XW 0x433A00
XwShellSceneResult B3_640_Play(struct XwShellContext* shell) {
	ResFile* resourceFile;
	Rect frame;
	xfade_AddTimedText("I can't shake him!", B3_640_FIRST_CAPTION_START, B3_640_FIRST_CAPTION_END,
					   B3_640_CAPTION_FONT, B3_640_CAPTION_X, B3_640_FIRST_CAPTION_Y,
					   B3_640_FIRST_CAPTION_COLOR);
	xfade_AddTimedText("I'm on him.", B3_640_SECOND_CAPTION_START, B3_640_SECOND_CAPTION_END,
					   B3_640_CAPTION_FONT, B3_640_CAPTION_X, B3_640_SECOND_CAPTION_Y,
					   B3_640_SECOND_CAPTION_COLOR);
	xfade_AddTimedText("Nice shot, Red Two!", B3_640_THIRD_CAPTION_START, B3_640_THIRD_CAPTION_END,
					   B3_640_CAPTION_FONT, B3_640_CAPTION_X, B3_640_THIRD_CAPTION_Y,
					   B3_640_FIRST_CAPTION_COLOR);
	resourceFile = xres_Open_Resource(g_b3ResourceFilename);
	xrect_Set_Rect(&frame, 0, 0, B3_640_SCENE_WIDTH, B3_640_SCENE_HEIGHT);
	g_b3Film = xfilm_Res_Film(resourceFile, g_b3FilmName, &frame, 0, 0, 0);
	g_b3CloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, B3_640_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_b3CloseActor, B3_640_user_Close);
	xactor_Set_Actor_Draw_Function(g_b3CloseActor, XwCutscene_DrawClose);
	xfilm_Set_Film_Def_Palette(g_b3Film, shell->standardPalette);
	xview_Set_View_Update_Function(B3_640_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	B3_640_OpenMusic(resourceFile, g_b3Film);
	B3_640_OpenSounds(resourceFile, g_b3Film);
#ifdef XW_MODERN
	XwB3_RunView(resourceFile);
#else
	j_xviewadd_Handle_View();
	B3_640_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xmemhdl_Free_Handle(g_b3LegacyBufferHandle);
	xres_Close_Resource(resourceFile);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x433BB0
void B3_640_end_View(int unusedTime) {
	int16_t exitScene;
	(void)unusedTime;
	if (g_savedShellPreferences.introPlaybackMode == 0) {
		if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_INTRO_DOGFIGHT_HRC4, XW_SCENE_REGISTER_INITIAL,
									  g_b3Film->cur_cel == g_b3Film->cels) != 0) {
			xerror_Set_Landru_Exit(exitScene);
		}
	} else if (g_b3Film->cur_cel == g_b3Film->cels) {
		xerror_Set_Landru_Exit(XW_SCENE_INTRO_DOGFIGHT_HRC4);
	}
}

// FUNCTION: XW 0x433C10
void B3_640_user_Close(Actor* actor, int unusedTime) {
	(void)unusedTime;
	if (g_b3Film->cur_cel == g_b3Film->cels) {
		actor->var1 = 1;
	} else {
		actor->var1 = 0;
	}
}

// FUNCTION: XW 0x434FC0
void B3_640_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_b3MusicState.film = sceneFilm;
		g_b3MusicState.sound = xsound_Find_Gmid(g_b3MusicName);
		if (g_b3MusicState.sound == NULL) {
			ResFile* resourceFile = xres_Open_Resource(g_b3MusicFilename);
			g_b3MusicState.sound = xsound_Res_Music(resourceFile, g_b3MusicName);
			xres_Close_Resource(resourceFile);
			soundext_Start_Resource_Sound(g_b3MusicState.sound);
			soundext_ScanMidi(g_b3MusicState.sound, 0, B3_MUSIC_START_BEAT, 0);
		}
		soundext_SetHook(g_b3MusicState.sound, 0, B3_MUSIC_INITIAL_CONTROL, 0);
		xsound_Set_Sound_Keep(g_b3MusicState.sound);
	}
}

// FUNCTION: XW 0x435070
void B3_640_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_b3MusicState.sound = xsound_Find_Gmid(g_b3MusicName);
		if (g_b3MusicState.sound != NULL) {
			ResFile* resourceFile = xres_Open_Resource(g_b3NextMusicFilename);
			Sound* nextMusic = xsound_Res_Music(resourceFile, g_b3NextMusicName);
			xres_Close_Resource(resourceFile);
			soundext_ClearTriggers();
			soundext_SetTriggerContext(g_b3MusicState.sound, B3_MUSIC_TRANSITION_MARKER);
			soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_START, nextMusic->id, 0, 0, 0, 0, 0);
			soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_STOP, g_b3MusicState.sound->id, 0, 0, 0, 0, 0);
			soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_END, 0, 0, 0, 0, 0, 0);
			xsound_Set_Sound_Keep(nextMusic);
		}
	}
}

// FUNCTION: XW 0x435140
void B3_640_OpenSounds(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	g_b3SpeechState.film = sceneFilm;
	if (ShellPreferences_GetSfxEnabled() != 0) {
		soundext_LoadSfx(XW_SHELL_SFX_FLY_SHOTS, 0, NULL, 0, 0);
		soundext_LoadSfx(XW_SHELL_SFX_EXPLOSION_FAR, 0, NULL, 0, 1);
	}
	if (ShellPreferences_GetSfxEnabled() != 0) {
		g_b3SpeechState.speech[0] =
			soundext_LoadSpeech(XW_SHELL_SPEECH_SHAKE_HIM, 0, B3_640_SpeechCallback, 0);
		g_b3SpeechState.speech[1] = soundext_LoadSpeech(XW_SHELL_SPEECH_ON_HIM, 1, B3_640_SpeechCallback, 0);
		g_b3SpeechState.speech[2] = soundext_LoadSpeech(XW_SHELL_SPEECH_RED_TWO, 2, B3_640_SpeechCallback, 0);
	}
}

// FUNCTION: XW 0x4351D0
void B3_640_SpeechCallback(Sound* sound, int unusedTime) {
	(void)unusedTime;
	switch (g_b3SpeechState.film->cur_cel) {
		case B3_SPEECH_FIRST_CEL:
			if (sound->var1 == 0) {
				soundext_Start_Resource_SFX(g_b3SpeechState.speech[0]);
			}
			break;
		case B3_SPEECH_SECOND_CEL:
			if (sound->var1 == 1) {
				soundext_Start_Resource_SFX(g_b3SpeechState.speech[1]);
			}
			break;
		case B3_SPEECH_THIRD_CEL:
			if (sound->var1 == 2) {
				soundext_Start_Resource_SFX(g_b3SpeechState.speech[2]);
			}
			break;
	}
}
