#include "flpch.h"
#include "VulkanDevice.h"

namespace Flux {

	VulkanDevice::~VulkanDevice()
	{
		vkb::destroy_device(m_Device);
	}

	void VulkanDevice::Init(vkb::Instance& instance, VkSurfaceKHR surface)
	{
		VkPhysicalDeviceFeatures features{};
		features.samplerAnisotropy = VK_TRUE;

		vkb::PhysicalDeviceSelector selector(instance);

		auto physResult = selector
			.set_surface(surface)
			.set_minimum_version(1, 2)
			.set_required_features(features)
			.prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
			.select();

		FL_CORE_ASSERT(physResult.has_value(), "Failed to select physical device!");
		m_PhysicalDevice = physResult.value();

		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(m_PhysicalDevice.physical_device, &props);
		FL_CORE_INFO("GPU selected: {0}", props.deviceName);

		vkb::DeviceBuilder deviceBuilder(m_PhysicalDevice);

		auto deviceResult = deviceBuilder.build();
		FL_CORE_ASSERT(deviceResult.has_value(), "Failed to create logical device!");
		m_Device = deviceResult.value();

		auto queueResult = m_Device.get_queue(vkb::QueueType::graphics);
		FL_CORE_ASSERT(queueResult.has_value(), "Failed to get graphics queue!");
		m_GraphicsQueue = queueResult.value();

		auto queueFamilyResult = m_Device.get_queue_index(vkb::QueueType::graphics);
		FL_CORE_ASSERT(queueFamilyResult.has_value(), "Failed to get graphics queue family!");
		m_GraphicsQueueFamily = queueFamilyResult.value();

		FL_CORE_INFO("Vulkan device created");
	}

}