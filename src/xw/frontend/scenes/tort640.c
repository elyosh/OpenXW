#include "xw/frontend/scenes/tort640.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/b1b640.h"
#include "xw/frontend/scenes/cutscene.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/tort640_task.h"
#endif
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/font.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/view.h>
#include <landru/viewadd.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4D78B0
const char* g_tort640ResourceNames[TORT640_RESOURCE_COUNT] = { "tort640.lfd", "tort640", "tort640" };

// GLOBAL: XW 0x4D78E8
char g_tort640CaptionPrefix[TORT640_CAPTION_PREFIX_CAPACITY] = "Now we will discuss the position";

// GLOBAL: XW 0x4FAD5C
XwSceneMusicHandles g_tort640MusicState = { NULL, NULL };

// GLOBAL: XW 0x4FAD68
Actor* g_tort640BackgroundActor = NULL;

// GLOBAL: XW 0x4FAD6C
int16_t g_tort640VerticalOffset = 0;

// GLOBAL: XW 0x4FAD70
Film* g_tort640Film = NULL;

// GLOBAL: XW 0x4FAD78
Rect g_tort640PreviousDirtyRect = { 0 };

// GLOBAL: XW 0x4FAD80
Actor* g_tort640CloseActor = NULL;

// GLOBAL: XW 0x4FAD84
Actor* g_tort640ScrollActor = NULL;

// GLOBAL: XW 0x4FAD88
Rect g_tort640CurrentDirtyRect = { 0 };

// GLOBAL: XW 0x4FAD90
LandruHandle g_tort640BackgroundHandle = 0;

// GLOBAL: XW 0x4FAD94
Sound* g_tort640Speech = NULL;

// FUNCTION: XW 0x462900
void Tort640_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_tort640MusicState.film = sceneFilm;
		g_tort640MusicState.sound = xsound_Find_Gmid("torture");
		if (g_tort640MusicState.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource("rsmusic.lfd");
			g_tort640MusicState.sound = xsound_Res_Music(musicResource, "torture");
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_tort640MusicState.sound);
			soundext_ScanMidi(g_tort640MusicState.sound, 0, TORT640_MUSIC_START_BEAT, 0);
		}
		xsound_Set_Sound_Keep(g_tort640MusicState.sound);
		xsound_Set_Sound_User_Function(g_tort640MusicState.sound, Tort640_user_Music);
	}
}

// FUNCTION: XW 0x4629A0
void Tort640_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0) {
		Sound* music = xsound_Find_Gmid("torture");
		g_tort640MusicState.sound = music;
		if (music != NULL) {
			/* Resource pointers are outside the numeric flight-sound ID range. */
			soundext_SetPriority(0, 0);
			soundext_FadeVolume(g_tort640MusicState.sound, 0, TORT640_MUSIC_FADE_DURATION);
		}
	}
}

// FUNCTION: XW 0x4629F0
void Tort640_user_Music(Sound* unusedSound, int unusedTime) {
	(void)unusedSound;
	(void)unusedTime;
	switch (g_tort640MusicState.film->cur_cel) {
		case TORT640_MUSIC_QUIET_CEL:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_FadeVolume(g_tort640MusicState.sound, TORT640_MUSIC_QUIET_VOLUME,
									TORT640_MUSIC_CUE_FADE_DURATION);
			}
			break;
		case TORT640_MUSIC_LOUD_CEL:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_FadeVolume(g_tort640MusicState.sound, TORT640_MUSIC_LOUD_VOLUME,
									TORT640_MUSIC_CUE_FADE_DURATION);
			}
			break;
	}
}

