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

		inline const BufferLayout& GetLayout() const { return m_Layout; };
		inline void SetLayout(const BufferLayout& layout) { m_Layout = layout; }

	private: 
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;

		BufferLayout m_Layout;
	};


	class VulkanIndexBuffer final : public IndexBuffer
	{
	public:
		VulkanIndexBuffer(uint32_t* indices, uint32_t count);

		~VulkanIndexBuffer();

		void Bind() const override;
		void Unbind() const override;

		inline uint32_t GetCount() const override { return m_Count; }

	private:
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;

		uint32_t m_Count = 0;
	};

}