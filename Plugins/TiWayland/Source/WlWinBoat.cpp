#include <wayland-client.h>
#include "WlWinBoat.h"
#include "TiWayland.h"
#include "TugBoat/Gfx/RHI.h"

#include <TugBoat/Core/RemoveShortTypes.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_wayland.h>
#include <TugBoat/Core/AddShortTypes.h>

#include "xdg-shell-client-protocol.h"

#include <sys/poll.h>

static void xdg_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial){
	xdg_wm_base_pong(xdg_wm_base, serial);
	Log("Wayland") << Info << "PONG!";
}

const struct xdg_wm_base_listener boat_xdg_wm_listener = {
	.ping = xdg_ping,
};

static void xdg_surface_configure(void* data, struct xdg_surface* xdg_surface, uint32_t serial){
	auto self = (WindowData*)data;
	xdg_surface_ack_configure(xdg_surface, serial);
}
const struct xdg_surface_listener boat_xdg_surface_listener = {
	.configure = xdg_surface_configure,
};

static void xdg_toplevel_close(void *data, struct xdg_toplevel *xdg_toplevel){
    auto self = (WindowData*)data;
    Log("Wayland") << Info << "CLOSE!";
    self->boat->OnClose.Invoke(self->id);
}

static void xdg_toplevel_configure(void *data,
         struct xdg_toplevel *xdg_toplevel,
         int32_t width,
         int32_t height,
         struct wl_array *states){
    Log("Wayland") << Info << "CONFIG!";
    auto self = (WindowData*)data;

}
static void xdg_toplevel_conf_bounds(void *data,
                              struct xdg_toplevel *xdg_toplevel,
                              int32_t width,
                              int32_t height){

    Log("Wayland") << Info << "CONFIG BOUNDS!";
}

static void xdg_toplevel_wm_caps(void *data,
                          struct xdg_toplevel *xdg_toplevel,
                          struct wl_array *capabilities){

    Log("Wayland") << Info << "WM CAPS!";
}

const struct xdg_toplevel_listener boat_xdg_toplevel_listener = {
        .configure = xdg_toplevel_configure,
        .close = xdg_toplevel_close,
        .configure_bounds = xdg_toplevel_conf_bounds,
        .wm_capabilities = xdg_toplevel_wm_caps
};

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

    auto data = CreateRef<WindowData>();
    data->boat = this;
    data->surface = surface;
    data->xdgSurface = xdgSurface;
    data->xdgToplevel = xdgTopLvl;
    data->decor = decor;
    data->size = size;
    
	xdg_wm_base_add_listener(TiWayland::GetXdgWmBase(), &boat_xdg_wm_listener, nullptr);
    xdg_surface_add_listener(xdgSurface, &boat_xdg_surface_listener, data.get());
    xdg_toplevel_add_listener(xdgTopLvl, &boat_xdg_toplevel_listener, data.get());
    
	xdg_surface_set_window_geometry(xdgSurface, 0, 0, (int32_t)size.x, (int32_t)size.y);
	xdg_toplevel_set_title(xdgTopLvl, title.c_str());

	if(flags & WindowFlags_Resizable){
		xdg_toplevel_set_min_size(xdgTopLvl, 20, 20);
		xdg_toplevel_set_max_size(xdgTopLvl, INT32_MAX, INT32_MAX);
	}else{
		xdg_toplevel_set_min_size(xdgTopLvl, (int32_t)size.x, (int32_t)size.y);
		xdg_toplevel_set_max_size(xdgTopLvl, (int32_t)size.x, (int32_t)size.y);
	};
    
	auto id = CreateID();
    data->id = id;
	m_Windows[id] = data;
    
    zxdg_toplevel_decoration_v1_set_mode(decor, zxdg_toplevel_decoration_v1_mode::ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    
    wl_surface_commit(surface);
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
void WlWinBoat::SetWindowTitle(BID window, const std::string &title)
{
	if(m_Windows.find(window) == m_Windows.end()){
		m_Log << Error << "Failed to set window title: " << window << " does not exist";
		return;
	}

	auto data = m_Windows.at(window);
	xdg_toplevel_set_title(data->xdgToplevel, title.c_str());
    wl_display_flush(TiWayland::GetDisplay());
}
