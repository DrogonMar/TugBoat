#pragma once
#include <TugBoat/Core.h>
#include <TugBoat/Core/Log.h>
#include <TugBoat/ClassDef.h>
#include <vector>

#include <TugBoat/Core/RemoveShortTypes.h>
#include <vulkan/vulkan.hpp>
#include <TugBoat/Core/AddShortTypes.h>
#include <vma/vk_mem_alloc.h>

namespace TugBoat
{
class TB_API Image{
public:
	TB_API Image(class Gpu* gpu, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags,
	VkMemoryPropertyFlags propFlags);
	TB_API ~Image();

	[[nodiscard]] bool TB_API IsValid() const { return m_Image != VK_NULL_HANDLE; }

	[[nodiscard]] VkImage TB_API GetVkImage() const { return m_Image; }
private:
	VmaAllocation m_Memory = nullptr;
	class Gpu* m_Gpu;
	VkImage m_Image = VK_NULL_HANDLE;
	LOG("Image");
};

class TB_API ImageView{
public:
	TB_API ImageView(class Gpu* gpu, const Ref<class Image>& image, VkImageViewCreateInfo createInfo);
	TB_API ~ImageView();

	[[nodiscard]] bool TB_API IsValid() const { return m_ImageView != VK_NULL_HANDLE; }

	[[nodiscard]] VkImageView TB_API GetVkImageView() const { return m_ImageView; }
private:
	class Gpu* m_Gpu;
	Ref<class Image> m_Image;
	VkImageView m_ImageView = VK_NULL_HANDLE;

	LOG("ImageView");
};

class TB_API Sampler{
public:
	TB_API Sampler(class Gpu* gpu, VkSamplerCreateInfo createInfo);
	TB_API ~Sampler();

	[[nodiscard]] bool TB_API IsValid() const { return m_Sampler != VK_NULL_HANDLE; }

	[[nodiscard]] VkSampler TB_API GetVkSampler() const { return m_Sampler; }
private:
	class Gpu* m_Gpu;
	VkSampler m_Sampler = VK_NULL_HANDLE;

	LOG("Sampler");
};

class TB_API Texture2D{
public:
	TB_API Texture2D(class Gpu* gpu, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags,
	VkMemoryPropertyFlags propFlags, VkImageAspectFlags aspectFlags);
	TB_API ~Texture2D();

	[[nodiscard]] bool TB_API IsValid() const { return m_Image != VK_NULL_HANDLE; }

	[[nodiscard]] VkImage TB_API GetVkImage() const { return m_Image->GetVkImage(); }
	[[nodiscard]] VkImageView TB_API GetVkImageView() const { return m_ImageView->GetVkImageView(); }
private:
	class Gpu* m_Gpu;
	Ref<class Image> m_Image;
	Ref<class ImageView> m_ImageView;

	LOG("Texture2D");
};
}