#include "xw/frontend/scenes/med640.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/med640_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/memhdl.h>
#include <landru/timer.h>
#include <landru/view.h>
#include <landru/viewadd.h>

// GLOBAL: XW 0x4D5F10
const int8_t g_med640MotionXTable[MED640_MOTION_SAMPLE_COUNT] = { 0, 1, 2, 3, 4, 5, 5, 5,
																  4, 3, 4, 5, 4, 3, 2, 1 };

// GLOBAL: XW 0x4D5F20
const int8_t g_med640MotionYTable[MED640_MOTION_SAMPLE_COUNT] = { -1, 0,  0,  -1, -2, -3, -4, -5,
																  -6, -5, -4, -3, -4, -3, -2, -1 };

// GLOBAL: XW 0x4F87E4
XwSceneMusicHandles g_med640MusicState = { NULL, NULL };

// GLOBAL: XW 0x4F87F0
int16_t g_med640MotionX = 0;

// GLOBAL: XW 0x4F87F4
int16_t g_med640MotionY = 0;

// GLOBAL: XW 0x4F87F8
Actor* g_med640BackgroundActor = NULL;

// GLOBAL: XW 0x4F8800
Rect g_med640PreviousDirtyRect = { 0 };

// GLOBAL: XW 0x4F8808
Actor* g_med640CloseActor = NULL;

// GLOBAL: XW 0x4F8810
Rect g_med640CurrentDirtyRect = { 0 };

// GLOBAL: XW 0x4F8818
Film* g_med640Film = NULL;

// GLOBAL: XW 0x4F881C
LandruHandle g_med640BackgroundHandle = LANDRU_NULL_HANDLE;

// FUNCTION: XW 0x458C10
void Med640_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_med640MusicState.film = sceneFilm;
		g_med640MusicState.sound = xsound_Find_Gmid("rescue");
		if (g_med640MusicState.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource("rsmusic.lfd");
			g_med640MusicState.sound = xsound_Res_Music(musicResource, "rescue");
			soundext_Start_Resource_Sound(g_med640MusicState.sound);
			xres_Close_Resource(musicResource);
			soundext_ScanMidi(g_med640MusicState.sound, 0, MED640_MUSIC_START_BEAT, 0);
		}
		xsound_Set_Sound_Keep(g_med640MusicState.sound);
		xsound_Set_Sound_User_Function(g_med640MusicState.sound, Cutscene_IgnoreSoundEvent);
	}
}

// FUNCTION: XW 0x458CB0
void Med640_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0) {
		Sound* music = xsound_Find_Gmid("rescue");
		g_med640MusicState.sound = music;
		if (music != NULL) {
			/* Resource pointers are outside the numeric flight-sound ID range. */
			soundext_SetPriority(0, 0);
			soundext_FadeVolume(g_med640MusicState.sound, 0, MED640_MUSIC_FADE_DURATION);
		}
	}
}

// FUNCTION: XW 0x458D10
void Med640_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	if (ShellPreferences_GetSfxEnabled() != 0) {
		soundext_LoadSfx(XW_SHELL_SFX_BEEP_4, 0, NULL, 0, 0);
		soundext_LoadSfx(XW_SHELL_SFX_BEEP_5, 0, NULL, 0, 0);
		soundext_LoadSfx(XW_SHELL_SFX_BEEP_6, 0, NULL, 0, 0);
		soundext_LoadSfx(XW_SHELL_SFX_DROID, 0, NULL, 0, 0);
		soundext_LoadSfx(XW_SHELL_SFX_DROID_4, 0, NULL, 0, 0);
	}
	ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x458D80
void Med640_PlaySoundCue(int16_t cue) {
	if (ShellPreferences_GetSfxEnabled() != 0) {
		switch (cue) {
			case MED640_CUE_BEEP_4:
				soundext_Play_SFX(XW_SHELL_SFX_BEEP_4);
				break;
			case MED640_CUE_BEEP_5:
				soundext_Play_SFX(XW_SHELL_SFX_BEEP_5);
				break;
			case MED640_CUE_BEEP_6:
				soundext_Play_SFX(XW_SHELL_SFX_BEEP_6);
				break;
			case MED640_CUE_DROID:
				soundext_Play_SFX(XW_SHELL_SFX_DROID);
				break;
			case MED640_CUE_DROID_4:
				soundext_Play_SFX(XW_SHELL_SFX_DROID_4);
				break;
		}
	}
}

