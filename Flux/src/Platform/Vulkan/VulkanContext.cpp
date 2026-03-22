#include "flpch.h"
#include "VulkanContext.h"
#include "Flux/Renderer/RendererAPI.h"


namespace Flux {

	VulkanContext* VulkanContext::s_Instance = nullptr;

	VulkanContext::VulkanContext(GLFWwindow* window)
		: m_Window(window)
	{
		FL_CORE_ASSERT(m_Window, "Window handle is null");
		s_Instance = this;
	}

	VulkanContext::~VulkanContext()
	{
		vkDeviceWaitIdle(m_Device.GetDevice());

		vmaDestroyAllocator(m_Allocator);

		for (size_t i = 0; i < m_ImageAvailableSemaphores.size(); i++)
		{
			vkDestroySemaphore(m_Device.GetDevice(), m_ImageAvailableSemaphores[i], nullptr);
			vkDestroyFence(m_Device.GetDevice(), m_InFlightFences[i], nullptr);
		}

		for (size_t i = 0; i < m_RenderFinishedSemaphores.size(); i++)
		{
			vkDestroySemaphore(m_Device.GetDevice(), m_RenderFinishedSemaphores[i], nullptr);
		}

		vkDestroyCommandPool(m_Device.GetDevice(), m_CommandPool, nullptr);
		vkDestroyDescriptorPool(m_Device.GetDevice(), m_DescriptorPool, nullptr);
	}

	void VulkanContext::Init()
	{
		FL_CORE_INFO("Renderer API: {0}", RendererAPI::GetAPIName());

		m_Instance.Init("Flux");
		m_Instance.CreateSurface(m_Window);

		m_Device.Init(m_Instance.GetVkbInstance(), m_Instance.GetSurface());

		VmaAllocatorCreateInfo allocatorInfo{};
		allocatorInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
		allocatorInfo.instance = m_Instance.GetInstance();
		allocatorInfo.physicalDevice = m_Device.GetPhysicalDevice();
		allocatorInfo.device = m_Device.GetDevice();
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;

		FL_CORE_ASSERT(
			vmaCreateAllocator(&allocatorInfo, &m_Allocator) == VK_SUCCESS,
			"Failed to create VMA allocator!"
		);

		m_Swapchain.Init(m_Device.GetVkbDevice(), m_Instance.GetSurface(), m_Window);

		CreateDescriptorPool();
		CreateCommandPool();
		CreateCommandBuffers();
		CreateSyncObjects();

		FL_CORE_INFO("Vulkan context ready");

	}

	void VulkanContext::CreateDescriptorPool()
	{
		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER,                1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1000 },
		};

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 1000;
		poolInfo.poolSizeCount = (uint32_t)std::size(poolSizes);
		poolInfo.pPoolSizes = poolSizes;

		FL_CORE_ASSERT(
			vkCreateDescriptorPool(m_Device.GetDevice(), &poolInfo, nullptr, &m_DescriptorPool) == VK_SUCCESS,
			"Failed to create descriptor pool!"
		);
	}

	void VulkanContext::CreateCommandPool()
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = m_Device.GetGraphicsQueueFamily();

		FL_CORE_ASSERT(
			vkCreateCommandPool(m_Device.GetDevice(), &poolInfo, nullptr, &m_CommandPool) == VK_SUCCESS,
			"Failed to create command pool!"
		);
	}

	void VulkanContext::CreateCommandBuffers()
	{
		m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

		FL_CORE_ASSERT(
			vkAllocateCommandBuffers(m_Device.GetDevice(), &allocInfo, m_CommandBuffers.data()) == VK_SUCCESS,
			"Failed to allocate command buffers!"
		);
	}

	void VulkanContext::CreateSyncObjects()
	{
		m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		uint32_t imageCount = m_Swapchain.GetImageCount();
		m_RenderFinishedSemaphores.resize(imageCount);

		VkSemaphoreCreateInfo semInfo{};
		semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkCreateSemaphore(m_Device.GetDevice(), &semInfo, nullptr, &m_ImageAvailableSemaphores[i]);
			vkCreateFence(m_Device.GetDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]);
		}

		for (uint32_t i = 0; i < imageCount; i++)
		{
			vkCreateSemaphore(m_Device.GetDevice(), &semInfo, nullptr, &m_RenderFinishedSemaphores[i]);
		}
	}

	void VulkanContext::BeginFrame()
	{
		vkWaitForFences(m_Device.GetDevice(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

		VkResult result = vkAcquireNextImageKHR(
			m_Device.GetDevice(),
			m_Swapchain.GetSwapchain(),
			UINT64_MAX,
			m_ImageAvailableSemaphores[m_CurrentFrame],
			VK_NULL_HANDLE,
			&m_CurrentImageIndex
		);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			m_Swapchain.Recreate(m_Window);
			m_FrameStarted = false;
			return; 
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
			FL_CORE_ASSERT(false, "Failed to acquire swapchain image!");

		vkResetFences(m_Device.GetDevice(), 1, &m_InFlightFences[m_CurrentFrame]);

		VkCommandBuffer cmd = m_CommandBuffers[m_CurrentFrame];
		vkResetCommandBuffer(cmd, 0);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkBeginCommandBuffer(cmd, &beginInfo);

		VkClearValue clearColor = { {0.0, 0.0, 0.0, 1.0} };
		VkRenderPassBeginInfo rpInfo{};
		rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpInfo.renderPass = m_Swapchain.GetRenderPass();
		rpInfo.framebuffer = m_Swapchain.GetFramebuffer(m_CurrentImageIndex); 
		rpInfo.renderArea.offset = { 0, 0 };
		rpInfo.renderArea.extent = m_Swapchain.GetExtent();
		rpInfo.clearValueCount = 1;
		rpInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkExtent2D extent = m_Swapchain.GetExtent();

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)extent.width;
		viewport.height = (float)extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(cmd, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = extent;
		vkCmdSetScissor(cmd, 0, 1, &scissor);

		float blendConstants[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		vkCmdSetBlendConstants(cmd, blendConstants);

		m_FrameStarted = true;
	}

	void VulkanContext::EndFrame()
	{
		if (!m_FrameStarted)
			return;		
		m_FrameStarted = false;

		VkCommandBuffer cmd = m_CommandBuffers[m_CurrentFrame];

		vkCmdEndRenderPass(cmd);
		vkEndCommandBuffer(cmd);

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &m_ImageAvailableSemaphores[m_CurrentFrame];
		submitInfo.pWaitDstStageMask = &waitStage;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;

		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &m_RenderFinishedSemaphores[m_CurrentImageIndex];

		FL_CORE_ASSERT(vkQueueSubmit(m_Device.GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) == VK_SUCCESS, "Failed to submit draw command buffer!");

		VkSwapchainKHR swapchain = m_Swapchain.GetSwapchain();

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &m_RenderFinishedSemaphores[m_CurrentImageIndex];
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.pImageIndices = &m_CurrentImageIndex;

		VkResult result = vkQueuePresentKHR(m_Device.GetGraphicsQueue(), &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) 
			m_Swapchain.Recreate(m_Window);
		
		else if (result != VK_SUCCESS) 
			FL_CORE_ASSERT(false, "Failed to present swapchain image!");
		

		m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

}