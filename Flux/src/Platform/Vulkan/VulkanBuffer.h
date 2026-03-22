#pragma once


#include "Flux/Renderer/Buffer.h"

#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"

namespace Flux {

	class VulkanVertexBuffer final : public VertexBuffer
	{
	public: 
		VulkanVertexBuffer(uint32_t size);
		VulkanVertexBuffer(float* vertices, uint32_t size);

		~VulkanVertexBuffer();

		void Bind() const override;
		void Unbind() const override;

	private: 
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;
	};


	class VulkanIndexBuffer final : public IndexBuffer
	{
	public:
		VulkanIndexBuffer(uint32_t size);
		VulkanIndexBuffer(uint32_t* indices, uint32_t size);

		~VulkanIndexBuffer();

		void Bind() const override;
		void Unbind() const override;

	private:
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;
	};

}