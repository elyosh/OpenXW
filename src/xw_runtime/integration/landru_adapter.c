/* Host callback shape follows OpenTIE; X-Wing retains its own game interfaces. */
#include "xw_runtime/integration/landru_adapter.h"
#include "xw/flight/feinput.h"
#include "xw/input/joystick.h"
#include "xw/util/landru_display.h"
#include "xw_runtime/input/controller_mapping.h"
#include "xw_runtime/input/input_bridge.h"
#include "xw_runtime/input/system_cursor.h"
#include "xw_runtime/integration/landru_sound.h"
#include "xw_runtime/platform/classic_surfaces.h"
#include "xw_runtime/runtime/frontend_task.h"
#include "xw_runtime/runtime/presentation.h"
#include "xw_runtime/runtime/profile.h"
#include "xw_runtime/storage/directory.h"
#include "xw_runtime/storage/file_io.h"
#include "xw_runtime/storage/storage.h"
#include "xw_runtime/timing/host_clock.h"
#include <aeron/compat/mmsystem.h>
#include <aeron/log.h>
#include <ctype.h>
#include <landru/cursor.h>
#include <landru/host.h>
#include <landru/io.h>
#include <landru/surface.h>
#include <landru/vesa.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

static bool initialized, video_open;

static AeronVfsRoot Root(LandruFileRoot root) {
	switch (root) {
		case LANDRU_FILE_ROOT_ASSET:
		case LANDRU_FILE_ROOT_AUXILIARY_ASSET:
			return AERON_VFS_ROOT_ASSET;
		case LANDRU_FILE_ROOT_USER:
			return AERON_VFS_ROOT_USER;
		case LANDRU_FILE_ROOT_TEMP:
			return AERON_VFS_ROOT_TEMP;
		default:
			return AERON_VFS_ROOT_COUNT;
	}
}

static void Log(void* user, LandruLogLevel level, const char* format, va_list args) {
	(void)user;
	static const AeronLogLevel levels[] = { AERON_LOG_TRACE, AERON_LOG_INFO, AERON_LOG_WARN,
											AERON_LOG_ERROR };
	Aeron_LogMessageV((unsigned)level < 4 ? levels[level] : AERON_LOG_ERROR, "xw.landru", format, args);
}

static LandruFile* Open(void* user, LandruFileRoot root, const char* path, const char* mode) {
	(void)user;
	AeronVfsRoot target = Root(root);
	if (target == AERON_VFS_ROOT_ASSET) {
		if (!mode || (strcmp(mode, "r") && strcmp(mode, "rb")))
			return NULL;
		if (!strncmp(path, "@mission/", 9))
			return XwStorage_OpenMission(path + 9);
		XwGameVersion version = XwProfile_ActiveFrontend()->version;
		const char* extension = strrchr(path, '.');
		if (extension && strlen(extension) == 4) {
			int first = tolower((unsigned char)extension[1]);
			int second = tolower((unsigned char)extension[2]);
			int third = tolower((unsigned char)extension[3]);
			/* In-flight records and briefing text belong to the mission, not the menu artwork. */
			if ((first == 'x' && second == 'w' && third == 'i') ||
				(first == 'b' && second == 'r' && third == 'f'))
				return XwStorage_OpenMission(path);
			/* Recorded soundtrack and ambience remain in the Windows media installation. */
			if (first == 'w' && second == 'a' && third == 'v')
				version = XW_GAME_VERSION_98;
		}
		return XwStorage_OpenInstallation(version, path);
	}
	return XwStorage_OpenRoot(target, path, mode);
}

ResFile* XwLandru_OpenMissionResource(const char* path) {
	char tagged[RESOURCE_FILENAME_CAPACITY];
	const char* name = strrchr(path, '\\');
	if (!name)
		name = strrchr(path, '/');
	name = name ? name + 1 : path;
	/* The supported DOS93 B-Wing installation keeps its expanded catalogue separately. */
	if (XwProfile_MissionFlight()->mission_version == XW_GAME_VERSION_93 && !strcmp(name, "missions.lfd"))
		name = "BMISSION.LFD";
	int length = snprintf(tagged, sizeof tagged, ":@mission/RESOURCE/%s", name);
	return length > 0 && (size_t)length < sizeof tagged ? xres_Open_Resource(tagged) : NULL;
}

