#include "xw/frontend/scenes/brdg1.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/util/landru_display.h"

#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/brdg1_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/view.h>
#include <landru/viewadd.h>

// GLOBAL: XW 0x4D0B20
const char* g_bridge1ResourceNames[BRDG1_RESOURCE_NAME_COUNT] = { "br1_640.lfd", "brdg1_s", "brg1f640" };

// GLOBAL: XW 0x4F5790
XwSceneMusicHandles g_bridge1MusicState = { NULL, NULL };

// GLOBAL: XW 0x4F5798
Sound* g_bridge1SpeechSounds[XW_BRDG1_SPEECH_COUNT] = { NULL, NULL, NULL, NULL, NULL, NULL };

// GLOBAL: XW 0x4F57B0
Film* g_bridge1SpeechFilm = NULL;

// GLOBAL: XW 0x4F57D0
Film* g_bridge1Film = NULL;

// GLOBAL: XW 0x4F57D4
Actor* g_bridge1BackgroundActor = NULL;

// GLOBAL: XW 0x4F57D8
Rect g_bridge1PreviousDirtyRect = { 0 };

// GLOBAL: XW 0x4F57E0
Actor* g_bridge1EraseActor = NULL;

// GLOBAL: XW 0x4F57E8
Rect g_bridge1DirtyRect = { 0 };

// GLOBAL: XW 0x4F57F0
LandruHandle g_bridge1BackgroundHandle = LANDRU_NULL_HANDLE;

// FUNCTION: XW 0x438460
void Brdg1_OpenMusic(ResFile* unusedResourceFile, Film* film) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_bridge1MusicState.film = film;
		g_bridge1MusicState.sound = xsound_Find_Gmid("inattack");
		if (g_bridge1MusicState.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource("inmusic.lfd");
			g_bridge1MusicState.sound = xsound_Res_Music(musicResource, "inattack");
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_bridge1MusicState.sound);
			soundext_ScanMidi(g_bridge1MusicState.sound, 0, BRDG1_MUSIC_START_BEAT, 0);
		}
		xsound_Set_Sound_Keep(g_bridge1MusicState.sound);
		xsound_Set_Sound_User_Function(g_bridge1MusicState.sound, Brdg1_user_Music);
	}
}

// FUNCTION: XW 0x438500
void Brdg1_user_Music(Sound* unusedSound, int unusedTime) {
	(void)unusedSound;
	(void)unusedTime;
	switch (g_bridge1MusicState.film->cur_cel) {
		case BRDG1_MUSIC_QUIET_CEL:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_FadeVolume(g_bridge1MusicState.sound, BRDG1_MUSIC_QUIET_VOLUME,
									BRDG1_MUSIC_FADE_DURATION);
			}
			break;
		case BRDG1_MUSIC_LOUD_CEL:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_FadeVolume(g_bridge1MusicState.sound, BRDG1_MUSIC_LOUD_VOLUME,
									BRDG1_MUSIC_FADE_DURATION);
			}
			break;
	}
}

// FUNCTION: XW 0x438560
void Brdg1_LoadSpeech(ResFile* unusedResourceFile, Film* film) {
	(void)unusedResourceFile;
	g_bridge1SpeechFilm = film;
	ShellPreferences_GetSfxEnabled();
	if (ShellPreferences_GetSfxEnabled()) {
		g_bridge1SpeechSounds[BRDG1_SPEECH_SIR] =
			soundext_LoadSpeech(XW_SHELL_SPEECH_SIR, BRDG1_SPEECH_SIR, NULL, 0);
		g_bridge1SpeechSounds[BRDG1_SPEECH_OUR_TIES] =
			soundext_LoadSpeech(XW_SHELL_SPEECH_OUR_TIES, BRDG1_SPEECH_OUR_TIES, NULL, 0);
		g_bridge1SpeechSounds[BRDG1_SPEECH_EXCELLENT] =
			soundext_LoadSpeech(XW_SHELL_SPEECH_EXCELLENT, BRDG1_SPEECH_EXCELLENT, NULL, 0);
		g_bridge1SpeechSounds[BRDG1_SPEECH_THE_ATTACK] =
			soundext_LoadSpeech(XW_SHELL_SPEECH_THE_ATTACK, BRDG1_SPEECH_THE_ATTACK, NULL, 0);
		g_bridge1SpeechSounds[BRDG1_SPEECH_MOVE_OUR] =
			soundext_LoadSpeech(XW_SHELL_SPEECH_MOVE_OUR, BRDG1_SPEECH_MOVE_OUR, NULL, 0);
		g_bridge1SpeechSounds[BRDG1_SPEECH_ONCE_SIR] =
			soundext_LoadSpeech(XW_SHELL_SPEECH_ONCE_SIR, BRDG1_SPEECH_ONCE_SIR, NULL, 0);
	}
}

