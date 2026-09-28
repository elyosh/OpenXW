#include "xw/frontend/scenes/rescue64.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#include "xw_runtime/integration/cutscene_callbacks.h"

#ifdef XW_MODERN
#include "xw_runtime/integration/landru_sound.h"
#include "xw_runtime/runtime/rescue64_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/view.h>
#include <landru/viewadd.h>

// GLOBAL: XW 0x4D7228
const char* g_rescue64ResourceNames[RESCUE64_RESOURCE_NAME_COUNT] = { "rescue64.lfd", "rescr_s", "resci_s",
																	  "rescr_f",      "resci_f", "newreb",
																	  "newimprs" };

// GLOBAL: XW 0x4FA108
Actor* g_rescue64BackgroundActor = NULL;

// GLOBAL: XW 0x4FA110
Rect g_rescue64PreviousDirtyRect = { 0 };

// GLOBAL: XW 0x4FA118
Actor* g_rescue64CloseActor = NULL;

// GLOBAL: XW 0x4FA120
Rect g_rescue64CurrentDirtyRect = { 0 };

// GLOBAL: XW 0x4FA128
Film* g_rescue64Film = NULL;

// GLOBAL: XW 0x4FA12C
LandruHandle g_rescue64BackgroundHandle = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4FA144
XwSceneMusicHandles g_rescue64MusicState = { NULL, NULL };

// GLOBAL: XW 0x4FA150
Sound* g_rescue64Speech[RESCUE64_SPEECH_COUNT] = { NULL };

// GLOBAL: XW 0x4FA15C
Film* g_rescue64SpeechFilm = NULL;

// FUNCTION: XW 0x45F360
XwShellSceneResult Rescue64_Play(struct XwShellContext* shell) {
	ResFile* sceneResource;
	Rect frame;
	if (shellext_Get_Cur_Scene() == XW_SCENE_PILOT_RESCUED) {
		xfade_AddTimedText("We've found him!", RESCUE64_FIRST_CAPTION_START, RESCUE64_FIRST_CAPTION_END,
						   RESCUE64_CAPTION_FONT, RESCUE64_CAPTION_X, RESCUE64_CAPTION_Y,
						   RESCUE64_RESCUE_COLOR);
		xfade_AddTimedText("He's OK.", RESCUE64_SECOND_CAPTION_START, RESCUE64_SECOND_CAPTION_END,
						   RESCUE64_CAPTION_FONT, RESCUE64_CAPTION_X, RESCUE64_CAPTION_Y,
						   RESCUE64_RESCUE_COLOR);
	} else {
		xfade_AddTimedText("There's the rebel!", RESCUE64_FIRST_CAPTION_START, RESCUE64_FIRST_CAPTION_END,
						   RESCUE64_CAPTION_FONT, RESCUE64_CAPTION_X, RESCUE64_CAPTION_Y,
						   RESCUE64_CAPTURE_COLOR);
		xfade_AddTimedText("Bring him in.", RESCUE64_SECOND_CAPTION_START, RESCUE64_SECOND_CAPTION_END,
						   RESCUE64_CAPTION_FONT, RESCUE64_CAPTURE_SECOND_X, RESCUE64_CAPTION_Y,
						   RESCUE64_CAPTURE_COLOR);
	}
	sceneResource = xres_Open_Resource(g_rescue64ResourceNames[RESCUE64_RESOURCE_FILE]);
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	if ((uint16_t)xio_Is_System_Slower_Than(RESCUE64_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
		Film* slowFilm;
		xviewadd_Clear_View();
		xview_Disable_All_View_Erase();
		xcanvas_Invalid_Screen_Diff();
		g_rescue64BackgroundHandle = xmemhdl_Alloc_Clear_Handle(
			RESCUE64_BACKGROUND_WIDTH * RESCUE64_BACKGROUND_HEIGHT, LANDRU_MEMORY_RESOURCE);
		if (shellext_Get_Cur_Scene() == XW_SCENE_PILOT_RESCUED)
			slowFilm =
				xfilm_Res_Callback_Film(sceneResource, g_rescue64ResourceNames[RESCUE64_SLOW_RESCUE_FILM],
										&frame, 0, 0, 0, Rescue64_film_Callback);
		else
			slowFilm =
				xfilm_Res_Callback_Film(sceneResource, g_rescue64ResourceNames[RESCUE64_SLOW_CAPTURE_FILM],
										&frame, 0, 0, 0, Rescue64_film_Callback);
		g_rescue64Film = slowFilm;
		g_rescue64BackgroundActor =
			xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, RESCUE64_BACKGROUND_Z);
		xactor_Set_Actor_User_Function(g_rescue64BackgroundActor, Rescue64_user_Background);
		xactor_Set_Actor_Draw_Function(g_rescue64BackgroundActor, Rescue64_draw_Background);
		g_rescue64CloseActor =
			xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, RESCUE64_CLOSE_Z);
		xactor_Set_Actor_User_Function(g_rescue64CloseActor, Rescue64_user_Close);
		xactor_Set_Actor_Draw_Function(g_rescue64CloseActor, XwCutscene_DrawCloseOnRefresh);
	} else {
		Film* fastFilm;
		if (shellext_Get_Cur_Scene() == XW_SCENE_PILOT_RESCUED)
			fastFilm =
				xfilm_Res_Callback_Film(sceneResource, g_rescue64ResourceNames[RESCUE64_FAST_RESCUE_FILM],
										&frame, 0, 0, 0, Rescue64_film_Callback);
		else
			fastFilm =
				xfilm_Res_Callback_Film(sceneResource, g_rescue64ResourceNames[RESCUE64_FAST_CAPTURE_FILM],
										&frame, 0, 0, 0, Rescue64_film_Callback);
		g_rescue64Film = fastFilm;
	}
	xfilm_Set_Film_Def_Palette(g_rescue64Film, shell->standardPalette);
	xview_Set_View_Update_Function(Rescue64_end_View);
	Rescue64_OpenMusic(sceneResource, g_rescue64Film);
	Rescue64_LoadSoundEffects(sceneResource, g_rescue64Film);
