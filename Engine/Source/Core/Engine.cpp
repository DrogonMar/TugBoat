#include "TugBoat/Core/Log.h"
#include "TugBoat/Core/Engine.h"
#include "TugBoat/Core/OS.h"
#include "TugBoat/Boats/WindowBoat.h"
#include "TugBoat/Gfx/RHI.h"
#include "TugBoat/Gfx/Swapchain.h"
#include "TugBoat/Gfx/Image.h"
#include <TugBoat/DeltaTime.h>
#include <math.h>


//Convert WCHAR* to string
#define WCHARTOSTRING(x, store) {std::wstring ws(x); store = std::string(ws.begin(), ws.end());}

using namespace TugBoat;

//Some nice util functions
//A function to set the preferred boat with the param just being the environment variable name
template <typename T>
bool SetPreferredBoatEnv(const std::string& envName, Log &log){
	auto env = TugBoat::OS::GetInstance()->GetEnvVar(envName);
	if(!env.hasValue)
		return true;

	auto types = T::GetBoatNames();
	if(std::find(types.begin(), types.end(), env.value) == types.end()){
		log << Warning << "Environment variable " << envName << " is set to " << env.value << " but that boat is not registered";
		log << Warning << "Valid types are:";
		for(auto& type : types){
			log << Warning << "\t" << type;
		}
		return false;
	}
	log << Info << "Setting preferred boat to " << env.value;
	return T::SetPreferredBoat(env.value);
}

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

void Engine::OnWindowClose(BID windowID){
    //Honestly coudlnt care...
	m_Running = false;
}

