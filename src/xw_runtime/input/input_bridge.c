/* Raw text conversion follows OpenXvT; the queue exposes Landru's BIOS key contract. */
#include "xw_runtime/input/input_bridge.h"
#include "xw/flight/feinput.h"
#include "xw/flight/flight_input.h"
#include "xw/flight/player/user.h"
#include "xw/frontend/register.h"
#include "xw/input/dinput.h"
#include "xw/input/joystick.h"
#include "xw/input/win_mouse.h"
#include "xw/landru_config.h"
#include "xw_runtime/config/config.h"
#include "xw_runtime/config/controller_config.h"
#include "xw_runtime/input/capture.h"
#include "xw_runtime/input/controller_mapping.h"
#include "xw_runtime/input/flight_controls.h"
#include "xw_runtime/input/keyboard_mapping.h"
#include "xw_runtime/input/mouse_flight.h"
#include "xw_runtime/runtime/flight_sim.h"
#include "xw_runtime/runtime/frontend_task.h"
#include "xw_runtime/runtime/port.h"
#include "xw_runtime/runtime/presentation.h"
#include "xw_runtime/runtime/replay_save_task.h"
#include <aeron/aeron.h>
#include <aeron/compat/host.h>
#include <landru/dialog.h>
#include <landru/dlgjoy.h>
#include <landru/io.h>
#include <landru/surface.h>
#include <string.h>

enum { KEY_CAPACITY = 256 };

static uint16_t keys[KEY_CAPACITY];
static unsigned read_index, write_index;
static bool scan_pending, have_frame, have_pointer;
static uint64_t last_frame;
static int modifiers, mouse_x, mouse_y, delta_x, delta_y, mouse_buttons;
static int pending_mouse_presses, observed_mouse_presses;
static XwKeyboardRoute route = XW_KEYBOARD_BLOCKED;
static bool suppressed_frame;
static uint8_t flight_keys[KEY_CAPACITY];
static unsigned flight_read, flight_write;
static uint64_t reacquire_frame = UINT64_MAX, config_generation;
static XwKeyboardBindings installed_keyboard;
static uint32_t devices[AERON_CONTROLLER_MAX];
static AeronWinmmJoystickState joystick;
static int connected;
static int32_t cached_axes[3];

static void XwInput_Append(unsigned int key) {
	unsigned next = (write_index + 1) % KEY_CAPACITY;
	if (next == read_index) {
		read_index = write_index;
		scan_pending = false;
	}
	keys[write_index] = (uint16_t)key;
	write_index = next;
}

static unsigned int XwInput_Windows1252(unsigned int cp) {
	static const unsigned short extended[32] = { 0x20ac, 0,      0x201a, 0x0192, 0x201e, 0x2026, 0x2020,
												 0x2021, 0x02c6, 0x2030, 0x0160, 0x2039, 0x0152, 0,
												 0x017d, 0,      0,      0x2018, 0x2019, 0x201c, 0x201d,
												 0x2022, 0x2013, 0x2014, 0x02dc, 0x2122, 0x0161, 0x203a,
												 0x0153, 0,      0x017e, 0x0178 };
	unsigned int i;
	if ((cp >= 32 && cp < 127) || (cp >= 160 && cp <= 255))
		return cp;
	for (i = 0; i < 32; ++i)
		if (cp && cp == extended[i])
			return 128 + i;
	return 0;
}

static void XwInput_Text(const AeronInputSnapshot* input) {
	uint32_t i = 0;
	while (i < input->text_length) {
		unsigned int cp = (unsigned char)input->text[i++];
		unsigned int count = 0;
		unsigned int minimum = 0;
		if (cp >= 0xc2 && cp <= 0xdf) {
			cp &= 31;
			count = 1;
			minimum = 128;
		} else if (cp >= 0xe0 && cp <= 0xef) {
			cp &= 15;
			count = 2;
			minimum = 2048;
		} else if (cp >= 128) {
			continue;
		}
		if (count > input->text_length - i)
			break;
		while (count--) {
			unsigned int tail = (unsigned char)input->text[i++];
			if ((tail & 0xc0) != 0x80) {
				cp = 0;
				break;
			}
			cp = (cp << 6) | (tail & 63);
		}
		if (cp < minimum)
			continue;
		cp = XwInput_Windows1252(cp);
		if (cp)
			XwInput_Append(cp);
	}
}