static size_t Read(void* user, void* buffer, size_t size, size_t count, LandruFile* file) {
	(void)user;
	return XwFile_Read(buffer, size, count, file);
}

static size_t Write(void* user, const void* buffer, size_t size, size_t count, LandruFile* file) {
	(void)user;
	return XwFile_Write(buffer, size, count, file);
}

static int Seek(void* user, LandruFile* file, long offset, int origin) {
	(void)user;
	return offset < INT32_MIN || offset > INT32_MAX ? -1 : XwFile_Seek(file, (int32_t)offset, origin);
}

static long Tell(void* user, LandruFile* file) {
	(void)user;
	return XwFile_Tell(file);
}

static int Close(void* user, LandruFile* file) {
	(void)user;
	return XwFile_Close(file);
}

static LandruDir* OpenDirectory(void* user, LandruFileRoot root, const char* path) {
	(void)user;
	return XwStorage_OpenDirectory(Root(root), path);
}

static int NextEntry(void* user, LandruDir* directory, LandruDirEntry* entry) {
	(void)user;
	XwDirectoryEntry value;
	if (!entry || !XwStorage_NextEntry(directory, &value))
		return 0;
	memcpy(entry->name, value.name, sizeof entry->name);
	entry->size = value.size;
	entry->is_dir = value.is_directory;
	return 1;
}

static void CloseDirectory(void* user, LandruDir* directory) {
	(void)user;
	XwStorage_CloseDirectory(directory);
}

static int IsDirectory(void* user, LandruFileRoot root, const char* path) {
	(void)user;
	return XwStorage_IsDirectory(Root(root), path);
}

static int KeyPending(void* user) {
	(void)user;
	return XwInput_KeyPending();
}

static int ReadKey(void* user) {
	(void)user;
	return XwInput_ReadKey();
}

static int Modifiers(void* user) {
	(void)user;
	return XwInput_Modifiers();
}

static void MousePosition(void* user, int16_t* buttons, int16_t* x, int16_t* y) {
	(void)user;
	XwInput_MousePosition(buttons, x, y);
}

static void MouseMovement(void* user, int16_t* x, int16_t* y) {
	(void)user;
	XwInput_MouseMovement(x, y);
}

static void MouseSetPosition(void* user, int16_t x, int16_t y) {
	(void)user;
	XwPresentation_WarpScene(x, y);
}

static void MouseShow(void* user, bool show) {
	(void)user;
	XwPort_SetSystemCursorDisplayCount(show ? 0 : -1);
}

static int JoystickCount(void* user) {
	(void)user;
	return XwControllerMapping_Present();
}

static uint32_t JoystickReadXw(void* user, int32_t* x, int32_t* y, int32_t* z, int16_t device) {
	(void)user;
	return XwInput_JoystickRead(x, y, z, device);
}

static void JoystickRead(void* user, int port, int16_t* axes, int count, uint16_t* buttons) {
	int32_t values[3] = { 0 };
	uint32_t mask = JoystickReadXw(user, &values[0], &values[1], &values[2], (int16_t)port);
	for (int i = 0; i < count; ++i)
		axes[i] = i < 3 ? (int16_t)values[i] : 0;
	*buttons = (uint16_t)mask;
}

static void SetPalette(void* user, const uint8_t* rgb, int start, int count) {
	(void)user;
	(void)rgb;
	if (count <= 0)
		return;
	if (start < 0) {
		if (start <= -count)
			return;
		count += start;
		start = 0;
	}
	if (count > 0)
		FlightDisplay_SetPaletteEntries(xsurface_Get_Effective_Palette(), start, (unsigned)count);
}

static void SetMode(void* user, uint16_t mode) {
	(void)user;
	/* A shell visit may coexist with retained flight resources. */
	XwPresentation_SelectBaseSource(XW_PRESENT_FRONTEND);
	FlightDisplay_RebuildForMode(mode);
	FlightDisplay_LockSurface();
	XwDisplay_BindLandruVideo();
	FlightDisplay_UnlockSurface();
}

static void Lock(void* user) {
	(void)user;
	FlightDisplay_LockSurface();
}

