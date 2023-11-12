#include <TugBoat/Gfx/Gpu.h>
#include <set>
#include <map>
#include "TugBoat/Gfx/RHI.h"
using namespace TugBoat;

const char *TugBoat::GpuTypeToString(GpuType type)
{
	switch (type) {
		case GpuType_Other:
			return "Other";
		case GpuType_Integrated:
			return "Integrated";
		case GpuType_Discrete:
			return "Discrete";
		case GpuType_Virtual:
			return "Virtual";
		case GpuType_Cpu:
			return "CPU";
		default:
			return "Unknown";
	}
}

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
};

const std::vector<const char*> raytracingExts = {
	VK_KHR_RAY_QUERY_EXTENSION_NAME,
	VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
	VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
	VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME
};



static bool Internal_CheckDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<VkExtensionProperties> &extensions, const std::vector<const char*> &checkExtensions) {
	for (const auto& deviceExtension : checkExtensions)
	{
		bool hasExtension = false;
		for (const auto& extension : extensions)
		{
			if (strcmp(deviceExtension, extension.extensionName) == 0)
			{
				hasExtension = true;
				break;
			}
		}

		if (!hasExtension)
			return false;
	}

	return true;
}

Gpu::Gpu(VkPhysicalDevice device)
{
	physicalDevice = device;
	auto info = GetInfo();
	m_Log.ChangeClassName("Gpu-"+std::to_string(info->deviceId));
	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto &queueFamily : queueFamilies) {
			// First check if queue family has at least 1 queue in that family (could have none)
			// queue can be multiple types defined though bitfield
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT && QueueGraphicsFamily == -1)
				QueueGraphicsFamily = i;

			//check for compute support
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT && QueueComputeFamily == -1)
				QueueComputeFamily = i;

			//Transfer support
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT && QueueTransferFamily == -1)
				QueueTransferFamily = i;

			//Sparse binding support
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT && QueueSparseBindingFamily == -1)
				QueueSparseBindingFamily = i;

			//check for video support
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR && QueueVideoDecodeFamily == -1)
				QueueVideoDecodeFamily = i;

			i++;
		}
	}

	std::vector<VkExtensionProperties> extensions;
	{
		uint32_t extensionCount = 0;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		extensions.resize(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());
	}

	bool extensionsSupported = Internal_CheckDeviceExtensionSupport(device, extensions, deviceExtensions);
	SupportsRayTracing = Internal_CheckDeviceExtensionSupport(device, extensions, raytracingExts);

	if(extensionsSupported && QueueGraphicsFamily >= 0)
		valid = true;

	m_Log << Info << GpuTypeToString(info->type) << " - " << info->name;
	m_Log << Info << "\tValid? " << (valid ? "yes" : "no");
	//print if it supports the queue families
	m_Log << Info << "\tSupports Graphics? " << (CanSupportGraphics() ? "yes" : "no");
	m_Log << Info << "\tSupports Compute? " << (CanSupportCompute() ? "yes" : "no");
	m_Log << Info << "\tSupports Transfer? " << (CanSupportTransfer() ? "yes" : "no");
	m_Log << Info << "\tSupports Sparse Binding? " << (CanSupportSparseBinding() ? "yes" : "no");
	m_Log << Info << "\tSupports Video Decode? " << (CanSupportVideoDecode() ? "yes" : "no");
	m_Log << Info << "\tSupports Ray Tracing? " << (SupportsRayTracing ? "yes" : "no");
}

Gpu::~Gpu()
{
	if(!m_Initialized)
		return;

	m_Log << Info << "Destroying Gpu";
	vkDeviceWaitIdle(logicalDevice);

	std::set<VkCommandBuffer> cmdBuffers;
	cmdBuffers.insert(GraphicsCommandBuffer);
	cmdBuffers.insert(PresentCommandBuffer);
	cmdBuffers.insert(ComputeCommandBuffer);
	cmdBuffers.insert(TransferCommandBuffer);
	cmdBuffers.insert(SparseBindingCommandBuffer);
	cmdBuffers.insert(VideoDecodeCommandBuffer);

	for(auto buffer : cmdBuffers)
		vkFreeCommandBuffers(logicalDevice, GraphicsCommandPool, 1, &buffer);

	//Command pools could be the same for multiple queues
	//so we need to make sure we only delete them once
	std::set<VkCommandPool> pools;
	pools.insert(GraphicsCommandPool);
	pools.insert(PresentCommandPool);
	pools.insert(ComputeCommandPool);
	pools.insert(TransferCommandPool);
	pools.insert(SparseBindingCommandPool);
	pools.insert(VideoDecodeCommandPool);

	for(auto pool : pools)
		vkDestroyCommandPool(logicalDevice, pool, nullptr);

	vmaDestroyAllocator(allocator);
	vkDestroyDevice(logicalDevice, nullptr);
}

