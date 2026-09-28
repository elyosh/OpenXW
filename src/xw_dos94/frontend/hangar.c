#include "xw/audio/lolevel.h"
#include "xw/audio/soundext.h"
#include "xw/flight/player/user.h"
#include "xw/flight/xw.h"
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
#include <landru/film.h>
#include <landru/io.h>
#include <landru/view.h>
#include <landru/viewadd.h>
#include <stdlib.h>

/* Hangar1 and Hangar2 are mutually exclusive overlay scenes. */
static ResFile* resource;
static Film* film;
static LandruHandle background;
static Rect dirty, previous_dirty;
static Actor* companion;
static Sound* intro_music;
static Sound* speech[2];
static int hangar_number, hull_damage;

static bool return_scene(int scene) {
	return (scene >= 320 && scene <= 323) || (scene >= 330 && scene <= 333) || (scene >= 340 && scene <= 343);
}

/* DOS94 0x482674 / 0x4839fa: Hangar2 passes the actor's frame directly. */
static int16_t stamp_background(Actor* actor) {
	Rect bounds, clip, saved_clip;
	uint8_t* saved_pixels;
	int16_t saved_width, saved_height, result = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&bounds);
	uint8_t* pixels = xmemhdl_Lock_Handle(background);
	xcanvas_Push_Canvas(&saved_pixels, pixels, &saved_clip, &saved_width, &saved_height, 320, 200, 0);
	if (actor->draw) {
		clip = actor->frame;
		if (hangar_number == 1)
			xcanvas_Clip_Rect_To_Canvas(&clip);
		xcanvas_Set_Drawing_Canvas_Clip(&clip);
		result = actor->draw(actor, &bounds, &clip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(saved_pixels, &saved_clip, saved_width, saved_height);
	xmemhdl_Unlock_Handle(background);
	return result;
}

/* DOS94 0x482d52 / 0x484212. */
static void sound_action(int action) {
	if (!ShellPreferences_GetSfxEnabled())
		return;
	if (hangar_number == 2) {
		if (action == 2)
			soundext_Play_SFX(23);
		return;
	}
	switch (action) {
		case 1:
			soundext_Play_SFX(19);
			break;
		case 2:
			soundext_Play_SFX(18);
			break;
		case 3:
			soundext_Play_SFX(20);
			break;
		case 4:
			if (speech[0])
				soundext_Start_Resource_SFX(speech[0]);
			break;
		case 5:
			if (speech[1])
				soundext_Start_Resource_SFX(speech[1]);
			break;
		case 9:
			soundext_Fade_SFX(19, 0, 120);
			break;
		case 10:
			soundext_Fade_SFX(18, 0, 30);
			break;
		case 11:
			soundext_Stop_SFX(20);
			break;
	}
}

/* DOS94 0x482760 / 0x483ac8. */
static void update_sound(Actor* actor, int32_t time) {
	(void)time;
	if (hangar_number == 2 && return_scene(shellext_Get_Cur_Scene()) && !hull_damage && actor->var2 == 2)
		return;
	if (actor->var2)
		sound_action(actor->var2);
}

/* DOS94 0x48282e / 0x483be6. */
static void update_actor(Actor* actor, int32_t time) {
	Rect bounds, frame;
	if (xactor_Is_Actor_Visible(actor)) {
		xactor_Get_Actor_Rect(actor, &bounds);
		xactor_Get_Actor_Frame(actor, &frame);
		xcanvas_Clip_Rect_To_Canvas(&bounds);
		xrect_Clip_Rect(&bounds, &frame);
		xrect_Enclose_Rect(&dirty, &bounds);
	} else if (hangar_number == 2)
		return;
	if (hangar_number == 1) {
		if (actor->var2)
			film->var1 = 1;
		return;
	}
	if (actor->var2 != 1)
		return;
	if (!time) {
		actor->id = 2;
		xactor_Set_Actor_State(actor, 1, 0);
		if (companion)
			xactor_Hide_Actor(companion);
		return;
	}
	--actor->id;
	if (actor->id > 0)
		return;
	if (actor->id == 0) {
		if (companion) {
			xactor_Show_Actor(companion);
			xactor_Set_Actor_State(companion, 0, 0);
		}
		sound_action(2);
	} else if (actor->id > -7) {
		xactor_Set_Actor_State(actor, actor->state ^ 1, 0);
		if (companion)
			xactor_Set_Actor_State(companion, companion->state + 1, 0);
	} else if (actor->id == -7) {
		if (companion)
			xactor_Hide_Actor(companion);
		xactor_Set_Actor_State(actor, 1, 0);
		actor->id = (uint16_t)film->cur_cel < (uint16_t)(film->cels - 12) ? (rand() & 15) + 3 : 99;
	}
}

/* DOS94 0x4825f2 / 0x4838ea. Filtering follows role setup and background stamping. */
static int16_t film_callback(Film* loading, FilmObject* object) {
	int16_t consumed = 0;
	if (object->id != FTC_ACTOR)
		return 0;
	xfilm_Rewind_Actor_Film(loading, object, object + 1);
	Actor* actor = object->object;
	switch (actor->var1) {
		case 10:
			xactor_Set_Actor_User_Function(actor, update_actor);
			if (hangar_number == 2 && actor->var2 == 2)
				companion = actor;
			break;
		case 20:
			stamp_background(actor);
			consumed = 1;
			break;
		case 50:
			xactor_Set_Actor_User_Function(actor, update_sound);
			break;
	}
	if (hangar_number == 2 && return_scene(shellext_Get_Cur_Scene()) && actor->var2 > 9) {
		if (!hull_damage) {
			if (actor->var2 != 20)
				consumed = 1;
		} else if (!((actor->var2 >= 10 && actor->var2 <= 9 + hull_damage) ||
					 actor->var2 == 20 + hull_damage))
			consumed = 1;
	}
	return consumed;
}

/* DOS94 0x482786 / 0x483b2c. */
static void begin_frame(Actor* actor, int32_t time) {
	(void)actor;
	if (!time)
		xcanvas_Get_Drawing_Canvas_Bounds(&previous_dirty);
	else
		previous_dirty = dirty;
	xrect_Clear_Rect(&dirty);
}

/* DOS94 0x4827c8 / 0x483b6e. */
static int16_t draw_background(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	(void)actor;
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (!refresh)
		return 0;
	if (hangar_number == 1 || (uint16_t)film->cur_cel < (uint16_t)film->cels) {
		const uint8_t* pixels = xmemhdl_Lock_Handle(background);
		stub_Copy_From_Clipped_Buffer(pixels, &previous_dirty, previous_dirty.left, previous_dirty.top, 320,
									  200);
		xmemhdl_Unlock_Handle(background);
	}
	return 1;
}

/* DOS94 0x4828bc / 0x483de0. */
static void close_frame(Actor* actor, int32_t time) {
	(void)time;
	Rect frame;
	bool full = hangar_number == 1 ? film->cur_cel == film->cels || film->var1 != 0
								   : (uint16_t)(film->cels - 2) < (uint16_t)film->cur_cel;
	if (full) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		if (hangar_number == 1)
			film->var1 = 0;
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
	if (hangar_number == 2)
		xcanvas_Invalid_Screen_Diff();
}

static int16_t draw_close(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (!refresh)
		return 0;
	if (actor->var1)
		xcanvas_Erase_Canvas();
	if (hangar_number == 2)
		xcanvas_Invalid_Screen_Diff();
	return 1;
}

/* DOS94 0x4825ac / 0x483580. */
static void end_view(int32_t time) {
	(void)time;
	int16_t next, section, exit_scene;
	int scene = shellext_Get_Cur_Scene();
	if (hangar_number == 1) {
		next = 14;
		section = 26;
	} else if (scene == 16) {
		next = 17;
		section = 26;
	} else if ((scene >= 260 && scene <= 262) || scene == 266) {
		next = scene + 3 - (scene == 266 ? 2 : 0);
		section = 2;
	} else if ((scene >= 270 && scene <= 272) || scene == 276) {
		next = scene + 3 - (scene == 276 ? 2 : 0);
		section = 3;
	} else if (scene == 281) {
		next = 282;
		section = 4;
	} else if (scene >= 283 && scene <= 285) {
		next = scene + 3;
		section = 4;
	} else if (scene >= 320 && scene <= 323)
		next = section = 116;
	else if (scene >= 330 && scene <= 333)
		next = section = 115;
	else if (scene >= 340 && scene <= 343) {
		section = shipext_Get_Pending_Tour_Cutscene();
		next = shipext_Get_Pending_Tour_Cutscene();
	} else
		return;
	if (shellext_Check_Scene_Exit(&exit_scene, next, section, film->cur_cel == film->cels))
		xerror_Set_Landru_Exit(exit_scene);
}

/* DOS94 0x482b02. */
static void hangar1_music(Sound* sound, int32_t time) {
	(void)sound;
	(void)time;
	if (!intro_music || film->cur_cel != 1)
		return;
	soundext_FadeVolume(intro_music, 0, 2100);
	ResFile* music = xres_Open_Resource("hr1music.lfd");
	Sound* hangar = xsound_Res_Music(music, "hangar");
	xres_Close_Resource(music);
	soundext_Start_Resource_Sound(hangar);
	soundext_SetHook(hangar, 0, 1, 0);
	soundext_ClearTriggers();
	soundext_SetTriggerContext(hangar, 1);
	soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_SET_SPEED, hangar->id, 130, 0, 0, 0, 0);
	soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_SET_HOOK, hangar->id, 0, 4, 0, 0, 0);
	soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_END, 0, 0, 0, 0, 0, 0);
	xsound_Set_Sound_Keep(hangar);
}

