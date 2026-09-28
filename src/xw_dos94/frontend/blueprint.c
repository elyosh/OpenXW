#include "xw/audio/soundext.h"
#include "xw/frontend/blueprnt.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"
#include "xw_dos94/frontend/scenes.h"
#include "xw_runtime/runtime/scene_view_task.h"
#include <landru/actanim.h>
#include <landru/actdelt.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/font.h>
#include <landru/inpattr.h>
#include <landru/io.h>
#include <landru/paint.h>
#include <landru/pal.h>
#include <landru/paragrp.h>
#include <landru/style.h>
#include <landru/view.h>
#include <landru/viewadd.h>
#include <stdlib.h>
#include <string.h>

static Palette* initial_palette;
static Palette* simple_palette;
static Palette* hologram_palettes[4];
static Actor* bwing;
static Actor* simple_ships[2];
static int simple_palette_active;

/* DOS94 g_blueprintFrameMap at 0x384008. Only these two fields have consumers. */
static const struct {
	int16_t base, animated_component;
} frames[15] = { { 0, 7 },   { 16, 9 },  { 33, 7 },  { 49, -1 }, { 56, -1 },
				 { 63, -1 }, { 72, -1 }, { 81, -1 }, { 82, -1 }, { 83, -1 },
				 { 84, -1 }, { 85, -1 }, { 86, -1 }, { 87, -1 }, { -1, -1 } };

/* DOS94 0x381d8c. */
static int component_frame(int ship, int component, int phase) {
	int first = frames[ship].base + component + 1;
	int animated = frames[ship].animated_component;
	if (animated == -1 || component < animated)
		return first;
	return component == animated ? first + (phase & 3) : first + 3;
}

/* DOS94 0x382174/0x38232c, using the native canvas pitch. */
static void remap_rect(const Rect* rect, unsigned int map) {
	Rect clipped = *rect, raster;
	xcanvas_Get_Raster_Clip(&raster);
	xrect_Clip_Rect(&clipped, &raster);
	if (xrect_Empty_Rect(&clipped))
		return;
	uint8_t* pixels = xcanvas_Get_Draw_Buffer();
	int stride = xcanvas_Get_Current_Canvas_Bitmap()->w;
	const uint8_t* colors = g_blueprintColorTable + 256 * map;
	for (int y = clipped.top; y < clipped.bottom; ++y)
		for (int x = clipped.left; x < clipped.right; ++x)
			pixels[y * stride + x] = colors[pixels[y * stride + x]];
}

static int remap_phase(int value) {
	int phase = abs(value % 6);
	return phase > 3 ? 6 - phase : phase;
}

/* DOS94 0x381166. Palette cycling changes authored ramps, not animation cels. */
static void update_hologram(Actor* actor, int32_t time) {
	if (!time)
		xactor_Show_Actor(actor);
	switch (g_blueprintDisplayPhase) {
		case 1:
			if (g_blueprintPhaseTick == 36) {
				g_blueprintDisplayPhase = 2;
				g_blueprintPhaseTick = 0;
			} else
				++g_blueprintPhaseTick;
			break;
		case 2:
		case 3:
			if (++g_blueprintPhaseTick == 16) {
				g_blueprintDisplayPhase = 5;
				g_blueprintPhaseTick = 0;
			}
			break;
		case 5:
			if (++g_blueprintPhaseTick == 12) {
				g_blueprintDisplayPhase = 6;
				g_blueprintPhaseTick = 0;
			}
			g_blueprintDisplayedShip = g_blueprintSelectedShip;
			g_blueprintDisplayedComponent = g_blueprintSelectedComponent;
			break;
		case 6:
			g_blueprintPhaseTick = (int16_t)(g_blueprintTextRevealTick >> 1);
			if (g_blueprintPhaseTick > 3)
				g_blueprintPhaseTick = 3;
			actor->var2 = 1;
			++g_blueprintTextRevealTick;
			g_blueprintDisplayedShip = g_blueprintSelectedShip;
			g_blueprintDisplayedComponent = g_blueprintSelectedComponent;
			break;
	}
	if (g_blueprintDisplayPhase <= 4 || g_blueprintPhaseTick <= 1)
		return;
	if (g_blueprintDisplayedShip >= g_blueprintDetailedShipCount) {
		if (simple_palette_active)
			return;
		xpal_Screen_To_Dest_Palette(0, 0, 255);
		xpal_Set_Dest_Palette(simple_palette);
		xfade_Start_Color_Fade(1, 1, 0, 0, 0);
		simple_palette_active = 1;
	} else {
		int index = (time >> 1) & 7;
		if (index > 3)
			index = 7 - index;
		if ((time & 1) && !simple_palette_active)
			return;
		xpal_Screen_To_Dest_Palette(0, 0, 255);
		xpal_Set_Dest_Palette(hologram_palettes[index]);
		xfade_Start_Color_Fade(1, 1, 0, 0, 0);
		simple_palette_active = 0;
	}
}

