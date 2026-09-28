#include "xw/audio/soundext.h"
#include "xw/flight/shell_flight.h"
#include "xw/frontend/brief.h"
#include "xw/frontend/register.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"
#include "xw_dos94/frontend/scenes.h"
#include "xw_runtime/audio/music_policy.h"
#include <landru/actanim.h>
#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/file.h>
#include <landru/filedir.h>
#include <landru/font.h>
#include <landru/inpattr.h>
#include <landru/io.h>
#include <landru/paint.h>
#include <landru/style.h>
#include <landru/task.h>
#include <landru/view.h>
#include <landru/viewadd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static ResFile* resource;
static Film* film;
static Actor *door, *log_actor, *hint, *faces, *ships, *bwing;
static Input *group_panel, *pilot_panel;
static Directory directory;
static REGISTER_FastPilotRecord* pilots;
static int16_t pilot_slots, pilot_count, page_count, page, first_pilot, selected_pilot, focus;
static Sound *music, *previous_music;

/* DOS94 formation/roster/navigation tables at 0x344e16..0x3450be. */
static const char rank_abbreviations[][4] = { "FC", "FO", "LT", "CP", "CM", "GN" };
static const char rank_names[][14] = { "Flt. Cadet", "Flt. Officer", "Lieutenant",
									   "Captain",    "Commander",    "General" };
static const char skill_labels[][8] = { "Top Ace", "Ace", "Veteran", "Officer", "Rookie" };
static const int16_t formation_x[10][6] = {
	{ 0, 2, -2, 0, 2, -2 }, { 0, -2, 3, 4, -3, -4 },   { 0, 0, 0, 0, 0, 0 },    { 0, 1, -1, 2, -2, 3 },
	{ 0, 1, 2, 3, 4, 5 },   { 0, -1, -2, -3, -4, -5 }, { 2, 2, 2, -2, -2, -2 }, { 0, 1, 0, -1, 0, 0 },
	{ 0, 0, 0, 0, 0, 0 },   { 0, 1, -1, 1, -1, 0 },
};
static const int16_t formation_y[10][6] = {
	{ 0, -2, -2, -4, -6, -6 }, { 0, -2, -6, -7, -6, -7 }, { 0, -1, -2, -3, -4, -5 },
	{ 0, 0, 0, 0, 0, 0 },      { 0, -1, -2, -3, -4, -5 }, { 0, -1, -2, -3, -4, -5 },
	{ 0, -2, -4, 0, -2, -4 },  { 0, -1, -2, -1, -1, -1 }, { 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, -1 },
};
static const int16_t focus_x[40] = { 22,  188, 188, 280, 22,  88,  232, 280, 22,  109, 186, 280, 22,  109,
									 186, 280, 22,  109, 186, 280, 22,  109, 186, 280, 22,  109, 186, 280,
									 22,  109, 186, 280, 22,  109, 186, 280, 22,  88,  232, 280 };
static const int16_t focus_y[40] = { 80,  52,  52,  110, 80,  99,  99,  110, 80,  127, 127, 110, 80,  134,
									 134, 110, 80,  141, 141, 110, 162, 148, 148, 110, 162, 155, 155, 110,
									 162, 162, 162, 110, 162, 169, 169, 110, 162, 186, 186, 110 };

static REGISTER_FastPilotRecord* pilot_at(int token) {
	for (int i = 0; i < pilot_slots; ++i) {
		if (!pilots[i].deleted && token-- == 0)
			return &pilots[i];
	}
	return NULL;
}

/* DOS94 0x342730, using native pilot records instead of the DOS disk layout. */
static void load_pilots(void) {
	xfiledir_Init_Directory(&directory, ".PLT", 0);
	xfiledir_Read_Directory(&directory);
	pilot_slots = directory.count;
	pilots = calloc(pilot_slots ? pilot_slots : 1, sizeof(*pilots));
	if (!pilots)
		abort();
	const DirEntry* entries = xmemhdl_Lock_Handle(directory.entries);
	pilot_count = 0;
	for (int i = 0; i < pilot_slots; ++i) {
		REGISTER_PilotFileRecord record;
		char path[FILEDIR_NAME_CAPACITY + 5];
		pilots[i].deleted = -1;
		snprintf(path, sizeof path, "%s.PLT", entries[i].name);
		LandruFile* file = xfile_Open_File(LANDRU_FILE_ROOT_USER, path, "rb");
		if (!file)
			continue;
		size_t read = register_ReadPilotRecord(file, &record);
		xfile_Close_File(file);
		if (read == 0 || record.lost_status)
			continue;
		snprintf(pilots[i].name, sizeof pilots[i].name, "%s", entries[i].name);
		pilots[i].field_18 = record.field_0;
		pilots[i].deleted = record.deleted;
		pilots[i].rank = record.rank;
		pilots[i].skillValue = record.skillValue;
		pilots[i].score = record.score;
		++pilot_count;
	}
	xmemhdl_Unlock_Handle(directory.entries);
	page_count = (pilot_count + 13) / 14;
}

