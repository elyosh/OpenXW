#include "xw_dos94/audio/soundext.h"
#include "xw_dos94/audio/hilevel.h"
#include "xw_runtime/audio/imuse_session.h"
#include "xw_runtime/audio/music_policy.h"

// FUNCTION: DOS94 0x1228C
int16_t Dos94_soundext_Start_Resource_Sound(const Sound* sound) {
	if (!sound)
		return -1;
	if (sound->type == digitalSound)
		return Dos94_soundext_Start_Resource_SFX(sound);
	return XwMusicPolicy_UsesImuse() ? Dos94_hilevel_ImStartMusic((intptr_t)sound) : -1;
}

// FUNCTION: DOS94 0x122B0
int16_t Dos94_soundext_Start_Resource_SFX(const Sound* sound) {
	return sound ? Dos94_hilevel_ImStartSfx((intptr_t)sound) : -1;
}

// FUNCTION: DOS94 0x122C2
int16_t Dos94_soundext_Start_Resource_Voice(const Sound* sound) {
	return sound ? Dos94_hilevel_ImStartVoice((intptr_t)sound) : -1;
}

// FUNCTION: DOS94 0x122D4
int16_t Dos94_soundext_Stop_Resource_Sound(const Sound* sound) {
	return g_dos94Imuse && sound ? imuse_stop_sound(g_dos94Imuse, (intptr_t)sound) : 0;
}

// FUNCTION: DOS94 0x122EC
int16_t Dos94_soundext_Count_Resource_Instances(const Sound* sound) {
	return g_dos94Imuse && sound
			   ? imuse_get_param(g_dos94Imuse, (intptr_t)sound, IMUSE_PARAM_SOUND_PLAY_COUNT)
			   : 0;
}

// FUNCTION: DOS94 0x1232E
int16_t Dos94_soundext_GetMusicParam(const Sound* sound, int selector) {
	imuse_t* im = g_dos94Imuse;
	if (!im || !sound || imuse_get_param(im, (intptr_t)sound, IMUSE_PARAM_SOUND_PLAY_COUNT) <= 0)
		return -1;
	intptr_t id = (intptr_t)sound;
	switch (selector) {
		case 1:
			return imuse_get_param(im, id, IMUSE_PARAM_SOUND_VOL);
		case 6:
			return imuse_get_param(im, id, IMUSE_PARAM_MIDI_CHUNK) - 1;
		case 7:
			return (imuse_get_param(im, id, IMUSE_PARAM_MIDI_MEASURE) - 1) * 4 +
				   imuse_get_param(im, id, IMUSE_PARAM_MIDI_BEAT) - 1;
		case 8:
			return imuse_get_param(im, id, IMUSE_PARAM_MIDI_TICK);
		default:
			return -1;
	}
}

// FUNCTION: DOS94 0x123E2
int16_t Dos94_soundext_SetPriority(const Sound* sound, uint16_t value) {
	return g_dos94Imuse ? imuse_set_param(g_dos94Imuse, (intptr_t)sound, IMUSE_PARAM_SOUND_PRIORITY, value)
						: -1;
}

// FUNCTION: DOS94 0x123FC
int16_t Dos94_soundext_SetVolume(const Sound* sound, uint16_t value) {
	return g_dos94Imuse ? imuse_set_param(g_dos94Imuse, (intptr_t)sound, IMUSE_PARAM_SOUND_VOL, value) : -1;
}

// FUNCTION: DOS94 0x12430
int16_t Dos94_soundext_SetTranspose(const Sound* sound, int16_t skipReset, int16_t value) {
	if (!g_dos94Imuse)
		return -1;
	if (!skipReset)
		imuse_set_param(g_dos94Imuse, (intptr_t)sound, IMUSE_PARAM_SOUND_TRANSPOSE, 0);
	return imuse_set_param(g_dos94Imuse, (intptr_t)sound, IMUSE_PARAM_SOUND_TRANSPOSE, value);
}

// FUNCTION: DOS94 0x1247A
int16_t Dos94_soundext_SetGroup(const Sound* sound, uint16_t value) {
	return g_dos94Imuse ? imuse_set_param(g_dos94Imuse, (intptr_t)sound, IMUSE_PARAM_SOUND_GROUP, value) : -1;
}