/* DOS94 0x482998; both original position queries use selector 7. */
static void open_hangar1_music(void) {
	if (!ShellPreferences_GetMusicEnabled())
		return;
	intro_music = xsound_Find_Gmid("inattack");
	if (!intro_music) {
		ResFile* music = xres_Open_Resource("hr1music.lfd");
		Sound* hangar = xsound_Res_Music(music, "hangar");
		xres_Close_Resource(music);
		soundext_Start_Resource_Sound(hangar);
		soundext_SetHook(hangar, 0, 4, 0);
		xsound_Set_Sound_Keep(hangar);
		return;
	}
	lolevel_ImPause();
	int16_t beat = soundext_GetMusicParam(intro_music, 7, 0);
	int16_t tick = soundext_GetMusicParam(intro_music, 7, 0);
	lolevel_ImResume();
	if (beat < 227)
		soundext_JumpMidi(intro_music, 0, 227, tick);
	xsound_Set_Sound_User_Function(intro_music, hangar1_music);
}

/* DOS94 0x483eaa. */
static void open_hangar2_music(void) {
	if (!ShellPreferences_GetMusicEnabled())
		return;
	const char* name = "hangar";
	const char* filename = "hr1music.lfd";
	int scene = shellext_Get_Cur_Scene(), group = 1, beat = 70;
	/* The original swirls branch leaves this pointer uninitialized. */
	Sound* previous = NULL;
	if ((scene >= 260 && scene <= 262) || scene == 266) {
		previous = xsound_Find_Gmid("rebels");
		name = "launch";
		filename = "hr2music.lfd";
		group = 2;
	} else if ((scene >= 270 && scene <= 272) || scene == 276 || scene == 281 ||
			   (scene >= 283 && scene <= 285)) {
		previous = xsound_Find_Gmid("adrift");
		name = "launch";
		filename = "hr2music.lfd";
	} else if (return_scene(scene)) {
		name = "swirls";
		filename = "ldmusic.lfd";
		beat = 24;
	}
	Sound* music = xsound_Find_Gmid(name);
	if (!music) {
		ResFile* archive = xres_Open_Resource(filename);
		music = xsound_Res_Music(archive, name);
		xres_Close_Resource(archive);
		soundext_Start_Resource_Sound(music);
		if (previous) {
			soundext_ShareParts(previous, music);
			int tick = soundext_GetMusicParam(previous, 8, 0);
			soundext_ScanMidi(music, 0, 4, tick);
			soundext_JumpMidi(previous, group, 0, 0);
			soundext_FadeVolume(music, 127, 180);
		} else
			soundext_ScanMidi(music, 0, beat, 240);
	}
	xsound_Set_Sound_Keep(music);
}

