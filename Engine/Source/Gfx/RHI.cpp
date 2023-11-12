#include <TugBoat/Gfx/RHI.h>
#include "TugBoat/Core/Engine.h"
#include <TugBoat/Info.h>
#include <TugBoat/Core/OS.h>
#include <TugBoat/Boats/WindowBoat.h>
#include <algorithm>

using namespace TugBoat;

RHI* RHI::m_Instance = nullptr;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData){

	Log log = Log("Vulkan");

	switch (messageSeverity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			log << Debug << pCallbackData->pMessage;
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			log << Info << pCallbackData->pMessage;
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			log << Warning << pCallbackData->pMessage;
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			log << Error << pCallbackData->pMessage;
			break;
	}
	return VK_FALSE;
}

static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo){
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

	createInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

void RHI::Init(bool useValidation)
{
	m_UsingValidation = useValidation;
	CreateVkInstance();
	CreateDebugCallback();

	{
		ui32 deviceCount = 0;
		vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, nullptr);

		if(deviceCount == 0) {
			m_Log << Error << "Failed to find GPUs with Vulkan support!";
			return;
		}

		auto devices = std::vector<VkPhysicalDevice>(deviceCount);
		vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, devices.data());

		//We will go though this vector and purge any devices that don't meet our requirements

		//print devices
		for(const auto& device : devices) {
			auto gpu = new Gpu(device);
			if(gpu->IsValid())
				m_Gpus.push_back(gpu);
		}
	}
}

void RHI::Shutdown()
{
	for (auto& gpu : m_Gpus) {
		delete gpu;
	}
	m_Gpus.clear();

	if(m_UsingValidation)
		DestroyDebugUtilsMessengerEXT(m_VkInstance, m_VkDbgCallback, nullptr);

	vkDestroyInstance(m_VkInstance, nullptr);
}

bool RHI::SupportsInstanceExtension(const char *extensionName)
{
	auto it = std::find_if(m_SupportedInstanceExtensions.begin(), m_SupportedInstanceExtensions.end(), [extensionName](const VkExtensionProperties& ext){
		return strcmp(ext.extensionName, extensionName) == 0;
	});
	return it != m_SupportedInstanceExtensions.end();
}

bool RHI::SupportsInstanceLayer(const char *layerName)
{
	auto it = std::find_if(m_SupportedInstanceLayers.begin(), m_SupportedInstanceLayers.end(), [layerName](const VkLayerProperties& layer){
		return strcmp(layer.layerName, layerName) == 0;
	});
	return it != m_SupportedInstanceLayers.end();
}

void RHI::FillInstanceVectors()
{
	{
		ui32 extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		m_SupportedInstanceExtensions = std::vector<VkExtensionProperties>(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, m_SupportedInstanceExtensions.data());
	}
	{
		ui32 layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		m_SupportedInstanceLayers = std::vector<VkLayerProperties>(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, m_SupportedInstanceLayers.data());
	}
}

void RHI::CreateVkInstance()
{
	std::vector<const char*> validationLayers;
	validationLayers.push_back("VK_LAYER_KHRONOS_validation");

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = Engine::GetInstance()->GetAppName().c_str();
	auto appVers = Engine::GetInstance()->GetAppVersion();
	appInfo.applicationVersion = VK_MAKE_API_VERSION(0, appVers.major, appVers.minor, appVers.patch);
	auto eVers = ENGINE_VERSION;
	appInfo.engineVersion = VK_MAKE_API_VERSION(0, eVers.major, eVers.minor, eVers.patch);
	appInfo.pEngineName = ENGINE_NAME;
	appInfo.apiVersion = GetVkApiVersion();

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	std::vector<const char*> instanceExt;
	std::vector<std::string> windowExt;

	//WindowBoat Ext
	{
		auto winBoat = WindowBoat::Get();
		if(winBoat){
			windowExt = winBoat->GetInstanceExtensions();
		}
		for (const auto & winExt : windowExt) {
			instanceExt.push_back(winExt.c_str());
		}
	}

	if(m_UsingValidation && !SupportsInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)){
		m_UsingValidation = false;
		m_Log << Warning << "Requested validation but instance extension doesn't exist, turning off validation.";
	}

	if(m_UsingValidation){
		bool haveWantedLayer = true;
		m_Log << Info << "Checking support for wanted validation layers.";
		for (const auto& wantedLayer : validationLayers) {
			if(SupportsInstanceLayer(wantedLayer)){
				m_Log << Info << wantedLayer << ": o";
			}else{
				m_Log << Info << wantedLayer << ": x";
				haveWantedLayer = false;
			}
		}
		if(!haveWantedLayer){
			m_Log << Warning << "No support for wanted validation layers, turning off validation";
			m_UsingValidation = false;
		}
	}

	if(m_UsingValidation)
		instanceExt.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	bool haveWantedExts = true;

	m_Log << Info << "Checking support for wanted instance extensions.";
	for (const auto& wantedExt : instanceExt) {
		if(SupportsInstanceExtension(wantedExt)){
			m_Log << Info << wantedExt << ": o";
		}else{
			m_Log << Info << wantedExt << ": x";
			haveWantedExts = false;
		}
	}

	if(!haveWantedExts){
		TB_SHOW_FATAL_ERROR("Failed to get all required extensions.");
		return;
	}

	createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExt.size());
	createInfo.ppEnabledExtensionNames = instanceExt.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo {};

	if(m_UsingValidation){
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}else{
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
	}

	auto result = vkCreateInstance(&createInfo, nullptr, &m_VkInstance);
	if(result != VK_SUCCESS){
		m_Log << Fatal << "Failed to create Vulkan instance.";
	}
}

void RHI::CreateDebugCallback()
{
	if(!m_UsingValidation){
		return;
	}

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	auto result = CreateDebugUtilsMessengerEXT(m_VkInstance, &createInfo, nullptr, &m_VkDbgCallback);
	if(result != VK_SUCCESS){
		TB_SHOW_FATAL_ERROR("Failed to setup debug messenger.");
		return;
	}
}

RHI::RHI()
{
	m_Instance = this;
	FillInstanceVectors();
}

uint32_t RHI::GetGpuCount()
{
	return static_cast<uint32_t>(m_Gpus.size());
}

Gpu* RHI::GetGpu(uint32_t index)
{
	return m_Gpus[index];
}

const char *GpuTypeToString(GpuType type)
{
	switch(type){
		case GpuType::GpuType_Discrete:
			return "Discrete";
		case GpuType::GpuType_Integrated:
			return "Integrated";
		case GpuType::GpuType_Virtual:
			return "Virtual";
		case GpuType::GpuType_Cpu:
			return "Cpu";
		case GpuType::GpuType_Other:
			return "Other";
		default:
			return "Unknown";
	}
}