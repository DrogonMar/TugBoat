#pragma once
#include <TugBoat/Core.h>
#include <TugBoat/ClassDef.h>
#include <vector>

#include <TugBoat/Core/RemoveShortTypes.h>
#include <vulkan/vulkan.hpp>
#include <TugBoat/Core/AddShortTypes.h>

namespace TugBoat
{
class TB_API GSemaphore{
public:
	TB_API GSemaphore(class Gpu* gpu);
	TB_API GSemaphore(class Gpu* gpu, VkSemaphore input_sema);
	TB_API ~GSemaphore();

	[[nodiscard]] bool TB_API IsValid() const { return m_Sema != VK_NULL_HANDLE; }

	inline VkSemaphore GetVk(){
		return m_Sema;
	}

	operator VkSemaphore() const{
		return m_Sema;
	}
private:
	class Gpu* m_Gpu;
	VkSemaphore m_Sema;
};
}