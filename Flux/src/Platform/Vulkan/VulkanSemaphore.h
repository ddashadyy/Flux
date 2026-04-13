#pragma once


#include "Flux/Renderer/RHISemaphore.h"

#include <vulkan/vulkan.h>

namespace Flux {

	class VulkanSemaphore final : public RHISemaphore
	{
	public:
		VulkanSemaphore(VkDevice device);
		~VulkanSemaphore();

		// сигналится и ждет через сабмит очереди
		void Signal() override {};
		void Wait()   override {};

	private:
		VkDevice    m_Device    = VK_NULL_HANDLE;
		VkSemaphore m_Semaphore = VK_NULL_HANDLE;

	protected:
		void* GetHandleImpl() const override { return m_Semaphore; }

	};
}