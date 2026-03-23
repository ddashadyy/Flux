#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <VkBootstrap.h>

#include "Flux/Core.h"

namespace Flux {

	class VulkanInstance
	{
	public:

		VulkanInstance() = default;
		~VulkanInstance();

		FL_NON_COPYABLE(VulkanInstance);

		void Init(const char* appName = "Flux");

		inline VkInstance       GetInstance() const { return m_Instance.instance; }
		inline VkSurfaceKHR     GetSurface()  const { return m_Surface; }
		inline vkb::Instance&   GetVkbInstance() { return m_Instance; }

		void CreateSurface(GLFWwindow* window);

	private:
		vkb::Instance m_Instance;
		VkSurfaceKHR  m_Surface = VK_NULL_HANDLE;
	};

}