static const char* skill_label(uint16_t skill) {
	static const uint16_t thresholds[] = { 65535, 49152, 32768, 16384, 0 };
	for (int i = 0; i < 5; ++i)
		if (skill >= thresholds[i])
			return skill_labels[i];
	return skill_labels[4];
}

static void refresh_panels(void) {
	xinpattr_Refresh_Input(group_panel);
	xinpattr_Refresh_Input(pilot_panel);
}

/* DOS94 0x342c16. */
static void assign_player(void) {
	g_localPilotAssignmentToken = -1;
	int token = 0;
	REGISTER_FastPilotRecord* record;
	while ((record = pilot_at(token)) != NULL) {
		if (!strcmp(record->name, g_RegisterShellPilot.name)) {
			g_localPilotAssignmentToken = token;
			break;
		}
		++token;
	}
	if (g_localPilotAssignmentToken == -1)
		return;
	for (int group = 0; group < g_launchGroupCount; ++group) {
		if (g_launchGroupPlayerCraftOrdinals[group]) {
			g_launchSelectedGroupIndex = group;
			g_launchSelectedCraftIndex = g_launchGroupPlayerCraftOrdinals[group] - 1;
			g_launchCraftPilotTokens[group][g_launchSelectedCraftIndex] = token;
			++g_launchAssignedPilotCount;
			break;
		}
	}
	selected_pilot = token;
	page = token / 14;
	first_pilot = page * 14;
}

/* DOS94 0x342d64: every assigned pilot contributes to the flight roster. */
static void build_roster(void) {
	memset(g_PilotSlotNames, 0, sizeof g_PilotSlotNames);
	memset(g_pilotSlotSkillValues, 0, sizeof g_pilotSlotSkillValues);
	memset(g_pilotSlotCraftIndices, 0, sizeof g_pilotSlotCraftIndices);
	memset(g_pilotSlotFlightGroupIndices, 0, sizeof g_pilotSlotFlightGroupIndices);
	int count = 0;
	for (int group = 0; group < g_launchGroupCount; ++group) {
		for (int craft = 0; craft < g_launchGroupCraftCounts[group]; ++craft) {
			REGISTER_FastPilotRecord* record = pilot_at(g_launchCraftPilotTokens[group][craft]);
			if (!record)
				continue;
			snprintf(g_PilotSlotNames[count], sizeof g_PilotSlotNames[count], "%s", record->name);
			g_pilotSlotSkillValues[count] = record->skillValue;
			g_pilotSlotFlightGroupIndices[count] = g_launchGroupMissionIndices[group];
			g_pilotSlotCraftIndices[count] = craft;
			++count;
		}
	}
}

static void door_sound(void) {
	if (ShellPreferences_GetSfxEnabled())
		soundext_Play_SFX(33);
}

/* DOS94 0x3410a8 / 0x34112e. */
static void update_door(Actor* actor, int time) {
	(void)time;
	if (actor->var1 == 1) {
		if (actor->state == 0) {
			door_sound();
			xactor_Set_Actor_State(actor, 1, 0);
			xactor_Refresh_Actor(actor);
		}
		actor->var1 = 0;
	} else if (actor->state) {
		door_sound();
		xactor_Set_Actor_State(actor, 0, 0);
		xactor_Refresh_Actor(actor);
	}
}

static void update_log(Actor* actor, int time) {
	(void)time;
	if (actor->var1 == 1) {
		if (xactor_Is_Actor_Visible(actor)) {
			if (actor->state != actor->arraySize - 1)
				xactor_Set_Actor_State(actor, actor->state + 1, 0);
		} else {
			door_sound();
			xactor_Show_Actor(actor);
			xactor_Set_Actor_State(actor, 0, 0);
		}
		actor->var1 = 0;
	} else if (actor->state) {
		door_sound();
		xactor_Set_Actor_State(actor, actor->state - 1, 0);
	} else if (xactor_Is_Actor_Visible(actor)) {
		door_sound();
		xactor_Hide_Actor(actor);
	} else
		return;
	xactor_Refresh_Actor(actor);
	xactor_Refresh_Actor(door);
}

/* DOS94 0x340df8 / 0x340e12 / 0x340f6c. */
static void update_stars(Actor* actor, int time) {
	(void)time;
	if (++actor->x >= 320)
		actor->x -= 640;
}