static void release_scene(void) {
	xview_Clear_View_Update_Function();
	xmemhdl_Free_Handle(background);
	background = 0;
	xres_Close_Resource(resource);
	resource = NULL;
}

static void finish_scene(void) {
	Rect frame;
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xmemhdl_Free_Handle(background);
	background = 0;
	if (hangar_number == 2) {
		if (ShellPreferences_GetMusicEnabled() && return_scene(shellext_Get_Cur_Scene())) {
			Sound* music = xsound_Find_Gmid("swirls");
			if (music)
				soundext_FadeVolume(music, 0, 120);
		}
	}
	if (hangar_number == 2)
		soundext_ResetEnabledSfxCache();
}

/* Common original background/close actor setup, DOS94 0x48244f / 0x483403. */
static void create_view_actors(Rect* frame) {
	Actor* back = xactcust_Alloc_Custom_Actor(0, frame, 0, 0, 200);
	xactor_Set_Actor_User_Function(back, begin_frame);
	xactor_Set_Actor_Draw_Function(back, draw_background);
	Actor* close = xactcust_Alloc_Custom_Actor(0, frame, 0, 0, -200);
	xactor_Set_Actor_User_Function(close, close_frame);
	xactor_Set_Actor_Draw_Function(close, draw_close);
}

/* DOS94 0x48236e. Modern hosts use the full-memory film. */
void Dos94_Hangar1(XwShellContext* shell) {
	hangar_number = 1;
	xfade_AddTimedText("On board the Rebel Flagship Independence.", 3, 80, 0, 50, 180, 16);
	resource = xres_Open_Resource("hanger1.lfd");
	Rect frame;
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	background = xmemhdl_Alloc_Clear_Handle(64000, LANDRU_MEMORY_RESOURCE);
	film = xfilm_Res_Callback_Film(resource, "hang1_f", &frame, 0, 0, 0, film_callback);
	xfilm_Set_Film_Def_Palette(film, shell->standardPalette);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	create_view_actors(&frame);
	xview_Set_View_Update_Function(end_view);
	if (xcursor_Is_Cursor_Visible())
		xcursor_Hide_Cursor();
	open_hangar1_music();
	/* Keep cue sounds available to the shared Hangar1 film callbacks. */
	speech[0] = speech[1] = NULL;
	if (ShellPreferences_GetSfxEnabled()) {
		static const int sounds[] = { 19, 18, 20, 22, 23, 24 };
		for (int i = 0; i < 6; ++i)
			soundext_LoadSfx(sounds[i], 0, NULL, sounds[i] == 19, sounds[i] == 24);
		speech[0] = soundext_LoadSpeech(25, 0, NULL, 0);
		speech[1] = soundext_LoadSpeech(22, 0, NULL, 0);
	}
	XwScene_RunView(finish_scene, release_scene);
}

