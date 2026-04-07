#pragma once


#include "Flux/Renderer/RHIBuffer.h"

#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"

namespace Flux {

	class VulkanBuffer final : public RHIBuffer
	{
	public:
		VulkanBuffer(VmaAllocator allocator, const BufferSpec& spec);
		~VulkanBuffer();

		void* Map() override;
		void  Unmap() override;

		inline uint64_t    GetSize()  const override { return m_Spec.Size; }
		inline BufferUsage GetUsage() const override { return m_Spec.Usage; }

		inline VkBuffer    GetHandle() const { return m_Buffer; }

	private:
		BufferSpec m_Spec{};
		VkBuffer   m_Buffer = VK_NULL_HANDLE;

		VmaAllocator  m_Allocator  = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;
	};

}