static void update_ship(Actor* actor, int time) {
	int small = actor->var1 == 2;
	if (!time) {
		actor->var2 = rand() & (small ? 127 : 255);
		xactor_Activate_Actor(actor);
		xactor_Hide_Actor(actor);
		return;
	}
	if (actor->var2) {
		if (xactor_Is_Actor_Visible(actor) &&
			((actor->x >= 320 && actor->xv > 0) || (actor->x <= 180 && actor->xv < 0))) {
			xactor_Hide_Actor(actor);
			actor->xv = 0;
		}
		--actor->var2;
		return;
	}
	actor->var2 = (rand() & 255) + 128;
	if (small) {
		if (rand() & 1) {
			actor->x = 180;
			actor->xv = 5 - (rand() & 3);
			xactor_Set_Actor_Flip(actor, 0, 0);
		} else {
			actor->x = 320;
			actor->xv = (rand() & 3) - 5;
			xactor_Set_Actor_Flip(actor, 1, 0);
		}
		actor->y = (rand() & 31) + 60;
	} else {
		int depth = rand() & 3;
		if (rand() & 1) {
			actor->x = 180;
			actor->xv = 5 - depth;
			actor->state = (rand() & 3) + 4;
		} else {
			actor->x = 320;
			actor->xv = depth - 5;
			actor->state = rand() & 3;
		}
		actor->y = (rand() & 63) + 64;
		xactor_Set_Actor_ZPlane(actor, depth + 20);
		xactor_Set_Actor_Scale(actor, (8 - depth) * 32, (8 - depth) * 32);
	}
	xactor_Show_Actor(actor);
}

static int16_t film_callback(Film* loading, FilmObject* object) {
	if (object->id == FTC_ACTOR) {
		xfilm_Rewind_Actor_Film(loading, object, object + 1);
		Actor* actor = object->object;
		switch (actor->var1) {
			case 1:
			case 2:
				xactor_Set_Actor_User_Function(actor, update_ship);
				break;
			case 5:
				door = actor;
				xactor_Non_Refreshable_Actor(actor);
				xactor_Set_Actor_User_Function(actor, update_door);
				break;
			case 6:
				log_actor = actor;
				xactor_Non_Refreshable_Actor(actor);
				xactor_Set_Actor_User_Function(actor, update_log);
				break;
			case 10:
				xactor_Set_Actor_User_Function(actor, update_stars);
				break;
			case 20:
				xactor_Non_Refreshable_Actor(actor);
				break;
		}
	}
	return 0;
}

/* DOS94 0x342508. Offsets are relative to the formation panel. */
static void formation_layout(const Rect* frame, int* ox, int* oy, int* min_x, int* min_y) {
	int max_x = 0, max_y = 0;
	*min_x = *min_y = 0;
	int group = g_launchSelectedGroupIndex;
	int formation = g_launchGroupFormations[group];
	for (int i = 0; i < g_launchGroupCraftCounts[group]; ++i) {
		int x = formation_x[formation][i], y = -formation_y[formation][i];
		if (x < *min_x)
			*min_x = x;
		if (x > max_x)
			max_x = x;
		if (y < *min_y)
			*min_y = y;
		if (y > max_y)
			max_y = y;
	}
	*ox = (frame->right - frame->left - ((max_x - *min_x) * 8 + 5)) >> 1;
	*oy = (frame->bottom - frame->top - ((max_y - *min_y) * 8 + 5)) >> 1;
}

/* DOS94 0x342266. */
static void draw_formation(Rect* frame) {
	int group = g_launchSelectedGroupIndex;
	int formation = g_launchGroupFormations[group];
	int base = g_launchGroupCraftTypes[group] - 1;
	int ox, oy, min_x, min_y;
	xactor_Set_Actor_State(ships, base, 0);
	formation_layout(frame, &ox, &oy, &min_x, &min_y);
	ox += frame->left;
	oy += frame->top;
	if (g_launchSelectedCraftIndex != -1) {
		int craft = g_launchSelectedCraftIndex;
		int x = ox + (formation_x[formation][craft] - min_x) * 8;
		int y = oy - (formation_y[formation][craft] + min_y) * 8;
		Rect outline;
		xrect_Set_Rect(&outline, x - 2, y - 2, x + ships->w + 2, y + ships->h + 2);
		xpaint_Frame_Clipped_Rect(&outline, 49);
	}
	for (int craft = 0; craft < g_launchGroupCraftCounts[group]; ++craft) {
		int token = g_launchCraftPilotTokens[group][craft];
		int variant = token == -1 ? 1 : token == g_localPilotAssignmentToken ? 2 : 0;
		Actor* icon = ships;
		int state = base + variant * 3;
		if (g_launchGroupCraftTypes[group] == 2 && g_launchGroupInitialStatus[group] >= 10) {
			icon = bwing;
			state = variant;
		}
		xactor_Set_Actor_State(icon, state, 0);
		xactanim_Draw_Anim_Actor(icon, frame, frame, ox + (formation_x[formation][craft] - min_x) * 8,
								 oy - (formation_y[formation][craft] + min_y) * 8, 1);
	}
}

