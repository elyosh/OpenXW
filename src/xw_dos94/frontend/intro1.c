#include "xw/frontend/scenes/intro1.h"
#include "xw/frontend/shellext.h"
#include "xw/util/landru_display.h"
#include "xw_dos94/frontend/intro.h"
#include "xw_runtime/runtime/intro1_task.h"
#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/io.h>
#include <landru/view.h>
#include <landru/viewadd.h>

static LandruHandle background;
static Rect dirty, previous_dirty;

/* DOS94 0x4c2aec. */
static int16_t stamp_background(Actor* actor) {
	Rect bounds, saved_clip, clip;
	uint8_t* saved_pixels;
	int16_t saved_width, saved_height;
	int16_t result = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&bounds);
	uint8_t* pixels = xmemhdl_Lock_Handle(background);
	xcanvas_Push_Canvas(&saved_pixels, pixels, &saved_clip, &saved_width, &saved_height, 320, 200, 0);
	if (actor->draw) {
		clip = actor->frame;
		xcanvas_Clip_Rect_To_Canvas(&clip);
		xcanvas_Set_Drawing_Canvas_Clip(&clip);
		result = actor->draw(actor, &bounds, &clip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(saved_pixels, &saved_clip, saved_width, saved_height);
	xmemhdl_Unlock_Handle(background);
	return result;
}

/* DOS94 0x4c2bfe. */
static void begin_dirty_frame(Actor* actor, int32_t time) {
	(void)actor;
	if (!time)
		xcanvas_Get_Drawing_Canvas_Bounds(&previous_dirty);
	else
		previous_dirty = dirty;
	xrect_Clear_Rect(&dirty);
}

/* DOS94 0x4c2c40. */
static int16_t draw_background(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	(void)actor;
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (!refresh)
		return 0;
	const uint8_t* pixels = xmemhdl_Lock_Handle(background);
	stub_Copy_From_Clipped_Buffer(pixels, &previous_dirty, previous_dirty.left, previous_dirty.top, 320, 200);
	xmemhdl_Unlock_Handle(background);
	return 1;
}

/* DOS94 0x4c2ca6: var2 also stamps changing actors into the saved background. */
static void accumulate_dirty(Actor* actor, int32_t time) {
	(void)time;
	Rect bounds, frame;
	if (xactor_Is_Actor_Visible(actor)) {
		xactor_Get_Actor_Rect(actor, &bounds);
		xactor_Get_Actor_Frame(actor, &frame);
		xcanvas_Clip_Rect_To_Canvas(&bounds);
		xrect_Clip_Rect(&bounds, &frame);
		xrect_Enclose_Rect(&dirty, &bounds);
	}
	if (actor->var2)
		stamp_background(actor);
}

/* DOS94 0x4c2d32. */
static void erase_frame(Actor* actor, int32_t time) {
	(void)time;
	Rect frame;
	if (g_intro1Film->cur_cel == g_intro1Film->cels) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		actor->var1 = 1;
	} else {
		frame = previous_dirty;
		xrect_Enclose_Rect(&frame, &dirty);
		actor->var1 = 0;
	}
	if (!xrect_Empty_Rect(&frame)) {
		xview_Set_View_Frame(0, &frame);
		xview_Set_View_Pos(0, frame.left, frame.top);
	}
}

/* DOS94 0x4c2dca. */
static int16_t draw_close(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (!refresh)
		return 0;
	if (actor->var1)
		xcanvas_Erase_Canvas();
	return 1;
}

/* DOS94 0x4c2a3c. */
static int16_t film_callback(Film* film, FilmObject* object) {
	int16_t consumed = 0;
	if (object->id == FTC_ACTOR) {
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		Actor* actor = object->object;
		if (xio_Is_System_Slower_Than(2)) {
			if (actor->var1 == 20) {
				stamp_background(actor);
				consumed = 1;
			}
			if (actor->var1 == 10)
				xactor_Set_Actor_User_Function(actor, accumulate_dirty);
		}
		if (actor->var1 == 50)
			xactor_Set_Actor_User_Function(actor, Intro1_user_SoundAction);
	}
	return consumed;
}

/* DOS94 0x4c29e8. */
static void end_view(int32_t time) {
	(void)time;
	int16_t exit_scene;
	if (shellext_Check_Scene_Exit(&exit_scene, 13, 26, g_intro1Film->cur_cel == g_intro1Film->cels))
		xerror_Set_Landru_Exit(exit_scene);
}

/* DOS94 0x4c2760. The task owns the resource until the view completes. */
void Dos94_Intro1_Opening(XwShellContext* shell) {
	ResFile* resource = xres_Open_Resource("intro.lfd");
	Rect frame;
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xfade_AddTimedText("Star Destroyer patrol near the planet Turkana", 30, 90, 0, 30, 180, 16);
	if (xio_Is_System_Slower_Than(2)) {
		xviewadd_Clear_View();
		xview_Disable_All_View_Erase();
		xcanvas_Invalid_Screen_Diff();
		background = xmemhdl_Alloc_Clear_Handle(64000, LANDRU_MEMORY_RESOURCE);
		g_intro1Film = xfilm_Res_Callback_Film(resource, "intro1_s", &frame, 0, 0, 0, film_callback);
		xfilm_Set_Film_Def_Palette(g_intro1Film, shell->standardPalette);
		Actor* back = xactcust_Alloc_Custom_Actor(0, &frame, 0, 0, 200);
		xactor_Set_Actor_User_Function(back, begin_dirty_frame);
		xactor_Set_Actor_Draw_Function(back, draw_background);
		Actor* erase = xactcust_Alloc_Custom_Actor(0, &frame, 0, 0, -200);
		xactor_Set_Actor_User_Function(erase, erase_frame);
		xactor_Set_Actor_Draw_Function(erase, draw_close);
	} else {
		g_intro1Film = xfilm_Res_Callback_Film(resource, "intro1_f", &frame, 0, 0, 0, film_callback);
		xfilm_Set_Film_Def_Palette(g_intro1Film, shell->standardPalette);
	}
	xview_Set_View_Update_Function(end_view);
	Intro1_OpenMusic(resource, g_intro1Film);
	if (xcursor_Is_Cursor_Visible())
		xcursor_Hide_Cursor();
	XwIntro1_RunView(resource);
}

void Dos94_Intro1_ReleaseBackground(void) {
	if (!background)
		return;
	Rect frame;
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xmemhdl_Free_Handle(background);
	background = 0;
}
