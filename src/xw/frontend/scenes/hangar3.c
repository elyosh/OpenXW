#include "xw/frontend/scenes/hangar3.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#include "xw_runtime/integration/cutscene_callbacks.h"

#ifdef XW_MODERN
#include "xw_runtime/runtime/hangar3_task.h"
#include "xw_runtime/runtime/hangar3_view_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/font.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/timer.h>
#include <landru/view.h>
#include <landru/viewadd.h>

// GLOBAL: XW 0x4D3CC8
char g_hangar3ResourceNames[HANGAR3_RESOURCE_NAME_COUNT][HANGAR3_RESOURCE_NAME_SIZE] = {
	"hanger3.lfd", "intro5_s", "intro5_f", "hang3x_s",  "hang3x_f",     "hang3y_s", "hang3y_f", "hang3a_s",
	"hang3a_f",    "retx_s",   "retx_f",   "rety_s",    "rety_f",       "reta_s",   "reta_f",   "retb_s",
	"retb_f",      "hang3b_s", "hang3b_f", "bwing.lfd", "hgr3_640.lfd", "launch2s", "launch2f", "hang3x_s",
	"hang3x_f",    "hang3y_s", "hang3y_f", "hang3a_s",  "hang3a_f",     "retx_s",   "retx_f",   "rety_s",
	"rety_f",      "reta_s",   "reta_f",   "retb_s",    "retb_f",       "hang3b_s", "hang3b_f"
};

// GLOBAL: XW 0x4F75F8
Actor* g_hangar3BackgroundActor = NULL;

// GLOBAL: XW 0x4F7600
Rect g_hangar3PreviousDirtyRect = { 0 };

// GLOBAL: XW 0x4F7608
Film* g_hangar3Film = NULL;

// GLOBAL: XW 0x4F760C
Actor* g_hangar3EraseActor = NULL;

// GLOBAL: XW 0x4F7610
Rect g_hangar3DirtyRect = { 0 };

// GLOBAL: XW 0x4F7618
LandruHandle g_hangar3BackgroundHandle = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4F7698
XwSceneMusicHandles g_hangar3MusicState = { NULL, NULL };