/* DOS94 0x381714..0x38186a and 0x381b71..0x381d6c. */
static void draw_description(int ship, int x, int y, bool detailed) {
	char text[64];
	int paragraph = detailed ? ship + 2 : 1;
	int base = 5 * (detailed ? g_blueprintDisplayedComponent : ship - g_blueprintDetailedShipCount);
	int count = 4;
	if (detailed) {
		count = 0;
		for (int i = 1; i < 5; ++i) {
			xparagrp_Get_Paragraph_String(g_blueprintText, text, paragraph, base + i);
			if (strlen(text) > 1)
				++count;
		}
	}
	int text_x = x + (detailed ? 151 : 76), text_y = y + (detailed ? 106 : 111);
	int height = 3 * ((int16_t)g_blueprintTextRevealTick + 2);
	int maximum = detailed ? 9 * count - 3 : 32;
	if (height > maximum)
		height = maximum;
	Rect rect;
	xrect_Set_Rect(&rect, text_x - 3, text_y - 2, text_x + (detailed ? 109 : 166), text_y + height + 3);
	xpaint_Frame_Clipped_Rect(&rect, 16);
	xrect_Inset_Rect(&rect, 1, 1);
	remap_rect(&rect, remap_phase(g_blueprintTextRevealTick + 3));
	xfont_Enable_FontID_Shadow(1);
	for (int i = 1, bias = 6; i <= count; ++i, bias -= 3, text_y += 9) {
		xparagrp_Get_Paragraph_String(g_blueprintText, text, paragraph, base + i);
		if (detailed && strlen(text) <= 1)
			continue;
		int16_t color = ((int16_t)g_blueprintTextRevealTick + bias) * 4;
		if (color > 47)
			color = 47;
		if (bias == 6)
			color = 61;
		if (color >= 24)
			xfont_Print_Clipped_Text(text, text_x, text_y, 1, color);
	}
	xfont_Disable_FontID_Shadow(1);
}

/* DOS94 0x38186d..0x381b68. */
static void draw_ship(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	int ship = g_blueprintDisplayedShip;
	if (ship >= g_blueprintDetailedShipCount) {
		int simple = ship - g_blueprintDetailedShipCount;
		if (simple >= 2) {
			xactor_Set_Actor_State(actor, frames[simple + 5].base, 0);
			xactanim_Draw_Anim_Actor(actor, frame, clip, x - 28, y - 8, refresh);
		} else {
			xactdelt_Draw_Delta_Actor(simple_ships[simple], frame, clip, x + (simple ? 112 : 92),
									  y + (simple ? 52 : 66), refresh);
		}
		char text[64];
		Rect title;
		xfont_Enable_FontID_Shadow(0);
		xrect_Set_Rect(&title, x + 56, y + 40, x + 260, y + 52);
		xparagrp_Get_Paragraph_String(g_blueprintText, text, 1, 5 * simple);
		xfont_Print_Centered_Text(text, &title, 0, 61);
		xfont_Disable_FontID_Shadow(0);
		return;
	}
	Actor* image = ship == 3 && bwing ? bwing : actor;
	int index = ship - (bwing && ship > 3);
	xactor_Set_Actor_State(image, image == bwing ? 0 : frames[index].base, 0);
	xactanim_Draw_Anim_Actor(image, frame, clip, x, y, refresh);
	if (!actor->var2)
		return;
	if (image == bwing) {
		int component = g_blueprintDisplayedComponent + 1;
		xactor_Set_Actor_State(image, component, component);
	} else {
		int phase = g_blueprintDisplayPhase == 6 ? g_blueprintPhaseTick : 0;
		xactor_Set_Actor_State(image, component_frame(index, g_blueprintDisplayedComponent, phase), 0);
	}
	xactanim_Draw_Anim_Actor(image, frame, clip, x, y, refresh);
}

