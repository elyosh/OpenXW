#include "xw/frontend/scenes/awards640.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#ifdef XW_MODERN
#include "xw_runtime/runtime/awards640_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/font.h>
#include <landru/io.h>
#include <landru/view.h>
#include <landru/viewadd.h>

// GLOBAL: XW 0x4CFAF8
char g_awards640CongratulationsText[AWARDS640_CAPTION_CAPACITY] =
	"Congratulations on behalf of the Rebel Alliance.";

// GLOBAL: XW 0x4F4A98
Rect g_awards640DirtyRect = { 0 };

// GLOBAL: XW 0x4F4AA0
Actor* g_awards640BackgroundActor = NULL;

// GLOBAL: XW 0x4F4AA4
Actor* g_awards640ScrollActor = NULL;

// GLOBAL: XW 0x4F4AA8
Actor* g_awards640CloseActor = NULL;

// GLOBAL: XW 0x4F4AAC
Film* g_awards640Film = NULL;

// GLOBAL: XW 0x4F4AB0
LandruHandle g_awards640BackgroundHandle = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4F4AB4
int16_t g_awards640FullRefreshCountdown = 0;

// GLOBAL: XW 0x4F4AB8
int16_t g_awards640PreviousScrollY = 0;

// GLOBAL: XW 0x4F4ABC
Sound* g_awards640Music = NULL;

// GLOBAL: XW 0x4F4AC0
Film* g_awards640MusicFilm = NULL;

// GLOBAL: XW 0x4F4AC4
Sound* g_awards640Speech = NULL;

