#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace Flux::VulkanUtils {

    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer, VmaAllocation& allocation);
    void MapMemory(const void* data, size_t size, VmaAllocation allocation);

}