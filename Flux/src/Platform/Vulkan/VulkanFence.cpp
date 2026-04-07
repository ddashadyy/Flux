#include "flpch.h"
#include "VulkanFence.h"

#include "Flux/Core.h"


namespace Flux {

	VulkanFence::VulkanFence(VkDevice device, bool signaled)
		: m_Device(device)
	{
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.pNext = nullptr;
		fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

		FL_CORE_ASSERT(vkCreateFence(m_Device, &fenceInfo, nullptr, &m_Fence) == VK_SUCCESS,
			"Failed to create Vulkan Fence");

		FL_CORE_INFO("Created Vulkan Fence");
	}

	VulkanFence::~VulkanFence()
	{
		vkDestroyFence(m_Device, m_Fence, nullptr);
	}

	void VulkanFence::Wait(uint64_t timeout)
	{
		vkWaitForFences(m_Device, 1, &m_Fence, VK_TRUE, timeout);
	}

	void VulkanFence::Reset()
	{
		vkResetFences(m_Device, 1, &m_Fence);
	}

	bool VulkanFence::IsSignaled() const
	{
		return vkGetFenceStatus(m_Device, m_Fence) == VK_SUCCESS;
	}
}