// FUNCTION: XW 0x44C470
XwShellSceneResult Hangar3_Play(struct XwShellContext* shell) {
	int manageSceneAudio = 1;
	ResFile* resourceFile;
	int16_t filmPairIndex;
	Rect frame;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_TOUR_LAUNCH_BWING:
		case XW_SCENE_TOUR_LAUNCH_XWING:
		case XW_SCENE_TOUR_LAUNCH_YWING:
		case XW_SCENE_TOUR_LAUNCH_AWING: {
			int16_t halfWidth = xfont_Get_String_Width_0(HANGAR3_CAPTION_FONT, "Leaving the Defiance.") >> 1;
			xfade_AddTimedText("Leaving the Defiance.", HANGAR3_CAPTION_START, HANGAR3_CAPTION_END,
							   HANGAR3_CAPTION_FONT, HANGAR3_BACKGROUND_WIDTH / 2 - halfWidth,
							   HANGAR3_CAPTION_Y, HANGAR3_CAPTION_COLOR);
			break;
		}
		case XW_SCENE_TOUR_RETURN_AWING:
		case XW_SCENE_TOUR_RETURN_XWING:
		case XW_SCENE_TOUR_RETURN_YWING:
		case XW_SCENE_TOUR_RETURN_BWING: {
			int16_t halfWidth =
				xfont_Get_String_Width_0(HANGAR3_CAPTION_FONT, "Returning to the Defiance.") >> 1;
			xfade_AddTimedText("Returning to the Defiance.", HANGAR3_CAPTION_START, HANGAR3_CAPTION_END,
							   HANGAR3_CAPTION_FONT, HANGAR3_BACKGROUND_WIDTH / 2 - halfWidth,
							   HANGAR3_CAPTION_Y, HANGAR3_CAPTION_COLOR);
			break;
		}
		case XW_SCENE_TOUR_RETURN_INDEPENDENCE: {
			int16_t halfWidth =
				xfont_Get_String_Width_0(HANGAR3_CAPTION_FONT, "Returning to Independence.") >> 1;
			xfade_AddTimedText("Returning to Independence.", HANGAR3_CAPTION_START, HANGAR3_CAPTION_END,
							   HANGAR3_CAPTION_FONT, HANGAR3_BACKGROUND_WIDTH / 2 - halfWidth,
							   HANGAR3_CAPTION_Y, HANGAR3_CAPTION_COLOR);
			break;
		}
	}
	resourceFile = xres_Open_Resource(g_hangar3ResourceNames[HANGAR3_HIGH_RESOLUTION_OFFSET]);
	xrect_Set_Rect(&frame, 0, 0, HANGAR3_BACKGROUND_WIDTH, HANGAR3_BACKGROUND_HEIGHT);
	g_hangar3BackgroundHandle = xmemhdl_Alloc_Clear_Handle(
		HANGAR3_BACKGROUND_WIDTH * HANGAR3_BACKGROUND_HEIGHT, LANDRU_MEMORY_RESOURCE);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	filmPairIndex = HANGAR3_INTRO_FILM;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_INTRO_HANGAR_LAUNCH:
			xtimer_Set_FrameRatePreset(LANDRU_FRAME_PRESET_4);
			filmPairIndex = HANGAR3_INTRO_FILM;
			break;
		case XW_SCENE_TOUR_RETURN_INDEPENDENCE:
		case XW_SCENE_TOUR_LAUNCH_XWING:
			filmPairIndex = HANGAR3_LAUNCH_XWING_FILM;
			manageSceneAudio = 0;
			break;
		case XW_SCENE_TOUR_LAUNCH_YWING:
			filmPairIndex = HANGAR3_LAUNCH_YWING_FILM;
			manageSceneAudio = 0;
			break;
		case XW_SCENE_TOUR_LAUNCH_AWING:
			filmPairIndex = HANGAR3_LAUNCH_AWING_FILM;
			manageSceneAudio = 0;
			break;
		case XW_SCENE_TOUR_LAUNCH_BWING:
			filmPairIndex = HANGAR3_LAUNCH_BWING_FILM;
			manageSceneAudio = 0;
			break;
		case XW_SCENE_TOUR_RETURN_XWING:
			filmPairIndex = HANGAR3_RETURN_XWING_FILM;
			manageSceneAudio = 0;
			break;
		case XW_SCENE_TOUR_RETURN_YWING:
			filmPairIndex = HANGAR3_RETURN_YWING_FILM;
			manageSceneAudio = 0;
			break;
		case XW_SCENE_TOUR_RETURN_AWING:
			filmPairIndex = HANGAR3_RETURN_AWING_FILM;
			manageSceneAudio = 0;
			break;
		case XW_SCENE_TOUR_RETURN_BWING:
			filmPairIndex = HANGAR3_RETURN_BWING_FILM;
			manageSceneAudio = 0;
			break;
			/* Unsupported scenes retain the intro film pair instead of pointer bits. */
	}
	if ((uint16_t)xio_Is_System_Slower_Than(HANGAR3_SPEED_THRESHOLD) == 0)
		++filmPairIndex;
	filmPairIndex += HANGAR3_HIGH_RESOLUTION_OFFSET;
	g_hangar3Film = xfilm_Res_Callback_Film(resourceFile, g_hangar3ResourceNames[filmPairIndex], &frame, 0, 0,
											0, Hangar3_film_Callback);
	xfilm_Set_Film_Def_Palette(g_hangar3Film, shell->standardPalette);
	g_hangar3BackgroundActor =
		xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, HANGAR3_BACKGROUND_Z);
	xactor_Set_Actor_User_Function(g_hangar3BackgroundActor, Hangar3_user_BeginDirtyFrame);
	xactor_Set_Actor_Draw_Function(g_hangar3BackgroundActor, Hangar3_draw_Background);
	g_hangar3EraseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, HANGAR3_ERASE_Z);
	xactor_Set_Actor_User_Function(g_hangar3EraseActor, Hangar3_user_Erase);
	xactor_Set_Actor_Draw_Function(g_hangar3EraseActor, XwCutscene_DrawCloseOnRefresh);
	if (manageSceneAudio != 0) {
		Hangar3_OpenMusic(resourceFile, g_hangar3Film);
		Hangar3_LoadSoundEffects(resourceFile, g_hangar3Film);
	}
	xview_Set_View_Update_Function(Hangar3_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
#ifdef XW_MODERN
	XwHangar3_RunView(resourceFile, manageSceneAudio);
#else
	j_xviewadd_Handle_View();
	xtimer_Set_FrameRatePreset(LANDRU_FRAME_PRESET_2);
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xmemhdl_Free_Handle(g_hangar3BackgroundHandle);
	if (manageSceneAudio != 0) {
		Hangar3_CloseMusic();
		soundext_ResetEnabledSfxCache();
	}
	xres_Close_Resource(resourceFile);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x44C8D0
void Hangar3_end_View(int unusedTime) {
	int16_t exitScene;
	int16_t nextScene;
	int16_t nextSection;
	(void)unusedTime;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_INTRO_HANGAR_LAUNCH:
			if (g_savedShellPreferences.introPlaybackMode == 0) {
				nextScene = XW_SCENE_INTRO_FORMATION;
				nextSection = XW_SCENE_REGISTER_INITIAL;
			} else {
				nextScene = XW_SCENE_INTRO_FORMATION;
				nextSection = XW_SCENE_INTRO_FORMATION;
			}
			break;
		case XW_SCENE_TOUR_RETURN_INDEPENDENCE:
			nextScene = XW_SCENE_CONCOURSE;
			nextSection = XW_SCENE_CONCOURSE;
			break;
		case XW_SCENE_TOUR_LAUNCH_BWING:
			nextScene = XW_SCENE_FLIGHT_TOUR;
			nextSection = XW_SCENE_FLIGHT_TOUR;
			break;
		case XW_SCENE_TOUR_LAUNCH_XWING:
		case XW_SCENE_TOUR_LAUNCH_YWING:
		case XW_SCENE_TOUR_LAUNCH_AWING:
			nextScene = XW_SCENE_FLIGHT_TOUR;
			nextSection = XW_SCENE_FLIGHT_TOUR;
			break;
		case XW_SCENE_TOUR_RETURN_AWING:
			nextScene = nextSection = shipext_Get_Pending_Tour_Cutscene();
			break;
		case XW_SCENE_TOUR_RETURN_XWING:
			nextScene = nextSection = shipext_Get_Pending_Tour_Cutscene();
			break;
		case XW_SCENE_TOUR_RETURN_YWING:
			nextScene = nextSection = shipext_Get_Pending_Tour_Cutscene();
			break;
		case XW_SCENE_TOUR_RETURN_BWING:
			nextScene = nextSection = shipext_Get_Pending_Tour_Cutscene();
			break;
#ifdef XW_MODERN
		default:
			return;
#endif
	}
	if (shellext_Check_Scene_Exit(&exitScene, nextScene, nextSection,
								  g_hangar3Film->cur_cel == g_hangar3Film->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x44C9C0
int16_t Hangar3_film_Callback(Film* film, FilmObject* object) {
	int16_t handled = 0;
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		switch (actor->var1) {
			case CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS:
				xactor_Set_Actor_User_Function(actor, Hangar3_user_DirtyBounds);
				break;
			case CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND:
				Hangar3_StampBackground(actor);
				handled = 1;
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, Hangar3_user_SoundAction);
				break;
		}
	}
	return handled;
}

// FUNCTION: XW 0x44CA40
int16_t Hangar3_StampBackground(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_hangar3BackgroundHandle);
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						HANGAR3_BACKGROUND_WIDTH, HANGAR3_BACKGROUND_HEIGHT, 0);
	if (actor->draw != NULL) {
		xrect_Copy_Rect(&actorClip, &actor->frame);
		xcanvas_Clip_Rect_To_Canvas(&actorClip);
		xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
		drawResult = actor->draw(actor, &canvasBounds, &actorClip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_hangar3BackgroundHandle);
	return drawResult;
}