static unsigned ControlKey(AeronKeyChord chord) {
	unsigned key = chord.key;
	if (key == AERON_KEY_RETURN || key == AERON_KEY_KP_ENTER)
		return 13;
	if (key == AERON_KEY_ESCAPE)
		return 27;
	if (key == AERON_KEY_BACKSPACE)
		return 8;
	if (key == AERON_KEY_TAB)
		return chord.modifiers & AERON_KEY_MOD_SHIFT ? 0x0f00 : 9;
	static const unsigned char navigation[] = { 0x52, 0x47, 0x49, 0x53, 0x4f, 0x51, 0x4d, 0x4b, 0x50, 0x48 };
	if (key >= AERON_KEY_INSERT && key <= AERON_KEY_UP)
		return (unsigned)navigation[key - AERON_KEY_INSERT] << 8;
	if (key >= AERON_KEY_F1 && key <= AERON_KEY_F1 + 9)
		return (key - AERON_KEY_F1 + 0x3b) << 8;
	if (key >= AERON_KEY_A && key < AERON_KEY_A + 26) {
		if (chord.modifiers & AERON_KEY_MOD_CTRL)
			return key - AERON_KEY_A + 1;
		if (chord.modifiers & AERON_KEY_MOD_ALT) {
			static const unsigned char scans[26] = { 0x1e, 0x30, 0x2e, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17,
													 0x24, 0x25, 0x26, 0x32, 0x31, 0x18, 0x19, 0x10, 0x13,
													 0x1f, 0x14, 0x16, 0x2f, 0x11, 0x2d, 0x15, 0x2c };
			return (unsigned)scans[key - AERON_KEY_A] << 8;
		}
	}
	return 0;
}

void XwInput_ResetPointer(void) {
	have_pointer = false;
	delta_x = delta_y = 0;
	XwPresentation_MouseToScene(Aeron_InputSnapshot(), &mouse_x, &mouse_y);
	g_winMousePrevPos = g_winMousePos = g_winMouseCursorPos = (XwMousePosition) { mouse_x, mouse_y };
	if (io_module_gbl) {
		mouse_cursor_x_gbl = mouse_x_gbl = (int16_t)mouse_x;
		mouse_cursor_y_gbl = mouse_y_gbl = (int16_t)mouse_y;
	}
	memset(g_winMouseButtonDown, 0, sizeof g_winMouseButtonDown);
	memset(g_winMouseButtonPressed, 0, sizeof g_winMouseButtonPressed);
	memset(g_winMouseButtonReleased, 0, sizeof g_winMouseButtonReleased);
	mouse_buttons = 0;
	pending_mouse_presses = observed_mouse_presses = 0;
}

void XwInput_SetPointer(int x, int y) {
	mouse_x = x;
	mouse_y = y;
	delta_x = delta_y = 0;
	have_pointer = false;
}

void XwInput_CursorPosition(int* x, int* y) {
	Rect bounds;
	xsurface_Get_Logical_Bounds(&bounds);
	*x = io_module_gbl ? mouse_cursor_x_gbl + delta_x : mouse_x;
	*y = io_module_gbl ? mouse_cursor_y_gbl + delta_y : mouse_y;
	if (*x < 0)
		*x = 0;
	if (*y < 0)
		*y = 0;
	if (*x >= bounds.right)
		*x = bounds.right > 0 ? bounds.right - 1 : 0;
	if (*y >= bounds.bottom)
		*y = bounds.bottom > 0 ? bounds.bottom - 1 : 0;
}

void XwInput_Reset(void) {
	read_index = write_index = 0;
	scan_pending = have_frame = false;
	modifiers = 0;
	XwInput_ClearCommands();
	XwInput_ResetCapture();
	XwInput_ResetPointer();
}

static void UpdateJoystick(void);
static unsigned FlightKey(AeronKeyChord chord);
static unsigned int VirtualKey(int key);