#ifdef XW_MODERN
	XwRescue64_RunView(sceneResource);
#else
	j_xviewadd_Handle_View();
	j_ShellPreferences_GetMusicEnabled();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	if ((uint16_t)xio_Is_System_Slower_Than(RESCUE64_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
		xview_Enable_All_View_Erase();
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		xview_Set_View_Frame(0, &frame);
		xview_Set_View_Pos(0, frame.left, frame.top);
		xmemhdl_Free_Handle(g_rescue64BackgroundHandle);
	}
	xres_Close_Resource(sceneResource);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x45F5F0
void Rescue64_end_View(int unusedTime) {
	int16_t exitScene;
	int16_t nextScene;
	int16_t nextSection;
	(void)unusedTime;
	if (shellext_Get_Cur_Scene() == XW_SCENE_PILOT_RESCUED) {
		nextScene = XW_SCENE_RESCUE_ARRIVE_SALVATION;
		nextSection = shipext_Get_Pending_Tour_Cutscene();
	} else {
		nextScene = XW_SCENE_CAPTURE_ARRIVE_EXECUTOR;
		nextSection = XW_SCENE_REGISTER_RETURN;
	}
	if (shellext_Check_Scene_Exit(&exitScene, nextScene, nextSection,
								  g_rescue64Film->cur_cel == g_rescue64Film->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x45F650
int16_t Rescue64_film_Callback(Film* film, FilmObject* object) {
	int16_t consumeObject = 0;
	Actor* actor;
	if (object->id == FTC_ACTOR) {
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		switch (actor->var1) {
			case CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS:
				if (xio_Is_System_Slower_Than(RESCUE64_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
					xactor_Set_Actor_User_Function(actor, Rescue64_user_DirtyActor);
				}
				break;
			case CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND:
				if (xio_Is_System_Slower_Than(RESCUE64_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
					Rescue64_film_Actor_To_Background(actor);
					consumeObject = 1;
				}
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, Rescue64_user_SoundCue);
				break;
		}
	}
	return consumeObject;
}

// FUNCTION: XW 0x45F6F0
int16_t Rescue64_film_Actor_To_Background(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_rescue64BackgroundHandle);
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						RESCUE64_BACKGROUND_WIDTH, RESCUE64_BACKGROUND_HEIGHT, 0);
	if (actor->draw != NULL) {
		xrect_Copy_Rect(&actorClip, &actor->frame);
		xcanvas_Clip_Rect_To_Canvas(&actorClip);
		xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
		drawResult = actor->draw(actor, &canvasBounds, &actorClip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_rescue64BackgroundHandle);
	return drawResult;
}

// FUNCTION: XW 0x45F7D0
void Rescue64_user_SoundCue(Actor* actor, int unusedTime) {
	int16_t cue = actor->var2;
	(void)unusedTime;
	if (cue != 0)
		Rescue64_PlaySoundCue(cue);
}

// FUNCTION: XW 0x45F7F0
void Rescue64_user_Background(Actor* unusedActor, int time) {
	(void)unusedActor;
	if (time == 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&g_rescue64PreviousDirtyRect);
	} else {
		xrect_Copy_Rect(&g_rescue64PreviousDirtyRect, &g_rescue64CurrentDirtyRect);
	}
	xrect_Clear_Rect(&g_rescue64CurrentDirtyRect);
}

// FUNCTION: XW 0x45F840
int16_t Rescue64_draw_Background(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
								 int16_t refresh) {
	const uint8_t* backgroundPixels;
	(void)actor;
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (refresh == 0) {
		return 0;
	}
	backgroundPixels = xmemhdl_Lock_Handle(g_rescue64BackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_rescue64PreviousDirtyRect,
								  g_rescue64PreviousDirtyRect.left, g_rescue64PreviousDirtyRect.top,
								  RESCUE64_BACKGROUND_WIDTH, RESCUE64_BACKGROUND_HEIGHT);
	xmemhdl_Unlock_Handle(g_rescue64BackgroundHandle);
	return 1;
}

// FUNCTION: XW 0x45F8A0
void Rescue64_user_DirtyActor(Actor* actor, int unusedTime) {
	(void)unusedTime;
	if (xactor_Is_Actor_Visible(actor) != 0 && actor->var2 == 0) {
		Rect visibleBounds;
		Rect actorFrame;
		xactor_Get_Actor_Rect(actor, &visibleBounds);
		xactor_Get_Actor_Frame(actor, &actorFrame);
		xcanvas_Clip_Rect_To_Canvas(&visibleBounds);
		xrect_Clip_Rect(&visibleBounds, &actorFrame);
		xrect_Enclose_Rect(&g_rescue64CurrentDirtyRect, &visibleBounds);
	}
}

// FUNCTION: XW 0x45F910
void Rescue64_user_Close(Actor* actor, int unusedTime) {
	Rect frame;
	(void)unusedTime;
	if (g_rescue64Film->cur_cel == g_rescue64Film->cels) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		actor->var1 = 1;
	} else {
		xrect_Copy_Rect(&frame, &g_rescue64PreviousDirtyRect);
		xrect_Enclose_Rect(&frame, &g_rescue64CurrentDirtyRect);
		actor->var1 = 0;
	}
	if (!xrect_Empty_Rect(&frame)) {
		xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
	}
}

// FUNCTION: XW 0x45FD40
void Rescue64_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		const char* musicName;
		unsigned int startBeat;
		if (shipext_Get_Mission_Outcome() != SHIPEXT_MISSION_OUTCOME_RESCUED) {
			musicName = "torture";
			startBeat = RESCUE64_TORTURE_START_BEAT;
		} else {
			musicName = "rescue";
			startBeat = RESCUE64_RESCUE_START_BEAT;
		}
		g_rescue64MusicState.film = sceneFilm;
		g_rescue64MusicState.sound = xsound_Find_Gmid(musicName);
		if (g_rescue64MusicState.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource("rsmusic.lfd");
			g_rescue64MusicState.sound = xsound_Res_Music(musicResource, musicName);
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_rescue64MusicState.sound);
			if (startBeat != 0) {
				soundext_ScanMidi(g_rescue64MusicState.sound, 0, startBeat, 0);
			}
		}
		xsound_Set_Sound_Keep(g_rescue64MusicState.sound);
		xsound_Set_Sound_User_Function(g_rescue64MusicState.sound, Cutscene_IgnoreSoundEvent);
	}
}

