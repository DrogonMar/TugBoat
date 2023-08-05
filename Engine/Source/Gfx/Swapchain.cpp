#include <TugBoat/Gfx/Swapchain.h>
#include <limits>
#include <TugBoat/Vectors.h>
#include "TugBoat/Boats/WindowBoat.h"
#include "TugBoat/Gfx/RHI.h"

using namespace TugBoat;

VkSurfaceFormatKHR Internal_ChooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	//if all formats supported, just return the best.
	if(availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
		return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
	}

	for(const auto& availableFormat : availableFormats) {
		if ((availableFormat.format == VK_FORMAT_R8G8B8A8_UNORM || availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM) &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
			return availableFormat;
		}
	}

	//If cant find the optimal format, just return the first
	return availableFormats[0];
}

VkPresentModeKHR Internal_ChooseBestPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	//If mailbox is available, use it
	for(const auto& availablePresentMode : availablePresentModes) {
		if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	//If not, use FIFO as fallback
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Internal_ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities, UIVec windowSize){
	//if current extent is at numeric limits, then extent can vary. otherwise it is the size of the window
	if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return surfaceCapabilities.currentExtent;
	}
	else
	{
		//if value can vary, need to set manually

		//create new extent using window size
		VkExtent2D newExtent = {};
		newExtent.width = static_cast<uint32_t>(windowSize.x);
		newExtent.height = static_cast<uint32_t>(windowSize.y);

		// Surface also defines max and min, so make sure within boundaries by clamping
		//no idea why the tut isn't using clamp, but okay?
		newExtent.width = std::max(surfaceCapabilities.minImageExtent.width,
								   std::min(surfaceCapabilities.maxImageExtent.width, newExtent.width));
		newExtent.height = std::max(surfaceCapabilities.minImageExtent.height,
									std::min(surfaceCapabilities.maxImageExtent.height, newExtent.height));

		return newExtent;
	}
}

Swapchain::Swapchain(const Ref<Gpu>& gpu, const Ref<Surface>& surface)
{
	m_Gpu = gpu;
	m_Surface = surface;

	auto vkSurface = surface->GetVkSurface();

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu->physicalDevice, vkSurface, &surfaceCapabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(gpu->physicalDevice, vkSurface, &formatCount, nullptr);

	std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	if(formatCount != 0) {
		vkGetPhysicalDeviceSurfaceFormatsKHR(gpu->physicalDevice, vkSurface, &formatCount, surfaceFormats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(gpu->physicalDevice, vkSurface, &presentModeCount, nullptr);

	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	if(presentModeCount != 0) {
		vkGetPhysicalDeviceSurfacePresentModesKHR(gpu->physicalDevice, vkSurface, &presentModeCount, presentModes.data());
	}

	m_SurfaceFormat = Internal_ChooseBestSurfaceFormat(surfaceFormats);
	m_PresentMode = Internal_ChooseBestPresentMode(presentModes);
	m_Extent = Internal_ChooseSwapExtent(surfaceCapabilities, WindowBoat::Get()->GetWindowSize(surface->GetWindow()));

	//How many images are in the swap chain? get 1 more than the minimum to allow triple buffering.
	ui32 imageCount = surfaceCapabilities.minImageCount + 1;

	//if imagecount is higher than max, then clamp down to max
	// if 0, then limitless
	if(surfaceCapabilities.maxImageCount > 0 && surfaceCapabilities.maxImageCount < imageCount) {
		imageCount = surfaceCapabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

	swapchainCreateInfo.surface = vkSurface;
	swapchainCreateInfo.imageFormat = m_SurfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = m_SurfaceFormat.colorSpace;
	swapchainCreateInfo.presentMode = m_PresentMode;
	swapchainCreateInfo.imageExtent = m_Extent;
	swapchainCreateInfo.minImageCount = imageCount; //min images in swapchain
	swapchainCreateInfo.imageArrayLayers = 1; // number of layers for each image in chain
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //what attachment images will be used as
	swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform; //transform to perform on swapchain images
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //how to handle blending images with external graphics
	swapchainCreateInfo.clipped = VK_TRUE; //whether to clip parts of image not in view (e.g. behind another window, off screen, etc)

	// if graphics and presentation families are different, then swap chain must let images be shared between families
	if(m_Gpu->QueueGraphicsFamily != m_Gpu->QueuePresentFamily){
		ui32 queueFamilyIndices[] = {static_cast<uint32_t>(m_Gpu->QueueGraphicsFamily), static_cast<uint32_t>(m_Gpu->QueuePresentFamily)};
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; //image share handling
		swapchainCreateInfo.queueFamilyIndexCount = 2; //number of queues to share images between
		swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices; //array of queues to share between
	}else{
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; //image share handling
		swapchainCreateInfo.queueFamilyIndexCount = 0; //number of queues to share images between
		swapchainCreateInfo.pQueueFamilyIndices = nullptr; //array of queues to share between
	}

	//if old swapchain been destroyed and this one replaces it, then link old one to quickly hand over responsibilities
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	if(vkCreateSwapchainKHR(gpu->logicalDevice, &swapchainCreateInfo, nullptr, &m_Swapchain) != VK_SUCCESS){
		m_Swapchain = VK_NULL_HANDLE;
		return;
	}

	ui32 swapchainImageCount;
	vkGetSwapchainImagesKHR(gpu->logicalDevice, m_Swapchain, &swapchainImageCount, nullptr);

	m_Images.resize(swapchainImageCount);
	vkGetSwapchainImagesKHR(gpu->logicalDevice, m_Swapchain, &swapchainImageCount, m_Images.data());

	for(VkImage image : m_Images){
 		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

		imageViewCreateInfo.image = image;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = m_SurfaceFormat.format;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		auto result = vkCreateImageView(gpu->logicalDevice, &imageViewCreateInfo, nullptr, &imageView);
		if(result != VK_SUCCESS){
			//entire swapchain is invalid
			//go though and destroy all image views
			for(VkImageView imageViewIter : m_ImageViews){
				vkDestroyImageView(gpu->logicalDevice, imageViewIter, nullptr);
			}
			m_ImageViews.clear();

			vkDestroySwapchainKHR(gpu->logicalDevice, m_Swapchain, nullptr);
			m_Swapchain = VK_NULL_HANDLE;
			return;
		}

		m_ImageViews.push_back(imageView);
	}
}
Swapchain::~Swapchain()
{
	m_Log << Info << "Destroying Swapchain";
	m_Gpu->WaitIdle();


	if(m_Swapchain != VK_NULL_HANDLE){
		for(VkImageView imageView : m_ImageViews){
			vkDestroyImageView(m_Gpu->logicalDevice, imageView, nullptr);
		}

		vkDestroySwapchainKHR(m_Gpu->logicalDevice, m_Swapchain, nullptr);
	}
	m_Gpu = nullptr;
	m_Surface = nullptr;
}
