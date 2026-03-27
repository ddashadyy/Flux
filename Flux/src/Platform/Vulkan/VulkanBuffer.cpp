#include "flpch.h"
#include "VulkanContext.h"
#include "VulkanBuffer.h"
#include "VulkanUtils.h"

namespace Flux {

	////////////////////////////////////////////////////////////////
	///// Vertex Buffer ////////////////////////////////////////////
	////////////////////////////////////////////////////////////////

	VulkanVertexBuffer::VulkanVertexBuffer(uint32_t size)
	{
		VulkanUtils::CreateBuffer(sizeof(uint32_t) * static_cast<VkDeviceSize>(size), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, m_Buffer, m_Allocation);
		FL_CORE_INFO("Created Vulkan Vertex buffer with size {0} bytes", size);
	}

	VulkanVertexBuffer::VulkanVertexBuffer(float* vertices, uint32_t size)
	{
		VulkanUtils::CreateBuffer(static_cast<VkDeviceSize>(size), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, m_Buffer, m_Allocation);
		VulkanUtils::MapMemory(vertices, sizeof(uint32_t) * size, m_Allocation);

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

	////////////////////////////////////////////////////////////////
	///// Index Buffer ////////////////////////////////////////////
	////////////////////////////////////////////////////////////////

	VulkanIndexBuffer::VulkanIndexBuffer(uint32_t* indices, uint32_t count)
	{
		VulkanUtils::CreateBuffer(sizeof(uint32_t) * static_cast<VkDeviceSize>(count), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, m_Buffer, m_Allocation);
		VulkanUtils::MapMemory(indices, sizeof(uint32_t) * count, m_Allocation);

		m_Count = count;

		FL_CORE_INFO("Created Vulkan Index buffer with size {0} bytes", count);
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
