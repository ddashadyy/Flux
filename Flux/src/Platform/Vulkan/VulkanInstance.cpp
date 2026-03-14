#include "flpch.h"
#include "VulkanInstance.h"

#include "Flux/Log.h"
#include "Flux/Core.h"

namespace Flux {

	VulkanInstance::~VulkanInstance()
	{
		if (m_Surface != VK_NULL_HANDLE)
			vkDestroySurfaceKHR(m_Instance.instance, m_Surface, nullptr);

		vkb::destroy_instance(m_Instance);
	}

	void VulkanInstance::Init(const char* appName)
	{
		vkb::InstanceBuilder builder;

		auto result = builder
			.set_app_name(appName)
		#ifdef FL_DEBUG
			.request_validation_layers(true)   
			.use_default_debug_messenger()     
		#endif
			.require_api_version(1, 2, 0)
			.build();

		FL_CORE_ASSERT(result.has_value(), "Failed to create Vulkan instance!");

		m_Instance = result.value();

		FL_CORE_INFO("Vulkan instance created");
	}

	void VulkanInstance::CreateSurface(GLFWwindow* window)
	{
		FL_CORE_ASSERT(
			glfwCreateWindowSurface(m_Instance.instance, window, nullptr, &m_Surface) == VK_SUCCESS,
			"Failed to create window surface!"
		);

		FL_CORE_INFO("Vulkan surface created");
	}

}