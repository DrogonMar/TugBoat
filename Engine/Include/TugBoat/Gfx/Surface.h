#pragma once
#include <TugBoat/Core.h>
#include <TugBoat/Core/Log.h>
#include <TugBoat/ClassDef.h>
#include <vector>

#include <TugBoat/Core/RemoveShortTypes.h>
#include <vulkan/vulkan.hpp>
#include <TugBoat/Core/AddShortTypes.h>

namespace TugBoat
{
class TB_API Surface{
public:
	TB_API Surface(class Gpu* gpu, BID window);
	TB_API ~Surface();

	[[nodiscard]] bool TB_API IsValid() const { return m_Surface != VK_NULL_HANDLE; }

	[[nodiscard]] BID TB_API GetWindow() const { return m_Window; }
	[[nodiscard]] VkSurfaceKHR TB_API GetVkSurface() const { return m_Surface; }

private:
	BID m_Window = INVALID_BID;
	VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

	LOG("Surface");
};
}