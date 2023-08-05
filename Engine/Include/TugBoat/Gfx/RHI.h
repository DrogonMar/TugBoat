#pragma once
#include <TugBoat/Core.h>
#include <TugBoat/Core/Log.h>
#include <vector>

#include <TugBoat/Core/RemoveShortTypes.h>
#include <vulkan/vulkan.hpp>
#include <TugBoat/Core/AddShortTypes.h>
#include "Gpu.h"

/*
 * The class RHI and its subclasses like Gpu, Texture, etc. are not made to be thread safe.
 * thats up to the user of the class to make sure that the class is used in a thread safe manner.
 * This is a friendly way to interact with Vulkan and other graphics APIs in the future.
 */

namespace TugBoat
{



class TB_API RHI
{
public:
	TB_API RHI();

	void TB_API Init(bool useValidation = false);
	void TB_API Shutdown();


	//Gpu functions
	uint32_t TB_API GetGpuCount();
	Ref<Gpu> TB_API GetGpu(uint32_t index);

	VkInstance TB_API GetVkInstance() { return m_VkInstance; }

	static RHI TB_API *GetInstance()
	{ return m_Instance; }

	static Version VkVersionToVersion(const uint32_t vkVers){
		return {static_cast<uint8_t>(vkVers >> 22), static_cast<uint16_t>((vkVers >> 12) & 0x3ff), static_cast<uint16_t>(vkVers & 0xfff)};
	}
private:
	bool SupportsInstanceExtension(const char *extensionName);
	bool SupportsInstanceLayer(const char *layerName);

	void FillInstanceVectors();
	void CreateVkInstance();
	void CreateDebugCallback();

	static RHI *m_Instance;

	bool m_UsingValidation = false;
	std::vector<VkExtensionProperties> m_SupportedInstanceExtensions;
	std::vector<VkLayerProperties> m_SupportedInstanceLayers;
	std::vector<Ref<Gpu>> m_Gpus;

	//Vulkan
	VkInstance m_VkInstance;
	VkDebugUtilsMessengerEXT m_VkDbgCallback;

	LOG("Renderer");
};
}