// FUNCTION: XW 0x462A50
XwShellSceneResult Tort640_Play(struct XwShellContext* shell) {
	ResFile* resourceFile;
	Rect frame;
	char caption[TORT640_CAPTION_CAPACITY];
	if ((uint16_t)xio_Is_System_Slower_Than(TORT640_SPEED_THRESHOLD) != 0) {
		int16_t halfWidth;
		strcpy(caption, g_tort640CaptionPrefix);
		strcat(caption, g_b1b640CaptionSeparator);
		strcat(caption, "of the secret rebel base!");
		halfWidth = xfont_Get_String_Width_0(TORT640_CAPTION_FONT, caption) >> 1;
		xfade_AddTimedText(caption, TORT640_SLOW_CAPTION_START, TORT640_SLOW_CAPTION_END,
						   TORT640_CAPTION_FONT, TORT640_CAPTION_CENTER - halfWidth, TORT640_CAPTION_Y,
						   TORT640_CAPTION_COLOR);
	} else {
		int16_t halfWidth;
		strcpy(caption, g_tort640CaptionPrefix);
		strcat(caption, g_b1b640CaptionSeparator);
		strcat(caption, "of the secret rebel base!");
		halfWidth = xfont_Get_String_Width_0(TORT640_CAPTION_FONT, caption) >> 1;
		xfade_AddTimedText(caption, TORT640_FAST_CAPTION_START, TORT640_FAST_CAPTION_END,
						   TORT640_CAPTION_FONT, TORT640_CAPTION_CENTER - halfWidth, TORT640_CAPTION_Y,
						   TORT640_CAPTION_COLOR);
	}
	resourceFile = xres_Open_Resource(g_tort640ResourceNames[TORT640_RESOURCE_FILE]);
	xrect_Set_Rect(&frame, 0, 0, TORT640_BACKGROUND_WIDTH, TORT640_BACKGROUND_HEIGHT);
	g_tort640BackgroundHandle = xmemhdl_Alloc_Clear_Handle(
		TORT640_BACKGROUND_WIDTH * TORT640_BACKGROUND_HEIGHT, LANDRU_MEMORY_RESOURCE);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	if ((uint16_t)xio_Is_System_Slower_Than(TORT640_SPEED_THRESHOLD) != 0) {
		g_tort640ScrollActor = NULL;
		g_tort640VerticalOffset = 0;
		g_tort640Film = xfilm_Res_Callback_Film(resourceFile, g_tort640ResourceNames[TORT640_SLOW_FILM],
												&frame, 0, 0, 0, Tort640_film_Callback);
	} else {
		g_tort640VerticalOffset = TORT640_FAST_VERTICAL_OFFSET;
		g_tort640Film = xfilm_Res_Callback_Film(resourceFile, g_tort640ResourceNames[TORT640_FAST_FILM],
												&frame, 0, 0, 0, Tort640_film_Callback);
	}
	xfilm_Set_Film_Def_Palette(g_tort640Film, shell->standardPalette);
	g_tort640BackgroundActor =
		xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, TORT640_BACKGROUND_Z);
	xactor_Set_Actor_User_Function(g_tort640BackgroundActor, Tort640_user_Background);
	xactor_Set_Actor_Draw_Function(g_tort640BackgroundActor, Tort640_draw_Background);
	g_tort640CloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, TORT640_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_tort640CloseActor, Tort640_user_Close);
	xactor_Set_Actor_Draw_Function(g_tort640CloseActor, Cutscene_DrawConditionalErase);
	xview_Set_View_Update_Function(Tort640_end_View);
	Tort640_OpenMusic(resourceFile, g_tort640Film);
	Tort640_LoadSoundEffects(resourceFile, g_tort640Film);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
#ifdef XW_MODERN
	XwTort640_RunView(resourceFile);
#else
	j_xviewadd_Handle_View();
	Tort640_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xmemhdl_Free_Handle(g_tort640BackgroundHandle);
	xres_Close_Resource(resourceFile);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x462DB0
void Tort640_end_View(int time) {
	int16_t exitScene;
	if (ShellPreferences_GetSfxEnabled() != 0) {
		if (time % TORT640_BREATH_INTERVAL == 0) {
			Tort640_PlaySoundCue(TORT640_CUE_BREATH);
		}
	} else if (time == 0) {
		Tort640_PlaySoundCue(TORT640_CUE_BREATH);
	}
	if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_REGISTER_RETURN, XW_SCENE_REGISTER_RETURN,
								  g_tort640Film->cur_cel == g_tort640Film->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x462E20
int16_t Tort640_film_Callback(Film* film, FilmObject* object) {
	int16_t consumed = 0;
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = (Actor*)object->object;
		switch (actor->var1) {
			case CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS:
				xactor_Set_Actor_User_Function(actor, Tort640_user_DirtyActor);
				if (actor->var2 == TORT640_SCROLL_ACTOR) {
					g_tort640ScrollActor = actor;
				}
				break;
			case CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND:
				Tort640_film_Actor_To_Background(actor);
				consumed = 1;
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, Tort640_user_SoundCue);
				break;
		}
	}
	return consumed;
}

// FUNCTION: XW 0x462EA0
int16_t Tort640_film_Actor_To_Background(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_tort640BackgroundHandle);
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						TORT640_BACKGROUND_WIDTH, TORT640_BACKGROUND_HEIGHT, 0);
	if (actor->draw != NULL) {
		xrect_Copy_Rect(&actorClip, &actor->frame);
		xcanvas_Clip_Rect_To_Canvas(&actorClip);
		xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
		drawResult =
			actor->draw(actor, &canvasBounds, &actorClip, actor->x, actor->y - g_tort640VerticalOffset, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_tort640BackgroundHandle);
	return drawResult;
}

// FUNCTION: XW 0x462F90
void Tort640_user_SoundCue(Actor* actor, int unusedTime) {
	(void)unusedTime;
	if (actor->var2 != 0) {
		Tort640_PlaySoundCue(actor->var2);
	}
}

// FUNCTION: XW 0x462FB0
void Tort640_user_Background(Actor* unusedActor, int time) {
	(void)unusedActor;
	if (time == 0 || g_tort640ScrollActor->var2 == TORT640_SCROLL_FULL_REFRESH) {
		xcanvas_Get_Drawing_Canvas_Bounds(&g_tort640PreviousDirtyRect);
	} else {
		xrect_Copy_Rect(&g_tort640PreviousDirtyRect, &g_tort640CurrentDirtyRect);
	}
	xrect_Clear_Rect(&g_tort640CurrentDirtyRect);
}

