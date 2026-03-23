#include "flpch.h"
#include "VulkanUniformBuffer.h"
#include "VulkanContext.h"
#include "VulkanUtils.h"



namespace Flux {

	VulkanUniformBuffer::VulkanUniformBuffer(uint32_t size)
	{
		VulkanUtils::CreateBuffer(static_cast<VkDeviceSize>(size), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, m_Buffer, m_Allocation);
		FL_CORE_INFO("Created Vulkan uniform buffer with size {0} bytes", size);
	}

	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		vmaDestroyBuffer(VulkanContext::Get().GetAllocator(), m_Buffer, m_Allocation);
	}

	void VulkanUniformBuffer::SetData(const UniformBufferObject& ubo)
	{
		VulkanUtils::MapMemory(&ubo, sizeof(ubo), m_Allocation);
	}
}