/* DOS94 0x38138e. */
static int16_t draw_hologram(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	if (!refresh)
		return 0;
	Rect aperture;
	bool edges = false, content = false, descriptions = false;
	switch (g_blueprintDisplayPhase) {
		case 1:
			if (g_blueprintPhaseTick == 36) {
				g_blueprintDisplayPhase = 2;
				g_blueprintPhaseTick = 0;
				xpal_Screen_To_Dest_Palette(0, 0, 255);
				xpal_Set_Dest_Palette(initial_palette);
				xfade_Start_Color_Fade(5, 1, 16, 0, 0);
			} else
				++g_blueprintPhaseTick;
			break;
		case 2:
		case 3:
			if (g_blueprintPhaseTick & 4) {
				Rect border = *frame;
				xrect_Inset_Rect(&border, 96, 94);
				xpaint_Frame_Clipped_Rect(&border, 60);
				xfont_Enable_FontID_Shadow(0);
				xfont_Print_Centered_Text(g_blueprintDisplayPhase == 2 ? "Accessing Schematics"
																	   : "Accessing Holograms",
										  frame, 0, 14);
				xfont_Disable_FontID_Shadow(0);
			}
			break;
		case 5:
			xrect_Set_Rect(&aperture, 56, 97, 260, 99);
			if (g_blueprintPhaseTick > 2)
				xrect_Inset_Rect(&aperture, 0, (2 - g_blueprintPhaseTick) * 8);
			if (aperture.top < 40)
				aperture.top = 40;
			if (aperture.bottom > 156)
				aperture.bottom = 156;
			edges = content = true;
			break;
		case 6:
			xrect_Set_Rect(&aperture, 56, 40, 260, 156);
			content = descriptions = true;
			break;
	}
	if (!content)
		return 0;
	if (!xio_Is_System_Slower_Than(2))
		remap_rect(&aperture, remap_phase(edges ? g_blueprintPhaseTick / 2 : g_blueprintTextRevealTick >> 1));
	if (edges) {
		xpaint_Horiz_Clipped_Line(aperture.left, aperture.top, aperture.right - aperture.left, 60);
		xpaint_Horiz_Clipped_Line(aperture.left, aperture.bottom - 1, aperture.right - aperture.left, 60);
	}
	xrect_Inset_Rect(&aperture, 0, 1);
	xrect_Clip_Rect(&aperture, clip);
	if (xrect_Empty_Rect(&aperture))
		return 0;
	xcanvas_Set_Drawing_Canvas_Clip(&aperture);
	bool detailed = g_blueprintDisplayedShip < g_blueprintDetailedShipCount;
	if (!detailed)
		draw_description(g_blueprintDisplayedShip, x, y, false);
	draw_ship(actor, frame, &aperture, x, y, refresh);
	if (detailed && descriptions)
		draw_description(g_blueprintDisplayedShip, x, y, true);
	return 0;
}

