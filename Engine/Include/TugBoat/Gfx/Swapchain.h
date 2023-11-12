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
	TB_API Swapchain(class Gpu* gpu, const Ref<Surface>& surface);
	TB_API ~Swapchain();

	[[nodiscard]] bool TB_API IsValid() const {return m_Swapchain != VK_NULL_HANDLE;}

	bool TB_API AcquireNextImage(VkSemaphore semaphore = VK_NULL_HANDLE, VkFence fence = VK_NULL_HANDLE);
	bool TB_API AcquireNextImage(Ref<class GSemaphore> semaphore, VkFence fence = VK_NULL_HANDLE);
	bool TB_API Present(VkSemaphore semaphore = VK_NULL_HANDLE);
	bool TB_API Present(Ref<class GSemaphore> semaphore);

	class Gpu* m_Gpu;
	Ref<Surface> m_Surface;

	VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
	VkSurfaceFormatKHR m_SurfaceFormat{};
	VkPresentModeKHR m_PresentMode{};
	VkExtent2D m_Extent{};
	std::vector<VkImage> m_Images;
	std::vector<VkImageView> m_ImageViews;

	VkFence m_ImageAvailable = VK_NULL_HANDLE;

	uint32_t m_CurrentImageIndex = 0;

	[[nodiscard]] VkImage GetCurrentImage() const { return m_Images[m_CurrentImageIndex]; }
	[[nodiscard]] VkImageView GetCurrentImageView() const { return m_ImageViews[m_CurrentImageIndex]; }
private:
	LOG("Swapchain");
};
}