// FUNCTION: XW 0x463000
int16_t Tort640_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
								int16_t unusedY, int16_t refresh) {
	Rect sourceRect;
	int16_t destinationX;
	int16_t destinationY;
	const uint8_t* backgroundPixels;
	(void)unusedActor;
	(void)unusedFrame;
	(void)unusedClip;
	(void)unusedX;
	(void)unusedY;
	if (refresh == 0) {
		return 0;
	}
	xrect_Set_Rect(&sourceRect, 0, 0, TORT640_BACKGROUND_WIDTH, TORT640_BACKGROUND_HEIGHT);
	if (g_tort640ScrollActor != NULL) {
		xrect_Offset_Rect(&sourceRect, 0, g_tort640ScrollActor->y + g_tort640VerticalOffset);
	}
	xrect_Clip_Rect(&sourceRect, &g_tort640PreviousDirtyRect);
	destinationX = sourceRect.left;
	destinationY = sourceRect.top;
	if (g_tort640ScrollActor != NULL) {
		xrect_Offset_Rect(&sourceRect, 0, -(g_tort640ScrollActor->y + g_tort640VerticalOffset));
	}
	backgroundPixels = (const uint8_t*)xmemhdl_Lock_Handle(g_tort640BackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &sourceRect, destinationX, destinationY,
								  TORT640_BACKGROUND_WIDTH, TORT640_BACKGROUND_HEIGHT);
	xmemhdl_Unlock_Handle(g_tort640BackgroundHandle);
	return 1;
}

// FUNCTION: XW 0x4630E0
void Tort640_user_DirtyActor(Actor* actor, int unusedTime) {
	(void)unusedTime;
	if (xactor_Is_Actor_Visible(actor) != 0 && actor->var2 == 0) {
		Rect visibleBounds;
		Rect actorFrame;
		xactor_Get_Actor_Rect(actor, &visibleBounds);
		xactor_Get_Actor_Frame(actor, &actorFrame);
		xcanvas_Clip_Rect_To_Canvas(&visibleBounds);
		xrect_Clip_Rect(&visibleBounds, &actorFrame);
		xrect_Enclose_Rect(&g_tort640CurrentDirtyRect, &visibleBounds);
	}
}

// FUNCTION: XW 0x463150
void Tort640_user_Close(Actor* actor, int unusedTime) {
	Rect frame;
	(void)unusedTime;
	if (g_tort640Film->cur_cel == g_tort640Film->cels) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		actor->var1 = 1;
	} else {
		xrect_Copy_Rect(&frame, &g_tort640PreviousDirtyRect);
		xrect_Enclose_Rect(&frame, &g_tort640CurrentDirtyRect);
		actor->var1 = 0;
	}
	if (!xrect_Empty_Rect(&frame)) {
		xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
	}
}

// FUNCTION: XW 0x4631F0
void Tort640_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	if (ShellPreferences_GetSfxEnabled()) {
		soundext_LoadSfx(XW_SHELL_SFX_BREATH, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_HUM_1, 0, NULL, 0, 0);
		soundext_LoadSfx(XW_SHELL_SFX_PAIN_TOOL, 0, NULL, 0, 0);
	}
	if (ShellPreferences_GetSfxEnabled()) {
		g_tort640Speech = soundext_LoadSpeech(XW_SHELL_SPEECH_LOCATION, 0, NULL, 0);
	} else {
		g_tort640Speech = NULL;
	}
}

// FUNCTION: XW 0x463260
void Tort640_PlaySoundCue(int16_t cue) {
	switch (cue) {
		case TORT640_CUE_BREATH:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				if (g_tort640Speech != NULL) {
					if (soundext_Count_Resource_Instances(g_tort640Speech) == 0) {
						soundext_Play_SFX(XW_SHELL_SFX_BREATH);
					}
				} else {
					soundext_Play_SFX(XW_SHELL_SFX_BREATH);
				}
			}
			break;
		case TORT640_CUE_HUM:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Play_SFX(XW_SHELL_SFX_HUM_1);
			}
			break;
		case TORT640_CUE_PAIN_TOOL:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				/* The original repeats the preference query with this opposite condition. */
				if (ShellPreferences_GetSfxEnabled() == 0) {
					soundext_Stop_SFX(XW_SHELL_SFX_BREATH);
				}
				soundext_Play_SFX(XW_SHELL_SFX_PAIN_TOOL);
			}
			break;
		case TORT640_CUE_SPEECH:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Start_Resource_SFX(g_tort640Speech);
			}
			break;
		case TORT640_CUE_FADE_HUM:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Fade_SFX(XW_SHELL_SFX_HUM_1, 0, TORT640_HUM_FADE_DURATION);
			}
			break;
	}
}