// FUNCTION: XW 0x44CB20
void Hangar3_user_SoundAction(Actor* actor, int unusedTime) {
	int16_t action = actor->var2;
	(void)unusedTime;
	if (action != 0)
		Hangar3_HandleSoundAction(action);
}

// FUNCTION: XW 0x44CB40
void Hangar3_user_BeginDirtyFrame(Actor* unusedActor, int time) {
	(void)unusedActor;
	if (time == 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&g_hangar3PreviousDirtyRect);
	} else {
		xrect_Copy_Rect(&g_hangar3PreviousDirtyRect, &g_hangar3DirtyRect);
	}
	xrect_Clear_Rect(&g_hangar3DirtyRect);
}

// FUNCTION: XW 0x44CB90
int16_t Hangar3_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
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
	backgroundPixels = xmemhdl_Lock_Handle(g_hangar3BackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_hangar3PreviousDirtyRect,
								  g_hangar3PreviousDirtyRect.left, g_hangar3PreviousDirtyRect.top,
								  HANGAR3_BACKGROUND_WIDTH, HANGAR3_BACKGROUND_HEIGHT);
	xmemhdl_Unlock_Handle(g_hangar3BackgroundHandle);
	return 1;
}

// FUNCTION: XW 0x44CBF0
void Hangar3_user_DirtyBounds(Actor* actor, int unusedTime) {
	Rect actorRect;
	Rect actorFrame;
	(void)unusedTime;
	if (xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &actorRect);
		xactor_Get_Actor_Frame(actor, &actorFrame);
		xcanvas_Clip_Rect_To_Canvas(&actorRect);
		xrect_Clip_Rect(&actorRect, &actorFrame);
		xrect_Enclose_Rect(&g_hangar3DirtyRect, &actorRect);
	}
}

