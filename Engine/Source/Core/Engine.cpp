#include "TugBoat/Core/Log.h"
#include "TugBoat/Core/Engine.h"
#include "TugBoat/Core/OS.h"
#include "TugBoat/Boats/WindowBoat.h"
#include "TugBoat/Gfx/RHI.h"
#include "TugBoat/Gfx/Swapchain.h"
#include <TugBoat/DeltaTime.h>

#ifdef TB_PLATFORM_LINUX
#endif


//Convert WCHAR* to string
#define WCHARTOSTRING(x, store) {std::wstring ws(x); store = std::string(ws.begin(), ws.end());}

using namespace TugBoat;

Engine* Engine::s_Instance = nullptr;

Engine* Engine::GetInstance()
{
	if (s_Instance == nullptr)
	{
		s_Instance = new Engine();
	}
	return s_Instance;
}

Engine::Engine(){
	m_Log << Info << "Engine Created";
}

Engine::~Engine(){
	m_Log << Info << "Engine Destroyed";
}

int Engine::Main()
{
	m_PluginManager = CreateRef<PluginManager>();
	m_PluginManager->LoadPlugins();

	auto renderer = new RHI();
	renderer->Init(TB_DEBUG);

	auto exePath = OS::GetInstance()->GetExePath();
	m_Log << Info << "Exe Path: " << exePath;
	auto args = OS::GetInstance()->GetArgs();
	m_Log << Info << "Args: ";
	for (auto& arg : args) {
		m_Log << Info << arg;
	}


	std::vector<Ref<Gpu>> supportedGpus;
	for (int i = 0; i < renderer->GetGpuCount(); i++) {
		auto gpu = renderer->GetGpu(i);
		//Were gonna require graphics, compute, transfer and ray tracing, gpu->CanSupportGraphics()
		if(!gpu->CanSupportGraphics() || !gpu->CanSupportCompute() || !gpu->CanSupportTransfer() || !gpu->CanSupportRayTracing()){
			m_Log << Info << "GPU " << gpu->GetInfo()->name << " does not support all required features";
			continue;
		}
		supportedGpus.push_back(gpu);
	}

	if(supportedGpus.empty()){
		m_Log << Error << "No supported GPU found";
		m_Log << Error << "Gpu needs to support Graphics, Compute, Transfer and Ray Tracing";
		OS::GetInstance()->ShowMessageBox(MessageBox_Error, "No GPU found", "No supported GPU found.\n"
																			"Please make sure your GPU supports Graphics, Compute, Transfer and Ray Tracing");
		return -1;
	}

	if(supportedGpus.size() > 1 && false) {
		std::vector<std::string> gpuNames;
		for (auto &gpu : supportedGpus) {
			gpuNames.push_back(gpu->GetInfo()->name);
		}

		auto wantedGpu = OS::GetInstance()->AskSelection("Select GPU", "Please select a GPU:", gpuNames);
		if (wantedGpu == -1) {
			m_Log << Error << "No GPU selected";
			OS::GetInstance()->ShowMessageBox(MessageBox_Error, "No GPU selected", "No GPU selected");
			return -1;
		}
		m_Log << Info << "Selected GPU: " << gpuNames[wantedGpu];
		m_Gpu = supportedGpus[wantedGpu];
	}else{
		m_Log << Info << "Only one GPU found, using it.";
		m_Gpu = supportedGpus[0];
	}

	if(!m_Gpu->IsValid()){
		m_Log << Error << "Failed to get GPU";
		OS::GetInstance()->ShowMessageBox(MessageBox_Error, "Failed to get GPU", "Failed to get GPU");
		return -1;
	}

	auto window = WindowBoat::Get()->CreateWindow("TugBoat", {1280, 720}, WindowFlags_Shown);
	if(window == INVALID_BID){
		TB_SHOW_ERROR("Failed to create window");
		return -1;
	}
	auto surface = CreateRef<Surface>(m_Gpu, window);
	if(!surface->IsValid()){
		TB_SHOW_ERROR("Failed to create surface");
		return -1;
	}

	m_Gpu->Init(surface);
	if(!m_Gpu->IsInitialized()){
		TB_SHOW_ERROR("Failed to initialize GPU");
		return -1;
	}

	m_Swapchain = CreateRef<Swapchain>(m_Gpu, surface);
	if(!m_Swapchain->IsValid()){
		TB_SHOW_ERROR("Failed to create swapchain");
		return -1;
	}

	m_Running = true;
	return 0;
}



void Engine::Run()
{
	ulong oldTicks = OS::GetInstance()->GetTicksUsec();
	DeltaTime delta;

	//Get needed sub-systems
	auto windowMgr = WindowBoat::Get();

	//just for funziez
	auto winId = windowMgr->CreateWindow("TugBoat", {1280, 720}, WindowFlags_Shown);
	if (winId == INVALID_BID) {
		m_Log << Error << "Failed to create window";
		return;
	}

	while (m_Running){
		ulong newTicks = OS::GetInstance()->GetTicksUsec();
		//Delta needs to be in seconds
		delta.Update(oldTicks, newTicks);
		oldTicks = newTicks;

		//update
		if(windowMgr != nullptr){
			windowMgr->ProcessEvents();
		}

		m_Running =false;
	}

	windowMgr->DestroyWindow(winId);
}

void Engine::Shutdown()
{
	WindowBoat::Get()->DestroyWindow(m_Swapchain->m_Surface->GetWindow());

	//unref all the things
	m_Swapchain = nullptr;
	m_Gpu = nullptr;

	//Removing the window boats will cause it to be deleted
	WindowBoat::ClearBoats();
	RHI::GetInstance()->Shutdown();
	m_PluginManager->UnloadPlugins();
}