void XwInput_BeginFrame(const AeronInputSnapshot* input, bool suppressed, int32_t delta_us) {
	if (!input) {
		XwInput_Reset();
		return;
	}
	if (have_frame && input->frame_id == last_frame)
		return;
	have_frame = true;
	last_frame = input->frame_id;
	suppressed = suppressed || !input->has_focus;
	suppressed_frame = suppressed;
	XwInput_ReconcileKeyboard();
	XwControllerMapping_Update(input);
	XwFlightControls_UpdateThrottleContext();
	XwInput_UpdateMouseCapture(input, delta_us);
	XwKeyboardMapping_BeginFrame(input);
	if (route == XW_KEYBOARD_GAMEPLAY) {
		for (unsigned i = 0; !input->key_events_overflow && i < input->key_event_count; ++i)
			XwKeyboardMapping_Event(&input->key_events[i],
									XwInput_KeyBlocked(input->key_events[i].chord.key));
	}
	UpdateJoystick();
	if (route == XW_KEYBOARD_RAW && XwFlightSim_IsPaused())
		for (int i = 0; i < AERON_CONTROLLER_MAX; ++i)
			if (input->controllers[i].connected &&
				(input->controllers[i].gamepad_pressed_buttons & (1u << AERON_GAMEPAD_BUTTON_START)))
				XwInput_QueueFlightKey('p');
	memset(g_windowVirtualKeyDown, 0, sizeof g_windowVirtualKeyDown);
	g_lastReleasedVirtualKey = 0;
	for (int key = 0; key < AERON_KEY_COUNT; ++key) {
		if (XwInput_KeyBlocked(key))
			continue;
		unsigned vk = VirtualKey(key);
		if (vk < FLIGHT_VIRTUAL_KEY_COUNT && vk) {
			g_windowVirtualKeyDown[vk] = input->key_down[key] != 0;
			if (input->key_released[key])
				g_lastReleasedVirtualKey = vk;
		}
	}
	g_windowVirtualKeyDown[0x10] = g_windowVirtualKeyDown[0xa0] | g_windowVirtualKeyDown[0xa1];
	g_windowVirtualKeyDown[0x11] = g_windowVirtualKeyDown[0xa2] | g_windowVirtualKeyDown[0xa3];
	g_windowVirtualKeyDown[0x12] = g_windowVirtualKeyDown[0xa4] | g_windowVirtualKeyDown[0xa5];
	bool text_blocked = false;
	for (int key = 0; key < AERON_KEY_COUNT; ++key)
		text_blocked |= input->key_down[key] && XwInput_KeyBlocked(key);
	pending_mouse_presses &= ~observed_mouse_presses;
	observed_mouse_presses = 0;
	if (route == XW_KEYBOARD_BLOCKED) {
		g_dinputShiftDown = g_dinputCtrlDown = g_dinputAltDown = 0;
		memset(g_windowVirtualKeyDown, 0, sizeof g_windowVirtualKeyDown);
		read_index = write_index = 0;
		scan_pending = false;
		modifiers = 0;
		XwInput_ResetPointer();
		return;
	}
	modifiers = (input->key_down[AERON_KEY_RSHIFT] ? 1 : 0) | (input->key_down[AERON_KEY_LSHIFT] ? 2 : 0) |
				((input->key_down[AERON_KEY_LCTRL] || input->key_down[AERON_KEY_RCTRL]) ? 4 : 0) |
				((input->key_down[AERON_KEY_LALT] || input->key_down[AERON_KEY_RALT]) ? 8 : 0);
	if (route != XW_KEYBOARD_RAW) {
		XwInput_FlushRawKeyboard();
	} else if (input->key_events_overflow) {
		read_index = write_index = 0;
		scan_pending = false;
	} else
		for (unsigned i = 0; i < input->key_event_count; ++i) {
			if (input->key_events[i].down && !XwInput_KeyBlocked(input->key_events[i].chord.key)) {
				unsigned key = ControlKey(input->key_events[i].chord);
				if (key == LANDRU_JOYSTICK_CALIBRATE_KEY && XwFrontend_IsActive()) {
					XwPort_RequestSettingsPage(XW_SETTINGS_CONTROLLER);
					return;
				}
				XwInput_QueueFlightKey(FlightKey(input->key_events[i].chord));
				if (key)
					XwInput_Append(key);
			}
		}
	if (route == XW_KEYBOARD_RAW && !text_blocked && !(modifiers & 12))
		XwInput_Text(input);
	int x, y;
	bool inside = XwPresentation_MouseToScene(input, &x, &y) != 0;
	if (!inside)
		XwInput_BlockMouseButtons(input->mouse.buttons | input->mouse.pressed_buttons);
	delta_x += have_pointer ? x - mouse_x : 0;
	delta_y += have_pointer ? y - mouse_y : 0;
	mouse_x = x;
	mouse_y = y;
	if (!have_pointer)
		g_winMousePrevPos = g_winMousePos = g_winMouseCursorPos = (XwMousePosition) { x, y };
	have_pointer = true;
	mouse_buttons = 0;
	static const uint32_t buttons[3] = { AERON_MOUSE_BUTTON_LEFT, AERON_MOUSE_BUTTON_RIGHT,
										 AERON_MOUSE_BUTTON_MIDDLE };
	for (int i = 0; i < 3; ++i) {
		bool down = inside && XwInput_FilterMouseButtons(buttons[i]) && (input->mouse.buttons & buttons[i]);
		g_winMouseButtonDown[i] = down;
		if (inside && XwInput_FilterMouseButtons(buttons[i]) && (input->mouse.pressed_buttons & buttons[i])) {
			g_winMouseButtonPressed[i] = 1;
			pending_mouse_presses |= 1 << i;
		}
		if (XwInput_FilterMouseButtons(input->mouse.released_buttons) & buttons[i])
			g_winMouseButtonReleased[i] = 1;
		if (down)
			mouse_buttons |= 1 << i;
	}
}

