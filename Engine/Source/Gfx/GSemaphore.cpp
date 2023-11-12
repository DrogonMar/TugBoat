#include <TugBoat/Gfx/GSemaphore.h>
#include <TugBoat/Gfx/Gpu.h>
using namespace TugBoat;

GSemaphore::GSemaphore(Gpu *gpu)
{
	m_Gpu = gpu;
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	if(vkCreateSemaphore(gpu->logicalDevice, &semaphoreInfo, nullptr, &m_Sema) != VK_SUCCESS){
		m_Sema = VK_NULL_HANDLE;
		return;
	}
}

GSemaphore::GSemaphore(Gpu *gpu, VkSemaphore input_sema)
{
	m_Gpu = gpu;
	m_Sema = input_sema;
}

GSemaphore::~GSemaphore()
{
	vkDestroySemaphore(m_Gpu->logicalDevice, m_Sema, nullptr);
}