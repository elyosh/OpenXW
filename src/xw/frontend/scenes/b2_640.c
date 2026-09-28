#include "xw/frontend/scenes/b2_640.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/b2_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/memhdl.h>
#include <landru/view.h>
#include <landru/viewadd.h>

// GLOBAL: XW 0x4CFEB0
const char* g_b2ResourceFilename = "b2_640.lfd";

// GLOBAL: XW 0x4CFEB4
const char* g_b2FilmName = "hrc2_f";

// GLOBAL: XW 0x4D02D0
const char* g_b2MusicFilename = "bl1music.lfd";

// GLOBAL: XW 0x4D02D4
const char* g_b2MusicName = "trofight";

// GLOBAL: XW 0x4F4B1C
Film* g_b2Film = NULL;

// GLOBAL: XW 0x4F4B20
Actor* g_b2CloseActor = NULL;

// GLOBAL: XW 0x4F4B24
LandruHandle g_b2BackgroundHandle = 0;

// GLOBAL: XW 0x4F4BA4
XwSceneMusicHandles g_b2MusicState = { NULL, NULL };

// GLOBAL: XW 0x4F4BB8
Film* g_b2SoundFilm = NULL;

// FUNCTION: XW 0x433560
XwShellSceneResult B2_640_Play(struct XwShellContext* shell) {
	ResFile* resourceFile;
	Rect frame;
	xfade_AddTimedText("I got him.", B2_640_FIRST_CAPTION_START, B2_640_FIRST_CAPTION_END,
					   B2_640_CAPTION_FONT, B2_640_FIRST_CAPTION_X, B2_640_FIRST_CAPTION_Y,
					   B2_640_CAPTION_COLOR);
	xfade_AddTimedText("Follow me.", B2_640_SECOND_CAPTION_START, B2_640_SECOND_CAPTION_END,
					   B2_640_CAPTION_FONT, B2_640_SECOND_CAPTION_X, B2_640_SECOND_CAPTION_Y,
					   B2_640_CAPTION_COLOR);
	resourceFile = xres_Open_Resource(g_b2ResourceFilename);
	xrect_Set_Rect(&frame, 0, 0, B2_640_SCENE_WIDTH, B2_640_SCENE_HEIGHT);
	g_b2BackgroundHandle = xmemhdl_Alloc_Clear_Handle(B2_640_BACKGROUND_BYTES, LANDRU_MEMORY_RESOURCE);
	g_b2Film = xfilm_Res_Callback_Film(resourceFile, g_b2FilmName, &frame, 0, 0, 0, B2_640_film_Callback);
	g_b2CloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, B2_640_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_b2CloseActor, B2_640_user_Close);
	xactor_Set_Actor_Draw_Function(g_b2CloseActor, XwCutscene_DrawClose);
	xfilm_Set_Film_Def_Palette(g_b2Film, shell->standardPalette);
	xview_Set_View_Update_Function(B2_640_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	B2_640_OpenMusic(resourceFile, g_b2Film);
	B2_640_OpenSounds(resourceFile, g_b2Film);
#ifdef XW_MODERN
	XwB2_RunView(resourceFile);
#else
	j_xviewadd_Handle_View();
	j_ShellPreferences_GetMusicEnabled();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xmemhdl_Free_Handle(g_b2BackgroundHandle);
	xres_Close_Resource(resourceFile);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x433710
void B2_640_end_View(int unusedTime) {
	int16_t exitScene;
	(void)unusedTime;
	if (g_savedShellPreferences.introPlaybackMode == 0) {
		if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_INTRO_DOGFIGHT_HRC3, XW_SCENE_REGISTER_INITIAL,
									  g_b2Film->cur_cel == g_b2Film->cels) != 0) {
			xerror_Set_Landru_Exit(exitScene);
		}
	} else if (g_b2Film->cur_cel == g_b2Film->cels) {
		xerror_Set_Landru_Exit(XW_SCENE_INTRO_DOGFIGHT_HRC3);
	}
}

// FUNCTION: XW 0x433770
int16_t B2_640_film_Callback(Film* film, FilmObject* object) {
	int16_t consumeObject = 0;
	Actor* actor;
	if (object->id == FTC_ACTOR) {
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		if (actor->var1 == CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND) {
			B2_640_film_Actor_To_Background(actor);
			if (actor->var2 == B2_640_BACKGROUND_TILED) {
				xactor_Set_Actor_Draw_Function(actor, B2_640_draw_Background);
			}
			consumeObject = actor->var2 == B2_640_BACKGROUND_CONSUME;
		} else if (actor->var1 == CUTSCENE_ACTOR_ROLE_SOUND) {
			xactor_Set_Actor_User_Function(actor, B2_640_user_Sound);
		}
	}
	return consumeObject;
}

