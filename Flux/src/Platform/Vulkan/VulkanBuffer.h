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

		inline uint64_t    GetSize()  override { return m_Spec.Size; }
		inline BufferUsage GetUsage() override { return m_Spec.Usage; }

	private:
		BufferSpec m_Spec{};
		VkBuffer   m_Buffer = VK_NULL_HANDLE;

		VmaAllocator  m_Allocator  = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;
	};

}