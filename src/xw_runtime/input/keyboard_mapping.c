#include "xw_runtime/input/keyboard_mapping.h"

#include "aeron/aeron.h"
#include "xw_runtime/runtime/port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct KeyboardPress {
	XwInputAction action;
	bool down;
	bool ignored;
} KeyboardPress;

static struct {
	uint16_t actions[AERON_KEY_COUNT][16];
	KeyboardPress pressed[AERON_KEY_COUNT];
	bool debug_available;
	bool enabled;
	uint16_t holds[XW_INPUT_ACTION_COUNT];
	uint8_t queue[256];
	unsigned read, write;
	uint16_t pending;
	uint16_t observed;
} g_keyboard;

void XwKeyboardMapping_SetPolicy(bool debug_available) { g_keyboard.debug_available = debug_available; }

XwKeyboardShortcut XwKeyboardMapping_Shortcut(AeronKeyChord source) {
	if (source.key == AERON_KEY_TAB && !source.modifiers)
		return XW_KEYBOARD_SHORTCUT_RENDERER;
	if (source.key == AERON_KEY_ESCAPE)
		return XW_KEYBOARD_SHORTCUT_SETTINGS;
	if (source.key == AERON_KEY_GRAVE && g_keyboard.debug_available)
		return XW_KEYBOARD_SHORTCUT_DEBUG;
	if (source.key == AERON_KEY_A + ('m' - 'a') &&
		(source.modifiers & (AERON_KEY_MOD_CTRL | AERON_KEY_MOD_ALT)) ==
			(AERON_KEY_MOD_CTRL | AERON_KEY_MOD_ALT))
		return XW_KEYBOARD_SHORTCUT_MOUSE;
	return XW_KEYBOARD_SHORTCUT_NONE;
}

int XwKeyboardMapping_Trigger(const AeronInputSnapshot* input, XwKeyboardShortcut shortcut) {
	if (!input || !input->has_focus || input->key_events_overflow)
		return -1;
	for (uint16_t i = 0; i < input->key_event_count; ++i) {
		const AeronKeyEvent* event = &input->key_events[i];
		if (event->down && !event->repeat && XwKeyboardMapping_Shortcut(event->chord) == shortcut)
			return event->chord.key;
	}
	return -1;
}

bool XwKeyboardMapping_SourceValid(AeronKeyChord source) {
	return source.key > 0 && source.key < AERON_KEY_COUNT && source.modifiers < 16 &&
		   AeronKey_Name((AeronKey)source.key)[0] &&
		   (!AeronKey_Modifier((AeronKey)source.key) || source.modifiers == 0) &&
		   XwKeyboardMapping_Shortcut(source) == XW_KEYBOARD_SHORTCUT_NONE;
}

#if defined(__APPLE__)
#define GUI_MODIFIER_LABEL "Command+"
#else
#define GUI_MODIFIER_LABEL "Super+"
#endif

void XwKeyboardMapping_FormatSource(char* text, size_t capacity, AeronKeyChord source) {
	snprintf(text, capacity, "%s%s%s%s%s", (source.modifiers & AERON_KEY_MOD_CTRL) ? "Ctrl+" : "",
			 (source.modifiers & AERON_KEY_MOD_ALT) ? "Alt+" : "",
			 (source.modifiers & AERON_KEY_MOD_SHIFT) ? "Shift+" : "",
			 (source.modifiers & AERON_KEY_MOD_GUI) ? GUI_MODIFIER_LABEL : "",
			 AeronKey_Name((AeronKey)source.key));
}

size_t XwKeyboardMapping_Find(const XwKeyboardBindings* profile, AeronKeyChord source) {
	for (size_t i = 0; i < profile->count; ++i)
		if (profile->bindings[i].source.key == source.key &&
			profile->bindings[i].source.modifiers == source.modifiers)
			return i;
	return SIZE_MAX;
}

static int BindingCompare(const void* left, const void* right) {
	const XwKeyboardBinding* a = left;
	const XwKeyboardBinding* b = right;
	if (a->action != b->action)
		return (int)a->action - (int)b->action;
	if (a->source.key != b->source.key)
		return (int)a->source.key - (int)b->source.key;
	return (int)a->source.modifiers - (int)b->source.modifiers;
}

void XwKeyboardMapping_Sort(XwKeyboardBindings* profile) {
	qsort(profile->bindings, profile->count, sizeof profile->bindings[0], BindingCompare);
}

bool XwKeyboardMapping_Equal(const XwKeyboardBindings* a, const XwKeyboardBindings* b) {
	if (a->count != b->count)
		return false;
	for (size_t i = 0; i < a->count; ++i)
		if (BindingCompare(&a->bindings[i], &b->bindings[i]))
			return false;
	return true;
}

void XwKeyboardMapping_Remove(XwKeyboardBindings* profile, size_t index) {
	if (index >= profile->count)
		return;
	memmove(&profile->bindings[index], &profile->bindings[index + 1],
			(profile->count - index - 1) * sizeof profile->bindings[0]);
	--profile->count;
}