// FUNCTION: DOS94 0x12494
int16_t Dos94_soundext_JumpMidi(const Sound* sound, int chunk, unsigned int beat, int tick) {
	if (!g_dos94Imuse || !sound || beat > 3999)
		return -1;
	return imuse_midi_jump(g_dos94Imuse, (intptr_t)sound, chunk + 1, beat / 4 + 1, beat % 4 + 1, tick, 1);
}

// FUNCTION: DOS94 0x124C8
int16_t Dos94_soundext_ScanMidi(const Sound* sound, int chunk, unsigned int beat, int tick) {
	if (!g_dos94Imuse || !sound || beat > 3999)
		return -1;
	return imuse_midi_scan(g_dos94Imuse, (intptr_t)sound, chunk + 1, beat / 4 + 1, beat % 4 + 1, tick);
}

// FUNCTION: DOS94 0x124F8
int16_t Dos94_soundext_SetPartEnabled(const Sound* sound, int selector, int16_t enabled) {
	return g_dos94Imuse
			   ? imuse_xwing_set_part_volume(g_dos94Imuse, (intptr_t)sound, selector, enabled ? 127 : 0)
			   : -1;
}

// FUNCTION: DOS94 0x12518
int16_t Dos94_soundext_SetHook(const Sound* sound, int mode, int value, int channel) {
	imuse_t* im = g_dos94Imuse;
	if (!im || !sound || (mode != 0 && mode != 2))
		return -1;
	return imuse_xwing_set_hook(im, (intptr_t)sound, mode, value, mode == 0 ? 0 : channel);
}

// FUNCTION: DOS94 0x12558
int16_t Dos94_soundext_FadeVolume(const Sound* sound, int volume, int duration) {
	return imuse_fade_param(g_dos94Imuse, (intptr_t)sound, IMUSE_PARAM_SOUND_VOL, volume, duration);
}

// FUNCTION: DOS94 0x12574
int16_t Dos94_soundext_SetTriggerContext(Sound* sound, uint16_t marker) {
	g_soundTriggerContext.sound = sound;
	g_soundTriggerContext.marker = marker;
	return 0;
}

// FUNCTION: DOS94 0x12594
int16_t Dos94_soundext_QueueTriggerCommand(uint16_t command, intptr_t soundId, intptr_t arg1, intptr_t arg2,
										   intptr_t arg3) {
	imuse_t* im = g_dos94Imuse;
	Sound* owner = g_soundTriggerContext.sound;
	if (command == XW_MUSIC_TRIGGER_END) {
		g_soundTriggerContext.sound = NULL;
		g_soundTriggerContext.marker = 0;
		return 0;
	}
	if (!im || !owner || !XwMusicPolicy_UsesImuse())
		return 0;
	ImuseCmd cmd = { 0 };
	cmd.args[0] = soundId;
	switch (command) {
		case XW_MUSIC_TRIGGER_START:
			cmd.opcode = IMUSE_CMD_START_SOUND;
			cmd.args[1] = 64;
			break;
		case XW_MUSIC_TRIGGER_STOP:
			cmd.opcode = IMUSE_CMD_STOP_SOUND;
			break;
		case XW_MUSIC_TRIGGER_SET_VOLUME:
			cmd.opcode = IMUSE_CMD_SET_PARAM;
			cmd.args[1] = IMUSE_PARAM_SOUND_VOL;
			cmd.args[2] = (uint16_t)arg1;
			break;
		case XW_MUSIC_TRIGGER_JUMP:
			cmd.opcode = IMUSE_CMD_JUMP_MIDI;
			cmd.args[1] = arg1 + 1;
			cmd.args[2] = arg2 / 4 + 1;
			cmd.args[3] = arg2 % 4 + 1;
			cmd.args[4] = arg3;
			cmd.args[5] = 1;
			break;
		case XW_MUSIC_TRIGGER_SET_HOOK:
			cmd.opcode = IMUSE_CMD_SET_HOOK;
			cmd.args[1] = (uint16_t)arg2;
			break;
		case XW_MUSIC_TRIGGER_FADE_VOLUME:
			cmd.opcode = IMUSE_CMD_FADE_PARAM;
			cmd.args[1] = IMUSE_PARAM_SOUND_VOL;
			cmd.args[2] = (uint16_t)arg1;
			cmd.args[3] = (uint16_t)arg2;
			break;
		case XW_MUSIC_TRIGGER_SET_SPEED:
			cmd.opcode = IMUSE_CMD_SET_PARAM;
			cmd.args[1] = IMUSE_PARAM_MIDI_SPEED;
			cmd.args[2] = (uint16_t)arg1;
			break;
		case XW_MUSIC_TRIGGER_SHARE_PARTS:
			cmd.opcode = IMUSE_CMD_SHARE_PARTS;
			cmd.args[1] = arg1;
			break;
		default:
			return 0;
	}
	imuse_set_trigger(im, (intptr_t)owner, g_soundTriggerContext.marker, &cmd);
	if (command == XW_MUSIC_TRIGGER_START) {
		cmd.opcode = IMUSE_CMD_SET_PARAM;
		cmd.args[1] = IMUSE_PARAM_SOUND_GROUP;
		cmd.args[2] = IMUSE_GROUP_MUSIC;
		imuse_set_trigger(im, (intptr_t)owner, g_soundTriggerContext.marker, &cmd);
	}

	return 0;
}

