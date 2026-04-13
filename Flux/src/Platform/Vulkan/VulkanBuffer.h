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

		uint64_t    GetSize()   const override { return m_Spec.Size; }
		BufferUsage GetUsage()  const override { return m_Spec.Usage; }

	private:
		BufferSpec m_Spec{};
		VkBuffer   m_Buffer = VK_NULL_HANDLE;

		VmaAllocator  m_Allocator  = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;

	protected:
		void* GetHandleImpl() const override { return m_Buffer; }
	};

}