void TB_API Gpu::Init(const Ref<Surface>& surface)
{
	if(m_Initialized)
		return;

	m_Log << Info << "Initializing GPU";

	if(surface != nullptr){
		//Get the queue family indices for the surface
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto &queueFamily : queueFamilies) {
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface->GetVkSurface(), &presentSupport);

			if (queueFamily.queueCount > 0 && presentSupport && QueuePresentFamily == -1)
				QueuePresentFamily = i;

			i++;
		}
	}

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {};

	if(QueueGraphicsFamily >= 0)
		uniqueQueueFamilies.insert(QueueGraphicsFamily);

	if(QueuePresentFamily >= 0)
		uniqueQueueFamilies.insert(QueuePresentFamily);

	if(QueueComputeFamily >= 0)
		uniqueQueueFamilies.insert(QueueComputeFamily);

	if(QueueTransferFamily >= 0)
		uniqueQueueFamilies.insert(QueueTransferFamily);

	if(QueueSparseBindingFamily >= 0)
		uniqueQueueFamilies.insert(QueueSparseBindingFamily);

	if(QueueVideoDecodeFamily >= 0)
		uniqueQueueFamilies.insert(QueueVideoDecodeFamily);

	m_Log << Info << "Only need to create " << uniqueQueueFamilies.size() << " queues";

	for(auto queueFamilyIndex : uniqueQueueFamilies){

		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilyIndex;  // The index of the family to create a queue from
		queueCreateInfo.queueCount = 1;
		float priority = 1.0f;
		queueCreateInfo.pQueuePriorities = &priority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	std::vector<const char*> enabledExtensions;
	enabledExtensions.insert(enabledExtensions.end(), deviceExtensions.begin(), deviceExtensions.end());
	if(SupportsRayTracing)
		enabledExtensions.insert(enabledExtensions.end(), raytracingExts.begin(), raytracingExts.end());

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();

	VkPhysicalDeviceRayTracingPipelineFeaturesKHR raytracingFeatures = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
		.rayTracingPipeline = VK_TRUE
	};
	VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
		.dynamicRendering = VK_TRUE
	};

	if(SupportsRayTracing){

		dynamicRenderingFeatures.pNext = &raytracingFeatures;
	}

	deviceCreateInfo.pNext = &dynamicRenderingFeatures;

	//Physical device features the logical device will be using
	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	//Create the logical device for the given physical device
	VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice);
	if(result != VK_SUCCESS){
		m_Log << Error << "Failed to create logical device!";
		return;
	}

	if(CanSupportGraphics())
		vkGetDeviceQueue(logicalDevice, QueueGraphicsFamily, 0, &GraphicsQueue);

	if(CanSupportPresent())
		vkGetDeviceQueue(logicalDevice, QueuePresentFamily, 0, &PresentQueue);

	if(CanSupportCompute())
		vkGetDeviceQueue(logicalDevice, QueueComputeFamily, 0, &ComputeQueue);

	if(CanSupportTransfer())
		vkGetDeviceQueue(logicalDevice, QueueTransferFamily, 0, &TransferQueue);

	if(CanSupportSparseBinding())
		vkGetDeviceQueue(logicalDevice, QueueSparseBindingFamily, 0, &SparseBindingQueue);

	if(CanSupportVideoDecode())
		vkGetDeviceQueue(logicalDevice, QueueVideoDecodeFamily, 0, &VideoDecodeQueue);

	//Create command pool for queues
	//Lets only create as many pools as we need, so if the queue index is the same, we can reuse the pool
	//Maybe use a map
	std::map<uint32_t, VkCommandPool> commandPools;
	if(CanSupportGraphics()){
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = QueueGraphicsFamily;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

		if(vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &GraphicsCommandPool) != VK_SUCCESS){
			m_Log << Error << "Failed to create graphics command pool!";
			return;
		}
		commandPools[QueueGraphicsFamily] = GraphicsCommandPool;
	}
	//if commandpool already exists, use it
	//Present
	if(CanSupportPresent()){
		if(commandPools.find(QueuePresentFamily) == commandPools.end()){
			VkCommandPoolCreateInfo poolInfo = {};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = QueuePresentFamily;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

			if(vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &PresentCommandPool) != VK_SUCCESS){
				m_Log << Error << "Failed to create compute command pool!";
				return;
			}
			commandPools[QueuePresentFamily] = PresentCommandPool;
		}else{
			PresentCommandPool = commandPools[QueuePresentFamily];
		}
	}
	//Compute
	if(CanSupportCompute()){
		if(commandPools.find(QueueComputeFamily) == commandPools.end()){
			VkCommandPoolCreateInfo poolInfo = {};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = QueueComputeFamily;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

			if(vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &ComputeCommandPool) != VK_SUCCESS){
				m_Log << Error << "Failed to create compute command pool!";
				return;
			}
			commandPools[QueueComputeFamily] = ComputeCommandPool;
		}else{
			ComputeCommandPool = commandPools[QueueComputeFamily];
		}
	}
	//Transfer
	if(CanSupportTransfer()){
		if(commandPools.find(QueueTransferFamily) == commandPools.end()){
			VkCommandPoolCreateInfo poolInfo = {};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = QueueTransferFamily;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

			if(vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &TransferCommandPool) != VK_SUCCESS){
				m_Log << Error << "Failed to create transfer command pool!";
				return;
			}
			commandPools[QueueTransferFamily] = TransferCommandPool;
		}else{
			TransferCommandPool = commandPools[QueueTransferFamily];
		}
	}
	//Sparse binding
	if(CanSupportSparseBinding()){
		if(commandPools.find(QueueSparseBindingFamily) == commandPools.end()){
			VkCommandPoolCreateInfo poolInfo = {};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = QueueSparseBindingFamily;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

			if(vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &SparseBindingCommandPool) != VK_SUCCESS){
				m_Log << Error << "Failed to create sparse binding command pool!";
				return;
			}
			commandPools[QueueSparseBindingFamily] = SparseBindingCommandPool;
		}else{
			SparseBindingCommandPool = commandPools[QueueSparseBindingFamily];
		}
	}
	//Video decode
	if(CanSupportVideoDecode()){
		if(commandPools.find(QueueVideoDecodeFamily) == commandPools.end()){
			VkCommandPoolCreateInfo poolInfo = {};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = QueueVideoDecodeFamily;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

			if(vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &VideoDecodeCommandPool) != VK_SUCCESS){
				m_Log << Error << "Failed to create video decode command pool!";
				return;
			}
			commandPools[QueueVideoDecodeFamily] = VideoDecodeCommandPool;
		}else{
			VideoDecodeCommandPool = commandPools[QueueVideoDecodeFamily];
		}
	}

	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = physicalDevice;
	allocatorInfo.device = logicalDevice;
	allocatorInfo.instance = RHI::GetInstance()->GetVkInstance();
	allocatorInfo.vulkanApiVersion = RHI::GetInstance()->GetVkApiVersion();

	if(vmaCreateAllocator(&allocatorInfo, &allocator) != VK_SUCCESS){
		m_Log << Error << "Failed to create VMA allocator!";
		return;
	}

	if(CanSupportGraphics()){
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = GraphicsCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if(vkAllocateCommandBuffers(logicalDevice, &allocInfo, &GraphicsCommandBuffer) != VK_SUCCESS){
			m_Log << Error << "Failed to allocate graphics command buffer!";
			return;
		}
	}
	if(CanSupportPresent()){
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = PresentCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if(vkAllocateCommandBuffers(logicalDevice, &allocInfo, &PresentCommandBuffer) != VK_SUCCESS){
			m_Log << Error << "Failed to allocate present command buffer!";
			return;
		}
	}
	if(CanSupportCompute()){
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = ComputeCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if(vkAllocateCommandBuffers(logicalDevice, &allocInfo, &ComputeCommandBuffer) != VK_SUCCESS){
			m_Log << Error << "Failed to allocate compute command buffer!";
			return;
		}
	}
	if(CanSupportTransfer()){
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = TransferCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if(vkAllocateCommandBuffers(logicalDevice, &allocInfo, &TransferCommandBuffer) != VK_SUCCESS){
			m_Log << Error << "Failed to allocate transfer command buffer!";
			return;
		}
	}
	if(CanSupportSparseBinding()){
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = SparseBindingCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if(vkAllocateCommandBuffers(logicalDevice, &allocInfo, &SparseBindingCommandBuffer) != VK_SUCCESS){
			m_Log << Error << "Failed to allocate sparse binding command buffer!";
			return;
		}
	}
	if(CanSupportVideoDecode()){
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = VideoDecodeCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if(vkAllocateCommandBuffers(logicalDevice, &allocInfo, &VideoDecodeCommandBuffer) != VK_SUCCESS){
			m_Log << Error << "Failed to allocate video decode command buffer!";
			return;
		}
	}


	m_Initialized = true;
}

