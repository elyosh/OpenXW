#include "xw/frontend/scenes/assault.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#include "xw/util/landru_display.h"
#include "xw/util/shared.h"
#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/assault_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/memhdl.h>
#include <landru/view.h>
#include <landru/viewadd.h>

// GLOBAL: XW 0x4F4A64
Sound* g_assaultMusic = NULL;

// GLOBAL: XW 0x4F4A68
Film* g_assaultMusicFilm = NULL;

// GLOBAL: XW 0x4F4A70
Actor* g_assaultBackgroundActor = NULL;

// GLOBAL: XW 0x4F4A78
Rect g_assaultPreviousDirtyRect = { 0 };

// GLOBAL: XW 0x4F4A80
Actor* g_assaultCloseActor = NULL;

// GLOBAL: XW 0x4F4A84
Film* g_assaultFilm = NULL;

// GLOBAL: XW 0x4F4A88
Rect g_assaultCurrentDirtyRect = { 0 };

// GLOBAL: XW 0x4F4A90
LandruHandle g_assaultBackgroundHandle = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4F4A94
Sound* g_assaultSpeech = NULL;

// FUNCTION: XW 0x4310F0
void Assault_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	unsigned int startBeat = 0;
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		int scene;
		const char* musicName;
		const char* musicFilename;
		g_assaultMusicFilm = sceneFilm;
		scene = shellext_Get_Cur_Scene();
		switch (scene) {
			case XW_SCENE_EMPIRE_ASSAULT_1:
				musicName = "inattack";
				musicFilename = "inmusic.lfd";
				startBeat = ASSAULT_MUSIC_FIRST_START_BEAT;
				break;
			case XW_SCENE_EMPIRE_ASSAULT_2:
				musicName = "inattack";
				musicFilename = "inmusic.lfd";
				startBeat = ASSAULT_MUSIC_SECOND_START_BEAT;
				break;
			case XW_SCENE_EMPIRE_ASSAULT_3:
				g_assaultMusic = xsound_Find_Gmid("inattack");
				soundext_FadeVolume(g_assaultMusic, 0, ASSAULT_MUSIC_ATTACK_FADE_DURATION);
				musicName = "darth";
				musicFilename = "asmusic.lfd";
				break;
			default:
				musicName = "inattack";
				musicFilename = "inmusic.lfd";
				break;
		}
		g_assaultMusic = xsound_Find_Gmid(musicName);
		if (g_assaultMusic == NULL) {
			ResFile* musicResourceFile = xres_Open_Resource(musicFilename);
			g_assaultMusic = xsound_Res_Music(musicResourceFile, musicName);
			xres_Close_Resource(musicResourceFile);
			soundext_Start_Resource_Sound(g_assaultMusic);
			if (scene != XW_SCENE_EMPIRE_ASSAULT_3)
				soundext_ScanMidi(g_assaultMusic, 0, startBeat, ASSAULT_MUSIC_START_TICK);
		}
		xsound_Set_Sound_Keep(g_assaultMusic);
		xsound_Set_Sound_User_Function(g_assaultMusic, Assault_MusicCallback);
	}
}

// FUNCTION: XW 0x431210
void Assault_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0) {
		int scene = shellext_Get_Cur_Scene();
		if (scene < XW_SCENE_EMPIRE_ASSAULT_1 || scene > XW_SCENE_EMPIRE_ASSAULT_2) {
			Sound* music;
			if (shellext_Get_Cur_Scene() == XW_SCENE_EMPIRE_ASSAULT_3) {
				music = xsound_Find_Gmid("darth");
			} else {
				music = xsound_Find_Gmid("inattack");
			}
			g_assaultMusic = music;
			if (music != NULL) {
				soundext_FadeVolume(music, 0, ASSAULT_MUSIC_FADE_DURATION);
			}
		}
	}
}