// FUNCTION: XW 0x44CC60
void Hangar3_user_Erase(Actor* actor, int unusedTime) {
	Rect frame;
	(void)unusedTime;
	if (g_hangar3Film->cur_cel == g_hangar3Film->cels) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		actor->var1 = 1;
	} else {
		xrect_Copy_Rect(&frame, &g_hangar3PreviousDirtyRect);
		xrect_Enclose_Rect(&frame, &g_hangar3DirtyRect);
		actor->var1 = 0;
	}
	if (!xrect_Empty_Rect(&frame)) {
		xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
	}
}

// FUNCTION: XW 0x44D9A0
void Hangar3_OpenMusic(ResFile* unusedResourceFile, Film* film) {
	unsigned int startBeat = 0;
	const char* musicFilename;
	const char* musicName;
	const char* hangarName = "hangar";
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled()) {
		switch (shellext_Get_Cur_Scene()) {
			case XW_SCENE_TOUR_LAUNCH_BWING:
			case XW_SCENE_TOUR_LAUNCH_XWING:
			case XW_SCENE_TOUR_LAUNCH_YWING:
			case XW_SCENE_TOUR_LAUNCH_AWING:
				musicFilename = "hr2music.lfd";
				musicName = "launch";
				startBeat = HANGAR3_LAUNCH_START_BEAT;
				break;
			case XW_SCENE_TOUR_RETURN_INDEPENDENCE:
				musicFilename = "pnmusic.lfd";
				musicName = "plans";
				break;
			case XW_SCENE_TOUR_RETURN_AWING:
			case XW_SCENE_TOUR_RETURN_XWING:
			case XW_SCENE_TOUR_RETURN_YWING:
			case XW_SCENE_TOUR_RETURN_BWING:
				musicFilename = "ldmusic.lfd";
				musicName = "swirls";
				break;
			default:
				musicFilename = "hr1music.lfd";
				musicName = hangarName;
				startBeat = HANGAR3_MUSIC_START_BEAT;
				break;
		}
		g_hangar3MusicState.sound = xsound_Find_Gmid(musicName);
		if (!g_hangar3MusicState.sound) {
			ResFile* resource = xres_Open_Resource(musicFilename);
			g_hangar3MusicState.sound = xsound_Res_Music(resource, musicName);
			xres_Close_Resource(resource);
			soundext_Start_Resource_Sound(g_hangar3MusicState.sound);
			if (startBeat)
				soundext_ScanMidi(g_hangar3MusicState.sound, 0, startBeat, HANGAR3_MUSIC_START_TICK);
		}
		xsound_Set_Sound_Keep(g_hangar3MusicState.sound);
		g_hangar3MusicState.film = film;
		if (musicName == hangarName) {
			ResFile* resource = xres_Open_Resource("bl1music.lfd");
			Sound* transition = xsound_Res_Music(resource, "trofight");
			xres_Close_Resource(resource);
			soundext_ClearTriggers();
			soundext_SetTriggerContext(g_hangar3MusicState.sound, 1);
			soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_START, transition->id, 0, 0, 0, 0, 0);
			soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_SHARE_PARTS, g_hangar3MusicState.sound->id,
										 transition->id, 0, 0, 0, 0);
			soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_END, 0, 0, 0, 0, 0, 0);
			xsound_Set_Sound_Keep(transition);
		}
	}
}