/* DOS94 0x380d40. */
static void select_blueprint(Input* input, int time) {
	int16_t shipChanged = 0;
	(void)time;
	if (xinpattr_Get_Input_Selected(input) != 0) {
		switch (input->id) {
			case BP_PREVIOUS_SHIP:
				if (g_blueprintSelectedShip != 0)
					--g_blueprintSelectedShip;
				else
					g_blueprintSelectedShip =
						xparagrp_Count_Paragraph_Strings(g_blueprintText, BP_SHIP_NAME_PARAGRAPH) - 1;
				shipChanged = 1;
				g_blueprintSelectedComponent = 0;
				break;
			case BP_NEXT_SHIP:
				if (g_blueprintSelectedShip ==
					xparagrp_Count_Paragraph_Strings(g_blueprintText, BP_SHIP_NAME_PARAGRAPH) - 1)
					g_blueprintSelectedShip = 0;
				else
					++g_blueprintSelectedShip;
				shipChanged = 1;
				g_blueprintSelectedComponent = 0;
				break;
			case BP_PREVIOUS_COMPONENT:
				if (g_blueprintSelectedShip < g_blueprintDetailedShipCount) {
					if (g_blueprintSelectedComponent != 0)
						--g_blueprintSelectedComponent;
					else
						g_blueprintSelectedComponent =
							xparagrp_Count_Paragraph_Strings(
								g_blueprintText, g_blueprintSelectedShip + BP_COMPONENT_PARAGRAPH_BASE) /
								BP_COMPONENT_STRING_COUNT -
							1;
				}
				break;
			case BP_NEXT_COMPONENT:
				if (g_blueprintSelectedShip < g_blueprintDetailedShipCount) {
					if (g_blueprintSelectedComponent ==
						xparagrp_Count_Paragraph_Strings(g_blueprintText, g_blueprintSelectedShip +
																			  BP_COMPONENT_PARAGRAPH_BASE) /
								BP_COMPONENT_STRING_COUNT -
							1)
						g_blueprintSelectedComponent = 0;
					else
						++g_blueprintSelectedComponent;
				}
				break;
		}
		xinpattr_Refresh_Input(g_blueprintControlsInput);
		if (shipChanged) {
			int phase = g_blueprintDisplayPhase;
			if (phase == 5 || phase == 6) {
				bool was_detailed = g_blueprintDisplayedShip < g_blueprintDetailedShipCount;
				bool is_detailed = g_blueprintSelectedShip < g_blueprintDetailedShipCount;
				g_blueprintDisplayPhase = was_detailed != is_detailed ? (is_detailed ? 3 : 2) : 5;
				g_blueprintPhaseTick = 0;
			}
		}
		g_blueprintTextRevealTick = 0;
	}
}

/* DOS94 0x38109e. */
static void update_door(Actor* actor, int time) {
	int16_t needsRefresh = 1;
	if (time == 0)
		actor->var1 = 0;
	if (time >= BP_DOOR_ANIMATION_START_TIME) {
		if (actor->var1 == 0) {
			if (actor->state == actor->arraySize - 1)
				blueprnt_HandleSoundAction(BP_SOUND_DOOR_CLOSE);
			if (actor->state != 0)
				xactor_Set_Actor_State(actor, actor->state - 1, 0);
			else
				needsRefresh = 0;
		} else {
			if (actor->state == 0)
				blueprnt_HandleSoundAction(BP_SOUND_DOOR_OPEN);
			if (actor->state != actor->arraySize - 1)
				xactor_Set_Actor_State(actor, actor->state + 1, 0);
			else
				needsRefresh = 0;
			actor->var1 = 0;
		}
	}
	if (needsRefresh != 0)
		xactor_Refresh_Actor(actor);
}

/* DOS94 0x381002. */
static void draw_label(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	char label[BP_LABEL_CAPACITY];
	(void)clip;
	if (refresh != 0) {
		xpaint_Paint_Clipped_Bevel(frame, 38, 26, 0, 1);
		if (input->id == BP_SHIP_NAME_INPUT) {
			xparagrp_Get_Paragraph_String(g_blueprintText, label, BP_SHIP_NAME_PARAGRAPH,
										  g_blueprintSelectedShip);
		} else if (g_blueprintSelectedShip >= g_blueprintDetailedShipCount) {
			memcpy(label, g_blueprintUnavailableLabel, sizeof(g_blueprintUnavailableLabel));
		} else {
			xparagrp_Get_Paragraph_String(g_blueprintText, label,
										  g_blueprintSelectedShip + BP_COMPONENT_PARAGRAPH_BASE,
										  g_blueprintSelectedComponent * BP_COMPONENT_STRING_COUNT);
		}
		xfont_Print_Centered_Text(label, frame, BP_LABEL_FONT, BP_LABEL_TEXT_COLOR);
	}
}

/* DOS94 0x38097e and 0x380a4c. */
static int16_t film_callback(Film* film, FilmObject* object) {
	if (object->id != FTC_ACTOR)
		return 0;
	xfilm_Rewind_Actor_Film(film, object, object + 1);
	Actor* actor = object->object;
	if (actor->var1 == 20) {
		if (actor->draw) {
			Rect frame = actor->frame;
			xcanvas_Clip_Rect_To_Canvas(&frame);
			xcanvas_Set_Drawing_Canvas_Clip(&frame);
			actor->draw(actor, &frame, &frame, actor->x, actor->y, 1);
			xcanvas_Max_Drawing_Canvas_Clip();
		}
		return 1;
	}
	if (actor->var1 == 10 || actor->var1 == 11) {
		actor->id = actor->var1 - 10;
		if (actor->id)
			g_blueprintRightDoorActor = actor;
		else
			g_blueprintLeftDoorActor = actor;
		xactor_Set_Actor_User_Function(actor, update_door);
	}
	return 0;
}

