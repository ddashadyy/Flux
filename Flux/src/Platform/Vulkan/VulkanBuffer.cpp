#include "flpch.h"
#include "VulkanBuffer.h"

namespace Flux {

	static VkBufferUsageFlags GetVkBufferUsageFlags(BufferUsage usage)
	{
		switch (usage)
		{
			case BufferUsage::Vertex:  return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			case BufferUsage::Index:   return VK_BUFFER_USAGE_INDEX_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			case BufferUsage::Uniform: return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			case BufferUsage::Storage: return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			case BufferUsage::Staging: return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		}

		FL_CORE_ASSERT(false, "Unknown buffer usage!");
		return VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;
	}

	static const char* BufferUsageToString(BufferUsage usage)
	{
		switch (usage)
		{
		case BufferUsage::Vertex:  return "Vertex";
		case BufferUsage::Index:   return "Index";
		case BufferUsage::Uniform: return "Uniform";
		case BufferUsage::Storage: return "Storage";
		case BufferUsage::Staging: return "Staging";
		}

		FL_CORE_ASSERT(false, "Unknown buffer usage!");
		return "Unknown";
	}

	VulkanBuffer::VulkanBuffer(VmaAllocator allocator, const BufferSpec& spec)
		: m_Spec(spec), m_Allocator(allocator)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = spec.Size;
		bufferInfo.usage = GetVkBufferUsageFlags(spec.Usage);
		
		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = spec.CpuVisible ? VMA_MEMORY_USAGE_CPU_TO_GPU : VMA_MEMORY_USAGE_GPU_ONLY;

		vmaCreateBuffer(m_Allocator, &bufferInfo, &allocInfo, &m_Buffer, &m_Allocation, nullptr);

		FL_CORE_INFO("Created Vulkan buffer, size {0} bytes, usage: {1}", spec.Size, BufferUsageToString(spec.Usage));
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
}