// FUNCTION: XW 0x438600
void Brdg1_HandleSpeechAction(int16_t action) {
	if (ShellPreferences_GetSfxEnabled()) {
		soundext_Start_Resource_SFX(g_bridge1SpeechSounds[action - 1]);
	}
}

// FUNCTION: XW 0x4387D0
XwShellSceneResult Brdg1_Play(struct XwShellContext* shell) {
	ResFile* sceneResource;
	Film* sceneFilm;
	Rect frame;
	xfade_AddTimedText("Sir, our TIE Interceptors have located a", BRDG1_REPORT_START, BRDG1_REPORT_END,
					   BRDG1_CAPTION_FONT, BRDG1_REPORT_X, BRDG1_REPORT_FIRST_Y, BRDG1_OFFICER_COLOR);
	xfade_AddTimedText("Rebel Fleet orbiting the planet Turkana.", BRDG1_REPORT_START, BRDG1_REPORT_END,
					   BRDG1_CAPTION_FONT, BRDG1_REPORT_X, BRDG1_REPORT_SECOND_Y, BRDG1_OFFICER_COLOR);
	xfade_AddTimedText("Excellent!  Prepare the attack.", BRDG1_ATTACK_START, BRDG1_ATTACK_END,
					   BRDG1_CAPTION_FONT, BRDG1_ATTACK_X, BRDG1_ATTACK_Y, BRDG1_COMMANDER_COLOR);
	xfade_AddTimedText("Move our Star Destroyers within range", BRDG1_ORDERS_START, BRDG1_ORDERS_END,
					   BRDG1_CAPTION_FONT, BRDG1_ORDERS_X, BRDG1_ORDERS_FIRST_Y, BRDG1_COMMANDER_COLOR);
	xfade_AddTimedText("and launch all TIE Fighter Squadrons.", BRDG1_ORDERS_START, BRDG1_ORDERS_END,
					   BRDG1_CAPTION_FONT, BRDG1_ORDERS_X, BRDG1_ORDERS_SECOND_Y, BRDG1_COMMANDER_COLOR);
	xfade_AddTimedText("At once, Sir!", BRDG1_ACK_START, BRDG1_ACK_END, BRDG1_CAPTION_FONT, BRDG1_ACK_X,
					   BRDG1_ACK_Y, BRDG1_OFFICER_COLOR);
	sceneResource = xres_Open_Resource(g_bridge1ResourceNames[BRDG1_RESOURCE_FILE]);
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	g_bridge1BackgroundHandle = xmemhdl_Alloc_Clear_Handle(BRDG1_BACKGROUND_BYTES, LANDRU_MEMORY_RESOURCE);
	if ((uint16_t)xio_Is_System_Slower_Than(BRDG1_FILM_SPEED_THRESHOLD) != 0)
		sceneFilm = xfilm_Res_Callback_Film(sceneResource, g_bridge1ResourceNames[BRDG1_RESOURCE_SLOW_FILM],
											&frame, 0, 0, 0, Brdg1_film_Callback);
	else
		sceneFilm = xfilm_Res_Callback_Film(sceneResource, g_bridge1ResourceNames[BRDG1_RESOURCE_FAST_FILM],
											&frame, 0, 0, 0, Brdg1_film_Callback);
	g_bridge1Film = sceneFilm;
	g_bridge1BackgroundActor =
		xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, BRDG1_BACKGROUND_Z);
	xactor_Set_Actor_User_Function(g_bridge1BackgroundActor, Brdg1_user_BeginDirtyFrame);
	xactor_Set_Actor_Draw_Function(g_bridge1BackgroundActor, Brdg1_draw_Background);
	g_bridge1EraseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, BRDG1_ERASE_Z);
	xactor_Set_Actor_User_Function(g_bridge1EraseActor, Brdg1_user_Erase);
	xactor_Set_Actor_Draw_Function(g_bridge1EraseActor, XwCutscene_DrawCloseOnRefresh);
	xfilm_Set_Film_Def_Palette(g_bridge1Film, shell->standardPalette);
	Brdg1_OpenMusic(sceneResource, g_bridge1Film);
	Brdg1_LoadSpeech(sceneResource, g_bridge1Film);
	xview_Set_View_Update_Function(Brdg1_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
#ifdef XW_MODERN
	XwBrdg1_RunView(sceneResource);
#else
	j_xviewadd_Handle_View();
	j_ShellPreferences_GetMusicEnabled();
	soundext_RecheckSfxPreference();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xmemhdl_Free_Handle(g_bridge1BackgroundHandle);
	xres_Close_Resource(sceneResource);
	LandruDisplay_ForwardLegacyNoOp(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x438A50
void Brdg1_end_View(int unusedTime) {
	int16_t exitScene;
	(void)unusedTime;
	if (g_savedShellPreferences.introPlaybackMode == 0) {
		if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_INTRO_FLEET_ATTACK, XW_SCENE_REGISTER_INITIAL,
									  g_bridge1Film->cur_cel == g_bridge1Film->cels) != 0) {
			xerror_Set_Landru_Exit(exitScene);
		}
	} else if (g_bridge1Film->cur_cel == g_bridge1Film->cels) {
		xerror_Set_Landru_Exit(XW_SCENE_INTRO_FLEET_ATTACK);
	}
}

