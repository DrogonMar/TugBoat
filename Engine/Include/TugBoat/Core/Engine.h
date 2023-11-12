#pragma once
#include "TugBoat/Core.h"
#include "TugBoat/Info.h"
#include "TugBoat/Gfx/Gpu.h"
#include "TugBoat/Gfx/GSemaphore.h"
#include <TugBoat/Core/Log.h>
#include <TugBoat/Core/PluginManager.h>

namespace TugBoat {
class TB_API Engine {
public:
	static TB_API Engine* GetInstance();

	TB_API Engine();
	TB_API ~Engine();

	int TB_API Main();
	void TB_API Run();
	void TB_API Shutdown();
    
    void OnWindowClose(BID windowId);

	Ref<PluginManager> TB_API GetPluginManager() { return m_PluginManager; }

	std::string TB_API GetAppName() {return ENGINE_NAME;}
	Version TB_API GetAppVersion() {return {0,0,1};}

	Gpu* m_Gpu;
	Ref<class Swapchain> m_Swapchain;
	Ref<class Texture2D> m_DepthTexture;
	uint32_t m_MaxFramesInFlight = 2;
	uint32_t m_CurrentFrame = 0;
	std::vector<VkFence> m_InFlightFences;
	std::vector<Ref<GSemaphore>> m_ImageAvailableSemaphores;
	std::vector<Ref<GSemaphore>> m_RenderFinishedSemaphores;
	std::vector<VkCommandBuffer> m_CommandBuffers;

	bool m_Running = true;
private:
	LOG("Engine");

	Ref<PluginManager> m_PluginManager;

	static Engine* s_Instance;
};
}