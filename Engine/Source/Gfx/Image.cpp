#include <TugBoat/Gfx/Image.h>
#include "TugBoat/Gfx/RHI.h"

using namespace TugBoat;
Image::Image(Gpu* gpu,
	uint32_t width,
			 uint32_t height,
			 VkFormat format,
			 VkImageTiling tiling,
			 VkImageUsageFlags useFlags,
			 VkMemoryPropertyFlags propFlags)
{
	m_Gpu = gpu;
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width  = width;
	imageCreateInfo.extent.height = height;
	imageCreateInfo.extent.depth  = 1;
	imageCreateInfo.mipLevels	  = 1;
	imageCreateInfo.arrayLayers   = 1;
	imageCreateInfo.format		  = format;
	imageCreateInfo.tiling		  = tiling;		//How image data should be "tiled" (arranged for optimal reading)
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;  //layout of image data on creation
	imageCreateInfo.usage		  = useFlags;
	imageCreateInfo.samples		  = VK_SAMPLE_COUNT_1_BIT; //num of samples for multi-sampling
	imageCreateInfo.sharingMode	  = VK_SHARING_MODE_EXCLUSIVE; //Whether image can be shared between queues

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

	if (vmaCreateImage(m_Gpu->allocator, &imageCreateInfo, &allocInfo, &m_Image, &m_Memory, nullptr) != VK_SUCCESS) {
		m_Log << Error << "Failed to create image";
		m_Image = VK_NULL_HANDLE;
		m_Gpu = nullptr;
		return;
	}
}
Image::~Image()
{
	m_Log << Info << "Destroying Image";
	vkDestroyImage(m_Gpu->logicalDevice, m_Image, nullptr);
	vmaFreeMemory(m_Gpu->allocator, m_Memory);
	m_Gpu = nullptr;
	m_Image = VK_NULL_HANDLE;
}

ImageView::ImageView(Gpu* gpu, const Ref<Image> &image, VkImageViewCreateInfo createInfo)
{
	if (vkCreateImageView(gpu->logicalDevice, &createInfo, nullptr, &m_ImageView) != VK_SUCCESS) {
		m_Log << Error << "Failed to create image view";
		return;
	}
	m_Gpu = gpu;
	m_Image = image;

}

ImageView::~ImageView()
{
	m_Log << Info << "Destroying Image View";
	vkDestroyImageView(m_Gpu->logicalDevice, m_ImageView, nullptr);
	m_Gpu = nullptr;
	m_Image = nullptr;
}
Sampler::Sampler(Gpu* gpu, VkSamplerCreateInfo createInfo)
{
	if (vkCreateSampler(m_Gpu->logicalDevice, &createInfo, nullptr, &m_Sampler) != VK_SUCCESS) {
		m_Log << Error << "Failed to create sampler";
		return;
	}
	m_Gpu = gpu;
}
Sampler::~Sampler()
{
	m_Log << Info << "Destroying Sampler";
	vkDestroySampler(m_Gpu->logicalDevice, m_Sampler, nullptr);
	m_Gpu = nullptr;
}
Texture2D::Texture2D(Gpu* gpu,
					 uint32_t width,
					 uint32_t height,
					 VkFormat format,
					 VkImageTiling tiling,
					 VkImageUsageFlags useFlags,
					 VkMemoryPropertyFlags propFlags,
					 VkImageAspectFlags aspectFlags)
{
	m_Gpu = gpu;
	m_Image = CreateRef<Image>(m_Gpu, width, height, format, tiling, useFlags, propFlags);
	if (!m_Image) {
		m_Log << Error << "Failed to create image";
		return;
	}
	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.image = m_Image->GetVkImage();
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
	m_ImageView = CreateRef<ImageView>(m_Gpu, m_Image, imageViewCreateInfo);
	if (!m_ImageView) {
		m_Log << Error << "Failed to create image view";
		return;
	}
}
Texture2D::~Texture2D()
{
	m_Log << Info << "Destroying Texture2D";
	m_Image = nullptr;
	m_ImageView = nullptr;
	m_Gpu = nullptr;
}
