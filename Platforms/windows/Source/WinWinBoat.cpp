#include "WinWinBoat.h"
#include "TugBoat/Gfx/RHI.h"
#include <windows.h>
#include <TugBoat/Core/RemoveShortTypes.h>
#include <vulkan/vulkan.hpp>
#include <TugBoat/Core/AddShortTypes.h>

using namespace TugBoat;
const LPCSTR CLASS_NAME = "TugBoatWindowClass";

std::vector<std::string> WinWinBoat::GetInstanceExtensions() const
{
	return {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	};
}

BID WinWinBoat::CreateWindow(const std::string& title, UIVec size, WindowFlags flags){
	HWND hwnd = CreateWindowEx(
		0,                              // Optional window styles.
		CLASS_NAME,                     // Window class
		title.data(),    // Window text
		WS_OVERLAPPEDWINDOW,            // Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, size.x, size.y,

		nullptr,       // Parent window
		nullptr,       // Menu
		nullptr,  // Instance handle
		nullptr        // Additional application data
	);

	if(hwnd == nullptr){
		return INVALID_BID;
	}
	BID id = CreateID();
	//attach the id to the window
	SetWindowLongPtr(hwnd, GWLP_USERDATA, id);

	ShowWindow(hwnd, SW_SHOWDEFAULT);
	UpdateWindow(hwnd);

	Ref<WindowData> data = CreateRef<WindowData>();
	data->window = hwnd;

	m_Windows[id] = data;

	return id;

}

void WinWinBoat::DestroyWindow(BID window){
	auto it = m_Windows.find(window);
	if(it == m_Windows.end()){
		return;
	}

	::DestroyWindow(it->second->window);
	m_Windows.erase(it);
}

void WinWinBoat::ProcessEvents(){
	MSG msg = { };
	while(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

//make winproc
LRESULT CALLBACK WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	switch(uMsg){
	case WM_CLOSE:
		//show a message box with an ok button just saying that its un-handled
		MessageBox(hwnd, "I can't send this event to the engine\nso your gonna need to do something else to close it.\n:3", "TugBoat", MB_OK);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

WinWinBoat::WinWinBoat(){
	WNDCLASS wc = { };
	wc.lpfnWndProc = WinProc;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);
}

WinWinBoat::~WinWinBoat(){
	UnregisterClass(CLASS_NAME, GetModuleHandle(nullptr));
}

NativeWindowHandle WinWinBoat::GetNativeWindowHandle(BID window)
{
	auto it = m_Windows.find(window);
	if(it == m_Windows.end()){
		return {};
	}

	NativeWindowHandle handle {NativeWinDataType_Win32};
	handle.win32.hWnd = it->second->window;
	handle.win32.hInstance = GetModuleHandle(nullptr);
	return handle;
}

VkSurfaceKHR WinWinBoat::GetSurface(BID window)
{
	auto it = m_Windows.find(window);
	if(it == m_Windows.end()){
		return {};
	}

	VkWin32SurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd = it->second->window;
	createInfo.hinstance = GetModuleHandle(nullptr);

	VkSurfaceKHR surface;
	vkCreateWin32SurfaceKHR(RHI::GetInstance()->GetVkInstance(), &createInfo, nullptr, &surface);
	return surface;
}

void WinWinBoat::SetWindowTitle(BID window, const std::string &title)
{
	auto it = m_Windows.find(window);
	if(it == m_Windows.end()){
		return;
	}
	SetWindowText(it->second->window, title.data());
}

