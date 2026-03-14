#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <VkBootstrap.h>

#include "Flux/Core.h"

namespace Flux {

	class VulkanDevice
	{
	public:
		FL_NON_COPYABLE(VulkanDevice);

		VulkanDevice() = default;
		~VulkanDevice();

		void Init(vkb::Instance& instance, VkSurfaceKHR surface);

		inline VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice.physical_device; }
		inline VkDevice         GetDevice()         const { return m_Device.device; }
		inline VkQueue          GetGraphicsQueue()  const { return m_GraphicsQueue; }
		inline uint32_t         GetGraphicsQueueFamily() const { return m_GraphicsQueueFamily; }

		inline vkb::PhysicalDevice& GetVkbPhysicalDevice() { return m_PhysicalDevice; }
		inline vkb::Device&         GetVkbDevice() { return m_Device; }

	private:
		vkb::PhysicalDevice m_PhysicalDevice;
		vkb::Device         m_Device;

		VkQueue  m_GraphicsQueue = VK_NULL_HANDLE;
		uint32_t m_GraphicsQueueFamily = 0;
	};

}