int XwInput_KeyPending(void) {
	if (XwInput_ReconcileKeyboard() != XW_KEYBOARD_RAW)
		return 0;
	if (read_index == write_index)
		return 0;
	return scan_pending ? keys[read_index] >> 8 : keys[read_index];
}

int XwInput_ReadKey(void) {
	if (XwInput_ReconcileKeyboard() != XW_KEYBOARD_RAW)
		return -1;
	if (read_index == write_index)
		return -1;
	uint16_t key = keys[read_index];
	if (key > 255 && !scan_pending) {
		scan_pending = true;
		return 0;
	}
	read_index = (read_index + 1) % KEY_CAPACITY;
	int value = scan_pending ? key >> 8 : key;
	scan_pending = false;
	return value;
}

int XwInput_Modifiers(void) { return modifiers; }

void XwInput_MousePosition(int16_t* buttons, int16_t* x, int16_t* y) {
	if (buttons) {
		*buttons = XwInput_IsCaptured() ? 0 : (int16_t)(mouse_buttons | pending_mouse_presses);
		observed_mouse_presses |= pending_mouse_presses;
	}
	if (x)
		*x = (int16_t)mouse_x;
	if (y)
		*y = (int16_t)mouse_y;
}

void XwInput_MouseMovement(int16_t* x, int16_t* y) {
	*x = (int16_t)delta_x;
	*y = (int16_t)delta_y;
	delta_x = delta_y = 0;
}

void XwInput_QueueFlightKey(uint16_t key) {
	if (key == USER_KEY_ESCAPE && XwInput_GameplayActive()) {
		XwPort_RequestSettings();
		return;
	}
	if (!key || key == USER_KEY_DETAIL || key == USER_KEY_INTERLACE || key == USER_KEY_REBUILD_COCKPIT ||
		key == USER_KEY_FRAME_RATE)
		return;
	unsigned next = (flight_write + 1) % KEY_CAPACITY;
	if (next == flight_read) {
		Aeron_LogWarn("xw.input", "Flight command queue is full");
		return;
	}
	flight_keys[flight_write] = (uint8_t)key;
	flight_write = next;
}

