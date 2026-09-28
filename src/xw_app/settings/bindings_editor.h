#ifndef XW_BINDINGS_EDITOR_H
#define XW_BINDINGS_EDITOR_H

#include "aeron/scene/ui.h"
#include "xw_runtime/input/actions.h"

typedef struct XwBindingsEditor {
	int category;
	size_t action_selected;
	size_t binding_selected;
	XwInputAction selected_action;
	int binding_modal_open;
} XwBindingsEditor;

void XwBindingsEditor_Init(XwBindingsEditor* editor);
void XwBindingsEditor_Select(XwBindingsEditor* editor, XwInputAction action, bool open_modal);
void XwBindingsEditor_Category(XwBindingsEditor* editor, AeronUiContext* ui);
void XwBindingsEditor_Actions(XwBindingsEditor* editor, AeronUiContext* ui, const AeronUiListItem* items,
							  size_t count, float trailing_height);
bool XwBindingsEditor_BeginDetail(XwBindingsEditor* editor, AeronUiContext* ui);
void XwBindingsEditor_List(XwBindingsEditor* editor, AeronUiContext* ui, const AeronUiListItem* items,
						   size_t count, const char* empty_text);
bool XwBindingsEditor_Remove(AeronUiContext* ui);
void XwBindingsEditor_EndDetail(XwBindingsEditor* editor, AeronUiContext* ui);
/* Returns true only when Replace is chosen. The dialog owns its open flag. */
bool XwBindingsEditor_Conflict(AeronUiContext* ui, int* open, const char* source, XwInputAction previous,
							   XwInputAction replacement);
#endif
