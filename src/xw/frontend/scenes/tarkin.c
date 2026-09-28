#include "xw/frontend/scenes/tarkin.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#include "xw/util/landru_display.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/tarkin_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/memhdl.h>
#include <landru/view.h>

// GLOBAL: XW 0x4FA1C0
Actor* g_tarkinBackgroundActor = NULL;

// GLOBAL: XW 0x4FA1C4
Film* g_tarkinFilm = NULL;

// GLOBAL: XW 0x4FA1C8
Rect g_tarkinPreviousDirtyRect = { 0 };

// GLOBAL: XW 0x4FA1D0
Actor* g_tarkinCloseActor = NULL;

// GLOBAL: XW 0x4FA1D8
Rect g_tarkinCurrentDirtyRect = { 0 };

// GLOBAL: XW 0x4FA1E0
LandruHandle g_tarkinBackgroundHandle = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4FAD44
XwSceneMusicHandles g_tarkinMusicState = { NULL, NULL };

// GLOBAL: XW 0x4FAD4C
Sound* g_tarkinSpeech = NULL;

// FUNCTION: XW 0x461220
XwShellSceneResult Tarkin_Play(struct XwShellContext* shell) {
	ResFile* sceneResource;
	Rect frame;
	LandruDisplay_SetLowResolutionMode(1);
	xfade_AddTimedText("This station is insignificant compared", TARKIN_CAPTION_START, TARKIN_CAPTION_END,
					   TARKIN_CAPTION_FONT, TARKIN_FIRST_CAPTION_X, TARKIN_FIRST_CAPTION_Y,
					   TARKIN_CAPTION_COLOR);
	xfade_AddTimedText("with the power of the force.", TARKIN_CAPTION_START, TARKIN_CAPTION_END,
					   TARKIN_CAPTION_FONT, TARKIN_SECOND_CAPTION_X, TARKIN_SECOND_CAPTION_Y,
					   TARKIN_CAPTION_COLOR);
	sceneResource = xres_Open_Resource("tarkin.lfd");
	xrect_Set_Rect(&frame, 0, 0, TARKIN_BACKGROUND_WIDTH, TARKIN_BACKGROUND_HEIGHT);
	g_tarkinBackgroundHandle = xmemhdl_Alloc_Clear_Handle(TARKIN_BACKGROUND_BYTES, LANDRU_MEMORY_RESOURCE);
	xcanvas_Erase_Canvas();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	g_tarkinBackgroundActor =
		xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, TARKIN_BACKGROUND_Z);
	xactor_Set_Actor_User_Function(g_tarkinBackgroundActor, Tarkin_user_Background);
	xactor_Set_Actor_Draw_Function(g_tarkinBackgroundActor, Tarkin_draw_Background);
	g_tarkinCloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, TARKIN_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_tarkinCloseActor, Tarkin_user_Close);
	g_tarkinFilm = xfilm_Res_Callback_Film(sceneResource, "dsdn4b_f", &frame, 0, 0, 0, Tarkin_film_Callback);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	xfilm_Set_Film_Def_Palette(g_tarkinFilm, shell->standardPalette);
	xview_Set_View_Update_Function(Tarkin_end_View);
	Tarkin_OpenMusic(sceneResource, g_tarkinFilm);
	Tarkin_LoadSpeech(sceneResource, g_tarkinFilm);
#ifdef XW_MODERN
	XwTarkin_RunView(sceneResource);
#else
	j_xviewadd_Handle_View();
	j_ShellPreferences_GetMusicEnabled();
	soundext_RecheckSfxPreference();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xmemhdl_Free_Handle(g_tarkinBackgroundHandle);
	xres_Close_Resource(sceneResource);
	LandruDisplay_SetLowResolutionMode(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x461410
void Tarkin_end_View(int unusedTime) {
	int16_t exitScene;
	(void)unusedTime;
	if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_DEATH_STAR_COMPLETION_FIRE_ORDER,
								  shipext_Get_Pending_Tour_Cutscene(),
								  g_tarkinFilm->cur_cel == g_tarkinFilm->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x461450
int16_t Tarkin_film_Callback(Film* film, FilmObject* object) {
	int16_t consumeObject = 0;
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		switch (actor->var1) {
			case CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS:
				xactor_Set_Actor_User_Function(actor, Tarkin_user_DirtyActor);
				break;
			case CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND:
				Tarkin_film_Actor_To_Background(actor);
				consumeObject = 1;
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, Tarkin_user_SoundCue);
				break;
		}
	}
	return consumeObject;
}

// FUNCTION: XW 0x4614D0
int16_t Tarkin_film_Actor_To_Background(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_tarkinBackgroundHandle);
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						TARKIN_BACKGROUND_WIDTH, TARKIN_BACKGROUND_HEIGHT, 0);
	if (actor->draw != NULL) {
		xcanvas_Set_Drawing_Canvas_Clip(&actor->frame);
		drawResult = actor->draw(actor, &canvasBounds, &actor->frame, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_tarkinBackgroundHandle);
	return drawResult;
}

// FUNCTION: XW 0x461590
void Tarkin_user_SoundCue(Actor* actor, int unusedTime) {
	int16_t cue = actor->var2;
	(void)unusedTime;
	if (cue != 0) {
		Tarkin_PlaySoundCue(cue);
	}
}