void XwInput_FlushRawKeyboard(void) {
	if (g_dinputKeyboardDevice) {
		uint32_t count = UINT32_MAX;
		g_dinputKeyboardDevice->lpVtbl->GetDeviceData(g_dinputKeyboardDevice, sizeof(DIDEVICEOBJECTDATA),
													  NULL, &count, 0);
	}
	g_dinputShiftDown = g_dinputCtrlDown = g_dinputAltDown = 0;
	g_keyReady = g_lastKeyCode = 0;
}

void XwInput_ClearCommands(void) {
	XwInput_ResetPointer();
	read_index = write_index = flight_read = flight_write = 0;
	scan_pending = false;
	XwInput_FlushRawKeyboard();
	XwKeyboardMapping_Suspend();
	XwControllerMapping_ReleaseCommands();
	XwFlightControls_Reset();
	XwMouseFlight_Reset();
}

bool XwInput_GameplayActive(void) { return route == XW_KEYBOARD_GAMEPLAY; }

XwKeyboardRoute XwInput_ReconcileKeyboard(void) {
	const AeronInputSnapshot* input = Aeron_InputSnapshot();
	XwKeyboardRoute next = XW_KEYBOARD_RAW;
	if (suppressed_frame || !input || !input->has_focus || XwInput_IsCaptured() || Aeron_DebugUiVisible())
		next = XW_KEYBOARD_BLOCKED;
	else if (XwFlightSim_IsPlayerControl() && !XwFlightSim_IsPaused() &&
			 (!view_gbl || !xdialog_Is_Active_Dialog()))
		next = XW_KEYBOARD_GAMEPLAY;
	if (next != route) {
		XwInput_BlockHeldKeys();
		XwInput_ClearCommands();
		route = next;
	}
	XwKeyboardMapping_Enable(route == XW_KEYBOARD_GAMEPLAY, input);
	return route;
}

int XwInput_FlightKeyPending(void) {
	if (XwInput_ReconcileKeyboard() == XW_KEYBOARD_BLOCKED)
		return 0;
	XwFlightControls_CollectCommands();
	return flight_read != flight_write;
}

uint16_t XwInput_ReadFlightKey(void) {
	if (!XwInput_FlightKeyPending())
		return 0;
	uint16_t key = flight_keys[flight_read];
	flight_read = (flight_read + 1) % KEY_CAPACITY;
	return key;
}

int XwInput_CanReacquireKeyboard(void) {
	const AeronInputSnapshot* input = Aeron_InputSnapshot();
	if (!input || !input->has_focus || input->frame_id == reacquire_frame)
		return 0;
	reacquire_frame = input->frame_id;
	return 1;
}

static bool HasTextEntry(Input* list) {
	for (Input* i = list; i; i = i->next) {
		if ((i->flags & (INPUT_VISIBLE | INPUT_ACTIVE)) != (INPUT_VISIBLE | INPUT_ACTIVE))
			continue;
		if (i->draw == register_idraw_Reg_String_Button)
			return true;
		if (HasTextEntry(i->child))
			return true;
	}
	return false;
}

bool XwInput_SettingsShortcutAllowed(void) {
	if (view_gbl && (xdialog_Is_Active_Dialog() || HasTextEntry(xinput_Get_Active_Input_List())))
		return false;
	return XwFrontend_IsActive() || XwFlightSim_IsPlayerControl();
}

bool XwInput_RendererShortcutAllowed(void) {
	return XwPresentation_BaseSource() != XW_PRESENT_FRONTEND && !XwPort_SettingsOpen() &&
		   !Aeron_DebugUiVisible() && !XwInput_IsCaptured() && !XwReplayInput_IsSavePending() &&
		   (!view_gbl || (!xdialog_Is_Active_Dialog() && !HasTextEntry(xinput_Get_Active_Input_List())));
}

static int JoystickSource(AeronWinmmJoystickState* out, void* user) {
	(void)user;
	*out = joystick;
	return connected;
}

