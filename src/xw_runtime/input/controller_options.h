#ifndef XW_RUNTIME_INPUT_CONTROLLER_OPTIONS_H
#define XW_RUNTIME_INPUT_CONTROLLER_OPTIONS_H
#include "aeron/input.h"
#include "xw_runtime/input/actions.h"
#include <stddef.h>

typedef enum XwInputAxis {
	XW_INPUT_AXIS_YAW,
	XW_INPUT_AXIS_PITCH,
	XW_INPUT_AXIS_ROLL,
	XW_INPUT_AXIS_THROTTLE,
	XW_INPUT_AXIS_COUNT
} XwInputAxis;

typedef struct XwInputAxisBinding {
	int8_t source;
	bool invert;
	float deadzone;
} XwInputAxisBinding;

typedef struct XwInputMapping {
	XwInputAxisBinding axes[XW_INPUT_AXIS_COUNT];
} XwInputMapping;

enum {
	XW_CONTROLLER_BINDING_CAP =
		AERON_CONTROLLER_BUTTON_MAX + 2 * AERON_CONTROLLER_AXIS_MAX + 4 * AERON_CONTROLLER_HAT_MAX
};

typedef struct XwInputActionBinding {
	AeronControllerDigitalSource source;
	XwInputAction action;
} XwInputActionBinding;

typedef struct XwControllerProfile {
	XwInputMapping mapping;
	XwInputActionBinding bindings[XW_CONTROLLER_BINDING_CAP];
	size_t binding_count;
} XwControllerProfile;

enum { XW_CONTROLLER_MODEL_CAP = 8 };

typedef struct XwControllerModel {
	char guid[33];
	char name[AERON_CONTROLLER_NAME_CAPACITY];
	AeronControllerKind kind; /* Saved Aeron kind; must match the connected snapshot. */
	XwControllerProfile profile;
} XwControllerModel;

typedef struct XwControllerOptions {
	XwControllerModel models[XW_CONTROLLER_MODEL_CAP];
	size_t count;
} XwControllerOptions;

bool XwControllerOptions_Equals(const XwControllerOptions* left, const XwControllerOptions* right);
bool XwControllerOptions_ValidateProfile(const XwControllerProfile* profile, AeronControllerKind kind,
										 char* error, size_t capacity);
bool XwControllerOptions_EffectiveAxisInvert(AeronControllerKind kind, XwInputAxis axis, bool invert);
bool XwControllerOptions_ProfileEqual(const XwControllerProfile* left, const XwControllerProfile* right);
void XwControllerOptions_ClearProfile(XwControllerProfile* profile, AeronControllerKind kind);
int XwControllerOptions_FindModel(const XwControllerOptions* options, const char* guid);
bool XwControllerOptions_Validate(const XwControllerOptions* options, char* error, size_t capacity);
bool XwControllerOptions_AddModel(XwControllerOptions* options, const AeronControllerSnapshot* device,
								  char* error, size_t capacity);
bool XwControllerOptions_InitializeGamepads(XwControllerOptions* options, const XwControllerProfile* defaults,
											const AeronInputSnapshot* input, char* error, size_t capacity);
#endif
