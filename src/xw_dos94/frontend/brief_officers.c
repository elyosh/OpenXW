#include "xw_dos94/frontend/brief_officers.h"
#include "xw/audio/soundext.h"
#include "xw/frontend/brief.h"
#include "xw/frontend/shell_preferences.h"
#include <landru/actanim.h>
#include <landru/actdelt.h>
#include <stdlib.h>

static Actor *body, *head, *hand, *turn, *arm;
static int16_t pose_mode, next_pose_mode, pose_tick;
static uint8_t speech_phase;

/* DOS94 head-pose tables at 0x1862bc and 0x18632c. */
static const int16_t dodonna_poses[][2] = { { 1, 2 },  { 1, 2 },  { 0, 1 },  { 0, 1 },  { 0, 6 },  { 1, 4 },
											{ 1, 4 },  { 0, 10 }, { 0, 10 }, { 1, 8 },  { 1, 8 },  { 0, 14 },
											{ 0, 14 }, { 0, 14 }, { 1, 13 }, { 0, 16 }, { 1, 19 }, { 1, 19 },
											{ 1, 19 }, { 0, 18 }, { 0, 18 }, { 0, 24 }, { 0, 24 }, { 1, 22 },
											{ 1, 25 }, { 0, 24 }, { 1, 25 }, { 1, 25 } };
static const int16_t ackbar_poses[][2] = { { 0, 1 },  { 1, 0 },  { 0, 3 },  { 1, 2 },  { 0, 1 },
										   { 0, 3 },  { 0, 7 },  { 1, 6 },  { 0, 9 },  { 1, 8 },
										   { 1, 8 },  { 0, 12 }, { 1, 13 }, { 0, 12 }, { 0, 12 },
										   { 0, 18 }, { 0, 18 }, { 0, 18 }, { 1, 19 }, { 0, 18 } };

/* DOS94 0x1843a4 / 0x1844a8. */
static void move_head(int target, int speech_mode, const int16_t poses[][2]) {
	int state = brief_Move_To_Value(head->state, target, 1);
	if (speech_mode && poses[state][0] != speech_mode - 1) {
		if (head->state < target) {
			while (state <= target && poses[state][0] != speech_mode - 1)
				++state;
			if (state > target)
				state = poses[target][1];
		} else if (head->state > target) {
			while (state >= target && poses[state][0] != speech_mode - 1)
				--state;
			if (state < target)
				state = poses[target][1];
		} else
			state = poses[state][1];
	}
	xactor_Set_Actor_State(head, state, 0);
}

static void move_part(Actor* actor, int target) {
	xactor_Set_Actor_State(actor, brief_Move_To_Value(actor->state, target, 1), 0);
}

static int narration_phase(void) {
	if (!g_shellPreferences.sfxEnabled || !g_shellPreferences.sfxVolume)
		return 0;
	if (g_briefingNarrationSound && soundext_Count_Resource_Instances(g_briefingNarrationSound))
		return speech_phase + 1;
	return 1;
}

static void advance_pose(int complete) {
	if (complete) {
		pose_mode = next_pose_mode;
		next_pose_mode = rand() & 3;
		pose_tick = -(rand() & 31);
	} else if (pose_tick > 0 || g_briefingRuntime->playbackActive)
		++pose_tick;
}

/* DOS94 0x181920. */
static void update_ackbar(Actor* actor, int time) {
	(void)actor;
	if (time & 1)
		speech_phase ^= 1;
	if (time == 0) {
		xactor_Hide_Actor(hand);
		xactor_Hide_Actor(turn);
		next_pose_mode = rand() & 3;
		pose_tick = -12;
		return;
	}
	int tick = pose_tick >= 0 ? pose_tick >> 1 : 0;
	int complete = 0, head_target = 0, limb_target = 0;
	/* DOS leaves this local undefined when switching from an even to an odd pose. */
	int turn_target = 0;
	switch (pose_mode) {
		case 0:
			head_target = tick % 39;
			complete = head_target == 0 && tick != 0;
			if (head_target > 19)
				head_target = 38 - head_target;
			limb_target = (tick + 10) % 39;
			if (limb_target > 6)
				limb_target = 0;
			else if (limb_target > 3)
				limb_target = 6 - limb_target;
			break;
		case 1:
		case 3:
			head_target = tick % 6;
			if (head_target > 2)
				head_target = 5 - head_target;
			head_target += 17;
			turn_target = tick % 20;
			complete = turn_target == 0 && tick != 0;
			if (turn_target > 16)
				turn_target = 19 - turn_target;
			else if (turn_target > 3)
				turn_target = 3;
			if (turn_target == 3) {
				limb_target = (tick >> 1) % 5;
				if (limb_target > 2)
					limb_target = 4 - limb_target;
			}
			break;
		case 2: {
			int cycle = tick % 25;
			complete = cycle == 0 && tick != 0;
			head_target = cycle > 12 ? 24 - cycle : cycle;
			limb_target = cycle > 21 ? 24 - cycle : cycle > 3 ? 3 : cycle;
			break;
		}
	}
	int speech = narration_phase();
	advance_pose(complete);
	if (complete) {
		if (pose_mode & 1) {
			move_part(hand, limb_target);
			move_part(turn, turn_target);
		} else
			move_part(arm, limb_target);
		move_head(head_target, speech, ackbar_poses);
		return;
	}
	if (pose_mode & 1) {
		if (xactor_Is_Actor_Visible(body)) {
			xactor_Hide_Actor(body);
			xactor_Hide_Actor(arm);
			xactor_Show_Actor(hand);
			xactor_Show_Actor(turn);
		}
		move_part(hand, limb_target);
		move_part(turn, turn_target);
		move_head(head_target, speech, ackbar_poses);
		if (turn_target == 3)
			xactor_Show_Actor(hand);
		else
			xactor_Hide_Actor(hand);
	} else {
		if (!xactor_Is_Actor_Visible(body)) {
			xactor_Hide_Actor(hand);
			xactor_Hide_Actor(turn);
			xactor_Show_Actor(body);
			xactor_Show_Actor(arm);
		}
		move_part(arm, limb_target);
		move_head(head_target, speech, ackbar_poses);
	}
}

