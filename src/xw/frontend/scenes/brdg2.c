#include "xw/frontend/scenes/brdg2.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/brdg2_task.h"
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

// GLOBAL: XW 0x4D0AD0
const char* g_bridge2MusicResourceNames[BRDG2_MUSIC_RESOURCE_NAME_COUNT] = { "hr1music.lfd", "hangar" };

// GLOBAL: XW 0x4D0D30
const char* g_bridge2ResourceNames[BRDG2_RESOURCE_NAME_COUNT] = { "br2_640.lfd", "brdg2_s", "brdg2_f" };

// GLOBAL: XW 0x4F57B4
XwSceneMusicHandles g_bridge2MusicState = { NULL, NULL };

// GLOBAL: XW 0x4F57C0
Sound* g_bridge2SpeechSounds[XW_BRDG2_SPEECH_COUNT] = { NULL, NULL, NULL };

// GLOBAL: XW 0x4F57CC
Film* g_bridge2SpeechFilm = NULL;

// GLOBAL: XW 0x4F57F8
Film* g_bridge2Film = NULL;

// GLOBAL: XW 0x4F57FC
Actor* g_bridge2BackgroundActor = NULL;

// GLOBAL: XW 0x4F5808
Rect g_bridge2PreviousDirtyRect = { 0 };

// GLOBAL: XW 0x4F5810
Actor* g_bridge2EraseActor = NULL;

// GLOBAL: XW 0x4F5818
Rect g_bridge2DirtyRect = { 0 };

// GLOBAL: XW 0x4F5820
LandruHandle g_bridge2BackgroundHandle = LANDRU_NULL_HANDLE;

// FUNCTION: XW 0x438620
void Brdg2_OpenMusic(ResFile* unusedResourceFile, Film* film) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_bridge2MusicState.film = film;
		g_bridge2MusicState.sound = xsound_Find_Gmid(g_bridge2MusicResourceNames[BRDG2_MUSIC_RESOURCE_NAME]);
		if (g_bridge2MusicState.sound == NULL) {
			ResFile* musicResource =
				xres_Open_Resource(g_bridge2MusicResourceNames[BRDG2_MUSIC_RESOURCE_FILE]);
			g_bridge2MusicState.sound =
				xsound_Res_Music(musicResource, g_bridge2MusicResourceNames[BRDG2_MUSIC_RESOURCE_NAME]);
			soundext_Start_Resource_Sound(g_bridge2MusicState.sound);
			xres_Close_Resource(musicResource);
			soundext_ScanMidi(g_bridge2MusicState.sound, 0, BRDG2_MUSIC_START_BEAT, 0);
		}
		xsound_Set_Sound_Keep(g_bridge2MusicState.sound);
		xsound_Set_Sound_User_Function(g_bridge2MusicState.sound, Brdg2_user_Music);
	}
}

// FUNCTION: XW 0x4386C0
void Brdg2_user_Music(Sound* unusedSound, int unusedTime) {
	(void)unusedSound;
	(void)unusedTime;
	switch (g_bridge2MusicState.film->cur_cel) {
		case BRDG2_MUSIC_QUIET_CEL:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_FadeVolume(g_bridge2MusicState.sound, BRDG2_MUSIC_QUIET_VOLUME,
									BRDG2_MUSIC_FADE_DURATION);
			}
			break;
		case BRDG2_MUSIC_LOUD_CEL:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_FadeVolume(g_bridge2MusicState.sound, BRDG2_MUSIC_LOUD_VOLUME,
									BRDG2_MUSIC_FADE_DURATION);
			}
			break;
	}
}

// FUNCTION: XW 0x438720
void Brdg2_LoadSpeech(ResFile* unusedResourceFile, Film* film) {
	(void)unusedResourceFile;
	g_bridge2SpeechFilm = film;
	ShellPreferences_GetSfxEnabled();
	if (ShellPreferences_GetSfxEnabled()) {
		g_bridge2SpeechSounds[0] = soundext_LoadSpeech(XW_SHELL_SPEECH_ACKBAR_1, 0, NULL, 0);
		g_bridge2SpeechSounds[1] = soundext_LoadSpeech(XW_SHELL_SPEECH_ACKBAR_2, 0, NULL, 0);
		g_bridge2SpeechSounds[2] = soundext_LoadSpeech(XW_SHELL_SPEECH_ACKBAR_3, 0, NULL, 0);
	}
}

