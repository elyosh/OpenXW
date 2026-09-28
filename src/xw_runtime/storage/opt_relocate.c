/* Native OPT loading/relocation adapted from OpenXvT. */
#include "xw_runtime/storage/opt_native.h"
#include "xw_runtime/storage/storage.h"
#include <stdlib.h>

typedef struct XwOptRelocation {
	void** visited;
	size_t count, capacity;
	intptr_t delta;
} XwOptRelocation;

static void* XwOpt_Move(const void* pointer, intptr_t delta) {
	return pointer ? (void*)((uintptr_t)pointer + (uintptr_t)delta) : NULL;
}

static int XwOpt_Seen(XwOptRelocation* state, void* pointer) {
	for (size_t i = 0; i < state->count; ++i)
		if (state->visited[i] == pointer)
			return 1;
	if (state->count == state->capacity) {
		size_t capacity = state->capacity ? state->capacity * 2 : 64;
		void** grown = realloc(state->visited, capacity * sizeof(*grown));
		if (!grown) {
			XwStorage_Fatal("Cannot relocate OPT graph", 1);
			return 1;
		}
		state->visited = grown;
		state->capacity = capacity;
	}
	state->visited[state->count++] = pointer;
	return 0;
}

static void XwOpt_MoveNode(XwOptRelocation* state, OptNode* node, unsigned depth) {
	if (!node || XwOpt_Seen(state, node))
		return;
	if (depth >= 256) {
		XwStorage_Fatal("OPT graph exceeds relocation depth", 1);
		return;
	}
	node->pName = XwOpt_Move(node->pName, state->delta);
	node->param2 = XwOpt_Move(node->param2, state->delta);
	if (node->nodeType == OPT_TEXTURE && node->param2 && !XwOpt_Seen(state, node->param2)) {
		OptTextureData* texture = node->param2;
		if (!texture->paletteType)
			texture->palette = XwOpt_Move(texture->palette, state->delta);
	}
	node->pChildren = XwOpt_Move(node->pChildren, state->delta);
	if (node->pChildren && !XwOpt_Seen(state, node->pChildren)) {
		for (int i = 0; i < node->childCount; ++i) {
			node->pChildren[i] = XwOpt_Move(node->pChildren[i], state->delta);
			XwOpt_MoveNode(state, node->pChildren[i], depth + 1);
		}
	}
}

void XwOpt_RelocateNode(OptNode* node, intptr_t delta) {
	XwOptRelocation state = { .delta = delta };
	XwOpt_MoveNode(&state, node, 0);
	free(state.visited);
}

void XwOpt_Relocate(OptimizedPolyObject* model) {
	if (!model || model->selfMarker == model)
		return;
	XwOptRelocation state = { .delta = (intptr_t)((uintptr_t)model - (uintptr_t)model->selfMarker) };
	model->selfMarker = model;
	model->rootNodes = XwOpt_Move(model->rootNodes, state.delta);
	for (int i = 0; i < model->rootNodeCount; ++i) {
		model->rootNodes[i] = XwOpt_Move(model->rootNodes[i], state.delta);
		XwOpt_MoveNode(&state, model->rootNodes[i], 0);
	}
	free(state.visited);
}
