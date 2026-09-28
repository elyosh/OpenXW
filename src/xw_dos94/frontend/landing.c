#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"
#include "xw_dos94/frontend/scenes.h"
#include "xw_runtime/integration/cutscene_callbacks.h"
#include "xw_runtime/runtime/scene_view_task.h"
#include <landru/actback.h>
#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/io.h>
#include <landru/view.h>
#include <landru/viewadd.h>
#include <stdio.h>

static ResFile* resource;
static Film* film;
static Actor* back_z0;
static Actor* back_z5;
static Rect dirty, previous_dirty;
static int slow;

/* DOS94 0x480596. */
static void end_view(int time) {
	int scene = shellext_Get_Cur_Scene();
	int16_t next, section, exit;
	(void)time;
	switch (scene) {
		case 255:
		case 256:
		case 257:
		case 258:
			next = scene + 85;
			section = shipext_Get_Pending_Tour_Cutscene();
			break;
		case 293:
		case 294:
		case 295:
		case 297:
			next = section = 115;
			break;
		case 303:
			next = 320;
			section = 116;
			break;
		case 304:
			next = 321;
			section = 116;
			break;
		case 305:
			next = 322;
			section = 116;
			break;
		case 307:
			next = 323;
			section = 116;
			break;
		default:
			return;
	}
	if (shellext_Check_Scene_Exit(&exit, next, section, film->cur_cel == film->cels))
		xerror_Set_Landru_Exit(exit);
}

/* DOS94 0x480742 / 0x481a6a. */
static void sound_cue(Actor* actor, int time) {
	(void)time;
	if (!ShellPreferences_GetSfxEnabled())
		return;
	if (actor->var2 == 1)
		soundext_Play_SFX(12);
	else if (actor->var2 == 2)
		soundext_Play_SFX(8);
}

/* DOS94 0x4807aa: back actors receive unclipped bounds. */
static void dirty_bounds(Actor* actor, int time) {
	Rect bounds, frame;
	(void)time;
	if (!xactor_Is_Actor_Visible(actor))
		return;
	xactor_Get_Actor_Rect(actor, &bounds);
	xactor_Get_Actor_Frame(actor, &frame);
	if (actor->zplane == 0)
		xactback_Set_Back_Actor_Rect(back_z0, &bounds);
	else if (actor->zplane == 5)
		xactback_Set_Back_Actor_Rect(back_z5, &bounds);
	xcanvas_Clip_Rect_To_Canvas(&bounds);
	xrect_Clip_Rect(&bounds, &frame);
	xrect_Enclose_Rect(&dirty, &bounds);
}

/* DOS94 0x480674. The native port uses the full-memory actor set. */
static int16_t film_callback(Film* loading, FilmObject* object) {
	if (object->id == FTC_ACTOR) {
		xfilm_Rewind_Actor_Film(loading, object, object + 1);
		Actor* actor = object->object;
		if (actor->var1 == 10 && slow)
			xactor_Set_Actor_User_Function(actor, dirty_bounds);
		else if (actor->var1 == 20 && slow)
			xactor_Non_Refreshable_Actor(actor);
		else if (actor->var1 == 50)
			xactor_Set_Actor_User_Function(actor, sound_cue);
	}
	return 0;
}

/* DOS94 0x480768. */
static void begin_frame(Actor* actor, int time) {
	(void)actor;
	if (time == 0)
		xcanvas_Get_Drawing_Canvas_Bounds(&previous_dirty);
	else
		previous_dirty = dirty;
	xrect_Clear_Rect(&dirty);
}

/* DOS94 0x480868. */
static void close_frame(Actor* actor, int time) {
	Rect frame;
	(void)time;
	if (film->cur_cel == film->cels) {
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

/* DOS94 0x4818da: reuse swirls without restarting it. */
static void open_music(void) {
	if (!ShellPreferences_GetMusicEnabled())
		return;
	Sound* music = xsound_Find_Gmid("swirls");
	if (!music) {
		ResFile* music_resource = xres_Open_Resource("ldmusic.lfd");
		music = xsound_Res_Music(music_resource, "swirls");
		soundext_Start_Resource_Sound(music);
		xres_Close_Resource(music_resource);
	}
	xsound_Set_Sound_Keep(music);
	xsound_Set_Sound_User_Function(music, Cutscene_IgnoreSoundEvent);
}

static void release_view(void) {
	/* DOS94 0x4819b8 retains swirls for the subsequent hangar scenes. */
	if (ShellPreferences_GetMusicEnabled() && xerror_Get_Landru_Exit() == 115) {
		Sound* music = xsound_Find_Gmid("swirls");
		if (music)
			soundext_FadeVolume(music, 0, 120);
	}
	xview_Clear_View_Update_Function();
	if (slow) {
		Rect frame;
		xview_Enable_All_View_Erase();
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		xview_Set_View_Frame(0, &frame);
		xview_Set_View_Pos(0, frame.left, frame.top);
	}
	xres_Close_Resource(resource);
	resource = NULL;
}

/* DOS94 0x480000. */
void Dos94_Landing(XwShellContext* shell) {
	int scene = shellext_Get_Cur_Scene();
	const char* stem;
	int bwing = 0;
	char name[9];
	Rect frame;
	switch (scene) {
		case 255:
			stem = "landa";
			break;
		case 256:
			stem = "landx";
			break;
		case 257:
			stem = "landy";
			break;
		case 258:
			stem = "landb";
			bwing = 1;
			break;
		case 293:
		case 303:
			stem = "nebla";
			break;
		case 294:
		case 304:
			stem = "neblx";
			break;
		case 295:
		case 305:
			stem = "nebly";
			break;
		case 297:
		case 307:
			stem = "neblb";
			bwing = 1;
			break;
		default:
			return;
	}
	resource = xres_Open_Resource("land.lfd");
	xrect_Set_Rect(&frame, 0, 0, 320, 200);
	slow = xio_Is_System_Slower_Than(2);
	if (slow) {
		back_z5 = xactback_Alloc_Back_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, 6);
		xactback_Alloc_Back_Actor_Buffer(back_z5, 12000);
		back_z0 = xactback_Alloc_Back_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, 6);
		xactback_Alloc_Back_Actor_Buffer(back_z0, 14000);
		xviewadd_Clear_View();
		xview_Disable_All_View_Erase();
		xcanvas_Invalid_Screen_Diff();
	}
	snprintf(name, sizeof name, "%s_%c", stem, slow ? 's' : 'f');
	ResFile* film_resource = bwing ? xres_Open_Resource("bwing.lfd") : resource;
	film = xfilm_Res_Callback_Film(film_resource, name, &frame, 0, 0, 0, film_callback);
	if (bwing)
		xres_Close_Resource(film_resource);
	if (slow) {
		Actor* actor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, 200);
		xactor_Set_Actor_User_Function(actor, begin_frame);
		actor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, -200);
		xactor_Set_Actor_User_Function(actor, close_frame);
		xactor_Set_Actor_Draw_Function(actor, XwCutscene_DrawCloseOnRefresh);
	}
	xfilm_Set_Film_Def_Palette(film, shell->standardPalette);
	xview_Set_View_Update_Function(end_view);
	if (xcursor_Is_Cursor_Visible())
		xcursor_Hide_Cursor();
	open_music();
	XwScene_RunView(NULL, release_view);
}
