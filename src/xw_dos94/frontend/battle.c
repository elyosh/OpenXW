#include "xw/audio/soundext.h"
#include "xw/flight/mission/mission.h"
#include "xw/frontend/scenes/b1b640.h"
#include "xw/frontend/scenes/b2_640.h"
#include "xw/frontend/scenes/b3_640.h"
#include "xw/frontend/scenes/b4_640.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/scenes/sun_shot.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"
#include "xw/util/landru_display.h"
#include "xw_dos94/frontend/scenes.h"
#include "xw_runtime/runtime/scene_view_task.h"
#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/view.h>
#include <landru/viewadd.h>

static ResFile* resource;
static Film* film;
static LandruHandle background;
static int active_scene, background_height;

/* DOS94 0x1405a2, 0x140e2a, 0x141750, 0x141eee, 0x142738. */
static void stamp_background(Actor* actor) {
	Rect bounds, clip, saved_clip;
	uint8_t* saved_pixels;
	int16_t width, height;
	xcanvas_Get_Drawing_Canvas_Bounds(&bounds);
	uint8_t* pixels = xmemhdl_Lock_Handle(background);
	xcanvas_Push_Canvas(&saved_pixels, pixels, &saved_clip, &width, &height, 320, background_height, 0);
	if (actor->draw) {
		clip = actor->frame;
		xcanvas_Clip_Rect_To_Canvas(&clip);
		xcanvas_Set_Drawing_Canvas_Clip(&clip);
		actor->draw(actor, &bounds, &clip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(saved_pixels, &saved_clip, width, height);
	xmemhdl_Unlock_Handle(background);
}

/* DOS94 0x1406b4, 0x140f3c, 0x141862, 0x142000, 0x14284a. */
static int16_t draw_background(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	(void)actor;
	(void)frame;
	(void)clip;
	if (!refresh)
		return 0;
	int16_t base_x = x >= 0 ? x - 320 : x;
	int16_t base_y = y >= 0 ? y - 200 : y;
	Rect source;
	xrect_Set_Rect(&source, 0, 0, 320, background_height);
	const uint8_t* pixels = xmemhdl_Lock_Handle(background);
	for (int row = 0; row < 400; row += background_height)
		for (int col = 0; col < 640; col += 320)
			stub_Copy_From_Clipped_Buffer(pixels, &source, base_x + col, base_y + row, 320,
										  background_height);
	xmemhdl_Unlock_Handle(background);
	return 1;
}

static void update_sound(Actor* actor, int32_t time) {
	(void)time;
	int action = actor->var2;
	if (!action)
		return;
	switch (active_scene) {
		case 20:
			SunShot_PlaySoundAction(action);
			break;
		case 21:
			B2_640_PlaySoundCue(action);
			break;
		case 22:
			/* DOS94 0x142476; Windows B3 has no actor sound-action callback. */
			if (ShellPreferences_GetSfxEnabled() && (action == 1 || action == 2))
				soundext_Play_SFX(action == 1 ? 4 : 7);
			break;
		case 23: {
			/* DOS94 0x142af6. */
			static const int sounds[] = { 10, 7, 0, 16 };
			if (ShellPreferences_GetSfxEnabled() && action >= 1 && action <= 4)
				soundext_Play_SFX(sounds[action - 1]);
			break;
		}
		default:
			B1b640_PlaySoundCue(action);
			break;
	}
}

/* DOS94 B2 callback family; formation additionally installs the jitter callbacks. */
static int16_t film_callback(Film* loading, FilmObject* object) {
	if (object->id != FTC_ACTOR)
		return 0;
	xfilm_Rewind_Actor_Film(loading, object, object + 1);
	Actor* actor = object->object;
	if (actor->var1 == 20) {
		stamp_background(actor);
		if (actor->var2 == 1)
			xactor_Set_Actor_Draw_Function(actor, draw_background);
		return actor->var2 == 0;
	}
	if (actor->var1 == 50)
		xactor_Set_Actor_User_Function(actor, update_sound);
	if (actor->var1 == 10 && (active_scene == 27 || active_scene >= 310)) {
		xactor_Set_Actor_User_Function(actor, B1b640_user_Jitter);
		xactor_Set_Actor_Draw_Function(actor, B1b640_draw_Jitter);
	}
	return 0;
}

static void close_frame(Actor* actor, int32_t time) {
	(void)time;
	actor->var1 = film->cur_cel == film->cels;
}

static int16_t draw_close(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	(void)refresh;
	if (actor->var1)
		xcanvas_Erase_Canvas();
	return 1;
}

static void end_view(int32_t time) {
	(void)time;
	int16_t next, section = 26, exit_scene;
	if (active_scene == 27)
		next = 20;
	else if (active_scene >= 310 && active_scene <= 313) {
		static const int16_t destinations[] = { 290, 291, 292, 296 };
		next = destinations[active_scene - 310];
		section = 115;
	} else
		next = active_scene + 1;
	if (shellext_Check_Scene_Exit(&exit_scene, next, section, film->cur_cel == film->cels))
		xerror_Set_Landru_Exit(exit_scene);
}

static void release_scene(void) {
	xview_Clear_View_Update_Function();
	xmemhdl_Free_Handle(background);
	background = 0;
	xres_Close_Resource(resource);
	resource = NULL;
}

static void finish_scene(void) {
	if (active_scene == 22)
		B3_640_CloseMusic();
	else if (active_scene == 27 || active_scene >= 310)
		B1b640_CloseMusic();
	Rect frame;
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
}

/* DOS94 0x140306 / 0x14098e / 0x14146a / 0x141bec / 0x1424a4. */
void Dos94_Battle(XwShellContext* shell) {
	active_scene = shellext_Get_Cur_Scene();
	background_height = active_scene == 23 ? 100 : 200;
	const char* filename;
	const char* film_name;
	switch (active_scene) {
		case 20:
			filename = "battle1.lfd";
			film_name = "bat1_f";
			break;
		case 21:
			xfade_AddTimedText("I got him.", 26, 36, 0, 80, 120, 14);
			xfade_AddTimedText("Follow me.", 43, 53, 0, 160, 120, 14);
			filename = "battle2.lfd";
			film_name = "bat2_f";
			break;
		case 22:
			xfade_AddTimedText("I can't shake him!", 4, 13, 0, 100, 120, 14);
			xfade_AddTimedText("I'm on him.", 14, 24, 0, 30, 40, 51);
			xfade_AddTimedText("Nice shot, Red Two!", 25, 39, 0, 180, 140, 14);
			filename = "battle3.lfd";
			film_name = "bat3_f";
			break;
		case 23:
			filename = "battle4.lfd";
			film_name = "bat4_f";
			break;
		default: {
			static const char* const names[] = { "tenda_f", "tendx_f", "tendy_f", "tendb_f" };
			g_b1b640CaptionVariant = 0;
			if (active_scene != 27) {
				g_b1b640CaptionVariant =
					g_missionRuntimeState.provingGroundsLevel <= shipext_Get_Train_Level() + 1 ? 1
					: g_missionRuntimeState.provingGroundsLevel <= 6                           ? 2
																							   : 3;
			}
			B1B640Caption* caption = &g_b1b640Captions[g_b1b640CaptionVariant];
			xfade_AddTimedText(caption->firstLine, 10, 50, 0, 80, 176, 14);
			xfade_AddTimedText(caption->secondLine, 10, 50, 0, 80, 186, 14);
			filename = "battle1b.lfd";
			film_name = active_scene == 27 ? "bat1b_f" : names[active_scene - 310];
			break;
		}
	}
	resource = xres_Open_Resource(filename);
	Rect frame;
	xrect_Set_Rect(&frame, 0, 0, 320, 200);
	background = xmemhdl_Alloc_Clear_Handle(320 * background_height, LANDRU_MEMORY_RESOURCE);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	ResFile* source = active_scene == 313 ? xres_Open_Resource("bwing.lfd") : resource;
	film = xfilm_Res_Callback_Film(source, film_name, &frame, 0, 0, 0, film_callback);
	if (source != resource)
		xres_Close_Resource(source);
	Actor* close = xactcust_Alloc_Custom_Actor(0, &frame, 0, 0, -200);
	xactor_Set_Actor_User_Function(close, close_frame);
	xactor_Set_Actor_Draw_Function(close, draw_close);
	xfilm_Set_Film_Def_Palette(film, shell->standardPalette);
	xview_Set_View_Update_Function(end_view);
	if (xcursor_Is_Cursor_Visible())
		xcursor_Hide_Cursor();
	switch (active_scene) {
		case 20:
			Cutscene_EnsureTroFightMusic(resource, film);
			break;
		case 21:
			B2_640_OpenMusic(resource, film);
			break;
		case 22:
			B3_640_OpenMusic(resource, film);
			break;
		case 23:
			B4_640_OpenMusic(resource, film);
			break;
		default:
			B1b640_OpenMusic(resource, film);
			B1b640_OpenSounds(resource, film, g_b1b640CaptionVariant);
			break;
	}
	XwScene_RunView(finish_scene, release_scene);
}