/* DOS94 0x34139a / 0x34216c. */
static int16_t update_formation(Input* input, Rect* frame, Rect* clip, int16_t key, int left, int right,
								int16_t x, int16_t y) {
	(void)clip;
	if (key || input->id != 3)
		return 0;
	if ((left ? left : right) != 1)
		return 1;
	int ox, oy, min_x, min_y;
	int group = g_launchSelectedGroupIndex;
	int formation = g_launchGroupFormations[group];
	int chosen = g_launchSelectedCraftIndex;
	formation_layout(frame, &ox, &oy, &min_x, &min_y);
	if (!xio_Is_Mouse_Input() && !xio_Is_Joystick_Input()) {
		if (g_launchGroupCraftCounts[group])
			chosen = (chosen + 1) % g_launchGroupCraftCounts[group];
	} else
		for (int craft = 0; craft < g_launchGroupCraftCounts[group]; ++craft) {
			Rect bounds;
			int cx = ox + (formation_x[formation][craft] - min_x) * 8;
			int cy = oy - (formation_y[formation][craft] + min_y) * 8;
			xrect_Set_Rect(&bounds, cx, cy, cx + ships->w, cy + ships->h);
			if (xrect_Point_In_Rect(&bounds, x, y)) {
				chosen = craft;
				break;
			}
		}
	if (chosen != g_launchSelectedCraftIndex) {
		g_launchSelectedCraftIndex = chosen;
		selected_pilot = g_launchCraftPilotTokens[group][chosen];
		if (selected_pilot != -1) {
			page = selected_pilot / 14;
			first_pilot = page * 14;
		}
		refresh_panels();
	}
	return 1;
}

/* DOS94 0x341496, portrait branch. */
static void draw_pilot(Rect* frame, Rect* clip) {
	REGISTER_FastPilotRecord* record = pilot_at(selected_pilot);
	if (!record) {
		xpaint_Paint_Clipped_Rect(frame, 23);
		return;
	}
	Rect text;
	int x = frame->left + ((frame->right - frame->left - 38) >> 1);
	xrect_Set_Rect(&text, x - 1, frame->top, x + 39, frame->top + 50);
	xpaint_Frame_Clipped_Rect(&text, 200);
	int sum = 0;
	for (const unsigned char* p = (const unsigned char*)record->name; *p; ++p)
		sum += *p;
	xactor_Set_Actor_State(faces, abs((sum - 1) % 24), 0);
	xactanim_Draw_Anim_Actor(faces, frame, clip, x, frame->top + 1, 1);
	text = *frame;
	text.top = text.bottom - 22;
	text.bottom -= 16;
	xfont_Enable_FontID_Shadow(1);
	xfont_Print_Centered_Text(rank_names[record->rank], &text, 1, 15);
	xrect_Offset_Rect(&text, 0, 7);
	xfont_Print_Centered_Text(record->name, &text, 1, 15);
	xrect_Offset_Rect(&text, 0, 7);
	char skill[40];
	snprintf(skill, sizeof skill, "%s: %u", skill_label(record->skillValue), record->skillValue);
	xfont_Print_Centered_Text(skill, &text, 1, 15);
	xfont_Disable_FontID_Shadow(1);
}

static void draw_panel(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	if (!refresh)
		return;
	switch (input->id) {
		case 0:
			xpaint_Paint_Clipped_Rect(frame, 23);
			break;
		case 1:
			xfont_Enable_FontID_Shadow(0);
			xfont_Print_Centered_Text("Pilot Assignment", frame, 0, 15);
			xfont_Disable_FontID_Shadow(0);
			break;
		case 2:
			draw_pilot(frame, clip);
			break;
		case 3:
			xpaint_Frame_Clipped_Rect(frame, 200);
			xrect_Inset_Rect(frame, 1, 1);
			xpaint_Paint_Clipped_Rect(frame, 16);
			draw_formation(frame);
			break;
		case 4:
			xpaint_Paint_Clipped_Rect(frame, 1);
			break;
		case 5:
			xpaint_Horiz_Clipped_Line(frame->left, frame->top, frame->right - frame->left, 16);
			xfont_Enable_FontID_Shadow(0);
			xfont_Print_Centered_Text("Pilot Roster", frame, 0, 15);
			xfont_Disable_FontID_Shadow(0);
			break;
		case 6:
			xpaint_Paint_Clipped_Rect(frame, 62);
			break;
		default:
			xpaint_Frame_Clipped_Rect(frame, 15);
			break;
	}
}