// FUNCTION: XW 0x431B50
XwShellSceneResult Awards640_Play(struct XwShellContext* shell) {
	ResFile* resourceFile;
	Rect frame;
	resourceFile = xres_Open_Resource("awards64.lfd");
	if (shellext_Get_Cur_Scene() == XW_SCENE_AWARD_CEREMONY_1) {
		int16_t captionWidth =
			xfont_Get_String_Width_0(AWARDS640_CAPTION_FONT, g_awards640CongratulationsText);
		xfade_AddTimedText(g_awards640CongratulationsText, AWARDS640_CAPTION_START, AWARDS640_CAPTION_END,
						   AWARDS640_CAPTION_FONT, AWARDS640_BACKGROUND_WIDTH / 2 - (captionWidth >> 1),
						   AWARDS640_CAPTION_Y, AWARDS640_CAPTION_COLOR);
	}
	xrect_Set_Rect(&frame, 0, 0, AWARDS640_BACKGROUND_WIDTH, AWARDS640_BACKGROUND_HEIGHT);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	g_awards640PreviousScrollY = 0;
	g_awards640FullRefreshCountdown = 0;
	g_awards640BackgroundHandle = xmemhdl_Alloc_Clear_Handle(
		AWARDS640_BACKGROUND_WIDTH * AWARDS640_BACKGROUND_HEIGHT, LANDRU_MEMORY_RESOURCE);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	g_awards640ScrollActor = NULL;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_AWARD_CEREMONY_1:
			if ((uint16_t)xio_Is_System_Slower_Than(AWARDS640_SPEED_THRESHOLD) != 0)
				g_awards640Film = xfilm_Res_Callback_Film(resourceFile, "award1_s", &frame, 0, 0, 0,
														  Awards640_film_Callback);
			else
				g_awards640Film = xfilm_Res_Callback_Film(resourceFile, "award1_f", &frame, 0, 0, 0,
														  Awards640_film_Callback);
			break;
		case XW_SCENE_AWARD_CEREMONY_2:
			if ((uint16_t)xio_Is_System_Slower_Than(AWARDS640_SPEED_THRESHOLD) != 0)
				g_awards640Film = xfilm_Res_Callback_Film(resourceFile, "award2_s", &frame, 0, 0, 0,
														  Awards640_film_Callback);
			else
				g_awards640Film = xfilm_Res_Callback_Film(resourceFile, "award2_f", &frame, 0, 0, 0,
														  Awards640_film_Callback);
			break;
		case XW_SCENE_AWARD_CEREMONY_3:
			if ((uint16_t)xio_Is_System_Slower_Than(AWARDS640_SPEED_THRESHOLD) != 0)
				g_awards640Film = xfilm_Res_Callback_Film(resourceFile, "award3_s", &frame, 0, 0, 0,
														  Awards640_film_Callback);
			else
				g_awards640Film = xfilm_Res_Callback_Film(resourceFile, "award3_f", &frame, 0, 0, 0,
														  Awards640_film_Callback);
			break;
		case XW_SCENE_VICTORY_MEDAL_CEREMONY_1:
			g_awards640Film =
				xfilm_Res_Callback_Film(resourceFile, "medal1_f", &frame, 0, 0, 0, Awards640_film_Callback);
			break;
		case XW_SCENE_VICTORY_MEDAL_CEREMONY_2:
			if ((uint16_t)xio_Is_System_Slower_Than(AWARDS640_SPEED_THRESHOLD) != 0)
				g_awards640Film = xfilm_Res_Callback_Film(resourceFile, "medal2_s", &frame, 0, 0, 0,
														  Awards640_film_Callback);
			else
				g_awards640Film = xfilm_Res_Callback_Film(resourceFile, "medal2_f", &frame, 0, 0, 0,
														  Awards640_film_Callback);
			break;
		case XW_SCENE_VICTORY_MEDAL_CEREMONY_3:
			if ((uint16_t)xio_Is_System_Slower_Than(AWARDS640_SPEED_THRESHOLD) != 0)
				g_awards640Film = xfilm_Res_Callback_Film(resourceFile, "medal3_s", &frame, 0, 0, 0,
														  Awards640_film_Callback);
			else
				g_awards640Film = xfilm_Res_Callback_Film(resourceFile, "medal3_f", &frame, 0, 0, 0,
														  Awards640_film_Callback);
			break;
	}
	xfilm_Set_Film_Def_Palette(g_awards640Film, shell->standardPalette);
	g_awards640BackgroundActor =
		xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, AWARDS640_BACKGROUND_Z);
	xactor_Set_Actor_Draw_Function(g_awards640BackgroundActor, Awards640_draw_Background);
	g_awards640CloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, AWARDS640_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_awards640CloseActor, Awards640_user_Close);
	xactor_Set_Actor_Draw_Function(g_awards640CloseActor, Cutscene_DrawConditionalErase);
	xrect_Clear_Rect(&g_awards640DirtyRect);
	xview_Set_View_Update_Function(Awards640_end_View);
	Awards640_OpenMusic(resourceFile, g_awards640Film);
	Awards640_OpenSounds(resourceFile, g_awards640Film);
#ifdef XW_MODERN
	XwAwards640_RunView(resourceFile);
#else
	j_xviewadd_Handle_View();
	Awards640_CloseMusic();
	soundext_RecheckSfxPreference();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xmemhdl_Free_Handle(g_awards640BackgroundHandle);
	xres_Close_Resource(resourceFile);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x431EA0