static void Unlock(void* user) {
	(void)user;
	FlightDisplay_UnlockSurface();
}

static void Copy(void* user) {
	(void)user;
	if (!xsurface_Uses_Native_Vga_Presentation())
		LandruDisplay_CopyCanvasToBackBuffer();
}

static void Present(void* user) {
	(void)user;
	if (!xsurface_Uses_Native_Vga_Presentation())
		FlightDisplay_Flip();
}

static void LogicalSize(void* user, int width, int height) {
	(void)user;
	XwPresentation_SetSceneExtent(width, height);
}

static uint64_t Now(void* user) {
	(void)user;
	return XwTime_GetElapsedUs();
}

bool XwLandru_Init(void) {
	if (initialized)
		return true;
	if (!XwStorage_Vfs())
		return false;
	LandruHost host = { .log = Log,
						.file_open = Open,
						.file_read = Read,
						.file_write = Write,
						.file_seek = Seek,
						.file_tell = Tell,
						.file_close = Close,
						.dir_open = OpenDirectory,
						.dir_next = NextEntry,
						.dir_close = CloseDirectory,
						.path_is_dir = IsDirectory,
						.key_pending = KeyPending,
						.key_read = ReadKey,
						.modifier_keys = Modifiers,
						.mouse_position = MousePosition,
						.mouse_movement = MouseMovement,
						.mouse_set_position = MouseSetPosition,
						.mouse_show = MouseShow,
						.joystick_count = JoystickCount,
						.joystick_read = JoystickRead,
						.joystick_read_xw = JoystickReadXw,
						.palette_set = SetPalette,
						.now_us = Now,
						.video = { .set_mode = SetMode,
								   .lock = Lock,
								   .unlock = Unlock,
								   .copy_to_present_surface = Copy,
								   .present = Present,
								   .logical_size_changed = LogicalSize } };
	XwLandru_ConfigureSoundHost(&host);
	if (!landru_set_host(&host))
		return false;
	if (!landru_port_Set_Native_Vga_Presentation(true) ||
		!landru_port_Set_Initial_Video_Backend(LANDRU_PORT_VIDEO_PLATFORM)) {
		landru_clear_host();
		landru_port_Set_Native_Vga_Presentation(false);
		landru_port_Set_Initial_Video_Backend(LANDRU_PORT_VIDEO_SOFTWARE);
		return false;
	}
	xcursor_port_Set_External_Presentation(true);
	XwInput_Reset();
	initialized = true;
	return true;
}

bool XwLandru_OpenVideo(void) {
	if (!initialized)
		return false;
	if (video_open)
		return true;
	xvesa_Create_Vesa_Module(g_landruDoublePixelsEnabled ? LANDRU_VESA_VGA : LANDRU_VESA_SVGA);
	if (!xsurface_Create_Surface_Module(false)) {
		xvesa_Destroy_VESA_Module();
		return false;
	}
	video_open = true;
	XwInput_ResetPointer();
	XwPresentation_Invalidate();
	return true;
}

void XwLandru_CloseVideo(void) {
	if (!video_open)
		return;
	xsurface_Destroy_Surface_Module();
	xvesa_Destroy_VESA_Module();
	XwPresentation_Invalidate();
	video_open = false;
}

void XwLandru_SetLogicalViewport(int low, int width, int height) {
	if (XwProfile_DosFrontend() && XwFrontend_IsActive()) {
		low = 1;
		width = 320;
		height = 200;
	}
	LandruSurfaceSet set = low ? LANDRU_SURFACE_VGA : LANDRU_SURFACE_SVGA;
	if (xsurface_Has_Surface_Set(set))
		xsurface_Select_Surface_Set(set);
	else
		xbm_Set_VGA_Compatibility_Mode(low, width, height);
	XwPresentation_SetSceneExtent(width, height);
	Rect bounds;
	xsurface_Get_Logical_Bounds(&bounds);
	xio_Set_Mouse_Limits(&bounds);
}

void XwLandru_Shutdown(void) {
	if (!initialized)
		return;
	XwLandru_CloseVideo();
	xcursor_port_Set_External_Presentation(false);
	landru_clear_host();
	landru_port_Set_Native_Vga_Presentation(false);
	XwInput_Reset();
	initialized = false;
}
