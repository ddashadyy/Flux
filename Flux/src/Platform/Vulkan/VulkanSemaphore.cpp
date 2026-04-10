#include "flpch.h"
#include "VulkanSemaphore.h"

#include "Flux/Core.h"

namespace Flux {

	VulkanSemaphore::VulkanSemaphore(VkDevice device)
		: m_Device(device)
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreInfo.pNext = nullptr;
		semaphoreInfo.flags = 0;

		FL_CORE_ASSERT(vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_Semaphore) == VK_SUCCESS, "Failed to create Semaphore");
		FL_CORE_INFO("Created Vulkan Semaphore");
	}

	VulkanSemaphore::~VulkanSemaphore()
	{
		vkDestroySemaphore(m_Device, m_Semaphore, nullptr);
	}
}