// FUNCTION: XW 0x431270
void Assault_MusicCallback(Sound* unusedSound, int unusedTime) {
	uint16_t cel = g_assaultMusicFilm->cur_cel;
	int scene = shellext_Get_Cur_Scene();
	(void)unusedSound;
	(void)unusedTime;
	if (cel == ASSAULT_MUSIC_FIRST_CUE_CEL) {
		if (scene == XW_SCENE_EMPIRE_ASSAULT_1) {
			soundext_SetHook(g_assaultMusic, 0, ASSAULT_MUSIC_SCENE_1_FIRST_CONTROL, 0);
		} else if (scene == XW_SCENE_EMPIRE_ASSAULT_2) {
			soundext_SetHook(g_assaultMusic, 0, ASSAULT_MUSIC_SCENE_2_CONTROL, 0);
		}
	} else if (cel == ASSAULT_MUSIC_SECOND_CUE_CEL && scene == XW_SCENE_EMPIRE_ASSAULT_1) {
		soundext_SetHook(g_assaultMusic, 0, ASSAULT_MUSIC_SCENE_1_SECOND_CONTROL, 0);
	}
}

// FUNCTION: XW 0x4312F0
XwShellSceneResult Assault_Play(struct XwShellContext* shell) {
	ResFile* resourceFile;
	Rect frame;
	LandruDisplay_SetLowResolutionMode(1);
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_EMPIRE_ASSAULT_1:
			xfade_AddTimedText("Star Destroyer fleet orbiting a", ASSAULT_INTRO_CAPTION_START,
							   ASSAULT_INTRO_CAPTION_END, ASSAULT_CAPTION_FONT, ASSAULT_ORBIT_CAPTION_X,
							   ASSAULT_ORBIT_FIRST_Y, ASSAULT_ORBIT_CAPTION_COLOR);
			xfade_AddTimedText("Rebel base on the planet Orion IV.", ASSAULT_INTRO_CAPTION_START,
							   ASSAULT_INTRO_CAPTION_END, ASSAULT_CAPTION_FONT, ASSAULT_ORBIT_CAPTION_X,
							   ASSAULT_ORBIT_SECOND_Y, ASSAULT_ORBIT_CAPTION_COLOR);
			break;
		case XW_SCENE_EMPIRE_ASSAULT_2:
			xfade_AddTimedText("AT-AT Walkers advance on the Rebel base.", ASSAULT_INTRO_CAPTION_START,
							   ASSAULT_INTRO_CAPTION_END, ASSAULT_CAPTION_FONT, ASSAULT_WALKER_CAPTION_X,
							   ASSAULT_GROUND_CAPTION_Y, ASSAULT_WALKER_CAPTION_COLOR);
			break;
		case XW_SCENE_EMPIRE_ASSAULT_3:
			xfade_AddTimedText("Planet has been secured.", ASSAULT_SECURED_CAPTION_START,
							   ASSAULT_SECURED_CAPTION_END, ASSAULT_CAPTION_FONT, ASSAULT_SECURED_CAPTION_X,
							   ASSAULT_GROUND_CAPTION_Y, ASSAULT_SECURED_CAPTION_COLOR);
			break;
	}
	resourceFile = xres_Open_Resource("assault.lfd");
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	g_assaultBackgroundHandle = xmemhdl_Alloc_Clear_Handle(
		ASSAULT_BACKGROUND_WIDTH * ASSAULT_BACKGROUND_HEIGHT, LANDRU_MEMORY_RESOURCE);
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_EMPIRE_ASSAULT_1:
			g_assaultFilm =
				xfilm_Res_Callback_Film(resourceFile, "ass1_f", &frame, 0, 0, 0, Assault_film_Callback);
			break;
		case XW_SCENE_EMPIRE_ASSAULT_2:
			if (Shared_ReturnZero() != 0)
				g_assaultFilm =
					xfilm_Res_Callback_Film(resourceFile, "ass2_l", &frame, 0, 0, 0, Assault_film_Callback);
			else
				g_assaultFilm =
					xfilm_Res_Callback_Film(resourceFile, "ass2_f", &frame, 0, 0, 0, Assault_film_Callback);
			break;
		case XW_SCENE_EMPIRE_ASSAULT_3:
			g_assaultFilm =
				xfilm_Res_Callback_Film(resourceFile, "ass3_f", &frame, 0, 0, 0, Assault_film_Callback);
			break;
	}
	xfilm_Set_Film_Def_Palette(g_assaultFilm, shell->standardPalette);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	g_assaultBackgroundActor =
		xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, ASSAULT_BACKGROUND_Z);
	xactor_Set_Actor_User_Function(g_assaultBackgroundActor, Assault_user_Background);
	xactor_Set_Actor_Draw_Function(g_assaultBackgroundActor, Assault_draw_Background);
	g_assaultCloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, ASSAULT_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_assaultCloseActor, Assault_user_Close);
	xactor_Set_Actor_Draw_Function(g_assaultCloseActor, XwCutscene_DrawCloseOnRefresh);
	xview_Set_View_Update_Function(Assault_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	Assault_OpenMusic(resourceFile, g_assaultFilm);
	Assault_OpenSounds(resourceFile, g_assaultFilm);
#ifdef XW_MODERN
	XwAssault_RunView(resourceFile);
#else
	j_xviewadd_Handle_View();
	Assault_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xmemhdl_Free_Handle(g_assaultBackgroundHandle);
	xres_Close_Resource(resourceFile);
	LandruDisplay_SetLowResolutionMode(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x431580
void Assault_end_View(int unusedTime) {
	int16_t exitScene;
	int16_t nextScene;

	(void)unusedTime;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_EMPIRE_ASSAULT_1:
			nextScene = XW_SCENE_EMPIRE_ASSAULT_2;
			break;
		case XW_SCENE_EMPIRE_ASSAULT_2:
			nextScene = XW_SCENE_EMPIRE_ASSAULT_3;
			break;
		case XW_SCENE_EMPIRE_ASSAULT_3:
			nextScene = shipext_Get_Pending_Medal_Scene();
			break;
#ifdef XW_MODERN
		default:
			nextScene = shipext_Get_Pending_Medal_Scene();
			break;
#endif
	}
	if (shellext_Check_Scene_Exit(&exitScene, nextScene, shipext_Get_Pending_Medal_Scene(),
								  g_assaultFilm->cur_cel == g_assaultFilm->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x4315F0
int16_t Assault_film_Callback(Film* film, FilmObject* object) {
	int16_t consumeObject = 0;
	Actor* actor;
	if (object->id == FTC_ACTOR) {
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = (Actor*)object->object;
		if (actor->var1 == CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND) {
			Assault_ActorToBackground(actor);
			consumeObject = 1;
		} else if (actor->var1 == CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS) {
			xactor_Set_Actor_User_Function(actor, Assault_user_Actor);
		}
		if (actor->var1 == CUTSCENE_ACTOR_ROLE_SOUND) {
			xactor_Set_Actor_User_Function(actor, Assault_user_Sound);
		}
	}
	return consumeObject;
}

// FUNCTION: XW 0x431660
int16_t Assault_ActorToBackground(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_assaultBackgroundHandle);
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						ASSAULT_BACKGROUND_WIDTH, ASSAULT_BACKGROUND_HEIGHT, 0);
	if (actor->draw != NULL) {
		xrect_Copy_Rect(&actorClip, &actor->frame);
		xcanvas_Clip_Rect_To_Canvas(&actorClip);
		xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
		drawResult = actor->draw(actor, &canvasBounds, &actorClip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_assaultBackgroundHandle);
	return drawResult;
}

// FUNCTION: XW 0x431740
void Assault_user_Sound(Actor* actor, int unusedTime) {
	int16_t cue = actor->var2;
	(void)unusedTime;
	if (cue != 0)
		Assault_PlaySoundCue(cue);
}

// FUNCTION: XW 0x431760
void Assault_user_Background(Actor* unusedActor, int time) {
	(void)unusedActor;
	if (time == 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&g_assaultPreviousDirtyRect);
	} else {
		xrect_Copy_Rect(&g_assaultPreviousDirtyRect, &g_assaultCurrentDirtyRect);
	}
	xrect_Clear_Rect(&g_assaultCurrentDirtyRect);
}

// FUNCTION: XW 0x4317B0
int16_t Assault_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
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
	backgroundPixels = xmemhdl_Lock_Handle(g_assaultBackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_assaultPreviousDirtyRect,
								  g_assaultPreviousDirtyRect.left, g_assaultPreviousDirtyRect.top,
								  ASSAULT_BACKGROUND_WIDTH, ASSAULT_BACKGROUND_HEIGHT);
	xmemhdl_Unlock_Handle(g_assaultBackgroundHandle);
	return 1;
}

// FUNCTION: XW 0x431810
void Assault_user_Actor(Actor* actor, int time) {
	Rect dirtyBounds;
	Rect actorFrame;
	int16_t action;
	if (xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &dirtyBounds);
		xactor_Get_Actor_Frame(actor, &actorFrame);
		xcanvas_Clip_Rect_To_Canvas(&dirtyBounds);
		xrect_Clip_Rect(&dirtyBounds, &actorFrame);
		xrect_Enclose_Rect(&g_assaultCurrentDirtyRect, &dirtyBounds);
	}
	action = actor->var2;
	switch (action) {
		case ASSAULT_ACTOR_REQUEST_REFRESH:
			g_assaultFilm->var1 = 1;
			break;
		case ASSAULT_ACTOR_MOVE_ODD_TICKS:
			if ((time & 1) != 0)
				xactor_Set_Actor_Pos(actor, actor->x - 1, actor->y + 1, 0, 0);
			break;
		case ASSAULT_ACTOR_MOVE_EVEN_TICKS:
			if ((time & 1) == 0)
				xactor_Set_Actor_Pos(actor, actor->x - 1, actor->y + 1, 0, 0);
			break;
		case ASSAULT_ACTOR_HALF_SCALE:
			if (time == 0)
				xactor_Set_Actor_Scale(actor, ASSAULT_ACTOR_HALF_SCALE_VALUE, ASSAULT_ACTOR_HALF_SCALE_VALUE);
			break;
	}
}