// FUNCTION: XW 0x45FE10
void Rescue64_LoadSoundEffects(ResFile* unusedResourceFile, Film* sceneFilm) {
#ifdef XW_MODERN
	void (*speechCallback)(Sound*, int) = XwLandru_RescueSpeechCallback;
#else
	void (*speechCallback)(Sound*, int) = (void (*)(Sound*, int))j_shellext_Get_Cur_Scene;
#endif
	(void)unusedResourceFile;
	if (ShellPreferences_GetSfxEnabled() != 0) {
		soundext_LoadSfx(XW_SHELL_SFX_DOOR_1A, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_TOUR_DOOR_CLOSE_1, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_TRACTOR, 0, NULL, 0, 1);
	}
	if (ShellPreferences_GetSfxEnabled() != 0) {
		if (shellext_Get_Cur_Scene() == XW_SCENE_PILOT_RESCUED) {
			g_rescue64Speech[0] = soundext_LoadSpeech(XW_SHELL_SPEECH_FOUND, 0, speechCallback, 0);
			g_rescue64Speech[1] = soundext_LoadSpeech(XW_SHELL_SPEECH_HES_OK, 1, speechCallback, 0);
		} else {
			g_rescue64Speech[0] = soundext_LoadSpeech(XW_SHELL_SPEECH_REBEL, 0, speechCallback, 0);
			g_rescue64Speech[1] = soundext_LoadSpeech(XW_SHELL_SPEECH_BRING, 1, speechCallback, 0);
		}
		g_rescue64SpeechFilm = sceneFilm;
	}
}

// FUNCTION: XW 0x45FEC0
int16_t j_shellext_Get_Cur_Scene(void) { return shellext_Get_Cur_Scene(); }

// FUNCTION: XW 0x45FED0
void Rescue64_PlaySoundCue(int16_t cue) {
	if (ShellPreferences_GetSfxEnabled()) {
		switch (cue) {
			case RESCUE64_CUE_DOOR_OPEN:
				soundext_Play_SFX(XW_SHELL_SFX_DOOR_1A);
				break;
			case RESCUE64_CUE_DOOR_CLOSE:
				soundext_Play_SFX(XW_SHELL_SFX_TOUR_DOOR_CLOSE_1);
				break;
			case RESCUE64_CUE_TRACTOR:
				soundext_Play_SFX(XW_SHELL_SFX_TRACTOR);
				break;
		}
	}
}
