#include "flpch.h"
#include "VulkanContext.h"
#include "VulkanBuffer.h"
#include "VulkanUtils.h"

namespace Flux {

	VulkanVertexBuffer::VulkanVertexBuffer(uint32_t size)
	{
		VulkanUtils::CreateBuffer(static_cast<VkDeviceSize>(size), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, m_Buffer, m_Allocation);
		FL_CORE_INFO("Created Vulkan Vertex buffer with size {0} bytes", size);
	}

	VulkanVertexBuffer::VulkanVertexBuffer(float* vertices, uint32_t size)
	{
		VulkanUtils::CreateBuffer(static_cast<VkDeviceSize>(size), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, m_Buffer, m_Allocation);
		VulkanUtils::MapMemory(vertices, size, m_Allocation);

		FL_CORE_INFO("Created Vulkan Vertex buffer with size {0} bytes", size);
	}

	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		vmaDestroyBuffer(VulkanContext::Get().GetAllocator(), m_Buffer, m_Allocation);
	}

	void VulkanVertexBuffer::Bind() const
	{
		VkCommandBuffer cmd = VulkanContext::Get().GetCurrentCommandBuffer();
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(cmd, 0, 1, &m_Buffer, &offset);
	}

	void VulkanVertexBuffer::Unbind() const
	{
		
	}

	VulkanIndexBuffer::VulkanIndexBuffer(uint32_t size)
	{
		VulkanUtils::CreateBuffer(static_cast<VkDeviceSize>(size), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, m_Buffer, m_Allocation);
		FL_CORE_INFO("Created Vulkan Index buffer with size {0} bytes", size);
	}

	VulkanIndexBuffer::VulkanIndexBuffer(uint32_t* indices, uint32_t size)
	{
		VulkanUtils::CreateBuffer(static_cast<VkDeviceSize>(size), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, m_Buffer, m_Allocation);
		VulkanUtils::MapMemory(indices, size, m_Allocation);

		FL_CORE_INFO("Created Vulkan Index buffer with size {0} bytes", size);
	}

	VulkanIndexBuffer::~VulkanIndexBuffer()
	{
		vmaDestroyBuffer(VulkanContext::Get().GetAllocator(), m_Buffer, m_Allocation);
	}

	void VulkanIndexBuffer::Bind() const
	{
		VkCommandBuffer cmd = VulkanContext::Get().GetCurrentCommandBuffer();
		vkCmdBindIndexBuffer(cmd, m_Buffer, 0, VK_INDEX_TYPE_UINT32);
	}

	void VulkanIndexBuffer::Unbind() const
	{
	}
}