/* DOS94 0x181d86. */
static void update_dodonna(Actor* actor, int time) {
	(void)actor;
	if (time & 1)
		speech_phase ^= 1;
	if (time == 0) {
		next_pose_mode = rand() & 3;
		pose_tick = -12;
		xactor_Hide_Actor(hand);
		return;
	}
	int tick = pose_tick >= 0 ? pose_tick >> 1 : 0;
	int complete = 0, hand_target = -1, head_target = 7, body_target = 1;
	switch (pose_mode) {
		case 0: {
			int cycle = tick % 39;
			complete = cycle == 0 && tick != 0;
			body_target = (cycle >> 3) + 1;
			if (cycle >= 24 && cycle < 32)
				body_target = 2;
			if (cycle >= 32)
				body_target = 0;
			head_target = cycle < 5 ? cycle + 5 : cycle;
			if (head_target > 20)
				head_target = 38 - head_target;
			break;
		}
		case 1:
		case 3: {
			int cycle = tick % 56;
			complete = cycle == 0 && tick != 0;
			body_target = cycle >> 3;
			if (cycle >= 32 && cycle < 48)
				body_target = 4;
			if (cycle >= 48)
				body_target = 9 - body_target;
			if (cycle >= 32 && cycle < 36)
				hand_target = cycle - 32;
			else if (cycle >= 36 && cycle < 44)
				hand_target = 4;
			else if (cycle >= 44 && cycle < 48)
				hand_target = 47 - cycle;
			if (cycle < 39)
				head_target = (cycle <= 20 ? 24 : 58) - cycle;
			else if (cycle < 48)
				head_target = rand() % 7 + 21;
			else
				head_target = 74 - cycle;
			break;
		}
		case 2: {
			int cycle = tick % 39;
			complete = cycle == 0 && tick != 0;
			body_target = 3 - (cycle >> 3);
			if (cycle >= 24 && cycle < 32)
				body_target = 2;
			if (cycle >= 32)
				body_target = 3;
			head_target = cycle < 16 ? 27 - cycle : cycle < 30 ? cycle - 8 : cycle - 20;
			break;
		}
	}
	advance_pose(complete);
	if (head_target > 27)
		head_target = 27;
	move_part(body, body_target);
	move_head(head_target, narration_phase(), dodonna_poses);
	if (hand_target == -1) {
		if (xactor_Is_Actor_Visible(hand))
			xactor_Hide_Actor(hand);
	} else {
		if (!xactor_Is_Actor_Visible(hand))
			xactor_Show_Actor(hand);
		move_part(hand, hand_target);
	}
}

/* Officer setup within DOS94 brief_Brief, 0x18057a..0x1807bd. */
void Dos94_Brief_CreateOfficer(ResFile* resource, Rect* frame, int scene) {
	if (scene == 111) {
		body = xactanim_Res_Anim_Actor(resource, "janbody", frame, -3, 13, 2);
		xactor_Set_Actor_User_Function(body, update_dodonna);
		xactor_Set_Actor_Flip(body, 1, 0);
		head = xactanim_Res_Anim_Actor(resource, "janhead", frame, 3, 14, 1);
		xactor_Set_Actor_Flip(head, 1, 0);
		hand = xactanim_Res_Anim_Actor(resource, "janhand", frame, 33, 14, 0);
		xactor_Set_Actor_Flip(hand, 1, 0);
		xrect_Set_Rect(&g_briefingOfficerRect, 16, 32, 100, 92);
	} else if (scene == 112) {
		body = xactdelt_Res_Delta_Actor(resource, "ackbody2", frame, 0, 0, 2);
		xactor_Set_Actor_User_Function(body, update_ackbar);
		hand = xactanim_Res_Anim_Actor(resource, "ackhand3", frame, 0, 0, 0);
		turn = xactanim_Res_Anim_Actor(resource, "ackturn4", frame, 0, 0, 2);
		head = xactanim_Res_Anim_Actor(resource, "ackhead3", frame, 0, 0, 1);
		arm = xactanim_Res_Anim_Actor(resource, "ackarm2", frame, 0, 0, 0);
		xrect_Set_Rect(&g_briefingOfficerRect, 236, 34, 290, 94);
	}
}