void Awards640_end_View(int unusedTime) {
	int16_t exitScene;
	int16_t nextScene;
	int16_t nextSection;
	(void)unusedTime;
	nextSection = shipext_Get_Post_Award_Scene();
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_AWARD_CEREMONY_1:
			nextScene = XW_SCENE_AWARD_CEREMONY_2;
			break;
		case XW_SCENE_AWARD_CEREMONY_2:
			nextScene = XW_SCENE_AWARD_CEREMONY_3;
			break;
		case XW_SCENE_VICTORY_MEDAL_CEREMONY_1:
			nextScene = XW_SCENE_VICTORY_MEDAL_CEREMONY_2;
			break;
		case XW_SCENE_VICTORY_MEDAL_CEREMONY_2:
			nextScene = XW_SCENE_VICTORY_MEDAL_CEREMONY_3;
			break;
		case XW_SCENE_AWARD_CEREMONY_3:
		case XW_SCENE_VICTORY_MEDAL_CEREMONY_3:
			nextScene = shipext_Get_Post_Award_Scene();
			break;
#ifdef XW_MODERN
		default:
			nextScene = nextSection;
			break;
#endif
	}
	if (shellext_Check_Scene_Exit(&exitScene, nextScene, nextSection,
								  g_awards640Film->cur_cel == g_awards640Film->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x431F50
int16_t Awards640_film_Callback(Film* film, FilmObject* object) {
	int16_t consumeObject = 0;
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		switch (actor->var1) {
			case CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS:
				xactor_Set_Actor_User_Function(actor, Awards640_user_Actor);
				break;
			case AWARDS640_ACTOR_SCROLL:
				g_awards640ScrollActor = actor;
				break;
			case CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND:
				if (actor->var2 == AWARDS640_STAMP_MEDAL) {
					int16_t medalIndex = shipext_Get_Pending_Medal_Index();
					if (medalIndex < AWARDS640_MEDAL_LINEAR_END) {
						if (medalIndex < AWARDS640_MEDAL_LINEAR_FIRST) {
							xactor_Set_Actor_State(actor, AWARDS640_MEDAL_DEFAULT_STATE, 0);
						} else {
							xactor_Set_Actor_State(actor, medalIndex + AWARDS640_MEDAL_STATE_OFFSET, 0);
						}
					} else {
						switch (medalIndex) {
							case AWARDS640_MEDAL_SPECIAL_7:
								xactor_Set_Actor_State(actor, AWARDS640_MEDAL_7_STATE, 0);
								break;
							case AWARDS640_MEDAL_SPECIAL_8:
								xactor_Set_Actor_State(actor, AWARDS640_MEDAL_8_STATE, 0);
								break;
							case AWARDS640_MEDAL_SPECIAL_9:
								xactor_Set_Actor_State(actor, AWARDS640_MEDAL_9_STATE, 0);
								break;
						}
					}
				}
				Awards640_ActorToScreenAndBackground(actor);
				consumeObject = 1;
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, Awards640_user_Sound);
				break;
		}
	}
	return consumeObject;
}

// FUNCTION: XW 0x432070
int16_t Awards640_ActorToScreenAndBackground(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	int16_t drawResult = 0;
	int backgroundYOffset = 0;
	int16_t pass;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	for (pass = 0; pass < AWARDS640_DRAW_PASS_COUNT; ++pass) {
		if (pass != 0) {
			uint8_t* backgroundPixels = xmemhdl_Lock_Handle(g_awards640BackgroundHandle);
			xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth,
								&previousHeight, AWARDS640_BACKGROUND_WIDTH, AWARDS640_BACKGROUND_HEIGHT, 0);
		}
		if (actor->draw != NULL) {
			drawResult =
				actor->draw(actor, &canvasBounds, &canvasBounds, actor->x, actor->y + backgroundYOffset, 1);
		}
		if (pass != 0) {
			xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
			xmemhdl_Unlock_Handle(g_awards640BackgroundHandle);
		}
		switch ((int16_t)shellext_Get_Cur_Scene()) {
			case XW_SCENE_AWARD_CEREMONY_1:
				backgroundYOffset -= AWARDS640_BACKGROUND_HEIGHT;
				break;
			case XW_SCENE_AWARD_CEREMONY_3:
			case XW_SCENE_VICTORY_MEDAL_CEREMONY_3:
				backgroundYOffset += AWARDS640_BACKGROUND_HEIGHT;
				break;
		}
	}
	return drawResult;
}

// FUNCTION: XW 0x432170
void Awards640_user_Sound(Actor* actor, int unusedTime) {
	int16_t cue = actor->var2;
	(void)unusedTime;
	if (cue != 0) {
		Awards640_PlaySoundCue(cue);
	}
}

