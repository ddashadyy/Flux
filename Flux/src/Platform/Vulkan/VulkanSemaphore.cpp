#include "flpch.h"
#include "VulkanSemaphore.h"


namespace Flux {

	VulkanSemaphore::VulkanSemaphore(VkDevice device)
		: m_Device(device)
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreInfo.pNext = nullptr;
		semaphoreInfo.flags = 0;

		vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_Semaphore);
	}

	VulkanSemaphore::~VulkanSemaphore()
	{
		vkDestroySemaphore(m_Device, m_Semaphore, nullptr);
	}
}