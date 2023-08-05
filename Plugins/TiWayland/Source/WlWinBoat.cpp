#include <wayland-client.h>
#include "WlWinBoat.h"
#include "TiWayland.h"
#include "TugBoat/Gfx/RHI.h"

#include <TugBoat/Core/RemoveShortTypes.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_wayland.h>
#include <TugBoat/Core/AddShortTypes.h>

#include <sys/poll.h>

WlWinBoat::WlWinBoat()
{
	m_fd = wl_display_get_fd(TiWayland::GetDisplay());
}

WlWinBoat::~WlWinBoat()
{
	m_Log << Info << "Destroying WlWinBoat";
}

BID WlWinBoat::CreateWindow(const std::string& title, UIVec size, WindowFlags flags)
{
	auto surface = wl_compositor_create_surface(TiWayland::GetCompositor());
	if(surface == nullptr){
		m_Log << Error << "Failed to create surface";
		return INVALID_BID;
	}
	auto xdgSurface = xdg_wm_base_get_xdg_surface(TiWayland::GetXdgWmBase(), surface);
	if(xdgSurface == nullptr){
		m_Log << Error << "Failed to create xdg surface";
		wl_surface_destroy(surface);
		return INVALID_BID;
	}
	auto xdgTopLvl = xdg_surface_get_toplevel(xdgSurface);
	if(xdgTopLvl == nullptr){
		m_Log << Error << "Failed to create xdg top level surface";
		xdg_surface_destroy(xdgSurface);
		wl_surface_destroy(surface);
		return INVALID_BID;
	}

	auto decor = zxdg_decoration_manager_v1_get_toplevel_decoration(TiWayland::GetDecorManager(), xdgTopLvl);
	if(decor == nullptr){
		m_Log << Error << "Failed to create surface decor";
		xdg_toplevel_destroy(xdgTopLvl);
		xdg_surface_destroy(xdgSurface);
		wl_surface_destroy(surface);
		return INVALID_BID;
	}

	xdg_surface_set_window_geometry(xdgSurface, 0, 0, (int32_t)size.x, (int32_t)size.y);
	xdg_toplevel_set_title(xdgTopLvl, title.c_str());

	if(flags & WindowFlags_Resizable){
		xdg_toplevel_set_min_size(xdgTopLvl, 20, 20);
		xdg_toplevel_set_max_size(xdgTopLvl, INT32_MAX, INT32_MAX);
	}

	wl_surface_commit(surface);

	auto data = CreateRef<WindowData>();
	data->surface = surface;
	data->xdgSurface = xdgSurface;
	data->xdgToplevel = xdgTopLvl;
	data->decor = decor;
	data->size = size;

	auto id = CreateID();
	m_Windows[id] = data;

	return id;
}

void WlWinBoat::DestroyWindow(BID window)
{
	if(m_Windows.find(window) == m_Windows.end()){
		m_Log << Error << "Failed to destroy window: " << window << " does not exist";
		return;
	}

	auto data = m_Windows[window];
	zxdg_toplevel_decoration_v1_destroy(data->decor);
	xdg_toplevel_destroy(data->xdgToplevel);
	xdg_surface_destroy(data->xdgSurface);
	wl_surface_destroy(data->surface);


	m_Windows.erase(window);
}

void WlWinBoat::ProcessEvents(){
	//check if there are event using POLLIN
	struct pollfd pfd = {m_fd, POLLIN};
	int ret = poll(&pfd, 1, 0);
	//if we have events we will do a wayland dispatch
	//if not continue
	if(ret < 0){
		m_Log << Error << "Failed to poll wayland events";
		return;
	}
	if(ret == 0){
		return;
	}

	wl_display_dispatch(TiWayland::GetDisplay());
}
const std::vector<std::string> _instanceExt = {
	VK_KHR_SURFACE_EXTENSION_NAME,
	VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME
};

std::vector<std::string> WlWinBoat::GetInstanceExtensions() const
{
	return _instanceExt;
}
NativeWindowHandle WlWinBoat::GetNativeWindowHandle(BID window)
{
	if(m_Windows.find(window) == m_Windows.end()){
		m_Log << Error << "Failed to get native window handle: " << window << " does not exist";
		return {};
	}

	auto data = m_Windows.at(window);
	auto handle = NativeWindowHandle{NativeWinDataType_Wayland};
	handle.wayland.display = TiWayland::GetDisplay();
	handle.wayland.surface = data->surface;
	return handle;
}

UIVec WlWinBoat::GetWindowSize(BID window)
{
	if(m_Windows.find(window) == m_Windows.end()){
		m_Log << Error << "Failed to get window size: " << window << " does not exist";
		return {};
	}

	auto data = m_Windows.at(window);
	return data->size;
}
VkSurfaceKHR WlWinBoat::GetSurface(BID window)
{
	if(m_Windows.find(window) == m_Windows.end()){
		m_Log << Error << "Failed to get surface: " << window << " does not exist";
		return {};
	}

	auto data = m_Windows.at(window);
	VkSurfaceKHR surface;
	VkWaylandSurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
	createInfo.display = TiWayland::GetDisplay();
	createInfo.surface = data->surface;

	if(vkCreateWaylandSurfaceKHR(RHI::GetInstance()->GetVkInstance(), &createInfo, nullptr, &surface) != VK_SUCCESS){
		m_Log << Error << "Failed to create surface";
		return {};
	}

	return surface;
}
