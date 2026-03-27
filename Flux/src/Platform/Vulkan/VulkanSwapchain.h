#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <VkBootstrap.h>

#include "Flux/Core.h"

#include <vector>



namespace Flux {

	class VulkanSwapchain
	{
	public:
		VulkanSwapchain() = default;
		~VulkanSwapchain();

		FL_NON_COPYABLE(VulkanSwapchain);

		void Init(vkb::Device& device, VkSurfaceKHR surface, GLFWwindow* window);
		void Recreate(GLFWwindow* window);

		inline VkSwapchainKHR          GetSwapchain()    const { return m_Swapchain.swapchain; }
		inline VkRenderPass            GetRenderPass()   const { return m_RenderPass; }
		inline VkExtent2D              GetExtent()       const { return m_Swapchain.extent; }
		inline VkFormat                GetFormat()       const { return m_Swapchain.image_format; }
		inline uint32_t                GetImageCount()   const { return (uint32_t)m_Images.size(); }
		inline VkFramebuffer           GetFramebuffer(uint32_t index) const { return m_Framebuffers[index]; }

		inline VkImage					     GetImage(uint32_t index) const { return m_Images[index]; }
		inline const std::vector<VkImage>&   GetImages() const { return m_Images; }

	private:
		void CreateRenderPass();
		void CreateFramebuffers();
		void Cleanup();

	private:
		vkb::Device* m_Device = nullptr;

		vkb::Swapchain             m_Swapchain;
		VkRenderPass               m_RenderPass = VK_NULL_HANDLE;

		std::vector<VkImage>       m_Images;
		std::vector<VkImageView>   m_ImageViews;
		std::vector<VkFramebuffer> m_Framebuffers;
	};

}