// FUNCTION: XW 0x431920
void Assault_user_Close(Actor* actor, int unusedTime) {
	Rect frame;
	(void)unusedTime;
	if (g_assaultFilm->cur_cel == g_assaultFilm->cels || g_assaultFilm->var1 != 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		g_assaultFilm->var1 = 0;
		actor->var1 = 1;
	} else {
		xrect_Copy_Rect(&frame, &g_assaultPreviousDirtyRect);
		xrect_Enclose_Rect(&frame, &g_assaultCurrentDirtyRect);
		actor->var1 = 0;
	}
	if (!xrect_Empty_Rect(&frame)) {
		xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
	}
}

// FUNCTION: XW 0x4319D0
void Assault_OpenSounds(ResFile* unusedResourceFile, Film* unusedFilm) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_EMPIRE_ASSAULT_1:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_LoadSfx(XW_SHELL_SFX_GUN_BLAST, 0, NULL, 0, 1);
			break;
		case XW_SCENE_EMPIRE_ASSAULT_2:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_LoadSfx(XW_SHELL_SFX_ROBOT_WALK, 0, NULL, 0, 0);
				soundext_LoadSfx(XW_SHELL_SFX_FLYBY_6A, 0, NULL, 0, 0);
				soundext_LoadSfx(XW_SHELL_SFX_FLYBY_6, 0, NULL, 0, 0);
				soundext_LoadSfx(XW_SHELL_SFX_GUN_BLAST, 0, NULL, 0, 1);
			}
			break;
		case XW_SCENE_EMPIRE_ASSAULT_3:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_LoadSfx(XW_SHELL_SFX_EXPLOSION_1, 0, NULL, 0, 1);
			if (ShellPreferences_GetSfxEnabled() != 0)
				g_assaultSpeech = soundext_LoadSpeech(XW_SHELL_SPEECH_PLANET, 0, NULL, 0);
			break;
	}
}