/* DOS94 0x3417d2. */
static int16_t update_roster(Input* input, Rect* frame, Rect* clip, int16_t key, int left, int right,
							 int16_t x, int16_t y) {
	(void)frame;
	(void)clip;
	(void)x;
	if (key)
		return 0;
	if ((left ? left : right) != 3)
		return 1;
	int row = (y + 6) / 7 - 1;
	if (row < 0 || row >= 7)
		return 1;
	int token = first_pilot + row + (input->id ? 7 : 0);
	if (!pilot_at(token))
		return 1;
	int previous = selected_pilot;
	selected_pilot = token == selected_pilot && token != g_localPilotAssignmentToken ? -1 : token;
	int found = 0;
	if (selected_pilot != -1) {
		for (int group = 0; group < g_launchGroupCount && !found; ++group) {
			for (int craft = 0; craft < 6 && !found; ++craft) {
				if (g_launchCraftPilotTokens[group][craft] != selected_pilot)
					continue;
				if (group == g_launchSelectedGroupIndex)
					g_launchSelectedCraftIndex = craft;
				else
					selected_pilot = previous;
				found = 1;
			}
		}
	}
	if (previous == g_localPilotAssignmentToken && selected_pilot != -1 && !found) {
		selected_pilot = previous;
		found = 1;
	}
	if (!found && g_launchSelectedCraftIndex != -1) {
		int16_t* slot = &g_launchCraftPilotTokens[g_launchSelectedGroupIndex][g_launchSelectedCraftIndex];
		if (selected_pilot == -1) {
			*slot = -1;
			--g_launchAssignedPilotCount;
		} else if (g_launchAssignedPilotCount < 16) {
			if (*slot == -1)
				++g_launchAssignedPilotCount;
			*slot = selected_pilot;
		} else
			selected_pilot = previous;
	}
	xinpattr_Refresh_Input(pilot_panel);
	xinpattr_Refresh_Input(group_panel);
	return 1;
}

/* DOS94 0x341a60. */
static void draw_roster(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	(void)clip;
	if (!refresh)
		return;
	xpaint_Paint_Clipped_Rect(frame, 16);
	xfont_Enable_FontID_Shadow(1);
	int y = frame->top + 1;
	for (int row = 0; row < 7; ++row) {
		int token = first_pilot + (input->id ? 7 : 0) + row;
		REGISTER_FastPilotRecord* record = pilot_at(token);
		if (!record)
			continue;
		int name_color = g_launchAssignedPilotCount == 16 ? 26 : 7;
		int rank_color = g_launchAssignedPilotCount == 16 ? 26 : 51;
		for (int group = 0; group < g_launchGroupCount; ++group) {
			for (int craft = 0; craft < 6; ++craft) {
				if (g_launchCraftPilotTokens[group][craft] == token)
					name_color = rank_color = group == g_launchSelectedGroupIndex ? 62 : 63;
			}
		}
		if (token == g_localPilotAssignmentToken) {
			name_color = 15;
			if (rank_color == 62)
				rank_color = 15;
		}
		if (token == selected_pilot)
			name_color = 14;
		xfont_Print_Clipped_Text(rank_abbreviations[record->rank], frame->left + 2, y, 1, rank_color);
		xfont_Print_Clipped_Text(record->name, frame->left + 14, y, 1, name_color);
		const char* skill = skill_label(record->skillValue);
		xfont_Print_Clipped_Text(skill, frame->left + 76 - xfont_Get_String_Width_0(1, skill), y, 1,
								 name_color);
		y += 7;
	}
	xfont_Disable_FontID_Shadow(1);
}

/* DOS94 0x341ccc. */
static void navigate(Input* input, int time) {
	(void)time;
	if (!xinpattr_Get_Input_Selected(input))
		return;
	if (input->id < 2 && g_launchGroupCount > 1) {
		int group = g_launchSelectedGroupIndex + (input->id ? 1 : -1);
		if (group < 0)
			group = g_launchGroupCount - 1;
		if (group >= g_launchGroupCount)
			group = 0;
		g_launchSelectedGroupIndex = group;
		g_launchSelectedCraftIndex = 0;
		selected_pilot = g_launchCraftPilotTokens[group][0];
		if (selected_pilot != -1) {
			page = selected_pilot / 14;
			first_pilot = page * 14;
		}
		xinpattr_Refresh_Input(group_panel);
		xinpattr_Refresh_Input(pilot_panel);
	} else if (input->id == 2 || input->id == 3) {
		page += input->id == 2 ? -1 : 1;
		if (page < 0)
			page = page_count - 1;
		if (page >= page_count)
			page = 0;
		xinpattr_Refresh_Input(pilot_panel);
	}
	if (page_count > 1)
		first_pilot = page * 14;
}

static void draw_arrow(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	PushButton* button = (PushButton*)input;
	if (refresh) {
		xstyle_Style_Paint_Border(frame, button->pressed);
		xstyle_Style_Draw_Centered_Icon(input->id == 0 || input->id == 2 ? 1 : 3, frame, clip,
										button->pressed);
	}
}

