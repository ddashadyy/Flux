#pragma once


#include "Flux/Renderer/RHIFence.h"

#include <vulkan/vulkan.h>

namespace Flux {
	class VulkanFence final : public RHIFence
	{
	public:
		VulkanFence(VkDevice device, bool signaled = false);
		~VulkanFence();

		void Wait(uint64_t timeout = UINT64_MAX) override;
		void Reset() override;
		bool IsSignaled() const override;

	private:
		VkDevice m_Device = VK_NULL_HANDLE;
		VkFence  m_Fence  = VK_NULL_HANDLE;

	protected:
		void* GetHandleImpl() const override { return m_Fence; }
	};
}