static void UpdateJoystick(void) {
	int present = XwControllerMapping_Present();
	if (present != connected) {
		connected = present;
		memset(g_joystickCalibrationInitialized, 0, sizeof g_joystickCalibrationInitialized);
		g_joystickCachedButtons = 0;
		memset(cached_axes, 0, sizeof cached_axes);
		XwFlightControls_Reset();
	}
	joystick = (AeronWinmmJoystickState) {
		.name = "OpenXW Controllers", .button_count = 2, .has_pov = 1, .pov_direction = -1
	};
	for (int i = 0; i < 4; ++i)
		joystick.axes[i] = 32768;
	if (route != XW_KEYBOARD_BLOCKED && present) {
		joystick.axes[0] = 32768 + 256 * XwControllerMapping_MenuAxis(XW_INPUT_AXIS_YAW);
		joystick.axes[1] = 32768 + 256 * XwControllerMapping_MenuAxis(XW_INPUT_AXIS_PITCH);
		joystick.buttons = XwControllerMapping_MenuButtons();
		unsigned hat = XwControllerMapping_MenuHat();
		if (joystick.axes[0] == 32768)
			joystick.axes[0] = hat & 2 ? 65535 : hat & 8 ? 0 : 32768;
		if (joystick.axes[1] == 32768)
			joystick.axes[1] = hat & 4 ? 65535 : hat & 1 ? 0 : 32768;
		uint16_t throttle;
		uint32_t generation;
		if (XwControllerMapping_ThrottleSample(&throttle, &generation))
			joystick.axes[2] = throttle;
		joystick.axes[3] = 32768 + 256 * XwControllerMapping_MenuAxis(XW_INPUT_AXIS_ROLL);
		joystick.pov_direction = hat & 1 ? 0 : hat & 2 ? 1 : hat & 4 ? 2 : hat & 8 ? 3 : -1;
	}
}

uint32_t XwInput_JoystickRead(int32_t* x, int32_t* y, int32_t* z, int16_t device) {
	if (XwInput_IsCaptured()) {
		*x = *y = *z = 0;
		return 0;
	}
	if (!g_joystickPollingSuppressed) {
		if (!connected || device != 0) {
			cached_axes[0] = cached_axes[1] = cached_axes[2] = JOYSTICK_AXIS_UNAVAILABLE;
			g_joystickCachedButtons = 0;
		} else {
			cached_axes[0] = ((int)joystick.axes[0] - 32768) / 256;
			cached_axes[1] = ((int)joystick.axes[1] - 32768) / 256;
			cached_axes[2] = ((int)joystick.axes[2] - 32768) / 256;
			g_joystickCachedButtons = joystick.buttons;
			if (joystick.pov_direction >= 0)
				g_joystickCachedButtons |= JOYSTICK_POV_FIRST_BUTTON << joystick.pov_direction;
		}
	}
	*x = cached_axes[0];
	*y = cached_axes[1];
	*z = cached_axes[2];
	return g_joystickCachedButtons;
}

void XwInput_Init(void) {
	XwInput_Reset();
	suppressed_frame = true;
	route = XW_KEYBOARD_BLOCKED;
	connected = 0;
	config_generation = 0;
	memset(devices, 0, sizeof devices);
	memset(&installed_keyboard, 0, sizeof installed_keyboard);
	XwControllerMapping_Init(&XwConfig_Settings()->controller);
	AeronCompat_SetJoystickSource(JoystickSource, NULL);
	XwInput_ApplySettings(Aeron_InputSnapshot());
}

void XwInput_ApplySettings(const AeronInputSnapshot* input) {
	const XwSettings* settings = XwConfig_Settings();
	if (!settings)
		return;
	bool changed = false;
	if (input)
		for (int i = 0; i < AERON_CONTROLLER_MAX; ++i) {
			uint32_t id = input->controllers[i].connected ? input->controllers[i].instance_id : 0;
			changed |= devices[i] != id;
			devices[i] = id;
		}
	if (changed && !XwPort_SettingsOpen()) {
		XwControllerOptions options = settings->controller;
		char error[1024];
		if (!XwControllerOptions_InitializeGamepads(&options, &settings->gamepad_defaults, input, error,
													sizeof error))
			Aeron_LogWarn("xw.input", "%s", error);
		else if (!XwControllerOptions_Equals(&options, &settings->controller)) {
			AeronConfigFile* candidate = NULL;
			AeronConfigError detail = { 0 };
			if (!AeronConfigFile_Clone(XwConfig_UserDocument(), &candidate, &detail) ||
				!XwControllerConfig_Write(candidate, &options, &detail))
				Aeron_LogError("xw.input", "%s", detail.message);
			else if (!XwConfig_UpdateUser(candidate, 1, error, sizeof error))
				Aeron_LogError("xw.input", "%s", error);
			AeronConfigFile_Destroy(candidate);
		}
		XwInput_BlockHeldKeys();
		XwInput_ClearCommands();
		XwControllerMapping_Suspend();
	}
	settings = XwConfig_Settings();
	if (config_generation != XwConfig_Generation()) {
		config_generation = XwConfig_Generation();
		if (!XwKeyboardMapping_Equal(&installed_keyboard, &settings->keyboard)) {
			installed_keyboard = settings->keyboard;
			XwInput_BlockHeldKeys();
			XwInput_ClearCommands();
			XwKeyboardMapping_Install(&installed_keyboard);
		}
		XwControllerMapping_SetOptions(&settings->controller);
	}
	XwControllerMapping_ApplyPending();
}