bool Internal_QueueSupportsPresent(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface){
	VkBool32 presentSupport = false;
	vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, &presentSupport);
	return presentSupport;
}

bool Gpu::CanSupportSurface(VkSurfaceKHR surface)
{
	VkBool32 presentSupport = false;

	//Just see if graphics queue supports present
	if(CanSupportGraphics()){
		if(Internal_QueueSupportsPresent(physicalDevice, QueueGraphicsFamily, surface))
			return true;
	}
	//or any other queue
	if(CanSupportCompute()){
		if(Internal_QueueSupportsPresent(physicalDevice, QueueComputeFamily, surface))
			return true;
	}
	if(CanSupportTransfer()){
		if(Internal_QueueSupportsPresent(physicalDevice, QueueTransferFamily, surface))
			return true;
	}
	if(CanSupportSparseBinding()){
		if(Internal_QueueSupportsPresent(physicalDevice, QueueSparseBindingFamily, surface))
			return true;
	}
	if(CanSupportVideoDecode()){
		if(Internal_QueueSupportsPresent(physicalDevice, QueueVideoDecodeFamily, surface))
			return true;
	}


	return false;
}

Ref<GpuInfo> Gpu::GetInfo()
{
	auto info = CreateRef<GpuInfo>();
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(physicalDevice, &props);

	//copy name
	info->name = props.deviceName;

	switch (props.deviceType) {
		case VK_PHYSICAL_DEVICE_TYPE_OTHER:
			info->type = GpuType_Other;
			break;
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			info->type = GpuType_Integrated;
			break;
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			info->type = GpuType_Discrete;
			break;
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			info->type = GpuType_Virtual;
			break;
		case VK_PHYSICAL_DEVICE_TYPE_CPU:
			info->type = GpuType_Cpu;
			break;
		default:
			info->type = GpuType_Other;
			break;
	}
	info->apiVersion = RHI::VkVersionToVersion(props.apiVersion);
	info->driverVersion = RHI::VkVersionToVersion(props.driverVersion);
	info->vendorId = props.vendorID;
	info->deviceId = props.deviceID;

	return info;
}