int Engine::Main()
{
	m_PluginManager = CreateRef<PluginManager>();
	m_PluginManager->LoadPlugins();

    WindowBoat::SetPreferredBoat("Wayland");
	SetPreferredBoatEnv<WindowBoat>("WB_IMPL", m_Log);

	bool useValidation = false;
#ifdef TB_DEBUG
	useValidation = true;
#endif

	auto renderer = new RHI();
	renderer->Init(useValidation);

	auto exePath = OS::GetInstance()->GetExePath();
	m_Log << Info << "Exe Path: " << exePath;
	auto args = OS::GetInstance()->GetArgs();
	m_Log << Info << "Args: ";
	for (auto& arg : args) {
		m_Log << Info << arg;
	}


	std::vector<Gpu*> supportedGpus;
	for (int i = 0; i < renderer->GetGpuCount(); i++) {
		auto gpu = renderer->GetGpu(i);
		//Were gonna require graphics, compute, transfer and ray tracing, gpu->CanSupportGraphics()
		if(!gpu->CanSupportGraphics() || !gpu->CanSupportCompute() || !gpu->CanSupportTransfer()){
			m_Log << Info << "GPU " << gpu->GetInfo()->name << " does not support all required features";
			continue;
		}
		supportedGpus.push_back(gpu);
	}

	if(supportedGpus.empty()){
		m_Log << Error << "No supported GPU found";
		TB_SHOW_ERROR("No supported GPU found.\n"
					  "Please make sure your GPU supports Graphics, Compute, Transfer and Ray Tracing");
		return -1;
	}

	auto showGpuSelectionDialog = OS::GetInstance()->GetEnvVar("TB_GPU_SELECTION");
	auto shouldShowGpuSelect = showGpuSelectionDialog.hasValue && showGpuSelectionDialog.value == "1";

	if(supportedGpus.size() > 1 && shouldShowGpuSelect) {
		std::vector<std::string> gpuNames;
		for (auto &gpu : supportedGpus) {
			gpuNames.push_back(gpu->GetInfo()->name);
		}

		auto wantedGpu = OS::GetInstance()->AskSelection("Select GPU", "Please select a GPU:", gpuNames);
		if (wantedGpu == -1) {
			TB_SHOW_ERROR("No GPU selected");
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
    WindowBoat::Get()->OnClose.Register(&Engine::OnWindowClose, this);
    
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

	auto maxImagesInSwapchain = m_Swapchain->m_Images.size();
	m_MaxFramesInFlight = maxImagesInSwapchain;
	//now create our inflight fences
	for (int i = 0; i < m_MaxFramesInFlight; i++) {
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VkFence fence;
		if(vkCreateFence(m_Gpu->logicalDevice, &fenceInfo, nullptr, &fence) != VK_SUCCESS){
			TB_SHOW_ERROR("Failed to create fence");
			return -1;
		}
		m_InFlightFences.push_back(fence);
	}

	//now create our semaphores
	for(int i = 0; i < maxImagesInSwapchain; i++){
		Ref<GSemaphore> imageAvailableSemaphore = m_Gpu->CreateSemaphore();
		Ref<GSemaphore> renderFinishedSemaphore = m_Gpu->CreateSemaphore();
		if(!imageAvailableSemaphore->IsValid() || !renderFinishedSemaphore->IsValid()){
			TB_SHOW_ERROR("Failed to create semaphores");
			return -1;
		}
		m_ImageAvailableSemaphores.push_back(imageAvailableSemaphore);
		m_RenderFinishedSemaphores.push_back(renderFinishedSemaphore);
	}

	for(int i = 0; i < maxImagesInSwapchain; i++){
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_Gpu->GraphicsCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		if(vkAllocateCommandBuffers(m_Gpu->logicalDevice, &allocInfo, &commandBuffer) != VK_SUCCESS){
			TB_SHOW_ERROR("Failed to allocate command buffer");
			return -1;
		}
		m_CommandBuffers.push_back(commandBuffer);
	}

	auto depthFormat = m_Gpu->ChooseSupportedFormat({VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT},
													VK_IMAGE_TILING_OPTIMAL,
													VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	if(depthFormat == VK_FORMAT_UNDEFINED){
		TB_SHOW_ERROR("Failed to find depth format");
		return -1;
	}

	m_DepthTexture = CreateRef<Texture2D>(m_Gpu,
											 m_Swapchain->m_Extent.width,
											 m_Swapchain->m_Extent.height,
											 depthFormat,
											 VK_IMAGE_TILING_OPTIMAL,
											 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
											 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
											 VK_IMAGE_ASPECT_DEPTH_BIT);
	if(!m_DepthTexture->IsValid()){
		TB_SHOW_ERROR("Failed to create depth texture");
		return -1;
	}


	//setup ray tracing vk stuff


	m_Running = true;
	return 0;
}



void Engine::Run()
{
	auto *os = OS::GetInstance();
	ulong oldTicks = os->GetTicksUsec();
	DeltaTime delta;

	std::vector<DeltaTime> deltas = std::vector<DeltaTime>();

	//Get needed sub-systems
	auto *windowMgr = WindowBoat::Get();

	while (m_Running){
		ulong newTicks = os->GetTicksUsec();
		//Delta needs to be in seconds
		delta.Update(oldTicks, newTicks);
		//Make a hard cap to 1000 FPS
		if(delta.m_DeltaSecs < 1.0 / 1000.0){
			if(windowMgr != nullptr){
				windowMgr->ProcessEvents();
			}
			continue;
		}

		oldTicks = newTicks;

		deltas.push_back(delta);
		if(deltas.size() > 128){
			//calculate average
			double sum = 0;
			for(auto& d : deltas){
				sum += d.m_DeltaSecs;
			}
			double avg = sum / deltas.size();
			deltas.clear();

			//now we can calculate the FPS
			auto fps = 1.0 / avg;
			//fps to int going to nearest dont just add 0.5 since thats hacky
			int fpsInt = round(fps);
			windowMgr->SetWindowTitle(0, std::string("TugBoat | FPS: ") + std::to_string(fpsInt));
		}



		auto maxImagesInSwapchain = m_Swapchain->m_Images.size();

		//Remember swapchain contains the image available semaphore renderFinishedSemaphore
		//and inFlightFences

		//wait for the fence to be available
		vkWaitForFences(
			m_Gpu->logicalDevice,
			1,
			&m_InFlightFences[m_CurrentFrame],
			VK_TRUE,
			std::numeric_limits<uint32_t>::max());
		vkResetFences(m_Gpu->logicalDevice, 1, &m_InFlightFences[m_CurrentFrame]);

		m_Swapchain->AcquireNextImage(m_ImageAvailableSemaphores[m_CurrentFrame]);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		auto buffer = m_CommandBuffers[m_CurrentFrame];
		vkBeginCommandBuffer(buffer, &beginInfo);

		{
			const VkImageMemoryBarrier image_memory_barrier {
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				.image = m_Swapchain->GetCurrentImage(),
				.subresourceRange = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
				}
			};

			vkCmdPipelineBarrier(
				buffer,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,  // srcStageMask
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // dstStageMask
				0,
				0,
				nullptr,
				0,
				nullptr,
				1, // imageMemoryBarrierCount
				&image_memory_barrier // pImageMemoryBarriers
			);

		}

		//make clear color #97d488
		const VkClearValue clearColor = {0.592f, 0.831f, 0.533f, 1.0f};

		const VkRenderingAttachmentInfo colorAttachmentInfo {
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.imageView = m_Swapchain->GetCurrentImageView(),
			.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = clearColor,
		};
		const VkRenderingAttachmentInfo depthAttachmentInfo {
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.imageView = m_DepthTexture->GetVkImageView(),
			.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.clearValue = {1.0f, 0},
		};

		const VkRenderingInfoKHR renderInfo {
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			.renderArea = {{0, 0}, m_Swapchain->m_Extent},
			.layerCount = 1,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachmentInfo,
			.pDepthAttachment = &depthAttachmentInfo,
		};

		vkCmdBeginRendering(buffer, &renderInfo);

		vkCmdEndRendering(buffer);

		//Transition swapchain image to present layout
		{
			const VkImageMemoryBarrier imageMemoryBarrier {
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				.image = m_Swapchain->GetCurrentImage(),
				.subresourceRange = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1
				}
			};

			vkCmdPipelineBarrier(
				buffer,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
				0,
				0,
				nullptr,
				0,
				nullptr,
				1,
				&imageMemoryBarrier
			);
		}

		vkEndCommandBuffer(buffer);

		//submit the command buffer
		auto *imageAvailSemaVk = m_ImageAvailableSemaphores[m_CurrentFrame]->GetVk();
		auto *renderFinishedSemaVk = m_RenderFinishedSemaphores[m_CurrentFrame]->GetVk();
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &imageAvailSemaVk;
		VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentFrame];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &renderFinishedSemaVk;

		VkResult result = vkQueueSubmit(m_Gpu->GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]);
		if(result != VK_SUCCESS){
			TB_SHOW_ERROR("Failed to submit draw command buffer");
			return;
		}

		m_Swapchain->Present(m_RenderFinishedSemaphores[m_CurrentFrame]);

		m_CurrentFrame = (m_CurrentFrame + 1) % maxImagesInSwapchain;

		//update
		if(windowMgr != nullptr){
			windowMgr->ProcessEvents();
		}

	}
}

void Engine::Shutdown()
{
	WindowBoat::Get()->DestroyWindow(m_Swapchain->m_Surface->GetWindow());

	//unref all the things
	m_DepthTexture = nullptr;
	m_Swapchain = nullptr;
	for (auto& fence: m_InFlightFences) {
		vkDestroyFence(m_Gpu->logicalDevice, fence, nullptr);
	}
	m_ImageAvailableSemaphores.clear();
	m_RenderFinishedSemaphores.clear();

	m_Gpu = nullptr;

	//Removing the window boats will cause it to be deleted
	WindowBoat::ClearBoats();
	RHI::GetInstance()->Shutdown();
	m_PluginManager->UnloadPlugins();
}