static void draw_page(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	(void)clip;
	if (refresh) {
		char text[40];
		xstyle_Style_Paint_TextField(frame);
		if (input->id)
			snprintf(text, sizeof text, "Page %d of %d", page + 1, page_count);
		else
			snprintf(text, sizeof text, "Flight Group %d of %d", g_launchSelectedGroupIndex + 1,
					 g_launchGroupCount);
		xfont_Print_Centered_Text(text, frame, 0, 15);
	}
}

/* DOS94 0x341240 / 0x341298. */
static void update_hint(Actor* actor, int time) {
	(void)time;
	if (actor->var1 != actor->var2) {
		actor->var2 = actor->var1;
		xactor_Refresh_Actor(actor);
	}
	actor->var1 = 0;
}

static int16_t draw_hint(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	(void)clip;
	(void)x;
	(void)y;
	if (!refresh)
		return 0;
	static const char* const text[] = { "", "Return To Briefing", "Enter Mission", "View Medals",
										"View Pilot Log" };
	if (actor->var2) {
		if (xinpattr_Is_Input_Visible(pilot_panel))
			xinpattr_Hide_Input(pilot_panel);
		xpaint_Paint_Clipped_Rect(frame, 23);
		xfont_Enable_FontID_Shadow(0);
		xfont_Print_Centered_Text(text[actor->var2], frame, 0, 15);
		xfont_Disable_FontID_Shadow(0);
	} else {
		if (!xinpattr_Is_Input_Visible(pilot_panel)) {
			xinpattr_Show_Input(pilot_panel);
			xinpattr_Refresh_Input(pilot_panel);
		}
		xpaint_Paint_Clipped_Rect(frame, 23);
	}
	return 1;
}

/* DOS94 0x341f26 / 0x341ff6. */
static int16_t update_details(Input* input, Rect* frame, Rect* clip, int16_t key, int left, int right,
							  int16_t x, int16_t y) {
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (key)
		return 0;
	int event = left ? left : right;
	Actor* actor = input->id ? door : log_actor;
	hint->var1 = 4 - input->id;
	if (event <= 3)
		actor->var1 = 1;
	if (event == 3)
		input->var1 = 1;
	return event != 0;
}

static void open_details(Input* input, int time) {
	(void)time;
	if (input->var1) {
		int combat = shellext_Get_Cur_Scene() == 120;
		xerror_Set_Landru_Exit(input->id ? (combat ? 125 : 126) : (combat ? 127 : 128));
	}
}

static int16_t update_exit(Input* input, Rect* frame, Rect* clip, int16_t key, int left, int right, int16_t x,
						   int16_t y) {
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (key)
		return 0;
	hint->var1 = input->id ? 2 : 1;
	if (left == 3 || right == 3)
		input->var1 = 1;
	return 1;
}

/* DOS94 0x342b30. */
static int write_combat_selection(void) {
	char path[sizeof g_RegisterShellPilot.name + 5];
	REGISTER_PilotFileRecord record;
	snprintf(path, sizeof path, "%s.PLT", g_RegisterShellPilot.name);
	LandruFile* file = xfile_Open_File(LANDRU_FILE_ROOT_USER, path, "rb");
	if (!file)
		return 0;
	int complete = register_ReadPilotRecord(file, &record) != 0;
	xfile_Close_File(file);
	if (!complete)
		return 0;
	record.combatShip = shipext_Get_Combat_Ship();
	record.combatMission = shipext_Get_Combat_Mission();
	file = xfile_Open_File(LANDRU_FILE_ROOT_USER, path, "wb");
	if (!file)
		return 0;
	complete = xfile_Write_Data_To_File(file, &record, sizeof record) != 0;
	xfile_Close_File(file);
	return complete;
}

static void exit_scene(Input* input, int time) {
	(void)time;
	if (!input->var1)
		return;
	int combat = shellext_Get_Cur_Scene() == 120;
	if (!input->id) {
		xerror_Set_Landru_Exit(combat ? 111 : 112);
		return;
	}
	build_roster();
	if (combat && !write_combat_selection())
		return;
	static const int16_t combat_scenes[] = { 270, 271, 272, 276 };
	static const int16_t tour_scenes[] = { 283, 284, 285, 281 };
	int ship = shipext_Get_Mission_Ship();
	if (ship >= 0 && ship < 4)
		xerror_Set_Landru_Exit(combat ? combat_scenes[ship] : tour_scenes[ship]);
}

static void update_view(int time) {
	if (!time && !xcursor_Is_Cursor_Visible())
		xcursor_Show_Cursor();
	int16_t key = xio_Get_Free_Key();
	if (key && shellext_MoveGridFocus(&focus, focus_x, focus_y, 10, 4, key)) {
		xio_Set_Mouse_Position(focus_x[focus], focus_y[focus]);
		xio_Get_Key();
	}
}

