#include "TiWayland.h"
#include "WlWinBoat.h"
#include <string.h>
#include <climits>

using namespace TugBoat;

PLUGIN_IMPL(1, TiWayland);

TiWayland* TiWayland::m_Instance = nullptr;
struct wl_display* TiWayland::m_Display = nullptr;
struct wl_registry* TiWayland::m_Registry = nullptr;
struct wl_shm* TiWayland::m_WlShm = nullptr;
struct wl_compositor* TiWayland::m_Compositor = nullptr;
struct xdg_wm_base* TiWayland::m_XdgWmBase = nullptr;
struct zxdg_decoration_manager_v1* TiWayland::m_DecorManager = nullptr;

#define IS_INTERFACE(wantedInter) strcmp(interface, wantedInter.name) == 0
#define SET_INTERFACE(member, type) self->member = static_cast<type*>(wl_registry_bind(registry, id, &type##_interface , version))

void TiWayland::global_registry_handler(void *data,
										struct wl_registry *registry,
										uint32_t id,
										const char *interface,
										uint32_t version)
{
	auto self = static_cast<TiWayland*>(data);

	if(IS_INTERFACE(wl_compositor_interface)){
		SET_INTERFACE(m_Compositor, wl_compositor);
	} else if (IS_INTERFACE(xdg_wm_base_interface)){
		SET_INTERFACE(m_XdgWmBase, xdg_wm_base);
	} else if (IS_INTERFACE(wl_shm_interface)){
		SET_INTERFACE(m_WlShm, wl_shm);
	} else if (IS_INTERFACE(zxdg_decoration_manager_v1_interface)){
		SET_INTERFACE(m_DecorManager, zxdg_decoration_manager_v1);
	}
}

void TiWayland::global_registry_remove(void *data, struct wl_registry *registry, uint32_t name)
{
	auto self = static_cast<TiWayland*>(data);

	//uh oh....
	self->m_Log << Warning << "Been instructed to remove a interface, but no idea how to handle...";
	self->m_Log << Warning << "Just gonna ignore, haha, ohh god this ship is going down isn't it?";
}

TiWayland::TiWayland()
{
	m_Instance = this;
	Name = "TiWayland";
	Version = "0.0.1";
}

TiWayland::~TiWayland()
{
	m_Instance = nullptr;
}

void TiWayland::Init()
{
	m_Display = wl_display_connect(nullptr);
	if(m_Display == nullptr){
		m_Log << Error << "Failed to connect to a wayland display";
		return;
	}

	m_Log << Info << "Connected to display";

	m_Registry = wl_display_get_registry(m_Display);
	wl_registry_add_listener(m_Registry, &registry_listener, this);
	wl_display_roundtrip(m_Display);

	WindowBoat::Register("Wayland", CreateRef<WlWinBoat>());
}

void TiWayland::Shutdown()
{
	WindowBoat::Unregister("Wayland");
	wl_display_disconnect(m_Display);
}