// FUNCTION: XW 0x438AB0
int16_t Brdg1_film_Callback(Film* film, FilmObject* object) {
	int16_t handled = 0;
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		switch (actor->var1) {
			case CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS:
				xactor_Set_Actor_User_Function(actor, Brdg1_user_DirtyBounds);
				break;
			case CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND:
				Brdg1_StampBackground(actor);
				handled = 1;
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, Brdg1_user_SpeechAction);
				break;
		}
	}
	return handled;
}

// FUNCTION: XW 0x438B30
int16_t Brdg1_StampBackground(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_bridge1BackgroundHandle);
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						BRDG1_BACKGROUND_WIDTH, BRDG1_BACKGROUND_HEIGHT, 0);
	if (actor->draw != NULL) {
		xrect_Copy_Rect(&actorClip, &actor->frame);
		xcanvas_Clip_Rect_To_Canvas(&actorClip);
		xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
		drawResult = actor->draw(actor, &canvasBounds, &actorClip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_bridge1BackgroundHandle);
	return drawResult;
}

// FUNCTION: XW 0x438C10
void Brdg1_user_SpeechAction(Actor* actor, int unusedTime) {
	int16_t action = actor->var2;
	(void)unusedTime;
	if (action != 0) {
		Brdg1_HandleSpeechAction(action);
	}
}

// FUNCTION: XW 0x438C30
void Brdg1_user_BeginDirtyFrame(Actor* unusedActor, int time) {
	(void)unusedActor;
	if (time == 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&g_bridge1PreviousDirtyRect);
	} else {
		xrect_Copy_Rect(&g_bridge1PreviousDirtyRect, &g_bridge1DirtyRect);
	}
	xrect_Clear_Rect(&g_bridge1DirtyRect);
}

// FUNCTION: XW 0x438C80
int16_t Brdg1_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
							  int16_t unusedY, int16_t refresh) {
	const uint8_t* backgroundPixels;
	(void)unusedActor;
	(void)unusedFrame;
	(void)unusedClip;
	(void)unusedX;
	(void)unusedY;
	if (refresh == 0) {
		return 0;
	}
	backgroundPixels = xmemhdl_Lock_Handle(g_bridge1BackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_bridge1PreviousDirtyRect,
								  g_bridge1PreviousDirtyRect.left, g_bridge1PreviousDirtyRect.top,
								  BRDG1_BACKGROUND_WIDTH, BRDG1_BACKGROUND_HEIGHT);
	xmemhdl_Unlock_Handle(g_bridge1BackgroundHandle);
	return 1;
}

// FUNCTION: XW 0x438CE0
void Brdg1_user_DirtyBounds(Actor* actor, int unusedTime) {
	Rect actorRect;
	Rect actorFrame;
	(void)unusedTime;
	if (xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &actorRect);
		xactor_Get_Actor_Frame(actor, &actorFrame);
		xcanvas_Clip_Rect_To_Canvas(&actorRect);
		xrect_Clip_Rect(&actorRect, &actorFrame);
		xrect_Enclose_Rect(&g_bridge1DirtyRect, &actorRect);
	}
}

// FUNCTION: XW 0x438D50
void Brdg1_user_Erase(Actor* actor, int unusedTime) {
	Rect frame;
	(void)unusedTime;
	if (g_bridge1Film->cur_cel == g_bridge1Film->cels) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		actor->var1 = 1;
	} else {
		xrect_Copy_Rect(&frame, &g_bridge1PreviousDirtyRect);
		xrect_Enclose_Rect(&frame, &g_bridge1DirtyRect);
		actor->var1 = 0;
	}
	if (!xrect_Empty_Rect(&frame)) {
		xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
	}
}
