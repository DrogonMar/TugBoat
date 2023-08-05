#pragma once
#include <TugBoat/Core.h>
#include <TugBoat/Core/Log.h>
#include <TugBoat/ClassDef.h>
#include <vector>

#include <TugBoat/Core/RemoveShortTypes.h>
#include <vulkan/vulkan.hpp>
#include <TugBoat/Core/AddShortTypes.h>
#include "Surface.h"

namespace TugBoat
{
enum GpuType{
	GpuType_Discrete,
	GpuType_Integrated,
	GpuType_Virtual,
	GpuType_Cpu,
	GpuType_Other
};
static const char* GpuTypeToString(GpuType type);

struct GpuInfo{
	std::string name;
	GpuType type = GpuType_Other;
	Version apiVersion{};
	Version driverVersion{};
	uint32_t vendorId{};
	uint32_t deviceId{};
};

class TB_API Gpu{
public:
	explicit TB_API Gpu(VkPhysicalDevice physicalDevice);
	TB_API ~Gpu();

	void TB_API Init(const Ref<class Surface>& surface);

	bool TB_API CanSupportSurface(VkSurfaceKHR surface);

	[[nodiscard]] bool TB_API CanSupportGraphics() const { return QueueGraphicsFamily >= 0; }
	[[nodiscard]] bool TB_API CanSupportPresent() const { return QueuePresentFamily >= 0; }
	[[nodiscard]] bool TB_API CanSupportCompute() const { return QueueComputeFamily >= 0; }
	[[nodiscard]] bool TB_API CanSupportTransfer() const { return QueueTransferFamily >= 0; }
	[[nodiscard]] bool TB_API CanSupportSparseBinding() const { return QueueSparseBindingFamily >= 0; }
	[[nodiscard]] bool TB_API CanSupportVideoDecode() const { return QueueVideoDecodeFamily >= 0; }
	[[nodiscard]] bool TB_API CanSupportRayTracing() const { return SupportsRayTracing; }

	Ref<GpuInfo> TB_API GetInfo();

	[[nodiscard]] bool TB_API IsValid() const;
	[[nodiscard]] bool TB_API IsInitialized() const;

	void TB_API WaitIdle() const;

	int QueueGraphicsFamily = -1;
	VkQueue GraphicsQueue = VK_NULL_HANDLE;
	VkCommandPool GraphicsCommandPool = VK_NULL_HANDLE;

	int QueuePresentFamily = -1;
	VkQueue PresentQueue = VK_NULL_HANDLE;
	VkCommandPool PresentCommandPool = VK_NULL_HANDLE;

	int QueueComputeFamily = -1;
	VkQueue ComputeQueue = VK_NULL_HANDLE;
	VkCommandPool ComputeCommandPool = VK_NULL_HANDLE;

	int QueueTransferFamily = -1;
	VkQueue TransferQueue = VK_NULL_HANDLE;
	VkCommandPool TransferCommandPool = VK_NULL_HANDLE;

	int QueueSparseBindingFamily = -1;
	VkQueue SparseBindingQueue = VK_NULL_HANDLE;
	VkCommandPool SparseBindingCommandPool = VK_NULL_HANDLE;

	int QueueVideoDecodeFamily = -1;
	VkQueue VideoDecodeQueue = VK_NULL_HANDLE;
	VkCommandPool VideoDecodeCommandPool = VK_NULL_HANDLE;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice logicalDevice = VK_NULL_HANDLE;
private:
	Log m_Log = Log("Gpu");
	bool valid = false;
	bool SupportsRayTracing = false;
	bool m_Initialized = false;
};
}