static uint16_t ButtonBit(XwInputAction action) {
	return action == XW_INPUT_ACTION_FIRE_WEAPON ? 1 : action == XW_INPUT_ACTION_TARGET_ROLL_MODIFIER ? 2 : 0;
}

static void Dispatch(XwInputAction action, bool down, bool repeat) {
	uint16_t bit = ButtonBit(action);
	uint16_t release = XwInputActions_ReleaseKey(action);
	if ((bit || release) && !repeat) {
		if (down) {
			if (g_keyboard.holds[action]++)
				return;
		} else if (!g_keyboard.holds[action] || --g_keyboard.holds[action])
			return;
	}
	if (bit || (!down && !release) ||
		(repeat && (action == XW_INPUT_ACTION_PAUSE || action == XW_INPUT_ACTION_ESCAPE)))
		return;
	unsigned next = (g_keyboard.write + 1) % 256;
	if (next == g_keyboard.read) {
		Aeron_LogWarn("xw.input", "Keyboard command queue is full");
		return;
	}
	g_keyboard.queue[g_keyboard.write] = (uint8_t)(down ? XwInputActions_Key(action) : release);
	g_keyboard.write = next;
}

void XwKeyboardMapping_Suspend(void) {
	memset(g_keyboard.pressed, 0, sizeof g_keyboard.pressed);
	memset(g_keyboard.holds, 0, sizeof g_keyboard.holds);
	g_keyboard.read = g_keyboard.write = 0;
	g_keyboard.pending = g_keyboard.observed = 0;
	g_keyboard.enabled = false;
}

static void Compile(uint16_t table[AERON_KEY_COUNT][16], const XwKeyboardBindings* profile) {
	memset(table, 0, sizeof g_keyboard.actions);
	for (size_t i = 0; i < profile->count; ++i) {
		const XwKeyboardBinding* b = &profile->bindings[i];
		if (XwKeyboardMapping_SourceValid(b->source))
			table[b->source.key][b->source.modifiers] = (uint16_t)b->action;
	}
}

void XwKeyboardMapping_Install(const XwKeyboardBindings* profile) {
	XwKeyboardMapping_Suspend();
	Compile(g_keyboard.actions, profile);
}

void XwKeyboardMapping_Enable(bool enabled, const AeronInputSnapshot* input) {
	if (enabled == g_keyboard.enabled)
		return;
	XwKeyboardMapping_Suspend();
	g_keyboard.enabled = enabled;
	if (enabled && input)
		for (int key = 0; key < AERON_KEY_COUNT; ++key)
			g_keyboard.pressed[key].ignored = input->key_down[key] != 0;
}

void XwKeyboardMapping_BeginFrame(const AeronInputSnapshot* input) {
	g_keyboard.pending &= (uint16_t)~g_keyboard.observed;
	g_keyboard.observed = 0;
	if (input->key_events_overflow && g_keyboard.enabled) {
		XwKeyboardMapping_Suspend();
		XwKeyboardMapping_Enable(true, input);
	}
}

void XwKeyboardMapping_Event(const AeronKeyEvent* event, bool suppressed) {
	if (!g_keyboard.enabled || event->chord.key >= AERON_KEY_COUNT)
		return;
	KeyboardPress* press = &g_keyboard.pressed[event->chord.key];
	if (!event->down) {
		if (press->down && press->action != XW_INPUT_ACTION_NONE)
			Dispatch(press->action, false, false);
		*press = (KeyboardPress) { 0 };
		return;
	}
	if (press->ignored)
		return;
	if (suppressed) {
		if (press->down && press->action != XW_INPUT_ACTION_NONE)
			Dispatch(press->action, false, false);
		*press = (KeyboardPress) { .ignored = true };
		return;
	}
	if (!press->down) {
		if (event->repeat)
			return; /* Resuming never turns typematic into a fresh press. */
		AeronKeyChord chord = event->chord;
		chord.modifiers &= (uint8_t)~AeronKey_Modifier((AeronKey)chord.key);
		press->down = true;
		if (XwKeyboardMapping_Shortcut(event->chord) != XW_KEYBOARD_SHORTCUT_NONE)
			return;
		press->action = (XwInputAction)g_keyboard.actions[chord.key][chord.modifiers];
		if (press->action != XW_INPUT_ACTION_NONE)
			g_keyboard.pending |= ButtonBit(press->action);
	} else if (!event->repeat) {
		return;
	}
	if (press->action != XW_INPUT_ACTION_NONE)
		Dispatch(press->action, true, event->repeat != 0);
}

uint16_t XwKeyboardMapping_ReadKey(void) {
	if (g_keyboard.read == g_keyboard.write)
		return 0;
	uint16_t key = g_keyboard.queue[g_keyboard.read];
	g_keyboard.read = (g_keyboard.read + 1) % 256;
	return key;
}

uint16_t XwKeyboardMapping_ReadButtons(void) {
	g_keyboard.observed |= g_keyboard.pending;
	return (g_keyboard.holds[XW_INPUT_ACTION_FIRE_WEAPON] ? 1 : 0) |
		   (g_keyboard.holds[XW_INPUT_ACTION_TARGET_ROLL_MODIFIER] ? 2 : 0) | g_keyboard.pending;
}
