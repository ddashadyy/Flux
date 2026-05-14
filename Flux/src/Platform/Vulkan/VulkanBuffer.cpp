#include "flpch.h"
#include "VulkanBuffer.h"


namespace {

    constexpr VkBufferUsageFlags GetVkBufferUsageFlags(Flux::BufferUsage usage)
    {
        using enum Flux::BufferUsage;

        VkBufferUsageFlags flags = 0;

        if (HasFlag(usage, Vertex))
            flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        if (HasFlag(usage, Index))
            flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        if (HasFlag(usage, Uniform))
            flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

        if (HasFlag(usage, Storage))
            flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

        if (HasFlag(usage, Staging))
            flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        if (HasFlag(usage, Indirect))
            flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

        FL_CONSTEXPR_ASSERT(flags != 0, "BufferUsage resulted in empty VkBufferUsageFlags!");

        return flags;
    }

} // anonymous namespace

namespace Flux {

    VulkanBuffer::VulkanBuffer(VmaAllocator allocator, const BufferSpec& spec)
        : m_Spec(spec), m_Allocator(allocator)
    {
        VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufferInfo.size = spec.Size;
        bufferInfo.usage = GetVkBufferUsageFlags(spec.Usage);

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = spec.CpuVisible ? VMA_MEMORY_USAGE_CPU_TO_GPU : VMA_MEMORY_USAGE_GPU_ONLY;

        FL_CORE_ASSERT(vmaCreateBuffer(m_Allocator, &bufferInfo, &allocInfo,
            &m_Buffer, &m_Allocation, nullptr) == VK_SUCCESS,
            "Failed to create Vulkan Buffer");

        FL_CORE_INFO("Created Vulkan Buffer {} bytes", spec.Size);
    }

    VulkanBuffer::~VulkanBuffer()
    {
        vmaDestroyBuffer(m_Allocator, m_Buffer, m_Allocation);
    }

    void* VulkanBuffer::Map()
    {
        void* data;
        vmaMapMemory(m_Allocator, m_Allocation, &data);
        return data;
    }

    void VulkanBuffer::Unmap()
    {
        vmaUnmapMemory(m_Allocator, m_Allocation);
    }

} // namespace Flux
