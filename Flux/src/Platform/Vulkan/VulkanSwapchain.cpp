#include "flpch.h"
#include "VulkanSwapchain.h"

#include "Flux/Log.h"
#include "Flux/Core.h"

namespace Flux {

	VulkanSwapchain::~VulkanSwapchain()
	{
		Cleanup();
	}

	void VulkanSwapchain::Init(vkb::Device& device, VkSurfaceKHR surface, GLFWwindow* window)
	{
		m_Device = &device;

		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		vkb::SwapchainBuilder builder(*m_Device);

		auto result = builder
			.set_desired_format({ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
			.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
			.set_desired_extent(width, height)
			.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			.set_composite_alpha_flags(VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
			.set_old_swapchain(m_Swapchain)
			.build();

		FL_CORE_ASSERT(result.has_value(), "Failed to create swapchain!");
		m_Swapchain = result.value();

		m_Images = m_Swapchain.get_images().value();
		m_ImageViews = m_Swapchain.get_image_views().value();

		CreateRenderPass();
		CreateFramebuffers();

		FL_CORE_INFO("Vulkan swapchain created ({0}x{1})", width, height);
	}

	void VulkanSwapchain::CreateRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_Swapchain.image_format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorRef{};
		colorRef.attachment = 0;
		colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		FL_CORE_ASSERT(
			vkCreateRenderPass(m_Device->device, &renderPassInfo, nullptr, &m_RenderPass) == VK_SUCCESS,
			"Failed to create render pass!"
		);
	}

	void VulkanSwapchain::CreateFramebuffers()
	{
		m_Framebuffers.resize(m_ImageViews.size());

		for (size_t i = 0; i < m_ImageViews.size(); i++)
		{
			VkFramebufferCreateInfo fbInfo{};
			fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			fbInfo.renderPass = m_RenderPass;
			fbInfo.attachmentCount = 1;
			fbInfo.pAttachments = &m_ImageViews[i];
			fbInfo.width = m_Swapchain.extent.width;
			fbInfo.height = m_Swapchain.extent.height;
			fbInfo.layers = 1;

			FL_CORE_ASSERT(
				vkCreateFramebuffer(m_Device->device, &fbInfo, nullptr, &m_Framebuffers[i]) == VK_SUCCESS,
				"Failed to create framebuffer!"
			);
		}
	}

	void VulkanSwapchain::Recreate(GLFWwindow* window)
	{
		int width = 0, height = 0;
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(m_Device->device);
		Cleanup();
		m_Swapchain = vkb::Swapchain{}; 

		vkb::SwapchainBuilder builder(*m_Device);

		auto result = builder
			.set_desired_format({ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
			.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
			.set_desired_extent(width, height)
			.set_old_swapchain(m_Swapchain)
			.build();

		FL_CORE_ASSERT(result.has_value(), "Failed to recreate swapchain!");

		m_Swapchain = result.value();
		m_Images = m_Swapchain.get_images().value();
		m_ImageViews = m_Swapchain.get_image_views().value();

		CreateRenderPass();
		CreateFramebuffers();

		FL_CORE_INFO("Vulkan swapchain recreated ({0}x{1})", width, height);
	}

	void VulkanSwapchain::Cleanup()
	{
		if (m_Device == nullptr)
			return;

		vkDeviceWaitIdle(m_Device->device);

		for (auto fb : m_Framebuffers)
			vkDestroyFramebuffer(m_Device->device, fb, nullptr);
		m_Framebuffers.clear();

		if (m_RenderPass != VK_NULL_HANDLE)
		{
			vkDestroyRenderPass(m_Device->device, m_RenderPass, nullptr);
			m_RenderPass = VK_NULL_HANDLE;
		}

		m_Swapchain.destroy_image_views(m_ImageViews);
		vkb::destroy_swapchain(m_Swapchain);
	}

}