// FUNCTION: XW 0x432190
int16_t Awards640_draw_Background(Actor* actor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
								  int16_t unusedY, int16_t refresh) {
	Rect scrollBounds;
	(void)unusedFrame;
	(void)unusedClip;
	(void)unusedX;
	(void)unusedY;
	if (refresh == 0)
		return 0;
	if (g_awards640ScrollActor != NULL &&
		(g_awards640PreviousScrollY != g_awards640ScrollActor->y || g_awards640ScrollActor->var2 != 0)) {
		uint8_t* screenPixels = xcanvas_Get_Screen_Buffer();
		int16_t deltaY;
		uint8_t* backgroundPixels;
		int16_t scene;
		xcanvas_Get_Drawing_Canvas_Bounds(&scrollBounds);
		if (shellext_Get_Cur_Scene() == XW_SCENE_AWARD_CEREMONY_1)
			scrollBounds.bottom = g_awards640PreviousScrollY + AWARDS640_BACKGROUND_HEIGHT;
		else
			scrollBounds.top = g_awards640PreviousScrollY;
		deltaY = g_awards640ScrollActor->y - g_awards640PreviousScrollY;
		if (actor->var2 == 0)
			xcanvas_Scroll_Clipped_Buffer(screenPixels, &scrollBounds, 0, deltaY, AWARDS640_BACKGROUND_WIDTH,
										  AWARDS640_BACKGROUND_HEIGHT);
		xcanvas_Get_Drawing_Canvas_Bounds(&scrollBounds);
		backgroundPixels = xmemhdl_Lock_Handle(g_awards640BackgroundHandle);
		scene = shellext_Get_Cur_Scene();
		if (scene == XW_SCENE_AWARD_CEREMONY_1)
			stub_Copy_From_Clipped_Buffer(backgroundPixels, &scrollBounds, 0,
										  deltaY + g_awards640PreviousScrollY + AWARDS640_BACKGROUND_HEIGHT,
										  AWARDS640_BACKGROUND_WIDTH, AWARDS640_BACKGROUND_HEIGHT);
		else
			stub_Copy_From_Clipped_Buffer(backgroundPixels, &scrollBounds, 0,
										  deltaY + g_awards640PreviousScrollY - AWARDS640_BACKGROUND_HEIGHT,
										  AWARDS640_BACKGROUND_WIDTH, AWARDS640_BACKGROUND_HEIGHT);
		xmemhdl_Unlock_Handle(g_awards640BackgroundHandle);
		g_awards640PreviousScrollY = g_awards640ScrollActor->y;
	}
	return 1;
}

// FUNCTION: XW 0x4322C0
void Awards640_user_Actor(Actor* actor, int unusedTime) {
	Rect dirtyBounds;
	(void)unusedTime;
	if (xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &dirtyBounds);
		xcanvas_Clip_Rect_To_Canvas(&dirtyBounds);
		xrect_Enclose_Rect(&g_awards640DirtyRect, &dirtyBounds);
	}
}

// FUNCTION: XW 0x432310
void Awards640_user_Close(Actor* actor, int unusedTime) {
	Rect frame;
	(void)unusedTime;
	if (g_awards640Film->cur_cel == g_awards640Film->cels) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		actor->var1 = 1;
	} else {
		if (g_awards640ScrollActor != NULL) {
			if (g_awards640ScrollActor->xv != 0 || g_awards640ScrollActor->yv != 0 ||
				g_awards640ScrollActor->xvf != 0 || g_awards640ScrollActor->yvf != 0)
				g_awards640FullRefreshCountdown = AWARDS640_FULL_REFRESH_FRAMES;
			if (g_awards640FullRefreshCountdown != 0 || g_awards640ScrollActor->var2 != 0) {
				xcanvas_Get_Drawing_Canvas_Bounds(&frame);
				if (g_awards640FullRefreshCountdown != 0)
					--g_awards640FullRefreshCountdown;
			} else {
				xrect_Copy_Rect(&frame, &g_awards640DirtyRect);
				xcanvas_Clip_Rect_To_Canvas(&frame);
			}
		} else {
			xrect_Copy_Rect(&frame, &g_awards640DirtyRect);
			xcanvas_Clip_Rect_To_Canvas(&frame);
		}
		actor->var1 = 0;
	}
	xrect_Clear_Rect(&g_awards640DirtyRect);
	if (xrect_Empty_Rect(&frame) == 0) {
		xview_Set_View_Frame(AWARDS640_MAIN_VIEW, &frame);
		xview_Set_View_Pos(AWARDS640_MAIN_VIEW, frame.left, frame.top);
	}
}

