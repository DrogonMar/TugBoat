#include "XWindBoat.h"
#include "TugBoat/Gfx/RHI.h"

#include <TugBoat/Core/RemoveShortTypes.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_xlib.h>
#include <TugBoat/Core/AddShortTypes.h>

using namespace TugBoat;
std::vector<std::string> XWindBoat::GetInstanceExtensions() const
{
	return {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_XLIB_SURFACE_EXTENSION_NAME
	};
}

BID XWindBoat::CreateWindow(const std::string &title, UIVec size, TugBoat::WindowFlags flags)
{
	auto data = CreateRef<XWindowData>();
	auto xplug = TiX11::GetInstance();
	data->window =
		XCreateSimpleWindow(
			xplug->m_Display, RootWindow(xplug->m_Display, xplug->m_Screen),
			0, 0, size.x, size.y,
			0, 0, 0);

	//Add title
	XStoreName(xplug->m_Display, data->window, title.c_str());

	if(flags & WindowFlags::WindowFlags_Shown){
		XMapWindow(xplug->m_Display, data->window);
	}
    
    XSetWMProtocols(xplug->m_Display, data->window, &m_atomWmDelete, 1);

	//Add window to list
	auto id = CreateID();
	m_Windows[id] = data;
	return id;
}

UIVec XWindBoat::GetWindowSize(BID window)
{
	if(m_Windows.find(window) == m_Windows.end()){
		m_Log << Error << "Window " << window << " does not exist\n";
		return {0, 0};
	}
	auto data = m_Windows[window];

	XWindowAttributes attr;
	XGetWindowAttributes(TiX11::GetInstance()->m_Display, data->window, &attr);

	return {static_cast<uint32_t>(attr.width), static_cast<uint32_t>(attr.height)};
}

void XWindBoat::DestroyWindow(BID window)
{
	if(m_Windows.find(window) == m_Windows.end()){
		m_Log << Error << "Window " << window << " does not exist\n";
		return;
	}
	auto data = m_Windows[window];

	XDestroyWindow(TiX11::GetInstance()->m_Display, data->window);

	m_Windows.erase(window);
}

void XWindBoat::ProcessEvents()
{
	auto xplug = TiX11::GetInstance();
	XEvent event;
    if (!XEventsQueued(xplug->m_Display, QueuedAlready)){
        return;
    }
    
    XNextEvent(xplug->m_Display, &event);

    switch (event.type) {
        case ClientMessage:
            if ((Atom)event.xclient.data.l[0] == m_atomWmDelete){
                //get our BID
                auto bid = GetBIDFromX11Window(event.xclient.window);
                if (bid.has_value()){
                    OnClose.Invoke(bid.value());
                }
            }
            break;
        default:
            m_Log << Info << "Unknown";
            break;
    }
}

VkSurfaceKHR XWindBoat::GetSurface(BID window)
{
	if(m_Windows.find(window) == m_Windows.end()){
		m_Log << Error << "Window " << window << " does not exist\n";
		return VK_NULL_HANDLE;
	}
	auto data = m_Windows[window];

	VkXlibSurfaceCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.dpy = TiX11::GetInstance()->m_Display;
	createInfo.window = data->window;

	VkSurfaceKHR surface;
	if(vkCreateXlibSurfaceKHR(RHI::GetInstance()->GetVkInstance(), &createInfo, nullptr, &surface) != VK_SUCCESS){
		m_Log << Error << "Failed to create surface\n";
		return VK_NULL_HANDLE;
	}

	return surface;
}

TugBoat::NativeWindowHandle XWindBoat::GetNativeWindowHandle(BID window)
{
	if(m_Windows.find(window) == m_Windows.end()){
		m_Log << Error << "Window " << window << " does not exist\n";
		return {};
	}
	auto data = m_Windows[window];

	NativeWindowHandle handle{NativeWinDataType_X11};
	handle.x11.display = TiX11::GetInstance()->m_Display;
	handle.x11.window = data->window;
	handle.x11.screen = TiX11::GetInstance()->m_Screen;
	return handle;
}

XWindBoat::XWindBoat()
{
    m_atomWmDelete = XInternAtom(TiX11::GetInstance()->m_Display, "WM_DELETE_WINDOW", True);

}

XWindBoat::~XWindBoat()
{

}

void XWindBoat::SetWindowTitle(BID window, const std::string &title)
{
	if(m_Windows.find(window) == m_Windows.end()){
		m_Log << Error << "Window " << window << " does not exist\n";
		return;
	}
	auto data = m_Windows[window];

	XStoreName(TiX11::GetInstance()->m_Display, data->window, title.c_str());
}

std::optional<BID> XWindBoat::GetBIDFromX11Window(const Window &window) const {
    for(auto& win : m_Windows){
        if (win.second->window == window){
            return win.first;
        }
    }

    return std::nullopt;
}