// FUNCTION: XW 0x438780
void Brdg2_HandleSpeechAction(int16_t action) {
	if (ShellPreferences_GetSfxEnabled() != 0) {
		switch (action) {
			case XW_BRDG2_SPEECH_ACTION_FIRST:
				soundext_Start_Resource_SFX(g_bridge2SpeechSounds[0]);
				break;
			case XW_BRDG2_SPEECH_ACTION_SECOND:
				soundext_Start_Resource_SFX(g_bridge2SpeechSounds[1]);
				break;
			case XW_BRDG2_SPEECH_ACTION_THIRD:
				soundext_Start_Resource_SFX(g_bridge2SpeechSounds[2]);
				break;
		}
	}
}

// FUNCTION: XW 0x438DF0
XwShellSceneResult Brdg2_Play(struct XwShellContext* shell) {
	ResFile* sceneResource;
	Film* sceneFilm;
	Rect frame;
	xfade_AddTimedText("We are under attack by Imperial Star Destroyers!", BRDG2_CAPTION_START,
					   BRDG2_CAPTION_END, BRDG2_CAPTION_FONT, BRDG2_CAPTION_X, BRDG2_FIRST_CAPTION_Y,
					   BRDG2_CAPTION_COLOR);
	xfade_AddTimedText("Begin evasive maneuvers.  Launch the X-Wing fighters.", BRDG2_CAPTION_START,
					   BRDG2_CAPTION_END, BRDG2_CAPTION_FONT, BRDG2_CAPTION_X, BRDG2_SECOND_CAPTION_Y,
					   BRDG2_CAPTION_COLOR);
	sceneResource = xres_Open_Resource(g_bridge2ResourceNames[BRDG2_RESOURCE_FILE]);
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	g_bridge2BackgroundHandle = xmemhdl_Alloc_Clear_Handle(BRDG2_BACKGROUND_BYTES, LANDRU_MEMORY_RESOURCE);
	if ((uint16_t)xio_Is_System_Slower_Than(BRDG2_FILM_SPEED_THRESHOLD) != 0)
		sceneFilm = xfilm_Res_Callback_Film(sceneResource, g_bridge2ResourceNames[BRDG2_RESOURCE_SLOW_FILM],
											&frame, 0, 0, 0, Brdg2_film_Callback);
	else
		sceneFilm = xfilm_Res_Callback_Film(sceneResource, g_bridge2ResourceNames[BRDG2_RESOURCE_FAST_FILM],
											&frame, 0, 0, 0, Brdg2_film_Callback);
	g_bridge2Film = sceneFilm;
	g_bridge2BackgroundActor =
		xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, BRDG2_BACKGROUND_Z);
	xactor_Set_Actor_User_Function(g_bridge2BackgroundActor, Brdg2_user_BeginDirtyFrame);
	xactor_Set_Actor_Draw_Function(g_bridge2BackgroundActor, Brdg2_draw_Background);
	g_bridge2EraseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, BRDG2_ERASE_Z);
	xactor_Set_Actor_User_Function(g_bridge2EraseActor, Brdg2_user_Erase);
	xactor_Set_Actor_Draw_Function(g_bridge2EraseActor, XwCutscene_DrawCloseOnRefresh);
	xfilm_Set_Film_Def_Palette(g_bridge2Film, shell->standardPalette);
	Brdg2_OpenMusic(sceneResource, g_bridge2Film);
	Brdg2_LoadSpeech(sceneResource, g_bridge2Film);
	xview_Set_View_Update_Function(Brdg2_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
#ifdef XW_MODERN
	XwBrdg2_RunView(sceneResource);
#else
	j_xviewadd_Handle_View();
	j_ShellPreferences_GetMusicEnabled();
	soundext_RecheckSfxPreference();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xmemhdl_Free_Handle(g_bridge2BackgroundHandle);
	xres_Close_Resource(sceneResource);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x438FF0
void Brdg2_end_View(int unusedTime) {
	int16_t exitScene;
	(void)unusedTime;
	if (g_savedShellPreferences.introPlaybackMode == 0) {
		if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_INTRO_HANGAR_LAUNCH, XW_SCENE_REGISTER_INITIAL,
									  g_bridge2Film->cur_cel == g_bridge2Film->cels) != 0) {
			xerror_Set_Landru_Exit(exitScene);
		}
	} else if (g_bridge2Film->cur_cel == g_bridge2Film->cels) {
		xerror_Set_Landru_Exit(XW_SCENE_INTRO_HANGAR_LAUNCH);
	}
}