// FUNCTION: XW 0x431AA0
void Assault_PlaySoundCue(int16_t cue) {
	switch (cue) {
		case ASSAULT_CUE_ROBOT_WALK:
			if (ShellPreferences_GetSfxEnabled())
				soundext_Play_SFX(XW_SHELL_SFX_ROBOT_WALK);
			break;
		case ASSAULT_CUE_FLYBY_6A:
			if (ShellPreferences_GetSfxEnabled())
				soundext_Play_SFX(XW_SHELL_SFX_FLYBY_6A);
			break;
		case ASSAULT_CUE_GUN_BLAST:
			if (ShellPreferences_GetSfxEnabled())
				soundext_Play_SFX(XW_SHELL_SFX_GUN_BLAST);
			break;
		case ASSAULT_CUE_EXPLOSION:
			if (ShellPreferences_GetSfxEnabled())
				soundext_Play_SFX(XW_SHELL_SFX_EXPLOSION_1);
			break;
		case ASSAULT_CUE_SPEECH:
			if (ShellPreferences_GetSfxEnabled())
				soundext_Start_Resource_SFX(g_assaultSpeech);
			break;
		case ASSAULT_CUE_FLYBY_6:
			if (ShellPreferences_GetSfxEnabled())
				soundext_Play_SFX(XW_SHELL_SFX_FLYBY_6);
			break;
	}
}
