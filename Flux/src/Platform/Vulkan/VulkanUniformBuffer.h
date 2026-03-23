#pragma once

#include "Flux/Renderer/UniformBuffer.h"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace Flux {

	class VulkanUniformBuffer final : public UniformBuffer
	{
	public:
		VulkanUniformBuffer(uint32_t size);
		~VulkanUniformBuffer();

		void SetData(const UniformBufferObject& ubo) override;

		inline VkBuffer GetBuffer() const { return m_Buffer; }

	private:
		VkBuffer      m_Buffer     = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;
	};
}