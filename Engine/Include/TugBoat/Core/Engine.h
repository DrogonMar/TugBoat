#pragma once
#include "TugBoat/Core.h"
#include "TugBoat/Info.h"
#include "TugBoat/Gfx/Gpu.h"
#include <TugBoat/Core/Log.h>
#include <TugBoat/Core/PluginManager.h>

namespace TugBoat {
class TB_API Engine {
public:
	static TB_API Engine* GetInstance();

	Engine();
	~Engine();

	int Main();
	void Run();
	void Shutdown();

	Ref<PluginManager> GetPluginManager() { return m_PluginManager; }

	std::string GetAppName() {return ENGINE_NAME;}
	Version GetAppVersion() {return {0,0,1};}

	Ref<Gpu> m_Gpu;
	Ref<class Swapchain> m_Swapchain;

	bool m_Running = true;
private:
	LOG("Engine");

	Ref<PluginManager> m_PluginManager;

	static Engine* s_Instance;
};
}