// FUNCTION: XW 0x44DC10
void Hangar3_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0) {
		switch (shellext_Get_Cur_Scene()) {
			case XW_SCENE_TOUR_LAUNCH_BWING:
			case XW_SCENE_TOUR_LAUNCH_XWING:
			case XW_SCENE_TOUR_LAUNCH_YWING:
			case XW_SCENE_TOUR_LAUNCH_AWING: {
				ResFile* transitionResource;
				Sound* transitionSound;
				g_hangar3MusicState.sound = xsound_Find_Gmid("launch");
				transitionResource = xres_Open_Resource("hr2music.lfd");
				transitionSound = xsound_Res_Music(transitionResource, "tran");
				xres_Close_Resource(transitionResource);
				if (transitionSound != NULL) {
					soundext_ClearTriggers();
					if (g_hangar3MusicState.sound != NULL) {
						int transitionTick;
						int instanceCount;
						soundext_SetTriggerContext(g_hangar3MusicState.sound, HANGAR3_TRANSITION_MARKER);
						soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_START, transitionSound->id, 0, 0, 0, 0,
													 0);
						soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_SHARE_PARTS,
													 g_hangar3MusicState.sound->id, transitionSound->id, 0, 0,
													 0, 0);
						soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_FADE_VOLUME, transitionSound->id,
													 HANGAR3_TRANSITION_VOLUME, HANGAR3_TRANSITION_DURATION,
													 0, 0, 0);
						soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_END, 0, 0, 0, 0, 0, 0);
						transitionTick =
							soundext_GetMusicParam(g_hangar3MusicState.sound, XW_SOUND_QUERY_TICK, 0);
						if (transitionTick > HANGAR3_TRANSITION_TICK_LIMIT)
							transitionTick = HANGAR3_TRANSITION_TICK_LIMIT;
						soundext_JumpMidi(g_hangar3MusicState.sound, HANGAR3_TRANSITION_GROUP,
										  HANGAR3_TRANSITION_BEAT, transitionTick);
						instanceCount = soundext_Count_Resource_Instances(transitionSound);
						if (instanceCount == HANGAR3_INSTANCE_ERROR || g_hangar3MusicState.sound == NULL)
							return;
					}
					if (shellext_Is_Sudden_Scene_End() != 0) {
						soundext_ClearTriggers();
						soundext_Start_Resource_Sound(transitionSound);
						soundext_FadeVolume(transitionSound, HANGAR3_TRANSITION_VOLUME,
											HANGAR3_TRANSITION_DURATION);
					}
#ifdef XW_MODERN
					XwHangar3_WaitForTransition(transitionSound);
#else
					while (soundext_Count_Resource_Instances(transitionSound) != 1) {
					}
#endif
				}
				break;
			}
			case XW_SCENE_TOUR_RETURN_INDEPENDENCE: {
				Sound* plansSound = xsound_Find_Gmid("plans");
				g_hangar3MusicState.sound = plansSound;
				if (plansSound != NULL)
					soundext_FadeVolume(plansSound, 0, HANGAR3_TRANSITION_DURATION);
				break;
			}
			case XW_SCENE_TOUR_RETURN_AWING:
			case XW_SCENE_TOUR_RETURN_XWING:
			case XW_SCENE_TOUR_RETURN_YWING:
			case XW_SCENE_TOUR_RETURN_BWING:
				if (g_hangar3MusicState.sound != NULL)
					soundext_FadeVolume(g_hangar3MusicState.sound, 0, HANGAR3_RETURN_FADE_DURATION);
				break;
			default:
				return;
		}
	}
}

// FUNCTION: XW 0x44DEE0
void Hangar3_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	if (ShellPreferences_GetSfxEnabled()) {
		soundext_LoadSfx(XW_SHELL_SFX_FLYBY_8, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_FLYBY_2, 0, NULL, 0, 1);
	}
	ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x44DF20
void Hangar3_HandleSoundAction(int16_t action) {
	if (ShellPreferences_GetSfxEnabled()) {
		switch (action) {
			case HANGAR3_ACTION_FLYBY_8:
				soundext_Play_SFX(XW_SHELL_SFX_FLYBY_8);
				break;
			case HANGAR3_ACTION_FLYBY_2:
				soundext_Play_SFX(XW_SHELL_SFX_FLYBY_2);
				break;
			case HANGAR3_ACTION_FLYBY_8_MODE1:
				soundext_PlaySfxMode1(XW_SHELL_SFX_FLYBY_8);
				break;
		}
	}
}