// FUNCTION: XW 0x458DF0
XwShellSceneResult Med640_Play(struct XwShellContext* shell) {
	ResFile* sceneResource = xres_Open_Resource("med640.lfd");
	Rect frame;
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	g_med640BackgroundHandle = xmemhdl_Alloc_Clear_Handle(MED640_BACKGROUND_BYTES, LANDRU_MEMORY_RESOURCE);
	g_med640Film = xfilm_Res_Callback_Film(sceneResource, "med640", &frame, 0, 0, 0, Med640_film_Callback);
	xfilm_Set_Film_Def_Palette(g_med640Film, shell->standardPalette);
	g_med640BackgroundActor =
		xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, MED640_BACKGROUND_Z);
	xactor_Set_Actor_User_Function(g_med640BackgroundActor, Med640_user_Background);
	xactor_Set_Actor_Draw_Function(g_med640BackgroundActor, Med640_draw_Background);
	g_med640CloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, MED640_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_med640CloseActor, Med640_user_Close);
	xactor_Set_Actor_Draw_Function(g_med640CloseActor, XwCutscene_DrawCloseOnRefresh);
	xview_Set_View_Update_Function(Med640_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	xtimer_Set_FrameRatePreset(LANDRU_FRAME_PRESET_2);
	Med640_OpenMusic(sceneResource, g_med640Film);
	Med640_LoadSoundEffects(sceneResource, g_med640Film);
#ifdef XW_MODERN
	XwMed640_RunView(sceneResource);
#else
	j_xviewadd_Handle_View();
	Med640_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xtimer_Set_FrameRatePreset(LANDRU_FRAME_PRESET_2);
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xmemhdl_Free_Handle(g_med640BackgroundHandle);
	xres_Close_Resource(sceneResource);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x458FB0
void Med640_end_View(int time) {
	int16_t exitScene;
	int16_t motionIndex;

	if (shellext_Check_Scene_Exit(&exitScene, shipext_Get_Pending_Tour_Cutscene(),
								  shipext_Get_Pending_Tour_Cutscene(),
								  g_med640Film->cur_cel == g_med640Film->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
	motionIndex = (time >> MED640_MOTION_TIME_SHIFT) & MED640_MOTION_PHASE_MASK;
	if (motionIndex >= MED640_MOTION_SAMPLE_COUNT) {
		motionIndex = MED640_MOTION_PHASE_MASK - motionIndex;
	}
	g_med640MotionX = g_med640MotionXTable[motionIndex];
	g_med640MotionY = g_med640MotionYTable[motionIndex];
}

// FUNCTION: XW 0x459030
int16_t Med640_film_Callback(Film* film, FilmObject* object) {
	int16_t consumeObject = 0;
	Actor* actor;
	if (object->id == FTC_ACTOR) {
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		if (actor->var1 == CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND) {
			Med640_film_Actor_To_Background(actor);
			consumeObject = 1;
		} else if (actor->var1 == CUTSCENE_ACTOR_ROLE_SOUND) {
			xactor_Set_Actor_User_Function(actor, Med640_user_SoundCue);
		} else {
			if (actor->var2 != 0) {
				xactor_Set_Actor_Flag1(actor);
			}
			if (actor->var1 == CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS) {
				xactor_Set_Actor_User_Function(actor, Med640_user_DirtyActor);
			}
		}
	}
	return consumeObject;
}

// FUNCTION: XW 0x4590C0
int16_t Med640_film_Actor_To_Background(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_med640BackgroundHandle);
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						MED640_BACKGROUND_WIDTH, MED640_BACKGROUND_HEIGHT, 0);
	if (actor->draw != NULL) {
		xrect_Copy_Rect(&actorClip, &actor->frame);
		xcanvas_Clip_Rect_To_Canvas(&actorClip);
		xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
		drawResult = actor->draw(actor, &canvasBounds, &actorClip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_med640BackgroundHandle);
	return drawResult;
}

// FUNCTION: XW 0x4591A0
void Med640_user_SoundCue(Actor* actor, int unusedTime) {
	int16_t cue = actor->var2;
	(void)unusedTime;
	if (cue != 0) {
		Med640_PlaySoundCue(cue);
	}
}

// FUNCTION: XW 0x4591C0
void Med640_user_Background(Actor* unusedActor, int time) {
	(void)unusedActor;
	if (time == 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&g_med640PreviousDirtyRect);
	} else {
		xrect_Copy_Rect(&g_med640PreviousDirtyRect, &g_med640CurrentDirtyRect);
	}
	xrect_Clear_Rect(&g_med640CurrentDirtyRect);
}

// FUNCTION: XW 0x459210
int16_t Med640_draw_Background(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	const uint8_t* backgroundPixels;
	(void)actor;
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (refresh == 0) {
		return 0;
	}
	backgroundPixels = xmemhdl_Lock_Handle(g_med640BackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_med640PreviousDirtyRect,
								  g_med640PreviousDirtyRect.left, g_med640PreviousDirtyRect.top,
								  MED640_BACKGROUND_WIDTH, MED640_BACKGROUND_HEIGHT);
	xmemhdl_Unlock_Handle(g_med640BackgroundHandle);
	return 1;
}

// FUNCTION: XW 0x459270
void Med640_user_DirtyActor(Actor* actor, int time) {
	Rect visibleBounds;
	Rect actorFrame;
	if ((uint16_t)xactor_Is_Actor_Flag1(actor) != 0) {
		if (time == 0) {
			actor->var1 = actor->x;
			actor->var2 = actor->y;
		} else {
			actor->x = actor->var1 + g_med640MotionX;
			actor->y = actor->var2 + g_med640MotionY;
		}
	}
	if ((uint16_t)xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &visibleBounds);
		xactor_Get_Actor_Frame(actor, &actorFrame);
		xcanvas_Clip_Rect_To_Canvas(&visibleBounds);
		xrect_Clip_Rect(&visibleBounds, &actorFrame);
		xrect_Enclose_Rect(&g_med640CurrentDirtyRect, &visibleBounds);
	}
}

// FUNCTION: XW 0x459320
void Med640_user_Close(Actor* actor, int unusedTime) {
	Rect frame;
	(void)unusedTime;
	if (g_med640Film->cur_cel == g_med640Film->cels) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		actor->var1 = 1;
	} else {
		xrect_Copy_Rect(&frame, &g_med640PreviousDirtyRect);
		xrect_Enclose_Rect(&frame, &g_med640CurrentDirtyRect);
		actor->var1 = 0;
	}
	if (!xrect_Empty_Rect(&frame)) {
		xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
	}
}