/* DOS94 0x380f98. */
static void draw_button(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	if (!refresh)
		return;
	PushButton* button = (PushButton*)input;
	xstyle_Style_Paint_Border(frame, button->pressed);
	xstyle_Style_Draw_Centered_Icon(input->id == 0 || input->id == 2 ? 1 : 3, frame, clip, button->pressed);
}

/* DOS94 0x3808ca. */
static void end_view(int32_t time) {
	if (!time && !xcursor_Is_Cursor_Visible())
		xcursor_Show_Cursor();
	int16_t key = xio_Get_Free_Key();
	if (key &&
		shellext_MoveGridFocus(&g_blueprintFocusIndex, g_blueprintFocusX, g_blueprintFocusY, 2, 4, key)) {
		xio_Set_Mouse_Position(g_blueprintFocusX[g_blueprintFocusIndex],
							   g_blueprintFocusY[g_blueprintFocusIndex]);
		xio_Get_Key();
	}
	if (time == 8)
		blueprnt_HandleSoundAction(3);
	if (time == 16)
		blueprnt_HandleSoundAction(4);
}

static void release_scene(void) {
	xview_Clear_View_Update_Function();
	xparagrp_Free_Paragraph(g_blueprintText);
	g_blueprintText = 0;
	xres_Close_Resource(g_blueprintResourceFile);
	g_blueprintResourceFile = NULL;
}

static void finish_scene(void) {
	/* DOS94 0x381f4a preserves waiting when entering the film room. */
	if (ShellPreferences_GetMusicEnabled() && xerror_Get_Landru_Exit() != 90) {
		if (g_blueprintWaitingMusic &&
			(uint8_t)soundext_Count_Resource_Instances(g_blueprintWaitingMusic) != 1) {
			if (g_blueprintHallMarchMusic)
				soundext_SetHook(g_blueprintHallMarchMusic, 0, 3, 0);
		} else {
			if (g_blueprintWaitingMusic) {
				soundext_SetPriority((intptr_t)g_blueprintWaitingMusic, 0);
				soundext_FadeVolume(g_blueprintWaitingMusic, 0, 300);
			}
			if (g_blueprintHallMarchMusic) {
				xsound_Clear_Sound_Keep(g_blueprintHallMarchMusic);
				xsound_Free_Sound(g_blueprintHallMarchMusic);
			}
		}
		soundext_ClearTriggers();
	}
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xio_Clear_Key_Buttons();
	if (xcursor_Is_Cursor_Visible())
		xcursor_Hide_Cursor();
}

