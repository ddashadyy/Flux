#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Flux/Core.h"

#include "Flux/Renderer/GraphicsContext.h"

#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"

#include <vector>

namespace Flux {

	class VulkanContext : public GraphicsContext
	{
	public:
		VulkanContext(GLFWwindow* window);
		~VulkanContext();

		FL_NON_COPYABLE(VulkanContext);

		void Init()       override;
		void BeginFrame() override;
		void EndFrame()   override;

		// Getters for ImGui
		inline VkInstance        GetInstance()        const { return m_Instance.GetInstance(); }
		inline VkPhysicalDevice  GetPhysicalDevice()  const { return m_Device.GetPhysicalDevice(); }
		inline VkDevice          GetDevice()          const { return m_Device.GetDevice(); }
		inline VkQueue           GetGraphicsQueue()   const { return m_Device.GetGraphicsQueue(); }
		inline VkRenderPass      GetRenderPass()      const { return m_Swapchain.GetRenderPass(); }
		inline VkDescriptorPool  GetDescriptorPool()  const { return m_DescriptorPool; }
		inline uint32_t          GetImageCount()      const { return m_Swapchain.GetImageCount(); }
		inline uint32_t          GetMinImageCount()   const { return 2; }
		inline VkCommandBuffer   GetCurrentCommandBuffer() const { return m_CommandBuffers[m_CurrentFrame]; }
		inline uint32_t			 GetGraphicsQueueFamily() const { return m_Device.GetGraphicsQueueFamily(); }

	private:
		void CreateDescriptorPool();
		void CreateCommandPool();
		void CreateCommandBuffers();
		void CreateSyncObjects();

	private:
		GLFWwindow* m_Window;

		VulkanInstance   m_Instance;
		VulkanDevice     m_Device;
		VulkanSwapchain  m_Swapchain;

		VkDescriptorPool             m_DescriptorPool = VK_NULL_HANDLE;
		VkCommandPool                m_CommandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> m_CommandBuffers;

		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence>     m_InFlightFences;

		uint32_t m_CurrentFrame = 0;
		uint32_t m_CurrentImageIndex = 0;

		bool m_FrameStarted = false;
	};

}