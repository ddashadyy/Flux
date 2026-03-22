#include "flpch.h"
#include "VulkanUtils.h"
#include "VulkanContext.h"

namespace Flux::VulkanUtils {

	void VulkanUtils::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer, VmaAllocation& allocation)
	{
		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.pNext = VK_NULL_HANDLE;
		bufferCreateInfo.flags = 0;
		bufferCreateInfo.size = static_cast<VkDeviceSize>(size);
		bufferCreateInfo.usage = usage;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferCreateInfo.queueFamilyIndexCount = 0;
		bufferCreateInfo.pQueueFamilyIndices = VK_NULL_HANDLE;

		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		FL_CORE_ASSERT(
			vmaCreateBuffer(Flux::VulkanContext::Get().GetAllocator(),
				&bufferCreateInfo, &allocationCreateInfo,
				&buffer, &allocation, nullptr) == VK_SUCCESS,
			"Failed to create vertex buffer!"
		);
	}

	void MapMemory(const void* data, size_t size, VmaAllocation allocation)
	{
		void* mapped = nullptr;
		vmaMapMemory(Flux::VulkanContext::Get().GetAllocator(), allocation, &mapped);
		memcpy(mapped, data, size);
		vmaUnmapMemory(Flux::VulkanContext::Get().GetAllocator(), allocation);
	}
}