/* DOS94 0x380000. */
void Dos94_Blueprint(XwShellContext* shell) {
	g_blueprintFocusIndex = 2;
	xio_Set_Mouse_Position(214, 170);
	Rect frame;
	/* The DOS B-Wing allocation receives an uninitialized frame; native clipping uses the scene bounds. */
	xrect_Set_Rect(&frame, 0, 0, 320, 200);
	bwing = NULL;
	g_blueprintDetailedShipCount = 7;
	if (shipext_IsTourAvailable(4)) {
		ResFile* extra = xres_Open_Resource("bwing.lfd");
		bwing = xactanim_Res_Anim_Actor(extra, "blueb", &frame, 0, 0, 0);
		xactor_Set_Actor_Time(bwing, 0, 0);
		bwing->id = 1;
		g_blueprintText = xparagrp_Res_Paragraph(extra, "blueprnt");
		xres_Close_Resource(extra);
		g_blueprintDetailedShipCount = 8;
	}
	ResFile* resource = xres_Open_Resource("blueprnt.lfd");
	g_blueprintResourceFile = resource;
	blueprnt_LoadColorTable(resource);
	if (!shipext_IsTourAvailable(4))
		g_blueprintText = xparagrp_Res_Paragraph(resource, "blueprnt");
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	g_blueprintFilm = xfilm_Res_Callback_Film(resource, "blueprnt", &frame, 0, 0, 0, film_callback);
	xfilm_Set_Film_Def_Palette(g_blueprintFilm, shell->standardPalette);
	initial_palette = xpal_Res_Palette(resource, "prints");
	simple_palette = xpal_Res_Palette(resource, "picts");
	static const char* const palette_names[] = { "bluepal1", "bluepal2", "bluepal3", "bluepal4" };
	for (int i = 0; i < 4; ++i)
		hologram_palettes[i] = xpal_Res_Palette(resource, palette_names[i]);
	g_blueprintBaseHologramActor = xactanim_Res_Anim_Actor(resource, "blueship", &frame, 0, 0, 0);
	xactor_Set_Actor_User_Function(g_blueprintBaseHologramActor, update_hologram);
	xactor_Set_Actor_Draw_Function(g_blueprintBaseHologramActor, draw_hologram);
	simple_ships[0] = xactdelt_Res_Delta_Actor(resource, "calblue", &frame, 0, 0, 0);
	xactor_Set_Actor_Time(simple_ships[0], 0, 0);
	simple_ships[1] = xactdelt_Res_Delta_Actor(resource, "nebblue", &frame, 0, 0, 0);
	xactor_Set_Actor_Time(simple_ships[1], 0, 0);
	g_blueprintDisplayPhase = 1;
	g_blueprintPhaseTick = g_blueprintTextRevealTick = simple_palette_active = 0;
	g_blueprintSelectedShip = g_blueprintDisplayedShip = 0;
	g_blueprintSelectedComponent = g_blueprintDisplayedComponent = 0;
	g_blueprintWorldInput = xinput_Alloc_Input(NULL, &frame, 0, 0);
	xinpattr_Refreshable_Input(g_blueprintWorldInput);
	const int doors[][4] = { { 0, 54, 62, 164 }, { 256, 40, 320, 200 } };
	Input** inputs[] = { &g_blueprintLeftDoorInput, &g_blueprintRightDoorInput };
	for (int i = 0; i < 2; ++i) {
		xrect_Set_Rect(&frame, doors[i][0], doors[i][1], doors[i][2], doors[i][3]);
		*inputs[i] = xinput_Alloc_Input(g_blueprintWorldInput, &frame, 0, 0);
		xinpattr_Set_Input_Update_Function(*inputs[i], blueprnt_iupdate_Blueprint_Door);
		xinpattr_Set_Input_User_Function(*inputs[i], blueprnt_iuser_Blueprint_Door);
		(*inputs[i])->mouseUsage = allInput;
		(*inputs[i])->id = i;
	}
	xrect_Set_Rect(&frame, 94, 163, 222, 197);
	g_blueprintDoorHintInput = xinput_Alloc_Input(g_blueprintWorldInput, &frame, 0, 0);
	xinpattr_Set_Input_Draw_Function(g_blueprintDoorHintInput, blueprnt_DrawDoorHint);
	xinpattr_Hide_Input(g_blueprintDoorHintInput);
	g_blueprintControlsInput = xinput_Alloc_Input(g_blueprintWorldInput, &frame, 0, 0);
	/* Allocate each row's two buttons followed by its label, as in the DOS body. */
	for (int row = 0; row < 2; ++row) {
		for (int col = 0; col < 2; ++col) {
			xrect_Set_Rect(&frame, 0, 0, 16, 16);
			PushButton* button = xbtnpush_Alloc_Button(g_blueprintControlsInput, &frame, 0, select_blueprint,
													   NULL, 2 * row + col);
			xinpattr_Set_Input_Draw_Function(&button->header, draw_button);
			xinpattr_Set_Input_Allign(&button->header, 2 * col, 2 * row);
		}
		xrect_Set_Rect(&frame, 18, 1, 110, 15);
		Input* label = xinput_Alloc_Input(g_blueprintControlsInput, &frame, 0, 0);
		xinpattr_Set_Input_Draw_Function(label, draw_label);
		label->id = row;
		if (row)
			xinpattr_Set_Input_Allign(label, 0, 2);
	}
	xio_Set_Key_Buttons();
	xview_Set_View_Update_Function(end_view);
	blueprnt_OpenMusic(resource, g_blueprintFilm);
	blueprnt_LoadUiSounds();
	XwScene_RunView(finish_scene, release_scene);
}
