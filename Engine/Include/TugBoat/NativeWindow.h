#pragma once
#include <TugBoat/Core.h>

#ifdef TB_WAYLAND
#include <wayland-client.h>
#endif
#ifdef TB_X11
#include <X11/Xlib.h>
#endif
#ifdef TB_PLATFORM_WINDOWS
#include <windows.h>
#endif

namespace TugBoat{
enum NativeWinDataType{
	NativeWinDataType_Unknown,
	NativeWinDataType_KMSDRM,
	NativeWinDataType_Wayland,
	NativeWinDataType_X11,
	NativeWinDataType_Cocoa,
	NativeWinDataType_UIKit,
	NativeWinDataType_Win32,
	NativeWinDataType_Android,
};

struct NativeWindowHandle{
	NativeWinDataType type = NativeWinDataType_Unknown;
	union{
#ifdef TB_WAYLAND
		struct{
			struct wl_display* display;
			struct wl_surface* surface;
		} wayland;
#endif
#ifdef TB_X11
		struct{
			Display* display;
			Window window;
			int screen;
		} x11;
#endif
#ifdef TB_PLATFORM_WINDOWS
		struct{
			HINSTANCE hInstance;
			HWND hWnd;
		} win32;
#endif
	};
};

}