bool Gpu::IsValid() const
{
	return valid;
}

void TB_API Gpu::WaitIdle() const
{
	vkDeviceWaitIdle(logicalDevice);
}
bool TB_API Gpu::IsInitialized() const
{
	return m_Initialized;
}

VkFormat Gpu::ChooseSupportedFormat(const std::vector<VkFormat> &formats,
									VkImageTiling tiling,
									VkFormatFeatureFlags featureFlags)
{
	for(auto format : formats){
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);

		if(tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & featureFlags) == featureFlags){
			return format;
		}else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & featureFlags) == featureFlags)
		{
			return format;
		}
	}

	m_Log << Error << "Failed to find supported format!";
	return VK_FORMAT_UNDEFINED;
}
bool Gpu::FindMemoryTypeIndex(uint32_t allowedTypes, VkMemoryPropertyFlags properties, uint32_t* output)
{
	// Get properties of physical device memory
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	for(uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++){
		//allowed types is like 010010110001
		//000000000001
		//000000000010
		//000000000100
		//etc
		//pretty much checking every type and if two ones interact then we go into the if statement
		if((allowedTypes & (1 << i)) &&
			(memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) //desired property bit flags are part of memory types flags
		{
			*output = i;
			return true;
		}
	}

	m_Log << Error << "Failed to find suitable memory type!";
	return false;
}

#include <TugBoat/Gfx/GSemaphore.h>

Ref<GSemaphore> Gpu::CreateSemaphore()
{
	return CreateRef<GSemaphore>(this);
}
