#pragma once
#include <TugBoat/Core.h>
#include <TugBoat/Core/Log.h>
#include <TugBoat/Core/Plugin.h>
#include <utility>
#include <vector>
#include <functional>
#include <algorithm>
#include <wayland-client.hpp>
#include <xdg-shell-client-protocol.h>
#include <xdg-decoration-unstable-v1-client-protocol.h>
#include "WlWinBoat.h"

using namespace TugBoat;

class TiWayland : public Plugin {
public:
	TiWayland();
	~TiWayland() override;

	void Init() override;
	void Shutdown() override;

	static wl_display* GetDisplay() { return m_Display; }
	static wl_compositor* GetCompositor() { return m_Compositor; }
	static xdg_wm_base* GetXdgWmBase() { return m_XdgWmBase; }
	static zxdg_decoration_manager_v1* GetDecorManager() { return m_DecorManager; }
	static wl_shm* GetShm() { return m_WlShm; }

	static TiWayland* GetInstance(){return m_Instance;}
private:
	LOG("TiWayland");

	static void global_registry_handler(void* data, struct wl_registry* registry, uint32_t id, const char* interface, uint32_t version);
	static void global_registry_remove(void* data, struct wl_registry* registry, uint32_t name);

	const struct wl_registry_listener registry_listener = {
		.global = global_registry_handler,
		.global_remove = global_registry_remove,
	};

	// Global
	static TiWayland* m_Instance;
	static struct wl_display* m_Display;
	static struct wl_registry* m_Registry;
	static struct wl_shm* m_WlShm;
	static struct wl_compositor* m_Compositor;
	static struct xdg_wm_base* m_XdgWmBase;
	static struct zxdg_decoration_manager_v1* m_DecorManager;
};