// FUNCTION: XW 0x4615B0
void Tarkin_user_Background(Actor* unusedActor, int time) {
	(void)unusedActor;
	if (time == 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&g_tarkinPreviousDirtyRect);
	} else {
		xrect_Copy_Rect(&g_tarkinPreviousDirtyRect, &g_tarkinCurrentDirtyRect);
	}
	xrect_Clear_Rect(&g_tarkinCurrentDirtyRect);
	g_tarkinBackgroundActor->var1 = 0;
}

// FUNCTION: XW 0x461600
int16_t Tarkin_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
							   int16_t unusedY, int16_t refresh) {
	(void)unusedActor;
	(void)unusedFrame;
	(void)unusedClip;
	(void)unusedX;
	(void)unusedY;
	if (refresh == 0) {
		return 0;
	}
	if (g_tarkinBackgroundActor->var1 != 0) {
		xcanvas_Erase_Canvas();
	} else {
		const uint8_t* backgroundPixels = xmemhdl_Lock_Handle(g_tarkinBackgroundHandle);
		stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_tarkinPreviousDirtyRect,
									  g_tarkinPreviousDirtyRect.left, g_tarkinPreviousDirtyRect.top,
									  TARKIN_BACKGROUND_WIDTH, TARKIN_BACKGROUND_HEIGHT);
		xmemhdl_Unlock_Handle(g_tarkinBackgroundHandle);
	}
	return 1;
}

// FUNCTION: XW 0x461670
void Tarkin_user_DirtyActor(Actor* actor, int unusedTime) {
	Rect visibleBounds;
	Rect actorFrame;
	(void)unusedTime;
	if (xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &visibleBounds);
		xactor_Get_Actor_Frame(actor, &actorFrame);
		xcanvas_Clip_Rect_To_Canvas(&visibleBounds);
		xrect_Clip_Rect(&visibleBounds, &actorFrame);
		xrect_Enclose_Rect(&g_tarkinCurrentDirtyRect, &visibleBounds);
	}
	if (actor->var2 != 0) {
		g_tarkinBackgroundActor->var1 = TARKIN_ERASE_FULL_CANVAS;
	}
}

// FUNCTION: XW 0x4616F0
void Tarkin_user_Close(Actor* unusedActor, int unusedTime) {
	Rect frame;
	(void)unusedActor;
	(void)unusedTime;
	if (g_tarkinBackgroundActor->var1 != 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	} else {
		xrect_Copy_Rect(&frame, &g_tarkinPreviousDirtyRect);
		xrect_Enclose_Rect(&frame, &g_tarkinCurrentDirtyRect);
	}
	if (xrect_Empty_Rect(&frame) == 0) {
		xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
	}
}

// FUNCTION: XW 0x462530
void Tarkin_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_tarkinMusicState.film = sceneFilm;
		g_tarkinMusicState.sound = xsound_Find_Gmid("empire");
		if (g_tarkinMusicState.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource("ddmusic.lfd");
			g_tarkinMusicState.sound = xsound_Res_Music(musicResource, "empire");
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_tarkinMusicState.sound);
			soundext_ScanMidi(g_tarkinMusicState.sound, 0, TARKIN_MUSIC_START_BEAT, 0);
		}
		xsound_Set_Sound_Keep(g_tarkinMusicState.sound);
		xsound_Set_Sound_User_Function(g_tarkinMusicState.sound, Tarkin_user_Music);
	}
}

// FUNCTION: XW 0x4625D0
void Tarkin_user_Music(Sound* unusedSound, int unusedTime) {
	int filmCel = g_tarkinMusicState.film->cur_cel;
	(void)unusedSound;
	(void)unusedTime;
	if (filmCel == TARKIN_MUSIC_QUIET_CEL) {
		if (ShellPreferences_GetSfxEnabled() != 0) {
			soundext_FadeVolume(g_tarkinMusicState.sound, TARKIN_MUSIC_QUIET_VOLUME,
								TARKIN_MUSIC_FADE_DURATION);
		}
	} else if (filmCel == TARKIN_MUSIC_LOUD_CEL) {
		if (ShellPreferences_GetSfxEnabled() != 0) {
			soundext_FadeVolume(g_tarkinMusicState.sound, TARKIN_MUSIC_LOUD_VOLUME,
								TARKIN_MUSIC_FADE_DURATION);
		}
	}
}

// FUNCTION: XW 0x462630
void Tarkin_LoadSpeech(ResFile* unusedResourceFile, Film* unusedFilm) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	ShellPreferences_GetSfxEnabled();
	if (ShellPreferences_GetSfxEnabled()) {
		g_tarkinSpeech = soundext_LoadSpeech(XW_SHELL_SPEECH_STATION, 0, NULL, 0);
	}
}

// FUNCTION: XW 0x462660
void Tarkin_PlaySoundCue(int16_t cue) {
	switch (cue) {
		case XW_TARKIN_CUE_SPEECH:
			if (ShellPreferences_GetSfxEnabled()) {
				soundext_Start_Resource_SFX(g_tarkinSpeech);
			}
			break;
	}
}
