#include "xw_runtime/input/controller_options.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static bool XwControllerOptions_SameSource(const AeronControllerDigitalSource* left,
										   const AeronControllerDigitalSource* right) {
	return left->kind == right->kind && left->index == right->index &&
		   (left->kind != AERON_CONTROLLER_DIGITAL_HAT || left->hat_direction == right->hat_direction);
}

bool XwControllerOptions_ProfileEqual(const XwControllerProfile* left, const XwControllerProfile* right) {
	if (left->binding_count != right->binding_count)
		return false;
	for (int axis = 0; axis < XW_INPUT_AXIS_COUNT; ++axis) {
		const XwInputAxisBinding* a = &left->mapping.axes[axis];
		const XwInputAxisBinding* b = &right->mapping.axes[axis];
		if (a->source != b->source || a->invert != b->invert || a->deadzone != b->deadzone)
			return false;
	}
	for (size_t index = 0; index < left->binding_count; ++index) {
		const XwInputActionBinding* a = &left->bindings[index];
		const XwInputActionBinding* b = &right->bindings[index];
		if (a->action != b->action || !XwControllerOptions_SameSource(&a->source, &b->source) ||
			a->source.threshold != b->source.threshold)
			return false;
	}
	return true;
}

bool XwControllerOptions_Equals(const XwControllerOptions* left, const XwControllerOptions* right) {
	if (!left || !right)
		return left == right;
	if (left->count != right->count)
		return false;
	for (size_t i = 0; i < left->count; ++i) {
		const XwControllerModel *a = &left->models[i], *b = &right->models[i];
		if (strcmp(a->guid, b->guid) || strcmp(a->name, b->name) || a->kind != b->kind ||
			!XwControllerOptions_ProfileEqual(&a->profile, &b->profile))
			return false;
	}
	return true;
}

bool XwControllerOptions_EffectiveAxisInvert(AeronControllerKind kind, XwInputAxis axis, bool invert) {
	/* X-Wing consumes the original joystick Y convention directly. */
	(void)kind;
	(void)axis;
	return invert;
}

static bool XwControllerOptions_ValidationError(char* error, size_t capacity, const char* message) {
	if (error && capacity)
		snprintf(error, capacity, "%s", message);
	return false;
}

bool XwControllerOptions_ValidateProfile(const XwControllerProfile* profile, AeronControllerKind kind,
										 char* error, size_t capacity) {
	const int axis_limit =
		kind == AERON_CONTROLLER_KIND_GAMEPAD ? AERON_GAMEPAD_AXIS_COUNT : AERON_CONTROLLER_AXIS_MAX;
	const int button_limit =
		kind == AERON_CONTROLLER_KIND_GAMEPAD ? AERON_GAMEPAD_BUTTON_COUNT : AERON_CONTROLLER_BUTTON_MAX;
	if (!profile || (kind != AERON_CONTROLLER_KIND_GAMEPAD && kind != AERON_CONTROLLER_KIND_JOYSTICK))
		return XwControllerOptions_ValidationError(error, capacity, "invalid controller profile");
	if (profile->binding_count > XW_CONTROLLER_BINDING_CAP)
		return XwControllerOptions_ValidationError(error, capacity, "controller binding capacity exceeded");
	for (int axis = 0; axis < XW_INPUT_AXIS_COUNT; ++axis) {
		const XwInputAxisBinding* binding = &profile->mapping.axes[axis];
		if (binding->source < -1 || binding->source >= axis_limit || !isfinite(binding->deadzone) ||
			binding->deadzone < 0.0f || binding->deadzone > (axis == XW_INPUT_AXIS_THROTTLE ? 0.0f : 1.0f))
			return XwControllerOptions_ValidationError(error, capacity, "controller axis binding is invalid");
		if (binding->source < 0)
			continue;
		for (int prior = 0; prior < axis; ++prior)
			if (profile->mapping.axes[prior].source == binding->source)
				return XwControllerOptions_ValidationError(
					error, capacity, "controller axis source is assigned more than once");
	}
	for (size_t index = 0; index < profile->binding_count; ++index) {
		const XwInputActionBinding* binding = &profile->bindings[index];
		const AeronControllerDigitalSource* source = &binding->source;
		if (binding->action <= XW_INPUT_ACTION_NONE || binding->action >= XW_INPUT_ACTION_COUNT)
			return XwControllerOptions_ValidationError(error, capacity,
													   "controller action binding is invalid");
		if (!isfinite(source->threshold) || source->threshold <= 0.0f || source->threshold > 1.0f)
			return XwControllerOptions_ValidationError(error, capacity,
													   "controller digital threshold is invalid");
		switch (source->kind) {
			case AERON_CONTROLLER_DIGITAL_BUTTON:
				if (source->index >= button_limit)
					return XwControllerOptions_ValidationError(error, capacity,
															   "controller button index is out of range");
				break;
			case AERON_CONTROLLER_DIGITAL_AXIS_POSITIVE:
			case AERON_CONTROLLER_DIGITAL_AXIS_NEGATIVE:
				if (source->index >= axis_limit)
					return XwControllerOptions_ValidationError(error, capacity,
															   "controller digital axis is invalid");
				break;
			case AERON_CONTROLLER_DIGITAL_HAT:
				if (kind != AERON_CONTROLLER_KIND_JOYSTICK || source->index >= AERON_CONTROLLER_HAT_MAX ||
					(source->hat_direction != AERON_CONTROLLER_HAT_UP &&
					 source->hat_direction != AERON_CONTROLLER_HAT_RIGHT &&
					 source->hat_direction != AERON_CONTROLLER_HAT_DOWN &&
					 source->hat_direction != AERON_CONTROLLER_HAT_LEFT))
					return XwControllerOptions_ValidationError(error, capacity,
															   "controller hat binding is invalid");
				break;
			default:
				return XwControllerOptions_ValidationError(error, capacity,
														   "controller source kind is invalid");
		}
		for (size_t prior = 0; prior < index; ++prior)
			if (XwControllerOptions_SameSource(source, &profile->bindings[prior].source))
				return XwControllerOptions_ValidationError(error, capacity,
														   "controller source is bound more than once");
	}
	return true;
}