// FUNCTION: DOS94 0x12714
int16_t Dos94_soundext_ClearTriggers(void) {
	if (!g_dos94Imuse)
		return 0;
	/* Native scene cleanup leaves unrelated digital markers intact. */
	for (Sound* sound = xsound_Ask_Sound_List(); sound; sound = sound->next)
		if (sound->type == gmidiSound)
			imuse_clear_trigger(g_dos94Imuse, (intptr_t)sound, -1, -1);
	return 0;
}

// FUNCTION: DOS94 0x12726
int16_t Dos94_soundext_CheckTriggers(void) {
	if (!g_dos94Imuse)
		return 0;
	for (Sound* sound = xsound_Ask_Sound_List(); sound; sound = sound->next)
		if (sound->type == gmidiSound && imuse_check_trigger(g_dos94Imuse, (intptr_t)sound, -1, -1))
			return 1;
	return 0;
}

// FUNCTION: DOS94 0x12750
int16_t Dos94_soundext_ShareParts(const Sound* first, const Sound* second) {
	return imuse_share_parts(g_dos94Imuse, (intptr_t)first, (intptr_t)second);
}

// FUNCTION: DOS94 0x101F4E
void Dos94_soundext_Action_iMuse(XwSoundAction action, Sound* sound, int16_t value, int16_t duration) {
	if (!g_dos94Imuse)
		return;
	if (action > XW_SOUND_ACTION_RESUME &&
		(!sound || (sound->type != digitalSound && !XwMusicPolicy_UsesImuse())))
		return;
	intptr_t id = (intptr_t)sound;
	switch (action) {
		case XW_SOUND_ACTION_PAUSE:
			imuse_pause(g_dos94Imuse);
			g_soundActionSavedGroupVolume = imuse_set_group_volume(g_dos94Imuse, IMUSE_GROUP_MASTER, 0);
			break;
		case XW_SOUND_ACTION_RESUME:
			imuse_set_group_volume(g_dos94Imuse, IMUSE_GROUP_MASTER, g_soundActionSavedGroupVolume);
			imuse_resume(g_dos94Imuse);
			break;
		case XW_SOUND_ACTION_START_MUSIC:
			Dos94_hilevel_ImStartMusic(id);
			break;
		case XW_SOUND_ACTION_START_SFX:
			Dos94_hilevel_ImStartSfx(id);
			break;
		case XW_SOUND_ACTION_START_SPEECH:
			Dos94_hilevel_ImStartVoice(id);
			break;
		case XW_SOUND_ACTION_STOP_SOUND:
			imuse_stop_sound(g_dos94Imuse, id);
			break;
		case XW_SOUND_ACTION_SET_VOLUME:
			imuse_set_param(g_dos94Imuse, id, IMUSE_PARAM_SOUND_VOL, value > 127 ? 127 : value);
			break;
		case XW_SOUND_ACTION_SET_PAN:
			imuse_set_param(g_dos94Imuse, id, IMUSE_PARAM_SOUND_PAN, value);
			break;
		case XW_SOUND_ACTION_FADE_VOLUME:
			imuse_fade_param(g_dos94Imuse, id, IMUSE_PARAM_SOUND_VOL, value > 127 ? 127 : value, duration);
			break;
		case XW_SOUND_ACTION_FADE_PAN:
			imuse_fade_param(g_dos94Imuse, id, IMUSE_PARAM_SOUND_PAN, value, duration);
			break;
	}
}