/* DOS94 0x482e1e: the original switch's complete set of film alternatives. */
void Dos94_Hangar2(XwShellContext* shell) {
	static const struct {
		int scene;
		const char* slow;
		const char* fast;
		bool bwing;
	} films[] = { { 16, "hang2x_s", "hang2x_f", false },  { 283, "hang2x_s", "hang2x_f", false },
				  { 260, "neb2x_s", "neb2x_f", false },   { 270, "neb2x_s", "neb2x_f", false },
				  { 261, "neb2y_s", "neb2y_f", false },   { 271, "neb2y_s", "neb2y_f", false },
				  { 262, "neb2a_s", "neb2a_f", false },   { 272, "neb2a_s", "neb2a_f", false },
				  { 266, "neb2b_s", "neb2b_f", true },    { 276, "neb2b_s", "neb2b_f", true },
				  { 281, "hang2b_s", "hang2b_f", true },  { 284, "hang2y_s", "hang2y_f", false },
				  { 285, "hang2a_s", "hang2a_f", false }, { 320, "nrepa_s", "nrepa_f", false },
				  { 330, "nrepa_s", "nrepa_f", false },   { 321, "nrepx_s", "nrepx_f", false },
				  { 331, "nrepx_s", "nrepx_f", false },   { 322, "nrepy_s", "nrepy_f", false },
				  { 332, "nrepy_s", "nrepy_f", false },   { 323, "nrepb_s", "nrepb_f", true },
				  { 333, "nrepb_s", "nrepb_f", true },    { 340, "repa_s", "repa_f", false },
				  { 341, "repx_s", "repx_f", false },     { 342, "repy_s", "repy_f", false },
				  { 343, "repb_s", "repb_f", true } };

	hangar_number = 2;
	hull_damage = g_playerFlightState.missionExitHullDamageQuarter;
	companion = NULL;
	resource = xres_Open_Resource("hanger2.lfd");
	Rect frame;
	xrect_Set_Rect(&frame, 0, 0, 320, 200);
	background = xmemhdl_Alloc_Clear_Handle(64000, LANDRU_MEMORY_RESOURCE);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	int scene = shellext_Get_Cur_Scene();
	for (size_t i = 0; i < sizeof films / sizeof films[0]; ++i) {
		if (films[i].scene != scene)
			continue;
		ResFile* source = films[i].bwing ? xres_Open_Resource("bwing.lfd") : resource;
		const char* name = xio_Is_System_Slower_Than(1) ? films[i].slow : films[i].fast;
		film = xfilm_Res_Callback_Film(source, name, &frame, 0, 0, 0, film_callback);
		if (films[i].bwing)
			xres_Close_Resource(source);
		break;
	}
	xfilm_Set_Film_Def_Palette(film, shell->standardPalette);
	create_view_actors(&frame);
	open_hangar2_music();
	if (ShellPreferences_GetSfxEnabled())
		soundext_LoadSfx(23, 0, NULL, 0, 0);
	xview_Set_View_Update_Function(end_view);
	if (xcursor_Is_Cursor_Visible())
		xcursor_Hide_Cursor();
	XwScene_RunView(finish_scene, release_scene);
}