// FUNCTION: XW 0x4337F0
int16_t B2_640_film_Actor_To_Background(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_b2BackgroundHandle);
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						B2_640_BACKGROUND_WIDTH, B2_640_BACKGROUND_HEIGHT, 0);
	if (actor->draw != NULL) {
		xrect_Copy_Rect(&actorClip, &actor->frame);
		xcanvas_Clip_Rect_To_Canvas(&actorClip);
		xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
		drawResult = actor->draw(actor, &canvasBounds, &actorClip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_b2BackgroundHandle);
	return drawResult;
}

// FUNCTION: XW 0x4338D0
void B2_640_user_Sound(Actor* actor, int unusedTime) {
	int16_t cue = actor->var2;
	(void)unusedTime;
	if (cue != 0) {
		B2_640_PlaySoundCue(cue);
	}
}

// FUNCTION: XW 0x4338F0
int16_t B2_640_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t x, int16_t y,
							   int16_t refresh) {
	Rect sourceRect;
	int16_t baseX;
	int16_t baseY;
	int16_t tileX;
	int16_t tileY;
	const uint8_t* backgroundPixels;
	(void)unusedActor;
	(void)unusedFrame;
	(void)unusedClip;
	if (refresh == 0) {
		return 0;
	}
	baseX = x;
	baseY = y;
	if (x >= 0) {
		baseX -= B2_640_BACKGROUND_WIDTH;
	}
	if (y >= 0) {
		baseY -= B2_640_BACKGROUND_HEIGHT;
	}
	xrect_Set_Rect(&sourceRect, 0, 0, B2_640_BACKGROUND_WIDTH, B2_640_BACKGROUND_HEIGHT);
	backgroundPixels = (const uint8_t*)xmemhdl_Lock_Handle(g_b2BackgroundHandle);
	for (tileY = 0; tileY < B2_640_TILE_ROWS * B2_640_BACKGROUND_HEIGHT; tileY += B2_640_BACKGROUND_HEIGHT) {
		for (tileX = 0; tileX < B2_640_TILE_COLUMNS * B2_640_BACKGROUND_WIDTH;
			 tileX += B2_640_BACKGROUND_WIDTH) {
			stub_Copy_From_Clipped_Buffer(backgroundPixels, &sourceRect, tileX + baseX, tileY + baseY,
										  B2_640_BACKGROUND_WIDTH, B2_640_BACKGROUND_HEIGHT);
		}
	}
	xmemhdl_Unlock_Handle(g_b2BackgroundHandle);
	return 1;
}

// FUNCTION: XW 0x4339D0
void B2_640_user_Close(Actor* actor, int unusedTime) {
	(void)unusedTime;
	if (g_b2Film->cur_cel == g_b2Film->cels) {
		actor->var1 = 1;
	} else {
		actor->var1 = 0;
	}
}

// FUNCTION: XW 0x434E30
void B2_640_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled()) {
		g_b2MusicState.film = sceneFilm;
		g_b2MusicState.sound = xsound_Find_Gmid(g_b2MusicName);
		if (g_b2MusicState.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource(g_b2MusicFilename);
			g_b2MusicState.sound = xsound_Res_Music(musicResource, g_b2MusicName);
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_b2MusicState.sound);
			soundext_ScanMidi(g_b2MusicState.sound, 0, B2_640_MUSIC_START_BEAT, 0);
		}
		xsound_Set_Sound_Keep(g_b2MusicState.sound);
	}
}

// FUNCTION: XW 0x434ED0
void B2_640_OpenSounds(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	g_b2SoundFilm = sceneFilm;
	if (ShellPreferences_GetSfxEnabled()) {
		soundext_LoadSfx(XW_SHELL_SFX_TIE_APPROACH_2, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_TORPEDO_2, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_EXPLOSION_2, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_EXPLOSION_FAR, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_FLYBY_1, 0, NULL, 0, 0);
	}
	ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x434F50
void B2_640_PlaySoundCue(int16_t cue) {
	if (ShellPreferences_GetSfxEnabled() != 0) {
		switch (cue) {
			case B2_640_CUE_TIE_APPROACH:
				soundext_Play_SFX(XW_SHELL_SFX_TIE_APPROACH_2);
				break;
			case B2_640_CUE_TORPEDO:
				soundext_Play_SFX(XW_SHELL_SFX_TORPEDO_2);
				break;
			case B2_640_CUE_EXPLOSION:
				soundext_Play_SFX(XW_SHELL_SFX_EXPLOSION_2);
				break;
			case B2_640_CUE_DISTANT_EXPLOSION:
				soundext_Play_SFX(XW_SHELL_SFX_EXPLOSION_FAR);
				break;
			case B2_640_CUE_FLYBY:
				soundext_Play_SFX(XW_SHELL_SFX_FLYBY_1);
				break;
		}
	}
}