static Input* panel(Input* parent, int left, int top, int right, int bottom, int id) {
	Rect frame;
	xrect_Set_Rect(&frame, left, top, right, bottom);
	Input* input = xinput_Alloc_Input(parent, &frame, 0, 0);
	input->id = id;
	xinpattr_Set_Input_Draw_Function(input, draw_panel);
	return input;
}

static void navigation(Input* parent, int first_id, int label_id) {
	Rect frame;
	xrect_Set_Rect(&frame, 1, 1, 17, 17);
	for (int i = 0; i < 2; ++i) {
		PushButton* button = xbtnpush_Alloc_Button(parent, &frame, 0, navigate, NULL, first_id + i);
		xinpattr_Set_Input_Draw_Function(&button->header, draw_arrow);
		xinpattr_Set_Input_Allign(&button->header, i * 2, 2);
	}
	xrect_Set_Rect(&frame, 0, 1, 125, 17);
	Input* label = xinput_Alloc_Input(parent, &frame, 0, 0);
	label->id = label_id;
	xinpattr_Set_Input_Draw_Function(label, draw_page);
	xinpattr_Set_Input_Allign(label, 1, 2);
}

static void create_inputs(void) {
	Rect frame;
	xrect_Set_Rect(&frame, 0, 0, 320, 200);
	Input* root = xinput_Alloc_Input(NULL, &frame, 0, 0);
	xrect_Set_Rect(&frame, 78, 3, 243, 196);
	Input* assignment = xinput_Alloc_Input(root, &frame, 0, 0);
	group_panel = panel(assignment, 0, 0, 165, 106, 0);
	panel(group_panel, 0, 0, 165, 14, 1);
	panel(group_panel, 0, 14, 67, 88, 2);
	Input* formation = panel(group_panel, 67, 14, 153, 85, 3);
	xinpattr_Set_Input_Update_Function(formation, update_formation);
	navigation(group_panel, 0, 0);
	pilot_panel = panel(assignment, 0, 106, 165, 193, 4);
	panel(pilot_panel, 0, 0, 165, 14, 5);
	Input* roster = panel(pilot_panel, 3, 14, 162, 66, 6);
	xrect_Set_Rect(&frame, 1, 1, 79, 51);
	for (int i = 0; i < 2; ++i) {
		Input* list = xinput_Alloc_Input(roster, &frame, 0, 0);
		list->id = i;
		xinpattr_Set_Input_Update_Function(list, update_roster);
		xinpattr_Set_Input_Draw_Function(list, draw_roster);
		if (i)
			xinpattr_Set_Input_Allign(list, 2, 0);
	}
	navigation(pilot_panel, 2, 1);
	for (int i = 0; i < 2; ++i) {
		xrect_Set_Rect(&frame, 0, i ? 120 : 98, 64, i ? 200 : 120);
		Input* input = xinput_Alloc_Input(root, &frame, 0, 0);
		input->id = i;
		input->mouseUsage = allInput;
		xinpattr_Set_Input_Update_Function(input, update_details);
		xinpattr_Set_Input_User_Function(input, open_details);
	}
	for (int i = 0; i < 2; ++i) {
		if (i)
			xrect_Set_Rect(&frame, 260, 56, 320, 200);
		else
			xrect_Set_Rect(&frame, 0, 0, 64, 98);
		Input* input = xinput_Alloc_Input(root, &frame, 0, 0);
		input->id = i;
		input->mouseUsage = allInput;
		xinpattr_Set_Input_Update_Function(input, update_exit);
		xinpattr_Set_Input_User_Function(input, exit_scene);
	}
}

/* DOS94 0x342f3e, with its tick wait resumed by the cooperative scene task. */
static int open_music(void) {
	previous_music = NULL;
	if (!ShellPreferences_GetMusicEnabled())
		return 0;
	music = xsound_Find_Gmid("adrift");
	if (music && (uint8_t)soundext_Count_Resource_Instances(music) == 1)
		return 0;
	ResFile* archive = xres_Open_Resource("plmusic.lfd");
	music = xsound_Res_Music(archive, "adrift");
	xres_Close_Resource(archive);
	if (shellext_Get_Cur_Scene() != 122)
		previous_music = xsound_Find_Gmid("patrol");
	soundext_Start_Resource_Sound(music);
	soundext_SetVolume((intptr_t)music, 95);
	if (previous_music && (uint8_t)soundext_Count_Resource_Instances(previous_music) == 1) {
		soundext_ShareParts(previous_music, music);
		return XwMusicPolicy_UsesImuse();
	}
	return 0;
}