void XwControllerOptions_ClearProfile(XwControllerProfile* profile, AeronControllerKind kind) {
	memset(profile, 0, sizeof *profile);
	for (int i = 0; i < XW_INPUT_AXIS_COUNT; ++i)
		profile->mapping.axes[i].source = -1;
	/* Raw HOTAS levers commonly decrease their reported value toward full power. */
	profile->mapping.axes[XW_INPUT_AXIS_THROTTLE].invert = kind == AERON_CONTROLLER_KIND_JOYSTICK;
}

int XwControllerOptions_FindModel(const XwControllerOptions* options, const char* guid) {
	if (options && guid)
		for (size_t i = 0; i < options->count; ++i)
			if (!strcmp(options->models[i].guid, guid))
				return (int)i;
	return -1;
}

static bool GuidValid(const char* guid) {
	bool nonzero = false;
	for (int i = 0; i < 32; ++i) {
		if (!((guid[i] >= '0' && guid[i] <= '9') || (guid[i] >= 'a' && guid[i] <= 'f')))
			return false;
		nonzero |= guid[i] != '0';
	}
	return nonzero && guid[32] == 0;
}

bool XwControllerOptions_Validate(const XwControllerOptions* options, char* error, size_t capacity) {
	if (!options || options->count > XW_CONTROLLER_MODEL_CAP)
		return XwControllerOptions_ValidationError(error, capacity, "controller model capacity exceeded");
	for (size_t i = 0; i < options->count; ++i) {
		const XwControllerModel* m = &options->models[i];
		if (!GuidValid(m->guid) || !memchr(m->name, 0, sizeof m->name))
			return XwControllerOptions_ValidationError(error, capacity, "invalid controller model identity");
		if (!XwControllerOptions_ValidateProfile(&m->profile, m->kind, error, capacity))
			return false;
		for (size_t j = 0; j < i; ++j) {
			if (!strcmp(m->guid, options->models[j].guid))
				return XwControllerOptions_ValidationError(error, capacity,
														   "duplicate controller model GUID");
			for (int axis = 0; axis < XW_INPUT_AXIS_COUNT; ++axis)
				if (m->profile.mapping.axes[axis].source >= 0 &&
					options->models[j].profile.mapping.axes[axis].source >= 0)
					return XwControllerOptions_ValidationError(error, capacity,
															   "logical axis has multiple model owners");
		}
	}
	return true;
}

bool XwControllerOptions_AddModel(XwControllerOptions* options, const AeronControllerSnapshot* device,
								  char* error, size_t capacity) {
	if (!device || !GuidValid(device->guid))
		return XwControllerOptions_ValidationError(error, capacity, "controller has no usable model GUID");
	if (XwControllerOptions_FindModel(options, device->guid) >= 0)
		return true;
	if (options->count >= XW_CONTROLLER_MODEL_CAP)
		return XwControllerOptions_ValidationError(error, capacity, "controller model capacity exceeded");
	XwControllerModel* m = &options->models[options->count++];
	memset(m, 0, sizeof *m);
	memcpy(m->guid, device->guid, sizeof m->guid);
	snprintf(m->name, sizeof m->name, "%s", device->name);
	m->kind = device->kind;
	XwControllerOptions_ClearProfile(&m->profile, device->kind);
	return true;
}

bool XwControllerOptions_InitializeGamepads(XwControllerOptions* options, const XwControllerProfile* defaults,
											const AeronInputSnapshot* input, char* error, size_t capacity) {
	const AeronControllerSnapshot* sorted[AERON_CONTROLLER_MAX];
	int count = 0;
	if (!input)
		return true;
	for (int slot = 0; slot < AERON_CONTROLLER_MAX; ++slot) {
		const AeronControllerSnapshot* d = &input->controllers[slot];
		if (!d->connected || d->kind != AERON_CONTROLLER_KIND_GAMEPAD)
			continue;
		int j = count++;
		while (j > 0 && strcmp(sorted[j - 1]->guid, d->guid) > 0) {
			sorted[j] = sorted[j - 1];
			--j;
		}
		sorted[j] = d;
	}
	for (int i = 0; i < count; ++i) {
		if (XwControllerOptions_FindModel(options, sorted[i]->guid) >= 0)
			continue;
		if (!XwControllerOptions_AddModel(options, sorted[i], error, capacity))
			return false;
		XwControllerProfile* p = &options->models[options->count - 1].profile;
		*p = *defaults;
		for (size_t j = 0; j + 1 < options->count; ++j)
			for (int axis = 0; axis < XW_INPUT_AXIS_COUNT; ++axis)
				if (options->models[j].profile.mapping.axes[axis].source >= 0)
					p->mapping.axes[axis].source = -1;
	}
	return true;
}