// FUNCTION: XW 0x439050
int16_t Brdg2_film_Callback(Film* film, FilmObject* object) {
	int16_t handled = 0;
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		switch (actor->var1) {
			case CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS:
				xactor_Set_Actor_User_Function(actor, Brdg2_user_DirtyBounds);
				break;
			case CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND:
				Brdg2_StampBackground(actor);
				handled = 1;
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, Brdg2_user_SpeechAction);
				break;
		}
	}
	return handled;
}

// FUNCTION: XW 0x4390D0
int16_t Brdg2_StampBackground(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_bridge2BackgroundHandle);
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						BRDG2_BACKGROUND_WIDTH, BRDG2_BACKGROUND_HEIGHT, 0);
	if (actor->draw != NULL) {
		xrect_Copy_Rect(&actorClip, &actor->frame);
		xcanvas_Clip_Rect_To_Canvas(&actorClip);
		xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
		drawResult = actor->draw(actor, &canvasBounds, &actorClip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_bridge2BackgroundHandle);
	return drawResult;
}

// FUNCTION: XW 0x4391B0
void Brdg2_user_SpeechAction(Actor* actor, int unusedTime) {
	int16_t action = actor->var2;
	(void)unusedTime;
	if (action != 0) {
		Brdg2_HandleSpeechAction(action);
	}
}

// FUNCTION: XW 0x4391D0
void Brdg2_user_BeginDirtyFrame(Actor* unusedActor, int time) {
	(void)unusedActor;
	if (time == 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&g_bridge2PreviousDirtyRect);
	} else {
		xrect_Copy_Rect(&g_bridge2PreviousDirtyRect, &g_bridge2DirtyRect);
	}
	xrect_Clear_Rect(&g_bridge2DirtyRect);
}

// FUNCTION: XW 0x439220
int16_t Brdg2_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
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
	backgroundPixels = xmemhdl_Lock_Handle(g_bridge2BackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_bridge2PreviousDirtyRect,
								  g_bridge2PreviousDirtyRect.left, g_bridge2PreviousDirtyRect.top,
								  BRDG2_BACKGROUND_WIDTH, BRDG2_BACKGROUND_HEIGHT);
	xmemhdl_Unlock_Handle(g_bridge2BackgroundHandle);
	return 1;
}

// FUNCTION: XW 0x439280
void Brdg2_user_DirtyBounds(Actor* actor, int unusedTime) {
	Rect actorRect;
	Rect actorFrame;
	(void)unusedTime;
	if (xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &actorRect);
		xactor_Get_Actor_Frame(actor, &actorFrame);
		xcanvas_Clip_Rect_To_Canvas(&actorRect);
		xrect_Clip_Rect(&actorRect, &actorFrame);
		xrect_Enclose_Rect(&g_bridge2DirtyRect, &actorRect);
	}
}

// FUNCTION: XW 0x4392F0
void Brdg2_user_Erase(Actor* actor, int unusedTime) {
	Rect frame;
	(void)unusedTime;
	if (g_bridge2Film->cur_cel == g_bridge2Film->cels) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		actor->var1 = 1;
	} else {
		xrect_Copy_Rect(&frame, &g_bridge2PreviousDirtyRect);
		xrect_Enclose_Rect(&frame, &g_bridge2DirtyRect);
		actor->var1 = 0;
	}
	if (!xrect_Empty_Rect(&frame)) {
		xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
	}
}