// FUNCTION: XW 0x432410
void Awards640_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		const char* musicFilename;
		const char* musicName;
		unsigned int startBeat;
		g_awards640MusicFilm = sceneFilm;
		musicFilename = "ldmusic.lfd";
		musicName = "recruits";
		switch (shellext_Get_Cur_Scene()) {
			case XW_SCENE_VICTORY_MEDAL_CEREMONY_1:
				startBeat = XW_AWARDS640_VICTORY_BEAT_1;
				musicFilename = "yvmusic.lfd";
				musicName = "victory";
				break;
			case XW_SCENE_VICTORY_MEDAL_CEREMONY_2:
				startBeat = XW_AWARDS640_VICTORY_BEAT_2;
				musicFilename = "yvmusic.lfd";
				musicName = "victory";
				break;
			case XW_SCENE_VICTORY_MEDAL_CEREMONY_3:
				startBeat = XW_AWARDS640_VICTORY_BEAT_3;
				musicFilename = "yvmusic.lfd";
				musicName = "victory";
				break;
			case XW_SCENE_AWARD_CEREMONY_1:
			case XW_SCENE_AWARD_CEREMONY_2:
			case XW_SCENE_AWARD_CEREMONY_3:
				startBeat = 0;
				break;
			default:
				startBeat = 0;
				break;
		}
		g_awards640Music = xsound_Find_Gmid(musicName);
		if (g_awards640Music == NULL) {
			ResFile* resourceFile = xres_Open_Resource(musicFilename);
			g_awards640Music = xsound_Res_Music(resourceFile, musicName);
			xres_Close_Resource(resourceFile);
			soundext_Start_Resource_Sound(g_awards640Music);
			if (startBeat != 0) {
				soundext_ScanMidi(g_awards640Music, 0, startBeat, 0);
			}
		}
		xsound_Set_Sound_Keep(g_awards640Music);
	}
}

// FUNCTION: XW 0x432510
void Awards640_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0 && shellext_Get_Cur_Scene() == XW_SCENE_AWARD_CEREMONY_3) {
		Sound* music = xsound_Find_Gmid("recruits");
		g_awards640Music = music;
		if (music != NULL) {
			soundext_FadeVolume(music, 0, AWARDS640_MUSIC_FADE_DURATION);
			/* Resource pointers are outside the numeric flight-sound ID range. */
			soundext_SetPriority(0, 0);
		}
	}
}

// FUNCTION: XW 0x432560
void Awards640_OpenSounds(ResFile* unusedResourceFile, Film* unusedFilm) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	ShellPreferences_GetSfxEnabled();
	if (ShellPreferences_GetSfxEnabled() && shellext_Get_Cur_Scene() == XW_SCENE_AWARD_CEREMONY_1) {
		g_awards640Speech = soundext_LoadSpeech(XW_SHELL_SPEECH_CONGRATS, 0, NULL, 1);
	}
}

// FUNCTION: XW 0x432590
void Awards640_PlaySoundCue(int16_t cue) {
	if (ShellPreferences_GetSfxEnabled()) {
		switch (cue) {
			case XW_AWARDS640_CUE_SPEECH:
				if (g_awards640Speech != NULL) {
					soundext_Start_Resource_SFX(g_awards640Speech);
				}
				break;
		}
	}
}
