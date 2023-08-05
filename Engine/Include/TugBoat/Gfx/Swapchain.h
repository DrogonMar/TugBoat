#pragma once
#include <TugBoat/Core.h>
#include <TugBoat/Core/Log.h>
#include <TugBoat/ClassDef.h>
#include <vector>

#include <TugBoat/Core/RemoveShortTypes.h>
#include <vulkan/vulkan.hpp>
#include <TugBoat/Core/AddShortTypes.h>
#include "Gpu.h"
#include "Surface.h"

namespace TugBoat
{
class TB_API Swapchain{
public:
	TB_API Swapchain(const Ref<Gpu>& gpu, const Ref<Surface>& surface);
	TB_API ~Swapchain();

	[[nodiscard]] bool TB_API IsValid() const {return m_Swapchain != VK_NULL_HANDLE;}

	Ref<Gpu> m_Gpu;
	Ref<Surface> m_Surface;

	VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
	VkSurfaceFormatKHR m_SurfaceFormat{};
	VkPresentModeKHR m_PresentMode{};
	VkExtent2D m_Extent{};
	std::vector<VkImage> m_Images;
	std::vector<VkImageView> m_ImageViews;
private:
	LOG("Swapchain");
};
}