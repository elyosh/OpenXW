#ifndef XW_KEYBOARD_MAPPING_H
#define XW_KEYBOARD_MAPPING_H

#include "aeron/input.h"
#include "xw_runtime/input/actions.h"
#include <stddef.h>

#define XW_KEYBOARD_BINDING_CAP 256

typedef struct XwKeyboardBinding {
	AeronKeyChord source;
	XwInputAction action;
} XwKeyboardBinding;

typedef struct XwKeyboardBindings {
	XwKeyboardBinding bindings[XW_KEYBOARD_BINDING_CAP];
	size_t count;
} XwKeyboardBindings;

typedef enum XwKeyboardShortcut {
	XW_KEYBOARD_SHORTCUT_NONE,
	XW_KEYBOARD_SHORTCUT_SETTINGS,
	XW_KEYBOARD_SHORTCUT_DEBUG,
	XW_KEYBOARD_SHORTCUT_MOUSE,
	XW_KEYBOARD_SHORTCUT_RENDERER,
} XwKeyboardShortcut;

void XwKeyboardMapping_SetPolicy(bool debug_available);
int XwKeyboardMapping_Trigger(const AeronInputSnapshot* input, XwKeyboardShortcut shortcut);
XwKeyboardShortcut XwKeyboardMapping_Shortcut(AeronKeyChord source);
bool XwKeyboardMapping_SourceValid(AeronKeyChord source);
void XwKeyboardMapping_FormatSource(char* text, size_t capacity, AeronKeyChord source);
size_t XwKeyboardMapping_Find(const XwKeyboardBindings* profile, AeronKeyChord source);
bool XwKeyboardMapping_Equal(const XwKeyboardBindings* a, const XwKeyboardBindings* b);
void XwKeyboardMapping_Sort(XwKeyboardBindings* profile);
void XwKeyboardMapping_Remove(XwKeyboardBindings* profile, size_t index);
void XwKeyboardMapping_Install(const XwKeyboardBindings* profile);
void XwKeyboardMapping_Suspend(void);
void XwKeyboardMapping_Enable(bool enabled, const AeronInputSnapshot* input);
void XwKeyboardMapping_BeginFrame(const AeronInputSnapshot* input);
void XwKeyboardMapping_Event(const AeronKeyEvent* event, bool suppressed);
uint16_t XwKeyboardMapping_ReadKey(void);
uint16_t XwKeyboardMapping_ReadButtons(void);

#endif