static void close_music(void) {
	if (!ShellPreferences_GetMusicEnabled())
		return;
	int scene = xerror_Get_Landru_Exit();
	if (scene == 111 || (scene >= 270 && scene <= 272) || scene == 276 || scene == 281 ||
		(scene >= 283 && scene <= 285))
		return;
	music = xsound_Find_Gmid("adrift");
	if (music) {
		soundext_SetPriority((intptr_t)music, 0);
		soundext_FadeVolume(music, 0, 300);
	}
}

static void release_scene(void* state) {
	(void)state;
	close_music();
	soundext_ResetEnabledSfxCache();
	xio_Clear_Key_Buttons();
	xview_Clear_View_Update_Function();
	xfiledir_Free_Directory(&directory);
	xview_Enable_All_View_Erase();
	free(pilots);
	pilots = NULL;
	if (xcursor_Is_Cursor_Visible())
		xcursor_Hide_Cursor();
	xres_Close_Resource(resource);
	resource = NULL;
}

typedef struct PilotSelectionTask {
	int phase;
} PilotSelectionTask;

static LandruTaskStepResult step_scene(void* self) {
	PilotSelectionTask* task = self;
	if (task->phase == 0)
		task->phase = open_music() ? 1 : 2;
	if (task->phase == 1) {
		if (!soundext_GetMusicParam(music, XW_SOUND_QUERY_TICK, 0))
			return LANDRU_TASK_STEP_YIELD;
		j_lolevel_ImPause();
		int group = soundext_GetMusicParam(previous_music, XW_SOUND_QUERY_CHUNK, 0);
		int beat = soundext_GetMusicParam(previous_music, XW_SOUND_QUERY_BEAT, 0);
		int tick = soundext_GetMusicParam(previous_music, XW_SOUND_QUERY_TICK, 0);
		j_lolevel_ImResume();
		soundext_ScanMidi(music, group, beat, tick);
		soundext_JumpMidi(previous_music, 1, 0, 0);
		task->phase = 2;
	}
	if (task->phase == 2) {
		if (ShellPreferences_GetMusicEnabled()) {
			xsound_Set_Sound_Keep(music);
			xsound_Set_Sound_User_Function(music, Cutscene_IgnoreSoundEvent);
		}
		if (ShellPreferences_GetSfxEnabled())
			soundext_LoadSfx(33, 0, NULL, 0, 1);
		task->phase = 3;
		j_xviewadd_Handle_View();
		return LANDRU_TASK_STEP_YIELD;
	}
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable scene_task = { step_scene, release_scene, NULL, NULL };

/* DOS94 0x340000. */
void Dos94_PilotSelection(XwShellContext* shell) {
	Rect frame;
	focus = 31;
	xio_Set_Mouse_Position(258, 110);
	page_count = 1;
	selected_pilot = g_localPilotAssignmentToken = -1;
	g_launchSelectedGroupIndex = g_launchSelectedCraftIndex = g_launchGroupCount =
		g_launchAssignedPilotCount = 0;
	page = first_pilot = pilot_count = pilot_slots = 0;
	memset(g_launchCraftPilotTokens, 0xff, sizeof g_launchCraftPilotTokens);
	resource = xres_Open_Resource("pilot.lfd");
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	film = xfilm_Res_Callback_Film(resource, "pilot", &frame, 0, 0, 0, film_callback);
	xfilm_Set_Film_Def_Palette(film, shell->standardPalette);
	faces = xactanim_Res_Anim_Actor(resource, "faces", &frame, 0, 0, 0);
	xactor_Set_Actor_Time(faces, 0, 0);
	ships = xactanim_Res_Anim_Actor(resource, "smlships", &frame, 0, 0, 0);
	xactor_Set_Actor_Time(ships, 0, 0);
	bwing = NULL;
	if (shipext_IsTourAvailable(4)) {
		ResFile* expansion = xres_Open_Resource("bwing.lfd");
		bwing = xactanim_Res_Anim_Actor(expansion, "smlbwing", &frame, 0, 0, 0);
		xactor_Set_Actor_Time(bwing, 0, 0);
		xres_Close_Resource(expansion);
	}
	xrect_Set_Rect(&frame, 78, 110, 243, 196);
	hint = xactcust_Alloc_Custom_Actor(0, &frame, 0, 0, 0);
	xactor_Set_Actor_User_Function(hint, update_hint);
	xactor_Set_Actor_Draw_Function(hint, draw_hint);
	xactor_Non_Refreshable_Actor(hint);
	create_inputs();
	brief_LoadLaunchFlightGroups();
	xcursor_Set_Cursor(1);
	load_pilots();
	xcursor_Set_Cursor(0);
	assign_player();
	xio_Set_Key_Buttons();
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xview_Set_View_Update_Function(update_view);
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	PilotSelectionTask* task = landru_task_push(&scene_task);
	if (!task)
		abort();
	task->phase = 0;
}
