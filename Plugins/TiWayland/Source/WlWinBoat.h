#pragma once
#include <TugBoat/Boats/WindowBoat.h>
#include <TugBoat/Core/Log.h>
#include <unordered_map>

using namespace TugBoat;

struct WindowData{
    //This is just so doing stuff in wayland liseners isnt annoying
    BID id;
    class WlWinBoat* boat;
    
    
	struct wl_surface* surface;
	struct xdg_surface* xdgSurface;
	struct xdg_toplevel* xdgToplevel;
	struct zxdg_toplevel_decoration_v1* decor;
	UIVec size;
};

class WlWinBoat : public WindowBoat
{
public:
	BID CreateID(){
		//find an unused id
		BID id = 0;
		while(m_Windows.find(id) != m_Windows.end()){
			id++;
		}

		return id;
	}
	std::vector<std::string> GetInstanceExtensions() const override;

	BID CreateWindow(const std::string& title, UIVec size, WindowFlags flags) override;
	UIVec GetWindowSize(BID window) override;
	void SetWindowTitle(BID window, const std::string& title) override;

	void DestroyWindow(BID window) override;

	void ProcessEvents() override;

	VkSurfaceKHR GetSurface(BID window) override;
	NativeWindowHandle GetNativeWindowHandle(BID window) override;

	WlWinBoat();

	~WlWinBoat();
private:
	int m_fd = -1;
	std::unordered_map<BID, Ref<WindowData>> m_Windows;

	LOG("WlWinBoat");
};