void XwInput_Shutdown(void) {
	XwInput_Reset();
	XwControllerMapping_Shutdown();
	AeronCompat_SetJoystickSource(NULL, NULL);
	connected = 0;
}

static unsigned int KeyToDik(int key) {
	static const unsigned char letters[26] = {
		0x1e, 0x30, 0x2e, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25, 0x26, 0x32,
		0x31, 0x18, 0x19, 0x10, 0x13, 0x1f, 0x14, 0x16, 0x2f, 0x11, 0x2d, 0x15, 0x2c,
	};
	static const unsigned char keypadDigits[10] = {
		0x52, 0x4f, 0x50, 0x51, 0x4b, 0x4c, 0x4d, 0x47, 0x48, 0x49
	};

	if (key >= AERON_KEY_A && key < AERON_KEY_A + 26) {
		return letters[key - AERON_KEY_A];
	}
	if (key >= AERON_KEY_1 && key < AERON_KEY_1 + 9) {
		return 0x02u + (unsigned int)(key - AERON_KEY_1);
	}
	if (key == AERON_KEY_1 + 9) {
		return 0x0bu;
	}
	if (key >= AERON_KEY_F1 && key < AERON_KEY_F1 + 10) {
		return 0x3bu + (unsigned int)(key - AERON_KEY_F1);
	}
	if (key == AERON_KEY_F1 + 10) {
		return 0x57u;
	}
	if (key == AERON_KEY_F1 + 11) {
		return 0x58u;
	}
	if (key >= AERON_KEY_KP_1 && key <= AERON_KEY_KP_9) {
		return keypadDigits[key - AERON_KEY_KP_1 + 1];
	}

	switch (key) {
		case AERON_KEY_ESCAPE:
			return 0x01u;
		case AERON_KEY_MINUS:
			return 0x0cu;
		case AERON_KEY_EQUALS:
			return 0x0du;
		case AERON_KEY_BACKSPACE:
			return 0x0eu;
		case AERON_KEY_TAB:
			return 0x0fu;
		case AERON_KEY_LEFTBRACKET:
			return 0x1au;
		case AERON_KEY_RIGHTBRACKET:
			return 0x1bu;
		case AERON_KEY_RETURN:
			return 0x1cu;
		case AERON_KEY_LCTRL:
			return 0x1du;
		case AERON_KEY_SEMICOLON:
			return 0x27u;
		case AERON_KEY_APOSTROPHE:
			return 0x28u;
		case AERON_KEY_GRAVE:
			return 0x29u;
		case AERON_KEY_LSHIFT:
			return 0x2au;
		case AERON_KEY_BACKSLASH:
			return 0x2bu;
		case AERON_KEY_COMMA:
			return 0x33u;
		case AERON_KEY_PERIOD:
			return 0x34u;
		case AERON_KEY_SLASH:
			return 0x35u;
		case AERON_KEY_RSHIFT:
			return 0x36u;
		case AERON_KEY_LALT:
			return 0x38u;
		case AERON_KEY_SPACE:
			return 0x39u;
		case AERON_KEY_CAPSLOCK:
			return 0x3au;
		case AERON_KEY_PRINTSCREEN:
			return 0xb7u;
		case AERON_KEY_SCROLLLOCK:
			return 0x46u;
		case AERON_KEY_PAUSE:
			return 0xc5u;
		case AERON_KEY_INSERT:
			return 0xd2u;
		case AERON_KEY_HOME:
			return 0xc7u;
		case AERON_KEY_PAGEUP:
			return 0xc9u;
		case AERON_KEY_DELETE:
			return 0xd3u;
		case AERON_KEY_END:
			return 0xcfu;
		case AERON_KEY_PAGEDOWN:
			return 0xd1u;
		case AERON_KEY_RIGHT:
			return 0xcdu;
		case AERON_KEY_LEFT:
			return 0xcbu;
		case AERON_KEY_DOWN:
			return 0xd0u;
		case AERON_KEY_UP:
			return 0xc8u;
		case AERON_KEY_KP_DIVIDE:
			return 0xb5u;
		case AERON_KEY_KP_MULTIPLY:
			return 0x37u;
		case AERON_KEY_KP_MINUS:
			return 0x4au;
		case AERON_KEY_KP_PLUS:
			return 0x4eu;
		case AERON_KEY_KP_ENTER:
			return 0x9cu;
		case AERON_KEY_KP_0:
			return keypadDigits[0];
		case AERON_KEY_KP_PERIOD:
			return 0x53u;
		case AERON_KEY_RCTRL:
			return 0x9du;
		case AERON_KEY_RALT:
			return 0xb8u;
		default:
			return 0;
	}
}

static unsigned FlightKey(AeronKeyChord chord) {
	unsigned scan = KeyToDik(chord.key);
	if (!scan)
		return 0;
	const uint8_t* table = chord.modifiers & AERON_KEY_MOD_SHIFT  ? g_dinputShiftKeyCodeTable
						   : chord.modifiers & AERON_KEY_MOD_CTRL ? g_dinputCtrlKeyCodeTable
						   : chord.modifiers & AERON_KEY_MOD_ALT  ? g_dinputAltKeyCodeTable
																  : g_dinputKeyCodeTable;
	return table[scan] < DINPUT_SHIFT_MARKER ? table[scan] : 0;
}

static unsigned int VirtualKey(int key) {
	static const unsigned char controls[] = { 13,   27, 8,    9,    32,   0xbd, 0xbb, 0xdb, 0xdd,
											  0xdc, 0,  0xba, 0xde, 0xc0, 0xbc, 0xbe, 0xbf, 0x14 };
	static const unsigned char navigation[] = { 0x2c, 0x91, 0x13, 0x2d, 0x24, 0x21, 0x2e,
												0x23, 0x22, 0x27, 0x25, 0x28, 0x26 };
	if (key >= AERON_KEY_A && key < AERON_KEY_A + 26)
		return 'A' + key - AERON_KEY_A;
	if (key >= AERON_KEY_1 && key < AERON_KEY_1 + 10)
		return key == AERON_KEY_1 + 9 ? '0' : '1' + key - AERON_KEY_1;
	if (key >= AERON_KEY_RETURN && key <= AERON_KEY_CAPSLOCK)
		return controls[key - AERON_KEY_RETURN];
	if (key >= AERON_KEY_F1 && key < AERON_KEY_F1 + 12)
		return 0x70 + key - AERON_KEY_F1;
	if (key >= AERON_KEY_PRINTSCREEN && key <= AERON_KEY_UP)
		return navigation[key - AERON_KEY_PRINTSCREEN];
	if (key >= AERON_KEY_LCTRL && key <= AERON_KEY_RGUI) {
		static const unsigned char modifiers[] = { 0xa2, 0xa0, 0xa4, 0x5b, 0xa3, 0xa1, 0xa5, 0x5c };
		return modifiers[key - AERON_KEY_LCTRL];
	}
	if (key >= AERON_KEY_KP_1 && key <= AERON_KEY_KP_9)
		return 0x61 + key - AERON_KEY_KP_1;
	if (key == AERON_KEY_KP_0)
		return 0x60;
	if (key == AERON_KEY_KP_ENTER)
		return 13;
	return 0;
}
